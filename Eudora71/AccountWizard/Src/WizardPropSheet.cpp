// WizardPropSheet.cpp : implementation file
//

#include "stdafx.h"
#include "regif.h"
#include "rs.h"
#include "mainfrm.h"
#include "persona.h"
#include "PersonalityView.h"

#include "jjnet.h"
#include "progress.h"
#include "guiutils.h"

#include "WizardAcapPage.h"
#include "WizardClientPage.h"
#include "WizardConfirmPage.h"
#include "WizardFinishPage.h"
#include "WizardImapDirPage.h"
#include "WizardInServerPage.h"
#include "WizardLoginPage.h"
#include "WizardOutServerPage.h"
#include "WizardPersNamePage.h"
#include "WizardRetAddPage.h"
#include "WizardWelcomePage.h"
#include "WizardYourNamePage.h"
#include "WizardImportPage.h"

#include "QCMailboxDirector.h"
extern QCMailboxDirector g_theMailboxDirector;

#include "WizardPropSheet.h"

#include "DebugNewHelpers.h"


// --------------------------------------------------------------------------
// CWizardPropSheet

// --------------------------------------------------------------------------

#define safedelete(p) { if (p) { delete (p); (p) = NULL; } }

// --------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CWizardPropSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CWizardPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CWizardPropSheet)
		ON_BN_CLICKED( IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWizardPropSheet::CWizardPropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, bool bDominant /*false*/)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage), m_bIsDominant(bDominant), m_bIsACAPSettings(false)
{
	InitializeAll();
}

CWizardPropSheet::CWizardPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage, bool bDominant /*false*/)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage), m_bIsDominant(bDominant), m_bIsACAPSettings(false)
{
	InitializeAll();
}

CWizardPropSheet::~CWizardPropSheet()
{
	safedelete(m_pPersName);
	safedelete(m_pWelcome);
	safedelete(m_pClientDlg);
	safedelete(m_pAcapDlg);
	safedelete(m_pDialConfirm);
	safedelete(m_pYourName);
	safedelete(m_pRetAdd);
	safedelete(m_pDialLogin);
	safedelete(m_pInServer);
	safedelete(m_pFinish);
	safedelete(m_pImapDir);
	safedelete(m_pOutServer);
	safedelete(m_pImportPage);
	
	safedelete(m_pParams);
	safedelete(m_pImporter);
}

/////////////////////////////////////////////////////////////////////////////
// CWizardPropSheet message handlers

void CWizardPropSheet::InitializeAll()
{
	m_SettingType = PST_UNKNOWN;
	m_ConfirmChange = ACH_UNKNOWN;

	m_pWelcome = NULL;
	m_pPersName = NULL;
	m_pClientDlg = NULL;
	m_pAcapDlg = NULL;
	m_pDialConfirm = NULL;
	m_pYourName = NULL;
	m_pRetAdd = NULL;
	m_pDialLogin = NULL;
	m_pInServer = NULL;
	m_pOutServer = NULL;
	m_pFinish = NULL;
	m_pImapDir = NULL;

	//create current accountinfo object
	m_pParams = DEBUG_NEW CPersParams;

	// The import mail object is only created if the user choses to import settings
	m_pImporter = NULL;
	m_pImportChild = NULL;
	m_bImportMail = m_bImportAddr = m_bImportLDIF = false;
	m_sImportFileLDIF.Empty();

	CFont ANSIFont;
	ANSIFont.CreateStockObject(ANSI_VAR_FONT); // Get the stock variable-width font

	LOGFONT fontdata;
	ANSIFont.GetObject(sizeof(LOGFONT), &fontdata); // Get the font metrics

	fontdata.lfWeight = FW_BOLD; // Make it bold
	fontdata.lfHeight = 28; // Make it bigger

	m_StatFont.CreateFontIndirect(&fontdata); // Create the static font

	AddPages(m_bIsDominant);
}

// --------------------------------------------------------------------------

//this function calls DoModal and encapsulates all writing to Eudora.ini
// if user hits finish. This simplifies the responsibility of the caller

