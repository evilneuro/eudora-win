// STATBAR.CPP: CStatusBarEx implementation
//

#include "stdafx.h"
#include "resource.h"
#include "EudoraMsgs.h"
#include "TaskErrorView.h"
#include "rs.h"
#include "TaskStatus.h"
#include "QCTaskManager.h"

#include "statbar.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char BASED_CODE THIS_FILE[] = __FILE__;
	#define new DEBUG_NEW
#endif

// overwrite mode for editing is global state
extern bool g_bOverwriteMode;

// --------------------------------------------------------------------------

CStatusBarEx *CStatusBarEx::m_pStatBar = NULL;

// --------------------------------------------------------------------------
// Image list info

enum { SB_IL_EMPTY = 0,
			SB_IL_RUNNING1, SB_IL_RUNNING2, SB_IL_RUNNING3, SB_IL_RUNNING4,
			SB_IL_RUNNING5, SB_IL_RUNNING6, SB_IL_RUNNING7, SB_IL_RUNNING8,
			SB_IL_WAIT1, SB_IL_WAIT2, SB_IL_WAIT3, SB_IL_WAIT4,
			SB_IL_ERROR1, SB_IL_ERROR2
	 };

const int nImageListWidth = 14;
const unsigned int nImageListBitmapID = IDB_IL_STATBAR_TASK_STATUS;
const COLORREF nImageListBgColor = RGB(128,0,0);

// --------------------------------------------------------------------------

#define RUNNING_TIMER_ID  (1)
#define WAITING_TIMER_ID  (2)
#define ERROR_TIMER_ID    (3)

// --------------------------------------------------------------------------

#define TASK_STATUS_INDEX  (1)
#define WAIT_STATUS_INDEX  (2)
#define ERROR_STATUS_INDEX (3)
//#define CAPS_LOCK_INDEX    (4)
//#define NUM_LOCK_INDEX     (5)
//#define OVR_LOCK_INDEX     (6)

// --------------------------------------------------------------------------

//#define DISABLEDTASKINDEX 15
//#define DISABLEDWAITINDEX 16
//#define DISABLEDERRORINDEX 17


static const UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,         // status line indicator
	0,                    // Task Status bitmap
	0,                    // Wait Status bitmap
	0,                    // Error Status bitmap
	ID_INDICATOR_CAPS,    // Caps lock
	ID_INDICATOR_NUM,     // Num lock
	ID_INDICATOR_OVR      // Insert key state
//	ID_INDICATOR_SCRL,
};

// --------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CStatusBarEx, SECStatusBar)
	//{{AFX_MSG_MAP(CStatusBarEx)
	ON_WM_DRAWITEM_REFLECT()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(msgStatusBarTask, OnMsgTaskStatus)
	ON_MESSAGE(msgStatusBarError, OnMsgErrorStatus)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

CStatusBarEx *CStatusBarEx::QCGetStatusBar() // STATIC
{
	ASSERT(m_pStatBar);
	return (m_pStatBar);
}

// --------------------------------------------------------------------------

CStatusBarEx::CStatusBarEx()
{
	ASSERT(m_pStatBar == NULL);

	m_bTaskError = false;
	m_RunningTimerID = m_WaitingTimerID = m_ErrorTimerID = 0;

	m_RunningImageIdx = SB_IL_RUNNING1;
	m_ErrorImageIdx = SB_IL_ERROR1;
	m_WaitingImageIdx = SB_IL_WAIT1;

	m_TaskImageList = new CImageList;
	ASSERT(m_TaskImageList);
	VERIFY(m_TaskImageList->Create(nImageListBitmapID, nImageListWidth, 0, nImageListBgColor));

	m_bBlink = (GetIniShort(IDS_INI_STATBAR_BLINK) != 0);

	m_nRunningAniRate = GetIniInt(IDS_INI_STATBAR_ANIMATION_RATE_RUNNING);

	if (m_nRunningAniRate < 10)
		m_nRunningAniRate = 10;

	m_nWaitingAniRate = GetIniInt(IDS_INI_STATBAR_ANIMATION_RATE_WAITING);

	if (m_nWaitingAniRate < 10)
		m_nWaitingAniRate = 10;

	m_nErrorAniRate = GetIniInt(IDS_INI_STATBAR_ANIMATION_RATE_ERROR);

	if (m_nErrorAniRate < 10)
		m_nErrorAniRate = 10;

	m_bInitToolTip = false;

	// Read INI (also has defaults)
	GetIniColorRef(IDS_INI_TASKSTATUS_GRAPHBORDERCOLOR, m_GraphBorderColor);
	
	GetIniColorRef(IDS_INI_TASKSTATUS_GRAPHCOMPLETEDCOLOR, m_GraphCompletedColor);
	GetIniColorRef(IDS_INI_TASKSTATUS_GRAPHREMAINCOLOR, m_GraphRemainColor);

	GetIniColorRef(IDS_INI_TASKSTATUS_RECV_COMPLETEDCOLOR, m_RecvCompleteColor);
	GetIniColorRef(IDS_INI_TASKSTATUS_RECV_REMAINCOLOR, m_RecvRemainColor);

	if ((m_GraphCompletedColor == m_RecvCompleteColor) && (m_GraphRemainColor == m_RecvRemainColor))
	{
		m_DefaultCompleteColor = m_GraphCompletedColor;
		m_DefaultRemainColor = m_GraphRemainColor;
	}
	else
	{
		m_DefaultCompleteColor = GetSysColor(COLOR_HIGHLIGHT);
		m_DefaultRemainColor = GetSysColor(COLOR_WINDOW);
	}
}

