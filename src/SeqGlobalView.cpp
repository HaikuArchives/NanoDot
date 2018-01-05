// SeqGlobalView.cpp (nanodot)
// e.moon apr98

#include <PopUpMenu.h>
#include <StorageKit.h>
#include "SeqGlobalView.h"
#include "NanoDotApp.h"

#include "InvocableMenuItem.h"

#include <stdio.h>
#include <stdlib.h>


// sets up a SeqGlobalView member:
// - stores a reference to the given SeqGlobal instance
// - hands off to the parent c'tor
// - enumerates available MIDI ports & picks the first
// - populates the view with components


// c'tor helper: adds a menu item to the given menu for each
// port in the given list
/*static*/
void SeqGlobalView::addPortMenuItems(BMenu* pMenu, BList& list) {

	// for each port:
	for(int nCount = 0; nCount < list.CountItems(); nCount++) {
		// fetch entry (name & BMidiPort object)
		PortEntry* pCur = (PortEntry*)list.ItemAt(nCount);

		// create message to select this port				
		BMessage* pMsg = new BMessage(SEQGLOBAL_SET_PORT);
		pMsg->AddString("port-name", pCur->m_pszName);
		pMsg->AddPointer("port-entry", pCur);
		
		// add a menu item to trigger the above message:
		// (by default, the target of the message will be the owning window,
		// which is overridden in the AllAttached() method)
		InvocableMenuItem* pItem = new InvocableMenuItem(pCur->m_pszName, pMsg);
		pMenu->AddItem(pItem);
	}
}

/*static*/
void SeqGlobalView::addChannelMenuItems(BMenu* pMenu) {
	char buffer[16];
	for(int16 n = 1; n <= 16; n++) {
		// create message to select this port				
		BMessage* pMsg = new BMessage(SEQGLOBAL_SET_CHANNEL);
		pMsg->AddInt16("channel", n);
		
		// add a menu item to trigger the above message:
		sprintf(buffer, "%02d", n);
		InvocableMenuItem* pItem = new InvocableMenuItem(buffer, pMsg);
		pMenu->AddItem(pItem);
	}
}

// c'tor
SeqGlobalView::SeqGlobalView(SeqGlobal& seqGlobal, BRect frame) :
	m_seqGlobal(seqGlobal),
	BView(frame, "SeqGlobalView", B_FOLLOW_TOP|B_FOLLOW_LEFT,
		B_WILL_DRAW),
	m_pOffsetField(0) {
	
	// set up port menu-field:		
	BPopUpMenu* pMenu = new BPopUpMenu("port-menu");
		
	// populate the menu:
	addPortMenuItems(pMenu, *((NanoDotApp*)be_app)->getPortList());
	
	// build & add the control:
	BRect curFrame(2, 6, 140, 24);
	m_pPortField = new BMenuField(curFrame,
		"port-menu-field", "Port:", pMenu, B_FOLLOW_TOP, B_WILL_DRAW);	
	AddChild(m_pPortField);

/* removed; used for testing Be MIDI services.
   when enabled, the offset field sets the time (in milliseconds)
   by which each track is offset from the last.
   
  (this may turn into a swing/quantize control some day, hope
   hope...)
   
	// set up offset field
	BRect oFrame = curFrame; oFrame.OffsetBy(0.0, 24.0);
	m_pOffsetField = new BTextControl(oFrame,
		"offset-field", "Offset (ms):", "0.0", new BMessage(SEQGLOBAL_SET_OFFSET));
	AddChild(m_pOffsetField);
*/

	// set up channel menu-field:
	pMenu = new BPopUpMenu("channel-menu");
	
	// populate:
	addChannelMenuItems(pMenu);
	
	// create control:
	curFrame.OffsetBy(162, 0);
	curFrame.right -= 20;
	m_pChannelField = new BMenuField(curFrame,
		"channel-menu-field", "Channel:", pMenu, B_FOLLOW_TOP, B_WILL_DRAW);
	AddChild(m_pChannelField);


	// build & add clone button:
	curFrame.OffsetBy(125, -2);
	curFrame.right = curFrame.left + 60;

	m_pCloneButton = new BButton(curFrame, "clone-button", "Clone",
		new BMessage(SEQGLOBAL_CLONE));
	AddChild(m_pCloneButton);
	
	// build & add mute button:
	curFrame.OffsetBy(70, 0);
	m_pMuteButton = new BButton(curFrame, "mute-button",
		(m_seqGlobal.isMuted()) ? "Unmute" : "Mute",
		new BMessage(SEQGLOBAL_MUTE));

	AddChild(m_pMuteButton);
		
	// build & add start/stop button:	
	// (it should be labeled 'Stop' if the sequencer is in the
	// process of starting; ie. being auto-started from its clone/parent)
	
	curFrame.OffsetBy(70, 0);
	curFrame.right += 16;
	m_pStartStopButton = new BButton(curFrame, "start-button",
		(m_seqGlobal.isStarting()) ? "Stop" : "Start",
		new BMessage(SEQGLOBAL_STARTSTOP));

	AddChild(m_pStartStopButton);
	
	// prepare for drawing; set color:
	rgb_color bgGray = {219,219,219,255};
	SetViewColor(bgGray);
}

SeqGlobalView::~SeqGlobalView() {;}


//----------------------------------------------------------------------//

