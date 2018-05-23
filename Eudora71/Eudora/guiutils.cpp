// GUIUTILS.CPP
//
// Various interface utilities
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#include <io.h>
#include <vfw.h>

#include <hes_api.h>

#include "resource.h"
#include "..\Version.h"
#include "guiutils.h"
#include "utils.h"
#include "doc.h"
#include "debug.h"
#include "rs.h"
#include "eudora.h"
#include "cursor.h"
#include "fileutil.h"
#include "jjnet.h"
#include "pop.h"
#include "helpxdlg.h"
#include "ddeclien.h"
#include "urledit.h"
#include "compmsgd.h"
#include "summary.h"
#include "wininet.h"
#include "address.h"
#include "qcrasconnection.h"
#include "QCWorkerSocket.h"
#include "persona.h"
#include "tocdoc.h"
#include "mainfrm.h"

#include "LinkHistoryManager.h"
#include "QCSharewareManager.h"
#include "PaymentAndRegistrationDlg.h"

#include "EudoraMsgs.h"
#include "SearchDoc.h"
#include "SearchView.h"

#include "ExtLaunchMgr.h"
#include "QCMailboxDirector.h"
extern QCMailboxDirector g_theMailboxDirector;
#include "QCMailboxCommand.h"
#include "QCCommandStack.h"
extern QCCommandStack g_theCommandStack;

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

#ifdef IMAP4 // 
#include "ImapGuiUtils.h"
#include "ImapAccountMgr.h"
#endif // IMAP4

#include "DebugNewHelpers.h"


// Function pointers for SHLWAPI.DLL functions 
typedef BOOL    (WINAPI * PFN_PathIsDirectory)(LPCSTR pszPath);


// Mailbox Score object for Transfer to Selection with fuzzy matches
CMailboxScore::CMailboxScore(QCMailboxCommand *pCommand, int iScore):
	m_pCommand(pCommand),
	m_iScore(iScore)
{
}

CMailboxScore::~CMailboxScore()
{
}


// Alert Dialog extension
static BOOL g_bCheckForAutoOk = FALSE;
void CheckForAutoOk( BOOL bOn ) { g_bCheckForAutoOk = bOn; }

/////////////////////////////////////////////////////////////////////////////
// CAlertDialog dialog

class CAlertDialog : public CHelpxDlg
{
// Construction
public:
	CAlertDialog(UINT DialogID, const char* Message, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CAlertDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	const char* m_Message;
	va_list m_Args;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CAlertDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
	UINT m_DialogID;
	UINT m_TimerID;
	time_t m_EndTime;
};
/////////////////////////////////////////////////////////////////////////////
// CAlertDialog dialog


CAlertDialog::CAlertDialog(UINT DialogID, const char* Message, CWnd* pParent /*=NULL*/)
	: CHelpxDlg(DialogID, pParent)
{
	//{{AFX_DATA_INIT(CAlertDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_DialogID = DialogID;
	m_Message = Message;
	m_TimerID = 0;
}

BOOL CAlertDialog::OnInitDialog()
{
	char buf[1024];
	
	CHelpxDlg::OnInitDialog();
	
	CWnd* Control = GetDlgItem(IDC_ALERT_TEXT);
	*buf = 0;
	if (Control)
	{
		if (!m_Message)
		{
			CString Format;
			Control->GetWindowText(Format);
			_vsnprintf(buf, sizeof(buf) - 1, Format, m_Args);
		}
		Control->SetWindowText(m_Message? m_Message : buf);
	}
	va_end(m_Args);

	ASSERT(!Control  || m_Message || *buf);
	PutDebugLog(DEBUG_MASK_DIALOG, m_Message? m_Message : buf);
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	// If we can auto-OK, notify every second
	if ((gPOP || NetConnection || g_bCheckForAutoOk) &&
		GetIniShort(IDS_INI_AUTO_OK) && GetDlgItem(IDOK))
	{
		m_EndTime = time(NULL) + GetIniLong(IDS_INI_AUTO_OK_TIMEOUT);
		m_TimerID = SetTimer(1, 1000, NULL);
	}
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CAlertDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlertDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAlertDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CAlertDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlertDialog message handlers

BOOL CAlertDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WORD Command = HIWORD(wParam);
	WORD ID = LOWORD(wParam);

	// If we got a pushbutton press, then end the dialog
	if (Command == BN_CLICKED && ID != IDOK && ID != IDCANCEL)
	{
		CWnd* Wnd = GetDlgItem(ID);
		if (Wnd)
		{
			DWORD ButtonStyle = (Wnd->GetStyle() & 0xF);
			if (ButtonStyle == BS_PUSHBUTTON || ButtonStyle == BS_DEFPUSHBUTTON)
			{
				EndDialog(ID);
				return (TRUE);
			}
		}
	}

	return (CHelpxDlg::OnCommand(wParam, lParam));
}

void CAlertDialog::OnOK() 
{
	// TODO: Add extra validation here
	if((m_DialogID == IDD_NEW_MAIL) ||(m_DialogID == IDD_NO_NEW_MAIL))
	{
		int nWarnCheckBox = ((CButton*)GetDlgItem(IDC_WARN_CHECKBOX))->GetCheck();		
		if(nWarnCheckBox == 1)
		{
			SetIniShort(IDS_INI_ALERT, 0);
		}
	}
	CDialog::OnOK();
}

void CAlertDialog::OnTimer(UINT nIDEvent)
{
	CHelpxDlg::OnTimer(nIDEvent);
	
	// Shall we auto-OK?
	if (m_TimerID && time(NULL) > m_EndTime)
		OnOK();
}

void CAlertDialog::OnDestroy()
{
	CHelpxDlg::OnDestroy();

	if (m_TimerID)
		KillTimer(m_TimerID);	
}

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxWithURL dialog

class CMsgBoxWithURL : public CHelpxDlg
{
// Construction
public:
	CMsgBoxWithURL(CString szTitle, CString szText, CString szUrlTxt, CString szUrlSite, int iIcon = IDD_SETTINGS_GET_ATTEN, CWnd* pParent = NULL);
private:
	int	m_iIcon;
	CString	m_szMessageText;
	CString	m_szURLLink;
	CString	m_szURLText;
	CString m_szTitle;

public:
// Dialog Data
	//{{AFX_DATA(CMsgBoxWithURL)
	enum { IDD = IDD_MSGBOX_URL };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgBoxWithURL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMsgBoxWithURL)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMessageUrl();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CMsgBoxWithURL dialog

CMsgBoxWithURL::CMsgBoxWithURL(CString szTitle, CString szText, CString szUrlTxt, CString szUrlSite, int iIcon, CWnd* pParent)
	: CHelpxDlg(CMsgBoxWithURL::IDD, pParent)
{
	if(szUrlSite.IsEmpty())
	{
		GetJumpURL(&szUrlSite);
		ConstructURLWithQuery(szUrlSite, ACTION_SUPPORT_NO_TOPIC);
	}
	if(szUrlTxt.IsEmpty())
		szUrlTxt = szUrlSite;

	m_iIcon = iIcon;
	m_szMessageText = szText;
	m_szURLLink = szUrlSite;
	m_szURLText = szUrlTxt;
	m_szTitle = szTitle;

}


void CMsgBoxWithURL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgBoxWithURL)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

HBRUSH CMsgBoxWithURL::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch(nCtlColor)
	{
		case CTLCOLOR_STATIC:
			if(pWnd->GetDlgCtrlID() == IDC_MESSAGE_URL)
				pDC->SetTextColor(RGB(0,0,128));
			break;


		default:
			break;
	}


	return hbr;
}

void CMsgBoxWithURL::OnMessageUrl() 
{

	ShellExecute(NULL, "open", m_szURLLink, NULL, NULL, SW_SHOW);
	EndDialog(MB_OK);
}

BOOL CMsgBoxWithURL::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	(GetDlgItem(IDC_MESSAGE_URL))->SetWindowText(m_szURLText);
	GetDlgItem(IDC_MESSAGE_TEXT)->SetWindowText(m_szMessageText);

	CWnd *pWnd = GetDlgItem(IDC_MSG_URL_ICON);
	HICON hicon;

	if(m_iIcon)
		hicon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(m_iIcon)); 
	else // oh poo, Neither was passed in? Ick! Looks like a good warning default.
		hicon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDD_SETTINGS_GET_ATTEN));
		
	
	((CStatic *)pWnd)->SetIcon(hicon);
	SetWindowText(m_szTitle);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CMsgBoxWithURL, CHelpxDlg)
	//{{AFX_MSG_MAP(CMsgBoxWithURL)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MESSAGE_URL, OnMessageUrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWarnDialog dialog

class CWarnDialog : public CHelpxDlg
{
// Construction
public:
	CWarnDialog(UINT IniID, const char* Message, CWnd* pParent = NULL, BOOL bYesNo = FALSE);

// Dialog Data
	//{{AFX_DATA(CWarnDialog)
	enum { IDD = IDD_WARN };
	CStatic	m_AlertText;
	BOOL	m_NoMoreWarn;
	//}}AFX_DATA
	const char* m_Message;
	UINT m_IniID;
	BOOL m_bYesNo;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CWarnDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CWarnDialog dialog


CWarnDialog::CWarnDialog(UINT IniID, const char* Message, CWnd* pParent /*=NULL*/, BOOL bYesNo /*=FALSE*/)
	: CHelpxDlg(CWarnDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWarnDialog)
	m_NoMoreWarn = FALSE;
	//}}AFX_DATA_INIT
	m_Message = Message;
	m_IniID = IniID;
	m_bYesNo = bYesNo;
}

BOOL CWarnDialog::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	
	m_AlertText.SetWindowText(m_Message);

	if ( m_bYesNo )
	{
		CRString yes( IDS_YES_BUTTON );
		CRString no( IDS_NO_BUTTON );

		GetDlgItem( IDOK )->SetWindowText( yes );
		GetDlgItem( IDCANCEL )->SetWindowText( no );
	}

	// conditionally hide the "stop bugging me" box
	if ( m_IniID == 0 )
	{
		CWnd* pWnd = GetDlgItem( IDC_WARN_CHECKBOX );
		pWnd->ShowWindow( SW_HIDE );
	}

	PutDebugLog(DEBUG_MASK_DIALOG, m_Message);
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CWarnDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWarnDialog)
	DDX_Control(pDX, IDC_ALERT_TEXT, m_AlertText);
	DDX_Check(pDX, IDC_WARN_CHECKBOX, m_NoMoreWarn);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		// If user checked the "Don't warn me anymore" checkbox, then set the switch
		if (m_NoMoreWarn)
			SetIniShort(m_IniID, FALSE);
	}
}

BEGIN_MESSAGE_MAP(CWarnDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CWarnDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWarnMoodDialog dialog
/////////////////////////////////////////////////////////////////////////////
// CWarnMoodDialog dialog
class CWarnMoodDialog : public CHelpxDlg
{
// Construction
public:
	CWarnMoodDialog(UINT IniID,UINT nNeverID, const char* Message, int nMoodScore, CWnd* pParent = NULL, BOOL bYesNo = FALSE);

// Dialog Data
	//{{AFX_DATA(CWarnMoodDialog)
	enum { IDD = IDD_WARN_MOODWATCH};
	CStatic	m_AlertText;
	BOOL	m_NoMoreWarn;
	//}}AFX_DATA
	CString m_strMessage;
	UINT m_IniID;
	UINT m_nIniNeverID;
	BOOL m_bYesNo;
	int m_nMoodScore;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CWarnMoodDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


CWarnMoodDialog::CWarnMoodDialog(UINT IniID, UINT nNeverID, const char* Message, int nMoodScore,
								CWnd* pParent /*=NULL*/, BOOL bYesNo /*=FALSE*/)
	: CHelpxDlg(CWarnMoodDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWarnMoodDialog)
	m_NoMoreWarn = FALSE;
	//}}AFX_DATA_INIT
	m_strMessage = Message;
	m_IniID = IniID;
	m_bYesNo = bYesNo;
	m_nIniNeverID = nNeverID;
	m_nMoodScore = nMoodScore;

}

BOOL CWarnMoodDialog::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	CStatic *pWnd = (CStatic*)GetDlgItem(IDC_MOOD_PICTURE);
	if(pWnd)
	{
		if(m_nMoodScore == 2)
			pWnd->SetIcon( QCLoadIcon(MAKEINTRESOURCE(IDI_MOODWATCH_32_SCORE1)));
		else if(m_nMoodScore == 3)
			pWnd->SetIcon( QCLoadIcon(MAKEINTRESOURCE(IDI_MOODWATCH_32_SCORE2)));
		else if(m_nMoodScore == 4)
			pWnd->SetIcon( QCLoadIcon(MAKEINTRESOURCE(IDD_SETTINGS_MOODMAIL)));
	}

	m_AlertText.SetWindowText(m_strMessage);

	if ( m_bYesNo )
	{
		/*CRString yes( IDS_YES_BUTTON );
		CRString no( IDS_NO_BUTTON );

		GetDlgItem( IDOK )->SetWindowText( yes );
		GetDlgItem( IDCANCEL )->SetWindowText( no );*/
	}
	// conditionally hide the "stop bugging me" box
	if ( m_IniID == 0 )
	{
		CWnd* pWnd = GetDlgItem( IDC_WARN_CHECKBOX );
		pWnd->ShowWindow( SW_HIDE );
	}

	PutDebugLog(DEBUG_MASK_DIALOG, m_strMessage);
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CWarnMoodDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWarnMoodDialog)
	DDX_Control(pDX, IDC_ALERT_TEXT, m_AlertText);
	DDX_Check(pDX, IDC_WARN_CHECKBOX, m_NoMoreWarn);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		// If user checked the "Don't warn me anymore" checkbox, then set the switch
		if (m_NoMoreWarn)
		{
			SetIniShort(m_IniID, FALSE);
			SetIniShort(m_nIniNeverID, TRUE);
		}
	}
}

