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

	CImageList m_ImageList;		// for shell icons
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

	CImageList m_ImageList;
	int m_nListCtrlSortColumn;
	BOOL m_bListCtrlReverseSort;
	BOOL m_bOutaHere;

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
	CString m_strAttachDir;
};

#endif // __FILEBROWSEVIEW_H__
