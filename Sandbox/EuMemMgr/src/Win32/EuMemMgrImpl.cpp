#if defined(WIN32) // Only for Win32.
#define __EU_MEMMGR_IMPL_CPP_
#include "EuMemMgrInternalIface.h"

extern
void InitCritSection();
extern
void DelCritSection();

BOOL WINAPI
DllMain(HMODULE, DWORD fdwReason, LPVOID)
{
    BOOL retVal = TRUE;

    switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	    ::InitCritSection();
	    retVal = ::InitializeDebugMemoryManagerMT();
	    break;
	case DLL_PROCESS_DETACH:
	    ::DelCritSection();
	    break;
    }

    return(retVal);
}
#endif
