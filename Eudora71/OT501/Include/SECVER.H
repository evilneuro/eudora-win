// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//
//  Author:		Scot Wingo
//  Description:	Version information for SEC
//  Created:		Tue Aug 29 14:55:33 1995
//
//  CHANGELOG:
//    MSW       9/30/95  Code Review
// 

#ifndef __SECVER_H__
#define __SECVER_H__

/////////////////////////////////////////////////////////////////////////////
// Master version numbers
/////////////////////////////////////////////////////////////////////////////

#define _SEC     1      // SEC classes
#define _SEC_VER 0x0501 // SEC Version 5.01 FCS

#if (_MFC_VER > 0x0421)
// If you get this warning message, call Stingray for an update to
// Objective Toolkit that is compatible with this version of MFC.
#pragma message("Warning! This release of Objective Toolkit is not supported with versions")
#pragma message("         of MFC newer than 4.21. Contact Stingray Software to obtain an update.")
#endif

//
// Automatic library inclusion macros that use the #pragma/lib feature
//
// Define _SECNOAUTOLIB when compiling SEC so that it does
// not try and link to itself. -chicken and egg.
//
#ifndef _SECNOAUTOLIB

#if _MFC_VER == 0x0400
	#ifndef _UNICODE
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot40d.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot40r.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota40d.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota40r.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5040d.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5040r.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#else
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot40ud.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot40ur.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota40ud.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota40ur.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5040ud.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5040ur.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#endif
#elif _MFC_VER == 0x0410
	#ifndef _UNICODE
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot41d.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot41r.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota41d.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota41r.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5041d.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5041r.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#else
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot41ud.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot41ur.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota41ud.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota41ur.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5041ud.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5041ur.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#endif
#elif _MFC_VER == 0x0420 
	#ifndef _UNICODE
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot42d.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot42r.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota42d.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota42r.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5042d.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5042r.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#else
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot42ud.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot42ur.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota42ud.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota42ur.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5042ud.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5042ur.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#endif
#elif _MFC_VER >= 0x0421
	#ifndef _UNICODE
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot50d.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot50r.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota50d.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota50r.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5050d.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5050r.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#else
		#if !defined(_SECDLL) && !defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot50ud.lib"
		#elif !defined(_SECDLL) && !defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot50ur.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota50ud.lib"
		#elif !defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ota50ur.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5050ud.lib"
		#elif defined(_SECDLL) && defined(_AFXDLL) && !defined(_DEBUG)
			#define _SECAUTOLIBNAME "ot5050ur.lib"
		#elif defined(_SECDLL) && !defined(_AFXDLL)
			#error _SECDLL is defined but _AFXDLL is not. You must defined both.
		#endif
	#endif
#else //16-bit
	#if !(defined(_SECDLL) || defined(_SECEXT))
		#ifndef _AFXDLL
			#ifdef _DEBUG
				#define _SECAUTOLIBNAME "ot16d.lib"
			#else
				#define _SECAUTOLIBNAME "ot16r.lib"
			#endif
		#else
			#ifdef _DEBUG
				#define _SECAUTOLIBNAME "ota16d.lib"
			#else
				#define _SECAUTOLIBNAME "ota16r.lib"
			#endif
		#endif
	#else
		#ifdef _DEBUG
			#define _SECAUTOLIBNAME "ot5016d.lib"
		#else
			#define _SECAUTOLIBNAME "ot5016r.lib"
		#endif
	#endif
#endif //16-bits

#if _MFC_VER >= 0x0300

	// You may turn off SEC++ specific compilation message
	// by adding _SECNOMSG to your C/C++ settings.
#ifndef _SECNOMSG
#pragma message( "Objective Toolkit will automatically link with " _SECAUTOLIBNAME )
#pragma comment(lib, _SECAUTOLIBNAME)
#endif //_SECNOMSG

#endif //end of 32-bit message

#endif //_SECNOAUTOLIB
/////////////////////////////////////////////////////////////////////////////
// Standard preprocessor symbols if not already defined

#if !defined(_WIN32)
#include "gxtchar.h"

// provide types and macros that 32-bit wtypes.h usually provides
#define TEXT    _T
#define LPCTSTR LPCSTR
#define LPTSTR  LPSTR

#endif

#define SEC_DATA

#endif // __SECVER_H__
