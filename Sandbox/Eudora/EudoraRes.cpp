// EudoraRes.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE Eudora32DLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("EUDORA32.DLL Initializing!\n");
		
		AfxInitExtensionModule(Eudora32DLL, hInstance);

		new CDynLinkLibrary(Eudora32DLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("EUDORA32.DLL Terminating!\n");
	}
	return 1;
}
