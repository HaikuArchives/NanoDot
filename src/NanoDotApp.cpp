// NanoDotApp.cpp (nanodot)
// e.moon apr98

#include <stdio.h>
#include <Application.h>
#include <StorageKit.h>
#include "NanoDotApp.h"
#include "NanoDotWindow.h"
#include "NanoError.h"

int main() {
	NanoDotApp app;
	app.Run();
	return 0;
}

//---------------------------------------------------------------//

// c'tor: starts app by adding an initial window:
NanoDotApp::NanoDotApp() :
	BApplication("application/x-vnd.eamoon-nanodot-r1"),
	m_pMidiSynth(NULL) {
	
	// initialize synth:
	initSynth();
	
	// get a list of available ports:
	enumeratePorts();

	// add first window:
	addWindow(new NanoDotWindow());
}

// d'tor:
// - delete Midi synth object if necessary:
NanoDotApp::~NanoDotApp() {
	// delete ports:
	for(int nCount = 0; nCount < m_portList.CountItems(); nCount++) {
		// fetch entry:
		PortEntry* pCur = (PortEntry*)m_portList.ItemAt(nCount);
		
		// deallocate port object (unless it's the internal synth)
		if(pCur->m_pPort != m_pMidiSynth) {
			delete (BMidiPort*)pCur->m_pPort;
		}
		delete pCur;
	}

	// delete synth
	if(m_pMidiSynth != NULL)
		delete m_pMidiSynth;
		
}

// adds a window to the internal window-list:
void NanoDotApp::addWindow(NanoDotWindow* pWnd) {
	m_windowList.AddItem(pWnd);
}

// removes the given window; if no more exist, quits:
void NanoDotApp::removeWindow(NanoDotWindow* pWnd) {

	// remove; bail if unsuccessful:
	if(!m_windowList.RemoveItem(pWnd))
		NanoError("NanoDotApp::removeWindow(): window not in list!").displayAndQuit();

	// if that was the last window, kill myself:
	if(m_windowList.CountItems() == 0)		
		PostMessage(B_QUIT_REQUESTED);
}

//---------------------------------------------------------------//

void NanoDotApp::initSynth() {

	// load patches
	status_t ret = m_synth.LoadSynthData(B_BIG_SYNTH);
	if(ret)
		return;
	
	// create port object
	m_pMidiSynth = new BMidiSynth();
	ret = m_pMidiSynth->EnableInput(true, true);
	if(ret) {
		// whoops, that didn't work; destroy port object
		delete m_pMidiSynth; m_pMidiSynth = NULL;
	}	
}

// counts available midi ports by listing all entries in /dev/midi
// 3june98 eamoon: added MIDI synth for the module-impaired
// 16oct98 eamoon: migrated to NanoDotApp class

void NanoDotApp::enumeratePorts() {

	BMidiPort p;
	int32 nDevices = p.CountDevices();
	char* pCurName = new char[B_OS_NAME_LENGTH];
	for(int32 n = 0; n < nDevices; n++) {	
		BMidiPort* pNewPort = new BMidiPort();
		if(p.GetDeviceName(n, pCurName) != B_NO_ERROR)
			continue;
		status_t ret = pNewPort->Open(pCurName);

		// make sure it opened ok
		if(ret != B_NO_ERROR) {
			NanoError("SeqGlobalView(): BMidiPort::Open() failed\n").displayAndQuit();
		}

		// add to list of ports:
		m_portList.AddItem(new PortEntry(pCurName, pNewPort));
	}
	
	delete [] pCurName;

	// Add internal synth:
	
	if(m_pMidiSynth != NULL)
		m_portList.AddItem(new PortEntry("[synth]", m_pMidiSynth));
}


// END -- NanoDotApp.cpp --