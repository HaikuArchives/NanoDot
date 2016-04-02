// TrackView.h (nanodot)
// e.moon 27apr98

#ifndef __TRACKVIEW_H__
#define __TRACKVIEW_H__

#include <View.h>
#include <SupportKit.h>
#include <MenuField.h>
#include <PopUpMenu.h>

#include "Track.h"

class TrackView : public BView {
public: // MESSAGES
	enum trackview_msg {
		TRACK_EDIT_SEQ = 'edsq',
		TRACK_SET_PITCH = 'edpt',
		TRACK_INVERT_SEQ = 'insq',
		TRACK_SHIFT_SEQ = 'shsq',
		TRACK_CLEAR_SEQ = 'clsq'
	};

public: // METHODS
	TrackView(Track& track, BRect frame);
	~TrackView();

	// fetch track	
	Track* getTrack() { return &m_track; }
	
	// handle edit messages:
	virtual void handleEditSeq(BMessage* pMsg);
	virtual void handleSetPitch(BMessage* pMsg);
	virtual void handleInvertSeq(BMessage* pMsg);
	virtual void handleShiftSeq(BMessage* pMsg);
	virtual void handleClearSeq(BMessage* pMsg);
	
	void handleSeqStep(BMessage* pMsg);
	
	// extends AllAttached():
	// targets controls in m_controls to this view
	void AllAttached();
	
	void MessageReceived(BMessage* pMsg);
	void MouseDown(BPoint point);

	// painting
	void Draw(BRect rect);
	
#ifdef B_BEOS_VERSION_4
	void DrawAfterChildren(BRect rect);
#endif
	
protected: // HELPERS
	void buildNoteMenu(BMenu* pMenu); // NYI
	
#ifdef B_BEOS_VERSION_4
	void highlightStep(int16 nStep, bool bOn);
#endif
	
private: // IMPLEMENTATION
	Track& m_track;
	
	BMenuField* m_pNoteField;
	BList m_controls;
	BRect m_menuRect;
	
	BPopUpMenu* m_pContextMenu;
	class BCheckBox** m_ppCheckBoxen;
	
	int16	m_nPos;
};

#endif /* __TRACKVIEW_H__ */