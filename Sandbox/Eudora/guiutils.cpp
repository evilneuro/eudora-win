// GUIUTILS.CPP
//
// Various interface utilities
//


#include "stdafx.h"

#include <io.h>

#include <hes_api.h>

#include "resource.h"
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

#define DIM( a ) ( sizeof( a ) / sizeof( a[0] ) )

#ifdef IMAP4 // 
#include "ImapGuiUtils.h"
#endif // IMAP4


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

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
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
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

	ASSERT(m_Message || *buf);
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
// CWarnOptionsDialog dialog:  This differs from CWarnDialog because it 
// has two options rather than just OK.
/////////////////////////////////////////////////////////////////////////////
// CWarnDialog dialog

class CWarnOptionsDialog : public CHelpxDlg
{
// Construction
public:
	CWarnOptionsDialog(UINT IniID, const char* Message, UINT button1,
									   UINT button2, CWnd* pParent /*=NULL*/);


// Dialog Data
	//{{AFX_DATA(CWarnOptionsDialog)
	enum { IDD = IDD_WARN_OPTIONS };
	CStatic	m_AlertText;
	BOOL	m_NoMoreWarn;
	//}}AFX_DATA
	const char* m_Message;
	UINT m_IniID;
	UINT m_Button1, m_Button2;

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


CWarnOptionsDialog::CWarnOptionsDialog(UINT IniID, const char* Message, UINT button1,
									   UINT button2, CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CWarnOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWarnOptionsDialog)
	m_NoMoreWarn = FALSE;
	//}}AFX_DATA_INIT
	m_Message = Message;
	m_IniID = IniID;
	m_Button1 = button1;
	m_Button2 = button2;

}