void CWizardPropSheet::StartWizard()
{
	m_pParams->GetDefaultParams();

	if (DoModal() == ID_WIZFINISH)
	{
		if (m_SettingType == CWizardPropSheet::PST_ADVANCED)
		{
			CPersonalityView *pView = CMainFrame::QCGetMainFrame()->GetActivePersonalityView();
			
			if (!pView) { 
			// Due to "just in time" wazoo window creation, personality view may not 
			// have been created yet. Try switching to the personality view to make 
			// sure that it's been created, and then try to get it again. 
			CMainFrame::QCGetMainFrame()->SendMessage(WM_COMMAND, IDM_VIEW_PERSONALITIES); 
			pView = CMainFrame::QCGetMainFrame()->GetActivePersonalityView(); 
			
			} 

			ASSERT(pView);
			
			if (pView)
			{
				if (m_bIsDominant)
					pView->PostMessage(msgPersViewEditDom);
				else
					pView->PostMessage(msgPersViewAdvNew);
			}

			return;
		}

		m_pParams->UpdatePopAccount();

		CPersParams::ServType type;
		VERIFY(m_pParams->GetInServType(type));

		switch (type)
		{
			case CPersParams::IST_POP:
			{
				// No special defaults
			}
			break;

			case CPersParams::IST_IMAP:
			{
				m_pParams->bPassword = TRUE; // Default to passwords.
				m_pParams->bAPop = m_pParams->bKerberos = m_pParams->bRPA = FALSE;
			}
			break;
		}

		if (!m_bIsDominant)
		{
			// ok this is a new personality
			ASSERT(!m_pParams->PersName.IsEmpty());
			ASSERT(!g_Personalities.IsA( m_pParams->PersName));
			VERIFY(g_Personalities.Add(*m_pParams));
		}
		else
		{
			m_pParams->PersName = CRString(IDS_DOMINANT);
			// ok its a dominant account, which always exists (so just modify)
			VERIFY(g_Personalities.Modify(*m_pParams));
		}
		
		UpdateWazoo();

		if (m_SettingType == PST_IMPORT)
		{
			ASSERT(m_pImporter);
			ASSERT(m_pImportChild);

			if (m_bImportMail)
				m_pImporter->ImportMail2(m_pImportChild, this);

			if (m_bImportAddr || m_bImportLDIF)
			//if (m_bImportAddr)
				m_pImporter->ImportAddresses2(m_pImportChild, this);
		
			if (m_bImportPersona)
				m_pImporter->ImportPersonalities2(m_pImportChild, m_bIsProfile, NULL);

		}

		//
		// Notify mailbox director that a Persona has been added.
		g_theMailboxDirector.AddPersona( m_pParams->PersName, TRUE);
	}
	else 
	{
		// else user hit cancel at some time during wizard existence.
		// If something has been committed to the INI File, then go and delete it
		// For now this will only happen if the user is configuring from ACAP
		//
		// ..and this is VERY, VERY BAD.
		// We should commit nothing until the users choses to do so.

		//here we clear the INI file settings only if we got them
		//through the ACAP server and the user has cancelled. so we need undo
		//changes to the INI file that were got from the ACAP server
		//added the extra check so that we undo any INI settings only if they
		//were got through ACAP, since sites might have installed the INI file
		//with pre filled settings to allow users to set up their account easily
		//need to make sure these pre filled settings are not lost
		if( m_bIsDominant && m_bIsACAPSettings) // don't want to delete entries if this is a new personality!
			ClearIniEntries();
	}

}

// Check if the host is valid.  If not then give an error dialog to the user asking
// if they want to continue with the current host or edit it.
bool CWizardPropSheet::ValidateHost(LPCTSTR pHost, UINT ServiceID)
{
	if (GetIniShort(IDS_INI_WARN_VALIDATE_HOST) == FALSE)
		return true;

	UINT PortIniID;
	UINT DefaultPort;
	bool bContinue = false;

	switch (ServiceID)
	{
	case IDS_POP_SERVICE:	PortIniID = GetIniShort(IDS_INI_POP_PORT);  DefaultPort = 110; break;
	case IDS_IMAP_SERVICE:	PortIniID = GetIniShort(IDS_INI_IMAP_PORT); DefaultPort = 143; break;
	case IDS_SMTP_SERVICE:	PortIniID = GetIniShort(IDS_INI_SMTP_PORT); DefaultPort =  25; break;
	default:
		ASSERT(0);
		return false;
	}

	if (CreateNetConnection())
	{
		if (NetConnection->Open(pHost, ServiceID, PortIniID, DefaultPort) >= 0)
			bContinue = true;
		else
		{
			if (YesNoDialog(IDS_INI_WARN_VALIDATE_HOST, 0, IDS_ERR_VALIDATE_HOST, pHost) == IDCANCEL)
				bContinue = true;
		}
		delete NetConnection;
		NetConnection = NULL;
		CloseProgress();
	}

	return bContinue;
}

