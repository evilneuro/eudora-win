// TaskStatusView.cpp : implementation file

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
#include "StatBar.h"
#include "TaskStatus.h"
#include "TaskSummary.h"
#include "persona.h" // g_Personalities

#include "TaskStatusView.h"

// Debug Memory Manager block
// This should be after all system includes and before any user includes
#ifdef _DEBUG 
	#undef THIS_FILE 
	static char THIS_FILE[] = __FILE__; 
	#define new DEBUG_NEW 
#endif // _DEBUG

// --------------------------------------------------------------------------

// #define _TRACE_MSGS_

CTaskStatusView* g_TaskStatusView = NULL;


// --------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CTaskStatusView, C3DFormView)

BEGIN_MESSAGE_MAP(CTaskStatusView, C3DFormView)
	//{{AFX_MSG_MAP(CTaskStatusView)
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(umsgActivateWazoo, OnActivateWazoo)
	ON_WM_SIZE()
	ON_MESSAGE( msgTaskViewInfoAlive, OnMsgNewInfo )
	ON_MESSAGE( msgTaskViewInfoChanged, OnMsgChangeInfo )
	ON_MESSAGE( msgTaskViewInfoDead, OnMsgDeleteInfo )
	ON_MESSAGE( msgTaskViewUpdateCols, OnMsgUpdateCols )
	ON_REGISTERED_MESSAGE( msgListCtrlEx_RBtn, OnMsgListRBtn )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

CTaskStatusView* QCGetTaskStatusView()
{
	ASSERT(g_TaskStatusView != NULL);
	return g_TaskStatusView;
}

// --------------------------------------------------------------------------

CTaskStatusView::CTaskStatusView()
	: C3DFormView(CTaskStatusView::IDD)
{
	ASSERT(g_TaskStatusView == NULL);	// Should be only one of these
	g_TaskStatusView = this;
	
	m_nOldCX = m_nOldCY = 0;
	//m_IntStrCache = NULL;
	InitCacheIntStrs();

	m_bInitilized = false;

	m_pColPosition[COL_STATE] = m_pColPosition[COL_PERSONA] = m_pColPosition[COL_STATUS] = m_pColPosition[COL_DETAILS] = m_pColPosition[COL_PROGRESS] = (-1);
}

// --------------------------------------------------------------------------

CTaskStatusView::~CTaskStatusView()
{
	/*if (m_IntStrCache)
	{
		for (int i=0; i<101; i++)
			delete[] m_IntStrCache[i];
		delete[] m_IntStrCache;
	}*/
}

// --------------------------------------------------------------------------

int CTaskStatusView::GetImageIndex(TaskStatusState nState, TaskType nType)
{
	int nIdx = TIL_UNKNOWN;

	switch (nType)
	{
		case TASK_TYPE_SENDING:
		{
			switch (nState)
			{
				case TSS_CREATED:			nIdx = TIL_CREATED; break;
				case TSS_WAITING_TO_QUEUE:  nIdx = TIL_SEND_QUEUED; break;
				case TSS_QUEUED:			nIdx = TIL_SEND_QUEUED; break;
				case TSS_RUNNING:			nIdx = TIL_SEND_RUNNING; break;
				case TSS_COMPLETE:			nIdx = TIL_COMPLETE; break;
			}
		}
		break;

		case TASK_TYPE_RECEIVING:
		{
			switch (nState)
			{
				case TSS_CREATED:			nIdx = TIL_CREATED; break;
				case TSS_WAITING_TO_QUEUE:	nIdx = TIL_REC_QUEUED; break;
				case TSS_QUEUED:			nIdx = TIL_REC_QUEUED; break;
				case TSS_RUNNING:			nIdx = TIL_REC_RUNNING; break;
				case TSS_COMPLETE:			nIdx = TIL_COMPLETE; break;
			}
		}
		break;
	}

	return (nIdx);
}

// --------------------------------------------------------------------------

