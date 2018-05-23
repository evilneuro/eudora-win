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
//  Description:	Macro dialog - Implementation
//  Created:		Sun Jan 28 1996
//

#include "stdafx.h"
#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL  
#ifdef WIN32

#include "SCutDlg.h"
#include "SCutKDlg.h"
#include "secres.h"

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
// SECShortcutDlg dialog


SECShortcutDlg::SECShortcutDlg(SECCommandList& commands, SECShortcutTable& shortcuts, CWnd* pParent /*=NULL*/)
	: CDialog(SECShortcutDlg::IDD, pParent),
	  m_Commands(commands),
	  m_Shortcuts(shortcuts)
{
	m_bDirty = FALSE;

	//{{AFX_DATA_INIT(SECShortcutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SECShortcutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SECShortcutDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SECShortcutDlg, CDialog)
	//{{AFX_MSG_MAP(SECShortcutDlg)
	ON_LBN_SELCHANGE(IDC_SHORTCUT_NAME_LIST, OnSelchangeShortcutNameList)
	ON_BN_CLICKED(IDC_SHORTCUT_REMOVE, OnShortcutRemove)
	ON_LBN_DBLCLK(IDC_SHORTCUT_NAME_LIST, OnShortcutCreate)
	ON_BN_CLICKED(IDC_SHORTCUT_CREATE, OnShortcutCreate)
	ON_BN_CLICKED(IDC_SHORTCUT_RESET, OnShortcutReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SECShortcutDlg message handlers


BOOL SECShortcutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_pctlNames = (CListBox *)GetDlgItem(IDC_SHORTCUT_NAME_LIST);
	for (int i=0; i<m_Commands.GetSize(); i++) {
		ASSERT(m_Commands[i].m_strName.GetLength() > 0);
		int index = m_pctlNames->AddString(m_Commands[i].m_strName);
		// We do not store "i" because the array contents
		// move around.
		m_pctlNames->SetItemData(index, m_Commands[i].m_nID);

	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void SECShortcutDlg::OnSelchangeShortcutNameList() 
{
	CStatic* ctlDesc = (CStatic*)GetDlgItem(IDC_SHORTCUT_DESCRIPTION);
	ASSERT_VALID(ctlDesc);
	int nCurSel = m_pctlNames->GetCurSel();
	if (nCurSel >= 0) {
		int nID = m_pctlNames->GetItemData(nCurSel);
		ASSERT(nID > 0 && nID < 0xffff);
		int i = m_Commands.Find(nID);
		ASSERT(i != -1);	// Not found is BAD!
		ctlDesc->SetWindowText(m_Commands[i].m_strDescription);
	}

	UpdateKeys();
}


void SECShortcutDlg::UpdateKeys()
{
	CStatic* ctlDesc = (CStatic*)GetDlgItem(IDC_SHORTCUT_DESCRIPTION);
	ASSERT_VALID(ctlDesc);
	int nCurSel = m_pctlNames->GetCurSel();
	if (nCurSel == LB_ERR)
		return;
	int nID = m_pctlNames->GetItemData(nCurSel);

	CListBox* ctlKeys = (CListBox*)GetDlgItem(IDC_SHORTCUT_KEY_LIST);
	ctlKeys->ResetContent();
	
	// Find all accelerators that refer to this cmd id.
	// Convert the accelerator to text and add it to the list box.
	CString str;
	for (int count=0; count<m_Shortcuts.GetSize(); count++) {
		DWORD hotkey;
		if ( m_Shortcuts[count].cmd == nID ) {
			AccelToHotKey( m_Shortcuts[count], &hotkey );
			::GetHotKeyDescription(hotkey, str);
			int index = ctlKeys->AddString(str);
			if (index == CB_ERR || index == CB_ERRSPACE)
				return;
			ctlKeys->SetItemData(index, count);
		}
	}
}


void SECShortcutDlg::OnShortcutRemove() 
{
	CListBox* pctlKeys = (CListBox*)GetDlgItem(IDC_SHORTCUT_KEY_LIST);
	int listIndex = pctlKeys->GetCurSel();

	if (listIndex == LB_ERR) {
		AfxMessageBox(IDS_SHORTCUT_SELECT_BEFORE_REMOVE, MB_ICONEXCLAMATION|MB_OK);
		return;
	}
	int shortcutIndex = pctlKeys->GetItemData(listIndex);
	m_Shortcuts.RemoveAt(shortcutIndex);
	UpdateKeys();

	// The user changed something.
	m_bDirty = TRUE;
}


void SECShortcutDlg::OnShortcutCreate() 
{
	int listIndex = m_pctlNames->GetCurSel();
	if (listIndex == LB_ERR) {
		AfxMessageBox(IDS_SHORTCUT_SELECT_BEFORE_CREATE, MB_ICONEXCLAMATION|MB_OK);
		m_pctlNames->SetFocus();
		return;
	}

	SECAssignKeyDlg dlg(m_Commands, m_Shortcuts, this);

	// Handle cancel by doing nothing.
	if (dlg.DoModal() != IDOK)
		return;

	DWORD nID = m_pctlNames->GetItemData(listIndex);
	if (nID == LB_ERR) {
		ASSERT(FALSE);
		return;
	}

	m_Shortcuts.AssignAccel(dlg.m_HotkeyValue, nID);

	// The user changed something.
	m_bDirty = TRUE;

	// Update the rest of the dialog.
	OnSelchangeShortcutNameList();
}

void SECShortcutDlg::OnShortcutReset() 
{
	if (AfxMessageBox(IDS_SHORTCUT_RESET_WARN, MB_OKCANCEL) == IDCANCEL)
		return;

	// Don't say I didn't warn you...
	m_Shortcuts.ResetAll();

	// Update the rest of the dialog.
	OnSelchangeShortcutNameList();
	UpdateKeys();

	// Remember that the user has made changes.
   	m_bDirty = TRUE;
}

#endif //WIN32