// --------------------------------------------------------------------------

CStatusBarEx::~CStatusBarEx()
{
}

// --------------------------------------------------------------------------

BOOL CStatusBarEx::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	ASSERT(m_pStatBar == NULL);

	if (m_pStatBar != NULL)
		return (FALSE);

	if (!SECStatusBar::Create(pParentWnd, dwStyle, nID))
		return FALSE;

	if (!SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
		return FALSE;

	UINT ID;
	UINT nStyle;
	int cxWidth;

	GetPaneInfo(TASK_STATUS_INDEX, ID, nStyle, cxWidth);
	SetPaneInfo(TASK_STATUS_INDEX, ID, SBPS_NORMAL, nImageListWidth + 2); // GetIniShort(IDS_INI_STATBAR_GRAPH_WIDTH)
	VERIFY(GetStatusBarCtrl().SetText(NULL, TASK_STATUS_INDEX, SBT_OWNERDRAW));

	GetPaneInfo(WAIT_STATUS_INDEX, ID, nStyle, cxWidth);
	SetPaneInfo(WAIT_STATUS_INDEX, ID, SBPS_POPOUT, nImageListWidth + 2);
	VERIFY(GetStatusBarCtrl().SetText(NULL, WAIT_STATUS_INDEX, SBT_OWNERDRAW));

	GetPaneInfo(ERROR_STATUS_INDEX, ID, nStyle, cxWidth);
	SetPaneInfo(ERROR_STATUS_INDEX, ID, SBPS_POPOUT, nImageListWidth + 2);
	VERIFY(GetStatusBarCtrl().SetText(NULL, ERROR_STATUS_INDEX, SBT_OWNERDRAW));

//	GetPaneInfo(NUM_LOCK_INDEX, ID, nStyle, cxWidth);
//	SetPaneInfo(NUM_LOCK_INDEX, ID, SBPS_POPOUT, 0);
//	VERIFY(GetStatusBarCtrl().SetText(NULL, NUM_LOCK_INDEX, SBT_OWNERDRAW));

//	GetPaneInfo(CAPS_LOCK_INDEX, ID, nStyle, cxWidth);
//	SetPaneInfo(CAPS_LOCK_INDEX, ID, SBPS_POPOUT, 0);
//	VERIFY(GetStatusBarCtrl().SetText(NULL, CAPS_LOCK_INDEX, SBT_OWNERDRAW));

//	GetPaneInfo(OVR_LOCK_INDEX, ID, nStyle, cxWidth);
//	SetPaneInfo(OVR_LOCK_INDEX, ID, SBPS_POPOUT, 0);
//	VERIFY(GetStatusBarCtrl().SetText(NULL, OVR_LOCK_INDEX, SBT_OWNERDRAW));

	VERIFY(EnableToolTips(TRUE));

	m_pStatBar = this;

	return TRUE;
}

// --------------------------------------------------------------------------

LRESULT CStatusBarEx::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SETTEXT && lParam)
	{
		// Eudora has some multi-line info strings. We only show the first line.
		char* Newline = strchr((const char*)lParam, '\n');
		if (Newline)
		{
			char Buffer[128];
	
			int NumChars = Newline - (const char*)lParam;
			if (NumChars < sizeof(Buffer))
			{
				strncpy(Buffer, (const char*)lParam, NumChars);
				Buffer[NumChars] = 0;
				lParam = (LPARAM)Buffer;
			}
		}
	}
	
	return (SECStatusBar::WindowProc(message, wParam, lParam));
}

// --------------------------------------------------------------------------

void CStatusBarEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// YUCK! This is the worst, ugliest, most foul-smelling place I
	// can imagine to do the tooltip initialization! Unfortunately this
	// seems to be the only place that works. Create() doesn't work,
	// OnSize() doesn't work, constructor doesn't work...
	//
	// Oh well, as long as it works, right?
	//
	// 5/98, Scott Manjourides
	//
	// // Here we make it even worse. We're not going to initialize tool tips
	// // til we're done resizing the last of the statusbar panes.
	// // So we create the panes with zero size. We resize them each on the first
	// // run through DrawItem and on the last of the panes we set all
	// // The ToolTips so they point to the right boxes.
	// // 8/11/98 Josh
	if (!m_bInitToolTip)
	{
		// Create the tooltip
		VERIFY(m_ToolTip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX));

		// Add the tool tips, we'll change the rct in OnSize() IDS_STATUSBAR_TOOLTIP_STATUSIDLE
		CRect rct;
		GetItemRect(TASK_STATUS_INDEX, &rct);
		ASSERT(rct.Width());
		VERIFY(m_ToolTip.AddTool(this, CRString(IDS_STATUSBAR_TOOLTIP_TASK_IDLE), rct, TASK_STATUS_INDEX));

		GetItemRect(WAIT_STATUS_INDEX, &rct);
		ASSERT(rct.Width());
		VERIFY(m_ToolTip.AddTool(this, CRString(IDS_STATUSBAR_TOOLTIP_WAIT_IDLE), rct, WAIT_STATUS_INDEX));

		GetItemRect(ERROR_STATUS_INDEX, &rct);
		ASSERT(rct.Width());
		VERIFY(m_ToolTip.AddTool(this, CRString(IDS_STATUSBAR_TOOLTIP_ERROR_IDLE), rct, ERROR_STATUS_INDEX));

//		GetItemRect(NUM_LOCK_INDEX, &rct);
//		ASSERT(rct.Width());
//		VERIFY(m_ToolTip.AddTool(this, CRString(IDS_STATUSBAR_TOOLTIP_NUMLOCK), rct, NUM_LOCK_INDEX));
		
//		GetItemRect(CAPS_LOCK_INDEX, &rct);
//		ASSERT(rct.Width());
//		VERIFY(m_ToolTip.AddTool(this, CRString(IDS_STATUSBAR_TOOLTIP_CAPSLOCK), rct, CAPS_LOCK_INDEX));

//		GetItemRect(OVR_LOCK_INDEX, &rct);
//		ASSERT(rct.Width());
//		VERIFY(m_ToolTip.AddTool(this, CRString(IDS_STATUSBAR_TOOLTIP_OVRLOCK), rct, OVR_LOCK_INDEX));

		m_bInitToolTip = true;

		SetStatusToolTipText(TASK_STATUS_INDEX);
		SetStatusToolTipText(WAIT_STATUS_INDEX);
		SetStatusToolTipText(ERROR_STATUS_INDEX);
	}
	ASSERT(m_TaskImageList);

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect(lpDrawItemStruct->rcItem);

	// Get the left edge of the image when centered within the draw rect
	CPoint point(rect.left + ((rect.Width() - nImageListWidth)/2), rect.top);

	// Left-most edge is the rect edge
	if (point.x < rect.left)
		point.x = rect.left;

//	if (lpDrawItemStruct->itemID > 3)	// Handles all the indicators.
//	{
//		char indicatorText[4];
//		if (lpDrawItemStruct->itemID == NUM_LOCK_INDEX)
//			strcpy(indicatorText, CRString(ID_INDICATOR_NUM));
//		else if (lpDrawItemStruct->itemID == OVR_LOCK_INDEX)
//			strcpy(indicatorText, CRString(ID_INDICATOR_OVR));
//		else if (lpDrawItemStruct->itemID == CAPS_LOCK_INDEX)
//			strcpy(indicatorText, CRString(ID_INDICATOR_CAPS));
//
//		if (rect.right - rect.left < 5)	// It's uninitialized this should happen once , but it can't be done in create because we don't
//										// have the DC up there and we need it to determine how wide the chars are.
//		{
//			CSize size;
//			UINT nStyle;
//			int cxWidth;
//			UINT ID;
//			size = pDC->GetTextExtent( indicatorText, 3);
//			GetPaneInfo(lpDrawItemStruct->itemID, ID, nStyle, cxWidth);
//			SetPaneInfo(lpDrawItemStruct->itemID, ID, SBPS_POPOUT, size.cx);
//
//
//		}
//
//		CPoint point2(rect.left+2, rect.top);
//		CSize size(rect.Width(), rect.Height());

//		bool enabled;

