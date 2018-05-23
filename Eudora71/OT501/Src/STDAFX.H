// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved.
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
//
// Header file that's the nexus for PCH stuff.
//  
#define WINVER 0x0410

// Turn on leak checking with malloc (must be defined before stdlib.h is included)
#define _CRTDBG_MAP_ALLOC

#include /* */ <afxwin.h>         // MFC core and standard components
#include /* */ <afxext.h>         // MFC extensions

#ifndef _AFX_NO_DB_SUPPORT
#include /* */ <afxdb.h>          // MFC database classes
#endif

#if _MFC_VER >= 0x0400
#ifndef _AFX_NO_DAO_SUPPORT
#include /* */ <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include /* */ <afxcmn.h>			// MFC support for Windows 95 Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#endif

#include /* */ <windowsx.h>

#include /* */ <afxpriv.h>
#include /* */ <ctype.h>

#ifdef _SECDLL 
	// needed for MFC 3.0
	#define SEC_DATAEXT	 	AFX_DATA_EXPORT
	#define SEC_DATADEF  
#endif

// No need to automatically link with Objective Toolkit libs for this build
#define _SECNOAUTOLIB

#include /* */ <secall.h>

#if _SEC_VER != 0x0501
#pragma message( "You have tried to compile with old Objective Toolkit header files." )
#pragma message( "Please make sure that you have correctly setup your include directory!" )
#error Wrong header files.
#endif

// BEGIN QUALCOMM NEW_CODE

#include "qcassert.h"		// ASSERT/VERIFY logging

// END QUALCOMM NEW_CODE
