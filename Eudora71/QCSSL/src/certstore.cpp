// certstore.cpp
//
// Implementation file for CertificateStore class.
//

#include "stdafx.h"

#include "certstore.h"  
#include "qcssl.h"
#include "ClientCertInfo.h"
#include "resource.h"

// Define CRTDBG_MAP_ALLOC and include the files in the correct order
// to allow leak checking with malloc.
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "DebugNewHelpers.h"

CertificateStore::CertificateStore(HCERTSTORE hStore)
{
	m_Mutex.Lock();
	m_hStore = hStore;
	m_bNeedsSave = false;
	m_Mutex.Unlock();
}

BOOL CertificateStore::Open(LPCSTR lpszStoreProvider,                
							DWORD dwMsgAndCertEncodingType,         
							HCRYPTPROV hCryptProv,                   
							DWORD dwFlags,                           
							const void *pvPara)
{
	m_Mutex.Lock();
	m_hStore =::CertOpenStore(lpszStoreProvider,                
							  dwMsgAndCertEncodingType,         
							  hCryptProv,                   
							  dwFlags,                           
							  pvPara);
	BOOL bSuccess = (m_hStore != NULL);
	m_Mutex.Unlock();
	return bSuccess;
}

				
BOOL CertificateStore::Close(DWORD dwFlags)             
{
	if (m_hStore == NULL)
	{
		return FALSE;
	}

	m_Mutex.Lock();
	BOOL bRetval =::CertCloseStore(m_hStore, dwFlags);
	m_hStore = NULL;
	m_Mutex.Unlock();
	return bRetval;
}
	
CertificateStore CertificateStore::Duplicate()
{
	if (m_hStore == NULL)
	{
		return FALSE;
	}
	m_Mutex.Lock();
	HCERTSTORE hDupStore = ::CertDuplicateStore(m_hStore);
	m_Mutex.Unlock();
	if (hDupStore == NULL)
	{
		return FALSE;
	}
	return CertificateStore(hDupStore);
}

bool CertificateStore::CreateEmptyStore()
{
	return this->Open(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING, NULL, 0, 0) != 0;
}

BOOL CertificateStore::LoadFromFile(const char* filename)
{
	m_strFilename = filename;

	//dont put locks unlocks here else there will be a deadlock
	this->Open(CERT_STORE_PROV_FILENAME_A,
		        X509_ASN_ENCODING,
				NULL,
				0,
				filename);

	return (m_hStore !=NULL);
}

bool CertificateStore::SaveStore()
{
	if ((m_hStore == NULL) || m_strFilename.IsEmpty())
	{
		return false;
	}

	if (!m_bNeedsSave)
	{
		return true;
	}

	bool bSuccess = false;

	m_Mutex.Lock();
	HANDLE hStoreFileHandle = NULL;

	// Attempt to create/load the user cert file.
	hStoreFileHandle = CreateFile(m_strFilename,
								  GENERIC_WRITE,
								  0,
								  NULL,
								  CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);

	if (hStoreFileHandle != INVALID_HANDLE_VALUE)
	{
		// Save the cert store.
		bSuccess = ::CertSaveStore(m_hStore,
								   X509_ASN_ENCODING|PKCS_7_ASN_ENCODING,
								   CERT_STORE_SAVE_AS_PKCS7,
								   CERT_STORE_SAVE_TO_FILE,
								   hStoreFileHandle,
								   0 ) != 0;
		CloseHandle(hStoreFileHandle);
	}
    
	if (!bSuccess)
	{
		// We couldn't write the file: warn the user.
		DWORD		 dwCode = GetLastError();
		CResString	 strCertFileErrorFmt(IDS_ERR_CERTFILE);
		CString		 strCertFileError;
		strCertFileError.Format(strCertFileErrorFmt, dwCode);
		AfxMessageBox(strCertFileError);
	}

	m_Mutex.Unlock();

	return bSuccess;
}

//
//	CertificateStore::AddCertificateToStore()
//
//	Add the specified certificate to this store.
//
//	NOTE: This function does not save changes to the store.
//
BOOL CertificateStore::AddCertificateToStore(Certificate *pCertificate)
{
	m_Mutex.Lock();

	m_bNeedsSave = true;

	if(!m_hStore)
	{
		CreateEmptyStore();
	}
	BOOL bSuccess = CertAddCertificateContextToStore(m_hStore,
													 pCertificate->GetContext(),
													 CERT_STORE_ADD_NEW,
													 NULL);    
	m_Mutex.Unlock();
	return bSuccess;
}

//
//	CertificateStore::DeleteCertificateFromStore()
//
//	Delete the specified cert from this store.
//
//	NOTE: This function does not save changes to the store.
//
BOOL CertificateStore::DeleteCertificateFromStore(CertData *pCertData)
{
	if(pCertData == NULL)
		return FALSE;

	m_bNeedsSave = true;

	m_Mutex.Lock();
	CERT_CONTEXT *pContext = FindCertificate(pCertData);
	BOOL bSuccess = FALSE;
	if(pContext)
	{
		bSuccess = ::CertDeleteCertificateFromStore(pContext);

	}
	m_Mutex.Unlock();
	return bSuccess;
}