bool CTaskStatusView::UpdateFullListInfo()
{
	const int nCount = m_TaskList.GetItemCount();

	if (nCount < 1)
		return (true); // Nothing to update

	CTaskSummary *pTaskSum = NULL;
	CTaskInfoMT *pInfo = NULL;
	unsigned int nUID = 0;

	for (int idx = 0; idx < nCount; idx++)
	{
		pTaskSum = (CTaskSummary *) m_TaskList.GetItemData(idx);
		ASSERT(pTaskSum);
		if (!pTaskSum)
			continue;

		nUID = (unsigned int) pTaskSum->m_UID;
		pInfo = QCGetTaskManager()->GetTaskInfo(nUID);
		ASSERT(pInfo);
		if (!pInfo)
			continue;

		UpdateListInfo(idx, pInfo);
	}

	return (true);
}

// --------------------------------------------------------------------------

bool CTaskStatusView::UpdateListInfo(unsigned int nIdx, CTaskInfoMT *pInfo)
{
	ASSERT(pInfo);

	if (!pInfo)
		return (false);


	if (m_pColPosition[COL_STATE] != (-1))
	{
		const int nImageIdx = GetImageIndex(pInfo->GetState(), pInfo->GetTaskType());
		const char *sIntStr = GetCacheIntStr(nImageIdx);
		
		const CString txt = CString(sIntStr) + CString(',') + pInfo->GetTitle();
		m_TaskList.SetItemText( nIdx, m_pColPosition[COL_STATE], txt);
	}

	if (m_pColPosition[COL_PERSONA] != (-1))
		m_TaskList.SetItemText( nIdx, m_pColPosition[COL_PERSONA], pInfo->GetPersona());

	if (m_pColPosition[COL_STATUS] != (-1))
		m_TaskList.SetItemText( nIdx, m_pColPosition[COL_STATUS], pInfo->GetMainText());

	if (m_pColPosition[COL_DETAILS] != (-1))
		m_TaskList.SetItemText( nIdx, m_pColPosition[COL_DETAILS], pInfo->GetSecondaryText());

	if (m_pColPosition[COL_PROGRESS] != (-1))
	{
		switch (pInfo->GetTaskType())
		{
			case TASK_TYPE_RECEIVING:
			{
				// Use default coloring
				CString sPct = GetCacheIntStr(pInfo->GetPercentDone());

				if (!sPct.IsEmpty())
					sPct += m_sRecvSuffix;

				m_TaskList.SetItemText( nIdx, m_pColPosition[COL_PROGRESS], ((LPCTSTR)sPct) );
			}
			break;

			default:
			case TASK_TYPE_SENDING:
			{
				// Use default coloring
				const char *sIntStr = GetCacheIntStr(pInfo->GetPercentDone());
				m_TaskList.SetItemText( nIdx, m_pColPosition[COL_PROGRESS], sIntStr);
			}
			break;
		}
	}

	CTaskSummary *pTaskSum = (CTaskSummary *) m_TaskList.GetItemData(nIdx);
	
	if (pTaskSum)
	{
		if (pTaskSum->SetInfo(pInfo))
			NotifyStatusBar();
	}


	return (true);
}

// --------------------------------------------------------------------------

void CTaskStatusView::NotifyStatusBar()
{
	CStatusBarEx *pStatusBar = CStatusBarEx::QCGetStatusBar();
	ASSERT(pStatusBar);

	if (pStatusBar)
	{
		const int nCount = m_TaskList.GetItemCount();
		int idx;
		CTaskSummary *pTaskSum = NULL;
		CTaskStatus ts;

		for (idx=0; (idx < nCount); idx++)
		{
			if (pTaskSum = (CTaskSummary *) m_TaskList.GetItemData(idx))
			{
				ts.ProcessSummary(pTaskSum);
			}
		}

		if ((m_TheStatus.IsDifferent(ts)) || (ts.GetPercent() == 0) || (ts.GetPercent() == 100))
		{
			m_TheStatus = ts;
			pStatusBar->SendMessage(msgStatusBarTask, (WPARAM)&m_TheStatus);
		}
	}
}

