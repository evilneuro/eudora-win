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
//  Description:	Command description classes - Definitions
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


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW




/////////////////////////////////////////////////////////
//
//	SECCommand
//
/////////////////////////////////////////////////////////

SECCommand::SECCommand()
: m_nID(0)
{
}

SECCommand::SECCommand(UINT nID)
: m_nID(nID)
{
}


/////////////////////////////////////////////////////////
//
//	SECCommandList
//
/////////////////////////////////////////////////////////

SECCommandList::SECCommandList()
{
	// By default, disallow unmodified & shifted keys (they turn to CTRL by default)
	SetRules(HKCOMB_NONE|HKCOMB_S, HOTKEYF_CONTROL);

	// This data usually gets throw away, but we only
	// get here when the dialog is about to come up
	// and the elapsed real time is negligible.
	// Without this it is real hard to figure out
	// if the user forgot to set us up properly.
	DeriveDefaults(); 
}

// VIRTUAL
SECCommandList::~SECCommandList()
{
}


void SECCommandList::DeriveDefaults()
{
	// Allocate array in large chunks.
	SetSize(0,25);

	BOOL rc = AddDefaultIds();
	rc = FillEmpties();

	// Shrink to fit, decrease chunk size
	SetSize(GetSize(), 5);
}


void SECCommandList::ClearCommandIds()
{
	RemoveAll();
}

void SECCommandList::AddCommandIds(const SECDefaultCommandId ids[], int count)
{
	SetSize(GetSize() + count, 5);

	for (int i=0; i<count; i++) {
		SECCommand& cmd = ElementAt(i);
		cmd.m_nID = ids[i].m_nID;
		if (ids[i].m_nName)
			cmd.m_strName.LoadString(ids[i].m_nName);
		if (ids[i].m_nDescription)
			cmd.m_strDescription.LoadString(ids[i].m_nDescription);
	}

	FillEmpties();
}


/*******************************************************************************
*
*		SECCommandList::AddDefaultIds
*
* Desc:	If no other handler is available for AddDefaultIds,
*		then we try and load a reasonable set of default ID's
*		based on the menu system.
*/

// VIRTUAL
BOOL SECCommandList::AddDefaultIds()
{
	CFrameWnd* pMainFrame = (CFrameWnd*)::AfxGetMainWnd();
	ASSERT_VALID(pMainFrame);
	
	LoadIdsFromMenu(pMainFrame->m_hMenuDefault);
	LoadIdsFromAccelTable();
	
	CWinApp* pApp = AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	while (pos) {
		CDocTemplate* pTemplate = pApp->GetNextDocTemplate( pos );
		if ( pTemplate->IsKindOf( RUNTIME_CLASS( CMultiDocTemplate ) ) ) {
			LoadIdsFromMenu(((CMultiDocTemplate*)pTemplate)->m_hMenuShared);
		}
	}
	return TRUE;
}


/*******************************************************************************
 *
 *		SECCommandList::QueryExcludeId
 *
 * Desc:	Certain command ID's, such as those on the
 *			current window list, should never show up
 *			in the Names list.  This virtual function
 *			allows the developer to decide which should
 *			be left out.
 *
 *			Overriding next and previous pane doesn't work.
 *
 * Returns: TRUE if the id should be excluded
 *			FALSE if the id should be included
 */

//VIRTUAL
BOOL SECCommandList::QueryExcludeId(UINT nID)
{
	return ((nID >= ID_FILE_MRU_FIRST && nID <= ID_FILE_MRU_LAST)
		|| (nID >= AFX_IDM_FIRST_MDICHILD && nID <= AFX_IDM_FIRST_MDICHILD+255)
		|| nID == ID_NEXT_PANE || nID == ID_PREV_PANE );
}


/*******************************************************************************
 *
 *		SECCommandList::LoadIdsFromMenu
 *
 * Desc:	Given a handle to a menu, try and add each
 *			command ID for all the menu items.  This
 *			routine is normally used by AddDefaultIds
 *			when no other table is supplied.  It typically
 *			does not make sense to use this routine
 *			from anywhere else.
 */