BEGIN_MESSAGE_MAP(CWarnMoodDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CWarnMoodDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWarnOptionsDialog dialog:  This differs from CWarnDialog because it 
// has two options rather than just OK.
/////////////////////////////////////////////////////////////////////////////
// CWarnDialog dialog

class CWarnOptionsDialog : public CHelpxDlg
{
// Construction
public:
	CWarnOptionsDialog( UINT IniID, const char * Message, UINT button1,
						UINT button2, UINT nDefaultButtonID = 0, CWnd * pParent = NULL);


// Dialog Data
	//{{AFX_DATA(CWarnOptionsDialog)
	enum { IDD = IDD_WARN_OPTIONS };
	CStatic	m_AlertText;
	BOOL	m_NoMoreWarn;
	//}}AFX_DATA
	const char* m_Message;
	UINT m_IniID;
	UINT m_Button1, m_Button2;
	UINT m_nDefaultButtonID;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CWarnDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	virtual void OnButton2();
	virtual void OnButton1();

	DECLARE_MESSAGE_MAP()
};


CWarnOptionsDialog::CWarnOptionsDialog(
	UINT				IniID,
	const char *		Message,
	UINT				button1,
	UINT				button2,
	UINT				nDefaultButtonID /* = IDC_WARN_BUTTON1 */,
	CWnd *				pParent /* = NULL */)
	: CHelpxDlg(CWarnOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWarnOptionsDialog)
	m_NoMoreWarn = FALSE;
	//}}AFX_DATA_INIT
	m_Message = Message;
	m_IniID = IniID;
	m_Button1 = button1;
	m_Button2 = button2;
	m_nDefaultButtonID = nDefaultButtonID;
}

BOOL CWarnOptionsDialog::OnInitDialog()
{
	BOOL	bWindowsShouldSetFocus = TRUE;
	
	CHelpxDlg::OnInitDialog();
	
	m_AlertText.SetWindowText(m_Message);

	if (m_Button1 > 0)
	{
		CRString szButton1( m_Button1 );
		GetDlgItem( IDC_WARN_BUTTON1 )->SetWindowText( szButton1 );
	}
	else
	{
		GetDlgItem( IDC_WARN_BUTTON1 )->ShowWindow(SW_HIDE);
	}

	if (m_Button2 > 0)
	{
		CRString szButton2( m_Button2 );
		GetDlgItem( IDC_WARN_BUTTON2 )->SetWindowText( szButton2 );
	}
	else
	{
		GetDlgItem( IDC_WARN_BUTTON2 )->ShowWindow(SW_HIDE);
	}
	
	// conditionally hide the "stop bugging me" box
	if ( m_IniID == 0 )
	{
		CWnd* pWnd = GetDlgItem( IDC_WARN_CHECKBOX );
		pWnd->ShowWindow( SW_HIDE );

		pWnd = GetDlgItem( IDC_ALERT_TEXT );
		
		CRect txtRct, btnRct;
		pWnd->GetWindowRect(txtRct);
		ScreenToClient(txtRct);

		GetDlgItem( IDCANCEL )->GetWindowRect(btnRct);
		ScreenToClient(btnRct);


		txtRct.bottom = btnRct.top - 10;
		pWnd->MoveWindow(txtRct);
	}

	if (m_nDefaultButtonID != 0)
	{
		SetDefID(m_nDefaultButtonID);
		CButton *		pButton = reinterpret_cast<CButton *>( GetDlgItem(m_nDefaultButtonID) );
		if (pButton)
		{
			pButton->SetButtonStyle(BS_DEFPUSHBUTTON);
			pButton->SetFocus();
			bWindowsShouldSetFocus = FALSE;
		}
	}

	PutDebugLog(DEBUG_MASK_DIALOG, m_Message);
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return bWindowsShouldSetFocus;  // return TRUE  unless you set the focus to a control
}

void CWarnOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWarnDialog)
	DDX_Control(pDX, IDC_ALERT_TEXT, m_AlertText);
	DDX_Check(pDX, IDC_WARN_CHECKBOX, m_NoMoreWarn);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		// If user checked the "Don't warn me anymore" checkbox, then set the switch
		if (m_NoMoreWarn)
			SetIniShort(m_IniID, FALSE);
	}
}

void CWarnOptionsDialog::OnButton2()
{
	UpdateData(TRUE);
	EndDialog(IDC_WARN_BUTTON2);
}

void CWarnOptionsDialog::OnButton1()
{
	UpdateData(TRUE);
	EndDialog(IDC_WARN_BUTTON1);
}


BEGIN_MESSAGE_MAP(CWarnOptionsDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CWarnOptionsDialog)
	ON_BN_CLICKED( IDC_WARN_BUTTON1, OnButton1)
	ON_BN_CLICKED( IDC_WARN_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CNoteDialog dialog
// Override some behaviors of the warning dialog to provide a kinder,
// gentler "note" dialog.
//
class CNoteDialog : public CWarnDialog
{
// Construction
public:
	CNoteDialog(UINT IniID, const char* Message, CWnd* pParent = NULL) :
		CWarnDialog(IniID, Message, pParent)
		{ }

protected:
	// Generated message map functions
	//{{AFX_MSG(CNoteDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
};

BOOL CNoteDialog::OnInitDialog()
{
	//
	// Do base class stuff first ... then do some overrides later.
	//
	CWarnDialog::OnInitDialog();

	// Change the dialog title.
	SetWindowText(CRString(IDS_NOTE_TITLE));

	// Change the check box text.
	CButton* p_checkbox = (CButton *) GetDlgItem(IDC_WARN_CHECKBOX);
	ASSERT(p_checkbox);
	p_checkbox->SetWindowText(CRString(IDS_NOTE_CHECKBOX));

	// Change the icon.
	HICON h_icon = ::LoadIcon(NULL, IDI_ASTERISK);
	CStatic* p_icon = (CStatic *) GetDlgItem(IDC_WARN_ICON);
	ASSERT(p_icon);
	p_icon->SetIcon(h_icon);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CYesNoDlg dialog

class CYesNoDlg : public CHelpxDlg
{
// Construction
public:
	CYesNoDlg(UINT WarnIniID, UINT responseINI, const char* Message, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CYesNoDialog)
	enum { IDD = IDD_YES_NO };
	CStatic	m_AlertText;
	BOOL	m_NoMoreWarn;
	//}}AFX_DATA
	const char* m_Message;
	UINT m_WarnID;
	UINT m_ResponseID;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CYesNoDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CYesNoDlg dialog
CYesNoDlg::CYesNoDlg(UINT WarnIniID, UINT responseIniID, const char* Message, CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CYesNoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CYesNoDialog)
	m_NoMoreWarn = FALSE;
	//}}AFX_DATA_INIT
	m_Message = Message;
	m_WarnID = WarnIniID;
	m_ResponseID = responseIniID;
}

BOOL CYesNoDlg::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	
	m_AlertText.SetWindowText(m_Message);

	PutDebugLog(DEBUG_MASK_DIALOG, m_Message);
	
	if (m_WarnID == 0)
		GetDlgItem(IDC_WARN_CHECKBOX)->ShowWindow(SW_HIDE);

	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CYesNoDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYesNoDialog)
	DDX_Control(pDX, IDC_ALERT_TEXT, m_AlertText);
	DDX_Check(pDX, IDC_WARN_CHECKBOX, m_NoMoreWarn);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		// If user checked the "Don't warn me anymore" checkbox, then set the switch
		if ((m_WarnID > 0) && (m_NoMoreWarn))
			SetIniShort(m_WarnID, 0);
	}
}

void CYesNoDlg::OnOK()
{
	UpdateData(TRUE);
	if ((m_ResponseID > 0) && (m_NoMoreWarn))
		SetIniShort(m_ResponseID, 1);
	CHelpxDlg::OnOK();
}

void CYesNoDlg::OnCancel()
{
	UpdateData(TRUE);
	if ((m_ResponseID > 0) && (m_NoMoreWarn))
		SetIniShort(m_ResponseID, 0);
	CHelpxDlg::OnCancel();
}


BEGIN_MESSAGE_MAP(CYesNoDlg, CHelpxDlg)
	//{{AFX_MSG_MAP(CYesNoDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int AlertDialog(UINT DialogID, ...)
{
	ASSERT(::IsMainThreadMT());

//	CloseSplashScreen();

	CAlertDialog dlg(DialogID, NULL);
	
	va_start(dlg.m_Args, DialogID);

	int Result = dlg.DoModal();
	
	// Code duplicated below in WarnDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), Result);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated below in WarnDialog()

	return (Result);
}

void ErrorDialog(UINT StringID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);
	
	AlertDialog(IDD_ERROR_DIALOG, buf);
}

int WarnMoodDialog(UINT IniID,UINT nNeverID, CString Message, int nMoodScore,...)
{
	ASSERT(::IsMainThreadMT());

	
	MessageBeep(MB_ICONEXCLAMATION);

	
	if (IniID > 0)
	{
		ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	}
	
	CWarnMoodDialog dlg(IniID, nNeverID, Message,nMoodScore);
	
	int Result = dlg.DoModal();
	
	return (Result);
}


int WarnDialog(UINT IniID, UINT StringID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	ASSERT(*buf != 0);
	
	if (IniID > 0)
	{
		ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	}
	
	CWarnDialog dlg(IniID, buf);
	
	int Result = dlg.DoModal();
	
	// Code duplicated above in AlertDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), Result);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated above in AlertDialog()

	return (Result);
}

int WarnUrlDialog(CString szTitle, CString szText, CString szUrlTxt, CString szUrlSite, int iIcon, CWnd* pParent)
{
	ASSERT(::IsMainThreadMT());

	MessageBeep(MB_ICONEXCLAMATION);

	CMsgBoxWithURL dlg(szTitle, szText, szUrlTxt, szUrlSite, iIcon, pParent);

	int iRet = dlg.DoModal();
	// Code duplicated above in AlertDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), iRet);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated above in AlertDialog()

	return iRet;

}

int WarnYesNoDialog(UINT IniID, UINT StringID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	ASSERT(*buf != 0);

	if (IniID > 0)
	{
		ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	}

	CWarnDialog dlg(IniID, buf, NULL, TRUE);
	
	int Result = dlg.DoModal();
	
	// Code duplicated above in AlertDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), Result);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated above in AlertDialog()

	return (Result);
}

int InternalWarnOneOptionCancelDialog(UINT IniID, UINT StringID, UINT BtnTxtID, UINT nDefaultButtonID, va_list args)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	CRString str(StringID);
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	_vsnprintf(buf, sizeof(buf) - 1, str, args);

	ASSERT(*buf != 0);
	
	if (IniID > 0)
	{
		ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	}

	CWarnOptionsDialog dlg(IniID, buf, 0, BtnTxtID, nDefaultButtonID);
	
	int nResult = dlg.DoModal();
	
	// Code duplicated above in AlertDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), nResult);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated above in AlertDialog()

	return nResult;
}

int WarnOneOptionCancelDialog(UINT IniID, UINT StringID, UINT BtnTxtID, ...)
{
	va_list args;
	va_start(args, BtnTxtID);
	int nResult = InternalWarnOneOptionCancelDialog(IniID, StringID, BtnTxtID, 0, args);
	va_end(args);

	return nResult;
}

int WarnOneOptionCancelDialogWithDefButton(UINT IniID, UINT StringID, UINT BtnTxtID, UINT nDefaultButtonID, ...)
{
	va_list args;
	va_start(args, nDefaultButtonID);
	int nResult = InternalWarnOneOptionCancelDialog(IniID, StringID, BtnTxtID, nDefaultButtonID, args);
	va_end(args);

	return nResult;
}

