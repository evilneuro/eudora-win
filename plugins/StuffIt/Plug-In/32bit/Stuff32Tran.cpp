/*
 *  Aladdin Stuffit Plugin (32-bit version)
 *
 *  Filename: AttachTran.cpp
 *
 *  Last edited: Monday, August 25, 1997
 *
 *  Author: Scott Manjourides
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

/*
This is an automatic response to return unique ID numbers for plugins
created for Eudora using the EMSAPI. If you want more information about
creating plugins you can write to <mailto:emsapi-info@qualcomm.com> or 
visit our EMS API web site at <http://www.eudora.com/developers/emsapi/>.

Here are your IDs, as requested.  There are five so you have some extra just
in case.  If you lose track of the five here, you can always get a few more.
The sole purpose of these ID's is to make sure each plugin has a unique 
identifier.

Plugin IDs:

     1330
     1331
     1332
     1333
     1334
*/

#ifdef _DEBUG
#include <ctype.h>
#endif // _DEBUG

#include <afxwin.h> // MFC & Windows master header
#include <afxdlgs.h>
#include <cderr.h> // Common dialog error returns
#include <windows.h>

#include <malloc.h> // malloc, free
#include <string.h> // strdup

#include <stdio.h>
#include <string.h>

#include "resource.h" // Resource IDs for dialogs and icons

#include "ems-win.h" // The EMS API
#include "Stuff32.h" 

#include "AladdinProgDlg.h" /* Progress dialog that does Aladdin Engine stuff */

/* The keyword 'bool' is used throughout the Stuffit engine, so let's ignore the warning. */
#pragma warning(disable:4237) /* warning C4237: nonstandard extension used : 'bool' keyword is reserved for future use */

#include "Engine.h"

using namespace StuffItEngine;

/*****************************************************************************/
/* CONSTANTS */

#ifdef _COMMERCIAL_VER
	static const int kPluginID = 1330;
	#define _COMPRESS_
	#define _EXPAND_
#else // _FREE_VER
	#ifdef _FREE_VER
		static const int kPluginID = 1331;
		#define _EXPAND_
	#else // _OEM_VER
		static const int kPluginID = 1332;
		#define _COMPRESS_
	#endif
#endif

static const unsigned long kPluginIconID = IDI_MAIN;

/*****************************************************************************/
/* GLOBALS */

// This externs into Stuff32.cpp
extern CStuffDLL theDLL;

/*	We don't know what formats the Stuffit engine will support, so we test
	at runtime for each format. The ones that work, we add to the menu.

    This applies to compression only, as the engine decides for itself
	what it can uncompress. Could list all types from EngineType.h, but
	we only check the main ones we are interested in.
*/
static struct gArchiveInfo_tag {
	UINT menuTextID;
	UINT fileTitleID;
	engineFormat engVal;
} gArchiveInfo[] = {
	{ IDS_STUFFIT_MENU_STR, IDS_STUFFIT_TITLE_STR, engineFormatStuffIt },
	{ IDS_ZIP_MENU_STR, IDS_ZIP_TITLE_STR, engineFormatZip },
	{ IDS_ARC_MENU_STR, IDS_ARC_TITLE_STR, engineFormatArc }
};

static const int gMaxEngCount = 3; /* Number of types we have listed above */

static int gValidCount = 0; /* This will increase at runtime if any compression types are supported */
static int gValidArr[gMaxEngCount] = { -1 };

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); p=NULL; } }

/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

// Generalized functions
void DoIconInit(long trans_id, HICON FAR*FAR* trans_icon);

/*****************************************************************************/

/*
   We need a dialog to prompt the user for a directory when decompressing. There is no
   common control or dialog which does this, so roll your own. We take the standard
   non-explorer file selection dialog, play around with the controls and use it
   as a directory-only selection.

   This is done via subclassing CFileDialog to provide a clean, usable interface to
   our exotic dialog.
*/

// Needed to get the control IDs for the the controls to hide
#include <dlgs.h>

