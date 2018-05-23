// mtransop.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "mtransop.h"
#include "cursor.h"
#include "jjnet.h"
#include "QCFileIO.h"   //For CFileIO
#include "pop.h"
#include "HostList.h"
#include "persona.h"
#include "rs.h"
#include "QCSharewareManager.h"

#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CMailTransferOptionsDialog dialog


CMailTransferOptionsDialog::CMailTransferOptionsDialog(BOOL Sending /*= FALSE*/,
													   CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CMailTransferOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMailTransferOptionsDialog)
	m_RetrieveNew = TRUE;
	m_DeleteMarked = FALSE;
	m_RetrieveMarked = FALSE;
	m_DeleteAll = FALSE;
	m_FetchHeaders = FALSE;
	m_PersonaOptions = GetIniShort(IDS_INI_TRANSFER_PERSONA_OPTIONS);
	//}}AFX_DATA_INIT

	m_SendingFlag = Sending;

	if (m_SendingFlag)
	{
		m_Send = TRUE;
		m_RetrieveNew = FALSE;
	}
	else
	{
		m_Send = GetIniShort(IDS_INI_SEND_ON_CHECK);
		m_DeleteRetrieved = (GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER) == FALSE);
	}

	m_DeleteMarkedExist = FALSE;
	m_RetrieveMarkedExist = FALSE;
	m_lpNameList = NULL; 
}

void CMailTransferOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMailTransferOptionsDialog)
	DDX_Control(pDX, IDC_PERSONAS, m_Personas);
	DDX_Check(pDX, IDC_RETRIEVE_NEW_MAIL, m_RetrieveNew);
	DDX_Check(pDX, IDC_SEND_QUEUED_MESSAGES, m_Send);
	DDX_Check(pDX, IDC_DELETE_MARKED_MESSAGES, m_DeleteMarked);
	DDX_Check(pDX, IDC_RETRIEVE_MARKED_MESSAGES, m_RetrieveMarked);
	DDX_Check(pDX, IDC_DELETE_RETRIEVED_MESSAGES, m_DeleteRetrieved);
	DDX_Check(pDX, IDC_DELETE_ALL_MESSAGES, m_DeleteAll);
	DDX_Check(pDX, IDC_FETCH_MESSAGE_HEADERS, m_FetchHeaders);
	DDX_Radio(pDX, IDC_LEFT_OPTIONS, m_PersonaOptions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMailTransferOptionsDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CMailTransferOptionsDialog)
	ON_BN_CLICKED(IDC_FETCH_MESSAGE_HEADERS, OnFetchHeadersClicked)
	ON_LBN_DBLCLK(IDC_PERSONAS, OnDblclkPersonas)
	ON_BN_CLICKED(IDC_LEFT_OPTIONS, OnOptionsClicked)
	ON_BN_CLICKED(IDC_NORMAL_OPTIONS, OnOptionsClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailTransferOptionsDialog message handlers

BOOL CMailTransferOptionsDialog::OnInitDialog()
{
	if (!CHelpxDlg::OnInitDialog())
		return (FALSE);

	LPSTR lpPersonalities = g_Personalities.List();
	LPSTR lpAltPersona = lpPersonalities + strlen( lpPersonalities ) + 1;	//Skip <Dominant>

	if (!lpAltPersona[0] || (GetSharewareMode() == SWM_MODE_LIGHT))
	{
		m_bMultiplePersonas = FALSE;

		CPopHost* CurrentHost = CHostList::GetHostForCurrentPersona();

		if (CurrentHost)
		{
			CurrentHost->GetTransferOptions(m_SendingFlag, m_DeleteMarkedExist, m_DeleteMarked,
							m_RetrieveMarkedExist, m_RetrieveMarked);
		
		}

		if (!m_DeleteMarkedExist)
			GetDlgItem(IDC_DELETE_MARKED_MESSAGES)->EnableWindow(FALSE);
		if (!m_RetrieveMarkedExist)
			GetDlgItem(IDC_RETRIEVE_MARKED_MESSAGES)->EnableWindow(FALSE);

		// no personalities - shrink the dialog
		RECT barrect;
		CWnd * theBar = GetDlgItem( IDC_VERTBAR );
		theBar->GetWindowRect( &barrect );

		RECT dlgrect;
		GetWindowRect( &dlgrect );
		dlgrect.right = barrect.left;
		dlgrect.right = barrect.right;
#ifdef WIN32
		if (IsWinNT() && GetMajorVersion() == 3 && GetMinorVersion() <= 51)
			dlgrect.right += 2;
#endif

		MoveWindow( &dlgrect, TRUE );

		// Disable the controls associated with multiple personalites
		CWnd* Ctrl = GetDlgItem(IDC_LEFT_OPTIONS);
		if (Ctrl)
			Ctrl->EnableWindow(FALSE);
		Ctrl = GetDlgItem(IDC_NORMAL_OPTIONS);
		if (Ctrl)
			Ctrl->EnableWindow(FALSE);
		Ctrl = GetDlgItem(IDC_PERSONAS);
		if (Ctrl)
			Ctrl->EnableWindow(FALSE);
	}
	else
	{
		m_bMultiplePersonas = TRUE;

		// add personalities to the list box control
		while ( lpPersonalities[ 0 ] )
		{
			m_Personas.AddString( lpPersonalities );

			// advance to next personality
			lpPersonalities += strlen( lpPersonalities ) + 1;
		}

		// auto select personalities
		LPSTR lpList = g_Personalities.CheckMailList();
		while ( lpList[ 0 ] )
		{
			int idx = m_Personas.FindString( -1, lpList );
			if ( idx != LB_ERR )
			{
				m_Personas.SetSel( idx, TRUE );
			}

			lpList += strlen( lpList ) + 1;
		}

		// To be completely correct we need to go through each personality to see whether or
		// not there are messages marked to be deleted and/or retrieved, but for now it
		// will suffice to always set them to true when there are multiple personalities.
		m_DeleteMarkedExist = TRUE;
		m_RetrieveMarkedExist = TRUE;

		// Make sure controls are enabled/disabled correctly
		OnOptionsClicked();
	}

	return (TRUE);
}

void CMailTransferOptionsDialog::OnFetchHeadersClicked() 
{
	CButton* RetrieveNew =		(CButton*)GetDlgItem(IDC_RETRIEVE_NEW_MAIL);
	CButton* DeleteMarked =		(CButton*)GetDlgItem(IDC_DELETE_MARKED_MESSAGES);
	CButton* RetrieveMarked =	(CButton*)GetDlgItem(IDC_RETRIEVE_MARKED_MESSAGES);
	CButton* DeleteRetrieved =	(CButton*)GetDlgItem(IDC_DELETE_RETRIEVED_MESSAGES);
	CButton* DeleteAll =		(CButton*)GetDlgItem(IDC_DELETE_ALL_MESSAGES);
	CButton* FetchHeaders =		(CButton*)GetDlgItem(IDC_FETCH_MESSAGE_HEADERS);

	BOOL RetrieveNewEnable =		TRUE;
	BOOL DeleteMarkedEnable =		TRUE;
	BOOL RetrieveMarkedEnable =		TRUE;
	BOOL DeleteRetrievedEnable =	TRUE;
	BOOL DeleteAllEnable =			TRUE;
	BOOL FetchHeadersEnable =		TRUE;

	if (FetchHeaders->GetCheck() == 1)
	{
		RetrieveNewEnable = FALSE;
		DeleteMarkedEnable = FALSE;
		RetrieveMarkedEnable = FALSE;
		DeleteRetrievedEnable = FALSE;
		DeleteAllEnable = FALSE;
	}

	RetrieveNew->EnableWindow(RetrieveNewEnable);
	DeleteMarked->EnableWindow(m_DeleteMarkedExist? DeleteMarkedEnable : FALSE);
	RetrieveMarked->EnableWindow(m_RetrieveMarkedExist? RetrieveMarkedEnable : FALSE);
	DeleteRetrieved->EnableWindow(DeleteRetrievedEnable);
	DeleteAll->EnableWindow(DeleteAllEnable);
	FetchHeaders->EnableWindow(FetchHeadersEnable);
}


void CMailTransferOptionsDialog::OnOptionsClicked() 
{
	CButton* RetrieveNew =		(CButton*)GetDlgItem(IDC_RETRIEVE_NEW_MAIL);
	CButton* SendQueued =		(CButton*)GetDlgItem(IDC_SEND_QUEUED_MESSAGES);
	CButton* DeleteMarked =		(CButton*)GetDlgItem(IDC_DELETE_MARKED_MESSAGES);
	CButton* RetrieveMarked =	(CButton*)GetDlgItem(IDC_RETRIEVE_MARKED_MESSAGES);
	CButton* DeleteRetrieved =	(CButton*)GetDlgItem(IDC_DELETE_RETRIEVED_MESSAGES);
	CButton* DeleteAll =		(CButton*)GetDlgItem(IDC_DELETE_ALL_MESSAGES);
	CButton* FetchHeaders =		(CButton*)GetDlgItem(IDC_FETCH_MESSAGE_HEADERS);

	BOOL Enable = IsDlgButtonChecked(IDC_LEFT_OPTIONS);

	if (Enable)
	{
		OnFetchHeadersClicked();
		SendQueued->EnableWindow(TRUE);
	}
	else
	{
		RetrieveNew->EnableWindow(FALSE);
		SendQueued->EnableWindow(FALSE);
		DeleteMarked->EnableWindow(FALSE);
		RetrieveMarked->EnableWindow(FALSE);
		DeleteRetrieved->EnableWindow(FALSE);
		DeleteAll->EnableWindow(FALSE);
		FetchHeaders->EnableWindow(FALSE);
	}
}

int CMailTransferOptionsDialog::DoModal()
{
	int Result = CHelpxDlg::DoModal();
	
	if (Result == IDOK)
	{
		unsigned long Flags = 0;
		
		if (m_bMultiplePersonas)
			SetIniShort(IDS_INI_TRANSFER_PERSONA_OPTIONS, (short)m_PersonaOptions);
		if (m_bMultiplePersonas && m_PersonaOptions == 1)
		{
			// The user selected the normal checking/sending option for the selected
			// personalities, so set up the flags as they would be for a normal
			// manual check/send
			if (m_SendingFlag)
				Flags = kSendMailBit;
			else
				Flags = kManualMailCheckBits;
		}
		else
		{
			if (m_Send)
				Flags |= kSendMailBit;

			// If the user wants to download the headers of all the messages on the server,
			// then the options to delete/retrieve shouldn't be done at the same time
			if (m_FetchHeaders)
				Flags |= kFetchHeadersBit;
			else
			{
				if (m_RetrieveNew)
					Flags |= kRetrieveNewBit;
				if (m_DeleteMarked)
					Flags |= kDeleteMarkedBit;
				if (m_RetrieveMarked)
					Flags |= kRetrieveMarkedBit;
				if (m_DeleteRetrieved)
					Flags |= kDeleteRetrievedBit;
				if (m_DeleteAll)
					Flags |= kDeleteAllBit;
			}
		}

		if (Flags)
		{
			if ( m_bMultiplePersonas )
			{
				if ( m_lpNameList )
					GetMail(Flags, m_lpNameList);
			}
			else
			{
				GetMail(Flags);
			}
		}

		if ( m_lpNameList )
			delete [] m_lpNameList;
	}

	return (Result);
}

void CMailTransferOptionsDialog::OnOK() 
{
	// build up a check mail list
	int	cnt = m_Personas.GetSelCount();

	if ( cnt )
	{
		CString csItem;

		// allow for the list terminator
		LPINT	lpSelList = DEBUG_NEW_NOTHROW int[ ( cnt + 1 ) ];
		m_lpNameList = DEBUG_NEW_NOTHROW char[ ( cnt + 1 ) * MAX_PERSONA_LEN ];
	
		if ( lpSelList && m_lpNameList )
		{
			LPSTR lpTmp = m_lpNameList;

			m_Personas.GetSelItems( cnt, lpSelList );
		
			for ( int i = 0; i < cnt; i++ )
			{
				m_Personas.GetText( lpSelList[ i ], csItem );

				strcpy( lpTmp, csItem );
				lpTmp += strlen( lpTmp ) + 1;
			}

			// null-delimit the list
			*lpTmp++ = '\0';
			*lpTmp   = '\0';	// in case there are no members
		}

		if ( lpSelList )
			delete [] lpSelList;
	}

	// call base class
	CHelpxDlg::OnOK();
}

void CMailTransferOptionsDialog::OnDblclkPersonas() 
{
	OnOK();
}
