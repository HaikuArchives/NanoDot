// Track.h (nanodot)
// e.moon apr98

// TO DO +++++
// send note off on close (clean up tickXXX() interface)

#ifndef __TRACK_H__
#define __TRACK_H__

#include <MidiKit.h>
#include <Alert.h>
#include <assert.h>

#include <Messenger.h>
const int32 TRACK_SEQ_STEP		= 'trSt';

// Track: generates a repeating sequence of note on/off commands.

class Track : public BMidi {
public: // interface
	Track();
	~Track();
	
	// copy c'tor/operator (needed for inclusion in a
	// container)
	Track(const Track& clone);
	Track& operator=(const Track& clone);
	
	void setSeqGlobal(class SeqGlobal* pSeqGlobal, double fOffset) {
		m_pSeqGlobal = pSeqGlobal; m_fOffset = fOffset; }
	
	void setPort(BMidi* pPort);
	BMidi* getPort() {
		return m_pOutPort;
	}
	
	void tickOn(ulong nWhen);
	void tickOff(ulong nWhen);
	
	void setData(uint16 nPos, bool bValue);
	bool getData(uint16 nPos) {
		return (nPos < m_nSeqLength) ?
			m_pbSeq[nPos] :
			false;
	}
	
	void setPitch(uint16 nPitch) {
		lockParams();
		m_nPitch = nPitch;
		unlockParams();
	}
	uint16 getPitch() { return m_nPitch; }
	
	void setChannel(uint16 nChannel) {
		lockParams();
		m_nChannel = nChannel;
		unlockParams();
	}
	uint16 getChannel() { return m_nChannel; }
	
	uint16 getSeqLength() { return m_nSeqLength; }
	
	// set position:
	void setPos(uint16 nPos) { 
		lockParams(); m_nSeqPos = nPos; unlockParams();
	}
	
	uint16 getPos() {
		lockParams();
		uint16 nPos = m_nSeqPos;
		unlockParams();
		return nPos;
	}
	
	// 'callback' messenger for runtime messages
	void setMessenger(BMessenger* pMsgr) {
		m_pMsgr = pMsgr;
	}
	
	// implementation
	void Run(); //nyi

protected: // helpers

	// common c'tor code:
	void init(); 
	
	// lock/unlock helpers:
	void lockParams() {
		if(acquire_sem(m_lockParams) != B_NO_ERROR) {
			// error; alert:
			(new BAlert("LOCK ERROR", "Couldn't acquire m_lockParams",
				"Ouch", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			return;
		}
	}
	
	void unlockParams() {
		if(release_sem(m_lockParams) != B_NO_ERROR) {
			// error; alert:
			(new BAlert("LOCK ERROR", "Couldn't release m_lockParams",
				"Ouch", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			return;
		}
	}

protected: // members
	static const uint16 s_nDefaultSeqLength;
			
private: // implementation

	uchar m_nChannel; // note channel
	uchar m_nPitch;   // note pitch

	uint16 m_nSeqLength;
	bool* m_pbSeq; // sequence
	uint16 m_nSeqPos; // position in the sequence
	
	// the output port:
	BMidi* m_pOutPort;
	
	class SeqGlobal* 	m_pSeqGlobal;
	double							m_fOffset;
	
	// semaphore for control of param changes:
	sem_id m_lockParams;
	
	BMessenger*				m_pMsgr;
};

#endif /* __TRACK_H__ */