int WarnTwoOptionsCancelDialog(UINT IniID, UINT StringID, UINT button1,UINT button2,...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, button2);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	ASSERT(*buf != 0);

	if (IniID > 0)
	{
		ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	}

	CWarnOptionsDialog dlg(IniID, buf, button1, button2);
	
	int Result = dlg.DoModal();
	
	// Code duplicated above in AlertDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), Result);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated above in AlertDialog()

	return (Result);
}



int NoteDialog(UINT IniID, UINT StringID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONINFORMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	ASSERT(*buf != 0);
	ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	
	CNoteDialog dlg(IniID, buf);
	
	int Result = dlg.DoModal();
	
	return (Result);
}

int YesNoDialogPlain(UINT StringID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONINFORMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	ASSERT(*buf != 0);
	
	CYesNoDlg dlg(0, 0, buf);
	
	int Result = dlg.DoModal();
	
	return (Result);
}

int YesNoDialog(UINT WarnID, UINT responseID, UINT StringID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	
	MessageBeep(MB_ICONINFORMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	ASSERT(*buf != 0);
	ASSERT(GetIniShort(WarnID) == TRUE);		// Switch better be turned off
	
	CYesNoDlg dlg(WarnID, responseID, buf);
	
	int Result = dlg.DoModal();
	
	return (Result);
}


void HesiodErrorDialog(int nHesiodError)
{
	ASSERT(::IsMainThreadMT());

	const UINT IDS_HESIOD_BASE_ERROR = 9050;
	UINT BaseLoadError = IDS_HESIOD_BASE_ERROR + 1;

	switch (nHesiodError)
	{
	case HES_ER_UNINIT:
	case HES_ER_NOTFOUND:
	case HES_ER_CONFIG:
	case HES_ER_NET:
		ErrorDialog(BaseLoadError + nHesiodError);
		break;

	default:		// Unknown error value <nHesiodError>
		ErrorDialog(BaseLoadError + 1, nHesiodError);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// GetGMTOffset [extern]
//
// Backwards compatibility wrapper for new thread-safe GetGMTOffsetMT()
// function.
////////////////////////////////////////////////////////////////////////
int GetGMTOffset()
{
	ASSERT(::IsMainThreadMT());

	int nGMTOffset = 0;		// returned
	char szTimeZone[64] = { '\0' };
	::GetIniString(IDS_INI_TIME_ZONE, szTimeZone, sizeof(szTimeZone));	// can be empty
	if (FAILED(::GetGMTOffsetMT(szTimeZone, &nGMTOffset)))
		::ErrorDialog(IDS_ERR_BAD_TZ, "TZ");

	return nGMTOffset;
}

bool
IsInAttachmentDir(
	LPCTSTR				Filename)
{
	bool		bIsInAttachmentDir = false;

	CString		szAttachDir;
	GetIniString(IDS_INI_AUTO_RECEIVE_DIR, szAttachDir);
	
	//	Check user setting for attachment directory
	if ( !szAttachDir.IsEmpty() && (strnicmp(szAttachDir, Filename, szAttachDir.GetLength()) == 0) )
		bIsInAttachmentDir = true;
	
	if (!bIsInAttachmentDir)
	{
		//	Check default attachment directory
		szAttachDir = EudoraDir;
		szAttachDir += CRString(IDS_ATTACH_FOLDER);
		
		if ( !szAttachDir.IsEmpty() && (strnicmp(szAttachDir, Filename, szAttachDir.GetLength()) == 0) )
			bIsInAttachmentDir = true;
	}

	return bIsInAttachmentDir;
}


bool
IsProgram(
	LPCTSTR				Filename,
	UINT				nINIAddExtensions,
	UINT				nINIRemoveExtensions)
{
	if (!Filename)
		return FALSE;
	
	//	Start out assuming file is not program
	bool		bIsProgram = false;
	CString		FileExt = Filename;

	//	Remove any trailing spaces and periods so that we can correctly recognize the
	//	actual file extension. Windows doesn't allow trailing spaces or periods and just
	//	silently ignores them
	FileExt.TrimRight(" .");

	int		nLastPeriodIndex = FileExt.ReverseFind('.');

	if (nLastPeriodIndex <= 0)
	{
		//	No file extension at all is rare, and not necessarily suspicious,
		//	but some security issues reported by Paul Szabo have taken advantage
		//	of the fact that Windows prefers to find files with extensions when
		//	no extension is included.
		bIsProgram = true;
	}
	else
	{
		FileExt = FileExt.Right(FileExt.GetLength() - nLastPeriodIndex - 1);

		CRString defaultExtString(IDS_WARN_LAUNCH_DEFAULTS);//default values from resource
		CString addExtString(::GetIniString(nINIAddExtensions));
		CString removeExtString(::GetIniString(nINIRemoveExtensions));
		CStringArray defExt, addExt, removeExt;

		ExtStringToStringArray(defaultExtString, defExt);
		ExtStringToStringArray(addExtString, addExt);
		ExtStringToStringArray(removeExtString, removeExt);

		bool inDefault = ContainsString(defExt, FileExt);
		bool inAdd = ContainsString(addExt, FileExt);
		bool inRemove = ContainsString(removeExt, FileExt);
		
		if (addExtString == "|" && !inRemove)
			bIsProgram = true;

		if ( inAdd || (inDefault && !inRemove) )
			bIsProgram = true;
	}

	return bIsProgram;
}


CString ExtStringArrayToString(CStringArray& elist)
{
	CString Extensions;
	int nSize = elist.GetSize();
	Extensions.Empty();

	for(int i =0 ;i <nSize ; i++)
		{
		 if (!elist[i].IsEmpty() )
			{
			Extensions += elist[i];
			if(i != nSize -1)
				Extensions += "|";
			}
		}
	return Extensions;
}


bool ExtStringToStringArray(CString Extensions, CStringArray& elist)
{
	if (Extensions.IsEmpty())
		return FALSE;

	if (Extensions[Extensions.GetLength() - 1] != '|')
		Extensions += "|"; // add last "|" for uniformity and ease of parsing.

	LPTSTR Extension = Extensions.GetBuffer(Extensions.GetLength());
	LPTSTR Separator;

	while (Separator = strchr(Extension, '|'))
	{
		if (Separator)
			*Separator = 0;
		CString extension(Extension);
		if (!extension.IsEmpty())
			elist.Add(extension);

		if (!Separator)
			break;

		Extension = Separator + 1;
	}
	Extensions.ReleaseBuffer();
	return elist.GetSize()>0;
}

bool ContainsString(CStringArray& elist, LPCTSTR str)
{
	int nSize = elist.GetSize();
	int nMain = strlen(str);

	for( int i =0 ; i < nSize; i++)
		{
		int nSub = elist[i].GetLength();
		if ( nSub <= nMain )
			{
	    	if(_strnicmp(str,elist[i],nSub) == 0)
				{
				return TRUE;
				}
			}
		}
	return FALSE;
}


//list 3 = list1 - list2
//list3 contains all elements that are in list1 but not in list2
bool Difference(CStringArray& list1, CStringArray& list2, CStringArray& diff)
{
	int nSize = list1.GetSize();

	for(int i =0 ;i <nSize ; i++)
		{
		if (!list1[i].IsEmpty() && !ContainsString(list2, list1[i]) && !ContainsString(diff, list1[i]))
			diff.Add(list1[i]);
		}
	return diff.GetSize()>0;
}



BOOL SaveAttachmentSomewhere(const char *Filename, char *newPath/*=NULL*/)
{
	CRString titleFormat(IDS_SAVEAS_FORMAT);
	CRString filterFormat(IDS_SAVEAS_FILTER_FORMAT);
	char title[MAX_PATH+65];
	char path[MAX_PATH+1];
	char *ext = strrchr(Filename,'.');
	char *bslash = strrchr(Filename,'\\');
	char filter[MAX_PATH];

	if (ext) ext++;
	if (bslash) bslash++;

	sprintf(title,titleFormat,bslash?bslash:Filename);
	if (ext) sprintf(filter,filterFormat,ext,ext);
	
	//StartDir = EudoraDir;
	
	CFileDialog dlg (FALSE, ext, Filename,
		OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT, ext?filter:NULL,
		AfxGetMainWnd());
	
	dlg.m_ofn.Flags &= ~OFN_SHOWHELP;
	dlg.m_ofn.lpstrTitle = title;
	//dlg.m_ofn.lpstrInitialDir = StartDir;

	if (dlg.DoModal() == IDOK)
	{
		strcpy(path,dlg.GetPathName());

		// now copy the file
		JJFile srcfile;
		
		// Don't display errors when attachments can't be found
		srcfile.DisplayErrors(FALSE);
		if (FAILED(srcfile.Open(Filename, O_RDONLY)))
			return FALSE;

		struct stat stat_info;
		if (FAILED(srcfile.Stat(&stat_info)))
			return FALSE;

		JJFile destfile;
		if (FAILED(destfile.Open(path, O_CREAT | O_TRUNC | O_WRONLY)))
			return FALSE;

		if (FAILED(srcfile.JJBlockMove(0L, stat_info.st_size, &destfile)))
		{
			destfile.Delete();
			return FALSE;
		}

		// return the new path if anybody cares
		if (newPath) strcpy(newPath,path);
		return TRUE;
	}

	return FALSE;
}

BOOL
DoFileCommand(
	const char *		Filename,
	int					command /* = 0 */,
	bool				bWarnIfOpeningAndNotInAttachmentDir /* = true  */,
	int					extraFlags /* = 0 */ )
{
	ASSERT(::IsMainThreadMT());

	if (!Filename || !*Filename)
		return FALSE;

	//	If we could be opening the file, then check to see if the attachment
	//	is suspicious.
	if ( (command != ID_SAVEAS_ATTACHMENT) &&
		 (command != ID_EXPLORE_ATTACHMENT) &&
		 (command != ID_DELETE_ATTACHMENT) )
	{
		//	Check to see what we're opening, and whether or not that's something
		//	we need to warn the user about. The current rules are:
		//	* We're opening a directory - no warning
		//	* We're opening a program - warning if setting is on
		//	* We're opening a file that's not in the attachment directory - warning always
		bool		bIsDirectory = false;

		//	The PathIsDirectory API should be generally available, but for Windows 95
		//	or NT it's only available when Internet Explorer 4.0 or later is installed.
		//	Dynamically check for availability of PathIsDirectory to be certain that
		//	we don't break ourselves for those versions of Windows.
		HINSTANCE	hModSHLWAPI = LoadLibrary( _T("SHLWAPI.DLL") );
		
		if (hModSHLWAPI)
		{
			PFN_PathIsDirectory		pfnPathIsDirectory =
				reinterpret_cast<PFN_PathIsDirectory>( GetProcAddress(hModSHLWAPI, "PathIsDirectoryA") );

			if (pfnPathIsDirectory)
				bIsDirectory = (pfnPathIsDirectory(Filename) != FALSE);

			FreeLibrary(hModSHLWAPI);
		}
		
		//	Assume that the attachment is safe
		bool		bIsInAttachmentDir = false;
		bool		bIsSuspicious = false;
		
		if (!bIsDirectory)
		{
			//	We're not dealing with a directory - check to see if the file is in
			//	the attachment directory. If not, then it's automatically suspicious.
			bIsInAttachmentDir = bWarnIfOpeningAndNotInAttachmentDir ?
								 IsInAttachmentDir(Filename) : true;

			//	If the file is in the attachment directory, check to see if it's a program.
			bIsSuspicious = !bIsInAttachmentDir || IsProgram(Filename);
		}

		if (bIsSuspicious)
		{
			CExternalLaunchManager::RecacheDataFromRegistry();
			CExternalLaunchManager::UpdateAlreadyWarnedSetting(Filename, TRUE);

			//	If the file's not in the attachment directory, consider that really
			//	suspicious and warn no matter what.
			if (GetIniShort(IDS_INI_WARN_LAUNCH_PROGRAM) || !bIsInAttachmentDir)
			{
				//	Separate the file name from the directory name to make sure that the
				//	user can read the entire file name (the directory name may still
				//	be long enough to be cut off).
				CString		File = Filename;
				CString		Dir;

				int			i = File.ReverseFind('\\');
				if (i > 0)
				{
					Dir = File.Left(i);
					File = File.Right(File.GetLength() - i - 1);
				}
				
				int		nResult = WarnOneOptionCancelDialog( 0, IDS_WARN_LAUNCH_PROGRAM,
															 IDS_WARN_LAUNCH_BTN, File, Dir);
				if (nResult == IDCANCEL)
				{
					// Reset the AlreadyWarned setting.
					CExternalLaunchManager::UpdateAlreadyWarnedSetting(Filename, FALSE);

					// It's *VERY* important to return TRUE here.  We want it to look like we handled
					// launching the URL, and not let any default code run it anyway.
					return TRUE;
				}	
			}
		}
	}

#ifdef IMAP4
	// Go see if this is an IMAP attachment file and if we need to fetch it's 
	// contents. It will simply replace the contents of the file and let us continue. 
	// Only bother with this if we won't be deleting the file (because
	// it makes no sense to fetch the attachment only to delete it).
	if ( (command != ID_DELETE_ATTACHMENT) && !ImapGuiFetchAttachment (Filename) )
	{
		return FALSE;
	}
		
#endif // IMAP4

	SHELLEXECUTEINFO sei;
	CString Verb, File, Parameters;

	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.hwnd = AfxGetMainWnd()->GetSafeHwnd();
	sei.nShow = SW_SHOWNORMAL;

	// Does the user want special processing?
	switch (command)
	{
	case ID_SAVEAS_ATTACHMENT:
		SaveAttachmentSomewhere(Filename);
		return TRUE;

	case ID_OPENWITH_ATTACHMENT:
		Parameters = "shell32.dll,OpenAs_RunDLL ";
		Parameters += Filename;
		File = "rundll32.exe";
		break;

	case ID_EXPLORE_ATTACHMENT:
		if (GetIniShort(IDS_INI_USE_EXPLORE_VERB))
		{
			Verb = "explore";
			File = Filename;
			int i;
			if ((i = File.ReverseFind('\\')) > 0)
				File.ReleaseBuffer(i + 1);
		}
		else
		{
			File = "explorer.exe";
			Parameters = GetIniString(IDS_INI_EXPLORE_ATTACH_OPTIONS);
			Parameters += Filename;
		}
		break;

	case ID_DELETE_ATTACHMENT:
		SHFILEOPSTRUCT sfos;
		ZeroMemory(&sfos, sizeof(sfos));
		sfos.hwnd = sei.hwnd;
		sfos.wFunc = FO_DELETE;

		// SHFileOperation() takes a list of files, so it has to be double NULL-terminated
		TCHAR Files[_MAX_PATH + 2];
		ZeroMemory(&Files, sizeof(Files));
		strncpy(Files, Filename, sizeof(Files) - 2);
		sfos.pFrom = Files;
		
		sfos.fFlags = static_cast<FILEOP_FLAGS>(FOF_SILENT | extraFlags);
		if (!ShiftDown())
			sfos.fFlags |= FOF_ALLOWUNDO;

		return (SHFileOperation(&sfos) == 0);

	default:
		// Is there a special command we don't know about?
		ASSERT(command == 0);
		// Don't use the "open" verb as it should do whatever action the user
		// has specified is the default action
		//Verb = "open";
		File = Filename;
		break;
	}

	sei.lpVerb = (LPCTSTR)Verb;
	sei.lpFile = (LPCTSTR)File;
	sei.lpParameters = (LPCTSTR)Parameters;

	TCHAR SysDir[MAX_PATH + 1];
	GetSystemDirectory(SysDir, sizeof(SysDir) / sizeof(TCHAR));
	sei.lpDirectory = SysDir;

	ShellExecuteEx(&sei);

	return TRUE;
}


// EscapePressed
// Checks to see if the escape key has been hit (usually for when an operation
// needs to be canceled).  It also gives back time to other processes
// (as a side-effect of PeekMessage().
// If Repost is TRUE, then this function will Repost the message
// so functions using this just to give back time don't swallow the escape
// Returns TRUE if escape key was found, otherwise FALSE.
//
int EscapePressed(int Repost /*= FALSE*/)
{
	ASSERT(::IsMainThreadMT());

	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		// If Esc key was hit, restore mailbox and quit out
		if (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE)
		{
			if (Repost)
				::PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
			return (TRUE);
		}
		DispatchMessage(&msg);

	}

	return (FALSE);
}


// AsynchEscapePressed
// Checks to see if the escape key has been hit since the last time
// this function was called with a 'bReset' value of TRUE.
// Returns TRUE if escape key was pressed, otherwise FALSE.
//
BOOL AsyncEscapePressed(BOOL bReset /* =FALSE */)
{
	if (bReset)
	{
		//
		// Not checking this time through... Setup and wait for 
		// the next call...
		//
		int state = ::GetAsyncKeyState(VK_ESCAPE);
		if (state < 0)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		int state = ::GetAsyncKeyState(VK_ESCAPE);
		if (state)
			return TRUE;		// key is currently down, or was pressed since last call to GetAsyncKeyState()
		else
			return FALSE;
	}
}


// MAPIInstall
// Installs Eudora MAPI DLLs, giving error dialogs if failed

CMapiInstaller::Status MAPIInstall()
{
	ASSERT(::IsMainThreadMT());

	CMapiInstaller::Status Stat = CMapiInstaller::Install();

	ASSERT(Stat != CMapiInstaller::STATUS_NOT_INSTALLED);

	if (Stat > CMapiInstaller::STATUS_NOT_INSTALLED)
	{
		CRString Reason(IDS_ERR_MAPI_DLL_NOT_FOUND + Stat - CMapiInstaller::STATUS_SOURCE_DLL_NOT_FOUND);
		ErrorDialog(IDS_ERR_MAPI_INSTALL, (const char*)Reason);
	}

	return (Stat);
}

// MAPIUninstall
// Uninstalls Eudora MAPI DLLs, giving error dialogs if failed

CMapiInstaller::Status MAPIUninstall(BOOL bShuttingDown /*=FALSE*/)
{
	ASSERT(::IsMainThreadMT());

	CMapiInstaller::Status Stat = CMapiInstaller::Uninstall();

	if (Stat > CMapiInstaller::STATUS_NOT_INSTALLED)
	{
		CRString Reason(IDS_ERR_MAPI_DLL_NOT_FOUND + Stat - CMapiInstaller::STATUS_SOURCE_DLL_NOT_FOUND);

		if (bShuttingDown)
		{
			//
			// Shutting down Eudora case gets special treatment ... we want
			// to give the user a chance to bail the shutdown sequence.
			//
			BOOL warn = GetIniShort(IDS_INI_WARN_MAPI_UNINSTALL);
			if ((!warn) || (warn && (YesNoDialog(IDS_INI_WARN_MAPI_UNINSTALL,
												IDS_INI_QUIT_MAPI_UNINSTALL, 
												IDS_WARN_MAPI_UNINSTALL, Reason) == IDOK)))
			{
				//
				// User wants to quit anyway, so fool the caller
				// into thinking that everything went swimmingly.
				//
				Stat = CMapiInstaller::STATUS_NOT_INSTALLED;
			}
		}
		else
			ErrorDialog(IDS_ERR_MAPI_UNINSTALL, (const char*)Reason);			
	}

	return (Stat);
}



/////////////////////////////////////////////////////////////////////////////
// CChangeOptionDialog dialog

class CChangeOptionDialog : public CDialog
{
// Construction
public:
	CChangeOptionDialog(LPCTSTR SectionName, LPCTSTR EntryName, LPCTSTR NewValue = NULL, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CChangeOptionDialog)
	enum { IDD = IDD_CHANGE_OPTION };
	CString	m_SectionName;
	CString	m_EntryName;
	CString	m_DefaultValue;
	CString	m_CurrentValue;
	CString	m_NewValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeOptionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeOptionDialog)
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	UINT m_ID;
};
/////////////////////////////////////////////////////////////////////////////
// CChangeOptionDialog dialog


CChangeOptionDialog::CChangeOptionDialog(LPCTSTR SectionName, LPCTSTR EntryName, LPCTSTR NewValue /*=NULL*/, CWnd* pParent /*=NULL*/)
	: CDialog(CChangeOptionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeOptionDialog)
	m_SectionName = SectionName;
	m_EntryName = EntryName;
	m_CurrentValue = _T("");
	m_NewValue = NewValue;
	//}}AFX_DATA_INIT

	m_ID = 0;

	for (UINT i = IDS_INISN_SETTINGS + 1; i <= _______IDS_IN_THE_RANGE_10000_TO_11999_ARE_FOR_INI_ENTRIES_ONLY_; i++)
	{
		// Skip past the OT strings
		if (i == _______IDS_IN_THE_RANGE_11000_TO_11099_ARE_FOR_OBJECTIVE_TOOLKIT_STRINGS)
			i = IDS_INISN_SETTINGS2 + 1;

		TCHAR INIEntryName[256];
		if (QCLoadString(i, INIEntryName, sizeof(INIEntryName)))
		{
			int Len = strlen(EntryName);
			if (strnicmp(EntryName, INIEntryName, Len) == 0 &&
				(INIEntryName[Len] == 0 || INIEntryName[Len] == '\n'))
			{
				UINT SectionNameID;
				TCHAR SupposedSectionName[256];

				if (i < IDS_INISN_DEBUG || i > IDS_INISN_SETTINGS2)
					SectionNameID = IDS_INISN_SETTINGS;
				else
					SectionNameID = (i / 100) * 100;
				if (!QCLoadString(SectionNameID, SupposedSectionName, sizeof(SupposedSectionName)))
				{
					// There should always be a section name for an INI entry.  If this assert
					// happens, the string resources for INI entries are badly formatted.
					ASSERT(0);
				}

				if (m_SectionName.IsEmpty())
					m_SectionName = SupposedSectionName;
				else
				{
					if (m_SectionName.CompareNoCase(SupposedSectionName))
					{
						// The specified section name takes precedence over the section name of the
						// matched entry name, even if it's not going to match any known IDs.
						continue;
					}
				}

				m_ID = i;
				GetIniString(m_ID, m_CurrentValue);
				GetDefaultIniSetting(m_ID, m_DefaultValue);
				break;
			}
		}
	}

	if (m_SectionName.IsEmpty())
		m_SectionName = CRString(IDS_INISN_SETTINGS);

	if (!m_ID)
	{
		// If it's not a known ID, then still allow it, but we have
		// to get the current value through the SDK INI functions.
		CString Value;
		if (GetEudoraProfileString(m_SectionName, m_EntryName, &Value))
			m_CurrentValue = Value;
	}

	if (!NewValue)
	{
		// If there was no specified new value (note that this is different from a specified
		// value that is empty), then default the new value to the current value.
		// This will allow someone to keep around a URL to a particular option, and can slightly
		// modify the current value easily (which is the behavior you see in the Options dialog).
		m_NewValue = m_CurrentValue;
	}
}


void CChangeOptionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeOptionDialog)
	DDX_Text(pDX, IDC_CO_SECTION_NAME, m_SectionName);
	DDX_Text(pDX, IDC_CO_ENTRY_NAME, m_EntryName);
	DDX_Text(pDX, IDC_CO_DEFAULT_VALUE, m_DefaultValue);
	DDX_Text(pDX, IDC_CO_CURRENT_VALUE, m_CurrentValue);
	DDX_Text(pDX, IDC_CO_NEW_VALUE, m_NewValue);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		if (m_ID)
		{
			SetIniString(m_ID, m_NewValue);

			// Notify anyone who cares that an INI value has changed
			NotifyIniUpdates(m_ID);
		}
		else
		{
			WriteEudoraProfileString(m_SectionName, m_EntryName, m_NewValue);
		}
	}
	else
	{
		CWnd* HelpDirections = GetDlgItem(IDC_CO_HELP_DIRECTIONS);
		if (m_ID && HelpDirections)
			HelpDirections->ShowWindow(SW_SHOWNA);
	}
}


BEGIN_MESSAGE_MAP(CChangeOptionDialog, CDialog)
	//{{AFX_MSG_MAP(CChangeOptionDialog)
	ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeOptionDialog message handlers

LRESULT CChangeOptionDialog::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0 && m_ID)
	{
		AfxGetApp()->WinHelp(m_ID + HID_BASE_COMMAND);
		return TRUE;
	}

	return CDialog::OnCommandHelp(wParam, lParam);
}


