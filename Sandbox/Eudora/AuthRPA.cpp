// AuthRPA.cpp
//
// RPA authentication

#include "stdafx.h"

#include "resource.h"
#include "Base64.h"
#include "QP.h"

#include "rs.h"  //Only for CRString

//#include "guiutils.h"
#include "AuthRPA.h"

//THINGS TODO
//Get rid of Decode64 adn encode64 and NetConnection related stuff




//////////////////////////////////////////////////////////////////////
// Beginning of Remote-Passphrase Authentication via SSPI  
//////////////////////////////////////////////////////////////////////

QCAuthRPA::QCAuthRPA(Network *NetStream, Callback1<const char *> err)
	: m_NetStream(NetStream), ReportError(err)
{ 
	ASSERT(m_NetStream); 
	securityDLL = NULL; 
	m_bValidCred = false;
	memset(&credentials, 0, sizeof(credentials));
	memset(&context, 0, sizeof(context));
}
	

void QCAuthRPA::ErrorDialog(UINT StringID, ...)
{
	char buf[1024] = {0};
		
	va_list argList;
	va_start(argList, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), argList);
	va_end(argList);

	ReportError(buf);
}




int QCAuthRPA::LoadRPALibrary()
{

	// Find out which security DLL to use, depending on 
    // whether we are on NT or Win95
    CString strError;
    

	versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

    if (!GetVersionEx (&versionInfo))
	{
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, "");
		return (-1);
	}

    if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	    lstrcpy (securityDllName, CRString(IDS_POP_NT_SECURITY_DLL) ); 
    else if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	    lstrcpy (securityDllName, CRString(IDS_POP_95_SECURITY_DLL) ); 
	else
	{
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, "");
		return (-1);
	}

    // Load and initialize the security DLL

	securityDLL = LoadLibrary (securityDllName);

    if (NULL == securityDLL)  
	{   
		strError.LoadString(8207);
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, strError);
		return (-1);
	}

    // Locate InitSecurityInterface function.

	initFunctionTable = (INIT_SECURITY_INTERFACE) GetProcAddress (
		    securityDLL, SECURITY_ENTRYPOINT);

	if (NULL == initFunctionTable)  
	{                                         
		strError.LoadString(8205);
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, strError);
		if (securityDLL != 0)
			FreeLibrary (securityDLL);
		securityDLL = NULL;
		return (-1);
	}

	// Get the function table for SSPI functions.

	securityFuncs = initFunctionTable ();

	if (NULL == securityFuncs)  
	{                                                 
		strError.LoadString(8205);
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, strError);
		if (securityDLL != 0)
			FreeLibrary (securityDLL);
		securityDLL = NULL;
		return (-1);
	}
    return (0);
}

//static TimeStamp        lifetime;
//static CredHandle       credentials;

int QCAuthRPA::GetCredentialHandle()
{
	SECURITY_STATUS         securityStatus;
	// Get out credential handle. 

	securityStatus = securityFuncs->AcquireCredentialsHandle (
                    NULL,   // principal
                    (char *)(LPCSTR)CRString(IDS_POP_RPA),
                    SECPKG_CRED_OUTBOUND,
                    NULL,   // LOGON id
                    NULL,   // auth data
                    NULL,   // get key fn
                    NULL,   // get key arg
                    &credentials,
                    &lifetime);

	if (securityStatus != SEC_E_OK)
	{                             
		CString strError;                    
		strError.LoadString(8205);
		ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, strError);
		return (-1);
	}
	m_bValidCred = true;
    return (0);
}

//static CtxtHandle              context;


