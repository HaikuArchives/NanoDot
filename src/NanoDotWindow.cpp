// NanoDotWindow.cpp
// e.moon apr98

// TO DO: +++++
// - indicate seq. playback position!

#include <stdio.h>

#include "NanoDotWindow.h"
#include "NanoDotApp.h"
#include "TrackView.h"

NanoDotWindow::NanoDotWindow() :
	BWindow(BRect(200, 200, 710, 452), "nanodot", B_TITLED_WINDOW, 
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_WILL_ACCEPT_FIRST_CLICK
#ifdef B_BEOS_VERSION_4
		| B_ASYNCHRONOUS_CONTROLS
#endif
) {

	m_pSeqGlobal = new SeqGlobal();	
	init();
}

NanoDotWindow::NanoDotWindow(BRect frame) :
	BWindow(frame, "nanodot", B_TITLED_WINDOW,
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_WILL_ACCEPT_FIRST_CLICK
#ifdef B_BEOS_VERSION_4
		| B_ASYNCHRONOUS_CONTROLS
#endif
) {
	
	m_pSeqGlobal = new SeqGlobal();	
	init();
}

NanoDotWindow::NanoDotWindow(NanoDotWindow& clone, BRect frame) :
	BWindow(frame, "nanodot", B_TITLED_WINDOW,
		B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_WILL_ACCEPT_FIRST_CLICK
#ifdef B_BEOS_VERSION_4
		| B_ASYNCHRONOUS_CONTROLS
#endif
) {
	
	// clone the sequencer, then the window:
	m_pSeqGlobal = new SeqGlobal(*clone.m_pSeqGlobal);
	init();
}

// init() handles common c'tor-based initialization:

void NanoDotWindow::init() {

	const float fGlobalsHeight = 36;  //56 if offset control enabled
	const float fTrackHeight = 40;
	const float fTempoHeight = 36;
	
	BRect bounds = Bounds();
	
	// throw down a top-level view:
	// (in case we want to do any drawing)
	BView* pTopView = new BView(bounds, "top-level", B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	AddChild(pTopView);
		
	// place globals display at top of window
	// +++++ real layout please
	BRect globalRect = bounds;
	globalRect.bottom = fGlobalsHeight;
	m_pSeqGlobalView = new SeqGlobalView(*m_pSeqGlobal, globalRect);
	pTopView->AddChild(m_pSeqGlobalView);
	
	// make a view for each track:
	BRect trackRect(0, globalRect.bottom, bounds.right,
		globalRect.bottom + fTrackHeight);
	BList* pTrackList = m_pSeqGlobal->getTrackList();
	int nLast = pTrackList->CountItems();
	
	for(int nCount = 0; nCount < nLast; nCount++) {
		Track* pTrack = (Track*)pTrackList->ItemAt(nCount);
	
		// create & add track's view:
		// ++++++ real layout
		TrackView* pTrackView = new TrackView(*pTrack, trackRect);
		pTopView->AddChild(pTrackView);
		trackRect.OffsetBy(0.0, fTrackHeight);
	}
	
	// place tempo view at bottom, flush beneath last track view:
	bounds.bottom = trackRect.top + fTempoHeight;
	BRect tempoRect(0, trackRect.top, bounds.right,
		bounds.bottom);
	m_pTempoView = new TempoView(*m_pSeqGlobal, tempoRect);
	pTopView->AddChild(m_pTempoView);

	ResizeTo(bounds.Width(), bounds.Height());
		
	// ta-da.
	Show();
}

NanoDotWindow::~NanoDotWindow() {
	//BeOS does it for you	
	//RemoveChild(m_pView);
	//delete m_pView;

	delete m_pSeqGlobal;
}

bool NanoDotWindow::QuitRequested() {
	((NanoDotApp*)be_app)->removeWindow(this);
	return true;
}

// clones this window: creates a new window with duplicated contents, and sets it
// up to be automatically started by the current window's sequencer
void NanoDotWindow::handleClone() {

	// create new window directly below this one:
	// +++++ don't create off-screen!
	BRect newFrame = Frame();
	newFrame.OffsetBy(0, newFrame.Height()+32);

	// create new window based on myself:	
	NanoDotWindow* pWnd = new NanoDotWindow(*this, newFrame);
	
	// hand off to the application:
	((NanoDotApp*)be_app)->addWindow(pWnd);
}

// message handler:
void NanoDotWindow::MessageReceived(BMessage* pMsg) {
	switch(pMsg->what) {
	case SeqGlobalView::SEQGLOBAL_CLONE:
		handleClone();
		break;
		
	default:
		BWindow::MessageReceived(pMsg);
		break;
	}
}

// end -- NanoDotWindow.cpp --