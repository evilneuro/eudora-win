// AcapDlg.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "acapdlg.h"
#include "QCWorkerSocket.h"
#include "guiutils.h"
#include "progress.h"
#include "password.h"
#include "md5.h"
#include "Base64.h"
#include "QCNetSettings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


extern BOOL g_bShowProgress;	
BOOL g_bProgressHasFocus; // shared with Progress.cpp (yuk)


int LOGIN_FLAG=0;

static char g_Password[1024];
char ClearText[1024];


const char* EncodePassword(int len);

//////////////////////////////////////////////////
///			HELPER FUNCTIONS
/////////////////////////////////////////////////

const char* EncodePassword(int len)
{
	Base64Encoder	TheEncoder;
	char*			OutSpot = g_Password;
	LONG			OutLen = 0;

	if (ClearText)
	{
		TheEncoder.Init(g_Password, OutLen,0);
		OutSpot += OutLen;
		TheEncoder.Encode(ClearText, len, OutSpot, OutLen);
		OutSpot += OutLen;
		TheEncoder.Done(OutSpot, OutLen);
		OutLen += OutSpot - g_Password;
	}
	g_Password[OutLen] = 0;

	return (g_Password);
}

/************************************************************************
 * GetStamp - grab the timestamp out of a POP banner
 ************************************************************************/
unsigned char * 
GetStamp(unsigned char * stamp,unsigned char * banner)
{
	unsigned char * cp1,*cp2;
	int		len = 0;
	//*stamp = 0;
	
	//banner[*banner+1] = 0;
	
	if (cp1 = (unsigned char *)strchr((char *)banner,'<'))
	{
		if (cp2=(unsigned char *)strchr((char *)cp1+1,'>'))
		{
			len = cp2-cp1+1;
			strncpy((char *)stamp,(char *)cp1,len);
		} 
	}
	
	return (stamp);
}  

/************************************************************************
 * GenerateDigest - generate a digest for APOP
 ************************************************************************/
BOOL 
GenerateDigest(char* banner,char* secret,char* digest)
{
	char 		stamp[255];
	MD5_CTX 	md5;
	static char hex[]="0123456789abcdef";
	short 		i;
	UINT		len = 0;
	
	memset(stamp, 0, sizeof(stamp));
		
	if (!*GetStamp((unsigned char *)stamp,(unsigned char *)banner))
	{
		// Error_Notify(IDS_APOP_NO_BANNER_ERROR, -1);
		 return (FALSE);
	}
	
	MD5Init(&md5);
    
	len = strlen(stamp);
	MD5Update(&md5,(unsigned char *)stamp,len);

	len = strlen(secret);
	MD5Update(&md5,(unsigned char *)secret,len);
	MD5Final(&md5);

	for (i = 0; i < sizeof(md5.digest); i++)
	{
		digest[2*i+1] = hex[(md5.digest[i]>>4)&0xf];
		digest[2*i+2] = hex[md5.digest[i]&0xf];
	}

	digest[0] = 2 * sizeof(md5.digest);

	return (TRUE);
}


//produce hmac using md5

int hmac_md5(unsigned char* text,unsigned char* key ,char* digest)
{
        MD5_CTX context;
        unsigned char stamp[255];

        unsigned char k_ipad[64];   
        unsigned char k_opad[64];    /* outer padding -
                                      * key XORd with opad
                                      */
        memset(stamp,0,sizeof(stamp));

       	if (!*GetStamp((unsigned char *)stamp,(unsigned char *)text))
    	{
	    	// Error_Notify(IDS_APOP_NO_BANNER_ERROR, -1);
		     return (FALSE);
	    }

        unsigned char tk[16];
        static char hex[]="0123456789abcdef";
        
        text=stamp;
        
        int i;
        int ntext=strlen((const char* )text);
        int nkey=strlen((const char* )key);
        
        if (nkey > 64) 
        {
             MD5_CTX      tctx;
             MD5Init(&tctx);
             MD5Update(&tctx,(unsigned char*) key, nkey);
             MD5Final(&tctx);
          
             key  = (unsigned char* )tk;
             nkey = 16;
        }

        memset( k_ipad,0, sizeof(k_ipad));
        memset( k_opad,0, sizeof(k_opad));
        memcpy( k_ipad,key, nkey);
        memcpy( k_opad,key,nkey);

        
        for (i=0; i<64; i++) 
        {
            k_ipad[i] ^= 0x36;
            k_opad[i] ^= 0x5C;
        }
            
        MD5Init(&context);                
        MD5Update(&context, k_ipad, 64);  
        MD5Update(&context, text, ntext); 
        MD5Final(&context);          
        
         

        MD5Init(&context);                
        MD5Update(&context, k_opad, 64);  
        MD5Update(&context, (unsigned char* )context.digest, 16); 
        MD5Final(&context);
        
      	for (i = 0; i < sizeof(context.digest); i++)
    	{
	    	digest[2*i+1] = hex[(context.digest[i]>>4)&0xf];
		    digest[2*i+2] = hex[context.digest[i]&0xf];
	    }

	    digest[0] = 2 * sizeof(context.digest);
    
	return (TRUE);

}

