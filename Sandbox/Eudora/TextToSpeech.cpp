#include "stdafx.h"
#include <mmsystem.h>

#include <initguid.h>
#include <spchwrap.h>

#include "TextToSpeech.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CTTSHelper Helper;

bool FillComboWithVoices(CComboBox *pCombo, CString strSel, int *pSel)
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
	pCVTxt = new CVoiceText;
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


bool CTTSHelper::FillComboBox( CComboBox *pCombo, CString strSel, int *pSel)
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
		pCombo->AddString(szModeName);
		
		gid = TTSModeInfo.gModeID;
		
		LPWSTR szTmp;
		char szVoiceGUID[124] = {0};
		StringFromCLSID(gid, &szTmp);
		WideCharToMultiByte (CP_ACP, 0, szTmp, -1, szVoiceGUID, sizeof(szVoiceGUID), 0,0);
		
		pCombo->SetItemDataPtr(index, strdup(szVoiceGUID));
		if( strSel == szVoiceGUID)
			*pSel = index;
		
		CoTaskMemFree(szTmp);
		
		index++;
	}

	return (index?true:false);
	
}


int TextToSpeech(const char *strSpeakMe, CString strGUID)
{
	HRESULT hRes;
	
	ASSERT( strlen(strSpeakMe) < 1024);

	if(!Helper.Initialize())
		return -1;

	wchar_t strText[512];
	if( MultiByteToWideChar(CP_ACP, 0, strSpeakMe, -1, strText, sizeof(strText)) == 0)
		return -1;
 
	GUID VoiceGUID;
	wchar_t szTmp[1024]; //we speak a max of only 1024 wide characters

	if( 0 == MultiByteToWideChar(CP_ACP, 0, (const char *)strGUID, -1, szTmp, 1024))
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


int SpeakFilterText(int nSpeakOptions, CString strGUID, CString strWho, CString strSubject)
{

	ASSERT(nSpeakOptions > 0);

	char *szSpeak = new char[strSubject.GetLength()+strWho.GetLength()+24];
	memset(szSpeak, 0, sizeof(szSpeak));
	if(nSpeakOptions & SPEAK_WHO)
	{
		strcpy(szSpeak, "From: ");
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

	TextToSpeech(szSpeak, strGUID);

	delete [] szSpeak;

	return 1;
}