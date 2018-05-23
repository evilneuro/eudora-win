// QCSSL.h
//
// Definitions of SSL classes, structures and other data for that is publicly available.
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

#ifndef _QCSSL_H_
#define _QCSSL_H_

#include <afxcoll.h>
#include <windows.h>
#include <afxdisp.h>


// Get rid of warning on debug info for identifiers greater than 255 characters long.
// The primary source of this warning is the Standard Template Library that comes with MSVC.
// Some other warnings in the MSVC STL are ignored as well.
#pragma warning (disable: 4663 4244 4018 4146)

class Certificate;

#define MAX_BUF 500

class CResString: public CString
{
public:
	CResString(UINT nID)
	{
		char buffer[MAX_BUF];
		HINSTANCE hInstance = ::GetModuleHandle("qcssl.dll");
		::LoadString(hInstance, nID, buffer, MAX_BUF-1);

		this->Insert(0, buffer);
	}
};


struct Hash
{
	unsigned int m_AlgId;
	CString m_AlgName;
	CString m_HashData;
	Hash(unsigned int algId, const CString& algName, const CString& hashData)
	{
		m_AlgId = algId;
		m_AlgName = algName;
		m_HashData = hashData;
	}
};


struct CertData
{	
	bool m_bTrusted;
	CString m_Subject;
	COleDateTime m_ValidFrom;
	COleDateTime m_ValidTo;
	CString m_Issuer;
	CString m_CertFile;
	CString m_CommonName;
	CString m_SerialNumber;
	LONG m_PublicKeyLength;
	unsigned char *m_CertBlob;
	unsigned long m_CertBlobLength;
	CPtrList m_HashList;

	~CertData()
	{
		if(m_CertBlob)
			delete [] m_CertBlob;
		POSITION pos = m_HashList.GetHeadPosition();
		while(pos != NULL)
		{
			Hash *pHash = (Hash*)m_HashList.GetNext(pos);
			delete pHash;
		}
		m_HashList.RemoveAll();

	}

	bool SetCertBlob(unsigned char * data, unsigned long len)
	{
		delete [] m_CertBlob;
		m_CertBlob = NULL;
		m_CertBlobLength = 0;

		if(len >0 && data != NULL)
		{
			m_CertBlobLength = len ;
			m_CertBlob = new unsigned char[m_CertBlobLength];
			memcpy(m_CertBlob, data,m_CertBlobLength);
			return true;
		}
		return false;
	}

	CertData():m_bTrusted(false), m_CertBlob(NULL),m_CertBlobLength(0), m_PublicKeyLength(-1)
	{
	}

	CertData(const CertData& certdata)
	{
		m_bTrusted = certdata.m_bTrusted;
		m_Subject = certdata.m_Subject ;
		m_ValidFrom = certdata.m_ValidFrom ;
		m_ValidTo = certdata.m_ValidTo ;
		m_CertFile = certdata.m_CertFile ;
		m_Issuer = certdata.m_Issuer ;
		m_SerialNumber = certdata.m_SerialNumber;
		m_PublicKeyLength = certdata.m_PublicKeyLength;
		delete [] m_CertBlob;
		m_CertBlob = NULL;

		m_CertBlobLength = certdata.m_CertBlobLength ;
		m_CertBlob = new unsigned char[m_CertBlobLength];
		memcpy(m_CertBlob, certdata.m_CertBlob,m_CertBlobLength);

		POSITION pos = m_HashList.GetHeadPosition();
		while (pos != NULL)
		{
			Hash *pHash = (Hash*)m_HashList.GetNext(pos);
			delete pHash;
		}
		m_HashList.RemoveAll();
		pos = certdata.m_HashList.GetHeadPosition();
		while(pos != NULL)
		{
			Hash *pHash = (Hash*)certdata.m_HashList.GetNext(pos);
			ASSERT(pHash);
			Hash *newhash = new Hash(*pHash);
			m_HashList.AddTail(newhash);
		}
	}

	bool SetCertDataValues(Certificate *pCert);
	bool CalculateFingerPrints(Certificate *pCert);

