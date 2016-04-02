// NanoError.h (nanodot)
// e.moon 3may98
//
// A simple exception class that can display its message
// via a BAlert.

#ifndef __NANOERROR_H__
#define __NANOERROR_H__

#include <Application.h>
#include <Alert.h>

class NanoError {
public: // METHODS
	NanoError();
	NanoError(char* pszMsg);
	~NanoError();
	
	void display(const char* pszLabel="Exit",
		alert_type nAlertType=B_STOP_ALERT);
		
	void displayAndQuit(const char* pszLabel="Exit",
		alert_type nAlertType=B_STOP_ALERT) {
		
		display(pszLabel, nAlertType);
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
		
	char* getMessage() { return m_pszMsg; }
		
protected: // MEMBERS
	char* m_pszMsg;
};

#endif /* __NANOERROR_H__ */
