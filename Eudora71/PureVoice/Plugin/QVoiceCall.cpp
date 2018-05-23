/*
 *  QVoice32 EMS plugin: Qualcomm PureVoice Attachment
 *
 *  Filename: QVoiceCall.cpp
 *
 *  Last Edited: April 30, 1997
 *
 *  Author: chuckh, lsong 
 *			(original sample code, Lower-Upper, from Scott Manjourides)
 *
 *  Copyright 1995, 1996 QUALCOMM Inc.
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


 *
 *  REVISIONS:
 *  01.17.97   lss   -Added settings for immediate record
 *  03.24.97   lss   -Added user definable attach name
 *  04.30.97   lss   -Separate exe name and command line strings
 *  05.21.97   lss   -Reads user defined outbox for purevoice in .ini
 *					 -Name and location of PureVoice definable via .ini
 */

#ifdef _DEBUG
#include <ctype.h>
#endif // _DEBUG

#include <afxwin.h> // MFC & Windows master header
#include "resource.h" // Resource IDs for dialogs and icons

//#include <ctype.h>

#include <malloc.h> // malloc, free
#include <string.h> // strdup
#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

#include <windows.h>
#include "ems-win.h" // The EMS API

#include "mimetype.h"
#include "stdafx.h"
#include "QVoice32.h" // for YieldEvents
#include "DummyDlg.h" // for CDummyDlg
#include "PropPageGeneral.h" // for CPropPageGeneral

/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 515;
//static const char *kPluginDescription = "QUALCOMM PureVoice Plug-In v4.0";
static const unsigned long kPluginIconID = IDI_MAIN;

static const int kNumTrans       = 0;
static const int kNumAttach      = 1;
static const int kNumSpecial     = 0;

static const int kBufferSize = 8192;

static const char *kFileErrorStr = "File I/O Error";
static const char *kTransFailedStr = "Translator Failed";

/*****************************************************************************/
/* GLOBALS */

/*
static struct AttachInfoStruct {
	char *description;
} gAttachInfo[] = {
	{
		"PureVoice",
	}
};
*/

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); p=NULL; } }


/*****************************************************************************/
/* TRANSLATER API FUNCTIONS */

/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

