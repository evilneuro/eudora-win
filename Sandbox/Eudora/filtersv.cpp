// FILTERSV.CPP
//

#include "stdafx.h"

#include <mmsystem.h>

#include "QCUtils.h"

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "controls.h"
#include "filtersd.h"
#include "3dformv.h"
#include "guiutils.h"
#include "helpcntx.h"
#include "font.h"
#include "bmpcombo.h"
#include "eudora.h"
#include "newmbox.h"
#include "WazooWnd.h"
#include "MainFrm.h"
#include "persona.h"
#include "QCGraphics.h"
#include "utils.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCLabelDirector.h"
#include "QCStationeryDirector.h"

#include "filtersv.h"
#include "TextToSpeech.h"
#include "hsregex.h"

#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
 
// --------------------------------------------------------------------------

void AFXAPI AfxSetWindowText(HWND hWndCtrl, LPCTSTR lpszNew); 

// --------------------------------------------------------------------------

extern QCStationeryDirector	g_theStationeryDirector;

// Necessary for 16-bit context menus
#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU WM_RBUTTONUP
#endif

// Globals
static int g_3DGROWTH = 4;

/////////////////////////////////////////////////////////////////////////////
// CFilterLabelButton

CFilterLabelButton::CFilterLabelButton()
{
	m_Label = 0;
}

CFilterLabelButton::~CFilterLabelButton()
{
}

void CFilterLabelButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rect(lpDIS->rcItem);
	
	// Draw focus rect
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	
	// The control doesn't change its look for selection
	if (!(lpDIS->itemAction & ODA_DRAWENTIRE))
		return;

	// Background color is the selected color with a black frame
	CBrush Brush(m_Label ? (QCLabelDirector::GetLabelDirector()->GetLabelColor(m_Label)) : GetSysColor(COLOR_WINDOW));
	pDC->SelectObject(&Brush);
	pDC->SelectStockObject(BLACK_PEN);
	pDC->Rectangle(&rect);
	pDC->SelectStockObject(BLACK_BRUSH);
	
	// Text color is window background color
	const char* Text = "";
	if (m_Label > 0)
	{
		Text = (QCLabelDirector::GetLabelDirector()->GetLabelText(m_Label));
		pDC->SetTextColor(GetSysColor(COLOR_WINDOW)); // WHITE when the background is the label color
	}
	else
	{
		Text = "None";
		pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT)); // BLACK when the background is WHITE
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(Text, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

	// Draw focus rect
	if (lpDIS->itemState & ODS_FOCUS)
		pDC->DrawFocusRect(&rect);
}

BEGIN_MESSAGE_MAP(CFilterLabelButton, CButton)
	//{{AFX_MSG_MAP(CFilterLabelButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterLabelButton message handlers




/////////////////////////////////////////////////////////////////////////////
// CFiltersViewLeft

IMPLEMENT_DYNCREATE(CFiltersViewLeft, CFormView)

CFiltersViewLeft::CFiltersViewLeft()
	: CFormView(CFiltersViewLeft::IDD)
{
	//{{AFX_DATA_INIT(CFiltersViewLeft)
	//}}AFX_DATA_INIT
}

CFiltersViewLeft::~CFiltersViewLeft()
{
}

void CFiltersViewLeft::OnInitialUpdate()
{
	if (m_FiltersList.GetSafeHwnd())
	{
		ASSERT(0);
		return;				// guard against bogus double initializations
	}

	CFormView::OnInitialUpdate();

	CFiltersDoc* doc = GetDocument();

	//
	// Since we're creating the View manually, we need to register this
	// view with the Document manually.
	//
	doc->AddView(this);

	//
	// Add the filters and select the first filter
	//
	DoRefresh();

	// Frame window needs to be just big enough to hold the view
    // Size the controls
#ifdef WIN32
	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
#else
	SetScaleToFitSize(CSize(0, 0));
#endif
    OnSize(SIZE_RESTORED, -1, -1);
	
	//
	// As a hack, trick the parent Wazoo window into properly saving
	// the initial control focus for this form.
	//
	CWazooWnd::SaveInitialChildFocus(&m_FiltersList);
}

// [PUBLIC] DoOnRemove
//
// Simply calls OnRemove. Allows public access to the protected function.
//
void CFiltersViewLeft::DoOnRemove()
{
	OnRemove();
}

////////////////////////////////////////////////////////////////////////
// DoRefresh [protected]
//
// Refresh the UI controls with the current document info.
////////////////////////////////////////////////////////////////////////
void CFiltersViewLeft::DoRefresh()
{
	m_FiltersList.ResetContent();

	CFiltersDoc* doc = GetDocument();
	POSITION pos = doc->m_Filters.GetHeadPosition();
	
	int i = 0;
	while (pos)
	{
		CString Name = doc->m_Filters.GetNext(pos)->SetName();
		m_FiltersList.AddString(Name);
	}
	if (m_FiltersList.GetCount())
	{
		m_FiltersList.SetCurSel(0);
	}
	
	m_Sel = 0;
	OnSelChange();
}


////////////////////////////////////////////////////////////////////////
// GetDocument [public]
//
// The following very impolitely hides the public, NON-VIRTUAL
// CView::GetDocument() method....  this is very, very bad C++ style.
// :-) In Eudora 3.x or earlier, this was just a convenient way to
// type cast the CDocument pointer to a derived CFiltersDoc pointer.
// In Eudora 4.x or later, this override actually makes sure that we
// create and load up the One True CFiltersDoc(tm).
////////////////////////////////////////////////////////////////////////
CFiltersDoc* CFiltersViewLeft::GetDocument()
{
	CFiltersDoc* Doc = GetFiltersDoc();

	ASSERT_KINDOF(CFiltersDoc, Doc);
	
	return Doc;
}


void CFiltersViewLeft::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFiltersViewLeft)
	DDX_Control(pDX, IDC_FILTERS_LIST, m_FiltersList);
	DDX_Control(pDX, IDC_UP, m_Up);
	DDX_Control(pDX, IDC_REMOVE, m_Remove);
	DDX_Control(pDX, IDC_NEW, m_New);
	DDX_Control(pDX, IDC_DOWN, m_Down);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFiltersViewLeft, CFormView)
	//{{AFX_MSG_MAP(CFiltersViewLeft)
	ON_LBN_SELCHANGE(IDC_FILTERS_LIST, OnSelChange)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
#ifdef WIN32
	 ON_MESSAGE(WM_RBUTTONUP, OnRightButton)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFiltersViewLeft diagnostics

#ifdef _DEBUG
void CFiltersViewLeft::AssertValid() const
{
	CFormView::AssertValid();
}

void CFiltersViewLeft::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFiltersViewLeft message handlers
#ifdef WIN32
long CFiltersViewLeft::OnRightButton(WPARAM wParam, LPARAM lParam)
{
	if (IsWin32s() || (IsWinNT() &&	GetMajorVersion() == 3 && GetMinorVersion() < 51))
		return (OnContextMenu(wParam, lParam));
	return (CFormView::DefWindowProc(WM_RBUTTONUP, wParam, lParam));
}
#endif

long CFiltersViewLeft::OnContextMenu(WPARAM wParam, LPARAM lParam)
{				  
	return TRUE;
}


void CFiltersViewLeft::OnSelChange()
{
	CFiltersDoc* doc = GetDocument();

	int Sel = m_FiltersList.GetCurSel();
	m_Remove.EnableWindow(Sel >= 0);
	m_Up.EnableWindow(Sel > 0);
	m_Down.EnableWindow(Sel >= 0 && Sel != m_FiltersList.GetCount() - 1);

	if (LB_ERR != Sel && Sel != m_Sel)
	{
		RECT rect;
		m_FiltersList.GetItemRect(m_Sel, &rect);
		m_FiltersList.InvalidateRect(&rect);
		int hint = m_Sel = Sel;
		doc->UpdateAllViews(this,FUT_CHANGESEL,(CObject *)&hint);
	}
}


void CFiltersViewLeft::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if (m_FiltersList.m_hWnd == NULL || nType == SIZE_MINIMIZED)
		return;

	CRect rect, listRect, upRect, downRect, newRect, removeRect;
		
	GetClientRect(&rect);
	
	//
	// If the client window dimensions fall below certain reasonable
	// minimums, then fool the proportional sizing code below to think
	// it has a larger window.  This causes the displayed controls to
	// be clipped from the window rather than causing them to shrink
	// unusably small.
	//
	ASSERT((0 == rect.left) && (0 == rect.top));
	if (rect.right < 120)
		rect.right = 120;
	if (rect.bottom < 85)
		rect.bottom = 85;

	m_FiltersList.GetWindowRect(&listRect);		ScreenToClient(&listRect);
	m_Up.GetWindowRect(&upRect);				ScreenToClient(&upRect);
	m_Down.GetWindowRect(&downRect);			ScreenToClient(&downRect);
	m_New.GetWindowRect(&newRect);				ScreenToClient(&newRect);
	m_Remove.GetWindowRect(&removeRect);		ScreenToClient(&removeRect);
	                                                               
	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int xSpace = LOWORD(DBU) / 2;
	const int ySpace = HIWORD(DBU) / 2;
	const int ySpaceHalf = HIWORD(DBU) / 4;
	const int ButtonHeight = upRect.Height();
	int width;
	
	// Up & Down Buttons
 	width = rect.right - xSpace * 2;
	upRect.top = downRect.top = ySpaceHalf;
	upRect.bottom = downRect.bottom = downRect.top + ButtonHeight;
	upRect.left = xSpace;
	upRect.right = upRect.left + width/2 - xSpace;
	downRect.right = rect.right - xSpace;
	downRect.left = downRect.right - width/2 + xSpace;
	
	// New & Remove Buttons
	newRect.bottom = removeRect.bottom = rect.bottom - ySpaceHalf;
 	newRect.top = removeRect.top = removeRect.bottom - ButtonHeight;
	newRect.left = upRect.left;
	newRect.right = upRect.right;
	removeRect.left = downRect.left;
	removeRect.right = downRect.right;

 	// List boxes
	listRect.left = upRect.left;
	listRect.right = downRect.right;

	// Let's hide the up/down buttons since we have drag & drop
	listRect.top = downRect.top;
//	listRect.top = downRect.bottom + ySpaceHalf;
	listRect.bottom = newRect.top - ySpaceHalf;


	// Now resize all the controls
	m_Up.MoveWindow(&upRect, TRUE);
	m_Down.MoveWindow(&downRect, TRUE);
	
	m_FiltersList.MoveWindow(&listRect, TRUE);
	m_New.MoveWindow(&newRect, TRUE);
	m_Remove.MoveWindow(&removeRect, TRUE);

	// Hide up & down since we have drag & drop
	m_Up.ShowWindow(SW_HIDE);
	m_Down.ShowWindow(SW_HIDE);
	
}


/////////////////////////////////////////////////////////////////////////////

void CFiltersViewLeft::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(LOWORD(lHint))	 
	{
		case FUT_UPDATEVIEW:
			UpdateData(FALSE);
			break;
		case FUT_UPDATEDOC:
			UpdateData(TRUE);
			break;
		case FUT_CHANGENAME:
			UpdateName();
			break;
		case FUT_NEWITEM:
			{
				CFilter *pFilt = (CFilter *)pHint;
				NewItem(pFilt);
			}
			break;
		case FUT_REFRESH:
			DoRefresh();
			break;
		default:
			CFormView::OnUpdate(pSender, lHint, pHint);
			break;
	}
}


void CFiltersViewLeft::UpdateName()
{
	CFiltersDoc* doc = GetDocument();

	CFilter* filt = doc->m_Filters.IndexData(m_Sel);
	
	if (filt)
	{
		m_FiltersList.DeleteString(m_Sel);
		m_FiltersList.InsertString(m_Sel, filt->m_Name);
		m_FiltersList.SetCurSel(m_Sel);
	}
}

BOOL CFiltersViewLeft::MoveItem(int src, int dest, BOOL insertBefore /* = TRUE */)
{
	BOOL bSuccess = FALSE;
	CFiltersDoc* doc = GetDocument();
	int Count = doc->m_Filters.GetCount();

	// Make sure we're moving within the range
	if ( src < 0 || src >= Count || dest < 0 || dest >= Count)
	{
		ASSERT(src < 0);
		return FALSE;
	}
	
	POSITION srcDocPos = doc->m_Filters.FindIndex(src);
	POSITION dstDocPos = doc->m_Filters.FindIndex(dest);
	
	if (srcDocPos && dstDocPos)
	{
		CFilter* pSrcFilt = doc->m_Filters.GetAt( srcDocPos );
		CFilter* pDestFilt = doc->m_Filters.GetAt( dstDocPos );
		doc->m_Filters.RemoveAt(srcDocPos);
		
		dstDocPos =  doc->m_Filters.Find( pDestFilt, NULL ); 
		if (insertBefore)
			doc->m_Filters.InsertBefore(dstDocPos, pSrcFilt);
		else
			doc->m_Filters.InsertAfter(dstDocPos, pSrcFilt);

		m_FiltersList.DeleteString(src);

		if ((src < dest) && (insertBefore))
			dest--;
		else if ((src > dest) && (!insertBefore))
			dest++;

		m_Sel = m_FiltersList.InsertString(dest, pSrcFilt->m_Name);

		// Set the current selection
		m_FiltersList.SetCurSel(m_Sel);
		int hint = m_Sel;
		doc->UpdateAllViews(this,FUT_DIRTYSEL,(CObject *)&hint);
		OnSelChange();
		
		doc->SetModifiedFlag();
	}
	return bSuccess;
}

void CFiltersViewLeft::OnUp()
{
	MoveItem(m_Sel, m_Sel - 1);
}

void CFiltersViewLeft::OnDown()
{
	MoveItem(m_Sel, m_Sel + 1, FALSE);
}

void CFiltersViewLeft::OnNew()
{
	CFiltersDoc* doc 	= GetDocument();
	CFilter* filt 		= new CFilter;
	int curSel = 0;

	if (filt)
	{
		// Make sure previous filter info is saved
		UpdateData(TRUE);
		doc->UpdateAllViews(this, FUT_UPDATEDOC);
	
			
		// Default to an incoming filter
		filt->SetIncoming();
		curSel = m_FiltersList.GetCurSel();
			
		if (curSel >= 0)
		{
			POSITION POS = doc->m_Filters.FindIndex(curSel);
			doc->m_Filters.InsertAfter(POS, (CObject*)filt);
			m_FiltersList.InsertString(curSel + 1, filt->SetName());
			m_FiltersList.SetCurSel(curSel + 1);
		}
		else
		{
			doc->m_Filters.AddTail(filt);
			m_FiltersList.AddString(filt->SetName());
			m_FiltersList.SetCurSel(m_FiltersList.GetCount() - 1);
		}
		int hint = m_Sel = -2;
		doc->UpdateAllViews(this, FUT_DIRTYSEL, (CObject *)&hint);
		OnSelChange();
		doc->SetModifiedFlag();
	}
}

void CFiltersViewLeft::OnRemove()
{
	CFiltersDoc* 	doc 		= GetDocument();
	POSITION 		pos 		= doc->m_Filters.FindIndex(m_Sel);
                	
	
	if (pos)
	{
		CFilter* filt = doc->m_Filters.GetAt(pos);
		
		doc->m_Filters.RemoveAt(pos);
		delete filt;
		m_FiltersList.DeleteString(m_Sel);
		
		// Force the OnSelChange routine to update the data
		if (m_Sel == m_FiltersList.GetCount())
			m_Sel--;
		m_FiltersList.SetCurSel(m_Sel);
		int hint = m_Sel = -2;
		doc->UpdateAllViews(this, FUT_DIRTYSEL, (CObject *)&hint);
		OnSelChange();
		
		doc->SetModifiedFlag();	
	}
}

void CFiltersViewLeft::NewItem( CFilter *pFilter)
{
	CFiltersDoc* doc 	= GetDocument();

	int curSel = 0;

	if (pFilter)
	{
		// Make sure previous filter info is saved
		UpdateData(TRUE);
		doc->UpdateAllViews(this, FUT_UPDATEDOC);
	
		// Add the new filter	
		doc->m_Filters.AddTail(pFilter);
		m_FiltersList.AddString(pFilter->SetName());
		m_FiltersList.SetCurSel(m_FiltersList.GetCount() - 1);
		
		// Update the View
		int hint = m_Sel = -2;
		doc->UpdateAllViews(this, FUT_DIRTYSEL, (CObject *)&hint);
		OnSelChange();
		doc->SetModifiedFlag();
	}
}
/////////////////////////////////////////////////////////////////////////////
// CFiltersViewRight

