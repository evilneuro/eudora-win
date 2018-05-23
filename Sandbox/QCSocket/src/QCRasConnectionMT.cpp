// QCRASConnection.cpp
//
// Management of the global RAS connection.
//

#include "stdafx.h"

#include <direct.h>
#include <QCUtils.h>

#include "QCRas.h"
#include "QCSockCallbacks.h"
#include "Callbacks.h"


#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


class CRString : public CString
{
public:
	CRString(UINT StringID) { LoadString(StringID); }
};



//
// Declaration and initialization of statics.
//
RAS_RASDIALDLG* QCRasLibrary::m_pfnRasDialDlg = NULL;
RAS_RASCREATEPHONEBOOKENTRY* QCRasLibrary::m_pfnRasCreatePhonebookEntry = NULL;
RAS_RASEDITPHONEBOOKENTRY* QCRasLibrary::m_pfnRasEditPhonebookEntry = NULL;

RAS_RASDIAL* QCRasLibrary::m_pfnRasDial = NULL;
RAS_RASENUMCONNECTIONS* QCRasLibrary::m_pfnRasEnumConnections = NULL;
RAS_RASENUMENTRIES* QCRasLibrary::m_pfnRasEnumEntries = NULL;
RAS_RASGETCONNECTSTATUS* QCRasLibrary::m_pfnRasGetConnectStatus = NULL;
RAS_RASGETERRORSTRING* QCRasLibrary::m_pfnRasGetErrorString = NULL;
RAS_RASHANGUP* QCRasLibrary::m_pfnRasHangUp = NULL;
RAS_RASGETENTRYDIALPARAMS* QCRasLibrary::m_pfnRasGetEntryDialParams = NULL;
RAS_RASSETENTRYDIALPARAMS* QCRasLibrary::m_pfnRasSetEntryDialParams = NULL;


HINSTANCE QCRasLibrary::m_hRasLibrary = NULL;
bool QCRasLibrary::m_bLibraryLoaded = false;

CString QCRasLibrary::m_strEntry;
bool QCRasLibrary::m_bActiveConn = false;
HRASCONN QCRasLibrary::m_hRasConn = NULL;

HINSTANCE QCRasLibrary::m_hRasDlgLibrary = NULL;
bool QCRasLibrary::m_bCanUseRasDialDlg = false;
bool QCRasLibrary::m_bStopRequested = false;

bool QCRasLibrary::m_bConnectedByEudora = false;
CString QCRasLibrary::m_strEntryNotConnectedByEudora;
CCriticalSection QCRasLibrary::m_RasGuard;
bool QCRasLibrary::m_bUseExistingConn = false;

QCRasLibrary::ProgressCB QCRasLibrary::g_Progress  =0;
QCRasLibrary::ErrorCB QCRasLibrary::g_Error = 0;


void QCRasLibrary::ResynchRasConnection()
{
	CSingleLock lock(&m_RasGuard, TRUE);

	if( !m_bLibraryLoaded)
		return; 

	// First check to see if there's already a connection open.  
	// If so, then don't dial.
	//
	RASCONN RasConn[6]; //allow an arbitary number of connections (6) hoping people wouldnt have more than 6 modems 
						//going  at a time..
	memset(&RasConn[0], 0, sizeof(RasConn));
	RasConn[0].dwSize = sizeof(RASCONN);
	DWORD dwSize = sizeof(RasConn);
	DWORD dwNumConnections = 0;

	if (QCRasLibrary::m_pfnRasEnumConnections(&RasConn[0], &dwSize, &dwNumConnections) == 0 )
	{
		if(dwNumConnections > 0)
		{
			m_hRasConn = RasConn[0].hrasconn;
			m_strEntry = RasConn[0].szEntryName;
			m_bActiveConn = true;
		}
		else
		{
			m_hRasConn = NULL;
			m_strEntry = "";
			m_bActiveConn = false;
		}
	}
}


void QCRasLibrary::Hangup()
{
	if(m_hRasConn && QCRasLibrary::m_pfnRasHangUp)
	{
		//Need to Hangup even if unsuccessful..
		QCRasLibrary::m_pfnRasHangUp(m_hRasConn);
		
		int ms=0;
		RASCONNSTATUS status;
		status.dwSize = sizeof(RASCONNSTATUS);
		while(m_pfnRasGetConnectStatus(m_hRasConn, &status) != ERROR_INVALID_HANDLE && ms <= 3000)
		{
			Sleep(100);
			ms+=100;
		}

	}
	m_hRasConn = NULL;
}



