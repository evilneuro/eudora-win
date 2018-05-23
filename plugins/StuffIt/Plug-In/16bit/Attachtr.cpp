/*
 *  Aladdin Stuffit attacher (16-bit)
 *
 *  Filename: Attach16.cpp
 *
 *  Last edited: <still in beta>
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


 */

/* One of the following MUST be defined -- ONLY ONE */
/* Pro is both attacher and expander */

// #define _ATTACHER_ONLY_VER_
// #define _EXPANDER_ONLY_VER_
#define _PRO_VER_

/* Logfile is used for debugging */

#define _LOGFILE_ON_
#define LOGFILENAME ("STUFF.LOG")

/* These IDs are available for use by this plug-in */

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

/* Now for the good stuff */

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
#include "ems-win.h" // The EMS API
#include "attach.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>

#include "Engine.h"

/* VC++ 1.52 doesn't support namespaces */
//using namespace StuffItEngine;

/*****************************************************************************/
/* CONSTANTS */

// For each version, define what it does: compress or expand or both
// Also pick a different plug-in ID for each build

#ifdef _PRO_VER_
	static const int kPluginID = 1330;
	#define _COMPRESS_
	#define _EXPAND_
#else // _EXPANDER_ONLY_VER_
	#ifdef _EXPANDER_ONLY_VER_
		static const int kPluginID = 1331;
		#define _EXPAND_
	#else // _ATTACHER_ONLY_VER_
		static const int kPluginID = 1332;
		#define _COMPRESS_
	#endif
#endif

static const unsigned long kPluginIconID = IDI_MAIN;

/*****************************************************************************/
/* GLOBALS */

// This externs into Attach.cpp
extern CAttachDLL theDLL;

