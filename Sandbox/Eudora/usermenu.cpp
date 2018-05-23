// USERMENU.CPP
//
// Routines to handle user-defined menus, like Mailbox, Transfer, and recipient list menus

#include "stdafx.h"

#include <fcntl.h>

#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "cursor.h"
#include "fileutil.h"
#include "rs.h"
#include "resource.h"
#include "usermenu.h"
#include "font.h"
#include "utils.h"


#ifdef WIN32
#include "mainfrm.h"
#include "mboxtree.h"

#endif

#include "eudora.h"
#include "ems-wglu.h"
#include "trnslate.h"
#include "toolmenu.h"
#include "persona.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern char* StripMailboxName(char* Name);

// Data Items
HMENU g_WindowHMENU;
UINT g_CurrentUserMenuID;
static CBitmap g_UnreadStatusBitmap;

IMPLEMENT_DYNAMIC(CUserMenuItem, CObject)

CUserMenuItem::CUserMenuItem(const char* Name, UINT Type, BOOL AssignID /*= TRUE*/) :
	CObject(), m_Name(Name), m_Type(Type), m_ID(0)
{
	if (AssignID)
		m_ID = g_CurrentUserMenuID++;
}



CUserMenu::~CUserMenu()
{
	if (!IsMenu(m_hMenu))
		Detach();

	RemoveAll();
}

BOOL CUserMenu::RemoveAll()
{
	// This is going to get called for recpient list menus when the
	// program exists.  Windows deletes all menus/menu items that are connected to
	// a window when the window is destroyed, so there's no need to remove any menu items
	// here.  In fact, you'll get all kinds of errors trying to delete menus/menu items
	// that don't exist.
	BOOL HasMenu = IsMenu(m_hMenu);
	
	while (m_Items.IsEmpty() == FALSE)
	{
		CUserMenuItem* Item = m_Items.RemoveHead();
		
		if (HasMenu)
			VERIFY(DeleteMenu(Item->m_ID, MF_BYCOMMAND));
			
		delete Item;
	}
	
	return (TRUE);
}

BOOL CUserMenu::Add(CUserMenuItem* NewItem)
{
	UINT nFlags = MF_STRING;
	UINT nID = NewItem->m_ID;
	
	if (nID == 0)
		nFlags = MF_SEPARATOR;
	else
	{
		ASSERT(IsUserMenuID(nID));
//		if (!GetMenuItemCount())
//			m_LowID = nID;
//		m_HighID = nID;
	}

	// Actually add the menu item
	BOOL success = AppendMenu(nFlags, nID, NewItem->m_Name);
	ASSERT(success);
	m_Items.AddTail(NewItem);
	
	return (success);
}

BOOL CUserMenu::Insert(CUserMenuItem* NewItem)
{
	UINT nFlags = MF_STRING | MF_BYCOMMAND;
	UINT found = 0;
	BOOL success;
	
	ASSERT(IsUserMenuID(NewItem->m_ID));

	POSITION pos = m_Items.GetHeadPosition();
	while (pos)
	{
		CUserMenuItem* Item = m_Items.GetNext(pos);
		
		if (Item->m_Name.CompareNoCase(NewItem->m_Name) > 0)
		{
			found = Item->m_ID;
			break;
		}
	}

	if (found)
	{
		success = InsertMenu(found, nFlags, NewItem->m_ID, NewItem->m_Name);
		m_Items.InsertBefore(pos, NewItem);
	}
	else
	{
		success = AppendMenu(nFlags, NewItem->m_ID, NewItem->m_Name);
		m_Items.AddTail(NewItem);
	}

	ASSERT(success);

	return (success);
}

CUserMenuItem* CUserMenu::GetItem(UINT nID)
{
//	if (nID < m_LowID || nID > m_HighID)
//		return (NULL);
		
	POSITION pos = m_Items.GetHeadPosition();
	while (pos)
	{
		CUserMenuItem* Item = m_Items.GetNext(pos);
		
		if (Item->m_ID == nID)
			return (Item);
	}

	// Low and high IDs are not necessarily contiguous, so we could
	// find ourselves getting here
	return (NULL);
}

