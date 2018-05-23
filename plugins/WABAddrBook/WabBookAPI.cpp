/*
 *  Sample EMS plugin: Launch
 *
 *  Filename: lnchAPI.cpp
 *
 *  Last edited: April 1, 1997
 *
 *  Author: Julia Blumin
 *
 *  Copyright 1997 QUALCOMM Inc.
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
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#include "stdafx.h"


#ifdef _DEBUG
#include <ctype.h>
#endif // _DEBUG

#include <afxwin.h> // MFC & Windows master header
#include <afxdlgs.h>
#include <cderr.h> // Common dialog error returns
#include <windows.h>

#include <malloc.h> // malloc, free
#include <string.h> // strdup
#include <string>
#include <ctime>

#include "resource.h" // Resource IDs for dialogs and icons

#include "ems-win.h" // The EMS API
#include "WabBook.h" 

#include "WabInterface.h"
#include "CIMAPNotifier.h"

//	Prototype
HRESULT CreateAddrBook ( WabFile &book, const char *nickFolderPath );

/*****************************************************************************/
/* CONSTANTS */

static const unsigned long kPluginIconID = IDI_MAIN;
static const int kNumTools = 1;

static const int kLaunchToolID   = 1;

/*****************************************************************************/
/* GLOBALS */

// This externs into launch.cpp
extern CLaunchDLL theDLL;

static UINT gToolDescIds[] = {
	IDS_TOOL_MENU_STRING
};

static UINT gToolIconIds[] = {
	IDI_LAUNCH
};

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); p=NULL; } }

/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

interface MySink;
static MySink *gSink = NULL;

// Generalized functions
void DoIconInit(long trans_id, HICON FAR*FAR* trans_icon);

static short TellEudoraAddrBookHasChanged ( emsCallBacksP callBacks, const char *bookPath );
static long GenerateAddrBook ( emsCallBacksP callBacks );
static void InitSink ( emsCallBacksP callBacks );
static void CleanupSink ();

WabFilePtr	gWabFile;
WabInterface	gWab;
char gNickNamePath [ _MAX_PATH + 1 ];
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
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

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
    void FAR*FAR* globals,          /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	CString sTitle, sVer, sPluginID;
	int		iPluginID;

	if ( pluginInfo == NULL )
		return EMSR_UNKNOWN_FAIL;

	sTitle.LoadString(AFX_IDS_APP_TITLE);
	sVer.LoadString(IDS_VERSION_STR);
	sPluginID.LoadString(IDS_PLUGIN_ID);
	iPluginID = atoi(sPluginID);

	pluginInfo->numTrans		= 0;
	pluginInfo->numSpecials		= 0;
	pluginInfo->numAttachers	= 0;

	pluginInfo->desc = strdup(((const char *)CString(sTitle + ", " + sVer)));
	DoIconInit(-1, &(pluginInfo->icon));
	pluginInfo->id = iPluginID;

//	Locate the Nicknames folder and create the address book!
	emsGetDirectoryData dirInfo;
	dirInfo.size			= sizeof ( dirInfo );
	dirInfo.which			= EMS_PluginNicknamesDir;
	dirInfo.directory [ 0 ] = '\0';
	(void) mailConfig->callBacks->EMSGetDirectoryCB ( &dirInfo );
	strcpy ( gNickNamePath, dirInfo.directory );
	strcat ( gNickNamePath, "Wab.txt" );

	gWabFile = gWab.OpenWabFile ( NULL );
	(void) GenerateAddrBook ( mailConfig->callBacks );

	InitSink ( mailConfig->callBacks );
	return EMSR_OK;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translator_info: Get details about a plugin
*
\*****/

extern "C" long WINAPI ems_translator_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo         /* Out: Return Translator Info */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return EMSR_OK;
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
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CleanupSink ();
	delete gWabFile;
	gWabFile = NULL;
	return EMSR_OK;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_config: End use of a plugin and clean up
*
\*****/

extern "C" long WINAPI ems_plugin_config(
    void FAR* globals,           /* Pointer to translator instance structure */
	emsMailConfigP mailConfig
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return EMSR_TRANS_FAILED;	// Why is this here? 
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    long attacher_icon_id, /* ID of icon to get info for */
    HICON FAR*FAR* icon   /* Return for icon data */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!icon)
		return;

	if (attacher_icon_id < 0) /* Main plugin icon, not specific translator */
	{
		*icon = (HICON *)malloc(sizeof(HICON));
		**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else /* The actual translators */
	{
		*icon = (HICON *)malloc(sizeof(HICON));
		**icon = theDLL.LoadIcon(attacher_icon_id); /* 16x16 */
	}
}

//	tell Eudora that the nicknames have changed
static short TellEudoraAddrBookHasChanged ( emsCallBacksP callBacks, const char *bookPath ) {
	emsRegenerateData regen;
	regen.size		 = sizeof ( regen );
	regen.which		 = emsRegenerateNicknames;
	strcpy ( regen.path, bookPath );
	return callBacks->EMSRegenerateCB ( &regen );
}


static long GenerateAddrBook ( emsCallBacksP callBacks ) {
	long retVal = E_FAIL;
	if ( NOERROR == CreateAddrBook ( *gWabFile, gNickNamePath ))
		retVal = TellEudoraAddrBookHasChanged ( callBacks, gNickNamePath );

	return retVal;
	}


interface MySink : public CIMAPNotifier {
public:
	MySink ( emsCallBacksP callBacks ) : CIMAPNotifier (), fCallBacks ( callBacks ) {}
	virtual ~MySink () {}

//	IMAPIAdviseSink bits
	virtual unsigned long STDMETHODCALLTYPE OnNotify ( ULONG cNotification, LPNOTIFICATION  lpNotifications );
protected:
	emsCallBacksP fCallBacks;
private:
	MySink ();
};

unsigned long MySink::OnNotify ( ULONG cNotification, LPNOTIFICATION  lpNotifications ) {	
	GenerateAddrBook ( fCallBacks );
	return NOERROR;
}
 

LPMAPIADVISESINK gASync;
ULONG			 gAConnectionID = 0;

#define	kEventMask	\
	( fnevObjectCreated | fnevObjectDeleted | fnevObjectModified | fnevTableModified )

static void InitSink ( emsCallBacksP callBacks ) {
	gSink = new MySink ( callBacks );

	ENTRYID	entryID;
	gWabFile->GetBook ()->Advise ( sizeof ( entryID ), &entryID, kEventMask, gSink, &gAConnectionID );
	gSink->Release ();

}

static void CleanupSink () {
	if ( gAConnectionID != 0 ) {
		gWabFile->GetBook ()->Unadvise ( gAConnectionID );
		gAConnectionID = 0;
	}
//	gSync will be released by the unadvise call
//	delete gSink;
//	gSink = NULL;

//	gASync = NULL;
}
