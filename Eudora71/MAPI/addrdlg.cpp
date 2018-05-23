// addrdlg.cpp : implementation file
//

#include "stdafx.h"
//#include <afxwin.h>		// FORNOW, might be better to use precompiled AFX headers

#ifndef WIN32
typedef unsigned long ULONG;
#endif // WIN32

#include "mapires.h"
#include "addrdlg.h"
#include "reciplst.h"

#include "..\Eudora\ddeclien.h"

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CAddressDlg dialog


////////////////////////////////////////////////////////////////////////
// CAddressDlg
//
////////////////////////////////////////////////////////////////////////
CAddressDlg::CAddressDlg(
	const CString& dlgCaption,
	ULONG editFieldCode,
	CWnd* pParent /*=NULL*/)
	: CDialog(CAddressDlg::IDD, pParent),
	  m_DlgCaption(dlgCaption)
{
	//{{AFX_DATA_INIT(CAddressDlg)
	m_Bcc = "";
	m_Cc = "";
	m_To = "";
	//}}AFX_DATA_INIT

	//
	// Translate MAPI address UI code into mnemonic values.  See the
	// Microsoft documentation for the nEditFields parameter to 
	// MAPIAddress().
	//
	switch (editFieldCode)
	{
	case 0:
		m_EditFieldCode = EDIT_BROWSE_ONLY;
		break;
	case 1:
		m_EditFieldCode = EDIT_TO;
		break;
	case 2:
		m_EditFieldCode = EDIT_TO_CC;
		break;
	case -1:
	case 3:
	case 4:
		m_EditFieldCode = EDIT_TO_CC_BCC;
		break;
	default:
		ASSERT(0);
		m_EditFieldCode = EDIT_BROWSE_ONLY;
		break;
	}
}

////////////////////////////////////////////////////////////////////////
// DoDataExchange
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddressDlg)
	DDX_Text(pDX, IDC_BCC_EDIT, m_Bcc);
	DDX_Text(pDX, IDC_CC_EDIT, m_Cc);
	DDX_Text(pDX, IDC_TO_EDIT, m_To);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddressDlg, CDialog)
	//{{AFX_MSG_MAP(CAddressDlg)
	ON_BN_CLICKED(IDC_BCC_BUTTON, OnBccButton)
	ON_BN_CLICKED(IDC_CC_BUTTON, OnCcButton)
	ON_BN_CLICKED(IDC_TO_BUTTON, OnToButton)
	ON_LBN_SELCHANGE(IDC_NICKNAMES, SetControlStates)
	ON_LBN_DBLCLK(IDC_NICKNAMES, OnDblClickNicknames)
	ON_EN_UPDATE(IDC_BCC_EDIT, SetOKButtonState)
	ON_EN_UPDATE(IDC_CC_EDIT, SetOKButtonState)
	ON_EN_UPDATE(IDC_TO_EDIT, SetOKButtonState)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAddressDlg message handlers


////////////////////////////////////////////////////////////////////////
// OnOK
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::OnOK()
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}


////////////////////////////////////////////////////////////////////////
// OnInitDialog
//
////////////////////////////////////////////////////////////////////////
BOOL CAddressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//
	// Set dialog caption based on caller's preference.
	//
	SetWindowText(m_DlgCaption);

	//
	// Fetch a comma-separated list of nickname names from Eudora 
	// by establishing a temporary DDE conversation.  This will fail 
	// if Eudora is not running.  If successful, populate the nicknames
	// list box with a list of Eudora nickname names.
	//
	CDDEClient dde_client;
	CString nicknames;
	if (dde_client.GetEudoraNicknames(nicknames) &&
	    (! nicknames.IsEmpty()))
	{
		//
		// Fetch list box control.
		// 
		CListBox* p_listbox = (CListBox *) GetDlgItem(IDC_NICKNAMES);
		ASSERT(p_listbox != NULL);

		//
		// Tokenize the comma-delimited names into a list of
		// individual CString objects, then add them one by one to the
		// list box.
		//
		CRecipientList nickname_list;
		nickname_list.Tokenize(nicknames);
		while (! nickname_list.IsEmpty())
		{
			CString nickname = nickname_list.RemoveHead();
			p_listbox->AddString(nickname);
		}

		//
		// Highlight the first entry by default.
		//
		if (p_listbox->GetCount() > 0)
			p_listbox->SetSel(0, TRUE);
	}
	
	//
	// Set enable/disable states of controls, based on caller-selected
	// field editing mode and current nicknames selection.
	//
	SetControlStates();

	//
	// Force evaluation of the enable/disable state of the OK button.
	//
	SetOKButtonState();

	//
	// Set focus to the TO edit control.
	//
	CEdit* p_to_edit = (CEdit *) GetDlgItem(IDC_TO_EDIT);
	ASSERT(p_to_edit != NULL);
	p_to_edit->SetFocus();

	return FALSE;  // return TRUE  unless you set the focus to a control
}