// --------------------------------------------------------------------------

LONG CTaskStatusView::OnMsgNewInfo( WPARAM wParam, LPARAM lParam ) // ADD
{
	CTaskInfoMT *pInfo = (CTaskInfoMT *) wParam;
	ASSERT(pInfo);
	unsigned int nUID = (unsigned int) lParam;

#ifdef _TRACE_MSGS_
	TRACE( "CTaskStatusView: Message 'msgTaskViewInfoAlive' received, UID = %u\n", nUID );
#endif // _TRACE_MSGS_

	VERIFY(FindTaskIndex(nUID) == -1); // Make sure we dont already have an item with this UID

	int idx = m_TaskList.InsertItem(0, "");
	CTaskSummary *pTaskSum = new CTaskSummary(pInfo);
	ASSERT(pTaskSum);

	if (pTaskSum)
	{
		m_TaskList.SetItemData(idx, DWORD(pTaskSum));
		UpdateListInfo(idx, pInfo);
		NotifyStatusBar();
	}

	if (GetIniShort(IDS_INI_TASK_STATUS_BRINGTOFRONT))
		SwitchAndHighlight(idx);

	return (0); // Not used
}

// --------------------------------------------------------------------------

void CTaskStatusView::SwitchAndHighlight(unsigned int nIdx)
{
	CMainFrame::QCGetMainFrame()->SendMessage(WM_COMMAND, IDM_VIEW_TASK_STATUS);
	SetFocus();

	if (m_TaskList.GetItemCount() > ((int) nIdx))
		m_TaskList.SetCurSel(nIdx);
}

// --------------------------------------------------------------------------

LONG CTaskStatusView::OnMsgChangeInfo( WPARAM wParam, LPARAM lParam ) // CHANGE
{
	const unsigned int nUID = (unsigned int) lParam;
	CTaskInfoMT *pInfo = QCGetTaskManager()->GetTaskInfo(nUID);

	if (pInfo)
	{
#ifdef _TRACE_MSGS_
		TRACE( "CTaskStatusView: Message 'msgTaskViewInfoChanged' received, UID = %u [pct=%d, persona=\"%s\", tasktype=%d, maintext=\"%s\", sectext\"%s\", total=%d, sofar=%d, state=%d, title=\"%s\"]\n",
			nUID,
			(int) pInfo->GetPercentDone(),
			(LPCSTR) pInfo->GetPersona(),
			(int) pInfo->GetTaskType(),
			(LPCSTR) pInfo->GetMainText(),
			(LPCSTR) pInfo->GetSecondaryText(),
			(int) pInfo->GetTotal(),
			(int) pInfo->GetSoFar(),
			(int) pInfo->GetState(),
			(LPCSTR) pInfo->GetTitle());
#endif // _TRACE_MSGS_

		int idx = FindTaskIndex(nUID);
		ASSERT(idx != (-1));

		if (idx != (-1))
		{
			UpdateListInfo(idx, pInfo);
//			NotifyStatusBar();
		}
	}

	return (0); // Not used
}

// --------------------------------------------------------------------------

LONG CTaskStatusView::OnMsgDeleteInfo( WPARAM wParam, LPARAM lParam ) // DELETE
{
	CTaskInfoMT *pInfo = (CTaskInfoMT *) wParam;
	ASSERT(pInfo);
	if (!pInfo)
		return (0);

	unsigned int nUID = (unsigned int) lParam;

#ifdef _TRACE_MSGS_
	TRACE( "CTaskStatusView: Message 'msgTaskViewInfoDead' received, UID = %u\n", nUID );
#endif // _TRACE_MSGS_

	int idx = FindTaskIndex(nUID);
	ASSERT(idx != (-1));
	if (idx == (-1))
		return (0);

	CTaskSummary *pTaskSum = (CTaskSummary *) m_TaskList.GetItemData(idx);
	if (pTaskSum)
		delete pTaskSum;

	m_TaskList.DeleteItem(idx);
	VERIFY(QCGetTaskManager()->RemoveTaskInfo(nUID)); // It should always be in the list, so this shouldn't fail

	NotifyStatusBar();

	return (0); // Not used
}

