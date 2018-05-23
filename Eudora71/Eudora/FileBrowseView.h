// FileBrowseView.h
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

#ifndef __FILEBROWSEVIEW_H__
#define __FILEBROWSEVIEW_H__

//
// FileBrowseView.h : Smart CFileBrowseView class implementing an
// Explorer-like view for the file system.
//
#include <shlobj.h>

// STRUCTURES
typedef struct tagLISTDATA
{
	LPSHELLFOLDER	piParentShellFolder;
	LPITEMIDLIST	pIdList;
	ULONG			ulAttribs;
} LISTDATA, *LPLISTDATA;

typedef struct tagTREEDATA
{
	LPSHELLFOLDER	piParentShellFolder;
	LPITEMIDLIST	pIdList;
	LPITEMIDLIST	pFullyQualifiedIdList;
} TREEDATA, *LPTREEDATA;


////////////////////////////////////////////////////////////////////////
// CDriveComboBox
//
// PRIVATE class used only by CFileBrowseView.
////////////////////////////////////////////////////////////////////////
class CDriveComboBox : public CComboBox
{
	friend class CFileBrowseView;

//
// Private constructor/destructor used only by CFileBrowseView class.
//
private:
	CDriveComboBox();
	virtual ~CDriveComboBox();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDriveComboBox)
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDriveComboBox)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	// For shell icons
	// Can't use CImageList because it expects only one object to attach to an imagelist
	static HIMAGELIST	s_hImageList;
};


////////////////////////////////////////////////////////////////////////
// CFileBrowseView
//
////////////////////////////////////////////////////////////////////////
class CFileBrowseView : public C3DFormView
{
protected:
	DECLARE_DYNCREATE(CFileBrowseView)

	CFileBrowseView();
	virtual ~CFileBrowseView();

protected:
// Form Data
	//{{AFX_DATA(CFileBrowseView)
	enum { IDD = IDD_FILE_BROWSE };
	CDriveComboBox m_DriveCombo;
	CListCtrl m_ListCtrl;
	CTreeCtrl m_TreeCtrl;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	int m_nListCtrlSortColumn;
	BOOL m_bListCtrlReverseSort;
	BOOL m_bOutaHere;

	BOOL			m_bFileListOnly;
	CString			m_FileListOnlyRootDir;
	LPSHELLFOLDER	m_piRootShellFolder;
	LPITEMIDLIST	m_pRootIdList;
	LPSHELLFOLDER	m_piCurrentShellFolder;
	LPITEMIDLIST	m_pCurrentIdList;

	typedef struct	tagNavItem { LPSHELLFOLDER piShellFolder; LPITEMIDLIST pIDL; } NavItem;
	typedef CTypedPtrList<CPtrList, NavItem*> NavItemList;
	NavItemList		m_NavList;
	int				m_CurrentNavListItem;
	void			TruncateNavList(int AtIndex);
	void			NavToItem(int AtIndex);

	//
	// Handy OLE interfaces available for use while this object is alive.
	//
	LPSHELLFOLDER m_piDesktopFolder;		// IShellFolder interface
	LPITEMIDLIST m_pDesktopIdList;			// PIDL
	LPMALLOC m_piMalloc;					// IMalloc interface

// Attributes
public:
	enum
	{
		TRACKER_HEIGHT = 6
	};

// Operations
public:
	static int CALLBACK ListViewCompareProc(LPARAM, LPARAM, LPARAM);
	static int CALLBACK TreeViewCompareProc(LPARAM, LPARAM, LPARAM);

	// Generic functions that deal with PIDLs
	static LPITEMIDLIST ConcatPidls(LPCITEMIDLIST pIdList1, LPCITEMIDLIST pIdList2);
	static LPITEMIDLIST CopySinglePidl(LPITEMIDLIST pIdList);
	static LPITEMIDLIST CopyFullPidlExceptForLast(LPITEMIDLIST pIdList);
	static LPITEMIDLIST GetLastPidl(LPITEMIDLIST pIdList);
	static BOOL GetName(LPSHELLFOLDER piShellFolder, LPITEMIDLIST pIdList, DWORD dwFlags, LPSTR pszFriendlyName);
	static LPITEMIDLIST AllocPidl(UINT cbSize);
	static UINT GetSize(LPCITEMIDLIST pIdList);
	static LPITEMIDLIST Next(LPCITEMIDLIST pIdList);

	void SetFileListOnly(LPCTSTR RootDirectory = NULL);
	void SetViewState(int NewState);
	int  GetViewState();

