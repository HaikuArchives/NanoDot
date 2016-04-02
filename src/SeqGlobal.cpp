// SeqGlobal.cpp (nanodot)
// e.moon apr98

#include <math.h>
#include <stdio.h>

#include "SeqGlobal.h"
#include "NanoDotApp.h"

#include <StorageKit.h>

// statics

/*static*/
const int16 SeqGlobal::s_nNumDefaultTracks = 4;

// c'tor

SeqGlobal::SeqGlobal(BMidiSynth* pMidiSynth /*=NULL*/) :
	m_nTempo(120000),
	m_nCurChannel(1),
	m_bMuted(false),
	m_pCurPortEntry(NULL),
	m_bStarting(false),
	m_fStartTime(0.0),
	m_pMidiSynth(pMidiSynth) {

	// do common initialization:
	init();
	
	// create default tracks:
	initDefaultTracks();
}

SeqGlobal::~SeqGlobal() {
	if(IsRunning())
		Stop();

	// delete tracks:
	// (this disconnects them from m_pMidiText)
	for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
		Track* pCur = (Track*)m_trackList.ItemAt(nCount);
		delete pCur; // ok
	}

	// delete trace/thru object (disconnects it from port, if
	// connected)
	delete m_pMidiText;		

	delete_sem(m_lockTempo);
}

//----------------------------------------------------------------------//
// copy c'tor & operator=

SeqGlobal::SeqGlobal(SeqGlobal& clone) :
	m_nTempo(clone.m_nTempo),
	m_nCurChannel(clone.m_nCurChannel),
	m_bMuted(false),
	m_pCurPortEntry(NULL),
	m_bStarting(false), // NOTE: might be set to true in initClone()
	m_fStartTime(0.0) {

	// do common initialization:
	init();

	// do clone initialization:
	initClone(clone);
}


//----------------------------------------------------------------------//

// basic c'tor initialization:
void SeqGlobal::init() {

	m_lockTempo = create_sem(1, "m_lockTempo");
	if(m_lockTempo < B_NO_ERROR) {
		NanoError("SeqGlobal (ctor): couldn't create m_lockTempo").displayAndQuit();
	}

	// create the output 'plug' (the output port is connected
	// to this object to simplify port changes)
	m_pMidiText = new MidiThruText();
	
	// set to 'true' to display events as they are output
	m_pMidiText->echoText(false);	
}

// clone initialization
void SeqGlobal::initClone(SeqGlobal& clone) {

	// copy & add clone's tracks:
	int nLast = clone.m_trackList.CountItems();
	for(int n = 0; n < nLast; n++) {
		Track* pCur = (Track*)clone.m_trackList.ItemAt(n);
		addTrack(new Track(*pCur));
	}
	
	// set port:
	m_pCurPortEntry = clone.m_pCurPortEntry;
	
	// register with clone for auto-start (only if clone is
	// currently playing)
	if(clone.IsRunning()) {
		m_bMuted = true;
		m_bStarting = true;
		clone.registerClone(this);
	}
}


// create a default set of tracks:
void SeqGlobal::initDefaultTracks() {
	for(int16 n = 0; n < s_nNumDefaultTracks; n++)
		addTrack(new Track());
}

// register a clone to be auto-started
// NOTE: fails loudly if not currently running

void SeqGlobal::registerClone(SeqGlobal* pClone) {
	// sanity check
	if(!IsRunning())
		NanoError("SeqGlobal::registerClone(): NOT RUNNING!").displayAndQuit();
			
	lockTempo();
	m_registeredCloneList.AddItem(pClone);
	unlockTempo();
}

//----------------------------------------------------------------------//

void SeqGlobal::addTrack(Track* pTrack) {

	lockTempo();
	
	// set port:
	pTrack->setPort(m_pMidiText);

	// add to end of track-list
	m_trackList.AddItem(pTrack);

	unlockTempo();	
}

// a rather straightforward method
void SeqGlobal::setTempo(int32 nTempo) {

	lockTempo();
	m_nTempo = nTempo;
	unlockTempo();
}

//----------------------------------------------------------------------//

void SeqGlobal::setPort(PortEntry* pPortEntry) {

	// routed after text-display:
	// disconnect old;
	if(m_pCurPortEntry != NULL && !m_bMuted)
		m_pMidiText->Disconnect(m_pCurPortEntry->m_pPort);
	
	// connect new
	m_pCurPortEntry = pPortEntry;
	
	if(!m_bMuted)
		m_pMidiText->Connect(m_pCurPortEntry->m_pPort);
}

void SeqGlobal::setChannel(int16 nChannel) {
	m_nCurChannel = nChannel;
	
	// walk tracks, setting channel for each:
	for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
		Track* pCur = (Track*)m_trackList.ItemAt(nCount);
		pCur->setChannel(nChannel);
	}
}