IMPLEMENT_DYNCREATE(CFiltersViewRight, CFormView)

CFiltersViewRight::CFiltersViewRight()
	: CFormView(CFiltersViewRight::IDD)
{
	//{{AFX_DATA_INIT(CFiltersViewRight)
	m_Action0 = -1;
	m_Action1 = -1;
	m_Action2 = -1;
	m_Action3 = -1;
	m_Action4 = -1;
	//}}AFX_DATA_INIT
	m_ActionIndex = -1;
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		m_ActionCommands[i] = ID_FLT_NONE;
		m_RCheckValue[i] = 0;
		m_LCheckValue[i] = 0;
		m_EditValue[i].Empty();
		m_PriorityValue[i] = 0;
		m_StatusValue[i] = 0;
		m_PersonaValue[i].Empty();
		m_VoicesValue[i] = 0;
	}

	m_bHasSpeechEngine = false;
	for (i = 0; i < 5; i++)
		QCLoadBitmap( IDB_PRIOR_HIGHEST + i, m_priorBM[ i ] );

	m_Sel = -1;

}

CFiltersViewRight::~CFiltersViewRight()
{
	CFiltItem *p_Item = NULL;
	while( !m_FiltItemList.IsEmpty( ) )
	{
		p_Item = m_FiltItemList.RemoveHead( );
		if (p_Item)
		{
			delete p_Item;
			p_Item = NULL;
		}
	}
}


////////////////////////////////////////////////////////////////////////
// GetDocument [public]
//
// The following very impolitely hides the public, NON-VIRTUAL
// CView::GetDocument() method....  this is very, very bad C++ style.
// :-) In Eudora 3.x or earlier, this was just a convenient way to
// type cast the CDocument pointer to a derived CFiltersDoc pointer.
// In Eudora 4.x or later, this override actually makes sure that we
// create and load up the One True CFiltersDoc(tm).
////////////////////////////////////////////////////////////////////////
CFiltersDoc* CFiltersViewRight::GetDocument()
{
	CFiltersDoc* Doc = GetFiltersDoc();

	ASSERT_KINDOF(CFiltersDoc, Doc);
	
	return Doc;
}


void CFiltersViewRight::OnInitialUpdate()
{
	if (m_Verb0Combo.GetSafeHwnd())
	{
		ASSERT(0);
		return;				// guard against bogus double initializations
	}

	CFormView::OnInitialUpdate();
	
	CFiltersDoc* doc = GetDocument();

	//
	// Since we're creating the View manually, we need to register this
	// view with the Document manually.
	//
	doc->AddView(this);

	// Create a list of filt Items
	CFiltItem *p_MenuItem;
	CString ItemText;
	for (int id = ID_FLT_NONE; id <= ID_FLT_SKIP_REST; id++)
	{
		m_ActionMenu.GetMenuString( id, ItemText, MF_BYCOMMAND );
		p_MenuItem = new CFiltItem(id, (LPCTSTR)ItemText);
		m_FiltItemList.AddTail( p_MenuItem );
	}

	m_ActionMenu.ModifyItems(&m_FiltItemList);
	//
	// Initialize the controls with the current data from the doc.
	//
	DoRefresh();

	// Frame window needs to be just big enough to hold the view
	//GetParentFrame()->RecalcLayout();
	//ResizeParentToFit(TRUE);
#ifdef WIN32
	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
#else
	SetScaleToFitSize(CSize(0, 0));
#endif
	OnSize(SIZE_RESTORED, -1, -1);

	// Shareware: Register that we want to know of feature mode changes
	QCSharewareManager *pSWM = GetSharewareManager();
	if (pSWM)
	{
		pSWM->QCCommandDirector::Register((QICommandClient*)this);
	}
}

////////////////////////////////////////////////////////////////////////
// OnDestroy [PROTECTED]
//
////////////////////////////////////////////////////////////////////////
void CFiltersViewRight::OnDestroy()
{
	// Shareware: Unregister ourselves from notifications
	QCSharewareManager *pSWM = GetSharewareManager();
	if (pSWM)
	{
		pSWM->UnRegister(this);
	}

	CFormView::OnDestroy();
}

////////////////////////////////////////////////////////////////////////
// DoRefresh [protected]
//
// Refresh the UI controls with the current document info.
////////////////////////////////////////////////////////////////////////
void CFiltersViewRight::DoRefresh()
{
	// Fill in the comboboxes
	CString ItemText;
	int i;

	m_Header0Combo.ResetContent();
	m_Header1Combo.ResetContent();
	for (i = IDS_FHEADER_TO; ItemText.LoadString(i) && ItemText.IsEmpty() == FALSE; i++)
	{
		m_Header0Combo.AddString(ItemText);
		m_Header1Combo.AddString(ItemText);
	}

	m_Verb0Combo.ResetContent();
	m_Verb1Combo.ResetContent();
	for (i = VERB_FIRST; i <= VERB_LAST; i++)
	{
		m_Verb0Combo.AddString(CRString(i));
		m_Verb1Combo.AddString(CRString(i));
	}

	m_ConjunctionCombo.ResetContent();
	for (i = IDS_IGNORE; i <= IDS_UNLESS; i++)
	{
		m_ConjunctionCombo.AddString(CRString(i));
	}

#ifdef WIN32
	//
	// The Verb combo boxes are a fixed width, but localized strings
	// for the verbs contained in the list box are sometimes longer than
	// the default width of the dropdown list allows.  Therefore, let's
	// figure out the actual length of the longest string in the dropdown
	// list and set the dropdown width accordingly.
	//
	{
		//
		// Get a temporary DC for the verb combo box and use that for
		// test rendering the list strings.
		//
		CDC* pDC = m_Verb0Combo.GetDC();
		if (pDC)
		{
			CRect comboRect;
			m_Verb0Combo.GetWindowRect(&comboRect);
			ScreenToClient(&comboRect);

			CString str;

			for (i = 0; i < m_Verb0Combo.GetCount(); i++)
			{
				m_Verb0Combo.GetLBText(i, str);
				CSize size(pDC->GetTextExtent(str, str.GetLength()));
				if (size.cx > comboRect.Width())
					comboRect.right = comboRect.left + size.cx;
			}

			//
			// Set BOTH verb combo boxes based on the size of the first
			// combo box.
			//
			m_Verb0Combo.SetDroppedWidth(comboRect.Width());
			m_Verb1Combo.SetDroppedWidth(comboRect.Width());
			m_Verb0Combo.ReleaseDC( pDC );
		}
	}
#endif // WIN32
	
	for (i = 0; i < NUM_FILT_ACTS; i++)
		m_ActionCombo[i].ResetContent();

	// Fill in the Action combo boxes
	CFiltItem * p_item = NULL;
	POSITION pos = m_FiltItemList.GetHeadPosition();
	
	i = 0;
	while ( pos )
	{
		p_item =  m_FiltItemList.GetNext( pos );
		if (p_item)
		{
			ItemText = p_item->GetItemText();
			StripAccel(ItemText);
			for (int j = 0; j < NUM_FILT_ACTS; j++)
			{
				m_ActionCombo[j].AddString(ItemText);
				m_ActionCombo[j].SetItemData( i, (DWORD)p_item );
			}
			i++;
		}
	}

	for (i = 0; i < NUM_FILT_ACTS; i++)
		m_ActionCombo[i].SetCurSel(0);

	m_Sel = -1;
	UpdateData(TRUE);
	m_Sel = 0;
	UpdateData(FALSE);
}


void CFiltersViewRight::DoDataExchange(CDataExchange* pDX)
{
	CFiltersDoc* doc = GetDocument();
	CFilter* filt = NULL;
	
	if (m_Sel >= 0)
		filt = doc->m_Filters.IndexData(m_Sel);
	m_CurFilter = filt;

	
	// If we're filling in controls, then stick in info before calling DDX_ routines
	if (pDX->m_bSaveAndValidate == FALSE)
	{
		if (filt)
		{
			BOOL bModified = doc->IsModified();

			m_Incoming = filt->IsIncoming();
			m_Outgoing = filt->IsOutgoing();
			m_Manual = filt->IsManual();
			m_Header0 = filt->m_Header[0];
			m_Header1 = filt->m_Header[1];
			m_Verb0 = filt->m_Verb[0];
			m_Verb1 = filt->m_Verb[1];
			m_Value0 = filt->m_Value[0];
			m_Value1 = filt->m_Value[1];
			m_Conjunction = filt->m_Conjunction;

			// Clear out the Action combos
			int i = 0;
			for (i = 0; i< NUM_FILT_ACTS; i++)
				m_ActionCombo[i].SetCurSel(0);
			OnSelOKCombo0();
			OnSelOKCombo1();
			OnSelOKCombo2();
			OnSelOKCombo3();
			OnSelOKCombo4();

			/* This was improperly removing the modified flag when the tree selection changes */
			/* Bug #3100, fixed March 9, 1998 (Scott Manjourides) */
			
			// Restore the modified flag. 
			doc->SetModifiedFlag(bModified); 
			
			// Load with old style filters
			for (i = 0; i < NUM_FILT_ACTS; i++)
				LoadFilter(filt, filt->m_Actions[i], i);

			// Update 
			OnSelOKCombo0();
			OnSelOKCombo1();
			OnSelOKCombo2();
			OnSelOKCombo3();
			OnSelOKCombo4();
		}
		else
		{
			// No filter selected, so clear everything
			m_Incoming = m_Outgoing = m_Manual = FALSE;
			m_Header0.Empty();
			m_Header1.Empty();
			m_Verb0 = m_Verb1 = 0;
			m_Value0.Empty();
			m_Value1.Empty();
			m_Conjunction = 0;
			
			for (int i = 0; i < NUM_FILT_ACTS; i++)
			{
				m_EditValue[i].Empty();
				m_LCheckValue[i] = FALSE;
				m_LCheckValue[i] = FALSE;
			}
		}

		// Fix bug #3376: Filter action causes crash
		//
		// Problem was the old code assumed tab ordering, new code doesn't care
		//
		CWnd *pCtrl = GetWindow(GW_CHILD);
		ASSERT(pCtrl);

		if (pCtrl)
		{
			BOOL bFiltEnable = (filt != NULL); // Do we have a valid filter
			BOOL bWndEnabled = pCtrl->IsWindowEnabled(); // Is the right-side enabled?

			// If a filter is selected, enable all the controls, else disable them.
			// Only do this if it's a change in the enabled/disabled state
			if (bFiltEnable != bWndEnabled)
			{
				while (pCtrl)
				{
					pCtrl->EnableWindow(bFiltEnable);
					pCtrl = pCtrl->GetNextWindow();
				}
			}

		}
	}
	
	//{{AFX_DATA_MAP(CFiltersViewRight)
	DDX_Control(pDX, IDC_ACTION_COMBO4, m_ActionCombo[4]);
	DDX_Control(pDX, IDC_ACTION_COMBO3, m_ActionCombo[3]);
	DDX_Control(pDX, IDC_ACTION_COMBO2, m_ActionCombo[2]);
	DDX_Control(pDX, IDC_ACTION_COMBO1, m_ActionCombo[1]);
	DDX_Control(pDX, IDC_ACTION_COMBO0, m_ActionCombo[0]);
	DDX_Control(pDX, IDC_VERB1, m_Verb1Combo);
	DDX_Control(pDX, IDC_VERB0, m_Verb0Combo);
	DDX_Control(pDX, IDC_HEADER1, m_Header1Combo);
	DDX_Control(pDX, IDC_HEADER0, m_Header0Combo);
	DDX_Control(pDX, IDC_CONJUNCTION, m_ConjunctionCombo);
	DDX_CBIndex(pDX, IDC_CONJUNCTION, m_Conjunction);
	DDX_CBString(pDX, IDC_HEADER0, m_Header0);
	DDX_CBString(pDX, IDC_HEADER1, m_Header1);
	DDX_Check(pDX, IDC_INCOMING, m_Incoming);
	DDX_Check(pDX, IDC_MANUAL, m_Manual);
	DDX_Check(pDX, IDC_OUTGOING, m_Outgoing);
	DDX_Text(pDX, IDC_VALUE0, m_Value0);
	DDX_Text(pDX, IDC_VALUE1, m_Value1);
	DDX_CBIndex(pDX, IDC_VERB0, m_Verb0);
	DDX_CBIndex(pDX, IDC_VERB1, m_Verb1);
	DDX_CBIndex(pDX, IDC_ACTION_COMBO0, m_Action0);
	DDX_CBIndex(pDX, IDC_ACTION_COMBO1, m_Action1);
	DDX_CBIndex(pDX, IDC_ACTION_COMBO2, m_Action2);
	DDX_CBIndex(pDX, IDC_ACTION_COMBO3, m_Action3);
	DDX_CBIndex(pDX, IDC_ACTION_COMBO4, m_Action4);
	//}}AFX_DATA_MAP

	//Data Exchange for  dynamic controls
	for (int i = 0; i < NUM_FILT_ACTS; i++)
		DataEx(pDX,m_ActionCommands[i],i);

	
	// If we're retreiving from controls, then get info after calling DDX_ routines
	if (pDX->m_bSaveAndValidate == TRUE && filt)
	{
		BOOL ActionChanged = FALSE;

		for (int i = 0; i < NUM_FILT_ACTS && !ActionChanged; i++)
			ActionChanged = DidFiltersChange(filt, m_ActionCommands[i], i);

		// If at least one of the fields has changed, set the modified flag of the document
		if (ActionChanged ||
			m_Incoming != filt->IsIncoming() ||
			m_Outgoing != filt->IsOutgoing() ||
			m_Manual != filt->IsManual() ||
			m_Verb0 != filt->m_Verb[0] ||
			m_Verb1 != filt->m_Verb[1] ||
			m_Conjunction != filt->m_Conjunction ||
			m_Header0 != filt->m_Header[0] ||
			m_Header1 != filt->m_Header[1] ||
			m_Value0 != filt->m_Value[0] ||
			m_Value1 != filt->m_Value[1]) 

		{
			doc->SetModifiedFlag();
		}
			

		filt->SetIncoming(m_Incoming);
		filt->SetOutgoing(m_Outgoing);
		filt->SetManual(m_Manual);
		filt->m_Header[0] = m_Header0;
		filt->m_Header[1] = m_Header1;
		filt->m_Verb[0] = m_Verb0;
		filt->m_Verb[1] = m_Verb1;
		filt->m_Value[0] = m_Value0;
		filt->m_Value[1] = m_Value1;
		filt->m_Conjunction = m_Conjunction;

		for (i = 0; i < NUM_FILT_ACTS; i++)
			SaveIntoFilter(filt, m_ActionCommands[i], i);
	}
}