int QCAuthRPA::Authenticate(CString strPOPAccount)
{
	// This is a copy of the sample provided by Compuserve, butwe added the server interaction.
	// The Do loop could be written in a better way by doing the sending to the server first and then
	// retrieving the response from the server .
	TimeStamp               lifetime;
//	char*                   serverName = "server.domain";
	CString serverName;
	//CString strPOPAccount(GetIniString(IDS_INI_POP_ACCOUNT));
	ULONG                   contextRequirements = ISC_REQ_MUTUAL_AUTH;
	ULONG                   contextAttributes;
	LPBYTE                   inputBuffer = NULL;
	LPBYTE                   outputBuffer = NULL;
	SECURITY_STATUS         securityStatus;
	SecBufferDesc           outBuffDesc;
	SecBuffer               outSecBuff;
	SecBufferDesc           inBuffDesc;
	SecBuffer               inSecBuff;
	BOOL                    firstTime = TRUE;
	DWORD                   maxTokenSize;
	PSecPkgInfo             pkgInfo;

	BOOL					bError = FALSE;
    
	int index = strPOPAccount.ReverseFind('@');
	index++;
	serverName = strPOPAccount.Mid(index);

    // Query for the package we're interested in for max 
	// token size. This will control how big our buffers are.

    securityStatus = securityFuncs->QuerySecurityPackageInfo (
	        (char*)(LPCSTR)CRString(IDS_POP_RPA), &pkgInfo);

    if (securityStatus != SEC_E_OK)
	{                                                 
		CString strError;
		strError.LoadString(8205);
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, strError);
		return (-1);
	}

    maxTokenSize = pkgInfo->cbMaxToken;
	securityFuncs->FreeContextBuffer (pkgInfo);

    // Allocate input buffers and output buffers for sending 
	// and receiving tokens.

    inputBuffer = new BYTE [maxTokenSize];
	outputBuffer = new BYTE [maxTokenSize];

    // Initialize the input and output buffer descriptors to refer 
	// to the input and output security buffers. 

    inBuffDesc.ulVersion = 0;
	inBuffDesc.cBuffers = 1;
    inBuffDesc.pBuffers = &inSecBuff;

	outBuffDesc.ulVersion = 0;
    outBuffDesc.cBuffers = 1;
	outBuffDesc.pBuffers = &outSecBuff;

	char* pErrorMsg = new char [maxTokenSize];
	do 
	{
	    // Initialize the input buffer. The first time we make the 
        // call, we don't have a token from the server since the 
		// client initiates the authentication. If it's not the first 
	    // time, then read the next token from the server.

		inSecBuff.BufferType = SECBUFFER_TOKEN;
		inSecBuff.pvBuffer = inputBuffer;

		if (firstTime)
	    {
			inSecBuff.cbBuffer = 0;
		}
		else
	    {
			// INSERT CODE: Read token from server and store in 
			// inputBuffer. Set inSecBuffer.cbBuffer = to the number 
		    // of bytes read. 
			Base64Decoder TheDecoder;
			LONG OutLen = 0;
			char* pOutBuff = new char [maxTokenSize];
			char* pTemp = pOutBuff;
			m_NetStream->GetLine(pOutBuff, maxTokenSize);
			
			strcpy(pErrorMsg, pOutBuff);

			if (strncmp(pOutBuff, "+ ", 2) == 0)
				pTemp = pOutBuff + 2;
			else if (strncmp(pOutBuff, "+", 1) == 0)
				pTemp = pOutBuff + 1;

			strncat(pTemp, "===", 4 - (strlen(pTemp) % 4));

			TheDecoder.Init();
			TheDecoder.Decode(pTemp, strlen(pTemp), (char *)inSecBuff.pvBuffer, (LONG &)inSecBuff.cbBuffer);
			delete [] pOutBuff;
		}
    
	    // Initialize the output buffer. IntializeSecurityContext 
		// will store the token to send to the server in the output 
		// buffer.

	    outSecBuff.cbBuffer = maxTokenSize;
		outSecBuff.BufferType = SECBUFFER_TOKEN;
		outSecBuff.pvBuffer = outputBuffer;

	    // Perform the next step in the authentication process.

		securityStatus = securityFuncs->InitializeSecurityContext (
                         &credentials,
                         firstTime ? NULL : &context,
                         (char *)(LPCSTR)serverName, 
                         contextRequirements,  
                         0,
                         SECURITY_NATIVE_DREP,
                         &inBuffDesc,
                         0,
                         &context,
                         &outBuffDesc,
                         &contextAttributes,
                         &lifetime);

		// Check to make sure everything we have a successful return.
	    // Also, "complete" the token if necessary.

		if (securityStatus == SEC_I_COMPLETE_NEEDED ||
			securityStatus == SEC_I_COMPLETE_AND_CONTINUE)
		{
			SECURITY_STATUS completeStatus;
			completeStatus = securityFuncs->CompleteAuthToken (
				    &context, &outBuffDesc);

			if (completeStatus != SEC_E_OK)
			{
				bError = TRUE;
				strcpy(pErrorMsg, (LPCSTR)CRString(8205));
				break;
			}
		}
		else if (securityStatus != SEC_E_OK && 
		         securityStatus != SEC_I_CONTINUE_NEEDED)  
	    {
			strcpy(pErrorMsg, (LPCSTR)CRString(8205));
			break;
		}

		firstTime = FALSE;

	    // Send token to the server if we need to.

		if (outSecBuff.cbBuffer > 0)
		{
			// INSERT CODE: Send outSecBuffer.cbBuffer number of bytes
			// stored in outputBuffer to the server.
			Base64Encoder TheEncoder;
			LONG OutLen = 0;
			char* pTmp;
			char* pOutBuff = new char [maxTokenSize];
			char* OutBuff = pOutBuff;
			unsigned long int i=0;
			for (i=0; i < maxTokenSize; i++)
			{
				pOutBuff[i] = '\0';
			}

			TheEncoder.Init(pOutBuff, OutLen, 0);
			pOutBuff += OutLen;
			TheEncoder.Encode((char*)outSecBuff.pvBuffer, outSecBuff.cbBuffer, pOutBuff, OutLen);
			pOutBuff += OutLen;
			TheEncoder.Done(pOutBuff, OutLen);
			pTmp = OutBuff;
			while ((pTmp = strchr(pTmp, '\r')) != 0)
				strcpy(pTmp, (pTmp+2));
			strcat(OutBuff, "\r\n");
			m_NetStream->PutDirect(OutBuff);
			delete [] OutBuff;
		}
	}
	while (securityStatus == SEC_I_CONTINUE_NEEDED ||
			securityStatus == SEC_I_COMPLETE_AND_CONTINUE);
    
    if(!bError)
    {            //DONT DO THIS IF WE HAVE AN ERROR
		//Do this check for the server response to the command "YAEA" (in Base64)that was last sent.
		char* pInBase64buff = new char [maxTokenSize];
		m_NetStream->GetLine(pInBase64buff, maxTokenSize);
		if (strncmp(pInBase64buff, "+OK", 3) != 0)
		{
			bError = TRUE;
			strcpy(pErrorMsg, pInBase64buff);
		}
		delete [] pInBase64buff;
	}

	if (securityStatus != SEC_E_OK || bError)
	{
	    ErrorDialog(IDS_SEC_E_INTERNAL_ERROR, pErrorMsg);
		delete [] inputBuffer;
		delete [] outputBuffer;
		delete [] pErrorMsg;

		// Cleanup any partially-created security context.

		if (! firstTime)
			securityFuncs->DeleteSecurityContext (&context);
		return (-1);
	}

	delete [] pErrorMsg;
	delete [] inputBuffer;
	delete [] outputBuffer;
	return (0);
}

void QCAuthRPA::Cleanup()
{
	if (securityDLL)
	{
		securityFuncs->DeleteSecurityContext (&context);
		if(m_bValidCred)
			securityFuncs->FreeCredentialHandle (&credentials);
		FreeLibrary (securityDLL);
	}
}

//////////////////////////////////////////////////////////////////////
// End of Remote-Passphrase Authentication via SSPI  
//////////////////////////////////////////////////////////////////////

