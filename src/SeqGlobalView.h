// SeqGlobalView.h (nanodot)
// e.moon apr98

#ifndef __SEQGLOBALVIEW_H__
#define __SEQGLOBALVIEW_H__

#include <View.h>
#include <SupportKit.h>
#include <string.h>

#include <Button.h>
#include <MenuField.h>
#include <TextControl.h>

#include "SeqGlobal.h"

class SeqGlobalView : public BView {
public: // MESSAGES
	enum msg {
		SEQGLOBAL_SET_PORT = 'sprt',
		SEQGLOBAL_SET_CHANNEL = 'schn',
		SEQGLOBAL_STARTSTOP = 'stst',
		SEQGLOBAL_MUTE = 'mute',
		SEQGLOBAL_CLONE = 'clon',
		SEQGLOBAL_SET_OFFSET = 'soff'
	};


public: // METHODS
	SeqGlobalView(SeqGlobal& seqGlobal, BRect frame);
	~SeqGlobalView();

	// handle events:
	virtual void handleStartStop(BMessage* pMsg);
	virtual void handleMute(BMessage* pMsg);
	virtual void handleSetPort(BMessage* pMsg);
	virtual void handleSetChannel(BMessage* pMsg);
	virtual void handleSetOffset(BMessage* pMsg);

	// set up (AllAttached) and handle (MessageReceived) control messages	
	void AllAttached();
	void MessageReceived(BMessage*);
	
	// draw the view's border
	void Draw(BRect rect);
	
protected: // HELPERS

	// populates the given menu from the given list, sending
	// 
	static void addPortMenuItems(BMenu* pMenu, BList& list);
	
	static void addChannelMenuItems(BMenu* pMenu);
	
private: // IMPLEMENTATION


	
	SeqGlobal& m_seqGlobal;
	
	// the port & channel menu fields:
	BMenuField* m_pPortField,* m_pChannelField;
	
	// the start/stop button:
	BButton* m_pStartStopButton;
	
	// the mute button:
	BButton* m_pMuteButton;
	
	// the clone button:
	BButton* m_pCloneButton;
	
	// the offset field
	BTextControl* m_pOffsetField;
};

#endif /* __SEQGLOBALVIEW_H__ */
