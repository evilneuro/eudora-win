// NICKPAGE.CPP : implementation file
//
// Property page classes for the CNicknamesPropSheet property sheet.

#include "stdafx.h"
#include "resource.h"
#include "QCUtils.h"
#include "rescomm.h"
#include "urledit.h"	// for NICKPAGE.H
#include "nickpage.h"
#include "guiutils.h"
#include "doc.h"		// for NICKDOC.H
#include "nickdoc.h"
#include "font.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CNicknamesPropertyPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// CNicknamesPropertyPage base class

CNicknamesPropertyPage::CNicknamesPropertyPage(UINT nIDTemplate) : 
	CPropertyPage(nIDTemplate),
//FORNOW	m_EditControlHasFocus(FALSE),
	m_StaticHeight(0)
{
	// do nothing
}


////////////////////////////////////////////////////////////////////////
// WindowProc [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
LRESULT CNicknamesPropertyPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_COMMAND == message)
	{
		int control_id = GET_WM_COMMAND_ID(wParam, lParam);
		int notify_code = GET_WM_COMMAND_CMD(wParam, lParam);

		switch (notify_code)
		{
//FORNOW		case EN_SETFOCUS:
//FORNOW			if (IsEditControl(control_id))
//FORNOW			{
//FORNOW				TRACE1("CNicknamesPropertyPage::WindowProc: got EN_SETFOCUS(id=%d)\n", control_id);
//FORNOW				CWnd* p_wnd = GetDlgItem(control_id);
//FORNOW				ASSERT(p_wnd != NULL);
//FORNOW				if (::IsWindow(p_wnd->m_hWnd))
//FORNOW					m_EditControlHasFocus = TRUE;
//FORNOW			}
//FORNOW			break;
//FORNOW		case EN_KILLFOCUS:
//FORNOW			if (IsEditControl(control_id))
//FORNOW			{
//FORNOW				CURLEdit* p_edit = (CURLEdit *) GetDlgItem(control_id);
//FORNOW				ASSERT(p_edit != NULL);
//FORNOW				ASSERT(p_edit->IsKindOf(RUNTIME_CLASS(CURLEdit)));
//FORNOW				if (::IsWindow(p_edit->m_hWnd))
//FORNOW				{
//FORNOW					TRACE1("CNicknamesPropertyPage::WindowProc: got EN_KILLFOCUS(id=%d)\n", control_id);
//FORNOW					if (m_EditControlHasFocus && (! IsEditControlReadOnly(control_id)) && p_edit->GetModify())
//FORNOW					{
//FORNOW						if (g_Nicknames)
//FORNOW							g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_REQUESTING_NICKNAME, NULL);
//FORNOW					}
//FORNOW					m_EditControlHasFocus = FALSE;
//FORNOW				}
//FORNOW			}
//FORNOW			break;
		case EN_CHANGE:
			if (IsEditControl(control_id) && g_Nicknames /*FORNOW&& m_EditControlHasFocusFORNOW*/ && (! IsEditControlReadOnly(control_id)))
			{
				//
				// Hack, hack.  Kludge, kludge.  For some reason, the 32-bit rich edit control
				// is sending us an EN_CHANGE when you just click in the edit control to
				// set focus to the rich edit control WITHOUT actually changing the text.
				// Therefore, we need to check the dirty bit to see if something changed.
				//
				CURLEdit* p_edit = (CURLEdit *) GetDlgItem(control_id);
				ASSERT(p_edit != NULL);
				ASSERT(p_edit->IsKindOf(RUNTIME_CLASS(CURLEdit)));
				if ((CWnd::GetFocus() == p_edit) && p_edit->GetModify())
				{
					TRACE1("CNicknamesPropertyPage::WindowProc: got EN_CHANGE(id=%d)\n", control_id);
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_REQUESTING_NICKNAME, NULL);
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
				}
			}
			break;
		}
	}
	return CPropertyPage::WindowProc(message, wParam, lParam);
}


BEGIN_MESSAGE_MAP(CNicknamesPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesPropertyPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNicknamesAddressPage dialog

CNicknamesAddressPage::CNicknamesAddressPage() :
	CNicknamesPropertyPage(CNicknamesAddressPage::IDD)
{
	//{{AFX_DATA_INIT(CNicknamesAddressPage)
	//}}AFX_DATA_INIT
}

void CNicknamesAddressPage::DoDataExchange(CDataExchange* pDX)
{
	CNicknamesPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNicknamesAddressPage)
	//}}AFX_DATA_MAP
}

