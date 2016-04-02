// NanoDotApp.h (nanodot)
// e.moon apr98

#include <Application.h>
#include <SupportKit.h>
#include "NanoDotWindow.h"
#include <MidiKit.h>

#ifndef __NANODOTAPP_H__
#define __NANODOTAPP_H__

#include "PortEntry.h"

class NanoDotApp : public BApplication {

public: // METHODS
	NanoDotApp();
	~NanoDotApp();

	// signals that a window has been added:
	void addWindow(NanoDotWindow* pWnd); 
	
	// signals that a window has been removed;
	// if this was the last window, the application
	// should quit:
	void removeWindow(NanoDotWindow* pWnd);
	
	// returns the internal Midi synth object (only one of these
	// can exist in a given application)
	BMidiSynth* getSynth() { return m_pMidiSynth; }

	// returns all MIDI ports found:	
	BList* /* of PortEntry */ getPortList() { return &m_portList; }

protected: // implementation
	// initialize the internal MIDI synth
	void initSynth();

	// build a list of available MIDI ports (and open each one)
	void enumeratePorts();
		
private: // IMPLEMENTATION

	// all my windows:
	// +++++ is this list maintained by the OS?	
	BList m_windowList;
	
	// the built-in MIDI synth:
	BSynth m_synth;
	BMidiSynth* m_pMidiSynth;
	
	// available MIDI ports
	BList /* of PortEntry */ m_portList;

};

#endif /* __NANODOTAPP_H__ */
