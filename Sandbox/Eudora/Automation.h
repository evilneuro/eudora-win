#ifndef _Automation_h_
#define _Automation_h_

#include "EudoraExe.h"
#include "CEudoraModule.h"
#include "QComObject.h"

// -----------------------------------------------------------------------------
// Automation global functions

void AutomationEnable(void);
void AutomationDisable(void);
void AutomationRegisterFactory(void);
void AutomationUnregisterFactory(void);
void AutomationStart(void);
void AutomationStop(void);
long GetUniqueID(void);

// -----------------------------------------------------------------------------
// Automation call class

class CAutomationCall
{
public:
	CAutomationCall();
	~CAutomationCall();
};

// -----------------------------------------------------------------------------

#define ROOTFOLDER "RootFolder"

extern bool gbAutomationEnabled;
extern bool gbAutomationFactoryRegistered;
extern bool gbAutomationRunning;
extern bool gbAutomationExternalStart;
extern bool gbAutomationCall;
extern bool gbAutomationCommandQueued;
extern bool gbAutomationCheckMail;
extern bool gbAutomatedShutdown;
extern long g_NextId;
extern HINSTANCE ghResourceInst;

inline STDMETHODIMP CheckReturnPointer(void * pVal)
{
	try
	{ 
		// Pointer must not be NULL
		if (!pVal) return E_POINTER;
		// Pointer must be safely writable
		*((short*)pVal) = 0xCD;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

#endif // _Automation_h_
