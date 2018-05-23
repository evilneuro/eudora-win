// TextToSpeech.cpp
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include <mmsystem.h>

#include <initguid.h>
#include <spchwrap.h>

#include "TextToSpeech.h"

#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"


CTTSHelper Helper;

bool FillComboWithVoices(CComboBox *pCombo, const char* strSel, int *pSel)
{
	return Helper.FillComboBox(pCombo, strSel, pSel);
}


CTTSHelper::~CTTSHelper()
{

	if(pCVTxt)
		delete pCVTxt;
	
	if(bInit)
	{
		bInit = false;
		CoUninitialize();
	}
}

HRESULT CTTSHelper::Speak(wchar_t *str, GUID vid)
{
	if(pCVTxt)
	{
		pCVTxt->TTSModeSet(vid);
		return pCVTxt->Speak(str);
	}
	else return E_FAIL;
}


bool CTTSHelper::Initialize()
{
	if(bInit)
		return true;

	CoInitialize(NULL);
	HRESULT hRes;
	
	/*CTTSEnum *pTTSEnum = new CTTSEnum();
	if(!pTTSEnum)
		return false;
	
	HRESULT hRes = pTTSEnum->Init();
	if (hRes)
		return false;

	TTSMODEINFOW *pTTSModeInfo = new TTSMODEINFOW;

	if(!pTTSModeInfo) 
		return false;

	char szModeName[256];
	int n=0;
	GUID gid;
	while (pTTSEnum->Next(pTTSModeInfo) == NOERROR)
	{
		n++;
		WideCharToMultiByte (CP_ACP, 0, pTTSModeInfo->szModeName, -1, szModeName, sizeof(szModeName), 0,0);
		printf("Mode: %s\n", szModeName);
		gid = pTTSModeInfo->gModeID;
	}

	if( pTTSEnum->Next(pTTSModeInfo) == NOERROR)
		gid = pTTSModeInfo->gModeID;
	*/

	//The application then creates a new Voice Text object and initializes it:
	pCVTxt = DEBUG_NEW_NOTHROW CVoiceText;
	if (!pCVTxt) 
		return false;
    
	hRes = pCVTxt->Init(L"Eudora Pro");
	if (hRes != NOERROR)
	{
		delete pCVTxt;
		pCVTxt = NULL;
		CoUninitialize();
		return false;
	}
	
	/*hRes = pCVTxt->TTSModeSet(gid);
	if (hRes)
		return false;*/
	
	bInit = true;
	
	return true;
}


bool CTTSHelper::FillComboBox( CComboBox *pCombo, const char* strSel, int *pSel)
{
	CTTSEnum TTSEnum;
	
	HRESULT hRes = TTSEnum.Init();
	if (hRes)
		return false;

	TTSMODEINFOW TTSModeInfo;

	
	char szModeName[256];
	
	int index=0;
	GUID gid;
	
	*pSel = 0; //default to the first voice in case
	while (TTSEnum.Next(&TTSModeInfo) == NOERROR)
	{
		WideCharToMultiByte (CP_ACP, 0, TTSModeInfo.szModeName, -1, szModeName, sizeof(szModeName), 0,0);
		//printf("Mode: %s\n", szModeName);
		index = pCombo->AddString(szModeName);
		if(index != CB_ERR) {
			gid = TTSModeInfo.gModeID;
		
			LPWSTR szTmp;
			char szVoiceGUID[124] = {0};
			StringFromCLSID(gid, &szTmp);
			WideCharToMultiByte (CP_ACP, 0, szTmp, -1, szVoiceGUID, sizeof(szVoiceGUID), 0,0);
		
			pCombo->SetItemDataPtr(index, strdup(szVoiceGUID));
			if (stricmp(strSel, szVoiceGUID) == 0)
				*pSel = index;
		
			CoTaskMemFree(szTmp);
		
		}
	}

	return (index?true:false);
	
}


int TextToSpeech(const char *strSpeakMe, const char* strGUID)
{
	HRESULT hRes;
	
	ASSERT( strlen(strSpeakMe) < 1024);

	if(!Helper.Initialize())
		return -1;

	char *pszSpeakThis = NULL;

#ifndef	SPEECH_API_GETS_A_CLUE
	// For some reason, the TTS code goes nuts
	// if it gets 8-bit characters.  So, we kill the
	// 8-bit characters, which won't make furriners
	// ecstatic, but it's better than having vcmd sit
	// on the CPU for arbitrary amounts of time
	char strSpeak[512];
	if (GetIniShort(IDS_INI_FLATTEN_TTS))
	{
		strncpy(strSpeak,strSpeakMe,sizeof(strSpeak));
		strSpeak[sizeof(strSpeak)-1] = 0;
		for (pszSpeakThis = strSpeak; *pszSpeakThis; pszSpeakThis++)
			*pszSpeakThis = (char)(*(unsigned char *)pszSpeakThis & 0x7f);
		pszSpeakThis = strSpeak;
	}
#endif

	wchar_t strText[512];
	if( MultiByteToWideChar(CP_ACP, 0, pszSpeakThis ? pszSpeakThis : strSpeakMe, -1, strText, sizeof(strText)) == 0)
		return -1;
 
	GUID VoiceGUID;
	wchar_t szTmp[1024]; //we speak a max of only 1024 wide characters

	if( 0 == MultiByteToWideChar(CP_ACP, 0, strGUID, -1, szTmp, 1024))
	{
		ASSERT(0);
		return -1;
	}

	if(NOERROR != CLSIDFromString(szTmp, &VoiceGUID))
	{
		ASSERT(0);  //wahts up with the GUID
		return -1;
	}

	//By calling Init() with an application name, the application automatically registers. There are other Init() calls that allow an application better control of the speech engine.
	//To speak, the application calls CVoiceText::Speak():
	hRes = Helper.Speak(strText, VoiceGUID);
    if (hRes) 
        return -1;
   
    return 0;
}


int SpeakFilterText(int nSpeakOptions, const char* strGUID, const char* strWho, const char* strSubject, BOOL bOutgoing)
{
	ASSERT(nSpeakOptions > 0);

	char* szSpeak = DEBUG_NEW char[strlen(strSubject) + strlen(strWho) + 24];
	*szSpeak = 0;
	if(nSpeakOptions & SPEAK_WHO)
	{
		if(!bOutgoing)
			strcpy(szSpeak, "From: ");
		else
			strcpy(szSpeak, "To: ");

		strcat(szSpeak, strWho);
		strcat(szSpeak, ". ");
	}
			
	if(nSpeakOptions & SPEAK_SUBJECT)
	{
		char *szTmpSubject = szSpeak+strlen(szSpeak);
		strcpy(szTmpSubject, strSubject);
			
		char * floater;
		while (1)
		{
			if (!_strnicmp(szTmpSubject, "re:",3))
				floater = szTmpSubject+3;
			else if (!_strnicmp(szTmpSubject, "fwd:",4))
				floater = szTmpSubject+4;
			else 
				break;
			
			while (*floater == ' ')
				floater++;
		
			strcpy(szTmpSubject, floater);
		}
	}

	if (*szSpeak)
		TextToSpeech(szSpeak, strGUID);

	delete [] szSpeak;

	return 1;
}