CUserMenuItem* CUserMenu::FindItem(const char* Name)
{
	POSITION pos = m_Items.GetHeadPosition();

	while (pos)
	{
		CUserMenuItem* Item = m_Items.GetNext(pos);
		
		if (Item)
		{
			char NameBuf[64];
			
			strcpy(NameBuf, Item->m_Name);
			StripMailboxName(NameBuf);
			if (stricmp(NameBuf, Name) == 0)
				return (Item);
		}
	}

	return (NULL);
}


//
// This menu is built at in parallel with the Plugin items real
// top-level Tools menu (implemented as CSpecialMenu).  However, it
// contains only the Plugin menu items that reside in the top-level
// Tools menu so that it can be displayed while customizing the
// toolbar.  This menu is only displayed for the purpose of toolbar
// customization.
// 


// GetUserMenuItem
//
// Given a menu ID, return the menu item
CUserMenuItem* GetUserMenuItem(UINT nID)
{
	CUserMenuItem* Found = NULL;
	
#ifdef OLDSTUFF	
	if ((Found = g_TranslationMenu.GetItem(nID)))
		return (Found);
	if ((Found = g_AttacherMenu.GetItem(nID)))
		return (Found);
	if ((Found = g_SpecialMenu.GetItem(nID)))
		return (Found);
#endif	

	return (NULL);
}


////////////////////////////////////////////////////////////////////////
// GetUserMenuItemId [extern]
//
// Given a user-defined menu item type (ID_MAILBOX_USER, etc) and a
// corresponding menu item string, search the appropriate menus for
// the first menu item with a matching menu item string.
//
// If successful, return the dynamically-assigned command id for
// that menu item.  Otherwise, return 0 if there was no match.
//
////////////////////////////////////////////////////////////////////////
UINT GetUserMenuItemId(UINT menuItemType, const CString& searchString)
{
	ASSERT(! searchString.IsEmpty());

	CUserMenuItem* p_menuitem = NULL;

	switch (menuItemType)
	{
#ifdef OLDSTUFF	
	case ID_TRANSLATOR:
		if ((p_menuitem = g_TranslationMenu.FindItem(searchString)) != NULL)
			return p_menuitem->m_ID;
		break;
	case ID_MESSAGE_ATTACH_PLUGIN:
		if ((p_menuitem = g_AttacherMenu.FindItem(searchString)) != NULL)
			return p_menuitem->m_ID;
		break;
	case ID_SPECIAL_PLUGIN:
		if ((p_menuitem = g_SpecialMenu.FindItem(searchString)) != NULL)
			return p_menuitem->m_ID;
		break;
#endif
	default:
		ASSERT(0);
		break;
	}

	return 0;		// inappropriate type, or no matching menu item
}


///////////////////////////////////////////////////////////////////////////////
//
//	BOOL DeleteItem(
//	UINT uID );
//
//	Removes & deletes the item associated with a specific id.
//
//	Input:
//		UINT	uID	-	the id
//
//	Return Value:
//		TRUE on success, otherwise FALSE.
//
BOOL CUserMenu::DeleteItem(
UINT	uID,
UINT	uType )
{
	CUserMenuItem*	pItem;	
	POSITION		pos;
	POSITION		pos2;

	pos = m_Items.GetHeadPosition();
	pItem = NULL;
	
	// find the item
	while ( pos != NULL )
	{
		// save the current position
		pos2 = pos;

		// get the next item and increment the position
		pItem = ( CUserMenuItem* ) m_Items.GetNext( pos );
		
		if ( ( pItem -> m_Type == uType ) && ( pItem -> m_ID == uID ) )
		{
			// found it
						
			// remove it from the list
			m_Items.RemoveAt( pos2 );
			
			if ( IsMenu( m_hMenu ) )
			{
				// delete it from the menu
				DeleteMenu( uID, MF_BYCOMMAND );
			}

			// delete the item 
			delete pItem;

			return TRUE;
		}
	}
	return FALSE;
}

