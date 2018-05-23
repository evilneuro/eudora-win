/*
 *  Sample EMS plugin: TooMenu
 *
 *  Filename: MenuTran.cpp
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
#include "resource.h" // Resource IDs for dialogs and icons
#include "ToolMenu.h" 

#include <malloc.h> // malloc, free
#include <string.h> // strdup

#include <windows.h>
#include "emsapi-win.h" // The EMS API

#include <afxdlgs.h>


/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 28;
static const char *kPluginDescription = "ToolMenu Sample Plugin, v1.0";
static const unsigned long kPluginIconID = IDI_MAIN;

static const int kSpecialNothing = 1;
static const int kMoreSpecialMoreNothing = 2;
static const int kNumSpecial     = 2;

/*****************************************************************************/
/* GLOBALS */

// This externs into ToolMenu.cpp
extern CToolMenuDLL theDLL;


static struct SpecialInfoStruct {
	char *description;
} gSpecialInfo[] = {
	{
		"Do nothing"
	},
	{
		"I just take up menu space!"
	}
};

/*****************************************************************************/
/* TYEPDEFS */


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
	if (pluginInfo)
	{
		pluginInfo->numTrans = 0;
		pluginInfo->numAttachers = 0;
		pluginInfo->numSpecials = kNumSpecial;
		
		pluginInfo->desc = strdup(kPluginDescription);
		
		DoIconInit(-1, &(pluginInfo->icon));
		
		pluginInfo->id = kPluginID;
	}
	
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_info: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_special_info(
    void * globals,              /* Out: Return for allocated instance structure */
    emsMenuP specialMenu         /* Out: The menu */
)
{
	SpecialInfoStruct *InfoPtr = NULL;

	if ( (specialMenu->id <= 0) || (specialMenu->id > kNumSpecial) )
		return (EMSR_INVALID_TRANS);

	InfoPtr = gSpecialInfo + (specialMenu->id - 1);
	specialMenu->desc = strdup(InfoPtr->description);
	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_special_hook: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_special_hook(
    void * globals,             /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* In: The menu */
)
{
	if (!specialMenu) 
		return (EMSR_UNKNOWN_FAIL);
	
	if (specialMenu->id == kSpecialNothing)
	{
		// Here's where to hook the code
	}
	else if(specialMenu->id == kMoreSpecialMoreNothing)
	{
		// Here's where to hook the code
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
	if (mem)
		safefree(mem);

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    long trans_id,         /* ID of translator to get info for */
    HICON ** icon          /* Return for icon data */
)
{
	if (!icon)
		return;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());


	if (trans_id < 0) /* Main plugin icon, not specific translator */
	{
		*icon = (HICON *)malloc(sizeof(HICON));
		**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else
		*icon = NULL;
}
