// ImGui.CPP
//
// Various interface utilities
//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


// #include <ctl3d.h>

#include <io.h>

#include "resource.h"
// #include "netstream.h"
// #include "imapmail.h"
#include "imGui.h"


// Alert Dialog extension
static BOOL g_bCheckForAutoOk = FALSE;
void CheckForAutoOk( BOOL bOn ) { g_bCheckForAutoOk = bOn; }


/////////////////////////////////////////////////////////////////////////////
// CAlertDialog dialog

class CAlertDialog : public CDialog
{
// Construction
public:
//	CAlertDialog(UINT DialogID, const char* Message, CWnd* pParent = NULL);

	CAlertDialog();

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
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};
/////////////////////////////////////////////////////////////////////////////
// CAlertDialog dialog


//CAlertDialog::CAlertDialog(UINT DialogID, const char* Message, CWnd* pParent /*=NULL*/)
//	: CDialog(DialogID, pParent)
CAlertDialog::CAlertDialog()
{
	//{{AFX_DATA_INIT(CAlertDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//m_Message = Message;

	m_Message = NULL;
}


BOOL CAlertDialog::OnInitDialog()
{
	char buf[1024];
	
#ifdef WIN32    
//	Ctl3dSubclassDlgEx(m_hWnd, CTL3D_ALL);
#endif
	
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
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CAlertDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlertDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAlertDialog, CDialog)
	//{{AFX_MSG_MAP(CAlertDialog)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlertDialog message handlers

BOOL CAlertDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// If we got a button press, then end the dialog
	if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED && GET_WM_COMMAND_ID(wParam, lParam) >= 1000)
	{
		EndDialog(GET_WM_COMMAND_ID(wParam, lParam));
		return (TRUE);
	}

	return (CDialog::OnCommand(wParam, lParam));
}


void CAlertDialog::OnDestroy()
{
	CDialog::OnDestroy();
}


// ================ External interface ===============//

int AlertDialog(UINT DialogID, ...)
{
	// Coddle compiler.
	if ( DialogID );

#if 0 // JOK

	HRSRC			hResInfo;
	HGLOBAL			hDlg;
	LPCDLGTEMPLATE  pDlgTemplate = NULL;
	HINSTANCE		hInst = (HINSTANCE)g_hModule;


	// Load the dialog template resource from the DLL's resources.
	if ((hResInfo = ::FindResource(hInst, (LPCSTR)DialogID, RT_DIALOG)) &&
		(hDlg = ::LoadResource(hInst, hResInfo)))
	{
		pDlgTemplate = (LPCDLGTEMPLATE) ::LockResource(hDlg);
		if (pDlgTemplate)
		{
			DWORD nResLen = ::SizeofResource(hInst, hResInfo);

			LPCDLGTEMPLATE pMemDlgTemplate = (LPCDLGTEMPLATE) malloc ( nResLen );
			if (pMemDlgTemplate)
				memcpy ( (void *) pMemDlgTemplate, (void *)pDlgTemplate, nResLen);

			// Can now free the original resource.
			::UnlockResource(hDlg);

			if (pMemDlgTemplate)
			{
				// Call the dialogs simple constructor.
				CAlertDialog dlg;

				if ( dlg.InitModalIndirect (pMemDlgTemplate, NULL) )
				{
					va_start(dlg.m_Args, DialogID);

					return dlg.DoModal();
				}
			}
		}
	}

	return IDCANCEL;

#endif 

#if 0 
	// Do this so we load our own resource.
//	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	// Call the dialogs simple constructor.
	CAlertDialog dlg(DialogID, NULL);
	
	va_start(dlg.m_Args, DialogID);

	return dlg.DoModal();
#endif

	return IDCANCEL;

}

#endif // IMAP4