// Returns true if the global state is offline. Otherwise, false.
BOOL IsOffline(void)
{
	// Hack if someone is known to be always connected (e.g, LAN or ISDN), but below methods fail.
	if (GetIniShort(IDS_INI_ALWAYS_CONNECTED))
		return FALSE;

	static BOOL sLastCheckState = FALSE;
	static DWORD sLastCheckTime = 0;
	DWORD Now = GetTickCount();
	if (Now < sLastCheckTime + GetIniLong(IDS_INI_CONNECTION_STATUS_CACHE_TIME))
		return sLastCheckState;
	sLastCheckTime = Now;

	// InternetQueryOption() is the wrong function to use.  All it tells you is if the user has
	// chosen to be offline, which means that you shouldn't ever try to automatically establish
	// a connection.  In that sense, it is very much like Eudora's offline option.
	//
	// So what we will do is use the fucntion InternetGetConnectedState() to see if the user
	// actually is connected at the time.  The only problem is that InternetGetConnectedState()
	// is not available before wininet version 4.71 (the version that shipped with IE 4.0), so
	// we have to dynamically load it.  If the function is not there, we assume we are offline
	// and continue to use other tests to determine if we are actually online.

	typedef  BOOL (WINAPI* PFNINTERNETGETCONNECTEDSTATE)(LPDWORD, DWORD);
	static PFNINTERNETGETCONNECTEDSTATE pfnInternetGetConnectedState = (PFNINTERNETGETCONNECTEDSTATE)-1;
	sLastCheckState = TRUE;

	if (pfnInternetGetConnectedState)
	{
		if (pfnInternetGetConnectedState == (PFNINTERNETGETCONNECTEDSTATE)-1)
		{
			HINSTANCE hInst = GetModuleHandle("wininet.dll");
			if (!hInst)
				pfnInternetGetConnectedState = NULL;
			else
				pfnInternetGetConnectedState = (PFNINTERNETGETCONNECTEDSTATE)GetProcAddress(hInst, "InternetGetConnectedState");
		}
		if (pfnInternetGetConnectedState)
		{
			DWORD dwFlags = 0;
			sLastCheckState = !(*pfnInternetGetConnectedState)(&dwFlags, 0);
		}
	}

	// If wininet says we're not connected, we'll just see if maybe they're
	// unaware of the fact that we know how to connect too!
	if (sLastCheckState)
		sLastCheckState = (QCRasLibrary::IsActive() == false);

	if (sLastCheckState)
	{
		// InternetGetConnectedState() seems to correctly tell if we're online or not if it started
		// up the connection.  In that case, don't call QCWorkerSocket::HasOpenConnection()
		// because it can return false positives.
		if (!pfnInternetGetConnectedState)
			sLastCheckState = (QCWorkerSocket::HasOpenConnection() == false);
	}

	return sLastCheckState;
}


BOOL LaunchURL(LPCSTR in_szURL, LPCSTR in_szLinkText /* = NULL */, int in_command /* = 0 */)
{
	BOOL		bLaunchGood = TRUE;

	//	Strip "URL:" from start of URL if present to comply with RFC 1738
	CString		szURL = in_szURL;
	StripLeadingResourceStrings(szURL, IDS_URL_DESIGNATOR, ',');
	
	//	Check ShouldLaunchURL first, which checks offline status and
	//	user offline URL preferences (i.e. visit, remind, bookmark).
	//	ShouldLaunchURL only limits for URL kinds that the LinkHistoryManager
	//	tracks so this should work fine for all URLs.
	//	Second parameter false because this URL is not an ad (ads go through CAdView::LaunchURL)
	if ( LinkHistoryManager::Instance()->ShouldLaunchURL(szURL, false, in_szLinkText) )
	{
		bLaunchGood = LaunchURLNoHistory(szURL, in_szLinkText, in_command);

		//	Last parameter false because this URL is not an ad (ads go through CAdView::LaunchURL)
		LinkHistoryManager::Instance()->AddVisitedURL(szURL, bLaunchGood, false);
	}

	return bLaunchGood;
}


BOOL LaunchURLNoHistory(LPCSTR szURL, LPCSTR szLinkText /* = NULL */, int command /* = 0 */)
{
	CString EscapedParens;
	CString	sz;
	INT		i;

	// Zeroth, let's see if these are our special "buying" urls
	if (!stricmp(szURL,CRString(IDS_EUDORA_BUYING_BY_HTTP_URL)) || !stricmp(szURL,CRString(IDS_EUDORA_BUYING_BY_MAIL_URL)))
	{
		// help the user pay
		CPaymentAndRegWebStepInfoDlg	dlg( IDD_PAYMENT_REG_INFO,
											 IDS_WEB_STEP_INFO_DIALOG_PAYMENT_DLG_TITLE,
											 IDS_WEB_STEP_INFO_DIALOG_PAYMENT_MSG_TITLE,
											 IDS_WEB_STEP_INFO_DIALOG_PAYMENT_MSG );
		if (dlg.DoModal() == IDOK)
		{
			LaunchURLWithQuery(NULL, ACTION_PAY);
		}
		return TRUE;
	}

	// First, let's check out what the URL is, because we want to intercept a couple of them
	if (!strnicmp( szURL, "mailto:", 7))
	{
		CString to;
		CString cc;
		CString bcc;
		CString subject;
		CString xeudora;
		CString body;
 
		sz = UnEscapeURL( szURL );

		if( CURLEdit::ParseMailtoArgs( sz, to, cc, bcc, xeudora, subject, body))
		{
			//
			// Create Eudora composition window with as many fields
			// filled as possible, then display it.
			//
			CCompMessageDoc* pComp = NewCompDocument(to, NULL, subject, cc, bcc, NULL, body);
	
			if (pComp)
			{
				// Add in X-Eudora headers.
				pComp->m_ExtraHeaders += xeudora;
				// Show the composition document.
				pComp->m_Sum->Display();
				return TRUE;
			}
		}

		return FALSE;
	}
	else if (!strnicmp( szURL, "x-eudora-option:", 16))
	{
		TCHAR UrlBuf[1024];
		LPTSTR EntryName = UrlBuf;
		LPTSTR Value = EntryName;

		strncpy(UrlBuf, UnEscapeURL(szURL+16), sizeof(UrlBuf));
		UrlBuf[sizeof(UrlBuf) - 1] = 0;

		while (*Value && *Value != '=' && *Value != '&')
			Value++;

		if (Value == EntryName)
			ASSERT(0);
		else
		{
			LPTSTR SectionName = strchr(Value, '&');

			if (*Value == '=')
				*Value++ = 0;
			else
			{
				if (*Value == '&')
					*Value = 0;
				Value = NULL;
			}

			if (SectionName)
			{
				if (strnicmp(SectionName + 1, "section=", 8) == 0)
				{
					*SectionName = 0;
					SectionName += 9;
				}
				else
					SectionName = NULL;
			}

			CChangeOptionDialog dlg(SectionName, EntryName, Value);

			dlg.DoModal();
		}

		return TRUE;
	}
	else if (!strnicmp( szURL, "x-eudora-transfer:", 18))
	{
		// Special functionality to transfer message(s) to a mailbox at the root level
		// that's named in the URL.  Particularly handy with the Selected Text URL feature.
		const char* Name = szURL + 18;

		while (isspace((unsigned char)*Name))
			Name++;

		return TransferToSelection(Name);
	}
#ifdef IMPLEMENTED
	else if (!strnicmp(szURL, "ph:", 3) || !strnicmp(szURL, "finger:", 7))
	{
		BOOL IsPh = (tolower(*szURL) == 'p');
		sz = UnEscapeURL( szURL );
		
		CString strBuffer = sz.Right( sz.GetLength() - (IsPh? 3 : 7));

		//
		// First, strip off leading slashes.
		//
		while (strBuffer[0] == '/')
			strBuffer = strBuffer.Right(strBuffer.GetLength() - 1);

		//
		// Next, decide whether we have a URL of the form
		// "command@server" or "server/command" and parse out the
		// substrings appropriately.
		//
		CString strServer;
		CString strCommand;
		int nAtSign = strBuffer.Find('@');
		if (nAtSign != -1)
		{
			//
			// Have URL of the form "command@server"
			//
			strCommand = strBuffer.Left(nAtSign);
			strServer = strBuffer.Right(strBuffer.GetLength() - nAtSign - 1);
		}
		else
		{
			//
			// Have URL of the form "server/command", where the "/command"
			// part is optional.
			//
			int nSlash = strBuffer.Find('/');
			if (nSlash != -1)
			{
				strServer = strBuffer.Left(nSlash);
				strCommand = strBuffer.Right(strBuffer.GetLength() - nSlash - 1);
			}
			else
				strServer = strBuffer;
		}

		return (LookupQuery(strServer, strCommand, IsPh));
	}
#endif
	
	BOOL bIsFile = FALSE;
	CString	szFile;
	if( strnicmp( szURL, "file:", 5 ) == 0 )
	{
		bIsFile = TRUE;
		sz = UnEscapeURL( szURL );

		for( i = 5; ( i < sz.GetLength() ) && ( sz[ i ] == '/' ); i ++ );
		szFile = sz.Right( sz.GetLength() - i );
		
		// apparently, open file doesn't like forward slashes
		i = szFile.Find( '/' );

		while( i >= 0 )
		{
			sz = szFile.Left( i );
			sz += '\\';
			szFile = sz + szFile.Right( szFile.GetLength() - i - 1 );
			i = szFile.Find( '/' );
		}

		szURL = szFile;
	}

	if (bIsFile || szURL[1] == ':' || (szURL[0] == '\\' && szURL[1] == '\\'))
	{
		return ::DoFileCommand( szURL, command );
	}


	if (GetIniShort(IDS_INI_NETSCAPE_URL_DDE))
	{
		//
		// By executive decision, the first crack at launching the URL
		// goes to Netscape since it supports a DDE link.  If that fails,
		// then it probably means that Netscape is not currently running,
		// so the fallback is to use either the ShellExecute() function or
		// the URLHelper INI thing.
		//
		CDDEClient dde_client;
		if( dde_client.OpenNetscapeURL( szURL ) )
			return (TRUE);
	}

	// wininet 4.70.x has a bug where you can't shellexec a URL that has parentheses in it
	if (!IsAtLeastWININET4_71())
	{
		// Makes sure there's enough space in the destination buffer so we're not
		// doing the CString constant buffer reallocation lameness.
		EscapedParens.GetBuffer(strlen(szURL) * 3);

		while (*szURL)
		{
			TCHAR ch = *szURL++;
			switch (ch)
			{
			case '(':	EscapedParens += "%28";	break;
			case ')':	EscapedParens += "%29";	break;
			default:	EscapedParens += ch;	break;
			}
		}

		// szURL needs to point to a buffer that still exists, which is why
		// EscapedParens is declared at the top scope of this function
		szURL = EscapedParens;
	}

	LPCTSTR pOperation = GetIniString((IDS_INI_LAUNCH_URL_OPERATION));
	if (pOperation && !*pOperation)
		pOperation = NULL;
	int		SE = reinterpret_cast<int>( ShellExecute(AfxGetMainWnd()->m_hWnd, pOperation, szURL, NULL, NULL, SW_SHOWNORMAL) );
	if (SE > 32)
		return TRUE;

	// Don't continue unless the error indicates that file association could be
	// the cause of failure and the path is short enough to be a valid file
	// (and one that will fit in the Title buffer below without overflowing it).
	if ( ((SE != SE_ERR_ASSOCINCOMPLETE) && (SE != SE_ERR_NOASSOC)) ||
		 (strlen(szURL) > _MAX_PATH) )
	{
		return FALSE;
	}

	while (1)
	{
		const char* URLHelper = GetIniString(IDS_INI_URL_HELPER);
		if (URLHelper && *URLHelper)
		{
			SE = reinterpret_cast<int>( ShellExecute(AfxGetMainWnd()->m_hWnd, NULL, URLHelper, szURL, NULL, SW_SHOWNORMAL) );
			if (SE > 32)
				return (TRUE);

			// If the we got any error besides path/file doesn't exist, then we're done
			if ( (SE != ERROR_FILE_NOT_FOUND) && (SE != ERROR_PATH_NOT_FOUND) )
				return (FALSE);
		}

		char Title[_MAX_PATH + 64];
		CRString ProgramsFilter(IDS_PROGRAMS_FILTER);
		
		CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_SHAREAWARE,
			ProgramsFilter, AfxGetMainWnd());
			
		sprintf(Title, CRString(IDS_EXEC_FILE_TITLE), szURL);
		dlg.m_ofn.lpstrTitle = Title;
		dlg.m_ofn.Flags &= ~OFN_SHOWHELP;
		
		if (dlg.DoModal() != IDOK)
			return (FALSE);

		SetIniString(IDS_INI_URL_HELPER, dlg.GetPathName());
	}

	return (FALSE);
}