void UTF8To88591(char * inStr, long inLen, char * outStr, long *outLen)
{
	long len;
	BYTE tempChar;

	len = 0L;
	while(--inLen >= 0L)
	{
		tempChar = *inStr++;
		if(tempChar & 0x80)
		{
			if(tempChar & 0x3C)
			{
				*outStr++ = '?';
				++len;
				while((tempChar <<= 1) & 0x80)
				{
					--inLen;
					++inStr;
				}
			}
			else
			{
				*outStr++ = static_cast<char>( ((tempChar & 0x03) << 6) +
				(*inStr & 0x7F) );
				++len;
				--inLen;
			}
		}
		else
		{
			*outStr++ = tempChar;
			++len;
		}
	}
	*outLen = len;
}

//////////////////////////////////////////////////////////////////

CAcapSettings::CAcapSettings()
{
	m_pStringIDList = new CObList;
	m_szStamp = new char[1024];
	m_pPositions = new CObArray;

}

CAcapSettings::~CAcapSettings()
{
	if(m_pStringIDList)
		delete m_pStringIDList;
	m_pStringIDList = NULL;
	if(m_szStamp)
		delete [] m_szStamp;
	if(m_pPositions)
	{
		m_pPositions->RemoveAll();
		delete m_pPositions;
		m_pPositions = NULL;
	}
	if (DeleteNetObj == TRUE)
	{
		NetConnection->Close();
		delete NetConnection;
		NetConnection = NULL;
		CloseProgress();
	}

}

BOOL CAcapSettings::RetrieveSettings(CString strServer, CString strUserID, CString strPass)
{
	CString strError;
	char buf[64];
	BOOL bSuccess = FALSE;
	m_strUserID = strUserID;
	m_strServerName = strServer;
	m_strPassword = strPass;
	int nStatus	=	1 ;

	
	if (!NetConnection) // use the global Netconnection object
	{
		g_bShowProgress = TRUE;
		g_bProgressHasFocus = TRUE;

		if (!CreateNetConnection()) 
		{
			Connected = FALSE;
			return FALSE ;
		}
		
	}
	{
		DeleteNetObj = TRUE;

		if (NetConnection)
		{
			int nReturn = ConnectAcapServer();
			
			if(nReturn < 0 )
			{
				if(nReturn != -2)
				{
					CString strError;

					strError.LoadString(IDS_ACAP_ERROR_HOST);
			
					ErrorDialog(IDS_ACAP_ERROR_CONNECT, strError);
					LOGIN_FLAG=0;
					return FALSE;
				}	

			}
			else
			{
				CString strOldPasswd;
		
				LoadStrings();
				nReturn = ConfigureClient();

				if( nReturn) 
				{
					//Check to see if the user already has a pop password, if not then copy this one
					strOldPasswd = DecodePassword(GetIniString(IDS_INI_SAVE_PASSWORD_TEXT,buf, sizeof(buf)));
					CString strUserName = GetIniString(IDS_INI_LOGIN_NAME);
					
					if(strOldPasswd.IsEmpty() && (_stricmp(strOldPasswd, strUserName) ==0))
						SetIniString(IDS_INI_SAVE_PASSWORD_TEXT, EncodePassword(m_strPassword));
					bSuccess = TRUE;
				}
				else
					bSuccess = FALSE;
				
			}
		}
		else
			LOGIN_FLAG = 1;

		if(LOGIN_FLAG)
		{
			CString strError;
			strError.LoadString(IDS_ACAP_ERROR_STARTUP);
			ErrorDialog(IDS_ACAP_ERROR_CONNECT, strError);
			bSuccess = FALSE;	
		}
	}
	g_bShowProgress = FALSE;
	g_bProgressHasFocus = FALSE;
	return bSuccess;
}




//+---------------------------------------------------------------------------
//
//  Function:	ConnectAcapServer
//
//  Synopsis:   This basically creates a socket and connects to the acap server.
//				As of now it does not do any guessing if the acap server is not 
//				present.

//  Arguments:  void

//	Return:		

