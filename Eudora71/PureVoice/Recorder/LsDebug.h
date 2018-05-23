/*////////////////////////////////////////////////////////////////////////////

NAME:
	LsDebug - 

FILE:		LsDebug.h
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

#ifndef _LSDEBUG_H_
#define _LSDEBUG_H_

class LsDebug
{
public:
//	LIFECYCLE
	LsDebug(){};
	~LsDebug(){};
	// Copy ctor or assignment operator not allowed

//	OPERATIONS
	void SystemErrorMsgBox( LPCTSTR msg = NULL );
	void DebugMsg( LPCTSTR msg );
	
//	ACCESS
	static int m_EmptyBuffer;

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS

//	ATTRIBUTES

// Do not allow copy ctor or assignment
	LsDebug( const LsDebug& );
	LsDebug& operator=( const LsDebug& );
};

#endif

