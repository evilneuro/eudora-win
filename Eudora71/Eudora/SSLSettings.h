// SSLSettings.h
//
// Copyright (c) 2000-2001 by QUALCOMM, Incorporated
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

#ifndef __SSLSETTINGS_H
#define __SSLSETTINGS_H


struct SSLSettings
{
	enum SSLUsage
	{
		SSLUseNever,
		SSLUseIfAvailable,
		SSLUseAlternatePort,
		SSLUseAlways
	};

	enum SSLVersion
	{
		SSLVersionUndetermined,
		SSLVersion_3_0_with_2_0_Hello,
		SSLVersion_3_0_Only,
		TLSVersion_1_0_Only,
		TLSVersion1_0_With_2_0_Hello,
		SSLVersion_2_0,
		SSLVersion_3_0,
		TLS_Version_1_0
	};

	enum CryptoStrength
	{
		SSLAllCrypto,
		SSLStrongCryptoOnly,
		SSLExportCryptoOnly,
		SSLServerGatedCrypto
	};

	enum CipherSuite
	{
		SSLUseCipherAll=0xF,//0xF
		SSLUseCipherRSA=0x1,//0x01
		SSLUseCipherECC =0x02
    };

	short   m_nSSLReceiveUsage;
    short   m_nSSLReceiveVersion;
	short m_nSSLReceiveAllowVersion2;
    short	m_nSSLReceiveCryptoStrength;
    short	m_nSSLReceiveCryptoWarn;
	short   m_nSSLReceiveIgnoreExpiredCerts;
	CString m_SSLReceiveOutputCertDir;
	long m_SSLReceiveMinEncryption;
	long m_SSLSendMinEncryption;
	long m_SSLReceiveMinKeyExchangeLength;
	long m_SSLSendMinKeyExchangeLength;
	short m_SSLReceiveCipherSuite;
    short m_SSLSendCipherSuite;
    short m_SSLReceiveLogSession;
	short m_SSLSendLogSession;
	short m_SSLReceiveAllowRegExp;
	short m_SSLSendAllowRegExp;

	short   m_nSSLSendUsage;
    short   m_nSSLSendVersion;
	short m_nSSLSendAllowVersion2;
    short	m_nSSLSendCryptoStrength;
    short	m_nSSLSendCryptoWarn;
	short   m_nSSLSendIgnoreExpiredCerts;
	CString m_SSLSendOutputCertDir;

	CString m_InputCertsDir;
	short m_POPAlternatePort;
	short m_IMAPAlternatePort;
	short m_SMTPAlternatePort;
	short m_nSSLAltPortReceiveVersion;
	short m_nSSLAltPortSendVersion;
	bool m_SSLReceiveAvailableOrAlways;
	bool m_SSLSendAvailableOrAlways;
	CString m_SSLReceiveClientCertsFile;
    CString m_SSLSendClientCertsFile;
	CString m_SSLReceiveClientCertInfo;
	CString	m_SSLSendClientCertInfo;
	CString m_SSLReceivePrivateKey;
	CString m_SSLReceivePrivateKeyPassword;
	CString m_SSLSendPrivateKey;
	CString m_SSLSendPrivateKeyPassword;

