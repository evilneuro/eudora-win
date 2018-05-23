// PASSWORD.CPP
//
// Routines for changing the password
//

#include "stdafx.h"

#include "resource.h"
#include "progress.h"
#include "guiutils.h"
#include "rs.h"
#include "jjnet.h"
#include "QCNetSettings.h"
#include "persona.h"
#include "Base64.h"
#include "password.h"


#include "QCImapMailboxCommand.h"
#include "ImapMailMgr.h"
#include "ImapAccountMgr.h"
#include "QCPopSettings.h"


#include "DebugNewHelpers.h"



CString POPPassword;

static int PassConnected;

#define MAX_PASSWORD_SIZE			128
#define MAX_ENCODED_PASSWORD_SIZE	(MAX_PASSWORD_SIZE * 4 / 3)

/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog dialog

class CPasswordDialog : public CHelpxDlg
{
// Construction
public:
	CPasswordDialog(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CPasswordDialog)
	enum { IDD = IDD_GET_PASSWORD };
	CStatic	m_Prompt;
	CString	m_Password;
	CString	m_Personality;
	CString	m_Username;
	CString	m_Server;
	int m_RememberPassword;
	//}}AFX_DATA
	int		m_PromptID;

// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CPasswordDialog)
	afx_msg BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog dialog


CPasswordDialog::CPasswordDialog(CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CPasswordDialog::IDD, pParent)
	, m_RememberPassword(0)
{
	//{{AFX_DATA_INIT(CPasswordDialog)
	m_Password = "";
	m_Personality = "";
	m_Username = "";
	m_Server = "";
	//}}AFX_DATA_INIT
}

void CPasswordDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPasswordDialog)
	DDX_Control(pDX, IDC_PASSWORD_PROMPT, m_Prompt);
	DDX_Text(pDX, IDC_PASSWORD, m_Password);
	DDV_MaxChars(pDX, m_Password, MAX_PASSWORD_SIZE - 1);
	DDX_Text(pDX, IDC_PASSWORD_PERSONALITY, m_Personality);
	DDX_Text(pDX, IDC_PASSWORD_USERNAME, m_Username);
	DDX_Text(pDX, IDC_PASSWORD_SERVER, m_Server);
	DDX_Check(pDX, IDC_PASSWORD_REMEMBER, m_RememberPassword);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPasswordDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CPasswordDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog message handlers

BOOL CPasswordDialog::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	
	if (m_PromptID != 0)
		m_Prompt.SetWindowText(CRString(m_PromptID));
	
	CenterWindow(AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


// ClearPassword
// Erases the in-memory copy of the password and diasbles the Forget Password
// menu item
//
void ClearPassword()
{
	POPPassword.Empty();
	//QCRasConnection::ForgetRasPassword();
	SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, NULL);
	SetIniLong(IDS_INI_POP_LAST_AUTH,0);
//	Disable(ForgetPassItem);
}

// ClearPasswordFor
// Call ClearPassword, but set things up for a particular named personality
void ClearPasswordFor(const CString szPersona)
{
	// Make sure settings changes affect correct personality
	CString strHomie = g_Personalities.GetCurrent();
	g_Personalities.SetCurrent(szPersona);
	
	// Clear the password
	::ClearPassword();

	// and put the personality back...
	g_Personalities.SetCurrent(strHomie);
}

// GetPassword
// Get the POP account password from the user
//
BOOL GetPassword(CString& NewPassword, UINT Prompt /*= 0*/, BOOL ChangePass /*= FALSE*/, const char* PersonaName /*= NULL*/)
{
	CPasswordDialog PassDialog;
	CString strHomie;
	BOOL Result = FALSE;

	if (PersonaName)
	{
		strHomie = g_Personalities.GetCurrent();
		g_Personalities.SetCurrent(PersonaName);
	}

	CString PersName(g_Personalities.GetCurrent());
	if (PersName.IsEmpty())
		PersName.LoadString(IDS_DOMINANT);

	PassDialog.m_Personality = PersName;
	PassDialog.m_Username = GetIniString(IDS_INI_LOGIN_NAME);
	PassDialog.m_Server = GetIniString(IDS_INI_POP_SERVER);
	PassDialog.m_RememberPassword = GetIniLong(IDS_INI_SAVE_PASSWORD);

	PassDialog.m_PromptID = Prompt;
		
	if (PassDialog.DoModal() == IDOK && !PassDialog.m_Password.IsEmpty())
	{
		NewPassword = PassDialog.m_Password;
		SetIniLong(IDS_INI_SAVE_PASSWORD, PassDialog.m_RememberPassword);
		if (PassDialog.m_RememberPassword == 0)
			SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, NULL);
		Result = TRUE;
	}

	if (PersonaName)
		g_Personalities.SetCurrent(strHomie);

	return Result;
}

