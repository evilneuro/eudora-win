#if !defined(AFX_SIGNATURETREE_H__B86B71B1_A488_11D0_97B0_00805FD2F268__INCLUDED_)
#define AFX_SIGNATURETREE_H__B86B71B1_A488_11D0_97B0_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SignatureTree.h : header file
//

#include "QCImageList.h"
#include "QCTree.h"
#include "QICommandClient.h"

/////////////////////////////////////////////////////////////////////////////
// CSignatureTree window

class CSignatureTree : public QCTreeCtrl, public QICommandClient
{
    COleDataSource		m_OleDataSource;
	QCMailboxImageList	m_ImageList;

	HTREEITEM	FindItem(
	HTREEITEM	hItem,
	LPCSTR		szName );

// Construction
public:
	CSignatureTree();


	BOOL Init();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSignatureTree)
	public:
	//}}AFX_VIRTUAL

	DROPEFFECT	OnDragEnter(
	COleDataObject*	pDataObject,
	DWORD			dwKeyState, 
	CPoint			point);

	void		OnDragLeave();

	DROPEFFECT	OnDragOver(
	COleDataObject*	pDataObject,
	DWORD			dwKeyState, 
	CPoint			thePoint);

// Implementation
public:
	HTREEITEM AddItem( LPCSTR szName );
	virtual ~CSignatureTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSignatureTree)
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	virtual void Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIGNATURETREE_H__B86B71B1_A488_11D0_97B0_00805FD2F268__INCLUDED_)
