// helpcntx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CContextHelp window
#define MAX_CONTOL_IDS_PER_WINDOW 100
#define HID_OFFSET 0x10000UL 	// value of HID_BASE_COMMAND in afxpriv.h

class CContextHelp 
{
// Construction
public:
	virtual ~CContextHelp();
	CContextHelp();

#ifdef WIN32
	// For WM_HELP messages
	CContextHelp(HWND hWnd); 		
#endif
	// For WM_HELPHITTEST and WM_CONTEXTMENU msgs
	CContextHelp (HWND hWnd, CPoint point, BOOL bClientCoords=TRUE);	

	
// Attributes
public:
	DWORD m_aHelpID[ (MAX_CONTOL_IDS_PER_WINDOW+1)*2];

};

/////////////////////////////////////////////////////////////////////////////

// MP_ stands for Menu Position
#define MP_POPUP_NON_CLIENT		0
#define MP_POPUP_COMP_MSG 		1
#define MP_POPUP_RECEIVED_MSG	2
#define MP_POPUP_MAILBOX		3
#define MP_POPUP_NICKNAME_LIST	4
#define MP_POPUP_OUT_MAILBOX	5
#define MP_POPUP_IMAP_MAILBOX	6	// New IMAP-only mailbox menu.

#define MP_TRANSFER_TOC			5	// Position to Insert Transfer menu on TOC context menu
#define MP_TRANSFER_READMSG		7	// Position to Insert Transfer menu on ReadMsg context menu
#define MP_ATTACH_PLUGINS 		12	// Position of Attach Plug-Ins menu in the Comp Msg menu
#define MP_INSERT_RECIP 		11	// Position of Insert Recipient in the Comp Msg menu
#define MP_MDICHILDWND_LIST		6	// Begining of MDI Child window list in Window menu

class CContextMenu 
{
// Construction
public:
	virtual ~CContextMenu();
	CContextMenu();
	CContextMenu(UINT nMenuPos, int xPos, int yPos, CWnd* ParentWnd=NULL);
	CContextMenu(CMenu* pPopupMenu, int xPos, int yPos, CWnd* ParentWnd=NULL);

	static void MatchCoordinatesToWindow(HWND hWnd, CPoint& ptScreen);
};

/////////////////////////////////////////////////////////////////////////////