BOOL CNicknamesAddressPage::OnInitDialog()
{
	m_AddressEdit.SubclassEdit(IDC_NICKNAME_ADDRESS_ADDRESS, this);
	m_AddressEdit.SetFont(&ScreenFont, FALSE);
#ifndef WIN32
	m_AddressEdit.EatEscapeKeyPresses(TRUE);
#endif // !WIN32
	return CNicknamesPropertyPage::OnInitDialog();
}

////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesAddressPage::OnSize(UINT nType, int cx, int cy)
{
	CNicknamesPropertyPage::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! ::IsWindow(m_AddressEdit.m_hWnd)) || (SIZE_MINIMIZED == nType))
		return;

	CRect rect;
	GetClientRect(&rect);

	// Fetch control client rects
	CRect edit_rect;			// for Addresses edit box
	m_AddressEdit.GetWindowRect(&edit_rect);
	ScreenToClient(&edit_rect);

	// Set static height member to be the "native" height of the dialog, before resizing it
	if (0 == m_StaticHeight)
		m_StaticHeight = edit_rect.bottom;

	// Some pre-established values
	const int MARGIN = edit_rect.left - 1;
	
	edit_rect.right = rect.right - MARGIN;
	edit_rect.bottom = rect.bottom - MARGIN;
	m_AddressEdit.MoveWindow(&edit_rect, TRUE);
}


////////////////////////////////////////////////////////////////////////
// ClearAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesAddressPage::ClearAllControls()
{
	if (! ::IsWindow(m_AddressEdit.m_hWnd))
		return;
	m_AddressEdit.SetWindowText("");
}


////////////////////////////////////////////////////////////////////////
// EnableAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesAddressPage::EnableAllControls(BOOL bEnable)
{
	if (! ::IsWindow(m_AddressEdit.m_hWnd))
		return;
	m_AddressEdit.SetReadOnly(!bEnable);
}


////////////////////////////////////////////////////////////////////////
// SetEditFont [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesAddressPage::SetEditFont(CFont* pFont)
{
	if (! ::IsWindow(m_AddressEdit.m_hWnd))
		return;
	m_AddressEdit.SetFont(pFont, FALSE);
}


////////////////////////////////////////////////////////////////////////
// IsEditControl [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesAddressPage::IsEditControl(int nCtrlID) const
{
	return IDC_NICKNAME_ADDRESS_ADDRESS == nCtrlID;
}


////////////////////////////////////////////////////////////////////////
// IsEditControlReadOnly [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesAddressPage::IsEditControlReadOnly(int nCtrlID) const
{
	ASSERT(IsEditControl(nCtrlID));

	DWORD style = m_AddressEdit.GetStyle();
	if (style & ES_READONLY)
		return TRUE;

	return FALSE;
}


BEGIN_MESSAGE_MAP(CNicknamesAddressPage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesAddressPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNicknamesAddressPage message handlers


//--------------------------------------------------------------------//
//--------------------------------------------------------------------//
//                                                                    //
//              C N i c k n a m e s N o t e s P a g e                 //
//                                                                    //
//--------------------------------------------------------------------//
//--------------------------------------------------------------------//


/////////////////////////////////////////////////////////////////////////////
// CNicknamesNotesPage dialog

CNicknamesNotesPage::CNicknamesNotesPage() :
	CNicknamesPropertyPage(CNicknamesNotesPage::IDD)
{
	//{{AFX_DATA_INIT(CNicknamesNotesPage)
	//}}AFX_DATA_INIT
}

void CNicknamesNotesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNicknamesNotesPage)
	//}}AFX_DATA_MAP
}

BOOL CNicknamesNotesPage::OnInitDialog()
{
	m_NotesEdit.SubclassEdit(IDC_NICKNAME_NOTES_NOTES, this);
	m_NotesEdit.SetFont(&ScreenFont, FALSE);
#ifndef WIN32
	m_NotesEdit.EatEscapeKeyPresses(TRUE);
#endif // !WIN32
	return CNicknamesPropertyPage::OnInitDialog();
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesNotesPage::OnSize(UINT nType, int cx, int cy)
{
	CNicknamesPropertyPage::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! ::IsWindow(m_NotesEdit.m_hWnd)) || (SIZE_MINIMIZED == nType))
		return;

	CRect rect;
	GetClientRect(&rect);

	// Fetch control client rects
	CRect edit_rect;			// for Notes edit box
	m_NotesEdit.GetWindowRect(&edit_rect);
	ScreenToClient(&edit_rect);

	// Set static height member to be the "native" height of the dialog, before resizing it
	if (0 == m_StaticHeight)
		m_StaticHeight = edit_rect.bottom;

	// Some pre-established values
	const int MARGIN = edit_rect.left - 1;
	
	edit_rect.right = rect.right - MARGIN;
	edit_rect.bottom = rect.bottom - MARGIN;
	m_NotesEdit.MoveWindow(&edit_rect, TRUE);
}


