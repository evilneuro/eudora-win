// QCTipOfDayDlg.cpp : implementation file


#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "TipOfDayDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(QCTipOfDayDlg, SECTipOfDay)
	//{{AFX_MSG_MAP(QCTipOfDayDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCTipOfDayDlg dialog

QCTipOfDayDlg::QCTipOfDayDlg
(
	const TCHAR * lpszTipFileName,
	int nTip /*= 1*/,
	BOOL bShowAtStart /*= 1*/,
	CWnd* pParentWnd /*= NULL*/
) :
	SECTipOfDay(lpszTipFileName, nTip, bShowAtStart, pParentWnd)
{
	//{{AFX_DATA_INIT(QCTipOfDayDlg)
	//}}AFX_DATA_INIT
}


QCTipOfDayDlg::~QCTipOfDayDlg()
{
	// nothing to do
}


BOOL QCTipOfDayDlg::OnInitDialog()
{
	SECTipOfDay::OnInitDialog();

	//
	// Hide the Help button.
	//
	CWnd* pButton = GetDlgItem(IDC_TOD_HELP_BUTTON);
	if (pButton)
		pButton->ShowWindow(SW_HIDE);
	else
	{
		ASSERT(0);
	}

	//
	// Change the text on the OK button to "Close"
	//
	pButton = GetDlgItem(IDC_TOD_OK_BUTTON);
	if (pButton)
		pButton->SetWindowText(CRString(IDS_CLOSE));
	else
	{
		ASSERT(0);
	}

	SetTipIcon(IDR_COMPMESS);

	if (m_fontLeadIn.CreateFont(20,
								8,
								0,
								0,
								FW_BOLD,
								0,
								0,
								0,
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY,
								TMPF_TRUETYPE | FF_ROMAN, //DEFAULT_PITCH,
								_T("Times New Roman")))
	{
		SetLeadInFont(&m_fontLeadIn);
	}

	if (m_fontTip.CreatePointFont(90, "Arial"))
		SetTipFont(&m_fontTip);

	CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

