#if !defined(AFX_EUGRAPH_H__51E2C86D_367B_11D4_8554_0008C7D3B6F8__INCLUDED_)
#define AFX_EUGRAPH_H__51E2C86D_367B_11D4_8554_0008C7D3B6F8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// EuGraph.h : main header file for EUGRAPH.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CEuGraphApp : See EuGraph.cpp for implementation.

class CEuGraphApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EUGRAPH_H__51E2C86D_367B_11D4_8554_0008C7D3B6F8__INCLUDED)
