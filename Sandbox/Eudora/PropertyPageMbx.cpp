// PropertyPageMbx.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PropertyPageMbx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CPropertyPageMbx1, CPropertyPage)
IMPLEMENT_DYNCREATE(CPropertyPageMbx2, CPropertyPage)
IMPLEMENT_DYNCREATE(CPropertyPageMbx3, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMbx1 property page

CPropertyPageMbx1::CPropertyPageMbx1() : CPropertyPage(CPropertyPageMbx1::IDD)
{
	//{{AFX_DATA_INIT(CPropertyPageMbx1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

CPropertyPageMbx1::~CPropertyPageMbx1()
{
}

void CPropertyPageMbx1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyPageMbx1)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyPageMbx1, CPropertyPage)
	//{{AFX_MSG_MAP(CPropertyPageMbx1)
	ON_BN_CLICKED(IDC_PROP_MBX_COMPACT, OnPropMbxCompact)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMbx2 property page

CPropertyPageMbx2::CPropertyPageMbx2() : CPropertyPage(CPropertyPageMbx2::IDD)
{
	//{{AFX_DATA_INIT(CPropertyPageMbx2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

CPropertyPageMbx2::~CPropertyPageMbx2()
{
}

void CPropertyPageMbx2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyPageMbx2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyPageMbx2, CPropertyPage)
	//{{AFX_MSG_MAP(CPropertyPageMbx2)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMbx3 property page

CPropertyPageMbx3::CPropertyPageMbx3() : CPropertyPage(CPropertyPageMbx3::IDD)
{
	//{{AFX_DATA_INIT(CPropertyPageMbx3)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

CPropertyPageMbx3::~CPropertyPageMbx3()
{
}

void CPropertyPageMbx3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertyPageMbx3)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyPageMbx3, CPropertyPage)
	//{{AFX_MSG_MAP(CPropertyPageMbx3)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CPropertyPageMbx1::OnPropMbxCompact() 
{
	// TODO: Add your control notification handler code here
	
}