	//
	// MFC is screwed up here.  The virtual Create method is public 
	// in CWnd, but protected in CFormView.  Therefore, we need to
	// to make a public wrapper to call the protected CFormView version.
	//
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
						DWORD dwRequestedStyle, const RECT& rect, 
						CWnd* pParentWnd, UINT nID,
						CCreateContext* pContext = NULL)
		{ 
			return C3DFormView::Create(lpszClassName, lpszWindowName, 
										dwRequestedStyle, rect,
										pParentWnd, nID, pContext);
		}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBrowseView)
	public:
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	void OnInitialUpdate();

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFileBrowseView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCmdRefresh();
	afx_msg void OnDestroy();
	afx_msg void OnAppExit();
	afx_msg void OnDriveComboSelchange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg long OnActivateWazoo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOK();
	afx_msg void OnNavBack();
	afx_msg void OnNavForward();
	afx_msg void OnNavUp();
	afx_msg void OnSetHomeFolder();
	afx_msg void OnProperties();
	afx_msg void OnNewFolder();
	afx_msg void OnDelete();
	afx_msg void OnViewLargeIcons();
	afx_msg void OnViewSmallIcons();
	afx_msg void OnViewList();
	afx_msg void OnViewDetails();
	afx_msg void OnUpdateNavBack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavForward(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavUp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedsListSelection(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLargeIcons(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSmallIcons(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewList(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDetails(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void ResetDriveCombo();
	void PopulateDriveComboFolder(int nLevel,
									LPITEMIDLIST lpMyComputerIdList, 
									LPSHELLFOLDER lpShellFolder, 
									LPITEMIDLIST lpFullyQualifiedIdList);
	void AddDriveComboFolder(const CString& strFolderName, const CString& strFolderPathname);
	int GetWindowsDriveComboIndex();
	BOOL ResetTreeRoot(LPSHELLFOLDER lpsf, LPITEMIDLIST pFullyQualifiedIdList, LPSHELLFOLDER piParentShellFolder, LPITEMIDLIST pIdList);
	BOOL PopulateTreeFolder(LPSHELLFOLDER lpsf, LPITEMIDLIST pFullyQualifiedIdList, HTREEITEM hParent);
	HTREEITEM AddTreeItem(HTREEITEM hParent, const CString& strName, BOOL bHasChildren, LPITEMIDLIST lpFullyQualifiedIdList, LPSHELLFOLDER lpParentShellFolder, LPITEMIDLIST lpIdList);
	int GetIcon(LPITEMIDLIST pIdList, UINT uFlags);
	void GetIconIndexes(LPITEMIDLIST pFullyQualifiedIdList, int& nImageIndex, int& nSelectedImageIndex);
	LRESULT NotifyHandler(UINT message, WPARAM wParam, LPARAM lParam); 
	BOOL DoTheMenuThing(HWND hwnd, LPSHELLFOLDER piParentShellFolder, LPITEMIDLIST  pIdList, LPPOINT lppt);
	BOOL ResetListView(LPSHELLFOLDER lpsf, LPITEMIDLIST lpFullyQualifiedIdList);
	BOOL InitListViewImageLists();
	BOOL PopulateListView(LPSHELLFOLDER lpsf);
	void SortListView(LPITEMIDLIST pFullyQualifiedIdList);
	LPITEMIDLIST GetFullyQualifiedPidlFromListItem(LPLISTDATA pListData);
	LPITEMIDLIST GetFullyQualPidlFromPath(LPCTSTR lpszFullPathName);
	LPITEMIDLIST* CreateListSelectionPIDL(UINT& Count);
	BOOL OpenItem(LPLISTDATA pListData);
	LPLISTDATA GetListDataAtPoint(CPoint ptScreen);


private:
	//
	// Data used while tracking a ghosted resize bar.
	//
	struct TrackerData
	{
		BOOL bIsTracking;
		int nTopLimit;
		int nBottomLimit;
		CRect rect;
	} m_trackerData;

	void DrawTrackerGhost(const CRect& rect);
	void GetResizeBarRect(CRect& rectClient) const;
	BOOL IsOnResizeBar(CPoint pt) const;
	HCURSOR m_hResizeCursor;
	CString m_strHomeDir;
	CString m_strAttachDir;

	// Drop target support
	BOOL GetTargetFolder(CPoint pt, LPTSTR FolderBuffer);
	void DragHighlightItem(int nItem, BOOL bAdd);
	LPTSTR m_DropFiles;
	BOOL m_bDropSourceOnSameDrive;
	int m_LastDropTargetItem;
	COleDropTarget m_DropTarget;
};

#endif // __FILEBROWSEVIEW_H__