//  History:    
//
//  Notes:
//
//----------------------------------------------------------------------------
int CAcapSettings::ConnectAcapServer()
{
	//WSADATA WsaData;
	LPHOSTENT lpHostEntry = NULL;
	CString szPortNumber;
	int error=0;
	
	//Make sure we have a username and password before we do any socket stuff...don't waste time.
	if((strlen(m_strUserID)==0) || (strlen(m_strPassword)==0))
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONNECT,strError);
		LOGIN_FLAG=0;	//set this to 0 so we don't get multiple error messages
		return -1;
	}
		
	

	szPortNumber="674"; //This can be made an option if the port no. is not decided on.

	if(!QCWinSockLibMT::LoadWSLibrary())
	{
		CString strError;

		strError.LoadString(IDS_ACAP_ERROR_STARTUP);  
		ErrorDialog(IDS_ACAP_ERROR_CONNECT, strError);
		LOGIN_FLAG=0;
		return -1;
	}

	if( !QCWinSockLibMT::Init())
	{
		ErrorDialog(IDS_ERR_NO_WINSOCK_DLL);
		return -1;
	}

			
	if(m_strServerName.IsEmpty())
	{
		CString strHostName;

		lpHostEntry=QCWinSockLibMT::gethostbyname("LOCALHOST");	
	
		long err = GetLastError();
		if(lpHostEntry!=NULL)
		{
			strHostName = lpHostEntry->h_name;
			//Try to find an acap server if possible
			m_strServerName = GetAcapServer(strHostName);
		}
		else
			m_strServerName = "";
	}

	lpHostEntry=QCWinSockLibMT::gethostbyname(m_strServerName);

	if(lpHostEntry==NULL)
	{
		CString strError;

		strError.LoadString(IDS_ACAP_ERROR_HOST);
		
		ErrorDialog(IDS_ACAP_ERROR_CONNECT, strError);
		LOGIN_FLAG=0;
		//QCWinSockLibMT::FreeWSLibrary();
		return -1;
	}
	QCWinSockLibMT::Cleanup(GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE)?true:false);  //decrement the ref count

	
	SetIniString(IDS_INI_ACAP_SERVER, m_strServerName);

	int nStatus;

	nStatus = NetConnection->Open(m_strServerName, IDS_INI_ACAP_SERVER, 674, 674);

	if( nStatus < 0) 
	{
		return nStatus;
	}

				
	return 1;
		
}



//+---------------------------------------------------------------------------
//
//  Function:	GetAcapServer
//
//  Synopsis:   This function tries to find the ACAP server using the local machine
//				name first and then the DNS server.

//  Arguments:  CString strRemainingName - host name to start the search from

//	Return:		Server name

//  History:    
//
//  Notes:
//
//----------------------------------------------------------------------------
CString CAcapSettings::GetAcapServer(CString strRemainingName)
{
	//WSADATA WsaData;
	LPHOSTENT lpHostEntry = NULL;
	CString	strHostName;
	CString strACAP = "acap.";
	int error=0;

	/*error= QCWinSockLibMT::WSAStartup(0x0101,&WsaData);
	if(error==SOCKET_ERROR)
	{
		CString strError;

		strError.LoadString(IDS_ACAP_ERROR_STARTUP);  
		ErrorDialog(IDS_ACAP_ERROR_CONNECT, strError);
		LOGIN_FLAG=0;
		return "";
	}*/

	strHostName = strACAP + strRemainingName;

	while(strHostName.Find('.') != strHostName.ReverseFind('.'))
	{
		//Keep going if we have 2 or more elements after "acap"
		lpHostEntry= QCWinSockLibMT::gethostbyname(strHostName);
		if(lpHostEntry)
			return strHostName;

		strRemainingName = strRemainingName.Right(strRemainingName.GetLength() - (strRemainingName.Find('.') + 1));
		strHostName = strACAP + strRemainingName;

	}

	return "";
}

//+---------------------------------------------------------------------------
//
//  Function:	ConfigureClient
//
//  Synopsis:   This function does all the work.  Get's the greeting, then authenticates.

//  Arguments:  void

//	Return:		

//  History:    
//
//  Notes:
//
//----------------------------------------------------------------------------

int CAcapSettings::ConfigureClient(void)
{
	
	char szBuffer[1024];
	
	//This will always be successful??  should ask Anand about this
	int nReturn = GetGreeting();

   	if(nReturn < 0)
		return nReturn;
	else
	{
    	if((m_Authentication==AUTH_XCRAM) ||  (m_Authentication==AUTH_CRAM) ||(m_Authentication==AUTH_APOP))
			nReturn = AuthenticateAPOPCRAMAcap(m_Authentication);
        else if(m_Authentication == AUTH_PLAIN)
			nReturn = AuthenticatePlainAcap();

		if(nReturn  < 0)
		{
			LOGIN_FLAG = 1;
			if(nReturn == -2)
				LOGIN_FLAG = 0;
			return nReturn;
		}
	
        strcpy(szBuffer,". search \"/Vendor.Eudora/~/\" return (\"*\") equal \"entry\" \"en-nocase\" \"settings\"\r\n");
	
		nReturn = NetConnection->PutDirect( szBuffer);
		if(  nReturn < 0) 
		{
			CString strError;
			strError.LoadString(IDS_ACAP_ERROR_LOGIN);
			ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);  
			LOGIN_FLAG=0;
			CloseConnection();
			DeleteNetObj = FALSE;
			return nReturn;
		}
			
		ReceiveSearch(szBuffer);// This is where the data from the acap server has to be parsed
		
		LogoutAcap();
	}
	

    return 1;
}