void CFiltersViewRight::DataEx(CDataExchange* pDX, UINT actionID, int index)
{
	switch(actionID)
	{
		case ID_FLT_NONE:
		case ID_FLT_SKIP_REST:
		case ID_FLT_PRINT:
		case ID_FLT_ADD_TO_HISTORY:
			break;
		
		case ID_FLT_LABEL:
			if (IsWindow(m_LabelButton[index].m_hWnd))
				DDX_Control(pDX, IDC_LABEL0+index, m_LabelButton[index]);
			break;
		
		case ID_FLT_COPY:
		case ID_FLT_TRANSFER:
			if (IsWindow(m_Mailbox[index].m_hWnd))
				DDX_Control(pDX, IDC_MAILBOX0+index, m_Mailbox[index]);
			break;
		case ID_FLT_REPLY:
			if (IsWindow(m_StationeryCombo[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_STATIONERY0+index, m_StationeryCombo[index]);
				DDX_CBString(pDX, IDC_STATIONERY0+index, m_StationeryValue[index]);
			}
			break;			
		case ID_FLT_PERSONALITY:
			if (IsWindow(m_PersonaCombo[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_PERSONA0+index, m_PersonaCombo[index]);
				DDX_CBString(pDX, IDC_FILT_PERSONA0+index, m_PersonaValue[index]);
			}
			break;
		case ID_FLT_SUBJECT:
		case ID_FLT_FORWARD:
		case ID_FLT_REDIRECT:
			if (IsWindow(m_FiltEdit[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_EDIT0+index, m_FiltEdit[index]);
				DDX_CBString(pDX, IDC_FILT_EDIT0+index, m_EditValue[index]);
			}
			break;
		case ID_FLT_OPEN: 			
		case ID_FLT_NOTIFY_USER:
		case ID_FLT_SERVER_OPT:
			if (IsWindow(m_LCheckBox[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_LCHECK0+index, m_LCheckBox[index]);
				DDX_Check(pDX, IDC_FILT_LCHECK0+index, m_LCheckValue[index]);
			}
			if (IsWindow(m_RCheckBox[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_RCHECK0+index, m_RCheckBox[index]);
				DDX_Check(pDX, IDC_FILT_RCHECK0+index, m_RCheckValue[index]);
			}
			break;

		case ID_FLT_PRIORITY:
			if (IsWindow(m_PriorityCombo[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_PRIOR0+index, m_PriorityCombo[index]);
				DDX_CBIndex(pDX, IDC_FILT_PRIOR0+index, m_PriorityValue[index]);
			}
			break;
		case ID_FLT_STATUS:
			if (IsWindow(m_StatusCombo[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_STATUS0+index, m_StatusCombo[index]);
				DDX_CBIndex(pDX, IDC_FILT_STATUS0+index, m_StatusValue[index]);
			}
			break;
		
		case ID_FLT_SOUND:
			if (IsWindow(m_AppBrz[index].m_hWnd))
				DDX_Control(pDX, IDC_FILT_SOUND0+index, m_AppBrz[index]);
			if (IsWindow(m_FiltEdit[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_EDIT0+index, m_FiltEdit[index]);
				DDX_CBString(pDX, IDC_FILT_EDIT0+index, m_EditValue[index]);
			}
			break;
		
		case ID_FLT_SPEAK:
			if (IsWindow(m_LCheckBox[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_LCHECK0+index, m_LCheckBox[index]);
				DDX_Check(pDX, IDC_FILT_LCHECK0+index, m_LCheckValue[index]);
			}
			if (IsWindow(m_RCheckBox[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_RCHECK0+index, m_RCheckBox[index]);
				DDX_Check(pDX, IDC_FILT_RCHECK0+index, m_RCheckValue[index]);
			}
			if (IsWindow(m_VoicesCombo[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_VOICE0+index, m_VoicesCombo[index]);
				DDX_CBIndex(pDX, IDC_FILT_VOICE0+index, m_VoicesValue[index]);
			}
			break;

		case ID_FLT_NOTIFY_APP:			
			if (IsWindow(m_AppBrz[index].m_hWnd))
				DDX_Control(pDX, IDC_APP_BRZ0+index, m_AppBrz[index]);
			if (IsWindow(m_FiltEdit[index].m_hWnd))
			{
				DDX_Control(pDX, IDC_FILT_EDIT0+index, m_FiltEdit[index]);
				DDX_CBString(pDX, IDC_FILT_EDIT0+index, m_EditValue[index]);
			}
			break;
	}

}


void CFiltersViewRight::LoadFilter(CFilter* filt, UINT actionID, int index)
{
	int ActionSel = 0;

	switch(actionID)
	{
		case ID_FLT_NONE:
			break;
		case ID_FLT_SKIP_REST:
   			if (filt->m_SkipRest)
				ActionSel = ID_FLT_SKIP_REST - ID_FLT_NONE;
			break;
		case ID_FLT_PRINT:
   			if (filt->m_Print)
				ActionSel = ID_FLT_PRINT - ID_FLT_NONE;
			break;
			
		case ID_FLT_ADD_TO_HISTORY:
   			if (filt->m_AddToHistory)
				ActionSel = ID_FLT_ADD_TO_HISTORY - ID_FLT_NONE;
			break;
		
		case ID_FLT_LABEL:
   			if (filt->m_Label >= 0)
				ActionSel = ID_FLT_LABEL - ID_FLT_NONE;
			break;
		
		case ID_FLT_COPY:
			if (filt->m_DoCopyTo[index]) //!filt->m_CopyTo[index].IsEmpty())
				ActionSel = ID_FLT_COPY-ID_FLT_NONE;
			break;
		case ID_FLT_TRANSFER:
			if (filt->IsTransfer()) 
				ActionSel = ID_FLT_TRANSFER-ID_FLT_NONE;
			break;
		case ID_FLT_PERSONALITY:
			if (filt->m_DoPersonality)//!filt->m_MakeSubject.IsEmpty())
				ActionSel = ID_FLT_PERSONALITY - ID_FLT_NONE;
			break;
		
		case ID_FLT_SUBJECT:
			if (filt->m_ChangeSubject)//!filt->m_MakeSubject.IsEmpty())
				ActionSel = ID_FLT_SUBJECT - ID_FLT_NONE;
			break;
		case ID_FLT_FORWARD:
			if (filt->m_DoForward[index])//!filt->m_Forward[index].IsEmpty())
				ActionSel = ID_FLT_FORWARD - ID_FLT_NONE;
			break;
		case ID_FLT_REDIRECT:
			if (filt->m_DoRedirect[index])//!filt->m_Redirect[index].IsEmpty())
				ActionSel = ID_FLT_REDIRECT - ID_FLT_NONE;
			break;
		case ID_FLT_REPLY:
			if (filt->m_DoReply[index])//!filt->m_Reply[index].IsEmpty())
				ActionSel = ID_FLT_REPLY - ID_FLT_NONE;
			break;
		case ID_FLT_OPEN:
			if (filt->m_OpenMM >= 0)
				ActionSel = ID_FLT_OPEN - ID_FLT_NONE;
			break;
		case ID_FLT_NOTIFY_USER:
			if (filt->m_NotfyUserNR >= 0)
				ActionSel = ID_FLT_NOTIFY_USER - ID_FLT_NONE;
			break;
		case ID_FLT_SERVER_OPT:
			if (filt->m_ServerOpt >= 0)
				ActionSel = ID_FLT_SERVER_OPT - ID_FLT_NONE;
			break;

		case ID_FLT_PRIORITY:
			if (filt->m_MakePriority >= 0 || filt->IsRaise() || filt->IsLower())
				ActionSel = ID_FLT_PRIORITY - ID_FLT_NONE;
			break;
		case ID_FLT_STATUS:
			if (filt->m_MakeStatus >= 0)
				ActionSel = ID_FLT_STATUS - ID_FLT_NONE;
			break;
		case ID_FLT_SOUND:
			if (filt->m_PlaySound[index]) 
				ActionSel = ID_FLT_SOUND-ID_FLT_NONE;
			break;
		case ID_FLT_SPEAK:
			if (filt->m_SpeakOptions >= 0)
				ActionSel = ID_FLT_SPEAK - ID_FLT_NONE;
			break;
		case ID_FLT_NOTIFY_APP:
			if (filt->m_DoNotifyApp[index]) 
				ActionSel = ID_FLT_NOTIFY_APP-ID_FLT_NONE;
			break;
	}
 	if (ActionSel)
		m_ActionCombo[index].SetCurSel(ActionSel);

}

BOOL CFiltersViewRight::DidFiltersChange(CFilter* filt, UINT actionID, int index)
{
	if (actionID != filt->m_Actions[index])
		return(TRUE);

	CString tmpStr;
	int		tmpInt;

	switch(actionID)
	{
		case ID_FLT_NONE:
		case ID_FLT_SKIP_REST:
		case ID_FLT_PRINT:
		case ID_FLT_ADD_TO_HISTORY:
			return(FALSE);
			break;
		
		case ID_FLT_LABEL:
			return (filt->m_Label != m_LabelButton[index].m_Label);
			break;
		
		case ID_FLT_COPY:
		case ID_FLT_TRANSFER:
			m_Mailbox[index].GetWindowText(tmpStr);
			return (tmpStr != filt->m_Desc[index]);
			break;
		case ID_FLT_SUBJECT:
			return (filt->m_MakeSubject != m_EditValue[index]);
			break;
		case ID_FLT_FORWARD:
			return (filt->m_Forward[index] != m_EditValue[index]);
			break;
		case ID_FLT_REDIRECT:
			return (filt->m_Redirect[index] != m_EditValue[index]);
			break;
		case ID_FLT_REPLY:
			return (filt->m_Reply[index] != m_StationeryValue[index]);
			break;
	
		case ID_FLT_OPEN:
		case ID_FLT_NOTIFY_USER:
		case ID_FLT_SERVER_OPT:
			tmpInt = 0;
			if (m_LCheckValue[index]) 
				tmpInt |= CBA_LBUTTON;
			if (m_RCheckValue[index])
 				tmpInt |= CBA_RBUTTON;

			if (actionID == ID_FLT_OPEN)
				return (tmpInt != filt->m_OpenMM);
			if (actionID == ID_FLT_NOTIFY_USER)
				return (tmpInt != filt->m_NotfyUserNR);
			if (actionID == ID_FLT_SERVER_OPT)
				return (tmpInt != filt->m_ServerOpt);
			break;
		case ID_FLT_PRIORITY:
			return(filt->m_MakePriority != m_PriorityValue[index]);
			break;
		case ID_FLT_PERSONALITY:
			return(filt->m_Personality != m_PersonaValue[index]);
			break;
		case ID_FLT_STATUS:
			return (filt->m_MakeStatus != m_StatusValue[index]);
			break;
		case ID_FLT_SOUND:
			return (filt->m_Sound[index] != m_EditValue[index]);
			break;
		case ID_FLT_SPEAK:
			
			if(m_bHasSpeechEngine)
			{
				tmpInt = 0;
				if (m_LCheckValue[index]) 
					tmpInt |= SPEAK_WHO;
				if (m_RCheckValue[index])
 					tmpInt |= SPEAK_SUBJECT;
				
				//ASSERT( m_VoicesValue[index] != -1); possible if you Text-to-speech engine is not available
				return ((filt->m_SpeakOptions != tmpInt) || 
					(filt->m_strVoiceGUID != (char *)(m_VoicesCombo[index].GetItemDataPtr(m_VoicesValue[index]))) );
			}
			else
				return false;
			break;

		case ID_FLT_NOTIFY_APP:
			return (filt->m_NotifyApp[index] != m_EditValue[index]);
			break;
	}

	return (FALSE);
}

void CFiltersViewRight::SaveIntoFilter(CFilter* filt, UINT actionID, int index)
{
	ClearAction(filt, filt->m_Actions[index], index);
	filt->m_Actions[index] = actionID; 
	switch(actionID)
	{
		case ID_FLT_NONE:
			break;
		case ID_FLT_SKIP_REST:
			filt->m_SkipRest = TRUE;
			break;
		case ID_FLT_PRINT:
			filt->m_Print = TRUE;
			break;
		case ID_FLT_ADD_TO_HISTORY:
			filt->m_AddToHistory = TRUE;
			break;
		case ID_FLT_LABEL:
			filt->m_Label = m_LabelButton[index].m_Label;
			break;
		
		case ID_FLT_COPY:
			filt->m_DoCopyTo[index] = TRUE;
			m_Mailbox[index].GetWindowText(filt->m_Desc[index]);

			// keeps same directory in case we're just changin actions from trans->copy
			if (filt->m_CopyTo[index].IsEmpty())
				filt->m_CopyTo[index] = filt->m_Mailbox;
			break;

		case ID_FLT_TRANSFER:
			filt->SetTransfer();
			filt->SetTransferTo();
			m_Mailbox[index].GetWindowText(filt->m_Desc[index]);

			// keeps same directory in case we're just changin actions from copy->trans
			if (filt->m_Mailbox.IsEmpty())
				filt->m_Mailbox = filt->m_CopyTo[index];
			break;
		case ID_FLT_PERSONALITY:
			filt->m_Personality = m_PersonaValue[index];
			filt->m_DoPersonality = TRUE;
			break;
		case ID_FLT_SUBJECT:
			filt->m_MakeSubject = m_EditValue[index];
			filt->m_ChangeSubject = TRUE;
			break;
		case ID_FLT_FORWARD:
			filt->m_Forward[index] = m_EditValue[index];
			filt->m_DoForward[index] = TRUE;
			break;
		case ID_FLT_REDIRECT:
			filt->m_Redirect[index] = m_EditValue[index];
			filt->m_DoRedirect[index] = TRUE;
			break;
		case ID_FLT_REPLY:
			filt->m_Reply[index] = m_StationeryValue[index];
			filt->m_DoReply[index] = TRUE;
			break;
		case ID_FLT_OPEN:
			filt->m_OpenMM = 0;
			if (m_LCheckValue[index]) 
				filt->m_OpenMM |= CBA_LBUTTON;
			if (m_RCheckValue[index])
 				filt->m_OpenMM |= CBA_RBUTTON;
			break;
		case ID_FLT_NOTIFY_USER:
			filt->m_NotfyUserNR = 0;
			if (m_LCheckValue[index]) 
				filt->m_NotfyUserNR |= CBA_LBUTTON;
			if (m_RCheckValue[index])
 				filt->m_NotfyUserNR |= CBA_RBUTTON;
			break;
		case ID_FLT_SERVER_OPT:
			filt->m_ServerOpt = 0;
			if (m_LCheckValue[index]) 
				filt->m_ServerOpt |= CBA_LBUTTON;
			if (m_RCheckValue[index])
 				filt->m_ServerOpt |= CBA_RBUTTON;
			break;

		case ID_FLT_PRIORITY:
			filt->m_MakePriority = m_PriorityValue[index];
			if (filt->m_MakePriority == PRIOR_LOWER)
				filt->SetLower();
			else if (filt->m_MakePriority == PRIOR_RAISE)
				filt->SetRaise();
			else
				filt->SetNoPriority();
			break;
		case ID_FLT_STATUS:
			filt->m_MakeStatus = m_StatusValue[index];
			break;
		case ID_FLT_SOUND:
			filt->m_Sound[index] = m_EditValue[index];
			filt->m_PlaySound[index] = TRUE;
			/*filt->m_PlaySound[index] = TRUE;
			m_SoundButton[index].GetWindowText(filt->m_Sound[index] );
			*/
			break;
		
		case ID_FLT_SPEAK:
			filt->m_SpeakOptions = 0;
			if (m_LCheckValue[index]) 
				filt->m_SpeakOptions |= SPEAK_WHO;
			if (m_RCheckValue[index])
 				filt->m_SpeakOptions |= SPEAK_SUBJECT;
			
			if(m_VoicesValue[index] == -1)
				m_VoicesValue[index] = 0;

			filt->m_strVoiceGUID = (char *)m_VoicesCombo[index].GetItemDataPtr(m_VoicesValue[index]);
			break;

		case ID_FLT_NOTIFY_APP:
			filt->m_NotifyApp[index] = m_EditValue[index];
			filt->m_DoNotifyApp[index] = TRUE;
			break;
		break;
	}

}

void CFiltersViewRight::ClearAction(CFilter* filt, UINT actionID, int index)
{
	switch(actionID)
	{
		case ID_FLT_ADD_TO_HISTORY:
			filt->m_AddToHistory = FALSE;
			break;
		case ID_FLT_SKIP_REST:
			filt->m_SkipRest = FALSE;
			break;
		case ID_FLT_PRINT:
			filt->m_Print = FALSE;
			break;		
		case ID_FLT_LABEL:
			filt->m_Label = -1;
			break;		
		case ID_FLT_COPY:
			filt->m_DoCopyTo[index] = FALSE;
			break;
		case ID_FLT_TRANSFER:
			filt->m_Transfer = 0;
			break;		
		case ID_FLT_PERSONALITY:
			filt->m_DoPersonality = FALSE;
			break;
		case ID_FLT_SUBJECT:
			filt->m_ChangeSubject = FALSE;
			break;
		case ID_FLT_FORWARD:
			filt->m_DoForward[index] = FALSE;
			break;
		case ID_FLT_REDIRECT:
			filt->m_DoRedirect[index] = FALSE;
			break;
		case ID_FLT_REPLY:
			filt->m_DoReply[index] = FALSE;
			break;
		case ID_FLT_OPEN:
			filt->m_OpenMM = -1;
			break;
		case ID_FLT_NOTIFY_USER:
			filt->m_NotfyUserNR = -1;
			break;
		case ID_FLT_SERVER_OPT:
			filt->m_ServerOpt = -1;
			break;
		case ID_FLT_PRIORITY:
			filt->m_MakePriority = -1;
			break;
		case ID_FLT_STATUS:
			filt->m_MakeStatus = -1;
			break;
		case ID_FLT_SOUND:
			filt->m_PlaySound[index] = FALSE;
			break;
		case ID_FLT_SPEAK:
			filt->m_SpeakOptions = -1;
			filt->m_strVoiceGUID = "";
			break;
		case ID_FLT_NOTIFY_APP:
			filt->m_DoNotifyApp[index] = FALSE;
			break;
		break;
	}
}

void CFiltersViewRight::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	int *theSel = (int *)pHint;
	switch(lHint)
	{
		case FUT_UPDATEVIEW:
			UpdateData(FALSE);
			break;
		case FUT_UPDATEDOC:
			UpdateData(TRUE);
			break;
		case FUT_DIRTYSEL:
			m_Sel = *theSel;
		case FUT_CHANGESEL:
			UpdateData(TRUE);
			m_Sel = *theSel;
			UpdateData(FALSE);
			break;
		case FUT_REFRESH:
			DoRefresh();
			break;

		default:
			CFormView::OnUpdate(pSender, lHint, pHint);
			break;
	}
}

BEGIN_MESSAGE_MAP(CFiltersViewRight, CFormView)
	//{{AFX_MSG_MAP(CFiltersViewRight)
    ON_WM_DESTROY()
	ON_EN_KILLFOCUS(IDC_VALUE0, OnKillFocusValue0)
	ON_EN_CHANGE(IDC_VALUE0, OnChange)
	ON_EN_KILLFOCUS(IDC_VALUE1, OnKillFocusValue1)
	ON_EN_CHANGE(IDC_VALUE1, OnChange)
	ON_BN_CLICKED(IDC_INCOMING, OnChange)
	ON_BN_CLICKED(IDC_OUTGOING, OnChange)
	ON_BN_CLICKED(IDC_MANUAL, OnChange)
	ON_CBN_EDITCHANGE(IDC_HEADER0, OnChange)
	ON_CBN_EDITCHANGE(IDC_HEADER1, OnChange)
	ON_CBN_SELCHANGE(IDC_HEADER0, OnChange)
	ON_CBN_SELCHANGE(IDC_HEADER1, OnChange)
	ON_CBN_SELCHANGE(IDC_VERB0, OnChange)
	ON_CBN_SELCHANGE(IDC_VERB1, OnChange)
	ON_CBN_SELCHANGE(IDC_CONJUNCTION, OnChange)
	ON_WM_SIZE()
	ON_CBN_DROPDOWN(IDC_ACTION_COMBO0, OnDropdownActionCombo0)
	ON_CBN_DROPDOWN(IDC_ACTION_COMBO1, OnDropdownActionCombo1)
	ON_CBN_DROPDOWN(IDC_ACTION_COMBO2, OnDropdownActionCombo2)
	ON_CBN_DROPDOWN(IDC_ACTION_COMBO3, OnDropdownActionCombo3)
	ON_CBN_DROPDOWN(IDC_ACTION_COMBO4, OnDropdownActionCombo4)

	ON_CBN_SELCHANGE(IDC_ACTION_COMBO0, OnSelChangeCombo0)
	ON_CBN_SELCHANGE(IDC_ACTION_COMBO1, OnSelChangeCombo1)
	ON_CBN_SELCHANGE(IDC_ACTION_COMBO2, OnSelChangeCombo2)
	ON_CBN_SELCHANGE(IDC_ACTION_COMBO3, OnSelChangeCombo3)
	ON_CBN_SELCHANGE(IDC_ACTION_COMBO4, OnSelChangeCombo4)
	
	ON_CBN_SELENDOK(IDC_ACTION_COMBO0, OnSelOKCombo0)
	ON_CBN_SELENDOK(IDC_ACTION_COMBO1, OnSelOKCombo1)
	ON_CBN_SELENDOK(IDC_ACTION_COMBO2, OnSelOKCombo2)
	ON_CBN_SELENDOK(IDC_ACTION_COMBO3, OnSelOKCombo3)
	ON_CBN_SELENDOK(IDC_ACTION_COMBO4, OnSelOKCombo4)

	ON_CBN_DROPDOWN(IDC_FILT_PRIOR0, OnDropdownPrio0)
	ON_CBN_DROPDOWN(IDC_FILT_PRIOR1, OnDropdownPrio1)
	ON_CBN_DROPDOWN(IDC_FILT_PRIOR2, OnDropdownPrio2)
	ON_CBN_DROPDOWN(IDC_FILT_PRIOR3, OnDropdownPrio3)
	ON_CBN_DROPDOWN(IDC_FILT_PRIOR4, OnDropdownPrio4)
	ON_CBN_KILLFOCUS(IDC_HEADER0, OnKillFocusCondition0)
	ON_BN_CLICKED(IDC_MAILBOX0, OnMailboxSel0)
	ON_BN_CLICKED(IDC_MAILBOX1, OnMailboxSel1)
	ON_BN_CLICKED(IDC_MAILBOX2, OnMailboxSel2)
	ON_BN_CLICKED(IDC_MAILBOX3, OnMailboxSel3)
	ON_BN_CLICKED(IDC_MAILBOX4, OnMailboxSel4)
	ON_BN_CLICKED(IDC_LABEL0, OnLabelSel0)
	ON_BN_CLICKED(IDC_LABEL1, OnLabelSel1)
	ON_BN_CLICKED(IDC_LABEL2, OnLabelSel2)
	ON_BN_CLICKED(IDC_LABEL3, OnLabelSel3)
	ON_BN_CLICKED(IDC_LABEL4, OnLabelSel4)
	ON_BN_CLICKED(IDC_FILT_SOUND0, OnSoundSel0)
	ON_BN_CLICKED(IDC_FILT_SOUND1, OnSoundSel1)
	ON_BN_CLICKED(IDC_FILT_SOUND2, OnSoundSel2)
	ON_BN_CLICKED(IDC_FILT_SOUND3, OnSoundSel3)
	ON_BN_CLICKED(IDC_FILT_SOUND4, OnSoundSel4)
	ON_BN_CLICKED(IDC_APP_BRZ0, OnAppSel0)
	ON_BN_CLICKED(IDC_APP_BRZ1, OnAppSel1)
	ON_BN_CLICKED(IDC_APP_BRZ2, OnAppSel2)
	ON_BN_CLICKED(IDC_APP_BRZ3, OnAppSel3)
	ON_BN_CLICKED(IDC_APP_BRZ4, OnAppSel4)
	ON_BN_CLICKED(IDC_FILT_PERSONA0, OnDropdownPersona0)
	ON_BN_CLICKED(IDC_FILT_PERSONA1, OnDropdownPersona1)
	ON_BN_CLICKED(IDC_FILT_PERSONA2, OnDropdownPersona2)
	ON_BN_CLICKED(IDC_FILT_PERSONA3, OnDropdownPersona3)
	ON_BN_CLICKED(IDC_FILT_PERSONA4, OnDropdownPersona4)
	//}}AFX_MSG_MAP

	// Label changing messages
	ON_COMMAND_EX(ID_MESSAGE_LABEL_NONE, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_1, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_2, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_3, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_4, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_5, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_6, SetLabel)
	ON_COMMAND_EX(ID_MESSAGE_LABEL_7, SetLabel)
	
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
#ifdef WIN32
	ON_MESSAGE(WM_RBUTTONUP, OnRightButton)
#endif
	ON_COMMAND( ID_NEW_FILTER_MBOX_IN_ROOT, CMailboxButton::OnNewMailboxInRoot )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, CMailboxButton::OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, CMailboxButton::OnUpdateDynamicCommand )

END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CFiltersViewRight diagnostics

#ifdef _DEBUG
void CFiltersViewRight::AssertValid() const
{
	CFormView::AssertValid();
}

void CFiltersViewRight::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFiltersViewRight message handlers
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
long CFiltersViewRight::OnRightButton(WPARAM wParam, LPARAM lParam)
{
	if (IsWin32s() || (IsWinNT() &&	GetMajorVersion() == 3 && GetMinorVersion() < 51))
		return (OnContextMenu(wParam, lParam));
	return (CFormView::DefWindowProc(WM_RBUTTONUP, wParam, lParam));
}
#endif

long CFiltersViewRight::OnContextMenu(WPARAM wParam, LPARAM lParam)
{				  
	return TRUE;
}


void CFiltersViewRight::OnMailbox()
{
	CMenu	theTransferMenu;
	CRect	rect;	
	CButton *mboxButton = NULL;
	
	if (IsWindow(m_Mailbox[m_ActionIndex].m_hWnd))
	{
		if ((m_Mailbox[m_ActionIndex].SelectMailbox()) == false)
			return; // Nothing changed, so do nothing
		
		CFiltersDoc* pDoc = GetDocument();
		CFilter* pFilter = ( m_Sel >= 0 ) ? pDoc->m_Filters.IndexData(m_Sel) : NULL;
	
		if( pFilter != NULL )
		{
			if (m_Mailbox[m_ActionIndex].m_MailboxPath.IsEmpty())
			{
				// default to the in mailbox
				CTocDoc	*InTOC = ::GetInToc();
				if ( InTOC )
				{
					m_Mailbox[m_ActionIndex].m_MailboxPath = InTOC->MBFilename();
					m_Mailbox[m_ActionIndex].m_ButtonDesc = CRString( IDS_IN_MBOX_NAME );
				}
			}
			
			if( pFilter->m_Actions[m_ActionIndex] == ID_FLT_TRANSFER )
				pFilter->m_Mailbox = m_Mailbox[m_ActionIndex].m_MailboxPath;
			else if( pFilter->m_Actions[m_ActionIndex] == ID_FLT_COPY )
				pFilter->m_CopyTo[m_ActionIndex] = m_Mailbox[m_ActionIndex].m_MailboxPath;

			pFilter->m_Desc[m_ActionIndex] = m_Mailbox[m_ActionIndex].m_ButtonDesc;
			pDoc->SetModifiedFlag();
		}
	
	}
	else
	{
		return;
	}
}

BOOL CFiltersViewRight::SetLabel(UINT nID)
{
	m_LabelButton[m_ActionIndex].m_Label = (int)(nID - ID_MESSAGE_LABEL_1 + 1);
	m_LabelButton[m_ActionIndex].Invalidate(FALSE);

	return (TRUE);
}

void CFiltersViewRight::OnLabel()
{
	CRect		rect;
	CMenu*		Menu;
	CMainFrame*	pMainFrame;
	UINT		uOffset;
	BOOL		bMaximized;

	bMaximized = FALSE;
	pMainFrame = ( CMainFrame* ) AfxGetMainWnd();
	pMainFrame->MDIGetActive( &bMaximized );
	uOffset = ( bMaximized ? 1 : 0 );

	((CSplitterWnd*)GetParent())->SetActivePane(0,1);	// Force the goddam thing to the right side.  SD

	Menu = pMainFrame->GetMenu()->GetSubMenu( MAIN_MENU_MESSAGE + uOffset );

	if( Menu == NULL )
	{
		return;
	}

	// Get the Change Menu
	Menu = Menu->GetSubMenu( 18 );

	if( Menu == NULL )
	{
		return;
	}

	CMenu *labelMenu = Menu->GetSubMenu(3);
	
	if(labelMenu != NULL)
	{
		m_LabelButton[m_ActionIndex].GetWindowRect(&rect);
		labelMenu->TrackPopupMenu(0, rect.left, rect.bottom, AfxGetMainWnd());
	}	
}

void CFiltersViewRight::OnSound()
{
	CRString SoundTitle(IDS_SOUND_DIALOG_TITLE);
	CRString SoundFilter(IDS_SOUND_FILTER);
	
	//StartDir = EudoraDir;
	
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		SoundFilter, this);
	dlg.m_ofn.Flags &= ~OFN_SHOWHELP;
	dlg.m_ofn.lpstrTitle = SoundTitle;
	//dlg.m_ofn.lpstrInitialDir = StartDir;

	if (dlg.DoModal() == IDOK)
	{
		CString selSnd = dlg.GetPathName();
		m_FiltEdit[m_ActionIndex].SetWindowText(dlg.GetPathName());
#ifdef WIN32
		PlaySound(selSnd, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
#else
		sndPlaySound(selSnd, SND_ASYNC | SND_NODEFAULT);
#endif
	}
}

void CFiltersViewRight::OnApp()
{
	CRString ProgTitle(IDS_PROGRAM_DIALOG_TITLE);
	CRString ProgFilter(IDS_PROGRAMS_FILTER);
	
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		ProgFilter, this);
	dlg.m_ofn.Flags &= ~OFN_SHOWHELP;
	dlg.m_ofn.lpstrTitle = ProgTitle;

	if (dlg.DoModal() == IDOK)
		m_FiltEdit[m_ActionIndex].SetWindowText(dlg.GetPathName());
}

void CFiltersViewRight::OnKillFocusValue0()
{
	CWnd *activeWnd = GetActiveWindow();
			
	OnKillFocusCondition0();
	
	//Validate Regex if verb is regex
	CFiltersDoc* doc = GetDocument();
	CFilter* filt = doc->m_Filters.IndexData(m_Sel);
	int verb = VERB_FIRST + filt->m_Verb[0];

	if( verb == IDS_MATCHES_REGEX_ICASE || verb == IDS_MATCHES_REGEX)
	{
		regex_t Reg;
		int nRes = regcomp(&Reg, filt->m_Value[0], REG_EXTENDED | REG_NOSUB);
		if(nRes != 0)
		{
			char error[256];
			regerror(nRes, &Reg, error, sizeof(error));
			
			CWnd* vWnd = GetDlgItem(IDC_VALUE0); 
			vWnd->MessageBox(error, "Invalid Regular Expression");

			//if( activeWnd) 
			//	activeWnd->SetActiveWindow();
			//if(vWnd)
			//	vWnd->SetFocus();

		}
	}

}

void CFiltersViewRight::OnKillFocusValue1()
{
	//Validate Regex if verb is regex
	CFiltersDoc* doc = GetDocument();
	CFilter* filt = doc->m_Filters.IndexData(m_Sel);

	UpdateData(TRUE);



}


void CFiltersViewRight::OnKillFocusCondition0()
{
	CFiltersDoc* doc = GetDocument();
	CFilter* filt = doc->m_Filters.IndexData(m_Sel);

	UpdateData(TRUE);
	if (filt)	filt->SetName();
	doc->UpdateAllViews(this,FUT_CHANGENAME);
}

void CFiltersViewRight::OnChange()
{
	CFiltersDoc* doc = GetDocument();
	CFilter* filt = doc->m_Filters.IndexData(m_Sel);
	doc->SetModifiedFlag();
}


void CFiltersViewRight::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);

	// Don't do any of this if the controls aren't created yet, or the window is being minimized
	if (m_Verb1Combo.m_hWnd == NULL || nType == SIZE_MINIMIZED)
		return;
 
	CRect rect, matchRect, actionRect, val0Rect, val1Rect, actRect;
		
	GetClientRect(&rect);
	CWnd* Wnd = GetDlgItem(IDC_MANUAL);
	Wnd->GetWindowRect(&actRect);		
	ScreenToClient(&actRect);

	//
	// If the client window dimensions fall below certain reasonable
	// minimums, then fool the proportional sizing code below to think
	// it has a larger window.  This causes the displayed controls to
	// be clipped from the window rather than causing them to shrink
	// unusably small.
	//
	ASSERT((0 == rect.left) && (0 == rect.top));
	if (rect.right < 260)
		rect.right = 260;

	//if (rect.Width() < actRect.right)
	//{	
	//	ScrollToPosition(CPoint(0,0));
	//	return;
	//}

	CWnd* MatchWnd = GetDlgItem(IDC_MATCH);
	CWnd* ActionWnd = GetDlgItem(IDC_ACTION); 
	CWnd* val0Wnd = GetDlgItem(IDC_VALUE0); 
	CWnd* val1Wnd = GetDlgItem(IDC_VALUE1); 

	MatchWnd->GetWindowRect(&matchRect);		
	ScreenToClient(&matchRect);
	
	ActionWnd->GetWindowRect(&actionRect);		
	ScreenToClient(&actionRect);
	
	val0Wnd->GetWindowRect(&val0Rect);			
	ScreenToClient(&val0Rect);
	
	val1Wnd->GetWindowRect(&val1Rect);			
	ScreenToClient(&val1Rect);
 
	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int xSpace = LOWORD(DBU) / 2;
	const int ySpace = HIWORD(DBU) / 2;
	const int xSpaceHalf = LOWORD(DBU) / 4;
	const int ySpaceHalf = HIWORD(DBU) / 4;

	// Match & Action Rect
	matchRect.right = actionRect.right = rect.right - xSpaceHalf;

	// Value Text Edits
	val0Rect.right  = val1Rect.right = matchRect.right - 2*xSpace;

	// Now resize all the controls
	MatchWnd->MoveWindow(&matchRect, TRUE);
	ActionWnd->MoveWindow(&actionRect, TRUE);
	val0Wnd->MoveWindow(&val0Rect, TRUE);
	val1Wnd->MoveWindow(&val1Rect, TRUE);

	// Resize Text Edit's to fit
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		CRect ctrlRect;
		if (IsWindow(m_FiltEdit[i].m_hWnd))
		{
			m_FiltEdit[i].GetWindowRect(&ctrlRect);		
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_FiltEdit[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_LabelButton[i].m_hWnd))
		{
			m_LabelButton[i].GetWindowRect(&ctrlRect);		
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_LabelButton[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_Mailbox[i].m_hWnd))
		{
			m_Mailbox[i].GetWindowRect(&ctrlRect);		
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_Mailbox[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_StationeryCombo[i].m_hWnd))
		{
			m_StationeryCombo[i].GetWindowRect(&ctrlRect);		
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_StationeryCombo[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_StatusCombo[i].m_hWnd))
		{
			m_StatusCombo[i].GetWindowRect(&ctrlRect);
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_StatusCombo[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_PriorityCombo[i].m_hWnd))
		{
			m_PriorityCombo[i].GetWindowRect(&ctrlRect);
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_PriorityCombo[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_PersonaCombo[i].m_hWnd))
		{
			m_PersonaCombo[i].GetWindowRect(&ctrlRect);
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			m_PersonaCombo[i].MoveWindow(&ctrlRect, TRUE);
		}
		else if (IsWindow(m_VoicesCombo[i].m_hWnd))
		{
			//CRect CheckRect;
			//m_RCheckBox[m_ActionIndex].GetWindowRect(&checkRect);
			//ScreenToClient(&checkRect);
			m_VoicesCombo[i].GetWindowRect(&ctrlRect);
			ScreenToClient(&ctrlRect);
			ctrlRect.right = actionRect.right - 2*xSpace;
			if(ctrlRect.right < ctrlRect.left + 50)
				ctrlRect.right = ctrlRect.left + 50;
			m_VoicesCombo[i].MoveWindow(&ctrlRect, TRUE);
		} 

	}
}


BOOL CFiltersViewRight::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	long id = GET_WM_COMMAND_ID(wParam,lParam);
	long command = GET_WM_COMMAND_CMD(wParam, lParam);

	// Action Menu Command
	if (id >= ID_FLT_NONE && id <= ID_FLT_SKIP_REST)
	{
		m_ActionCombo[m_ActionIndex].SetCurSel((int)(id - ID_FLT_NONE));
		return (TRUE);
	}
	
	if (id >= ID_PRIORITY_HIGHEST && id <= ID_PRIORITY_RAISE)
	{
		m_PriorityCombo[m_ActionIndex].SetCurSel((int)(id - ID_PRIORITY_HIGHEST));
		return (TRUE);
	}
	
	return CFormView::OnCommand(wParam, lParam);
}

extern CTTSHelper Helper;

void CFiltersViewRight::DropdownAction() 
{
	int id;
	CRect rect;
	m_ActionCombo[m_ActionIndex].GetWindowRect(&rect);

	// Uncheck previous selection, enable it
	for (id = ID_FLT_NONE; id <= ID_FLT_SKIP_REST; id++)
	{
		m_ActionMenu.CheckMenuItem(id, MF_BYCOMMAND | MF_UNCHECKED );
		m_ActionMenu.EnableMenuItem( id, MF_BYCOMMAND | MF_ENABLED );
	}

	/*if( !Helper.Initialize())
	{
		m_ActionMenu.EnableMenuItem( ID_FLT_SPEAK, MF_BYCOMMAND | MF_GRAYED );
	}*/

	// Shareware: Disable restricted action items in REDUCED FEATURE mode
	if (!UsingFullFeatureSet())
	{
		for (UINT nActionID = ID_FLT_NONE; nActionID <= ID_FLT_SKIP_REST; nActionID++)
		{
			if (CFilter::IsRestrictedFilterAction(nActionID))
				m_ActionMenu.EnableMenuItem( nActionID, MF_BYCOMMAND | MF_GRAYED );
		}
	}

	// Disallow selections that anther action combo is using
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		if (i != m_ActionIndex)
		{
			switch (m_ActionCommands[i])
			{
				//case ID_FLT_NONE:
				case ID_FLT_STATUS:
				case ID_FLT_PRIORITY:
				case ID_FLT_LABEL:
				case ID_FLT_PERSONALITY:
				case ID_FLT_SUBJECT:
				//case ID_FLT_SOUND:
				case ID_FLT_SPEAK:
				case ID_FLT_OPEN:
				case ID_FLT_PRINT:
				case ID_FLT_NOTIFY_USER:
				//case ID_FLT_NOTIFY_APP:
				//case ID_FLT_FORWARD:
				//case ID_FLT_REDIRECT:
				//case ID_FLT_REPLY:
				case ID_FLT_SERVER_OPT:
				//case ID_FLT_COPY:
				case ID_FLT_TRANSFER:
				case ID_FLT_SKIP_REST:
					m_ActionMenu.EnableMenuItem( m_ActionCommands[i], MF_BYCOMMAND | MF_GRAYED );
					break;
			}
		}
	}
	
	m_ActionMenu.CheckMenuItem(m_ActionCommands[m_ActionIndex], MF_BYCOMMAND | MF_CHECKED );
	m_ActionMenu.EnableMenuItem( m_ActionCommands[m_ActionIndex], MF_BYCOMMAND | MF_ENABLED );

	CContextMenu(&m_ActionMenu, rect.left, rect.bottom-1, FromHandle(m_hWnd));
	extern BOOL g_bPopupOn; 
	if (!g_bPopupOn)
		::PostMessage(m_ActionCombo[m_ActionIndex].m_hWnd, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, MAKELPARAM(rect.right,rect.Height()/2));
}

void CFiltersViewRight::DropdownPriority() 
{
	CRect rect;
	m_PriorityCombo[m_ActionIndex].GetWindowRect(&rect);

	// Get the menu that contains all the context popups
	CMenu menu;
	HMENU hMenu = QCLoadMenu(IDR_FILTER_MENUS);
	menu.Attach( hMenu );
	CMenu* menuPopup = menu.GetSubMenu(FM_PRIORITY);
  	
	for (int i = 0; IDB_PRIOR_LOWEST - i >= IDB_PRIOR_HIGHEST; i++)
	{
		menuPopup->SetMenuItemBitmaps( i, MF_BYPOSITION, &m_priorBM[i], &m_priorBM[i]);
		menuPopup->CheckMenuItem( i, MF_BYPOSITION | MF_CHECKED );
	}

	CContextMenu(menuPopup, rect.left, rect.bottom-1, FromHandle(m_hWnd));
	extern BOOL g_bPopupOn; 
	if (!g_bPopupOn)
		::PostMessage(m_PriorityCombo[m_ActionIndex].m_hWnd, WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON, MAKELPARAM(rect.right,rect.Height()/2));
}

void CFiltersViewRight::DropdownPersona() 
{
	// !!!JAB
}


void CFiltersViewRight::OnDropdownActionCombo0() { m_ActionIndex = 0; DropdownAction();}
void CFiltersViewRight::OnDropdownActionCombo1() { m_ActionIndex = 1; DropdownAction();}
void CFiltersViewRight::OnDropdownActionCombo2() { m_ActionIndex = 2; DropdownAction();}
void CFiltersViewRight::OnDropdownActionCombo3() { m_ActionIndex = 3; DropdownAction();}
void CFiltersViewRight::OnDropdownActionCombo4() { m_ActionIndex = 4; DropdownAction();}

void CFiltersViewRight::OnSelOKCombo0() {m_ActionIndex = 0;OnSelOKAction();m_Action0 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelOKCombo1() {m_ActionIndex = 1;OnSelOKAction();m_Action1 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelOKCombo2() {m_ActionIndex = 2;OnSelOKAction();m_Action2 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelOKCombo3() {m_ActionIndex = 3;OnSelOKAction();m_Action3 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelOKCombo4() {m_ActionIndex = 4;OnSelOKAction();m_Action4 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}

void CFiltersViewRight::OnSelChangeCombo0() {m_ActionIndex = 0;OnSelOKAction();m_Action0 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelChangeCombo1() {m_ActionIndex = 1;OnSelOKAction();m_Action1 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelChangeCombo2() {m_ActionIndex = 2;OnSelOKAction();m_Action2 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelChangeCombo3() {m_ActionIndex = 3;OnSelOKAction();m_Action3 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}
void CFiltersViewRight::OnSelChangeCombo4() {m_ActionIndex = 4;OnSelOKAction();m_Action4 = (int)(m_ActionCommands[m_ActionIndex]-ID_FLT_NONE);}

void CFiltersViewRight::OnDropdownPrio0() { m_ActionIndex = 0; DropdownPriority();}
void CFiltersViewRight::OnDropdownPrio1() { m_ActionIndex = 1; DropdownPriority();}
void CFiltersViewRight::OnDropdownPrio2() { m_ActionIndex = 2; DropdownPriority();}
void CFiltersViewRight::OnDropdownPrio3() { m_ActionIndex = 3; DropdownPriority();}
void CFiltersViewRight::OnDropdownPrio4() { m_ActionIndex = 4; DropdownPriority();}

void CFiltersViewRight::OnMailboxSel0(){m_ActionIndex = 0; OnMailbox();}
void CFiltersViewRight::OnMailboxSel1(){m_ActionIndex = 1; OnMailbox();}
void CFiltersViewRight::OnMailboxSel2(){m_ActionIndex = 2; OnMailbox();}
void CFiltersViewRight::OnMailboxSel3(){m_ActionIndex = 3; OnMailbox();}
void CFiltersViewRight::OnMailboxSel4(){m_ActionIndex = 4; OnMailbox();}

void CFiltersViewRight::OnLabelSel0(){m_ActionIndex = 0; OnLabel();}
void CFiltersViewRight::OnLabelSel1(){m_ActionIndex = 1; OnLabel();}
void CFiltersViewRight::OnLabelSel2(){m_ActionIndex = 2; OnLabel();}
void CFiltersViewRight::OnLabelSel3(){m_ActionIndex = 3; OnLabel();}
void CFiltersViewRight::OnLabelSel4(){m_ActionIndex = 4; OnLabel();}

void CFiltersViewRight::OnSoundSel0(){m_ActionIndex = 0; OnSound();}
void CFiltersViewRight::OnSoundSel1(){m_ActionIndex = 1; OnSound();}
void CFiltersViewRight::OnSoundSel2(){m_ActionIndex = 2; OnSound();}
void CFiltersViewRight::OnSoundSel3(){m_ActionIndex = 3; OnSound();}
void CFiltersViewRight::OnSoundSel4(){m_ActionIndex = 4; OnSound();}

void CFiltersViewRight::OnAppSel0(){m_ActionIndex = 0; OnApp();}
void CFiltersViewRight::OnAppSel1(){m_ActionIndex = 1; OnApp();}
void CFiltersViewRight::OnAppSel2(){m_ActionIndex = 2; OnApp();}
void CFiltersViewRight::OnAppSel3(){m_ActionIndex = 3; OnApp();}
void CFiltersViewRight::OnAppSel4(){m_ActionIndex = 4; OnApp();}

void CFiltersViewRight::OnDropdownPersona0() { m_ActionIndex = 0; DropdownPersona();}
void CFiltersViewRight::OnDropdownPersona1() { m_ActionIndex = 1; DropdownPersona();}
void CFiltersViewRight::OnDropdownPersona2() { m_ActionIndex = 2; DropdownPersona();}
void CFiltersViewRight::OnDropdownPersona3() { m_ActionIndex = 3; DropdownPersona();}
void CFiltersViewRight::OnDropdownPersona4() { m_ActionIndex = 4; DropdownPersona();}


void CFiltersViewRight::OnSelOKAction() 
{
	ASSERT(m_CurFilter);

	ValidChange(m_ActionIndex);

	CString text;
	int curSel = m_ActionCombo[m_ActionIndex].GetCurSel();

	// If the selection has not changed, do nothing
	if (m_ActionCommands[m_ActionIndex] == (UINT)(ID_FLT_NONE+curSel))
	{
		return;
	}
	else
	{
		// Remove the controls for the old action
		RemoveControl(m_ActionCommands[m_ActionIndex], m_ActionIndex);
	}
	
	// Has the action changed?
	if (m_CurFilter->m_Actions[ m_ActionIndex ] != (UINT)(ID_FLT_NONE+curSel) )
	{
		//set dirty flag...
		CFiltersDoc* doc = GetDocument();
		doc->SetModifiedFlag();
	}
		
	m_ActionCommands[m_ActionIndex] = (UINT)( ID_FLT_NONE+curSel );
	
	switch(ID_FLT_NONE+curSel)
	{
		case ID_FLT_NONE:
		case ID_FLT_SKIP_REST:
		case ID_FLT_PRINT:
			break;
		
		case ID_FLT_LABEL:
			SetupLabel(m_CurFilter->m_Label);
			break;
		
		case ID_FLT_COPY:
		case ID_FLT_TRANSFER:
			{
				// Still have to come up with a way to clear when switching
				SetupMailbox(m_CurFilter->m_Desc[m_ActionIndex]);

				m_CurFilter->m_Actions[ m_ActionIndex ] = ID_FLT_NONE+curSel;

				// The following code is legacy code that should never happen
				// This was in place to set any missing mailbox references
				//  to the In box. This is now done on startup, so m_Desc
				//  should never get this far and be empty.
				//
				// Leave in for 4.0.1, smanjo, 3/12/98
				CString tmpStr;
				m_Mailbox[ m_ActionIndex ].GetWindowText(tmpStr);
				if ( tmpStr.IsEmpty() )
				{
					// default to the in mailbox
					CTocDoc	*InTOC = ::GetInToc();
					if ( InTOC )
					{
						// Fixed: Scott Manjourides, 3/12/98
						if ((ID_FLT_NONE+curSel) == ID_FLT_TRANSFER)
						{
							m_CurFilter->m_Mailbox = InTOC->MBFilename();
						}
						else
						{
							m_CurFilter->m_CopyTo[ m_ActionIndex ] = InTOC->MBFilename();
						}

						m_CurFilter->m_Desc[m_ActionIndex] = CRString( IDS_IN_MBOX_NAME );
						m_Mailbox[m_ActionIndex].SetWindowText( CRString( IDS_IN_MBOX_NAME ) );

						//set dirty flag...
						CFiltersDoc* doc = GetDocument();
						doc->SetModifiedFlag();
					}
				}
			}

		break;
		
		case ID_FLT_PERSONALITY:
			SetupPersonaAction(m_CurFilter->m_Personality);
			break;
		
		case ID_FLT_SUBJECT:
			SetupEdit(m_CurFilter->m_MakeSubject);
			break;

		case ID_FLT_FORWARD:
		{
			SetupEdit(m_CurFilter->m_Forward[m_ActionIndex]);

			// Shareware: Disable the forwared edit field in REDUCED FEATURE mode
			if (!UsingFullFeatureSet())
			{
				// REDUCED FEATURE
				m_FiltEdit[m_ActionIndex].EnableWindow( FALSE );
			}
		}
		break;

		case ID_FLT_REDIRECT:
		{
			SetupEdit(m_CurFilter->m_Redirect[m_ActionIndex]);

			// Shareware: Disable the redirect edit field in REDUCED FEATURE mode
			if (!UsingFullFeatureSet())
			{
				// REDUCED FEATURE
				m_FiltEdit[m_ActionIndex].EnableWindow( FALSE );
			}
		}
		break;

		case ID_FLT_REPLY:
			SetupReplyWith(m_CurFilter->m_Reply[m_ActionIndex]);
			break;
		
		case ID_FLT_OPEN:
			if (m_CurFilter->m_OpenMM < 0) 
				m_CurFilter->m_OpenMM = 0;
			SetupCheckboxes(CRString(IDS_FILTER_OPEN_MAILBOX),(m_CurFilter->m_OpenMM & CBA_LBUTTON)? TRUE:0,
							CRString(IDS_FILTER_OPEN_MESSAGE),(m_CurFilter->m_OpenMM & CBA_RBUTTON)? TRUE:0);
			break;
		
		case ID_FLT_NOTIFY_USER:
			if (m_CurFilter->m_NotfyUserNR < 0) 
			{
				m_CurFilter->m_NotfyUserNR = 0;
				m_CurFilter->m_NotfyUserNR |= CBA_LBUTTON;
				if(GetIniShort(IDS_INI_FILTER_REPORT))
					m_CurFilter->m_NotfyUserNR |= CBA_RBUTTON;
			}
			SetupCheckboxes(CRString(IDS_FILTER_NOT_USER_NORMAL),(m_CurFilter->m_NotfyUserNR & CBA_LBUTTON)? TRUE:0,
							CRString(IDS_FILTER_NOT_USER_REP),(m_CurFilter->m_NotfyUserNR & CBA_RBUTTON)? TRUE:0);
			break;
		
		case ID_FLT_SERVER_OPT:
			if (m_CurFilter->m_ServerOpt < 0) 
				m_CurFilter->m_ServerOpt = 0;
			SetupCheckboxes(CRString(IDS_FILTER_SERV_OPT_FETCH),(m_CurFilter->m_ServerOpt & CBA_LBUTTON)? TRUE:0,
							CRString(IDS_FILTER_SERV_OPT_DEL),(m_CurFilter->m_ServerOpt & CBA_RBUTTON)? TRUE:0);
			break;
		
		case ID_FLT_PRIORITY:
			SetupPriority();
			break;

		case ID_FLT_STATUS:
			SetupStatus();
			break;
		
		case ID_FLT_SOUND:
			SetupSound();
			break;
		
		case ID_FLT_SPEAK:
			if (m_CurFilter->m_SpeakOptions < 0) 
				m_CurFilter->m_SpeakOptions = 0;
			
			SetupSpeak();
			break;
		
		case ID_FLT_NOTIFY_APP:
			SetupNotifyApp();
			break;
	}
}


void CFiltersViewRight::ValidChange(int index) 
{
	// This function dissallows selections that are exclusive and another
	// action has it already

	// check if we are going up or down
	int step = 1;
	int curSel = m_ActionCombo[index].GetCurSel();
	
	if ((UINT)(curSel + ID_FLT_NONE) < m_ActionCommands[index])
		step = -1;

	int i = 0;
	while ( i < NUM_FILT_ACTS && i >= 0) 
	{
		if (i != index)
		{
			// if this action is happening already, is more than one allowed
			if ((UINT)(curSel + ID_FLT_NONE) == m_ActionCommands[i])
			{
				switch (m_ActionCommands[i])
				{
					//case ID_FLT_NONE:
					case ID_FLT_STATUS:
					case ID_FLT_PRIORITY:
					case ID_FLT_LABEL:
					case ID_FLT_PERSONALITY:
					case ID_FLT_SUBJECT:
					//case ID_FLT_SOUND:
					case ID_FLT_SPEAK:
					case ID_FLT_OPEN:
					case ID_FLT_PRINT:
					case ID_FLT_NOTIFY_USER:
					//case ID_FLT_NOTIFY_APP:
					//case ID_FLT_FORWARD:
					//case ID_FLT_REDIRECT:
					//case ID_FLT_REPLY:
					case ID_FLT_SERVER_OPT:
					//case ID_FLT_COPY:
					case ID_FLT_TRANSFER:
					case ID_FLT_SKIP_REST:
							curSel += step;
							i = -1;
						break;
				}
			}
			
		}
		i++;
	}
	if (curSel + ID_FLT_NONE > ID_FLT_SKIP_REST) 
		curSel = (int)( m_ActionCommands[index] - ID_FLT_NONE );

	m_ActionCombo[index].SetCurSel(curSel);
}


void CFiltersViewRight::RemoveControl(UINT ActionID, int index)
{
	switch(ActionID)
	{
		case ID_FLT_NONE:
		case ID_FLT_SKIP_REST:
		case ID_FLT_PRINT:
			break;
		case ID_FLT_LABEL:
			m_LabelButton[index].DestroyWindow();	
			break;
		case ID_FLT_COPY:
		case ID_FLT_TRANSFER:
			m_Mailbox[index].DestroyWindow();	
			break;
		case ID_FLT_SUBJECT:
		case ID_FLT_FORWARD:
		case ID_FLT_REDIRECT:
			m_EditValue[index].Empty();
			m_FiltEdit[index].DestroyWindow();
			break;

		case ID_FLT_REPLY:
			m_StationeryCombo[index].DestroyWindow();	
			break;
		
		case ID_FLT_PERSONALITY:
			m_PersonaCombo[index].DestroyWindow();	
			break;
		
		case ID_FLT_OPEN:
		case ID_FLT_NOTIFY_USER:
		case ID_FLT_SERVER_OPT:
			m_LCheckValue[index] = 0;
			m_LCheckBox[index].DestroyWindow();
			m_RCheckValue[index] = 0;
			m_RCheckBox[index].DestroyWindow();	
			break;
		case ID_FLT_PRIORITY:
			m_PriorityValue[index] = 0;
			m_PriorityCombo[index].DestroyWindow();	
			break;
		case ID_FLT_STATUS:
			m_StatusValue[index] = 0;
			m_StatusCombo[index].DestroyWindow();	
			break;
		case ID_FLT_SOUND:
			m_AppBrz[index].DestroyWindow();
			m_FiltEdit[index].DestroyWindow();
			break;
		case ID_FLT_SPEAK:
			//destory who, subject checkboxes
			m_LCheckValue[index] = 0;
			m_LCheckBox[index].DestroyWindow();
			m_RCheckValue[index] = 0;
			m_RCheckBox[index].DestroyWindow();	

			//destory the voice combobox
			m_VoicesCombo[index].DestroyWindow();
			break;
			
		case ID_FLT_NOTIFY_APP:
			m_AppBrz[index].DestroyWindow();
			m_FiltEdit[index].DestroyWindow();
			break;
	}
}

void CFiltersViewRight::GetLargeRectPos(CRect &ctrlRect)
{
	CRect actRect, frameRect;
	//CRect viewRect;
	CWnd* ActionWnd = GetDlgItem(IDC_ACTION); 

	// Some pre-established values
	const DWORD DBU = ::GetDialogBaseUnits();
	const int xSpace = LOWORD(DBU) / 2;
	
	ActionWnd->GetWindowRect(&frameRect);		
	ScreenToClient(&frameRect);
	

	m_ActionCombo[m_ActionIndex].GetWindowRect(&actRect);
	ScreenToClient(&actRect);
	//GetClientRect(&viewRect);
	
	ctrlRect.top = actRect.top;
	ctrlRect.bottom = actRect.bottom;
	ctrlRect.left = actRect.right + 2*xSpace;
	ctrlRect.right = frameRect.right - 2*xSpace;
}

CFont * CFiltersViewRight::GetSysFont(int whichFont /*=-1*/)
{
	CFont *tmp = GetFont();
	if (whichFont == -1)
	{
#ifdef WIN32
	if (IsWin95())
		whichFont = DEFAULT_GUI_FONT;
	else
#endif
		whichFont = SYSTEM_FONT;
	}
	return (tmp);
	//return(tmp.FromHandle((HFONT)GetStockObject(whichFont)));
}


void CFiltersViewRight::SetupLabel(int sel) 
{
	CRect rect;
	GetLargeRectPos(rect);
	
	m_LabelButton[m_ActionIndex].Create( "", BS_OWNERDRAW | WS_TABSTOP | WS_VISIBLE, 
											rect, this, IDC_LABEL0+m_ActionIndex);
	m_LabelButton[m_ActionIndex].SetFont(GetSysFont());

	// Initialize Data
	m_LabelButton[m_ActionIndex].m_Label = ( sel>=0 ? sel: 0);
	m_LabelButton[m_ActionIndex].Invalidate();
}

void CFiltersViewRight::SetupMailbox(CString &text)
{
	CRect rect;
	GetLargeRectPos(rect);
	
	m_Mailbox[m_ActionIndex].Create( "", BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE, 
							rect, this, IDC_MAILBOX0+m_ActionIndex);
	m_Mailbox[m_ActionIndex].SetFont(GetSysFont());

	AfxSetWindowText(m_Mailbox[m_ActionIndex].m_hWnd, text);
}

void CFiltersViewRight::SetupReplyWith(CString &text)
{
	int		iSel;

	CRect rect;
	GetLargeRectPos(rect);
	rect.bottom = 500;

	m_StationeryCombo[m_ActionIndex].Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 
									rect, this, IDC_STATIONERY0+m_ActionIndex);

	
	g_theStationeryDirector.FillComboBox( &( m_StationeryCombo[ m_ActionIndex ] ) );

	m_StationeryCombo[m_ActionIndex].SetFont(GetSysFont());

	if( text.IsEmpty() )
	{
		if (m_StationeryCombo[m_ActionIndex].GetCount() > 0)
			m_StationeryCombo[m_ActionIndex].GetLBText( 0, text );
	}
	
	m_StationeryValue[m_ActionIndex] = text;

	iSel = m_StationeryCombo[m_ActionIndex].FindStringExact( -1, text );

	if( iSel != - 1 )
	{
		m_StationeryCombo[m_ActionIndex].SetCurSel( iSel );
	}
	else
	{
		m_StationeryCombo[m_ActionIndex].SetCurSel( 0 );
	}

	// Shareware: Disable the stationary combo control in REDUCED FEATURE mode
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		m_StationeryCombo[m_ActionIndex].EnableWindow( FALSE );
	}
}

void CFiltersViewRight::SetupEdit(CString &text) 
{
	CRect rect;
	GetLargeRectPos(rect);
	
	m_FiltEdit[m_ActionIndex].Create(WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 
								rect, this, IDC_FILT_EDIT0+m_ActionIndex);
	m_FiltEdit[m_ActionIndex].SetFont(GetSysFont());
	m_FiltEdit[m_ActionIndex].SetWindowText(text);
}

////////////////////////////////////////////////////////////////////////
// SetupCheckboxes [private]
//
// 
//
////////////////////////////////////////////////////////////////////////
void CFiltersViewRight::SetupCheckboxes(LPCTSTR lpLeftLabel, BOOL bIsLeftChecked, 
										LPCTSTR lpRightLabel, BOOL bIsRightChecked)
{
	// Some pre-established values
	const DWORD DLGUNITS = ::GetDialogBaseUnits();
	const int XSPACE = LOWORD(DLGUNITS) / 2;
	CString leftLabel = lpLeftLabel;
	CString rightLabel = lpRightLabel;

	//
	// Get position of dropdown "action" list box sitting to the
	// immediate left of the checkboxes.  This is our reference point
	// for positioning the checkboxes.
	//
	CRect comboRect;
	m_ActionCombo[m_ActionIndex].GetWindowRect(&comboRect);
	ScreenToClient(&comboRect);

	//
	// As a first pass, position the checkboxes on top of each other
	// with hardcoded widths.  We need to create the checkboxes
	// *somewhere* before we can get a usable DC for resizing the
	// checkboxes based on the label text extent.
	//
	CRect leftRect;
	CRect rightRect;
	leftRect.top = rightRect.top = comboRect.top;
	leftRect.bottom = rightRect.bottom = comboRect.bottom;
	leftRect.left = rightRect.left = comboRect.right + 2*XSPACE;
	leftRect.right = rightRect.right = leftRect.left + 80;			// as a first pass, just some hardcoded width

	//
	// Now, create the checkbox controls in their initial, dummy positions.
	//
	m_LCheckBox[m_ActionIndex].Create(leftLabel, BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE, 
										leftRect, this, IDC_FILT_LCHECK0+m_ActionIndex);
	m_LCheckBox[m_ActionIndex].SetCheck(bIsLeftChecked);
	m_LCheckValue[m_ActionIndex] = bIsLeftChecked;
	m_LCheckBox[m_ActionIndex].SetFont(GetSysFont());
	
	m_RCheckBox[m_ActionIndex].Create(rightLabel, BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE, 
										rightRect, this, IDC_FILT_RCHECK0+m_ActionIndex);
	m_RCheckBox[m_ActionIndex].SetCheck(bIsRightChecked);
	m_RCheckValue[m_ActionIndex] = bIsRightChecked;
	m_RCheckBox[m_ActionIndex].SetFont(GetSysFont());

	//
	// Now that the controls are created as bona-fide windows, grab
	// a DC from the controls for determining the actual width of
	// the label text.  We dynamically determine this width to resize
	// the checkboxes properly for localized label text.
	//
	{
		CDC* pDC = m_LCheckBox[m_ActionIndex].GetDC();
		if (pDC)
		{
			//
			// Resize left hand checkbox according to actual text length.
			// If the length is pretty short, then arbitrarily set the
			// length a bit longer so that if you have a filter
			// with multiple actions that all have checkboxes, there is a
			// better chance that they will line up more aesthetically
			// into columns.
			//
			CSize size(pDC->GetTextExtent(leftLabel, leftLabel.GetLength()));
			if (size.cx < 50)
				size.cx = 50;
			leftRect.right = leftRect.left + size.cx + leftRect.Height();
			m_LCheckBox[m_ActionIndex].MoveWindow(leftRect);

			//
			// Reposition and resize right hand checkbox to be
			// adjacent to the newly resize left hand checkbox.  Note
			// that this could actually be off the screen in some
			// cases!
			//
			rightRect.left = leftRect.right + 2*XSPACE;
			size = pDC->GetTextExtent(rightLabel, rightLabel.GetLength());
			rightRect.right = rightRect.left + size.cx + rightRect.Height();
			m_RCheckBox[m_ActionIndex].MoveWindow(rightRect);
		}
		else
		{
			ASSERT(0);
		}
	}
}

void CFiltersViewRight::SetupPriority() 
{
	CRect rect;
	CString ItemText;
	CMenu menu;

	GetLargeRectPos(rect);

	m_PriorityCombo[m_ActionIndex].Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 
									rect, this, IDC_FILT_PRIOR0+m_ActionIndex);
	
	// Get the menu that contains all the context popups
	HMENU hMenu = QCLoadMenu(IDR_FILTER_MENUS);
	menu.Attach( hMenu );
	CMenu* menuPopup = menu.GetSubMenu(FM_PRIORITY);
  	int numItems = menuPopup->GetMenuItemCount();
	char tmp[255];

	for(int i = 0; i < numItems; i++)
	{
		if (!(menuPopup->GetMenuState( i, MF_BYPOSITION ) & MF_SEPARATOR)) 
		{
			menuPopup->GetMenuString( i, tmp, 255, MF_BYPOSITION );
			ItemText = tmp;
			
			StripAccel(ItemText);
			m_PriorityCombo[m_ActionIndex].AddString(ItemText);
		}
	}
	m_PriorityCombo[m_ActionIndex].SetFont(GetSysFont());
	m_PriorityValue[m_ActionIndex] = m_CurFilter->m_MakePriority>=0 ? m_CurFilter->m_MakePriority : 2;
	m_PriorityCombo[m_ActionIndex].SetCurSel(m_PriorityValue[m_ActionIndex]);
}

void CFiltersViewRight::SetupStatus() 
{
	CRect rect;
	GetLargeRectPos(rect);
	CMenu menu;
	CString ItemText;

	rect.bottom = 500;
	
	m_StatusCombo[m_ActionIndex].Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 
	
								rect, this, IDC_FILT_STATUS0+m_ActionIndex);
	HMENU hMenu = QCLoadMenu(IDR_FILTER_MENUS);
	menu.Attach( hMenu );
	CMenu* menuPopup = menu.GetSubMenu(FM_STATUS);
  	int numItems = menuPopup->GetMenuItemCount();
	char tmp[255];

	// Load up Combo box
	for(int i = 0; i < numItems; i++)
	{
		if (!(menuPopup->GetMenuState( i, MF_BYPOSITION ) & MF_SEPARATOR)) 
		{
			menuPopup->GetMenuString( i, tmp, 255, MF_BYPOSITION );
			ItemText = tmp;
			m_StatusCombo[m_ActionIndex].AddString(ItemText);
		}
	}
	m_StatusCombo[m_ActionIndex].SetFont(GetSysFont());
	m_StatusValue[m_ActionIndex] = m_CurFilter->m_MakeStatus>=0 ? m_CurFilter->m_MakeStatus : 0;
	m_StatusCombo[m_ActionIndex].SetCurSel(m_StatusValue[m_ActionIndex]);
}

void CFiltersViewRight::SetupSound() 
{
	CRect rect, ctrlRect;
	GetLargeRectPos(rect);

	ctrlRect = rect;
	ctrlRect.right = ctrlRect.left + 20;
	
	m_AppBrz[m_ActionIndex].Create( "...", BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE, 
							ctrlRect, this, IDC_FILT_SOUND0+m_ActionIndex);
	m_AppBrz[m_ActionIndex].SetFont(GetSysFont());

	ctrlRect.left = ctrlRect.right + 5;
	ctrlRect.right = rect.right;
	m_FiltEdit[m_ActionIndex].Create(WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 
								ctrlRect, this, IDC_FILT_EDIT0+m_ActionIndex);
	m_FiltEdit[m_ActionIndex].SetFont(GetSysFont());

	m_FiltEdit[m_ActionIndex].SetWindowText(m_CurFilter->m_Sound[m_ActionIndex]);

	// Shareware: Disable the something in REDUCED FEATURE mode
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		m_AppBrz[m_ActionIndex].EnableWindow( FALSE );
		m_FiltEdit[m_ActionIndex].EnableWindow( FALSE );
	}
}

void CFiltersViewRight::SetupSpeak() 
{

	// Some pre-established values
	const DWORD DLGUNITS = ::GetDialogBaseUnits();
	const int XSPACE = LOWORD(DLGUNITS) / 2;

	ASSERT( m_CurFilter->m_SpeakOptions >= 0);
	BOOL bIsLeftChecked  = m_CurFilter->m_SpeakOptions & SPEAK_WHO;
	BOOL bIsRightChecked = m_CurFilter->m_SpeakOptions & SPEAK_SUBJECT;
	CString leftLabel = CRString(IDS_FILTER_SPEAK_WHO);
	CString rightLabel = CRString(IDS_FILTER_SPEAK_SUBJECT);
	
	/*CRect comboRect;
	CRect leftRect;
	CRect rightRect;
	leftRect.top = rightRect.top = comboRect.top;
	leftRect.bottom = rightRect.bottom = comboRect.bottom;
	leftRect.left = rightRect.left = comboRect.right + 2*XSPACE;
	leftRect.right = rightRect.right = leftRect.left + 80;			// as a first pass, just some hardcoded width
	*/

	CRect ctrlRect;
	GetLargeRectPos(ctrlRect);
	CRect leftRect = ctrlRect, rightRect = ctrlRect;
	
	//
	// Now, create the checkbox controls in their initial, dummy positions.
	//
	m_LCheckBox[m_ActionIndex].Create(leftLabel, BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE, 
										leftRect, this, IDC_FILT_LCHECK0+m_ActionIndex);
	m_LCheckBox[m_ActionIndex].SetCheck(bIsLeftChecked);
	m_LCheckValue[m_ActionIndex] = bIsLeftChecked;
	m_LCheckBox[m_ActionIndex].SetFont(GetSysFont());
	

	m_RCheckBox[m_ActionIndex].Create(rightLabel, BS_AUTOCHECKBOX | WS_TABSTOP | WS_VISIBLE, 
										rightRect, this, IDC_FILT_RCHECK0+m_ActionIndex);
	m_RCheckBox[m_ActionIndex].SetCheck(bIsRightChecked);
	m_RCheckValue[m_ActionIndex] = bIsRightChecked;
	m_RCheckBox[m_ActionIndex].SetFont(GetSysFont());

	//
	// Now that the controls are created as bona-fide windows, grab
	// a DC from the controls for determining the actual width of
	// the label text.  We dynamically determine this width to resize
	// the checkboxes properly for localized label text.
	//
	{
		CDC* pDC = m_LCheckBox[m_ActionIndex].GetDC();
		if (pDC)
		{
			//
			// Resize left hand checkbox according to actual text length.
			// If the length is pretty short, then arbitrarily set the
			// length a bit longer so that if you have a filter
			// with multiple actions that all have checkboxes, there is a
			// better chance that they will line up more aesthetically
			// into columns.
			//
			CSize size(pDC->GetTextExtent(leftLabel, leftLabel.GetLength()));
			if (size.cx < 50)
				size.cx = 50;
			leftRect.right = leftRect.left + size.cx + leftRect.Height();
			m_LCheckBox[m_ActionIndex].MoveWindow(leftRect);

			//
			// Reposition and resize right hand checkbox to be
			// adjacent to the newly resize left hand checkbox.  Note
			// that this could actually be off the screen in some
			// cases!
			//
			rightRect.left = leftRect.right + 2*XSPACE;
			size = pDC->GetTextExtent(rightLabel, rightLabel.GetLength());
			rightRect.right = rightRect.left + size.cx + rightRect.Height();
			m_RCheckBox[m_ActionIndex].MoveWindow(rightRect);
		}
		else
		{
			ASSERT(0);
		}
	}


	//Draw the combo

	ctrlRect.left = rightRect.right + 20;
	ctrlRect.bottom = 500;
	m_VoicesCombo[m_ActionIndex].Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 
									ctrlRect, this, IDC_FILT_VOICE0+m_ActionIndex);

	m_VoicesCombo[m_ActionIndex].SetFont(GetSysFont());
	
	//populate the combo and set the selection to the Guid sent as 2nd arg
	bool bRes = FillComboWithVoices(& m_VoicesCombo[m_ActionIndex], m_CurFilter->m_strVoiceGUID, &m_VoicesValue[m_ActionIndex]);
	if(!bRes)
	{
		m_bHasSpeechEngine = false;

		//if speech engine can not be initialized, disable the combo
		m_VoicesCombo[m_ActionIndex].EnableWindow(FALSE);

		m_LCheckBox[m_ActionIndex].EnableWindow(FALSE);
		m_RCheckBox[m_ActionIndex].EnableWindow(FALSE);
	}
	else
	{
		m_bHasSpeechEngine = true;
		m_VoicesCombo[m_ActionIndex].SetCurSel( m_VoicesValue[m_ActionIndex] );
	}
	
}


void CFiltersViewRight::SetupNotifyApp() 
{
	CRect rect, ctrlRect;
	GetLargeRectPos(rect);

	ctrlRect = rect;
	ctrlRect.right = ctrlRect.left + 20;
	
	CRString browse( IDS_BROWSE );
	m_AppBrz[m_ActionIndex].Create( "...", BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE, 
							ctrlRect, this, IDC_APP_BRZ0+m_ActionIndex);
	m_AppBrz[m_ActionIndex].SetFont(GetSysFont());

	ctrlRect.left = ctrlRect.right + 5;
	ctrlRect.right = rect.right;
	m_FiltEdit[m_ActionIndex].Create(WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 
								ctrlRect, this, IDC_FILT_EDIT0+m_ActionIndex);
	m_FiltEdit[m_ActionIndex].SetFont(GetSysFont());

	m_FiltEdit[m_ActionIndex].SetWindowText(m_CurFilter->m_NotifyApp[m_ActionIndex]);
}

/////////////////////////////////////////////////////////////////////////////
void CFiltersViewRight::SetupPersonaAction(CString &persona) 
{
	int		iSel;

	CRect rect;
	GetLargeRectPos(rect);
	rect.bottom = 500;

	m_PersonaCombo[m_ActionIndex].Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 
									rect, this, IDC_FILT_PERSONA0+m_ActionIndex);

	m_PersonaCombo[m_ActionIndex].SetFont(GetSysFont());
	
	//m_PersonaCombo.AddString("");		// empty entry

	// add alternate personalities
	LPSTR pszList = g_Personalities.List();
	while (pszList && *pszList)
	{
		m_PersonaCombo[m_ActionIndex].AddString(pszList);
		pszList += strlen(pszList) + 1;
	}


	if( persona.IsEmpty() )
		m_PersonaCombo[m_ActionIndex].GetLBText( 0, persona );
	
	m_PersonaValue[m_ActionIndex] = persona;

	iSel = m_PersonaCombo[m_ActionIndex].FindStringExact( -1, persona );

	if( iSel != - 1 )
		m_PersonaCombo[m_ActionIndex].SetCurSel( iSel );
	else
		m_PersonaCombo[m_ActionIndex].SetCurSel( 0 );

	// Shareware: Disable the personality combo control in REDUCED FEATURE mode
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		m_PersonaCombo[m_ActionIndex].EnableWindow( FALSE );
	}
}

void
CFiltersViewRight::Notify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData)
{
	if (theAction == CA_SWM_CHANGE_FEATURE)
	{

		// Make sure previous filter info is saved
		UpdateData(TRUE);
		
		// Refresh the view from the doc -- this will cause restricted actions to be grayed
		UpdateData(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFiltListBox

CFiltListBox::CFiltListBox()
{
	m_FiltImageList.Create();
}

CFiltListBox::~CFiltListBox()
{
}


BEGIN_MESSAGE_MAP(CFiltListBox, CListBox)
	//{{AFX_MSG_MAP(CFiltListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_WM_KEYUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFiltListBox message handlers
void CFiltListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		case VK_DELETE:
			{
				// The Del key is a shortcut for the Delete command.
				CWnd* pParentWnd = GetParent();
				ASSERT(pParentWnd != NULL);
				ASSERT_KINDOF(CFiltersViewLeft, pParentWnd);
				((CFiltersViewLeft *)pParentWnd)->DoOnRemove();
			}
			break;
		default:
			CDragListBox::OnKeyUp(nChar, nRepCnt, nFlags);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFiltListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	if (NULL == pDC || NULL == pDC->m_hDC)
		return;
	

	// Draw focus rect
	CRect rect(lpDIS->rcItem);
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	int leftOffset = 0;

	// Get the filter from the Doc
	CFilter* filt = GetFiltersDoc()->m_Filters.IndexData(lpDIS->itemID);
	if (!filt)
	{
		ASSERT(filt);
		return;
	}
	
	DrawInsert(-1);

	// Set up the colors
	COLORREF BackColor, ForeColor;
	BOOL bDisabled = FALSE;
	int Label = filt->m_Label;
	
	if (!filt->m_WhenToApply)
		bDisabled = TRUE;
	
	// Setup the Colors
	if (lpDIS->itemState & ODS_SELECTED)
	{
		if (Label > 0)
		{
			BackColor = (QCLabelDirector::GetLabelDirector()->GetLabelColor(Label)); // GetIniLong(LabelID);
			ForeColor = GetSysColor(COLOR_WINDOW);
		}
		else
		{
			ForeColor = bDisabled ? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_HIGHLIGHTTEXT);
			BackColor = GetSysColor(COLOR_HIGHLIGHT);
		}
	}
	else
	{
		if (Label > 0)
			ForeColor = (QCLabelDirector::GetLabelDirector()->GetLabelColor(Label)); // GetIniLong(LabelID);
		else
			ForeColor = bDisabled ? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_WINDOWTEXT);
		BackColor = GetSysColor(COLOR_WINDOW);
	}
	
	// Draw background
	CBrush BackBrush(BackColor);
	pDC->FillRect(&rect, &BackBrush);

	// Draw the icons
	POINT pt;
	pt.x = rect.left;
	pt.y = rect.top;
	rect.left = DrawIcons(pDC, pt, filt);
	
	// Draw the text
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(ForeColor);
	pDC->DrawText(filt->m_Name, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);
}


// Draw the action bitmaps for this filter
// Return the offset this draws to
int	CFiltListBox::DrawIcons( CDC *pDC, POINT pt, CFilter * pFilt )
{
	if ((m_FiltImageList.GetSafeHandle()) && (m_FiltImageList.GetImageCount() < 1))
	{
		ASSERT(0); // Houston, we have a problem!
		return (0);
	}

	IMAGEINFO ii;
	UINT nImageIdx = 0;

	// Walk through the actions, drawing an icon for each
	for (int idx = 0; idx < NUM_FILT_ACTS; idx++)
	{
		if ((ID_FLT_TRANSFER == pFilt->m_Actions[idx])
				&& (pFilt->m_Mailbox.CompareNoCase(EudoraDir + CRString( IDS_TRASH_MBOX_FILENAME) + CRString(IDS_MAILBOX_EXTENSION) ) == 0))
		{
			// Special case for the icon, if we transfer to trash draw a trash icon
			nImageIdx = CFilterImageList::IMAGE_TRANSFER_TRASH;
		}
		else
		{
			CFiltItem fi(pFilt->m_Actions[idx], "");
			nImageIdx = fi.GetImageOffset();
		}

		if (CFilterImageList::IMAGE_NOTHING != nImageIdx)
		{
			if (m_FiltImageList.Draw(pDC, nImageIdx, pt, ILD_NORMAL))
			{		
				if (m_FiltImageList.GetImageInfo(nImageIdx, &ii));
					pt.x += (ii.rcImage.right - ii.rcImage.left);
			}
		}
	}

	return pt.x;
}

int CFiltListBox::ItemFromPt( CPoint pt, BOOL bAutoScroll /* = TRUE */ ) const
{
	const int nOffset = (GetItemHeight(0) / 2);

	CRect rect;
	GetClientRect(&rect);
	ClientToScreen(&rect);

	rect.top -= nOffset;
	rect.bottom += nOffset;

	if (!rect.PtInRect(pt))
		return (-1);

	int up_idx, idx, dwn_idx;
	CPoint up_pt, dwn_pt;
	
	up_pt = pt;
	up_pt.y -= nOffset;

	dwn_pt = pt;
	dwn_pt.y += nOffset;

	up_idx = CDragListBox::ItemFromPt(up_pt, FALSE);
	dwn_idx = CDragListBox::ItemFromPt(dwn_pt, FALSE);
	idx = CDragListBox::ItemFromPt(pt, bAutoScroll);

	// Special case if we are off the top or bottom (but within our rect)
	if (idx == (-1))
	{
		if (dwn_idx != (-1))
		{
			return (dwn_idx);
		}
		else if (up_idx != (-1))
		{
			return (up_idx + 1);
		}
		else
		{
			return (-1);
		}
	}

	if (idx == up_idx)
		return (idx + 1);

	return (idx);
}

void CFiltListBox::DrawInsert(int nIndex)
{
	if (m_nLast != nIndex)
	{
		EraseSingle(m_nLast);

		if (nIndex == (-1))
			EraseSingle(nIndex);
		else
		{
			if (!DrawSingle(nIndex))
				nIndex = -1; // The draw failed
		}

		m_nLast = nIndex;
	}
}

#define ARROW_SIZE (4)

void CFiltListBox::GetDividerRgn(int nIndex, CRgn &rgn)
{
	CRect rct;
	if ((nIndex > 0) && (nIndex == GetCount())) // Draw insert for end of list (special case)
	{
		// Get rect for last item, but draw at bottom of rect
		GetItemRect((nIndex - 1), &rct);
		rct.top = rct.bottom;
		rct.top -= 2;
		rct.bottom += 2;

		CPoint PtArr[7];
		PtArr[0] = CPoint(rct.left, rct.top - ARROW_SIZE);
		PtArr[1] = CPoint(rct.left + ARROW_SIZE, rct.top);
		PtArr[2] = CPoint(rct.right - ARROW_SIZE, rct.top);
		PtArr[3] = CPoint(rct.right, rct.top - ARROW_SIZE);
		PtArr[4] = CPoint(rct.right, rct.bottom);
		PtArr[5] = CPoint(rct.left, rct.bottom);
		PtArr[6] = PtArr[0]; // Close the region

		int LenArr[1] = { 7 };

		VERIFY(rgn.CreatePolyPolygonRgn(PtArr, LenArr, 1, WINDING));
	}
	else if (nIndex == 0) // Beginning of list
	{
		GetItemRect(nIndex, &rct);
		rct.bottom = rct.top;
		rct.top -= 2;
		rct.bottom += 2;

		CPoint PtArr[7];
		PtArr[0] = CPoint(rct.left, rct.top);
		PtArr[1] = CPoint(rct.right, rct.top);
		PtArr[2] = CPoint(rct.right, rct.bottom + ARROW_SIZE);
		PtArr[3] = CPoint(rct.right - ARROW_SIZE, rct.bottom);
		PtArr[4] = CPoint(rct.left + ARROW_SIZE, rct.bottom);
		PtArr[5] = CPoint(rct.left, rct.bottom + ARROW_SIZE);
		PtArr[6] = PtArr[0]; // Close the region

		int LenArr[1] = { 7 };

		VERIFY(rgn.CreatePolyPolygonRgn(PtArr, LenArr, 1, WINDING));
	}
	else // Normal case
	{
		GetItemRect(nIndex, &rct);
		rct.bottom = rct.top;
		rct.top -= 2;
		rct.bottom += 2;

		CPoint PtArr[9];
		PtArr[0] = CPoint(rct.left, rct.top - ARROW_SIZE);
		PtArr[1] = CPoint(rct.left + ARROW_SIZE, rct.top);
		PtArr[2] = CPoint(rct.right - ARROW_SIZE, rct.top);
		PtArr[3] = CPoint(rct.right, rct.top - ARROW_SIZE);
		PtArr[4] = CPoint(rct.right, rct.bottom + ARROW_SIZE);
		PtArr[5] = CPoint(rct.right - ARROW_SIZE, rct.bottom);
		PtArr[6] = CPoint(rct.left + ARROW_SIZE, rct.bottom);
		PtArr[7] = CPoint(rct.left, rct.bottom + ARROW_SIZE);
		PtArr[8] = PtArr[0]; // Close the region

		int LenArr[1] = { 9 };

		VERIFY(rgn.CreatePolyPolygonRgn(PtArr, LenArr, 1, WINDING));
	}
}

BOOL CFiltListBox::EraseSingle(int nIndex)
{
	if (nIndex == -1)
		return (FALSE);

	CRect client_rect;
	GetClientRect(&client_rect);

	CRgn div_rgn;
	GetDividerRgn(nIndex, div_rgn);

	if (div_rgn.RectInRegion(client_rect))
	{
		InvalidateRgn(&div_rgn);
		return (TRUE);
	}

	return (FALSE);
}

BOOL CFiltListBox::DrawSingle(int nIndex)
{
	if (nIndex == -1)
		return (FALSE);

	CRect client_rect;
	GetClientRect(&client_rect);

	CRgn div_rgn;
	GetDividerRgn(nIndex, div_rgn);

	if (div_rgn.RectInRegion(client_rect))
	{
		CRgn client_rgn;
		client_rgn.CreateRectRgnIndirect(&client_rect);

		CDC* pDC = GetDC();

		pDC->SelectClipRgn(&client_rgn);

		CBrush the_brush;
		VERIFY(the_brush.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT)));
		pDC->FillRgn(&div_rgn, &the_brush);

//		CBrush OutlineBrush;
//		VERIFY(OutlineBrush.CreateSolidBrush(GetSysColor(COLOR_WINDOW)));
//		pDC->FrameRgn(&div_rgn, &OutlineBrush, 1, 1);

		ReleaseDC(pDC);
		return (TRUE);
	}

	return (FALSE);
}

