// NanoError.cpp (nanodot)
// e.moon 3may98

#include "NanoError.h"
#include <string.h>
#include <stdio.h>

// NanoError c'tors

NanoError::NanoError() :
	m_pszMsg("unknown error") {;}

NanoError::NanoError(char* pszMsg) :
	m_pszMsg(strdup(pszMsg)) {;}

// NanoError d'tor

NanoError::~NanoError() {
	if(m_pszMsg)
		delete [] m_pszMsg;
}

// display(): show myself in a BAlert box:

void NanoError::display(const char* pszLabel,
		alert_type nAlertType) {

	BAlert* pAlert = new BAlert("ERROR", m_pszMsg,
		pszLabel, NULL, NULL, B_WIDTH_AS_USUAL,
		nAlertType);

	printf("NanoError: %s\n", m_pszMsg);
		
	pAlert->Go();	
}

// END -- NanoError.cpp --