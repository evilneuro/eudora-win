// PropertyPanel.cpp : implementation file
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>

#include "resource.h"

#include "eudora.h"
#include "rs.h"
#include "doc.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "msgdoc.h"
#include "tocdoc.h"
#include "readmsgd.h"
#include "PropertyPanel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPropertyPageEmpty, CPropertyPage)
/////////////////////////////////////////////////////////////////////////////
// CPropertyPageEmpty property page

CPropertyPageEmpty::CPropertyPageEmpty() : CPropertyPage(CPropertyPageEmpty::IDD)
{
	//{{AFX_DATA_INIT(CPropertyPageEmpty)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPropertyPageEmpty::~CPropertyPageEmpty()
{
}

void CPropertyPageEmpty::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyPageEmpty)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyPageEmpty, CPropertyPage)
	//{{AFX_MSG_MAP(CPropertyPageEmpty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyPanel

IMPLEMENT_DYNAMIC(CPropertyPanel, CPropertySheet)

CPropertyPanel::CPropertyPanel(CWnd* pWndParent, CDocument* pDoc)
	 : CPropertySheet(IDS_PROPSHT_CAPTION, pWndParent)
{
	// Add all of the property pages here.  Note that
	// the order that they appear in here will be
	// the order they appear in on screen.  By default,
	// the first page of the set is the active one.
	// One way to make a different property page the 
	// active one is to call SetActivePage().
	ASSERT(pWndParent);
	if (pWndParent != NULL)
		m_pWndParent = pWndParent;
	else
		m_pWndParent = NULL;

	m_pDoc = pDoc;

	m_bPropertyChanged = FALSE;

/*	if (m_pDoc->IsKindOf( RUNTIME_CLASS(CMailboxesDoc)))
	{
		AddPage(&m_PageMbx1);
			m_PageMbx1.m_pDoc = m_pDoc;
		AddPage(&m_PageMbx2);
			m_PageMbx2.m_pDoc = m_pDoc;
		AddPage(&m_PageMbx3);
			m_PageMbx3.m_pDoc = m_pDoc;
	}
	else*/ if (m_pDoc->IsKindOf( RUNTIME_CLASS(CReadMessageDoc)))
	{
		AddPage(&m_PageReadMessage1);
			m_PageReadMessage1.m_pDoc = m_pDoc;
		AddPage(&m_PageReadMessage2);
			m_PageReadMessage2.m_pDoc = m_pDoc;
		SetTitle("Incoming Message", PSH_PROPTITLE);
	}
	//else if (pWndParent->IsKindOf( RUNTIME_CLASS(CCompMessage)))
		//AddPage(&m_PageComposition1);
		//AddPage(&m_PageComposition2);
		//AddPage(&m_PageComposition3);
	//}
	//else if (pWndParent->IsKindOf( RUNTIME_CLASS(CAttachment)))
		//AddPage(&m_PageAttachment1);
	else
		AddPage(&m_EmptyPage);

	m_bPressedTack = FALSE;
}

CPropertyPanel::~CPropertyPanel()
{
}


BEGIN_MESSAGE_MAP(CPropertyPanel, CPropertySheet)
	//{{AFX_MSG_MAP(CPropertyPanel)
	ON_COMMAND(ID_APPLY_NOW, OnApply)
	ON_WM_KILLFOCUS()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_COMMAND(ID_PROP_TACK, OnPropTack)
	ON_COMMAND(ID_PROP_MULT_WIN, OnPropMultiWin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyPanel message handlers
int CPropertyPanel::m_bMultiWin = FALSE;

//static int rgiButtons[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };

CFont * CPropertyPanel::GetSysFont(int whichFont /*=-1*/)
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
}


