// SeqGlobal.h (nanodot)
// Handles timing of sequence playback & hands off to
// Track objects for actual note generation
// e.moon apr98

// +++++ handle cloning

#ifndef __SEQGLOBAL_H__
#define __SEQGLOBAL_H__

#include <MidiKit.h>
#include <KernelKit.h>
#include "Track.h"
#include "MidiThruText.h"
#include "NanoError.h"

#include <string.h>
#include "PortEntry.h"

class SeqGlobal : public BMidi {
	
public: // interface
	SeqGlobal(BMidiSynth* pSynth=NULL);
	~SeqGlobal();
	
	// copy/clone c'tor & operator:
	SeqGlobal(SeqGlobal& clone); // NYI
	SeqGlobal& operator=(SeqGlobal& clone); // NYI

	void addTrack(Track* pTrack);
	BList* /* of Track */ getTrackList() { return &m_trackList; }

	// tempo is measured in thousandths of a BPM
	void setTempo(int32 nTempo);
	int32 getTempo() { return m_nTempo; }
	
	void setPort(PortEntry* pPortEntry);
	PortEntry* getPort() { return m_pCurPortEntry; }
	
	void setChannel(int16 nChannel);
	int16 getChannel() { return m_nCurChannel; }
	
	void setOffset(double fOffset) { m_fTrackOffset = fOffset; }
	
	// mute controller
	void setMuted(bool bMuted);
	bool isMuted() const { return m_bMuted; }
	
	// am I auto-starting:
	bool isStarting() const { return m_bStarting; }
	
	// reset all tracks' position pointers
	void rewind();
	
	// set all tracks' position pointers to the given value
	void setPos(uint16 nPos);
	
	// set the time the first note will be played (used for auto-start)
	void setStartTime(double fStartTime) { m_fStartTime = fStartTime; }
	
	// calculate the delay for one 'tick' or sequencer step:
	double tempoDelay(int nDivision=4); // default division (16th notes)

	// Start(): overridden to start all tracks, etc.
	status_t Start();
	
	void Stop();
	
	// main loop:
	void Run();

	
	// convenience functions: lock & unlock tempo/parameters
	void lockTempo() {
		if(acquire_sem(m_lockTempo) != B_NO_ERROR)
			NanoError("SeqGlobal::AddTrack(): couldn't acquire m_lockTempo").
				displayAndQuit();
	}
	
	void unlockTempo() {
		if(release_sem(m_lockTempo) != B_NO_ERROR)
			NanoError("SeqGlobal::addTrack(): couldn't release m_lockTempo").
				displayAndQuit();
	}

	// lock first!
	double thisTick() const { return m_fThisTick; }
	double nextTick() const { return m_fNextTick; }
	double period() const { return m_fPeriod; }

	uint16 pos() { lockTempo(); uint16 nRet = m_nPos;
		unlockTempo(); return nRet; }
		
	uint16 fetch(double& fThis, double& fNext, double& fPeriod) {
		lockTempo();
		fThis = m_fThisTick;
		fNext = m_fNextTick;
		fPeriod = m_fPeriod;
		uint16 nRet = m_nPos;
		unlockTempo();
		return nRet;
	}

protected: // helpers
	
	// do basic c'tor initialization:
	void init();
	
	// clone self from given SeqGlobal:
	void initClone(SeqGlobal& clone);
	
	// create a default set of tracks:
	void initDefaultTracks();

	// register another SeqGlobal object to be automatically started
	// on the next tick
	// (super-primitive synchronization!)
	
	void registerClone(SeqGlobal* pClone); // NYI
	
protected: // STATIC MEMBERS
	static const int16 s_nNumDefaultTracks;

private: // implementation
	// internal synth
	BMidiSynth*		m_pMidiSynth;
	
	// current port
	PortEntry* m_pCurPortEntry;
	
	// merge object (connects to current port)
	MidiThruText* m_pMidiText;

	// current channel (used for new tracks)
	int m_nCurChannel;

	// muted? (disconnects from output port)
	bool m_bMuted;
	
	// the tracks
	BList m_trackList;
	
	// clones to automatically start:
	BList m_registeredCloneList;
	
	// If this sequencer was cloned while its parent was running, m_bStarting
	// will be true until Start() is called, and fStartTime will be set
	// to the time at which the first note should be sent
	bool m_bStarting;
	double m_fStartTime;

	double			m_fTrackOffset;

	// tempo lock object
	sem_id m_lockTempo;

	// current tempo:
	int32 m_nTempo;

	// (when playing only) the current tick (in milliseconds)
	double		m_fThisTick;
	
	// the next tick (milliseconds)
	double		m_fNextTick;
	
	// tick length
	double		m_fPeriod;
	
	// tick position
	uint16	m_nPos;
};

#endif /* __SEQGLOBAL_H__ */
