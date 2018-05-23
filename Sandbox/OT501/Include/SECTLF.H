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
//  Author:			Dean Hallman
//  Description:	Declarations for SECToplevelFrame
//

#ifndef __SECTLF_H__
#define __SECTLF_H__

#ifndef __SWINFRM_H__
#include "swinfrm.h"
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
#endif //_SECDLL

class SECToplevelFrame : public SECFrameWnd
{  
    DECLARE_DYNCREATE(SECToplevelFrame);

// Construction
protected: // create from serialization only
    SECToplevelFrame();	

// Attributes:
protected:
    static CObList s_tlfList;	// Static list of toplevel frames

// Operations
public:
	// These functions required as a workaround to properly
	// import/export static member data s_tlfList across DLL boundary
	void GetCopyTLFList(CObList& rList);
	void SetTLFList(CObList& rNewList);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECToplevelFrame)
public:
    virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~SECToplevelFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    //{{AFX_MSG(SECToplevelFrame)
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnWindowNew();
    afx_msg void OnClose();
   //}}AFX_MSG
    DECLARE_MESSAGE_MAP();    
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif //__SECTLF_H__

/////////////////////////////////////////////////////////////////////////////
