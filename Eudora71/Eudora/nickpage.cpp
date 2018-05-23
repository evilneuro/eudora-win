// NICKPAGE.CPP : implementation file
//
// Property page classes for the CNicknamesPropSheet property sheet.
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "QCUtils.h"
#include "rescomm.h"
#include "urledit.h"	// for NICKPAGE.H
#include "nickpage.h"
#include "guiutils.h"
#include "doc.h"		// for NICKDOC.H
#include "nickdoc.h"
#include "font.h"
#include "rs.h"
#include "NicknamesWazooWnd.h"

/*************************************************************
Mapping of numbered fields - 1/23/01 - sagar.
(useful for debugging - subject to change)

HOME TAB:
0-address
1-city
2-state
3-country
4-zip
5-phone
6-fax
7-mobile
8-web


WORK TAB:
0-title
1-company
2-address2
3-city2
4-state2
5-country2
6-zip2
7-phone2
8-fax2
9-mobil2
10-web2
***************************************************************/








#include "DebugNewHelpers.h"


BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	::EnableWindow(hwnd, BOOL(lParam));
		return TRUE;
}



IMPLEMENT_DYNAMIC(CNicknamesPropertyPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// CNicknamesPropertyPage base class

CNicknamesPropertyPage::CNicknamesPropertyPage(UINT nIDTemplate) : 
	CPropertyPage(nIDTemplate),
//FORNOW	m_EditControlHasFocus(FALSE),
	m_StaticHeight(0)
{
	m_rectOldFrame.left = 0;
	m_rectOldFrame.top = 0;
	m_rectOldFrame.right = 0;
	m_rectOldFrame.bottom = 0;
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
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


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

BOOL CNicknamesNotesPage::OnInitDialog()
{
	m_NotesEdit.SubclassEdit(IDC_NICKNAME_NOTES_NOTES, this);
	m_NotesEdit.SetFont(&ScreenFont, FALSE);
#ifndef WIN32
	m_NotesEdit.EatEscapeKeyPresses(TRUE);
#endif // !WIN32
	return CNicknamesPropertyPage::OnInitDialog();
}

void CNicknamesNotesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNicknamesNotesPage)
	//}}AFX_DATA_MAP
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
	::EnumChildWindows(this->m_hWnd, EnumChildProc, LPARAM(bEnable));
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
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_LABEL10, m_EditLabelArray[10]);
	DDX_Control(pDX, IDC_NICKNAME_CUSTOM_FIELD10, m_FieldNameArray[10]);
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
	m_EditArray[10].SubclassEdit(IDC_NICKNAME_CUSTOM_EDIT10, this);

	for (int i = 0; i < 11; i++)
	{
		m_EditArray[i].SetFont(&ScreenFont, FALSE);
#ifndef WIN32
		m_EditArray[i].EatEscapeKeyPresses(TRUE);
#endif // !WIN32
	}

	return CNicknamesPropertyPage::OnInitDialog();
}


////////////////////////////////////////////////////////////////////////
// InitPage [public, virtual]
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
// NumNickNameFields [public, virtual]
//
// Default behavior is to return m_NumFieldsInUse.  Subclasses might
// return a different number if fields are used but not shown.
////////////////////////////////////////////////////////////////////////
int CNicknamesCustomPage::NumNickNameFields()
{
	return m_NumFieldsInUse;
}


