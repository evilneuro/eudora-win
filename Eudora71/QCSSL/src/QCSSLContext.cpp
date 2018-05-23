// QCSSLContext.cpp
//
// Functions that handle the actual SSL work, calling into the appropriate SSL library.
//
// Copyright (c) 2000-2004 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include<afx.h>

// Define CRTDBG_MAP_ALLOC and include the files in the correct order
// to allow leak checking with malloc.
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "QCSSLContext.h"
#include "qcssl.h"
#include "qccertificate.h"
#include "qcthread.h"
#include "certstore.h"
#include "sslconnectionmanager.h"
#include "sslutils.h"
#include "resource.h"

#include "OpenSSL\err.h"

#include "DebugNewHelpers.h"


#pragma warning( disable : 4100 4786)  


const char *szRootCertFileName = "rootcerts.p7b";
const char *szUserCertFileName = "usercerts.p7b";

ConnectionInfoMap	 g_cimConnectionInfoMap;

QCMutex				 g_Mutex;				// Mutex

CertificateStore	 g_cerstoreRootStore;	// Store for root certs.
CertificateStore	 g_certstoreUserStore;	// Store for user-trusted certs.
CPtrList			 g_RootCertList;		// List of root certs (used by Cert Info Manager)
CPtrList			 g_UserCertList;		// List of user-trusted certs (used by Cert Info Manager)


//
// BIO Input/Ouput object for OpenSSL.
//

//	Given an error, decide if it's an error that we should retry on.
int BIO_ws_non_fatal_error(int iErr)
{
	ASSERT(iErr <= 0);
	// printf ( "BIO_ot_non_fatal_error: %d\n", err );

#if 0
	switch (iErr)
	{
		default:
			// printf ( "BIO_ot_non_fatal_error returning 0\n" );
			return 0;		// fatal errors
	}
#endif
	
	// printf ( "BIO_ot_non_fatal_error returning 0 (2)\n" );
	return 0;
}

int BIO_ws_should_retry(int iErr)
{
	ASSERT(iErr <= 0);

	//	printf ( "BIO_ws_should_retry: %d\n", err );

	return BIO_ws_non_fatal_error(iErr);
}

static int ws_new(BIO *pBIO)
{
	if (!pBIO)
	{
		ASSERT(0);
		return 0;
	}

	pBIO->init = 0;
	pBIO->num = 0;
	pBIO->ptr = NULL;
	pBIO->flags = 0;

	return 1;
}

static int ws_free(BIO *pBIO)
{
	if (!pBIO)
	{
		return 0;
	}

	if (pBIO->shutdown)
	{
		// Should we close?
		if (pBIO->init)
		{	// Did we init??
			//	Close the socket
			//	SHUTDOWN2(a->num);
		}
		pBIO->init = 0;		// no longer opened
		pBIO->flags = 0;
	}

	return 1;
}

static int ws_read(BIO *pBIO, char *szIn, int iInLen)
{
	int		 iRet = 0;

	ASSERT(iInLen > 0);
	if (iInLen <= 0)
	{
		return 0;
	}

	if (szIn)
	{
		QCSSLReference		*pSSLReference = (QCSSLReference*)pBIO->num;
		if (!pSSLReference)
		{
			return 0;
		}

		iRet = (int)pSSLReference->m_fnQCSSLReadCallback(NULL, 0, szIn, iInLen, (long)pSSLReference, 0);

		if (iRet < iInLen)
		{
			iRet = -1;
		}

		//if ( iErr > 0 )
		//	print_data ( out, iErr );

		BIO_clear_retry_flags(pBIO);

		if (iRet <= 0)
		{
			// we didn't read any bytes
			if (BIO_ws_should_retry(iRet))
			{
				BIO_set_retry_read(pBIO);
				iRet = -1;	// retry me!
			}
			else
			{
				iRet = 0;	//	general purpose error
			}
		}
	}

	return iRet;
}

