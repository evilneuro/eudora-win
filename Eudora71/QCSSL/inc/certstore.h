// certstore.h
//
// Header file for CertificateStore class.
//

#include "cert.h"
#include <windows.h> 
#include <stdio.h>
#include "qcthread.h"
#include <WinCrypt.h>
#include "QCSSLInt.h"

struct CertData;
struct QCSSLReference;
class CertificateStore
{
public:
    CertificateStore(HCERTSTORE hStore = NULL);
    
	BOOL				 Open(LPCSTR lpszStoreProvider,                
							  DWORD dwMsgAndCertEncodingType,          
							  HCRYPTPROV hCryptProv,                   
							  DWORD dwFlags,                           
							  const void *pvPara);			
    BOOL				 Close(DWORD dwFlags = CERT_CLOSE_STORE_FORCE_FLAG);          
	
	CertificateStore	 Duplicate();

	bool				 CreateEmptyStore();
	BOOL				 LoadFromFile(const char* filename);
	bool				 SaveStore();

	BOOL				 AddCertificateToStore(Certificate *pCertificate);
	BOOL				 DeleteCertificateFromStore(CertData *pCertData);
	CERT_CONTEXT		*FindCertificate(CertData *pCertData);
	unsigned int		 GetNumberOfCerts();

    unsigned long		 AddCertificatesToContext(SSL_CTX *pSSLCtx);
	unsigned long		 AddCertificatesToList(CPtrList* pCertDataList);

	HCERTSTORE			 m_hStore;

private:
    QCMutex				 m_Mutex;
	bool				 m_bNeedsSave;
	CString				 m_strFilename;
};