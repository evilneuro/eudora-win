/*////////////////////////////////////////////////////////////////////////////

NAME:
	CNameToolBar - 

FILE:		NameToolBar.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	Derived from CToolBar. Must call Init. Uses IDD_NAME toolbar resource

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/05/97   lss     -Initial
05/08/97   lss     -When shown, edit box will have focus and name selected
05/19/97   lss     -CLsEdit

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#include "QVoice.h"
#include "MainFrm.h"
#include "ErrorHandler.h"

// LOCAL INCLUDES
#include "NameToolBar.h"

// LOCAL DEFINES
#define NAMEBAR_WIDTH	240
#define NAMEBAR_HEIGHT	32

#define WRITE_PROFILE_EDITNAME(bSet)	\
					AfxGetApp()->WriteProfileInt( "Settings", "EditNameBar", bSet )
#define GET_PROFILE_EDITNAME()			\
					AfxGetApp()->GetProfileInt( "Settings", "EditNameBar", 0 )

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CNameToolBar::CNameToolBar() : mb_Show(FALSE), mb_firstTime(FALSE),
	m_Height(NAMEBAR_HEIGHT)
{
}

CNameToolBar::~CNameToolBar() 
{
	WRITE_PROFILE_EDITNAME( mb_Show ? 1 : 0 );
}

BOOL CNameToolBar::Init(CWnd* pParentWnd) 
{
	BOOL ret = CDialogBar::Create( pParentWnd, IDD_NAME, CBRS_TOP|CBRS_TOOLTIPS, IDD_NAME );
	mp_MainFrame = GetParentFrame();

	if ( ret )
	 {
		HWND hWndCtrl = ::GetDlgItem( GetSafeHwnd(), IDC_EDIT_NAME);
		if ( !m_edtName.SubclassWindow(hWndCtrl) ) return FALSE;

		m_edtName.SetFont(GetFont());
		m_edtName.SetLimitText(256);
		// get state from previous session
		if ( 0 == GET_PROFILE_EDITNAME() )
		 {
			mb_Show = FALSE;
			ShowWindow( SW_HIDE ); 
		 }
		else
		 {
			mb_Show = TRUE;
			setup();
		 }
	 }
	mb_firstTime = TRUE;
	return ret;

/*
	if (!Create(pParentWnd) ||
		!LoadToolBar(IDR_NAME))
	 {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	 }
	
	mp_MainFrame = (CMainFrame*)pParentWnd;

	CSize size = CalcFixedLayout( FALSE, TRUE );
    m_Height = size.cy;
    
	// Turn the dummy button into a separator, of the width that we wish to
	// make our edit box.
	SetButtonInfo(2, IDC_EDIT_NAME, TBBS_SEPARATOR, WIDTH_EDITBOX);

	// Get the dimensions of our new toolbar real estate
	CRect rect;
	GetItemRect(2, rect);

	// Create the edit on the toolbar
	if (!m_edtName.Create( WS_BORDER | WS_VISIBLE | WS_TABSTOP |
			 CBS_AUTOHSCROLL | CBS_SIMPLE, rect, this, IDC_EDIT_NAME))
	{
		TRACE0("Failed to create Name Toolbar\n");
		return FALSE;
	}
	// Set the font for the Combo to the same as the dialog default
	// This is usually "MS Sans Serif" 8pt
	m_edtName.SetFont(GetFont());
//	if ( 0 == GET_PROFILE_EDITNAME() )
//	 {
		mb_Show = FALSE;
		ShowWindow( SW_HIDE ); 
//	 }
//	else
//		mb_Show = TRUE;

	mb_firstTime = TRUE;
    return TRUE;
*/
}

LPCTSTR CNameToolBar::Name()
{
	char buf[256];
	m_edtName.GetWindowText( buf, 256 );
	m_name = buf;
	return LPCTSTR(m_name);
}

LPCTSTR CNameToolBar::Name( LPCTSTR name )
{
	CString tmp = m_name;
	m_edtName.SetWindowText( name );
	m_name = name;
	return LPCTSTR(tmp);
}

void CNameToolBar::Show( BOOL bShow ) //=TRUE
{
	int state = SW_SHOWNA;
	if ( !bShow ) state = SW_HIDE;
	BOOL prev = ShowWindow( state );
	if ( state == prev ) return;
	mb_Show = bShow;
	if ( bShow && (prev == SW_HIDE) )
	 {
//		CSize size = CalcFixedLayout( FALSE, TRUE );
		CRect frmRect;
		mp_MainFrame->GetWindowRect( LPRECT(frmRect) );
		mp_MainFrame->SetWindowPos( NULL, 0, 0,
					frmRect.Width(), frmRect.Height()+m_Height,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );
		// have focus and select the name
		m_edtName.SetFocus();
		m_edtName.SetSel( 0, -1 );
	 }
	else if ( !bShow )
	 {
//		CSize size = CalcFixedLayout( FALSE, TRUE );
		CRect frmRect;
		mp_MainFrame->GetWindowRect( LPRECT(frmRect) );
		mp_MainFrame->SetWindowPos( NULL, 0, 0,
					frmRect.Width(), frmRect.Height()-m_Height,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );
	 }
	mp_MainFrame->RecalcLayout();
}

CSize CNameToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size = CDialogBar::CalcFixedLayout(bStretch, bHorz);
	if (bHorz) size.cy = m_Height;
	return size;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void CNameToolBar::setup()
{
	// resize our main frame window so we can fit in
/*		CRect frmRect;
	mp_MainFrame->GetWindowRect( LPRECT(frmRect) );
	mp_MainFrame->SetWindowPos( NULL, 0, 0, 
					frmRect.Width(), frmRect.Height()+m_Height,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );
*/
	// align and size edit box
	CRect edtRect;
	CPoint topleft( 6, 5 );
	int width = NAMEBAR_WIDTH - topleft.x*2 + 2;
	int height = m_Height - topleft.y - 3;

	m_edtName.GetWindowRect( LPRECT(edtRect) );

	edtRect.SetRect( topleft.x, topleft.y, 
					topleft.x + width, topleft.y + height );
	m_edtName.MoveWindow( LPCRECT(edtRect) );
}

//////////////////////////////////////////////////////////////////////////////
// CNameToolBar message handlers
//////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CNameToolBar, CDialogBar)
	//{{AFX_MSG_MAP(CNameToolBar)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNameToolBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialogBar::OnShowWindow(bShow, nStatus);
	if ( mb_firstTime && bShow && !nStatus )
	 {
		mb_firstTime = FALSE;
		setup();
	 }
}

//////////////////////////////////////////////////////////////////////////////
// CLsEdit Implementation
//////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CLsEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

void CLsEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	char buf[256];
	GetWindowText( buf, 256 );
	CString strName = buf;

	// check that we don't have path stuff
	if ( (strName.Find(TCHAR('\\')) > -1) || (strName.Find(TCHAR('/')) > -1) ||
		(strName.Find(TCHAR(':')) > -1) || (strName.Find(TCHAR('"')) > -1))
	{
		CErrorHandler err;
		err.ReportError( LS_ERROR_INVALID_NAME );
		Undo();
//		SetWindowText( m_oldName )
		SetFocus();
		SetSel( 0, -1 );
	}
//	else
//		m_oldName = strName;
}

void CLsEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// if return or tab, then set focus on our main wnd
	if ( (13 == nChar) || (9 == nChar) )
	 {
		AfxGetMainWnd()->SetFocus();
		return;
	 }
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}