BOOL CPropertyPanel::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	// add a preview window to the property sheet.
	CRect rectWnd;
	GetWindowRect(rectWnd);
	SetWindowPos(NULL, 0, 0,
		rectWnd.Width(),
		rectWnd.Height() + 30,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	GetWindowRect(rectWnd);
 
	CSize buttonSize(60,24);
	CPoint buttonOrig(rectWnd.Width() - buttonSize.cx -5,
						rectWnd.Height() - buttonSize.cy -5);
	CRect rectPreview(buttonOrig, buttonSize);

	m_cbApply.Create("&Apply", WS_CHILD |WS_VISIBLE, rectPreview, this, ID_APPLY_NOW);
	m_cbApply.EnableWindow(FALSE);
	m_cbApply.SetFont(GetSysFont());

	rectPreview.left -= (buttonSize.cx + 10);
	rectPreview.right -= (buttonSize.cx + 10);
	m_cbCancel.Create("&Cancel", WS_CHILD |WS_VISIBLE, rectPreview, this, IDCANCEL);
	m_cbCancel.SetFont(GetSysFont());

	rectPreview.left -= (buttonSize.cx + 10);
	rectPreview.right -= (buttonSize.cx + 10);
	m_cbOK.Create("&OK", WS_CHILD |WS_VISIBLE, rectPreview, this, IDOK);
	m_cbOK.SetFont(GetSysFont());

	rectPreview.left = rectWnd.Width() - 25;
	rectPreview.top = 7;
	rectPreview.right = rectWnd.Width() - 7;
	rectPreview.bottom = 26;

//	m_wndPreview.Create(NULL, NULL, WS_CHILD|WS_VISIBLE,
//		rectPreview, this, 0x1000);

	m_cbTack.Create(" ", WS_CHILD |WS_VISIBLE | BS_OWNERDRAW /*| 
			WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE*/,
					rectPreview, this, ID_PROP_TACK);
	m_cbTack.LoadBitmaps(IDB_PROP_TACK_OUT, IDB_PROP_TACK);
	m_cbTack.SizeToContent();

	rectPreview.left -= 20;
	rectPreview.right -= 20;

	m_cbWin.Create(" ", WS_CHILD |WS_VISIBLE | BS_OWNERDRAW,
					rectPreview, this, ID_PROP_MULT_WIN);
	m_cbWin.LoadBitmaps(IDB_PROP_MULT_WIN, IDB_PROP_MULT_WINS);
	m_cbWin.SizeToContent();

	if (m_bMultiWin)
		m_cbWin.SetState(m_bMultiWin);

	CenterWindow();
	return bResult;
}

void CPropertyPanel::PostNcDestroy()
{
	CPropertySheet::PostNcDestroy();
	delete this;
}

void CPropertyPanel::OnPropTack() 
{
	m_bPressedTack = !m_bPressedTack;	
	m_cbTack.SetState(m_bPressedTack);
}

void CPropertyPanel::OnPropMultiWin() 
{
	m_bMultiWin = !m_bMultiWin;
	m_cbWin.SetState(m_bMultiWin);
}

void CPropertyPanel::OnApply() 
{
/*	if (m_pDoc->IsKindOf( RUNTIME_CLASS(CMailboxesDoc)))
	{
//		m_PageMbx1.Apply();
//		m_PageMbx2.Apply();
//		m_PageMbx3.Apply();
	}
	else*/ if (m_pDoc->IsKindOf( RUNTIME_CLASS(CReadMessageDoc)))
	{
		m_PageReadMessage1.Apply();
		m_PageReadMessage2.Apply();
		((CReadMessageDoc*)m_pDoc)->m_Sum->m_TheToc->UpdateAllViews(NULL);
		((CReadMessageDoc*)m_pDoc)->UpdateAllViews(NULL);
	}
	m_bPropertyChanged = FALSE;
	m_cbApply.EnableWindow(m_bPropertyChanged);
}

void CPropertyPanel::UpdateApply()
{
	m_bPropertyChanged = TRUE;
	m_cbApply.EnableWindow(m_bPropertyChanged);
}


void CPropertyPanel::OnOK() 
{
	if (m_bPropertyChanged)
		OnApply();
	(GetParentFrame())->PostMessage(WM_CLOSE, 0L, 0L);
}

void CPropertyPanel::OnCancel() 
{
	(GetParentFrame())->PostMessage(WM_CLOSE, 0L, 0L);
}

void CPropertyPanel::OnKillFocus(CWnd* pNewWnd) 
{
	CPropertySheet::OnKillFocus(pNewWnd);
	
//	if (!m_bPressedTack)
//		(GetParentFrame())->PostMessage(WM_CLOSE, 0L, 0L);
}


