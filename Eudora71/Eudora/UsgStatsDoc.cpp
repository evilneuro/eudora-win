// UsgStatsDoc.cpp: implementation of the CUsageStatisticsDoc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UsgStatsDoc.h"
#include "UsgStatsView.h"
#include "StatMng.h"

#include "rs.h"

#include "DebugNewHelpers.h"

IMPLEMENT_DYNCREATE(CUsageStatisticsDoc, CDoc)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern StatDataPtr		gStatData;

CUsageStatisticsDoc::CUsageStatisticsDoc()
{

}

CUsageStatisticsDoc::~CUsageStatisticsDoc()
{

}


BOOL CUsageStatisticsDoc::OnNewDocument()
{		
	if (!CDoc::OnNewDocument())
		return FALSE;

	static CRString csTitle(IDS_STATISTICS_STATISTICS_STRING);
	SetTitle(csTitle);
	SetUsgStatsDocPtr(this);
	return TRUE;
}

void CUsageStatisticsDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	SetUsgStatsDocPtr(NULL);
	CDocument::OnCloseDocument();
}

void CUsageStatisticsDoc::UpdateAllAttachedViews()
{
	SetModifiedFlag();
	UpdateAllViews(NULL,0,0);
}


BOOL CUsageStatisticsDoc::SaveModified()
{
	return 1;
}

CUsageStatisticsDlgBar::CUsageStatisticsDlgBar()
{
	// Read the Selection Value from the INI file
	m_sPeriodSelected = GetIniShort(IDS_INI_STATISTICS_PERIOD_SELECTED);
	m_bMoreStatistics = GetIniShort(IDS_INI_STATISTICS_SHOW_MORE);
}

void CUsageStatisticsDlgBar ::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_CBString(pDX, IDC_US_PERIOD_CB, m_csPeriod);
	DDX_Control(pDX, IDC_US_PERIOD_CB, m_CBPeriod);	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUsageStatisticsDlgBar , CDialogBar)
	//{{AFX_MSG_MAP(CUsageStatisticsDlgBar)
	ON_CBN_SELCHANGE(IDC_US_PERIOD_CB, OnSelchangePeriodCombo)
	ON_BN_CLICKED(IDC_US_MORE_STATS_BTN, OnMoreStatistics)
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSCHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CUsageStatisticsDlgBar::OnSelchangePeriodCombo() 
{
	m_sPeriodSelected = (short)((CComboBox *)GetDlgItem(IDC_US_PERIOD_CB))->GetCurSel();
	((CUsageStatisticsView *)((CFrameWnd*)GetParent())->GetActiveView())->SetPeriodSelection(m_sPeriodSelected);
	((CUsageStatisticsView *)((CFrameWnd*)GetParent())->GetActiveView())->ReloadData(m_sPeriodSelected,m_bMoreStatistics);

	// Save the selected period
	SetIniShort(IDS_INI_STATISTICS_PERIOD_SELECTED,m_sPeriodSelected);	
}

void CUsageStatisticsDlgBar::OnMoreStatistics() 
{
	m_bMoreStatistics = ((CButton *)GetDlgItem(IDC_US_MORE_STATS_BTN))->GetCheck();
	((CUsageStatisticsView *)((CFrameWnd*)GetParent())->GetActiveView())->ReloadData(m_sPeriodSelected,m_bMoreStatistics);
	
	SetIniShort(IDS_INI_STATISTICS_SHOW_MORE,(short)m_bMoreStatistics);	
}

