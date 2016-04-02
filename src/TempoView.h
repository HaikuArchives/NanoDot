// TempoView.h (nanodot)
// e.moon 1may98

#ifndef __TEMPOVIEW_H__
#define __TEMPOVIEW_H__

#include <View.h>
#include "SeqGlobal.h"
#include "TempoSlider.h"

class TempoView : public BView {
public: // MESSAGES
	enum tempoview_msg {
		TEMPO_SET_BPM = 'sbpm',
		TEMPO_SET_BPM_FINE = 'sbpf'
	};
	
public: // METHODS
	// c'tor
	TempoView(SeqGlobal& seqGlobal, BRect frame);
	
	// action connector
	void AllAttached();
	
	// message handler
	void MessageReceived(BMessage* pMsg);
	
	// label updater
//	char* UpdateText() const;
	
	// painter
	void Draw(BRect rect);
	
protected: // MEMBERS
	SeqGlobal& m_seqGlobal;
	TempoSlider* m_pCoarseSlider,* m_pFineSlider;
	
	// printf() style string with one %f
	static const char* s_pszLabelFormat;
	
private: // IMPLEMENTATION
	void updateFineSliderLabels(int32 nBPMT);
};

#endif /* __TEMPOVIEW_H__ */
