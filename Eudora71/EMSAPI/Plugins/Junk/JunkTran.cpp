/*
 *  Sample EMS plugin: Junk
 *
 *  Filename: JunkTran.cpp
 *
 *  Copyright 2003 QUALCOMM, Incorporated
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
#include "Junk.h" 

#include <malloc.h> // malloc, free
#include <string.h> // strdup

#include <windows.h>
#include "emsapi-win.h" // The EMS API

#include <afxdlgs.h>


//	Constants
static const int				kPluginID = 29;
static const char *				kPluginDescription = "Junk Sample Plugin, v1.0";
static const char *				kTransDescription = "Junk Sample";
static const char *				kSpecialMenuItem = "Junk Sample Menu Item";

static const int				kJunkTransID	= 1;
static const int				kNumTrans		= 1;

static const int				kSpecialNothing	= 1;
static const int				kNumSpecial		= 1;


//	Globals
//	The one and only CJunkDLL object
CJunkDLL theDLL("Junk.DLL");


void DoIconInit(UINT iconID, HICON ** trans_icon);


// ---------------------------------------------------------------------------
//		* ems_plugin_version
// ---------------------------------------------------------------------------
//	Get the version of the API used for this plugin

extern "C" long WINAPI
ems_plugin_version(
	short *				api_version)		// Place to return api version
{
	*api_version = EMS_VERSION;
	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_plugin_init
// ---------------------------------------------------------------------------
//	Initialize plugin and get its basic info

extern "C" long WINAPI
ems_plugin_init(
	void **				globals,			// Out: Return for allocated instance structure
	short				eudAPIVersion,		// In: The API version eudora is using
	emsMailConfigP		mailConfig,			// In: Eudoras mail configuration
	emsPluginInfoP		pluginInfo)			// Out: Return Plugin Information
{
	if (pluginInfo)
	{
		pluginInfo->numTrans = kNumTrans;
		pluginInfo->numAttachers = 0;
		pluginInfo->numSpecials = kNumSpecial;
		
		pluginInfo->desc = strdup(kPluginDescription);
		
		DoIconInit(IDI_MAIN, &(pluginInfo->icon));
		
		pluginInfo->id = kPluginID;
	}
	
	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_plugin_config
// ---------------------------------------------------------------------------
//	Call the plug-ins configuration Interface

extern "C" long WINAPI
ems_plugin_config(
	void *				globals,		// In: Allocated instance structure returned in ems_plugin_init
	emsMailConfigP		mailConfig)		// In: Eudora mail info
{
	// Display your settings dialog here
	// Remove this code and the associated definition in *.def if no settings
	// are needed.

	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_translator_info
// ---------------------------------------------------------------------------
//	Get details about a translator in a plugin

extern "C" long WINAPI
ems_translator_info(
	void *				globals,			// Out: Return for allocated instance structure
	emsTranslatorP		transInfo)			// In/Out: Return Translator Information
{
	if (!transInfo)
		return EMSR_UNKNOWN_FAIL;
	
	if (transInfo->id != kJunkTransID)
		return EMSR_INVALID_TRANS;

	transInfo->type = EMST_NO_TYPE;

	// Set up to be called for junk scoring with all header information
	// Also use EMSF_PREFER_PLAIN if you'd prefer HTML messages to be stripped - more efficient when off
	transInfo->flags = EMSF_JUNK_MAIL | EMSF_BASIC_HEADERS | EMSF_ALL_HEADERS;

	transInfo->desc = strdup(kTransDescription);

	DoIconInit(IDI_JUNK, &transInfo->icon);

	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_score_junk
// ---------------------------------------------------------------------------
//	Called to score junk

extern "C" long WINAPI
ems_score_junk(
	void *					globals,		// In: Allocated instance structure returned in ems_plugin_init
	emsTranslatorP			trans,			// In: Translator Info
	emsJunkInfoP			junkInfo,		// In: Junk information
	emsMessageInfoP			message,		// In: Message to score
	emsJunkScoreP			junkScore,		// In/Out: Junk score
	emsResultStatusP		status)			// Out: Status information
{
	//	Test context bitwise, because more flags could be added in the future
	if (junkInfo->context & EMSFJUNK_SCORE_ON_ARRIVAL)
	{
		// We're scoring on arrival.
	}
	else if (junkInfo->context & EMSFJUNK_RESCORE)
	{
		// We're rescoring. Could be user initiated or could be done by Eudora for other reasons.
		
		// The previous junk score is provided in junkScore.
		// The plugin ID associated with the junk score is in junkInfo.pluginID.
		// If the plugin ID is 0, then the score was user assigned. A user assigned
		// score of 0 means that the message was previously considered not junk; a user
		// assigned score other than 0 means that the score was previously considered junk
		// (when the user marks a message as junk a user settable value is assigned - the
		// default is 100, but you can't count on the value being 100, just non-zero).
	}

	// For this example we're scoring outside of the above tests for on arrival vs. rescoring
	// because we don't care. To keep things extra simple we'll look for two strings in a case
	// sensitive fashion. One string will indicate that the message is spam, and the other will
	// indicate that the message should be white listed (the two test strings chosen are the same
	// length so that we can be extra lazy). If neither string is found the message will simply be
	// scored as 0 (not junk).
	//
	// Please note that normal scoring should be from 0 to 100 (inclusive) where 0 indicates
	// that the message is absolutely not junk and 100 indicates that the message is absolutely
	// junk. White listing (junk score of -1) indicates that the message is absolutely not
	// junk and that junk scores from any other plugins the user has installed should be ignored.
	// White listing should be rare. Many plugins won't need white listing functionality at all.
	// If in doubt don't use white listing.
	static const char *		szIsSpamString = "This message is SPAM!";
	static const char *		szWhiteListString = "This message is safe!";
	long					nTestStringLength = strlen(szIsSpamString);
	const char *			pcLastPossibleMatchStart = message->textSize - nTestStringLength + message->text;
	const char *			pcScan = message->text;

	// Start out with a junk score of 0 (not junk)
	junkScore->score = 0;

	while (pcScan <= pcLastPossibleMatchStart)
	{
		if (memcmp(pcScan, szIsSpamString, nTestStringLength) == 0)
		{
			junkScore->score = 100;
			break;
		}
		
		if (memcmp(pcScan, szWhiteListString, nTestStringLength) == 0)
		{
			junkScore->score = -1;
			break;
		}

		++pcScan;
	}

	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_user_mark_junk
// ---------------------------------------------------------------------------
//	Called to mark a message as junk or not junk. Allows your
//	plugin to be trained by the user.

extern "C" long WINAPI
ems_user_mark_junk(
	void *					globals,		// In: Allocated instance structure returned in ems_plugin_init
	emsTranslatorP			trans,			// In: Translator Info
	emsJunkInfoP			junkInfo,		// In: Junk information
	emsMessageInfoP			message,		// In: Message that is either being marked as Junk or Not Junk
	emsJunkScoreP			junkScore,		// In: Previous junk score
	emsResultStatusP		status)			// Out: Status information
{
	// The previous junk score is provided in junkScore.
	// The plugin ID associated with the junk score is in junkInfo.pluginID.
	// If the plugin ID is 0, then the score was user assigned. A user assigned
	// score of 0 means that the message was previously considered not junk; a user
	// assigned score other than 0 means that the score was previously considered junk
	// (when the user marks a message as junk a user settable value is assigned - the
	// default is 100, but you can't count on the value being 100, just non-zero).

	//	Test context bitwise, because more flags could be added in the future
	if (junkInfo->context & EMSFJUNK_MARK_IS_JUNK)
	{
		// User has marked this message as junk.
		// If the plugin ID is 0 (i.e. user assigned) and junkScore.score is non-zero,
		// then the message was already marked as junk. We might want to skip processing.
		// On the other hand we could have been installed after the user marked this
		// message as junk, so maybe we don't want to skip processing.
	}
	else if (junkInfo->context & EMSFJUNK_MARK_NOT_JUNK)
	{
		// User has marked this message as NOT junk.
		// If the plugin ID is 0 (i.e. user assigned) and junkScore.score is zero,
		// then there are several possiblities:
		// * Message was already marked as not junk by user
		// * No installed plugins when message was scored
		// * All installed plugins returned 0 when message was scored
	}
	
	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_special_info
// ---------------------------------------------------------------------------
//	Get details about a attacher in a plugin

extern "C" long WINAPI
ems_special_info(
	void *				globals,			// In: Allocated instance structure returned in ems_plugin_init
	emsMenuP			specialMenu)		// Out: The menu
{
	if ( (specialMenu->id <= 0) || (specialMenu->id > kNumSpecial) )
		return EMSR_INVALID_TRANS;

	// We currently only have one special menu item, but we could
	// do the same thing with additional menu items.
	specialMenu->desc = strdup(kSpecialMenuItem);

	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* ems_special_hook
// ---------------------------------------------------------------------------
//	Act when action is chosen

extern "C" long WINAPI
ems_special_hook(
	void *				globals,			// In: Allocated instance structure returned in ems_plugin_init
	emsMenuP			specialMenu)		// In: The menu
{
	if (!specialMenu) 
		return EMSR_UNKNOWN_FAIL;
	
	if (specialMenu->id == kSpecialNothing)
	{
		// Here's where to hook the code
	}

	return EMSR_OK;
	
}


// ---------------------------------------------------------------------------
//		* ems_plugin_finish
// ---------------------------------------------------------------------------
//	End use of a plugin and clean up

extern "C" long WINAPI
ems_plugin_finish(
	void *				globals)			// In: Allocated instance structure returned in ems_plugin_init
{
	// We don't use 'globals'
	return EMSR_OK; 
}


// ---------------------------------------------------------------------------
//		* ems_free
// ---------------------------------------------------------------------------
//	Free memory allocated by EMS plug-in

extern "C" long WINAPI
ems_free(
	void *				mem)				// Memory to free
{
	free(mem);

	return EMSR_OK;
}


// ---------------------------------------------------------------------------
//		* DoIconInit
// ---------------------------------------------------------------------------
//	Allocates memory and loads specified icon.

static void
DoIconInit(
	UINT				iconID,				// ID of icon to load
	HICON **			icon)				// Return for icon data
{
	if (!icon)
		return;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*icon = (HICON *)malloc(sizeof(HICON));
	**icon = theDLL.LoadIcon(iconID);
}