BOOL CWarnOptionsDialog::OnInitDialog()
{
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

	PutDebugLog(DEBUG_MASK_DIALOG, m_Message);
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
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

int WarnOneOptionCancelDialog(UINT IniID, UINT StringID, UINT BtnTxtID, ...)
{
	ASSERT(::IsMainThreadMT());

	char buf[1024];
	CRString str(StringID);
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, BtnTxtID);
	_vsnprintf(buf, sizeof(buf) - 1, str, args);
	va_end(args);

	ASSERT(*buf != 0);
	
	if (IniID > 0)
	{
		ASSERT(GetIniShort(IniID) == TRUE);		// Switch better be turned off
	}

	CWarnOptionsDialog dlg(IniID, buf, 0, BtnTxtID, NULL);
	
	int Result = dlg.DoModal();
	
	// Code duplicated above in AlertDialog()
	char Dismissed[64];
	wsprintf(Dismissed, CRString(IDS_DEBUG_DISMISSED_ALERT), Result);
	PutDebugLog(DEBUG_MASK_DIALOG, Dismissed);
	// Code duplicated above in AlertDialog()

	return (Result);
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

	CWarnOptionsDialog dlg(IniID, buf, button1, button2, NULL);
	
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

	default: break;		// Unknown error value <nHesiodError>
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


BOOL IsProgram(LPCTSTR Filename)
{
	if (!Filename)
		return FALSE;

	LPCTSTR LastPeriod = strrchr(Filename, '.');
	
	if (LastPeriod)
	{
		CString ProgramExtensions(::GetIniString(IDS_INI_WARN_PROGRAM_EXTENSIONS));
		LPTSTR Extension = ProgramExtensions.GetBuffer(ProgramExtensions.GetLength());
		LPTSTR Separator;

		while (Separator = strchr(Extension, '|'))
		{
			*Separator = 0;
			if (strnicmp(LastPeriod + 1, Extension, strlen(Extension)) == 0)
				return TRUE;
			Extension = Separator + 1;
		}
	}

	return FALSE;
}

BOOL OpenFile(const char* Filename)
{
	ASSERT(::IsMainThreadMT());

	if (!Filename || !*Filename)
		return (FALSE);

	if (IsProgram(Filename))
	{
		if (GetIniShort(IDS_INI_WARN_LAUNCH_PROGRAM))
		{
			if (WarnOneOptionCancelDialog(0,IDS_WARN_LAUNCH_PROGRAM,IDS_WARN_LAUNCH_BTN,(LPCTSTR)Filename) == IDCANCEL)
			{
				// It's *VERY* important to return TRUE here.  We want it to look like we handled
				// launching the URL, and not let any default code run it anyway.
				return TRUE;
			}
		}
	}

#ifdef IMAP4
	// Go see if this is an IMAP attachment file and if we need to fetch it's 
	// contents. It will simply replace the contents of the file and let us continue. 
	if (!ImapGuiFetchAttachment (Filename))
	{
		return FALSE;
	}
		
#endif // IMAP4

	SHELLEXECUTEINFO sei;

	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.hwnd = AfxGetMainWnd()->GetSafeHwnd();
	sei.lpFile = Filename;
	sei.nShow = SW_SHOWNORMAL;

	return ShellExecuteEx(&sei);
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

	for (UINT i = IDS_INISN_SETTINGS + 1; i < 10799; i++)
	{
		TCHAR INIEntryName[256];
		if (QCLoadString(i, INIEntryName, sizeof(INIEntryName)))
		{
			int Len = strlen(EntryName);
			if (strnicmp(EntryName, INIEntryName, Len) == 0 &&
				(INIEntryName[Len] == 0 || INIEntryName[Len] == '\n'))
			{
				UINT SectionNameID;
				TCHAR SupposedSectionName[256];

				if (i < IDS_INISN_DEBUG)
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
				m_CurrentValue = GetIniString(m_ID);
				GetDefaultIniSetting(m_ID, m_DefaultValue);
				break;
			}
		}
	}

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
			SetIniString(m_ID, m_NewValue);
		else
			WriteEudoraProfileString(m_SectionName, m_EntryName, m_NewValue);
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


BOOL LaunchURL(
LPCSTR szURL ) 
{
	INT		i;
	CString	sz;

	// First, let's check out what the URL is, because we want to intercept a couple of them
	if (!strnicmp( szURL, "mailto:", 7))
	{
		CString to;
		CString cc;
		CString bcc;
		CString subject;
		CString body;
 
		sz = UnEscapeURL( szURL );

		if( CURLEdit::ParseMailtoArgs( sz, to, cc, bcc, subject, body))
		{
			//
			// Create Eudora composition window with as many fields
			// filled as possible, then display it.
			//
			CCompMessageDoc* pComp = NewCompDocument(to, NULL, subject, cc, bcc, NULL, body);
	
			if (pComp)
			{
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

		strncpy(UrlBuf, szURL + 16, sizeof(UrlBuf));
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
		return ::OpenFile( szURL );
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


	// Only do the shell URL execute under Win 95
	if (IsVersion4() &&
		((int)ShellExecute(AfxGetMainWnd()->m_hWnd, "open", szURL, NULL, NULL, SW_SHOWNORMAL)) > 32)
	{
		return ( TRUE );
	}


	while (1)
	{
		const char* URLHelper = GetIniString(IDS_INI_URL_HELPER);
		if (URLHelper && *URLHelper)
		{
			int SE = (int)ShellExecute(AfxGetMainWnd()->m_hWnd, NULL, URLHelper, szURL, NULL, SW_SHOWNORMAL);
			if (SE > 32)
				return (TRUE);

			// If the we got any error besides path/file doesn't exist, then we're done
			// 16-bit Win SDK doesn't define constants for return values of ShellExecute()
			if (SE != 2 && SE != 3)
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
    CString sOut;
    char *szOut = sOut.GetBuffer(INTERNET_MAX_URL_LENGTH);

    szOut[0] = '\0';
    int len = strlen(szIn);
    const char *percent, *szLast;

    percent = szLast = szIn;

    if (len > INTERNET_MAX_URL_LENGTH)
        len = INTERNET_MAX_URL_LENGTH;

    while (percent && (percent = strchr(percent, '%')) ) {
        if (percent - szIn > len - 1) {
            // We must have got a incoming string that was longer than INTERNET_MAX_URL_LENGTH,
            // so just act like we didn't find any more % characters and break out of the loop
            ASSERT(strlen(szIn) > len);
            break;
        }

        // BOG: For http hrefs, not escaping percents followed by two digits is, I guess, an
        // ok thing to do, but for "file" hrefs we must---or InternetCanonicalizeUrl()
        // will hose them during decoding.

        if ( (percent - szIn <= len - 2) &&
             (!strnicmp( szIn, "file:", 5 ) || !isxdigit(percent[1]) || !isxdigit(percent[2])) ) {

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


CString UnEscapeURL(
LPCSTR	szURL )
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
