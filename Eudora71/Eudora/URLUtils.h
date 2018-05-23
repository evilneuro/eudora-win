//	URLUtils.h
//	
//	Collection of utilities useful for manipulating and otherwise handling URLs.
//
//	Copyright (c) 2004 by QUALCOMM, Incorporated
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



#ifndef __URLUtils_H__
#define __URLUtils_H__


#include <WINDEF.H>
#include <WININET.H>


//	CURLComponents wraps the URL_COMPONENTS and provides auto-initialization.
class CURLComponents : public URL_COMPONENTS
{
  public:    
						CURLComponents()
							{
								memset( this, 0, sizeof(URL_COMPONENTS) );
								dwStructSize = sizeof(URL_COMPONENTS);
							}
};


class CURLInfo
{
  public:
	enum
	{
		kURLIsNice = 0
	};

						CURLInfo();
						CURLInfo(
							const char *		in_szURL);
					
	bool				Analyze(
							const char *		in_szURL);

	static UINT			IsURLNaughty(
							const char *		in_szURL,
							const char *		in_szLinkText);

	//	Accessors
	bool				IsURLValid() const { return m_bURLValid; }
	INTERNET_SCHEME		GetScheme() const { return m_nScheme; }
	INTERNET_PORT		GetPort() const { return m_nPort; }
	const CString &		GetSchemeName() const { return m_strScheme; }
	const CString &		GetHostName() const { return m_strHostName; }
	const CString &		GetUserName() const { return m_strUserName; }
	const CString &		GetPassword() const { return m_strPassword; }
	const CString &		GetURLPath() const { return m_strURLPath; }
	const CString &		GetExtraInfo() const { return m_strExtraInfo; }

  protected:
	bool				m_bURLValid;
	INTERNET_SCHEME		m_nScheme;
	INTERNET_PORT		m_nPort;
	CString				m_strScheme;
	CString				m_strHostName;
	CString				m_strUserName;
	CString				m_strPassword;
	CString				m_strURLPath;
	CString				m_strExtraInfo;
};



#endif		//	__URLUtils_H__



