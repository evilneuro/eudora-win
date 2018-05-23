// NICKVIEW.CPP
//
// LHS (left-hand side) and RHS (right-hand side) CFormView classes 
// for the Address Book splitter window.  There are two implementations 
// of the LHS view -- one for 16-bit and the other for 32-bit, but much of the
// common stuff is handled in a LHS base class that is common
// to both 16-bit and 32-bit.
//
// Copyright (c) 1995-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>
#include <ctype.h>

#include "eudora.h"
#include "helpxdlg.h"
#include "resource.h"
#include "rs.h"
#include "guiutils.h"
#include "cursor.h"
#include "fileutil.h"
#include "doc.h"
#include "nickdoc.h"
#include "3dformv.h"
#include "mainfrm.h"

#include "nicktree.h"	// for CNicknamesViewLeft32

#include "urledit.h"	// for NICKPAGE.H
#include "nickpage.h"	// for NICKSHT.H
#include "nicksht.h"	// for NICKVIEW.H
#include "nickview.h"
#include "utils.h"
#include "address.h"
#include "summary.h"
#include "tocdoc.h"
#include "mdichild.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "namenick.h"
#include "helpcntx.h"
#include "NicknamesWazooWnd.h"
#include "headervw.h" // CHeaderView

#include "QCCommandActions.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"

#include "QCSharewareManager.h"
#include "QCFindMgr.h"
#include "textfileview.h"
#include "textfile.h"


extern QCRecipientDirector	g_theRecipientDirector;

#include "DebugNewHelpers.h"

// Necessary for 16-bit context menus
#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU WM_RBUTTONUP
#endif


//
// FORNOW, these should probably be moved to a single common place,
// like an enum in a class header.
//
#define LIST_NONE 0x0
#define LIST_ON 0x1
#define LIST_OFF 0x2
#define LIST_BOTH (LIST_ON | LIST_OFF)



/////////////////////////////////////////////////////////////////////////////
// CNicknameEdit

CNicknameEdit::CNicknameEdit()
	: CURLEdit(),
	m_strEditText(_T(""))
	
{
}

CNicknameEdit::~CNicknameEdit()
{
}

void CNicknameEdit::UpdateEdit(bool bValidate /* =true*/, bool bIsKillFocus /* =false*/)
{
	CString		strEditText;
	GetWindowText(strEditText);
	if (!(m_strEditText == strEditText) || bValidate) // on WM_KILLFOCUS , the two strings will be equal but we need to do this anyways!
	{
		m_strEditText = strEditText;
		CNicknamesViewRight	*pWndParent = reinterpret_cast<CNicknamesViewRight *>(GetParent());
		if (pWndParent)
		{
			pWndParent->NicknameEditChanged(m_strEditText, bValidate, bIsKillFocus);
		}
	}
}

LRESULT CNicknameEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// Skip CURLEdit::WindowProc which doesn't do anything useful for us and
	// directly call CRichEditCtrl::WindowProc. (CURLEdit::WindowProc contains
	// a fix for modal dialog activation, which doesn't affect us, and does
	// URL highlighting, which we don't want).
	LRESULT		result = CRichEditCtrl::WindowProc(message, wParam, lParam);

	if ((message == WM_KILLFOCUS) || (message == WM_SETFOCUS))
		UpdateEdit(true, (message == WM_KILLFOCUS) );
	else if ( (message == WM_CHAR) || (message == WM_KEYUP) || (message == WM_PASTE))
		UpdateEdit(false);

	return result;
}


/////////////////////////////////////////////////////////////////////////////
// CRecipientListButton

CRecipientListButton::CRecipientListButton()
	: CButton(),
	m_bChecked(FALSE)
{
}

CRecipientListButton::~CRecipientListButton()
{
}

void CRecipientListButton::ButtonClicked()
{
	BOOL		bChecked = GetCheck();
	if (bChecked != m_bChecked)
	{
		m_bChecked = bChecked;
		CNicknamesViewRight	*pWndParent = reinterpret_cast<CNicknamesViewRight *>(GetParent());
		if (pWndParent)
		{
			pWndParent->RecipientCheckboxClicked(m_bChecked);
		}
	}
}

LRESULT CRecipientListButton::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = CButton::WindowProc(message, wParam, lParam);
	if (message == BM_SETCHECK)
	{
		ButtonClicked();
	}
	return lr;
}

/////////////////////////////////////////////////////////////////////////////
// CBPListButton

CBPListButton::CBPListButton()
	: CButton(),
	m_bChecked(FALSE)
{
}

CBPListButton::~CBPListButton()
{
}

void CBPListButton::ButtonClicked()
{
	BOOL		bChecked = GetCheck();
	if (bChecked != m_bChecked)
	{
		m_bChecked = bChecked;
		CNicknamesViewRight	*pWndParent = reinterpret_cast<CNicknamesViewRight *>(GetParent());
		if (pWndParent)
		{
			pWndParent->BPCheckboxClicked(m_bChecked);
		}
	}
}

LRESULT CBPListButton::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = CButton::WindowProc(message, wParam, lParam);
	if (message == BM_SETCHECK)
	{
		ButtonClicked();
	}
	return lr;
}


/////////////////////////////////////////////////////////////////////////////
// CNicknamesViewRight

IMPLEMENT_DYNCREATE(CNicknamesViewRight, CFormView)

CNicknamesViewRight::CNicknamesViewRight()
	: CFormView(IDR_NICKNAMES_RIGHT),
	  m_bIsInitialized(FALSE)
{

	//TRACE0("CNicknamesViewRight::CNicknamesViewRight()\n");
	//{{AFX_DATA_INIT(CNicknamesViewRight)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//
	// Make sure the rich edit control is loaded, so that
	// the RICHEDIT window class is registered with Windows.
	//
	HINSTANCE h_inst = ::LoadLibrary("RICHED32.DLL");
	ASSERT(h_inst != NULL);
#ifdef _DEBUG
	WNDCLASS wcls;
	ASSERT(::GetClassInfo(NULL, "RICHEDIT", &wcls));
#endif

	m_selNickName = NULL;
	m_focusNickName = NULL;
	m_bFoundDupNickName = false;

}

CNicknamesViewRight::~CNicknamesViewRight()
{
}


void CNicknamesViewRight::DoDataExchange(CDataExchange* pDX)
{
	//TRACE0("CNicknamesViewRight::DoDataExchange()\n");
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNicknamesViewRight)
	//}}AFX_DATA_MAP
	
	if (pDX->m_bSaveAndValidate)
	{
	}
}


BEGIN_MESSAGE_MAP(CNicknamesViewRight, CFormView)
	//{{AFX_MSG_MAP(CNicknamesViewRight)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND( ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND( ID_FILE_PRINT_DIRECT, OnFilePrintOne)
	ON_UPDATE_COMMAND_UI( ID_FILE_PRINT_DIRECT, OnFilePrintOneUpdateUI)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnFilePrintUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnFilePrintPreviewUpdateUI)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnFileSaveAsUpdateUI)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CNicknamesViewRight::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	TRACE0("CNicknamesViewRight::OnCreate()\n");
	int nReturn;
	if ((nReturn = CFormView::OnCreate(lpCreateStruct)) != 0)
		return nReturn;

	//
	// Create the property sheet.  Make sure the SYSMENU
	// style is removed so that the Alt-minus shortcut doesn't popup
	// the system menu, allowing you to close the property sheet.
	//
	m_PropSheet.Create(this, WS_VISIBLE|WS_CHILD|WS_TABSTOP, 0);


	// Create the Recipient List check box.
	m_RecipientListCBox.Create(CRString(IDS_RECIPIENTLIST_LABEL), BS_AUTOCHECKBOX|WS_VISIBLE|WS_CHILD|WS_TABSTOP, CRect(275,7,400,21), this, 3);

	//create the Boss Protector check box
	//make the check box visible only in paid mode
	if (UsingPaidFeatureSet()) 
		m_BPListCBox.Create(CRString(IDS_BPLIST_LABEL), BS_AUTOCHECKBOX|WS_VISIBLE|WS_CHILD|WS_TABSTOP, CRect(275,24,400,38), this, 4);
	else
		m_BPListCBox.Create(CRString(IDS_BPLIST_LABEL), BS_AUTOCHECKBOX|WS_CHILD, CRect(275,24,400,38), this, 4);
	
/*  commented out for now - since GetOutputTextExtent does not give very accurate results
	CDC *pDC = NULL;
	CSize size;
	pDC =m_RecipientListCBox.GetDC();
	size.cx = 100;//400-300
	size.cy = 14;//24-10
	if(pDC);
		{
			size = pDC->GetOutputTextExtent(CRString(IDS_RECIPIENTLIST_LABEL));
		}
	m_RecipientListCBox.ReleaseDC(pDC);
	m_RecipientListCBox.MoveWindow(300,10,size.cx, size.cy);
*/
	// Create the Nickname label.
	m_NickNameLabel.Create(CRString(IDS_NICKNAME_LABEL), WS_VISIBLE|WS_CHILD, CRect(8,6,75,30), this, 1);
	m_NickNameLabel.SetWindowText(CRString(IDS_NICKNAME_LABEL));
/*	pDC =m_NickNameLabel.GetDC();
	size.cx = 67;//75-8
	size.cy = 24;//30-6
	if(pDC);
		{
			size = pDC->GetOutputTextExtent(CRString(IDS_NICKNAME_LABEL));
		}
	m_NickNameLabel.ReleaseDC(pDC);
	m_NickNameLabel.MoveWindow(8,6,size.cx, size.cy);

*/
	// Create the Nickname edit.
	m_NickNameEdit.Create(WS_VISIBLE|WS_CHILD|WS_BORDER|WS_TABSTOP, CRect(75,2,200,26), this, 2);

	return 0;
}


