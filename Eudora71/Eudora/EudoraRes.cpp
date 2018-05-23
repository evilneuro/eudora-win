// EudoraRes.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include "DebugNewHelpers.h"

static AFX_EXTENSION_MODULE EudoraResDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("EudoraRes.dll Initializing!\n");
		
		AfxInitExtensionModule(EudoraResDLL, hInstance);

		DEBUG_NEW CDynLinkLibrary(EudoraResDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("EudoraRes.dll Terminating!\n");
	}
	return 1;
}