// GetPasswordMP
// Get the POP account password from the user allowing for selection
// of personality
//
BOOL GetPasswordMP(CString& NewPassword, CString& Persona, UINT Prompt/* = 0 */)
{
	CGetPasswordMP	PassDialogMP;	// multiple personality
	int Result;
	
	Result = PassDialogMP.DoModal();
	if (Result == IDOK && !PassDialogMP.m_Password.IsEmpty())
	{
		NewPassword = PassDialogMP.m_Password;
		Persona = PassDialogMP.m_Persona;

		CRString Dominant( IDS_DOMINANT );
		if ( Persona == Dominant )
			Persona.Empty();

		return (TRUE);
	}

	return (FALSE);
}


// PassError
// Tell the user why the password couldn't be set
void PassError(const char* Message)
{
	ErrorDialog(IDS_ERR_PASS_CHANGE, Message);
}


// PassCommand
// Takes a command, adds arguments, sends it, and checks the return value
//
int PassCommand(UINT Command = 0, const char* Args = NULL)
{   
	int status = 0;
	char buf[QC_NETWORK_BUF_SIZE];
//	char t[80];
	BOOL found = FALSE;
	BOOL done = FALSE;

	if (Command)
	{
		// Copy the command into the command buffer
		CRString CommandStr(Command);
		strcpy(buf, CommandStr);
		if (Args)
			strcat(buf, Args);

		// Show command being sent to password server, but not actual passwords
		if (Command == IDS_PASS_QUIT)
			Progress(-1, buf);
		else
			Progress(-1, CommandStr);

		strcat(buf, "\r\n");

		// Send the command to the server
		status = NetConnection->PutDirect(buf);
		if (status < 0)
		{
			ErrorDialog(IDS_ERR_PASS_SEND_CMD);
			return (status);
		}
	}
	else
	{
		while (!done && (status = NetConnection->GetLine(buf, sizeof(buf))) >= 0)
		{
			if (buf[0] == '2' || buf[0] == '3')
			{
				done = TRUE;
				if (buf[3] == ' ')
					found = TRUE;
			}
//			if (strlen(t) && !strncmp(buf, t, strlen(t)))
//				status = -1;
		}

		if (status < 0)
		{
			ErrorDialog(IDS_ERR_PASS_CONNECT);
			return (status);
		}
	}

	// Quit closes the connection, so we can't get the response
	if (Command == IDS_PASS_QUIT)
		return (1);

	while (!found && status >= 0)
	{
		status = NetConnection->GetLine(buf, sizeof(buf));
		if (status >= 0 && buf[0] >= '2' && buf[0] <= '5' && buf[3] == ' ')
            found = TRUE;
	}

	if (status < 0)
	{
		ErrorDialog(IDS_ERR_PASS_GET_RESPONSE);
		return (status);
	}

	buf[status - 2] = '\0';
	if (*buf == '4' || *buf == '5')
	{
		char* bufptr = buf + 3;
		while (*bufptr && isspace((unsigned char)*bufptr))
			bufptr++;
		PassError(bufptr);
		return (-1);
	}

	return (1);
}


