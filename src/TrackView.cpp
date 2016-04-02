// TrackView.cpp (nanodot)
// e.moon 27apr98

#include <stdio.h>
#include <string.h>
#include "TrackView.h"
#include "InvocableMenuItem.h"

#include <PopUpMenu.h>
#include <CheckBox.h>
#include <Region.h>
#include "Colors.h"

TrackView::TrackView(Track& track, BRect frame) :
	m_track(track),
	m_nPos(-1),
	BView(frame, "TrackView", B_FOLLOW_NONE,
		B_WILL_DRAW
#ifdef B_BEOS_VERSION_4
		| B_DRAW_ON_CHILDREN
#endif
	) {

	// put myself together:
	BPopUpMenu* pNoteMenu = new BPopUpMenu("note-menu");
	buildNoteMenu(pNoteMenu);
	
	// build & add menu field:
	m_pNoteField = new BMenuField(BRect(3, 7, 95, 40),
		"note-menu-field", "Note:", pNoteMenu);
	AddChild(m_pNoteField);
	
	// edge of menu area:
	m_menuRect = BRect(0, 0, 100, 40);
	
	// build & add track-editing checkboxes
	BRect checkFrame(113, 9, 24, 26);
	
	m_ppCheckBoxen = new BCheckBox*[m_track.getSeqLength()];
	for(int16 nCount = 0; nCount < m_track.getSeqLength(); nCount++) {
		
		// construct a message:
		BMessage* pMsg = new BMessage(TRACK_EDIT_SEQ);
		
		// create checkbox
		BCheckBox* pCheckBox = new BCheckBox(
			checkFrame,
			NULL,
			"", // no label
			pMsg, // message to fire on click
			B_FOLLOW_NONE,
			B_WILL_DRAW); // turn off keyboard navigation			
		
		// get height
		pCheckBox->ResizeToPreferred();
		pCheckBox->SetValue(m_track.getData(nCount));
			
		// initialize the message:
		pMsg->AddInt16("seq-pos", nCount);
		pMsg->AddPointer("check-box", pCheckBox);
		pMsg->AddPointer("track-view", this);

		// bump frame along for next checkbox:
		checkFrame.OffsetBy(24.0, 0.0);
		
		// bump every 4 a little more:
		if((nCount % 4) == 3)
			checkFrame.OffsetBy(4.0, 0.0);
		
		// add checkbox to view:
		AddChild(pCheckBox);
		
		// add to retarget-list:
		m_controls.AddItem(pCheckBox);
		
		m_ppCheckBoxen[nCount] = pCheckBox;
	}

	// build context menu
	m_pContextMenu = new BPopUpMenu("track-context-menu",
		false, false);
	m_pContextMenu->AddItem(
		new BMenuItem("Invert",
		new BMessage(TRACK_INVERT_SEQ)));
	BMessage* pShiftRightMsg = new BMessage(TRACK_SHIFT_SEQ);
	pShiftRightMsg->AddInt16("offset", 1);
	BMessage* pShiftLeftMsg = new BMessage(TRACK_SHIFT_SEQ);
	pShiftLeftMsg->AddInt16("offset", -1);

	m_pContextMenu->AddItem(new BMenuItem("Shift Left", pShiftLeftMsg));
	m_pContextMenu->AddItem(new BMenuItem("Shift Right", pShiftRightMsg));

	m_pContextMenu->AddItem(
		new BMenuItem("Clear",
		new BMessage(TRACK_CLEAR_SEQ)));
	
	// background color
	rgb_color bgMenu = {219,219,219,255};
	SetViewColor(bgMenu);
}

TrackView::~TrackView() {
	delete [] m_ppCheckBoxen;
}

