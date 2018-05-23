// QCCustomizeGeneral.cpp: implementation of the QCCustomizeGeneral class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "QCCustomizeGeneralPage.h"


#include "DebugNewHelpers.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(QCCustomizeGeneralPage, QCToolBarCmdPage)

QCCustomizeGeneralPage::QCCustomizeGeneralPage() : 
	QCToolBarCmdPage(	IDD_GENERAL_TOOLBAR_CUSTOMIZATION, IDS_GENERAL_TITLE )
{
	//{{AFX_DATA_INIT(QCCustomizeGeneralPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

QCCustomizeGeneralPage::~QCCustomizeGeneralPage()
{

}


BEGIN_MESSAGE_MAP(QCCustomizeGeneralPage, QCToolBarCmdPage)
	//{{AFX_MSG_MAP(QCCustomizeGeneralPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void QCCustomizeGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	QCToolBarCmdPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCCustomizeGeneralPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// QCCustomizeGeneralPage message handlers

BOOL QCCustomizeGeneralPage::OnInitDialog() 
{
	// Must have at least 1 btn group defined.
	ASSERT(m_btnGroups.GetSize() > 0);
	
	QCToolBarCmdPage::OnInitDialog();
	CListBox* pList = (CListBox*) GetDlgItem(IDC_TOOLBAR_LIST);
	
	ASSERT( pList );

	for(int i=0; i<m_btnGroups.GetSize(); i++)
	{
		// Create and initialise the toolbar
		SECBtnGroup* pGroup = (SECBtnGroup*) m_btnGroups[i];
		// Add the name of this toolbar to the listbox
		pList->AddString(pGroup->lpszTitle);
	}

	pList->SetCurSel(0);

	return TRUE;
}