//		if (lpDrawItemStruct->itemID == OVR_LOCK_INDEX)
//			enabled = g_bOverwriteMode;
//		else if (lpDrawItemStruct->itemID == CAPS_LOCK_INDEX)
//		{
//			enabled = ((GetKeyState(  VK_CAPITAL ) & 0X000F) != 0);
//		}
//		else if (lpDrawItemStruct->itemID == NUM_LOCK_INDEX)
//		{
//			enabled = ((GetKeyState(  VK_NUMLOCK ) & 0X000F) != 0);
//		}
//
	//	pDC->SetBkMode( TRANSPARENT  );
//		if (enabled)
//			pDC->DrawState( point2, size, indicatorText, DSS_NORMAL, 0, 0 ,  (CBrush*)NULL);
//		else
//			pDC->DrawState( point2, size, indicatorText, DSS_DISABLED, 0, 0 , (CBrush*)NULL );

//	}
	if (TASK_STATUS_INDEX != lpDrawItemStruct->itemID)
	{
		int idx = SB_IL_EMPTY;

		switch (lpDrawItemStruct->itemID)
		{
			case TASK_STATUS_INDEX:	
			{
				if (m_TheTaskStatus.AnyRunning())
					idx = m_RunningImageIdx;
			//	else
			//		idx = DISABLEDTASKINDEX;	// I don't know if this ever gets called, but if it does, here's the index.
			}
			break;
			
			case WAIT_STATUS_INDEX:
			{
				if (m_TheTaskStatus.AnyWaiting())
					idx = m_WaitingImageIdx;
			//	else
			//		idx = DISABLEDWAITINDEX;
			}
			break;

			case ERROR_STATUS_INDEX:
			{
				if (m_bTaskError)
					idx = m_ErrorImageIdx;
			//	else
			//		idx = DISABLEDERRORINDEX;
			}
			break;

			default:
			{
				ASSERT(0); // Not handling an owner-draw item
			}
			break;
		}

		if (SB_IL_EMPTY != idx)
			VERIFY(m_TaskImageList->Draw(pDC, idx, point, ILD_TRANSPARENT));
//		else if (idx >= DISABLEDTASKINDEX)
//		{
//			CSize size(rect.Width(), rect.Height());
//			HICON theDisabledIcon;
//			theDisabledIcon = m_TaskImageList->ExtractIcon(idx);
//			pDC->DrawState( point, size, theDisabledIcon, DSS_DISABLED , (CBrush*)NULL );
//		}
		else
			pDC->FillSolidRect( rect, GetSysColor(COLOR_BTNFACE));	// Don't think this'll get called anymore, but just in case.
	}
	else // TASK_STATUS_INDEX
	{
		if (m_TheTaskStatus.AnyRunning())
		{
			if (m_TheTaskStatus.HasPercent())
			{
				CRect PctRct = rect;
				PctRct.right -= nImageListWidth + 3; // Percent to left of icon

				CRect ColumnRct = PctRct;
				ColumnRct.right--;
				ColumnRct.bottom--;
				CRect FullBarRct, CompletedRct, RemainingRct;

				FullBarRct = ColumnRct;
				
				FullBarRct.InflateRect(-1,-1);

				if ((FullBarRct.left + 2) < FullBarRct.right) // Min graph width is three
				{
					int nSavedDC = pDC->SaveDC();

					CompletedRct = FullBarRct;

					CompletedRct.InflateRect(-1,-1);
					
					RemainingRct = CompletedRct;

					CompletedRct.right = CompletedRct.left + (int(((CompletedRct.right - CompletedRct.left)+1) * ((double)m_TheTaskStatus.GetPercent()/100.0)));
					RemainingRct.left = CompletedRct.right;

					if (CompletedRct.right > RemainingRct.right)
						CompletedRct.right = RemainingRct.right;

					CBrush borderBrush(m_GraphBorderColor);
					CBrush* pBorderBrush = &borderBrush;
					pDC->FrameRect(FullBarRct, pBorderBrush);

					COLORREF nCompletedClr = m_DefaultCompleteColor, nRemainClr = m_DefaultRemainColor;

					if (m_TheTaskStatus.GetSendCount() > 0)
					{
						if (m_TheTaskStatus.GetRecvCount() == 0)
						{
							nCompletedClr = m_GraphCompletedColor;
							nRemainClr = m_GraphRemainColor;
						}
					}
					else if (m_TheTaskStatus.GetRecvCount() > 0)
					{
						nCompletedClr = m_RecvCompleteColor;
						nRemainClr = m_RecvRemainColor;
					}

					if (CompletedRct.left <= CompletedRct.right)
						pDC->FillSolidRect(CompletedRct, nCompletedClr);

					if (RemainingRct.left <= RemainingRct.right)
						pDC->FillSolidRect(RemainingRct, nRemainClr);

					pDC->RestoreDC(nSavedDC);

					point.x = PctRct.right + 1; // Icon is to right of graph
				}
			}

			// Running always shows spinny thing
			VERIFY(m_TaskImageList->Draw(pDC, m_RunningImageIdx, point, ILD_TRANSPARENT));
		}
		else
		{
		//	CSize size(rect.Width(), rect.Height());
		//	HICON theDisabledIcon;
		//	theDisabledIcon = m_TaskImageList->ExtractIcon(DISABLEDTASKINDEX);
		//	pDC->DrawState( point, size, theDisabledIcon, DSS_DISABLED , (CBrush*)NULL );
		//	VERIFY(m_TaskImageList->Draw(pDC, DISABLEDTASKINDEX, point, ILD_TRANSPARENT));
			pDC->FillSolidRect( rect, GetSysColor(COLOR_BTNFACE));
		}
	}
}