////////////////////////////////////////////////////////////////////////
// RegisterFields [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::RegisterFields()
{
	ASSERT(m_NumFieldsInUse > 0);

	for (int i = 0; i < NumNickNameFields(); i++)
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
			if (fieldname == NICKPAGE_EDIT_ADDRESSES) //dont add this entry to the "view by"
				continue;					//Its OK for the string to be hardcoded because its not a 
											//language dependent resource

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
			if(fieldname.GetAt(fieldname.GetLength()-1) == '2') //this entry is for work
				viewbyitem = CRString(IDS_ADDRBK_WORK) + " " + viewbyitem;

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
// GetEditControlByLabel [public]
//
// Return the edit control associated with the specified label.
////////////////////////////////////////////////////////////////////////
CURLEdit *CNicknamesCustomPage::GetEditControlByLabel(const char *szLabel)
{
	ASSERT(m_NumFieldsInUse > 0);

	for (int i = 0; i < NumNickNameFields(); i++)
	{
		ASSERT(::IsWindow(m_FieldNameArray[i].m_hWnd));

		CString tmpfieldname;
		m_FieldNameArray[i].GetWindowText(tmpfieldname);
		if (tmpfieldname == szLabel)
		{
			return &(m_EditArray[i]);
		}
	}
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// PopulateEditControls [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::PopulateEditControls(CNickname* pNickname)
{
	ASSERT(pNickname != NULL);
	ASSERT(m_NumFieldsInUse > 0);

	for (int i = 0; i < NumNickNameFields(); i++)
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

	for (int i = 0; i < NumNickNameFields(); i++)
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
	{
		return;
	}

	ASSERT(NumNickNameFields() > 0);
	for (int i = 0; i < NumNickNameFields(); i++)
	{
		m_EditArray[i].SetWindowText("");
	}
}


////////////////////////////////////////////////////////////////////////
// EnableAllControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::EnableAllControls(BOOL bEnable)
{
	::EnumChildWindows(this->m_hWnd, EnumChildProc, LPARAM(bEnable));
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
	case IDC_NICKNAME_CUSTOM_EDIT10:
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
	case IDC_NICKNAME_CUSTOM_EDIT10:  style = m_EditArray[10].GetStyle(); break;
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

	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int MARGIN = LOWORD(DBU) / 2 + 1;

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if ((! ::IsWindow(m_EditArray[0].m_hWnd)) || 
		(nType == SIZE_MINIMIZED))
	{
		if (m_rectOldFrame.right == 0)
		{
			m_rectOldFrame.right = 395;
			m_rectOldFrame.bottom = 485;
		}
		return;
	}
		
	if (m_NumFieldsInUse == 0)
	{
		if (m_StaticHeight == 0)
		{
			m_StaticHeight = ComputeStaticHeight();
		}
		return;
	}

	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	int			deltaX = 0;

	if (m_rectOldFrame.right != 0)
	{
		deltaX = (rect.right - rect.left) - (m_rectOldFrame.right - m_rectOldFrame.left);
	}

	GetClientRect(&m_rectOldFrame);

	//
	// Resize widths of all visible edit controls, leaving
	// heights alone.
	//
	ASSERT(m_NumFieldsInUse > 0);
	for (int i = 0; i < m_NumFieldsInUse; i++)
	{
		SizeNumberedField(i, rect, MARGIN, deltaX);
	}
	SizeSpecials(rect, MARGIN, deltaX);
}

////////////////////////////////////////////////////////////////////////
// SizeNumberedField [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX)
{
}

////////////////////////////////////////////////////////////////////////
// SizeSpecials [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::SizeSpecials(CRect rect, const int MARGIN, int deltaX)
{
}

////////////////////////////////////////////////////////////////////////
// SizeSpecialField [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesCustomPage::SizeSpecialField(int fieldNum, CRect rect,
										    const int MARGIN, int deltaX,
										    int leftFactor, int rightFactor)
{
	//
	// First, resize the width of the edit control to fit
	// in the client area.
	//
	CWnd		*pWnd = GetDlgItem(fieldNum);
	CRect		 rectWnd;
	if (pWnd)
	{
		pWnd->GetWindowRect(&rectWnd);
		ScreenToClient(&rectWnd);

		// Move the field by the specified factor.
		if (leftFactor > 0)
		{
			rectWnd.left += (deltaX / leftFactor);
		}
		if (rightFactor > 0)
		{
			rectWnd.right += (deltaX / rightFactor);
		}

		pWnd->MoveWindow(&rectWnd, TRUE);

		//
		// Now, check to see if the resized control fits 
		// *completely* in the client rect of the window.  
		// If not, then make it invisible (along with its
		// matching edit label) to avoid any ugly clipping.
		//
	//	CRect intersect_rect;
	//	if (intersect_rect.IntersectRect(rect, rectWnd) && (intersect_rect == rectWnd))
	//	{
			pWnd->ShowWindow(SW_SHOW);
	//	}
	//	else
	//	{
		//	pWnd->ShowWindow(SW_HIDE);
	//	}
	}
}

BEGIN_MESSAGE_MAP(CNicknamesCustomPage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesCustomPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNicknamesPersonalPage dialog

CNicknamesPersonalPage::CNicknamesPersonalPage() :
	CNicknamesCustomPage(CNicknamesPersonalPage::IDD)
{
	//{{AFX_DATA_INIT(CNicknamesPersonalPage)
	//}}AFX_DATA_INIT
}

BOOL CNicknamesPersonalPage::OnInitDialog()
{
	BOOL		bReturn = CNicknamesCustomPage::OnInitDialog();
	m_Swap.AttachButton(IDC_SWAPNAMES, SECBitmapButton::Al_Center, IDB_SWAP, this);

	InitPage();

	return bReturn;
}

////////////////////////////////////////////////////////////////////////
// SizeSpecials [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesPersonalPage::SizeSpecials(CRect rect, const int MARGIN, int deltaX)
{
	// Locate the Swap button appropriately.
	SizeSpecialField(IDC_SWAPNAMES, rect, MARGIN, deltaX, 2, 2);
}

////////////////////////////////////////////////////////////////////////
// SizeNumberedField [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesPersonalPage::SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX)
{
	//
	// First, resize the width of the edit control to fit
	// in the client area.
	//
	CRect edit_rect;			// for edit boxes
	m_EditArray[fieldNum].GetWindowRect(&edit_rect);
	ScreenToClient(&edit_rect);

	CRect label_rect;			// for edit boxes
	m_EditLabelArray[fieldNum].GetWindowRect(&label_rect);
	ScreenToClient(&label_rect);

	if (fieldNum == 0)
	{
		// Move the edit by half of the delta.
		edit_rect.left += (deltaX / 2);
		edit_rect.right += (deltaX / 2);
		// Move the label by half the delta.
		label_rect.left += (deltaX / 2);
		label_rect.right += (deltaX / 2);
	}
	else if (fieldNum == 3)
	{
		// Leave the left edge where it is,
		// keep the right edge snapped to the right boundary.
		edit_rect.right = rect.right - MARGIN;
		edit_rect.bottom = rect.bottom - MARGIN;

	}
	else if (fieldNum == 1)
	{
		// Leave the left edge where it is,
		// move the right edge in by half of the delta.
		edit_rect.right += (deltaX / 2);
	}
	else if (fieldNum == 2)
	{
		// Move the label by half the delta.
		label_rect.left += (deltaX / 2);
		label_rect.right = rect.right - MARGIN;
		// Move the left edit edge by half the delta,
		// keep the right edge snapped to the right boudary.
		edit_rect.left += (deltaX / 2);
		edit_rect.right = rect.right - MARGIN;
	}
	
	m_EditArray[fieldNum].MoveWindow(&edit_rect, TRUE);
	m_EditLabelArray[fieldNum].MoveWindow(&label_rect, TRUE);

	//
	// Now, check to see if the resized control fits 
	// *completely* in the client rect of the window.  
	// If not, then make it invisible (along with its
	// matching edit label) to avoid any ugly clipping.
	//
	//CRect intersect_rect;
//	if (intersect_rect.IntersectRect(rect, edit_rect) && (intersect_rect == edit_rect))
//	{
		m_EditArray[fieldNum].ShowWindow(SW_SHOW);
		m_EditLabelArray[fieldNum].ShowWindow(SW_SHOW);
//	}
//	else
//	{
	//	m_EditArray[fieldNum].ShowWindow(SW_HIDE);
	//	m_EditLabelArray[fieldNum].ShowWindow(SW_HIDE);
//	}
}

////////////////////////////////////////////////////////////////////////
// OnCommand [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesPersonalPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam == IDC_SWAPNAMES))
	{
		CString		firstname;
		CString		lastname;
		m_EditArray[1].GetWindowText(firstname);
		m_EditArray[2].GetWindowText(lastname);
		m_EditArray[1].SetWindowText(lastname);
		m_EditArray[2].SetWindowText(firstname);
		m_EditArray[1].SetModify(TRUE);
		m_EditArray[2].SetModify(TRUE);
		return TRUE;
	}
	return CNicknamesPropertyPage::OnCommand(wParam, lParam);
}

BEGIN_MESSAGE_MAP(CNicknamesPersonalPage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesPersonalPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void SwapHomeAndWork(CNicknamesHomePage *homePage, CNicknamesWorkPage *workPage)
{
	if (homePage && workPage)
	{
		for (int i = 0; i < homePage->m_NumFieldsInUse; ++i)
		{
			CString		homelabel;
			CString		worklabel;
			CURLEdit	*workEdit = NULL;
			homePage->m_FieldNameArray[i].GetWindowText(homelabel);
			worklabel = homelabel + "2";
			workEdit = workPage->GetEditControlByLabel(worklabel);
			if (workEdit)
			{
				CString		homeValue;
				CString		workValue;
				workEdit->GetWindowText(workValue);
				homePage->m_EditArray[i].GetWindowText(homeValue);
				workEdit->SetWindowText(homeValue);
				workEdit->SetModify(TRUE);
				homePage->m_EditArray[i].SetWindowText(workValue);
				homePage->m_EditArray[i].SetModify(TRUE);
			}
		}
		for (i = 0; i < homePage->m_NumFieldsInUse; ++i)
		{
			if(homePage->m_EditArray[i].GetModify())
			{
				g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
				break;
			}
		}
	}

}


/////////////////////////////////////////////////////////////////////////////
// CNicknamesHomePage dialog

CNicknamesHomePage::CNicknamesHomePage() :
	CNicknamesCustomPage(CNicknamesHomePage::IDD)
{
	//{{AFX_DATA_INIT(CNicknamesHomePage)
	//}}AFX_DATA_INIT
}

BOOL CNicknamesHomePage::OnInitDialog()
{
	BOOL		bReturn = CNicknamesCustomPage::OnInitDialog();

	InitPage();

	return bReturn;
}

////////////////////////////////////////////////////////////////////////
// NumNickNameFields [public, virtual]
//
// Return m_NumFieldsInUse + 1 because a text field is devoted to the
// "Primary Information" checkbox but isn't shown.
////////////////////////////////////////////////////////////////////////
int CNicknamesHomePage::NumNickNameFields()
{
	return m_NumFieldsInUse + 1;
}

////////////////////////////////////////////////////////////////////////
// PopulateEditControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesHomePage::PopulateEditControls(CNickname* pNickname)
{
	CNicknamesCustomPage::PopulateEditControls(pNickname);

	BOOL			bDidCheck = FALSE;

	CButton		*pButton = reinterpret_cast<CButton *>(GetDlgItem(IDC_PRIMARYCHECK));
	if (pButton)
	{
		CString		primary;
		m_EditArray[9].GetWindowText(primary);
		if (primary.Compare("home") == 0)
		{
			pButton->SetCheck(BST_CHECKED);
			bDidCheck = TRUE;
		}
		if (!bDidCheck)
		{
			pButton->SetCheck(BST_UNCHECKED);
		}
	}
}

////////////////////////////////////////////////////////////////////////
// SizeSpecials [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesHomePage::SizeSpecials(CRect rect, const int MARGIN, int deltaX)
{
	SizeSpecialField(IDC_PRIMARYCHECK, rect, MARGIN, deltaX, 1, 1);
	SizeSpecialField(IDC_LINE1, rect, MARGIN, deltaX, 0, 1);
}

////////////////////////////////////////////////////////////////////////
// SizeNumberedField [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesHomePage::SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX)
{
	//
	// First, resize the width of the edit control to fit
	// in the client area.
	//
	CRect edit_rect;			// for edit boxes
	m_EditArray[fieldNum].GetWindowRect(&edit_rect);
	ScreenToClient(&edit_rect);

	CRect label_rect;			// for edit boxes
	m_EditLabelArray[fieldNum].GetWindowRect(&label_rect);
	ScreenToClient(&label_rect);

	if ((fieldNum == 0) || (fieldNum == 8))
	{
		// Leave the left edge where it is,
		// keep the right edge snapped to the right boundary.
		edit_rect.right = rect.right - MARGIN;
	}
	else if ((fieldNum == 1) || (fieldNum == 3) || (fieldNum == 5) || (fieldNum == 7))
	{
		// Leave the left edge where it is,
		// move the right edge in by half of the delta.
		edit_rect.right += (deltaX / 2);
	}
	else if ((fieldNum == 2) || (fieldNum == 4) || (fieldNum == 6))
	{
		// Move the label by half the delta.
		label_rect.left += (deltaX / 2);
		label_rect.right += (deltaX / 2);
		// Move the left edit edge by half the delta,
		// keep the right edge snapped to the right boudary.
		edit_rect.right = rect.right - MARGIN;
		edit_rect.left += (deltaX/2);
		/*See table at the beggining of file mapping numbered fields*/
	}
	
	m_EditArray[fieldNum].MoveWindow(&edit_rect, TRUE);
	m_EditLabelArray[fieldNum].MoveWindow(&label_rect, TRUE);

	//
	// Now, check to see if the resized control fits 
	// *completely* in the client rect of the window.  
	// If not, then make it invisible (along with its
	// matching edit label) to avoid any ugly clipping.
	//
//	CRect intersect_rect;
//	if (intersect_rect.IntersectRect(rect, edit_rect) && (intersect_rect == edit_rect))
//	{
		m_EditArray[fieldNum].ShowWindow(SW_SHOW);
		m_EditLabelArray[fieldNum].ShowWindow(SW_SHOW);
//	}
//	else
//	{
	//	m_EditArray[fieldNum].ShowWindow(SW_HIDE);
//		m_EditLabelArray[fieldNum].ShowWindow(SW_HIDE);
//	}
}

////////////////////////////////////////////////////////////////////////
// OnCommand [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesHomePage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam == IDC_SWAPHOMEWITHWORK))
	{
		SwapHomeAndWork(this, workPage);
		return TRUE;
	}
	else if (LOWORD(wParam == IDC_PRIMARYCHECK))
	{
		CButton		*pButton = reinterpret_cast<CButton *>(GetDlgItem(IDC_PRIMARYCHECK));
		CButton		*pOtherButton = NULL;
		if (pButton)
		{
			if (workPage)
			{
				pOtherButton = reinterpret_cast<CButton *>(workPage->GetDlgItem(IDC_PRIMARYCHECK));
			}
			if (pButton->GetCheck())
			{
				if (pOtherButton)
				{
					pOtherButton->SetCheck(BST_UNCHECKED);
				}
				m_EditArray[9].SetWindowText("home");
				m_EditArray[9].SetModify(TRUE);
			}
			else
			{
				if (pOtherButton)
				{
					pOtherButton->SetCheck(BST_CHECKED);
				}
				m_EditArray[9].SetWindowText("work");
				m_EditArray[9].SetModify(TRUE);
			}
			if(m_EditArray[9].GetModify())
			{
				g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_REQUESTING_NICKNAME, NULL);
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
			}

		}
		return TRUE;
	}
	return CNicknamesPropertyPage::OnCommand(wParam, lParam);
}

