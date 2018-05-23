// TaskErrorView.cpp : implementation file

#include "stdafx.h"
#include "resource.h"
#include "EudoraMsgs.h"

#include "ListCtrlEx.h"
#include "3dformv.h"
#include "rs.h"
#include "WazooWnd.h"
#include "mainfrm.h"
#include "eudora.h"
#include "QCTaskManager.h"
#include "TaskErrorDlg.h"

#include "TaskErrorView.h"

// Debug Memory Manager block
// This should be after all system includes and before any user includes
#ifdef _DEBUG 
	#undef THIS_FILE 
	static char THIS_FILE[] = __FILE__; 
	#define new DEBUG_NEW 
#endif // _DEBUG

// --------------------------------------------------------------------------

// #define _TRACE_MSGS_

CTaskErrorView* g_TaskErrorView = NULL;

const int TaskErrorBitmapWidth = 16;

// --------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CTaskErrorView, C3DFormView)

BEGIN_MESSAGE_MAP(CTaskErrorView, C3DFormView)
	//{{AFX_MSG_MAP(CTaskErrorView)
	ON_REGISTERED_MESSAGE(umsgActivateWazoo, OnActivateWazoo)
	ON_WM_SIZE()
	ON_MESSAGE(msgErrorViewNewError, OnMsgNewError)
	ON_MESSAGE(msgErrorViewShowErrDlg, OnMsgShowDlg)
	ON_MESSAGE(msgErrorViewDelError, OnMsgDelError)
	ON_MESSAGE(msgErrorViewRemoveAll, OnMsgRemoveAll)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_LBtnDblClk, OnMsgListDblClk)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_DeleteKey, OnMsgListDeleteKey)
	ON_REGISTERED_MESSAGE(msgListCtrlEx_ReturnKey, OnMsgListReturnKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

CTaskErrorView* QCGetTaskErrorView()
{
	ASSERT(g_TaskErrorView != NULL);
	return g_TaskErrorView;
}

// --------------------------------------------------------------------------

CTaskErrorView::CTaskErrorView()
	: C3DFormView(CTaskErrorView::IDD)
{
	ASSERT(g_TaskErrorView == NULL);	// Should be only one of these
	g_TaskErrorView = this;
	
	m_nOldCX = m_nOldCY = 0;
	m_bInitilized = false;

	m_bDlgShown = false;
	m_bErrorStatus = false;
}

// --------------------------------------------------------------------------

CTaskErrorView::~CTaskErrorView()
{
}

// --------------------------------------------------------------------------

LONG CTaskErrorView::OnMsgListDeleteKey(WPARAM wParam, LPARAM lParam)
{
	const UINT nRepCnt = (const UINT) wParam;
	const CListCtrlEx *pList = (const CListCtrlEx *) lParam;

	if (pList == (&m_ErrorList))
	{
		list<int> SelList;
		if (!m_ErrorList.GetCurSel(&SelList))
			return (0);

		if (SelList.empty())
			return (0);

		int nFirstDel = *(SelList.begin());

		//first delete the errors from TaskManager
		unsigned int nUID = 0;

		list<int>::iterator LI;
		for(LI = SelList.begin(); LI != SelList.end(); ++LI)
		{
			int nIdx = *LI;
			nUID = (unsigned int) m_ErrorList.GetItemData(nIdx);

			// Tell the task manager to remove this error
			VERIFY(QCGetTaskManager()->RemoveTaskError(nUID));
		}

		VERIFY(m_ErrorList.DeleteItems(SelList));


		if (m_ErrorList.GetItemCount() <= nFirstDel)
			nFirstDel = (m_ErrorList.GetItemCount() - 1);

		m_ErrorList.SetCurSel(nFirstDel);

		UpdateStatusBar();
	}

	return (0);
}

// --------------------------------------------------------------------------

LONG CTaskErrorView::OnMsgListReturnKey(WPARAM wParam, LPARAM lParam)
{
	const UINT nRepCnt = (const UINT) wParam;
	const CListCtrlEx *pList = (const CListCtrlEx *) lParam;

	if (pList == (&m_ErrorList))
	{
		list<int> SelList;
		if (!m_ErrorList.GetCurSel(&SelList))
			return (0);

		if (SelList.size() != 1)
		{
			MessageBeep(MB_ICONASTERISK);
			return (0);
		}

		int nIdx = *(SelList.begin());
		unsigned int nUID = (unsigned int) m_ErrorList.GetItemData(nIdx);

		CTaskErrorMT *pError = QCGetTaskManager()->GetTaskError(nUID);
		ASSERT(pError);
		if (!pError)
			return (0);

		ShowErrorDlg(pError);
	}

	return (0);
}

// --------------------------------------------------------------------------

LONG CTaskErrorView::OnMsgListDblClk(WPARAM wParam, LPARAM lParam)
{
	UINT uFlags = (UINT) wParam; // MK_CONTROL, MK_LBUTTON, MK_MBUTTON, MK_RBUTTON, MK_SHIFT
	CPoint point((DWORD)lParam); // screen-based point of click

	CPoint clientPt = point;
	m_ErrorList.ScreenToClient(&clientPt);
	int idx = m_ErrorList.HitTest(clientPt);
	ASSERT(idx != (-1));
	if (idx == (-1))
		return (0);

	unsigned int nUID = (unsigned int) m_ErrorList.GetItemData(idx);
	CTaskErrorMT *pError = QCGetTaskManager()->GetTaskError(nUID);
	ASSERT(pError);
	if (!pError)
		return (0);

	ShowErrorDlg(pError);

	return (0); // Ignored
}

// --------------------------------------------------------------------------

LONG CTaskErrorView::OnMsgNewError( WPARAM wParam, LPARAM lParam ) // NEW ERROR
{
	CTaskErrorMT *pError = (CTaskErrorMT *) wParam;
	ASSERT(pError);
	unsigned int nUID = (unsigned int) lParam;

#ifdef _TRACE_MSGS_
	TRACE( "CTaskErrorView: Message 'msgErrorViewNewError' received, UID = %u\n", nUID );
#endif // _TRACE_MSGS_

	CTime t = pError->GetTimeStamp();
	CString timeStr = t.Format("%I:%M:%S %p");

	CString str;
	str = CString(pError->GetPersona()) + CString(", ");
	str += CString(pError->GetMainText()) + CString(", ");
	str += CString(pError->GetSecondaryText()) + CString(" [") + timeStr + CString("]");
	str += CString("\n");

	CString ErrStr = pError->GetErrText();
	int len = ErrStr.GetLength();
	char *cp = new char[len + 1];
	char *pmem = cp;
	const char *pErr = (LPCSTR) ErrStr.GetBuffer(0);
	
	char ch;
	while (len--)
	{
		ch = *(pErr++);
		if (isprint((int)(unsigned char)ch))
			*(cp++) = ch;
		else
			*(cp++) = ' ';
	}

	*cp = '\0';

	str += CString(pmem);
	delete[] pmem;

	int nIdx = m_ErrorList.InsertItem(0, "");
	m_ErrorList.SetItemData(nIdx, DWORD(nUID));

	m_ErrorList.SetItemText( nIdx, COL_ICON, "0"); // Only one icon, hard-code the index text
	m_ErrorList.SetItemText( nIdx, COL_TEXT, LPCSTR(str));

	UpdateStatusBar();

	if ((GetIniShort(IDS_INI_TASK_SHOW_ERR_DLG)) && (!m_bDlgShown))
		ShowErrorDlg(pError);
	
	if (GetIniShort(IDS_INI_TASK_ERR_BRINGTOFRONT))
		SwitchAndHighlight(nIdx);

	return (0); // Not used
}

// --------------------------------------------------------------------------

void CTaskErrorView::SwitchAndHighlight(unsigned int nIdx)
{
	CMainFrame::QCGetMainFrame()->SendMessage(WM_COMMAND, IDM_VIEW_TASK_ERROR);
	SetFocus();

	if (m_ErrorList.GetItemCount() > ((int) nIdx))
		m_ErrorList.SetCurSel(nIdx);
}

// --------------------------------------------------------------------------

int CTaskErrorView::FindTaskIndex(unsigned int UID)
{
	LV_FINDINFO FindInfo = { LVFI_PARAM, NULL, (LPARAM) UID };
	return (m_ErrorList.FindItem(&FindInfo));
}

////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CTaskErrorView::OnInitialUpdate()
{
	if (m_bInitilized)
		return; // guard against bogus double initializations
	
	C3DFormView::OnInitialUpdate();

	// Frame window needs to be just big enough to hold the view
	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
	OnSize(SIZE_RESTORED, -1, -1);

	//
	// As a hack, trick the parent Wazoo window into properly saving
	// the initial control focus for this form.
	//
//	CWazooWnd::SaveInitialChildFocus(&m_TasksList);

	m_DlgMargin = 0; // margin (border) size, in pixels

	QCGetTaskManager()->SetDisplayWindow(CTaskObjectMT::TOBJ_ERROR, this);

	m_ErrorList.SetHorizGrid(true);
	m_ErrorList.SetLinePerRow(GetIniShort(IDS_INI_TASKERROR_LINESPERROW));

	m_ErrorList.SetHighlightType(LVEX_HIGHLIGHT_ROW);
	
	m_ErrorList.InsertColumn(COL_ICON, "Ignored", LVCFMT_CENTER, TaskErrorBitmapWidth + 4);
	m_ErrorList.InsertColumn(COL_TEXT, "Ignored", LVCFMT_LEFT, 100);

	CImageList *pImageList = new CImageList;
	ASSERT(pImageList);
	VERIFY(pImageList->Create(IDB_IL_TASK_ERROR, TaskErrorBitmapWidth, 0, RGB(128,0,0)));

	m_ErrorList.SetColImageList(COL_ICON, pImageList);
	m_ErrorList.SetColDisplayType(COL_ICON, LVEX_DISPLAY_IMAGELIST);
	m_ErrorList.SetColumnAlignment(COL_ICON, LVCFMT_CENTER);

	m_bInitilized = true;
}

// --------------------------------------------------------------------------

void CTaskErrorView::DoDataExchange(CDataExchange* pDX)
{
	C3DFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskErrorView)
	DDX_Control(pDX, IDC_TASKERR_ERRLIST, m_ErrorList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorView diagnostics

#ifdef _DEBUG
void CTaskErrorView::AssertValid() const
{
	C3DFormView::AssertValid();
}

void CTaskErrorView::Dump(CDumpContext& dc) const
{
	C3DFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTaskErrorView message handlers


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [protected, virtual]
//
// Perform the Just In Time display initialization for this Wazoo.
////////////////////////////////////////////////////////////////////////
long CTaskErrorView::OnActivateWazoo(WPARAM, LPARAM)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CTaskErrorView::OnSize(UINT nType, int cx, int cy)
{
	C3DFormView::OnSize(nType, cx, cy);

	// Don't bother if the controls haven't been initialized,
	// or if we're being minimized...
	if (!m_bInitilized)
		return;

	if ((cx != m_nOldCX) || (cy != m_nOldCY))
	{
		CRect TaskListRct;	
		m_ErrorList.GetWindowRect(TaskListRct);
		ScreenToClient(TaskListRct);
		TaskListRct.SetRect(m_DlgMargin, m_DlgMargin, (cx - m_DlgMargin), (cy - m_DlgMargin));
		m_ErrorList.MoveWindow(TaskListRct);

		if (cx != m_nOldCX)
		{
			LV_COLUMN lvc = { 0,0,0,0,0,0 };
			lvc.mask = LVCF_WIDTH;

			VERIFY(m_ErrorList.GetColumn(COL_ICON, &lvc));
			int nIconWidth = lvc.cx;
			VERIFY(m_ErrorList.GetColumn(COL_TEXT, &lvc));
			lvc.cx = (TaskListRct.Width() - nIconWidth);

			if (lvc.cx < 10)
				lvc.cx = 10;

			VERIFY(m_ErrorList.SetColumn(COL_TEXT, &lvc));
		}

	}

	m_nOldCX = cx;
	m_nOldCY = cy;
}

void CTaskErrorView::ShowErrorDlg(CTaskErrorMT *pError)
{
	ASSERT(pError);
	if (!pError)
		return;

	if (m_bDlgShown) // Only allow one dlg at a time
		return;

	//Show the error context. Usually does nothing, but shows the message for per-msg errors
	//such as Bad Receipient
	pError->ShowContext();

	CTaskErrorDlg dlg(pError);
	m_bDlgShown = true;
	dlg.DoModal();
	m_bDlgShown = false;

	// Check if we should remove the error from our list
	if (GetIniShort(IDS_INI_TASKERROR_REMOVEFROMLIST))
	{
		// Take this error out of our list
		unsigned int nUID = pError->GetUID();
		int nIdx = FindTaskIndex(nUID);
		
		ASSERT(nIdx >= 0); // Should always be able to find this error

		if (nIdx >= 0)
			VERIFY(m_ErrorList.DeleteItem(nIdx));

		// Tell the task manager to remove this error
		VERIFY(QCGetTaskManager()->RemoveTaskError(nUID));

		UpdateStatusBar();
	}
}


LONG CTaskErrorView::OnMsgShowDlg( WPARAM, LPARAM )
{
	SwitchAndHighlight(0);

	if (m_ErrorList.GetItemCount() > 0)
	{
		unsigned int nUID = (unsigned int) m_ErrorList.GetItemData(0); // Get latest (top) error
		CTaskErrorMT *pError = QCGetTaskManager()->GetTaskError(nUID);
		
		ASSERT(pError);

		if (pError)
			ShowErrorDlg(pError);
	}

	return (0); // Ignored
}

LONG CTaskErrorView::OnMsgDelError(WPARAM wParam, LPARAM lParam)
{
	if (GetIniShort(IDS_INI_TASK_ERR_KEEPALL) != 0)
		return (0);

	CTaskErrorMT *pError = (CTaskErrorMT *) wParam;
	ASSERT(pError);
	unsigned int nUID = (unsigned int) lParam;

	int nIdx = FindTaskIndex(nUID);
		
	ASSERT(nIdx >= 0); // Should always be able to find this error

	if (nIdx >= 0)
		VERIFY(m_ErrorList.DeleteItem(nIdx));


	VERIFY(QCGetTaskManager()->RemoveTaskError(nUID));
	UpdateStatusBar();

	return (0); // Ignored
}

LONG CTaskErrorView::OnMsgRemoveAll( WPARAM, LPARAM )
{
	int nCount = m_ErrorList.GetItemCount();

	if (nCount < 1)
		return (0);
	
	unsigned int nUID = 0;

	for (int nIdx = 0; nIdx < nCount; nIdx++)
	{
		nUID = (unsigned int) m_ErrorList.GetItemData(nIdx);

		// Tell the task manager to remove this error
		VERIFY(QCGetTaskManager()->RemoveTaskError(nUID));
	}

	m_ErrorList.DeleteAllItems();
	UpdateStatusBar();

	return (0); // Ignored
}

void CTaskErrorView::UpdateStatusBar()
{
	const bool bErr = (m_ErrorList.GetItemCount() > 0);
	// m_bErrorStatus

	if (bErr != m_bErrorStatus)
	{
		m_bErrorStatus = bErr;

		CStatusBarEx *pStatusBar = CStatusBarEx::QCGetStatusBar();
		ASSERT(pStatusBar);
		if (pStatusBar)
			pStatusBar->PostMessage(msgStatusBarError, (WPARAM) m_bErrorStatus);
	}
}