// --------------------------------------------------------------------------

LONG CTaskStatusView::OnMsgListRBtn( WPARAM wParam, LPARAM lParam ) // Right-click in list
{
	UINT uFlags = (UINT) wParam; // MK_CONTROL, MK_LBUTTON, MK_MBUTTON, MK_RBUTTON, MK_SHIFT
	CPoint point((DWORD)lParam); // screen-based point of click

	CPoint clientPt = point;
	m_TaskList.ScreenToClient(&clientPt);
	int idx = m_TaskList.HitTest(clientPt);
	
	ASSERT(idx != (-1));
	if (idx == (-1))
		return (0);

	CTaskSummary *pTaskSum = (CTaskSummary *) m_TaskList.GetItemData(idx);
	ASSERT(pTaskSum);
	if (!pTaskSum)
		return (0);

	unsigned int nUID = (unsigned int) pTaskSum->m_UID;
	CTaskInfoMT *pInfo = QCGetTaskManager()->GetTaskInfo(nUID);

	if (!pInfo)
		return (0);

	CMenu menu;
	menu.CreatePopupMenu();

	const UINT nMENUID_PostProc = 1;
	const UINT nMENUID_Stop = 2;

	CString PostStr;
	pInfo->GetPostString(PostStr);

	if (PostStr.IsEmpty())
		PostStr = CRString(IDS_TASK_STATUS_RMENU_NOTHING_TXT);

	menu.AppendMenu(MF_STRING, nMENUID_PostProc, LPCSTR(PostStr));
	menu.AppendMenu(MF_SEPARATOR);

	if (!pInfo->NeedsPostProcessing())
		menu.EnableMenuItem(nMENUID_PostProc, MF_BYCOMMAND  | MF_GRAYED);

	menu.AppendMenu(MF_STRING, nMENUID_Stop, CRString(IDS_TASK_STATUS_RMENU_STOP));

	DWORD sel = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON
					| TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, this);

	switch (sel)
	{
		case nMENUID_PostProc:
		{
			QCGetTaskManager()->RequestPostProcessing(pInfo);
		}
		break;

		case nMENUID_Stop:
		{
			pInfo->RequestThreadStop();
		}
		break;
	}

	menu.DestroyMenu();

	return (0); // Not used
}

// --------------------------------------------------------------------------

int CTaskStatusView::FindTaskIndex(unsigned int nUID)
{
	const int nCount = m_TaskList.GetItemCount();
	int idx;
	CTaskSummary *pTaskSum = NULL;

	for (idx = 0; idx < nCount; idx++)
	{
		pTaskSum = (CTaskSummary *) m_TaskList.GetItemData(idx);
		if (pTaskSum)
		{
			if (pTaskSum->m_UID == nUID)
				return (idx);
		}
	}

	return (-1);
}

// --------------------------------------------------------------------------

LONG CTaskStatusView::OnMsgUpdateCols( WPARAM wParam, LPARAM lParam ) // UpdateCols
{
	UpdateColumns();
	return (0);
}

// --------------------------------------------------------------------------

void CTaskStatusView::DoDeletedCol(const int idx)
{
	if (m_pColPosition[COL_STATE] > idx)
		m_pColPosition[COL_STATE]--;
	
	if (m_pColPosition[COL_PERSONA] > idx)
		m_pColPosition[COL_PERSONA]--;
	
	if (m_pColPosition[COL_STATUS] > idx)
		m_pColPosition[COL_STATUS]--;
	
	if (m_pColPosition[COL_DETAILS] > idx)
		m_pColPosition[COL_DETAILS]--;

	if (m_pColPosition[COL_PROGRESS] > idx)
		m_pColPosition[COL_PROGRESS]--;
}