////////////////////////////////////////////////////////////////////////
// ClearAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesNotesPage::ClearAllControls()
{
	if (! ::IsWindow(m_NotesEdit.m_hWnd))
		return;
	m_NotesEdit.SetWindowText("");
}


////////////////////////////////////////////////////////////////////////
// EnableAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesNotesPage::EnableAllControls(BOOL bEnable)
{
	if (! ::IsWindow(m_NotesEdit.m_hWnd))
		return;
	m_NotesEdit.SetReadOnly(!bEnable);
}


////////////////////////////////////////////////////////////////////////
// SetEditFont [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesNotesPage::SetEditFont(CFont* pFont)
{
	if (! ::IsWindow(m_NotesEdit.m_hWnd))
		return;
	m_NotesEdit.SetFont(pFont, FALSE);
}


////////////////////////////////////////////////////////////////////////
// IsEditControl [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesNotesPage::IsEditControl(int nCtrlID) const
{
	return IDC_NICKNAME_NOTES_NOTES == nCtrlID;
}


////////////////////////////////////////////////////////////////////////
// IsEditControlReadOnly [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesNotesPage::IsEditControlReadOnly(int nCtrlID) const
{
	ASSERT(IsEditControl(nCtrlID));

	DWORD style = m_NotesEdit.GetStyle();
	if (style & ES_READONLY)
		return TRUE;

	return FALSE;
}


BEGIN_MESSAGE_MAP(CNicknamesNotesPage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesNotesPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNicknamesNotesPage message handlers

//--------------------------------------------------------------------//
//--------------------------------------------------------------------//
//                                                                    //
//             C N i c k n a m e s C u s t o m P a g e                //
//                                                                    //
//--------------------------------------------------------------------//
//--------------------------------------------------------------------//

/////////////////////////////////////////////////////////////////////////////
// CNicknamesCustomPage dialog

CNicknamesCustomPage::CNicknamesCustomPage(int nDlgTemplateID) :
	CNicknamesPropertyPage(nDlgTemplateID),
	m_NumFieldsInUse(0)
{
	//{{AFX_DATA_INIT(CNicknamesCustomPage)
	//}}AFX_DATA_INIT
}


void CNicknamesCustomPage::DoDataExchange(CDataExchange* pDX)
{
	CNicknamesPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomFormView)
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL0, m_EditLabelArray[0]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD0, m_FieldNameArray[0]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL1, m_EditLabelArray[1]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD1, m_FieldNameArray[1]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL2, m_EditLabelArray[2]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD2, m_FieldNameArray[2]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL3, m_EditLabelArray[3]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD3, m_FieldNameArray[3]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL4, m_EditLabelArray[4]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD4, m_FieldNameArray[4]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL5, m_EditLabelArray[5]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD5, m_FieldNameArray[5]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL6, m_EditLabelArray[6]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD6, m_FieldNameArray[6]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL7, m_EditLabelArray[7]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD7, m_FieldNameArray[7]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL8, m_EditLabelArray[8]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD8, m_FieldNameArray[8]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL9, m_EditLabelArray[9]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD9, m_FieldNameArray[9]);
	//}}AFX_DATA_MAP
}


BOOL CNicknamesCustomPage::OnInitDialog()
{
	m_EditArray[0].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT0, this);
	m_EditArray[1].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT1, this);
	m_EditArray[2].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT2, this);
	m_EditArray[3].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT3, this);
	m_EditArray[4].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT4, this);
	m_EditArray[5].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT5, this);
	m_EditArray[6].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT6, this);
	m_EditArray[7].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT7, this);
	m_EditArray[8].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT8, this);
	m_EditArray[9].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT9, this);

	for (int i = 0; i < 10; i++)
	{
		m_EditArray[i].SetFont(&ScreenFont, FALSE);
#ifndef WIN32
		m_EditArray[i].EatEscapeKeyPresses(TRUE);
#endif // !WIN32
	}

	return CNicknamesPropertyPage::OnInitDialog();
}


