// testplugin.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <fstream.h>

#include "testplugin.h"
#include "ems-win.h"
#include "SettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CTestpluginApp

BEGIN_MESSAGE_MAP(CTestpluginApp, CWinApp)
	//{{AFX_MSG_MAP(CTestpluginApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestpluginApp construction

CTestpluginApp::CTestpluginApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTestpluginApp object

CTestpluginApp theApp;

BOOL CTestpluginApp::InitInstance()
{
	m_OurIcon = LoadIcon(IDI_VCARD);
	m_MessagePlugin = new CMessagePlug(this);

	return TRUE;
}


extern "C" long WINAPI	ems_plugin_version(short FAR* version)
{
	*version = 4;
	return EMSR_OK;
}


// Douglas Adam's Email address is: dna@tdv.com

extern "C" long WINAPI	ems_plugin_init(void **globals, short APIVersion, emsMailConfigP MConfig, emsPluginInfoP pinfo)
{
	pinfo->numTrans =2;
	pinfo->numSpecials =0;
	pinfo->desc = new char[strlen(_T("VCard Access Plugin v1.0")) + 1];
	strcpy(pinfo->desc, _T("VCard Access Plugin v1.0"));
	pinfo->id = 30;


	CTestpluginApp* TheApp;
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		TheApp = (CTestpluginApp*)AfxGetApp();
		pinfo->icon = &TheApp->m_OurIcon;
	}
	TheApp->m_EudoraFuncs.EnumAB = (*(emsEnumAddressBooks )MConfig->callBacks->EMSEnumAddressBooksCB);
	TheApp->m_EudoraFuncs.AddNNToAB = (*(emsAddABEntry )MConfig->callBacks->EMSEnumAddressBooksCB);
	TheApp->m_INIFilename = new char[strlen(MConfig->configDir) + strlen(_T("\\EudVCard.ini")) +1];
	strcpy(TheApp->m_INIFilename, MConfig->configDir);
	strcat(TheApp->m_INIFilename, _T("\\EudVCard.ini"));

	if (AfxIsValidAddress( (void*)TheApp->m_EudoraFuncs.EnumAB, 1, FALSE) != 1 || 
		AfxIsValidAddress( (void*)TheApp->m_EudoraFuncs.AddNNToAB, 1, FALSE) != 1 )
		return EMSR_UNKNOWN_FAIL;

	return EMSR_OK;
}

extern "C" long WINAPI	ems_translator_info(void *globals, emsTranslatorP tinfo)
{
	CTestpluginApp* TheApp;
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		TheApp = (CTestpluginApp*)AfxGetApp();
		tinfo->icon = &TheApp->m_OurIcon; //&TheApp->m_MessagePlugin->m_Icon;
	}
	if (tinfo->id == 1)
	{
		tinfo->type = EMST_TEXT_FORMAT;
		tinfo->flags = EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY;
		tinfo->desc = new char[strlen(_T("VCard Processing Plugin"))+1];
		strcpy(tinfo->desc, _T("VCard Processing Plugin"));
	}
	else if (tinfo->id == 2)
	{
		tinfo->type = EMST_LANGUAGE;
		tinfo->flags = EMSF_ON_REQUEST;
		tinfo->desc = new char[strlen(_T("Import This VCard"))+1];
		strcpy(tinfo->desc, _T("Import This VCard"));	
	}
	else
		return EMSR_UNKNOWN_FAIL;

	return EMSR_OK;
}

extern "C" long WINAPI	ems_can_translate(void *globals, emsTranslatorP tinfo, emsDataFileP inTransData, emsResultStatusP result)
{
	if (inTransData->context == EMSF_ON_ARRIVAL)
	{
		if (strcmp(inTransData->info->type, _T("text")) == 0)
		{	
			if ( (stricmp(inTransData->info->subType, _T("x-vcard")) == 0) || 
				 (stricmp(inTransData->info->subType, _T("vcard")) == 0) )	// In case it gets made a standard subtype...
			{
				return EMSR_NOT_NOW;
			}
			else
				return EMSR_CANT_TRANS;
		}
		else
			return EMSR_CANT_TRANS;
		result->error =	NULL;
		result->code = 0;
	}
	else if (inTransData->context == EMSF_ON_DISPLAY)
	{
		return EMSR_NOW;
	}
	else if (inTransData->context == EMSF_ON_REQUEST)
	{
		int a=2;
		return EMSR_NOW;
	}
	else
		return EMSR_UNKNOWN_FAIL;
}

VCEntry *ScanVCard(char *Filename)
{
	ifstream * foo = new ifstream(Filename);

	char *newstring = new char[512];
	bool done = false;

	while (!done)
	{
		foo->getline(newstring, 99);
		if (newstring[0] == 0)
			done = true;
		else
		{
			sscanf


		

		}
	}


//	FILE *foo = fopen( Filename, _T("r"));

//	char *newstring = new char[100];

//	int a = fscanf( foo, "%s\\r\\n", newstring);




	return NULL;
}

extern "C" long WINAPI	ems_translate_file(void *globals, emsTranslatorP tinfo, emsDataFileP inTransData, emsProgress progress, emsDataFileP outfile, emsResultStatusP result)
{
	if (inTransData->context == EMSF_ON_DISPLAY && tinfo->id == 1)
	{
		if (strcmp(inTransData->info->type, _T("text")) == 0)
		{	
			if ( (stricmp(inTransData->info->subType, _T("x-vcard")) == 0) || 
				 (stricmp(inTransData->info->subType, _T("vcard")) == 0) )	// In case it gets made a standard subtype...
			{
				VCEntry *Entries = ScanVCard(inTransData->fileName);
				if (!Entries)
					return EMSR_NO_ENTRY;
			}
			else
				return EMSR_CANT_TRANS;
		}
		else
			return EMSR_CANT_TRANS;
		result->error =	NULL;
		result->code = 0;		

		return EMSR_OK;	
	}
	else if (inTransData->context == EMSF_ON_REQUEST && tinfo->id == 2)
	{

		return EMSR_OK;
	}
	else
		return EMSR_CANT_TRANS;
}

extern "C" long WINAPI	ems_plugin_config(void *globals, emsMailConfigP MailConfig)
{
	CWnd* pSettingsPanel = CWnd::GetForegroundWindow( );

	HGLOBAL TheDialog;
	HMODULE hModule = GetModuleHandle(NULL);
	CTestpluginApp* TheApp;

	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		TheApp = (CTestpluginApp*)AfxGetApp();		
	}
	if (!TheApp->m_ShowingSettingsPanel)
	{
		hModule = GetModuleHandle(_T("testplugin"));

		HRSRC blah = FindResource( hModule, (char *)IDD_SETTINGS, RT_DIALOG);
		
		TheDialog = LoadResource( hModule, blah);

		TheApp->m_Settings = new CSettingsDialog();
		TheApp->m_Settings->m_MainApp = TheApp;
		TheApp->m_Settings->CreateIndirect( TheDialog, pSettingsPanel);
		TheApp->m_Settings->ShowWindow(SW_SHOW);
		
		TheApp->m_ShowingSettingsPanel = true;
	}
	else
	{
		TheApp->m_Settings->SetFocus();
	}

	return EMSR_OK;	
}