void CFiltListBox::Dropped( int, CPoint pt )
{
	BOOL bInsertBefore = FALSE;
	int nDestIndex = ItemFromPt( pt, TRUE );

	DrawInsert(-1);

	if ((m_nStartDragIndex >= 0) && (nDestIndex >= 0) && (m_nStartDragIndex != nDestIndex) && (m_nStartDragIndex != (nDestIndex - 1)))
	{
		CFiltersViewLeft *pLeftView =  (CFiltersViewLeft *)GetParent();
		VERIFY(pLeftView);
		ASSERT_KINDOF(CFiltersViewLeft, pLeftView);

		// Bug #3256: Incorrect drag-n-drop moves. Always insert before the item under the drop point,
		//            this is because that's how the base class draws the insert indicator (always before).
		//            Of course this means you can never drag to the end of the list.
		bInsertBefore = TRUE;

		// Check if we're moving to end of list
		if ((nDestIndex > 0) && (nDestIndex == GetCount()))
		{
			nDestIndex--;
			bInsertBefore = FALSE;
		}

		pLeftView->MoveItem(m_nStartDragIndex, nDestIndex, bInsertBefore);
	}
	else
	{
		if (GetCurSel() != m_nStartDragIndex)
			SetCurSel(m_nStartDragIndex);

		SetFocus();
	}
}

