// 
// Stingray Software Extension Classes
// Copyright (C) 1996 Jim Beveridge
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:			Jim Beveridge
//  Description:	Hotkey control dialog - Implementation
//  Created:		Sun Jan 28 1996
//

#include "stdafx.h"

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL  
#ifdef WIN32

//AAB #include "SCutKDlg.h"
#include "afxcview.h"
#include "secall.h"

#ifdef _SECDLL
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif //_SECDLL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SECAssignKeyDlg dialog


SECAssignKeyDlg::SECAssignKeyDlg(
		SECCommandList& commands,
		SECShortcutTable& shortcuts,
		CWnd* pParent /* = NULL */)
	: CDialog(SECAssignKeyDlg::IDD, pParent),
		m_Shortcuts(shortcuts),
		m_Commands(commands)
{
	//{{AFX_DATA_INIT(SECAssignKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SECAssignKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SECAssignKeyDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate) {
		CHotKeyCtrl* pctlHotkey = (CHotKeyCtrl*)GetDlgItem(IDC_SHORTCUT_HOTKEY);
		m_HotkeyValue = pctlHotkey->GetHotKey();
	}
}


BEGIN_MESSAGE_MAP(SECAssignKeyDlg, CDialog)
	//{{AFX_MSG_MAP(SECAssignKeyDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// SECAssignKeyDlg overrides

BOOL SECAssignKeyDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
   UINT nID = LOWORD(wParam);
   int nCode = HIWORD(wParam);

	if (nID == IDC_SHORTCUT_HOTKEY && nCode == EN_CHANGE)
		UpdateKeyDescription();
	
	return CDialog::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
// SECAssignKeyDlg message handlers


void SECAssignKeyDlg::UpdateKeyDescription()
{
	CStatic* pctlTitle = (CStatic*)GetDlgItem(IDC_SHORTCUT_ASSIGNMENT_TITLE);
	CStatic* pctlText = (CStatic*)GetDlgItem(IDC_SHORTCUT_ASSIGNMENT_TEXT);
	CButton* pctlOk = (CButton*)GetDlgItem(IDOK);
	CHotKeyCtrl* pctlHotkey = (CHotKeyCtrl*)GetDlgItem(IDC_SHORTCUT_HOTKEY);
	DWORD hotkey = pctlHotkey->GetHotKey();

	ACCEL newAccel;
	HotKeyToAccel(hotkey, &newAccel);

	if (newAccel.key == VK_PAUSE || newAccel.key == VK_NUMLOCK || newAccel.key == VK_SCROLL)
	{
		pctlHotkey->SetHotKey(0,0);
		newAccel.key = 0;
	}

	if ( !newAccel.key) {
		pctlTitle->ShowWindow(SW_HIDE);
		pctlText->ShowWindow(SW_HIDE);
		pctlOk->EnableWindow(FALSE);
		return;
	}

	pctlTitle->ShowWindow(SW_SHOW);
	pctlText->ShowWindow(SW_SHOW);
	pctlOk->EnableWindow(TRUE);

	int size = m_Shortcuts.GetSize();
	for (int i = 0;  i < size; i++) {
		// We always include the standard flags because MFC uses
		// them by default and that is all we generate internally.
		ACCEL accel = m_Shortcuts[i];
		if ( ( accel.fVirt | kStandardAcceleratorFlags ) == newAccel.fVirt
					&& accel.key == newAccel.key) {
			int index = m_Commands.Find(accel.cmd);
			if (index == -1)
				pctlText->SetWindowText( _T("(No description available)") );
			else
				pctlText->SetWindowText( m_Commands[index].m_strName );
			break;
		}
	}
	if ( i == size ) {
		CString strUnassigned;
		strUnassigned.LoadString(IDS_SHORTCUT_UNASSIGNED);
		pctlText->SetWindowText( strUnassigned );
	}
}

void SECAssignKeyDlg::OnOK()
{
	CHotKeyCtrl* pctlHotkey = (CHotKeyCtrl*)GetDlgItem(IDC_SHORTCUT_HOTKEY);
	if (pctlHotkey) {
		DWORD hotkey = pctlHotkey->GetHotKey();
		HotKeyToAccel(hotkey, &m_Accel);
		m_Accel.fVirt &= ~kStandardAcceleratorFlags;

		// Some keys should not be reassigned because Windows
		// becomes distinctly unhappy about it.
		if ( 
			// Disallow F1 completely
			m_Accel.key == VK_F1
			// Disallow F4 or F6 with modifiers
			|| (m_Accel.fVirt &&
			(m_Accel.key == VK_F4
			|| m_Accel.key == VK_F6) ) )
		{
			AfxMessageBox( IDS_SHORTCUT_PERMANENT, MB_ICONEXCLAMATION | MB_OK );
			return;
		}
	}
	CDialog::OnOK();
}

BOOL SECAssignKeyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CHotKeyCtrl* pctlHotkey = (CHotKeyCtrl*)GetDlgItem(IDC_SHORTCUT_HOTKEY);
	pctlHotkey->SetRules(m_Commands.m_wInvalidComb, m_Commands.m_wModifiers);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#endif //WIN32