////////////////////////////////////////////////////////////////////////
// SetOKButtonState
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::SetOKButtonState()
{
	//
	// Upon each change to the contents of any edit control (TO, CC,
	// or BCC), reevaluate whether or not to disable or enable the
	// OK button.
	//
	UpdateData(TRUE);		// transfer edit control contents to data members

	//
	// Fetch a pointer to the buttons.
	//
	CButton* p_ok = (CButton *) GetDlgItem(IDOK);
	ASSERT(p_ok != NULL);

	//
	// Special case ... if we're setup for browse only mode, then never
	// enable the OK button.
	//
	if (EDIT_BROWSE_ONLY == m_EditFieldCode)
	{
		p_ok->EnableWindow(FALSE);
		return;
	}

	//
	// If any edit control has non-white characters, then enable the
	// OK button.  Otherwise, disable the OK button.
	//
	m_To.TrimLeft();
	m_Cc.TrimLeft();
	m_Bcc.TrimLeft();
	p_ok->EnableWindow((! m_To.IsEmpty()) || (! m_Cc.IsEmpty()) || (! m_Bcc.IsEmpty()));
}


////////////////////////////////////////////////////////////////////////
// OnDblClickNicknames [protected]
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::OnDblClickNicknames()
{
	CButton* p_to_button = (CButton *) GetDlgItem(IDC_TO_BUTTON);
	ASSERT(p_to_button);

	if (p_to_button->IsWindowEnabled())
		OnToButton();
}


////////////////////////////////////////////////////////////////////////
// OnToButton
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::OnToButton()
{
	//
	// Transfer all selected items from the nicknames list box to the
	// "To" edit field.
	//
	CStringList selected_nicknames;
	int num_selected = GetSelectedNicknames(selected_nicknames);
	ASSERT(num_selected > 0);
	ExpandNicknames(selected_nicknames);

	//
	// Transfer edit control contents to data members.
	//
	UpdateData(TRUE);

	//
	// Append the selected nicknames to the "To" field.
	//
	while (! selected_nicknames.IsEmpty())
	{
		CString nickname = selected_nicknames.RemoveHead();

		if (! m_To.IsEmpty())
			m_To += ", ";
		m_To += nickname;
	}

	//
	// Then, transfer the updated data back to the edit controls.
	//
	UpdateData(FALSE);
	SetOKButtonState();
}


////////////////////////////////////////////////////////////////////////
// OnCcButton
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::OnCcButton()
{
	//
	// Transfer all selected items from the nicknames list box to the
	// "To" edit field.
	//
	CStringList selected_nicknames;
	int num_selected = GetSelectedNicknames(selected_nicknames);
	ASSERT(num_selected > 0);
	ExpandNicknames(selected_nicknames);

	//
	// Transfer edit control contents to data members.
	//
	UpdateData(TRUE);

	//
	// Append the selected nicknames to the "Cc" field.
	//
	while (! selected_nicknames.IsEmpty())
	{
		CString nickname = selected_nicknames.RemoveHead();

		if (! m_Cc.IsEmpty())
			m_Cc += ", ";
		m_Cc += nickname;
	}

	//
	// Then, transfer the updated data back to the edit controls.
	//
	UpdateData(FALSE);
	SetOKButtonState();
}


////////////////////////////////////////////////////////////////////////
// OnBccButton
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::OnBccButton()
{
	//
	// Transfer all selected items from the nicknames list box to the
	// "To" edit field.
	//
	CStringList selected_nicknames;
	int num_selected = GetSelectedNicknames(selected_nicknames);
	ASSERT(num_selected > 0);
	ExpandNicknames(selected_nicknames);

	//
	// Transfer edit control contents to data members.
	//
	UpdateData(TRUE);

	//
	// Append the selected nicknames to the "Bcc" field.
	//
	while (! selected_nicknames.IsEmpty())
	{
		CString nickname = selected_nicknames.RemoveHead();

		if (! m_Bcc.IsEmpty())
			m_Bcc += ", ";
		m_Bcc += nickname;
	}

	//
	// Then, transfer the updated data back to the edit controls.
	//
	UpdateData(FALSE);
	SetOKButtonState();
}