BOOL CFiltListBox::BeginDrag(CPoint pt)
{
	m_nStartDragIndex = CDragListBox::ItemFromPt(pt, FALSE);
	m_nLast = -1;
	m_bFirstDragging = true;

	return TRUE;
}

void CFiltListBox::CancelDrag(CPoint)
{
	DrawInsert(-1);

	if (GetCurSel() != m_nStartDragIndex)
		SetCurSel(m_nStartDragIndex);

	SetFocus();
}

UINT CFiltListBox::Dragging(CPoint pt)
{
	if (m_bFirstDragging)
	{
		SetCurSel(-1);
		m_bFirstDragging = false;
	}

	int nIndex = ItemFromPt(pt, FALSE); // don't allow scrolling just yet

	if ((LB_ERR == nIndex)) // || (nIndex == m_nStartDragIndex) || (nIndex == (m_nStartDragIndex+1)))
	{
		DrawInsert(-1);
		return (DL_STOPCURSOR);
	}

	DrawInsert(nIndex);
	ItemFromPt(pt); // Scroll now

	return DL_MOVECURSOR;
}

/////////////////////////////////////////////////////////////////////////////
// CFiltItem
// This is where the bitmaps are mapped to the action
/////////////////////////////////////////////////////////////////////////////

CFiltItem::CFiltItem(UINT commandID, LPCTSTR itemText) : 
				m_CommandID(commandID), 
				m_ItemText(itemText)
{
	switch( commandID )
	{
		case ID_FLT_TRANSFER:
			m_ImageOffset = CFilterImageList::IMAGE_TRANSFER;
			break;
		case ID_FLT_COPY:
			m_ImageOffset = CFilterImageList::IMAGE_COPY_TO;
			break;
		case ID_FLT_SKIP_REST:
			m_ImageOffset = CFilterImageList::IMAGE_SKIP_REST;
			break;
		case ID_FLT_SOUND:
			m_ImageOffset = CFilterImageList::IMAGE_PLAY_SOUND;
			break;
		case ID_FLT_PERSONALITY:
			m_ImageOffset = CFilterImageList::IMAGE_PERSONALITY;
			break;
		case ID_FLT_PRINT:
			m_ImageOffset = CFilterImageList::IMAGE_PRINT;
			break;
		case ID_FLT_STATUS:
		case ID_FLT_PRIORITY:
		case ID_FLT_LABEL:
		case ID_FLT_SUBJECT:
		case ID_FLT_OPEN:
		case ID_FLT_NOTIFY_USER:
		case ID_FLT_NOTIFY_APP:
		case ID_FLT_FORWARD:
		case ID_FLT_REDIRECT:
		case ID_FLT_REPLY:
		case ID_FLT_SERVER_OPT:
		case ID_FLT_NONE:
		case ID_FLT_SPEAK:
		default:
			m_ImageOffset = CFilterImageList::IMAGE_NOTHING;
			break;
	}
	
}

