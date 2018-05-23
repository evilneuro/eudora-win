////////////////////////////////////////////////////////////////////////
// File: KERBEROS.CPP
//
//
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include <string.h>

#include <QCUtils.h>
//#include "guiutils.h"
//#include "fileutil.h"
#include "rs.h"             //CRString GetINI group of fuunctions
#include "mainfrm.h"       //GET_TICKET

// include the Kerberos stuff
#include "kclient.h"


//	Defines and other Constants	=============================
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif





//
// Kerberos specific stuff.  FORNOW, should probably encapsulate this
// stuff in a separate thread-safe Kerberos object.
//
// 
//static BOOL s_bKerbTicket = FALSE;
//static char s_szKerbTicketBuffer[QCKerberos::kTicketBufferSize];




HINSTANCE QCKerberos::s_hInstKerberosLib = NULL;
BOOL QCKerberos::s_bKerbAppLoaded = FALSE;
BOOL QCKerberos::s_bKerbTicket = FALSE;
DWORD QCKerberos::s_dwKerbTicketBufferLength = 0; 
char QCKerberos::s_szKerbTicketBuffer[QCKerberos::kTicketBufferSize];

BOOL (CALLBACK* QCKerberos::m_pfnSetUserName)(LPSTR) = NULL;
BOOL (CALLBACK* QCKerberos::m_pfnGetTicketForService)(LPSTR, LPSTR, LPDWORD) = NULL;
BOOL (CALLBACK* QCKerberos::m_pfnSendTicketForService)(LPSTR service, LPSTR version, int fd) = NULL;




////////////////////////////////////////////////////////////////////////
// ClearKerberosTicket [extern]
//
////////////////////////////////////////////////////////////////////////
void QCKerberos::ClearKerberosTicket()
{
	s_bKerbTicket		= FALSE;
	memset(s_szKerbTicketBuffer, 0,  kTicketBufferSize);
}




int QCKerberos::LoadKerberosLibrary(Callback1<const char *> ReportError)
{
    //
    // Load the library
	//
	ASSERT(NULL == s_hInstKerberosLib);
	if (! (s_hInstKerberosLib = LoadLibrary(CRString(IDS_KERBEROS32_DLL))))
    {
		UINT uError = WinExec(LPCSTR(CRString(IDS_INI_KERBEROS16)), SW_HIDE);
		if (uError > 31)
			s_bKerbAppLoaded = Kerb16LoadLib();
		else
		{
			switch (uError)
			{
			case ERROR_BAD_FORMAT:
				ReportError( CRString(IDS_KERB16_ERR_NO_LAUNCH_INVALID));
				break;
			default:
				ReportError(CRString(IDS_KERB16_ERR_NO_LAUNCH_NO_FIND));
				break;
			}
			s_hInstKerberosLib = NULL;
        	return -1;
		}

		if (! s_bKerbAppLoaded) 
		{
			const int IDS_WINSOCK_BASE_ERROR = 8000;
        	ReportError(CRString(IDS_WINSOCK_BASE_ERROR + 200 + 7));	// "couldn't load security DLL"
			s_hInstKerberosLib = NULL;
        	return -1;
		}
    }

    //
    // Initialize all function entry points...
	//
//#define SET_PA(fnName, fnNumber) \
	//*((FARPROC*)&fnName) = GetProcAddress(s_hInstKerberosLib, (LPSTR)GetIniString(fnNumber, szBuffer, sizeof(szBuffer)))

	//char szBuffer[64];
	ASSERT(NULL == m_pfnSetUserName && 
			NULL == m_pfnGetTicketForService &&
			NULL == m_pfnSendTicketForService);

    //SET_PA(m_pfnSetUserName,			IDS_KERB_SET_USERNAME);
    //SET_PA(m_pfnGetTicketForService,	IDS_KERB_GET_TICKET_FOR_SERVICE);
    //SET_PA(m_pfnSendTicketForService,	IDS_KERB_SEND_TICKET_FOR_SERVICE);
	*((FARPROC*)&m_pfnSetUserName) = GetProcAddress(s_hInstKerberosLib, "SetUserName");
	*((FARPROC*)&m_pfnGetTicketForService) = GetProcAddress(s_hInstKerberosLib, "GetTicketForService");
	*((FARPROC*)&m_pfnSendTicketForService) = GetProcAddress(s_hInstKerberosLib, "SendTicketForService");


	ASSERT(m_pfnSetUserName && 
			m_pfnGetTicketForService &&
			m_pfnSendTicketForService);

    return (0);
}