// handleStartStop(): deal with the start/stop button
void SeqGlobalView::handleStartStop(BMessage* pMsg) {
	handleSetOffset(NULL);
	
	// toggle playback:
	if(m_seqGlobal.IsRunning()) {
		m_seqGlobal.Stop();
		m_pStartStopButton->SetLabel("Start");
	} else {
		m_seqGlobal.rewind();
		m_seqGlobal.Start();
		m_pStartStopButton->SetLabel("Stop");
	}
}

// handleMute(): toggle muting
void SeqGlobalView::handleMute(BMessage* pMsg) {
	if(m_seqGlobal.isMuted()) {
		m_seqGlobal.setMuted(false);
		m_pMuteButton->SetLabel("Mute");
	} else {
		m_seqGlobal.setMuted(true);
		m_pMuteButton->SetLabel("Unmute");
	}
}

void SeqGlobalView::handleSetOffset(BMessage* pMsg) {
	if(m_pOffsetField) {
		double fVal = atof(m_pOffsetField->Text());
		m_seqGlobal.setOffset(fVal);
	}
}

// handleSetPort(): deal with the port menu
void SeqGlobalView::handleSetPort(BMessage* pMsg) {
	PortEntry* pPortEntry;
	if(pMsg->FindPointer("port-entry", (void**)&pPortEntry) != B_OK)
		return;
		
	m_seqGlobal.setPort(pPortEntry);
}

// handleSetChannel()
void SeqGlobalView::handleSetChannel(BMessage* pMsg) {
	int16 nChannel;
	if(pMsg->FindInt16("channel", &nChannel) != B_OK) {
		return;
	}
	
	m_seqGlobal.setChannel(nChannel);
}

// AllAttached() ['fix' MessageReceived]
//
// note: the 'Clone' button is NOT routed to the view, since
// its purpose is to request the window to clone itself...

void SeqGlobalView::AllAttached() {
	// target all controls to myself:
	m_pStartStopButton->SetTarget(this);
	m_pMuteButton->SetTarget(this);

	if(m_pOffsetField)	
		m_pOffsetField->SetTarget(this);

	// set up port-menu:
	BMenu* pMenu = m_pPortField->Menu();
	pMenu->SetTargetForItems(this);
	
	// trigger the current port-menu item:
	// +++++ there MUST be an easier way to do this!
	int nLast = pMenu->CountItems();
	PortEntry* pTargetPortEntry = m_seqGlobal.getPort();

	for(int n = 0; n < nLast; n++) {
		InvocableMenuItem* pCur = (InvocableMenuItem*)pMenu->ItemAt(n);
		
		// if no port is currently set in m_seqGlobal, trigger this one
		// & break (so the first gets selected)
		if(m_seqGlobal.getPort() == NULL) {
			pCur->Fire();
			break;
		}
		
		// otherwise, see if the port ref'd in the menu item's message is
		// the one we're looking for:
		BMessage* pMsg = pCur->Message();
		PortEntry* pCurPortEntry;
		if(pMsg->FindPointer("port-entry", (void**)&pCurPortEntry) == B_OK &&
			 pCurPortEntry == pTargetPortEntry) {
			// found it:
			pCur->Fire();
			break;
		}
	}

	// set up channel-menu:	
	pMenu = m_pChannelField->Menu();
	pMenu->SetTargetForItems(this);
	
	// trigger the current channel-menu item:
	// (channels are numbered starting at 1)
	int nTarget = m_seqGlobal.getChannel() - 1;

	InvocableMenuItem* pCur = (InvocableMenuItem*)pMenu->ItemAt(nTarget);
	if(pCur != NULL)
		pCur->Fire();
	
	BView::AllAttached();
}

// MessageReceived()
// dispatches view-specific messages to the appropriate
// handleXXX() method:

void SeqGlobalView::MessageReceived(BMessage* pMsg) {
	
	switch(pMsg->what) {
		case SeqGlobalView::SEQGLOBAL_STARTSTOP:
			handleStartStop(pMsg);
			break;
			
		case SeqGlobalView::SEQGLOBAL_MUTE:
			handleMute(pMsg);
			break;
			
		case SeqGlobalView::SEQGLOBAL_SET_PORT:
			handleSetPort(pMsg);
			break;
		
		case SeqGlobalView::SEQGLOBAL_SET_CHANNEL:
			handleSetChannel(pMsg);
			break;
		
		case SeqGlobalView::SEQGLOBAL_SET_OFFSET:
			handleSetOffset(pMsg);
			break;
			
		default:
			BView::MessageReceived(pMsg);
			break;
	}
}

//----------------------------------------------------------------------//

// draws border rect (menu-bar style)
void SeqGlobalView::Draw(BRect rect) {
	BRect bounds = Bounds();

	// colors:
	rgb_color borderHi = {153,153,153,255};
	rgb_color borderSh = {87,87,87,255};
	rgb_color edgeHi = {244,244,244,255};
	rgb_color edgeSh = {197,197,197,255};
	
	// draw the outer border:
	
	// top-left
	SetHighColor(edgeHi);
	StrokeLine(bounds.LeftTop(), bounds.RightTop());
	StrokeLine(bounds.LeftTop(), bounds.LeftBottom()-BPoint(0,1));
	
	// bottom-right
	SetHighColor(borderSh);
	StrokeLine(bounds.LeftBottom(), bounds.RightBottom());
	StrokeLine(bounds.RightTop(), bounds.RightBottom()-BPoint(0,1));
	SetHighColor(borderHi);
	bounds.bottom--;
	StrokeLine(bounds.LeftBottom(), bounds.RightBottom());
}

//----------------------------------------------------------------------//