// StartPass
// Establishes the connection to the password server.
int StartPass(const char* Machine)
{
	int status;
	PassConnected = FALSE;
	if (!CreateNetConnection())
		return (-1);

// Hesiod Kludge code.  There should really be a userinterface for this
	//
	// If we're using Hesiod, then override the 'Machine' value
	// with the host name retrieved from the Hesiod server.
	//
	char szPOPSystem[128] = { '\0' };
	{
		if (!stricmp(Machine, "hesiod"))
		{
			//
			// Grab the username (c_wsakai) out of the POP account setting
			// (c_wsakai@adept.qualcomm.com).
			//
			char szPOPUserName[128];
			::GetIniString(IDS_INI_POP_ACCOUNT, szPOPUserName, sizeof(szPOPUserName));
			{
				char* pszAtSign = strrchr(szPOPUserName, '@');
				if (NULL == pszAtSign)
					return -1;
				*pszAtSign = 0;
			}

			//
			// Do the Hesiod lookup.
			//
			if (SUCCEEDED(QCHesiodLibMT::LoadHesiodLibrary()))
			{
				int nHesiodError = -1;
				if (SUCCEEDED(QCHesiodLibMT::GetHesiodServer(CRString(IDS_POP_SERVICE), szPOPUserName, szPOPSystem, sizeof(szPOPSystem), &nHesiodError)))
					Machine = szPOPSystem;
				else
				{
					Machine = NULL;
					::HesiodErrorDialog(nHesiodError);
					::ErrorDialog(IDS_ERR_HESIOD_HOST_LOOKUP);
				}
				QCHesiodLibMT::FreeHesiodLibrary();

				if (! Machine)
				    return -1;
			}
			else
			{
				const int IDS_WINSOCK_BASE_ERROR = 8000;
				::ErrorDialog(IDS_WINSOCK_BASE_ERROR + 200 + 1);
				return -1;
			}
		}
	}
// End Hesiod kludge


	CString Persona = g_Personalities.GetCurrent();
	//the personality for which we are changing password

	QCPOPSettings *settings = DEBUG_NEW QCPOPSettings(Persona, 0);
	settings->GrabSettings();
	int sslUsage = ::GetIniShort(IDS_INI_SSL_PASSWORD_USE);
	
	//at this time we support alternate port only
	if(sslUsage== SSLSettings::SSLUseAlternatePort)
	{
		status = NetConnection->Open(Machine,IDS_CHANGE_PASS_SERVICE , GetIniShort(IDS_INI_SSLALTRNATEPORT_PASSWORD), 106);
	}
	else
	{
		// Go ahead and open the connection and return the status to the calling program
		status = NetConnection->Open(Machine, IDS_CHANGE_PASS_SERVICE, GetIniShort(IDS_INI_EUDORA_PW_PORT), 106);
	}


	if(status >=0 && sslUsage == QCSSLSettings::SSLUseAlternatePort)
	{
		NetConnection->SetSSLMode(TRUE,  Persona, &settings->m_SSLSettings, "PASSWD");
	}


	
	if (status >= 0)
	{
		status = PassCommand();
		if (status >= 0)
			PassConnected = TRUE;
	}
	delete settings;
	return (status);
}

int ClosePass()
{
	if (NetConnection)
	{
		if (GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE) ||
			GetIniShort(IDS_INI_CONNECT_OFFLINE) || GetIniShort(IDS_INI_AUTO_CONNECTION))
		{
			delete NetConnection;
			NetConnection = NULL;
		} else
		{
			NetConnection->Close();
		}
		PassConnected = FALSE;
	}
	CloseProgress();
	
	return (1);
}


BOOL ChangePassword()
{
	char	Username[128];
	BOOL	bMultiple = FALSE;
	CString OrgPersona;
	CString NewPass1, NewPass2, OldPass;


	// check for multiple personalities
	LPSTR lpPersonalities = g_Personalities.List();
	LPSTR lpAltPersona = lpPersonalities + strlen( lpPersonalities ) + 1;	//Skip <Dominant>
	if ( lpAltPersona[ 0 ] )
	{
		bMultiple = TRUE;

		OrgPersona = g_Personalities.GetCurrent();

		// get personality and password
		CString NewPersona;
		if ( ! GetPasswordMP( OldPass, NewPersona, IDS_PASS_RESP_1 ) )
			return FALSE;

		// perhaps change personalities
		if ( OrgPersona != NewPersona )
			g_Personalities.SetCurrent( NewPersona );
	}

	const char* POPAccount = GetIniString(IDS_INI_POP_ACCOUNT);
	if (!POPAccount || !*POPAccount)
	{
		ErrorDialog(IDS_ERR_NO_POP_ACCT);
		return (FALSE);
	}

	strcpy(Username, POPAccount);
	char *At = strrchr(Username, '@');
	if (!At || !At[1])
	{
		ErrorDialog(IDS_ERR_BAD_POP_ACCT, POPAccount);
		return (FALSE);
	}

	*At = 0;
	const char* POPServer = At + 1;

	// Get old password and double-checked new password
	while (1)
	{
		// already got OldPass (while(1) never loops...)
		if ( ! bMultiple )
		{
			if (!GetPassword(OldPass, IDS_PASS_RESP_1, TRUE))
				break;
		}
	
		if (!POPPassword.IsEmpty() && POPPassword != OldPass)
		{
			ErrorDialog(IDS_ERR_NOT_PASS);
			break;
		}
	
		if (!GetPassword(NewPass1, IDS_PASS_RESP_2, TRUE))
			break;
	
		if (!GetPassword(NewPass2, IDS_PASS_RESP_3, TRUE))
			break;
	
		if (NewPass1 != NewPass2)
		{   
			ErrorDialog(IDS_ERR_PASS_MISMATCH);
			break;
		}
		
		// Is there a specific server we're supposed to use?
		char pwServer[MAX_PATH];
		GetIniString(IDS_INI_CHANGE_PW_SERVER,pwServer,sizeof(pwServer));
		if (*pwServer) POPServer = pwServer;
	
		// Connect up to the password server and talk
		if ((StartPass(POPServer) < 0 ||
			PassCommand(IDS_PASS_USER_NAME, Username) < 0 ||
			PassCommand(IDS_PASS_OLD_PASS, OldPass) < 0 ||
			PassCommand(IDS_PASS_NEW_PASS, NewPass1) < 0))
		{
			break;
		}
	
		POPPassword = NewPass1;
		
		PassCommand(IDS_PASS_QUIT);
		ClosePass();
		ErrorDialog(IDS_PASS_ALERT_PASS_CHANGED);
	
		if ( bMultiple )
			g_Personalities.SetCurrent( OrgPersona );
	
		return (TRUE);
	}

	if (PassConnected)
		PassCommand(IDS_PASS_QUIT);
	ClosePass();

	if ( bMultiple )
		g_Personalities.SetCurrent( OrgPersona );
	
	return (FALSE);
}


