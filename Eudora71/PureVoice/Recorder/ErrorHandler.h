/*////////////////////////////////////////////////////////////////////////////

NAME:
	CErrorHandler - 

FILE:		ErrorHandler.H
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef _ERRORHANDLER_H_
#define _ERRORHANDLER_H_

class CErrorHandler
{
public:
//	LIFECYCLE
    CErrorHandler() : mi_ErrorCode(0){};
    ~CErrorHandler(){};

//	OPERATIONS
    int ReportError( UINT nError = 0, UINT nType = MB_OK );
    int ReportError( LPCTSTR msg, UINT nType = MB_OK );

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
    UINT mapErrorCode( UINT nError );

//	ATTRIBUTES
private:
    UINT    mi_ErrorCode;

private:
    CErrorHandler( const CErrorHandler& );
    CErrorHandler& operator=( const CErrorHandler& );
};

#endif