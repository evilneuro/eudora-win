// QCSSL.cpp
//
// DLL related functions and entry functions for outside DLLs.
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

#include "QCSSL.h"
#include "QCSSLContext.h"
#include "cert.h"
#include "resource.h"

#include "DebugNewHelpers.h"


BOOL APIENTRY DllMain(HANDLE /*hInst*/, DWORD dwReason, LPVOID /*lpReserved*/)
{
    switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;

}

bool APIENTRY QCSSLBeginSession(QCSSLReference *pSSLReference)
{
	return BeginQCSSLSession(pSSLReference);
}

bool APIENTRY QCSSLEndSession(void *pSSL)
{
	return EndQCSSLSession(pSSL);
}

bool APIENTRY QCSSLClean()
{
	return CleanQCSSLDll();
}

int APIENTRY QCSSLWrite(void *pData, int *piLen, void *pSSL)
{
	return WriteSSLData(pData, piLen, pSSL);
}

int APIENTRY QCSSLRead(void *pSSL, int *piLen, void *pData)
{
	return ReadSSLData(pSSL, piLen, pData);
}

ConnectionInfo* APIENTRY QCSSLGetConnectionInfo(const char * person, const char *protocol)												
{
    return GetConnectionInfo(person, protocol);
}

bool APIENTRY QCSSLAddTrustedCertFromFile(const char* filename)
{
	return AddTrustedCertFromFile(filename);
}

CPtrList* APIENTRY QCSSLGetRootCertList()
{
	return GetRootCertList();
}

CPtrList* APIENTRY QCSSLGetUserCertList()
{
	return GetUserCertList();
}

bool APIENTRY QCSSLAddTrustedUserCert(CertData *pData, const char *person, const char *protocol)
{
	return AddTrustedUserCert(pData, person, protocol);
}

bool APIENTRY QCSSLDeleteTrustedUserCert(CertData *pData, const char *person, const char *proto)
{
	return DeleteTrustedUserCert(pData, person, proto);
}


bool CertData::SetCertDataValues(Certificate *pCert)
{
	if (!pCert || !pCert->m_pContext)
	{
		return FALSE;
	}

	SetCertBlob((unsigned char*)pCert->m_pContext->pbCertEncoded, pCert->m_pContext->cbCertEncoded);
	m_Issuer  = pCert->GetIssuer();
	m_Subject = pCert->GetSubject();
	m_SerialNumber = pCert->GetSerialNumber();
	m_ValidFrom = pCert->GetValidFrom();
	m_ValidTo = pCert->GetValidTo();
	m_PublicKeyLength = pCert->GetPublicKeyLength();
	CalculateFingerPrints(pCert);
	return true;
}

bool CertData::CalculateFingerPrints(Certificate *pCert)
{

	if(!pCert)
		return false;

	POSITION pos = m_HashList.GetHeadPosition();
	while(pos != NULL)
		{
		Hash *pHash = (Hash*)m_HashList.GetNext(pos);
		delete pHash;
		}
	m_HashList.RemoveAll();

	Hash *pHash = NULL;


	pHash = DEBUG_NEW Hash(IDS_CALG_MD2, CResString(IDS_CALG_MD2), pCert->GetThumbPrint(CALG_MD2));
    this->m_HashList.AddTail(pHash);

	pHash = DEBUG_NEW Hash(IDS_CALG_MD4, CResString(IDS_CALG_MD4), pCert->GetThumbPrint(CALG_MD4));
    this->m_HashList.AddTail(pHash);

	pHash = DEBUG_NEW Hash(IDS_CALG_MD5, CResString(IDS_CALG_MD5), pCert->GetThumbPrint(CALG_MD5));
    this->m_HashList.AddTail(pHash);

	pHash = DEBUG_NEW Hash(IDS_CALG_SHA, CResString(IDS_CALG_SHA), pCert->GetThumbPrint(CALG_SHA));
    this->m_HashList.AddTail(pHash);

	return true;
}