static int ws_write(BIO *pBIO, const char *szOut, int iOutLen)
{
	int		 iRet = 0;
	
	ASSERT(szOut);
	ASSERT(iOutLen > 0);

	if (!szOut || (iOutLen <= 0))
	{
		return 0;
	}

	QCSSLReference		*pSSLReference = (QCSSLReference*)pBIO->num;
	if (!pSSLReference)
	{
		return 0;
	}

	iRet = (int)pSSLReference->m_fnQCSSLWriteCallback(NULL, 0, szOut, iOutLen, (long)pSSLReference, 0);
	if (iRet < iOutLen)
	{
		iRet = -1;
	}

	//if ( err > 0 )
	//	print_data ( in, err );

	BIO_clear_retry_flags(pBIO);
	if (iRet < 0)
	{
		// we didn't write any bytes
		if (BIO_ws_should_retry(iRet))
		{
			BIO_set_retry_write(pBIO);
			iRet = -1;	// retry me!
		}
		else
		{
			iRet = 0;	//	general purpose error
		}
	}
	
	return iRet;
}

static long ws_ctrl(BIO *pBIO, int iCmd, long lNum, void *ptr)
{
	long	 lRet = 1;

	// printf ( "ot_ctrl: %d\n", cmd );
	switch(iCmd)
	{
		case BIO_CTRL_RESET:
			lNum = 0;
			// fall into seek code
		case BIO_C_FILE_SEEK:
			return 0;
			
		case BIO_C_FILE_TELL:
		case BIO_CTRL_INFO:
			return 0;

		case BIO_C_SET_FD:
			// Set the endpoint for the BIO
			ws_free(pBIO);				//	Close whatever's already there
			pBIO->num = *((int*)ptr);	//	Set the new endpoint
			pBIO->shutdown = (int)lNum;	//	Set the close mode
			pBIO->init = 1;				//	We've been initialized
			break;

		case BIO_C_GET_FD:
			// Return the endpoint for the BIO
			if (pBIO->init)
			{
				if (ptr)
				{
					*((int*)ptr) = lRet = pBIO->num;
				}
			}
			else
			{
				lRet = -1;
			}
			break;

		case BIO_CTRL_GET_CLOSE:
			// Get the close flag
			lRet = pBIO->shutdown;
			break;

		case BIO_CTRL_SET_CLOSE:
			// Set the close flag
			pBIO->shutdown = (int)lNum;
			break;

		case BIO_CTRL_PENDING:
		case BIO_CTRL_WPENDING:
			lRet = 0;
			break;

		case BIO_CTRL_DUP:
		case BIO_CTRL_FLUSH:
			lRet = 1;
			break;

		default:
			lRet = 0;
			break;
	}

	return(lRet);
}

static int ws_puts(BIO *pBIO, const char *szStr)
{
	ASSERT(szStr != NULL);
	return ws_write(pBIO, szStr, strlen(szStr));
}

#define BIO_TYPE_WORKERSOCKET		(25|0x0400|0x0100)

BIO_METHOD *BIO_s_workersocket()
{
	static BIO_METHOD methods_ws =
	{
		BIO_TYPE_WORKERSOCKET,
		"workersocket_in_Eudora",
		ws_write,
		ws_read,
		ws_puts,
		NULL, /* sock_gets, */
		ws_ctrl,
		ws_new,
		ws_free,
		NULL,
	};
	return &methods_ws;
}

BIO *BIO_new_ws(int iWorkerSocket, int iCloseFlag)
{
	BIO		*pBIO = BIO_new(BIO_s_workersocket());
	if (pBIO)
	{
		BIO_set_fd(pBIO, iWorkerSocket, iCloseFlag);
	}
	return pBIO;
}


//
//	Helper functions
//


//
//	FileExists()
//
//	Returns true if the specified file exists, false otherwise.  This is used to determine where the
//	cert files reside.
//
bool FileExists(const char* fileName)
{
	CFileStatus		 unused;

	if (CFile::GetStatus(fileName, unused) == 0)
	{
		return false;
	}

	return true;
}

