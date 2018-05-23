// NEWMBOX.H
//
#ifndef NEWMBOX_H
#define NEWMBOX_H

#include "helpxdlg.h"
#include "QCMailboxCommand.h"

/////////////////////////////////////////////////////////////////////////////
// CNewMailboxDlg dialog

class CNewMailboxDlg : public CHelpxDlg
{
// Construction
public:
	CNewMailboxDlg( LPCSTR szParentName, CPtrList* pList, BOOL IsTransfer);

// Dialog Data
	//{{AFX_DATA(CNewMailboxDlg)
	enum { IDD = IDD_NEW_MAILBOX };
	BOOL	m_MakeFolder;
	BOOL	m_DontTransfer;
	CString	m_Prompt;
	CString	m_Name;
	//}}AFX_DATA
	
	LPCSTR		m_szParentName;
	CPtrList*	m_pChildList;
	BOOL		m_IsTransfer;
 	
	BOOL OnInitDialog();

	BOOL			DoTransfer() { return !m_DontTransfer; }
	const CString&	GetName() { return m_Name; }
	MailboxType		GetType() { return ( m_MakeFolder ? MBT_FOLDER : MBT_REGULAR ); }

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CNewMailboxDlg)
	afx_msg void OnMakeFolder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	void OnOK();
};

#ifdef OLDSTUFF
// Function Prototypes

class CMailboxesView;

UINT CreateNewMailbox(CMailboxesMenu* Menu, BOOL IsTransfer, CMailboxesView* MBView = NULL);
UINT CreateNewMailbox(UINT nID, BOOL IsTransfer);
#endif

#endif