/*	We don't know what formats the Stuffit engine will support, so we test
	at runtime for each format. The ones that work, we add to the menu.

    This applies to compression only, as the engine decides for itself
	what it can uncompress. Could list all types from EngineType.h, but
	we only check the main ones we are interested in.
*/
static struct _tag {
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

/* Thanks go to Julia Blumin for the hacked file select dlg */

// Needed to get the control IDs for the the controls of common dlgs
#include <dlgs.h>

/* We need a file-select dialog which has the Aladdin copyright notice on
   window. The 16-bit world seems extra mean today, so we have to do
   straight GDI stuff to the window -- yuck.
*/

class  CStuffyDlg : public CFileDialog
{  
private:
	CStatic		m_CopyRightCrap;
// Construction
public:
	CStuffyDlg( BOOL bOpenFileDialog, 
						LPCSTR lpszDefExt, 
						LPCSTR lpszFileName, 
						DWORD dwFlags, 
						LPCSTR lpszFilter,
						CWnd* pParentWnd);


// Operations
public:
	BOOL OnInitDialog();
};


CStuffyDlg::CStuffyDlg( BOOL bOpenFileDialog, 
						LPCSTR lpszDefExt, 
						LPCSTR lpszFileName, 
						DWORD dwFlags, 
						LPCSTR lpszFilter, 
						CWnd* pParentWnd) :
							CFileDialog(bOpenFileDialog,
									lpszDefExt,
									"",
									dwFlags,
									lpszFilter,
									pParentWnd)

{
}

BOOL CStuffyDlg::OnInitDialog()
{   
	BOOL ret = CFileDialog::OnInitDialog();  
	
	// Now Draw in the darn'ed Copyright stuff
	CRect ClientRect, WindowRect, rect;
	CWnd* Combo = GetDlgItem(cmb1);
	const DWORD DBU = ::GetDialogBaseUnits();
	const int xDLU = LOWORD(DBU);
	const int yDLU = HIWORD(DBU);
	    		
	GetClientRect(&ClientRect);
	GetWindowRect(&WindowRect);
			
	if (Combo)
	{
		Combo->GetWindowRect(&rect);
		ScreenToClient(&rect);
	}
	else
		rect.left = xDLU;
	rect.top = ClientRect.bottom;
	rect.bottom = rect.top + yDLU * 2;  
	rect.right = ClientRect.right;
	
	CString copyRightStr;
	copyRightStr.LoadString(IDS_COPYRIGHT_STR);
	m_CopyRightCrap.Create(copyRightStr,WS_CHILD | WS_VISIBLE, rect, this);
	
	WindowRect.bottom += yDLU * 2 + yDLU / 2;
	MoveWindow(&WindowRect, FALSE);
	return ret;
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

HWND *g_eudoraWnd = NULL;

extern "C" long WINAPI ems_plugin_init(
    void FAR*FAR* globals,          /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	/* eudoraWnd doesn't seem to ever be valid, may be a bug in EMS API */
	g_eudoraWnd = mailConfig->eudoraWnd;
	
	/* If we're going to compress, we need to know what formats
	   the library supports. So walk through our list and
	   build a list of supported formats. These supported formats
	   will show up in the 'attach' menu */

#ifdef _COMPRESS_
	if (gValidCount < 1)
	{
		/* Global counter of how many valid formats we can attach */
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

#ifdef _EXPANDER_ONLY_VER_
		{
			CString sAppendix;
			sAppendix.LoadString(IDS_FREEWARE_APPENDIX);
			sTitle = sTitle + " " + sAppendix;
		}
#endif // _EXPANDER_ONLY_VER_

		pluginInfo->desc = strdup(((const char *)CString(sTitle + ", " + sVer)));
		
		DoIconInit(-1, &(pluginInfo->icon));
		
		pluginInfo->id = kPluginID;
	}
	
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


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

/* Getting the correct handle isn't all that easy */

//	HWND mainWnd = GetActiveWindow();	
//	CWnd *pMainWnd = CWnd::FromHandle(mainWnd);
//	CWnd *pMainWnd = CWnd::FromHandle(*g_eudoraWnd);
	CWnd* pMainWnd = AfxGetMainWnd();

	ASSERT(pMainWnd);

	if ((!specialMenu) || (specialMenu->id != 1))
		return (EMSR_UNKNOWN_FAIL);

	const DWORD dwFlags = OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_FILEMUSTEXIST;

	CString sFilter;
	sFilter.LoadString(IDS_EXPAND_FILTER);

	CString sTitle;
	sTitle.LoadString(IDS_FILE_OPEN_TITLE);

	CStuffyDlg opendlg(TRUE, NULL, NULL, dwFlags, (LPCTSTR) sFilter, pMainWnd);
	opendlg.m_ofn.lpstrTitle = (LPCSTR) sTitle;

	CString archive_filename;
	CString archive_destdir;

	if (opendlg.DoModal() == IDOK)
	{
		/* Set the hourglass cursor */
		HCURSOR hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

		archive_filename = opendlg.GetPathName();

		int npos = archive_filename.ReverseFind('\\');
		if (npos > -1)
		{
			int nRight = npos + 1;
			int nLeft = archive_filename.GetLength() - nRight;

			archive_destdir = archive_filename.Left(nRight);
			archive_filename = archive_filename.Right(nLeft);
		}
		else
		{
			archive_destdir = archive_filename;
		}

		// Create the archive
		archiveReader reader = newArchiveReader();
		if (!openArchive((const char *) archive_filename, reader))
		{
			CString engineErr = getEngineErrorString(getReaderError(reader));
				
			if ((engineErr.Left(11)) == CString("engineError"))
				engineErr = engineErr.Right(engineErr.GetLength() - 11);

			CString displayErr;
			AfxFormatString1(displayErr, IDS_ERR_OPENING, engineErr);

			/* Restore the previous cursor. */
			SetCursor(hcurSave);

			AfxMessageBox((const char *)displayErr, MB_OK | MB_ICONEXCLAMATION);

			return (EMSR_OK);
		}

		setReaderDestination((const char *) archive_destdir, reader);

		bool bSuccess = decodeArchive(reader); // Do the work

		/* Restore the previous cursor. */
		SetCursor(hcurSave);

		if (!bSuccess)
		{
			CString engineErr = getEngineErrorStringLong(getReaderError(reader));

			if ((engineErr.Left(11)) == CString("engineError"))
				engineErr = engineErr.Right(engineErr.GetLength() - 11);

			CString displayErr;
			AfxFormatString1(displayErr, IDS_ERR_DECODING, engineErr);
			AfxMessageBox((const char *)displayErr, MB_OK | MB_ICONEXCLAMATION);

			return (EMSR_OK);
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

	if ( (attachMenu->id <= 0) || (attachMenu->id > gValidCount) ) // IDs are 1-based, yuck!
		return (EMSR_INVALID_TRANS);

	CString sDesc;
	sDesc.LoadString(gArchiveInfo[gValidArr[(attachMenu->id - 1)]].menuTextID);

	attachMenu->desc = strdup(((const char *)sDesc));
#endif // _COMPRESS_

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_attacher_hook: Do the attaching
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
#ifdef _COMPRESS_


#ifdef _LOGFILE_ON_
	fstream logfile(LOGFILENAME, ios::out | ios::ate);

	{
		// CTime cannot be used inside DLLs -- lets hear it for Microsoft!
		char dbuffer [9], tbuffer [9];
		_strdate( dbuffer );
		_strtime( tbuffer );

		logfile << "\n==================== ems_attacher_hook ("
			<< dbuffer
			<< " @ "
			<< tbuffer
			<< ") ====================\n";
		logfile.flush();
    }

#endif // _LOGFILE_ON_

	FILE *fp = NULL;      

	HWND activeWnd = GetActiveWindow();
	CWnd *pActiveWnd = CWnd::FromHandle(activeWnd);
//	CWnd *pMainWnd = CWnd::FromHandle(*g_eudoraWnd);
	CWnd* pMainWnd = AfxGetMainWnd();

#ifdef _LOGFILE_ON_
{
	char tmp[64];

	sprintf(tmp, "activeWnd = %p\n", activeWnd);
	logfile << tmp; logfile.flush();

	sprintf(tmp, "pMainWnd = %p\n", pMainWnd);
	logfile << tmp; logfile.flush();

	sprintf(tmp, "g_eudoraWnd = %p\n", g_eudoraWnd);
	logfile << tmp; logfile.flush();
}
#endif // _LOGFILE_ON_

  	ASSERT(pActiveWnd);

#ifdef _LOGFILE_ON_
	logfile << "AFX_MANAGE_STATE...\n"; logfile.flush();
#endif // _LOGFILE_ON_

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
 
	long id = attachMenu->id;

#ifdef _LOGFILE_ON_
	logfile << "id = " << id << "\n"; logfile.flush();
#endif // _LOGFILE_ON_

	if ( (id <= 0) || (id > gValidCount) ) // IDs are 1-based, bleach!
		return (EMSR_INVALID_TRANS);

	if ((!attachFiles) || (!attachMenu))
		return (EMSR_UNKNOWN_FAIL);

	int ret = 0;
	CStuffyDlg *pFileDlg = NULL; // (CStuffyDlg)

	const DWORD dwFlags = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_FILEMUSTEXIST;
	const LPCTSTR lpszFilter = "All Files (*.*)|*.*||";  

	CString sTitle = "";
	sTitle.LoadString(gArchiveInfo[gValidArr[(id - 1)]].fileTitleID);

#ifdef _LOGFILE_ON_
	logfile << "sTitle = '" << sTitle << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	const DWORD nBufSize = 4096;
	char FAR *pTheBuf = (char FAR *) _fmalloc(sizeof(char) * nBufSize);
	ASSERT(pTheBuf);

	memset(pTheBuf, '\0', nBufSize);

#ifdef _LOGFILE_ON_
	logfile << "pTheBuf allocated.\n"; logfile.flush();
#endif // _LOGFILE_ON_

	do
	{
		if (pFileDlg)
		{
			delete (pFileDlg);
			pFileDlg = NULL;
		}

#ifdef _LOGFILE_ON_
	logfile << "CStuffyDlg...\n"; logfile.flush();
#endif // _LOGFILE_ON_

		// Create the CFileDialog object, initialized (CStuffyDlg)
		pFileDlg = new CStuffyDlg(TRUE, NULL, NULL, dwFlags, lpszFilter, pActiveWnd);
		ASSERT(pFileDlg);

		pFileDlg->m_ofn.lpstrTitle = (LPCSTR) sTitle;

		pFileDlg->m_ofn.lpstrFile = (LPTSTR) pTheBuf;
		pFileDlg->m_ofn.nMaxFile = (DWORD) nBufSize; 

#ifdef _LOGFILE_ON_
	logfile << "CStuffyDlg domodal....\n"; logfile.flush();
#endif // _LOGFILE_ON_

		ret = pFileDlg->DoModal();

		if (ret != IDOK)
		{
#ifdef _LOGFILE_ON_
	logfile << "ret != IDOK\n"; logfile.flush();
#endif // _LOGFILE_ON_

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
					case CDERR_DIALOGFAILURE: // The dialog box could not be created. The common dialog box function's call to the DialogBox function failed. For example, this error occurs if the common dialog box call specifies an invalid window hand
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

#ifdef _LOGFILE_ON_
	logfile << "ret == IDOK\n"; logfile.flush();
#endif // _LOGFILE_ON_

	// ----- Process the request

	/* Set the hourglass cursor */
	HCURSOR hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

	// --------------- Begin Aladdin Engine Junk ---------------

#ifdef _LOGFILE_ON_
	logfile << "pFileDlg->m_ofn.lpstrFile = '" << pFileDlg->m_ofn.lpstrFile << "'\n"; logfile.flush();
	logfile << "pTheBuf = '" << pTheBuf << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	char pBuf[4096];

	// pFileDlg->m_ofn.lpstrFile is READ-ONLY memory, so make a copy we can play with
	strcpy(pBuf, pFileDlg->m_ofn.lpstrFile);

//	strcpy(pBuf, (const char *)(pFileDlg->GetPathName())); // Single-select only
//	strcpy(pBuf, pTheBuf); // Nice try, but doesn't work

	safefree(pTheBuf);

	if (pFileDlg)
	{
		delete (pFileDlg);
		pFileDlg = NULL;
	}

#ifdef _LOGFILE_ON_
	logfile << "pBuf = '" << pBuf << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	unsigned int len = strlen(pBuf);

#ifdef _LOGFILE_ON_
	logfile << "len = " << len << "\n"; logfile.flush();
#endif // _LOGFILE_ON_

	if (len < 1)
	{
		AfxMessageBox( IDS_INTERNALERR_STR, MB_ICONEXCLAMATION | MB_OK );
		return (EMSR_UNKNOWN_FAIL);
    }

	// First, count the files
	unsigned int nFileCount = 0;
	char FAR *cp = pBuf;

#ifdef _LOGFILE_ON_
	logfile << "cp = '" << cp << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	while (cp = strchr(cp, ' '))
	{
#ifdef _LOGFILE_ON_
	logfile << "cp = '" << cp << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

		nFileCount++;
		cp++;
	}

	if (nFileCount < 1)
	{
#ifdef _LOGFILE_ON_
	logfile << "** nFileCount < 1 **\n"; logfile.flush();
#endif // _LOGFILE_ON_

		nFileCount = 1;
		cp = strrchr(pBuf, '\\');
		ASSERT(cp);

		*cp = ' '; // Insert space to simulate multi-file (one file)
	}

#ifdef _LOGFILE_ON_
	logfile << "nFileCount = " << nFileCount << "\n"; logfile.flush();
#endif // _LOGFILE_ON_

	// Now ready the buffer
    char FAR **fileList = NULL;
    fileList = (char**) malloc((nFileCount + 1) * sizeof(const char*));
	ASSERT(fileList);

#ifdef _LOGFILE_ON_
	logfile << "fileList allocated.\n"; logfile.flush();
#endif // _LOGFILE_ON_

	// Now fill the buffer (list)	
	cp = strchr(pBuf, ' ');
	ASSERT(cp);
	
#ifdef _LOGFILE_ON_
	logfile << "cp = '" << cp << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	// For multiple files selected at root dir, the path ends in a slash -- remove it!
	if ((*(cp - 1)) == '\\')
		*(cp - 1) = '\0';
	else
		*cp = '\0';
	
#ifdef _LOGFILE_ON_
	logfile << "cp = '" << cp << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	cp++; // Move past the space

#ifdef _LOGFILE_ON_
	logfile << "cp = '" << cp << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

	const char FAR *pPath = pBuf;
	char FAR *cp2;
	unsigned int pathlen = strlen(pPath);
	unsigned int inx = 0;

	do {
		cp2 = strchr(cp, ' ');
		if (cp2)
			*cp2 = '\0';

		fileList[inx] = (char FAR *) _fmalloc(pathlen + strlen(cp) + 2); // 2 = slash + NULL
		ASSERT(fileList[inx]);
		
		_fstrcpy(fileList[inx], pPath);
		_fstrcat(fileList[inx], "\\");
		_fstrcat(fileList[inx], cp);

#ifdef _LOGFILE_ON_
	logfile << "fileList[" << inx << "] = '" << fileList[inx] << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

  		inx++;

		cp = cp2;
		if (cp)
			cp++;
	} while (cp);

    fileList[nFileCount] = (char *) engineFileListEnd;

#ifdef _LOGFILE_ON_
	logfile << "fileList[" << nFileCount << "] = engineFileListEnd\n"; logfile.flush();
#endif // _LOGFILE_ON_

    // Do the stuffit

#ifdef _LOGFILE_ON_
	logfile << "Creating writer...\n"; logfile.flush();
#endif // _LOGFILE_ON_

	// Create the archive
	archiveWriter writer = NULL;
	writer = newArchiveWriter();
	ASSERT(writer);

#ifdef _LOGFILE_ON_
	logfile << "setting writer format...\n"; logfile.flush();
#endif // _LOGFILE_ON_

    setWriterArchiveFormat(gArchiveInfo[gValidArr[(id - 1)]].engVal, writer);

    bool bSuccess = FALSE;

#ifdef _LOGFILE_ON_
	logfile << "creating archive...\n"; logfile.flush();
#endif // _LOGFILE_ON_

	bSuccess = createArchive((const char FAR **)fileList, writer); // Do the work
	
	if (!bSuccess)
	{
		// Oh no, Mr. Bill!!!! (something went wrong)
		CString engineErr = getEngineErrorStringLong(getWriterError(writer));

		if ((engineErr.Left(11)) == CString("engineError"))
			engineErr = engineErr.Right(engineErr.GetLength() - 11);

		CString displayErr;
		AfxFormatString1(displayErr, IDS_ERR_CREATING, engineErr);
		AfxMessageBox( ((const char *)displayErr), MB_OK | MB_ICONEXCLAMATION);

#ifdef _LOGFILE_ON_
	logfile << "archive failure: '" << engineErr << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_
	}
	else
	{
#ifdef _LOGFILE_ON_
	logfile << "archive success.\n"; logfile.flush();
#endif // _LOGFILE_ON_

		const char *pWriterName = getWriterArchiveName(writer);
//		const char *pWriterName = "temp.zip";
		ASSERT(pWriterName);

#ifdef _LOGFILE_ON_
	logfile << "pWriterName = '" << pWriterName << "'\n"; logfile.flush();
#endif // _LOGFILE_ON_

		*numAttach = 1;

		// Allocate the number of pointers
		*attachFiles = (emsDataFileP *) malloc( sizeof(emsDataFileP) * (*numAttach));
		ASSERT(*attachFiles);

		(*attachFiles)[0] = (emsDataFileP) malloc(sizeof(emsDataFile));
		ASSERT((*attachFiles)[0]);

		(*attachFiles)[0]->size = sizeof(emsDataFile);
		(*attachFiles)[0]->fileName = strdup(pWriterName);
		ASSERT((*attachFiles)[0]->fileName);
	}

	// Cleanup
    deleteArchiveWriter(writer);

    {
    	for (int inx = 0; fileList[inx] != engineFileListEnd; inx++)
    		safefree(fileList[inx]);

		safefree(fileList);
	}

	/* Restore the previous cursor. */
	SetCursor(hcurSave);

#ifdef _LOGFILE_ON_
	{
		// CTime cannot be used inside DLLs -- lets hear it for Microsoft!
		char dbuffer [9], tbuffer [9];
		_strdate( dbuffer );
		_strtime( tbuffer );

		logfile << "\n==================== DONE ("
			<< dbuffer
			<< " @ "
			<< tbuffer
			<< ") ====================\n";
		logfile.flush();
    }

	logfile.close();
#endif // _LOGFILE_ON_

#endif // _COMPRESS_
	
	return (bSuccess ? (EMSR_OK) : (EMSR_UNKNOWN_FAIL));
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
		*icon = (HICON *) malloc(sizeof(HICON));
		**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else
		*icon = NULL;
}