    bool DeleteCertFile()// THIS SHOULD NEVER BE CALLED IN THE DESTRUCTOR!!!!!!!!!
	{
		return false;
	}
	
	CertData& operator = (const CertData& certdata)
	{
		m_bTrusted = certdata.m_bTrusted;
		m_Subject = certdata.m_Subject ;
		m_ValidFrom = certdata.m_ValidFrom ;
		m_ValidTo = certdata.m_ValidTo ;
		m_CertFile = certdata.m_CertFile ;
		m_Issuer = certdata.m_Issuer ;
		m_CertFile = certdata.m_CertFile ;
		m_CertBlobLength = certdata.m_CertBlobLength ;
		m_SerialNumber = certdata.m_SerialNumber;
		m_PublicKeyLength = certdata.m_PublicKeyLength;
		delete [] m_CertBlob;
		m_CertBlob = NULL;

		m_CertBlob = new unsigned char[m_CertBlobLength];
		memcpy(m_CertBlob, certdata.m_CertBlob,m_CertBlobLength);
		POSITION pos = m_HashList.GetHeadPosition();
		while (pos != NULL)
		{
			Hash *pHash = (Hash*)m_HashList.GetNext(pos);
			delete pHash;
		}
		m_HashList.RemoveAll();
		pos = certdata.m_HashList.GetHeadPosition();
		while (pos != NULL)
		{
			Hash *pHash = (Hash*)certdata.m_HashList.GetNext(pos);
			ASSERT(pHash);
			Hash *newhash = new Hash(*pHash);
			m_HashList.AddTail(newhash);
		}

		return *this;
	}

	bool HasExpired()
	{
		COleDateTime nowTime = COleDateTime::GetCurrentTime();
		if((m_ValidFrom >nowTime) || (m_ValidTo< nowTime))
			return true;
		else 
			return false;

	}
};



struct SSLOutcome
{
	enum SSLOUTCOME{SSLUNKNOWN, SSLFAILED, SSLSUCCEEDED , SSLREJECTED};
	SSLOUTCOME	 m_Outcome;
	CString		 m_Errors;
	CString		 m_Comments;
	bool		 m_bCertRejected;
	CString		 m_strCertText;
	CertData	*m_pCertData;
	long		 m_ErrorCode;
	bool		 m_bNameMatch;
	bool		 m_bCheckedName;
	CString		 m_strCertRejection;

	void AddComments(CString comment = " ")
	{	
		if(comment.IsEmpty())
			comment =  " ";
		if(m_Comments.IsEmpty())
			m_Comments = comment + "\r\n";
		else
			m_Comments = m_Comments + comment + "\r\n";
	}
	void AddErrors(CString error = " ")
	{
		if(error.IsEmpty())
			error = " ";
		if(m_Errors.IsEmpty())
			m_Errors = error + "\r\n";
		else
			m_Errors = m_Errors +  error + "\r\n";
	}
	void AddErrors(UINT nId)
	{
		AddErrors(CResString(nId));
	}
	void AddComments(UINT nId)
	{
		AddComments(CResString(nId));
	}
	SSLOutcome():m_Outcome(SSLUNKNOWN),m_ErrorCode(0),m_bNameMatch(false),m_bCheckedName(false)
	{
	}
	SSLOutcome(const SSLOutcome& outcome)
	{
		m_Outcome = outcome.m_Outcome;
		m_Errors = outcome.m_Errors;
		m_Comments = outcome.m_Comments;
		m_ErrorCode = outcome.m_ErrorCode;
	}
	SSLOutcome& operator = (const SSLOutcome& outcome)
	{
		m_Outcome = outcome.m_Outcome;
		m_Errors = outcome.m_Errors;
		m_Comments = outcome.m_Comments;
		m_ErrorCode = outcome.m_ErrorCode;
		return *this;
	}
};



struct ConnectionInfo
{
	unsigned short m_Port;
	CString m_IPAddress;
	CString m_ServerName;
	SSLOutcome m_Outcome;
	CString m_UniqueName;
	CString m_Version;

