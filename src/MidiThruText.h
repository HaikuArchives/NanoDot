// MidiThruText.h (nanodot)
// extends BMidiText to pass events through
// (only note-on and note-off for the moment)
// e.moon 19apr98

// TO DO
// +++++ override the rest of the event handlers

#ifndef __MIDITHRUTEXT_H__
#define __MIDITHRUTEXT_H__

#include <MidiKit.h>

class MidiThruText : public BMidiText {
public:
	MidiThruText() :
		m_bEcho(true) {;}
	~MidiThruText() {;}
	
	void echoText(bool bEcho) { m_bEcho = bEcho; }
	
	virtual void NoteOn(uchar channel, 
         uchar note, 
         uchar velocity, 
         ulong time = B_NOW) {
    // spray FIRST; don't delay
    SprayNoteOn(channel,note,velocity,time);
    if(m_bEcho)
	    BMidiText::NoteOn(channel,note,velocity,/*time*/B_NOW);
  }

	virtual void NoteOff(uchar channel, 
          uchar note, 
          uchar velocity, 
          ulong time = B_NOW) {
    SprayNoteOff(channel,note,velocity,time);
    if(m_bEcho)
	    BMidiText::NoteOff(channel,note,velocity,/*time*/B_NOW);
  }
  
protected: // MEMBERS
	bool m_bEcho;
};

#endif /* __MIDITHRUTEXT_H__ */
