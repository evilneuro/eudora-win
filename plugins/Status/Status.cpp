// Status.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE StatusDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("Status.DLL Initializing!\n");
		
		AfxInitExtensionModule(StatusDLL, hInstance);

		new CDynLinkLibrary(StatusDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Status.DLL Terminating!\n");
	}
	return 1;
}