void CNicknamesViewRight::OnInitialUpdate()
{
	if (m_bIsInitialized)
	{
		ASSERT(0);
		return;			// guard against bogus double initializations
	}

	//TRACE0("CNicknamesViewRight::OnInitialUpdate()\n");
	CFormView::OnInitialUpdate();

	m_bIsInitialized = TRUE;

	//
	// Since we're creating the View manually, we need to register this
	// view with the Document manually.
	//
	{
		CNicknamesDoc* doc = GetDocument();
		ASSERT_KINDOF(CNicknamesDoc, doc);
		doc->AddView(this);
	}

	//
	// Position and size the property sheet, checkbox and edit.
	//
	{
		CRect rectWnd;
		m_PropSheet.GetWindowRect(rectWnd);

		ScreenToClient(&rectWnd);
		m_PropSheet.SetWindowPos(NULL, 0, 40, 
									rectWnd.Width(),
									rectWnd.Height() + 200 - 40,
									SWP_NOZORDER | SWP_NOACTIVATE);
		m_PropSheet.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		CRect		rectRCB;
		m_RecipientListCBox.GetWindowRect(&rectRCB);
		ScreenToClient(&rectRCB);
		m_RecipientListCBox.SetWindowPos(NULL, 
										 rectWnd.Width() - rectRCB.Width() - 25, rectRCB.TopLeft().y,
										 0, 0,
										 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

		CRect		rectBPCB;
		m_BPListCBox.GetWindowRect(&rectBPCB);
		ScreenToClient(&rectBPCB);
		m_BPListCBox.SetWindowPos(NULL, 
										 rectWnd.Width() - rectBPCB.Width() - 25, rectRCB.TopLeft().y + rectRCB.Height() + 5,
										 0, 0,
										 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);


		m_NickNameEdit.SetWindowPos(NULL, 0, 0, rectWnd.Width() - rectRCB.Width() - 16 - 32, 26, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		//we can have nicknames of maximum length - 32. hence we limit the amount of text that the user can enter
		m_NickNameEdit.LimitText(CNickname::MAX_NAME_LENGTH);
	}
	
    // Set the font of some of the controls to the dialog font
    m_NickNameEdit.SetFont(&ScreenFont, FALSE);
	m_NickNameLabel.SetFont(this->GetFont(), FALSE);
	m_RecipientListCBox.SetFont(this->GetFont(), FALSE);
	m_BPListCBox.SetFont(this->GetFont(), FALSE);

	//
	// Kludge to get DoDataExchange() to run to initialize all page
	// member variables with valid window handles.
	for (int page_idx = 0; page_idx < m_PropSheet.GetPageCount(); page_idx++)
		m_PropSheet.SetActivePage(page_idx);

	m_PropSheet.InitCustomPages();
	m_PropSheet.RegisterCustomFields();

	//
	// Now, that all of the pages are initialized, activate the last active page.
	//
	page_idx = GetIniShort(IDS_INI_LAST_ACTIVE_NICKNAME_PAGE);
	if (page_idx >= 0 && page_idx < m_PropSheet.GetPageCount())
		m_PropSheet.SetActivePage(page_idx);
	else
		m_PropSheet.SetActivePage(0);		// out of range, so force it back to default.

	//
	// FORNOW, disable all the controls and rely on the user's first
	// selection to set the control states properly.
	//
	EnableAllControls(FALSE);

    // Size the controls
	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
    OnSize(SIZE_RESTORED, -1, -1);
}


////////////////////////////////////////////////////////////////////////
// GetDocument [public]
//
// The following very impolitely hides the public, NON-VIRTUAL
// CView::GetDocument() method....  this is very, very bad C++ style.
// :-) In Eudora 3.x or earlier, this was just a convenient way to
// type cast the CDocument pointer to a derived CNicknamesDoc pointer.
// In Eudora 4.x or later, this override actually makes sure that we
// create and load up the One True CNicknamesDoc(tm) referred to by
// 'g_Nicknames'.
////////////////////////////////////////////////////////////////////////
CNicknamesDoc* CNicknamesViewRight::GetDocument()
{
	ASSERT(g_Nicknames);
	ASSERT_KINDOF(CNicknamesDoc, g_Nicknames);
	return g_Nicknames;
}


/////////////////////////////////////////////////////////////////////////////
// CNicknamesViewRight message handlers

void CNicknamesViewRight::OnSize(UINT nType, int cx, int cy)
{
	//TRACE3("CNicknamesViewRight::OnSize(%d,%d,%d)\n", nType, cx, cy);
	CFormView::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! m_bIsInitialized) || (SIZE_MINIMIZED == nType))
		return;
	
	CRect rect;					// for overall CFormView client area
	CRect propsheet_rect;		// for CNicknamesPropertySheet
	CRect proppage_rect;		// for active CNicknamesPropertyPage
	CRect tabctrl_rect;			// for CNicknamesPropertySheet tab control
	CRect edit_rect;			// for Nickname edit
	CRect checkbox_rect;		// for Recipient List checkbox
	CRect bp_checkbox_rect;		// for Boss Protector List checkbox

    CTabCtrl* p_tabctrl = m_PropSheet.GetTabControl();
	ASSERT(p_tabctrl != NULL);

	//
	// Fetch overall client rect in order to determine whether or not
	// this RHS view is open or closed.
	//
	GetClientRect(&rect);
	if (0 == rect.Width())
	{
		//
		// Since the client area has zero width, then the splitter
		// window has collapsed our view.  We can safely bail outta
		// here since there is no sizing stuff to do.  Before we
		// leave, though, notify the LHS that the RHS has closed.
		//
		//TRACE0("CNicknamesViewRight::OnSize() -- RHS closed\n");
		if (g_Nicknames)
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_RHS_CLOSED, NULL);
		return;
	}
	else
	{
		//
		// Notify the LHS that the RHS is still open for business.
		//
		//TRACE0("CNicknamesViewRight::OnSize() -- RHS open\n");
		if (g_Nicknames)
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_RHS_OPEN, NULL);
	}

	//
	// If the client window dimensions fall below certain reasonable
	// minimums, then fool the proportional sizing code below to think
	// it has a larger window.  This causes the displayed controls to
	// be clipped from the window rather than causing them to shrink
	// unusably small.
	//
	ASSERT((0 == rect.left) && (0 == rect.top));
	if (rect.right < 180)
		rect.right = 180;
	if (rect.bottom < 110)
		rect.bottom = 110;

	// Fetch control client rects
	m_PropSheet.GetWindowRect(&propsheet_rect);
	ScreenToClient(&propsheet_rect);
	m_NickNameEdit.GetWindowRect(&edit_rect);
	ScreenToClient(&edit_rect);
	m_RecipientListCBox.GetWindowRect(&checkbox_rect);
	ScreenToClient(&checkbox_rect);
	m_BPListCBox.GetWindowRect(&bp_checkbox_rect);
	ScreenToClient(&bp_checkbox_rect);

	// Fetch sub-control client rects
	p_tabctrl->GetWindowRect(&tabctrl_rect);
	m_PropSheet.ScreenToClient(&tabctrl_rect);
                
	CNicknamesPropertyPage* p_page = (CNicknamesPropertyPage *) m_PropSheet.GetActivePage();
	ASSERT(p_page != NULL);
	ASSERT(p_page->IsKindOf(RUNTIME_CLASS(CNicknamesPropertyPage)));
	p_page->GetWindowRect(&proppage_rect);
	m_PropSheet.ScreenToClient(&proppage_rect);

	// Some pre-established values
	const int MARGIN = 4;
	
	//
	// Resize the property sheet, tab control (32-bit only), and 
	// the active property page.
	//
	propsheet_rect.right = rect.right;
	propsheet_rect.bottom = rect.bottom;
	tabctrl_rect.right = propsheet_rect.Width() - MARGIN;
	tabctrl_rect.bottom = propsheet_rect.Height() - MARGIN;
	proppage_rect.right = tabctrl_rect.Width();
	proppage_rect.bottom = tabctrl_rect.Height();

	//
	// Resize the checkbox and edit.
	//
	//int		cbWidth = checkbox_rect.Width();
	int cbWidth = (checkbox_rect.Width() > bp_checkbox_rect.Width()) ? checkbox_rect.Width() : bp_checkbox_rect.Width();
	checkbox_rect.right = rect.right - 4;
	checkbox_rect.left = checkbox_rect.right - cbWidth;
	bp_checkbox_rect.right = checkbox_rect.right;
	bp_checkbox_rect.left = checkbox_rect.left;

	edit_rect.right = checkbox_rect.left - 16;

	m_PropSheet.MoveWindow(&propsheet_rect, TRUE);
	m_NickNameEdit.MoveWindow(&edit_rect, TRUE);
	m_RecipientListCBox.MoveWindow(&checkbox_rect, TRUE);
	m_BPListCBox.MoveWindow(&bp_checkbox_rect, TRUE);
	p_tabctrl->MoveWindow(&tabctrl_rect, TRUE);
	p_page->MoveWindow(&proppage_rect, TRUE);

	//
	// Finally, cache the new property page size in the child property
	// sheet.  This page size is used when we switch property pages.
	//
	m_PropSheet.m_PageRect = proppage_rect;
}


////////////////////////////////////////////////////////////////////////
// ResizeCurrentPropertyPage [public]
//
// The CPropertySheet class caches the *initial* size of its property
// pages.  This causes problems when we resize this view.  Therefore,
// this is a kludge hook that allows the child property sheet to notify
// us (the parent) whenever the current property page switches.  We use
// the new property page size that is cached by the child property sheet
// whenever it is resized (see the OnSize() handler for this view).
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewRight::ResizeCurrentPropertyPage(const CRect& pageRect)
{
	//TRACE0("CNicknamesViewRight::ResizeCurrentPropertyPage()\n");
	CNicknamesPropertyPage* p_page = (CNicknamesPropertyPage *) m_PropSheet.GetActivePage();
	ASSERT(p_page != NULL);
	ASSERT(p_page->IsKindOf(RUNTIME_CLASS(CNicknamesPropertyPage)));
	p_page->MoveWindow(&pageRect, TRUE);
}


void CNicknamesViewRight::NicknameEditChanged(CString strEditText, bool bValidate /*=false*/, bool bIsKillFocus /*=false*/)
{

	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if (pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if (pViewLeft)
		{
			CObList			 nicknameList;
			pViewLeft->GetSelectedNicknames(nicknameList);
			if(!nicknameList.IsEmpty())
			{
			CNickname*		 nickname = reinterpret_cast<CNickname *>(nicknameList.GetHead());
			if (nickname)
			{
				if(bValidate) //WM_KILLFOCUS message special case
				{
					nickname->SetValidateMode(false);
					if( strEditText != nickname->GetName())
					{
						if(!CNickname::IsValidNickName(strEditText))
						{
							MessageBeep(0);
							ASSERT(0);
						}		
						return;
					}
					

					if(bIsKillFocus) {
						
						if(CheckEmptyNickname(strEditText) || pViewLeft->CheckDuplicateNickname(nickname)) {

							m_bFoundDupNickName = true;
							if(m_NickNameEdit) {
								m_NickNameEdit.SetWindowText(m_focusNickName);
								nickname->SetName(m_focusNickName);
								pViewLeft->NicknameChanged(nickname);
								strEditText = m_focusNickName;
							} else {
								ASSERT(0);
							}

							m_NickNameEdit.SetFocus();
							m_NickNameEdit.SetSel(0, -1);

							//m_NickNameEdit.PostMessage(WM_ACTIVATE);
							//::SetActiveWindow(m_NickNameEdit);

/*
							m_NickNameEdit.PostMessage(WM_SETFOCUS);  
							m_NickNameEdit.PostMessage(WM_MOUSEACTIVATE);
							m_NickNameEdit.PostMessage(EM_SETSEL, 0, -1);
							m_NickNameEdit.PostMessage(WM_LBUTTONDOWN);
							m_NickNameEdit.PostMessage(WM_LBUTTONUP);*/
							//pViewLeft->PostMessage(WM_NCACTIVATE);
							

						} else {
							m_bFoundDupNickName = false;
							CString nicknameEditText;
							m_NickNameEdit.GetWindowText(nicknameEditText);
							nickname->SetName(nicknameEditText);
							pViewLeft->NicknameChanged(nickname);
							strEditText = nicknameEditText;
						}
					}
		

					//on focus save the nickname for future reference to know if it has been changed
					if(!bIsKillFocus) {
						if(m_selNickName)
							delete [] m_selNickName;
						m_selNickName = NULL;

						if (nickname->IsRecipient()) {
							m_selNickName = DEBUG_NEW char[256];
							strcpy(m_selNickName, (LPCTSTR)nickname->GetName());
						}
						
						if(!m_bFoundDupNickName) {
							m_focusNickName = DEBUG_NEW char[256];
							strcpy(m_focusNickName, (LPCTSTR)nickname->GetName());
						}

					}

					//refresh recepient list when the user renames the nickname
					if(bIsKillFocus) {
						QCRecipientCommand*     pCommand;
						if(m_selNickName) {
							pCommand = g_theRecipientDirector.Find( m_selNickName );
							if( pCommand != NULL )
								pCommand->Execute( CA_RENAME, ( void* ) (LPCSTR) nickname->GetName() );
						}
					}
		
				}

				nickname->SetValidateMode(true); //make validation active - just to be sure
				nickname->SetValidateMode(bValidate); //bValidate = true on WM_KILLFOCUS and false otherwise
				nickname->SetName(strEditText); //strEditText could be invalid but assert only on WM_KILLFOCUS 
				if (!bValidate || nickname->HasValidName())
				{
					if(!bValidate) //dont mark it dirty
					{
						pViewLeft->NicknameChanged(nickname);
						g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
					}
				}
				else
				{
					MessageBeep(0);
					nickname->CreateValidName(NULL);

						m_NickNameEdit.SetWindowText(nickname->GetName());
					pViewLeft->NicknameChanged(nickname);
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
				}
			nickname->SetValidateMode(true);

			}
			}
		}
	}
}


bool CNicknamesViewRight::CheckEmptyNickname(CString nickname) {

	bool bEmpty = (nickname && nickname.IsEmpty());
	if(bEmpty)
		ErrorDialog(IDS_ERR_NICK_EMPTY);
	return bEmpty;
}

void CNicknamesViewRight::RecipientCheckboxClicked(BOOL bChecked)
{
	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if (pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if (pViewLeft)
		{
			CObList			 nicknameList;
			pViewLeft->GetSelectedNicknames(nicknameList);
			if (!nicknameList.IsEmpty())
			{
				CNickname*		 nickname;
				//loop through all of the selected nicknames and add them to the recipient list
				for(POSITION pos = nicknameList.GetHeadPosition(); pos != NULL; ) {
					nickname = reinterpret_cast<CNickname *>(nicknameList.GetNext(pos));
					if (nickname)
					{
						if (bChecked)
						{
							nickname->AddRecipient();
							pViewLeft->RefreshRecipListDisplay();
						}
						else
						{
							nickname->RemoveRecipient();
						}
					}
				}
			}
		}
	}
}



