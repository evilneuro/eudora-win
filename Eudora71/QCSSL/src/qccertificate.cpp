// qccertificate.cpp
//
// Implementation file for SSL certificate handling class.
//

#include "stdafx.h"

// Define CRTDBG_MAP_ALLOC and include the files in the correct order
// to allow leak checking with malloc.
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <string.h>
#include<afx.h>
#include "qccertificate.h"
#include "qcssl.h"
#include "cert.h"
#include "certstore.h"
#include "regexp.h"
#include "resource.h"

#include "DebugNewHelpers.h"


//
//	QCCertificateUtils::CertificateCallback()
//
//	Certificate handling callback.
//
int QCCertificateUtils::CertificateCallback(int iOK, X509_STORE_CTX *pX509StoreCtx)
{
	bool			 bInStore = false;
	long			 lErrors = 0;

	if (!pX509StoreCtx)
	{
		return 0;
	}

	// Get the QCSSLReference and ConnectionInfo objects from the cert store's extra data.
	QCSSLReference *pSSLReference = (QCSSLReference*)CRYPTO_get_ex_data(&(pX509StoreCtx->ctx->ex_data), 0);
	ConnectionInfo* pInfo = NULL;
	if (pSSLReference)
	{
		pInfo = (ConnectionInfo*)pSSLReference->m_pConnectionManagerInfo;
	}
	if (!pInfo)
	{
		return 0;
	}

	// Get the user store object from the cert store's extra data.
	CertificateStore	*pUserStore = (CertificateStore*)CRYPTO_get_ex_data(&(pX509StoreCtx->ctx->ex_data), 1);

	// Bail now if there is no cert.
	if (!pX509StoreCtx->current_cert)
	{
		pInfo->m_Outcome.AddErrors(IDS_CERTERR_NOCERT);
		return 0;
	}

	// If cert is not OK see if the user has added it to their trusted list.
	if (iOK == 0)
	{
		bInStore = (CertIsInStore(pX509StoreCtx->current_cert, pUserStore) == 1);
		if (bInStore)
		{
			iOK = 1;
			lErrors = 0;
		}
	}

	// If cert is still not OK (i.e., if the above code did not find the cert in the list of
	// certs the user said was OK) process it through our normal means.
	if (iOK == 0)
	{
		lErrors = pX509StoreCtx->error;
		switch(pX509StoreCtx->error)
		{
			case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
				if (pInfo->m_Outcome.m_ErrorCode == 0)
				{
					pInfo->m_Outcome.m_ErrorCode = IDS_CERTERR_CHAINNOTTRUSTED;
				}
				pInfo->m_Outcome.AddErrors(IDS_CERTERR_CHAINNOTTRUSTED);
				if (bInStore) pInfo->m_Outcome.AddErrors(IDS_CERTWARN_IGNOREBCOSTRUSTED);
				break;
			case X509_V_ERR_CERT_NOT_YET_VALID:
			case X509_V_ERR_CERT_HAS_EXPIRED:
			case X509_V_ERR_CRL_NOT_YET_VALID:
			case X509_V_ERR_CRL_HAS_EXPIRED:
				if (pInfo->m_Outcome.m_ErrorCode == 0)
				{
					pInfo->m_Outcome.m_ErrorCode = IDS_CERTERR_ONECERTEXPIRED;
				}
				pInfo->m_Outcome.AddErrors(IDS_CERTERR_ONECERTEXPIRED);
				if (bInStore) pInfo->m_Outcome.AddErrors(IDS_CERTWARN_IGNOREBCOSTRUSTED);
				break;
			case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
				if (pInfo->m_Outcome.m_ErrorCode == 0)
				{
					pInfo->m_Outcome.m_ErrorCode = IDS_CERTERR_UNKNOWNROOT;
				}
				pInfo->m_Outcome.AddErrors(IDS_CERTERR_UNKNOWNROOT);
				if (bInStore) pInfo->m_Outcome.AddErrors(IDS_CERTWARN_IGNOREBCOSTRUSTED);
				break;
			case X509_V_ERR_CERT_UNTRUSTED:
			case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
				iOK = 1;
				break;
			case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			case X509_V_ERR_UNABLE_TO_GET_CRL:
			case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
				if (pInfo->m_Outcome.m_ErrorCode == 0)
				{
					pInfo->m_Outcome.m_ErrorCode = IDS_CERTERR_DATANOTFOUND;
				}
				pInfo->m_Outcome.AddErrors(IDS_CERTERR_DATANOTFOUND);
				break;
			case X509_V_ERR_CERT_SIGNATURE_FAILURE:
			case X509_V_ERR_CRL_SIGNATURE_FAILURE:
			case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
			case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
			case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
			case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
			case X509_V_ERR_OUT_OF_MEM:
			case X509_V_ERR_CERT_CHAIN_TOO_LONG:
			case X509_V_ERR_CERT_REJECTED:
			case X509_V_ERR_CERT_REVOKED:
			case X509_V_ERR_INVALID_CA:
			case X509_V_ERR_PATH_LENGTH_EXCEEDED:
			case X509_V_ERR_INVALID_PURPOSE:
			case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
			case X509_V_ERR_AKID_SKID_MISMATCH:
			case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
			case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
			case X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER:
			case X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION:
			case X509_V_ERR_KEYUSAGE_NO_CRL_SIGN:
			case X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION:
				if (pInfo->m_Outcome.m_ErrorCode == 0)
				{
					pInfo->m_Outcome.m_ErrorCode = IDS_UNKNOWNERROR;
				}
				pInfo->m_Outcome.AddErrors(IDS_UNKNOWNERROR);
				if (bInStore) pInfo->m_Outcome.AddErrors(IDS_CERTWARN_IGNOREBCOSTRUSTED);
				break;
		}
	}

	// Store the error in the cert store's extra data.
	X509_STORE_CTX_set_ex_data(pX509StoreCtx, 0, (void*)lErrors);

	// Create a new CertData object using the data from the current cert.  This object is used
	// by the Certificate Information Manager.
	CertData		*pNewCertData = DEBUG_NEW CertData();
	pInfo->m_CertDataList.AddTail(pNewCertData);
	pNewCertData->m_bTrusted = (iOK == 1);
	ExtractCertInfo(pX509StoreCtx->current_cert, pNewCertData);

	// If we haven't yet matched the certificate name, test it now.  If the server returns a chain of
	// certificates we only need to name match against one.
	if (!pInfo->m_Outcome.m_bNameMatch)
	{
		pInfo->m_Outcome.m_bCheckedName = true;
		pInfo->m_Outcome.m_bNameMatch = (CheckCertificateName(pNewCertData->m_CommonName.GetBuffer(0), pSSLReference) == 0);
	}

	return iOK;
}

