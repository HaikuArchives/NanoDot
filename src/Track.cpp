// Track.cpp (nanodot)
// e.moon apr98

#include <stdio.h>

#include "Track.h"
#include "SeqGlobal.h"

/* static */
const uint16 Track::s_nDefaultSeqLength = 16;

// c'tor, d'tor:

Track::Track() :
	m_nSeqLength(s_nDefaultSeqLength),
	m_nSeqPos(0),
	m_nChannel(1),
	m_nPitch(36), // C 1
	m_pOutPort(NULL),
	m_pMsgr(NULL) {
	
	init();
	
	// clear the pattern:
	for(uint16 nCount = 0; nCount < m_nSeqLength; nCount++)
		m_pbSeq[nCount] = false;
}

// copy c'tor
Track::Track(const Track& clone) :
	m_nSeqLength(clone.m_nSeqLength),
	m_nSeqPos(clone.m_nSeqPos),
	m_nChannel(clone.m_nChannel),
	m_nPitch(clone.m_nPitch),
	m_pOutPort(NULL),
	m_pMsgr(NULL) {

	init();
	
	// copy pattern:
	memcpy(m_pbSeq, clone.m_pbSeq, m_nSeqLength*sizeof(bool));
}

// copy operator
Track& Track::operator=(const Track& clone) {

	m_nSeqLength = clone.m_nSeqLength;
	m_nSeqPos = clone.m_nSeqPos;
	m_nChannel = clone.m_nChannel;
	m_nPitch = clone.m_nPitch;
	m_pOutPort = NULL;
	m_pMsgr = NULL;
	
	// copy pattern:
	memcpy(m_pbSeq, clone.m_pbSeq, m_nSeqLength*sizeof(bool));

	return *this;
}

Track::~Track() {
	// delete pattern:
	delete [] m_pbSeq;
	
	// delete semaphore
	delete_sem(m_lockParams);
	
	// clean up messenger
	if(m_pMsgr)
		delete m_pMsgr;
}

// c'tor helper
void Track::init() {

	// alloc sequence:
	m_pbSeq = new bool[m_nSeqLength];

	// create semaphore:
	m_lockParams = create_sem(1, "m_lockParams");
	if(m_lockParams < B_NO_ERROR) {
		(new BAlert("LOCK ERROR", "Couldn't create m_lockParams",
			"Ouch", NULL, NULL,	B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
		return;
	}
}

void Track::setPort(BMidi* pPort) {
	lockParams();
	
	// unplug the cable (if necessary)
	if(m_pOutPort != NULL)
		Disconnect(m_pOutPort);
		
	m_pOutPort = pPort;
	
	// plug it back in (if necessary)
	if(m_pOutPort != NULL)
		Connect(m_pOutPort);
	
	unlockParams();
}

//----------------------------------------------------------------------//

void Track::Run() {
//	printf("Track::Run()\n");
	// the main loop:

	double fPeriod, fThis, fNext;
	
	while(KeepRunning()) {

		// sync:
		int nWaitCount = 0;
		while(KeepRunning() && m_nSeqPos > m_pSeqGlobal->fetch(fThis, fNext, fPeriod)) {
			nWaitCount++;
			snooze(500);
		}
		//printf("%d: %d\n", m_nSeqPos, nWaitCount);

		if(m_pMsgr) {
			BMessage msg = new BMessage(TRACK_SEQ_STEP);
			msg.AddInt16("pos", m_nSeqPos);
			m_pMsgr->SendMessage(&msg);
		}

		double fOff = fThis + (fPeriod/2);

		// tick:
		tickOn((ulong)fThis+m_fOffset);
		tickOff((ulong)fOff+m_fOffset);

/*		if(++m_nSeqPos >= m_nSeqLength) {
			m_nSeqPos = 0;
			//printf("  [%f]\n", fNext);
		} */
		SnoozeUntil((ulong)fNext);	
	}
}

void Track::tickOn(ulong nWhen) {
	// play one sequence position's worth:
	SnoozeUntil(nWhen);
	if(m_pbSeq[m_nSeqPos]) {
		//printf("tickOn: %ld\n", nWhen);
		SprayNoteOn(m_nChannel, m_nPitch, 127, nWhen);
	}
}

void Track::tickOff(ulong nWhen) {
	SnoozeUntil(nWhen);
	if(m_pbSeq[m_nSeqPos]) {
		//printf("tickOff: %ld\n", nWhen);
		SprayNoteOff(m_nChannel, m_nPitch, 0, nWhen);
	}
		
	// increment position in sequence:
	if(++m_nSeqPos > m_nSeqLength-1)
		m_nSeqPos = 0;
}


void Track::setData(uint16 nPos, bool bValue) {

	lockParams();
	
	assert(nPos < 16);
	m_pbSeq[nPos] = bValue;

	unlockParams();
}

// -- end Track.cpp --