void CNicknamesViewRight::BPCheckboxClicked(BOOL bChecked)
{
	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if (pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if (pViewLeft)
		{
			CObList			 nicknameList;
			pViewLeft->GetSelectedNicknames(nicknameList);
			if (!nicknameList.IsEmpty())
			{
				CNickname*		 nickname;
				//loop through all of the selected nicknames and add them to the BP list
				for(POSITION pos = nicknameList.GetHeadPosition(); pos != NULL; ) {
					nickname = reinterpret_cast<CNickname *>(nicknameList.GetNext(pos));
					if (nickname)
					{
						if (bChecked)
						{
							nickname->AddToBP();
							pViewLeft->RefreshBPListDisplay();
						}
						else
						{
							nickname->RemoveFromBP();
							pViewLeft->RefreshBPListDisplay();
						}
					}
				}
			}

			CObList			 nickfileList;
			pViewLeft->GetSelectedNicknamefiles(nickfileList);
			if (!nickfileList.IsEmpty())
			{
				CNicknameFile*		 nickfile;
				//loop through all of the selected nickfiles and add them to the BP list
				for(POSITION pos = nickfileList.GetHeadPosition(); pos != NULL; ) {
					nickfile = reinterpret_cast<CNicknameFile *>(nickfileList.GetNext(pos));
					if (nickfile)
					{
						if (bChecked)
						{
							if(!nickfile->IsBPList()) {
								nickfile->SetBPList();
								g_Nicknames->AddABToBPList(nickfile);
								pViewLeft->RefreshBPListDisplay();
							}
						}
						else
						{
							if(nickfile->IsBPList()) {
								nickfile->SetBPList(FALSE);
								g_Nicknames->RemoveABFromBPList(nickfile);
								pViewLeft->RefreshBPListDisplay();
							}
						}
					}
				}
			}

			
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
// Save active nickname page to the INI file for next time.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewRight::OnDestroy()
{
	int page_idx = m_PropSheet.GetActiveIndex();

	SetIniShort(IDS_INI_LAST_ACTIVE_NICKNAME_PAGE, short(page_idx));

	CFormView::OnDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnUpdate [protected]
//
// Handle nickname view "update" protocol.  This is the handler for
// the UpdateAllViews() call in the nicknames document class.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewRight::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	//TRACE0("CNicknamesViewRight::OnUpdate()\n");
	switch (lHint)
	{
	case NICKNAME_HINT_NOSELECT_RECIPLIST_DISABLE:
	case NICKNAME_HINT_MULTI_NICKFILE_SELECTED:
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		m_NickNameEdit.SetWindowText("");
		m_NickNameEdit.EnableWindow(FALSE);
		m_NickNameLabel.EnableWindow(FALSE);
		m_RecipientListCBox.SetCheck(BST_UNCHECKED);
		m_RecipientListCBox.m_bChecked = FALSE;
		m_RecipientListCBox.EnableWindow(FALSE);
		break;
	case NICKNAME_HINT_SINGLE_NICKFILE_SELECTED:
		{
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		m_NickNameEdit.SetWindowText("");
		m_NickNameEdit.EnableWindow(FALSE);
		m_NickNameLabel.EnableWindow(FALSE);
		m_RecipientListCBox.SetCheck(BST_UNCHECKED);
		m_RecipientListCBox.m_bChecked = FALSE;
		m_RecipientListCBox.EnableWindow(FALSE);
		CNicknameFile* p_nickfile = (CNicknameFile *) pHint;
		ASSERT(p_nickfile != NULL);
		ASSERT(p_nickfile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));
		if (p_nickfile->IsBPList()) {
			m_BPListCBox.SetCheck(BST_CHECKED);
			m_BPListCBox.m_bChecked = TRUE;
		} else {
			m_BPListCBox.SetCheck(BST_UNCHECKED);
			m_BPListCBox.m_bChecked = FALSE;
		}
		}
		break;
	case NICKNAME_HINT_MULTISELECT_RECIPLIST_CHECK:
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		m_RecipientListCBox.SetCheck(BST_CHECKED);
		m_RecipientListCBox.m_bChecked = TRUE;
		m_BPListCBox.SetCheck(BST_UNCHECKED);
		m_BPListCBox.m_bChecked = FALSE;
		break;
	case NICKNAME_HINT_MULTISELECT_BPLIST_CHECK:
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		m_RecipientListCBox.SetCheck(BST_UNCHECKED);
		m_RecipientListCBox.m_bChecked = FALSE;
		m_BPListCBox.SetCheck(BST_CHECKED);
		m_BPListCBox.m_bChecked = TRUE;
		break;
	case NICKNAME_HINT_MULTISELECT_RECIPLIST_BPLIST_CHECK:
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		m_RecipientListCBox.SetCheck(BST_CHECKED);
		m_RecipientListCBox.m_bChecked = TRUE;
		m_BPListCBox.SetCheck(BST_CHECKED);
		m_BPListCBox.m_bChecked = TRUE;
		break;
	case NICKNAME_HINT_MULTISELECT_RECIPLIST_BPLIST_UNCHECK:
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		m_RecipientListCBox.SetCheck(BST_UNCHECKED);
		m_RecipientListCBox.m_bChecked = FALSE;
		m_BPListCBox.SetCheck(BST_UNCHECKED);
		m_BPListCBox.m_bChecked = FALSE;
		break;
	case NICKNAME_HINT_DISPLAYNICKNAME:
	case NICKNAME_HINT_DISPLAYNICKNAME_READONLY:
		{
			ClearAllControls();

			CNickname* p_nickname = (CNickname *) pHint;
			ASSERT(p_nickname != NULL);
			ASSERT(p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));
			ASSERT(! p_nickname->NeedsReadIn());
			CURLEdit	*pCEdit = m_PropSheet.m_personalPage.GetEditControlByLabel(kABookAddressLabel);
			if (pCEdit)
			{
				pCEdit->SetWindowText(p_nickname->GetAddresses());
				pCEdit->SetModify(FALSE);
			}
			m_PropSheet.m_notesPage.m_NotesEdit.SetWindowText(p_nickname->GetNotes());
			m_PropSheet.PopulateCustomEditControls(p_nickname);
			m_PropSheet.m_notesPage.m_NotesEdit.SetModify(FALSE);

			m_NickNameEdit.EnableWindow(TRUE);
			m_NickNameLabel.EnableWindow(TRUE);
			m_RecipientListCBox.EnableWindow(TRUE);
			if (!p_nickname->HasValidName())
			{
			p_nickname->CreateValidName(NULL);
			CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
			if (pWndSplitter)
				{
					CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
					pViewLeft->NicknameChanged(p_nickname);
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
				}
			}
			m_NickNameEdit.SetWindowText(p_nickname->GetName());

			if (p_nickname->IsRecipient())
			{
				m_RecipientListCBox.SetCheck(BST_CHECKED);
				m_RecipientListCBox.m_bChecked = TRUE;
			}
			else
			{
				m_RecipientListCBox.SetCheck(BST_UNCHECKED);
				m_RecipientListCBox.m_bChecked = FALSE;
			}

			CString bp;
			p_nickname->GetNotesFieldValue(CRString(IDS_NICK_FIELD_BOSS_PROTECTOR), bp);
			if(strcmp(bp, CRString(IDS_NICK_FIELD_BOSS_PROTECTOR_YES)) == 0) 
			{
				m_BPListCBox.SetCheck(BST_CHECKED);
				m_BPListCBox.m_bChecked = TRUE;
			}
			else
			{
				m_BPListCBox.SetCheck(BST_UNCHECKED);
				m_BPListCBox.m_bChecked = FALSE;
			}

			//
			// Decide whether or not to allow editing of nickname fields.
			// The exception is the recipient list checkbox ... it's
			// enabled even for read-only nicknames.
			//
			if (NICKNAME_HINT_DISPLAYNICKNAME_READONLY == lHint)
				EnableAllControls(FALSE);
			else
				EnableAllControls(TRUE);

			UpdateData(FALSE);
		}
		break;
	case NICKNAME_HINT_SENDING_NICKNAME:
		{
			//
			// LHS is returning singly-selected nickname that
			// we requested.  So, let's update it with any changed data
			// fields.
			//
			CNickname* p_nickname = (CNickname *) pHint;	// type cast
			ASSERT(p_nickname != NULL);
			ASSERT(p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));

			CURLEdit	*pCEdit = m_PropSheet.m_personalPage.GetEditControlByLabel(kABookAddressLabel);
			if (pCEdit && (pCEdit->GetModify()))
			{
				CString newaddr;
				pCEdit->GetWindowText(newaddr);
				p_nickname->SetAddresses(newaddr);
				p_nickname->SetAddressesLength(newaddr.GetLength());
				pCEdit->SetModify(FALSE);
				if (g_Nicknames)
				{
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_NICKNAME_WAS_CHANGED, p_nickname);
				}
			}

			if (m_PropSheet.m_notesPage.m_NotesEdit.GetModify())
			{
				CString newnotes;
				m_PropSheet.m_notesPage.m_NotesEdit.GetWindowText(newnotes);
				p_nickname->SetNotes(newnotes);
				//FORNOWp_nickname->SetNotesLength(newnotes.GetLength());
				m_PropSheet.m_notesPage.m_NotesEdit.SetModify(FALSE);

				//
				// Unlike the other cases, the LHS never displays the Notes field, 
				// so we don't need to send a NICKNAME_WAS_CHANGED update message.
				//
			}

			if (m_PropSheet.SaveModifiedCustomEditControls(p_nickname) || m_NickNameEdit.GetModify())
			{
				//
				// Found at least one "dirty" edit field, so update the
				// display.
				//
				if (g_Nicknames)
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_NICKNAME_WAS_CHANGED, p_nickname);
			}
		}
		break;
	case NICKNAME_HINT_ACTIVATE_ADDRESS_PAGE:
	{
		//
		// The idea here is that the user has just created a new nickname.
		// So, to encourage the user to enter addresses for the nickname,
		// we set the addresses page on the RHS to be current and also
		// set the keyboard focus there.
		//
		m_PropSheet.SetActivePage(0);
		m_PropSheet.PostMessage(WM_USER_RESIZE_PAGE);
		//CURLEdit	*pCEdit = m_PropSheet.m_personalPage.GetEditControlByLabel(kABookAddressLabel);
		//if (pCEdit)
		//{
		//	PostMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, 0), LPARAM(pCEdit->m_hWnd));
		//}
		if(::IsWindow(m_NickNameEdit.m_hWnd))
			{
			m_NickNameEdit.SetFocus();
			m_NickNameEdit.SetSel(0,-1);//select all characters
			}

		break;
	}
	case NICKNAME_HINT_REFRESH_LHS:
		ClearAllControls();
		EnableAllControls(FALSE);
		UpdateData(FALSE);
		break;
	case NICKNAME_HINT_RECIPLIST_ADD:
	case NICKNAME_HINT_RECIPLIST_REMOVE:
	case NICKNAME_HINT_BP_ADD:
	case NICKNAME_HINT_BP_REMOVE:
	case NICKNAME_HINT_RECIPLIST_CHANGED:
	case NICKNAME_HINT_MARK_NICKFILE_DIRTY:
	case NICKNAME_HINT_REQUESTING_NICKNAME:
	case NICKNAME_HINT_NICKNAME_WAS_CHANGED:
	case NICKNAME_HINT_ADDED_NEW_NICKNAME:
	case NICKNAME_HINT_ADDED_NEW_NICKFILE:
	case NICKNAME_HINT_FETCH_SELECTED_NICKNAMES:
	case NICKNAME_HINT_REGISTER_VIEWBY_ITEM:
	case NICKNAME_HINT_RHS_CLOSED:
	case NICKNAME_HINT_RHS_OPEN:
		// do nothing
		break;
	default:
		// Make sure we're not applying default handling to one of our hints
		ASSERT(lHint <= NICKNAME_HINT_FIRST || lHint >= NICKNAME_HINT_LAST);
		CFormView::OnUpdate(pSender, lHint, pHint);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// ClearAllControls [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewRight::ClearAllControls()
{
	//TRACE0("CNicknamesViewRight::ClearAllControls()\n");
	if (! m_bIsInitialized)
		return;		// don't bother with the controls if they haven't been initialized yet

	for (int i = 0; i < m_PropSheet.GetPageCount(); i++)
	{
		CNicknamesPropertyPage* p_page = (CNicknamesPropertyPage *) m_PropSheet.GetNicknamePage(i);
		ASSERT(p_page != NULL);
		ASSERT(p_page->IsKindOf(RUNTIME_CLASS(CNicknamesPropertyPage)));

		//
		// Call base class virtual method to forward the message to the
		// derived class implementations.
		//
		p_page->ClearAllControls();
	}
}