//
//	SetupConnectionInfo()
//
//	Set up the ConnectionInfo object.
//
ConnectionInfo *SetupConnectionInfo(QCSSLReference *pSSLReference)
{
	if (!pSSLReference)
	{
		return NULL;
	}

	if (pSSLReference->m_ProtocolInfo.m_ProtocolName.IsEmpty())
	{
		pSSLReference->m_ProtocolInfo.m_ProtocolName = "unknown";
	}

	CString			 strFullyUniqueName = pSSLReference->m_Persona + "\n" + pSSLReference->m_ProtocolInfo.m_ProtocolName;
	ConnectionInfo	*pConnectionInfo = g_cimConnectionInfoMap.AddConnectionInfo(strFullyUniqueName);
	
	ASSERT(pConnectionInfo);
	
	pSSLReference->m_pConnectionManagerInfo = pConnectionInfo;

	return pConnectionInfo;
}

//
//	ChooseCertFileDirectory()
//
//	The m_InputCertsDir member field of the QCSSLSettings class consists of the following:
//	EudoraDir + "\r\n" + ExecutableDir.  We must decide which of these two directories to
//	use for the certificate files.  This decision is made by determining if the specified
//	file exists, first in EudoraDir, then in ExecutableDir.  If a file exists in either of
//	these directories, that is the one we use.  If the file doesn't exist in either
//	directory we default to EudoraDir.
//
//	Eudora ships with root and user certificate files in ExecutableDir.  The above functionality
//	allows the user to effectively ignore these files by placing their own copies in EudoraDir.
//
void ChooseCertFileDirectory(CString &strDirectory, const CString &strFile)
{
	CString		 strDir1;
	CString		 strDir2;
	int			 nPos = strDirectory.Find("\r\n");
	
	// Extract the directory data preceding the "\r\n" and append a '\' if necessary.
	strDir1 = strDirectory.Left(nPos);
	if (strDir1.GetAt(strDir1.GetLength() - 1) != '\\')
	{
		strDir1 += "\\";
	}

	// Extract the directory data following the "\r\n" and append a '\' if necessary.
	strDir2 = strDirectory.Mid(nPos + 2);
	if (strDir2.GetAt(strDir2.GetLength() - 1) != '\\')
	{
		strDir2 += "\\";
	}
	
	// Default to the first directory (EudoraDir).
	strDirectory = strDir1;

	if (FileExists(strDir1 + strFile))
	{
		// File exists in EudoraDir.
		strDirectory = strDir1;
	}
	else if (FileExists(strDir2 + strFile))
	{
		// File exists in ExecutableDir.
		strDirectory = strDir2;
	}
}

//
//	ChooseCertFileDirectories()
//
//	Choose the directories for the root and user cert files which can reside either in EudoraDir or
//	ExecutableDir.
//
//	After removing defunct code to set the data fields m_ClientCertFile, m_ClientCertInfoFile,
//	m_ClientCertsDir and m_ClientCertsHeirarchyDir for pReference->m_CertificateInfo (which aren't
//	currently used anywhere) there isn't much left of this function.  It is tempting to remove this
//	function altogether but it may serve a purpose in the future if we try to do more with the
//	various data directories.
//
void ChooseCertFileDirectories(QCSSLReference *pSSLReference)
{
	if (pSSLReference)
	{
		ChooseCertFileDirectory(pSSLReference->m_CertificateInfo.m_RootCertStoreDir, szRootCertFileName);
		ChooseCertFileDirectory(pSSLReference->m_CertificateInfo.m_UserCertStoreDir, szUserCertFileName);
	}
}

