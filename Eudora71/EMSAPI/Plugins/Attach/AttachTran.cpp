/*
 *  Sample EMS plugin: Attach
 *
 *  Filename: AttachTran.cpp
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
#include "attach.h" 


/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 21;
static const unsigned long kPluginIconID = IDI_MAIN;
static const int kNumAttach = 2;

static const int kAttachOneID   = 1;
static const int kAttachManyID  = 2;

/*****************************************************************************/
/* GLOBALS */

// This externs into Attach.cpp
extern CAttachDLL theDLL;

static UINT gAttachDescIds[] = {
	IDS_SINGLEFILE_STR, // kAttachOneID
	IDS_MULTIFILE_STR // kAttachManyID
};

static UINT gAttachIconIds[] = {
	IDI_ATTACH_ONE, // kAttachOneID
	IDI_ATTACH_MANY // kAttachManyID
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

	if (pluginInfo)
	{
		pluginInfo->numTrans = 0;
		pluginInfo->numSpecials = 0;
		pluginInfo->numAttachers = kNumAttach;
		
		CString sTitle;
		CString sVer;

		sTitle.LoadString(AFX_IDS_APP_TITLE);
		sVer.LoadString(IDS_VERSION_STR);

		pluginInfo->desc = strdup(((const char *)CString(sTitle + ", " + sVer)));
		
		DoIconInit(-1, &(pluginInfo->icon));
		
		pluginInfo->id = kPluginID;
	}
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_attacher_info: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_attacher_info(
    void * globals,             /* Out: Return for allocated instance structure */
    emsMenuP attachMenu         /* Out: The menu */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ( (attachMenu->id <= 0) || (attachMenu->id > kNumAttach) )
		return (EMSR_INVALID_TRANS);

	CString sDesc;
	sDesc.LoadString(gAttachDescIds[(attachMenu->id - 1)]);

	attachMenu->desc = strdup(((const char *)sDesc));

	attachMenu->flags= EMSF_TOOLBAR_PRESENCE;
		
	DoIconInit(gAttachIconIds[(attachMenu->id - 1)], &(attachMenu->icon));
	
	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_attacher_hook: Get details about a attacher in a plugin
*
\*****/

extern "C" long WINAPI ems_attacher_hook(
    void * globals,             /* Out: Return for allocated instance structure */
    emsMenuP attachMenu,        /* In: The menu */
    LPSTR attachDir,            /* In: Location to put attachments */
    long * numAttach,           /* Out: Number of files attached */
    emsDataFileP ** attachFiles /* Out: Name of files written */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((!attachMenu) || ((attachMenu->id != kAttachOneID) && (attachMenu->id != kAttachManyID)))
		return (EMSR_UNKNOWN_FAIL);

	BOOL bOpenFileDialog = TRUE; // "OPEN"
	LPCTSTR lpszDefExt = NULL;
	LPCTSTR lpszFileName = NULL;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_FILEMUSTEXIST;
	LPCTSTR lpszFilter = "All Files (*.*)|*.*||";
	CWnd* pParentWnd = NULL;
	CString lpstrTitle;
	CFileDialog *pFileDlg = NULL;

	// Setup the initialization variables
	if (attachMenu->id == kAttachManyID)
		dwFlags |= OFN_ALLOWMULTISELECT;

	// Change the dialog title
	if (attachMenu->id == kAttachOneID)
	{
		lpstrTitle.LoadString(IDS_SINGLETITLE_STR); // "Attach File"
	}
	else // (attachMenu->id == kAttachManyID)
	{
		lpstrTitle.LoadString(IDS_MULTIPLETITLE_STR); // "Attach Multiple Files"
	}

	int ret = 0;

	do
	{
		// Create the CFileDialog object, initialized
		pFileDlg = new CFileDialog( bOpenFileDialog,
								lpszDefExt,
								lpszFileName,
								dwFlags,
								lpszFilter,
								pParentWnd );
		ASSERT(pFileDlg);

		pFileDlg->m_ofn.lpstrTitle = ((const char *)lpstrTitle);
		ret = pFileDlg->DoModal();

		if (ret != IDOK)
		{
			delete (pFileDlg);
			pFileDlg = NULL;

			DWORD err = CommDlgExtendedError();

			if (err == 0)
			{
				return (EMSR_UNKNOWN_FAIL); // CLICKED 'CANCEL'
			}
			else // (err > 0)
			{
				UINT nErrID = 0;

				switch (err)
				{
					// MEMORY ERRORS
					case CDERR_INITIALIZATION: // The common dialog box function failed during initialization. This error often occurs when sufficient memory is not available.
					case CDERR_MEMALLOCFAILURE: // The common dialog box function was unable to allocate memory for internal structures.
					case CDERR_MEMLOCKFAILURE: // The common dialog box function was unable to lock the memory associated with a handle.
					{
						nErrID = IDS_MEMORYERR_STR;
					}
					break;

					// RESOURCE ERRORS
					case CDERR_FINDRESFAILURE: // The common dialog box function failed to find a specified resource.
					case CDERR_LOADRESFAILURE: // The common dialog box function failed to load a specified resource.
					case CDERR_LOADSTRFAILURE: // The common dialog box function failed to load a specified string.
					case CDERR_LOCKRESFAILURE: // The common dialog box function failed to lock a specified resource.
					{
						nErrID = IDS_RESOURCEERR_STR;
					}
					break;

					case FNERR_BUFFERTOOSMALL: // The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the filename specified by the user. The first two bytes of the lpstrFile buffer contain an integer value specifying the size, in bytes (ANSI version) or characters (Unicode version), required to receive the full name. 
					{
						nErrID = IDS_TOOMANYFILESERR_STR;				
					}
					break;
					
					case FNERR_INVALIDFILENAME: // A filename is invalid.
					{
						nErrID = IDS_INVALIDFILEERR_STR;
					}
					break;

					// GENERAL INTERNAL ERRORS
					case CDERR_DIALOGFAILURE: // The dialog box could not be created. The common dialog box function's call to the DialogBox function failed. For example, this error occurs if the common dialog box call specifies an invalid window handle.
					case CDERR_NOHINSTANCE: // The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding instance handle.
					case CDERR_NOHOOK: // The ENABLEHOOK flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a pointer to a corresponding hook procedure.
					case CDERR_NOTEMPLATE: // The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding template.
					case CDERR_REGISTERMSGFAIL: // The RegisterWindowMessage function returned an error code when it was called by the common dialog box function.
					case CDERR_STRUCTSIZE: // The lStructSize member of the initialization structure for the corresponding common dialog box is invalid.
					case FNERR_SUBCLASSFAILURE: // An attempt to subclass a list box failed because sufficient memory was not available.
					
					default:
					{
						nErrID = IDS_INTERNALERR_STR;				
					}
					break;
				}

				if (AfxMessageBox( nErrID, MB_ICONEXCLAMATION | MB_RETRYCANCEL ) == IDCANCEL)
					return (EMSR_UNKNOWN_FAIL);
			}
		}
	} while (ret != IDOK);

	// Process the selected files(s)
	if (attachMenu->id == kAttachOneID)
	{
		*numAttach = 1;
		
		// Allocate the number of pointers
		*attachFiles = (emsDataFileP *) malloc( sizeof(emsDataFileP) * *numAttach);

		// Here's the data
		(*attachFiles)[0] = (emsDataFileP) malloc( sizeof(emsDataFile));
		(*attachFiles)[0]->size = sizeof(emsDataFile);
		(*attachFiles)[0]->fileName = strdup(pFileDlg->GetPathName());
	}
	else // (attachMenu->id == kAttachManyID)
	{
		*numAttach = 0;
		POSITION pos = pFileDlg->GetStartPosition();
		while (pos)
		{
			pFileDlg->GetNextPathName(pos);
			(*numAttach)++;
		}
		pos = pFileDlg->GetStartPosition();
		
		// Allocate the number of pointers
		*attachFiles = (emsDataFileP *) malloc( sizeof(emsDataFileP) * *numAttach);

		for (int i = 0; i < *numAttach; i++)
		{
			(*attachFiles)[i] = (emsDataFileP)malloc(sizeof(emsDataFile));
			(*attachFiles)[i]->size = sizeof(emsDataFile);
			(*attachFiles)[i]->fileName = strdup(pFileDlg->GetNextPathName(pos));

		}
	}
	
	delete (pFileDlg);

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
    HICON ** icon   /* Return for icon data */
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