int QCKerberos::FreeLibrary()
{
	int res = TRUE;
	if(!s_bKerbAppLoaded)
	{
		ASSERT(s_hInstKerberosLib);
		res = ::FreeLibrary(s_hInstKerberosLib);

		m_pfnSetUserName          = NULL;
		m_pfnGetTicketForService  = NULL;
		m_pfnSendTicketForService = NULL;
		s_hInstKerberosLib        = NULL;
	}
	else{
		Kerb16FreeLib();
		KerbQuitApp();
	}

	return res;
}

/*
void QCKerberos::LoadTicket(){
	GetIniString(IDS_KERB_FILLER_STRING, s_szKerbTicketBuffer, sizeof(s_szKerbTicketBuffer));
}*/

BOOL QCKerberos::SetUserName(LPSTR szUserName){

	int bSuccess;
	if (s_bKerbAppLoaded)
			bSuccess = Kerb16SetName(szUserName);	
	else
	{
		ASSERT( m_pfnSetUserName);
		bSuccess = m_pfnSetUserName(szUserName);
	}

	if(bSuccess)
		s_bKerbTicket = TRUE;
	
	return bSuccess;

}

BOOL QCKerberos::GetTicketForService(LPSTR s1, LPSTR s2, LPDWORD pWord){
	ASSERT(m_pfnGetTicketForService);
	return m_pfnGetTicketForService(s1, s2, pWord);
}


BOOL QCKerberos::SendTicketForService(LPSTR service, LPSTR version, int fd){
	ASSERT(m_pfnSendTicketForService);
	return m_pfnSendTicketForService(service, version, fd);
}



BOOL QCKerberos::SendTicket(char *szCommandString, SOCKET socket, BOOL *bSentTicket)
{

		//
		// Don't have the foggiest...just following Steve Dorner's 
		// suggestion...
		//
		//strcpy(s_szKerbTicketBuffer,"AUTHV0.1JUSTJUNK");
		int nSize = strlen(s_szKerbTicketBuffer);
		*bSentTicket = FALSE;
		BOOL bSuccess = FALSE;
		
		if (s_bKerbAppLoaded)
		{
			s_dwKerbTicketBufferLength = kTicketBufferSize;
			bSuccess = Kerb16GetTicket(szCommandString, (s_szKerbTicketBuffer+nSize), &s_dwKerbTicketBufferLength);
			//dwTickenLen = s_dwKerbTicketBufferLength;
		}
		else
		{
			if (QCKerberos::m_pfnSendTicketForService)
			{
				//
				// According to chu@platinum.com (Howard Chu) we need
				// only to call this for both kerb4 & kerb5, we no
				// longer need GetTicketForService...
				//
				//bSuccess = SendTicketForService(szCommandString, buf, NetConnection->GetSocket());
				char			buf[128];
				memset(buf,0, 128);

				bSuccess = SendTicketForService(szCommandString, buf, socket);
				if (bSuccess) 
					*bSentTicket = TRUE;
				
			}
			else if (QCKerberos::m_pfnGetTicketForService)
			{
				// Try the old original way...
				//bSuccess = QCKerberos::GetTicketForService(szCommandString, (s_szKerbTicketBuffer+nSize), &dwTickenLen);
				bSuccess = QCKerberos::GetTicketForService(szCommandString, 
							(s_szKerbTicketBuffer+nSize), &s_dwKerbTicketBufferLength);
			}
			else
				bSuccess = FALSE;
			
		}

		return bSuccess;
}



#ifdef WIN32
//-------------------------------------------------------------------
// The following are the messages that talks to the Kerberos app instead
// of the DLL
//-------------------------------------------------------------------
BOOL QCKerberos::Kerb16LoadLib()
{ 
	// Start up the app first     
	UINT nMessage = RegisterWindowMessage((LPCTSTR)CRString(IDS_INI_KERB16_REG));
	CWnd *pKerb16Wnd = NULL;
	int i = 0;
	while (	!pKerb16Wnd && i < 10000)
 	{
 		pKerb16Wnd = AfxGetMainWnd()->FindWindow((LPCTSTR)CRString(IDS_INI_KERB16_CLASS),(LPCTSTR)CRString(IDS_INI_KERB16_TITLE));
		i++;
	}
	if ( pKerb16Wnd && pKerb16Wnd->SendMessage(nMessage,LOAD_LIBRARY, (LPARAM)NULL) )
	{
   		//MessageBox(NULL,"WM_KERBEROS16... LOAD_LIBRARY message worked","Error!",MB_OK);
		return TRUE;
	}
	else
	{
   		//MessageBox(NULL,"WM_KERBEROS16... LOAD_LIBRARY message failed","Error!",MB_OK);
		return FALSE;
	}
}