//
//	CertIsInStore()
//
//	Manually compare the specified cert against the certs in the specified store, returning 1
//	if a match is found, 0 otherwise.
//
int QCCertificateUtils::CertIsInStore(X509 *pX509, CertificateStore *pStore)
{
	if (!pX509 || !pStore || !pStore->m_hStore)
	{
		return 0;
	}

	CERT_CONTEXT	*pPrevContext = NULL;
	CERT_CONTEXT	*pContext = NULL;
	unsigned char	*pbCertEncoded = NULL;
	unsigned char	*pbCertEncodedStart = NULL;
	do
	{
		pContext = (CERT_CONTEXT*)::CertEnumCertificatesInStore(pStore->m_hStore, pPrevContext);
		if(pContext)
		{
			// The function d2i_X509() (which is used to convert a cert from DER format (which is what
			// we use to store the certs) into PEM format (which OpenSSL prefers)) modifies the data in
			// place so we need to pass d2i_x509() a copy of the cert data, not the actual data.
			pbCertEncoded = (unsigned char*)malloc(pContext->cbCertEncoded);
			if (pbCertEncoded)
			{
				pbCertEncodedStart = pbCertEncoded;
				memcpy(pbCertEncoded, pContext->pbCertEncoded, pContext->cbCertEncoded);

				X509	*pX509Tmp = d2i_X509(NULL, (unsigned char **)(&pbCertEncoded), pContext->cbCertEncoded);
				if (pX509Tmp)
				{
					if (X509_cmp(pX509, pX509Tmp) == 0)
					{
						X509_free(pX509Tmp);
						free(pbCertEncodedStart);
						return 1;
					}
					X509_free(pX509Tmp);
				}
				free(pbCertEncodedStart);
				pbCertEncodedStart = NULL;
				pbCertEncoded = NULL;
			}
			pPrevContext = pContext;
		}
	} while(pContext != NULL);

	return 0;
}

