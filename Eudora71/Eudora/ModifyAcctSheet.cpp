#include "stdafx.h"
#include "regif.h"
#include "rs.h"
#include "PersonalityView.h"
#include "persona.h"
#include "resource.h"
#include "mainfrm.h"
#include "GuiUtils.h" // WarnYesNoDialog

#ifdef IMAP4
#include "QCMailboxDirector.h"
extern QCMailboxDirector g_theMailboxDirector;
#endif

#include "ModifyAcctSheet.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CModifyAcctSheet

IMPLEMENT_DYNAMIC(CModifyAcctSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CModifyAcctSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CModifyAcctSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		ON_BN_CLICKED(IDHELP, OnHelp)
		ON_BN_CLICKED(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// --------------------------------------------------------------------------

// CModifyAcctSheet -- Modify an existing personality
CModifyAcctSheet::CModifyAcctSheet(LPCTSTR pszCaption,
								   CWnd* pParentWnd/* NULL*/,
								   UINT iSelectPage/* 0*/,
								   const CString& strName /*<Dominant>*/,
								   bool bCreateNew /* = false */)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage),
		m_pParent(pParentWnd),
		m_bCreateNew(bCreateNew)
{
	if (m_bCreateNew)
		VERIFY(m_params.GetDefaultParams());
	else
		VERIFY(g_Personalities.GetParams(strName, m_params));

	m_original_params = m_params;
}

// --------------------------------------------------------------------------

CModifyAcctSheet::~CModifyAcctSheet()
{
	delete m_pGenericDlg;
	delete m_pServersDlg;

	m_pGenericDlg = NULL;
	m_pServersDlg = NULL;
}

// --------------------------------------------------------------------------

void CModifyAcctSheet::UpdateWazoo(LPCSTR sPersName)
{
	// instead of using the Parent of this wizard, grab the CPersonalityView
	// from the Mainframe

	CMainFrame::QCGetMainFrame()->SendMessage(WM_COMMAND, IDM_VIEW_PERSONALITIES);

//	CMainFrame* pMainWnd = (CMainFrame *) ::AfxGetMainWnd();	//FORNOW
//	ASSERT_KINDOF(CMainFrame, pMainWnd);						//FORNOW
	
	CPersonalityView *pView	=	CMainFrame::QCGetMainFrame()->GetActivePersonalityView();
	ASSERT(pView);
	
	if (pView)
	{
		pView->PopulateView();
		
		if (sPersName)
		{
			//
			// Select the newly-added personality.
			//

			LV_FINDINFO lvFindInfo;
			lvFindInfo.flags = LVFI_STRING;
			lvFindInfo.psz = sPersName;
			lvFindInfo.lParam = NULL;

			CListCtrl& theCtrl = pView->GetListCtrl();
			int nIndex = theCtrl.FindItem(&lvFindInfo);
			ASSERT(nIndex != -1);
			theCtrl.SetItemState(nIndex, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		}
	}
}

// --------------------------------------------------------------------------

void CModifyAcctSheet::Create()
{
	m_pGenericDlg = DEBUG_NEW CModifyAcct_Generic(this);
	m_pServersDlg = DEBUG_NEW CModifyAcct_Servers(this);

	AddPage(m_pGenericDlg);
	AddPage(m_pServersDlg);

	this->m_psh.dwFlags |= PSH_NOAPPLYNOW ; // remove the APPLY button

	if (DoModal() == IDOK)
	{
		m_params.UpdatePopAccount();

		if (m_bCreateNew)
		{
			ASSERT(!g_Personalities.IsA(m_params.PersName));
			VERIFY(g_Personalities.Add(m_params));
		}
		else
		{
			ASSERT(g_Personalities.IsA(m_params.PersName));
			VERIFY(g_Personalities.Modify(m_params));
		}

		UpdateWazoo(m_params.PersName);

#ifdef IMAP4
		g_theMailboxDirector.ModifyPersona(m_params.PersName);
#endif
	}
}

// --------------------------------------------------------------------------
// to add context sensitive help to the dialog

BOOL CModifyAcctSheet::OnInitDialog()
{
	this->ModifyStyleEx(0, WS_EX_CONTEXTHELP) ;

	return CPropertySheet::OnInitDialog();
}

// --------------------------------------------------------------------------

void CModifyAcctSheet::OnHelp()
{
	WinHelp(0L,HELP_FINDER); //voila!
}

// --------------------------------------------------------------------------

void CModifyAcctSheet::OnOK()
{
	ASSERT(m_pGenericDlg);
	ASSERT(m_pServersDlg);

	m_pGenericDlg->SaveDialogSettings();
	m_pServersDlg->SaveDialogSettings();

	if ((m_bCreateNew) && (!m_pGenericDlg->IsNameAcceptable(true)))
		return;

	if ((!m_bCreateNew) && (m_original_params.bIMAP) && (!m_params.bIMAP))
	{
		// Changing from IMAP to non-IMAP, so make sure the user is not an idiot
		if (WarnYesNoDialog(0, IDS_CHANGE_IMAP_TO_POP) == IDCANCEL) // BUG ALERT! WarnYesNoDialog returns IDCANCEL when the user selects NO
		{
			// OK they decided they truly are an idiot, so don't quit the dialog
			return;
		}
	}

	EndDialog(IDOK);
}