BOOL  ConstructURLWithQuery (CString& theURL, const ActionInfo& action,
							 // the arguments below are all optional
							 LPCSTR topicOrSpecialParam, const char *regCode, const char *firstName,
							 const char *lastName, const char *regLevel, const char *specialParamLabel)
{
	// TIME TO BAIL: When there is no URL.

	if (theURL.IsEmpty())  return FALSE;

	unsigned long  queryFlags = action.flags;

	// NOTE: If not already a part of the supplied URL, add the "?" character

	if ( theURL.GetAt (theURL.GetLength() - 1) != _T('?') )  theURL += _T('?');

	CString  queryPart (_T(""));
	CString  info (_T(""));
#define AddInfo(field) if (info.GetLength() > 0) queryPart += (_T(field) + EscapeURL(info));

	// NOTE: As the first query component, "action" is not prefixed with an '&'.

	info = action.name;
	AddInfo("action=");

	// On the pay, update, and archived actions, we want to send paid mode registration info, if it
	// exists, regardless of the which mode we're in.
	SharewareModeType mode;
	if (((!strcmp(action.name, ACTION_PAY_STRING)) || (!strcmp(action.name, ACTION_UPDATE_STRING))
		|| (!strcmp(action.name, ACTION_ARCHIVED_STRING))) && QCSharewareManager::PaidModeRegCodeExists()) {
		mode = SWM_MODE_PRO;
	}
	else {
		mode = QCSharewareManager::GetModeFromIni();
	}

	if ( queryFlags & QUERY_PART_PRODUCT ) {		// Product Identifier
		info = CRString(IDS_PRODUCT_NAME);
		AddInfo("&product=");
	}

	if ( queryFlags & QUERY_PART_VERSION ) {		// Product Version
		info = EUDORA_BUILD_VERSION;
		AddInfo("&version=");
	}

	if ( queryFlags & QUERY_PART_DISTRIBUTOR_ID ) {	// Distributor ID
		info = GetIniString(IDS_INI_DISTRIBUTOR);
		AddInfo("&distributorID=");
	}

	if ( queryFlags & QUERY_PART_EUDORA_MODE ) {	// Eudora's Mode
		switch (GetSharewareMode()) {
			case SWM_MODE_ADWARE : info = _T("ad");
				break;
			case SWM_MODE_LIGHT  : info = _T("free");
				break;
			case SWM_MODE_PRO    : info = _T("paid");
				break;
			default              : info = _T("");
				ASSERT (false);
		}

		AddInfo("&mode=");
	}

 
	if ( queryFlags & QUERY_PART_EMAIL_ADDRESS ) {	// Return address from the user's Dominant personality
		info = GetReturnAddress();
		StripAddress(info.GetBuffer(info.GetLength()));
		AddInfo("&email=");
	}

	// NOTE: No point in even trying to get information that's only valid if the user is
	//		 registered for the current mode.
	// It might be possible to improve this by checking for name info, even if it's
	// not for the current mode.

	if (QCSharewareManager::IsRegisteredForMode(mode, NULL, FALSE, TRUE)) {
		if ( queryFlags & QUERY_PART_REG_CODE ) {		// The registration code for the required mode
			info = regCode ? regCode : QCSharewareManager::GetRegCodeForMode(mode);
			AddInfo("&regcode=");
		}

		if ( queryFlags & QUERY_PART_REG_LEVEL ) {		// The registration code for the required mode
			info = regLevel ? regLevel : QCSharewareManager::GetRegLevelForMode(mode);
			AddInfo("&reglevel=");
		}

		if ( queryFlags & QUERY_PART_REG_FIRST_NAME ) {	// The user's registered first name
			info = firstName ? firstName : QCSharewareManager::GetFirstNameForMode(mode);
			AddInfo("&regfirst=");
		}

		if ( queryFlags & QUERY_PART_REG_LAST_NAME ) {	// The user's registered last name
			info = lastName ? lastName : QCSharewareManager::GetLastNameForMode(mode);
			AddInfo("&reglast=");
		}
	}

	if ( queryFlags & QUERY_PART_PROFILE ) {			// The user's profile
		info = GetIniString(IDS_INI_PROFILE);
		AddInfo("&profile=");
	}

	if ( queryFlags & QUERY_PART_LANGUAGE ) {			// The localized language
		info = CRString(IDS_LOCALIZED_LANGUAGE);
		AddInfo("&la=");
	}

	if ( queryFlags & QUERY_PART_DESTINATION_URL ) {	// The URL which the user wishes to visit
		info = _T("");	// [INCOMPLETE:pje] A method for accessing the destination URL is unknown.
		AddInfo("&destination=");
	}

	if ( queryFlags & QUERY_PART_AD_ID ) {				// The ID of the clicked ad
		info = _T("");	// [INCOMPLETE:pje] A method for accessing the ad ID is unknown.
		AddInfo("&adid=");
	}

	if (topicOrSpecialParam) {
		info = topicOrSpecialParam;

		if (specialParamLabel) {
			// Do it directly rather than using the macro so that we always add it.
			// We always want to add the parameter because our current use of this
			// is for "&query=". When there's no value we want the jump server to
			// take us to a blank search engine page (e.g. Google, but with no text).
			// Unfortunately at the moment the jump server balks and does nothing when
			// there's no "&query=" parameter. So for now at least we provide "&query="
			// but without any value.
			queryPart += _T(specialParamLabel);
			queryPart += EscapeURL(info);
		}
		else {
			ASSERT (queryFlags == ACTION_SUPPORT.flags);

			AddInfo("&topic=");
		}
	}

	if ( queryFlags & QUERY_PART_PLATFORM ) { 		// Platform Identifier
		// Not using version for now as it adds extra length to
		// the already long URL, and is of dubious value to us
		CString DummyVersion;
		ReadPlatform(info, DummyVersion);
		info.Replace(' ', '_');
		AddInfo("&platform=");
	}

	if ( queryFlags & QUERY_PART_REAL_NAME ) {		// Real name from the user's Dominant personality
		info = GetIniString (IDS_INI_REAL_NAME);
		AddInfo("&realname=");
	}

	ASSERT( queryPart.Find ("action=" ) == 0);

	ASSERT (theURL.GetAt (theURL.GetLength() - 1) == _T('?'));

	theURL += queryPart;

	return (TRUE);
}

BOOL  LaunchURLWithQuery (LPCSTR szURL, const ActionInfo& action,
 						  // the arguments below are all optional
						  LPCSTR topicOrSpecialParam, const char *regCode, const char *firstName,
						  const char *lastName, const char *regLevel, const char *specialParamLabel)
{
	CString theURL(szURL);

	if (theURL.IsEmpty())
	{
		if (!GetJumpURL(&theURL))
		{
			// TIME TO BAIL: When unable to get the jump.cgi URL
			return FALSE;
		}
	}
	
	// TIME TO BAIL: When unable to construct the URL.

	if ( !ConstructURLWithQuery (theURL, action, topicOrSpecialParam, regCode, firstName, lastName, regLevel, specialParamLabel))
		return FALSE;

	ASSERT (theURL.IsEmpty() == FALSE);

	return (LaunchURLNoHistory (theURL, NULL));
}


// Get the jump.cgi URL.
// In expiring builds, it'll look in Eudora.ini before the resource.
BOOL	GetJumpURL(CString *theURL)
{
#if defined(DEBUG) || defined(EXPIRING)
	CString iniURL = GetIniString(IDS_INI_EUDORA_JUMP_URL);
	if (!iniURL.IsEmpty())
	{
		*theURL = iniURL;
		return TRUE;
	}
#endif
	*theURL = CRString(IDS_EUDORA_JUMP_URL);
	return (!theURL->IsEmpty());
}

BOOL	URLIsValid(
LPCSTR	szURL )
{
	URL_COMPONENTS	theComponents;
	char			szScheme[100];
	char			szHostName[300];
	char			szUserName[300];
	char			szPassword[300];
	char			szUrlPath[1024];
	char			szExtraInfo[1024];
	BOOL			bTemp;
	DWORD			dwError;

	memset( &theComponents, 0, sizeof( theComponents ) );

	theComponents.dwStructSize = sizeof( theComponents );
	theComponents.nScheme = INTERNET_SCHEME_DEFAULT;

    theComponents.lpszScheme = szScheme;
    theComponents.dwSchemeLength = DIM( szScheme ) - 1;
    theComponents.lpszHostName = szHostName;
    theComponents.dwHostNameLength = DIM( szHostName ) - 1;
    theComponents.lpszUserName = szUserName;
    theComponents.dwUserNameLength = DIM( szUserName ) - 1;
    theComponents.lpszPassword = szPassword;
    theComponents.dwPasswordLength = DIM( szPassword ) - 1;
    theComponents.lpszUrlPath = szUrlPath;
    theComponents.dwUrlPathLength = DIM( szUrlPath ) - 1;
    theComponents.lpszExtraInfo = szExtraInfo;
    theComponents.dwExtraInfoLength = DIM( szExtraInfo ) - 1;

	memset( theComponents.lpszScheme, 0, theComponents.dwSchemeLength + 1 );
	memset( theComponents.lpszHostName, 0, theComponents.dwHostNameLength + 1 );
	memset( theComponents.lpszUserName, 0, theComponents.dwUserNameLength + 1 );
	memset( theComponents.lpszPassword, 0, theComponents.dwPasswordLength + 1 );
	memset( theComponents.lpszUrlPath, 0, theComponents.dwUrlPathLength + 1 );
	memset( theComponents.lpszExtraInfo, 0, theComponents.dwExtraInfoLength + 1 );

	bTemp = InternetCrackUrl( szURL, strlen( szURL ), ICU_ESCAPE, &theComponents );

	if( !bTemp ||
		(	( szHostName[0] == '\0' ) && 
			( szUrlPath[0] == '\0' ) ) )
	{
		// just add the character as usual
		dwError = ::GetLastError();
		return FALSE;
	}

	return TRUE;
}


CString EscapeURL( LPCSTR szURL )
{
    char        szOut[INTERNET_MAX_URL_LENGTH];
    CString     szIn;
    DWORD       dwSize;
    DWORD       dwErr;
    UINT        flags;
        
    dwSize = INTERNET_MAX_URL_LENGTH;
    szOut[0] = '\0';

    if( ( szURL == NULL ) || ( szURL[0] == '\0' ) ) {
        return ( CString ) szOut;
    }

    flags = 0;

    if( !strnicmp( szURL, "file:", 5 ) ) {

        // BOG: For "file:" urls we only encode the spaces, but before doing
        // that, we escape out all % or %xx to prevent them from being
        // decoded when the url is launched.
        flags |= ICU_ENCODE_SPACES_ONLY;
        szIn = EscapePercentsInURL(szURL);

        if( !InternetCanonicalizeUrl( szIn, szOut, &dwSize, flags ) ) {
            dwErr = ::GetLastError();
        }

        return CString(szOut);
    }
    else {
        if( !strnicmp( szURL, "http:", 5 ) || !strnicmp( szURL, "https:", 6 ) ) {
            flags |= ICU_BROWSER_MODE;
        }

        //The InternetCanonicalizeURL function always encodes by default.
        //We should also decode first, so that the % encodings are not encoded again.
        //i.e. www.try%20this.com to www.try%2520this.com
        flags |= ICU_DECODE;

        //Before escaping, make sure we don't have %s by themselves because on Win95,
        //it overwrites the buffer when escaping it to %25. So lets escape all %s to %25
        //before giving it to the function
        szIn = EscapePercentsInURL(szURL);

        if( !InternetCanonicalizeUrl( szIn, szOut, &dwSize, flags ) ) {
            dwErr = ::GetLastError();
        }

        //Percents are decoded no matter how we give them, escape or unescaped
        //The InternetCanonicalize function returns false if it sees an unescaped % in the URL
        return (EscapePercentsInURL(szOut)) ;
    }
}


CString EscapePercentsInURL(LPCTSTR szIn)
{
    int len = strlen(szIn);
    const char *percent, *szLast;

    percent = szLast = szIn;

    if (len > INTERNET_MAX_URL_LENGTH)
        len = INTERNET_MAX_URL_LENGTH;

    CString sOut;
    char *szOut = sOut.GetBuffer(len * 3);	// Worst case: entire URL of %s that get expanded to %25
    szOut[0] = 0;

    while (percent && (percent = strchr(percent, '%')) ) {
        if (percent - szIn > len - 1) {
            // We must have got a incoming string that was longer than INTERNET_MAX_URL_LENGTH,
            // so just act like we didn't find any more % characters and break out of the loop
            ASSERT(strlen(szIn) > (unsigned)len);
            break;
        }

        // BOG: For http hrefs, not escaping percents followed by two digits is, I guess, an
        // ok thing to do, but for "file" hrefs we must---or InternetCanonicalizeUrl()
        // will hose them during decoding.

        if ( (percent - szIn <= len - 2) &&
             (!strnicmp( szIn, "file:", 5 ) || !isxdigit((unsigned char)percent[1]) || !isxdigit((unsigned char)percent[2])) ) {

            strncat(szOut, szLast, percent - szLast + 1);
            strcat(szOut, "25");
            szLast = ++percent;
        }
        else {
            if (percent - szIn <= len - 2)
                percent += 3;
            else {
                strncat(szOut, szLast, percent - szLast + 1);
                strcat(szOut, "25");
                szLast = ++percent;
            }
        }
    }

    strncat(szOut, szLast, len - (szLast - szIn));
    sOut.ReleaseBuffer();
    return szOut;
}


