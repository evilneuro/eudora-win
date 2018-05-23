// QCMailboxTreeCtrl.h
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
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
DAMAGE. */

//


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
		ITEM_JUNK_MBOX,
		ITEM_TRASH_MBOX,
		ITEM_FOLDER,
		ITEM_USER_MBOX,
		ITEM_IMAP_ACCOUNT,	// Associated with 1 IMAP server.
		ITEM_IMAP_NAMESPACE,
		ITEM_IMAP_MAILBOX	// May or may not have inferiors.
	};

protected:

	QCMailboxImageList m_ImageList;

	HTREEITEM GetParentFromPathname(LPCSTR szPathname);
	HTREEITEM GetParentFromPathname(HTREEITEM hParent, LPCSTR szParentPath, LPCSTR szChildPath);

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
				case ITEM_JUNK_MBOX:
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

	HTREEITEM ImapGetParentFromPathname(LPCSTR szChildPath);

	virtual BOOL GetImapItemStruct(TV_INSERTSTRUCT &tvstruct,
								ItemType itemType,
								const char* itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);
	virtual BOOL AddImapItem(ItemType itemType,
								const char* itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);

	void UpdateImapItemImage (HTREEITEM hItem, UINT action);

#endif // IMAP4

	virtual BOOL GetItemStruct(TV_INSERTSTRUCT &tvstruct,
								ItemType itemType,
								const char* itemName, 
								QCMailboxCommand* pCommand,
								BOOL isChecked);

	virtual HTREEITEM AddItem(ItemType itemType,
								const char* itemName, 
								QCMailboxCommand*	pCommand,
								BOOL isChecked);

	QCMailboxTreeCtrl();

// Attributes
public:

// Operations
public:
	virtual void CheckItemByMenuId(const char* itemFilename, BOOL isChecked);
	virtual void RenameItemByMenuId(const char* oldItemFilename, const char* newItemFilename);
	virtual void RetypeItemByMenuId(const char* itemFilename, ItemType newType);

	// Auto-close operation to support drag and drop
	BOOL AutoCloseOpenedFolders();

	void UpdateRecentFolder();

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

	HTREEITEM m_hRecentFolderItem;
};




/////////////////////////////////////////////////////////////////////////////
#endif // QCMAILBOXTREECTRL_H