//
//	SetupCertificates()
//
//	Create the root and user cert stores, add the certs to the SSL_CTX's store and add the certs to the
//	lists used by the Certificate Information Manager.
//
bool SetupCertificates(SSL_CTX *pSSLCtx, QCSSLReference *pSSLReference)
{
	static bool	 bCertListCreated = false;
	int			 iCertsLoaded = 0;

	if (!pSSLCtx || !pSSLReference)
	{
		return false;
	}

	// Set up the certificate file directories.
	ChooseCertFileDirectories(pSSLReference);

	// Load the certs.
	g_cerstoreRootStore.LoadFromFile(pSSLReference->m_CertificateInfo.m_RootCertStoreDir + szRootCertFileName);
	iCertsLoaded += g_cerstoreRootStore.AddCertificatesToContext(pSSLCtx);

	g_certstoreUserStore.LoadFromFile(pSSLReference->m_CertificateInfo.m_UserCertStoreDir + szUserCertFileName);
	// For reasons yet to be explained, adding user-trusted certs to the context doesn't seem to actually
	// do anything, so don't bother doing it.  See QCCertificateUtils::CertificateCallback() for how we
	// handle user certs under OpenSSL.
	// iCertsLoaded += g_certstoreUserStore.AddCertificatesToContext(pSSLCtx);

	// If we haven't yet created the cert lists (for use by Cert Info Manager) and we have loaded any certs,
	// add them to the appropriate list.
	if (!bCertListCreated && (iCertsLoaded > 0))
	{
		bCertListCreated = true;
		g_cerstoreRootStore.AddCertificatesToList(&g_RootCertList);
		g_certstoreUserStore.AddCertificatesToList(&g_UserCertList);
	}

	return (iCertsLoaded > 0);
}

//
//	SetCipherSuites()
//
//	Set the list of cipher suites to offer to the server.
//
//	Note: With OpenSSL this call is optional because not specifying any cipher suites will result in
//	the default cipher suite list to be offered.  The below code has the same effect as not calling
//	this function at all.  If at some point we want to change the list of cipher suites then we would
//	need to change the below list and be sure to call this function.
//
int SetCipherSuites(SSL_CTX *pSSLCtx)
{
	if (pSSLCtx != NULL)
	{
		const char		*szCiphers = "DHE-RSA-AES256-SHA:DHE-DSS-AES256-SHA:AES256-SHA:"
									 "EDH-RSA-DES-CBC3-SHA:EDH-DSS-DES-CBC3-SHA:DES-CBC3-SHA:DES-CBC3-MD5:"
									 "DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA:AES128-SHA:"
									 "IDEA-CBC-SHA:IDEA-CBC-MD5:RC2-CBC-MD5:DHE-DSS-RC4-SHA:"
									 "RC4-SHA:RC4-MD5:RC4-MD5:RC4-64-MD5:"
									 "EXP1024-DHE-DSS-DES-CBC-SHA:EXP1024-DES-CBC-SHA:EXP1024-RC2-CBC-MD5:"
									 "EDH-RSA-DES-CBC-SHA:EDH-DSS-DES-CBC-SHA:DES-CBC-SHA:DES-CBC-MD5:"
									 "EXP1024-DHE-DSS-RC4-SHA:EXP1024-RC4-SHA:EXP1024-RC4-MD5:"
									 "EXP-EDH-RSA-DES-CBC-SHA:EXP-EDH-DSS-DES-CBC-SHA:"
									 "EXP-DES-CBC-SHA:EXP-RC2-CBC-MD5:EXP-RC2-CBC-MD5:EXP-RC4-MD5:EXP-RC4-MD5";
		return SSL_CTX_set_cipher_list(pSSLCtx, szCiphers);
	}
	return 0;
}

//
//	SetSSLVersion()
//
//	Set the SSL version to use for negotiation.
//
SSL_CTX *SetSSLVersion(QCSSLReference *pSSLReference)
{
	SSL_METHOD		*sslmethod = NULL;

	if (!pSSLReference)
	{
		ASSERT(0);
		return NULL;
	}

	switch (pSSLReference->m_ProtocolInfo.m_ProtocolVersion)
	{
	case 0:
		sslmethod = SSLv23_method();
		break;
	case 1:
		sslmethod = SSLv23_method();
		break;
	case 2:
		sslmethod = SSLv3_method();
		break;
	case 3:
		sslmethod = TLSv1_method();
		break;
	case 4:
		sslmethod = SSLv23_method();
		break;
	case 5:
		sslmethod = SSLv2_method();
		break;
	case 6:
		sslmethod = SSLv23_method();
		break;
	case 7:
		sslmethod = SSLv23_method();
		break;
	default:
		ConnectionInfo *pConnectionInfo = (ConnectionInfo*) pSSLReference->m_pConnectionManagerInfo;
		pConnectionInfo->m_Outcome.AddComments(CResString(IDS_ERR_VERSIONINVALID));
		break;
	}
	return SSL_CTX_new(sslmethod);
}