class CDirDialog : public CFileDialog
{
// Construction
public:
	/* Pass in a path to indicate the initial selection */
	CDirDialog(const char *pStartDir, CWnd* pParentWnd);

// Attributes
public:
		
// Operations
public:
	BOOL OnInitDialog();
	CString GetDirStr();

// Implementation
protected:
	/* These methods are used to muck around with the existing controls */
	void StretchControl(int nID1, int nID2);
	void HideControl(int nID);

	/* These are here in case someone needs them */
	CString m_Title;
	CString m_StartDir;
	CString m_Filter;
};

CDirDialog::CDirDialog(const char *pStartDir, CWnd* pParentWnd = NULL) :
	CFileDialog(FALSE, NULL, "A", OFN_HIDEREADONLY, NULL, pParentWnd)
{
#ifdef WIN32
	m_ofn.Flags &= ~ ( OFN_SHOWHELP | OFN_EXPLORER );
#else
	m_ofn.Flags &= ~OFN_SHOWHELP;
#endif
	
	m_Title.LoadString(IDS_FILE_DIR_TITLE);
	m_ofn.lpstrTitle = m_Title;
		
	m_StartDir = CString(pStartDir);
	m_ofn.lpstrInitialDir = m_StartDir;
	
	m_Filter.LoadString(IDS_ALL_FILES_FILTER);
	for (char* s = m_Filter.GetBuffer(1); *s; s++)
	{
		if (*s == '|')
			*s = 0;
	}
	m_ofn.lpstrFilter = m_Filter;
}

void CDirDialog::StretchControl(int nID1, int nID2)
{
	// nID2 will be stretched left to cover nID1

	// What this means is the left edge of nID2 will be moved to be the the
	// left edge of ID1.

	CWnd* pWnd = NULL;
	RECT rect1, rect2;

	pWnd = GetDlgItem(nID1);
	pWnd->GetWindowRect(&rect1);
	ScreenToClient(&rect1);

	pWnd = GetDlgItem(nID2);
	pWnd->GetWindowRect(&rect2);
	ScreenToClient(&rect2);

	pWnd->SetWindowPos(NULL,
						rect1.left,
						rect2.top,
						rect2.right - rect1.left,
						rect2.bottom - rect2.top,
						SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER );
}

void CDirDialog::HideControl(int nID)
{
	// Hide the control, if possible
	CWnd* pWnd = NULL;

	pWnd = GetDlgItem(nID);
	if (pWnd)
		pWnd->ShowWindow(SW_HIDE);
}

/* Return the selected directory after user is done. */
CString CDirDialog::GetDirStr()
{
	return (GetPathName().GetBufferSetLength(m_ofn.nFileOffset - 1));
}

BOOL CDirDialog::OnInitDialog()
{
	if (!CFileDialog::OnInitDialog())
		return (FALSE);

	/* !! NOTE: Microsoft screwed up, the "Folders" text has an ID of 0xFFFF !! */

	// Stretch the useful controls over the useless ones
	StretchControl(stc3, 0xFFFF); // Move the "Folders" text
	StretchControl(edt1, stc1);   // Move the selected directory text
	StretchControl(lst1, lst2);   // Move the directory list box
	StretchControl(stc2, stc4);   // Move the "Drives" text
	StretchControl(cmb1, cmb2);   // Move the drives combo box

	// Hide all the controls we don't need
	HideControl(stc3);
	HideControl(edt1);
	HideControl(lst1);
	HideControl(stc2);
	HideControl(cmb1);

	CString tmp;

	// Change text of OK button
	tmp.LoadString(IDS_EXTRACT_HERE);
	SetDlgItemText(IDOK, tmp);

	// Change the static text above the selection display
	tmp.LoadString(IDS_FOLDER_TXT); // Replace "Folders" with "Destination"
	SetDlgItemText(0xFFFF, tmp);

#ifdef NEVER_DO_THIS
	{
		// This code was used to walk through all the controls and get their IDs. This
		// is how I found 0xFFFF to be the mystery "Folders" text.
		CWnd *pWnd = GetTopWindow();
		int nID;

		while (pWnd)
		{
			nID = pWnd->GetDlgCtrlID();
			pWnd = pWnd->GetNextWindow();
		}
	}
#endif

	return (TRUE);
}

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

