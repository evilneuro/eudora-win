// TableDlgs.cpp : implementation file
//

#include "stdafx.h"
//#include "comm.h"
#include "resource.h"
#include "TableDlgs.h"

#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewTableDlg dialog


CNewTableDlg::CNewTableDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewTableDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewTableDlg)
	m_columnWidth = 0.0f;
	m_rows = 0;
	m_columns = 0;
	//}}AFX_DATA_INIT
}


void CNewTableDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewTableDlg)
	DDX_Text(pDX, IDC_COL_WIDTH, m_columnWidth);
	DDV_MinMaxFloat(pDX, m_columnWidth, 0.2f, 7.f);
	DDX_Text(pDX, IDC_NUM_ROWS, m_rows);
	DDV_MinMaxInt(pDX, m_rows, 1, 32767);
	DDX_Text(pDX, IDC_NUM_COLS, m_columns);
	DDV_MinMaxInt(pDX, m_columns, 1, 32);
	//}}AFX_DATA_MAP
}

BOOL CNewTableDlg::OnInitDialog()
{
	BOOL result = CDialog::OnInitDialog();

	CSpinButtonCtrl *spinCols = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_NUM_COLS);
	spinCols->SetRange(1,32);
	spinCols->SetPos(0);

	CSpinButtonCtrl *spinRows = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_NUM_ROWS);
	spinRows->SetRange(1,32);
	spinRows->SetPos(0);

	CSpinButtonCtrl *spinWidth = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_COL_WIDTH);
	spinWidth->SetRange(1,35);
	spinWidth->SetPos(0);
	//CRString initStr(IDS_AUTO);
	spinWidth->GetBuddy()->SetWindowText("0.2");

	return result;
}

BEGIN_MESSAGE_MAP(CNewTableDlg, CDialog)
	//{{AFX_MSG_MAP(CNewTableDlg)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewTableDlg message handlers

void CNewTableDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar->GetDlgCtrlID() == IDC_SPIN_COL_WIDTH)
	{
		int pos = ((CSpinButtonCtrl*)pScrollBar)->GetPos();
		pos = pos & 0xffff;
		
/*		if (pos == 0)
		{
			//CRString widthStr(IDS_AUTO);
			((CSpinButtonCtrl*)pScrollBar)->GetBuddy()->SetWindowText(CRString(IDS_AUTO));
		}
		else*/
		{
			CString widthStr;
			widthStr.Format("%2.1f", pos * 0.2);
			((CSpinButtonCtrl*)pScrollBar)->GetBuddy()->SetWindowText(widthStr);
		}
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