//
//	FillInConnectionInfo()
//
//	Fill in the data for the connection info object after the handshake has completed.
//
void FillInConnectionInfo(ConnectionInfo *pConnectionInfo, QCSSLReference *pSSLReference, int iErrorCode)
{
	if (!pConnectionInfo || !pSSLReference || !pSSLReference->m_pSSL)
	{
		ASSERT(0);
		return;
	}

	SSL			*pSSL = (SSL*)(pSSLReference->m_pSSL);

	pConnectionInfo->m_Outcome.m_bCertRejected = false;

	// If there was a cert error, extract the cert as text so we can later present the cert to the user
	// in a dialog to ask if they want to trust it.
	if ((pConnectionInfo->m_Outcome.m_ErrorCode >= IDS_UNKNOWNERROR) &&
		(pConnectionInfo->m_Outcome.m_ErrorCode <= IDS_CERT_SUCCESSFULPATTERNNOTALLOW))
	{
		pConnectionInfo->m_Outcome.m_strCertText = "";
		pConnectionInfo->m_Outcome.m_pCertData = NULL;

		BIO			*pBIOMem = BIO_new(BIO_s_mem());
		if (pBIOMem)
		{
			CertData	*pCertData = (CertData*)(pConnectionInfo->m_CertDataList.GetHead());
			if (pCertData)
			{
				pConnectionInfo->m_Outcome.m_pCertData = pCertData;
				unsigned char	*pcCertBlob = (unsigned char *)malloc(pCertData->m_CertBlobLength);
				unsigned char	*pcCertBlobStart = pcCertBlob;
				if (pcCertBlob)
				{
					memcpy(pcCertBlob, pCertData->m_CertBlob, pCertData->m_CertBlobLength);

					X509		*pX509 = NULL;
					pX509 = d2i_X509(NULL,
									 (unsigned char **)(&pcCertBlob),
									 pCertData->m_CertBlobLength);

					if (pX509)
					{
						X509_print(pBIOMem, pX509);
						char		pcLine[256];
						while (BIO_gets(pBIOMem, pcLine, sizeof(pcLine)))
						{
							pConnectionInfo->m_Outcome.m_strCertText += pcLine;
						}
						X509_free(pX509);
						pX509 = NULL;
					}
					free(pcCertBlobStart);
					pcCertBlobStart = NULL;
					pcCertBlob = NULL;
				}
			}
			BIO_free(pBIOMem);
			pBIOMem = NULL;
		}

		pConnectionInfo->m_Outcome.m_bCertRejected = true;
		pConnectionInfo->m_Outcome.m_strCertText.Replace("\n", "\r\n");
		pConnectionInfo->m_Outcome.m_strCertRejection = CResString(pConnectionInfo->m_Outcome.m_ErrorCode);
	}

	pConnectionInfo->m_Version = SSL_get_version(pSSL);

	if (pConnectionInfo->m_Outcome.m_ErrorCode == 0)
	{
		pConnectionInfo->m_Outcome.m_ErrorCode = iErrorCode;
	}
	if (iErrorCode == SSL_ERROR_NONE)
	{
		pConnectionInfo->m_Outcome.m_Outcome = SSLOutcome::SSLSUCCEEDED;
	}
	else
	{
		pConnectionInfo->m_Outcome.m_Outcome = SSLOutcome::SSLFAILED;
		switch(iErrorCode)
		{
			case SSL_ERROR_ZERO_RETURN:
			case SSL_ERROR_SSL:
			case SSL_ERROR_SYSCALL:
				pConnectionInfo->m_Outcome.AddErrors(CResString(IDS_ERR_CONNECTIONLOST));
				break;
			// At some point we should provide meaningful error messages for other errors as well.
		}
	}

	pConnectionInfo->m_Time = COleDateTime::GetCurrentTime();
	pConnectionInfo->m_IPAddress = pSSLReference->m_ProtocolInfo.m_IPAddress;
	pConnectionInfo->m_ServerName = pSSLReference->m_ProtocolInfo.m_ServerName ;
	pConnectionInfo->m_Port = pSSLReference->m_ProtocolInfo.m_Port ;

	SSL_CIPHER		*pSSLCipher = SSL_get_current_cipher(pSSL);
	if (pSSLCipher)
	{
		pConnectionInfo->m_CipherName = SSL_get_cipher(pSSL);

		int iBits = 0;
		pConnectionInfo->m_CipherKeyBits = SSL_CIPHER_get_bits(pSSLCipher, &iBits);

		char		 szBuf[128];
		CString		 strCipherDesc = SSL_CIPHER_description(pSSLCipher, szBuf, 128);
	}


#if 0
	GetKeyExchangeMethod(pConnectionInfo->m_CipherId, pConnectionInfo->m_CipherKeyExchangeMethod);
	GetHashName(pConnectionInfo->m_CipherId, pConnectionInfo->m_HashName);
#endif

    if ((iErrorCode != SSL_ERROR_NONE) &&
		(pConnectionInfo->m_Version.Compare("SSLv2") == 0) &&
		(pSSLReference->m_ProtocolInfo.m_AllowSSL_2_0 != 1))
	{
		pConnectionInfo->m_Outcome.AddErrors(IDS_ERR_SSL2NOTOK);
		pConnectionInfo->m_Outcome.m_Outcome = SSLOutcome::SSLREJECTED;
	}

#if 0
	LONG keyLength = pConnectionInfo->GetPublicKeyLength();
	if (keyLength < pSSLReference->m_ProtocolInfo.m_MinKeyExchangeLength)
	{
		CString csError;
		csError.Format(CResString(IDS_ERR_MINEXCHNONO), pSSLReference->m_ProtocolInfo.m_MinKeyExchangeLength, keyLength);
		pConnectionInfo->m_Outcome.AddErrors(csError);
		pConnectionInfo->m_Outcome.m_Outcome = SSLOutcome::SSLREJECTED;
	}
#endif
	
#if 0
	// No easy way to determine this under SSL Plus 4.X.
	if (pConnectionInfo->m_CipherKeyBits < pSSLReference->m_ProtocolInfo.m_MinEncryption)
	{
		CString csError;
		csError.Format(CResString(IDS_ERR_MINENCRYPTIONNONO), pSSLReference->m_ProtocolInfo.m_MinEncryption, pConnectionInfo->m_CipherKeyBits);
		pConnectionInfo->m_Outcome.AddErrors(csError);
		pConnectionInfo->m_Outcome.m_Outcome = SSLOutcome::SSLREJECTED;
	}
#endif
}

