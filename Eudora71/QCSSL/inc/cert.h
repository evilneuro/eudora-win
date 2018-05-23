
#ifndef _CERT_H_
#define _CERT_H_
#include <afx.h>


#include <windows.h> 
#include <stdio.h>
#include "qcthread.h"
#include <WinCrypt.h>

#include <afxdisp.h>

class Certificate
{
public: 
 CERT_CONTEXT *m_pContext;

CERT_CONTEXT * GetContext(){ return m_pContext;}


 Certificate(CERT_CONTEXT *pContext = NULL){ m_pContext = pContext;}
 BOOL CertCreateCertificateContext(
			const BYTE *pbCertEncoded,               
			DWORD cbCertEncoded                      
			);
 PCCERT_CONTEXT CertDuplicateCertificateContext();
 
 BOOL CertFreeCertificateContext();
 
 BOOL CertSerializeCertificateStoreElement(
  DWORD dwFlags,                  
  BYTE *pbElement,                
  DWORD *pcbElement               
 );
 BOOL LoadFromFile(const char *pFilename);
 
 ~Certificate(){ CertFreeCertificateContext();}

 CString GetSerialNumber();
 CString GetSignatureAlgorithm();
 CString GetIssuer();
 CString GetSubject();
 COleDateTime GetValidFrom();
 COleDateTime GetValidTo();
 LONG GetPublicKeyLength();
 CString GetThumbPrint(unsigned int algoID);
 CString GetSubjectCN();

 char m_Buffer[1000];
};

#endif