void SECCommandList::LoadIdsFromMenu(HMENU hMenu)
{
	CMenu menuBar;
	menuBar.Attach(hMenu);
	int barCount = menuBar.GetMenuItemCount();
	
	for (int barIndex = 0; barIndex<barCount; barIndex++) {
		UINT menuItemID = menuBar.GetMenuItemID(barIndex);
		if (menuItemID == 0)		// Separator
			continue;
		if (menuItemID == -1)	// Submenu
		{
			LoadIdsFromMenu( GetSubMenu(menuBar, barIndex) );
			continue;
		}
		if (QueryExcludeId(menuItemID))
			continue;
		if (Find(menuItemID) < 0) {
			SECCommand cmd(menuItemID);
			Add(cmd);
			}
	}
	menuBar.Detach();
}


/*******************************************************************************
 *
 *		SECCommandList::LoadIdsFromAccelTable
 *
 * Desc:	The accelerator table contains a variety
 *			of ids that the menus do not, such as
 *			Next MDI Window and Prev MDI Window
 *			(as well as next and previous pane if
 *			you are using splitters)
 */
void SECCommandList::LoadIdsFromAccelTable()
{
	SECShortcutTable shortcuts;
	shortcuts.LoadDefaultTable();

	int popupCount = shortcuts.GetSize();
	for (int index = 0; index<popupCount; index++) {
		UINT cmd = shortcuts[index].cmd;
		if (cmd == -1 || cmd == 0)
			continue;
		if (QueryExcludeId(cmd))
			continue;
		if (Find(cmd) < 0) {
			SECCommand command(cmd);
			Add(command);
			}
	}
}


/*******************************************************************************
 *
 *		SECCommandList::FillEmpties
 *
 * Desc:	At this point all of the command ids
 *			have been filled in, but some portion
 *			of the names and descriptions may use
 *			the default.
*/
 
// VIRTUAL
BOOL SECCommandList::FillEmpties()
{
	AutoFillEmptiesFromMenus();
	AutoFillFromStringIds();

	return TRUE;
}


int SECCommandList::Find(UINT nID)
{
	for (int i=0; i<GetSize(); i++) {
		if ( ElementAt(i).m_nID == nID)
			return i;
	}
	return -1;	// Not found
}


/*******************************************************************************
 *
 *		SECCommandList::AutoFillEmptiesFromMenus
 *
 * Desc:	Walk all menus in the mainframe and in the templates.
 *			Call FillEmptiesFromMenu for each menu.
 *
 * Returns:	TRUE on success.
 */
BOOL SECCommandList::AutoFillEmptiesFromMenus()
{
	CFrameWnd* pMainFrame = (CFrameWnd*)::AfxGetMainWnd();
	ASSERT_VALID(pMainFrame);
	
	FillEmptiesFromMenu(pMainFrame->m_hMenuDefault);
	
	CWinApp* pApp = AfxGetApp();
	POSITION pos = pApp->GetFirstDocTemplatePosition();
	while (pos) {
		CDocTemplate* pTemplate = pApp->GetNextDocTemplate( pos );
		if ( pTemplate->IsKindOf( RUNTIME_CLASS( CMultiDocTemplate ) ) ) {
			FillEmptiesFromMenu(((CMultiDocTemplate*)pTemplate)->m_hMenuShared);
		}
	}
	return TRUE;
}


/*******************************************************************************
 *
 *		SECCommandList::FillEmptiesFromMenu
 *
 * Desc:	Walk the menu tree.  For each command ID in the tree,
 *			see if that command ID is in the CommandList.  If
 *			it is, and no name is available, fill in the name
 *			with the text from the menu.
 *
 * Returns:	TRUE on success.
 */
BOOL SECCommandList::FillEmptiesFromMenu(HMENU hMenu, const CString& strPrevious)
{
	CMenu menuBar;
	menuBar.Attach(hMenu);
	int barCount = menuBar.GetMenuItemCount();
	
	TCHAR szItem[64];
	CString barDesc;
	
	for (int barIndex = 0; barIndex<barCount; barIndex++) {
		if (!(menuBar.GetMenuString(barIndex, szItem, sizeof(szItem)/sizeof(TCHAR), MF_BYPOSITION)))
			continue;
		ExtractMenuItemName(szItem);
		
		UINT nID = menuBar.GetMenuItemID(barIndex);
		if (nID == -1) {
			FillEmptiesFromMenu( GetSubMenu(menuBar, barIndex), strPrevious+szItem+_T(":") );
			continue;
		}

		int i = Find(nID);
		if (i < 0)
			continue;
		if (ElementAt(i).m_strName.IsEmpty()) {
			ElementAt(i).m_strName = strPrevious + szItem;
		}
	}
	menuBar.Detach();
	return TRUE;
}