//
//	BeginQCSSLSession()
//
//	Create the necessary objects and perform the SSL handshake.
//
bool BeginQCSSLSession(QCSSLReference *pSSLReference)
{
	g_Mutex.Lock();

	int			 iRet = 0;

	// Set up the Connection Info object.
	ConnectionInfo	*pConnectionInfo = SetupConnectionInfo(pSSLReference);

	// Set up the SSL library.
	SSL_load_error_strings();
	SSL_library_init();

	// Set the SSL version negotiation values.
	SSL_CTX		*pSSLCtx = SetSSLVersion(pSSLReference);
	if (!pSSLCtx)
	{
		ASSERT(0);
		g_Mutex.Unlock();
		return false;
	}

	// Set up the cipher suites.
	SetCipherSuites(pSSLCtx);

	// Set up the certificates.
	SetupCertificates(pSSLCtx, pSSLReference);

	// Set the certificate verification callback.
	SSL_CTX_set_verify(pSSLCtx, SSL_VERIFY_PEER, QCCertificateUtils::CertificateCallback);

	// Create the SSL object.
	SSL		*pSSL = NULL;
	pSSL = SSL_new(pSSLCtx);
	if (!pSSL)
	{
		SSL_CTX_free(pSSLCtx);
		g_Mutex.Unlock();
		return false;
	}

	// Create the BIO for reading and writing.
	BIO		*pBIO = BIO_new_ws((int)pSSLReference, BIO_NOCLOSE);
	if (!pBIO)
	{
		SSL_free(pSSL);
		g_Mutex.Unlock();
		return false;
	}
	SSL_set_bio(pSSL, pBIO, pBIO);

	// Specify client (connect) state.
	SSL_set_connect_state(pSSL);

	// The certificate verification callback needs two pieces of data: the QCSSLReference object and
	// the user store.  Set these objects as extra data for the cert store so they can be retrieved
	// inside the callback.
	X509_STORE			*pX509Store = pSSL->ctx->cert_store;
	if (pX509Store)
	{
		CRYPTO_set_ex_data(&(pX509Store->ex_data), 0, pSSLReference);
		CRYPTO_set_ex_data(&(pX509Store->ex_data), 1, &g_certstoreUserStore);
	}

	// Now that everything is created, set the parameters passed to us.
	pSSLReference->m_pSSL = pSSL;

	// Continually call SSL_do_handshake() until handshake succeeds or experiences fatal failure.
	bool		 bContinue = true;
	bool		 bSuccess = false;
	int			 iErr = SSL_ERROR_NONE;
	while(!bSuccess && bContinue)
	{
		iRet = SSL_do_handshake(pSSL);
		if (iRet == 1)
		{
			// Handshake succeeded.
			bSuccess = true;
		}
		else if (iRet <= 0)
		{
			// Handshake experienced temporary or fatal error.
			iErr = SSL_get_error(pSSL, iRet);
			switch (iErr)
			{
				case SSL_ERROR_NONE:
					// Shouldn't happen on iRet < 0.
					ASSERT(0);
					bContinue = false;
					break;
				case SSL_ERROR_ZERO_RETURN:
				case SSL_ERROR_SSL:
				case SSL_ERROR_SYSCALL:
					// Fatal error, stop trying.
					bContinue = false;
					break;
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_X509_LOOKUP:
				case SSL_ERROR_WANT_CONNECT:
				case SSL_ERROR_WANT_ACCEPT:
					// Temporary failure, call SSL_do_handshake() again.
					break;
			}
		}
	}

	g_Mutex.Unlock();

	// In the case of a handshake failure, iErr will be one of the following:
	// SSL_ERROR_ZERO_RETURN, SSL_ERROR_SSL, SSL_ERROR_SYSCALL.
	// What distinguishes the cert failure cases?

	// If we ended on a name mismatch, add that error string here.
	if (pConnectionInfo->m_Outcome.m_bCheckedName && !pConnectionInfo->m_Outcome.m_bNameMatch)
	{
		pConnectionInfo->m_Outcome.AddErrors(IDS_CERTERR_HOSTMISMATCH);
		pConnectionInfo->m_Outcome.AddErrors(IDS_CERTWARN_IGNOREBCOSTRUSTED);
	}

	// Set the connection data for the ConnectionInfo object.
	FillInConnectionInfo(pConnectionInfo, pSSLReference, iErr);

	if (!bSuccess)
	{
		return false;
	}

	return true;
}


