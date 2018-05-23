/////////////////////////////////////////////////////////////////////////////
// slocale.h : Header file for localization.
//
// Stingray Software Extension Classes
// Copyright (C) 1996-7 Stingray Software Inc,
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
//	Author:		Dean
//

#ifndef __SLOCALE_H__
#define __SLOCALE_H__

/////////////////////////////////////////////////////////////////////////////
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#ifdef WIN32
#ifndef _WINNLS_
#include "winnls.h"
#endif	
#endif

/////////////////////////////////////////////////////////////////////////////
// SECLocale 
//
//   This class aids in internationalization.
//


class SECLocale
{
// Construction
public:
	SECLocale();

// Attributes
public:

	static const int DaysPerWeek;
	static const int MonthsPerYear;

protected:

#ifdef WIN32
	static LCID m_lcid;
	static const LCTYPE DayNameIds[7][2];
	static const LCTYPE MonthNameIds[12][2];
#else // WIN32
	static TCHAR *dayAbbr[];
	static TCHAR *dayFull[];
	static TCHAR *monthAbbr[];
	static TCHAR *monthFull[];
#endif


// Operations
public:

	static BOOL GetDayName(CString& strDay, int nDay, BOOL bFullName = TRUE);
	static BOOL GetMonthName(CString& strMonth, int nMonth, BOOL bFullName = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECDayBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECLocale();

};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

/////////////////////////////////////////////////////////////////////////////

#endif // __SLOCALE_H__