////////////////////////////////////////////////////////////////////////
// CloseRasConnection [public]
//
////////////////////////////////////////////////////////////////////////
bool QCRasLibrary::CloseConnection(bool bHardClose /*=false*/)
{
	CSingleLock lock(&m_RasGuard, TRUE);

	if( !m_bLibraryLoaded)
		return false;

	if (m_hRasConn)
	{
		//If connection is not made by Eudora and user is using only one dialup connection
		//dont close it
		if( !m_bConnectedByEudora && bHardClose == false)
			return false;

		Hangup();
		/*ASSERT(m_pfnRasHangUp != NULL);
		if (m_pfnRasHangUp != NULL)
		{
			m_pfnRasHangUp(m_hRasConn);
			Sleep(3000); //make sure it is closed by  waiting a bit
		}
		m_hRasConn = NULL;*/


		m_bActiveConn = false;
		m_strEntry = "";
		
		//we disconnected the connection. Next time we may not strat the connection.
		m_bConnectedByEudora = false;
		
		return true;
	}

	return false;

}


bool QCRasLibrary::IsReady()
{
	if(m_bLibraryLoaded == false)
	{
		ASSERT(NULL == m_hRasLibrary);
		HRESULT hrLoad = LoadRasLibrary_(TRUE);
		return FAILED(hrLoad)?false:true;
	}
	return true;
}


bool QCRasLibrary::IsActive()
{ 
	if( !m_bLibraryLoaded)
		return false; 

	return m_bActiveConn;
}


////////////////////////////////////////////////////////////////////////
// GetRasConnectionList [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasLibrary::GetRasConnectionList(CStringList& strConnectionList)
{
	ASSERT(::IsMainThreadMT());
	ASSERT(strConnectionList.IsEmpty());

	HRESULT hr = E_FAIL;

	//
	// Load up RAS library, if necessary.
	//
	if( !QCRasLibrary::IsReady())
		return hr;

	//
	// If we get here, the RAS library should be loaded up, so call
	// RasEnumEntries() to enumerate the connection names into the
	// caller-provided CStringList.
	//
	RASENTRYNAME RasEntry[64];
	memset(&RasEntry[0], 0, sizeof(RasEntry));
	DWORD dwSize = sizeof(RasEntry);
	DWORD dwNumEntries = 0;
	RasEntry[0].dwSize = sizeof(RASENTRYNAME);
	ASSERT(QCRasLibrary::m_pfnRasEnumEntries);
	if (QCRasLibrary::m_pfnRasEnumEntries(NULL, NULL, &RasEntry[0], &dwSize, &dwNumEntries) == 0)
	{
		for (DWORD dwEntry = 0; dwEntry < dwNumEntries; dwEntry++)
			strConnectionList.AddTail(RasEntry[dwEntry].szEntryName);

		hr = S_OK;
	}

	//
	// If we loaded the RAS library, let's clean it up.
	//
	//if (bLoadedLibrary)
	//	FreeRasLibrary_();

	return hr;
}


////////////////////////////////////////////////////////////////////////
// FillRasConnectionNamesCombo [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasLibrary::FillRasConnectionNamesCombo(CComboBox* pComboBox, const char *strEntry)
{
	ASSERT(::IsMainThreadMT());

	if (!pComboBox)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	CStringList strConnectionList;
	if (SUCCEEDED(QCRasLibrary::GetRasConnectionList(strConnectionList)))
	{
		while (! strConnectionList.IsEmpty())
		{
			CString strConnection = strConnectionList.RemoveHead();
			pComboBox->AddString(strConnection);
		}

		//Hilight the dial-up account name with the given entry name
		//int nIndex = pComboBox->FindStringExact(-1, ::GetIniString(IDS_INI_AUTO_CONNECTION_NAME));
		int nIndex = pComboBox->FindStringExact(-1, strEntry);
		if (nIndex != CB_ERR)
			pComboBox->SetCurSel(nIndex);
		
		return S_OK;
	}

	return E_FAIL;
}