/*******************************************************************************
 *
 *		SECCommandList::AutoFillFromStringIds
 *
 * Desc:	Using the tooltips and status bar messages that are
 *			in the string table, try to fill in all of the
 *			unknown names and descriptions.
 *
 * Returns:	TRUE on success.
 */
BOOL SECCommandList::AutoFillFromStringIds()
{
	CString str;
	CString status;
	CString tooltip;
	for (int i=0; i<GetSize(); i++) {
		if (str.LoadString(ElementAt(i).m_nID)) {
			int nl = str.Find(_T('\n'));
			if (nl >= 0) {
				status = str.Left(nl);
				tooltip = str.Right(str.GetLength() - nl - 1);
			}
			else {
				status = str;
				tooltip = _T("");
			}
			if (ElementAt(i).m_strDescription.IsEmpty() && !status.IsEmpty())
				ElementAt(i).m_strDescription = status;
			if (ElementAt(i).m_strName.IsEmpty() && !tooltip.IsEmpty())
				ElementAt(i).m_strName = tooltip;
			
		}
	}
	return TRUE;
}


/*******************************************************************************
 *
 *		SECCommandList::ExtractMenuItemName
 *
 * Desc:	Take a menu string and make it presentable enough for the list box.
 *
 *			Four things must be removed:
 *			1. The \t and the accelerator text at the end.
 *			2. The embedded & that marks the underlined key.
 *			3. The text (&[A-Z]) which defines an underlined key for
 *				int'l keyboards.
 *			4. The ellipsis at the end of some menu entries.
 *
 * Returns:	Nothing.
 */
void SECCommandList::ExtractMenuItemName(LPTSTR szItem)
{
	LPTSTR pch = _tcsstr(szItem, _T("..."));
	if (pch) *pch = _T('\0');
	pch = _tcsstr(szItem, _T("(&"));
	if (pch) *pch = _T('\0');
	pch = _tcschr(szItem, _T('\t'));
	if (pch) *pch = _T('\0');

	pch = _tcschr(szItem, _T('&'));
	if (pch) {
		LPTSTR pch2 = _tcsinc(pch);
		int len = _tcslen(pch2)+1;					// Including trailing null
		memmove(pch, pch2, len*sizeof(TCHAR));	// Supports overlapping regions
	}
}

/////////////////////////////////////////////////////////
//
//	HotKey key code utility routines
//
/////////////////////////////////////////////////////////


/*****************************************************************************
 *
 *		AccelToHotKey
 *
 * Desc:	Given an accelerator table entry, return the
 *			corresponding hotkey code for the key.
 *
 * Returns:	hotkey code in destHotKey.
 *
 */
void AccelToHotKey(const ACCEL& accel, DWORD* destHotkey)
{
	WORD hotkey = accel.key;
	WORD fAccelFlags = accel.fVirt;
	WORD flags = 0;
	
	if (hotkey > VK_SPACE && hotkey <= VK_HELP
		|| (hotkey >= VK_LWIN && hotkey <= VK_APPS)
		|| (hotkey == VK_DIVIDE) )
		flags |= HOTKEYF_EXT;
	if (fAccelFlags & FALT)
		flags |= HOTKEYF_ALT;
	if (fAccelFlags & FCONTROL)
		flags |= HOTKEYF_CONTROL;
	if (fAccelFlags & FSHIFT)
		flags |= HOTKEYF_SHIFT;
	*destHotkey = hotkey | (flags<<8);
}

/*****************************************************************************
 *
 *		::HotKeyToAccel
 *
 * Desc:	Convert the hotkey value to an accelerator value
 *
 * Returns:	fVirt and key in ACCEL are filled in.
 *
 */