	bool m_ExportableCipher;
	unsigned long m_CipherKeyBits;
	CString m_CipherSuiteName;
	CString m_CipherKeyExchangeMethod;
	CString m_CipherName;
	CString m_CipherMode;
	CString m_HashName;

	COleDateTime m_Time;
	CPtrList m_CertDataList;

	CPtrList m_ClientCertList;


	ConnectionInfo():m_CertDataList(1),  m_Port(0),m_ClientCertList(1),
					m_ExportableCipher(false), m_CipherKeyBits(0)
	{ 
	}

	~ConnectionInfo()
	{
		POSITION pos = m_CertDataList.GetHeadPosition();
		while(pos != NULL)
		{
			CertData *pCertData = (CertData*)m_CertDataList.GetNext(pos);
			delete pCertData;
		}
		m_CertDataList.RemoveAll();

		pos = m_ClientCertList.GetHeadPosition();
		while(pos != NULL)
		{
			CertData *pCertData = (CertData*)m_ClientCertList.GetNext(pos);
			delete pCertData;
		}
		m_ClientCertList.RemoveAll();
	}

	void Copy(const ConnectionInfo& info)
	{
		m_Port = info.m_Port;
		m_IPAddress = info.m_IPAddress;
		m_Outcome = info.m_Outcome;
		m_UniqueName = info.m_UniqueName ;
		m_Version = info.m_Version;
		m_ServerName = info.m_ServerName;
		m_Time = info.m_Time;

		POSITION pos = m_CertDataList.GetHeadPosition();
		while (pos != NULL)
		{
			CertData *pCertData = (CertData*)m_CertDataList.GetNext(pos);
			delete pCertData;
		}

		m_CertDataList.RemoveAll();


		pos = m_ClientCertList.GetHeadPosition();
		while (pos != NULL)
		{
			CertData *pCertData = (CertData*)m_ClientCertList.GetNext(pos);
			delete pCertData;
		}
		m_ClientCertList.RemoveAll();

		pos = info.m_CertDataList.GetHeadPosition();
		while (pos != NULL)
		{
			CertData *pCertData = (CertData*)info.m_CertDataList.GetNext(pos);
			ASSERT(pCertData);
			CertData *newcertdata = new CertData(*pCertData);
			m_CertDataList.AddTail(newcertdata);
		}

		pos = info.m_ClientCertList.GetHeadPosition();
		while (pos != NULL)
		{
			CertData *pCertData = (CertData*)info.m_ClientCertList.GetNext(pos);
			ASSERT(pCertData);
			CertData *newcertdata = new CertData(*pCertData);
			m_ClientCertList.AddTail(newcertdata);
		}

		m_ExportableCipher = info.m_ExportableCipher;
		m_CipherKeyBits = info.m_CipherKeyBits;
		m_CipherSuiteName = info.m_CipherSuiteName;
		m_CipherKeyExchangeMethod = info.m_CipherKeyExchangeMethod;
		m_CipherName = info.m_CipherName;
		m_CipherMode = info.m_CipherMode;
		m_HashName = info.m_HashName;
	}

	ConnectionInfo(const ConnectionInfo & info)
	{  
		Copy(info);
	}

	ConnectionInfo& operator = (const ConnectionInfo& info)
	{
		Copy(info);
		return *this;
	}

	int GetNumberOfExpiredCerts()
	{
		int iExpired =0;
		POSITION pos = m_CertDataList.GetHeadPosition();
		while (pos != NULL)
		{
			CertData *pCertData = (CertData*)m_CertDataList.GetNext(pos);
			if(pCertData && pCertData->HasExpired())
				iExpired ++;
		}
		return iExpired;
	}

	bool DeleteCertFiles()
	{
		POSITION pos = m_CertDataList.GetHeadPosition();
		while(pos != NULL)
		{
			CertData *pCertData = (CertData*)m_CertDataList.GetNext(pos);
			ASSERT(pCertData);
			pCertData->DeleteCertFile();
		}
		return true;
	}