//this is just to get the initial greeting from the acap server.
int CAcapSettings::GetGreeting(void)
{

	char szBuffer[QC_NETWORK_BUF_SIZE];
	char szTempBuffer[1024];
	CString szMsg;
	int nReturn = NetConnection->GetLine( szBuffer, sizeof( szBuffer));
	if(nReturn < 0)
	{
		return nReturn;
	}

    
   	if(strcmp(szBuffer,"* Eudora-AutoConfigure (IMPLEMENTATION \"1.0.0\") (SASL \"PLAIN\")\r\n")!=0)
	{}		  //this is interesting???
	
	strcpy(szTempBuffer,szBuffer);

  	if(strstr(_strupr(szTempBuffer),"X-CRAM-MD5")!=0) //the server supports apop
	{
		GetStamp((unsigned char* )m_szStamp,(unsigned char* )szBuffer);
		m_Authentication = AUTH_XCRAM;
	}

	else if(strstr(_strupr(szTempBuffer),"CRAM-MD5")!=0) //the server supports apop
	{
		GetStamp((unsigned char* )m_szStamp,(unsigned char* )szBuffer);
		m_Authentication = AUTH_CRAM;
	}

	else if(strstr(_strupr(szTempBuffer),"APOP")!=0) //the server supports apop
	{
		GetStamp((unsigned char* )m_szStamp,(unsigned char* )szBuffer);
		m_Authentication = AUTH_APOP;
	}

    //since the server does not support apop check if it supports plain text authentication
	else if(strstr(_strupr(szTempBuffer),"PLAIN")!=0) 
		m_Authentication = AUTH_PLAIN;
	
	else //this means that there is no plain text also in the server
        m_Authentication = AUTH_NONE;
		
	
    
	return TRUE;

}


//this sends plain text over the n/w.	
int CAcapSettings::LoginAcap(void)
{
	char szBuffer[QC_NETWORK_BUF_SIZE];
	LOGIN_FLAG=0;

	strcpy(szBuffer,". login ");
	strcat(szBuffer,m_strUserID);
	
	strcat(szBuffer," ");
	strcat(szBuffer,m_strPassword);
	strcat(szBuffer,"\r\n");

	if(NetConnection->PutDirect( szBuffer) < 0) 
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);  //Not an accurate error but for now it will do.
		LOGIN_FLAG=0;
		CloseConnection();
		DeleteNetObj = FALSE;
		return FALSE;
	}

    memset(szBuffer,0,sizeof(szBuffer));

	if(NetConnection->GetLine( szBuffer, sizeof( szBuffer)) < 0)
	{
		CloseConnection();
		DeleteNetObj = FALSE;
		return FALSE;
	}


	if(strncmp(szBuffer,". OK", 4)!=0)
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONNECT,strError);
		LOGIN_FLAG=1;
		return FALSE;
	}
	
	return TRUE;
}


int CAcapSettings::AuthenticatePlainAcap(void)
{
	char szBuffer[QC_NETWORK_BUF_SIZE];
	LOGIN_FLAG=0;

	strcpy(szBuffer,". Authenticate \"PLAIN\" \"");
    strcat(szBuffer,m_strUserID);
    strcat(szBuffer," ");
    strcat(szBuffer,m_strPassword);

	strcat(szBuffer,"\"\r\n");

 	
	if(NetConnection->PutDirect( szBuffer) < 0) 		
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);  //Not an accurate error but for now it will do.
		LOGIN_FLAG=0;
		CloseConnection();
		DeleteNetObj = FALSE;
		return FALSE;
	}
	
	memset(szBuffer,0,sizeof(szBuffer));
	
	if(NetConnection->GetLine( szBuffer, sizeof( szBuffer)) < 0)
	{
		CloseConnection();
		DeleteNetObj = FALSE;
		return FALSE;
	}


	if(strncmp(szBuffer,". OK", 4)!=0)
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);
		LOGIN_FLAG=1;
		return FALSE;
	}

	return TRUE;
}



