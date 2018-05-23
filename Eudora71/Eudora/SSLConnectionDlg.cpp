// SSLConnectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"
#include "SSLConnectionDlg.h"
#include "qcssl.h"
#include "certmanagerdlg.h"
#include "rs.h"

#include "DebugNewHelpers.h"

#include "QCWorkerSocket.h"
/////////////////////////////////////////////////////////////////////////////
// CSSLConnectionDlg dialog


CSSLConnectionDlg::CSSLConnectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSSLConnectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSSLConnectionDlg)
	m_ServerName = _T("");
	//}}AFX_DATA_INIT
}


void CSSLConnectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSSLConnectionDlg)
	DDX_Text(pDX,IDC_EDIT_SERVERIP, m_ServerIP);
	DDX_Text(pDX, IDC_EDIT_SERVERPORT, m_ServerPort);
	DDX_Text(pDX, IDC_EDIT_SSLVERSION, m_SSLVersion);
	DDX_Text(pDX, IDC_EDIT_NEGOSTATUS, m_NegoStatus);
	DDX_Text(pDX, IDC_EDIT_KEA, m_KEA);
	DDX_Text(pDX, IDC_EDIT_EA, m_EA);
	DDX_Text(pDX, IDC_EDIT_HM, m_HM);
	DDX_Text(pDX, IDC_EDIT_SSL_NOTES, m_Notes);
	DDX_Text(pDX, IDC_EDIT_LASTNEGOTIME, m_LastTime);
	DDX_Text(pDX, IDC_EDIT_SERVERNAME, m_ServerName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSSLConnectionDlg, CDialog)
	//{{AFX_MSG_MAP(CSSLConnectionDlg)
	ON_BN_CLICKED(IDC_BUTTON_CERTINFO, OnButtonCertinfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSSLConnectionDlg message handlers
int CSSLConnectionDlg::Run(const CString & person, const CString& protocol)
{
	m_pConnectionInfo = NULL;
	FPNQCSSLGetConnectionInfo fnConnInfo =NULL;
	if(Network::GetQCSSLDll() == NULL)
	{
		CRString csWarn(IDS_WARN_SSL_NEVER);
		AfxMessageBox(csWarn);
		return 0;
	}

	fnConnInfo = Network::GetQCSSLGetConnectionInfo();
	m_pConnectionInfo= fnConnInfo((const char*)person,(const char*)protocol);
	if((m_pConnectionInfo== NULL) || m_pConnectionInfo->m_Outcome.m_Outcome == SSLOutcome::SSLUNKNOWN)
	{
		CString csWarn;
		if(protocol == "SMTP")
			csWarn = CRString(IDS_WARN_SSL_SEND_NONE);
		else
			csWarn = CRString(IDS_WARN_SSL_CHECK_NONE);
		AfxMessageBox(csWarn);
		return 0;
	}
	m_ServerName = m_pConnectionInfo->m_ServerName;

	m_ServerIP = m_pConnectionInfo->m_IPAddress ;
	m_ServerPort = m_pConnectionInfo->m_Port;
	m_SSLVersion = m_pConnectionInfo->m_Version ;
	if(m_pConnectionInfo->m_Outcome.m_Outcome == SSLOutcome::SSLSUCCEEDED)
		m_NegoStatus = CRString(IDS_SUCCEEDED);
	else if(m_pConnectionInfo->m_Outcome.m_Outcome == SSLOutcome::SSLREJECTED)
		m_NegoStatus = CRString(IDS_REJECTED);
	else
		m_NegoStatus = CRString(IDS_FAILED);

	CString keybits;
	keybits.Format("%ld ", m_pConnectionInfo->m_CipherKeyBits);
	keybits += CRString(IDS_BITS);

	m_KEA = m_pConnectionInfo->m_CipherKeyExchangeMethod ;

	/*The last cert has the key length*/
	POSITION pos = m_pConnectionInfo->m_CertDataList.GetHeadPosition();
	CertData *pCertData = NULL;
	while(pos != NULL)
		{
		pCertData = (CertData*)m_pConnectionInfo->m_CertDataList.GetNext(pos);
		ASSERT(pCertData);
		}
	if(pCertData && pCertData->m_PublicKeyLength>0)
		{
		CString keyLength;
		keyLength.Format("%ld ", pCertData->m_PublicKeyLength);
		keyLength += CRString(IDS_BITS);
		// No easy way in SSL Plus 4.X to determine key length.
		// m_KEA += "(" + keyLength + ")";
		}

	m_EA = m_pConnectionInfo->m_CipherName + "("+ keybits+")" ;
	m_HM = m_pConnectionInfo->m_HashName;
	m_Notes = m_pConnectionInfo->m_Outcome.m_Errors + "\r\n" + m_pConnectionInfo->m_Outcome.m_Comments;
	m_LastTime = m_pConnectionInfo->m_Time.Format("%A, %B %d, %Y %H:%M:%S" );
	m_Persona = person;
	m_Protocol = protocol;
	return DoModal();
}

void CSSLConnectionDlg::OnButtonCertinfo() 
{
	CCertManagerDlg dlg;
	dlg.Run(m_Persona, m_Protocol,m_pConnectionInfo);	
}