	long GetPublicKeyLength()
	{
		POSITION pos = m_CertDataList.GetHeadPosition();
		CertData *pCertData = NULL;
		while(pos != NULL)
		{
			pCertData = (CertData*)m_CertDataList.GetNext(pos);
			ASSERT(pCertData);
		}
		if(!pCertData)
			return -1;
		else
			return pCertData->m_PublicKeyLength;
	}
};

/*****************************END CONNECTION MANAGER RELATED********************************/

struct CertificateInfo
{
	CString m_RootCertStoreDir;
	CString m_UserCertStoreDir;
	short	m_bIgnoreExpired;
	CString m_csPassPhrase;
	CString m_csPrivateKey;

	CertificateInfo() 
	{
		m_bIgnoreExpired = 0;
	}
};

struct SessionResumptionInfo
{
	bool m_bSessionResumption;
	unsigned long m_ExpiryTime;
	SessionResumptionInfo() 
	{
		m_bSessionResumption = FALSE;
		m_ExpiryTime = 0;
	}
};

struct ProtocolInfo
{
	CString m_ProtocolName;//POP, IMAP, SMTP - only for reference for callbacks - not used in SSL	
	int m_ProtocolVersion;
	int m_CryptoGraphicStrength;
	CString m_ServerName;
	CString m_IPAddress;
	unsigned short m_Port;
	int m_AllowSSL_2_0;
    long m_MinEncryption;
    long m_MinKeyExchangeLength;
    short m_CipherSuite;
    short m_AllowRegExp;
	ProtocolInfo() 
	{
		m_ProtocolVersion = 0;
		m_CryptoGraphicStrength = 0;
		m_Port = 0;
		m_AllowSSL_2_0 = 1;
		m_MinEncryption = 0;
		m_MinKeyExchangeLength = 0;
		m_CipherSuite = 15;
		m_AllowRegExp = 1;
	}
};


/*************************** BEGIN CONNECTION MANAGER RELATED ***************************************/


struct QCSSLReference
{
	CString					 m_Persona;
	CertificateInfo			 m_CertificateInfo;
	SessionResumptionInfo	 m_SessionResumptionInfo;
	ProtocolInfo			 m_ProtocolInfo;
	short					 m_SSLLogSession;

	long		(*m_fnQCSSLWriteCallback)(void *bio, int cmd, const char *argp, int argi, long argl, long ret);
	long		(*m_fnQCSSLReadCallback)(void *bio, int cmd, const char *argp, int argi, long argl, long ret);
	bool		(*m_fnUpdateSSLProgressCallback) (int nCode, const char* message, void* ref);		

	void		*m_pSSL;
	void		*m_pIORef;
	void		*m_pConnectionManagerInfo;
	void		*m_pSurrenderRef;			// Not currently used but maybe useful someday.
	void		*m_pUpdateProgressRef;		// Not currently used but maybe useful someday.
};


// Function pointer declarations for access by other DLLs.
typedef bool (WINAPI * FPNQCSSLClean)();
typedef bool (WINAPI * FPNQCSSLBeginSession)(QCSSLReference *pSSLReference);
typedef bool (WINAPI * FPNQCSSLEndSession)(void *pSSL);
typedef int (WINAPI * FPNQCSSLWrite)(void *pData, int *piLen, void *pSSL);
typedef int (WINAPI * FPNQCSSLRead)(void *pSSL, int *piLen, void *pData);
typedef ConnectionInfo* (WINAPI * FPNQCSSLGetConnectionInfo)(const char * person, const char *protocol);
typedef bool (WINAPI * FPNQCSSLAddTrustedCertFromFile)(const char* filename);
typedef CPtrList* (WINAPI * FPNQCSSLGetRootCertList)();
typedef CPtrList* (WINAPI * FPNQCSSLGetUserCertList)();
typedef bool (WINAPI *FPNQCSSLAddTrustedUserCert)(CertData *pData, const char * person, const char *protocol);
typedef bool (WINAPI *FPNQCSSLDeleteTrustedUserCert)(CertData *pData, const char * person, const char *protocol);

#endif
