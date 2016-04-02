// TempoSlider.cpp (nanodot)
// e.moon 2may98

#include <stdlib.h>
#include <stdio.h>

#include "TempoSlider.h"

// c'tor
TempoSlider::TempoSlider(BRect frame, const char* pszName,
		const char* pszLabel, BMessage* pMsg,
		int32 nMin, int32 nMax, thumb_style nThumbType,
		uint32 nResizingMode, uint32 nFlags):
	
	m_pDragMessage(NULL),
	BSlider(frame, pszName, pszLabel, pMsg,
					nMin, nMax, nThumbType, nResizingMode, nFlags) {
	;
}

// d'tor
TempoSlider::~TempoSlider() {
	// clean up drag message (if any)
	if(m_pDragMessage != NULL)
		delete m_pDragMessage;
}

// pop in a new drag message:
void TempoSlider::setDragMessage(BMessage* pMsg) {
	// +++++ lock?
	m_pDragMessage = pMsg;
}

// END -- TempoSlider.cpp --