int CAcapSettings::AuthenticateAPOPCRAMAcap(int nType)
{
	char szBuffer[QC_NETWORK_BUF_SIZE];
    char szTempBuffer[1024];
	char szUserinfo[1024];
	char szDigest[1024];
    char* pszCont;

    CString strError;
	int nReturn;
	LOGIN_FLAG=0;

	memset(szDigest,0,sizeof(szDigest));
	
	strcpy(szUserinfo,m_strPassword);

    if(m_Authentication==AUTH_XCRAM)
    {
	    strcpy(szBuffer,". Authenticate \"CRAM-MD5\" \"");
        hmac_md5((unsigned char* )m_szStamp,(unsigned char* )szUserinfo,szDigest);
        
    }

    else if(m_Authentication==AUTH_CRAM)
    {   
        strcpy(szBuffer,". Authenticate \"CRAM-MD5\"\r\n");
		nReturn = NetConnection->PutDirect( szBuffer);
		if(  nReturn < 0) 
		{
			if(nReturn == -2)
				return nReturn;
			CString strError;
			strError.LoadString(IDS_ACAP_ERROR_LOGIN);
			ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);
			LOGIN_FLAG=0;
			CloseConnection();
			DeleteNetObj = FALSE;
			return -1;
		}
	
        memset(szBuffer,0,sizeof(szBuffer));
		nReturn = NetConnection->GetLine( szBuffer, sizeof( szBuffer));
		if(nReturn < 0)
		{
			CloseConnection();
			DeleteNetObj = FALSE;
			return -1;
		}

	    //ReceiveNormal(szBuffer);
    
        if(strchr(szBuffer,'+'))
        {
            pszCont=strchr(szBuffer,'"');
            
            if(pszCont)
            {
                strcpy(szTempBuffer,pszCont+1);
                pszCont=strchr(szTempBuffer,'"');
                if(pszCont)
                {
                    *pszCont=0;
                    strcpy(szBuffer,"\"");
                    strcat(szBuffer,m_strUserID);
                    strcat(szBuffer," ");
                    hmac_md5((unsigned char* )szTempBuffer,(unsigned char* )szUserinfo,szDigest);
                    strcat(szBuffer,szDigest+1);
                    strcat(szBuffer,"\"\r\n");

					if(NetConnection->PutDirect( szBuffer) >= 0) 
					{					
						memset(szBuffer,0,sizeof(szBuffer));
						//ReceiveNormal(szBuffer);

						NetConnection->GetLine(szBuffer, sizeof(szBuffer));

                   		if(strncmp(szBuffer,". OK", 4)!=0)
                		{
							strError.LoadString(IDS_ACAP_ERROR_LOGIN);
							ErrorDialog(IDS_ACAP_ERROR_CONNECT,strError);
							LOGIN_FLAG=1;
							return FALSE;
						}
						
						return TRUE;
					}
					else
					{
						CloseConnection();
						DeleteNetObj = FALSE;
						return -1;
					}
        
                }
            }
        }

        strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONNECT,strError);
		LOGIN_FLAG=1;
		return -1;

    }

    else if (m_Authentication==AUTH_APOP)
    {
        strcpy(szBuffer,". Authenticate \"APOP\" \"");
        GenerateDigest(m_szStamp,szUserinfo,szDigest);
    }

	strcat(szBuffer,m_strUserID);
	strcat(szBuffer," ");
	strcat(szBuffer,szDigest+1);
	strcat(szBuffer,"\"\r\n");
	
	if( NetConnection->PutDirect( szBuffer) < 0) 
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);  //Not an accurate error but for now it will do.
		CloseConnection();
		DeleteNetObj = FALSE;
		LOGIN_FLAG=0;
		return -1;
	}

	memset( szBuffer, 0, sizeof( szBuffer)); 
	
	if(NetConnection->GetLine( szBuffer, sizeof( szBuffer)) < 0) 
	{
		CloseConnection();
		DeleteNetObj = FALSE;
		LOGIN_FLAG = 0;
		return -1;
	}
	
	if(strncmp(szBuffer,". OK", 4)!=0)
	{
	
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONNECT,strError);
		LOGIN_FLAG=1;
		return -1;
	}

	
	return 1;
}

BOOL CAcapSettings::CloseConnection()
{
	//
	// Put up the progress dialog
	//
	if (NetConnection != NULL)
	{
		
		::MainProgress(CRString(IDS_POP_SHUTDOWN));

		//
		// Hide the progress bar
		//
		::Progress(-1, NULL, -1);
		LogoutAcap();
		NetConnection->Close();
		delete NetConnection;
		CloseProgress();
		Connected = FALSE;

	}

	//m_bConnected = FALSE;

	return TRUE;
}
//This does a logout from the acap server.	
int CAcapSettings::LogoutAcap(void)
{
	char szBuffer[1024];
	strcpy(szBuffer,". logout\r\n");
	
	if(NetConnection->PutDirect( szBuffer) < 0) 
	{
		CString strError;
		strError.LoadString(IDS_ACAP_ERROR_LOGIN);
		ErrorDialog(IDS_ACAP_ERROR_CONFIG,strError);  //Not an accurate error but for now it will do.
		LOGIN_FLAG=0;
		return FALSE;

	}
	
	return TRUE;	
}



