//	URLUtils.cpp
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



#include "stdafx.h"

#include "URLUtils.h"
#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"


bool IsHostChar(char c)
{
	return isalpha((unsigned char)c) || isdigit((unsigned char)c) || (c == '.') || (c == '-') || (c == '+') || (c == '_');
}


// ---------------------------------------------------------------------------
//		* CURLInfo													[Public]
// ---------------------------------------------------------------------------
//	CURLInfo default constructor

CURLInfo::CURLInfo()
	:	m_bURLValid(false), m_nScheme(INTERNET_SCHEME_UNKNOWN), m_nPort(0),
		m_strScheme(), m_strHostName(), m_strUserName(),
		m_strPassword(), m_strURLPath(), m_strExtraInfo()
{
}


// ---------------------------------------------------------------------------
//		* CURLInfo													[Public]
// ---------------------------------------------------------------------------
//	CURLInfo constructor which accepts a URL to analyze.

CURLInfo::CURLInfo(
	const char *		in_szURL)
{
	Analyze(in_szURL);
}


// ---------------------------------------------------------------------------
//		* Analyze													[Public]
// ---------------------------------------------------------------------------
//	Analyzes a URL for future information retrieval.

bool
CURLInfo::Analyze(
	const char *		in_szURL)
{
	//	Initialize back to default values
	m_nScheme = INTERNET_SCHEME_DEFAULT;
	m_nPort = 0;
	m_strScheme.Empty();
	m_strHostName.Empty();
	m_strUserName.Empty();
	m_strPassword.Empty();
	m_strURLPath.Empty();
	m_strExtraInfo.Empty();

	m_bURLValid = (in_szURL && *in_szURL);

	if (m_bURLValid)
	{
		//	Fill in the URL Components info
		DWORD				dwLength = strlen(in_szURL);
		CURLComponents		urlComponents;

		if (dwLength < 64)
			dwLength = 64;
		
		urlComponents.lpszScheme = m_strScheme.GetBuffer(dwLength);
		urlComponents.dwSchemeLength = dwLength;
		
		urlComponents.lpszHostName = m_strHostName.GetBuffer(dwLength);
		urlComponents.dwHostNameLength = dwLength;
		
		urlComponents.lpszUserName = m_strUserName.GetBuffer(dwLength);
		urlComponents.dwUserNameLength = dwLength;
		
		urlComponents.lpszPassword = m_strPassword.GetBuffer(dwLength);
		urlComponents.dwPasswordLength = dwLength;
		
		urlComponents.lpszUrlPath = m_strURLPath.GetBuffer(dwLength);
		urlComponents.dwUrlPathLength = dwLength;
		
		urlComponents.lpszExtraInfo = m_strExtraInfo.GetBuffer(dwLength);
		urlComponents.dwExtraInfoLength = dwLength;

		//	Crack the URL
		m_bURLValid = (InternetCrackUrl(in_szURL, 0, 0, &urlComponents) != FALSE);

		//	Release the buffers
		m_strScheme.ReleaseBuffer();
		m_strHostName.ReleaseBuffer();
		m_strUserName.ReleaseBuffer();
		m_strPassword.ReleaseBuffer();
		m_strURLPath.ReleaseBuffer();
		m_strExtraInfo.ReleaseBuffer();

		//	Remember the scheme and port
		m_nScheme = urlComponents.nScheme;
		m_nPort = urlComponents.nPort;

		//	InternetCrackUrl doesn't seem to mind empty hostnames, but we do.
		if ( m_strHostName.IsEmpty() )
		{
			//	If the scheme type would normally have a hostname, then
			//	consider the URL to be invalid.
			switch(m_nScheme)
			{
				case INTERNET_SCHEME_UNKNOWN:
				case INTERNET_SCHEME_FTP:
				case INTERNET_SCHEME_GOPHER:
				case INTERNET_SCHEME_HTTP:
				case INTERNET_SCHEME_HTTPS:
				case INTERNET_SCHEME_NEWS:
					m_bURLValid = false;
					break;
			}
		}
		else if (m_bURLValid)
		{
			//	InternetCrackUrl was happy, now let's check the host name
			//	and see if we're happy.
			const char *	szHostName = m_strHostName;
			int				nHostNameLength = m_strHostName.GetLength();
			char			cHostNameChar;
			int				nDots = 0;
			bool			bFoundNonDot = false;
			bool			bLastCharWasDot = false;

			for (int i = 0; i < nHostNameLength; i++)
			{
				cHostNameChar = szHostName[i];

				//	Count the number of dots surrounded by other letters
				//	(i.e. we don't count leading or trailing dots).
				if ( bLastCharWasDot && bFoundNonDot && (cHostNameChar != '.') )
					nDots++;

				if (cHostNameChar != '.')
					bFoundNonDot = true;

				bLastCharWasDot = (cHostNameChar == '.');

				//	Check to see if a character is legal in a host name
				if ( !IsHostChar(szHostName[i]) )
				{
					m_bURLValid = false;
					break;
				}
			}

			//	Must have at least one dot
			if (nDots == 0)
				m_bURLValid = false;
		}

		//	Remove and ignore trailing '.' in host name
		if (m_bURLValid)
			m_strHostName.TrimRight('.');
	}

	return m_bURLValid;
}


