// SelectPersonalityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include "QCImageList.h"
#include "persona.h"
#include "SelectPersonalityDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectPersonalityDlg dialog


CSelectPersonalityDlg::CSelectPersonalityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectPersonalityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectPersonalityDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectPersonalityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectPersonalityDlg)
	DDX_Control(pDX, IDC_PERSONALITY_LIST, m_PersonalityList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectPersonalityDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectPersonalityDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectPersonalityDlg message handlers

void CSelectPersonalityDlg::OnOK() 
{
	// Get the select
	int sel = m_PersonalityList.GetNextItem( -1, LVNI_SELECTED);

	if (sel >= 0)
		m_SelectedPersona = m_PersonalityList.GetItemText( sel, 0 );


	// Return the Selection
	CDialog::OnOK();
}

BOOL CSelectPersonalityDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// create the imagelist
	if (! m_ImageList.Load())
	{
		ASSERT(0);
		return FALSE;
	}

	m_PersonalityList.SetImageList(&m_ImageList, LVSIL_NORMAL);
	m_PersonalityList.SetImageList(&m_ImageList, LVSIL_SMALL);

	//
	// FORNOW, hardcode columns for the report view...
	//
	m_PersonalityList.InsertColumn(0, CRString(IDS_PERSONA_VIEW_NAME_LABEL), LVCFMT_LEFT, GetIniShort(IDS_INI_PERSONA_VIEW_NAME_WIDTH));
	m_PersonalityList.InsertColumn(1, CRString(IDS_PERSONA_VIEW_ACCOUNT_LABEL), LVCFMT_LEFT, GetIniShort(IDS_INI_PERSONA_VIEW_ACCOUNT_WIDTH));

	//
	// Save the current personality, and BE SURE TO RESTORE THIS
	// PERSONALITY BEFORE LEAVING THIS ROUTINE!!!!!
	//
	CString strCurrentPersona = g_Personalities.GetCurrent();

	//
	// Enumerate the list of personalities and each corresponding account.
	//
	for (LPSTR pszNameList = g_Personalities.List();
		 pszNameList && *pszNameList;
		 pszNameList += strlen(pszNameList) + 1)
	{
		LV_ITEM theItem;
		theItem.mask = LVIF_TEXT | LVIF_IMAGE;
		theItem.iItem = m_PersonalityList.GetItemCount();	
		theItem.iSubItem = 0;
		theItem.pszText = (char *) (const char *) pszNameList;
		theItem.iImage = QCMailboxImageList::IMAGE_PERSONALITY;
		theItem.lParam = NULL;

		// insert the item
		int nIndex = m_PersonalityList.InsertItem(&theItem);
		if (nIndex < 0)
		{
			g_Personalities.SetCurrent(strCurrentPersona);
			return FALSE;
		}

		//
		// Temporarily switch to this personality so that we can get
		// the account name.
		//
		CString strPersona(pszNameList);
		if (g_Personalities.SetCurrent(strPersona))
			m_PersonalityList.SetItemText(theItem.iItem, 1, GetIniString(IDS_INI_POP_ACCOUNT));
		else
		{
			ASSERT(0);
		}
	}

	//
	// Forcibly set the "focus" to the first persona in the list.  The
	// list control needs a focus item so that activated Wazoos
	// display a focus rect on the selected item.
	//
	m_PersonalityList.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);

	//
	// Last but not least, restore the current personality.
	//
	g_Personalities.SetCurrent(strCurrentPersona);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
