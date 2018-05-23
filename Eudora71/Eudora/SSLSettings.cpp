// SSLSettings.cpp
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

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "persona.h"
#include "SSLSettings.h"
  extern CString EudoraDir;
  extern CString ExecutableDir;


#include "DebugNewHelpers.h"


QCSSLSettings::QCSSLSettings()
{
  m_nSSLReceiveUsage = SSLUseNever;
  m_nSSLReceiveVersion = SSLVersionUndetermined;
  m_nSSLReceiveCryptoStrength = SSLAllCrypto;
  m_nSSLReceiveCryptoWarn = 0;
  m_nSSLSendUsage = SSLUseNever;
  m_nSSLSendVersion = SSLVersionUndetermined;
  m_nSSLSendCryptoStrength = SSLAllCrypto;
  m_nSSLSendCryptoWarn = 0;
  m_nSSLSendIgnoreExpiredCerts = 0;
  m_nSSLReceiveIgnoreExpiredCerts = 0;
  m_SSLReceiveAvailableOrAlways = false;
  m_SSLSendAvailableOrAlways = false;
  m_nSSLReceiveAllowVersion2 =1;
  m_nSSLSendAllowVersion2 = 1;
  m_SSLReceiveMinEncryption = 0;
  m_SSLSendMinEncryption = 0;
  m_SSLReceiveMinKeyExchangeLength = 0;
  m_SSLSendMinKeyExchangeLength = 0;
  m_SSLReceiveCipherSuite = 15;
  m_SSLSendCipherSuite = 15;
  m_SSLReceiveLogSession = 0;
  m_SSLSendLogSession = 0;
  m_SSLReceiveAllowRegExp = 1;
  m_SSLSendAllowRegExp = 1;

}

QCSSLSettings::~QCSSLSettings()
{


}
	
void QCSSLSettings::GrabSettings()
{
  m_nSSLReceiveUsage = ::GetIniShort (IDS_INI_SSL_RECEIVE_USE);
  m_nSSLReceiveVersion = ::GetIniShort (IDS_INI_SSL_RECEIVE_VERSION );
  m_nSSLReceiveAllowVersion2 = ::GetIniShort(IDS_INI_SSL_RECEIVE_ALLOWVERSION2);
  m_nSSLReceiveCryptoStrength = ::GetIniShort (IDS_INI_SSL_RECEIVE_CRYPTOSTRENGTH);
  m_nSSLReceiveCryptoWarn = ::GetIniShort (IDS_INI_SSL_RECEIVE_CRYPTOWARN);

  m_nSSLSendUsage = ::GetIniShort (IDS_INI_SSL_SEND_USE);
  m_nSSLSendVersion = ::GetIniShort (IDS_INI_SSL_SEND_VERSION );
  m_nSSLSendAllowVersion2 = ::GetIniShort(IDS_INI_SSL_SEND_ALLOWVERSION2);
  m_nSSLSendCryptoStrength = ::GetIniShort (IDS_INI_SSL_SEND_CRYPTOSTRENGTH);
  m_nSSLSendCryptoWarn = ::GetIniShort (IDS_INI_SSL_SEND_CRYPTOWARN);
  m_nSSLReceiveIgnoreExpiredCerts = ::GetIniShort(IDS_INI_SSL_RECEIVE_IGNOREEXPIREDCERT);
  m_nSSLSendIgnoreExpiredCerts =  ::GetIniShort(IDS_INI_SSL_SEND_IGNOREEXPIREDCERT);
  m_InputCertsDir = EudoraDir + "\r\n" + ExecutableDir;

  m_POPAlternatePort = ::GetIniShort(IDS_INI_SSLALTPORT_POP);
  m_IMAPAlternatePort = ::GetIniShort(IDS_INI_SSLALTPORT_IMAP);
  m_SMTPAlternatePort = ::GetIniShort(IDS_INI_SSLALTPORT_SMTP);
  m_nSSLAltPortReceiveVersion = ::GetIniShort(IDS_INI_SSLALTPORT_RCV_VERSION);
  m_nSSLAltPortSendVersion = ::GetIniShort(IDS_INI_SSLALTPORT_SEND_VERSION);
  m_SSLReceiveAvailableOrAlways = ((m_nSSLReceiveUsage == QCSSLSettings::SSLUseAlways) || 
						(m_nSSLReceiveUsage == QCSSLSettings::SSLUseIfAvailable));

  m_SSLSendAvailableOrAlways = ((m_nSSLSendUsage == QCSSLSettings::SSLUseAlways) || 
						(m_nSSLSendUsage == QCSSLSettings::SSLUseIfAvailable));
  m_SSLReceiveMinEncryption   = ::GetIniLong(IDS_INI_SSL_RECEIVE_MINENCRYPTION);
  m_SSLSendMinEncryption  = ::GetIniLong(IDS_INI_SSL_SEND_MINENCRYPTION);
  m_SSLReceiveMinKeyExchangeLength  = ::GetIniLong(IDS_INI_SSL_RECEIVE_MINKEYEXCHANGELENGTH);
  m_SSLSendMinKeyExchangeLength   = ::GetIniLong(IDS_INI_SSL_SEND_MINKEYEXCHANGELENGTH);
  m_SSLReceiveCipherSuite   =  ::GetIniShort(IDS_INI_SSL_RECEIVE_CIPHERSUITE);
  m_SSLSendCipherSuite  =  ::GetIniShort(IDS_INI_SSL_SEND_CIPHERSUITE);
  m_SSLReceiveLogSession    =  ::GetIniShort(IDS_INI_SSL_RECEIVE_LOGSESSION);
  m_SSLSendLogSession   =  ::GetIniShort(IDS_INI_SSL_SEND_LOGSESSION);
  m_SSLReceiveAllowRegExp	= ::GetIniShort(IDS_INI_SSL_RECEIVE_ALLOWREGULAREXP);
  m_SSLSendAllowRegExp	= ::GetIniShort(IDS_INI_SSL_SEND_ALLOWREGULAREXP);
  m_SSLReceiveClientCertsFile = GetIniString(IDS_INI_SSL_RECEIVE_CLIENTCERTFILE);
  m_SSLSendClientCertsFile = GetIniString(IDS_INI_SSL_SEND_CLIENTCERTFILE);
  m_SSLReceiveClientCertInfo = GetIniString(IDS_INI_SSL_RECEIVE_CLIENTCERTHEIRARCHY);
  m_SSLSendClientCertInfo = GetIniString(IDS_INI_SSL_SEND_CLIENTCERTHEIRARCHY);
  m_SSLReceivePrivateKey = GetIniString(IDS_INI_SSL_RECEIVE_PRIVATEKEY);
  m_SSLReceivePrivateKeyPassword = GetIniString(IDS_INI_SSL_RECEIVE_PRIVKEYPASS);
  m_SSLSendPrivateKey = GetIniString(IDS_INI_SSL_SEND_PRIVATEKEY);
  m_SSLSendPrivateKeyPassword = GetIniString(IDS_INI_SSL_SEND_PRIVKEYPASS);
}
