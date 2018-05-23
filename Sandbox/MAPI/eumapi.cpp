////////////////////////////////////////////////////////////////////////
// EUMAPI.CPP
//
// Main module for 16-bit/32-bit Eudora MAPI DLL.  This common file
// is used for both versions, with the appropriate #ifdef's, of course.
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
//#include <afxwin.h>		// FORNOW, might be better to use precompiled stdafx


/////////////////////////////////////////////////////////////////////////////
// Library init

class CEudoraMapiDLL : public CWinApp
{
public:
	virtual BOOL InitInstance(); // Initialization
	virtual int ExitInstance();  // Termination (WEP-like code)

	// nothing special for the constructor
	CEudoraMapiDLL(LPCTSTR pszAppName) : CWinApp(pszAppName) { }
};

BOOL CEudoraMapiDLL::InitInstance()
{
	// any DLL initialization goes here
#ifdef _DEBUG
#ifdef WIN32
	OutputDebugString("EUMAPI32.DLL initializing\n");
#else
	OutputDebugString("EUMAPI.DLL initializing\n");
#endif // WIN32
#endif // _DEBUG

	SetDialogBkColor();     // grey dialogs in the DLL as well

	return TRUE;
}

int CEudoraMapiDLL::ExitInstance()
{
	// any DLL termination goes here (WEP-like code)
	return CWinApp::ExitInstance();
}

#ifdef WIN32
CEudoraMapiDLL  NEAR eudoraMapiDLL(_T("eumapi32.dll"));
#else
CEudoraMapiDLL  NEAR eudoraMapiDLL("eumapi.dll");
#endif // WIN32