CString UnEscapeURL(LPCSTR szURL)
{
	char	szOut[INTERNET_MAX_URL_LENGTH];
	DWORD	dwSize;
	DWORD	dwErr;
	UINT	flags;
	CString	EncodedCRLF;
	
	dwSize = INTERNET_MAX_URL_LENGTH;
	szOut[0] = '\0';

	if( ( szURL == NULL ) || ( szURL[0] == '\0' ) )
	{
		return ( CString ) szOut;
	}

	flags = ICU_DECODE | ICU_NO_ENCODE;

	if( !strnicmp( szURL, "http:", 5 ) || !strnicmp( szURL, "https:", 6 ) )
	{
		flags |= ICU_BROWSER_MODE;
	}
	else if ( !strnicmp( szURL, "mailto:", 7 ) )
	{
		// This is too bogus.
		//
		// Trident will auto-decode % encodings of CR and LF to the literal
		// CR and LF characters, which will then get stripped when they go
		// through InternetCanonicalizeUrl().  Since it's impossible to have
		// literal CR and LF in a URL, we know the only way they got here was
		// by Trident's lameness, so let's just encode them so that they don't
		// get stripped out.
		EncodedCRLF.GetBuffer(strlen(szURL));
		for (const char* u = szURL; *u; u++)
		{
			if (*u == '\r')
				EncodedCRLF += "%0D";
			else if (*u == '\n')
				EncodedCRLF += "%0A";
			else
				EncodedCRLF += *u;
		}
		szURL = EncodedCRLF;
	}

	if( !InternetCanonicalizeUrl( szURL, szOut, &dwSize, flags ) )
	{
		dwErr = ::GetLastError();
	}

	return ( CString ) szOut;
}


////////////////////////////////////////////////////////////////////////
// FitTransparentBitmapToHeight [extern]
//
// Take a 16-color transparent bitmap (the first pixel determines the
// transparent color) and check to see if it would center nicely in the
// given target height.  If not, make it taller by reallocating and
// copying the bitmap bits.  This is used to center bitmap images
// next to "tall" menu items.
////////////////////////////////////////////////////////////////////////
BOOL FitTransparentBitmapToHeight(CBitmap& theBitmap, int nTargetHeight)
{
    // a little param checking
    if ( NULL == theBitmap.GetSafeHandle() ) {
		ASSERT(0);
		return FALSE;
    }

	// BOG: ok, since we should do this as fast as possible, we're gonna keep
	// the mfc to absolute minimum and do this the old fashioned way.

	// TODO: Error handling is MIA. The code is kinda ugly; could probably make
	// it a little more elegant. However, it doesn't leak anything and it works,
	// so I'm gonna check it in and take another pass at it later.


	HBITMAP hbmSrc = (HBITMAP) theBitmap;
	BITMAP bm;
	::GetObject( hbmSrc, sizeof(BITMAP), &bm );

    HDC hdcOffScreen = ::CreateCompatibleDC( NULL );
    assert( hdcOffScreen );

    HDC displayDC = ::GetDC( NULL );
	HBITMAP hbmTarget = ::CreateCompatibleBitmap( displayDC,
								bm.bmWidth,
								nTargetHeight );
    assert( hbmTarget );

	// get the src background color from 0,0
	HBITMAP hbmOld = (HBITMAP) ::SelectObject( hdcOffScreen, hbmSrc );
	COLORREF colorBk = ::GetPixel( hdcOffScreen, 0, 0 );
	HBRUSH brushBk = ::CreateSolidBrush( colorBk );
	
	// do the target bmp background
	::SelectObject( hdcOffScreen, hbmTarget );
	HBRUSH oldBrush = (HBRUSH) ::SelectObject( hdcOffScreen, brushBk );
	::PatBlt( hdcOffScreen,
		0, 0,
		bm.bmWidth,
		nTargetHeight,
		PATCOPY );

	::SelectObject( hdcOffScreen, oldBrush );
	::DeleteObject( brushBk );

	// do the target bmp foreground
    HDC hdcSrc = ::CreateCompatibleDC( NULL );
    assert( hdcSrc );
	HBITMAP hbmFart = (HBITMAP) ::SelectObject( hdcSrc, hbmSrc );
	int y = (nTargetHeight / 2) - (bm.bmHeight / 2);
	::BitBlt( hdcOffScreen,
		0, y,
		bm.bmWidth,
		bm.bmHeight,
		hdcSrc,
		0, 0,
		SRCCOPY );

	// now blow the old bitmap away and replace it
	::SelectObject( hdcSrc, hbmFart );
	theBitmap.DeleteObject();
	theBitmap.CreateCompatibleBitmap( CDC::FromHandle( displayDC ),
		bm.bmWidth,
		nTargetHeight );

	hbmFart = (HBITMAP) ::SelectObject( hdcSrc, theBitmap );

	::BitBlt( hdcSrc,
		0, 0,
		bm.bmWidth,
		nTargetHeight,
		hdcOffScreen,
		0, 0,
		SRCCOPY );

    // select out the bitmap & cleanup
    ::SelectObject( hdcOffScreen, hbmOld );
    ::DeleteDC( hdcOffScreen );
	::DeleteObject( hbmTarget );

	::SelectObject( hdcSrc, hbmFart );
	::DeleteDC( hdcSrc );
	::ReleaseDC( NULL, displayDC );

    return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CopyTextToClipboard [extern]
//
// Generic routine for copying text to the clipboard in CF_TEXT format.
////////////////////////////////////////////////////////////////////////
BOOL CopyTextToClipboard(const char* String, HWND hWndNewOwner)
{
	if (!String)
		return (FALSE);

	// Allocate memory block
	HGLOBAL h_mem = ::GlobalAlloc(GMEM_DDESHARE, strlen(String) + 1);
	if (NULL == h_mem)
		return (FALSE);

	// Copy the string to the memory block
	char* p_str = (char*)::GlobalLock(h_mem);
	if (NULL == p_str)
	{
		::GlobalFree(h_mem);
		return (FALSE);
	}
	
	strcpy(p_str, String);
	::GlobalUnlock(h_mem);
	p_str = NULL;

	// Copy the data to the clipboard
	BOOL status = FALSE;
	if ( ::OpenClipboard(hWndNewOwner) )
	{
		::EmptyClipboard();
		status = ::SetClipboardData(CF_TEXT, h_mem) ? TRUE : FALSE;
		::CloseClipboard();
	}

	if (!status)
		::GlobalFree(h_mem);
	
	return (status);
}

////////////////////////////////////////////////////////////////////////
// IsForegroundApp
//
// Return whether or not Eudora currently is the foreground app, but
// not if it is minimized.
////////////////////////////////////////////////////////////////////////
BOOL IsForegroundApp()
{
	HWND hWnd = ::GetForegroundWindow();
	DWORD dwProcessID;

	if (hWnd && GetWindowThreadProcessId(hWnd, &dwProcessID) &&
		GetCurrentProcessId() == dwProcessID &&
		AfxGetMainWnd()->IsIconic() == FALSE)
	{
		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// LeftClickAttachment
//
// Handles launch or drag-and-drop operation on an attachment.
//
////////////////////////////////////////////////////////////////////////
void LeftClickAttachment(CWnd* pWnd, CPoint pt, LPCTSTR Attach)
{
	BOOL bDragStarted = FALSE;
	BOOL bDoNormalLaunch = FALSE;
	BOOL bAllowAttachDrag = GetIniShort(IDS_INI_ALLOW_ATTACH_DRAG);

	pWnd->SetCapture();
	while (1)
	{
		if (CWnd::GetCapture() != pWnd)
			break;

		MSG msg;
		GetMessage(&msg, pWnd->GetSafeHwnd(), 0, 0);

		// Normal launch is left mouse down and up within restricted distance
		if (msg.message == WM_LBUTTONUP)
		{
			bDoNormalLaunch = TRUE;
			break;
		}
		else if (bAllowAttachDrag && msg.message == WM_MOUSEMOVE)
		{
			if (abs(pt.x - LOWORD(msg.lParam)) > 4 ||
				abs(pt.y - HIWORD(msg.lParam)) > 4)
			{
				bDragStarted = TRUE;
				break;
			}
		}
		// Cancel out if user hits the ESC key
		else if (msg.message == WM_KEYUP && msg.wParam == VK_ESCAPE)
			break;
	}
	ReleaseCapture();

	// Have to do these things after releasing the mouse capture
	if (bDoNormalLaunch)
		LaunchURL(Attach);
	else if (bDragStarted)
	{
		COleDataSource ods;
		
		if (AttachmentsToClipboard(Attach, TRUE, &ods))
			ods.DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_LINK, NULL, NULL);
	}
}

//
// ScoreMailbox()
//
// Scores the given mailbox name against the target name.
//
// Parameters
//	strMenu [in] - Name of the mailbox menu to score.
//	szName [in] - Target name to score against.
//
// Notes
//	This is essentially Steve's Mac scoring code and comments
//	shamelessly lifted and transplanted here with some minor
//	changes that do not affect the scoring.
//
int ScoreMailbox(CString strMenu, const char *szName)
{
	int		 iScore = 0;
	if (strMenu.CompareNoCase(szName) == 0)
	{
		return 0;
	}
	else
	{
		CString		strName = szName;
		strName.MakeLower();
		strMenu.MakeLower();
		int		 iPos = strMenu.Find(strName);
		if (iPos == -1)
		{
			iScore = -1;
		}
		else
		{
			// if we don't start at the start of the string, add 50
			if (iPos > 0)
			{
				iScore += 50;
				// if the character before us is a word character, add 20
				char		cBefore = strMenu.GetAt(iPos - 1);
				if (((cBefore >= 'A') && (cBefore <= 'Z')) ||
					((cBefore >= 'a') && (cBefore <= 'z')))
				{
					iScore += 20;
				}
			}
			// Now, check the end
			// if we don't end at the end of the string, add 50
			if ((iPos + strName.GetLength()) < strMenu.GetLength())
			{
				iScore += 50;
				// if the character after us is a word char, add 50
				char		cAfter = strMenu.GetAt(iPos + strName.GetLength());
				if (((cAfter >= 'A') && (cAfter <= 'Z')) ||
					((cAfter >= 'a') && (cAfter <= 'z')))
				{
					iScore += 20;
				}
			}
			// Finally, add points for any additional characters
			iScore += iPos;
		}
	}
	return iScore;
}

//
// GetMatchingCommands()
//
// Search the given menu's items for matches against the specified text.
//
// Paramters
//	poaMatches [in] - Array of CMailboxScore objects representing matching mailboxes.
//	pMenu [in] - Mailbox menu to search.
//	szName [in] - Name to match against.
//
void GetMatchingCommands(CObArray *poaMatches, CDynamicMailboxMenu *pMenu, const char *szName)
{
	int					 iItems = pMenu->GetMenuItemCount();
	CString				 strText;
	CDynamicMailboxMenu	*pSubMenu = NULL;
	unsigned int		 iScore = 0;
	int					 iPos = -1;
	CRString			 strNewMailbox(IDS_MAILBOX_NEW);

	if (!poaMatches || !pMenu || !szName)
	{
		return;
	}

	// Iterate over the items on the menu and drill down or score them as appropriate.
	for (int i = 0; i < iItems; ++i)
	{
		pSubMenu = (CDynamicMailboxMenu*)(pMenu->GetSubMenu(i));
		if (pSubMenu)
		{
			// This menu item has a sub-menu, check those for matches.
			if (!pSubMenu->HasBeenBuilt())
			{
				pSubMenu->OnInitMenuPopup(i, FALSE/*bSysMenu*/);
			}
			GetMatchingCommands(poaMatches, pSubMenu, szName);
		}
		else
		{
			// This menu item has no sub-menu, check it for a match.
			pMenu->GetMenuString(i, strText, MF_BYPOSITION);
			if (!strText.IsEmpty() && (strText.Compare(strNewMailbox) != 0))
			{
				// This is isn't empty and isn't the "New..." item: continue.
				// Strip off anything after the tab character.
				iPos = strText.Find('\t');
				if (iPos != -1)
				{
					strText = strText.Left(iPos);
				}
				// Remove any ampersands.
				strText.Remove('&');
				// Score this item.
				iScore = ScoreMailbox(strText, szName);
				if (iScore != -1)
				{
					QCCommandObject			*pCommand = NULL;
					COMMAND_ACTION_TYPE		 theAction;	
					g_theCommandStack.GetCommand((WORD)pMenu->GetMenuItemID(i), &pCommand, &theAction);

					// If we found a match create a CMailboxScore object and add it to the sorted list.
					if (pCommand)
					{
						ASSERT_KINDOF(QCMailboxCommand, pCommand);
						CMailboxScore	*pMailboxScore = DEBUG_NEW CMailboxScore((QCMailboxCommand*)pCommand, iScore);
						CMailboxScore	*pMailboxScoreTmp = NULL;
						int				 iIndex = -1;
						for (int i = 0; (i < poaMatches->GetSize()) && (iIndex == -1); ++i)
						{
							pMailboxScoreTmp = (CMailboxScore*)poaMatches->GetAt(i);
							if (pMailboxScoreTmp && (pMailboxScoreTmp->m_iScore > pMailboxScore->m_iScore))
							{
								iIndex = i;
							}
						}
						if (iIndex != -1)
						{
							poaMatches->InsertAt(iIndex, pMailboxScore);
						}
						else
						{
							poaMatches->Add(pMailboxScore);
						}
					}
				}
			}
		}
	}
}

//
// GenerateTransferToMenus()
//
// Search the mailboxes for matches to the specified string and generate
// "Transfer to" menus for the best matches.
//
// Parameters
//	pMenu [in] - Menu on which to place the newly generated "Transfer to" menus.
//	strText [in] - Text to match mailboxes against.
//	poaTransferMatches [out] - Array for holding objects representing matching mailboxes.
//
void GenerateTransferToMenus(CMenu* pMenu, CString &strText, CObArray *poaTransferMatches)
{
	if (!pMenu || !poaTransferMatches || strText.IsEmpty())
	{
		return;
	}

	int				 iMaxMenus = GetIniShort(IDS_INI_MAX_TRANSFER_TO_MENUS);
	if (iMaxMenus == 0)
	{
		// User doesn't want any "Transfer To" menus, remove the fixed menu.
		pMenu->RemoveMenu(ID_TRANSFER_TO_SELECTION, MF_BYCOMMAND);
	}
	else
	{
		// User wants at least one "Transfer To" menu so we need to generate the matches.
		CMailboxScore	*pMailboxScore = NULL;
		CMailboxScore	*pMailboxScoreLow = NULL;
		CString			 strMenuName;
		CRString		 strTransferToLabel(IDS_TRANSFER_TO_PREFIX);

		// Clear any previously stored matches.
		while (poaTransferMatches->GetSize() > 0)
		{
			pMailboxScore = (CMailboxScore*)poaTransferMatches->GetAt(0);
			poaTransferMatches->RemoveAt(0);
			delete pMailboxScore;
		}

		// Build the mailbox menu if needed.
		CMainFrame				*pMainFrame = CMainFrame::QCGetMainFrame();
		CDynamicMailboxMenu		*pMailboxMenu = pMainFrame->GetMailboxMenu();

		if (!pMailboxMenu->HasBeenBuilt())
		{
			pMailboxMenu->OnInitMenuPopup(MAIN_MENU_MAILBOX, FALSE/*bSysMenu*/);
		}

		// Generate matches for the current selection.
		GetMatchingCommands(poaTransferMatches, pMailboxMenu, strText);

		// Store a pointer to the best match.
		if (poaTransferMatches->GetSize() > 0)
		{
			pMailboxScoreLow = (CMailboxScore*)poaTransferMatches->GetAt(0);
		}

		// Bail if we found no matches at all.
		if (!pMailboxScoreLow || !pMailboxScoreLow->m_pCommand)
		{
			return;
		}

		// Determine how many menus to show: min of user setting and actual matches.
		if (poaTransferMatches->GetSize() < iMaxMenus)
		{
			iMaxMenus = poaTransferMatches->GetSize();
		}

		if (iMaxMenus > 0)
		{
			// Insert menu items for the specified number of matches.
			for (int i = 0; i < iMaxMenus; ++i)
			{
				pMailboxScore = (CMailboxScore*)poaTransferMatches->GetAt(i);
				if (pMailboxScore)
				{
					strMenuName = strTransferToLabel + pMailboxScore->m_pCommand->GetName();
					UINT uID = g_theCommandStack.AddCommand(pMailboxScore->m_pCommand, CA_TRANSFER_TO);
					pMenu->InsertMenu(ID_TRANSFER_TO_SELECTION, MF_BYCOMMAND, uID, strMenuName);
				}
			}

			// Remove the fixed "Transfer to" item.
			pMenu->RemoveMenu(ID_TRANSFER_TO_SELECTION, MF_BYCOMMAND);
		}
	}
}

//
//	TransferToSelection()
//
//	Try to find a mailbox whose name matches szName and transfer the current
//	message to that mailbox.  If a pView is specified and we retrieve an IMAP
//	TOC from it then start with that IMAP mailbox tree.
//
BOOL TransferToSelection(const char *szName, CView *pView)
{
	// If we're given the view, then attempt to get the TOC this message resides in.
	CTocDoc			*pTocDoc = NULL;

	if (pView)
	{
		CDocument		*pDoc = pView->GetDocument();
		
		if (pDoc)
		{
			if ( pDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)) )
			{
				CMessageDoc		*pMessageDoc = reinterpret_cast<CMessageDoc *>(pDoc);
				
				if (pMessageDoc->m_Sum)
					pTocDoc = pMessageDoc->m_Sum->m_TheToc;
			}
			else if (pDoc->IsKindOf(RUNTIME_CLASS(CTocDoc)))
			{
				pTocDoc = reinterpret_cast<CTocDoc *>(pDoc);
			}
		}
	}
	
	CImapAccount			*pAccountPreferred = NULL;
	QCMailboxCommand		*pMBCommand = NULL;

	// If the caller specified an IMAP TOC check there first.
	if (pTocDoc && pTocDoc->IsImapToc() && pTocDoc->m_pImapMailbox)
	{
		pAccountPreferred = g_ImapAccountMgr.FindAccount(pTocDoc->m_pImapMailbox->GetAccountID());
		if (pAccountPreferred)
		{
			pMBCommand = (QCMailboxCommand*)g_theMailboxDirector.ImapFindByImapNameInAccount(pAccountPreferred->GetAccountID(), 
																							 szName);
		}
	}

	// If still not found try the local mailboxes.
	if (!pMBCommand)
	{
		pMBCommand = g_theMailboxDirector.FindByNameInTree(NULL, szName);
	}

	// Lastly try the rest of the IMAP mailboxes.
	if (!pMBCommand)
	{
		LPSTR					 szPersonalityNameList;
		CString					 strPersona;
		CImapAccount			*pAccount = NULL;

		for (szPersonalityNameList = g_Personalities.List();
			 szPersonalityNameList && *szPersonalityNameList;
			 szPersonalityNameList += strlen(szPersonalityNameList) + 1)
		{
			strPersona = szPersonalityNameList;
			pAccount = g_ImapAccountMgr.FindAccount(g_Personalities.GetHash(strPersona));
			if (pAccount && (pAccount != pAccountPreferred))
			{
				pMBCommand = (QCMailboxCommand*)g_theMailboxDirector.ImapFindByImapNameInAccount(pAccount->GetAccountID(), 
																								 szName);
				if (pMBCommand)
				{
					break;
				}
			}
		}
	}
	
	// If we found a match do the transfer.
	if (pMBCommand)
	{
		WORD wCommandID = g_theCommandStack.FindOrAddCommandID(pMBCommand, CA_TRANSFER_TO);

		if (wCommandID)
		{
			AfxGetMainWnd()->PostMessage(WM_COMMAND, wCommandID);
			return TRUE;
		}
	}

	return FALSE;
}


