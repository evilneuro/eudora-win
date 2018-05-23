#include "stdafx.h"

#include "cert.h"

#include "DebugNewHelpers.h"

typedef DWORD (WINAPI * FPNCertGetPublicKeyLength)( DWORD dwCertEncodingType,  PCERT_PUBLIC_KEY_INFO pPublicKey);

BOOL Certificate::CertCreateCertificateContext(              
			const BYTE *pbCertEncoded,               
			DWORD cbCertEncoded                      
			)
{

	m_pContext =(_CERT_CONTEXT*) ::CertCreateCertificateContext(
			X509_ASN_ENCODING,                
			pbCertEncoded,               
			cbCertEncoded                      
			);
   return (m_pContext != NULL);
 }



PCCERT_CONTEXT Certificate::CertDuplicateCertificateContext()	 
 {
	return ::CertDuplicateCertificateContext(m_pContext);
 }
 
BOOL Certificate::CertFreeCertificateContext()
 {
	::CertFreeCertificateContext(m_pContext);
	m_pContext = NULL;
	return TRUE;
 }
 
BOOL Certificate::CertSerializeCertificateStoreElement(
   DWORD dwFlags,                  
  BYTE *pbElement,                
  DWORD *pcbElement               
 )
 {
	return ::CertSerializeCertificateStoreElement(m_pContext,
											dwFlags,                  
											pbElement, pcbElement);               
 }


BOOL Certificate::LoadFromFile(const char *pFilename)
{
	CFile file;
	file.Open(pFilename,CFile::modeRead);
	file.SeekToBegin();
	DWORD filelen = static_cast<DWORD>( file.GetLength() );
    char *pBuffer = DEBUG_NEW char[filelen +1];
	file.Read(pBuffer, filelen);
	CertCreateCertificateContext((const unsigned char*)pBuffer, filelen);
	delete [] pBuffer;
    file.Close();
    return m_pContext != NULL;
}


CString Certificate::GetSerialNumber()
{
	if(!m_pContext)
		return "";
	
	DWORD nLen = m_pContext->pCertInfo->SerialNumber.cbData;
	
	char *pSerNo = DEBUG_NEW char[nLen +1];
	memcpy(pSerNo, m_pContext->pCertInfo->SerialNumber.pbData, nLen);
	pSerNo[nLen] ='\0';
	unsigned char t;
	CString csSerial;
	for(DWORD j =0 ; j<nLen; j++)
		{
		t= pSerNo[j];
		char data[4];
		sprintf(data, "%02X", t);
		csSerial = data + csSerial;
		}
	delete [] pSerNo;
	return csSerial;
}

CString Certificate::GetSignatureAlgorithm()
{
	if(!m_pContext)
		return "";
//	::CertNameToStr(X509_ASN_ENCODING, &m_pContext->pCertInfo->SignatureAlgorithm ,CERT_SIMPLE_NAME_STR,m_Buffer, 999); 
	return CString(m_Buffer);
}

CString Certificate::GetIssuer()
{
	if(!m_pContext)
		return "";
	::CertNameToStr(X509_ASN_ENCODING, &m_pContext->pCertInfo->Issuer ,CERT_SIMPLE_NAME_STR,m_Buffer, 999); 
	return CString(m_Buffer);
}

CString Certificate::GetSubject()
{
	if(!m_pContext)
		return "";

	::CertNameToStr(X509_ASN_ENCODING, &m_pContext->pCertInfo->Subject ,CERT_SIMPLE_NAME_STR,m_Buffer, 999); 


	return CString(m_Buffer);
}
 
COleDateTime  Certificate::GetValidFrom()
{
	if(!m_pContext)
		return COleDateTime();

	FILETIME ft = this->m_pContext->pCertInfo->NotBefore;

	return COleDateTime(ft);
}

COleDateTime  Certificate::GetValidTo()
{
	if(!m_pContext)
		return COleDateTime();

	FILETIME ft = this->m_pContext->pCertInfo->NotAfter;

	return COleDateTime(ft);
}


LONG Certificate::GetPublicKeyLength()
{
	if(!m_pContext)
		return -1;
	
	LONG keyLength = -1;

	HMODULE hModule = ::GetModuleHandle("Crypt32.dll");
	if(hModule)
		{
		FPNCertGetPublicKeyLength fpnGetKeyLength = (FPNCertGetPublicKeyLength)::GetProcAddress(hModule, "CertGetPublicKeyLength");
		if(fpnGetKeyLength)
			{
			keyLength =	(LONG)fpnGetKeyLength(X509_ASN_ENCODING, &this->m_pContext->pCertInfo->SubjectPublicKeyInfo);
			}
		}
	return keyLength;
}


CString Certificate::GetThumbPrint(unsigned int algoID)
{
 CString thumbPrint;
 unsigned char *hashValue = NULL;

 BOOL bResult = FALSE;
 DWORD cbComputedHash =0;

 bResult = CryptHashCertificate(0,algoID,0,m_pContext->pbCertEncoded,
										   m_pContext->cbCertEncoded,
										   NULL,
										   &cbComputedHash);
								
 if(!bResult || cbComputedHash<=0)
 {
	return "";
 }

 hashValue = DEBUG_NEW unsigned char[cbComputedHash];
 ASSERT(hashValue);
 bResult = CryptHashCertificate(0,algoID,0,m_pContext->pbCertEncoded,
										   m_pContext->cbCertEncoded,
										   hashValue,
										   &cbComputedHash);
 if(!bResult)
	 cbComputedHash =0;

 for(DWORD j =0 ; j<cbComputedHash; j++)
		{
		unsigned int t= hashValue[j];
		char data[4];
		sprintf(data, "%02X", t);
		thumbPrint += data;
		}
 delete [] hashValue;
 return thumbPrint;
}

CString Certificate::GetSubjectCN()
{
	if(!m_pContext)
		return "";
	//unfortunately, there is no easy way to directly extract the host name in cryptoAPI from
	//a cert unless we have NT 4.0 SP4 +
	//So we have to use RFC 1779 for Distinguished Names representation

	::CertNameToStr(X509_ASN_ENCODING, &m_pContext->pCertInfo->Subject ,CERT_X500_NAME_STR,m_Buffer, 999); 
	
     CString csSubject(m_Buffer);
	 int nStart = csSubject.Find("CN=");
	 if(nStart <0)
		 return "";
	 int nEnd = csSubject.Find(",", nStart);
	
	 if(nEnd == -1)
		 csSubject = csSubject.Mid(nStart+3);
	 else
		 csSubject = csSubject.Mid(nStart+3, nEnd -nStart-3);

	 csSubject.TrimLeft(" ");
	 csSubject.TrimRight(" ");
	 return csSubject;
}


