// NICKSHT.CPP
//
// Property sheet for right-hand view of Address Book window.
//

#include "stdafx.h"
#include "resource.h"
#include "eudora.h"
#include "urledit.h"	// for NICKPAGE.H
#include "nickpage.h"	// for NICKSHT.H
#include "nicksht.h"
#include "guiutils.h"
#include "utils.h"

#include "doc.h"		// for NICKDOC.H
#include "nickdoc.h"	// for NICKTREE.H, NICKLIST.H
#ifdef WIN32
#include "nicktree.h"	// for NICKVIEW.H
#else
#include "nicklist.h"	// for NICKVIEW.H
#endif
#include "nickview.h"	// parent view class, needed for handling page resize



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CNicknamesPropSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CNicknamesPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CNicknamesPropSheet)
	ON_MESSAGE(WM_USER_RESIZE_PAGE, OnResizePage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CNicknamesPropSheet::CNicknamesPropSheet(CWnd* pWndParent)
	: CPropertySheet(IDR_NICKNAMES, pWndParent)
{
	// Reverse default of stacked tabs to go with scrolling tabs.
	EnableStackedTabs(FALSE);

	LoadPages();
}


BOOL CNicknamesPropSheet::LoadPages()
{
	//
	// The order that pages are added here determines the left 
	// to right tab order in the display.
	//
	//TRACE0("CNicknamesPropSheet::LoadPages()\n");
	AddPage(&m_addressPage);
	AddPage(&m_notesPage);
	//
	// Loop through and auto-detect whether any of the five allowed
	// custom dialog templates are present.  For each one that is
	// present, construct a new CNicknamesCustomPage object wrapper,
	// save it to the internal list of custom pages
	// (m_customPageList), and add it to this property sheet.
	//
	int dlg_id_array[5] = 
	{
#ifdef WIN32
		IDD_NICKNAME_CUSTOM_PAGE0_32,
		IDD_NICKNAME_CUSTOM_PAGE1_32,
		IDD_NICKNAME_CUSTOM_PAGE2_32,
		IDD_NICKNAME_CUSTOM_PAGE3_32,
		IDD_NICKNAME_CUSTOM_PAGE4_32
#else
		IDD_NICKNAME_CUSTOM_PAGE0,
		IDD_NICKNAME_CUSTOM_PAGE1,
		IDD_NICKNAME_CUSTOM_PAGE2,
		IDD_NICKNAME_CUSTOM_PAGE3,
		IDD_NICKNAME_CUSTOM_PAGE4
#endif // WIN32
	};

	for (int i = 0; i < 5; i++)
	{
		LPCSTR dlg_name = MAKEINTRESOURCE(dlg_id_array[i]);

		//
		// First, check resource DLLs
		//
		CPtrList* p_res_list = ((CEudoraApp *) AfxGetApp())->GetResListPtr();
		BOOL found = FALSE;
		POSITION pos = p_res_list->GetHeadPosition();
		while (pos != NULL)
		{
#ifdef WIN32
			HINSTANCE h_res_inst = (HINSTANCE) p_res_list->GetNext(pos);
#else
			HINSTANCE h_res_inst = (HINSTANCE)(WORD)(DWORD) p_res_list->GetNext(pos);
#endif

			if (h_res_inst && ::FindResource(h_res_inst, dlg_name, RT_DIALOG))
			{
				HINSTANCE h_temp = AfxGetInstanceHandle();
				HINSTANCE h_save_inst = AfxGetResourceHandle();
				AfxSetResourceHandle(h_res_inst);
				CNicknamesCustomPage* p_page = new CNicknamesCustomPage(dlg_id_array[i]);
				if (p_page != NULL)
				{
					m_customPageList.AddTail(p_page);
					AddPage(p_page);
				}
				AfxSetResourceHandle(h_save_inst);
				found = TRUE;
				break;
			}
		}

		if (! found)
		{
			HINSTANCE h_inst = QCFindResourceHandle(dlg_name, RT_DIALOG);
			if (::FindResource(h_inst, dlg_name, RT_DIALOG))
			{
				CNicknamesCustomPage* p_page = new CNicknamesCustomPage(dlg_id_array[i]);
				if (p_page != NULL)
				{
					m_customPageList.AddTail(p_page);
					AddPage(p_page);
				}
			}
		}
	}

	return TRUE;
}


CNicknamesPropSheet::~CNicknamesPropSheet()
{
	//
	// Loop through and delete any custom property pages that were
	// allocated at construction time.
	//
	while (! m_customPageList.IsEmpty())
	{
		CNicknamesCustomPage* p_page = (CNicknamesCustomPage *) m_customPageList.RemoveHead();
		ASSERT(p_page != NULL);
		delete p_page;
	}
}


////////////////////////////////////////////////////////////////////////
// GetNicknamePage [public]
//
// This is an ugly hack to work around what appears to be a compiler
// bug which only occurs when compiling in "release" mode.  Basically,
// the problem is that the calculation of the address of the 
// CPropertySheet::m_pages member variable is broken in that the computed
// offset is 12 bytes too big.  Hence, the workaround is to subtract
// 12 bytes from the value the compiler gives us to get the true offset
// from the 'this' pointer.
//
// The only place CPropertySheet::m_pages seems to be referenced inside
// of Eudora code is in CPropertySheet::GetPage().  Hence, this function
// replaces the CPropertySheet::GetPage() function.  Do NOT call 
// CPropertySheet:GetPage() anymore until this issue is resolved.
////////////////////////////////////////////////////////////////////////
CPropertyPage* CNicknamesPropSheet::GetNicknamePage(int nPage) const
{
#ifdef NDEBUG
	const CPtrArray* pPtrArray = (CPtrArray *) (((char *) &m_pages) - 12);
	CPropertyPage* pPage = (CPropertyPage *) pPtrArray->GetAt(nPage);
	ASSERT_KINDOF(CPropertyPage, pPage);
	return pPage;
#else
	return CPropertySheet::GetPage(nPage);
#endif
}

////////////////////////////////////////////////////////////////////////
// InitCustomPages [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesPropSheet::InitCustomPages()
{
	//TRACE0("CNicknamesPropSheet::InitCustomPages()\n");
	POSITION pos = m_customPageList.GetHeadPosition();
	while (pos != NULL)
	{
		CNicknamesCustomPage* p_page = (CNicknamesCustomPage *) m_customPageList.GetNext(pos);
		ASSERT(p_page != NULL);
		if (! p_page->InitPage())
			return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// RegisterCustomFields [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesPropSheet::RegisterCustomFields()
{
	//TRACE0("CNicknamesPropSheet::RegisterCustomFields()\n");
	POSITION pos = m_customPageList.GetHeadPosition();
	while (pos != NULL)
	{
		CNicknamesCustomPage* p_page = (CNicknamesCustomPage *) m_customPageList.GetNext(pos);
		ASSERT(p_page != NULL);
		p_page->RegisterFields();
	}
}


////////////////////////////////////////////////////////////////////////
// PopulateCustomEditControls [public]
//
////////////////////////////////////////////////////////////////////////
void CNicknamesPropSheet::PopulateCustomEditControls(CNickname* pNickname)
{
	ASSERT(pNickname != NULL);
	POSITION pos = m_customPageList.GetHeadPosition();
	while (pos != NULL)
	{
		CNicknamesCustomPage* p_page = (CNicknamesCustomPage *) m_customPageList.GetNext(pos);
		ASSERT(p_page != NULL);
		p_page->PopulateEditControls(pNickname);
	}
}

////////////////////////////////////////////////////////////////////////
// SaveModifiedCustomEditControls [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesPropSheet::SaveModifiedCustomEditControls(CNickname* pNickname)
{
	//TRACE0("CNicknamesPropSheet::SaveModifiedCustomEditControls()\n");
	ASSERT(pNickname != NULL);
	POSITION pos = m_customPageList.GetHeadPosition();
	BOOL found_modified = FALSE;
	while (pos != NULL)
	{
		CNicknamesCustomPage* p_page = (CNicknamesCustomPage *) m_customPageList.GetNext(pos);
		ASSERT(p_page != NULL);
		if (p_page->SaveModifiedEditControls(pNickname))
			found_modified = TRUE;
	}

	return found_modified;
}

////////////////////////////////////////////////////////////////////////
// PreTranslateMessage [public, virtual]
//
// HACK ALERT.  The base class implementation must be overridden so that
// certain "accelerator" keystrokes won't fall into the black hole
// known as IsDialogMessage().
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesPropSheet::PreTranslateMessage(MSG* pMsg)
{
	ASSERT_VALID(this);

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

	// allow sheet to translate Ctrl+Tab, Shift+Ctrl+Tab,
	//  Ctrl+PageUp, and Ctrl+PageDown
	if (pMsg->message == WM_KEYDOWN && GetAsyncKeyState(VK_CONTROL) < 0 &&
		(pMsg->wParam == VK_TAB || pMsg->wParam == VK_PRIOR || pMsg->wParam == VK_NEXT))
	{
		if (SendMessage(PSM_ISDIALOGMESSAGE, 0, (LPARAM)pMsg))
			return TRUE;
	}

	//
	// Be careful NOT to call base class implementation which ends up
	// calling IsDialogMessage().
	//
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// OnInitDialog [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CNicknamesPropSheet::OnInitDialog()
{
	//
	// See Microsoft Knowledge Base article Q149501.  This fixes a hang
	// caused by a bug in some versions of the Win32 property sheet
	// common control.
	//
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return CPropertySheet::OnInitDialog();
}


BOOL CNicknamesPropSheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR *) lParam;		// type cast

	if (UINT(TCN_SELCHANGE) == pNMHDR->code)
	{
		//
		// Kludge, Part 1.
		//
		// Okay, here goes a horrible kludge to work around the resize
		// limitation for CPropertyPages (see MS Knowledge Base
		// Article Q143291 for background).  Basically, the problem is
		// that CPropertySheets cache the *initial* size of the
		// property pages that it owns.  This creates problems when we
		// resize the view containing the property sheet since the
		// page sizes get out of sync.  Therefore, we must notify the
		// parent view whenever a new tab page is selected to resize
		// the property page based on the size cached from the last
		// time this sheet was resized.  We can't send the message
		// directly to the view from here since at the time we're
		// notified about the page change, the new page (which is the
		// one that needs to be resized) hasn't actually been made the
		// current page yet.  Geez.
		//
		PostMessage(WM_USER_RESIZE_PAGE);
	}

	return CPropertySheet::OnNotify(wParam, lParam, pResult);
}


////////////////////////////////////////////////////////////////////////
// OnResizePage [protected]
//
// Kludge, Part 2.
//
// Handler for user-defined WM_USER_RESIZE_PAGE message which is
// posted every time the current property page changes.  The job here
// is to notify the parent view that it needs to resize the current
// property page based on the cached property page size from the last
// time the view was resized.
//
////////////////////////////////////////////////////////////////////////
LONG CNicknamesPropSheet::OnResizePage(UINT, LONG)
{
	//TRACE0("CNicknamesPropSheet::OnResizePage()\n");
	CNicknamesViewRight* p_parent_view = (CNicknamesViewRight *) GetParent();
	ASSERT(p_parent_view->IsKindOf(RUNTIME_CLASS(CNicknamesViewRight)));
	p_parent_view->ResizeCurrentPropertyPage(m_PageRect);
	
	return 0;
}
