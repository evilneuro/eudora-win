//////////////////////////////////////////////////////////////////////////////
// DSMailImpl.cpp
// 
//
// Created: 09/24/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_MAIL_INTERFACE_IMPLEMENTATION_
#include "DSMailImpl.h"

void
DSMail::OnAddress(UINT HeaderNum, CString& strMailees, CWnd *pParent,
		  CFrameWnd *pParentFrame, BOOL bKeepOnTop)
{
    // Code mostly duplicated from lookup.cpp.
    CMainFrame *pMainFrame  = static_cast<CMainFrame *>(::AfxGetMainWnd());
    ASSERT_KINDOF(CMainFrame, pMainFrame);

    CWazooWnd *pParentWazoo = static_cast<CWazooWnd *>(pParent);
    ASSERT_KINDOF(CWazooWnd, pParentWazoo);

    CCompMessageDoc *comp
	= pMainFrame->HuntForTopmostCompMessage(pParentWazoo);

    // The moment of truth ...  did we find a target Comp message
    // window or not?
    BOOL NewComp = FALSE;
    if (!comp) {
	if (!(comp = NewCompDocument()))
	    return;
	NewComp = TRUE;
    }

    char* OldLine = ::SafeStrdupMT(comp->GetHeaderLine(HeaderNum));
    char* NewLine = OldLine;

    if (OldLine) {
	// Find end of old header and remove ending comma because one
	// will be added later
	::TrimWhitespaceMT(OldLine);
	char* t = OldLine + strlen(OldLine);
	if (t-- > OldLine && *t == ',')
	    *t = 0;
    }
    
    CString mailStr = NewLine;
    delete [] NewLine;

    if (!mailStr.IsEmpty()) {
	mailStr += ",";
	mailStr += strMailees;
    }
    else {
	mailStr = strMailees;
    }

//     if (NewComp) {
// 	comp->m_Sum->Display();
//     }

    BOOL bKOT = FALSE;

    SECControlBar *pParentBar =
 	(SECControlBar *) pParentWazoo->GetParentControlBar();
    ASSERT_KINDOF(SECControlBar, pParentBar);

    if (pParentBar->IsMDIChild()) {
	bKOT = bKeepOnTop;
    }

    if (NewComp || !bKOT) {
	comp->m_Sum->Display();
    }

    comp->SetHeaderLine(HeaderNum, mailStr);
    comp->SetModifiedFlag(TRUE);
    
    if (pParentBar->IsMDIChild() && bKOT) {
	CMDIChildWnd *pMDIChildWnd = (CMDIChildWnd *) pParentFrame;
	ASSERT_KINDOF(CMDIChildWnd, pMDIChildWnd);
	pMDIChildWnd->MDIActivate();
    }
}

void
DSMail::CreateAddress(CString& formatStr, CString& nameStr, CString& emailStr)
{
    ::FormatAddress(formatStr, nameStr, emailStr);
}
