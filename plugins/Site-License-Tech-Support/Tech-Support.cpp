// Tech-Support.cpp : Defines the initialization routines for the DLL.
// 
// Tech-Support.epi is a resource plugin that overrides the
// text that comes up when the user chooses "Help:Technical Support".
// The new text is customized for use with site licenses.

#include "stdafx.h"
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE TechSupportDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("Tech-Support.DLL Initializing!\n");
		
		AfxInitExtensionModule(TechSupportDLL, hInstance);

		new CDynLinkLibrary(TechSupportDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Tech-Support.DLL Terminating!\n");
	}
	return 1;
}
