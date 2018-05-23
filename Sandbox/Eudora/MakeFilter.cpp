// MakeFilter.cpp : implementation file
//

#include "stdafx.h"

#include <QCUtils.h>

#include "resource.h"
#include "helpcntx.h"
#include "usermenu.h"
#include "summary.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "ReadMessageDoc.h"
#include "msgutils.h"
#include "tocdoc.h"
#include "controls.h"
#include "filtersd.h"
#include "filtersv.h"
#include "mainfrm.h"
#include "address.h"
#include "rs.h"
#include "newmbox.h"
#include "fileutil.h"
#include "guiutils.h"

#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"

extern QCMailboxDirector	g_theMailboxDirector;

#include "MakeFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CMakeFilter dialog


CMakeFilter::CMakeFilter(CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CMakeFilter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMakeFilter)
	m_InCheck = TRUE;
	m_ManualCheck = FALSE;
	m_OutCheck = FALSE;

	m_MatchRadio = MATCH_FROM;

	m_SubjectText.Empty();
	m_FromText.Empty();
	m_RecipText.Empty();

	m_ActionRadio = TRANS_NEW;
	m_NewMBoxName.Empty();
	//}}AFX_DATA_INIT
	
	m_pMailboxEdit = NULL;
	m_pDefFolder = NULL;
	m_bUserNamedMbox = FALSE;
	m_AllRecips.Empty();

	// Remove the last slash
	CString temp = EudoraDir.Left( EudoraDir.GetLength() - 1 );
	int slash = temp.ReverseFind('\\');
	m_RootFolder = m_DefFolder = temp.Right(temp.GetLength() - slash - 1);
	m_DiffFolder.Empty();
}


void CMakeFilter::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMakeFilter)
	DDX_Control(pDX, IDC_DELETE, m_DeleteRadio);
	DDX_Control(pDX, IDC_TRAN_NEW, m_NewMBoxRadio);
	DDX_Control(pDX, IDC_TRAN_DIFF, m_SelMboxRadio);
	DDX_Control(pDX, IDC_SUBJECT, m_SubRadio);
	DDX_Control(pDX, IDC_FROM, m_FromRadio);
	DDX_Control(pDX, IDC_RECIP, m_RecipRadio);
	DDX_Check(pDX, IDC_INCOMING, m_InCheck);
	DDX_Check(pDX, IDC_OUTGOING, m_OutCheck);
	DDX_Check(pDX, IDC_MANUAL, m_ManualCheck);
	DDX_Radio(pDX, IDC_FROM, m_MatchRadio);
	DDX_Text(pDX, IDC_SUB_TEXT, m_SubjectText);
	DDX_Text(pDX, IDC_FROM_TEXT, m_FromText);
	DDX_Radio(pDX, IDC_TRAN_NEW, m_ActionRadio);
	DDX_Text(pDX, IDC_NEW_MBOX, m_NewMBoxName);
	DDV_MaxChars(pDX, m_NewMBoxName, 31);
	DDX_Control(pDX, IDC_MAILBOX, m_MailboxButton);
	DDX_Control(pDX, IDC_DETAILS, m_Details);
	DDX_Control(pDX, IDC_MAKE_FILT_RECIP_COMBO, m_RecipCombo);
	DDX_Control(pDX, IDC_SUB_TEXT, m_SubjectEdit);
	DDX_Control(pDX, IDC_FROM_TEXT, m_FromEdit);
	DDX_CBString(pDX, IDC_MAKE_FILT_RECIP_COMBO, m_RecipText);
	DDX_Text(pDX, IDC_MAKE_FILT_DEF_FOLDER, m_DefFolder);
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMakeFilter, CHelpxDlg)
	//{{AFX_MSG_MAP(CMakeFilter)
	ON_BN_CLICKED(IDC_DETAILS, OnDetails)
	ON_BN_CLICKED(IDC_SUBJECT, OnSubjectRadio)
	ON_BN_CLICKED(IDC_FROM, OnFromRadio)
	ON_BN_CLICKED(IDC_RECIP, OnRecipRadio)
	ON_EN_KILLFOCUS(IDC_NEW_MBOX, OnKillfocusNewMbox)
	ON_BN_CLICKED(IDC_MAILBOX, OnMailbox)
	ON_EN_SETFOCUS(IDC_NEW_MBOX, OnSetfocusNewMbox)
	ON_EN_SETFOCUS(IDC_FROM_TEXT, OnSetfocusFromText)
	ON_EN_SETFOCUS(IDC_MAKE_FILT_RECIP_COMBO, OnSetfocusRecipText)
	ON_EN_SETFOCUS(IDC_SUB_TEXT, OnSetfocusSubText)
	ON_CBN_SELENDOK(IDC_MAKE_FILT_RECIP_COMBO, OnRecipRadio)
	//}}AFX_MSG_MAP
	ON_COMMAND( ID_NEW_FILTER_MBOX_IN_ROOT, CMailboxButton::OnNewMailboxInRoot )
	ON_COMMAND_EX_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, CMailboxButton::OnDynamicCommand )
	ON_UPDATE_COMMAND_UI_RANGE( QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, CMailboxButton::OnUpdateDynamicCommand )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMakeFilter message handlers