// EncodePassword
//
// Encodes a clear text password by using bas64.
// For use in saving to disk.
//

CString EncodePassword(const char* ClearText)
{
	Base64Encoder	TheEncoder;
	char			tmp_Password[MAX_ENCODED_PASSWORD_SIZE];
	char*			OutSpot = tmp_Password;
	LONG			OutLen = 0;

	if (ClearText)
	{
		LONG		nPasswordLength = strlen(ClearText);
		
		//	Make sure we'll fit within our temp buffer
		if (nPasswordLength < MAX_PASSWORD_SIZE)
		{
			TheEncoder.Init(tmp_Password, OutLen, Base64Encoder::kDontWrapLines);
			OutSpot += OutLen;
			TheEncoder.Encode(ClearText, nPasswordLength, OutSpot, OutLen);
			OutSpot += OutLen;
			TheEncoder.Done(OutSpot, OutLen);
			OutLen += OutSpot - tmp_Password;
		}
	}
	tmp_Password[OutLen] = 0;

	return CString(tmp_Password);
}

// DecodePassword
//
// Decodes an encoded password using base64.
// For use in retrieving from disk.
//
CString DecodePassword(const char* EncodedText)
{
	Base64Decoder	TheDecoder;
	char			tmp_Password[MAX_PASSWORD_SIZE];
	LONG			OutLen = 0;

	if (EncodedText)
	{
		LONG		nPasswordLength = strlen(EncodedText);

		//	Make sure we'll fit within our temp buffer
		if (nPasswordLength < MAX_ENCODED_PASSWORD_SIZE)
		{
			TheDecoder.Init();
			TheDecoder.Decode(EncodedText, nPasswordLength, tmp_Password, OutLen);
		}
	}
	tmp_Password[OutLen] = 0;

	return CString(tmp_Password);
}


/////////////////////////////////////////////////////////////////////////////
// CGetPasswordMP dialog


CGetPasswordMP::CGetPasswordMP(CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CGetPasswordMP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetPasswordMP)
	m_Password = _T("");
	m_Persona = _T("");
	//}}AFX_DATA_INIT
}


void CGetPasswordMP::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetPasswordMP)
	DDX_Control(pDX, IDC_PERSONA_NAME, m_PersonaCombo);
	DDX_Text(pDX, IDC_PASSWORD, m_Password);
	DDV_MaxChars(pDX, m_Password, MAX_PASSWORD_SIZE - 1);
	DDX_CBString(pDX, IDC_PERSONA_NAME, m_Persona);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetPasswordMP, CHelpxDlg)
	//{{AFX_MSG_MAP(CGetPasswordMP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetPasswordMP message handlers