/////////////////////////////////////////////////////////////////////////////
// CFiltActionMenu: Owner Draw Menu
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CFiltActionMenu: CFiltActionMenu
//		Uses the resource menu to load up the actions
/////////////////////////////////////////////////////////////////////////////

CFiltActionMenu::CFiltActionMenu()
{
	HMENU hMenu = QCLoadMenu(IDR_FILTER_MENUS);
	ASSERT(hMenu);

	HMENU hAMenu = ::GetSubMenu( hMenu, FM_FILTER_ACTIONS);
	ASSERT(hAMenu);
	Attach(hAMenu);

	m_FiltImageList.Create();
}

/////////////////////////////////////////////////////////////////////////////
CFiltActionMenu::~CFiltActionMenu()
{
}

/////////////////////////////////////////////////////////////////////////////
// CFiltActionMenu: ModifyItems
//		Change this to an owner draw
//		p_ActionItemList contains all info about the filter associated with this
/////////////////////////////////////////////////////////////////////////////
void CFiltActionMenu::ModifyItems(CFiltItemList *p_ActionItemList)
{
	// Now turn this menu into a OwerDraw
	CFiltItem *p_item;
	POSITION pos = p_ActionItemList->GetHeadPosition();
	while ( pos )
	{
		p_item =  p_ActionItemList->GetNext( pos );
		if (p_item)
			ModifyMenu( p_item->GetCommandID(), 
							MF_BYCOMMAND | MF_OWNERDRAW, 
							p_item->GetCommandID(), 
							(const char *)p_item );
	}

}


