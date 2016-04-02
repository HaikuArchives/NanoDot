// NanoDotWindow.h (nanodot)
// e.moon apr98

#ifndef __NANODOTWINDOW_H__
#define __NANODOTWINDOW_H__

#include <Window.h>
#include "SeqGlobal.h"
#include "SeqGlobalView.h"
#include "TempoView.h"

class NanoDotWindow : public BWindow {
public: // METHODS

	// basic c'tor/d'tors
	NanoDotWindow();
	NanoDotWindow(BRect frame);
	~NanoDotWindow();

	// a special cloning c'tor:
	NanoDotWindow(NanoDotWindow& clone, BRect frame);
			
	// closes window & notifies app:
	bool QuitRequested();
		
	SeqGlobal* getSeqGlobal() const { return m_pSeqGlobal; }
	
	// message handler:
	void MessageReceived(BMessage* pMsg);

protected: // HELPERS

	// handles common construction duties:
	void init();
		
	// clones this window & its contents
	void handleClone();
		
private: // IMPLEMENTATION
	SeqGlobal* m_pSeqGlobal;
	SeqGlobalView* m_pSeqGlobalView;
	TempoView* m_pTempoView;
};

#endif /* __NANODOTWINDOW_H__ */