void HotKeyToAccel( DWORD hotkey, ACCEL* accel )
{
	WORD wVKCode = (WORD)(hotkey & 0xff);
	hotkey = HIBYTE(hotkey);
	BYTE fAccelFlags = 0;
	fAccelFlags |= kStandardAcceleratorFlags;
	if (hotkey & HOTKEYF_ALT)
		fAccelFlags |= FALT;
	if (hotkey & HOTKEYF_CONTROL)
		fAccelFlags |= FCONTROL;
	if (hotkey & HOTKEYF_SHIFT)
		fAccelFlags |= FSHIFT;
	
	/*
	 * Find the accelerator that the user modified
	 * and change its flags and virtual-key code
	 * as appropriate.
	 */
	
	accel->fVirt = (BYTE)fAccelFlags;
	accel->key = wVKCode;
}


/*****************************************************************************
 *
 *		::VkToName
 *
 * Desc:	Given a virtual keycode, convert it to a text string
 *			in the language of the current locale.
 *
 * Returns:	CString contains the text of the key.
 *
 * Remarks:	Each key on the keyboard is distinct, so Ctrl-A
 *			would be passed in as two separate keys.
 *			The bExt flag indicates a value from an extended
 *			101 key keyboard.  All of the keys such as Insert,
 *			Delete, Up Arrow, etc. are extended.  The
 *			HotKey control automatically supplies this
 *			flag, everyone else must figure it out themselves.
 */
CString VkToName(UINT nChar, BOOL bExt)
{
	TCHAR nametext[64];	// Key names just aren't that long!
	
	//UINT scancode = MapVirtualKey(nChar, 0)<<16 | 0x1;
	UINT scancode = MapVirtualKeyEx(nChar, 0, GetKeyboardLayout(0))<<16 | 0x1;
	
	if (bExt)
		scancode |= 0x01000000;
	
	int len = GetKeyNameText(scancode, nametext, sizeof(nametext)/sizeof(TCHAR));
	return nametext;
}


/*****************************************************************************
 *
 *		::GetHotKeyDescription
 *
 * Desc:	Given a hotkey value (DWORD), convert it into the complete
 *			text representation.
 *
 * Returns:	FALSE	If the hotkey is empty (no key pressed).
 *			TRUE	If the string was created successfully.
 *
 * Remarks:	Unlike VkToName, this routine generates a complete
 *			description of a set of keys.
 */
BOOL GetHotKeyDescription(DWORD hotkey, CString& strKeyName)
{
	strKeyName.Empty();
	
	if (hotkey) {
		
		UINT wVKCode = hotkey & 0xff;
		hotkey = HIBYTE(hotkey);
		UINT fAccelFlags = 0;
		fAccelFlags |= FVIRTKEY;
		
		if (hotkey & HOTKEYF_CONTROL)
			strKeyName += VkToName(VK_CONTROL, FALSE) + _T("+");
		if (hotkey & HOTKEYF_ALT)
			strKeyName += VkToName(VK_MENU, FALSE) + _T("+");
		if (hotkey & HOTKEYF_SHIFT)
			strKeyName += VkToName(VK_SHIFT, FALSE) + _T("+");
		
		strKeyName += VkToName(wVKCode, hotkey & HOTKEYF_EXT);
		return TRUE;
	}
	
	return FALSE;	// No hotkey is currently set.
}


/////////////////////////////////////////////////////////
//
//	Helpers for MFC 3.x
//
/////////////////////////////////////////////////////////


#if _MFC_VER < 0x0400
/*******************************************************************************
 *
 *		ConstructElements
 *
 * Desc:	Iterates over an array of SECCommand objects and
 *			explicitly constructs dynamically-allocated memory.
 */

void ConstructElements( SECCommand* pTarget, int iCount )
{
	ASSERT(pTarget != NULL);
	for (int i=0; i < iCount; i++, pTarget++) {
		// Call to ctor is illegal C++, but this is what
		// the documentation for 2.2 says to do.
		pTarget->SECCommand::SECCommand();
	}
}

/*******************************************************************************
 *
 *		DestructElements
 *
 * Desc:	Iterates over an array of SECCommand objects and
 *			explicitly destructs dynamically-allocated memory.
 */

void DestructElements( SECCommand* pTarget, int iCount )
{
	ASSERT(pTarget != NULL);
	for (int i = 0; i < iCount; i++, pTarget++) {
		pTarget->SECCommand::~SECCommand();
	}
}
#endif	//  _MFC_VER < 0x0400  
#endif //WIN32