void CAcapSettings::ReceiveSearch(char FAR* pszBuffer)
{

	char szTempBuffer[2050];
	char szBuffer[QC_NETWORK_BUF_SIZE];

	int nCount;
	int nID = -1;
	
	while(TRUE)
	{
		memset(szBuffer,0,sizeof(szBuffer));
		
		nCount = NetConnection->GetLine( szBuffer, sizeof( szBuffer));
		if (nCount < 0)
		{
			ASSERT(0);
			szBuffer[0] = 0;
			return;
		}

		szBuffer[nCount] = '\0';
		strcpy(szTempBuffer, szBuffer);

		if(strncmp(szBuffer, ". OK", 4) == 0)
			break;

		if(strncmp(szBuffer, ". NO", 4) == 0 ||
			strncmp(szBuffer, ". BAD", 5) == 0)
		{
			ASSERT(0);
			return;
		}
		
		for(int i= strlen(szBuffer) - 1; i >= 0; i--)
		{
			if((szBuffer[i] == '}') || (szBuffer[i] == '{') || (szBuffer[i] == ')')
				|| (szBuffer[i] == ' ') || (szBuffer[i] == '('))
				szBuffer[i] = '\0';
		}

		int temp = strlen(szBuffer);
		if(strstr(_strupr(szTempBuffer), "VENDOR.EUDORA."))
			nID = RetrieveID(szBuffer);
		else if(nID > 0)
		{
			SetFields(nID, szBuffer);
			nID = 0;
		}
	}

	//Remove this once the POP_ACCOUNT field finally disappers
	//ok, reflect the change in the POP_ACCOUNT
	CString strTemp ;

	strTemp = GetIniString( IDS_INI_LOGIN_NAME) ;
	strTemp += "@" ;
	strTemp += GetIniString( IDS_INI_POP_SERVER) ;
	
	SetIniString( IDS_INI_POP_ACCOUNT, strTemp) ;
	
	return;
}


//Parse the file we just received from the server.
void CAcapSettings::SetFields(int nID, char * strSetting)
{
	CString strTempSetting;
	
	short nSetting = 0;
	
	if(nID > 0)
	{
		//Some ID's don't exist in Windows but in Mac.  Need to do double checking for those..
		switch(nID)
		{
			case IDS_INI_REAL_NAME: 
				{
					long outLen;
					char outStr[255];
					long inLen = strlen(strSetting);
					
					
					UTF8To88591(strSetting, inLen, outStr, &outLen);
					outStr[outLen] = NULL;

					SetIniString(nID, outStr);


				}
				break;
			case IDS_FIRST_UNREAD:
				SetIniShort(IDS_INI_FIRST_UNREAD_NORMAL, (_stricmp(strSetting, "normal") == 0));
				SetIniShort(IDS_INI_FIRST_UNREAD_STATUS, (_stricmp(strSetting, "status") == 0));
				SetIniShort(IDS_INI_USE_POP_LAST, (_stricmp(strSetting, "poplast")==0));
				break;
	
			case IDS_POP_AUTHENTICATE:
				SetIniShort(IDS_INI_AUTH_APOP, (_stricmp(strSetting, "apop") == 0));
				SetIniShort(IDS_INI_AUTH_RPA, (_stricmp(strSetting, "rpa") == 0));
				SetIniShort(IDS_INI_AUTH_PASS, (_stricmp(strSetting, "password")==0));
				SetIniShort(IDS_INI_AUTH_KERB, (_stricmp(strSetting, "kerberos")==0));
				break;

			case IDS_SEND_FORMAT:
				SetIniShort(IDS_INI_SEND_MIME, ((_stricmp(strSetting, "applesingle") == 0) ||
												 (_stricmp(strSetting, "appledouble") == 0)));

				SetIniShort(IDS_INI_SEND_BINHEX, (_stricmp(strSetting, "binhex") == 0));
				SetIniShort(IDS_INI_SEND_UUENCODE, (_stricmp(strSetting, "uuencode")==0));
			
				break;
			
			case IDS_INI_ACCESS_METHOD:
				SetIniShort(IDS_INI_USES_POP, (_stricmp(strSetting, "pop") == 0));
				SetIniShort(IDS_INI_USES_IMAP, (_stricmp(strSetting, "imap") == 0));
				break;

			case IDS_INI_DS_DEFAULT:
				SetIniShort(IDS_INI_FINGER_DEFAULT_LOOKUP, (_stricmp(strSetting, "finger") == 0));
				SetIniShort(IDS_INI_LDAP_DEFAULT_LOOKUP, (_stricmp(strSetting, "ldap") == 0));
				SetIniShort(IDS_INI_PH_DEFAULT_LOOKUP, (_stricmp(strSetting, "ph") == 0));
				break;
			case IDS_INI_LEAVE_ON_SERVER_DAYS:
				if(atoi(strSetting) > 0)
					SetIniString(nID, strSetting);
				else
					SetIniShort(IDS_INI_DELETE_MAIL_FROM_SERVER, 0);
				break;

			default:
				//Set the puppy				
				SetIniString(nID, strSetting);
			break;
		}
	}

	if(GetIniShort(IDS_INI_LEAVE_MAIL_ON_SERVER))
	{
		if(GetIniShort(IDS_INI_LEAVE_ON_SERVER_DAYS) > 0)
			SetIniShort(IDS_INI_DELETE_MAIL_FROM_SERVER, 1);
		else
			SetIniShort(IDS_INI_DELETE_MAIL_FROM_SERVER, 0);
	}

		
}