// --------------------------------------------------------------------------

void CTaskStatusView::DoInsertedCol(const int idx)
{
	if (m_pColPosition[COL_STATE] >= idx)
		m_pColPosition[COL_STATE]++;
	
	if (m_pColPosition[COL_PERSONA] >= idx)
		m_pColPosition[COL_PERSONA]++;
	
	if (m_pColPosition[COL_STATUS] >= idx)
		m_pColPosition[COL_STATUS]++;
	
	if (m_pColPosition[COL_DETAILS] >= idx)
		m_pColPosition[COL_DETAILS]++;

	if (m_pColPosition[COL_PROGRESS] >= idx)
		m_pColPosition[COL_PROGRESS]++;
}

// --------------------------------------------------------------------------

int CTaskStatusView::FindInsertIndex(int nColID)
{
	// Find first visible col before this one
	for (int i = (nColID - 1); i >= 0; i--)
	{
		if (m_pColPosition[i] != (-1))
			return (m_pColPosition[i] + 1);
	}

	return (0); // No col to left, so it will be the first col
}

// --------------------------------------------------------------------------

void CTaskStatusView::UpdateColumns()
{
	bool bState = (GetIniInt(IDS_INI_TASKSTATUS_SHOWCOL_TASK) != 0);
	bool bPersona = ((g_Personalities.GetPersonaCount() > 1) && (GetIniInt(IDS_INI_TASKSTATUS_SHOWCOL_PERSONA) != 0));
	bool bStatus = (GetIniInt(IDS_INI_TASKSTATUS_SHOWCOL_STATUS) != 0);
	bool bDetails = (GetIniInt(IDS_INI_TASKSTATUS_SHOWCOL_DETAILS) != 0);
	bool bProgress = (GetIniInt(IDS_INI_TASKSTATUS_SHOWCOL_PROGRESS) != 0);

	bool bChangedCols = false;

	if (bState != (m_pColPosition[COL_STATE] != (-1))) // STATE (ICON w/ text)
	{
		bChangedCols = true;

		if (bState)
		{
			// Adding the column
			const int idx = m_TaskList.InsertColumn(FindInsertIndex(COL_STATE), CRString(IDS_TASK_STATUS_LIST_HDRTXT_STATE), LVCFMT_LEFT, GetIniShort(IDS_INI_TASKSTATUS_STATE_WIDTH));
			m_TaskList.NotifyInsertedCol(idx);
			DoInsertedCol(idx);
			m_pColPosition[COL_STATE] = idx;

			CImageList *pImageList = new CImageList;
			ASSERT(pImageList);
			
			if (pImageList)
			{
				VERIFY(pImageList->Create(IDB_IL_TASK_STATUS, 16, 0, RGB(231,231,214)));
				m_TaskList.SetColImageList(m_pColPosition[COL_STATE], pImageList);
			}
		}
		else
		{
			// Removing the column
			LV_COLUMN col;
			col.mask = LVCF_WIDTH;

			m_TaskList.GetColumn(m_pColPosition[COL_STATE], &col);
			SetIniShort(IDS_INI_TASKSTATUS_STATE_WIDTH, short(col.cx));

			m_TaskList.SetColImageList(m_pColPosition[COL_STATE], NULL);
			VERIFY(m_TaskList.DeleteColumn(m_pColPosition[COL_STATE]));
			m_TaskList.NotifyDeletedCol(m_pColPosition[COL_STATE]);
			DoDeletedCol(m_pColPosition[COL_STATE]);
			m_pColPosition[COL_STATE] = (-1);
		}
	}

	if (bPersona != (m_pColPosition[COL_PERSONA] != (-1))) // PERSONA
	{
		bChangedCols = true;

		if (bPersona)
		{
			// Adding the column
			const int idx = m_TaskList.InsertColumn(FindInsertIndex(COL_PERSONA),  CRString(IDS_TASK_STATUS_LIST_HDRTXT_PERSONA),  LVCFMT_LEFT, GetIniShort(IDS_INI_TASKSTATUS_PERSONA_WIDTH));
			m_TaskList.NotifyInsertedCol(idx);
			DoInsertedCol(idx);
			m_pColPosition[COL_PERSONA] = idx;
		}
		else
		{
			// Removing the column
			LV_COLUMN col;
			col.mask = LVCF_WIDTH;

			m_TaskList.GetColumn(m_pColPosition[COL_PERSONA], &col);
			SetIniShort(IDS_INI_TASKSTATUS_PERSONA_WIDTH, short(col.cx));

			VERIFY(m_TaskList.DeleteColumn(m_pColPosition[COL_PERSONA]));
			m_TaskList.NotifyDeletedCol(m_pColPosition[COL_PERSONA]);
			DoDeletedCol(m_pColPosition[COL_PERSONA]);
			m_pColPosition[COL_PERSONA] = (-1);
		}
	}

	if (bStatus != (m_pColPosition[COL_STATUS] != (-1))) // STATUS
	{
		bChangedCols = true;

		if (bStatus)
		{
			// Adding the column
			const int idx = m_TaskList.InsertColumn(FindInsertIndex(COL_STATUS),   CRString(IDS_TASK_STATUS_LIST_HDRTXT_STATUS),   LVCFMT_LEFT, GetIniShort(IDS_INI_TASKSTATUS_STATUS_WIDTH));
			m_TaskList.NotifyInsertedCol(idx);
			DoInsertedCol(idx);
			m_pColPosition[COL_STATUS] = idx;
		}
		else
		{
			// Removing the column
			LV_COLUMN col;
			col.mask = LVCF_WIDTH;

			m_TaskList.GetColumn(m_pColPosition[COL_STATUS], &col);
			SetIniShort(IDS_INI_TASKSTATUS_STATUS_WIDTH, short(col.cx));

			VERIFY(m_TaskList.DeleteColumn(m_pColPosition[COL_STATUS]));
			m_TaskList.NotifyDeletedCol(m_pColPosition[COL_STATUS]);
			DoDeletedCol(m_pColPosition[COL_STATUS]);
			m_pColPosition[COL_STATUS] = (-1);
		}
	}

	if (bDetails != (m_pColPosition[COL_DETAILS] != (-1))) // DETAILS
	{
		bChangedCols = true;

		if (bDetails)
		{
			// Adding the column
			const int idx = m_TaskList.InsertColumn(FindInsertIndex(COL_DETAILS),   CRString(IDS_TASK_STATUS_LIST_HDRTXT_DETAILS),   LVCFMT_LEFT, GetIniShort(IDS_INI_TASKSTATUS_DETAILS_WIDTH));
			m_TaskList.NotifyInsertedCol(idx);
			DoInsertedCol(idx);
			m_pColPosition[COL_DETAILS] = idx;
		}
		else
		{
			// Removing the column
			LV_COLUMN col;
			col.mask = LVCF_WIDTH;

			m_TaskList.GetColumn(m_pColPosition[COL_DETAILS], &col);
			SetIniShort(IDS_INI_TASKSTATUS_DETAILS_WIDTH, short(col.cx));

			VERIFY(m_TaskList.DeleteColumn(m_pColPosition[COL_DETAILS]));
			m_TaskList.NotifyDeletedCol(m_pColPosition[COL_DETAILS]);
			DoDeletedCol(m_pColPosition[COL_DETAILS]);
			m_pColPosition[COL_DETAILS] = (-1);
		}
	}

	if (bProgress != (m_pColPosition[COL_PROGRESS] != (-1))) // DETAILS
	{
		bChangedCols = true;

		if (bProgress)
		{
			// Adding the column
			const int idx = m_TaskList.InsertColumn(FindInsertIndex(COL_PROGRESS), CRString(IDS_TASK_STATUS_LIST_HDRTXT_PROGRESS), LVCFMT_LEFT, GetIniShort(IDS_INI_TASKSTATUS_PROGRESS_WIDTH));
			m_TaskList.NotifyInsertedCol(idx);
			DoInsertedCol(idx);
			m_pColPosition[COL_PROGRESS] = idx;

			m_TaskList.SetColDisplayType(m_pColPosition[COL_PROGRESS], LVEX_DISPLAY_BARGRAPH); // Make progress column show the graph
		}
		else
		{
			// Removing the column
			LV_COLUMN col;
			col.mask = LVCF_WIDTH;

			m_TaskList.GetColumn(m_pColPosition[COL_PROGRESS], &col);
			SetIniShort(IDS_INI_TASKSTATUS_PROGRESS_WIDTH, short(col.cx));

			VERIFY(m_TaskList.DeleteColumn(m_pColPosition[COL_PROGRESS]));
			m_TaskList.NotifyDeletedCol(m_pColPosition[COL_PROGRESS]);
			DoDeletedCol(m_pColPosition[COL_PROGRESS]);
			m_pColPosition[COL_PROGRESS] = (-1);
		}
	}

	if (bChangedCols)
		UpdateFullListInfo();
}