BOOL CUsageStatisticsDlgBar::PreTranslateMessage(MSG* pMsg) 
{
	BOOL ShiftKey;
	int nCtrlID = 0;
	HWND hTempDlgItem = ((CWnd *)GetDlgItem(IDC_US_PERIOD_CB))->m_hWnd;

	if ((WM_KEYDOWN == pMsg->message) && (VK_TAB == pMsg->wParam))
	{
		nCtrlID = ::GetDlgCtrlID(::GetFocus());
		ShiftKey = ::GetKeyState(VK_SHIFT);
		
		if (::GetKeyState(VK_SHIFT) >= 0)  // i.e Shift Key not pressed
		{
			if (nCtrlID == IDC_US_MORE_STATS_BTN)	// i.e the last control with WS_TABSTOP in the DialogBar
			{
				((CUsageStatisticsView *)((CFrameWnd*)GetParent())->GetActiveView())->SetFocus();
				return TRUE;
			}
		}
		else
		{
			if (nCtrlID == IDC_US_PERIOD_CB) // i.e the first control with WS_TABSTOP in the DialogBar
			{
				((CUsageStatisticsView *)((CFrameWnd*)GetParent())->GetActiveView())->SetFocus();
				return TRUE;
			}		
		}		
	}

	switch(pMsg->message)
	{
		case WM_SYSKEYDOWN:
			{
				// Hack Alert !!! ...Why do we do this ?
				// We have a UsageStatisticsFrame which contains a UsageStatisticsDlgBar & an UsageStatisticsView. The view
				// is what generally has the focus. We want to user to be able to use Acclerator (short-cut) keys (Hot-keys)
				// to get to the UI of the UsageStatisticsDlgBar (when UsageStatisticsDlgBar is not the one in focus).
				// This is the trick that does it. We overwrite the PreTranslateMessage in the UsageStatisticsView class & post
				// the WM_SYSKEYDOWN message (containing the approriate key virtual code that user pressed) & handle it here.

				// It turns out that if the pMsg->hWnd is a Child-Window handle of the UsageStatisticsDlgBar, then it works like a charm
				// We fake the same here to get what we need to get done ..as done above, we assign hTempDlgItem the value IDC_US_PERIOD_CB
				if (pMsg->hwnd == this->m_hWnd)
				{
					pMsg->hwnd = hTempDlgItem;
				}				
			}			
			break;
		default:
			break;
	}

	return CDialogBar::PreTranslateMessage(pMsg);
}

IMPLEMENT_DYNCREATE(CUsageStatisticsFrame , CMDIChild)

BEGIN_MESSAGE_MAP(CUsageStatisticsFrame, CMDIChild)
	//{{AFX_MSG_MAP(CMDIChild)
	ON_WM_CREATE()
	ON_WM_TIMECHANGE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CUsageStatisticsFrame ::CUsageStatisticsFrame ()
{

}

CUsageStatisticsFrame::~CUsageStatisticsFrame()
{

}

BOOL CUsageStatisticsFrame::PreCreateWindow( CREATESTRUCT& cs )
{
	CRect	rectWindow;
	// Read the Window Position from the rectMainFrame
	GetIniWindowPos( IDS_INI_STATISTICS_WIN_POS, rectWindow );

    if( ( rectWindow.left < rectWindow.right ) && ( rectWindow.top < rectWindow.bottom ) )
    {
		cs.x = rectWindow.left;
		cs.y = rectWindow.top;
		cs.cx = rectWindow.Width();
		cs.cy = rectWindow.Height();
	}

	return CMDIChild::PreCreateWindow(cs);

}

void CUsageStatisticsFrame::OnClose()
{

	WINDOWPLACEMENT wp;
	
	// Save the Window Position
	wp.length = sizeof(wp);
	if (GetWindowPlacement(&wp))
		SetIniWindowPos(IDS_INI_STATISTICS_WIN_POS, wp.rcNormalPosition);


	CMDIChild::OnClose();
}

int CUsageStatisticsFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChild::OnCreate(lpCreateStruct) == -1)
			return -1;

	m_secDialogBar.Create(this, IDD_USAGE_STATISTICS,
			CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY, IDD_USAGE_STATISTICS);

	((CComboBox *)(m_secDialogBar.GetDlgItem(IDC_US_PERIOD_CB)))->SetCurSel(m_secDialogBar.GetSelectedPeriod());
	((CButton *)(m_secDialogBar.GetDlgItem(IDC_US_MORE_STATS_BTN)))->SetCheck(m_secDialogBar.IsMoreStatisticsClicked());
	((CUsageStatisticsView *)GetActiveView())->SetPeriodSelection(m_secDialogBar.GetSelectedPeriod());
	((CUsageStatisticsView *)GetActiveView())->ShowMoreStatistics(m_secDialogBar.IsMoreStatisticsClicked());

	return 0;
}


void CUsageStatisticsFrame::OnTimeChange() 
{
	((CUsageStatisticsView *)GetActiveView())->ReloadData(m_secDialogBar.GetSelectedPeriod());
	CMDIChild::OnTimeChange();	
}