//Kinda a brute force approach, but it makes for a fast search mechanism.
//If you add to this list, please make sure they go in the right order.
//Also, for the unsed letters now (such as G, H, I ..) remove the current m_pPositions->Add((CObject*) 1);
//and replace accordingly
void CAcapSettings::LoadStrings(void)
{
	//A's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_ACAP_PORT));
	m_pStringIDList->AddTail((CObject*)IDS_INI_ACAP_SERVER);
	m_pStringIDList->AddTail((CObject*)IDS_INI_ACAP_USER_ID);
	m_pStringIDList->AddTail((CObject*)IDS_INI_ACCESS_METHOD);
	//B's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_BAD_PASSWORD_STRING));
	m_pStringIDList->AddTail((CObject*)IDS_INI_BIG_MESSAGE_THRESHOLD);
	//C's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_CHECK_FOR_MAIL));
	//D's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_DOMAIN_QUALIFIER));
	m_pStringIDList->AddTail((CObject*)IDS_INI_DS_DEFAULT);
	//E's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_EXTRA_NICKNAME_DIRS)); 
	//F's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_FINGER_PORT));
	m_pStringIDList->AddTail((CObject*)IDS_INI_FINGER_SERVER);
	m_pStringIDList->AddTail((CObject*)IDS_FIRST_UNREAD);
	//G's
	m_pPositions->Add((CObject*) 1);
	//H's
	m_pPositions->Add((CObject*) 1);
	//I's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_IMAP_PORT));
	//J's
	m_pPositions->Add((CObject*) 1);

	//K's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_KERBEROS_SET_USERNAME));
	//L's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_LDAP_PORT));
	m_pStringIDList->AddTail((CObject*)IDS_INI_LDAP_SERVER);
	m_pStringIDList->AddTail((CObject*)IDS_INI_LEAVE_MAIL_ON_SERVER);
	m_pStringIDList->AddTail((CObject*)IDS_INI_LEAVE_ON_SERVER_DAYS);
	//m_pStringIDList->AddTail((CObject*)IDS_INI_LOGIN_NAME);  //maps to MailName
	//M's
	m_pPositions->Add((CObject*) 1);
	//N's
	m_pPositions->Add((CObject*) 1);
	//O's
	m_pPositions->Add((CObject*) 1);
	//P's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_PASS_CHNG_PORT));
	m_pStringIDList->AddTail((CObject*)IDS_INI_PH_PORT);
	m_pStringIDList->AddTail((CObject*)IDS_INI_PH_RETURN);
	m_pStringIDList->AddTail((CObject*)IDS_INI_PH_SERVER);
	//m_pStringIDList->AddTail((CObject*)IDS_INI_POP_ACCOUNT); //3.x clients only
	m_pStringIDList->AddTail((CObject*)IDS_POP_AUTHENTICATE);
	m_pStringIDList->AddTail((CObject*)IDS_INI_POP_PORT);
	//m_pStringIDList->AddTail((CObject*)IDS_INI_POP_SERVER); //should map to MailServer
	//Q's
	m_pPositions->Add((CObject*) 1);
	//R's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_REAL_NAME));
	m_pStringIDList->AddTail((CObject*)IDS_INI_RETURN_ADDRESS);

	//S's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_SEND_FORMAT));
	m_pStringIDList->AddTail((CObject*)IDS_INI_SMTP_PORT);
	m_pStringIDList->AddTail((CObject*)IDS_INI_SMTP_SERVER);
	//T's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_USER_TABOO_HEADERS)); 
	//U's
	m_pPositions->Add((CObject*)m_pStringIDList->AddTail((CObject*)IDS_INI_USE_ACAP_SERVER));
	m_pStringIDList->AddTail((CObject*)IDS_INI_USE_POP_SEND);
	m_pStringIDList->AddTail((CObject*)IDS_INI_USE_QP); 
	//V's
	m_pPositions->Add((CObject*)1);
	
}

