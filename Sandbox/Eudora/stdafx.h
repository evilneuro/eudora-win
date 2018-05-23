// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __EUDORA_STDAFX_H_
#define __EUDORA_STDAFX_H_
///////////////////////////////////////////////////////////////////////////////
// Microsoft Foundation Class Library includes

#if _MSC_VER >= 1000
#pragma once
#endif

#define VC_EXTRALEAN

#include <afxwin.h>					// MFC core and standard components
#include <afxext.h>					// MFC extensions (including VB)
#include <afxcmn.h>					// MFC common controls
#include <afxole.h>					// MFC COleDropTarget, etc.
#ifndef WIN32
#error "This project is for WIN32 only"
#endif // WIN32


///////////////////////////////////////////////////////////////////////////////
// ActiveX Template Library includes

#include "CEudoraModule.h"  // template library is based off of our Eudora Module

///////////////////////////////////////////////////////////////////////////////
// Stingray Objective Toolkit includes

#include "secall.h"			// Stingray Objective Toolkit


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

///////////////////////////////////////////////////////////////////////////////
// Warning exclusions

#pragma warning(disable : 4706 4100)


#endif // __EUDORA_STDAFX_H_
