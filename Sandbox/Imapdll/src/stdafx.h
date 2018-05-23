// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2643AF57_E81A_11D1_90A9_00805F9B8439__INCLUDED_)
#define AFX_STDAFX_H__2643AF57_E81A_11D1_90A9_00805F9B8439__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Eudora Debug Memory Manager Interface. 
// smohanty 05/27/97
#include "NewIfaceMT.h"

// ASSERT/VERIFY logging
// smanjour, 7/2/98
//
#include <afx.h> // Include ASSERT/VERIFY, so it cannot be included AFTER this

#ifndef NOEXPRLOG
	#include "debug.h" // Logging (QCLogFileMT)
	#include "stdio.h" // sprintf

	#ifdef _DEBUG

		#define VERIFYLOG(expr) \
			do { \
				if (!(expr)) \
				{ \
					QCLogFileMT::WriteAssertLog(#expr, THIS_FILE, __LINE__); \
					if (AfxAssertFailedLine(THIS_FILE, __LINE__)) \
						AfxDebugBreak(); \
				} \
			} while (0)

		#define EXPRLOG(expr) VERIFYLOG(expr)

		#ifdef VERIFY
		#undef VERIFY
		#endif

		#define VERIFY(expr) VERIFYLOG(expr)

		#ifdef ASSERT
		#undef ASSERT
		#endif

		#define ASSERT(expr) VERIFYLOG(expr)
		
	#else // _DEBUG

		#define VERIFYLOG(expr) \
			do { \
				if (!(expr)) \
				{ \
					QCLogFileMT::WriteAssertLog(#expr, __FILE__, __LINE__); \
				} \
			} while (0)

		#define EXPRLOG(expr) \
			do { \
				if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_ASSERT) && !(expr)) \
				{ \
					QCLogFileMT::WriteAssertLog(#expr, __FILE__, __LINE__); \
				} \
			} while (0)

	#endif // _DEBUG

#else // NOEXPRLOG

	#define VERIFYLOG(expr) VERIFY(expr)
	#define EXPRLOG(expr)   ASSERT(expr)

#endif // NOEXPRLOG

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2643AF57_E81A_11D1_90A9_00805F9B8439__INCLUDED_)