////////////////////////////////////////////////////////////////////////
// InitPage [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesCustomPage::InitPage()
{
	//
	// Do one-time initialization to determine how many fields are
	// actually enabled in the dialog template and what the field
	// names are.
	//
	if (! ::IsWindow(m_EditArray[0].m_hWnd))
	{
		ASSERT(0);
		return FALSE;
	}
	else if (m_NumFieldsInUse != 0)
	{
		ASSERT(0);		// caller blew it already initialized once
		return TRUE;	
	}

	for (int i = 0; i < NUM_FIELDS; i++)
	{
		if (m_EditArray[i].IsWindowEnabled())
		{
			m_NumFieldsInUse = i + 1;

			// Make sure there is a corresponding field name
			CString fieldname;
			m_FieldNameArray[i].GetWindowText(fieldname);
			ASSERT(! fieldname.IsEmpty());
		}
		else
			break;
	}

	if (0 == m_NumFieldsInUse)
	{
		ASSERT(0);		// something's wrong...
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// RegisterFields [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::RegisterFields()
{
	ASSERT(m_NumFieldsInUse > 0);

	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		ASSERT(::IsWindow(m_FieldNameArray[i].m_hWnd));

		CString fieldname;
		m_FieldNameArray[i].GetWindowText(fieldname);

		if (g_Nicknames)
		{
			//
			// Register the field name with the nicknames database.
			//
			g_Nicknames->RegisterNicknameFieldName(fieldname);

			//
			// Extract the label from the dialog template and then
			// process it to remove any colon, ampersand, and newline
			// characters.  This is what we'll use to customize the
			// "View By" dropdown list on the LHS view.
			//
			CString viewbyitem;
			ASSERT(::IsWindow(m_EditLabelArray[i].m_hWnd));
			m_EditLabelArray[i].GetWindowText(viewbyitem);
			int idx = -1;
			while ((idx = viewbyitem.FindOneOf("&:\n")) != -1)
			{
				viewbyitem = viewbyitem.Left(idx) + viewbyitem.Right(viewbyitem.GetLength() - idx - 1);
			}

			//
			// Send the "View By" dropdown list box item and the
			// corresponding nickname field name as a pair to the LHS
			// view of the nicknames window.
			//
			CStringList viewbyitem_and_fieldname;
			viewbyitem_and_fieldname.AddTail(viewbyitem);
			viewbyitem_and_fieldname.AddTail(fieldname);

			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_REGISTER_VIEWBY_ITEM, &viewbyitem_and_fieldname);
		}
		else
		{
			ASSERT(0);
		}
	}
}


////////////////////////////////////////////////////////////////////////
// PopulateEditControls [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::PopulateEditControls(CNickname* pNickname)
{
	ASSERT(pNickname != NULL);
	ASSERT(m_NumFieldsInUse > 0);

	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		ASSERT(::IsWindow(m_FieldNameArray[i].m_hWnd));

		CString fieldname;
		CString fieldvalue;
		m_FieldNameArray[i].GetWindowText(fieldname);
		if (pNickname->GetNotesFieldValue(fieldname, fieldvalue))
			m_EditArray[i].SetWindowText(fieldvalue);
		m_EditArray[i].SetModify(FALSE);
	}
}


////////////////////////////////////////////////////////////////////////
// SaveModifiedEditControls [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesCustomPage::SaveModifiedEditControls(CNickname* pNickname)
{
	ASSERT(pNickname != NULL);
	ASSERT(m_NumFieldsInUse > 0);

	BOOL found_modified = FALSE;

	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		if (m_EditArray[i].GetModify())
		{
			ASSERT(::IsWindow(m_FieldNameArray[i].m_hWnd));

			CString fieldname;
			m_FieldNameArray[i].GetWindowText(fieldname);

			CString newvalue;
			m_EditArray[i].GetWindowText(newvalue);
			pNickname->SetNotesFieldValue(fieldname, newvalue);
			m_EditArray[i].SetModify(FALSE);

			found_modified = TRUE;
		}
	}

	return found_modified;
}


////////////////////////////////////////////////////////////////////////
// ClearAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::ClearAllControls()
{
	if (! ::IsWindow(m_EditArray[0].m_hWnd))
		return;

	ASSERT(m_NumFieldsInUse > 0);
	for (int i = 0; i < m_NumFieldsInUse; i++)
		m_EditArray[i].SetWindowText("");
}


////////////////////////////////////////////////////////////////////////
// EnableAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::EnableAllControls(BOOL bEnable)
{
	if (! ::IsWindow(m_EditArray[0].m_hWnd))
		return;

	ASSERT(m_NumFieldsInUse > 0);
	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		m_EditLabelArray[i].EnableWindow(bEnable);
		m_EditArray[i].SetReadOnly(!bEnable);
	}
}


////////////////////////////////////////////////////////////////////////
// SetEditFont [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::SetEditFont(CFont* pFont)
{
	if (! ::IsWindow(m_EditArray[0].m_hWnd))
		return;

	ASSERT(m_NumFieldsInUse > 0);
	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		m_EditArray[i].SetFont(pFont, FALSE);
	}
}