////////////////////////////////////////////////////////////////////////
// EnableAllControls [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewRight::EnableAllControls(BOOL bEnable)
{
	//TRACE0("CNicknamesViewRight::EnableAllControls()\n");
	if (! m_bIsInitialized)
		return;		// don't bother with the controls if they haven't been initialized yet

	for (int i = 0; i < m_PropSheet.GetPageCount(); i++)
	{
		CNicknamesPropertyPage* p_page = (CNicknamesPropertyPage *) m_PropSheet.GetNicknamePage(i);
		ASSERT(p_page != NULL);
		ASSERT(p_page->IsKindOf(RUNTIME_CLASS(CNicknamesPropertyPage)));

		//
		// Call base class virtual method to forward the message to the
		// derived class implementations.
		//
		p_page->EnableAllControls(bEnable);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CNewAddressBookDlg dialog


CNewAddressBookDlg::CNewAddressBookDlg()
	: CHelpxDlg(CNewAddressBookDlg::IDD, AfxGetMainWnd())
{
	//{{AFX_DATA_INIT(CNewAddressBookDlg)
	m_AddressBookName = "";
	//}}AFX_DATA_INIT
}

BOOL CNewAddressBookDlg::OnInitDialog()
{
	return(CHelpxDlg::OnInitDialog());
}

void CNewAddressBookDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	
	//{{AFX_DATA_MAP(CNewAddressBookDlg)
	DDX_Text(pDX, IDC_NEWABOOK_NAME_EDIT, m_AddressBookName);
	DDV_MaxChars(pDX, m_AddressBookName, 31);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewAddressBookDlg, CHelpxDlg)
	//{{AFX_MSG_MAP(CNewAddressBookDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNewAddressBookDlg message handlers

void CNewAddressBookDlg::OnOK()
{
	// Get entered name into m_AddressBookName
	if (!UpdateData())
	{
		return;
	}
	
	// Empty name, don't exit
	if (m_AddressBookName.IsEmpty())
	{
		return;
	}

	// See if there are any illegal characters in the path name.
	if (m_AddressBookName.FindOneOf("/\\:*?\"<>|") >= 0)
	{
		ErrorDialog(IDS_ADDRESSBOOK_BAD_CHARACTER);
		return;
	}


	CString pathname(EudoraDir);
	pathname += CRString(IDS_NICK_DIR_NAME);
	pathname += SLASHSTR;
	pathname += m_AddressBookName;
	pathname += CRString(IDS_NICK_FILE_EXTENSION);

	struct _stat	st;
	
	//fixed bug where Eudora allowed to create another Addressbook - "Eudora Nicknames"
	//Problem : Eudora Nicknames address book is called NNdbase.txt and is present in
	//the main Eudora directory where earlier we were not searching. We now add another
	//check to prevent the user from ever creating an addressbook by the name
	//"Eudora Nicknames"
	if ((_stat(pathname, &st) != -1) || (! m_AddressBookName.Compare( CRString(IDS_MAIN_NICK_FILE_NAME) ) ))
	{
		ErrorDialog(IDS_ADDRESSBOOK_ALREADY_EXISTS);
		return;
	}

	CHelpxDlg::OnOK();
}


//--------------------------------------------------------------------//
//--------------------------------------------------------------------//
//                                                                    //
//              C N i c k n a m e s V i e w L e f t                   //
//                                                                    //
//                  (common to Win16 and Win32)                       //
//                                                                    //
//--------------------------------------------------------------------//
//--------------------------------------------------------------------//

/////////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft form view

IMPLEMENT_DYNAMIC(CNicknamesViewLeft, CFormView)

BEGIN_MESSAGE_MAP(CNicknamesViewLeft, CFormView)
	//{{AFX_MSG_MAP(CNicknamesViewLeft)
	ON_CBN_SELCHANGE(IDC_NICKNAME_LEFT_VIEWBY, OnViewBySelchange)
	ON_BN_CLICKED(IDC_NICKNAME_LEFT_REMOVE, OnDelClicked)
	ON_BN_CLICKED(IDC_NICKNAME_LEFT_NEWBOOK, OnNewBookClicked)
	ON_BN_CLICKED(IDC_NICKNAME_LEFT_NEW, OnNewClicked)
	ON_BN_CLICKED(IDC_NICKNAME_LEFT_TO, OnToClicked)
	ON_BN_CLICKED(IDC_NICKNAME_LEFT_CC, OnCcClicked)
	ON_BN_CLICKED(IDC_NICKNAME_LEFT_BCC, OnBccClicked)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_SPECIAL_MAKENICKNAME, OnMakeNickname)
	ON_UPDATE_COMMAND_UI(ID_SPECIAL_MAKENICKNAME, OnUpdateMakeNickname)
	ON_COMMAND(ID_DBLCLK_NICKNAME, OnCmdDblClkNickname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft [protected, constructor]
//
////////////////////////////////////////////////////////////////////////
CNicknamesViewLeft::CNicknamesViewLeft(UINT uTemplateID)
	: CFormView(uTemplateID)
{
	//{{AFX_DATA_INIT(CNicknamesViewLeft)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft [protected, virtual, destructor]
//
////////////////////////////////////////////////////////////////////////
CNicknamesViewLeft::~CNicknamesViewLeft()
{
}


////////////////////////////////////////////////////////////////////////
// GetDocument [public]
//
// The following very impolitely hides the public, NON-VIRTUAL
// CView::GetDocument() method....  this is very, very bad C++ style.
// :-) In Eudora 3.x or earlier, this was just a convenient way to
// type cast the CDocument pointer to a derived CNicknamesDoc pointer.
// In Eudora 4.x or later, this override actually makes sure that we
// create and load up the One True CNicknamesDoc(tm) referred to by
// 'g_Nicknames'.
////////////////////////////////////////////////////////////////////////
CNicknamesDoc* CNicknamesViewLeft::GetDocument()
{
	ASSERT(g_Nicknames);
	ASSERT_KINDOF(CNicknamesDoc, g_Nicknames);
	return g_Nicknames;
}


////////////////////////////////////////////////////////////////////////
// DoDataExchange [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::DoDataExchange(CDataExchange* pDX)
{
	//TRACE0("CNicknamesViewLeft::DoDataExchange()\n");
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNicknamesViewLeft)
	DDX_Control(pDX, IDC_NICKNAME_LEFT_REMOVE, m_DelButton);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_NEWBOOK, m_GrowButton);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_NEW, m_NewButton);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_TO, m_ToButton);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_CC, m_CcButton);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_BCC, m_BccButton);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_VIEWBY, m_ViewByCombo);
	DDX_Control(pDX, IDC_NICKNAME_LEFT_VIEWBY_LABEL, m_ViewByComboLabel);
	//}}AFX_DATA_MAP
}


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
// Clean up "item data" memory, if any, in LHS combo box.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnDestroy()
{
	//
	// Hide window so we don't get repaint slowness while closing 
	// Nickname windows with lots of nicknames.
	//
	QCWorksheet* p_frame_wnd = (QCWorksheet *) GetParentFrame();
	if (p_frame_wnd && p_frame_wnd->IsKindOf(RUNTIME_CLASS(QCWorksheet)))
	{
		p_frame_wnd->ShowWindow(SW_HIDE);
		AfxGetMainWnd()->UpdateWindow();
	}

	for (int idx = 0; idx < m_ViewByCombo.GetCount(); idx++)
	{
		char* p_fieldname = (char *) m_ViewByCombo.GetItemDataPtr(idx);

		if (p_fieldname != NULL)
			delete [] p_fieldname;
		else
		{
			ASSERT(0);
		}
	}

	// save the combo box selection index
	SetIniShort(IDS_INI_NICKNAME_VIEWBY_INDEX, short(m_ViewByCombo.GetCurSel()));

	CFormView::OnDestroy();
}


