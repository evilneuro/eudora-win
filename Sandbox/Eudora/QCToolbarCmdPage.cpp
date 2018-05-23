// QCToolBarCmdPage.cpp : implementation file
//

#include "stdafx.h"
#include "QCToolBarCmdPage.h"
#include "QCCustomizeToolBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// QCToolBarCmdPage property page

IMPLEMENT_DYNAMIC(QCToolBarCmdPage, SECToolBarCmdPage)

QCToolBarCmdPage::QCToolBarCmdPage( UINT uID, UINT uTitleID ) : SECToolBarCmdPage( uID, uTitleID )
{
	//{{AFX_DATA_INIT(QCToolBarCmdPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCToolBarCmdPage::~QCToolBarCmdPage()
{
}

void QCToolBarCmdPage::DoDataExchange(CDataExchange* pDX)
{
	SECToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCToolBarCmdPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(QCToolBarCmdPage, SECToolBarCmdPage)
	//{{AFX_MSG_MAP(QCToolBarCmdPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCToolBarCmdPage message handlers

BOOL QCToolBarCmdPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Calc. where to create the toolbars on the page by positioning them
	// slightly inside the IDC_TOOLBAR_FRAME static control.
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetDlgItem(IDC_TOOLBAR_FRAME)->GetWindowPlacement(&wp);

	CRect rect = wp.rcNormalPosition;
	int x = 6;			// in DBU
	int y = 8;
	SECConvertDialogUnitsToPixels(*(GetFont()), x, y);
	rect.DeflateRect(x, y);

	int nID = AFX_IDW_TOOLBAR + 0xff;
	for(int i=0; i<m_btnGroups.GetSize(); i++, nID--)
	{
		// Create and initialise the toolbar
		SECBtnGroup* pGroup = (SECBtnGroup*) m_btnGroups[i];
		SECCustomizeToolBar* pToolBar = new QCCustomizeToolBar;
		pGroup->pToolBar = pToolBar;

		DWORD dwStyle = WS_VISIBLE | WS_CHILD | CBRS_BORDER_3D | 
						CBRS_SIZE_DYNAMIC | CBRS_FLOATING;
		DWORD dwExStyle = 0L;
		if(m_pManager && m_pManager->FlyByEnabled())
			dwStyle |= CBRS_FLYBY;

		if(m_pManager && m_pManager->CoolLookEnabled())
			dwExStyle |= (CBRS_EX_COOLBORDERS|CBRS_EX_GRIPPER);

		if(!pToolBar->CreateEx(dwExStyle, this, dwStyle, nID))
			return FALSE;

		if(m_pManager)
		{
			// Initialise bitmap for toolbar
			m_pManager->SetToolBarInfo(pToolBar);

			pToolBar->SetManager(m_pManager);

			// Set owner - used for flyby help
			pToolBar->SetOwner(m_pManager->GetFrameWnd());
		}

		if(!pToolBar->SetButtons(pGroup->lpBtnIDs, pGroup->nBtnCount))
			return FALSE;

		// Position and size the new toolbar.
		CRect r = rect;
		pToolBar->SetWindowPos(&wndTop, r.left, r.top, r.Width(), r.Height(), 
							   SWP_DRAWFRAME);
		pToolBar->DoSize(r);
		if(i != 0)
			// All toolbars are hidden except the very first one.
			pToolBar->ShowWindow(SW_HIDE);
	}

	return TRUE;
}


BOOL QCToolBarCmdPage::OnSetActive()
{
	INT iPos;
	QCCustomizeToolBar*	pBar;
	SECBtnGroup*		pBtnGroup;

	if(m_pManager)
	{
		// Put all toolbars into config mode.
		m_pManager->EnableConfigMode(TRUE);

		// Re-enable the main frame - so toolbar buttons can be dragged
		m_pManager->EnableMainFrame();

		// Don't allow buttons to be dropped on me.
		m_pManager->SetNoDropWnd(GetParent());

		// for the same reason as below, we need to update any bitmap changes		
		for( iPos = 0; iPos < m_btnGroups.GetSize(); iPos ++ )
		{
			pBtnGroup = ( SECBtnGroup* ) m_btnGroups[ iPos ];		
			pBar = ( QCCustomizeToolBar* ) ( pBtnGroup->pToolBar );		
			if( pBar->IsKindOf( RUNTIME_CLASS( QCCustomizeToolBar ) ) )
			{
				m_pManager->SetToolBarInfo( pBar );
			}
		}
	}

	// EnableConfigMode will only work on those toolbars returned by
	// the main frames GetControlBar function. Since my custom toolbars do
	// not have the main frame as a parent, they will not be in the list.
	// I therefore do this by hand.
	
#ifdef ORIGINAL
	CListBox* pList = (CListBox*) GetDlgItem(IDC_TOOLBAR_LIST);
	int nIndex = pList->GetCurSel();
	if(nIndex != LB_ERR)
	{
		((SECBtnGroup*) m_btnGroups[nIndex])->pToolBar->EnableConfigMode(TRUE);
		m_nCurSel = nIndex;
	}
#endif
		
	return TRUE;
}


UINT QCToolBarCmdPage::TranslateID(
UINT	uID )
{
	return uID;
}