void CWizardPropSheet::OnHelp()
{
	WinHelp(0L,HELP_FINDER); //voila!
}

void CWizardPropSheet::UpdateWazoo()
{
	CPersonalityView *pView = CMainFrame::QCGetMainFrame()->GetActivePersonalityView() ;

	if (pView)
	{
		pView->PopulateView();
		
		//
		// Select the newly-added personality.
		//
		LV_FINDINFO lvFindInfo;
		lvFindInfo.flags = LVFI_STRING;
		lvFindInfo.psz = m_pParams->PersName;
		lvFindInfo.lParam = NULL;

		CListCtrl& theCtrl = pView->GetListCtrl();
		int nIndex = theCtrl.FindItem(&lvFindInfo);
		ASSERT(nIndex != -1);
		theCtrl.SetItemState(nIndex, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
}

void CWizardPropSheet::ClearIniEntries()
{
	SetIniString(IDS_INI_POP_ACCOUNT, NULL);
	SetIniString(IDS_INI_POP_SERVER, NULL);
	SetIniString(IDS_INI_SMTP_SERVER, NULL);
	SetIniString(IDS_INI_LOGIN_NAME, NULL);
	SetIniString(IDS_INI_REAL_NAME, NULL);
	SetIniString(IDS_INI_RETURN_ADDRESS, NULL);
	SetIniString(IDS_INI_ACAP_USER_ID, NULL);
	SetIniString(IDS_INI_ACAP_SERVER, NULL);

	//and flush the cache
	FlushINIFile();
	RemoveAllFromCache();
}


/////////////////////////////////////////////////////////////////////////////
// CWizardPropSheet message handlers

void CWizardPropSheet::AddPages(bool bDominant)
{
	if (bDominant)
	{
		// this IS dominant account (first-time startup)
		m_pWelcome = DEBUG_NEW CWizardWelcomePage(this);
		this->AddPage(m_pWelcome);
	}

	// Add the pages (in order) to the sheet
	m_pClientDlg = DEBUG_NEW CWizardClientPage(this);
	this->AddPage(m_pClientDlg);

	m_pAcapDlg = DEBUG_NEW CWizardAcapPage(this);
	this->AddPage(m_pAcapDlg);      // Only for ACAP

	m_pDialConfirm = DEBUG_NEW CWizardConfirmPage(this);
	this->AddPage(m_pDialConfirm);  // Only for ACAP
	
	m_pImportPage = DEBUG_NEW CWizardImportPage(this);
	this->AddPage(m_pImportPage);   // Only for import

	if (!bDominant)
	{
		// this is a not dominant account
		m_pPersName = DEBUG_NEW CWizardPersNamePage(this);
		this->AddPage(m_pPersName);
	}

	m_pYourName = DEBUG_NEW CWizardYourNamePage(this);
	this->AddPage(m_pYourName);
	
	m_pRetAdd = DEBUG_NEW CWizardRetAddPage(this);
	this->AddPage(m_pRetAdd);
	
	m_pDialLogin = DEBUG_NEW CWizardLoginPage(this);
	this->AddPage(m_pDialLogin);
	
	m_pInServer = DEBUG_NEW CWizardInServerPage(this);
	this->AddPage(m_pInServer);
	
	m_pImapDir = DEBUG_NEW CWizardImapDirPage(this);
	this->AddPage(m_pImapDir);      // Only for IMAP
	
	m_pOutServer = DEBUG_NEW CWizardOutServerPage(this);
	this->AddPage(m_pOutServer);

	m_pFinish = DEBUG_NEW CWizardFinishPage(this);
	this->AddPage(m_pFinish);

	// Turn on wizard mode
	this->SetWizardMode();
}