////////////////////////////////////////////////////////////////////////
// OnUpdate [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	//TRACE0("CNicknamesViewLeft::OnUpdate()\n");
	switch (lHint)
	{
	case NICKNAME_HINT_NOSELECT_RECIPLIST_DISABLE: // no selection
		m_ToButton.EnableWindow(FALSE);
		m_CcButton.EnableWindow(FALSE);
		m_BccButton.EnableWindow(FALSE);
		m_DelButton.EnableWindow(FALSE);
		m_NewButton.EnableWindow(TRUE);
		break;
	case NICKNAME_HINT_SINGLE_NICKFILE_SELECTED: // selection is a single nickname file
		m_ToButton.EnableWindow(FALSE);
		m_CcButton.EnableWindow(FALSE);
		m_BccButton.EnableWindow(FALSE);
		m_DelButton.EnableWindow(TRUE);
		m_NewButton.EnableWindow(TRUE);
		break;
	case NICKNAME_HINT_MULTI_NICKFILE_SELECTED: // multiple selections may contain nicknames and nickname files
		m_ToButton.EnableWindow(FALSE);
		m_CcButton.EnableWindow(FALSE);
		m_BccButton.EnableWindow(FALSE);
		m_DelButton.EnableWindow(TRUE);
		m_NewButton.EnableWindow(FALSE);
		break;
	case NICKNAME_HINT_MULTISELECT_RECIPLIST_CHECK:
	case NICKNAME_HINT_MULTISELECT_BPLIST_CHECK:
	case NICKNAME_HINT_MULTISELECT_RECIPLIST_BPLIST_CHECK:
	case NICKNAME_HINT_MULTISELECT_RECIPLIST_BPLIST_UNCHECK:
	case NICKNAME_HINT_DISPLAYNICKNAME:
	case NICKNAME_HINT_DISPLAYNICKNAME_READONLY:
		m_ToButton.EnableWindow(TRUE);
		m_CcButton.EnableWindow(TRUE);
		m_BccButton.EnableWindow(TRUE);
		m_DelButton.EnableWindow(TRUE);
		break;
	case NICKNAME_HINT_RECIPLIST_ADD:
	case NICKNAME_HINT_RECIPLIST_REMOVE:
		{
			//
			// Handles the case where the recipient list status was
			// changed *inside* the nicknames window (i.e., by the
			// RHS View checkbox or the LHS context menu).
			//
			// FORNOW, Hmm.  This might be a candidate for a virtual
			// function implementation in the base class
			// CNicknamesViewLeft::OnUpdate() rather than cracking
			// individual update messages and calling the derived
			// class methods directly.
			//
			CObList nickname_list;
			if (GetSelectedNicknames(nickname_list))
			{
				BOOL changed = FALSE;
				while (! nickname_list.IsEmpty())
				{
					CNickname* p_nickname = (CNickname *) nickname_list.RemoveHead();
					if (NICKNAME_HINT_RECIPLIST_ADD == lHint)
					{
						// if off, turn it on
						if (! p_nickname->IsRecipient())
						{
							p_nickname->AddRecipient();
							changed = TRUE;
						}
					}
					else
					{
						// if on, turn it off
						ASSERT(NICKNAME_HINT_RECIPLIST_REMOVE == lHint);
						if (p_nickname->IsRecipient())
						{
							p_nickname->RemoveRecipient();
							changed = TRUE;
						}
					}
				}

				if (changed)
				{
					// Rebuild recipient menus
					RefreshRecipListDisplay();
				}
			}
		}
		break;
	case NICKNAME_HINT_BP_ADD:
	case NICKNAME_HINT_BP_REMOVE:
		{
			//
			// Handles the case where the BP list status was
			// changed *inside* the nicknames window (i.e., by the
			// RHS View checkbox or the LHS context menu).
			//
			CObList nickname_list;
			BOOL changed = FALSE;
			if (GetSelectedNicknames(nickname_list))
			{
				while (! nickname_list.IsEmpty())
				{
					CNickname* p_nickname = (CNickname *) nickname_list.RemoveHead();
					if (NICKNAME_HINT_BP_ADD == lHint)
					{		
						//if off, turn it on
						CString bp;
						p_nickname->GetNotesFieldValue(CRString(IDS_NICK_FIELD_BOSS_PROTECTOR), bp);
						if( (strcmp(bp, CRString(IDS_NICK_FIELD_BOSS_PROTECTOR_NO)) == 0) || (strcmp(bp, "") == 0) ) {
							//p_nickname->SetNotesFieldValue( CRString(IDS_NICK_FIELD_BOSS_PROTECTOR), CRString(IDS_NICK_FIELD_BOSS_PROTECTOR_YES) );
							p_nickname->AddToBP();
							changed = TRUE;
						}

					}
					else
					{
						// if on, turn it off
						ASSERT(NICKNAME_HINT_BP_REMOVE == lHint);
						CString bp;
						p_nickname->GetNotesFieldValue(CRString(IDS_NICK_FIELD_BOSS_PROTECTOR), bp);
						if(strcmp(bp, CRString(IDS_NICK_FIELD_BOSS_PROTECTOR_YES)) == 0) {
							//p_nickname->SetNotesFieldValue( CRString(IDS_NICK_FIELD_BOSS_PROTECTOR), CRString(IDS_NICK_FIELD_BOSS_PROTECTOR_NO) );
							p_nickname->RemoveFromBP();
							changed = TRUE;
						}
					}
				}
			}
			CObList nickfile_list;
			if (GetSelectedNicknamefiles(nickfile_list))
			{
				while (! nickfile_list.IsEmpty())
				{
					CNicknameFile* p_nickfile = (CNicknameFile *) nickfile_list.RemoveHead();
					if (NICKNAME_HINT_BP_ADD == lHint)
					{		
						//if off, turn it on
						if( !p_nickfile->IsBPList() ) {
							p_nickfile->SetBPList();	
							g_Nicknames->AddABToBPList(p_nickfile);
							changed = TRUE;
						}

					}
					else
					{
						// if on, turn it off
						ASSERT(NICKNAME_HINT_BP_REMOVE == lHint);
						if( p_nickfile->IsBPList() ) {
							p_nickfile->SetBPList(FALSE);		
							g_Nicknames->RemoveABFromBPList(p_nickfile);
							changed = TRUE;
						}					
					}
				}
			}

			if (changed)
			{
				// Rebuild BP menus
				RefreshBPListDisplay();
			}
		}
		break;
	case NICKNAME_HINT_FETCH_SELECTED_NICKNAMES:
		{
			//
			// Handles the case where somebody (anybody) wants a list
			// of the selected CNickname items.
			//
			CObList* p_oblist = (CObList *) pHint;	// type cast
			ASSERT(p_oblist != NULL);
			ASSERT(p_oblist->IsKindOf(RUNTIME_CLASS(CObList)));
			GetSelectedNicknames(*p_oblist);
		}
		break;
	case NICKNAME_HINT_REGISTER_VIEWBY_ITEM:
		{
			//
			// Handles the case where the RHS wants to customize the
			// LHS View By dropdown list with a new item.
			//
			CStringList* p_strlist = (CStringList *) pHint;	// type cast
			ASSERT(p_strlist != NULL);
			ASSERT(p_strlist->IsKindOf(RUNTIME_CLASS(CStringList)));

			CString viewbyitem;
			CString fieldname;
			ASSERT(2 == p_strlist->GetCount());

			viewbyitem = p_strlist->GetAt(p_strlist->FindIndex(0));
			fieldname = p_strlist->GetAt(p_strlist->FindIndex(1));

			//
			// Add commercial-only fields to View By combo box.
			//
			int idx = m_ViewByCombo.AddString(viewbyitem);

			char* p_fieldname = DEBUG_NEW_NOTHROW char [fieldname.GetLength() + 1];
			if (p_fieldname != NULL)
			{
				strcpy(p_fieldname, fieldname);
				m_ViewByCombo.SetItemDataPtr(idx, p_fieldname);
			}
		}
		break;
	case NICKNAME_HINT_SENDING_NICKNAME:
	case NICKNAME_HINT_ACTIVATE_ADDRESS_PAGE:
	case NICKNAME_HINT_RHS_CLOSED:
	case NICKNAME_HINT_RHS_OPEN:
		// ignore these
		break;
	case NICKNAME_HINT_RECIPLIST_CHANGED:
	case NICKNAME_HINT_MARK_NICKFILE_DIRTY:
	case NICKNAME_HINT_REQUESTING_NICKNAME:
	case NICKNAME_HINT_NICKNAME_WAS_CHANGED:
	case NICKNAME_HINT_ADDED_NEW_NICKNAME:
	case NICKNAME_HINT_ADDED_NEW_NICKFILE:
	case NICKNAME_HINT_REFRESH_LHS:
		ASSERT(0);	// whoops, this is supposed to be handled by derived classes
		break;
	default:
		// Make sure we're not applying default handling to one of our hints
		ASSERT(lHint <= NICKNAME_HINT_FIRST || lHint >= NICKNAME_HINT_LAST);
		CFormView::OnUpdate(pSender, lHint, pHint);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnInitialUpdate()
{
	//TRACE0("CNicknamesViewLeft::OnInitialUpdate()\n");
	CFormView::OnInitialUpdate();

	//
	// Since we're creating the View manually, we need to register this
	// view with the Document manually.
	//
	{
		CNicknamesDoc* doc = GetDocument();
		ASSERT_KINDOF(CNicknamesDoc, doc);
		doc->AddView(this);
	}

	//
	// Set the item data pointers for the default combo box items
	// to internally-defined strings.  This is for consistency with any
	// custom-defined combo box items that get added as the custom
	// field names get registered with the nicknames database.
	//
	ASSERT(m_ViewByCombo.GetCount() == 2);	// "Nickname" and "Address(es)"
	char* p_fieldname = DEBUG_NEW_NOTHROW char[strlen(NICKNAME_FIELD_NICKNAME) + 1];
	if (p_fieldname != NULL)
		strcpy(p_fieldname, NICKNAME_FIELD_NICKNAME);
	m_ViewByCombo.SetItemDataPtr(0 , p_fieldname);
	p_fieldname = DEBUG_NEW_NOTHROW char[strlen(NICKNAME_FIELD_ADDRESSES) + 1];
	if (p_fieldname != NULL)
		strcpy(p_fieldname, NICKNAME_FIELD_ADDRESSES);
	m_ViewByCombo.SetItemDataPtr(1 , p_fieldname);

	//
	// Restore the "view by" value to the previous value saved in the
	// INI file.  This must be done via post message since the combo
	// box won't be fully populated until the document is loaded.  See
	// the code for NICKNAME_HINT_REGISTER_VIEWBY_ITEM.
	//
	m_ViewByCombo.PostMessage(CB_SETCURSEL, GetIniShort(IDS_INI_NICKNAME_VIEWBY_INDEX));

	m_DelButton.EnableWindow(TRUE);		// FORNOW, hardcoded (should fix dialog resource)

	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
    OnSize(SIZE_RESTORED, -1, -1);
}


////////////////////////////////////////////////////////////////////////
// OnNewClicked [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnNewClicked()
{
	//
	// Create a new, empty nickname in the selected file.
	//
	CString			 strNicknameName = _T("Untitled");
	CNickname		 nickname(strNicknameName);
	CNicknameFile	*pNicknameFile = FindSingleSelectedNicknameFile();
	
	if(!pNicknameFile) //if there is no selected nickname file then just use the "Eudora Nicknames" file
		pNicknameFile = FindRootNicknameFile();

	int				 idx = 0;
	int				 iUniqueId = 0;
	if (pNicknameFile)
	{
		char		str[16];
		// Make sure name is unique
		while (pNicknameFile->FindNickname(strNicknameName))
		{
			++iUniqueId;
			idx = strNicknameName.Find("-");
			if (idx == -1)
			{
				strNicknameName += "-1";
			}
			else
			{
				strNicknameName = strNicknameName.Left(idx + 1);
				strNicknameName += itoa(iUniqueId, str, 10);
			}
		}

		// Add nickname
		nickname.SetName(strNicknameName);
		CNickname* p_new_nickname = pNicknameFile->AddNickname(nickname, FALSE);
		if (p_new_nickname)
		{
			// Notify everyone that there were some changes
			//FORNOWg_Nicknames->SetModifiedFlag();
			//
			// Since this is a new nickname, set focus to the addresses
			// edit field to encourage the user to fill in the address(es)
			// for the new nickname.  We need to do this *after* dismissing
			// the dialog, otherwise Windows will set the focus back to the
			// button which launched the dialog.
			//
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ACTIVATE_ADDRESS_PAGE, NULL);
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnDelClicked [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnDelClicked()
{
	//
	// Call virtual method to handle the remove key click.
	//
	RemoveSelectedItems();
}


////////////////////////////////////////////////////////////////////////
// OnGrowClicked [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnNewBookClicked()
{
	// Shareware: Reduced feature mode only allows one nickname file, so no CChooseNicknameFileDlg
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		ASSERT(0);
		return;
	}

	CNewAddressBookDlg	dlg;
	if (dlg.DoModal() == IDOK)
	{
		// Create the nickname file
		CreateNewNicknameFile(dlg.m_AddressBookName + CRString(IDS_NICK_FILE_EXTENSION));

		//
		// Add the new nickname file to the collection of nickname files
		// maintained by CNicknamesDoc.  Then, refresh the nicknames view.
		// 
		//if (g_Nicknames)
	//	{
	//		CNicknameFile* p_nickfile = g_Nicknames->AddNicknameFile(dlg.m_AddressBookName);

	//		if (p_nickfile != NULL)
	//		{
				//
				// Update the view.
				//
	//			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKFILE, p_nickfile);
	//			return;
	//		}
	//}
	}
	return;
}


////////////////////////////////////////////////////////////////////////
// OnCmdDblClkNickname [protected]
//
// Command handler for the double-click command action for
// a nickname tree item.
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnCmdDblClkNickname()
{
	if (m_ToButton.IsWindowEnabled())
		OnInsertNickname(HEADER_CURRENT);
	else
		::MessageBeep(MB_OK);
}


////////////////////////////////////////////////////////////////////////
// AutoClickNewButton [public]
//
// Public method that fakes a click on the "New" button.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::AutoClickNewButton()
{
	if (m_NewButton.IsWindowEnabled())
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_NICKNAME_LEFT_NEW, BN_CLICKED), LPARAM(m_NewButton.GetSafeHwnd()));
	else
		::MessageBeep(MB_OK);
}


////////////////////////////////////////////////////////////////////////
// AutoClickRemoveButton [public]
//
// Public method that fakes a click on the "Del" button.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::AutoClickRemoveButton()
{
	if (m_DelButton.IsWindowEnabled())
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_NICKNAME_LEFT_REMOVE, BN_CLICKED), LPARAM(m_DelButton.GetSafeHwnd()));
	else
		::MessageBeep(MB_OK);
}

////////////////////////////////////////////////////////////////////////
// MoveOrCopyNicknames [public]
//
// High-level 16-bit and 32-bit driver for Move To and Copy To nickname
// management commands.
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::MoveOrCopyNicknames(BOOL bIsMove, CObList& nicknameList, CNicknameFile* pTargetNicknameFile /*=NULL*/)
{
	// Shareware: Reduced feature mode only allows one nickname file, no COPY/MOVE TO
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE mode
		ASSERT(0);
		return;
	}

	if (nicknameList.IsEmpty())
	{
		// no source nicknames to process (probably a nickname file was selected)
		::MessageBeep(MB_ICONASTERISK);
		return;
	}

	if (NULL == pTargetNicknameFile)
	{
		//
		// Display a dialog asking the user to choose a destination
		// nickname file.
		//
		CNicknameFile* p_single_selected_nickname_file = NULL;
		if (bIsMove)
			p_single_selected_nickname_file = FindSingleSelectedNicknameFile();		// can be NULL
		CChooseNicknameFileDlg dlg(p_single_selected_nickname_file);
		if (IDOK != dlg.DoModal())
			return;

		pTargetNicknameFile = dlg.GetTargetNicknameFile();
	}

	if (NULL == pTargetNicknameFile)
	{
		ASSERT(0);		// should never happen
		return;
	}
	ASSERT(pTargetNicknameFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

	//
	// Make sure the target nickname file is not read-only.
	//
	if (pTargetNicknameFile->m_ReadOnly)
	{
		ErrorDialog(IDS_ERR_NICKFILE_READONLY);
		return;
	}

	//
	// Check for the case where one or more of the source nickname files
	// is the same as the target nickname file and give the user a warning
	// that they may about to do something stupid.  Also, check for the
	// case of attempting to move something out of a read-only file.
	//
	{
		POSITION pos = nicknameList.GetHeadPosition();
		while (pos != NULL)
		{
			CObject* p_obj = nicknameList.GetNext(pos);
			ASSERT(p_obj);
			if (p_obj->IsKindOf(RUNTIME_CLASS(CNicknameFile)))
			{
				CNicknameFile* p_src_nickfile = (CNicknameFile *) p_obj;

				if (bIsMove && p_src_nickfile->m_ReadOnly)
				{
					//
					// Don't allow move if source file is read-only.
					//
					ErrorDialog(IDS_ERR_NICK_CANNOT_MOVE_READONLY);
					return;
				}
				else if (p_src_nickfile == pTargetNicknameFile)
				{
					if (bIsMove)
					{
						//
						// Don't allow move if source and destination
						// are identical.
						//
						ErrorDialog(IDS_ERR_NICK_BAD_MOVE);
						return;
					}
					else 
					{
						//
						// Copy case with identical source and destination
						// files.  Allow it if user really wants to.
						//
						if (AlertDialog(IDD_NICKNAME_COPY) != IDOK)
							return;
					}
					break;
				}
			}
		}
	}

	//
	// Now process one CNickname/CNicknameFile pair at a time:
	//	1. add/merge it into the target nickname file, then
	//	2. remove the original nickname if we're doing a move.
	//
	while (! nicknameList.IsEmpty())
	{
		CNickname* p_nickname = (CNickname *) nicknameList.RemoveHead();
		ASSERT(p_nickname && p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));

		if (nicknameList.IsEmpty())
		{
			ASSERT(0);
			break;
		}

		CNicknameFile* p_nickfile = (CNicknameFile *) nicknameList.RemoveHead();
		ASSERT(p_nickfile && p_nickfile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

		// Make sure original addresses are read in
		if (p_nickname->NeedsReadIn()) {
			JJFile in;
			p_nickfile->ReadNickname(p_nickname, in);
			in.Close();
		}

		//
		// Add the nickname to the target nickname file.
		//
		CNickname nickname(*p_nickname);		// copy constructor

		if (! bIsMove)
		{
			//
			// Check for the Copy Nickname case where the source file
			// and destination file are the same.  If we detect this,
			// then automagically prepend a "Copy of" string to the
			// nickname name, taking care to not exceed the maximum 
			// nickname name length.
			// 
			if (p_nickfile == pTargetNicknameFile)
			{
				CString name(nickname.GetName());
				name = CRString(IDS_NICK_COPY_PREFIX) + " " + name;
				name = name.Left(CNickname::MAX_NAME_LENGTH);
				nickname.SetName(name);
			}
		}

		CNickname* p_new_nickname = pTargetNicknameFile->AddNickname(nickname);

		if (p_new_nickname && bIsMove)
		{
			//
			// Successfully copied a nickname to target file, so in the
			// "move" case, delete the original nickname from the view
			// and the nicknames database.
			//
			p_nickname->SetBeingMoved(true);	// so delete won't kill the recip list
			DeleteExistingNickname(p_nickfile, p_nickname, nicknameList.IsEmpty());
		}
	}

}

