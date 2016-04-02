// InvocableMenuItem.h (nanodot)
// Just like a BMenuItem, but provides access to
// Invoke() via the method Fire().
//
// e.moon 29apr98

#ifndef __INVOCABLEMENUITEM_H__
#define __INVOCABLEMENUITEM_H__

#include <Menu.h>
#include <MenuItem.h>

class InvocableMenuItem : public BMenuItem {
public: // METHODS
	InvocableMenuItem(const char* pszLabel, BMessage *pMessage, 
         char shortcut = 0, uint32 modifiers = 0) :
         BMenuItem(pszLabel, pMessage, shortcut, modifiers) {;}

	// this is naughty, because it effectively makes Invoke()
	// less private
	void Fire() {
		BMenuItem::Invoke();
	}
};

#endif /* __INVOCABLEMENUITEM_H__ */
