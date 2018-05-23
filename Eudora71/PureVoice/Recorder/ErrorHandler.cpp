/*////////////////////////////////////////////////////////////////////////////

NAME:
	CErrorHandler - 

FILE:		ErrorHandler.CPP
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-SOURCE

DESCRIPTION:

RESCRITICTIONS:
	Not meant to be derived. Copy ctor or assignment operator not allowed.
	Must call Init() before any use.

DEPENDENCIES:


FILES:


REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES

// LOCAL INCLUDES
#include "ErrorHandler.h"

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

int CErrorHandler::ReportError( UINT nError /* = 0 */,
	UINT nType /* = MB_OK */)
{
	TCHAR   szErrorMessage[256];
	TCHAR	*p = szErrorMessage;
	int     nDisposition;

    if (nError == 0)
		nError = AFX_IDP_NO_ERROR_AVAILABLE;
	else
	 {
//		wsprintf( szErrorMessage, "(%d) ", nError );
//		p = szErrorMessage + strlen(szErrorMessage);
	 }
	LoadString( AfxGetInstanceHandle(), nError, p, 246 );
	nDisposition = AfxMessageBox( szErrorMessage, nType );
	
    return nDisposition;
}

int CErrorHandler::ReportError( LPCTSTR msg, UINT nType /* = MB_OK */)
{
	return AfxMessageBox( msg, nType );
}

UINT CErrorHandler::mapErrorCode( UINT nError )
{
    return nError;
}