////////////////////////////////////////////////////////////////////////
// OnToClicked [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnToClicked()
{
	OnInsertNickname(HEADER_TO);
}


////////////////////////////////////////////////////////////////////////
// OnCcClicked [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnCcClicked()
{
	OnInsertNickname(HEADER_CC);
}


////////////////////////////////////////////////////////////////////////
// OnBccClicked [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnBccClicked()
{
	OnInsertNickname(HEADER_BCC);
}


////////////////////////////////////////////////////////////////////////
// OnViewBySelchange [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnViewBySelchange()
{
	//
	// Extract the "item data" string associated with the selection
	// then pass that string to the pure virtual function to reset 
	// the LHS display mode.
	//
	char* p_fieldname = (char *) m_ViewByCombo.GetItemDataPtr(m_ViewByCombo.GetCurSel());

	if (-1 == int(p_fieldname))
	{
		ASSERT(0);
		return;
	}

	//
	// Call pure virtual.
	//
	SetViewByMode(m_ViewByCombo.GetCurSel(), p_fieldname);
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnSize(UINT nType, int cx, int cy)
{
	//TRACE3("CNicknamesViewLeft::OnSize(%d,%d,%d)\n", nType, cx, cy);
	CFormView::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! ::IsWindow(m_ViewByCombo.m_hWnd)) || (SIZE_MINIMIZED == nType))
		return;
	
	CRect rect;					// for overall CFormView client area
	CRect del_rect;				// for Del button
	CRect grow_rect;   			// for Grow button
	CRect new_rect;				// for New button
	CRect to_rect;				// for To: button
	CRect cc_rect;				// for Cc: button
	CRect bcc_rect;				// for Bcc: button
	CRect viewby_combo_rect;	// for View By combo box
	CRect viewby_label_rect;	// for View By combo box label

	//
	// Fetch overall client rect.  If the client area has zero width,
	// then the splitter window has collapsed our view.  We can
	// safely bail outta here.
	//
	GetClientRect(&rect);
	if (0 == rect.Width())
		return;

	//
	// If the client window dimensions fall below certain reasonable
	// minimums, then fool the proportional sizing code below to think
	// it has a larger window.  This causes the displayed controls to
	// be clipped from the window rather than causing them to shrink
	// unusably small.
	//
	ASSERT((0 == rect.left) && (0 == rect.top));
	if (rect.right < 200)
		rect.right = 200;
	if (rect.bottom < 150)
		rect.bottom = 150;

	// Fetch control client rects
	m_DelButton.GetWindowRect(&del_rect);					ScreenToClient(&del_rect);
	m_GrowButton.GetWindowRect(&grow_rect);					ScreenToClient(&grow_rect);
	m_NewButton.GetWindowRect(&new_rect);					ScreenToClient(&new_rect);
	m_ToButton.GetWindowRect(&to_rect);						ScreenToClient(&to_rect);
	m_CcButton.GetWindowRect(&cc_rect);						ScreenToClient(&cc_rect);
	m_BccButton.GetWindowRect(&bcc_rect);					ScreenToClient(&bcc_rect);
	m_ViewByCombo.GetWindowRect(&viewby_combo_rect);		ScreenToClient(&viewby_combo_rect);
	m_ViewByComboLabel.GetWindowRect(&viewby_label_rect);	ScreenToClient(&viewby_label_rect);

	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int X_SPACING = LOWORD(DBU) / 2;
	const int Y_SPACING = HIWORD(DBU) / 2;
	const int MARGIN = viewby_label_rect.left;
	
	//
	// Set View By combo box width
	//
	viewby_combo_rect.right = rect.right - MARGIN;

	if (! IsVersion4())
	{
		//
		// For Win 3.x, setting the vertical height affects the
		// dropdown list box, so we need to preserve the original
		// height everytime we size the combo box.
		//
		CRect dropped_rect;
		m_ViewByCombo.GetDroppedControlRect(&dropped_rect);
		viewby_combo_rect.bottom = viewby_combo_rect.top + dropped_rect.Height();
	}

	m_ViewByCombo.MoveWindow(&viewby_combo_rect, TRUE);

	//
	// First, position To, Cc, and Bcc buttons across the bottom row
	//
	int button_width = (rect.Width() - 2*MARGIN - 2*X_SPACING) / 3;
	int button_height = to_rect.Height();
	to_rect.top = cc_rect.top = bcc_rect.top = 
		rect.bottom - ( button_height ) - MARGIN;
	to_rect.bottom = cc_rect.bottom = bcc_rect.bottom = to_rect.top + button_height;
	to_rect.left = MARGIN;
	to_rect.right = to_rect.left + button_width;
	cc_rect.left = to_rect.right + X_SPACING;
	cc_rect.right = cc_rect.left + button_width;
	bcc_rect.left = cc_rect.right + X_SPACING;
	bcc_rect.right = bcc_rect.left + button_width;

	m_ToButton.MoveWindow(&to_rect, TRUE);
	m_CcButton.MoveWindow(&cc_rect, TRUE);
	m_BccButton.MoveWindow(&bcc_rect, TRUE);

	//
	// Then, position New, Del, and Grow buttons above them on the 
	// next-to-bottom row
	//
	button_width = (rect.Width() - 2*MARGIN - 2*X_SPACING) / 3;
	button_height = new_rect.Height();
	new_rect.top = del_rect.top = grow_rect.top = to_rect.top - button_height - Y_SPACING;
	new_rect.bottom = del_rect.bottom = grow_rect.bottom = new_rect.top + button_height;
	new_rect.left = MARGIN;
	new_rect.right = new_rect.left + button_width;
	del_rect.left = new_rect.right + X_SPACING;
	del_rect.right = del_rect.left + button_width;
	grow_rect.left = del_rect.right + X_SPACING;
	grow_rect.right = grow_rect.left + button_width;

	m_NewButton.MoveWindow(&new_rect, TRUE);
	m_DelButton.MoveWindow(&del_rect, TRUE);
	m_GrowButton.MoveWindow(&grow_rect, TRUE);
}


////////////////////////////////////////////////////////////////////////
// OnMakeNickname [protected]
//
// Handler for ID_SPECIAL_MAKENICKNAME command.
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnMakeNickname()
{
	::MakeNickname();
}


////////////////////////////////////////////////////////////////////////
// OnUpdateMakeNickname [protected]
//
// CCmdUI handler for ID_SPECIAL_MAKENICKNAME command.
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnUpdateMakeNickname(CCmdUI* pCmdUI)
{
	CObList selected_nickname_list;
	GetSelectedNicknames(selected_nickname_list);		// invoke virtual function
	pCmdUI->Enable(! selected_nickname_list.IsEmpty());
}


////////////////////////////////////////////////////////////////////////
// OnInsertNickname [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft::OnInsertNickname(int HeaderNum)
{
	CMainFrame* pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMainFrame, pMainFrame);
	CWazooWnd* pParentWazoo = NULL;
	{
		// hunt for parent wazoo window
		CWnd* pParentWnd = GetParent();
		while (pParentWnd != NULL)
		{
			if (pParentWnd->IsKindOf(RUNTIME_CLASS(CWazooWnd)))
			{
				pParentWazoo = (CWazooWnd *) pParentWnd;
				break;
			}
			pParentWnd = pParentWnd->GetParent();
		}
	}

	CCompMessageDoc* comp = pMainFrame->HuntForTopmostCompMessage(pParentWazoo);

	//
	// The moment of truth ...  did we find a target Comp message
	// window or not?
	//
	if (!comp)
	{
		//
		// No existing window, so the easy case is to create a new
		// comp window from scratch.
		//
		// IMPORTANT NOTE.  If you're wondering why we go to all the
		// trouble of pre-creating a new window with the To/Cc/Bcc field
		// filled in before the window is displayed, we need to do this
		// just so that the initial focus is properly set to the first 
		// non-blank header field in the new window.  In the past,
		// we create a blank window first, then use the generic header
		// population code below to fill in the appropriate To/Cc/Bcc
		// field.  This worked, but the keyboard focus was all screwed
		// up.  Hence, this kludge.
		//
		char* p_nicknames = AppendSelectedNicknames(NULL);		
		
		//
		// Note that 'p_nicknames' can be NULL if there is no selection,
		// or the selection expands to an empty nickname.  This is okay
		// since NewCompDocument() handles NULL parameters just fine.
		//

		switch (HeaderNum)
		{
		case HEADER_CURRENT:
		case HEADER_TO:
			comp = NewCompDocument(p_nicknames);		// to
			break;
		case HEADER_CC:
			comp = NewCompDocument(NULL, 				// to
									NULL, 				// from
									NULL,				// subject
									p_nicknames);		// cc
			break;
		case HEADER_BCC:
			comp = NewCompDocument(NULL, 				// to
									NULL, 				// from
									NULL,				// subject
									NULL,				// cc
									p_nicknames);		// bcc
			break;
		default:
			ASSERT(0);
			break;
		}

		//
		// Display the new comp window, cleanup heap-allocated
		// buffers, and we're outta here.
		//
		if (comp)
			comp->m_Sum->Display();
		if (p_nicknames)
			delete[] p_nicknames;
	}
	else
	{
		if (HEADER_CURRENT == HeaderNum)
		{
			CHeaderView *pView = comp->GetHeaderView();
			ASSERT(pView);
			
			if (pView)
			{
				const int nHeader = pView->GetCurrentHeader();

				switch (nHeader)
				{
					case HEADER_TO:
					case HEADER_CC:
					case HEADER_BCC:
					{
						HeaderNum = nHeader;
					}
					break;

					default:
					{
						HeaderNum = HEADER_TO;
					}
					break;
				}
			}
		}

		//
		// Have an existing comp window, so append the new addresses
		// to the existing addresses in desired header field.
		// 
		char* p_old_nicknames = ::SafeStrdupMT(comp->GetHeaderLine(HeaderNum));
		char* p_new_nicknames = AppendSelectedNicknames(p_old_nicknames);

		//
		// Display the comp message before we update it
		// so that MFC's rich edit control doesn't assert.
		//
		comp->m_Sum->Display();

		// If we got something, then set the header in the message
		if (p_new_nicknames != p_old_nicknames)
		{
			comp->SetHeaderLine(HeaderNum, p_new_nicknames);
			comp->SetModifiedFlag(TRUE);
			delete[] p_new_nicknames;
		}

		delete[] p_old_nicknames;
	}
}