// buildNoteMenu: fills a BMenu with note numbers & names
void TrackView::buildNoteMenu(BMenu* pMenu) {
	// start octave
	int nOctave = 1;
	// start pitch (0=C)
	int nPitch = 0;
	// start note-number (36=C1)
	int nNote = 36;
	// last note-number
	int nEndNote = 60;
	
	// pitch names
	char ppszPitchNames[][3] = {
		"C ",
		"C#",
		"D ",
		"D#",
		"E ",
		"F ",
		"F#",
		"G ",
		"G#",
		"A ",
		"A#",
		"B "
	};
	
	char pNameBuffer[16];
	while(nNote <= nEndNote) {
		// name note;
		sprintf(pNameBuffer, "%s %d", ppszPitchNames[nPitch],
			nOctave);

		// Construct message:
		BMessage* pMsg = new BMessage(TRACK_SET_PITCH);
		pMsg->AddInt16("pitch", nNote);
		pMsg->AddPointer("track-view", this);
		
		// Build menu item:
		InvocableMenuItem* pItem = new InvocableMenuItem(pNameBuffer, pMsg);
			
		// Add to menu:
		pMenu->AddItem(pItem, 0);
		
		// add to retarget list:
		//m_controls.AddItem(pItem);
		
		// Reference menu:
		pMsg->AddPointer("menu", pMenu);	
		
		// Bump counts:
		++nNote;
		if(++nPitch > 11) {
			nPitch = 0;
			++nOctave;
		}
	}
}

// target messages:

void TrackView::AllAttached() {
	// walk m_controls; retarget each to myself:
	for(int nCount = 0; nCount < m_controls.CountItems(); nCount++) {
		BControl* pC = (BControl*)m_controls.ItemAt(nCount);
		pC->SetTarget(this);
//		printf("targeted: %d (%p)\n", nCount, pC);
	}
	
	// retarget note menu:
	BMenu* pMenu = m_pNoteField->Menu();
	pMenu->SetTargetForItems(this);
	
	// trigger the right item in note menu:
	// (walk it & compare pitch in message)
	// +++++ ICK
	
	int nLast = pMenu->CountItems();
	int nTarget = m_track.getPitch();
	
	for(int n = 0; n < nLast; n++) {
		InvocableMenuItem* pCur = (InvocableMenuItem*)pMenu->ItemAt(n);
		BMessage* pMsg = pCur->Message();
		
		int16 nPitch;
		if(pMsg->FindInt16("pitch", &nPitch) == B_OK &&
			 nPitch == nTarget) {
			 
			// found it; fire:
			pCur->Fire();
			break;
		}
	}
	
	m_pContextMenu->SetTargetForItems(this);

	// register w/ track object
	m_track.setMessenger(new BMessenger(this));

	// hand off to mommy
	BView::AllAttached();	
}

// handle edit messages:

void TrackView::handleEditSeq(BMessage* pMsg) {
	// get position in sequence
	int16 nSeqPos;
	if(pMsg->FindInt16("seq-pos", &nSeqPos) != B_OK)
		return;
		
	// get checkbox ptr:
	BCheckBox* pCheck;
	if(pMsg->FindPointer("check-box", (void**)&pCheck) != B_OK)
		return;
	
	// set value:
	m_track.setData(nSeqPos, (pCheck->Value() != 0));
}

void TrackView::handleSetPitch(BMessage* pMsg)  {
	int16 nPitch;
	if(pMsg->FindInt16("pitch", &nPitch) != B_OK)
		return;
		
	m_track.setPitch(nPitch);
}

void TrackView::handleInvertSeq(BMessage* pMsg) {
	for(int16 n = 0; n < m_track.getSeqLength(); n++) {
		bool bVal = !m_track.getData(n);
		m_track.setData(n, bVal);
		m_ppCheckBoxen[n]->SetValue(bVal ? 1 : 0);
	}
}

void TrackView::handleShiftSeq(BMessage* pMsg) {
	int16 nOffset;
	if(pMsg->FindInt16("offset", &nOffset) != B_OK)
		return;
		
	int16 nLength = m_track.getSeqLength();
	int16 nSrc, nDest;
	bool* pNew = new bool[nLength];
	nSrc = -nOffset;
	if(nSrc < 0) nSrc = nLength + nSrc;
	
	for(nDest = 0; nDest < nLength;	nDest++) {
	
		pNew[nDest] = m_track.getData(nSrc);	
		if(++nSrc == nLength)
			nSrc = 0;
	}
	
	for(nSrc = 0; nSrc < nLength; nSrc++) {
		m_track.setData(nSrc, pNew[nSrc]);
		m_ppCheckBoxen[nSrc]->SetValue(pNew[nSrc] ? 1 : 0);
	}
	
	delete [] pNew;
}

void TrackView::handleClearSeq(BMessage* pMsg) {

	int16 nLength = m_track.getSeqLength();
	for(int16 n = 0; n < nLength; n++) {
		m_track.setData(n, false);
		m_ppCheckBoxen[n]->SetValue(0);
	}
}