// ---------------------------------------------------------------------------
//		* IsURLNaughty										[Static Public]
// ---------------------------------------------------------------------------
//	Determines whether or not a URL is going to end up with a lump of coal
//	for Christmas.

UINT
CURLInfo::IsURLNaughty(
	const char *		in_szURL,
	const char *		in_szLinkText)
{
	//	Check to see if the URL is one of our special schemes that we
	//	handle internally.
	if ( StringStartsWithSettingString(in_szURL, IDS_INI_SW_URL_NOT_NAUGHTY_SCHEMES, ',') )
		return kURLIsNice;

	//	Strip "URL:" from start of URL if present to comply with RFC 1738
	CString		szURL = in_szURL;
	StripLeadingResourceStrings(szURL, IDS_URL_DESIGNATOR, ',');
	
	CURLInfo	urlInfo(szURL);
	
	if ( !urlInfo.IsURLValid() )
		return IDS_URL_NAUGHTY_BAD_SYNTAX_MSG;

	//	We don't currently have the smarts to detect suspicious file URLs,
	//	so just return that they are nice for now.
	if (urlInfo.GetScheme() == INTERNET_SCHEME_FILE)
		return kURLIsNice;

	CString			strURLHost = urlInfo.GetHostName();
	if ( strURLHost.IsEmpty() )
		return IDS_URL_NAUGHTY_BAD_SYNTAX_MSG;

	//	Strip "URL:" from start of URL if present to comply with RFC 1738
	CString		szLinkText = in_szLinkText;
	StripLeadingResourceStrings(szLinkText, IDS_URL_DESIGNATOR, ',');
	
	//	Try to parse the link text as a URL
	CURLInfo		linkTextURLInfo(szLinkText);

	if ( !szLinkText.IsEmpty() && !linkTextURLInfo.IsURLValid() &&
		 (urlInfo.GetScheme() >= INTERNET_SCHEME_FTP) &&
		 (urlInfo.GetScheme() <= INTERNET_SCHEME_NEWS) &&
		 (urlInfo.GetScheme() != INTERNET_SCHEME_FILE) )
	{
		//	Prepend the link text with a scheme to improve the chances that
		//	parsing the link text as a URL will work.
	
		//	Remove any previous scheme-like prefix. This avoids any funny
		//	business where we determine that the link text was not meant
		//	to look like a URL because the prefix is slightly mangled
		//	(like "http:", "http:/", "htp://", etc.). At worst we end up
		//	rechecking the same link text.
		int		nEndScheme = szLinkText.Find(':');

		//	Did we find a colon?
		if (nEndScheme >= 0)
		{
			int		nLinkTextLength = szLinkText.GetLength();
			
			//	Get the location of other important characters
			int		nFirstDot = szLinkText.Find('.');
			if (nFirstDot < 0)
				nFirstDot = nLinkTextLength;
			
			int		nFirstSlash = szLinkText.Find('/');
			if (nFirstSlash < 0)
				nFirstSlash = nLinkTextLength;
			
			int		nFirstBackSlash = szLinkText.Find('\\');
			if (nFirstBackSlash < 0)
				nFirstBackSlash = nLinkTextLength;
			
			//	Is the colon before the other key characters?
			if ( (nEndScheme < nFirstDot) && (nEndScheme < nFirstSlash) &&
				 (nEndScheme < nFirstBackSlash) )
			{
				//	Look for extra characters that could still look URL'ish,
				//	but could confuse us. We'll remove up to 4 characters
				//	that could have been thrown in to confuse us.
				char	cCheck;
				
				for (int i = 0; i < 4; i++)
				{
					if ( (nEndScheme+1) >= nLinkTextLength )
						break;
					
					cCheck = szLinkText[nEndScheme+1];
					
					//	Check for colons, forward slashes, and back slashes.
					if ( (cCheck == ':') || (cCheck == '/') || (cCheck == '\\') )
						nEndScheme++;
					else
						break;
				}
				
				//	Get the text minus the previous scheme-like prefix.
				szLinkText = szLinkText.Right(szLinkText.GetLength() - nEndScheme - 1);
			}
		}
		
		//	Try prepending the link text with a scheme with a scheme because
		//	it makes InternetCrackUrl happy.
		switch ( urlInfo.GetScheme() )
		{
			case INTERNET_SCHEME_HTTP:
				szLinkText.Insert(0, "http://");
				break;
				
			case INTERNET_SCHEME_HTTPS:
				szLinkText.Insert(0, "https://");
				break;
				
			case INTERNET_SCHEME_FTP:
				szLinkText.Insert(0, "ftp://");
				break;
				
			case INTERNET_SCHEME_MAILTO:
				szLinkText.Insert(0, "mailto://");
				break;
				
			case INTERNET_SCHEME_NEWS:
				szLinkText.Insert(0, "news://");
				break;
				
			case INTERNET_SCHEME_GOPHER:
				szLinkText.Insert(0, "gopher://");
				break;
		}
		
		linkTextURLInfo.Analyze(szLinkText);
	}

	//	If the link text is a URL, it better be similar to the actual URL!
	//	Limit hostname checking to HTTP, HTTPS, and FTP.
	INTERNET_SCHEME		scheme = urlInfo.GetScheme();

	if ( linkTextURLInfo.IsURLValid() &&
		 ((scheme == INTERNET_SCHEME_HTTP) || (scheme == INTERNET_SCHEME_HTTPS) || (scheme == INTERNET_SCHEME_FTP)) )
	{
		CString		strLinkHost = linkTextURLInfo.GetHostName();

		//	Strip noise prefixes from URL hosts
		StripLeadingSettingStrings(strURLHost, IDS_INI_SW_URL_NOT_NAUGHTY_PREFIXES, ',');
		StripLeadingSettingStrings(strLinkHost, IDS_INI_SW_URL_NOT_NAUGHTY_PREFIXES, ',');
		
		if (strURLHost.CompareNoCase(strLinkHost) != 0)
			return IDS_URL_NAUGHTY_LINK_MISMATCH_MSG;
	}

	//	Is the host numeric?
	bool	bAllDigits = true;

	for (int i = 0; i < strURLHost.GetLength(); i++)
	{
		if ( (strURLHost[i] != '.') && !isdigit((unsigned char)strURLHost[i]) )
		{
			bAllDigits = false;
			break;
		}
	}
	if (bAllDigits)
		return IDS_URL_NAUGHTY_NUMERICAL_IP_MSG;

	//	Is the host encoded?
	if (strURLHost.Find('%') != -1)
		return IDS_URL_NAUGHTY_ENCODED_MSG;

	//	Couldn't find any naughtiness - URL is nice.
	return kURLIsNice;
}