void OpenFindMessagesWindow(
	const char *		szMailboxName,
	const char *		szSearchPhrase,
	bool				bSelectEvenIfPreviousSelection,
	bool				bSelectParentFolder,
	bool				bSelectAll,
	bool				bStartSearch)
{
	if (szSearchPhrase)
	{
		//	"Anywhere;Contains;TheSearchPhrase"
		CString		szSaveStrSeperator = CRString(IDS_SEARCH_SAVE_CHR_SEPARATOR).GetAt(0);
		CString		szSearchCriteria = CRString(IDS_SEARCH_CATEGORYSTR_ANYWHERE) +
									   szSaveStrSeperator +
									   CRString(IDS_SEARCH_COMPARESTR_CONTAINS) +
									   szSaveStrSeperator +
									   szSearchPhrase;
		
		SetIniShort(IDS_INI_SEARCH_CRITERIA_COUNT, 1);
		SetIniString(IDS_INI_SEARCH_CRITERIA_1, szSearchCriteria);
	}
	
	//	Check to see if we already have a SearchView
	CSearchView *	pSearchView = NULL;	
	bool			bSelectMailbox = true;
	
	if ( gSearchDoc && gSearchDoc->GetFirstViewPosition() )
	{
		POSITION		pos = gSearchDoc->GetFirstViewPosition();

		if (pos)
			pSearchView = (CSearchView *) gSearchDoc->GetNextView(pos);	
		
		bSelectMailbox = bSelectEvenIfPreviousSelection;
	}
	
	// Opening the Find Messages window can take a while
	CCursor WaitCursor;
	NewChildDocFrame(SearchTemplate);
	
	if (!bSelectMailbox)
		return;
	
	// Now the global doc should be avail
	ASSERT(gSearchDoc);
	if (!gSearchDoc)
		return;
	
	if (pSearchView)
	{
		//	It already existed - so we need to tell it to reload the search criteria
		pSearchView->PostMessage(msgFindMsgReloadCriteria, NULL);
	}
	else
	{
		//	Get the SearchView
		POSITION		pos = gSearchDoc->GetFirstViewPosition();

		if (pos)
			pSearchView = (CSearchView *) gSearchDoc->GetNextView(pos);	

		ASSERT(pSearchView);
		if (NULL == pSearchView)
			return;
	}
	
	//	Select the appropriate mbx
	//	Needs to be posted for the item to be scrolled in to view correctly
	if (szMailboxName && !bSelectAll)
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname(szMailboxName);
		ASSERT(pCommand);
		if (pCommand)
		{
			UINT	nMessage = bSelectParentFolder ? msgFindMsgParentFolderSel : msgFindMsgMaiboxSel;
			
			pSearchView->PostMessage( nMessage, reinterpret_cast<WPARAM>(pCommand) );
		}
	}

	if (bSelectAll)
		pSearchView->PostMessage(msgFindMsgAllMailboxesSel, NULL);

	if (bStartSearch)
	{
		//	Post a message that the begin search button was clicked
		pSearchView->PostMessage( WM_COMMAND, MAKEWPARAM(IDC_SEARCHWND_BEGIN_BTN, BN_CLICKED),
								  LPARAM(pSearchView->m_BeginBtn.GetSafeHwnd()) );
	}
}

BOOL SyncPlayMedia(LPCTSTR pFilename, BOOL bDisableMainWindow /*= FALSE*/)
{
	BOOL bNotCancelled = TRUE;

	HWND hMedia = AsyncPlayMedia(pFilename);
	if (hMedia)
	{
		CCursor* pWaitCursor = NULL;
		BOOL bMediaPlaying = TRUE;
		MSG msg;

		if (bDisableMainWindow)
		{
			pWaitCursor = new CCursor;
			AfxGetMainWnd()->EnableWindow(FALSE);
		}

		while (bMediaPlaying && GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			if (msg.message == WM_CHAR && msg.wParam == VK_ESCAPE)
			{
				bNotCancelled = FALSE;
				break;
			}
			if (msg.message == WM_USER_MCI_NOTIFY_MODE)
			{
				if (msg.lParam == MCI_MODE_STOP || msg.lParam == MCI_MODE_NOT_READY)
				{
					bMediaPlaying = FALSE;

					// Need to enable the main window before the stop message is dispatched
					// because that message will cause the media player window to go away,
					// and if the main window is still disabled when the player window disappears
					// then another app will get the focus
					if (bDisableMainWindow)
					{
						AfxGetMainWnd()->EnableWindow(TRUE);
						delete pWaitCursor;
					}
				}
			}
			DispatchMessage(&msg);
		}


		MCIWndDestroy(hMedia);
	}

	return bNotCancelled;
}

HWND AsyncPlayMedia(LPCTSTR pFilename, HWND hwndOwner /*= NULL*/)
{
	HWND hMedia = NULL;

	if (pFilename && *pFilename)
	{
		DWORD dwFlags = GetIniLong(IDS_INI_PLAY_MEDIA_FLAGS);
		const BOOL bIsShown = (dwFlags & WS_VISIBLE);

		hMedia = MCIWndCreate(hwndOwner? hwndOwner : *AfxGetMainWnd(),
								AfxGetInstanceHandle(),
								dwFlags,
								pFilename);
		if (hMedia)
		{
			RECT rect;

			if (bIsShown)
			{
				if (GetIniWindowPos(IDS_INI_PLAY_MEDIA_WINDOW_POS, rect) && (rect.left || rect.top))
					SetWindowPos(hMedia, NULL, rect.left, rect.top, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);
			}
			MCIWndPlayFrom(hMedia, 0);
		}
	}

	return hMedia;
}