bool EndQCSSLSession(void *pSSL)
{
	g_Mutex.Lock();
	if (pSSL)
	{
		// It appears that the ERR_STATE structure is leaking.  It seems all we should need to do is
		// call ERR_remove_state() but that does not seem to do the trick.  There is no external
		// interface to allow us to access this object directly (ERR_get_state() is local to err.c),
		// otherwise the following code should do the free manually:
		// ERR_STATE	*pErrState = ERR_get_state();
		// if (pErrState)
		// {
		//	ERR_STATE_free(pErrState);
		// }
		// There is a note in the OpenSSL "CHANGES" file about a leak that appears to be essentially the
		// same as this one.  The note suggests that they have fixed the leak so either I am missing
		// something or they are.  For now I am giving up on try to fix this last leak.  In the meantime,
		// for each SSL session Eudora will leak 482 bytes which isn't the end of the world. -dwiggins
		ERR_remove_state(0);

		SSL_CTX		*pSSLCtx = (SSL_CTX*)SSL_get_SSL_CTX((SSL*)pSSL);

		SSL_free((SSL*)pSSL);

		if (pSSLCtx)
		{
			SSL_CTX_free(pSSLCtx);
		}
	}
	g_Mutex.Unlock();

	return true;
}

bool CleanQCSSLDll()
{
	g_Mutex.Lock();
//	g_certstoreUserStore.SaveStore(g_UserCertDir + szUserCertFileName);

	POSITION pos;	

	pos = g_UserCertList.GetHeadPosition();
	while (pos != NULL)
	{
		CertData *pCertData = (CertData*)g_UserCertList.GetNext(pos);
		delete pCertData;
	}
	g_UserCertList.RemoveAll();

	pos = g_RootCertList.GetHeadPosition();
	while (pos != NULL)
	{
		CertData *pCertData = (CertData*)g_RootCertList.GetNext(pos);
		delete pCertData;
	}
	g_RootCertList.RemoveAll();

	g_certstoreUserStore.Close();
	g_cerstoreRootStore.Close();

	// Clean up OpenSSL objects.
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
	EVP_cleanup();

	g_Mutex.Unlock();
	return TRUE;
}

