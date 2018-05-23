/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsDebug - 

FILE:		LsDebug.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/01/97   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// LOCAL INCLUDES
#include "LsDebug.h"

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

void LsDebug::SystemErrorMsgBox( LPCTSTR msg )
{
	DWORD err = GetLastError();
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	if ( msg )
	 {
		CString disp; disp.GetBuffer(1024);
		disp.Format("%s - %s", msg, lpMsgBuf);
		// Display the string.
		AfxMessageBox( (LPCTSTR)disp );
	 }
	else
		AfxMessageBox( (LPCTSTR)lpMsgBuf );

	// Free the buffer.
	LocalFree( lpMsgBuf );
}

void LsDebug::DebugMsg( LPCTSTR msg )
{
//	((CMainFrame*)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(0, msg);
	AfxGetMainWnd()->SetWindowText( msg );
}
