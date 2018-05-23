#include "stdafx.h"
#include "QCWorkerSocket.h"
#include "QCSockCallbacks.h"
#include "QCFileIO.h"    //for CFileIO
#include "QCNetSettings.h" //for QCNetworkSettings class
#include "fileutil.h"   //for JJfile
#include "rs.h"         //for INI functions
#include "Resource.h"   //for Resource IDs
#include "progress.h"   //for Progress bar functions
#include "guiutils.h"   //for AlertDialog
#include "password.h"   //for EncodePassword

#include "persona.h"  //for getting current personality as arg to qcworkersocket

#ifdef EXPIRING
	#include "timestmp.h"
	#include "mainfrm.h"
	extern CTimeStamp	g_TimeStamp;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//                                                                    //
//                          C F i l e I O                             //
//                                                                    //
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//========================================================================
// CFileIO: This class will redirect output into a file
//========================================================================

//IMPLEMENT_DYNAMIC(CFileIO, Network) 

//========================================================================
CFileIO::CFileIO(int Size /*= -1*/) :
	Network(NULL, Size<0?4096:Size)
{
	m_bIsFileBased = TRUE;
}


//========================================================================
CFileIO::~CFileIO()
{
}


//========================================================================
int 
CFileIO::Open(const char* Machine, UINT ServiceID, UINT PortIniID, UINT DefaultPort, int Quiet /*= FALSE*/, int FailOver /*= FALSE*/)
{
	// This function must be over-rode, but it doesn't make sense for a FileIO
	ASSERT(0);
	return 0;
}
	
//========================================================================
int 
CFileIO::OpenFile(const char* FileName, int mode)
{
	if (FAILED(m_jjFile.Open(FileName, mode)))
		return -1;

	return 1;
}
	
//========================================================================
int 
CFileIO::Close()
{
	if (FAILED(m_jjFile.Close()))
		return -1;

	return 0;
}

//========================================================================
void 
CFileIO::Reset()
{ 
	m_pBufPtr = m_pBuffer; 
	m_nValidBytes = 0; 
	m_jjFile.Reset();
}

//========================================================================
void 
CFileIO::Flush()
{ 
	m_jjFile.Flush();
}

//========================================================================
int 
CFileIO::PutDirect(const char* buf /*= NULL*/, int length /*= -1*/)
{
	if ( FAILED(m_jjFile.Put( buf,  length)))
		return -1;
	// JAB: Put Direct gets called for Net Commands, don't really want to write out that
	// CBL: Uncommented this line for Spooling.
	return 1;
}

//========================================================================
int 
CFileIO::Put(const char* bufptr, int length)
{
	if (FAILED(m_jjFile.Put( bufptr,  length)))
		return -1;
	return 1;
}

//========================================================================
int 
CFileIO::Read()
{
	return 0;
}

//========================================================================
int 
CFileIO::Write()
{
	if (FAILED(m_jjFile.Put( m_pBuffer,  m_nValidBytes)))
		return -1;
	return 1;
}






// Function Prototypes

//Network* CreateNetConnection(BOOL ShowProgress = TRUE, BOOL FileIO = FALSE );
Network* NetConnection;


void ReportNetError(const char *err, TaskErrorType)
{
	AlertDialog(IDD_ERROR_DIALOG, err);
}


////////////////////////////////////////////////////////////////////////
// CreateNetConnection [extern]
//
////////////////////////////////////////////////////////////////////////
Network* CreateNetConnection(BOOL ShowProgress /*= TRUE*/, BOOL FileIO /*= FALSE*/)
{
	//ASSERT(::IsMainThreadMT());

	//Network* theConnection = NULL;
	
#ifdef EXPIRING
	// this is the first line of defense
	if ( g_TimeStamp.IsExpired0() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
		return NULL;
	}
#endif
	
	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);
		else if (res == IDCANCEL)
			return (NULL);

//		ErrorDialog(IDS_ERR_OFFLINE);
//		return (NULL);
	}
	
	// Put up the progress dialog
	if (ShowProgress && !FileIO)
		::MainProgress(CRString(IDS_START_NET_CONNECTION));

	if (!NetConnection)
	{
		if (FileIO)
			//theConnection = new CFileIO;
			NetConnection = new CFileIO;
		else
		{
retry:
			if (GetIniShort(IDS_INI_CONNECT_WINSOCK))
			{
				QCNetworkSettings *settings = new QCNetworkSettings(g_Personalities.GetCurrent());
				settings->SetReportErrorCallback(makeCallback( (QCNetworkSettings::ErrorCallback *)0, ReportNetError));
				settings->SetProgressCallback(makeCallback( (Callback1<const char *> *)0, Progress));
				NetConnection = new QCWorkerSocket(settings);
			}
			else
			{
				//
				// Eudora no longer supports shell account "dialup"
				// connections.  So, prompt the user and ask whether or
				// not it is okay to change the connection method to
				// WinSock.  
				//
				if (IDYES == ::AfxMessageBox(IDS_CONVERT_TO_WINSOCK, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
				{
					SetIniShort(IDS_INI_CONNECT_WINSOCK, 1);
					goto retry;
				}
			}
		}

	    if (!NetConnection || !NetConnection->IsOK() )
		{
			if ( NetConnection )
			{
				// IsOK failed
				//::ErrorDialog( IDS_ERR_NOT_ENOUGH_MEMORY );
			}

		    delete NetConnection;
		}
	}


	return (NetConnection);
}





////////////////////////////////////////////////////////////////////////
// CreateNetConnection [extern]
//
////////////////////////////////////////////////////////////////////////
Network* CreateNetConnectionMT(QCNetworkSettings *pSettings, 
							   BOOL ShowProgress /*= TRUE*/, BOOL FileIO /*= FALSE*/)
{
	//ASSERT(::IsMainThreadMT());


#ifdef EXPIRING
	// this is the first line of defense
	if ( g_TimeStamp.IsExpired0() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
		return NULL;
	}
#endif

	Network* theConnection = NULL;
	
		//FORNOW, no thread-safe UI
#if 0

	if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{

		int res = AlertDialog(IDD_ERR_OFFLINE);

		if (res == IDC_ONLINE_BUTTON)
			SetIniShort(IDS_INI_CONNECT_OFFLINE, 0);
		else if (res == IDCANCEL)
			return (NULL);

//		ErrorDialog(IDS_ERR_OFFLINE);
//		return (NULL);

	}
#endif

	
	// Put up the progress dialog
	if (ShowProgress)
		::MainProgress(CRString(IDS_START_NET_CONNECTION));

	if (FileIO)
		theConnection = new CFileIO;
	else
	{
			theConnection = new QCWorkerSocket(pSettings);

#if 0		
retry:
		if (GetIniShort(IDS_INI_CONNECT_WINSOCK))
			theConnection = new QCWorkerSocket;

		else
		{
			//
			// Eudora no longer supports shell account "dialup"
			// connections.  So, prompt the user and ask whether or
			// not it is okay to change the connection method to
			// WinSock.  
			//
			if (IDYES == ::AfxMessageBox(IDS_CONVERT_TO_WINSOCK, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
			{
				SetIniShort(IDS_INI_CONNECT_WINSOCK, 1);
				goto retry;
			}
		}
#endif
	}


	
	if (!theConnection || !theConnection->IsOK() )
	{
		if ( theConnection )
		{
			// IsOK failed
			//::ErrorDialog( IDS_ERR_NOT_ENOUGH_MEMORY );
		}

		delete theConnection;
	}

	return (theConnection);
}




