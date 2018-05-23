// esoteric.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE esotericDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("esoteric.DLL Initializing!\n");
		
		AfxInitExtensionModule(esotericDLL, hInstance);

		new CDynLinkLibrary(esotericDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("esoteric.DLL Terminating!\n");
	}
	return 1;
}
