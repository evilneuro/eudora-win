// EuGraphPpg.cpp : Implementation of the CEuGraphPropPage property page class.

#include "stdafx.h"
#include "EuGraph.h"
#include "EuGraphPpg.h"

#include "DebugNewHelpers.h"


IMPLEMENT_DYNCREATE(CEuGraphPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CEuGraphPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CEuGraphPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CEuGraphPropPage, "EUGRAPH.EuGraphPropPage.1",
	0x51e2c868, 0x367b, 0x11d4, 0x85, 0x54, 0, 0x8, 0xc7, 0xd3, 0xb6, 0xf8)


/////////////////////////////////////////////////////////////////////////////
// CEuGraphPropPage::CEuGraphPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CEuGraphPropPage

BOOL CEuGraphPropPage::CEuGraphPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_EUGRAPH_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphPropPage::CEuGraphPropPage - Constructor

CEuGraphPropPage::CEuGraphPropPage() :
	COlePropertyPage(IDD, IDS_EUGRAPH_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CEuGraphPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphPropPage::DoDataExchange - Moves data between page and properties

void CEuGraphPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CEuGraphPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CEuGraphPropPage message handlers
