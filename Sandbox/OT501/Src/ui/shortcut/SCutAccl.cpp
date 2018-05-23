// 
// Stingray Software Extension Classes
// Copyright (C) 1996 Jim Beveridge
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:			Jim Beveridge
//  Description:	Apply shortcuts classes - Definitions
//  Created:		Sun Jan 28 10:10:29 1996
//

// stdafx.h for SEC includes secall.h
#include "stdafx.h"

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL   
#ifdef WIN32

#ifndef __SCUTACCL_H__
#include "SCutAccl.h"
#endif

#ifndef __SCUTCMDS_H__
#include "SCutCmds.h"
#endif

#ifdef _SECDLL
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif //_SECDLL

#include "secres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

HACCEL SECShortcutTable::mg_hOriginalAccelTable;


SECShortcutTable::SECShortcutTable()
{
	LoadFromFrame();
}

SECShortcutTable::~SECShortcutTable()
{
}


/*******************************************************************************
 *
 *		SECShortcutTable::ApplyAcceleratorsToMenus
 *
 * Desc:	Walk all menus in the mainframe and in the templates.
 *			Call ApplyAcceleratorsToMenu for each menu.
 *
 * Returns:	TRUE on success.
 */
BOOL SECShortcutTable::ApplyAcceleratorsToMenus()
{
	CFrameWnd* pFrame = (CFrameWnd*)::AfxGetMainWnd();
	ASSERT_VALID(pFrame);

	// Strictly speaking, this is legal, but for the purposes of debugging,
	// applying an empty accelerator table is not a terribly useful thing
	// to do.
	ASSERT(GetSize() > 0);
	
	ApplyAcceleratorsToMenu(pFrame->m_hMenuDefault);
	
	CWinApp* pApp = AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	while (pos) {
		CDocTemplate* pTemplate = pApp->GetNextDocTemplate( pos );
		if ( pTemplate->IsKindOf( RUNTIME_CLASS( CMultiDocTemplate ) ) ) {
			ApplyAcceleratorsToMenu(((CMultiDocTemplate*)pTemplate)->m_hMenuShared);
		}
	}
	return TRUE;
}


/*******************************************************************************
 *
 *		SECShortcutTable::ApplyAcceleratorsToMenu
 *
 * Desc:	Update the text in the menu with this accel table.
 *
 * Returns:	Nothing.
 */
void SECShortcutTable::ApplyAcceleratorsToMenu( HMENU hBarMenu )
{
	CMenu popupMenu;
	popupMenu.Attach(hBarMenu);

	TCHAR szItem[128];
	CString itemDesc;
	int size = GetSize();

	int popupCount = popupMenu.GetMenuItemCount();
	for (int popupIndex = 0; popupIndex<popupCount; popupIndex++) {

		if (!(popupMenu.GetMenuString(popupIndex, szItem, sizeof(szItem)/sizeof(TCHAR), MF_BYPOSITION)))
			continue;
		
		UINT nID = popupMenu.GetMenuItemID(popupIndex);

		// If the ID is -1, we found a submenu.  Recurse on it.
		if (nID == -1) {
			ApplyAcceleratorsToMenu( GetSubMenu(popupMenu, popupIndex) );
			continue;
		}

		// Find this command ID in our accelerator table.  If it exists, use it.
		for (int i = 0;  i < size; i++) {
			if (ElementAt(i).cmd == nID) {
				DWORD hotkey;
				AccelToHotKey(ElementAt(i), &hotkey);
				::GetHotKeyDescription(hotkey, itemDesc);
				break;
			}
		}
		
		// If the loop falls through, then the menu item's
		// accelerator is erased.
		UpdateMenuItem(szItem, itemDesc);
		itemDesc = _T("");	// Much cheaper than SetEmpty()
		popupMenu.ModifyMenu( nID, MF_BYCOMMAND | MF_STRING, nID, szItem );
	}
	
	popupMenu.Detach();
}



/*******************************************************************************
 *
 *		SECShortcutTable::UpdateMenuItem
 *
 * Desc:	Add (or replace) the accelerator description at the end of the
 *			current menu string.
 *
 * Returns:	Nothing.
 */
void SECShortcutTable::UpdateMenuItem(LPTSTR szItem, const CString& desc)
{
	/*
	 * Append the new accelerator text to the
	 * menu-item text, or clear the current text if all accels were deleted.
	 */
	LPTSTR pch = _tcschr(szItem, _T('\t'));
	if (pch)
		*pch = _T('\0');
	else
		pch = szItem;
	if (!desc.IsEmpty()) {
		_tcscat(pch, _T("\t"));
		_tcscat(pch, desc);
	}
}


/*************************************************************************
 *
 *		SECShortcutTable::AssignAccel
 *
 * Desc:	Translate the given hotkey to an accelerator structure,
 *			then either replace the current entry or add a new one,
 *			depending on whether that accelerator is already used.
 *
 * Returns:	void
 *
 */
