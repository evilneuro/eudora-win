// This is a part of the Stingray Extension Classes.
// Copyright (C) 1995 Stingray Software, Inc.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes 
// See the Stingray Extension Classes help files for 
// detailed information regarding using SEC classes.
//
// CHANGELOG: 
//
//    MSW     8/30/95  Started
//
//

#ifndef __SECDLL_H__
#define __SECDLL_H__

#ifdef _SECEXT
#undef _SECDLL
#define _SECDLL
#endif

#ifdef _SECDLL

#ifndef SEC_DATAEXT
	#define SEC_DATAEXT     AFX_DATA_IMPORT
	#define SEC_DATADEF
#endif

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif

extern HINSTANCE SECGetResourceHandle();
// *****************************************
// Note: May need to add a CDynLinkLibrary
// derivative here to encapsulate any global
// SEC state.
// *****************************************

//
// SEC extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR


#endif // _SECDLL

#endif // __SECDLL_H__

/////////////////////////////////////////////////////////////////////////////
