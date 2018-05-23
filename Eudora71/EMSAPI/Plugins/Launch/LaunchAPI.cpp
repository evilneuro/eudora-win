/*
 *  Sample EMS plugin: Launch
 *
 *  Filename: LaunchAPI.cpp
 *  Original Author: Julia Blumin
 *
 *  Copyright 1997-2003 QUALCOMM, Incorporated
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
 *  Send comments and questions to <eudora-emsapi@qualcomm.com>
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

#include "resource.h" // Resource IDs for dialogs and icons

#include "emsapi-win.h" // The EMS API
#include "launch.h" 


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

// Generalized functions
void DoIconInit(long trans_id, HICON ** trans_icon);


/*****************************************************************************/
/* TRANSLATER API FUNCTIONS */

/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

extern "C" long WINAPI ems_plugin_version(
    short * api_version      /* Place to return api version */
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
    void ** globals,            /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	CString sTitle, sVer, sPluginID;
	int		iPluginID;

	sTitle.LoadString(AFX_IDS_APP_TITLE);
	sVer.LoadString(IDS_VERSION_STR);
	sPluginID.LoadString(IDS_PLUGIN_ID);
	iPluginID = atoi(sPluginID);

	if (pluginInfo)
	{
		pluginInfo->numTrans = 0;
		pluginInfo->numSpecials = kNumTools;
		pluginInfo->numAttachers = 0;

		pluginInfo->desc = strdup(((const char *)CString(sTitle + ", " + sVer)));
		
		DoIconInit(-1, &(pluginInfo->icon));
		
		pluginInfo->id = iPluginID;
	}
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_info: Get details about a tools in a plugin
*
\*****/

extern "C" long WINAPI ems_special_info(
    void * globals,           /* Out: Return for allocated instance structure */
    emsMenuP toolMenu         /* Out: The menu */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ( (toolMenu->id <= 0) || (toolMenu->id > kNumTools) )
		return (EMSR_INVALID_TRANS);

	CString sDesc;
	sDesc.LoadString(gToolDescIds[(toolMenu->id - 1)]);
	toolMenu->desc = strdup(((const char *)sDesc));
	toolMenu->flags= EMSF_TOOLBAR_PRESENCE;
		
	DoIconInit(gToolIconIds[(toolMenu->id - 1)], &(toolMenu->icon));
	
	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_hook: Get details about a tools in a plugin
*
\*****/

extern "C" long WINAPI ems_special_hook(
    void * globals,				/* Out: Return for allocated instance structure */
    emsMenuP toolMenu			/* In: The menu */
)
{
	CString	sTitle;
	CString errString;
	CString regPath;
	UINT err = 0;
	long  len = _MAX_PATH;
	char  appPath[MAX_PATH+1];
    HKEY  hkResult;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	sTitle.LoadString(AFX_IDS_APP_TITLE);
	

	if ( !toolMenu || (toolMenu->id != kLaunchToolID) )
		return (EMSR_UNKNOWN_FAIL);


	// HKEY_CURRENT_USER registry
	regPath.LoadString(IDS_REG_PATH);

	// In this case HKEY_CLASSES_ROOT is used ... you may need to change this!
	err = RegOpenKey(HKEY_CLASSES_ROOT, regPath, &hkResult);
	err = RegQueryValue(hkResult,NULL, appPath, &len);
	RegCloseKey(hkResult);

	if (*appPath)
		err = WinExec(appPath, SW_SHOWNORMAL);
	else
		errString.Format(IDS_ERR_NOT_REG, (const char *)sTitle);
		
	if ( !errString.IsEmpty() || err < 32)
	{
		switch (err)
		{
			case 0:
				errString.Format(IDS_MEMORYERR_STR);
				break;
			case ERROR_BAD_FORMAT:
				errString.Format(IDS_BAD_EXE, appPath);
				break;
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				errString.Format(IDS_INVALIDFILEERR_STR, appPath);
				break;
			default:
				errString.Format(IDS_INTERNALERR_STR, appPath);
				break;
		}
		
		AfxMessageBox( errString, IDOK | MB_ICONEXCLAMATION);
		return (EMSR_UNKNOWN_FAIL);
	}

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_finish: End use of a plugin and clean up
*
\*****/

extern "C" long WINAPI ems_plugin_finish(
    void * globals           /* Pointer to translator instance structure */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

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
    void * mem               /* Memory to free */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (mem)
		safefree(mem);

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    long attacher_icon_id, /* ID of icon to get info for */
    HICON ** icon          /* Return for icon data */
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