	SSLSettings& operator =(const SSLSettings& settings)
	{
		m_nSSLReceiveUsage = settings.m_nSSLReceiveUsage;
		m_nSSLReceiveVersion = settings.m_nSSLReceiveVersion;
	    m_nSSLReceiveAllowVersion2 = settings.m_nSSLReceiveAllowVersion2;
		m_nSSLReceiveCryptoStrength = settings.m_nSSLReceiveCryptoStrength;
		m_nSSLReceiveCryptoWarn = settings.m_nSSLReceiveCryptoWarn;
		m_nSSLReceiveIgnoreExpiredCerts = settings.m_nSSLReceiveIgnoreExpiredCerts;
		m_SSLReceiveOutputCertDir = settings.m_SSLReceiveOutputCertDir;
		m_nSSLSendUsage = settings.m_nSSLSendUsage;
		m_nSSLSendVersion = settings.m_nSSLSendVersion;
		m_nSSLSendAllowVersion2 = settings.m_nSSLSendAllowVersion2;
		m_nSSLSendCryptoStrength = settings.m_nSSLSendCryptoStrength;
		m_nSSLSendCryptoWarn = settings.m_nSSLSendCryptoWarn;
		m_nSSLSendIgnoreExpiredCerts = settings.m_nSSLSendIgnoreExpiredCerts;
		m_SSLSendOutputCertDir = settings.m_SSLSendOutputCertDir;
		m_InputCertsDir =  settings.m_InputCertsDir;
		m_POPAlternatePort = settings.m_POPAlternatePort ;
		m_IMAPAlternatePort = settings.m_IMAPAlternatePort;
		m_SMTPAlternatePort = settings.m_SMTPAlternatePort;
		m_nSSLAltPortReceiveVersion = settings.m_nSSLAltPortReceiveVersion;
		m_nSSLAltPortSendVersion = settings.m_nSSLAltPortSendVersion;
		m_SSLReceiveAvailableOrAlways = settings.m_SSLReceiveAvailableOrAlways;
		m_SSLReceiveMinEncryption = settings.m_SSLReceiveMinEncryption;
		m_SSLSendMinEncryption = settings.m_SSLSendMinEncryption;
		m_SSLReceiveMinKeyExchangeLength = settings.m_SSLReceiveMinKeyExchangeLength;
		m_SSLSendMinKeyExchangeLength = settings.m_SSLSendMinKeyExchangeLength;
		m_SSLReceiveCipherSuite = settings.m_SSLReceiveCipherSuite;
		m_SSLSendCipherSuite = settings.m_SSLSendCipherSuite;
		m_SSLReceiveLogSession = settings.m_SSLReceiveLogSession;
		m_SSLSendLogSession = settings.m_SSLSendLogSession;
		m_SSLReceiveAllowRegExp = settings.m_SSLReceiveAllowRegExp;
		m_SSLSendAllowRegExp = settings.m_SSLSendAllowRegExp;
		m_SSLReceiveClientCertsFile = settings.m_SSLReceiveClientCertsFile;
		m_SSLSendClientCertsFile = settings.m_SSLSendClientCertsFile;
		m_SSLReceiveClientCertInfo = settings.m_SSLReceiveClientCertInfo;
		m_SSLSendClientCertInfo = settings.m_SSLSendClientCertInfo;
		m_SSLReceivePrivateKey = settings.m_SSLReceivePrivateKey;
		m_SSLReceivePrivateKeyPassword = settings.m_SSLReceivePrivateKeyPassword;
		m_SSLSendPrivateKey = settings.m_SSLSendPrivateKey;
		m_SSLSendPrivateKeyPassword =settings.m_SSLSendPrivateKeyPassword;

		return *this;
	}
};


class QCSSLSettings : public SSLSettings
{
public:


	QCSSLSettings();
	~QCSSLSettings();

	
	void	GrabSettings();//will grab setting for current personality that is set.

private:


public:
	short GetSSLReceiveUsage(){ return m_nSSLReceiveUsage;}
	short GetSSLReceiveVersion(){return m_nSSLReceiveVersion;}
	short GetSSLReceiveCryptoStrength(){ return m_nSSLReceiveCryptoStrength;}
	short GetSSLReceiveCryptoWarn(){ return m_nSSLReceiveCryptoWarn;}
	short GetSSLSendUsage(){ return m_nSSLSendUsage;}
	short GetSSLSendVersion(){return m_nSSLSendVersion;}
	short GetSSLSendCryptoStrength(){ return m_nSSLSendCryptoStrength;}
	short GetSSLSendCryptoWarn(){ return m_nSSLSendCryptoWarn;}
	short GetSSLSendIgnoreExpiredCerts(){ return m_nSSLSendIgnoreExpiredCerts;}
	short GetSSLReceiveIgnoreExpiredCerts(){ return m_nSSLReceiveIgnoreExpiredCerts;}
	long  GetSSLReceiveMinEncryption(){return m_SSLReceiveMinEncryption;}
	long  GetSSLSendMinEncryption(){ return m_SSLSendMinEncryption;}
	long  GetSSLReceiveMinKeyExchangeLength(){ return m_SSLReceiveMinKeyExchangeLength;}
	long  GetSSLSendMinKeyExchangeLength(){return m_SSLSendMinKeyExchangeLength;}
	short GetSSLReceiveCipherSuite(){return m_SSLReceiveCipherSuite;}
    short GetSSLSendCipherSuite(){return m_SSLSendCipherSuite;}
    short GetSSLReceiveLogSession(){ return m_SSLReceiveLogSession;}
	short GetSSLSendLogSession(){ return m_SSLSendLogSession;}
	short GetSSLReceiveAllowRegExp(){ return m_SSLReceiveAllowRegExp;}
	short GetSSLSendAllowRegExp(){return m_SSLSendAllowRegExp;}

};

	
#endif // __SSLSETTINGS_H

