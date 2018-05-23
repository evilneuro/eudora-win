// qccertificate.h
//
// Header file for SSL certificate handling class.
//

#ifndef _QCCERTIFICATE_H_
#define _QCCERTIFICATE_H_

#include "QCSSL.h"
#include "QCSSLInt.h"

class CertificateStore;
struct QCSSLInitInfo;
struct CertData;
struct QCSSLReference;

class QCCertificateUtils
{
public:
	static int CertificateCallback(int iOK, X509_STORE_CTX *pCtx);
	static int CertIsInStore(X509 *pX509, CertificateStore *pStore);
	static bool PatternMatchHostName(char * szCertsName, char* szGivenName);
	static bool ExtractCertInfo(X509 *pX509, CertData *pNewCertData);
	static int CheckCertificateName(char *serverName, QCSSLReference *pSSLReference);
};

#endif