BOOL CGetPasswordMP::OnInitDialog() 
{
	CHelpxDlg::OnInitDialog();
	
	// fill in combo boxes
	FillInPersonalities();

	CenterWindow(AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGetPasswordMP::OnOK() 
{
	// TODO: Add extra validation here
	
	CHelpxDlg::OnOK();
}

// helper methods
void CGetPasswordMP::FillInPersonalities() 
{
	m_PersonaCombo.ResetContent();

	// add personalities to the list box control
	LPSTR lpPersonalities = g_Personalities.List();
	while ( lpPersonalities[ 0 ] )
	{
		m_PersonaCombo.AddString( lpPersonalities );

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}

	// hilight the default persona
	if ( m_Persona.IsEmpty() )
	{
		m_PersonaCombo.SetCurSel( 0 );
	}
	else
	{
		int Index = m_PersonaCombo.FindStringExact( -1, m_Persona );
		if ( Index >= 0 )
			m_PersonaCombo.SetCurSel( Index );	// select current personality
		else
			ASSERT( FALSE );
	}
}


BOOL ForgetPasswordByDialog()
{
	BOOL	bMultiple = FALSE;
	CString OrgPersona;


	// check for multiple personalities
	LPSTR lpPersonalities = g_Personalities.List();
	LPSTR lpAltPersona = lpPersonalities + strlen( lpPersonalities ) + 1;	//Skip <Dominant>
	if ( lpAltPersona[ 0 ] )
		bMultiple = TRUE;

	if(!bMultiple)
	{
		g_Personalities.ForgetPasswords( g_Personalities.List());
	}
	else
	{
		CForgetPassword	ForgetDialog;	// multiple personality
		
		ForgetDialog.DoModal();

		if( ForgetDialog.GetPersonas() != NULL)
			g_Personalities.ForgetPasswords( ForgetDialog.GetPersonas() );

		return (TRUE);
		
	}
	return (FALSE);

}

////
/////////////////////////////////////////////////////////////////////////////
// CForgetPassword dialog


CForgetPassword::CForgetPassword(CWnd* pParent /*=NULL*/)
	: CHelpxDlg(CForgetPassword::IDD, pParent)
{
	//{{AFX_DATA_INIT(CForgetPassword)
	//}}AFX_DATA_INIT
	m_lpNameList = NULL;
}

CForgetPassword::~CForgetPassword()
{
	if(m_lpNameList)
		delete [] m_lpNameList;
}

void CForgetPassword::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForgetPassword)
	DDX_Control(pDX, IDC_FORGET_PERSONAS, m_PersonaList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CForgetPassword, CHelpxDlg)
	//{{AFX_MSG_MAP(CForgetPassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetPasswordMP message handlers

BOOL CForgetPassword::OnInitDialog() 
{
	CHelpxDlg::OnInitDialog();
	
	// fill in combo boxes
	FillInPersonalities();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CForgetPassword::OnOK() 
{
	// build up a check mail list
	int	cnt = m_PersonaList.GetSelCount();

	if ( cnt )
	{
	
		CString csItem;
	
		// allow for the list terminator
		LPINT	lpSelList = DEBUG_NEW_NOTHROW int[ ( cnt + 1 ) ];
		m_lpNameList = DEBUG_NEW_NOTHROW char[ ( cnt + 1 ) * MAX_PERSONA_LEN ];
	
		if ( lpSelList && m_lpNameList )
		{
			LPSTR lpTmp = m_lpNameList;

			m_PersonaList.GetSelItems( cnt, lpSelList );
		
			for ( int i = 0; i < cnt; i++ )
			{
				m_PersonaList.GetText( lpSelList[ i ], csItem );

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

	// TODO: Add extra validation here
	
	CHelpxDlg::OnOK();
}

// helper methods
void CForgetPassword::FillInPersonalities() 
{
	
	LPSTR lpPersonalities = g_Personalities.List();
	
	// add personalities to the list box control
	while ( lpPersonalities[ 0 ] )
	{
		m_PersonaList.AddString( lpPersonalities );

		// advance to next personality
		lpPersonalities += strlen( lpPersonalities ) + 1;
	}

	m_PersonaList.SelItemRange(TRUE, 0, m_PersonaList.GetCount()-1);
	/*// auto select personalities
	LPSTR lpList = g_Personalities.CheckMailList();
	while ( lpList[ 0 ] )
	{
		int idx = m_Personas.FindString( -1, lpList );
		if ( idx != LB_ERR )
		{
			m_Personas.SetSel( idx, TRUE );
		}

		lpList += strlen( lpList ) + 1;
	}*/
	
}

