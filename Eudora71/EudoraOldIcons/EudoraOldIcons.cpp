//	EudoraOldIcons.cpp : Defines the initialization routines for the DLL.
//
//	The strategy of the old vs. new icons is as follows. The new icons
//	are built into Eudora32.dll just like the old ones were, so that we can
//	always rely on the new icons being present (no special code to check for
//	them or to install them). The old icons are built into a resource plugin
//	by EudoraOldIcons.dsp every time Eudora is built. The user may switch
//	back to the old icons by installing EudoraOldIcons.epi in the same folder
//	as Eudora (actually can be in either the app directory or data directory
//	if they differ).
//
//	EudoraOldIcons.epi can be built in two ways. It is built everytime Eudora
//	is built. It also has its own workspace so that if desired it could be
//	separated out easily and released (possibly externally so that 3rd parties
//	can more easily do their own icons for Eudora).
//
//	NOTE: It is imperative that if we add icons or make one of the toolbar
//		  bitmaps wider in the new icons (Eudora/res/icons) that we make
//		  equivalent changes here (EudoraOldIcons/res/icons). The number of
//		  files in the two directories should remain in sync and the bitmap
//		  widths, etc. should also remain in sync.

#include "stdafx.h"
#include <afxdllx.h>

#include "DebugNewHelpers.h"


static AFX_EXTENSION_MODULE resourceDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("Eudora Old Icons DLL Initializing!\n");
		
		AfxInitExtensionModule(resourceDLL, hInstance);

		DEBUG_NEW CDynLinkLibrary(resourceDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Eudora Old Icons DLL Terminating!\n");
	}
	return 1;
}