#ifdef _COMPRESS_

	if (gValidCount < 1)
	{
		gValidCount = 0;
		
		for (int i = 0; i < gMaxEngCount; i++)
		{
			archiveWriter writer = newArchiveWriter();

			if (canWrite(gArchiveInfo[i].engVal, writer))
			{
				gValidArr[gValidCount++] = i;
			}

		    deleteArchiveWriter(writer);
		}
	}
#endif // _COMPRESS_

	if (pluginInfo)
	{
		pluginInfo->numTrans = 0;
#ifdef _EXPAND_
		pluginInfo->numSpecials = 1;
#else
		pluginInfo->numSpecials = 0;
#endif // _EXPAND_

		pluginInfo->numAttachers = gValidCount;

		CString sTitle;
		CString sVer;

		sTitle.LoadString(AFX_IDS_APP_TITLE);
		sVer.LoadString(IDS_VERSION_STR);

#ifdef _FREE_VER
		{
			CString sAppendix;
			sAppendix.LoadString(IDS_FREEWARE_APPENDIX);
			sTitle = sTitle + " " + sAppendix;
		}
#endif

		pluginInfo->desc = strdup(((const char *)CString(sTitle + ", " + sVer)));

		DoIconInit(-1, &(pluginInfo->icon));

		pluginInfo->id = kPluginID;
	}

	return (EMSR_OK);
}

/* ----- Info about special menu items hooks ------------------------------- */
extern "C" long WINAPI ems_special_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu         /* Out: The menu */
)
{
#ifdef _EXPAND_
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (specialMenu->id != 1)
		return (EMSR_INVALID_TRANS);

	CString str;
	str.LoadString(IDS_DECOMPRESS_MENU_TXT);

	specialMenu->desc = strdup((const char *) str);
#endif // _EXPAND_

	return (EMSR_OK);
}