//----------------------------------------------------------------------//

void SeqGlobal::setMuted(bool bMuted) {
	if(m_bMuted == bMuted)
		return; // nothing to do
	
	// +++++ lock?
	if(m_pCurPortEntry != NULL) {
		if(bMuted)
			// disconnect:
			m_pMidiText->Disconnect(m_pCurPortEntry->m_pPort);
		else
			// connect:
			m_pMidiText->Connect(m_pCurPortEntry->m_pPort);
	}
	
	m_bMuted = bMuted;
	// +++++ unlock?
}
	

//----------------------------------------------------------------------//

// rewind(): resets all tracks' position pointers:
void SeqGlobal::rewind() {
	for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
		Track* pCur = (Track*)m_trackList.ItemAt(nCount);
		pCur->setPos(0);
	}
}

// setPos(): sets all tracks' position pointers:
void SeqGlobal::setPos(uint16 nPos) {
	for(int n = 0; n < m_trackList.CountItems(); n++) {
		((Track*)m_trackList.ItemAt(n))->setPos(nPos);
	}
}

status_t SeqGlobal::Start()  {

	if(m_fStartTime != 0.0)
		SnoozeUntil(m_fStartTime);
	m_fStartTime = 0.0;

	m_nPos = 0;
	m_fThisTick = B_NOW;
	m_fPeriod = tempoDelay();
	m_fNextTick = m_fThisTick + m_fPeriod;

	// start tracks
	for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
		Track* pCur = (Track*)m_trackList.ItemAt(nCount);
		pCur->setSeqGlobal(this, nCount * m_fTrackOffset);
		pCur->Start();
	}

	// rather misleading:
	m_bStarting = false;
	
	return BMidi::Start();
}

void SeqGlobal::Stop() {

	// stop tracks
	for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
		Track* pCur = (Track*)m_trackList.ItemAt(nCount);
		pCur->Stop();
	}


	BMidi::Stop();
}

void SeqGlobal::Run() {

	//printf("SeqGlobal::Run(): %f (%f)\n", fNext, (double)B_NOW);
	
	// keep track of sequence position, and fetch
	// the sequence length from the first track.
	
	// this is cheating a bit, since in theory tracks could
	// be of different lengths (maybe that means that theory
	// is wrong :P)

	if(m_trackList.CountItems() == 0) {
		// no tracks, no start:
		return;
	}
		
	uint16 nPos = ((Track*)m_trackList.ItemAt(0))->getPos();
	uint16 nLength = ((Track*)m_trackList.ItemAt(0))->getSeqLength();
	//printf("  start pos: %d\n", nPos);
		
	// the main loop:
	bool bOn = true;
	
	double fMargin = m_fPeriod / 2;
	
	while(KeepRunning()) {

		SnoozeUntil(m_fNextTick-fMargin);

		// figure next tick:
		
		if(acquire_sem(m_lockTempo) != B_NO_ERROR)
			break;

		if(++m_nPos >= nLength) {
			m_nPos = 0;
			//printf("  [%f]\n", fNext);
		}

		m_fThisTick = m_fNextTick;
		m_fPeriod = tempoDelay();
		m_fNextTick = m_fThisTick + m_fPeriod;
		fMargin = m_fPeriod / 2;

		// auto-start any queued clones:	
		int nClones = m_registeredCloneList.CountItems();
		if(m_nPos == 0 && nClones > 0) {

			for(int n = 0; n < nClones; n++) {
				SeqGlobal* pCur = (SeqGlobal*)m_registeredCloneList.ItemAt(n);				
				pCur->setStartTime(m_fThisTick);
				pCur->Start();
			}
		
			// empty the list:
			m_registeredCloneList.MakeEmpty();
		}
		
		if(release_sem(m_lockTempo) != B_NO_ERROR)
			break;

/*
		double fOff = m_fThisTick + (m_fPeriod/2);
					
		for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
			Track* pCur = (Track*)m_trackList.ItemAt(nCount);
			pCur->tickOn(B_NOW);
		}
		SnoozeUntil(fOff);
		for(int nCount = 0; nCount < m_trackList.CountItems(); nCount++) {
			Track* pCur = (Track*)m_trackList.ItemAt(nCount);
			pCur->tickOff(B_NOW);
		}
*/
	}
}

//----------------------------------------------------------------------//


// returns a delay (in milliseconds) for the current tempo
// (in thousandths of a BPM) and the given note division
// (defaults to 4, which is a 16th note)

double SeqGlobal::tempoDelay(int nDivision) {
	return (double)60000000 / (m_nTempo * nDivision); 
}

// -- end SeqGlobal.cpp --