/////////////////////////////////////////////////////////////////////////////
// CFiltActionMenu: MeasureItem
//	This is owner Draw
/////////////////////////////////////////////////////////////////////////////

void CFiltActionMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	CDC	theDC;
	theDC.CreateCompatibleDC( NULL );
		
	int itemNumber = lpMIS->itemID;
	CFiltItem* p_item = NULL;
	if (itemNumber != -1)
		p_item = (CFiltItem *)lpMIS->itemData;

 	CString Text;
 	if (p_item) 
		Text = p_item->GetItemText();
	CSize size(theDC.GetTextExtent(Text, Text.GetLength()));

	IMAGEINFO ii;
	int height = 20, width = 0;
	if ( m_FiltImageList.GetImageInfo( CFilterImageList::IMAGE_NOTHING, &ii ))
	{
		height = ii.rcImage.bottom - ii.rcImage.top + g_3DGROWTH;
		width = ii.rcImage.right - ii.rcImage.left;
	}
	
	lpMIS->itemWidth = size.cx + width;
	lpMIS->itemHeight = __max( height, size.cy);
}

/////////////////////////////////////////////////////////////////////////////
// CFiltActionMenu: MeasureItem
//	This is owner Draw
/////////////////////////////////////////////////////////////////////////////
void CFiltActionMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS )
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	if (NULL == pDC || NULL == pDC->m_hDC)
		return;
	
	// Draw focus rect
	CRect rect(lpDIS->rcItem);
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	int leftOffset = 0;

	// Set up the colors
	COLORREF BackColor, ForeColor;
	
	// Is this grayed?
	BOOL bDisabled = (lpDIS->itemState & ODS_GRAYED) == ODS_GRAYED;
	BOOL bChecked = (lpDIS->itemState & ODS_CHECKED) == ODS_CHECKED;
	
	// Setup the Colors
	if (lpDIS->itemState & ODS_SELECTED)
	{
		ForeColor = bDisabled ? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_MENU);
		BackColor = GetSysColor(COLOR_HIGHLIGHT);
	}
	else
	{
		ForeColor = bDisabled ? GetSysColor(COLOR_GRAYTEXT) : GetSysColor(COLOR_WINDOWTEXT);
		BackColor = GetSysColor(COLOR_MENU);
	}
	

	// Draw background
	CBrush BackBrush(BackColor);
	pDC->FillRect(&rect, &BackBrush);

	// Draw the text
	CFiltItem *p_FiltItem = (CFiltItem *)lpDIS->itemData;
	ASSERT(p_FiltItem);

	// Draw the icons
	POINT pt;
	pt.x = rect.left;
	pt.y = rect.top;
	rect.left = DrawIcons(pDC, pt, bChecked, p_FiltItem);
	

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(ForeColor);
	pDC->DrawText(p_FiltItem->GetItemText(), -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
}