////////////////////////////////////////////////////////////////////////
// SetControlStates [protected]
//
// Evaluate whether to enable or disable the TO, CC, and BCC buttons
// and TO, CC, and BCC edit controls.  These decisions are based on
// the caller-settable edit mode (m_EditFieldCode) and the current
// nicknames selection.
//
////////////////////////////////////////////////////////////////////////
void CAddressDlg::SetControlStates(void)
{
	//
	// Fetch all applicable controls.
	//
	CEdit* p_to_edit = (CEdit *) GetDlgItem(IDC_TO_EDIT);
	CButton* p_to_button = (CButton *) GetDlgItem(IDC_TO_BUTTON);
	ASSERT(p_to_edit && p_to_button);
	CEdit* p_cc_edit = (CEdit *) GetDlgItem(IDC_CC_EDIT);
	CButton* p_cc_button = (CButton *) GetDlgItem(IDC_CC_BUTTON);
	ASSERT(p_cc_edit && p_cc_button);
	CEdit* p_bcc_edit = (CEdit *) GetDlgItem(IDC_BCC_EDIT);
	CButton* p_bcc_button = (CButton *) GetDlgItem(IDC_BCC_BUTTON);
	ASSERT(p_bcc_edit && p_bcc_button);
	CListBox* p_listbox = (CListBox *) GetDlgItem(IDC_NICKNAMES);
	ASSERT(p_listbox != NULL);

	BOOL has_selected_nicknames = (p_listbox->GetSelCount() > 0);

	//
	// Enable the edit controls as deemed necessary by the caller.  To
	// start, disable all controls then selectively enable them.
	//
	p_to_edit->EnableWindow(FALSE);
	p_to_button->EnableWindow(FALSE);
	p_cc_edit->EnableWindow(FALSE);
	p_cc_button->EnableWindow(FALSE);
	p_bcc_edit->EnableWindow(FALSE);
	p_bcc_button->EnableWindow(FALSE);

	switch (m_EditFieldCode)
	{
	case EDIT_TO_CC_BCC:
		p_bcc_edit->EnableWindow(TRUE);
		if (has_selected_nicknames)
			p_bcc_button->EnableWindow(TRUE);
		// fall through...
	case EDIT_TO_CC:
		p_cc_edit->EnableWindow(TRUE);
		if (has_selected_nicknames)
			p_cc_button->EnableWindow(TRUE);
		// fall through...
	case EDIT_TO:
		p_to_edit->EnableWindow(TRUE);
		if (has_selected_nicknames)
			p_to_button->EnableWindow(TRUE);
		break;
	case EDIT_BROWSE_ONLY:
		break;
	default:
		ASSERT(0);
		break;
	}
}

////////////////////////////////////////////////////////////////////////
// GetSelectedNicknames [private]
//
// Places a list of the selected strings in the caller-provided
// CStringList, returning the number of strings returned in the
// CStringList.
//
////////////////////////////////////////////////////////////////////////
int CAddressDlg::GetSelectedNicknames(CStringList& selectedNicknames)
{
	ASSERT(selectedNicknames.GetCount() == 0);

	CListBox* p_listbox = (CListBox *) GetDlgItem(IDC_NICKNAMES);
	ASSERT(p_listbox != NULL);

	const int NUMSELECTED = p_listbox->GetSelCount();
	ASSERT(NUMSELECTED >= 0);
	if (0 == NUMSELECTED)
		return 0;

	//
	// Allocate a temporary vector to hold the indices for the current
	// selection.
	//
	int* p_idx_array = new int[NUMSELECTED];
	if (NULL == p_idx_array)
		return 0;

	//
	// Fetch a vector of indices for the selected list box items.
	//
	if (p_listbox->GetSelItems(NUMSELECTED, p_idx_array) == LB_ERR)
	{
		delete [] p_idx_array;
		return 0;
	}
	
	for (int i = 0; i < NUMSELECTED; i++)
	{
		CString nickname;

		p_listbox->GetText(p_idx_array[i], nickname);
		ASSERT(! nickname.IsEmpty());
		selectedNicknames.AddTail(nickname);
	}

	delete [] p_idx_array;

	return NUMSELECTED;
	
}



////////////////////////////////////////////////////////////////////////
// ExpandNicknames [private]
//
// For each nickname entry in the caller-provided nickname list,
// expand each nickname into a comma-separated list of actual addresses.
//
////////////////////////////////////////////////////////////////////////
BOOL CAddressDlg::ExpandNicknames(CStringList& nicknameList)
{
	ASSERT(nicknameList.GetCount() > 0);

	//
	// Walk down the list of nicknames, expanding one nickname at
	// a time.
	//
	CDDEClient dde_client;
	POSITION pos = nicknameList.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION curpos = pos;		// save current position
		CString expanded_names;
		if (! dde_client.ExpandEudoraNickname(nicknameList.GetNext(pos), expanded_names))
			return FALSE;

		//
		// Eudora returns a comma-separated list of expanded names,
		// so just slam that back into the list at the current position.
		//
		nicknameList.SetAt(curpos, expanded_names);
	}

	return TRUE;
}