// --------------------------------------------------------------------------

LONG CStatusBarEx::OnMsgTaskStatus(WPARAM wParam, LPARAM lParam)
{
	const CTaskStatus *pTaskStatus = (const CTaskStatus *) wParam;
	ASSERT(pTaskStatus);

	if (!pTaskStatus)
	return (0);

	if (m_TheTaskStatus != (*pTaskStatus))
	{
		bool bStatusChanged = false;
		bool bWaitChanged = false;

		if ( (m_TheTaskStatus.AnyRunning() != pTaskStatus->AnyRunning())
			|| (m_TheTaskStatus.HasPercent() != pTaskStatus->HasPercent())
			|| (m_TheTaskStatus.GetPercent() != pTaskStatus->GetPercent())
			|| (m_TheTaskStatus.GetPctCount() != pTaskStatus->GetPctCount())
			|| (m_TheTaskStatus.GetRunCount() != pTaskStatus->GetRunCount())
			|| (m_TheTaskStatus.GetSendCount() != pTaskStatus->GetSendCount())
			|| (m_TheTaskStatus.GetRecvCount() != pTaskStatus->GetRecvCount()) )
		{
			bStatusChanged = true;

			if (m_TheTaskStatus.HasPercent() != pTaskStatus->HasPercent()) // If the percent status changes
			{
				if (pTaskStatus->HasPercent()) // changing from NO to YES percent
				{
					SetPaneInfo(TASK_STATUS_INDEX, 0, SBPS_NORMAL, GetIniShort(IDS_INI_STATBAR_GRAPH_WIDTH) + 1 + nImageListWidth + 2);
				}
				else
				{
					SetPaneInfo(TASK_STATUS_INDEX, 0, SBPS_NORMAL, nImageListWidth + 2);
				}

				// Because the pane rect has changed, we need to change the tooltip rect
				SetTooltipRect(TASK_STATUS_INDEX, TASK_STATUS_INDEX);
			}
		}

		if (m_TheTaskStatus.AnyWaiting() != pTaskStatus->AnyWaiting())
			bWaitChanged = true;

		m_TheTaskStatus = (*pTaskStatus);

		if (bStatusChanged)
			OnChangeRunningStatus();

		if (bWaitChanged)
			OnChangeWaitingStatus();
	}

	return (0);
}

// --------------------------------------------------------------------------