extern "C" long WINAPI ems_plugin_version(
    short FAR* api_version      /* Place to return api version */
)
{
	*api_version = EMS_VERSION;
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_init: Initialize plugin and get its basic info
*
\*****/

extern "C" long WINAPI ems_plugin_init(
#if (EMS_VERSION == 4)
    void FAR*FAR* globals,      /* Out: Return for allocated instance structure */
#else
    void FAR* globals,      /* Out: Return for allocated instance structure */
#endif
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	// We don't use 'globals'
	pluginInfo->numTrans = kNumTrans;
	pluginInfo->numAttachers = kNumAttach;
	pluginInfo->numSpecials = kNumSpecial;
	
	//pluginInfo->desc = strdup(kPluginDescription);
	
	pluginInfo->id = kPluginID;

	BOOL bRec = FALSE;	// by default, no immediate recording
	if ( AfxGetApp() &&
		(0 != AfxGetApp()->GetProfileInt( "Settings", "RecordNow", 0 )) )
	 {
		bRec = TRUE;
	 }

	BOOL bAttachReadMe = FALSE; // by default, no ReadMe.txt attachment
//	if ( AfxGetApp() &&
//		(0 != AfxGetApp()->GetProfileInt( "Settings", "PureVoiceReadMeAttach", 0 )) )
//	 {
//		bAttachReadMe = TRUE;
//	 }

	// get outbox dir from ini
	CString dir;
	if ( AfxGetApp() )
		dir = AfxGetApp()->GetProfileString( 
					"Settings", "PureVoiceOutboxDir" );
	// if it doesn't exist then try using eudora's
	if ( dir.IsEmpty() )
	 {
		if ( AfxGetApp() )
			dir = AfxGetApp()->GetProfileString( 
					"Settings", "AutoReceiveAttachmentsDirectory" );
/*
		// ok, if that doesn't do, try eudora's dir
		if ( dir.IsEmpty() )
		 {
			CString pathName;
			TCHAR szBuff[_MAX_PATH];
			VERIFY(::GetModuleFileName(AfxGetInstanceHandle( ),
													szBuff, _MAX_PATH));
			pathName = szBuff;
			dir = pathName.Left( pathName.ReverseFind( '\\' ) );
		 }
*/
	 }
	// get full name of purevoice executable from ini [optional]
	CString exe;
	if ( AfxGetApp() )
		exe = AfxGetApp()->GetProfileString( 
					"Settings", "PureVoiceExe" );
	
	// this is very important, we need to switch to this State data
	AFX_MANAGE_STATE( AfxGetStaticModuleState() )
	pluginInfo->icon = (HICON *)malloc(sizeof(HICON));
	*(pluginInfo->icon) = ::LoadIcon( AfxGetInstanceHandle(),
								MAKEINTRESOURCE(kPluginIconID));

	CString desc; desc.LoadString(IDS_MAIN);
	pluginInfo->desc = strdup(desc);

	if ( AfxGetApp() )
	 {
		((CQVoice32DLL*) AfxGetApp())->mb_ImmediateRecord = bRec;
		((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe = bAttachReadMe;
		((CQVoice32DLL*) AfxGetApp())->m_strOutboxDir = dir;
		((CQVoice32DLL*) AfxGetApp())->m_strExeFullName = exe;
	 }
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translator_info: Get details about a translator in a plugin
*
\*****/

extern "C" long WINAPI ems_translator_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
)
{
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_attacher_info: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_attacher_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP attachMenu         /* Out: The menu */
)
{
//	AttachInfoStruct *InfoPtr = NULL;

	if ( (attachMenu->id <= 0) || (attachMenu->id > kNumAttach) )
		return (EMSR_INVALID_TRANS);

//	InfoPtr = gAttachInfo + (attachMenu->id - 1);
//	attachMenu->desc = strdup(InfoPtr->description);

#if (EMS_VERSION == 4)
	attachMenu->size = sizeof(emsMenuS);
	attachMenu->id = kPluginID;
	// this is very important, we need to switch to this State data
	AFX_MANAGE_STATE( AfxGetStaticModuleState() )
	attachMenu->icon = (HICON *)malloc(sizeof(HICON));
	*(attachMenu->icon) = ::LoadIcon( AfxGetInstanceHandle(),
								MAKEINTRESOURCE(kPluginIconID));

	attachMenu->flags = EMSF_TOOLBAR_PRESENCE;
#endif
	
	CString desc; desc.LoadString(IDS_DESC);
	attachMenu->desc = strdup(desc);

	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_attacher_hook: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_attacher_hook(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP attachMenu,        /* In: The menu */
//    emsMIMEtypeP attachedMime,  /* Out: MIME type returned object */
//    LPSTR	 FAR* attachFile      /* Out:Name of file attacher should write data to */
    LPSTR	 attachDir,    /* In: Location to put attachments */
    long * numAttach,           /* Out: Number of files attached */
    emsDataFileP ** attachFiles  /* Out: Name of files written */
)
{
/*	emsMIMEtypeS voicemime = {
				10,
				"1.0",
				"audio",
				"vnd.qcelp",
				(emsMIMEparamS *)0,
				"attachment",
				(emsMIMEparamS *)0
	};

	emsMIMEparamS paramlist = {	10,	"name",	"voice.qcp",0 };
	emsMIMEparamS paramdisp = {	10,	"filename",	"voice.qcp",0 };

	paramlist.size = sizeof(paramlist);
	paramdisp.size = sizeof(paramdisp);
	voicemime.size = sizeof(voicemime);
	voicemime.params = &paramlist;
	voicemime.dispParams = & paramdisp;

	attachedMime = &voicemime;
*/
	//	if (attachMenu->id == kAttachCrap) 
	// *attachFile = strdup("c:\\eudora\\voice.qcp");
	// need to generate unique name everytime it creates a file for voice attachments
	
//////////////////////////////////////////////////////////////////////////////
// This section of code by lsong
	AFX_MANAGE_STATE( AfxGetStaticModuleState() ) // very important to have!

	CString title;
	CString dir = ((CQVoice32DLL*) AfxGetApp())->m_strOutboxDir;
	DWORD ret = -1;

	if ( !dir.IsEmpty() )
		ret = GetFileAttributes(dir);
	if ( (ret != -1) && (ret | FILE_ATTRIBUTE_DIRECTORY) )
		title = dir + CString("/");
	else	
		title = CString(attachDir) + CString("/");

	CString exeName = ((CQVoice32DLL*) AfxGetApp())->m_strExeFullName;
	CString cmdLine;
	if ( exeName.IsEmpty() )
	 {
		CString pathName;
		 {
			TCHAR szBuff[_MAX_PATH];
			VERIFY(::GetModuleFileName(AfxGetInstanceHandle( ),
													szBuff, _MAX_PATH));
			pathName = szBuff;
			pathName = pathName.Left( pathName.ReverseFind( '\\' ) );
		 }
		exeName.Format( "%s\\PureVoice.exe", pathName );
	 }
	cmdLine.Format( "/nomapi /eudora /o \"%s\"", title );

	CDummyDlg dummyDlg( exeName, cmdLine );//, AfxGetMainWnd() );
	dummyDlg.DoModal();

	title = dummyDlg.m_FileName;
	((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe = dummyDlg.mb_AttachReadMe;

	if ( ((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe )
		*numAttach = 2;
	else
		*numAttach = 1;

	// Allocate the number of pointers
	*attachFiles = (emsDataFileP *) malloc( sizeof(emsDataFileP) * *numAttach);

	(*attachFiles)[0] = (emsDataFileP)malloc(sizeof(emsDataFile));
	(*attachFiles)[0]->size = sizeof(emsDataFile);

	if ( ((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe )
	 {
		(*attachFiles)[1] = (emsDataFileP)malloc(sizeof(emsDataFile));
		(*attachFiles)[1]->size = sizeof(emsDataFile);
	 }

	// check if user canceled recording or not by checking
	// for the existence of the file
	FILE* fp;

	if ( fp = fopen( LPCTSTR(title), "r" ) )
	 {
		fclose( fp );

		CString finalAttachName = title;

		(*attachFiles)[0]->fileName = strdup( LPCTSTR(finalAttachName) );
		if ( ((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe )
		 {
			char tempPath[MAX_PATH];
			CString content; content.LoadString( IDS_README );
			GetTempPath( sizeof(tempPath), tempPath );
			CString readMeName = CString(tempPath) + CString("\\QCP_ReadMe.txt");
			FILE *fwp = fopen( readMeName, "wt" );
			if ( fwp )
			 {
				fwrite( LPCTSTR(content), sizeof(char), content.GetLength(), fwp );
				fclose( fwp );
				(*attachFiles)[1]->fileName = strdup( readMeName );

		emsMIMEtypeP type = (*attachFiles)[1]->info;
		type = (emsMIMEtypeP)malloc(sizeof(emsMIMEtypeS));
		type->size = sizeof(emsMIMEtypeS);
		type->version = strdup("1.0");
		type->type = strdup("text/plain");
		type->subType = 0;//strdup("");
		type->params = (emsMIMEParamP)malloc(sizeof(emsMIMEparamS));
		type->params->size = sizeof(emsMIMEparamS);
		type->params->name = strdup("charset");
		type->params->value = strdup("us-ascii");
		type->params->next = 0;

			 }
		 }
/*
		emsHeaderDataP header = (*attachFiles)[0]->header;
		header = (emsHeaderDataP)malloc(sizeof(emsHeaderDataS));
		header->size = sizeof(emsHeaderDataS);
		header->subject = strdup("Subject Test");
		header->rawHeaders = strdup("X-Http: <http://www.eudora.edu/purevoice>");
*/
		return (EMSR_OK);
	 }
//////////////////////////////////////////////////////////////////////////////

//	safefree((*attachFiles)[0]->fileName);
	safefree((*attachFiles)[0]);
	if ( ((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe )	
		safefree((*attachFiles)[1]);
	safefree(*attachFiles);

	return EMSR_ABORTED;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_info: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_special_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu         /* Out: The menu */
)
{
	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_hook: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_special_hook(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* In: The menu */
)
{
	return (EMSR_OK);
}
/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

extern "C" long WINAPI ems_plugin_config(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMailConfigP mailConfig   /* In: Eudora mail info */
)
{
	BOOL bRec;
	BOOL bAttachReadMe;
	CString dir;
	BOOL bModified = FALSE;
	{
		AFX_MANAGE_STATE( AfxGetStaticModuleState() ) // very important to have!
		bRec = ((CQVoice32DLL*) AfxGetApp())->mb_ImmediateRecord;
		bAttachReadMe = ((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe;
		dir = ((CQVoice32DLL*) AfxGetApp())->m_strOutboxDir;
		CPropertySheet ps( "QUALCOMM PureVoice Plug-In Settings" );
		CPropPageGeneral pp;
		pp.mb_ImmediateRecord = bRec;
		pp.mb_AttachReadMe = bAttachReadMe;
		pp.m_strOutboxDir = dir;
		ps.AddPage( &pp );
		if ( IDOK == ps.DoModal() )
		 {
			bRec = pp.mb_ImmediateRecord;
			((CQVoice32DLL*) AfxGetApp())->mb_ImmediateRecord = bRec;

			bAttachReadMe = pp.mb_AttachReadMe;
			((CQVoice32DLL*) AfxGetApp())->mb_AttachReadMe = bAttachReadMe;

			dir = pp.m_strOutboxDir;
			((CQVoice32DLL*) AfxGetApp())->m_strOutboxDir = dir;
			bModified = TRUE;
		 }
	}
	if ( bModified )
	{
	if ( AfxGetApp() )
	 {
	AfxGetApp()->WriteProfileInt( "Settings", "RecordNow", bRec ? 1 : 0);
//	AfxGetApp()->WriteProfileInt( "Settings", "PureVoiceReadMeAttach", bAttachReadMe ? 1 : 0);
//	AfxGetApp()->WriteProfileString( "Settings", "PureVoiceOutboxDir", dir );
	 }
	}
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_can_translate_file:
*     Check and see if a translation can be performed (file version)
*
\*****/

extern "C" long WINAPI ems_can_translate(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inTransData,   /* In: What to translate */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
{
	return (EMSR_CANT_TRANS);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translate_file: Actually perform a translation on a file
*
\*****/

extern "C" long WINAPI ems_translate_file(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inFile,        /* In: What to translate */
    emsProgress progress,       /* Func to report progress/check for abort */
    emsDataFileP outFile,       /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
{ 
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_finish: End use of a plugin and clean up
*
\*****/

extern "C" long WINAPI ems_plugin_finish(
    void FAR* globals           /* Pointer to translator instance structure */
)
{
	// We don't use 'globals'
	return (EMSR_OK); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_free: Free memory allocated by EMS plug-in
*
\*****/

extern "C" long WINAPI ems_free(
    void FAR* mem               /* Memory to free */
)
{
	if (mem)
		safefree(mem);

	return (EMSR_OK);
}