////////////////////////////////////////////////////////////////////////
// IsEditControl [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesCustomPage::IsEditControl(int nCtrlID) const
{
	switch (nCtrlID)
	{
	case IDC_NICKNAME_CUSTOM_EDIT0:
	case IDC_NICKNAME_CUSTOM_EDIT1:
	case IDC_NICKNAME_CUSTOM_EDIT2:
	case IDC_NICKNAME_CUSTOM_EDIT3:
	case IDC_NICKNAME_CUSTOM_EDIT4:
	case IDC_NICKNAME_CUSTOM_EDIT5:
	case IDC_NICKNAME_CUSTOM_EDIT6:
	case IDC_NICKNAME_CUSTOM_EDIT7:
	case IDC_NICKNAME_CUSTOM_EDIT8:
	case IDC_NICKNAME_CUSTOM_EDIT9:
		return TRUE;
	default:
		return FALSE;
	}
}


////////////////////////////////////////////////////////////////////////
// IsEditControlReadOnly [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesCustomPage::IsEditControlReadOnly(int nCtrlID) const
{
	DWORD style = 0;

	switch (nCtrlID)
	{
	case IDC_NICKNAME_CUSTOM_EDIT0:  style = m_EditArray[0].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT1:  style = m_EditArray[1].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT2:  style = m_EditArray[2].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT3:  style = m_EditArray[3].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT4:  style = m_EditArray[4].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT5:  style = m_EditArray[5].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT6:  style = m_EditArray[6].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT7:  style = m_EditArray[7].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT8:  style = m_EditArray[8].GetStyle(); break;
	case IDC_NICKNAME_CUSTOM_EDIT9:  style = m_EditArray[9].GetStyle(); break;
	default:
		ASSERT(0);
		return FALSE;
	}

	if (style & ES_READONLY)
		return TRUE;

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// ComputeStaticHeight [private]
//
////////////////////////////////////////////////////////////////////////
int CNicknamesCustomPage::ComputeStaticHeight() const
{
	//
	// At the time of this call, the controls should be created,
	// but we haven't yet queried the dialog template to see
	// how many of the custom controls are enabled.
	//
	ASSERT(0 == m_NumFieldsInUse);
	if (! ::IsWindow(m_EditArray[0].m_hWnd))
	{
		ASSERT(0);
		return 0;
	}

	int max_height = 0;
	for (int i = 0; i < NUM_FIELDS; i++)
	{
		if (m_EditArray[i].IsWindowEnabled())
		{
			CRect edit_rect;			// for edit boxes
			m_EditArray[i].GetWindowRect(&edit_rect);
			ScreenToClient(&edit_rect);
			max_height = __max(max_height, edit_rect.bottom);
		}
	}

	return max_height;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::OnSize(UINT nType, int cx, int cy)
{
	CNicknamesPropertyPage::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! ::IsWindow(m_EditArray[0].m_hWnd)) || 
		(SIZE_MINIMIZED == nType))
	{
		return;
	}
		
	if (0 == m_NumFieldsInUse)
	{
		if (0 == m_StaticHeight)
			m_StaticHeight = ComputeStaticHeight();
		return;
	}

	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int MARGIN = LOWORD(DBU) / 2 + 1;

	//
	// Resize widths of all visible edit controls, leaving
	// heights alone.
	//
	ASSERT(m_NumFieldsInUse > 0);
	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		//
		// First, resize the width of the edit control to fit
		// in the client area.
		//
		CRect edit_rect;			// for edit boxes
		m_EditArray[i].GetWindowRect(&edit_rect);
		ScreenToClient(&edit_rect);
		edit_rect.right = rect.right - MARGIN;
		m_EditArray[i].MoveWindow(&edit_rect, TRUE);

		//
		// Now, check to see if the resized control fits 
		// *completely* in the client rect of the window.  
		// If not, then make it invisible (along with its
		// matching edit label) to avoid any ugly clipping.
		//
		CRect intersect_rect;
		if (intersect_rect.IntersectRect(rect, edit_rect) && (intersect_rect == edit_rect))
		{
			m_EditArray[i].ShowWindow(SW_SHOW);
			m_EditLabelArray[i].ShowWindow(SW_SHOW);
		}
		else
		{
			m_EditArray[i].ShowWindow(SW_HIDE);
			m_EditLabelArray[i].ShowWindow(SW_HIDE);
		}
	}
}

BEGIN_MESSAGE_MAP(CNicknamesCustomPage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesCustomPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
