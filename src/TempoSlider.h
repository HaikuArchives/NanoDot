// TempoSlider.h (nanodot)
// e.moon 2may98
//
// An extended BSlider with the following extra functionality:
// +++++  proper mouse-drag response (workaround for BeOS R3/x86
//        BSlider bug)   [done, but in a REALLY naughty fashion]
//
// [done] mouse click -> keyboard focus (for easy changes via
//        arrow keys)
// [done] display of current value

#ifndef __TEMPOSLIDER_H__
#define __TEMPOSLIDER_H__

#include <stdio.h>
#include <stdlib.h>
#include <Slider.h>

class TempoSlider : public BSlider {
public: // METHODS
	TempoSlider(BRect frame, const char* pszName,
		const char* pszLabel, BMessage* pMsg,
		int32 nMin, int32 nMax,
		thumb_style nThumbType=B_BLOCK_THUMB,
		uint32 nResizingMode=B_FOLLOW_LEFT|B_FOLLOW_TOP,
		uint32 nFlags=B_FRAME_EVENTS|B_WILL_DRAW|B_NAVIGABLE);
		
	~TempoSlider();
	
	void setDragMessage(BMessage* pMsg); // NYI
	BMessage* getDragMessage() { return m_pDragMessage; }
	
	// MEDIUM-SIZED HACK:
	// grabs the focus whenever the control is clicked, to make
	// keyboard adjustment easier:
	
	void MouseDown(BPoint point) {
		BSlider::MouseDown(point);
		MakeFocus(true);
	}
	
	// MASSIVE HACK:
	// DrawText() *is* called during dragging; that makes this a good
	// time to send the modification message...
	// ...but Invoke() seems to be blocked during dragging, so
	//    I just tickle my owning view directly via MessageReceived()...
	
	void DrawText() {
		BSlider::DrawText();
		BRect frame = OffscreenView()->Bounds();
		
		// draw some label text:
		sprintf(m_pszLabelBuffer, "%.1f BPM", (float)Value() / 10.0);
		
		// centered, bottom:
		float fStringWidth = StringWidth(m_pszLabelBuffer);
		float fLeft = (frame.Width() - fStringWidth) / 2;	
		
		// get font height:
		font_height fontHeight;
		BFont curFont;
		OffscreenView()->GetFont(&curFont);
		curFont.GetHeight(&fontHeight);
		
		float fTop = frame.bottom - (fontHeight.descent);
		if	(fTop < 0) {
			// don't bother drawing
			return;
		}
		
		// draw string
		OffscreenView()->DrawString(m_pszLabelBuffer,
			BPoint(fLeft, fTop));	
			
		// now cheat & 'send' a modification message
		// (aka the 'drag message')
		if(m_pDragMessage)
			Parent()->MessageReceived(m_pDragMessage);
	}
	
	/*char* UpdateText() const {
		char* pszBuffer = new char[64];
		sprintf(pszBuffer, "%.1f", (float)Value() / 10.0);
		return pszBuffer;
	}*/
	
protected: // MEMBERS
	char m_pszLabelBuffer[64];
	
	BMessage* m_pDragMessage;
	
private: // IMPLEMENTATION
};

#endif /* __TEMPOSLIDER_H__ */
