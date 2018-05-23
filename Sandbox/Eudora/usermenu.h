// USERMENU.H
//
// Routines to handle user-defined menus, like Mailbox, Transfer, and recipient list menus


// Used to keep track of the next automatically assigned menu ID
extern UINT g_CurrentUserMenuID;
#define MAX_LENGTH_MENU_NAME 60

// Forward declarations
class CUserMenu;

// Base class for user-defined menu items
class CUserMenuItem : public CObject
{
	DECLARE_DYNAMIC(CUserMenuItem)
public:	
	CUserMenuItem(const char* Name, UINT Type, BOOL AssignID = TRUE);
		
// Attributes
public:
	CString		m_Name;		// Menu item text
	UINT		m_ID;		// Menu ID of this item
	UINT		m_Type;		// String ID of menu item type:
							//	ID_MAILBOX_NEW, ID_MAILBOX_USER, ID_TRANSFER_NEW, ID_TRANSFER_USER,
							//	ID_RECIPIENT_INSERT, ID_RECIPIENT_NEW_MESSAGE, ID_RECIPIENT_REPLY,
							//	ID_RECIPIENT_FORWARD, ID_RECIPIENT_REDIRECT, ID_RECIPIENT_REMOVE,
							//  ID_TOOL_SIGNATURE
};


// A derived class of CObList so that we don't have to do a bunch of casting
class CUserMenuItemObList : public CObList
{
public:
	CUserMenuItem* GetAt(POSITION position) const
		{ return ((CUserMenuItem*)CObList::GetAt(position)); }
	CUserMenuItem*& GetAt(POSITION position)
		{ return ((CUserMenuItem*&)CObList::GetAt(position)); }
	CUserMenuItem*& GetHead()
		{ return ((CUserMenuItem*&)CObList::GetHead()); }
	CUserMenuItem* GetHead() const
		{ return ((CUserMenuItem*)CObList::GetHead()); }
	CUserMenuItem*& GetTail()
		{ return ((CUserMenuItem*&)CObList::GetTail()); }
	CUserMenuItem* GetTail() const
		{ return ((CUserMenuItem*)CObList::GetTail()); }
	CUserMenuItem*& GetNext(POSITION& rPosition)
		{ return ((CUserMenuItem*&)CObList::GetNext(rPosition)); }
	CUserMenuItem* GetNext(POSITION& rPosition) const
		{ return ((CUserMenuItem*)CObList::GetNext(rPosition)); }
	CUserMenuItem*& GetPrev(POSITION& rPosition)
		{ return ((CUserMenuItem*&)CObList::GetPrev(rPosition)); }
	CUserMenuItem* GetPrev(POSITION& rPosition) const
		{ return ((CUserMenuItem*)CObList::GetPrev(rPosition)); }
	CUserMenuItem* RemoveHead()
		{ return ((CUserMenuItem*)CObList::RemoveHead()); }
	CUserMenuItem* RemoveTail()
		{ return ((CUserMenuItem*)CObList::RemoveTail()); }
};


class CUserMenu : public CMenu
{
public:
//	CUserMenu() : m_LowID(0), m_HighID(0) {}
	virtual ~CUserMenu();
		
// Attributes
public:
	CUserMenuItemObList m_Items;	// The menu items in this menu
//	UINT m_LowID, m_HighID;			// Lowest and highest menu IDs of this menu

// Operations
public:

	BOOL DeleteItem(
	UINT	uID,
	UINT	uType);

	virtual BOOL RemoveAll();
	virtual BOOL Add(CUserMenuItem* NewItem);
	virtual BOOL Insert(CUserMenuItem* NewItem);
	CUserMenuItem* GetItem(UINT nID);
	UINT GetType(UINT nID)
		{ CUserMenuItem* Item = GetItem(nID); return (Item? Item->m_Type : 0); }
	CUserMenuItem* FindItem(const char* Name);
};


extern HMENU g_WindowHMENU;
extern HMENU g_EudoraProHMENU;


// Function Prototypes
const UINT LOWEST_USER_MENU_ID = 0x9100;
const UINT HIGHEST_USER_MENU_ID = 0xDFFF;
inline BOOL IsUserMenuID(UINT nID)
	{ return (nID >= LOWEST_USER_MENU_ID && nID <= HIGHEST_USER_MENU_ID); }
inline BOOL IsPopup(UINT nID)
	{ return (nID < LOWEST_USER_MENU_ID); }
	
CUserMenuItem* GetUserMenuItem(UINT nID);
inline UINT GetUserMenuItemType(UINT nID)
	{
		CUserMenuItem* item = GetUserMenuItem(nID);
		return (item? item->m_Type : NULL);
	}
UINT GetUserMenuItemId(UINT menuItemType, const CString& searchString);