BOOL QCKerberos::Kerb16FreeLib()
{
	      
	UINT nMessage = RegisterWindowMessage((LPCTSTR)CRString(IDS_INI_KERB16_REG));
  	CWnd *pKerb16Wnd = AfxGetMainWnd()->FindWindow((LPCTSTR)CRString(IDS_INI_KERB16_CLASS),(LPCTSTR)CRString(IDS_INI_KERB16_TITLE));
  
	if ( pKerb16Wnd && pKerb16Wnd->SendMessage(nMessage,FREE_LIBRARY, (LPARAM)NULL) )
	{
   		//MessageBox(NULL,"WM_KERBEROS16... FREE_LIBRARY message worked","Error!",MB_OK);
		return TRUE;
	}
	else
	{
   		//MessageBox(NULL,"WM_KERBEROS16... FREE_LIBRARY message failed","Error!",MB_OK);
		return FALSE;
	}
	
	// Quit the App
}

BOOL QCKerberos::KerbQuitApp()
{
 	CWnd *pKerb16Wnd = AfxGetMainWnd()->FindWindow((LPCTSTR)CRString(IDS_INI_KERB16_CLASS),(LPCTSTR)CRString(IDS_INI_KERB16_TITLE));
  
	if ( pKerb16Wnd && pKerb16Wnd->PostMessage(WM_DESTROY) )
		return TRUE;
	else
		return FALSE;
}

BOOL QCKerberos::Kerb16SetName(LPSTR name)
{      
 	CWnd *pKerb16Wnd = AfxGetMainWnd()->FindWindow((LPCTSTR)CRString(IDS_INI_KERB16_CLASS),(LPCTSTR)CRString(IDS_INI_KERB16_TITLE));
	char buf[255];
 	strcpy(buf, name);

    COPYDATASTRUCT cds ;
	cds.dwData = SET_USERNAME;			//  Text command
	cds.lpData = buf;					//  Point to text
	cds.cbData = strlen(buf) + 1;		//  Length of string
				                  		//  Send to the other app.
	
	if ( pKerb16Wnd && 
		pKerb16Wnd->SendMessage(WM_COPYDATA, (WPARAM)AfxGetMainWnd()->m_hWnd,
											  (LPARAM)&cds ))
 	{
   		//MessageBox(NULL,"WM_KERBEROS16... SET_USERNAME message worked","Error!",MB_OK);
		return TRUE;
	}
	else
	{
   		//MessageBox(NULL,"WM_KERBEROS16... SET_USERNAME message failed","Error!",MB_OK);
		return FALSE;
	}
}

typedef struct tagTICKET
{
	char command[256];
	char buf[2000];
	long buflen;
} TICKET, *PTICKET; 
 
BOOL QCKerberos::Kerb16GetTicket(LPSTR commandString, LPSTR buffer, LPDWORD bufLen)	  
{      
 	CWnd *pKerb16Wnd = AfxGetMainWnd()->FindWindow((LPCTSTR)CRString(IDS_INI_KERB16_CLASS),(LPCTSTR)CRString(IDS_INI_KERB16_TITLE));
	//DWORD test = s_dwKerbTicketBufferLength;
	DWORD i = 0;
	
	TICKET tic;
 	strcpy(tic.command, commandString);
	strcpy(tic.buf, buffer);
    tic.buflen = *bufLen;

    COPYDATASTRUCT cds ;
	cds.dwData = GET_TICKET;			//  Text command
	cds.lpData = &tic;					//  Point to text
	cds.cbData = sizeof(TICKET);		//  Length of string
				                  		//  Send to the other app.
	
	if ( pKerb16Wnd && 
		pKerb16Wnd->SendMessage(WM_COPYDATA, (WPARAM)AfxGetMainWnd()->m_hWnd,
											  (LPARAM)&cds ))
	{
		return TRUE;
	}
	else
	{
   		//MessageBox(NULL,"WM_KERBEROS16... GET_TICKET message failed","Error!",MB_OK);
		s_dwKerbTicketBufferLength = 0;	
		strcpy(s_szKerbTicketBuffer+16, "failed");
		return FALSE;
	}
}

BOOL QCKerberos::SetKerb16Ticket(PCOPYDATASTRUCT pcds)
{
	switch (pcds->dwData)
	{ 
	case SEND_TICKET:
		PTICKET ptic;
		ptic = (PTICKET)pcds->lpData;
		if (ptic)
		{
			s_dwKerbTicketBufferLength = ptic->buflen;
			memcpy(s_szKerbTicketBuffer+16, ptic->buf, ptic->buflen);
			break;
		}
	case FAILED_SEND_TICKET:
	default:
		s_dwKerbTicketBufferLength = 0;
		strcpy(s_szKerbTicketBuffer+16, "failed");
		break;
	}
	
	return TRUE;
}

#endif // WIN32