////////////////////////////////////////////////////////////////////////
// AppendSelectedNicknames [protected]
//
// Given a heap-allocated string buffer to a nickname string (or NULL
// if there are no existing nicknames), return a pointer to a
// heap-allocated nickname string where the selected nicknames are
// appended to the existing nickname list.  Does nickname expansions
// if called for.
//
// WARNING!  In the case where the selected nicknames expand to an
// empty nickname list, it is possible for the original string pointer
// to be returned.  It is the caller's responsibility to delete the
// returned string as well as the caller-provided string, taking care
// not to delete the same buffer twice in the case where the incoming
// string is returned.
// 
////////////////////////////////////////////////////////////////////////
char* CNicknamesViewLeft::AppendSelectedNicknames(char* pExistingNicknames)		// can be NULL
{
	// We no longer have the expand checkbox, just use the INI setting
	BOOL bExpand = GetIniShort(IDS_INI_AUTO_EXPAND_NICKNAMES);

	if (pExistingNicknames && *pExistingNicknames)
	{
		// Find end of old header and remove ending comma because one
		// will be added later
		::TrimWhitespaceMT(pExistingNicknames);
		char* t = pExistingNicknames + strlen(pExistingNicknames);
		if (t-- > pExistingNicknames && *t == ',')
			*t = 0;
	}

	//
	// Call a pure virtual function to get the data from the
	// derived classes' implementation of the nickname list.
	//
	CObList nickname_list;
	if (! GetSelectedNicknames(nickname_list, TRUE))
		return pExistingNicknames;

	char* pUpdatedNicknames = pExistingNicknames;		// can be NULL

	while (! nickname_list.IsEmpty())
	{
		CNickname* p_nickname = (CNickname *) nickname_list.RemoveHead();
		ASSERT(p_nickname != NULL);
		ASSERT(p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));

		CNicknameFile* p_NNFile = (CNicknameFile *) nickname_list.RemoveHead();
		ASSERT(p_NNFile != NULL);
		ASSERT(p_NNFile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));

		const char* p_selected_nickname = p_nickname->GetName();
		const char* p_nickname_file = p_NNFile->GetName();

		JJFile in;
		CNickname* comparedNickname = g_Nicknames->ReadNickname(p_selected_nickname, in);
		in.Close();

		if (comparedNickname == p_nickname)		// No need to specify the file because there is no ambiguity as far as Eudora can tell
		{
			if (bExpand)
				p_selected_nickname = ExpandAliases(p_selected_nickname);

			if (p_selected_nickname && *p_selected_nickname)
			{
				//
				// Append the new nickname to the existing 'pUpdatedNicknames'
				// by allocating a new buffer that is big to hold the 
				// concatenated string.
				//
				char* p_add = NULL;
				if (pUpdatedNicknames)
					p_add = DEBUG_NEW_NOTHROW char[::SafeStrlenMT(pUpdatedNicknames) + strlen(p_selected_nickname) + 3];
				else
					p_add = DEBUG_NEW_NOTHROW char[strlen(p_selected_nickname) + 3];

				if (p_add)
				{
					if (!pUpdatedNicknames)
						*p_add = 0;
					else
					{
						char* nl = pUpdatedNicknames;
						strcpy(p_add, nl);
						while (isspace((int)(unsigned char)*nl))
							nl++;
						if (*nl)
							strcat(p_add, ", ");
					}
					strcat(p_add, p_selected_nickname);
					if (pUpdatedNicknames != pExistingNicknames)
						delete pUpdatedNicknames;
					pUpdatedNicknames = p_add;
				}
			}

			if (bExpand)
				delete [] (char *) p_selected_nickname;
		}
		else
		{
			char *pModifiedNN;
			char *pCompositeNN;
			pCompositeNN = DEBUG_NEW char[strlen(p_selected_nickname) + strlen(p_nickname_file) + 3];

			strcpy(pCompositeNN, p_nickname_file);
			strcat(pCompositeNN, "::");
			strcat(pCompositeNN, p_selected_nickname);

			if (bExpand)
				pModifiedNN = ExpandAliases(pCompositeNN);
			else
				pModifiedNN = pCompositeNN;

			if (pModifiedNN && *pModifiedNN)
			{
				//
				// Append the new nickname to the existing 'pUpdatedNicknames'
				// by allocating a new buffer that is big to hold the 
				// concatenated string.
				//
				char* p_add = NULL;
				if (pUpdatedNicknames)
					p_add = DEBUG_NEW_NOTHROW char[::SafeStrlenMT(pUpdatedNicknames) + strlen(pModifiedNN) + 3];
				else
					p_add = DEBUG_NEW_NOTHROW char[strlen(pModifiedNN) + 3];

				if (p_add)
				{
					if (!pUpdatedNicknames)
						*p_add = 0;
					else
					{
						char* nl = pUpdatedNicknames;
						strcpy(p_add, nl);
						while (isspace((int)(unsigned char)*nl))
							nl++;
						if (*nl)
							strcat(p_add, ", ");
					}
					strcat(p_add, pModifiedNN);
					if (pUpdatedNicknames != pExistingNicknames)
						delete pUpdatedNicknames;
					pUpdatedNicknames = p_add;
				}
			}

			if (bExpand)
				delete [] pModifiedNN;
			delete [] pCompositeNN;
		}
	}

	return pUpdatedNicknames;
}

//--------------------------------------------------------------------//
//--------------------------------------------------------------------//
//                                                                    //
//            C N i c k n a m e s V i e w L e f t 3 2                 //
//                                                                    //
//                          (Win32 only)                              //
//                                                                    //
//--------------------------------------------------------------------//
//--------------------------------------------------------------------//

/////////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft32 form view

IMPLEMENT_DYNCREATE(CNicknamesViewLeft32, CNicknamesViewLeft)

BEGIN_MESSAGE_MAP(CNicknamesViewLeft32, CNicknamesViewLeft)
	//{{AFX_MSG_MAP(CNicknamesViewLeft32)
	ON_WM_SIZE()
	ON_COMMAND( IDOK, OnEnterKeyPressed )
	ON_COMMAND( IDCANCEL, OnEscapeKeyPressed )
	ON_REGISTERED_MESSAGE(umsgActivateWazoo, OnActivateWazoo)
	ON_COMMAND( ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND( ID_FILE_PRINT_DIRECT, OnFilePrintOne)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnFilePrintUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnFilePrintPreviewUpdateUI)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnFileSaveAsUpdateUI)
	ON_UPDATE_COMMAND_UI( ID_FILE_PRINT_DIRECT, OnFilePrintOneUpdateUI)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft32 [protected, constructor]
//
////////////////////////////////////////////////////////////////////////
CNicknamesViewLeft32::CNicknamesViewLeft32()
	: CNicknamesViewLeft(CNicknamesViewLeft32::IDD),
	  m_bTreeCtrlInitialized(FALSE)
{
	//{{AFX_DATA_INIT(CNicknamesViewLeft32)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

////////////////////////////////////////////////////////////////////////
// CNicknamesViewLeft32 [protected, virtual, destructor]
//
////////////////////////////////////////////////////////////////////////
CNicknamesViewLeft32::~CNicknamesViewLeft32()
{
}

////////////////////////////////////////////////////////////////////////
// DoDataExchange [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::DoDataExchange(CDataExchange* pDX)
{
	CNicknamesViewLeft::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNicknamesViewLeft32)
	DDX_Control(pDX, IDC_NICKNAME_LEFT_LIST, m_NicknamesTree);
	//}}AFX_DATA_MAP
}


////////////////////////////////////////////////////////////////////////
// OnUpdate [protected, virtual]
//
// The LHS of the address book is subject to Just In Time display
// initialization, so skip any UI processing that requires the 
// tree control to be initialized.
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	switch (lHint)
	{
	case NICKNAME_HINT_RECIPLIST_CHANGED:
		//
		// Handles the case where the recipient list status was
		// changed *outside* of the nicknames window (i.e., in the
		// Eudora main menu).
		//
		if (m_bTreeCtrlInitialized)
		{
			m_NicknamesTree.RefreshRecipListDisplay();
			m_NicknamesTree.EvaluateSelection();
		}
		break;
	case NICKNAME_HINT_ADDED_NEW_NICKNAME:
		if (m_bTreeCtrlInitialized)
		{
			//
			// Handles the case where a new nickname was added
			// to the nickname database *outside* of the nicknames
			// window (i.e., from the "New Nickname" dialog
			// box), so we need to update our display.
			//
			CNickname* p_nickname = (CNickname *) pHint;	// type cast
			ASSERT(p_nickname != NULL);
			ASSERT(p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));

			//
			// As a clever hack, the caller is supposed to temporarily
			// stuff the parent CNicknameFile pointer into the "address offset"
			// member of the CNickname object.  Therefore, we extract
			// the pointer from that member here.  This works since
			// we know that this is a newly-created, and therefore empty,
			// CNickname object.  The caller is responsible for unmangling
			// the address offset value after we return from here.
			//
			ASSERT(p_nickname->GetAddressesOffset() != -1L);	// hack (see NAMENICK.CPP)
			CNicknameFile* p_nickfile = (CNicknameFile *) p_nickname->GetAddressesOffset();
			ASSERT(p_nickfile && p_nickfile->IsKindOf(RUNTIME_CLASS(CNicknameFile)));
			m_NicknamesTree.AddNewNickname(p_nickfile, p_nickname);
		}
		break;
	case NICKNAME_HINT_ADDED_NEW_NICKFILE:
		if (m_bTreeCtrlInitialized)
		{
			//
			// Handles the case where a new nickname file was added
			// to the nickname database *outside* of the nicknames
			// window (i.e., from the "New Nickname" dialog
			// box), so we need to update our display.
			//
			CNicknameFile* p_nickfile = DYNAMIC_DOWNCAST(CNicknameFile, pHint);
			if (p_nickfile != NULL)
				m_NicknamesTree.AddFile(p_nickfile);
		}
		break;
	case NICKNAME_HINT_MARK_NICKFILE_DIRTY:
		//
		// This notification tells us that an edit field for the
		// currently selected nickname has just changed.  Therefore,
		// we need to determine which file contains the selected
		// nickname so that we can mark it as "dirty".
		//
		if (m_bTreeCtrlInitialized)
			m_NicknamesTree.MarkSelectedNicknameDirty();
		break;
	case NICKNAME_HINT_REQUESTING_NICKNAME:
		//
		// The RHS is requesting the current, singly-selected
		// nickname.
		//
		if (m_bTreeCtrlInitialized)
			m_NicknamesTree.BroadcastSelectedNickname();
		break;
	case NICKNAME_HINT_NICKNAME_WAS_CHANGED:
		if (m_bTreeCtrlInitialized)
		{
			//
			// The RHS is notifying us that a nickname was
			// changed, so we may need to update our display.
			//
			CNickname* p_nickname = (CNickname *) pHint;	// type cast
			ASSERT(p_nickname != NULL);
			ASSERT(p_nickname->IsKindOf(RUNTIME_CLASS(CNickname)));
			m_NicknamesTree.NicknameWasChanged(p_nickname);
		}
		break;
	case NICKNAME_HINT_REFRESH_LHS:
		//
		// Completely refresh the LHS tree control contents.
		//
		if (m_bTreeCtrlInitialized)
		{
			m_NicknamesTree.Reset();
			DoRefresh();
		}
		break;
	default:
		CNicknamesViewLeft::OnUpdate(pSender, lHint, pHint);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected]
//
// Call the base class to do generic initializations, then do the
// 32-bit specific initializations here.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::OnInitialUpdate()
{
	if (m_NicknamesTree.GetSafeHwnd())
	{
		// Guard against bogus double initializations.
		ASSERT(0);
		return;
	}

	CNicknamesViewLeft::OnInitialUpdate();

	m_NicknamesTree.Init();

	//
	// Force an OnSize() to force the tree control to be resized properly.
	// For whatever reason, MFC doesn't call OnInitialUpdate() until 
	// all the normal WM_SIZE messages get handled, so the tree control
	// never gets a chance to set its initial size since DoDataExchange()
	// has not run until we get here.
	//
	OnSize(SIZE_RESTORED, -1 , -1);
	
	//FORNOWm_NicknamesTree.SetFocus();
//FORNOW	PostMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, 0), LPARAM(m_NicknamesTree.m_hWnd));

	//
	// As a hack, trick the parent Wazoo window into properly saving
	// the initial control focus for this form.
	//
	CWazooWnd::SaveInitialChildFocus(&m_NicknamesTree);
}

