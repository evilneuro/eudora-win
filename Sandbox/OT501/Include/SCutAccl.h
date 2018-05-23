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
//  Description:	Apply shortcuts classes - Implementation
//  Created:		Sun Jan 28 10:10:29 1996
//
//

#ifndef __SCUTACCL_H__
#define __SCUTACCL_H__
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
// This number should not have to change as long
//	as the Win32 ACCEL structure remains constant.
const WORD kShortcutTableVersion = 1;

	// kShortcutTableFileChars really is char, not TCHAR.  Do this so
	// that big endian does not look funny in the file.
const char kShortcutTableFileChars[] = { 'M','A','C','R' };
const DWORD kShortcutTableFileTag = *(DWORD*)kShortcutTableFileChars;


/************************************************************************
 *
 *		SECCommandList
 *
 *	@class	Maintain the list of command ID's that may be accelerated.
 *			The accelerator keys themselves are not kept in this
 *			structure because it is a one-to-many relationship.
 *
 */
class SECShortcutTable : public CArray<ACCEL,ACCEL&>
{

public:

	// These enums are used by GetDataFileName
	enum ENameType {
		// Returns the path+filename to store the
		// data in the same place as the EXE.
		MAIN_NAME,
		// Returns the path+filename to store the
		// data in the Windows dir (for those with
		// the EXE on a read-only server)
		ALTERNATE_NAME
	};

	// If the mainframe has a table, load it.  Otherwise
	// start out empty.
	SECShortcutTable();

	virtual ~SECShortcutTable();

	// Walk all menus and update their descriptions.
	BOOL ApplyAcceleratorsToMenus();

	// Return the full path and name of the data file.
	virtual void GetDataFileName(CString& str, ENameType type);

	// Load the user defined accels (override this to use
	// the registry or some other archive)
	virtual BOOL Load();

	// Save the user defined accels (override this to use
	// the registry or some other archive)
	virtual BOOL Save();

	// Create a new accel table for the mainframe from the
	// local table and update the menus.
	virtual BOOL Apply();

	// Revert this structure and the persistent table
	// back to the accel table in the resource file.
	// This function calls LoadDefaultTable().
	void ResetAll();

	// Reload this table with the accel table
	// from the resource file.
	void LoadDefaultTable();

protected:

// Attributes
public:
	static HACCEL mg_hOriginalAccelTable;

	// Operations
public:

// Overrides

// Implementation
public:
	// Associate the given hotkey with the given id.  The hotkey
	// is translated into an accelerator value first.
	void AssignAccel( DWORD hotkey, UINT id );

protected:

	// Load this structure with the mainframe's accel table
	BOOL LoadFromFrame();

	// Given a string from a menu item, replace the accelerator
	// description with "desc"
	void UpdateMenuItem( LPTSTR szItem, const CString& desc);

	// Update an entire menu tree with the accelerator
	// descriptions from this table.
	void ApplyAcceleratorsToMenu( HMENU hBarMenu );

};

#endif //WIN32

#endif	// __SCUTACCL_H__