void TrackView::handleSeqStep(BMessage* pMsg) {
	int16 nOld = m_nPos;
	if(pMsg->FindInt16("pos", &m_nPos) != B_OK ||
		m_nPos == nOld)
		return;
		
	BRect r = Bounds();
	BRect f;
	r.top = r.bottom - 16;
	r.bottom -= 2;
	if(nOld >= 0) {
		f = m_ppCheckBoxen[nOld]->Frame();
		r.left = f.left - 2;
		r.right = f.right + 2;
		Invalidate(r);
	}
	f = m_ppCheckBoxen[m_nPos]->Frame();
				
	r.left = f.left - 2;
	r.right = f.right + 2;
	Invalidate(r);
}
	
void TrackView::MessageReceived(BMessage* pMsg) {
	
	switch(pMsg->what) {
		case TRACK_EDIT_SEQ:
			handleEditSeq(pMsg);
			break;
		
		case TRACK_SET_PITCH:
			handleSetPitch(pMsg);
			break;
			
		case TRACK_INVERT_SEQ:
			handleInvertSeq(pMsg);
			break;
			
		case TRACK_SHIFT_SEQ:
			handleShiftSeq(pMsg);
			break;
			
		case TRACK_CLEAR_SEQ:
			handleClearSeq(pMsg);
			break;
			
		case TRACK_SEQ_STEP:
			handleSeqStep(pMsg);
			break;

		default:
			BView::MessageReceived(pMsg);
			break;
	}
}

// paint method: draws a 3d border

void TrackView::Draw(BRect rect) {
	BRect bounds = Bounds();

	// colors:
	rgb_color borderHi = {153,153,153,255};
	rgb_color borderSh = {87,87,87,255};
	rgb_color edgeHi = {244,244,244,255};
	rgb_color edgeSh = {197,197,197,255};
	rgb_color bgMenu = {219,219,219,255};
	rgb_color bgSequence = {0,0,0,255};

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

	// background
	bounds.InsetBy(1.0, 1.0);	
	BRegion bgRegion; bgRegion.Include(bounds);

	// step
	int16 nPos = m_nPos;
	if(nPos >= 0) {
		BRect f = m_ppCheckBoxen[nPos]->Frame();
		f.OffsetBy(3.0, f.Height()+3);
		f.bottom = f.top + 1;
		f.right = f.left + 8;

		SetHighColor(Blue);
		FillRect(f);
	}
}

#ifdef B_BEOS_VERSION_4
void TrackView::DrawAfterChildren(BRect rect) {
/*
	// step
	int16 nPos = m_nPos;

	if(nPos >= 0)
		highlightStep(nPos, true);
*/
}

void TrackView::highlightStep(int16 nStep, bool bOn) {
//	printf("highlightStep(%d, %s)\n", nStep, (bOn ? "true" : "false"));
	BRect bounds = Bounds();
	
	drawing_mode oldMode = DrawingMode();
	SetDrawingMode(B_OP_ALPHA);
	rgb_color lightenA = {255, 255, 255, 128};
	rgb_color lightenB = {255, 255, 255, 96};
	rgb_color darkenA = {0, 0, 0, 128};
	rgb_color darkenB = {0, 0, 0, 96};

	BRect f = m_ppCheckBoxen[nStep]->Frame();
	f.top = bounds.top; f.bottom = bounds.bottom - 2;
	f.left += 1; f.right = f.left + 13;
	
	SetHighColor(bOn ? lightenA : darkenA);
	FillRect(f);

	SetHighColor(bOn ? lightenB : darkenA);
	StrokeLine(BPoint(f.left-1, f.top), BPoint(f.left-1, f.bottom));		
	StrokeLine(BPoint(f.right+1, f.top), BPoint(f.right+1, f.bottom));		

	SetDrawingMode(oldMode);
}

#endif

// mouse-down: pop up menu
void TrackView::MouseDown(BPoint point) {
	uint32 nButtons;
	GetMouse(&point, &nButtons, false);
	ConvertToScreen(&point);
	if(nButtons & B_SECONDARY_MOUSE_BUTTON) {
		m_pContextMenu->Go(point, true, false, true);
	}
}
 
// END -- TrackView.cpp --