/* ----- Call a special menu item hook ------------------------------------- */
extern "C" long WINAPI ems_special_hook(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* In: The menu */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#ifdef _EXPAND_

	if ((!specialMenu) || (specialMenu->id != 1))
		return (EMSR_UNKNOWN_FAIL);

	const DWORD dwFlags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_FILEMUSTEXIST | OFN_EXPLORER;
//	const LPCTSTR lpszFilter = "Compressed Files (*.sit;*.zip)|*.sit;*.zip|Stuffit Files (*.sit)|*.sit|Zip Files (*.zip)|*.zip|All Files (*.*)|*.*||";

	CString sFilter;
	sFilter.LoadString(IDS_EXPAND_FILTER);

	CString sTitle;
	sTitle.LoadString(IDS_FILE_OPEN_TITLE);

	CFileDialog opendlg(TRUE, NULL, NULL, dwFlags, (LPCTSTR) sFilter, NULL);
	opendlg.m_ofn.lpstrTitle = (LPCSTR) sTitle;

	// Add copyright to bottom of file select dialog
		LPCSTR DlgName;
		opendlg.m_ofn.Flags |= OFN_ENABLETEMPLATE;
		DlgName = MAKEINTRESOURCE(IDD_OPENDLG_EXT);
		opendlg.m_ofn.lpTemplateName = DlgName;
		opendlg.m_ofn.hInstance = AfxFindResourceHandle( MAKEINTRESOURCE(IDD_OPENDLG_EXT), RT_DIALOG);

	CString archive_filename;
	if (opendlg.DoModal() == IDOK)
	{
		archive_filename = opendlg.GetPathName();

		CDirDialog savedlg((const char *)archive_filename, AfxGetMainWnd());
		if (savedlg.DoModal() == IDOK)
		{
			CString archive_destdir = savedlg.GetDirStr(); // returns "C:\blah\blah" (no trailing slash)

			// Create the archive
			archiveReader reader = newArchiveReader();
			if (!openArchive((const char *) archive_filename, reader))
			{
				CString engineErr = getEngineErrorString(getReaderError(reader));
				
				if ((engineErr.Left(11)) == CString("engineError"))
					engineErr = engineErr.Right(engineErr.GetLength() - 11);

				CString displayErr;
				AfxFormatString1(displayErr, IDS_ERR_OPENING, engineErr);
				AfxMessageBox((const char *)displayErr, MB_OK | MB_ICONEXCLAMATION);

				return (EMSR_OK);
			}

			setReaderDestination((const char *) archive_destdir, reader);

			// Setup the progress dialog to do the expanding
			CAladdinProgDlg	progDlg;
			progDlg.Create(IDD_ALADDIN_PROGRESS_DLG);
			progDlg.ShowWindow(SW_SHOW);
			progDlg.UpdateWindow();
			bool bSuccess = progDlg.DoExpandArchive(reader);
			progDlg.DestroyWindow();

			if (!bSuccess)
			{
				CString engineErr = getEngineErrorString(getReaderError(reader));
				
				if ((engineErr.Left(11)) == CString("engineError"))
					engineErr = engineErr.Right(engineErr.GetLength() - 11);

				CString displayErr;
				AfxFormatString1(displayErr, IDS_ERR_DECODING, engineErr);
				AfxMessageBox((const char *)displayErr, MB_OK | MB_ICONEXCLAMATION);

				return (EMSR_OK);
			}
		}
	}

#endif // _EXPAND_

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
#ifdef _COMPRESS_
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ( (attachMenu->id <= 0) || (attachMenu->id > gValidCount) ) // IDs are 1-based, bleach!
		return (EMSR_INVALID_TRANS);

	CString sDesc;
	sDesc.LoadString(gArchiveInfo[gValidArr[(attachMenu->id - 1)]].menuTextID);

	attachMenu->desc = strdup(((const char *)sDesc));
#endif

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
    LPSTR attachDir,            /* In: Location to put attachments */
    long * numAttach,           /* Out: Number of files attached */
    emsDataFileP ** attachFiles /* Out: Name of files written */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#ifdef _COMPRESS_

	long id = attachMenu->id;
	unsigned int idx;

	if ( (id <= 0) || (id > gValidCount) ) // IDs are 1-based, bleach!
		return (EMSR_INVALID_TRANS);

	if ((!attachFiles) || (!attachMenu))
		return (EMSR_UNKNOWN_FAIL);

	int ret = 0;
	CFileDialog *pFileDlg = NULL;

	const DWORD dwFlags = OFN_ALLOWMULTISELECT
							| OFN_HIDEREADONLY
							// | OFN_SHAREAWARE
							| OFN_FILEMUSTEXIST
							| OFN_EXPLORER;

	const LPCTSTR lpszFilter = "All Files (*.*)|*.*||";

	CString sTitle;
	sTitle.LoadString(gArchiveInfo[gValidArr[(id - 1)]].fileTitleID);

	const DWORD nBufSize = 4096;
	char pTheBuf[nBufSize] = {0};

	do
	{
		if (pFileDlg)
		{
			delete (pFileDlg);
			pFileDlg = NULL;
		}

		// Create the COpenMultiFileDlg object, initialized
		pFileDlg = new CFileDialog(FALSE, NULL, NULL, dwFlags, lpszFilter, NULL);
		ASSERT(pFileDlg);

		pFileDlg->m_ofn.lpstrTitle = (LPCSTR) sTitle;

		pFileDlg->m_ofn.lpstrFile = (LPTSTR) pTheBuf;
		pFileDlg->m_ofn.nMaxFile = (DWORD) nBufSize;

		LPCSTR DlgName;
		
		pFileDlg->m_ofn.Flags |= OFN_ENABLETEMPLATE;
		DlgName = MAKEINTRESOURCE(IDD_OPENDLG_EXT);
		pFileDlg->m_ofn.lpTemplateName = DlgName;
		pFileDlg->m_ofn.hInstance = AfxFindResourceHandle( MAKEINTRESOURCE(IDD_OPENDLG_EXT), RT_DIALOG);

		ret = pFileDlg->DoModal();

		if (ret != IDOK)
		{
			DWORD err = CommDlgExtendedError();

			if (err == 0)
			{
				return (EMSR_UNKNOWN_FAIL); // CLICKED 'CANCEL'
			}
			else // (err > 0)
			{
				UINT nErrID = 0;

				/* Check all the errors, just to be safe. Display some appropriate text */
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

					case FNERR_BUFFERTOOSMALL:
					{
						// The buffer pointed to by the lpstrFile member of the OPENFILENAME
						// structure is too small for the filename specified by the user. The
						// first two bytes of the lpstrFile buffer contain an integer value
						// specifying the size, in bytes (ANSI version) or characters
						// (Unicode version), required to receive the full name. 
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

	// ----- Process the request

	CWaitCursor waiting;

	// Count the files
	UINT nFileCount = 0;
	POSITION pos = pFileDlg->GetStartPosition();
	while (pos)
	{
		pFileDlg->GetNextPathName(pos);
		(nFileCount)++;
	}

	// Allocate room for the list
    const char** fileList;
    fileList = (const char**)malloc((nFileCount + 1) * sizeof(const char*));
	ASSERT(fileList);

	// Build the list
	pos = pFileDlg->GetStartPosition();
	for (idx = 0; idx < nFileCount; idx++)
		fileList[idx] = strdup(pFileDlg->GetNextPathName(pos));

    fileList[nFileCount] = engineFileListEnd;

	if (pFileDlg)
	{
		delete (pFileDlg);
		pFileDlg = NULL;
	}

	// Create the archive
	archiveWriter writer = newArchiveWriter();

	//	setWriterArchiveName("C:\\BLAH.SIT", writer);
    setWriterArchiveFormat(gArchiveInfo[gValidArr[(id - 1)]].engVal, writer);

	CAladdinProgDlg	progDlg;
	
	//progDlg.DoModal();
	progDlg.Create(IDD_ALADDIN_PROGRESS_DLG);
	progDlg.ShowWindow(SW_SHOW);
	progDlg.UpdateWindow();
	progDlg.DoCreateArchive(fileList, writer);
	progDlg.DestroyWindow();

	// -- Don't need this, the dialog does all the work --
	//
	// bSuccess = createArchive(fileList, writer); // Do the work
    // ASSERT(bSuccess);


	const char *pWriterName = getWriterArchiveName(writer);
	ASSERT(pWriterName);

	*numAttach = 1;

	// Allocate the number of pointers
	*attachFiles = (emsDataFileP *) malloc( sizeof(emsDataFileP) * (*numAttach));
	ASSERT(*attachFiles);

	(*attachFiles)[0] = (emsDataFileP) malloc(sizeof(emsDataFile));
	ASSERT((*attachFiles)[0]);

	(*attachFiles)[0]->size = sizeof(emsDataFile);
	(*attachFiles)[0]->fileName = strdup(pWriterName);
	ASSERT((*attachFiles)[0]->fileName);

    deleteArchiveWriter(writer);

	for (idx = 0; idx < nFileCount; idx++)
		free((char *)fileList[idx]);
	free(fileList);

#endif // _COMPRESS_

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
    void FAR* mem               /* Memory to free */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (mem)
		safefree(mem);

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    long trans_id,         /* ID of translator to get info for */
    HICON FAR*FAR* icon   /* Return for icon data */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!icon)
		return;

	if (trans_id < 0) /* Main plugin icon, not specific translator */
	{
		*icon = (HICON *)malloc(sizeof(HICON));
		**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else
		*icon = NULL;
}
