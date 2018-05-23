// EudoraIcons.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include "DebugNewHelpers.h"

static AFX_EXTENSION_MODULE EudoraIconsDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("EUDORAICONS.DLL Initializing!\n");
		
		AfxInitExtensionModule(EudoraIconsDLL, hInstance);

		DEBUG_NEW CDynLinkLibrary(EudoraIconsDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("EUDORAICONS.DLL Terminating!\n");
	}
	return 1;
}
