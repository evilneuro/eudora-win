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
//  Author:		Scot Wingo
//  Description:	SECTNWinApp - A WinApp derivative that automatically
//                      uses a thumbnail file dialog.  A very thin class that
//                      really just replaces CWinApp::OnFileOpen with our own
//  Created:		Wed Jul 26 18:48:12 1995
//
//

#ifndef __SECTNWIN_H__
#define __SECTNWIN_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


class SECTNWinApp : public CWinApp
{
    
//Construction
public:
    SECTNWinApp(); //does nothing
// Overrides
    
// Implementation
    afx_msg void OnFileOpen();    
};

//snagged from appdlgs.h
static void AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
			       CDocTemplate* pTemplate, CString* pstrDefaultExt);

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECTNWIN_H__