//Retrieves the ID from the list m_pPositions given the setting name e.g UseAcapServer
int CAcapSettings::RetrieveID(CString strSetting)
{
   	CString strCompare;
	int nID;
	POSITION startPos, endPos;

	
	CString strTemp = "Vendor.Eudora.";
	CString strStrippedSetting = strSetting.Mid(strTemp.GetLength(), (strSetting.GetLength() - strTemp.GetLength()));
	strStrippedSetting.MakeLower();

	TCHAR chTemp = strStrippedSetting.GetAt(0);
	//Find out the first letter so we know where to start from
	int nPos = chTemp - 97;

	ASSERT(nPos >= 0);
		

	// HACK!!! Two items are not alphabatized...
	// MailServer maps to IDS_INI_POP_SERVER
	// MailName maps to IDS_INI_LOGIN_NAME
	// Do a quick check for either

	if(_stricmp("MailServer", strStrippedSetting) == 0)
		return IDS_INI_POP_SERVER;
	if(_stricmp("MailName", strStrippedSetting) == 0)
		return IDS_INI_LOGIN_NAME;
	

	//Let's just deal with the section in the list we need to (alphabatized)
	//Array m_pPositions at position nPos contains the start POSITION in the list of ID's
	startPos = (POSITION)m_pPositions->GetAt(nPos);
	endPos = (POSITION)m_pPositions->GetAt(nPos+1);

	while(startPos && endPos && (startPos != endPos))
	{
		//if startPos is one this means we are dealing with an entry which we do not support
		if(startPos == (POSITION)1)
			break;
		nID = (int)m_pStringIDList->GetNext(startPos);
		strCompare.LoadString(nID);
		strCompare = strCompare.SpanExcluding("\n");
		if(_stricmp(strCompare, strStrippedSetting) == 0)
			return nID;
	}
	return -1;
} 


/*

	
    1.IDS_INI_REAL_NAME       "RealName"
    2.IDS_INI_RETURN_ADDRESS  "ReturnAddress"
	3.IDS_INI_USE_ACAP_SERVER "UseACAPServer\n0"
    4.IDS_INI_ACAP_SERVER     "ACAPServer"
    5.IDS_INI_ACAP_USER_ID    "ACAPUserID"
    6.IDS_INI_ACAP_PASSWORD   "ACAPPassword"
	7.IDS_INI_POP_ACCOUNT     "POPAccount"
	8.IDS_INI_POP_SERVER      "PopServer"
	9.IDS_INI_SMTP_SERVER     "SMTPServer"
    10.IDS_INI_PH_SERVER       "PhServer"
	11.IDS_INI_FINGER_SERVER   "FingerServer"
	12.IDS_INI_FINGER_DEFAULT_LOOKUP "FingerDefault\n0"
    
	17.IDS_INI_CHECK_FOR_MAIL  "CheckForMailEvery\n0"
    18.IDS_INI_BIG_MESSAGE_THRESHOLD "BigMessageThreshold\n40960"

	18.IDS_INI_KERBEROS_SET_USERNAME "KerberosSetUsername\n0" ***new

	19.IDS_INI_LEAVE_MAIL_ON_SERVER "LeaveMailOnServer\n0"
	20.IDS_INI_LEAVE_ON_SERVER_DAYS "LeaveOnServerDays\n0"
	21.IDS_INI_DELETE_MAIL_FROM_SERVER "DeleteMailFromServer\n0"
	22.IDS_INI_AUTH_PASS       "AuthenticatePassword\n1"
	23.IDS_INI_AUTH_APOP       "AuthenticateAPOP\n0"
	24.IDS_INI_BAD_PASSWORD_STRING "BadPasswordString"
	25.IDS_INI_DOMAIN_QUALIFIER "DomainQualifier"
	26.IDS_INI_SEND_MIME       "SendMIME\n1"
    27.IDS_INI_SEND_BINHEX     "SendBinHex\n0"
	
	29.IDS_INI_FIRST_UNREAD_NORMAL "FirstUnreadNormal\n1"
	30.IDS_INI_USE_POP_LAST    "UsePOPLast\n0"
	31.IDS_INI_FIRST_UNREAD_STATUS "FirstUnreadStatus\n0"
	
	33.IDS_INI_POP_PORT        "POPPort\n110"
	34.IDS_INI_SMTP_PORT       "SMTPPort\n25"
    35.IDS_INI_FINGER_PORT     "FingerPort\n79"
    36.IDS_INI_PH_PORT         "PHPort\n105"
	37.IDS_INI_PH_RETURN       "PhReturn"
  	38.IDS_INI_USER_TABOO_HEADERS  "TabooHeaders   ****new
    
    40.IDS_INI_USE_POP_SEND    "UsePOPSend\n0"
    41.IDS_INI_EXTRA_HEADERS   "ExtraHeaders"

	42.IDS_INI_USE_QP          "UseQP\n1"  ***new


  12.IDS_INI_EXTRA_NICKNAME_DIRS "ExtraNicknameDirs" ***new

	
	  
    
    
    
											 */