/////////////////////////////////////////////////////////////////////////////
BOOL CMakeFilter::OnInitDialog() 
{
	CHelpxDlg::OnInitDialog();
	m_pMailboxEdit = (CEdit *)GetDlgItem(IDC_NEW_MBOX);
	ASSERT(m_pMailboxEdit);
	m_pMailboxEdit->LimitText(31);
	
	m_pDefFolder = (CEdit *)GetDlgItem(IDC_MAKE_FILT_DEF_FOLDER);
	ASSERT(m_pDefFolder);

	CRString in(IDS_IN_MBOX_NAME);
	m_MailboxButton.SetWindowText(in);

	SetupRecipCombo();
	EnableSelectedCtrl((MATCH_COND) m_MatchRadio);

	// Disable unusable options...
	if (m_SubjectText.IsEmpty())
		m_SubRadio.EnableWindow(FALSE);
	
	if (m_FromText.IsEmpty())
		m_FromRadio.EnableWindow(FALSE);

	if (m_AllRecips.IsEmpty())
		m_RecipRadio.EnableWindow(FALSE);
	
	return TRUE;  
}

/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::OnKillfocusNewMbox() 
{
	// If the user actually modified the mailbox name, no longer change
	// to what we think is appropriate
	CString curText;
	m_pMailboxEdit->GetWindowText(curText);
	if (curText != m_NewMBoxName)
	{
		m_bUserNamedMbox = TRUE;
		m_NewMBoxName = curText;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::EnableSelectedCtrl(MATCH_COND matchSel /*= MATCH_FROM*/)
{
	m_RecipCombo.EnableWindow(FALSE);
	m_FromEdit.EnableWindow(FALSE);
	m_SubjectEdit.EnableWindow(FALSE);

	switch (matchSel)
	{
		case MATCH_RECIPIENT:
			m_RecipCombo.EnableWindow();
			break;
		case MATCH_FROM:
			m_FromEdit.EnableWindow();
			break;
		case MATCH_SUBJECT:
			m_SubjectEdit.EnableWindow();
			break;
	}

}

/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::SetupMailboxSuggest(const char *sugName, UINT defFolderID, BOOL isAddress /*=TRUE*/)
{
	char szDefFolder[_MAX_PATH+1];
	::GetIniString(defFolderID, szDefFolder, sizeof(szDefFolder));

	if (szDefFolder && *szDefFolder)
	{
		m_DefFolder = m_RootFolder + '\\' + szDefFolder;
		m_DiffFolder = szDefFolder;
	}
	else
	{
		m_DefFolder = m_RootFolder;
		m_DiffFolder.Empty();
	}
	
	if (m_pDefFolder)
		m_pDefFolder->SetWindowText(m_DefFolder);


	// See if we need to strip of the @ sign
	if(isAddress)
	{
		char *buffer = new char[::SafeStrlenMT(sugName) + 1];
		strcpy(buffer, sugName);
		char *sugAddress = GetRealName(buffer);
		
		m_NewMBoxName = sugAddress;

 		int at = -1;
		at = m_NewMBoxName.Find( '@' ); 
		
		if (at > 0)
			m_NewMBoxName = m_NewMBoxName.Left( at );

		delete []buffer;
	}
	else
		m_NewMBoxName = sugName;

	
	// Limit the mailbox name length
	m_NewMBoxName = m_NewMBoxName.Left(31);

}


/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::OnSubjectRadio() 
{
	// Update the suggested Mailbox Name
	if (!m_bUserNamedMbox)
	{
		SetupMailboxSuggest(m_SubjectText, IDS_INI_FILT_FOLDER_SUBJECT, FALSE);
		m_pMailboxEdit->SetWindowText(m_NewMBoxName);
	}
	EnableSelectedCtrl(MATCH_SUBJECT);
		
}

/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::OnFromRadio() 
{
	// Update the suggested Mailbox Name
	if (!m_bUserNamedMbox)
	{
		SetupMailboxSuggest(m_FromText, IDS_INI_FILT_FOLDER_FROM);
		m_pMailboxEdit->SetWindowText(m_NewMBoxName);
	}
	EnableSelectedCtrl(MATCH_FROM);
}

/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::OnRecipRadio() 
{
	// Update the suggested Mailbox Name
	if (!m_bUserNamedMbox)
	{
		int curSel = m_RecipCombo.GetCurSel();
		if (curSel >= 0)
		{
			m_RecipCombo.GetLBText( curSel, m_RecipText);
			SetupMailboxSuggest(m_RecipText, IDS_INI_FILT_FOLDER_RECIP);
		}
		else
			m_NewMBoxName.Empty();

		m_pMailboxEdit->SetWindowText(m_NewMBoxName);
	}

	EnableSelectedCtrl(MATCH_RECIPIENT);
}



/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::OnOK()
{
	// Make the filter and close the dialog 
	CFilter *pSmartFilter = SetupFilter();

	if (pSmartFilter)
	{
		GetFiltersDoc()->CreateFilterItem(pSmartFilter);
		CHelpxDlg::OnOK();
	}
}


/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::OnDetails() 
{
	// Make the filter
	CFilter *pSmartFilter = SetupFilter();

	if (pSmartFilter)
	{
		// bring up the filters dialog with this new one selected
		CMainFrame* p_mainframe = (CMainFrame *) AfxGetMainWnd();
		ASSERT(p_mainframe);
		p_mainframe->SendMessage(WM_COMMAND, ID_WINDOW_FILTERS);

		ASSERT(pSmartFilter);
		
		GetFiltersDoc()->CreateFilterItem(pSmartFilter);

		EndDialog( IDC_DETAILS );
	}
	
}


void CMakeFilter::OnMailbox()
{
	SetActionChoice(TRANS_DIFF);
	m_MailboxButton.SelectMailbox();
}

void CMakeFilter::OnSetfocusNewMbox() 
{
	SetActionChoice(TRANS_NEW);
}


void CMakeFilter::SetActionChoice(ACTION actionSel /*= TRANS_NEW*/)
{
	m_NewMBoxRadio.SetCheck(0);
	m_SelMboxRadio.SetCheck(0);
	m_DeleteRadio.SetCheck(0);

	switch (actionSel)
	{
		case TRANS_NEW:
			m_NewMBoxRadio.SetCheck(1);
			break;
		case TRANS_DIFF:
			m_SelMboxRadio.SetCheck(1);
			break;
		case DELETE_MSG:
			m_DeleteRadio.SetCheck(1);
			break;
	}
}


void CMakeFilter::OnSetfocusSubText() 
{
	SetMatchChoice(MATCH_SUBJECT);	
}

void CMakeFilter::OnSetfocusFromText() 
{
	SetMatchChoice(MATCH_FROM);
}

void CMakeFilter::OnSetfocusRecipText() 
{
	SetMatchChoice(MATCH_RECIPIENT);	
}



void CMakeFilter::SetMatchChoice( MATCH_COND matchSel /*= MATCH_FROM*/)
{
	m_SubRadio.SetCheck(0);
	m_FromRadio.SetCheck(0);
	m_RecipRadio.SetCheck(0);

	switch (matchSel)
	{
		case MATCH_RECIPIENT:
			m_RecipRadio.SetCheck(1);
			break;
		case MATCH_FROM:
			m_FromRadio.SetCheck(1);
			break;
		case MATCH_SUBJECT:
			m_SubRadio.SetCheck(1);
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CMakeFilter::SetupRecipCombo()
{
	m_RecipCombo.ResetContent();
	if (!m_AllRecips.IsEmpty())
	{
		const char* addrStart = m_AllRecips;
		const char* addrEnd;
		
		while (*addrStart)
		{
			addrEnd = FindAddressEnd(addrStart, FALSE);
			if (!addrEnd)
				break;
			CString address(addrStart, addrEnd - addrStart);
			address.TrimLeft(); address.TrimRight();
			m_RecipCombo.AddString(address);
			if (*addrEnd)
				addrStart = addrEnd + 1;
			else
				break;
		}
	}
	m_RecipCombo.SetCurSel(0);
}



/////////////////////////////////////////////////////////////////////////////
CFilter * CMakeFilter::SetupFilter()
{
	char *address;

	// Make sure the dlg data is current
	UpdateData();
	
	
	// Make the filter 
	CFilter *pSmartFilter = new CFilter();
	
	if (!pSmartFilter)
		return NULL;

	if (m_InCheck)
		pSmartFilter->SetIncoming();
	if (m_ManualCheck)
		pSmartFilter->SetManual();
	if (m_OutCheck)
		pSmartFilter->SetOutgoing();


	switch (m_MatchRadio)
	{
		case MATCH_SUBJECT:
			pSmartFilter->m_Header[0].LoadString(IDS_FHEADER_SUBJECT);
			pSmartFilter->m_Value[0] = m_SubjectText;
			break;
		case MATCH_FROM:
			// Just Match the Address part so it'll be a more general filter
			address = new char[::SafeStrlenMT(m_FromText) + 1];
			strcpy(address, m_FromText);
			StripAddress(address);
			pSmartFilter->m_Header[0].LoadString(IDS_FHEADER_FROM);
			pSmartFilter->m_Value[0] = address;
			delete []address;
			break;
		case MATCH_RECIPIENT:
			address = new char[::SafeStrlenMT(m_RecipText) + 1];
			strcpy(address, m_RecipText);
			StripAddress(address);
			pSmartFilter->m_Header[0].LoadString(IDS_FHEADER_ADDRESSEE);
			pSmartFilter->m_Value[0] = address;
			delete []address;
			break;
	}
	
	switch(m_ActionRadio)
	{
		case TRANS_NEW:
			// Create the new mailbox
			{
				BOOL dirOK = TRUE;
				QCMailboxCommand*	pCommand;
				QCMailboxCommand*	qcPath = NULL;
				if (!m_DiffFolder.IsEmpty())
				{
					qcPath = g_theMailboxDirector.FindByNamedPath(m_DiffFolder);
					if (!qcPath)
					{
						if (IDOK != WarnDialog(IDS_INI_WARN_BAD_FILT_DIR, IDS_ERR_MKFILT_NO_DIR, m_DiffFolder, m_RootFolder))
							dirOK = FALSE;
					}
				}

				if (dirOK)
				{
					pCommand = g_theMailboxDirector.AddCommand( m_NewMBoxName, MBT_REGULAR, qcPath );
					pCommand->Execute( CA_NEW_MAILBOX, NULL );

					// Set up the filters
					pSmartFilter->SetTransferMBOX(m_NewMBoxName, pCommand->GetPathname());
				}
				else
				{
					delete pSmartFilter;
					pSmartFilter = NULL;
				}
					
			}
			break;
		
		case TRANS_DIFF:
			pSmartFilter->SetTransferMBOX( m_MailboxButton.m_ButtonDesc, m_MailboxButton.m_MailboxPath);
			break;
		
		case DELETE_MSG:
			pSmartFilter->SetTransferMBOX( CRString( IDS_TRASH_MBOX_NAME ), EudoraDir + CRString( IDS_TRASH_MBOX_FILENAME ) + CRString( IDS_MAILBOX_EXTENSION ));
			break;
	}
	
	if (pSmartFilter)
		pSmartFilter->SetName();

	return pSmartFilter;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMakeFilter::GoFigure(CSumList *p_SummaryList)
{
	BOOL ALL_OK = TRUE;
	CSummary* p_Summary = NULL;
	POSITION pos = p_SummaryList->GetHeadPosition();
	if (pos)
		p_Summary = p_SummaryList->GetNext(pos);
	
	ASSERT(p_Summary);
	
	if (!p_Summary)
		return FALSE;
	
	if (p_Summary->IsComp())
	{
		m_InCheck = FALSE;
		m_ManualCheck = TRUE;
		m_OutCheck = TRUE;
	}
	else
	{
		m_InCheck = TRUE;
		m_ManualCheck = TRUE;
		m_OutCheck = FALSE;
	}

	BOOL SubConflict = FALSE, FromConflict = FALSE, RecipConflict = FALSE;
	
	while (p_Summary && (!SubConflict || !FromConflict || !RecipConflict))
	{
		if (!SubConflict)
			SubConflict = GoFigureSubject(p_Summary);		
	
		if (!FromConflict)
			FromConflict = GoFigureFrom(p_Summary);
		
		if (!RecipConflict)
			RecipConflict = GoFigureRecip(p_Summary);
		
		if (pos)
			p_Summary = p_SummaryList->GetNext(pos);
		else
			p_Summary = NULL;
	}


	if (!m_FromText.IsEmpty())
	{
		SetupMailboxSuggest(m_FromText, IDS_INI_FILT_FOLDER_FROM);
		m_MatchRadio = MATCH_FROM;
	}

	else if (!m_AllRecips.IsEmpty())
	{
		int commaOffset = m_AllRecips.Find(',');
		if (commaOffset < 0)
			m_RecipText = m_AllRecips;
		else
			m_RecipText = m_AllRecips.Left(commaOffset);
		
		SetupMailboxSuggest(m_RecipText, IDS_INI_FILT_FOLDER_RECIP);
		m_MatchRadio = MATCH_RECIPIENT;
	}
	

	else if (!m_SubjectText.IsEmpty())
	{
		SetupMailboxSuggest(m_SubjectText, IDS_INI_FILT_FOLDER_SUBJECT, FALSE);
		m_MatchRadio = MATCH_SUBJECT;
	}


	else if (m_SubjectText.IsEmpty() && m_FromText.IsEmpty() && m_RecipText.IsEmpty())
	{
		ErrorDialog(IDS_MAKE_FILTER_ERROR);
		ALL_OK = FALSE;
	}
	else
		m_MatchRadio = MATCH_SUBJECT;


	m_ActionRadio = TRANS_NEW;
	return ALL_OK;

}

/////////////////////////////////////////////////////////////////////////////
//	return: FALSE if this is OK
//			TRUE if a conflict occured
/////////////////////////////////////////////////////////////////////////////
BOOL CMakeFilter::GoFigureSubject(CSummary *p_Summary)
{
	BOOL ConflictSubject = FALSE;
	const char *nextSubject = RemoveSubjectPrefixMT((LPCSTR)p_Summary->m_Subject);

	// If this is the first go around ... it will be empty
	if (m_SubjectText.IsEmpty())
		m_SubjectText = nextSubject;
	// Do a real compare ... if it fails this field can't be used for a filter
	else if (m_SubjectText.CompareNoCase( nextSubject ))
	{
		ConflictSubject = TRUE;
		m_SubjectText.Empty();
	}

	return ConflictSubject;
}
/////////////////////////////////////////////////////////////////////////////
// These could be complete 822 address lines, just compare address part
/////////////////////////////////////////////////////////////////////////////
int CMakeFilter::CompareAddress(const char *add1, const char *add2)
{
	int cmp = 0;

	// Since StripAddress mucks with the string, lets make a new copies 
	char *str1 = new char [::SafeStrlenMT(add1) + 1];
	strcpy(str1, add1);
	StripAddress(str1);

	char *str2 = new char [::SafeStrlenMT(add2) + 1];
	strcpy(str2, add2);
	StripAddress(str2);
	
	cmp = stricmp(str1, str2);

	if (str1) delete []str1;
	if (str2) delete []str2;

	return cmp;
}


/////////////////////////////////////////////////////////////////////////////
//	return: FALSE if this is OK
//			TRUE if a conflict occured
/////////////////////////////////////////////////////////////////////////////
BOOL CMakeFilter::GoFigureFrom(CSummary *p_Summary)
{
	CString szFrom;
	BOOL ConflictFrom = FALSE;
	
	BOOL NewDoc = (p_Summary->FindMessageDoc() == NULL);
	CMessageDoc *p_Doc = p_Summary->GetMessageDoc();

	if (!p_Doc)
		return FALSE;

	if (p_Doc->IsKindOf(RUNTIME_CLASS(CReadMessageDoc)))
	{
		char *text = p_Doc->GetText();
		if (text)
		{
			char * from = HeaderContents(IDS_HEADER_FROM, text);

			//HeaderContents allocates ... so must delete
			if (from) 
			{
				szFrom = from;
				delete []from;
			}
		}
	}
	else if (p_Doc->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)))
	{
		// Make sure the message is loaded
		((CCompMessageDoc *)p_Doc)->GetText();
		szFrom = ((CCompMessageDoc *)p_Doc)->m_Headers[HEADER_FROM];
	}
	else
	{
		ASSERT(0);
		return FALSE;
	}

	if (NewDoc)
		p_Doc->OnCloseDocument();
	
	if (!szFrom.IsEmpty())
	{
		// If this is the first go around ... it will be empty
		if (m_FromText.IsEmpty())
			m_FromText = szFrom;
		
		// Compare just the address...if fails, we can't filter on this
		else if (CompareAddress(szFrom, m_FromText))
		{
			ConflictFrom = TRUE;
			m_FromText.Empty();
		}
	}

	return ConflictFrom;
}


/////////////////////////////////////////////////////////////////////////////
//	return: FALSE if this is OK
//			TRUE if a conflict occured
/////////////////////////////////////////////////////////////////////////////
BOOL CMakeFilter::GoFigureRecip(CSummary *p_Summary)
{
	CString szTo, szCC, szFrom;

	BOOL ConflictRecip = FALSE;	
	BOOL NewDoc = (p_Summary->FindMessageDoc() == NULL);
	CMessageDoc *p_Doc = p_Summary->GetMessageDoc();

	if (!p_Doc)
		return FALSE;

	// Get the data!
	if (p_Doc->IsKindOf(RUNTIME_CLASS(CReadMessageDoc)))
	{
		char *text = p_Doc->GetText();
		if (text)
		{
			char * to = HeaderContents(IDS_HEADER_TO, text);	
			char * cc = HeaderContents(IDS_HEADER_CC, text);


			//HeaderContents allocates ... so must delete
			if (to) 
			{	
				szTo = to; 
				delete []to;
			}
			if (cc)
			{
				szCC = cc;
				delete []cc;
			}
		}
	}
	else if (p_Doc->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)))
	{
		// Make sure the message is loaded
		((CCompMessageDoc *)p_Doc)->GetText();
		szTo = ((CCompMessageDoc *)p_Doc)->m_Headers[HEADER_TO];
		szCC = ((CCompMessageDoc *)p_Doc)->m_Headers[HEADER_CC];
	}
	else
	{
		ASSERT(0);
		return FALSE;
	}

	if (NewDoc)
		p_Summary->NukeMessageDocIfUnused();

	// Move all the address into the To Field
	if (!szTo.IsEmpty() )
	{
		if (!szCC.IsEmpty() )
		{
			szTo += ',';
			szTo += szCC;
		}
	}
	else
		szTo = szCC;

	// Just get out the Addresses
	if (!szTo.IsEmpty())
	{
		CString curRecipText;

		char * addTo = NULL, *tmpTo = NULL;

		// Just get the e-mail address
		tmpTo = addTo = new char [szTo.GetLength() + 1];
		strcpy(addTo, szTo);
		char* end = addTo + ::SafeStrlenMT(addTo);
		
		while(addTo < end)
		{
			char *addEnd = FindAddressEnd(addTo, FALSE);
			if (!addEnd)
				break;

			const BOOL bNoMore = (0 == *addEnd);

			*addEnd = 0;
			if (!curRecipText.IsEmpty())
				curRecipText += ',';
			curRecipText += addTo;
			if (bNoMore)
				break;
			addTo = addEnd + 1;
		}
		if (tmpTo) delete [] tmpTo;

		ConflictRecip = IntersectRecip(curRecipText);
	}
	
	return ConflictRecip;
}

/////////////////////////////////////////////////////////////////////////////
// Return TRUE - Conflict occured
//		  FALSE - It's OK
/////////////////////////////////////////////////////////////////////////////
BOOL CMakeFilter::IntersectRecip(CString& addList)
{
	BOOL Conflict = FALSE;
	char address[255], rawAddress[255];;

	// 1st time through, just return the whole string
	if (m_AllRecips.IsEmpty())
	{
		m_AllRecips = addList;
		return Conflict;
	}
	
	CString accumAdd;
	char *locAddList, *tmp;
	
	locAddList = tmp = new char[addList.GetLength() + 1];
	strcpy( locAddList, addList) ;
	
	// For each address in add List, see if it's in m_AllRecips
	while( *locAddList )
	{
		// Get the first address off
		char* addrEnd = FindAddressEnd(locAddList, FALSE);
		if (!addrEnd)
			break;

		const BOOL bNoMore = (0 == *addrEnd);

		*addrEnd = 0;
		strncpy(address, locAddList, sizeof(address) - 1);
		address[sizeof(address) - 1] = 0;
		
		::TrimWhitespaceMT(address);
		strcpy( rawAddress, address);

		// Just get the address out to compare
		::StripAddress(address);
		
		int dupOffset = m_AllRecips.Find(address);
		{
			if (dupOffset >= 0)
			{
				if (!accumAdd.IsEmpty())
					accumAdd += ',';
				accumAdd += rawAddress;
				Conflict = FALSE;
			}
		}
		
		if (bNoMore)
			break;
		locAddList = addrEnd + 1;
	}
	
	if (accumAdd.IsEmpty())
	{
		m_AllRecips.Empty();
		Conflict = TRUE;
	}
	else
		m_AllRecips = accumAdd;

	delete []tmp;

	return Conflict;
}