//
//	CertificateStore::FindCertificate()
//
//	Return a pointer to the cert in this store which contains the data specified in pCertData.
//
CERT_CONTEXT* CertificateStore::FindCertificate(CertData* pCertData)
{
	if	(!m_hStore)
	{
		return NULL;
	}

	m_Mutex.Lock();

	BOOL			 bCompare = FALSE;
	CERT_CONTEXT	*pContext = NULL;
	CERT_CONTEXT	*pPrevContext = NULL;

	Certificate		 cer;
	cer.CertCreateCertificateContext((const BYTE *)pCertData->m_CertBlob, pCertData->m_CertBlobLength);

	do
	{
		pContext = (CERT_CONTEXT*)::CertEnumCertificatesInStore(m_hStore, pPrevContext);
		if (pContext)
		{
			bCompare = ::CertCompareCertificate(X509_ASN_ENCODING|PKCS_7_ASN_ENCODING,
												pContext->pCertInfo,
												cer.m_pContext->pCertInfo);
			pPrevContext = pContext;
		}
	} while (pContext && !bCompare);

	if (!bCompare)
	{
		pContext = NULL;
	}

	m_Mutex.Unlock();

	return pContext;
}

//
//	CertificateStore::GetNumberOfCerts()
//
//	Get the number of certs in this store.
//
unsigned int CertificateStore::GetNumberOfCerts()
{
	m_Mutex.Lock();

	CERT_CONTEXT	*pContext = NULL;
	CERT_CONTEXT	*pPrevContext = NULL;
	unsigned int	 i = 0;
	do
	{
		pContext = (CERT_CONTEXT*)::CertEnumCertificatesInStore(m_hStore, pPrevContext);
		if (pContext)
		{
			i++;
			pPrevContext = pContext;
		}
	} while(pContext != NULL);
	m_Mutex.Unlock();
	return i;
}

//
//	CertificateStore::AddCertificatesToContext()
//
//	Add the certs from this store into the X509_STORE for the specified SSL_CTX.
//
unsigned long CertificateStore::AddCertificatesToContext(SSL_CTX *pSSLCtx)
{
	if ((pSSLCtx == NULL) || (m_hStore == NULL))
	{
		return 0;
	}

	m_Mutex.Lock();

	X509_STORE		*pX509Store = SSL_CTX_get_cert_store(pSSLCtx);
	if (!pX509Store)
	{
		ASSERT(0);
		return 0;
	}

	int				 iRet = 0;
	int				 iCertsLoaded = 0;
	CERT_CONTEXT	*pPrevContext = NULL;
	CERT_CONTEXT	*pContext = NULL;
	unsigned char	*pbCertEncoded = NULL;
	unsigned char	*pbCertEncodedStart = NULL;
	do
	{
		pContext = (CERT_CONTEXT*)::CertEnumCertificatesInStore(m_hStore, pPrevContext);
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

				X509		*pX509 = NULL;
				pX509 = d2i_X509(NULL,
								 (unsigned char **)(&pbCertEncoded),
								 pContext->cbCertEncoded);

				if (pX509)
				{
					iRet = X509_STORE_add_cert(pX509Store, pX509);
					if (iRet == 1)
					{
						++iCertsLoaded;
					}
					else
					{
						ASSERT(0);
					}
					X509_free(pX509);
				}

				free(pbCertEncodedStart);
				pbCertEncodedStart = NULL;
				pbCertEncoded = NULL;
			}
			pPrevContext = pContext;
		}
	} while(pContext != NULL);

	m_Mutex.Unlock();

	return iCertsLoaded;
}

//
//	CertificateStore::AddCertificatesToList()
//
//	Add the certs from this store to the specified list.
//
unsigned long CertificateStore::AddCertificatesToList(CPtrList* pCertDataList)
{
	if (!pCertDataList || !m_hStore)
	{
		return 0;
	}

	m_Mutex.Lock();

	CERT_CONTEXT		*pContext = NULL;
	CERT_CONTEXT		*pPrevContext = NULL;
	CertData			*pNewCertData = NULL;
	unsigned int		 i = 0;
	do
	{
		pContext = (CERT_CONTEXT*)::CertEnumCertificatesInStore(m_hStore, pPrevContext);
		if (pContext)
		{
			if (pCertDataList!=NULL)
			{
				Certificate cer;
				cer.CertCreateCertificateContext((const BYTE *)pContext->pbCertEncoded, pContext->cbCertEncoded);
				pNewCertData = DEBUG_NEW CertData();
				pCertDataList->AddTail(pNewCertData);
				pNewCertData->SetCertDataValues(&cer);
				i++;
			}
			pPrevContext = pContext;
		}
	} while(pContext != NULL);
	m_Mutex.Unlock();
	return i;	
}