////////////////////////////////////////////////////////////////////////
// LoadRasLibrary [private, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasLibrary::LoadRasLibrary_(BOOL /*bShowError*/)
{
	CSingleLock lock(&m_RasGuard, TRUE);

	if(m_bLibraryLoaded)
		return S_OK;

	//
	// Change the current drive so that if it is set to some
	// removable media drive (like a floppy), then the
	// LoadLibrary() call won't cause that drive to be searched
	// (which causes a "Cannot read from Drive X")
	//
	{
		char szPathname[MAX_PATH];
		if (! ::GetModuleFileName(NULL, szPathname, sizeof(szPathname)))
		{
			ASSERT(0);
			return E_FAIL;
		}

		int nExeDriveNum = ::toupper(szPathname[0]) - 'A' + 1;
		if (::_getdrive() != nExeDriveNum)
			::_chdrive(nExeDriveNum);
	}

	// Load the library
	ASSERT(NULL == QCRasLibrary::m_hRasLibrary);
	if ((QCRasLibrary::m_hRasLibrary = ::AfxLoadLibrary("RASAPI32.DLL")) == NULL)
	{
		return E_FAIL;
	}

	
	if (( m_hRasDlgLibrary = ::AfxLoadLibrary("RASDLG.DLL")) != NULL)
	{

#ifdef UNICODE
	QCRasLibrary::m_pfnRasDialDlg			= (RAS_RASDIALDLG *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasDialDlgW");
#else
	QCRasLibrary::m_pfnRasDialDlg			= (RAS_RASDIALDLG *) ::GetProcAddress(QCRasLibrary::m_hRasDlgLibrary, "RasDialDlgA");
#endif
	if( QCRasLibrary::m_pfnRasDialDlg)
	{
		m_bCanUseRasDialDlg = true;
	}


		//if (bShowError)
		//	ReportError(IDS_STRING8204);
		//return E_FAIL;
	}

	ASSERT(NULL == QCRasLibrary::m_pfnRasDial);
	ASSERT(NULL == QCRasLibrary::m_pfnRasEnumConnections);
	ASSERT(NULL == QCRasLibrary::m_pfnRasEnumEntries);
	ASSERT(NULL == QCRasLibrary::m_pfnRasGetConnectStatus);
	ASSERT(NULL == QCRasLibrary::m_pfnRasGetErrorString);
	ASSERT(NULL == QCRasLibrary::m_pfnRasHangUp);	
	ASSERT(NULL == QCRasLibrary::m_pfnRasGetEntryDialParams);
	ASSERT(NULL == QCRasLibrary::m_pfnRasSetEntryDialParams);

#ifdef UNICODE
	QCRasLibrary::m_pfnRasCreatePhonebookEntry = (RAS_RASCREATEPHONEBOOKENTRY *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasCreatePhonebookEntryW");
	QCRasLibrary::m_pfnRasEditPhonebookEntry = (RAS_RASEDITPHONEBOOKENTRY *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasEditPhonebookEntryW");
	QCRasLibrary::m_pfnRasDial				= (RAS_RASDIAL *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasDialW");
	QCRasLibrary::m_pfnRasEnumConnections	= (RAS_RASENUMCONNECTIONS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasEnumConnectionsW");
	QCRasLibrary::m_pfnRasEnumEntries		= (RAS_RASENUMENTRIES *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasEnumEntriesW");
	QCRasLibrary::m_pfnRasGetConnectStatus	= (RAS_RASGETCONNECTSTATUS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasGetConnectStatusW");
	QCRasLibrary::m_pfnRasGetErrorString		= (RAS_RASGETERRORSTRING *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasGetErrorStringW");
	QCRasLibrary::m_pfnRasHangUp				= (RAS_RASHANGUP *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasHangUpW");
	QCRasLibrary::m_pfnRasGetEntryDialParams	= (RAS_RASGETENTRYDIALPARAMS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasGetEntryDialParamsW");
	QCRasLibrary::m_pfnRasSetEntryDialParams	= (RAS_RASSETENTRYDIALPARAMS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasSetEntryDialParamsW");
	
#else
	QCRasLibrary::m_pfnRasCreatePhonebookEntry = (RAS_RASCREATEPHONEBOOKENTRY *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasCreatePhonebookEntryA");
	QCRasLibrary::m_pfnRasEditPhonebookEntry = (RAS_RASEDITPHONEBOOKENTRY *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasEditPhonebookEntryA");
	
	QCRasLibrary::m_pfnRasDial				= (RAS_RASDIAL *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasDialA");
	QCRasLibrary::m_pfnRasEnumConnections	= (RAS_RASENUMCONNECTIONS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasEnumConnectionsA");
	QCRasLibrary::m_pfnRasEnumEntries		= (RAS_RASENUMENTRIES *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasEnumEntriesA");
	QCRasLibrary::m_pfnRasGetConnectStatus	= (RAS_RASGETCONNECTSTATUS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasGetConnectStatusA");
	QCRasLibrary::m_pfnRasGetErrorString		= (RAS_RASGETERRORSTRING *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasGetErrorStringA");
	QCRasLibrary::m_pfnRasHangUp				= (RAS_RASHANGUP *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasHangUpA");
	QCRasLibrary::m_pfnRasGetEntryDialParams	= (RAS_RASGETENTRYDIALPARAMS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasGetEntryDialParamsA");
	QCRasLibrary::m_pfnRasSetEntryDialParams	= (RAS_RASSETENTRYDIALPARAMS *) ::GetProcAddress(QCRasLibrary::m_hRasLibrary, "RasSetEntryDialParamsA");
	
#endif

	if (!QCRasLibrary::m_pfnRasDial ||
		!QCRasLibrary::m_pfnRasEnumConnections ||
		!QCRasLibrary::m_pfnRasEnumEntries ||
		!QCRasLibrary::m_pfnRasGetConnectStatus ||
		!QCRasLibrary::m_pfnRasGetErrorString ||
		!QCRasLibrary::m_pfnRasHangUp||
		!QCRasLibrary::m_pfnRasGetEntryDialParams	||
		!QCRasLibrary::m_pfnRasSetEntryDialParams	
		
		)
	{
		//if (bShowError)
		//	ReportError(IDS_STRING8204);
		FreeRasLibrary_();
		return E_FAIL;
	}

	
	//ASSERT(NULL == m_hRasConn);

	m_bLibraryLoaded = true;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// FreeRasLibrary [private, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasLibrary::FreeRasLibrary_()
{
	CSingleLock lock(&m_RasGuard, TRUE);

	if (QCRasLibrary::m_hRasLibrary)
	{
		QCRasLibrary::m_pfnRasDial = NULL;
		QCRasLibrary::m_pfnRasEnumConnections = NULL;
		QCRasLibrary::m_pfnRasEnumEntries = NULL;
		QCRasLibrary::m_pfnRasGetConnectStatus = NULL;
		QCRasLibrary::m_pfnRasGetErrorString = NULL;
		QCRasLibrary::m_pfnRasHangUp = NULL;

		::AfxFreeLibrary(QCRasLibrary::m_hRasLibrary);
		QCRasLibrary::m_hRasLibrary = NULL;

		if( QCRasLibrary::m_hRasDlgLibrary)
		{
			QCRasLibrary::m_pfnRasDialDlg = NULL;
			::AfxFreeLibrary(QCRasLibrary::m_hRasDlgLibrary);
			QCRasLibrary::m_hRasDlgLibrary = NULL;
		}

		return S_OK;
	}
	else
	{
		ASSERT(0);
		return E_FAIL;
	}
}




void QCRasLibrary::ResetRasDialParams(RASDIALPARAMS *pRasDialParams, const char *szEntry /*NULL*/)
{
	memset(pRasDialParams, 0, sizeof(pRasDialParams));
	pRasDialParams->dwSize = sizeof(RASDIALPARAMS);

	pRasDialParams->szEntryName[0] = 0;
	pRasDialParams->szPhoneNumber[0] = 0;
	pRasDialParams->szCallbackNumber[0] = 0;
	pRasDialParams->szUserName[0] = 0;
	pRasDialParams->szPassword[0] = 0;
	pRasDialParams->szDomain[0] = 0;

	if(szEntry)
		strcpy(pRasDialParams->szEntryName, szEntry);
	
}


void QCRasLibrary::Init(bool bUseExisiting)
{
	m_bUseExistingConn = bUseExisiting;
}

bool QCRasLibrary::HasValidActiveConn(CString strEntry)
{
	CSingleLock lock(&m_RasGuard, TRUE);

	ResynchRasConnection();
	
	// If an existing connection, Eudora never started it, and the option is set to use the existing regardless of the entry,
	// then re-use it :)
	if( m_bActiveConn && !m_bConnectedByEudora && m_bUseExistingConn )
		return true;

	//in case of not using an exisitng conn, matched entries mean a conn to be re-used :)
	if( m_bActiveConn && (m_strEntry.CompareNoCase(strEntry) == 0) )
		return true;

	//no valid connection
	return false;
}
	


bool QCRasLibrary::GetRasPassword(CString strEntry, RASDIALPARAMS* pRasDialParams, bool *pSavePassFlag)
{
	CSingleLock lock(&m_RasGuard, TRUE);

	ASSERT( ::IsMainThreadMT());

	//initialize
	*pSavePassFlag = false;

	//ResynchRasConnection();
	//if( m_bActiveConn && (m_strEntry.CompareNoCase(strEntry) == 0) )
	//	return true;

	if( HasValidActiveConn(strEntry))
		return true;

	// No open connection, so we need to dial.  First, check to 
	// see if we have a connection name.
	if(strEntry == "")
	{
		//
		// Caller didn't specify a connection name, so enumerate all the
		// entries.  If there's just one, then use it.
		//
		RASENTRYNAME RasEntry;
		DWORD dwSize = sizeof(RASENTRYNAME);
		DWORD dwNumEntries = 0;
		RasEntry.dwSize = sizeof(RasEntry);
		if (QCRasLibrary::m_pfnRasEnumEntries(NULL, NULL, &RasEntry, &dwSize, &dwNumEntries) == 0)
			strEntry = RasEntry.szEntryName; //strcpy(RasDialParams.szEntryName, RasEntry.szEntryName);
	}

	if(strEntry == "")
	{
		MessageBox(NULL, "Could not dial to the server.\r\nPlease select a phonebook entry from Tools/Options/Advanced Network", 
			"Dial-up Networking Error", MB_OK);
		return false;
	}

	if( UsingRasDialDlg())
		return true;

	ResetRasDialParams(pRasDialParams, strEntry);
	
	// Get cached username, password and domains if any!
	BOOL bGotPassword = FALSE;
		
	if (QCRasLibrary::m_pfnRasGetEntryDialParams(
				NULL, pRasDialParams, &bGotPassword) == 0)
	{
		if(bGotPassword)
			return true;
	}


	QCRasAuthDlg dlg(strEntry, pRasDialParams);
	if(dlg.DoModal() != IDOK)
		return false;
	
	*pSavePassFlag = dlg.GetSavePassFlag();

	return true;
}



VOID WINAPI RDF( UINT ,								// always WM_RASDIALEVENT
						 RASCONNSTATE state,	    // connection state about to be entered
						 DWORD dwError				// error that may have occurred
						 )
{
	int fDoneState = (state & RASCS_DONE); 
	
	if( QCRasLibrary::g_Progress)
			QCRasLibrary::g_Progress(CRString(IDS_RAS_MESSAGE_FIRST + state));

	if(dwError || fDoneState)
	{
		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "RasConnEvent");
		PulseEvent(hEvent);
	}

	if(dwError)
	{
		//Notify errors to high level layers 
		char szError[256];
		if( QCRasLibrary::m_pfnRasGetErrorString(dwError, szError, sizeof(szError)) == 0)
		{
			//Call the error callback
			if(QCRasLibrary::g_Error)
				QCRasLibrary::g_Error(szError, TERR_RAS);
		}
	}
}


void QCRasLibrary::RequestStop()
{
	
	m_bStopRequested = true;
	
	//Get handle to the RasConnEvent
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "RasConnEvent");
	
	//Signal the event
	PulseEvent(hEvent);
}
	

////////////////////////////////////////////////////////////////////////
// OpenRasConnection [public]
//
////////////////////////////////////////////////////////////////////////
bool QCRasLibrary::OpenConnection(RASDIALPARAMS *pRasDialParams, bool bSavePass, int nTimeOut,
								  ProgressCB Progress, ErrorCB Error)
{
	CSingleLock lock(&m_RasGuard, TRUE);

	g_Progress = Progress;
	g_Error = Error;

	if(m_bStopRequested)
	{
		m_bStopRequested = false;
		return false;
	}

	if( !QCRasLibrary::IsReady() )
	{
		g_Error(CRString(IDS_STRING8204), TERR_RAS);
		return false;
	}

	CString strEntry = pRasDialParams->szEntryName;
	if( strEntry == "")
	{
		TRACE("Dialup: Must have Entry name to use dial-up networking\n");
	}

	if( HasValidActiveConn(strEntry))
		return true;

	//Asking for a new connection.  If there is an existing connection with some other entry, close it first
	//This requirement must be removed in the future if people start having more than one modem at a time
	if(m_bActiveConn)
	{
		//do a hard close as we need to dialup for another connection.
		///ASSUMPTION: User has capability to run one modem at a time only
		CloseConnection(true /*do hard close */);
	}


	bool bOpen = DialConnection(pRasDialParams, bSavePass, nTimeOut);
	g_Progress = 0;

	return bOpen;
}


bool QCRasLibrary::DialConnection(RASDIALPARAMS *pRasDialParams, bool bSavePass, int nTimeOut)
{

	ASSERT( m_hRasConn == NULL);

	if(m_bStopRequested)
	{
		m_bStopRequested = false;
		return false;
	}

	if(! QCRasLibrary::m_pfnRasDialDlg)
	{
		DWORD dwStatus = QCRasLibrary::m_pfnRasDial(NULL, NULL, pRasDialParams, 0, RDF, &m_hRasConn);
		if (dwStatus)
		{
			
			//ReportError(IDS_ERR_RAS_DIAL, dwStatus);
			CString errstr;
			errstr.Format( CRString(IDS_ERR_RAS_DIAL), dwStatus);
			if(g_Error)
				g_Error(errstr, TERR_RAS);
			
			/*if(m_hRasConn)
			{
				QCRasLibrary::m_pfnRasHangUp(m_hRasConn);
				Sleep(3000);
			}
			m_hRasConn = NULL;*/
			Hangup();
			return false;
		}

		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "RasConnEvent");
		
		//Timeout is in seconds
		WaitForSingleObject(hEvent, nTimeOut * 1000);

		if(!m_hRasConn)
		{
			ASSERT(0);
			return false;
		}

		if(m_bStopRequested)
		{
			g_Progress("Stopping in progress...");
			
			/*//Need to Hangup even if unsuccessful..
			QCRasLibrary::m_pfnRasHangUp(m_hRasConn);
			Sleep(3000);
			m_hRasConn = NULL;*/
			Hangup();
		
			m_bStopRequested = false;
			return false;
		}


		RASCONNSTATUS status;
		memset(&status, 0, sizeof(status));
		status.dwSize = sizeof(RASCONNSTATUS);

		if( QCRasLibrary::m_pfnRasGetConnectStatus(m_hRasConn, &status) == 0)
		{
			if( status.rasconnstate == RASCS_Connected)
			{
				m_strEntry = pRasDialParams->szEntryName;
				m_bActiveConn = true;
				m_bConnectedByEudora = true;

				//Save the RAS password if needed
				if(bSavePass)
					QCRasLibrary::m_pfnRasSetEntryDialParams(NULL, pRasDialParams, FALSE);

				TRACE("Successful dialup conn made to %s\n", m_strEntry);
				return true;
			}
		}
		

		/*if(m_hRasConn)
		{
			//Need to Hangup even if unsuccessful..
			QCRasLibrary::m_pfnRasHangUp(m_hRasConn);
			Sleep(3000);
		}
		m_hRasConn = NULL;*/
		
		Hangup();
			
		m_strEntry = "";
		m_bActiveConn = false;
		return false;
		
	}
	else
	{
		//RasDialDlg
		RASDIALDLG RasDlg;
		memset(&RasDlg, 0, sizeof(RasDlg));
		RasDlg.dwSize = sizeof(RASDIALDLG);
		char szEntry[256] = { 0 };
		strcpy(szEntry, pRasDialParams->szEntryName);

		if (QCRasLibrary::m_pfnRasDialDlg(NULL, szEntry, NULL, &RasDlg) == 0)
		{
			TRACE("RasDialup failed with error %d\n", RasDlg.dwError);
			
			//Notify errors to high level layers 
			char szError[256];
			if( QCRasLibrary::m_pfnRasGetErrorString(RasDlg.dwError, szError, sizeof(szError)) == 0)
			{
				//Call the error callback
				if(QCRasLibrary::g_Error)
					QCRasLibrary::g_Error(szError, TERR_RAS);
			}
				
			return false;
		}
		m_bConnectedByEudora = true;
		
		if( HasValidActiveConn(szEntry))
			return true;
	}

	return false;
}






QCRasAuthDlg::QCRasAuthDlg(CString strEntry, RASDIALPARAMS *pDialParams, CWnd* pParent)
	: CDialog(QCRasAuthDlg::IDD, pParent), m_pDialParams(pDialParams)
{
	ASSERT(::IsMainThreadMT());
	ASSERT(!strEntry.IsEmpty());

	m_strTitle.Format("Connect to %s", strEntry);
	
	m_strUserName = pDialParams->szUserName;
	m_strDomain = pDialParams->szDomain;
	m_bSavePassword = FALSE;

	//{{AFX_DATA_INIT(QCRasAuthDlg)
	//m_bSavePassword = GetIniShort(IDS_INI_AUTO_CONNECTION_SAVE_PASSWORD);
//FORNOW	m_Domain = GetIniString(IDS_INI_AUTO_CONNECTION_DOMAIN);
//FORNOW	m_strPassword = s_AutoConnectionPassword;
//FORNOW	m_Username = GetIniString(IDS_INI_AUTO_CONNECTION_USERNAME);
	//}}AFX_DATA_INIT
}

BOOL QCRasAuthDlg::OnInitDialog()
{
	UpdateData(FALSE);
	SetWindowText(m_strTitle);
	return CDialog::OnInitDialog();
}

void QCRasAuthDlg::OnOK()
{
	UpdateData(); //Get data from dialog into member variables
	UpdateInfo();

	//save the settings except password which will get saved if needed
	QCRasLibrary::m_pfnRasSetEntryDialParams(NULL, m_pDialParams, TRUE);
	
	CDialog::OnOK();
}


bool QCRasAuthDlg::GetSavePassFlag(){ return m_bSavePassword?true:false; }

void QCRasAuthDlg::UpdateInfo()
{
	strcpy(m_pDialParams->szUserName,  m_strUserName);
	strcpy(m_pDialParams->szDomain,  m_strDomain);
	strcpy(m_pDialParams->szPassword,  m_strPassword);
}



void QCRasAuthDlg::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(::IsMainThreadMT());

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(QCRasAuthDlg)
	//DDX_Control(pDX, IDS_INI_AUTO_CONNECTION_NAME, m_ConnectionCombo);
	DDX_Check(pDX, IDS_INI_AUTO_CONNECTION_SAVE_PASSWORD, m_bSavePassword);
	DDX_Text(pDX, IDS_INI_AUTO_CONNECTION_DOMAIN, m_strDomain);
	DDX_Text(pDX, IDS_INI_AUTO_CONNECTION_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDS_INI_AUTO_CONNECTION_USERNAME, m_strUserName);
	//}}AFX_DATA_MAP

	/*
	if (pDX->m_bSaveAndValidate)
	{
		SetIniString(IDS_INI_AUTO_CONNECTION_USERNAME, m_strUserName);
		SetIniShort(IDS_INI_AUTO_CONNECTION_SAVE_PASSWORD, (short)m_bSavePassword);
		QCRasConnection::SetRasPassword(m_strPassword, m_bSavePassword);
		SetIniString(IDS_INI_AUTO_CONNECTION_DOMAIN, m_strDomain);
		m_ConnectionCombo.GetWindowText(m_strConnectionName);
		if (m_strConnectionName.IsEmpty() == FALSE)
			SetIniString(IDS_INI_AUTO_CONNECTION_NAME, m_strConnectionName);
	}
	else
	{
		QCRasLibrary::FillRasConnectionNamesCombo(&m_ConnectionCombo, "");
		if (m_ConnectionCombo.GetCurSel() == CB_ERR)
		{
			int nIndex = m_ConnectionCombo.FindStringExact(-1, m_strConnectionName);
			if (nIndex != CB_ERR)
				m_ConnectionCombo.SetCurSel(nIndex);
		}
	}*/
}


BEGIN_MESSAGE_MAP(QCRasAuthDlg, CDialog)
	//{{AFX_MSG_MAP(QCRasAuthDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// QCRasAuthDlg message handlers



















/*		//
		// Open up the Auto Connection dialog if there's either no password or
		// no connection name specified
		//
		if (strPassword.IsEmpty() || !*RasDialParams.szEntryName || strUserName.IsEmpty())
		{
			CAutoConnectionDlg2 dlg;
			dlg.m_strUserName = strUserName;
			dlg.m_strPassword = strPassword;
			dlg.m_strDomain = strDomain;
			dlg.m_strConnectionName = strConnectionName;
			if (dlg.DoModal() != IDOK)
			{
				//QCRasConnection::FreeRasLibrary_();
				DestroyWindow();
				m_hWnd = NULL;
				return E_ABORT;
			}


			//
			// Go with user-provided data from dialog box.
			//
			strcpy(RasDialParams.szEntryName, dlg.m_strConnectionName);
			strcpy(RasDialParams.szUserName, dlg.m_strUserName);
			strcpy(RasDialParams.szPassword, dlg.m_strPassword);
			strcpy(RasDialParams.szDomain, dlg.m_strDomain);

			if(dlg.m_bSavePassword)
			{
				BOOL bRemovePassword = FALSE;
				QCRasConnection::m_pfnRasSetEntryDialParams(NULL, &RasDialParams, bRemovePassword);
			}
		}
		else
		{
			//
			// Go with the caller-provided data.
			//
			strcpy(RasDialParams.szUserName, strUserName);
			strcpy(RasDialParams.szPassword, strPassword);	//FORNOW ... does this get set by auto-connect dialog?
			strcpy(RasDialParams.szDomain, strDomain);
		}

		m_uRasDialEvent = ::RegisterWindowMessageA(RASDIALEVENT);
		if (0 == m_uRasDialEvent)
			m_uRasDialEvent = WM_RASDIALEVENT;

		m_dwTimeout = GetTickCount() + (1000L * nTimeout);
*/

	

/*
		DWORD dwStatus = QCRasConnection::m_pfnRasDial(NULL, NULL, &RasDialParams, 0xFFFFFFFF, m_hWnd, &QCRasConnection::m_hRasConn);
		if (dwStatus)
		{
			::ReportError(IDS_ERR_RAS_DIAL, dwStatus);
			QCRasConnection::m_pfnRasHangUp(QCRasConnection::m_hRasConn);
			QCRasConnection::m_hRasConn = NULL;
			//QCRasConnection::FreeRasLibrary_();
			DestroyWindow();
			m_hWnd = NULL;
			return E_FAIL;
		}

		m_RasConnState = RASCS_OpenPort;
		BOOL bOpenedProgress = !InProgress;
		MainProgress(CRString(IDS_STARTING_UP_RAS));
		m_lRasErrorValue = 0;
		int nResult = WaitForMessage_(WORD(m_uRasDialEvent), nTimeout);
		if (bOpenedProgress)
			CloseProgress();

		if (nResult < 0)
		{
			QCRasConnection::m_pfnRasHangUp(QCRasConnection::m_hRasConn);
			QCRasConnection::m_hRasConn = NULL;
			//QCRasConnection::FreeRasLibrary_();
			DestroyWindow();
			m_hWnd = NULL;
			return E_ABORT;
		}

		if (m_RasConnState == RASCS_Connected)
		{
			//
			// Minimize the Connect To dialog, if we can find it
			//
			CRString ConnectToClass(IDS_CONNECT_TO_CLASS);
			char szTitle[256];
			sprintf(szTitle, CRString(IDS_CONNECT_TO_TITLE), RasDialParams.szEntryName);
			CWnd* pConnectDlg = CWnd::FindWindow(ConnectToClass, szTitle);
			if (! pConnectDlg)
			{
				HWND hwndConnectTo = NULL;
				while (hwndConnectTo = ::FindWindowEx(NULL, hwndConnectTo, ConnectToClass, NULL))
				{
					// If we found a window of just that class, do a simple check to see
					// if this is really the right dialog by checking the existence of
					// some controls in the dialog
					pConnectDlg = CWnd::FromHandle(hwndConnectTo);
					for (UINT nID = 1001; nID <= 1006; nID++)
					{
						if (::GetDlgItem(hwndConnectTo, nID) == NULL)
						{
							pConnectDlg = NULL;
							break;
						}
					}
					if (pConnectDlg)
						break;
				}
			}
			if (pConnectDlg)
				pConnectDlg->ShowWindow(SW_MINIMIZE);
		}
		else
		{
			QCRasConnection::m_pfnRasHangUp(QCRasConnection::m_hRasConn);
			QCRasConnection::m_hRasConn = NULL;
			//QCRasConnection::FreeRasLibrary_();
			DestroyWindow();
			m_hWnd = NULL;
			return E_FAIL;
		}
	
	//
	// Always bump the reference count, even if we didn't create a connection.
	//
	//QCRasConnection::m_nRefCount++;

	return S_OK;
}
*/


#if 0

////////////////////////////////////////////////////////////////////////
// SetRasPassword [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasConnection::SetRasPassword(const CString& strRasPassword, BOOL bSavePassword)
{
	//ASSERT(::IsMainThreadMT());

	//QCRasConnection::m_strRasPassword = strRasPassword;

	if (bSavePassword)
		SetIniString(IDS_INI_AUTO_CONNECTION_PASSWORD, EncodePassword(strRasPassword));
	else
		SetIniString(IDS_INI_AUTO_CONNECTION_PASSWORD, NULL);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// GetRasPassword [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasConnection::GetRasPassword(CString& strRasPassword)
{
	//ASSERT(::IsMainThreadMT());
#if 0
	if (QCRasConnection::m_strRasPassword.IsEmpty())
	{
		if (::GetIniShort(IDS_INI_AUTO_CONNECTION_SAVE_PASSWORD))
			QCRasConnection::m_strRasPassword = ::DecodePassword(::GetIniString(IDS_INI_AUTO_CONNECTION_PASSWORD));
	}

	strRasPassword = QCRasConnection::m_strRasPassword;
#endif
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// ForgetRasPassword [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCRasConnection::ForgetRasPassword()
{

	/*
	ASSERT(::IsMainThreadMT());

	//
	// The IDS_INI_SAVE_DIALUP_PASSWORD_TEXT setting is a historical
	// setting for the old "modem shell account" access mode known as
	// "dialup".  This setting is no longer used, but for people with
	// old INI files, let's clear out the old setting value so that no
	// one can steal the password.
	//
	SetIniString(IDS_INI_SAVE_DIALUP_PASSWORD_TEXT, NULL);

//	QCRasConnection::m_strRasPassword.Empty();
	SetIniString(IDS_INI_AUTO_CONNECTION_PASSWORD, NULL);
*/
	return S_OK;
}
#endif