void CTaskStatusView::OnInitialUpdate()
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

	m_DlgSpacing = LOWORD(GetDialogBaseUnits()) * 2; // spacing between controls, in pixels
	m_DlgMargin = 0; // margin (border) size, in pixels

	QCGetTaskManager()->SetDisplayWindow(CTaskObjectMT::TOBJ_INFO, this);

	m_TaskList.SetHighlightType(LVEX_HIGHLIGHT_ROW);

	COLORREF clr;
	if (GetIniColorRef(IDS_INI_TASKSTATUS_GRAPHCOMPLETEDCOLOR, clr))
		m_TaskList.SetGraphCompletedColor(clr);

	if (GetIniColorRef(IDS_INI_TASKSTATUS_GRAPHBORDERCOLOR, clr))
		m_TaskList.SetGraphBorderColor(clr);

	if (GetIniColorRef(IDS_INI_TASKSTATUS_GRAPHREMAINCOLOR, clr))
		m_TaskList.SetGraphRemainColor(clr);

	COLORREF m_RecvCompleteColor, m_RecvRemainColor;
	GetIniColorRef(IDS_INI_TASKSTATUS_RECV_COMPLETEDCOLOR, m_RecvCompleteColor);
	GetIniColorRef(IDS_INI_TASKSTATUS_RECV_REMAINCOLOR, m_RecvRemainColor);

	m_sRecvSuffix.Format(":%d,%d,%d:%d,%d,%d",
							GetRValue(m_RecvCompleteColor),
							GetGValue(m_RecvCompleteColor),
							GetBValue(m_RecvCompleteColor),
							GetRValue(m_RecvRemainColor),
							GetGValue(m_RecvRemainColor),
							GetBValue(m_RecvRemainColor) );

	//InitCacheIntStrs();
	UpdateColumns();

	m_bInitilized = true;
}

