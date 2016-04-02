// TempoView.cpp (nanodot)
// e.moon 1may98

#include "TempoView.h"

// label (w/ one %f tag for sprintf)
/*static*/
const char* TempoView::s_pszLabelFormat = "BPM: %.1f";

//---------------------------------------------------------------//

// c'tor
TempoView::TempoView(SeqGlobal& seqGlobal, BRect frame) :
		m_seqGlobal(seqGlobal),
		BView(frame, "tempo-view", B_FOLLOW_TOP|B_FOLLOW_LEFT,
					B_WILL_DRAW) {
					
	// create sliders
	BRect sliderRect = Bounds();
	sliderRect.bottom = 40;
	// (leave room for the border)
	sliderRect.InsetBy(2,2);

	//---------------------------------------//	
	// coarse-tempo slider:
	// measured in 10ths
	//---------------------------------------//	

	m_pCoarseSlider = new TempoSlider(sliderRect,
		"coarse-bpm-slider", NULL,
		new BMessage(TEMPO_SET_BPM),
		10, 3000); // min & max values (x10)
		
	// set to init value
	int32 nBPMT = m_seqGlobal.getTempo(); // returns in 1000ths of a BPM
	m_pCoarseSlider->SetValue(nBPMT/100); 
	
	// set min/max labels
	m_pCoarseSlider->SetLimitLabels("1.0", "300.0");
	
	// set hash marks
	m_pCoarseSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	m_pCoarseSlider->SetHashMarkCount(10);
	
	// add
	AddChild(m_pCoarseSlider);
#if 0
	//---------------------------------------//	
	// fine-tempo slider (+/- 1 BPM)
	// measured in 1000ths
	//---------------------------------------//	

	sliderRect.OffsetBy(0, 40);
	m_pFineSlider = new TempoSlider(sliderRect,
		"fine-bpm-slider", NULL,
		new BMessage(TEMPO_SET_BPM_FINE),
		-1000, 1000); // min & max in BPM/1000

	// set to init (center) value
	m_pFineSlider->SetValue(0);

	// set labels
	updateFineSliderLabels(nBPMT);
	
	// set hash marks
	m_pFineSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	m_pFineSlider->SetHashMarkCount(3);
	
	m_pFineSlider->SetSnoozeAmount(10000);
	
	// add
	AddChild(m_pFineSlider);
#endif /* 0 */
	// prepare for drawing; set color:
	rgb_color bgGray = {219,219,219,255};
	SetViewColor(bgGray);
}

// routes slider messages -> this view
// also add a 'modification message' to continuously set
// the tempo while the mouse is being draggaed
void TempoView::AllAttached() {
	m_pCoarseSlider->SetTarget(this);
	m_pCoarseSlider->setDragMessage(new BMessage(TEMPO_SET_BPM));
	
//	m_pFineSlider->SetTarget(this);
//	m_pFineSlider->SetModificationMessage(new BMessage(TEMPO_SET_BPM_FINE));
}

// handles slider messages
void TempoView::MessageReceived(BMessage* pMsg) {
	switch(pMsg->what) {
		case TEMPO_SET_BPM: {
			int32 nBPMT = m_pCoarseSlider->Value()*100;
			m_seqGlobal.setTempo(nBPMT);
#if 0			
			printf("TempoView(): coarse update: %d\n", nBPMT);
			// update the fine-tempo slider:
			m_pFineSlider->SetValue(0);
			updateFineSliderLabels(nBPMT);
#endif /* 0 */
			break;
		}
#if 0		
		case TEMPO_SET_BPM_FINE: {
			int32 nBPMT = (m_pCoarseSlider->Value()*100) + m_pFineSlider->Value();
			m_seqGlobal.setTempo(nBPMT);
			printf("TempoView(): fine update: %d\n", nBPMT);
			break;
		}
#endif /* 0 */
		
		// hand off to mommy
		default:
			BView::MessageReceived(pMsg);
			break;
	}
}

// paint border & background
void TempoView::Draw(BRect rect) {
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

void TempoView::updateFineSliderLabels(int32 nBPMT) {
		
	int32 nMin = nBPMT - 1000;
	int32 nMax = nBPMT + 1000;
	
	// set min/max labels
	char minBuffer[16], maxBuffer[16];
	sprintf(minBuffer, "%.001f", (float)nMin/1000);
	sprintf(maxBuffer, "%.001f", (float)nMax/1000);
	m_pFineSlider->SetLimitLabels(minBuffer, maxBuffer);
}

// END -- TempoView.cpp --