int WriteSSLData(void *pData, int *piLen, void *pSSL)
{
	*piLen = SSL_write((SSL*)pSSL, pData, *piLen);
	return 0;
}

int ReadSSLData(void *pSSL, int *piLen, void *pData)
{
	*piLen = SSL_read((SSL*)pSSL, pData, *piLen);
	return 0;
}

ConnectionInfo*  GetConnectionInfo(const char * person, const char *protocol)
{

	CString uniquename = CString(person) + CString("\n");
	uniquename += protocol;
	return g_cimConnectionInfoMap.GetConnectionInfo(uniquename);
}

#pragma warning(default : 4189)

//Ww will eventually use CSingleLock
bool AddTrustedCertFromFile(const char* filename)
{
	bool bRet = false;
	g_Mutex.Lock();
	CString fileName(filename);
	int extpos = fileName.ReverseFind('.');
	if(extpos == -1)
	{ 
		g_Mutex.Unlock(); return false;
	}
	CString ext = fileName.Right(fileName.GetLength() - extpos);
	Certificate *pCertificate = DEBUG_NEW Certificate;
	if(pCertificate->LoadFromFile(filename))
	{
		CertData *pNewCertData = DEBUG_NEW CertData();
		pNewCertData->SetCertDataValues(pCertificate);
		bRet = AddTrustedUserCert(pNewCertData, "dontcare", "dontcare");
	}
	delete pCertificate;
	g_Mutex.Unlock();
	return bRet;
}

//	No one's currently looking the error returned from SSLAddTrustedCertificate,
//	but it could be useful when debugging.
//	Ignore the warning for "local variable is initialized but not referenced".
#pragma warning(disable : 4189)
bool AddTrustedUserCert(CertData *pCertData, const char *person, const char *proto)
{
	if(!pCertData)
		return false;

	g_Mutex.Lock();
	unsigned char *pData = pCertData->m_CertBlob;
	unsigned long len = pCertData->m_CertBlobLength;

	Certificate cert;
	cert.CertCreateCertificateContext(pData, len);
	bool bSuccess = g_certstoreUserStore.AddCertificateToStore(&cert) != 0;

	if(bSuccess)
	{
		g_certstoreUserStore.SaveStore();

		CertData *pNewCertData = DEBUG_NEW CertData();
		g_UserCertList.AddTail(pNewCertData);
		pNewCertData->SetCertDataValues(&cert);
	}
	g_Mutex.Unlock();
	return false;
}

bool DeleteTrustedUserCert(CertData *pCertData, const char* person, const char* proto)
{
  	if(!pCertData)
		return false;
	g_Mutex.Lock();

	g_certstoreUserStore.DeleteCertificateFromStore(pCertData);
	g_certstoreUserStore.SaveStore();

	POSITION pos = g_UserCertList.Find(pCertData);
	if(pos != NULL)
	{
		g_UserCertList.RemoveAt(pos);
		delete pCertData;
	}

	g_Mutex.Unlock();
	return false;
}

CPtrList* GetRootCertList()
{
	return &g_RootCertList; 
}

CPtrList* GetUserCertList()
{
	return &g_UserCertList;
}