/////////////////////////////////////////////////////////////////////////////
// CFiltActionMenu: MeasureItem
//	returns offset where it finished drawing horizontally
/////////////////////////////////////////////////////////////////////////////
int	CFiltActionMenu::DrawIcons( CDC *pDC, POINT pt, BOOL bChecked, CFiltItem *p_Item)
{
	// Draw the Action bitmaps
	IMAGEINFO ii;

	int imageOffset = p_Item->GetImageOffset();

	if ( m_FiltImageList.GetImageInfo( CFilterImageList::IMAGE_TRANSFER, &ii ))
	{
		int leftOffset = ii.rcImage.right - ii.rcImage.left;
		int i = 0;
		
		if (bChecked)
		{
			pDC->Draw3dRect( pt.x, pt.y, 
						g_3DGROWTH + ii.rcImage.right - ii.rcImage.left, 
						g_3DGROWTH + ii.rcImage.bottom - ii.rcImage.top, 
						RGB(0,0,0), RGB(255,255,255)); 
			if (imageOffset == CFilterImageList::IMAGE_NOTHING)
				m_FiltImageList.Draw( pDC,  CFilterImageList::IMAGE_CHECK, pt, ILD_TRANSPARENT );
			//else
			//	m_FiltImageList.Draw( pDC,  CFilterImageList::IMAGE_OUTLINE, pt, ILD_TRANSPARENT );
		}
		pt.x += g_3DGROWTH/2; pt.y += g_3DGROWTH/2;
		m_FiltImageList.Draw( pDC,  imageOffset, pt, ILD_NORMAL );
		
		
		pt.x+= leftOffset + g_3DGROWTH;
	}
	return pt.x;
}

	

/////////////////////////////////////////////////////////////////////////////
// CFiltComboBox: Owner Draw Combo box that get's dispayed for the actions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CFiltComboBox ... it uses the menu's measure functions
/////////////////////////////////////////////////////////////////////////////

void CFiltComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	CFiltersViewRight *pRightView = (CFiltersViewRight *)GetParent();
	ASSERT_KINDOF(CFiltersViewRight, pRightView);

	pRightView->m_ActionMenu.MeasureItem(lpMIS);
}

/////////////////////////////////////////////////////////////////////////////
// CFiltComboBox ... it uses the menu's draw icon functions
/////////////////////////////////////////////////////////////////////////////
void CFiltComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS )
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	if (NULL == pDC || NULL == pDC->m_hDC)
		return;
	
	// Draw focus rect
	CRect rect(lpDIS->rcItem);
	if ((lpDIS->itemAction & ODA_FOCUS) == ODA_FOCUS)
	{
		pDC->DrawFocusRect(&rect);
		return;
	}
	int leftOffset = 0;

	// Set up the colors
	COLORREF BackColor, ForeColor;
	
	// Is this grayed?
	
	// Setup the Colors
	if (lpDIS->itemState & ODS_SELECTED)
	{
		ForeColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
		BackColor = GetSysColor(COLOR_HIGHLIGHT);
	}
	else
	{
		ForeColor = GetSysColor(COLOR_WINDOWTEXT);
		BackColor = GetSysColor(COLOR_WINDOW);
	}
	

	// Draw background
	CBrush BackBrush(BackColor);
	pDC->FillRect(&rect, &BackBrush);

	// Make sure we got something here
	if ((int)lpDIS->itemID < 0)
		return;

	// Get the Item
	CFiltItem *p_FiltItem = (CFiltItem *)lpDIS->itemData;
	ASSERT(p_FiltItem);

	// Draw the icons
	POINT pt;
	pt.x = rect.left;
	pt.y = rect.top;
	CFiltersViewRight *pRightView = (CFiltersViewRight *)GetParent();
	ASSERT_KINDOF(CFiltersViewRight, pRightView);

	rect.left = pRightView->m_ActionMenu.DrawIcons(pDC, pt, FALSE, p_FiltItem);
	

	// Draw the text
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(ForeColor);
	pDC->DrawText(p_FiltItem->GetItemText(), -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER );
}

	


/////////////////////////////////////////////////////////////////////////////
void StripAccel(CString &str) 
{
	int j = str.Find('&');
	if (j >= 0)
	{
		for(;j+1 < str.GetLength(); j++)
			str.SetAt(j,str.GetAt(j+1));
		str.SetAt(j,' ');
	}
}