// --------------------------------------------------------------------------

const char *CTaskStatusView::GetCacheIntStr(int i)
{
	if ((m_IntStrCache) && ((i>= 0) && (i <= 100)))
		return (m_IntStrCache + i*4);
		//return (m_IntStrCache[i]);

	return ("");
}

// --------------------------------------------------------------------------

void CTaskStatusView::InitCacheIntStrs()
{
	CString str;
	//m_IntStrCache = new char *[101];

	for (int n=0; n<101; n++)
	{
		str.Format("%d", n);
		//m_IntStrCache[n] = new char[str.GetLength() + 1];
		strcpy(m_IntStrCache + n*4, (LPCSTR) str);
	}
}

// --------------------------------------------------------------------------

void CTaskStatusView::DoDataExchange(CDataExchange* pDX)
{
	C3DFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskStatusView)
	DDX_Control(pDX, IDC_TASKVIEW_TASKLIST, m_TaskList);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTaskStatusView diagnostics

#ifdef _DEBUG
void CTaskStatusView::AssertValid() const
{
	C3DFormView::AssertValid();
}

void CTaskStatusView::Dump(CDumpContext& dc) const
{
	C3DFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTaskStatusView message handlers


////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
// Do some cleanup when window is being destroyed.
////////////////////////////////////////////////////////////////////////
void CTaskStatusView::OnDestroy() 
{
	//
	// Save column widths to INI file.
	//
	ASSERT(::IsWindow(m_TaskList.GetSafeHwnd()));

	LV_COLUMN col;
	col.mask = LVCF_WIDTH;

	if (m_pColPosition[COL_STATE] != (-1))
	{
		m_TaskList.GetColumn(m_pColPosition[COL_STATE], &col);
		SetIniShort(IDS_INI_TASKSTATUS_STATE_WIDTH, short(col.cx));
	}

	if (m_pColPosition[COL_PERSONA] != (-1))
	{
		m_TaskList.GetColumn(m_pColPosition[COL_PERSONA], &col);
		SetIniShort(IDS_INI_TASKSTATUS_PERSONA_WIDTH, short(col.cx));
	}

	if (m_pColPosition[COL_STATUS] != (-1))
	{
		m_TaskList.GetColumn(m_pColPosition[COL_STATUS], &col);
		SetIniShort(IDS_INI_TASKSTATUS_STATUS_WIDTH, short(col.cx));
	}

	if (m_pColPosition[COL_DETAILS] != (-1))
	{
		m_TaskList.GetColumn(m_pColPosition[COL_DETAILS], &col);
		SetIniShort(IDS_INI_TASKSTATUS_DETAILS_WIDTH, short(col.cx));
	}

	if (m_pColPosition[COL_PROGRESS] != (-1))
	{
		m_TaskList.GetColumn(m_pColPosition[COL_PROGRESS], &col);
		SetIniShort(IDS_INI_TASKSTATUS_PROGRESS_WIDTH, short(col.cx));
	}

	C3DFormView::OnDestroy();
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [protected, virtual]
//
// Perform the Just In Time display initialization for this Wazoo.
////////////////////////////////////////////////////////////////////////
long CTaskStatusView::OnActivateWazoo(WPARAM, LPARAM)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CTaskStatusView::OnSize(UINT nType, int cx, int cy)
{
	C3DFormView::OnSize(nType, cx, cy);

	// Don't bother if the controls haven't been initialized,
	// or if we're being minimized...
	if (!m_bInitilized)
		return;

	if ((cx != m_nOldCX) || (cy != m_nOldCY))
	{
		CRect TaskListRct;	
		m_TaskList.GetWindowRect(TaskListRct);
		ScreenToClient(TaskListRct);
		TaskListRct.SetRect(m_DlgMargin, m_DlgMargin, (cx - m_DlgMargin), (cy - m_DlgMargin));
		m_TaskList.MoveWindow(TaskListRct);
	}

	m_nOldCX = cx;
	m_nOldCY = cy;
}

/*
IDS_INI_TASKSTATUS_SHOWCOL_TASK
IDS_INI_TASKSTATUS_SHOWCOL_PERSONA
IDS_INI_TASKSTATUS_SHOWCOL_STATUS
IDS_INI_TASKSTATUS_SHOWCOL_DETAILS
IDS_INI_TASKSTATUS_SHOWCOL_PROGRESS

IDS_INI_TASK_STATUS_BRINGTOFRONT
*/