void SECShortcutTable::AssignAccel(DWORD hotkey, UINT id)
{
	ACCEL newAccel;
	newAccel.cmd = (WORD) id;
	HotKeyToAccel(hotkey, &newAccel);

	for (int i = 0;  i < GetSize(); i++) {
		// HotKeyToAccel always includes kStandardAcceleratorFlags,
		// so we will too for the sake of comparison.
		if ( ( ElementAt(i).fVirt | kStandardAcceleratorFlags ) == newAccel.fVirt
					&& ElementAt(i).key == newAccel.key ) {
	      ElementAt(i) = newAccel;
			break;
		}
	}
	if (i == GetSize())		// Not found
		Add(newAccel);
}


/*******************************************************************************
 *
 *		SECShortcutTable::Load
 *
 * Desc:	Load a user defined accelerator table.  This default
 *			implementation loads an archive called MACROS.DAT
 *			that is searched for first in the Windows directory,
 *			then in the application's directory.
 *
 *			This function can be overriden anyway you like, as long
 *			as the accelerator array has been loaded when you are done.
 *
 * Returns:	TRUE if a user-defined table was loaded.
 *			FALSE otherwise.
 */

// VIRTUAL
BOOL SECShortcutTable::Load()
{
	CString filename;

	// Try and load from the file given by GetDataFileName
	try {
		CFile fd;
		CFileException e;	// not auto-deleting
		GetDataFileName( filename, ALTERNATE_NAME );
		if (!fd.Open( filename, CFile::modeRead, &e )) {
			GetDataFileName( filename, MAIN_NAME );
			if (!fd.Open( filename, CFile::modeRead, &e ))
				if (e.m_cause != CFileException::fileNotFound)
					AfxThrowFileException(e.m_cause, e.m_lOsError);
				else
					return FALSE;	// Normal - happens if no table has been saved
		}
		CArchive ar(&fd, CArchive::load);
		DWORD tag;
		ar >> tag;
		if (kShortcutTableFileTag != tag)
			AfxThrowArchiveException( CArchiveException::badClass );	// File was not written by Shortcut
		WORD version;
		ar >> version;
		if (kShortcutTableVersion != version)
			AfxThrowArchiveException( CArchiveException::badClass );	// obsolete table
		Serialize(ar);
		
		return TRUE;
	}
	catch( CFileException* e ) {
		// Tell the user in a locale-specific manner
		LPTSTR lpszMessageBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
							 | FORMAT_MESSAGE_FROM_SYSTEM,
											NULL,
											e->m_lOsError,
											LANG_NEUTRAL,
											(LPTSTR)&lpszMessageBuf,
											0,
											NULL );
		CString err;
		//AAB AfxFormatString2(err, IDS_SHORTCUT_READ_ERROR, filename, lpszMessageBuf);
		//AAB AfxMessageBox(err);
		LocalFree(lpszMessageBuf);

		// Clean up MFC
		e->Delete();
	}
	catch( CArchiveException* /* e */ ) {
		LoadDefaultTable();
		// Really need to display an error here, but MFC
		// does not provide a locale-sensitive way of doing it.
	}

	return FALSE;
}


/*******************************************************************************
 *
 *		SECShortcutTable::Save
 *
 * Desc:	Write the accelerator table to an archive.
 *
 * Returns:	TRUE if save was successful.
 */
BOOL SECShortcutTable::Save()
{
	CString filename;

	try {
		CFile fd;
		GetDataFileName( filename, MAIN_NAME );
		CFileException e;
		if (!fd.Open( filename, CFile::modeCreate | CFile::shareExclusive | CFile::modeWrite, &e )) {
			GetDataFileName( filename, ALTERNATE_NAME );
			if (!fd.Open( filename, CFile::modeCreate | CFile::shareExclusive | CFile::modeWrite, &e ))
				AfxThrowFileException(e.m_cause, e.m_lOsError);
		}
		
		CArchive ar(&fd, CArchive::store);
		// Write an ID tag and a version #.
		ar << kShortcutTableFileTag;
		ar << kShortcutTableVersion;
		Serialize(ar);
		return TRUE;
	}
	catch( CFileException* e ) {
		// Tell the user in a locale-specific manner
		LPTSTR lpszMessageBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
							 | FORMAT_MESSAGE_FROM_SYSTEM,
											NULL,
											e->m_lOsError,
											LANG_NEUTRAL,
											(LPTSTR)&lpszMessageBuf,
											0,
											NULL );
		CString err;
		//AAB AfxFormatString2(err, IDS_SHORTCUT_WRITE_ERROR, filename, lpszMessageBuf);
		//AAB AfxMessageBox(err);
		LocalFree(lpszMessageBuf);

		// Clean up MFC
		e->Delete();
	}
	catch( CArchiveException* /* e */ ) {
		// Really need to display an error here, but MFC
		// does not provide a locale-sensitive way of doing it.
	}

	return FALSE;
}