//
//	QCCertificateUtils::PatternMatchHostName()
//
//	Standard code for calling mozilla routines for host mismatch.
//
bool QCCertificateUtils::PatternMatchHostName(char * szCertsName, char* szGivenName)
{
	bool bReturn = false;
	if(!szGivenName || !*szGivenName || !szCertsName || !*szCertsName)
		return false;
	strlwr(szCertsName);
	strlwr(szGivenName);
	
	const char *szGivenRelativeName;		
	if((szGivenRelativeName = strchr(szGivenName, '.')) == NULL)
	{
		char *szDomainPart = strchr(szCertsName, ',');
		if(szDomainPart) 
			*szDomainPart = '\0';
	}
	
#ifndef NO_MOZILLA_REGEXP
	int nIsRegexp = PORT_RegExpValid(szCertsName);
	if(!nIsRegexp) 
	{
#endif
		if(stricmp(szCertsName, szGivenName) == 0) 
		{
			bReturn = true;
		}
		else if(szGivenRelativeName && stricmp(szGivenRelativeName, szCertsName) == 0) 
		{
			bReturn = true;
		}

#ifndef NO_MOZILLA_REGEXP
	} 
	else 
	{
		if(PORT_RegExpCaseSearch((char *)szGivenName, szCertsName) == 0)
			bReturn = true;		
	}		
#endif

	return(bReturn);
}

//
//	QCCertificateUtils::ExtractCertInfo()
//
//	Extract cert info from pX509 struct into pNewCertData class.
//
bool QCCertificateUtils::ExtractCertInfo(X509 *pX509, CertData *pNewCertData)
{
	if (!pX509 || !pNewCertData)
	{
		return false;
	}

	// Extract the common name from pX509.  It seems to me there should be an OpenSSL call
	// to do this but I cannot locate one so do it manually.
	char		*szBuf = X509_NAME_oneline(X509_get_subject_name(pX509), NULL, 0);
	CString		 strName = szBuf;
	free(szBuf);

	int				 iPos = strName.Find("/CN=", 0);
	if (iPos >= 0)
	{
		strName = strName.Right(strName.GetLength() - iPos - 4);
	}
	iPos = strName.Find("/", 0);
	if (iPos >= 0)
	{
		strName = strName.Left(iPos);
	}

	// Convert the data in pX509 to DER format and use it to create a CertData object.
	int				 iDataSize = 0;
	unsigned char	*pcBuf = NULL;
	unsigned char	*pcCertData = NULL;

	iDataSize = i2d_X509(pX509, NULL);
	pcBuf = (unsigned char*)malloc(iDataSize);
	if (!pcBuf)
	{
		return false;
	}
	pcCertData = pcBuf;
	i2d_X509(pX509, &pcBuf);

	Certificate cer;
	cer.CertCreateCertificateContext((const BYTE *)pcCertData, iDataSize);

	free(pcCertData);
	pcCertData = NULL;

	pNewCertData->SetCertDataValues(&cer);

	pNewCertData->m_CommonName = strName;

	return true;
}

//
//	QCCertificateUtils::CheckCertificateName()
//
//	Compares the server name in the certificate against the server name used to connect.
//
int QCCertificateUtils::CheckCertificateName(char *szCertServer, QCSSLReference *pSSLReference)
{
	if (!szCertServer || !pSSLReference)
	{
		return IDS_CERTERR_DATANOTFOUND;
	}

	bool			 bResult = false;
	const char		*szServer = LPCTSTR(pSSLReference->m_ProtocolInfo.m_ServerName);

	if (PatternMatchHostName(szCertServer, (char*)szServer))
	{
		bResult = true;
		if (!pSSLReference->m_ProtocolInfo.m_AllowRegExp)
		{
			ConnectionInfo* pInfo = (ConnectionInfo*) pSSLReference->m_pConnectionManagerInfo;
			if(pInfo)
			{
				CString csError ;
				csError.Format(CResString(IDS_CERT_SUCCESSFULPATTERNNOTALLOW),
							   (char*)LPCTSTR(szServer),
							   (char*)LPCTSTR(szCertServer));
				pInfo->m_Outcome.AddComments(csError);
				bResult = false;
			}
		}
	}

	if (!bResult)
	{
		return IDS_CERTERR_NAMESNOTEQUAL;
	}

	return 0;
}
