/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					configinit.cpp
Description:		Default initialization from an ".ini" file for PH or Finger
Date:						9/4/97
Version:  			1.0 
Module:					PH.DLL (PH protocol directory service object)
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma warning(disable : 4514 4710)
#include <afx.h>
#include <objbase.h>
#include <assert.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "ustring.h"
#include "registry.h"
#include "configinit.h"
#include "resource.h"
#include "QCUtils.h"


static const char szThisModule[] = "ph.dll";

static IENTRY arFixedIniKeys[] = {
/*-------------------------------------------------------------------
 type					pszKey (read)				nIDKey (write)			Default Value
 -------------------------------------------------------------------*/
 DT_STRING,		"HostName",					IDS_REGHOSTNAME,		0,
 DT_STRING,		"ServerName", 			IDS_REGNAME,				0,
 DT_INT,			"Port",							IDS_REGPORT,				105,
};
const int nNumFixedIniKeys = sizeof(arFixedIniKeys) / sizeof(IENTRY);


// constructor
CConfigInit::CConfigInit(IDirConfig * pIDirConfig)
{
	assert(pIDirConfig);
	m_pIDirConfig = pIDirConfig;
	m_pIDirConfig->AddRef();
	m_pszFileName = NULL;
}

// destructor
CConfigInit::~CConfigInit()
{
	if (m_pIDirConfig)
		m_pIDirConfig->Release();

	delete [] m_pszFileName;
}


BOOL CConfigInit::Initialize(LPSTR pszFileRoot)
{
	LPSTR			pEndToken = NULL;

	// get our current module handle
	HMODULE hModule = GetModuleHandle(szThisModule);

	if (!hModule || !GetModuleFileName(hModule, m_szBuffer, sizeof(m_szBuffer)-1)) 
		return FALSE;

	if (NULL == (pEndToken = strrchr(m_szBuffer, '\\'))) 
		return FALSE;

	*pEndToken = 0;		// null terminate string at the backslash
	
	if ((strlen(m_szBuffer) + strlen(pszFileRoot)) > sizeof(m_szBuffer)-1)
		return FALSE;

	strcat(m_szBuffer, "\\");
	strcat(m_szBuffer, pszFileRoot);

	// free previous file name 
	delete [] m_pszFileName;

	m_pszFileName = SafeStrdupMT(m_szBuffer);

	return TRUE;

}


void CConfigInit::StoreFixed(LPIENTRY pIEntry, int nNumEntries, LPSTR pszReadSection, LPSTR pszWriteSection)
{
	LPIENTRY	pi;
	int				i;

	// for all entries in our key table...
	for (i=0, pi=pIEntry; i < nNumEntries; pi++, i++) {
		switch(pi->dt) {
			case DT_STRING: 
				CopyString(pi->pszKey, pi->nIDKey, pszReadSection, pszWriteSection);	
				break;
			case DT_INT:    
				CopyInt(pi->pszKey, pi->nIDKey, pszReadSection, pszWriteSection, pi->nDefValue);  
				break;
		}
	}
}

void CConfigInit::CreateSection(LPSTR pszWriteSection)
{
	// create the section
	if (SUCCEEDED(m_pIDirConfig->CreateDirSection(pszWriteSection, NULL))) {
		GUID guid;
		if (SUCCEEDED(CoCreateGuid(&guid))) {
			char buffer[50];
			CLSIDtochar(guid, buffer, sizeof(buffer));
 			// save the GUID as the default (unnamed) value for the section
			m_pIDirConfig->SetDirStr(pszWriteSection, NULL, buffer);
		}
	}
}


// read a string value from the ".ini" file and store in our IDirConfig (virtualized registry)
void CConfigInit::CopyString(LPSTR pszReadKey, UINT uIDWriteKey, LPSTR pszReadSection, LPSTR pszWriteSection)
{
	CUString str(uIDWriteKey);
	if (0 < GetPrivateProfileString(pszReadSection, pszReadKey, "", m_szBuffer, sizeof(m_szBuffer), m_pszFileName)) {
		m_pIDirConfig->CreateDirKey(pszWriteSection, str);
		m_pIDirConfig->SetDirStr(pszWriteSection, str, m_szBuffer);
	}
}

// read an int value from the ".ini" file and store in our IDirConfig (virtualized registry)
void CConfigInit::CopyInt(LPSTR pszReadKey, UINT uIDWriteKey, LPSTR pszReadSection, LPSTR pszWriteSection, UINT nDefVal)
{
	CUString str(uIDWriteKey);
	UINT nValue = GetPrivateProfileInt(pszReadSection, pszReadKey, nDefVal, m_pszFileName);
	m_pIDirConfig->CreateDirKey(pszWriteSection, str);
	m_pIDirConfig->SetDirDword(pszWriteSection, str, (DWORD)nValue);
}


///////////////////////////////////////////////////////////////////////////////
// CPHInit class


BOOL CPHInit::LoadPreConfigData()
{
	const char szKeyRoot[] = "PH";
	const char szIniRoot[] = "ph.ini";

	Initialize((LPSTR)szIniRoot);

	int nCount = GetPrivateProfileInt("General", "ServerCount", 0, m_pszFileName);
	for (int i=0; i < nCount; i++) {
		char szSection[40];
		sprintf(szSection, "%s.%d", szKeyRoot, i);
		// create a new section
		CreateSection(szSection);
		// store the fixed key information
		StoreFixed(arFixedIniKeys, nNumFixedIniKeys, szSection, szSection);
		char szReadSection[40];
		sprintf(szReadSection, "%s.%d.Attributes", szKeyRoot, i);
		// store the attribute table
		StoreAttribTable(szReadSection, szSection);
	}

	return TRUE;
}

// for PH only, store the attribute table
void CPHInit::StoreAttribTable(LPSTR pszReadSection, LPSTR pszWriteSection)
{
	for (int i=(int)DS_NAME; i < (int)DS_MAXFIELD; i++) {
		char buffer[80];
		if (SUCCEEDED(m_pIDirConfig->GetFieldKey(DS_FIELD(i), buffer, sizeof(buffer)-1))) {
			if (0 < GetPrivateProfileString(pszReadSection, buffer, "", m_szBuffer, sizeof(m_szBuffer), m_pszFileName)) {
				m_pIDirConfig->CreateDirKey(pszWriteSection, m_szBuffer);
				m_pIDirConfig->SetDirStr(pszWriteSection, m_szBuffer, buffer);		// key/values are reversed in registry in relation to INI file
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// CFingerInit class

BOOL CFingerInit::LoadPreConfigData()
{
	const char szKeyRoot[] = "FINGER";
	const char szIniRoot[] = "finger.ini";

	Initialize((LPSTR)szIniRoot);

	int nCount = GetPrivateProfileInt("General", "ServerCount", 0, m_pszFileName);
	for (int i=0; i < nCount; i++) {
		char szSection[40];
		sprintf(szSection, "%s.%d", szKeyRoot, i);
		// create a new section
		CreateSection(szSection);
		// store the fixed key information
		StoreFixed(arFixedIniKeys, nNumFixedIniKeys, szSection, szSection);
	}

	return TRUE;
}