BEGIN_MESSAGE_MAP(CNicknamesHomePage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesHomePage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNicknamesWorkPage dialog

CNicknamesWorkPage::CNicknamesWorkPage() :
	CNicknamesCustomPage(CNicknamesWorkPage::IDD)
{
	//{{AFX_DATA_INIT(CNicknamesWorkPage)
	//}}AFX_DATA_INIT
}

BOOL CNicknamesWorkPage::OnInitDialog()
{
	BOOL		bReturn = CNicknamesCustomPage::OnInitDialog();

	InitPage();

	return bReturn;
}

////////////////////////////////////////////////////////////////////////
// PopulateEditControls [public, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWorkPage::PopulateEditControls(CNickname* pNickname)
{
	CNicknamesCustomPage::PopulateEditControls(pNickname);

	BOOL			bDidCheck = FALSE;

	CButton		*pButton = reinterpret_cast<CButton *>(GetDlgItem(IDC_PRIMARYCHECK));
	if (pButton)
	{
		if (homePage)
		{
			CString		primary;
			homePage->m_EditArray[9].GetWindowText(primary);
			if (primary.Compare("work") == 0)
			{
				pButton->SetCheck(BST_CHECKED);
				bDidCheck = TRUE;
			}
		}
		if (!bDidCheck)
		{
			pButton->SetCheck(BST_UNCHECKED);
		}
	}
}

////////////////////////////////////////////////////////////////////////
// SizeSpecials [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWorkPage::SizeSpecials(CRect rect, const int MARGIN, int deltaX)
{
	SizeSpecialField(IDC_PRIMARYCHECK, rect, MARGIN, deltaX, 1, 1);
	SizeSpecialField(IDC_LINE1, rect, MARGIN, deltaX, 0, 1);
	SizeSpecialField(IDC_LINE2, rect, MARGIN, deltaX, 0, 1);
}

////////////////////////////////////////////////////////////////////////
// SizeNumberedField [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesWorkPage::SizeNumberedField(int fieldNum, CRect rect, const int MARGIN, int deltaX)
{
	//
	// First, resize the width of the edit control to fit
	// in the client area.
	//
	CRect edit_rect;			// for edit boxes
	m_EditArray[fieldNum].GetWindowRect(&edit_rect);
	ScreenToClient(&edit_rect);

	CRect label_rect;			// for edit boxes
	m_EditLabelArray[fieldNum].GetWindowRect(&label_rect);
	ScreenToClient(&label_rect);

	if ((fieldNum == 2) || (fieldNum == 10))
	{
		// Leave the left edge where it is,
		// keep the right edge snapped to the right boundary.
		edit_rect.right = rect.right - MARGIN;
	}
	else if ((fieldNum == 0) || (fieldNum == 3) || (fieldNum == 5) ||
			 (fieldNum == 7) || (fieldNum == 9))
	{
		// Leave the left edge where it is,
		// move the right edge in by half of the delta.
		edit_rect.right += (deltaX / 2);
	}
	else if ((fieldNum == 1) || (fieldNum == 4) || (fieldNum == 6) ||
			 (fieldNum == 8))
	{
		// Move the label by half the delta.
		label_rect.left += (deltaX / 2);
		label_rect.right += (deltaX / 2);
		// Move the left edit edge by half the delta,
		// keep the right edge snapped to the right boudary.
		edit_rect.left += (deltaX / 2);
		edit_rect.right = rect.right - MARGIN;
	}
	
	m_EditArray[fieldNum].MoveWindow(&edit_rect, TRUE);
	m_EditLabelArray[fieldNum].MoveWindow(&label_rect, TRUE);

	//
	// Now, check to see if the resized control fits 
	// *completely* in the client rect of the window.  
	// If not, then make it invisible (along with its
	// matching edit label) to avoid any ugly clipping.
	//
//	CRect intersect_rect;
//	if (intersect_rect.IntersectRect(rect, edit_rect) && (intersect_rect == edit_rect))
//	{
		m_EditArray[fieldNum].ShowWindow(SW_SHOW);
		m_EditLabelArray[fieldNum].ShowWindow(SW_SHOW);
//	}
//	else
//	{
	//	m_EditArray[fieldNum].ShowWindow(SW_HIDE);
	//	m_EditLabelArray[fieldNum].ShowWindow(SW_HIDE);
//	}
}

////////////////////////////////////////////////////////////////////////
// OnCommand [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesWorkPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam == IDC_SWAPHOMEWITHWORK))
	{
		SwapHomeAndWork(homePage, this);
		return TRUE;
	}
	else if (LOWORD(wParam == IDC_PRIMARYCHECK))
	{
		CButton		*pButton = reinterpret_cast<CButton *>(GetDlgItem(IDC_PRIMARYCHECK));
		CButton		*pOtherButton = NULL;
		if (pButton)
		{
			if (homePage)
			{
				pOtherButton = reinterpret_cast<CButton *>(homePage->GetDlgItem(IDC_PRIMARYCHECK));
				if (pButton->GetCheck())
				{
					if (pOtherButton)
					{
						pOtherButton->SetCheck(BST_UNCHECKED);
					}
					homePage->m_EditArray[9].SetWindowText("work");
					homePage->m_EditArray[9].SetModify(TRUE);
				}
				else
				{
					if (pOtherButton)
					{
						pOtherButton->SetCheck(BST_CHECKED);
					}
					homePage->m_EditArray[9].SetWindowText("home");
					homePage->m_EditArray[9].SetModify(TRUE);
				}
				if (homePage->m_EditArray[9].GetModify())
				{
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_REQUESTING_NICKNAME, NULL);
					g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_MARK_NICKFILE_DIRTY, NULL);
				}
			}
		}
		return TRUE;
	}
	return CNicknamesPropertyPage::OnCommand(wParam, lParam);
}

BEGIN_MESSAGE_MAP(CNicknamesWorkPage, CNicknamesPropertyPage)
	//{{AFX_MSG_MAP(CNicknamesWorkPage)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
