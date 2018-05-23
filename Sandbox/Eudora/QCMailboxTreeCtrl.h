#ifndef QCMAILBOXTREECTRL_H
#define QCMAILBOXTREECTRL_H

#include "QCTree.h"
#include "QCImageList.h"

class QCMailboxCommand;


///////////////////////////////////////////////////////////////////////
// QCMailboxTreeCtrl
//
// The main tree control implementation class.  This is a smart 
// Eudora mailbox-specific tree control that is meant to be embedded
// in a parent container window of any sort.
///////////////////////////////////////////////////////////////////////
class QCMailboxTreeCtrl : public QCTreeCtrl
{
	DECLARE_DYNAMIC(QCMailboxTreeCtrl)


// Construction
public:
	enum ItemType
	{
		ITEM_ROOT = 0,
		ITEM_IN_MBOX,
		ITEM_OUT_MBOX,
		ITEM_TRASH_MBOX,
		ITEM_FOLDER,
#ifndef IMAP4
		ITEM_USER_MBOX
#else
		ITEM_USER_MBOX,
		ITEM_IMAP_ACCOUNT,	// Associated with 1 IMAP server.
		ITEM_IMAP_NAMESPACE,
		ITEM_IMAP_MAILBOX	// May or may not have inferiors.
#endif // IMAP4
	};

protected:

	QCMailboxImageList m_ImageList;


	HTREEITEM GetParentFromPathname(
	LPCSTR	szPathname );

	HTREEITEM GetParentFromPathname(
	HTREEITEM		hParent,
	LPCSTR			szParentPath,
	const CString&	szChildPath );

// Construction
public:

	struct CMboxTreeItemData
	{
		CMboxTreeItemData(	ItemType itemType, 
							QCMailboxCommand* pCommand) :
			m_itemType(itemType),
			m_pCommand(pCommand)
		{
		}

		~CMboxTreeItemData(void)
		{
		}

		CMboxTreeItemData(void);						// not implemented
		CMboxTreeItemData(const CMboxTreeItemData&);	// not implemented
		void operator=(const CMboxTreeItemData&);		// not implemented

		bool IsMbx() // Returns if this is an acutal mailbox
		{
			switch (m_itemType)
			{
//				case ITEM_ROOT:
//				case ITEM_FOLDER:
//				case ITEM_IMAP_ACCOUNT:
//				case ITEM_IMAP_NAMESPACE:

				case ITEM_IN_MBOX:
				case ITEM_OUT_MBOX:
				case ITEM_TRASH_MBOX:
				case ITEM_USER_MBOX:
				case ITEM_IMAP_MAILBOX:
					return (true); // Mailbox -- success!
			}					

			return (false); // Not mailbox -- failure
		}

		ItemType		  m_itemType;			// type of tree node
		QCMailboxCommand* m_pCommand;			// needed for menu ID and full pathname
	};

	virtual BOOL Init(void);

	//
	// Clients should NOT call CTreeCtrl::DeleteItem() or
	// CTreeCtrl::DeleteAllItems() directly, under pain of leaky
	// memory!
	//
	virtual BOOL Reset(void);

#ifdef IMAP4
	// Public

	HTREEITEM ImapGetParentFromPathname(
		LPCSTR			szChildPath );

	virtual BOOL GetImapItemStruct(TV_INSERTSTRUCT &tvstruct,
								ItemType itemType,
								const CString& itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);
	virtual BOOL AddImapItem(ItemType itemType,
								const CString& itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);

	void UpdateImapItemImage (HTREEITEM hItem, UINT action);

#endif // IMAP4

	virtual BOOL GetItemStruct(TV_INSERTSTRUCT &tvstruct,
								ItemType itemType,
								const CString& itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);

	virtual BOOL AddItem(ItemType itemType,
							const CString& itemName, 
							QCMailboxCommand*	pCommand,
							BOOL isChecked);

	QCMailboxTreeCtrl();

// Attributes
public:

// Operations
public:
	virtual void CheckItemByMenuId(const CString& itemFilename, BOOL isChecked);

	// Auto-close operation to support drag and drop
	BOOL AutoCloseOpenedFolders();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QCMailboxTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~QCMailboxTreeCtrl();


	// Generated message map functions
protected:

	//
	// Override base class hook to perform auto-open action.
	//
	virtual void DoSomethingWhileUserPausedMouseAtPoint(CPoint pt);

	//{{AFX_MSG(QCMailboxTreeCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnItemExpanded(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void DeleteItemData(HTREEITEM hItem);
	
	//
	// Stores list of auto-opened folders for drag and drop operations.
	//
	CPtrList m_autoOpenFolderList;
};




/////////////////////////////////////////////////////////////////////////////
#endif // QCMAILBOXTREECTRL_H