LONG CStatusBarEx::OnMsgErrorStatus(WPARAM wParam, LPARAM lParam)
{
	const bool bError = (wParam != 0);

	if (m_bTaskError != bError)
	{
		m_bTaskError = bError;
		OnChangeErrorStatus();
	}

	return (0);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnChangeRunningStatus()
{
	if (m_TheTaskStatus.AnyRunning())
	{
		if (!m_RunningTimerID)
			VERIFY(m_RunningTimerID = SetTimer(RUNNING_TIMER_ID, m_nRunningAniRate, NULL));
	}
	else
	{
		if (m_RunningTimerID)
			KillTimer(m_RunningTimerID), m_RunningTimerID = 0;
	}

	SetStatusToolTipText(TASK_STATUS_INDEX);
	
	CRect rct;
	GetItemRect(TASK_STATUS_INDEX, LPRECT(rct));
	InvalidateRect(rct, FALSE);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnChangeWaitingStatus()
{
	// Only turn on the timer if there is something waiting AND we should blink the icon
	if ((m_TheTaskStatus.AnyWaiting()) && (m_bBlink))
	{
		if (!m_WaitingTimerID)
			VERIFY(m_WaitingTimerID = SetTimer(WAITING_TIMER_ID, m_nWaitingAniRate, NULL));
	}
	else
	{
		if (m_WaitingTimerID)
			KillTimer(m_WaitingTimerID), m_WaitingTimerID = 0;
	}

	SetStatusToolTipText(WAIT_STATUS_INDEX);
	
	CRect rct;
	GetItemRect(WAIT_STATUS_INDEX, LPRECT(rct));
	InvalidateRect(rct, FALSE);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnChangeErrorStatus()
{
	// Only turn on the timer if there is an error AND we should blink the icon
	if ((m_bTaskError) && (m_bBlink))
	{
		if (!m_ErrorTimerID)
			VERIFY(m_ErrorTimerID = SetTimer(ERROR_TIMER_ID, m_nErrorAniRate, NULL));
	}
	else // !m_bTaskError
	{
		if (m_ErrorTimerID)
			KillTimer(m_ErrorTimerID), m_ErrorTimerID = 0;
	}

	SetStatusToolTipText(ERROR_STATUS_INDEX);
	
	CRect rct;
	GetItemRect(ERROR_STATUS_INDEX, LPRECT(rct));
	InvalidateRect(rct, FALSE);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == m_RunningTimerID)
	{
		switch (m_RunningImageIdx)
		{
			case SB_IL_RUNNING1: m_RunningImageIdx = SB_IL_RUNNING2; break;
			case SB_IL_RUNNING2: m_RunningImageIdx = SB_IL_RUNNING3; break;
			case SB_IL_RUNNING3: m_RunningImageIdx = SB_IL_RUNNING4; break;
			case SB_IL_RUNNING4: m_RunningImageIdx = SB_IL_RUNNING5; break;
			case SB_IL_RUNNING5: m_RunningImageIdx = SB_IL_RUNNING6; break;
			case SB_IL_RUNNING6: m_RunningImageIdx = SB_IL_RUNNING7; break;
			case SB_IL_RUNNING7: m_RunningImageIdx = SB_IL_RUNNING8; break;
			case SB_IL_RUNNING8: m_RunningImageIdx = SB_IL_RUNNING1; break;

			default:
			{
				ASSERT(0); // Image index is out of range
				m_RunningImageIdx = SB_IL_RUNNING1;
			}
			break;
		}
		
		CRect rct;
		GetItemRect(TASK_STATUS_INDEX, LPRECT(rct));
		rct.DeflateRect(1, 1);
		rct.left = rct.right - nImageListWidth;
		InvalidateRect(rct);
	}
	else if (nIDEvent == m_WaitingTimerID)
	{
		switch (m_WaitingImageIdx)
		{
			case SB_IL_WAIT1: m_WaitingImageIdx = SB_IL_WAIT2; break;
			case SB_IL_WAIT2: m_WaitingImageIdx = SB_IL_WAIT3; break;
			case SB_IL_WAIT3: m_WaitingImageIdx = SB_IL_WAIT4; break;
			case SB_IL_WAIT4: m_WaitingImageIdx = SB_IL_WAIT1; break;

			default:
			{
				ASSERT(0); // Image index is out of range
				m_WaitingImageIdx = SB_IL_WAIT1;
			}
			break;
		}

		CRect rct;
		GetItemRect(WAIT_STATUS_INDEX, LPRECT(rct));
		rct.DeflateRect(1, 1);
		rct.left = rct.right - nImageListWidth;
		InvalidateRect(rct);
	}
	else if (nIDEvent == m_ErrorTimerID)
	{
		switch (m_ErrorImageIdx)
		{
			case SB_IL_ERROR1: m_ErrorImageIdx = SB_IL_ERROR2; break;
			case SB_IL_ERROR2: m_ErrorImageIdx = SB_IL_ERROR1; break;

			default:
			{
				ASSERT(0); // Image index is out of range
				m_ErrorImageIdx = SB_IL_ERROR1;
			}
			break;
		}

		CRect rct;
		GetItemRect(ERROR_STATUS_INDEX, LPRECT(rct));
		rct.DeflateRect(1, 1);
		rct.left = rct.right - nImageListWidth;
		InvalidateRect(rct);
	}

	SECStatusBar::OnTimer(nIDEvent);
}

// --------------------------------------------------------------------------

BOOL CStatusBarEx::PreTranslateMessage(MSG* pMsg)
{
	// Pass events through the tooltip
	if (m_bInitToolTip)
		m_ToolTip.RelayEvent(pMsg);

	return CWnd::PreTranslateMessage(pMsg);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnDestroy() 
{
	SECStatusBar::OnDestroy();
	
	if (m_RunningTimerID)
	{
		KillTimer(m_RunningTimerID);
		m_RunningTimerID = 0;
	}

	if (m_WaitingTimerID)
	{
		KillTimer(m_WaitingTimerID);
		m_WaitingTimerID = 0;
	}

	if (m_ErrorTimerID)
	{
		KillTimer(m_ErrorTimerID);
		m_ErrorTimerID = 0;
	}

	if (m_TaskImageList)
		delete (m_TaskImageList);	
}

// --------------------------------------------------------------------------

void CStatusBarEx::SetTooltipRect(int nIndex, UINT nIDTool)
{
	if (m_bInitToolTip)
	{
		CRect rct;
		GetItemRect(nIndex, &rct);
	//	ASSERT(rct.Width());
		m_ToolTip.SetToolRect(this, nIDTool, rct);
	}
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnSize(UINT nType, int cx, int cy) 
{
	SECStatusBar::OnSize(nType, cx, cy);

	// OnSize() is called before we get a chance to initialize the
	// tooltips, so only do this if initialization has happened.
	if (m_bInitToolTip)
	{
		// Reposition the tooltips to match the status panes
		SetTooltipRect(TASK_STATUS_INDEX, TASK_STATUS_INDEX);
		SetTooltipRect(WAIT_STATUS_INDEX, WAIT_STATUS_INDEX);
		SetTooltipRect(ERROR_STATUS_INDEX, ERROR_STATUS_INDEX);
	}
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	// Calling the base class seems to be a bad idea, so DON'T!!
	//	SECStatusBar::OnActivate(nState, pWndOther, bMinimized);

	if (m_bInitToolTip)
		m_ToolTip.Activate(nState != WA_INACTIVE);
}

// --------------------------------------------------------------------------

void CStatusBarEx::SetStatusToolTipText(UINT nToolID)
{
	if (!m_bInitToolTip)
		return;

	CString str;

	switch (nToolID)
	{
		case TASK_STATUS_INDEX:
		{
			if (m_TheTaskStatus.AnyRunning())
			{
				if (m_TheTaskStatus.GetRunCount() == 1)
					str = CRString(IDS_STATUSBAR_TOOLTIP_TASK_BUSY_ONE);
				else
					str.Format(CRString(IDS_STATUSBAR_TOOLTIP_TASK_BUSY), m_TheTaskStatus.GetRunCount());
			}
			else
				str = CRString(IDS_STATUSBAR_TOOLTIP_TASK_IDLE);
		}
		break;
		
		case WAIT_STATUS_INDEX:
		{
			if (m_TheTaskStatus.AnyWaiting())
				str = CRString(IDS_STATUSBAR_TOOLTIP_WAIT_WAITING);
			else
				str = CRString(IDS_STATUSBAR_TOOLTIP_WAIT_IDLE);
		}
		break;

		case ERROR_STATUS_INDEX:
		{
			if (m_bTaskError)
				str = CRString(IDS_STATUSBAR_TOOLTIP_ERROR_ERRORS);
			else
				str = CRString(IDS_STATUSBAR_TOOLTIP_ERROR_IDLE);
		}
		break;

		default:
		{
			ASSERT(0);
		}
	}

	if (!str.IsEmpty())
		m_ToolTip.UpdateTipText(str, this, nToolID);
}

// --------------------------------------------------------------------------

BOOL CStatusBarEx::OnEraseBkgnd(CDC* pDC) 
{
	CRect rct1, rct2, rct3;

	GetItemRect(TASK_STATUS_INDEX, &rct1);
	rct1.InflateRect(-1,-1);
	pDC->ExcludeClipRect( rct1 );

	GetItemRect(WAIT_STATUS_INDEX, &rct2);
	rct2.InflateRect(-1,-1);
	pDC->ExcludeClipRect( rct2 );

	GetItemRect(ERROR_STATUS_INDEX, &rct3);
	rct3.InflateRect(-1,-1);
	pDC->ExcludeClipRect( rct3 );

	if (!SECStatusBar::OnEraseBkgnd(pDC))
		return (FALSE);

// Force the owner-drawn controls to redraw
	InvalidateRect(rct1, FALSE);
	InvalidateRect(rct2, FALSE);
	InvalidateRect(rct3, FALSE);

	UpdateWindow();

	return (TRUE);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!HandleLeftClick(nFlags, point))
		SECStatusBar::OnLButtonDown(nFlags, point);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (!HandleLeftClick(nFlags, point))
		SECStatusBar::OnLButtonDblClk(nFlags, point);
}

// --------------------------------------------------------------------------

void CStatusBarEx::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (!HandleRightClick(nFlags, point))
		SECStatusBar::OnRButtonDown(nFlags, point);
}

// --------------------------------------------------------------------------

int CStatusBarEx::FindPaneIndex(const CPoint &point)
{
	CRect rct;
	const int IdxArr[] = { TASK_STATUS_INDEX, WAIT_STATUS_INDEX, ERROR_STATUS_INDEX };
	const int IdxSize = sizeof(IdxArr) / sizeof(int);

	for (int i=0; i < IdxSize; i++)
	{
		GetItemRect(IdxArr[i], &rct);
		if (rct.PtInRect(point))
			return (IdxArr[i]);
	}

	return (-1);
}

// --------------------------------------------------------------------------

bool CStatusBarEx::HandleLeftClick(UINT nFlags, CPoint point) 
{
	switch (FindPaneIndex(point))
	{
		case TASK_STATUS_INDEX:
		{
			PostMessage(WM_COMMAND, IDM_VIEW_TASK_STATUS);

/*			if (m_TheTaskStatus.AnyRunning())
			{
			}
			else
			{
				// Do nothing
			}
*/
		}
		break;

		case WAIT_STATUS_INDEX:
		{
			if (m_TheTaskStatus.AnyWaiting())
			{
				QCGetTaskManager()->RequestPostProcessing();
			}
			else
			{
				// Do nothing
			}
		}
		break;

		case ERROR_STATUS_INDEX:
		{
			PostMessage(WM_COMMAND, IDM_VIEW_TASK_ERROR);

/*			if (m_bTaskError)
			{
			}
			else
			{
				// Do nothing
			}
*/
		}
		break;

		default:
			return (false);
	}

	return (true);
}


// --------------------------------------------------------------------------

bool CStatusBarEx::HandleRightClick(UINT nFlags, CPoint point)
{
	const int idx = FindPaneIndex(point);

	if (idx == (-1))
		return (false);

	bool bOffline = (GetIniShort(IDS_INI_CONNECT_OFFLINE) == 1);
	CPoint screenPt = point;
	ClientToScreen(&screenPt);

	CMenu menu;
	menu.CreatePopupMenu();

	const UINT nMENUID_ShowStatus    = 1;
	const UINT nMENUID_ShowErrors    = 2;
	const UINT nMENUID_DoPostAll     = 3;
	const UINT nMENUID_StopAll       = 4;
	const UINT nMENUID_OfflineToggle = 5;
	const UINT nMENUID_RemoveAllErrs = 6;

	switch (idx)
	{
		case TASK_STATUS_INDEX:
		{
			menu.AppendMenu(MF_STRING, nMENUID_StopAll, "Stop All Tasks");
			menu.AppendMenu(MF_STRING, nMENUID_OfflineToggle, bOffline ? "Go online" : "Go offline");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, nMENUID_ShowStatus, "Show Task Status");

			if (!m_TheTaskStatus.AnyRunning())
				menu.EnableMenuItem(nMENUID_StopAll, MF_BYCOMMAND | MF_GRAYED);
		}
		break;

		case WAIT_STATUS_INDEX:
		{
			menu.AppendMenu(MF_STRING, nMENUID_DoPostAll, "Process All Waiting Tasks \tCtrl+\\");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, nMENUID_ShowStatus, "Show Task Status");

			if (!m_TheTaskStatus.AnyWaiting())
				menu.EnableMenuItem(nMENUID_DoPostAll, MF_BYCOMMAND | MF_GRAYED);
		}
		break;

		case ERROR_STATUS_INDEX:
		{
			menu.AppendMenu(MF_STRING, nMENUID_RemoveAllErrs, "Remove All Errors");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, nMENUID_ShowErrors, "Show Task Errors");

			if (!m_bTaskError)
				menu.EnableMenuItem(nMENUID_RemoveAllErrs, MF_BYCOMMAND | MF_GRAYED);
		}
		break;
	}

	DWORD sel = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON
					| TPM_NONOTIFY | TPM_RETURNCMD, screenPt.x, screenPt.y, this);

	switch (sel)
	{
		case nMENUID_ShowStatus:
		{
			PostMessage(WM_COMMAND, IDM_VIEW_TASK_STATUS);
		}
		break;

		case nMENUID_ShowErrors:
		{
			PostMessage(WM_COMMAND, IDM_VIEW_TASK_ERROR);
		}
		break;

		case nMENUID_DoPostAll:
		{
			QCGetTaskManager()->RequestPostProcessing();
		}
		break;

		case nMENUID_StopAll:
		{
			QCGetTaskManager()->RequestAllThreadsToStop();
		}
		break;

		case nMENUID_RemoveAllErrs:
		{
			QCGetTaskErrorView()->SendMessage(msgErrorViewRemoveAll);
		}
		break;

		case nMENUID_OfflineToggle:
		{
			bOffline = !bOffline;
			short nNewVal = 0;
			
			if (bOffline)
				nNewVal = 1;

			SetIniShort(IDS_INI_CONNECT_OFFLINE, nNewVal);
		}
		break;
	}

	menu.DestroyMenu();

	return (true);
}