/*******************************************************************************
 *
 *		SECShortcutTable::LoadFromFrame
 *
 * Desc:	Load the accelerator table from the mainframe if possible
 *
 * Returns:	TRUE if a table was found in the mainframe and loaded
 *			FALSE otherwise
 *
 *			This function can throw a CMemoryException
 */

BOOL SECShortcutTable::LoadFromFrame()
{
	CFrameWnd* pFrame = (CFrameWnd*)::AfxGetMainWnd();
	ASSERT_VALID(pFrame);

	HACCEL hFrameTable = pFrame->m_hAccelTable;
	if (!pFrame || !pFrame->m_hAccelTable) {
		SetSize(0);	// Clear out the table
		return FALSE;
	}

	// Save it for ResetAll
	if (!mg_hOriginalAccelTable)
		mg_hOriginalAccelTable = hFrameTable;

	/*
	 * Count the number of entries in the current
	 * table, allocate a buffer for the table, and
	 * then copy the table into the buffer.
	 */
	int cAccelerators = ::CopyAcceleratorTable( hFrameTable, NULL, 0 );

	// Strictly speaking, this is legal, but for the purposes of debugging something
	// terrible probably happened.
	ASSERT(cAccelerators > 0);

	SetSize(cAccelerators);
   ::CopyAcceleratorTable( hFrameTable, &ElementAt(0), cAccelerators );

	return TRUE;
}


/*******************************************************************************
 *
 *		SECShortcutTable::Apply
 *
 * Desc:	Load the accelerator table into the mainframe
 *
 * Returns:	TRUE if the locally contained table was successfully
 *			created and loaded.
 *			FALSE otherwise.
 */
BOOL SECShortcutTable::Apply()
{
	CFrameWnd* pFrame = (CFrameWnd*) ::AfxGetMainWnd();
	if (!pFrame)
		return FALSE;

	HACCEL newTable = ::CreateAcceleratorTable(&ElementAt(0), GetSize());
	if ( newTable ) {
		// Keep the original lying around so we can do a ResetAll.
		if ( pFrame->m_hAccelTable != mg_hOriginalAccelTable )
			DestroyAcceleratorTable( pFrame->m_hAccelTable );
		pFrame->m_hAccelTable = newTable;
	}
	else {
		return FALSE;
	}
	
	ApplyAcceleratorsToMenus();

	return TRUE;
}


/*******************************************************************************
 *
 *		SECShortcutTable::GetDataFileName
 *
 * Desc:	Return the full path and name to the data file.
 *			By default, the file is written to where the
 *			app lives.
 *
 * Returns:	path is returned in str.  Original value of str
 *			is overwritten.
 *
 * Notes:	This function is MBCS and Unicode aware.
 */
void SECShortcutTable::GetDataFileName(CString& str, ENameType type)
{
	TCHAR path[MAX_PATH];
	DWORD rc = GetModuleFileName( NULL , path, MAX_PATH );
	LPTSTR c = _tcsrchr(path, _T('.'));
	c = _tcsinc(c);
	// MAC is a hardcoded extension for the macro file.
	_tcscpy(c, _T("MAC") );
	str = path;

	if (type == MAIN_NAME)
		return;

	// Extract the name, no path
	c = _tcsrchr(path, _T('\\'));
	str = c?c:path;

	// Put that name in the Windows directory.
	if (GetWindowsDirectory(path, MAX_PATH)) {
		str = CString(path) + str;
	}
}


/*******************************************************************************
 *
 *		SECShortcutTable::LoadDefaultTable
 *
 * Desc:	Using the original table that was saved when the mainframe
 *			was initialized by us, load the default accel table.
 *
 * Returns:	Nothing
 */
void SECShortcutTable::LoadDefaultTable()
{
	// Something blew up during app setup.
	ASSERT(mg_hOriginalAccelTable != NULL);

	/*
	 * The static member mg_hOriginalAccelTable
	 * was created the first time we looked at
	 * the mainframe's accelerator table.  Unless
	 * we keep a copy of it, there is no easy way
	 * to ever get it back again because we do not
	 * know its resource id and can't figure it out.
	 */
	int cAccelerators = ::CopyAcceleratorTable(
							mg_hOriginalAccelTable, NULL, 0 );
	SetSize(cAccelerators);
	::CopyAcceleratorTable( mg_hOriginalAccelTable, &ElementAt(0), cAccelerators );
}


/*******************************************************************************
 *
 *		SECShortcutTable::LoadDefaultTable
 *
 * Desc:	This default implementation just re-loads the default
 *			table.  We do not update the mainframe or the data
 *			file at this point so that the user can hit "Cancel"
 *			in the dialog and everything will still work.
 *
 * Returns:	Nothing
 */
void SECShortcutTable::ResetAll()
{
	LoadDefaultTable();
}
       
#endif //WIN32