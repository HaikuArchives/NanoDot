// PortEntry.h
// e.moon 16oct98
//
// Represents one entry in the app-wide MIDI gizmo list

#ifndef __PORTENTRY_H__
#define __PORTENTRY_H__

#include <MidiKit.h>
#include <string.h>

struct PortEntry {
	PortEntry(char* pszName, BMidi* pPort) {
		m_pszName = strdup(pszName); m_pPort = pPort;
	}
	~PortEntry() { delete [] m_pszName; }
		
	char* m_pszName;
	BMidi* m_pPort;
};

#endif /* __PORTENTRY_H__ */