//
// Stingray Software Extension Classes
// Copyright (C) 1996 Jim Beveridge
// All Rights Reserved
//
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
//
//  Author:			Jim Beveridge
//  Description:	Command description classes - Implementation
//  Created:		Sun Jan 28 10:10:29 1996
//
//

#ifndef __SCUTCMDS_H__
#define __SCUTCMDS_H__

#ifdef WIN32
#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif // __AFXTEMPL_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
        #undef AFXAPP_DATA
        #define AFXAPP_DATA AFXAPI_DATA
        #undef AFX_DATA
        #define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL



//
// These modifiers always seemed to be defined whenever an accelerator is defined.
// This is empirical, I am not positive what the "real" behavior should be.
const WORD kStandardAcceleratorFlags = FVIRTKEY | FNOINVERT;

void AccelToHotKey(const ACCEL& accel, DWORD* destHotkey);
void HotKeyToAccel( DWORD destHotkey, ACCEL* accel );
CString VkToName(UINT nChar, BOOL bExt);
BOOL GetHotKeyDescription(DWORD hotkey, CString& strKeyName);


/************************************************************************
 *
 *		SECDefaultCommandId
 *
 *	@class	This structure holds a command id, its short name
 *			its description.  To customize which command id's
 *			the user may assign a macro to, create an array
 *			of this structure and fill it in.
 *
 *
 * For example, this declaration gives the system a
 * ist of macro-able command id's, but all of the
 * short names and descriptions are figured out from
 * the corresponding string ids.
 *
 *	const SECDefaultCommandId defaultCommands[] =
 *	{
 *		{ ID_VIEW_TOOLBAR },
 *		{ ID_FILE_SAVE },
 *		// etc.
 *		{ ID_FILE_PRINT }
 *	};
 *
 *
 * This declaration gives the system a
 * list of macro-able command id's, but overrides
 * some of the default names and descriptions.
 *
 * const SECDefaultCommandId defaultCommands[] =
 *	{
 *		{ ID_VIEW_TOOLBAR, IDS_MAC_VIEW_TOOLBAR, IDS_DESC_VIEW_TOOLBAR },
 *		{ ID_FILE_OPEN, 0, IDS_DESC_FILE_OPEN },	// Default name
 *		{ ID_FILE_SAVE, IDS_MAC_FILE_SAVE, 0 },		// Default description
 *		// etc.
 *		{ ID_FILE_PRINT }							// Default both
 *	};
 */
struct SECDefaultCommandId
{
	// Id of this command, such as ID_VIEW_TOOLBAR
	UINT	m_nID;

	// String ID that gives the short name of the
	// command.  This name appears in the 
	// "Select a macro:" listbox.  If this is
	// zero, then the menu name or toolhelp text is used.
	UINT	m_nName;

	// String ID that gives the description of the
	// command.  This name appears in the 
	// "Description:" listbox.  If this is zero,
	// then the status bar text for this id is used.
	UINT	m_nDescription;
};


/************************************************************************
 *
 *		SECCommand
 *
 *	@class	This structure is the "cooked" version of SECCommandIdList.
 *			An array of SECDefaultCommandId's is used as the defaults,
 *			and then each entry is filled into here.
 */
struct SECCommand {
	SECCommand();
	SECCommand(UINT nID);

	UINT	m_nID;
	CString	m_strName;
	CString	m_strDescription;
};



/************************************************************************
 *
 *		SECCommandList
 *
 *	@class	Maintain the list of command ID's that may be accelerated.
 *			The accelerator keys themselves are not kept in this
 *			structure because it is a one-to-many relationship.
 *
 */
class SECCommandList : public CArray<SECCommand, SECCommand&>
{
public:
	SECCommandList();
	virtual ~SECCommandList();

	// Use this to allow various key combinations.  The legal values
	// are from CHotKeyCtrl::SetRules
	void SetRules( WORD wInvalidComb, WORD wModifiers )
		{ m_wInvalidComb = wInvalidComb; m_wModifiers = wModifiers; }

	// You automatically get all the menus.  Call this to
	// get rid of them.
	void ClearCommandIds();

	// Takes an array of DefaultCommandId and adds
	// each entry into the least of legal commands.
	// Non-zero names and descriptions are loaded automatically.
	void AddCommandIds(const SECDefaultCommandId ids[], int count);

	// Returns TRUE if id should be excluded.
	virtual BOOL QueryExcludeId(UINT nID);

	virtual BOOL FillEmpties();

	BOOL AutoFillEmptiesFromMenus();
	BOOL AutoFillFromStringIds();

protected:

// Attributes
public:
	WORD	m_wInvalidComb;
	WORD	m_wModifiers;

protected:

	// Operations
public:

// Overrides

// Implementation
public:

	// Fill in reasonable defaults from the menus, tooltips, etc.
	void DeriveDefaults();

	int Find(UINT nID);

	// The default implementation loads all the menu ID's.
	virtual BOOL AddDefaultIds();

	BOOL FillEmptiesFromMenu(HMENU hMenu, const CString& strPrevious = "");
	void LoadIdsFromMenu(HMENU hMenu);
	void LoadIdsFromAccelTable();

	// Get rid of the accelerator, the &, the (&x), and the ...
	void ExtractMenuItemName(LPTSTR str);

protected:

};


#if _MFC_VER < 0x0400
// VC 4.0  handles this problem all by itself.
void ConstructElements(SECCommand* pTarget, int iCount);
void DestructElements(SECCommand* pTarget, int iCount);
#endif	//  _MFC_VER < 0x0400

#endif
#endif	// __SCUTCMDS_H__