////////////////////////////////////////////////////////////////////////
// NicknameChanged [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::NicknameChanged(CNickname *pNickname)
{
	m_NicknamesTree.NicknameWasChanged(pNickname);
}

////////////////////////////////////////////////////////////////////////
// check for dumplicate nicknames and pop up dialog if duplicate is found
//
////////////////////////////////////////////////////////////////////////
bool CNicknamesViewLeft32::CheckDuplicateNickname(CNickname* pNickname)
{
	if(!pNickname)
		return false;

	if(m_NicknamesTree.CheckForDuplicateNicknameInTree()) {
		ErrorDialog(IDS_ERR_NICK_NAME_USED);
		return true;
	}

	return false;

}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
long CNicknamesViewLeft32::OnActivateWazoo(WPARAM, LPARAM)
{
	if (m_NicknamesTree.GetSafeHwnd() && !m_bTreeCtrlInitialized)
	{
		//
		// Populate the tree control.
		//
		DoRefresh();
		m_bTreeCtrlInitialized = TRUE;

		//
		// Now that the tree control has been populated, force an
		// evaluation of the selection change that should've occured
		// when we initialized the static part of the LHS UI -- see
		// CNicknamesViewLeft::OnInitialUpdate().
		//
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_NICKNAME_LEFT_VIEWBY, CBN_SELCHANGE), LPARAM(m_ViewByCombo.GetSafeHwnd()));
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////
// DoRefresh [protected]
//
// Refresh the tree control with nickname files.
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::DoRefresh()
{
	CCursor wait_cursor;

	// Add the nicknames from all the files and select the first nickname
	CNicknamesDoc* p_doc = GetDocument();
	ASSERT(p_doc != NULL);

	ASSERT(m_NicknamesTree.GetCount() == 0);
	for (CNicknameFile* p_nickfile = p_doc->GetFirstNicknameFile();
	     p_nickfile != NULL;
		 p_nickfile = p_doc->GetNextNicknameFile())
	{
		if(!p_nickfile->m_Invisible)
		{
			if (! m_NicknamesTree.AddFile(p_nickfile))
				break;
		}
	}

	m_NicknamesTree.RestoreVisualState();
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::OnSize(UINT nType, int cx, int cy)
{
	//TRACE3("CNicknamesViewLeft32::OnSize(%d,%d,%d)\n", nType, cx, cy);
	CNicknamesViewLeft::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! ::IsWindow(m_NicknamesTree.m_hWnd)) || (SIZE_MINIMIZED == nType))
		return;
	
	CRect tree_rect;			// for Nicknames tree control
	CRect new_rect;				// for New button
	CRect viewby_combo_rect;	// for View By combo box

	// Fetch all client rects
	m_NicknamesTree.GetWindowRect(&tree_rect);				ScreenToClient(&tree_rect);
	m_NewButton.GetWindowRect(&new_rect);					ScreenToClient(&new_rect);
	m_ViewByCombo.GetWindowRect(&viewby_combo_rect);		ScreenToClient(&viewby_combo_rect);

	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int Y_SPACING = HIWORD(DBU) / 2;
	
	//
	// Set tree control position.
	//
	tree_rect.right = viewby_combo_rect.right;
	tree_rect.bottom = new_rect.top - Y_SPACING;

	m_NicknamesTree.MoveWindow(&tree_rect, TRUE);
}


////////////////////////////////////////////////////////////////////////
// GetSelectedNicknames [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesViewLeft32::GetSelectedNicknames(CObList& nicknameList, BOOL bGetNicknameFiles /*=FALSE*/)
{
	if (!m_bTreeCtrlInitialized)
	{
		// Window has never been fully opened.  Ergo, it
		// is pretty unlikely to have a selection in it.
		// Ergo, we will just return FALSE and an empty
		// list, instead of ASSERTing like this was some
		// sort of unexpected and unrecoverable case.  SD 12/11/99
		return FALSE;
	}
	return m_NicknamesTree.GetSelectedNicknames(nicknameList, bGetNicknameFiles);
}


////////////////////////////////////////////////////////////////////////
// GetSelectedNicknames [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesViewLeft32::GetSelectedNicknamefiles(CObList& nickfileList)
{
	if (!m_bTreeCtrlInitialized)
	{
		// Window has never been fully opened.  Ergo, it
		// is pretty unlikely to have a selection in it.
		// Ergo, we will just return FALSE and an empty
		// list, instead of ASSERTing like this was some
		// sort of unexpected and unrecoverable case.  SD 12/11/99
		return FALSE;
	}
	return m_NicknamesTree.GetSelectedNicknamefiles(nickfileList);
}

////////////////////////////////////////////////////////////////////////
// FindSingleSelectedNicknameFile [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesViewLeft32::FindSingleSelectedNicknameFile()
{
	ASSERT(m_bTreeCtrlInitialized);
	return m_NicknamesTree.FindSingleSelectedNicknameFile();
}


////////////////////////////////////////////////////////////////////////
// FindRootNicknameFile [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
CNicknameFile* CNicknamesViewLeft32::FindRootNicknameFile()
{
	ASSERT(m_bTreeCtrlInitialized);
	return m_NicknamesTree.FindRootNicknameFile();
}



////////////////////////////////////////////////////////////////////////
// RefreshRecipListDisplay [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::RefreshRecipListDisplay()
{
	ASSERT(m_bTreeCtrlInitialized);
	m_NicknamesTree.RefreshRecipListDisplay();
}



////////////////////////////////////////////////////////////////////////
// RefreshBPListDisplay [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::RefreshBPListDisplay()
{
	ASSERT(m_bTreeCtrlInitialized);
	m_NicknamesTree.RefreshBPListDisplay();
}

////////////////////////////////////////////////////////////////////////
// RemoveSelectedItems [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::RemoveSelectedItems()
{
	ASSERT(m_bTreeCtrlInitialized);
	m_NicknamesTree.RemoveSelectedItems();
}


////////////////////////////////////////////////////////////////////////
// SetViewByMode [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::SetViewByMode(int nIndex, const char* pFieldName)
{
	ASSERT(pFieldName != NULL);
	ASSERT(m_bTreeCtrlInitialized);
	m_NicknamesTree.SetViewByMode(nIndex, pFieldName);
}


////////////////////////////////////////////////////////////////////////
// DeleteExistingNickname [protected, virtual]
//
// Implementation of a pure virtual from the base class that just
// passes the buck to the tree control.
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesViewLeft32::DeleteExistingNickname(CNicknameFile* pNicknameFile, CNickname* pNickname, BOOL bLastOne /*=TRUE*/)
{
	ASSERT(pNicknameFile);
	ASSERT(pNickname);
	ASSERT(m_bTreeCtrlInitialized);
	return m_NicknamesTree.DeleteExistingNickname(pNicknameFile, pNickname, bLastOne);
}


////////////////////////////////////////////////////////////////////////
// OnEnterKeyPressed [protected, virtual]
//
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::OnEnterKeyPressed()
{
	ASSERT(m_bTreeCtrlInitialized);
	if ( m_NicknamesTree.EditingName() )
	{
		m_NicknamesTree.SendMessage(TVM_ENDEDITLABELNOW, FALSE, 0);
	}
	else
	{
		if ( m_ToButton.IsWindowEnabled() )
		{
			// fake the default button behavior
			OnToClicked();
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnEscapeKeyPressed [protected, virtual]
//
//
////////////////////////////////////////////////////////////////////////
void CNicknamesViewLeft32::OnEscapeKeyPressed()
{
	ASSERT(m_bTreeCtrlInitialized);
	m_NicknamesTree.OnEscKeyPressed();
}

void CNicknamesViewLeft32::OnFilePrint()
{    
	
    CTextFileView* View = PreparePrint();

    ASSERT(View);

    if (View) {
	View->SendMessage(WM_COMMAND, ID_FILE_PRINT);
  // 	View->SendMessage(WM_CLOSE);
 }

}
void CNicknamesViewLeft32::OnFilePrintOne()
{    
	
    CTextFileView* View = PreparePrint();

    ASSERT(View);

    if (View) 
		{
		View->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
		}
	CDocument *pDoc = View->GetDocument();
	pDoc->OnCloseDocument();

}
void CNicknamesViewLeft32::OnFilePrintPreview()
{
	CTextFileView* View = PreparePrint();
    ASSERT(View);
    if (View) 
		{
		View->SetABOrigView(this);
		View->SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
		}

}
void CNicknamesViewLeft32::OnFilePrintUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());

	pCmdUI->Enable(pWndSplitter && pWndSplitter->IsWindowVisible());
}
void CNicknamesViewLeft32::OnFilePrintPreviewUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());

	pCmdUI->Enable(pWndSplitter && pWndSplitter->IsWindowVisible());
}

int CNicknamesViewLeft32::DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect)
{
	int iRes = m_NicknamesTree.DoFindNext(szSearch, bMatchCase, bWholeWord, bSelect);

	return iRes;

}
void CNicknamesViewRight::OnFilePrint()
{    
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFilePrint();
	}

}
void CNicknamesViewRight::OnFilePrintOne()
{    
	
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFilePrintOne();
	}

}
void CNicknamesViewRight::OnFilePrintPreview()
{
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFilePrintPreview();
	}

}
void CNicknamesViewRight::OnFilePrintUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFilePrintUpdateUI(pCmdUI);
	}
}

void CNicknamesViewRight::OnFilePrintPreviewUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFilePrintPreviewUpdateUI(pCmdUI);
	}
}

CTextFileView* CNicknamesViewLeft32::PreparePrint()
{
    CString csFileName = CreateHTMLFromDoc();
	CFile htmlFile(csFileName, CFile::modeRead);
	CArchive ar(&htmlFile, CArchive::load);
    CMDIChild::m_nCmdShow = SW_HIDE;
	CMultiDocTemplate *pTemplate = TextFileTemplate;
	CTextFileDoc *pDoc = (CTextFileDoc*)NewChildDocument(pTemplate);
	ASSERT(pDoc);
	pDoc->SetTitle( CRString(IDR_NICKNAMES) );
	CFrameWnd *pFrameWnd = NewChildFrame(pTemplate, pDoc);
	CTextFileView *pView = (CTextFileView*)pFrameWnd->GetActiveView();
	pView->AllowTables(true);	
	pDoc->Serialize(ar);
	ASSERT(pFrameWnd);
	ar.Close();	
	htmlFile.Close();
	::FileRemoveMT(csFileName);
	CMDIChild::m_nCmdShow = -1;
    return pView;
}

CString CNicknamesViewLeft32::CreateHTMLFromDoc()
{
	CNicknamesDoc* doc = GetDocument();
	CString csHTMLFileName;
	char buf[200];
	::GetTempPath(200, buf);
	::GetTempFileName(buf, "Eu",0,buf);
	csHTMLFileName = buf;
	csHTMLFileName += ".htm";
	doc->WriteToHTML(csHTMLFileName);
	return csHTMLFileName;

}

void CNicknamesViewLeft32::OnFileSaveAs()
{

	CFileDialog theDlg(	FALSE, CRString(IDS_HTM_EXTENSION), NULL, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						CRString(IDS_HTML_CSV_FILE_FILTER), NULL );

	if ( theDlg.DoModal() == IDOK )
	{
		CNicknamesDoc* doc = GetDocument();
		if(doc)
			{
			if(theDlg.GetFileExt() == CRString(IDS_CSV_EXTENSION))
				doc->WriteToCSV(theDlg.GetPathName());
			else
				doc->WriteToHTML(theDlg.GetPathName());
			}
	}

}

void CNicknamesViewRight::OnFileSaveAs()
{

	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFileSaveAs();
	}

}


void CNicknamesViewLeft32::OnFileSaveAsUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());

	pCmdUI->Enable(pWndSplitter && pWndSplitter->IsWindowVisible());
}

void CNicknamesViewRight::OnFileSaveAsUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFileSaveAsUpdateUI(pCmdUI);
	}
}
void CNicknamesViewLeft32::OnFilePrintOneUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd			*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());

	pCmdUI->Enable(pWndSplitter && pWndSplitter->IsWindowVisible());
}

void CNicknamesViewRight::OnFilePrintOneUpdateUI(CCmdUI* pCmdUI)
{
	CSplitterHelpWnd	*pWndSplitter = reinterpret_cast<CSplitterHelpWnd *>(GetParent());
	if( pWndSplitter)
	{
		CNicknamesViewLeft32	*pViewLeft = reinterpret_cast<CNicknamesViewLeft32 *>(pWndSplitter->GetPane(0, 0));
		if(pViewLeft)
			pViewLeft->OnFilePrintOneUpdateUI(pCmdUI);
	}
}



