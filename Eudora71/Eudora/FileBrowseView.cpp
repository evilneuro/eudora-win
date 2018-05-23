// FileBrowseView.cpp : implementation of the CFileBrowseView class
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

#include "stdafx.h"

#include <QCUtils.h>

#include "font.h"
#include "cursor.h"
#include "resource.h"
#include "fileutil.h"
#include "guiutils.h"
#include "3dformv.h"
#include "FileBrowseView.h"
#include "WazooWnd.h"
#include "rs.h"
#include "utils.h"
#include "mainfrm.h"

#include "progress.h"

#include "DebugNewHelpers.h"

// STRUCTURES
typedef struct tagCOMBODATA
{
	LPSHELLFOLDER   piParentShellFolder;	// parent shell folder (NULL for Desktop folder)
	LPSHELLFOLDER   piShellFolder;			// shell folder itself
	LPITEMIDLIST    pIdList;				// short folder id (NULL for Desktop folder)
	LPITEMIDLIST    pFullyQualifiedIdList;	// fully-qualified folder id
	int             nIndentLevel;
	int             nIconIndex;
} COMBODATA, *LPCOMBODATA;

typedef struct tagLVSORTINFO
{
	LPMALLOC		piMalloc;
	LPITEMIDLIST    pFullyQualifiedIdList;
	int             nColumn;
	BOOL            bReverse;
} LVSORTINFO, *LPLVSORTINFO;

// CLASSES

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView

IMPLEMENT_DYNCREATE(CFileBrowseView, C3DFormView)

BEGIN_MESSAGE_MAP(CFileBrowseView, C3DFormView)
	//{{AFX_MSG_MAP(CFileBrowseView)
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_BROWSE_REFRESH, OnCmdRefresh)
	ON_WM_DESTROY()
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_CBN_SELCHANGE(IDC_FILE_BROWSE_COMBO, OnDriveComboSelchange)
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_REGISTERED_MESSAGE(umsgActivateWazoo, OnActivateWazoo)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(ID_BROWSE_BACK, OnNavBack)
	ON_COMMAND(ID_BROWSE_FORWARD, OnNavForward)
	ON_COMMAND(ID_BROWSE_UP, OnNavUp)
	ON_COMMAND(ID_BROWSE_SET_HOME_FOLDER, OnSetHomeFolder)
	ON_COMMAND(ID_BROWSE_PROPERTIES, OnProperties)
	ON_COMMAND(ID_BROWSE_NEW_FOLDER, OnNewFolder)
	ON_COMMAND(ID_BROWSE_DELETE, OnDelete)
	ON_COMMAND(ID_BROWSE_LARGE_ICONS, OnViewLargeIcons)
	ON_COMMAND(ID_BROWSE_SMALL_ICONS, OnViewSmallIcons)
	ON_COMMAND(ID_BROWSE_LIST, OnViewList)
	ON_COMMAND(ID_BROWSE_DETAILS, OnViewDetails)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_BACK, OnUpdateNavBack)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_FORWARD, OnUpdateNavForward)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_UP, OnUpdateNavUp)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_DELETE, OnUpdateNeedsListSelection)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LARGE_ICONS, OnUpdateViewLargeIcons)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_SMALL_ICONS, OnUpdateViewSmallIcons)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_LIST, OnUpdateViewList)
	ON_UPDATE_COMMAND_UI(ID_BROWSE_DETAILS, OnUpdateViewDetails)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView construction/destruction

CFileBrowseView::CFileBrowseView() :
	C3DFormView(CFileBrowseView::IDD),
	m_bOutaHere(FALSE),
	m_bFileListOnly(FALSE),
	m_piRootShellFolder(NULL),
	m_pRootIdList(NULL),
	m_piCurrentShellFolder(NULL),
	m_pCurrentIdList(NULL),
	m_CurrentNavListItem(-1),
	m_hResizeCursor(NULL),
	m_nListCtrlSortColumn(0),		// default is sort by name
	m_bListCtrlReverseSort(FALSE),	// default is forward sort
	m_piDesktopFolder(NULL),
	m_pDesktopIdList(NULL),
	m_piMalloc(NULL),
	m_DropTarget(),
	m_DropFiles(NULL),
	m_bDropSourceOnSameDrive(FALSE),
	m_LastDropTargetItem(-1)
{
	//{{AFX_DATA_INIT(CFiltersViewLeft)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_trackerData.bIsTracking = FALSE;
}

CFileBrowseView::~CFileBrowseView()
{
	if (m_hResizeCursor != NULL)
		::DestroyCursor(m_hResizeCursor);

	if (m_piDesktopFolder)
		m_piDesktopFolder->Release();

	if (m_piRootShellFolder)
		m_piRootShellFolder->Release();

	if (m_piCurrentShellFolder)
		m_piCurrentShellFolder->Release();

	if (m_piMalloc)
	{
		if (m_pDesktopIdList)
			m_piMalloc->Free(m_pDesktopIdList);
		if (m_pRootIdList)
			m_piMalloc->Free(m_pRootIdList);
		if (m_pCurrentIdList)
			m_piMalloc->Free(m_pCurrentIdList);
		m_piMalloc->Release();
	}

	TruncateNavList(0);
}

////////////////////////////////////////////////////////////////////////
// TruncateNavList [protected]
//
// Helper routine to remove items off of the navigation list
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::TruncateNavList(int AtIndex)
{
	POSITION pos = m_NavList.FindIndex(AtIndex);
	
	while (pos)
	{
		POSITION OldPos = pos;
		NavItem* pNI = m_NavList.GetAt(OldPos);
		m_NavList.GetNext(pos);
		m_NavList.RemoveAt(OldPos);
		if (pNI)
		{
			if (pNI->piShellFolder)
				pNI->piShellFolder->Release();
			if (pNI->pIDL)
				m_piMalloc->Free(pNI->pIDL);
			delete pNI;
		}
	}

	ASSERT(m_NavList.GetCount() == AtIndex);
}

////////////////////////////////////////////////////////////////////////
// OnInitialUpdate [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnInitialUpdate()
{
	if (m_DriveCombo.GetSafeHwnd())
	{
		//
		// This shouldn't be happening.  But it does.  When it does
		// happen, it seems to occur only when the Wazoo container
		// is in MDI mode.  The following is a workaround for sizing
		// the controls properly for MDI mode windows.
		//
//FORNOW		ASSERT(0);
		CRect rectClient;
		GetClientRect(rectClient);
		OnSize(SIZE_RESTORED, rectClient.Width(), rectClient.Height());
		return;			// guard against bogus double initializations
	}

	C3DFormView::OnInitialUpdate();

	// Frame window needs to be just big enough to hold the view
    // Size the controls
	// This is a kludge so it doesn't assert every time this view is resized
	SetScaleToFitSize(CSize(1, 1));
    OnSize(SIZE_RESTORED, -1, -1);

	if (GetParent()->IsKindOf(RUNTIME_CLASS(CWazooWnd)))
	{
		//
		// As a hack, trick the parent Wazoo window into properly saving
		// the initial control focus for this form.
		//
		CWazooWnd::SaveInitialChildFocus(&m_TreeCtrl);
	}

	// Register drop target (will automatically be revoked when destroyed)
	VERIFY(m_DropTarget.Register(this));
}


// Drag and drop support

void CFileBrowseView::DragHighlightItem(int nItem, BOOL bAdd)
{
	if (nItem != -1)
	{
		m_ListCtrl.SetItemState(nItem, bAdd? LVIS_DROPHILITED : 0, LVIS_DROPHILITED);
//		m_ListCtrl.RedrawItems(nItem, nItem);
	}
}

BOOL CFileBrowseView::GetTargetFolder(CPoint pt, LPTSTR FolderBuffer)
{
	LPLISTDATA pListData = GetListDataAtPoint(pt);
	int nItem = -1;

	*FolderBuffer = 0;

	if (m_DropFiles)
	{
		if (pListData && pListData->ulAttribs & SFGAO_FOLDER)
		{
			if (!GetName(m_piCurrentShellFolder, pListData->pIdList, SHGDN_FORPARSING, FolderBuffer))
				*FolderBuffer = 0;
			else
			{
				m_ListCtrl.ScreenToClient(&pt);

				UINT uFlags;
				nItem = m_ListCtrl.HitTest(pt, &uFlags);

				if (!(uFlags & LVHT_ONITEM))
					nItem = -1;
			}
		}
		else
		{
			if (!GetName(m_piDesktopFolder, m_pCurrentIdList, SHGDN_FORPARSING, FolderBuffer))
				*FolderBuffer = 0;
		}

		// Can't drop on to a folder when the source contains the folder,
		// or the source is directly inside the folder (no-op)
		if (*FolderBuffer)
		{
			const FolderLen = _tcslen(FolderBuffer);
			int SourceLen;
			for (LPCTSTR pSource = m_DropFiles; *pSource; pSource += SourceLen + 1)
			{
				SourceLen = _tcslen(pSource);
				if (_tcsnicmp(FolderBuffer, pSource, SourceLen) == 0 ||
					(_tcsnicmp(pSource, FolderBuffer, FolderLen) == 0 && pSource[FolderLen] == '\\' &&
					!_tcschr(pSource + FolderLen + 1, '\\')))
				{
					*FolderBuffer = 0;
					break;
				}
			}
		}
	}

	if (!*FolderBuffer)
		nItem = -1;

	if (nItem != m_LastDropTargetItem)
	{
		DragHighlightItem(m_LastDropTargetItem, FALSE);
		DragHighlightItem(nItem, TRUE);
		m_LastDropTargetItem = nItem;
	}

	return (*FolderBuffer != 0);
}

DROPEFFECT CFileBrowseView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	// Temporarily turn off main window as drop target so that it doesn't interfere
	CMainFrame::QCGetMainFrame()->DragAcceptFiles(FALSE);

	m_bDropSourceOnSameDrive = TRUE;

	if (pDataObject->IsDataAvailable(CF_HDROP))
	{
		HDROP hDrop = (HDROP)pDataObject->GetGlobalData(CF_HDROP);
		if (hDrop)
		{
			TCHAR FolderBuffer[MAX_PATH + 1];
			TCHAR DriveUs[_MAX_DRIVE + 1];
			if (GetName(m_piDesktopFolder, m_pCurrentIdList, SHGDN_FORPARSING, FolderBuffer))
				_tsplitpath(FolderBuffer, DriveUs, NULL, NULL, NULL);
			else
			{
				*DriveUs = 0;
				m_bDropSourceOnSameDrive = FALSE;
			}

			const UINT NumFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
			int FileListSize = 0;
			UINT i;
			for (i = 0; i < NumFiles; i++)
				FileListSize += (DragQueryFile(hDrop, i, NULL, 0) + 1) * sizeof(TCHAR);

			m_DropFiles = DEBUG_NEW TCHAR[FileListSize + 1];
			int Offset = 0;
			for (i = 0; i < NumFiles; i++)
			{
				LPTSTR pFileStart = m_DropFiles + Offset;
				Offset += (DragQueryFile(hDrop, i, pFileStart, MAX_PATH) + 1) * sizeof(TCHAR);

				if (m_bDropSourceOnSameDrive && *DriveUs)
				{
					TCHAR DriveThem[_MAX_DRIVE + 1];
					_tsplitpath(pFileStart, DriveThem, NULL, NULL, NULL);
					if (!*DriveThem || _tcsicmp(DriveUs, DriveThem))
						m_bDropSourceOnSameDrive = FALSE;
				}
			}

			m_DropFiles[Offset] = 0;

			GlobalFree(hDrop);
		}
	}

	return OnDragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CFileBrowseView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ClientToScreen(&point);
	if (m_DropFiles && WindowFromPoint(point) == &m_ListCtrl)
	{
		TCHAR FolderPath[MAX_PATH + 1];

		if (!GetTargetFolder(point, FolderPath))
			return DROPEFFECT_NONE;

		// Holding down control forces a copy
		if (dwKeyState & MK_CONTROL)
			return DROPEFFECT_COPY;
		// Holding down shift forces a move
		if (dwKeyState & MK_SHIFT)
			return DROPEFFECT_MOVE;

		// If no modifiers held down, then if the source is on
		// the same drive then do a move, otherwise do a copy
		return m_bDropSourceOnSameDrive? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	}

	return DROPEFFECT_NONE;
}

BOOL CFileBrowseView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	if ((dropEffect != DROPEFFECT_MOVE && dropEffect != DROPEFFECT_COPY) || !m_DropFiles)
		ASSERT(0);	// How did we get here?
	else
	{
		char SourcePath[MAX_PATH + 1];

		ClientToScreen(&point);
		if (!GetTargetFolder(point, SourcePath))
			ASSERT(0);
		else
		{
			SHFILEOPSTRUCT fos;
			ZeroMemory(&fos, sizeof(fos));
			fos.hwnd = GetSafeHwnd();
			fos.wFunc = (dropEffect == DROPEFFECT_MOVE)? FO_MOVE : FO_COPY;
			fos.pFrom = m_DropFiles;
			fos.pTo = SourcePath;
			fos.fFlags = FOF_ALLOWUNDO;

			SHFileOperation(&fos);
			OnCmdRefresh();	// Always refresh because a partial operation may have occured
		}
	}

	// Clean up
	OnDragLeave();

	// Always return TRUE, otherwise the parent frame window will treat it as a drop and
	// will wind up creating a new composition message and attaching the files to it
	return TRUE;
}

void CFileBrowseView::OnDragLeave()
{
	DragHighlightItem(m_LastDropTargetItem, FALSE);
	m_LastDropTargetItem = -1;

	if (m_DropFiles)
	{
		delete [] m_DropFiles;
		m_DropFiles = NULL;
	}

	// Restore main window as drop target
	CMainFrame::QCGetMainFrame()->DragAcceptFiles(TRUE);
}

void CFileBrowseView::DoDataExchange(CDataExchange* pDX)
{
	C3DFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileBrowseView)
	DDX_Control(pDX, IDC_FILE_BROWSE_COMBO, m_DriveCombo);
	DDX_Control(pDX, IDC_FILE_BROWSE_TREE, m_TreeCtrl);
	DDX_Control(pDX, IDC_FILE_BROWSE_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
}

void CFileBrowseView::SetFileListOnly(LPCTSTR RootDirectory /*= NULL*/)
{
	m_bFileListOnly = TRUE;
	m_FileListOnlyRootDir = RootDirectory;

	PostMessage(umsgActivateWazoo);
}

void CFileBrowseView::SetViewState(int NewState)
{
	m_ListCtrl.ModifyStyle(LVS_TYPEMASK, NewState);
}

int CFileBrowseView::GetViewState()
{
	return m_ListCtrl.GetStyle() & LVS_TYPEMASK;
}

/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView diagnostics

#ifdef _DEBUG
void CFileBrowseView::AssertValid() const
{
	C3DFormView::AssertValid();
}

void CFileBrowseView::Dump(CDumpContext& dc) const
{
	C3DFormView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CFileBrowseView message handlers


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [protected, virtual]
//
// Perform the Just In Time display initialization for this Wazoo.
////////////////////////////////////////////////////////////////////////
long CFileBrowseView::OnActivateWazoo(WPARAM, LPARAM)
{
	if (m_DriveCombo.GetSafeHwnd() && (NULL == m_piDesktopFolder))
	{
		HRESULT hr = ::SHGetDesktopFolder(&m_piDesktopFolder);
		VERIFY(SUCCEEDED(hr));
		ASSERT(m_piDesktopFolder);

		ASSERT(NULL == m_pDesktopIdList);
		hr = ::SHGetSpecialFolderLocation(GetSafeHwnd(), CSIDL_DESKTOP, &m_pDesktopIdList);
		VERIFY(SUCCEEDED(hr));
		ASSERT(m_pDesktopIdList);

		ASSERT(NULL == m_piMalloc);
		hr = ::SHGetMalloc(&m_piMalloc);
		VERIFY(SUCCEEDED(hr));
		ASSERT(m_piMalloc);

		if (!m_bFileListOnly)
		{
			//
			// Setup font, item height, and dropped width for combo box.
			//
			CFont* pTempFont = CFont::FromHandle(HFONT(::GetStockObject(ANSI_VAR_FONT)));
			ASSERT(pTempFont);
			m_DriveCombo.SetFont(pTempFont);

			TEXTMETRIC tm;
			::GetTextMetrics(pTempFont, tm);		// see wacky overloads in FONT.H
			m_DriveCombo.SetItemHeight(0, tm.tmHeight + tm.tmExternalLeading + 4);

			m_DriveCombo.SetDroppedWidth(200);

			// Get the handle to the system image list, for our icons
			SHFILEINFO sfi;
			HIMAGELIST hImageList = HIMAGELIST(SHGetFileInfo((LPCSTR) "C:\\", 
												0,
												&sfi, 
												sizeof(SHFILEINFO), 
												SHGFI_SYSICONINDEX | SHGFI_SMALLICON));

			// Attach ImageList to TreeView
			if (hImageList)
				::SendMessage(m_TreeCtrl.m_hWnd, TVM_SETIMAGELIST, WPARAM(TVSIL_NORMAL), LPARAM(hImageList));

			//
			// Clear out the combo box and refresh the contents.
			//
			ResetDriveCombo();
		}

		//
		// FORNOW, hardcode columns for the report view...
		//
		int NameWidth;
		int TypeWidth;
		int SizeWidth;
		int ModifiedWidth;

		if (m_bFileListOnly)
		{
			CRect rect;
			GetClientRect(&rect);
			int width = rect.Width();

			NameWidth		= width / 2;
			TypeWidth		= width / 5;
			SizeWidth		= width / 10;
			ModifiedWidth	= width - NameWidth - TypeWidth - SizeWidth - 3;
		}
		else
		{
			NameWidth = GetIniShort(IDS_INI_FILE_BROWSE_NAME_WIDTH);
			TypeWidth = GetIniShort(IDS_INI_FILE_BROWSE_TYPE_WIDTH);
			SizeWidth = GetIniShort(IDS_INI_FILE_BROWSE_SIZE_WIDTH);
			ModifiedWidth = GetIniShort(IDS_INI_FILE_BROWSE_MODIFIED_WIDTH);
		}

		m_ListCtrl.InsertColumn(0, CRString(IDS_FILE_BROWSE_NAME_LABEL), LVCFMT_LEFT, NameWidth);
		m_ListCtrl.InsertColumn(1, CRString(IDS_FILE_BROWSE_TYPE_LABEL), LVCFMT_LEFT, TypeWidth);
		m_ListCtrl.InsertColumn(2, CRString(IDS_FILE_BROWSE_SIZE_LABEL), LVCFMT_RIGHT, SizeWidth);
		m_ListCtrl.InsertColumn(3, CRString(IDS_FILE_BROWSE_MODIFIED_LABEL), LVCFMT_LEFT, ModifiedWidth);

		if (m_bFileListOnly)
		{
			CRect rect;
			GetWindowRect(&rect);
			OnSize(SIZE_RESTORED, rect.Width(), rect.Height());
			m_piRootShellFolder = m_piDesktopFolder;
			m_piRootShellFolder->AddRef();
			m_pRootIdList = ConcatPidls(NULL, m_pDesktopIdList);
			if (m_FileListOnlyRootDir.IsEmpty() == FALSE)
			{
				// Set root to something other than the desktop
				LPITEMIDLIST pFullyQualifiedIdList = GetFullyQualPidlFromPath(m_FileListOnlyRootDir);
				if (pFullyQualifiedIdList)
				{
					LPSHELLFOLDER pTempShellFolder = NULL;
					hr = m_piDesktopFolder->BindToObject(pFullyQualifiedIdList, 0, IID_IShellFolder, (LPVOID *) &pTempShellFolder);
					if (FAILED(hr) || !pTempShellFolder)
						m_piMalloc->Free(pFullyQualifiedIdList);
					else
					{
						m_piRootShellFolder->Release();
						m_piRootShellFolder = pTempShellFolder;

						if (m_pRootIdList)
							m_piMalloc->Free(m_pRootIdList);
						m_pRootIdList = pFullyQualifiedIdList;
					}
				}
			}
			ResetListView(m_piRootShellFolder, m_pRootIdList);
		}
	}

	return 0;
}

void CFileBrowseView::OnOK()
{
	if (GetFocus() == &m_ListCtrl)
	{
		int FocusIndex = m_ListCtrl.GetNextItem(-1, LVNI_FOCUSED);
		if (FocusIndex >= 0)
		{
			LVITEM lvItem;
			lvItem.mask = LVIF_PARAM;
			lvItem.iItem = FocusIndex;
			lvItem.iSubItem = 0;
			if (m_ListCtrl.GetItem(&lvItem))
				OpenItem((LPLISTDATA)lvItem.lParam);
		}
	}
}

void CFileBrowseView::NavToItem(int AtIndex)
{
	int Count = m_NavList.GetCount();

	if (AtIndex >= 0 && AtIndex < Count)
	{
		POSITION pos = m_NavList.FindIndex(AtIndex);
		NavItem* pNI;
		if (!pos || !(pNI = m_NavList.GetAt(pos)))
			ASSERT(0);
		else
		{
			if (!pNI->piShellFolder || !pNI->pIDL)
				ASSERT(0);
			else
			{
				// Need to act like we're at the end of the list so that the
				// existing items on the list don't get truncated
				m_CurrentNavListItem = Count - 1;

				if (ResetListView(pNI->piShellFolder, pNI->pIDL))
				{
					ASSERT(m_NavList.GetCount() == Count + 1);
					TruncateNavList(Count);
					m_CurrentNavListItem = AtIndex;
				}
			}
		}
	}
}

void CFileBrowseView::OnNavBack()
{
	NavToItem(m_CurrentNavListItem - 1);
}

void CFileBrowseView::OnNavForward()
{
	NavToItem(m_CurrentNavListItem + 1);
}

void CFileBrowseView::OnNavUp()
{
	if (m_bFileListOnly && m_piCurrentShellFolder != m_piRootShellFolder)
	{
		LPITEMIDLIST pTempParentIdList = CopyFullPidlExceptForLast(m_pCurrentIdList);
		if (GetSize(pTempParentIdList) == GetSize(m_pRootIdList))
		{
			// We're at the root, so just used cached member variables
			ResetListView(m_piRootShellFolder, m_pRootIdList);
		}
		else
		{
			LPSHELLFOLDER pTempShellFolder = NULL;
			HRESULT hr = m_piDesktopFolder->BindToObject(pTempParentIdList, 0, IID_IShellFolder, (LPVOID *) &pTempShellFolder);
			if (SUCCEEDED(hr) && pTempShellFolder)
			{
				ResetListView(pTempShellFolder, pTempParentIdList);
				pTempShellFolder->Release();
			}
		}
		m_piMalloc->Free(pTempParentIdList);
	}
}

void CFileBrowseView::OnSetHomeFolder()
{
	char szPath[MAX_PATH + 1];

	if (::SHGetPathFromIDList(m_pCurrentIdList, szPath))
	{
		SetIniString(IDS_INI_FILE_BROWSE_HOME, szPath);
		OnCmdRefresh();
	}
}

void CFileBrowseView::OnProperties()
{
	LPSHELLFOLDER pShellFolder = NULL;
	UINT uSelCount;
	LPITEMIDLIST* pPidlArray = CreateListSelectionPIDL(uSelCount);

	if (pPidlArray)
	{
		pShellFolder = m_piCurrentShellFolder;
		pShellFolder->AddRef();
	}
	else
	{
		uSelCount = 1;
		pPidlArray = (LPITEMIDLIST *) DEBUG_NEW int[1];
		pPidlArray[0] = GetLastPidl(m_pCurrentIdList);

		LPITEMIDLIST pTempParentIdList = CopyFullPidlExceptForLast(m_pCurrentIdList);
		HRESULT hr = m_piDesktopFolder->BindToObject(pTempParentIdList, 0, IID_IShellFolder, (LPVOID *) &pShellFolder);
		m_piMalloc->Free(pTempParentIdList);
		if (FAILED(hr) || !pShellFolder)
			return;
	}

	LPCONTEXTMENU piContextMenu = NULL;
	HRESULT hr;
	hr = pShellFolder->GetUIObjectOf(GetSafeHwnd(),
										uSelCount,  // get attributes for this many objects
										(const struct _ITEMIDLIST **) pPidlArray,
										IID_IContextMenu,
										0,
										(LPVOID *) &piContextMenu);
	if (SUCCEEDED(hr))  
	{
		ASSERT(piContextMenu != NULL);

		CMINVOKECOMMANDINFO cmi;

		ZeroMemory(&cmi, sizeof(cmi));
		cmi.cbSize = sizeof(cmi);
		cmi.hwnd = GetSafeHwnd();
		cmi.lpVerb = "properties";
		cmi.nShow = SW_SHOWNORMAL;
		
		piContextMenu->InvokeCommand(&cmi);
	}

	if (pShellFolder)
		pShellFolder->Release();

	delete [] pPidlArray;
}

void CFileBrowseView::OnNewFolder()
{
	// One would think that one could use IContextMenu and CMDSTR_NEWFOLDER to create a new
	// folder, but one would be wrong if one were running Windows 2000. So, we have to do
	// all the tedious work ourselves.

	TCHAR FolderPath[MAX_PATH + 1];

	if (GetName(m_piDesktopFolder, m_pCurrentIdList, SHGDN_FORPARSING, FolderPath))
	{
		TCHAR NewFolderPath[MAX_PATH + 1];
		for (int i = 1; true; i++)
		{
			_stprintf(NewFolderPath, "%s\\%s", FolderPath, (LPCTSTR)CRString(IDS_NEW_FOLDER));
			if (i > 1)
				_stprintf(NewFolderPath + _tcslen(NewFolderPath), " (%d)", i);

			if (!FileExistsMT(NewFolderPath))
				break;
		}

		if (CreateDirectoryMT(NewFolderPath))
			OnCmdRefresh();
	}
}

void CFileBrowseView::OnDelete()
{
	UINT uSelCount;
	LPITEMIDLIST* pPidlArray = CreateListSelectionPIDL(uSelCount);

	if (!pPidlArray)
		return;

	SHFILEOPSTRUCT sfos;
	ZeroMemory(&sfos, sizeof(sfos));
	sfos.hwnd = GetSafeHwnd();
	sfos.wFunc = FO_DELETE;

	TCHAR FilePath[MAX_PATH + 1];
	UINT TotalSize = 0;
	UINT i;

	// Loop through the list and count how much space we're
	// going to need to store all the filenames
	for (i = 0; i < uSelCount; i++)
	{
		if (GetName(m_piCurrentShellFolder, pPidlArray[i], SHGDN_FORPARSING, FilePath))
			TotalSize += _tcslen(FilePath) + 1;
	}

	// Now allocate the the whole list, and put the files in the list
	LPTSTR FileList = DEBUG_NEW TCHAR[TotalSize + 1];
	int Offset = 0;
	for (i = 0; i < uSelCount; i++)
	{
		if (GetName(m_piCurrentShellFolder, pPidlArray[i], SHGDN_FORPARSING, FilePath))
		{
			_tcscpy(FileList + Offset, FilePath);
			Offset += _tcslen(FilePath) + 1;
		}
	}
	FileList[Offset] = 0;

	sfos.pFrom = FileList;
	sfos.fFlags = FOF_SILENT;
	if (!ShiftDown())
		sfos.fFlags |= FOF_ALLOWUNDO;

	if (SHFileOperation(&sfos) == 0)
		OnCmdRefresh();

	delete [] FileList;
	delete [] pPidlArray;
}

void CFileBrowseView::OnViewLargeIcons()
{
	m_ListCtrl.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}

void CFileBrowseView::OnViewSmallIcons()
{
	m_ListCtrl.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}

void CFileBrowseView::OnViewList()
{
	m_ListCtrl.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}

void CFileBrowseView::OnViewDetails()
{
	m_ListCtrl.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}

void CFileBrowseView::OnUpdateNavBack(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CurrentNavListItem > 0);
}

void CFileBrowseView::OnUpdateNavForward(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CurrentNavListItem < m_NavList.GetCount() - 1);
}

void CFileBrowseView::OnUpdateNeedsListSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ListCtrl.GetSelectedCount() > 0);
}

void CFileBrowseView::OnUpdateNavUp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_piCurrentShellFolder != m_piRootShellFolder);
}

void CFileBrowseView::OnUpdateViewLargeIcons(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio((m_ListCtrl.GetStyle() & LVS_TYPEMASK) == LVS_ICON);
}

void CFileBrowseView::OnUpdateViewSmallIcons(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio((m_ListCtrl.GetStyle() & LVS_TYPEMASK) == LVS_SMALLICON);
}

void CFileBrowseView::OnUpdateViewList(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio((m_ListCtrl.GetStyle() & LVS_TYPEMASK) == LVS_LIST);
}

void CFileBrowseView::OnUpdateViewDetails(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio((m_ListCtrl.GetStyle() & LVS_TYPEMASK) == LVS_REPORT);
}

void CFileBrowseView::OnEditPaste()
{
	COleDataObject odo;
	VERIFY(odo.AttachClipboard());

	if (odo.IsDataAvailable(CF_HDROP))
	{
		// Find out if the data was put on the clipboard as a copy or a cut.
		// All failures lead to copy, just for safety.
		static CLIPFORMAT CF_PDE = 0;
		if (!CF_PDE)
			CF_PDE = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);

		DROPEFFECT de = DROPEFFECT_COPY;
		STGMEDIUM stgMedium;
		if (CF_PDE && odo.IsDataAvailable(CF_PDE))
		{
			if (odo.GetData(CF_PDE, &stgMedium))
			{
				DWORD* pdwDE = (DWORD*)GlobalLock(stgMedium.hGlobal);
				if (pdwDE && *pdwDE == DROPEFFECT_MOVE)
					de = DROPEFFECT_MOVE;
				GlobalUnlock(stgMedium.hGlobal);
			}
		}

		odo.GetData(CF_HDROP, &stgMedium);

		// We've already got drag-and-drop code that does "paste", so use it
		CPoint BogusPoint(-1, -1);
		OnDragEnter(&odo, 0, BogusPoint);
		OnDrop(&odo, de, BogusPoint);
	}
}

void CFileBrowseView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsClipboardFormatAvailable(CF_HDROP));
}


////////////////////////////////////////////////////////////////////////
// OnSize [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnSize(UINT nType, int cx, int cy) 
{
	C3DFormView::OnSize(nType, cx, cy);

	//
	// Don't bother if the controls haven't been initialized,
	// or if we're being minimized...
	//
	if ((NULL == m_DriveCombo.m_hWnd) || (SIZE_MINIMIZED == nType))
		return;

	int nListTop = 0;

	if (m_bFileListOnly)
	{
		m_DriveCombo.ShowWindow(SW_HIDE);
		m_TreeCtrl.ShowWindow(SW_HIDE);
	}
	else
	{
		m_DriveCombo.ShowWindow(SW_SHOWNA);
		m_TreeCtrl.ShowWindow(SW_SHOWNA);

		CRect rectCombo;
		m_DriveCombo.GetWindowRect(&rectCombo);
		ScreenToClient(&rectCombo);

		const int SPACE_HEIGHT = 4;         // vertical spacing, in pixels

		m_DriveCombo.MoveWindow(0, 0, cx, rectCombo.Height());
		int nTreeHeight = GetIniShort(IDS_INI_FILE_BROWSE_TREE_HEIGHT) - rectCombo.Height() - SPACE_HEIGHT;
		m_TreeCtrl.MoveWindow(0, rectCombo.Height() + SPACE_HEIGHT, cx, nTreeHeight);

		nListTop = rectCombo.Height() + 2*SPACE_HEIGHT + nTreeHeight;
	}

	m_ListCtrl.MoveWindow(0, nListTop, cx, cy - nListTop);
}


////////////////////////////////////////////////////////////////////////
// OnLButtonDown [protected, virtual]
//
// Handles mouse down messages over the MboxBar client area.  Most of
// the time, this just means calling the default handler to begin
// a move, dock, or undock maneuver.
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_trackerData.bIsTracking)
	{
		ASSERT(0);
		m_trackerData.bIsTracking = FALSE;
	}

	if (IsOnResizeBar(point) && (!(nFlags & MK_RBUTTON)))
	{
		m_trackerData.bIsTracking = TRUE;
		if (GetCapture() != this)
			SetCapture();

		TRACE2("CFileBrowseView::OnLButtonDown: Begin Resize processing (%d, %d)...\n", point.x, point.y);

		//
		// Setup rectangle for tracking ghosted resize bar.
		//
		GetResizeBarRect(m_trackerData.rect);

		//
		// Determine the left/right boundary limits in client
		// coordinates.
		//
		CRect rectClient;
		GetClientRect(rectClient);
		m_trackerData.nTopLimit = 80;
		m_trackerData.nBottomLimit = rectClient.Height() - 80;

		DrawTrackerGhost(m_trackerData.rect);		// draw new one
	}
	else
		C3DFormView::OnLButtonDown(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// OnLButtonUp [protected, virtual]
//
// Handles mouse up messages over the client area.  Most of
// the time, this just means calling the default handler to end
// a move, dock, or undock maneuver.
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (GetCapture() == this)
		ReleaseCapture();

	if (m_trackerData.bIsTracking)
	{
		TRACE2("CFileBrowseView::OnLButtonDown: End Resize processing (%d, %d).\n", point.x, point.y);
		DrawTrackerGhost(m_trackerData.rect);	// erase old one

		SetIniShort(IDS_INI_FILE_BROWSE_TREE_HEIGHT, short(m_trackerData.rect.top + 1));
		CRect rectClient;
		GetClientRect(rectClient);
		OnSize(SIZE_RESTORED, rectClient.Width(), rectClient.Height());
		Invalidate();
		m_trackerData.bIsTracking = FALSE;
		return;
	}

	C3DFormView::OnLButtonUp(nFlags, point);
}


////////////////////////////////////////////////////////////////////////
// OnMouseMove [protected, virtual]
//
// Handles mouse move messages over the MboxBar client area.
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnMouseMove(UINT nFlags, CPoint pt) 
{
	if (m_trackerData.bIsTracking)
	{
		//
		// Resize ghosting in progress, so redraw the ghost in the
		// new cursor location ...
		//
		//TRACE2("CFileBrowseView::OnLButtonDown: handling resize processing! (%d, %d)\n", pt.x, pt.y);

		DrawTrackerGhost(m_trackerData.rect);	// erase old one
		if (pt.y < m_trackerData.nTopLimit)
			m_trackerData.rect.top = m_trackerData.nTopLimit;
		else if (pt.y > m_trackerData.nBottomLimit)
			m_trackerData.rect.top = m_trackerData.nBottomLimit;
		else
		{
			// fudging the value a bit seems to line it up with the cursor better
			m_trackerData.rect.top = pt.y - 3;
		}
		m_trackerData.rect.bottom = m_trackerData.rect.top + TRACKER_HEIGHT;
		DrawTrackerGhost(m_trackerData.rect);	// draw new one
	}
	else
	{
		if (IsOnResizeBar(pt) && (!(nFlags & MK_LBUTTON)) && (!(nFlags & MK_RBUTTON)))
		{
			if (NULL == m_hResizeCursor)
			{
				HINSTANCE h_inst = QCFindResourceHandle(MAKEINTRESOURCE(AFX_IDC_VSPLITBAR), RT_GROUP_CURSOR);

				// load up the resize cursor
				if ((m_hResizeCursor = ::LoadCursor(h_inst, MAKEINTRESOURCE(AFX_IDC_VSPLITBAR))) == NULL)
				{
					ASSERT(0); // Could not find splitter cursor
				}
			}
			::SetCursor(m_hResizeCursor);
		}
		else
		{
			//
			// Restore default cursor.
			//
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		}
	}
}


////////////////////////////////////////////////////////////////////////
// OnSetCursor [protected, virtual]
//
// Avoids flicker of resize cursor when mouse is dragged over resize
// bar.
//
////////////////////////////////////////////////////////////////////////
BOOL CFileBrowseView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT && pWnd == this && !m_trackerData.bIsTracking)
		return TRUE;    // we will handle it in the mouse move

	return C3DFormView::OnSetCursor(pWnd, nHitTest, message);
}


////////////////////////////////////////////////////////////////////////
// OnCmdRefresh [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnCmdRefresh() 
{
	if (m_bFileListOnly)
	{
		ResetListView(m_piCurrentShellFolder, m_pCurrentIdList);
		return;
	}

	//
	// Fetch the home directory and current attachment folder setting and
	// if either one is different than one we've seen before, then reset
	// the drive combo box, too.
	// Otherwise, leave the drive combo box alone and just refresh the
	// tree and list views.
	//
	CCursor busyCursor;

	char szDir[_MAX_PATH + 1];
	bool bNeedReset = false;

	::GetIniString(IDS_INI_FILE_BROWSE_HOME, szDir, sizeof(szDir));
	if (m_strHomeDir != szDir)
		bNeedReset = true;

	::GetIniString(IDS_INI_AUTO_RECEIVE_DIR, szDir, sizeof(szDir));
	if (!*szDir)
		::wsprintf(szDir, "%s%s", (const char *) ::EudoraDir, (const char *) CRString(IDS_ATTACH_FOLDER));
	if (m_strAttachDir != szDir)
		bNeedReset = true;

	if (bNeedReset)
		ResetDriveCombo();
	
	//
	// Grab the current selected folder, if any, in the tree control
	// and re-populate the contents of the selected folder.
	//
	HTREEITEM hSelectedItem = m_TreeCtrl.GetSelectedItem();
	if (NULL == hSelectedItem)
	{
		::MessageBeep(MB_OK);
		return;
	}

	//
	// Delete any existing children of the selected folder.
	//
	if (m_TreeCtrl.ItemHasChildren(hSelectedItem))
	{
		HTREEITEM hChildItem = NULL;
		while ((hChildItem = m_TreeCtrl.GetChildItem(hSelectedItem)) != NULL)
		{
			// FORNOW, does this do a proper recursive delete?
			// FORNOW, does this cleanup the item data memory?
			m_TreeCtrl.DeleteItem(hChildItem);
		}
	}

	LPTREEDATA pTreeData = (LPTREEDATA) m_TreeCtrl.GetItemData(hSelectedItem);
	if (NULL == pTreeData)
	{
		ASSERT(0);
		return;
	}

	ASSERT(pTreeData->pFullyQualifiedIdList);
	if (pTreeData->piParentShellFolder)
	{
		//
		// Non-Desktop item.
		//
		ASSERT(pTreeData->pIdList);
		{
			//
			// Before we re-populate the selected tree node, let's
			// do a test to see if is still valid.
			//
			ULONG ulAttrs = SFGAO_VALIDATE;
			HRESULT hrAttr = pTreeData->piParentShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&(pTreeData->pIdList), &ulAttrs);
			if (FAILED(hrAttr))
			{
				//
				// Oops.  Can't repopulate this tree node anymore since
				// it doesn't exist.  Need to nuke this node and
				// repopulate its parent.
				// 
				ASSERT(hSelectedItem);
				HTREEITEM hParentItem =  m_TreeCtrl.GetParentItem(hSelectedItem);
				// FORNOW, does this do a proper recursive delete?
				// FORNOW, does this cleanup the item data memory?
				m_TreeCtrl.DeleteItem(hSelectedItem);
				if (hParentItem)
				{
					m_TreeCtrl.SelectItem(hParentItem);
					OnCmdRefresh();		// let's get recursive!
					return;
				}
			}
		}
		LPSHELLFOLDER piChildShellFolder = NULL;
		HRESULT hr = pTreeData->piParentShellFolder->BindToObject(pTreeData->pIdList, 0, IID_IShellFolder, (LPVOID *) &piChildShellFolder);
		if (SUCCEEDED(hr))
		{
			PopulateTreeFolder(piChildShellFolder, pTreeData->pFullyQualifiedIdList, hSelectedItem);
			ResetListView(piChildShellFolder, pTreeData->pFullyQualifiedIdList);
			piChildShellFolder->Release();
		}
	}
	else
	{
		//
		// Desktop Item.  Get the desktop folder interface and PIDL.
		//
		ASSERT(NULL == pTreeData->pIdList);
		PopulateTreeFolder(m_piDesktopFolder, m_pDesktopIdList, hSelectedItem);
		ResetListView(m_piDesktopFolder, m_pDesktopIdList);
	}

	TV_SORTCB tvscb;
	tvscb.hParent     = hSelectedItem;
	tvscb.lpfnCompare = TreeViewCompareProc;
	tvscb.lParam      = 0;

	m_TreeCtrl.SendMessage(TVM_SORTCHILDRENCB, WPARAM(0), LPARAM(&tvscb));
}


////////////////////////////////////////////////////////////////////////
// ResetDriveCombo [protected]
//
// Fills the combo box with top-level desktop items, down to the
// file volume level as in a standard Explorer window.
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::ResetDriveCombo()
{
	if (m_bFileListOnly)
		return;

	//
	// First things first.  Blow away the existing contents.
	//
	m_DriveCombo.ResetContent();        // FORNOW, doesn't delete item storage?

	//
	// Add a top-level entry for the Home folder.
	//
	bool bHaveHomeFolder = false;;
	{
		char szHomeDir[_MAX_PATH + 1];

		::GetIniString(IDS_INI_FILE_BROWSE_HOME, szHomeDir, sizeof(szHomeDir));
		if (*szHomeDir)
		{
			AddDriveComboFolder(CRString(IDS_FILE_BROWSE_HOME_NAME), szHomeDir);
			bHaveHomeFolder = true;
		}

		//
		// Cache this value so that we know whether or not to refresh
		// the drive combo with a potentially new home folder name.
		//
		m_strHomeDir = szHomeDir;
	}

	//
	// Add a top-level entry for the Eudora attachments folder.
	//
	{
		char szAttachDir[_MAX_PATH + 1];

		::GetIniString(IDS_INI_AUTO_RECEIVE_DIR, szAttachDir, sizeof(szAttachDir));
		if (!*szAttachDir)
			::wsprintf(szAttachDir, "%s%s", (const char *) ::EudoraDir, (const char *) CRString(IDS_ATTACH_FOLDER));
		AddDriveComboFolder(CRString(IDS_MY_ATTACHMENTS), szAttachDir);

		//
		// Cache this value so that we know whether or not to refresh
		// the drive combo with a potentially new attachment folder
		// name.
		//
		m_strAttachDir = szAttachDir;
	}

	//
	// Get display name and small icon index for "root" Desktop folder.
	//
	{
		SHFILEINFO info;
		::SHGetFileInfo(LPCSTR(m_pDesktopIdList), 0, &info, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		int nIndex = m_DriveCombo.AddString(info.szDisplayName);
		if (nIndex != CB_ERR)
		{
			LPCOMBODATA pComboData = LPCOMBODATA(m_piMalloc->Alloc(sizeof(COMBODATA)));
			if (pComboData)
			{
				pComboData->piParentShellFolder = NULL;
				pComboData->piShellFolder = m_piDesktopFolder;
				m_piDesktopFolder->AddRef();
				pComboData->pIdList = NULL;
				pComboData->pFullyQualifiedIdList = ConcatPidls(NULL, m_pDesktopIdList);
				int nDummy;
				GetIconIndexes(pComboData->pFullyQualifiedIdList, pComboData->nIconIndex, nDummy);
				pComboData->nIndentLevel = 0;
				m_DriveCombo.SetItemDataPtr(nIndex, pComboData);
			}
		}
	}

	//
	// Get the PIDL for the "My Computer" folder.
	//
	LPITEMIDLIST pMyComputerIdList = NULL;
	HRESULT hr = ::SHGetSpecialFolderLocation(GetSafeHwnd(), CSIDL_DRIVES, &pMyComputerIdList);
	if (SUCCEEDED(hr))
	{
		ASSERT(pMyComputerIdList);

		PopulateDriveComboFolder(1, pMyComputerIdList, m_piDesktopFolder, m_pDesktopIdList);
		m_DriveCombo.SetCurSel(bHaveHomeFolder? 0 : GetWindowsDriveComboIndex());
		OnDriveComboSelchange();

		m_piMalloc->Free(pMyComputerIdList);
	}
}


////////////////////////////////////////////////////////////////////////
// PopulateDriveComboFolder [protected]
//
// Given a shell folder, enumerate the items in that folder, adding them
// to the combo box.
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::PopulateDriveComboFolder
(
	int nLevel,                             //(i) folder nesting level (0 is root, 1 is first level, etc)
	LPITEMIDLIST pMyComputerIdList,        //(i) pre-allocated PIDL for MyComputer folder (can be NULL)
	LPSHELLFOLDER piShellFolder,            //(i) shell folder to populate
	LPITEMIDLIST pFullyQualifiedIdList     //(i) fully-qualified PIDL to shell folder
) 
{
	if (m_bFileListOnly)
		return;

	ASSERT(piShellFolder);
	ASSERT(pFullyQualifiedIdList);

	//
	// Oh, bother.  The use of the "goto" cleanup mechanism requires 
	// that we initialize all temporaries before issuing the first
	// goto statement.
	//
	LPITEMIDLIST pIdList = NULL;
	LPENUMIDLIST pEnumIdList = NULL;
	ULONG ulNumItems = 0;
	CCursor busyCursor;			// display wait cursor

	//
	// Enumerate the folder and non-folder objects underneath the given shell folder.
	//
	HRESULT hr = piShellFolder->EnumObjects(GetSafeHwnd(), SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnumIdList);
	if (FAILED(hr))
		goto Cleanup;

	//
	// Walk the objects for the current subfolder.
	//
	while (S_OK == pEnumIdList->Next(1, &pIdList, &ulNumItems))
	{
		//
		// Determine what type of object you have.
		//
		ULONG ulAttrs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM;
		piShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&pIdList, &ulAttrs);

		if (ulAttrs & (SFGAO_HASSUBFOLDER | SFGAO_FOLDER))
		{
//#ifdef _DEBUG
//			{
//				char szDebugName[MAX_PATH] = { '\0' };
//				GetName(piShellFolder, pIdList, SHGDN_NORMAL, szDebugName);
//				CString strDebug(szDebugName);
//				if (ulAttrs & SFGAO_FOLDER) 
//					strDebug += " FOLDER";
//				if (ulAttrs & SFGAO_HASSUBFOLDER) 
//					strDebug += " HASSUBFOLDER";
//				if (ulAttrs & SFGAO_FILESYSTEM) 
//					strDebug += " FILESYSTEM";
//				if (ulAttrs & SFGAO_FILESYSANCESTOR)
//					strDebug += " FILESYSANCESTOR";
//				TRACE1("CFileBrowseView::PopulateDriveComboFolder: %s\n", strDebug);
//			}
//#endif // _DEBUG

			//
			// You need this next if statement to avoid adding objects
			// that are not real folders.  Some objects can have subfolders,
			// but aren't real folders.
			//
			if (ulAttrs & SFGAO_FOLDER)
			{
				//
				// Now get the friendly name to put in the combo box.
				//
				char szBuff[MAX_PATH];
				if (! GetName(piShellFolder, pIdList, SHGDN_NORMAL, szBuff))
					goto Cleanup; // Error - could not get friendly name

				//
				// Add the name to combo box, properly accounting for 
				// folder nesting depth.
				//
				int nIndex = m_DriveCombo.AddString(szBuff);
				if (nIndex != CB_ERR)
				{
					LPCOMBODATA pComboData = LPCOMBODATA(m_piMalloc->Alloc(sizeof(COMBODATA)));
					if (pComboData)
					{
						LPSHELLFOLDER piChildShellFolder = NULL;
						hr = piShellFolder->BindToObject(pIdList, 0, IID_IShellFolder, (LPVOID *) &piChildShellFolder);
						if (FAILED(hr))
						{
							m_piMalloc->Free(pComboData);
							goto Cleanup;
						}
						pComboData->piParentShellFolder = piShellFolder;
						piShellFolder->AddRef();
						pComboData->piShellFolder = piChildShellFolder;
						piChildShellFolder->AddRef();
						pComboData->pFullyQualifiedIdList = ConcatPidls(pFullyQualifiedIdList, pIdList);
						pComboData->pIdList = CopySinglePidl(pIdList);
						int nDummy;
						GetIconIndexes(pComboData->pFullyQualifiedIdList, pComboData->nIconIndex, nDummy);
						pComboData->nIndentLevel = nLevel;
						m_DriveCombo.SetItemDataPtr(nIndex, pComboData);
					}
				}

				//
				// Check to see if the current folder object is "My
				// Computer".  If so, walk the list of items.
				//
				if (pMyComputerIdList && (piShellFolder->CompareIDs(0, pMyComputerIdList, pIdList) == 0))
				{
					//
					// Recursively process "My Computer" subfolder.
					//
					LPSHELLFOLDER piMyComputerFolder = NULL;
					hr = piShellFolder->BindToObject(pIdList, 0, IID_IShellFolder, (LPVOID *) &piMyComputerFolder);
					if (FAILED(hr))
						goto Cleanup;
					PopulateDriveComboFolder(nLevel + 1, NULL, piMyComputerFolder, pMyComputerIdList);
					piMyComputerFolder->Release();
				}
			}
		}

		//
		// Free the PIDL we got back from the PIDL enumerator.
		//
		m_piMalloc->Free(pIdList);
		pIdList = NULL;
	}

Cleanup:
	if (pEnumIdList)  
		pEnumIdList->Release();

	//
	// The following should happen only on error...
	//
	if (pIdList)           
		m_piMalloc->Free(pIdList);
}


void CFileBrowseView::AddDriveComboFolder(const CString& strFolderName, const CString& strFolderPathname)
{
	if (m_bFileListOnly)
		return;

	ASSERT(m_piDesktopFolder);

	//
	// Step 1.  Convert the given pathname into a fully-qualified PIDL.
	//
	LPITEMIDLIST pFullyQualifiedIdList = GetFullyQualPidlFromPath(strFolderPathname);

	if (!pFullyQualifiedIdList)
		return;

	//
	// Step 2.  Create new combo box entry and corresponding data record.
	//
	int nIndex = m_DriveCombo.AddString(strFolderName);
	if (CB_ERR == nIndex)
	{
		m_piMalloc->Free(pFullyQualifiedIdList);
		return;
	}

	LPCOMBODATA pComboData = LPCOMBODATA(m_piMalloc->Alloc(sizeof(COMBODATA)));
	if (NULL == pComboData)
	{
		m_piMalloc->Free(pFullyQualifiedIdList);
		return;
	}

	//
	// Step 3.  Initialize data record fields.
	//

	//Set combo box "piShellFolder" field
	HRESULT hr = m_piDesktopFolder->BindToObject(pFullyQualifiedIdList, 0, IID_IShellFolder, (LPVOID *) &pComboData->piShellFolder);
	if (FAILED(hr))
	{
		m_piMalloc->Free(pFullyQualifiedIdList);
		m_piMalloc->Free(pComboData);
		return;
	}

	// Set combo box "piParentShellFolder" field
	LPITEMIDLIST pTempParentIdList = CopyFullPidlExceptForLast(pFullyQualifiedIdList);
	hr = m_piDesktopFolder->BindToObject(pTempParentIdList, 0, IID_IShellFolder, (LPVOID *) &pComboData->piParentShellFolder);
	m_piMalloc->Free(pTempParentIdList);
	if (FAILED(hr))
	{
		pComboData->piShellFolder->Release();
		m_piMalloc->Free(pFullyQualifiedIdList);
		m_piMalloc->Free(pComboData);
		return;
	}

	// Set combo box "pIdList" field
	ASSERT(pComboData->piShellFolder);
	pComboData->pIdList = CopySinglePidl(GetLastPidl(pFullyQualifiedIdList));

	// Set combo box "pFullyQualifiedIdList" field
	pComboData->pFullyQualifiedIdList = pFullyQualifiedIdList;
	int nDummy;
	GetIconIndexes(pComboData->pFullyQualifiedIdList, pComboData->nIconIndex, nDummy);
	pComboData->nIndentLevel = 0;
	m_DriveCombo.SetItemDataPtr(nIndex, pComboData);
}


////////////////////////////////////////////////////////////////////////
// GetWindowsDriveComboIndex [protected]
//
// Walk through the list of entries in the drive combo box, returning
// the index of the item that represents the volume containing the
// Windows folder.  Otherwise, return a hardcoded value of '1' to
// represent the "My Computer" item.
////////////////////////////////////////////////////////////////////////
int CFileBrowseView::GetWindowsDriveComboIndex()
{
	char szWindowsPath[MAX_PATH];
	::GetWindowsDirectory(szWindowsPath, sizeof(szWindowsPath));

	//
	// Grab just the drive letter.
	//
	{
		char szDrivePath[_MAX_DRIVE];
		_splitpath(szWindowsPath, szDrivePath, NULL, NULL, NULL);
		if (strlen(szDrivePath) > 0)
		{
			strcpy(szWindowsPath, szDrivePath);
			ASSERT(szWindowsPath[strlen(szWindowsPath) - 1] != '\\');
			strcat(szWindowsPath, "\\");
		}
	}

	//
	// Get the PIDL for the root of the drive containing the 
	// Windows directory.
	//

	for (int nIndex = 0; nIndex < m_DriveCombo.GetCount(); nIndex++)
	{
		LPCOMBODATA pComboData = LPCOMBODATA(m_DriveCombo.GetItemDataPtr(nIndex));
		if (pComboData)
		{
			char szPath[MAX_PATH];

			if (::SHGetPathFromIDList(pComboData->pFullyQualifiedIdList, szPath))
			{
				if (stricmp(szPath, szWindowsPath) == 0)
					return nIndex;
			}
		}
		else
		{
			ASSERT(0);
		}
	}

	//
	// If all else fails and we end up here ... just return the
	// value "1", which should be the index for the "My Computer" item.
	//
	return 1;
}


////////////////////////////////////////////////////////////////////////
// OnDriveComboSelchange [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnDriveComboSelchange()
{
	//
	// Extract the "item data" associated with the selection
	// then use that info to reset the corresponding tree view.
	//
	LPCOMBODATA pComboData = LPCOMBODATA(m_DriveCombo.GetItemDataPtr(m_DriveCombo.GetCurSel()));
	if (NULL == pComboData)
	{
		ASSERT(0);
		return;
	}

	//
	// Reset tree control to point to top of this tree.
	//
	ASSERT(! m_bOutaHere);
	m_bOutaHere = TRUE;
	BOOL bStatus = ResetTreeRoot(pComboData->piShellFolder, pComboData->pFullyQualifiedIdList, pComboData->piParentShellFolder, pComboData->pIdList);
	m_bOutaHere = FALSE;

	HTREEITEM hRootItem = m_TreeCtrl.GetRootItem();
	if (hRootItem)
	{
		if (bStatus)
		{
			//
			// Tree filled in successfully, so select the root item 
			// to automagically refresh the list view.
			//
			m_TreeCtrl.SelectItem(hRootItem);
		}
		else
		{
			//
			// Couldn't populate the tree fully, so clear the list view
			// of any remnant items.
			//
			m_ListCtrl.DeleteAllItems();
		}
	}
}


////////////////////////////////////////////////////////////////////////
// ResetTreeRoot [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CFileBrowseView::ResetTreeRoot
(
	LPSHELLFOLDER piShellFolder,		//(i) shell folder for the new root
	LPITEMIDLIST pFullyQualifiedIdList,	//(i) fully-qualified PIDL to root folder
	LPSHELLFOLDER piParentShellFolder,	//(i) parent of the new root shell folder (NULL for Desktop folder)
	LPITEMIDLIST pIdList				//(i) short PIDL to root folder (NULL for Desktop folder)
)
{
	if (m_bFileListOnly)
		return FALSE;

	ASSERT(piShellFolder);
	ASSERT(pFullyQualifiedIdList);

	// Initialize the tree view control to be empty.
	m_TreeCtrl.SetRedraw(FALSE);
	m_TreeCtrl.DeleteAllItems();

	//
	// Insert a new "root level" entry, saving the handle to the newly
	// created root item.  Then, get the display name for "root" folder.
	//
	SHFILEINFO info;
	::SHGetFileInfo(LPCSTR(pFullyQualifiedIdList), 0, &info, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME);
	HTREEITEM hTreeRoot = AddTreeItem(NULL, info.szDisplayName, FALSE, pFullyQualifiedIdList, piParentShellFolder, pIdList);
	if (NULL == hTreeRoot)
	{
		m_TreeCtrl.SetRedraw(TRUE);
		return FALSE;
	}

	// Fill in the first level children of the new root.
	BOOL bStatus = PopulateTreeFolder(piShellFolder, pFullyQualifiedIdList, hTreeRoot);
	UINT uState = m_TreeCtrl.GetItemState(hTreeRoot, TVIS_EXPANDED | TVIS_EXPANDEDONCE);
	m_TreeCtrl.SetItemState(hTreeRoot, uState | TVIS_EXPANDEDONCE, TVIS_EXPANDED | TVIS_EXPANDEDONCE);
	m_TreeCtrl.Expand(hTreeRoot, TVE_EXPAND);

	//
	// Sort the tree items using our custom callback function as the
	// compare routine.
	//
	TV_SORTCB tvscb;
	tvscb.hParent       = hTreeRoot;
	tvscb.lpfnCompare   = TreeViewCompareProc;
	tvscb.lParam        = 0;

	::SendMessage(m_TreeCtrl.m_hWnd, TVM_SORTCHILDRENCB, WPARAM(0), LPARAM(&tvscb));

	m_TreeCtrl.SetRedraw(TRUE);
	return bStatus;
}


/****************************************************************************
*
*  FUNCTION: PopulateTreeFolder(LPSHELLFOLDER piShellFolder,
*                          LPITEMIDLIST  pFullyQualifiedIdList,
*                          HTREEITEM     hParent)
*
*  PURPOSE: Fills a branch of the TreeView control.  Given the
*           shell folder, enumerate the subitems of this folder,
*           and add the appropriate items to the tree.
*
*  PARAMETERS:
*    piShellFolder         - Pointer to shell folder that we want to enumerate items 
*    pFullyQualifiedIdList        - Fully qualified item id list to the item that we are enumerating
*                   items for.  In other words, this is the PIDL to the item
*                   identified by the piShellFolder parameter.
*    hParent      - Parent node
*
*  COMMENTS:
*    This function enumerates the items in the folder identifed by piShellFolder.
*    Note that since we are filling the left hand pane, we will only add
*    items that are folders and/or have sub-folders.  We *could* put all
*    items in here if we wanted, but that's not the intent.
*
****************************************************************************/
BOOL CFileBrowseView::PopulateTreeFolder 
(
	LPSHELLFOLDER piShellFolder,		//(i) shell folder to be populated
	LPITEMIDLIST pFullyQualifiedIdList,	//(i) fully-qualified PIDL to shell folder
	HTREEITEM hParent					//(i) tree node to be populated
)
{
	if (m_bFileListOnly)
		return FALSE;

	ASSERT(piShellFolder);
	ASSERT(pFullyQualifiedIdList);
	ASSERT(hParent);

	//
	// Oh, bother.  The use of the "goto" cleanup mechanism requires 
	// that we initialize all temporaries before issuing the first
	// goto statement.
	//
	LPITEMIDLIST pIdList = NULL;
	LPENUMIDLIST piEnumList = NULL;
	LPITEMIDLIST pFullyQualifiedIdList2 = NULL;

	// Get the IEnumIDList object for the given folder.
	CCursor busyCursor;
	HRESULT hr = piShellFolder->EnumObjects(GetSafeHwnd(), SHCONTF_FOLDERS, &piEnumList);
	BOOL bStatus = FALSE;		// guilty until proven innocent
	if (SUCCEEDED(hr))
	{
		// Enumerate through the list of folder objects.
		ULONG ulFetched = 0;
		while (S_OK==piEnumList->Next(1, &pIdList, &ulFetched))
		{
			// Create a fully qualified path to the current item.
			// The SH* functions take a fully qualified path PIDL,
			// while the interface member functions take a 
			// relative path PIDL.
			ULONG ulAttrs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER;

			// Determine what type of object you have.
			piShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&pIdList, &ulAttrs);

			if (ulAttrs & (SFGAO_HASSUBFOLDER | SFGAO_FOLDER))
			{
				//
				// You need this next if statement to avoid adding
				// objects that are not real folders to the tree.  Some
				// objects can have subfolders, but aren't real folders.
				//
				if (ulAttrs & SFGAO_FOLDER)
				{
					BOOL bHasChildren = FALSE;
					if (ulAttrs & SFGAO_HASSUBFOLDER)
						bHasChildren = TRUE;

					// Now get the friendly name to 
					// put in the tree view control.
					char szBuff[MAX_PATH];
					if (! GetName(piShellFolder, pIdList, SHGDN_NORMAL, szBuff))
						goto Cleanup;   // Error - could not get friendly name

					if ((pFullyQualifiedIdList2 = ConcatPidls(pFullyQualifiedIdList, pIdList)) == NULL)
						goto Cleanup;

					if (NULL == AddTreeItem(hParent, szBuff, bHasChildren, pFullyQualifiedIdList2, piShellFolder, pIdList))
						goto Cleanup;
				}
			}

			m_piMalloc->Free(pFullyQualifiedIdList2);	// free PIDL we created
			pFullyQualifiedIdList2 = NULL;
			m_piMalloc->Free(pIdList);		// free PIDL the shell gave you
			pIdList = NULL;
		}

		//
		// If we make it this far, everything went okay.
		//
		bStatus = TRUE;
	}
	else
	{
		//
		// Analyze failure code for Win32 errors vs. simple enumeration
		// request failure.
		//
		DWORD hrFacility = HRESULT_FACILITY(hr);
		DWORD hrCode = HRESULT_CODE(hr);
		TRACE2("CFileBrowseView::PopulateTreeFolder: EnumObjects failed: Facility=0x%lX, Code=0x%lX\n", hrFacility, hrCode);

		if (E_FAIL == hr)
		{
			//
			// This is gonna sound weird, but a generic failure 
			// really just means there are no subfolders
			// in the given folder.  Return TRUE so that the caller
			// knows that it is okay to try enumerating non-folder
			// items in the list control.
			//
			bStatus = TRUE;
		}
		else if (FACILITY_WIN32 == hrFacility)
		{
			//
			// Assume that a Win32 error is due to accessing an
			// "offline" device or some similar situation where we
			// could not populate the tree items.  Return a FALSE
			// status so that the caller knows not to try enumerating
			// the items again in the list control.
			//
			bStatus = FALSE;
		}
		else
			bStatus = FALSE;		// unfiltered error
	}

Cleanup:
	if (piEnumList)  
		piEnumList->Release();

	// The following two if statements will be TRUE only if you got here
	// on an error condition from the goto statement.  Otherwise, free 
	// this memory at the end of the while loop above.
	if (pIdList)           
		m_piMalloc->Free(pIdList);

	if (pFullyQualifiedIdList2)
		m_piMalloc->Free(pFullyQualifiedIdList2);

	return bStatus;
}


////////////////////////////////////////////////////////////////////////
// AddTreeItem [protected]
//
// Convenient package for adding a new item to the tree control.
////////////////////////////////////////////////////////////////////////
HTREEITEM CFileBrowseView::AddTreeItem
(
	HTREEITEM hParent, 
	const CString& strName, 
	BOOL bHasChildren,
	LPITEMIDLIST pFullyQualifiedIdList,    //(i) fully-qualified PIDL to shell folder
	LPSHELLFOLDER piParentShellFolder,      //(i) parent shell folder (NULL for Desktop folder)
	LPITEMIDLIST pIdList                   //(i) short PIDL to shell folder (NULL for Desktop folder)
)
{
	ASSERT(pFullyQualifiedIdList);

	TV_INSERTSTRUCT tvstruct;               // tree view insert structure
	tvstruct.hParent = hParent;
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvstruct.item.hItem = 0;
	tvstruct.item.state = 0;
	tvstruct.item.stateMask = 0;    
	tvstruct.item.pszText = (char *) ((const char *) strName);
	tvstruct.item.cchTextMax = -1;          // unused
	tvstruct.item.iImage = -1;              // normally overwritten later
	tvstruct.item.iSelectedImage = -1;      // normally overwritten later
	tvstruct.item.lParam = NULL;            // normally overwritten later

	if (bHasChildren)
	{
		//
		// This item has subfolders, so put a plus sign in
		// the tree view control. The first time the user clicks
		// the item, you should populate the sub-folders.
		//
		tvstruct.item.cChildren = 1;
		tvstruct.item.mask |= TVIF_CHILDREN;
	}

	//
	// Populate a heap-allocated tree item data record, then associate
	// it with the tree item to be added.
	//
	LPTREEDATA pTreeData = LPTREEDATA(m_piMalloc->Alloc(sizeof(TREEDATA)));
	if (NULL == pTreeData)
		return FALSE;

	if (piParentShellFolder)
	{
		pTreeData->piParentShellFolder = piParentShellFolder;
		piParentShellFolder->AddRef();
		pTreeData->pIdList = CopySinglePidl(pIdList);
	}
	else
	{
		ASSERT(NULL == pIdList);
		pTreeData->piParentShellFolder = NULL;
		pTreeData->pIdList = NULL;
	}

	pTreeData->pFullyQualifiedIdList = ConcatPidls(NULL, pFullyQualifiedIdList);

	GetIconIndexes(pTreeData->pFullyQualifiedIdList, tvstruct.item.iImage, tvstruct.item.iSelectedImage);
	tvstruct.item.lParam = LPARAM(pTreeData);

	//
	// Display the file share icon overlay image as needed.
	//
	if (piParentShellFolder)
	{
		ULONG ulAttrs = SFGAO_DISPLAYATTRMASK;
		piParentShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&pIdList, &ulAttrs);
		if (ulAttrs & SFGAO_SHARE)
		{
			tvstruct.item.mask |= TVIF_STATE;
			tvstruct.item.stateMask = LVIS_OVERLAYMASK;
			tvstruct.item.state = INDEXTOOVERLAYMASK(1);
		}
	}

	//
	// Finally, after all that, we can do the add...
	//
	HTREEITEM hNewItem = m_TreeCtrl.InsertItem(&tvstruct);
	if (hNewItem)
		return hNewItem;

	//
	// Uh, oh.  If we get this far, we failed to insert the item into
	// the tree.  Gotta do some cleanup before we go home.
	//
	if (piParentShellFolder)
	{
		piParentShellFolder->Release();
		if (pTreeData->pIdList)
			m_piMalloc->Free(pTreeData->pIdList);
	}
	if (pTreeData->pFullyQualifiedIdList)
		m_piMalloc->Free(pTreeData->pFullyQualifiedIdList);
	m_piMalloc->Free(pTreeData);
	return NULL;
}


/****************************************************************************
*
*    FUNCTION: GetIcon(LPITEMIDLIST pIdList, UINT uFlags)
*
*    PURPOSE:  Gets the index for the current icon.  Index is index into system
*              image list.
*
*  PARAMETERS:
*    pIdList    - Fully qualified item id list for current item.
*    uFlags - Flags for SHGetFileInfo()
*
*  RETURN VALUE:
*    Icon index for current item.
****************************************************************************/
int CFileBrowseView::GetIcon(LPITEMIDLIST pIdList, UINT uFlags)
{
	ASSERT(pIdList);

	SHFILEINFO sfi;
	SHGetFileInfo((LPCSTR)pIdList, 0, &sfi, sizeof(SHFILEINFO), uFlags);
	return sfi.iIcon;
}


////////////////////////////////////////////////////////////////////////
// GetIconIndexes [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::GetIconIndexes
(
	LPITEMIDLIST pFullyQualifiedIdList,
	int& nImageIndex, 
	int& nOpenImageIndex
)
{
	ASSERT(pFullyQualifiedIdList);

	// Don't check the return value here. 
	// If IGetIcon() fails, you're in big trouble.
	nImageIndex = GetIcon(pFullyQualifiedIdList, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	nOpenImageIndex = GetIcon(pFullyQualifiedIdList, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON);
}


/****************************************************************************
*
*    FUNCTION: WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
*
*    PURPOSE:  Handles the WM_NOTIFY and WM_CONTEXTMENU messages.
*
****************************************************************************/
LRESULT CFileBrowseView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_NOTIFY:
		NotifyHandler(message, wParam, lParam);
		return 0;
	default:
		return C3DFormView::WindowProc(message, wParam, lParam);
	}
}


/****************************************************************************
*
*  FUNCTION: DoTheMenuThing(HWND hwnd, 
*                           LPSHELLFOLDER piParentShellFolder,
*                           LPITEMIDLIST  pIdList,
*                           LPPOINT lppt)
*
*  PURPOSE: Displays a popup context menu, given a parent shell folder,
*           relative item id and screen location.
*
*  PARAMETERS:
*    hwnd       - Parent window handle
*    piParentShellFolder - Pointer to parent shell folder.
*    pIdList        - Pointer to item id that is relative to piParentShellFolder
*    lppt       - Screen location of where to popup the menu.
*
*  RETURN VALUE:
*    Returns TRUE on success, FALSE on failure
*
****************************************************************************/
BOOL CFileBrowseView::DoTheMenuThing
(
	HWND hwnd, 
	LPSHELLFOLDER piParentShellFolder,
	LPITEMIDLIST pIdList,					//(i) non-NULL for single-selected tree item, NULL for list item(s)
	LPPOINT pPoint
)
{
	if (NULL == piParentShellFolder)
	{
		//
		// The only time the 'piParentShellFolder' should be NULL is if you try to
		// launch the context menu on the Desktop folder node.  There
		// is no such thing as a context menu for the Desktop folder
		// node.
		//
		ASSERT(NULL == pIdList);
		return TRUE;
	}

	LPCONTEXTMENU piContextMenu = NULL;
	int idCmd;
	BOOL bSuccess = TRUE;
	HRESULT hr = E_FAIL;
	if (pIdList)
	{
		//
		// Single-selected tree item.
		//
		hr = piParentShellFolder->GetUIObjectOf(hwnd,
												1,  // get attributes for this many objects
												(const struct _ITEMIDLIST **) &pIdList,
												IID_IContextMenu,
												0,
												(LPVOID *) &piContextMenu);
	}
	else
	{
		UINT uSelCount;
		LPITEMIDLIST* pPidlArray = CreateListSelectionPIDL(uSelCount);
		if (!pPidlArray)
			return FALSE;

		hr = piParentShellFolder->GetUIObjectOf(hwnd,
												uSelCount,  // get attributes for this many objects
												(const struct _ITEMIDLIST **) pPidlArray,
												IID_IContextMenu,
												0,
												(LPVOID *) &piContextMenu);

		delete [] pPidlArray;
	}

	if (SUCCEEDED(hr))  
	{
		ASSERT(piContextMenu != NULL);

		HMENU hMenu = CreatePopupMenu();

		if (hMenu)
		{
			// Get the context menu for the item.
			hr = piContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7fff, CMF_EXPLORE | CMF_NORMAL);
			if (SUCCEEDED(hr))
			{
				idCmd = TrackPopupMenu(hMenu, 
										TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
										pPoint->x, pPoint->y, 0, hwnd, NULL);

				if (idCmd)
				{
					// Execute the command that was selected.
					CMINVOKECOMMANDINFO cmi;
					cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
					cmi.fMask  = 0;
					cmi.hwnd   = hwnd;
					cmi.lpVerb = MAKEINTRESOURCE(idCmd-1);
					cmi.lpParameters = NULL;
					cmi.lpDirectory  = NULL;
					cmi.nShow        = SW_SHOWNORMAL;
					cmi.dwHotKey     = 0;
					cmi.hIcon        = NULL;
					
					// This is not so good.  Most operations are over pretty quickly, and
					// calling Progress winds up making the main window go to the background
					// when the operation causes a dialog to come up.
					//MainProgress(CRString(IDS_WORKING_ON_IT));

					hr = piContextMenu->InvokeCommand(&cmi);
					if (SUCCEEDED(hr))
					{
						//
						// Hmm.  Something could've happened to the
						// folder tree or to the file list as a result
						// of the context menu command.  That means we
						// need to refresh the display.  FORNOW, we
						// *always* refresh the display.  Really, we
						// should figure out a way to auto-refresh
						// only when something in the current display
						// actually changes.
						//
						OnCmdRefresh();		// can be expensive...
					}
					else
					{
						TRACE1("CFileBrowseView::DoTheMenuThing: InvokeCommand failed (hr = %lx)\n", hr);
					}
					
					//CloseProgress();
				}
			}
			else
				bSuccess = FALSE;

			DestroyMenu(hMenu);
		}
		else
			bSuccess = FALSE;

	   piContextMenu->Release();
	} 
	else
	{
		TRACE1("CFileBrowseView::DoTheMenuThing: GetUIObjectOf failed (hr = %lx)\n", hr);
		bSuccess = FALSE;
	}
	return bSuccess;
}


void CFileBrowseView::OnAppExit() 
{
	m_bOutaHere = TRUE;
	AfxGetMainWnd()->PostMessage(WM_COMMAND, SC_CLOSE);
}


////////////////////////////////////////////////////////////////////////
// OnContextMenu [protected]
//
// Called when the user releases the right button in the main window.
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
	//
	// See where the user clicked.  Of special interest are clicks
	// tree control or list control.
	//
	if (pWnd->GetSafeHwnd() == m_TreeCtrl.GetSafeHwnd())
	{
		//
		// Hit the tree control.
		//
		CPoint ptClient(ptScreen);
		m_TreeCtrl.ScreenToClient(&ptClient);
		TV_HITTESTINFO tvhti;
		tvhti.pt = ptClient;
		m_TreeCtrl.HitTest(&tvhti);
		if (tvhti.flags & TVHT_ONITEM)
		{
			if (tvhti.hItem != NULL)
				m_TreeCtrl.SelectItem(tvhti.hItem);
			LPTREEDATA pTreeData = LPTREEDATA(m_TreeCtrl.GetItemData(tvhti.hItem));
			ASSERT(pTreeData);
			if (pTreeData)
			{
				DoTheMenuThing(GetSafeHwnd(), pTreeData->piParentShellFolder, pTreeData->pIdList, &ptScreen);
				return;
			}
		}
	}
	else if (pWnd->GetSafeHwnd() == m_ListCtrl.GetSafeHwnd())
	{
		LPLISTDATA pListData = GetListDataAtPoint(ptScreen);
		if (pListData)
			DoTheMenuThing(GetSafeHwnd(), pListData->piParentShellFolder, NULL, &ptScreen);
		else
		{
			HMENU hMenu = QCLoadMenu(IDR_MAILBOX_FOLDER_VIEW);
			if (hMenu)
			{
				HMENU hPopupMenu = GetSubMenu(hMenu, 0);
				if (hPopupMenu)
				{
					// Remove the "Set as Home Folder" menu item for file list only instances
					if (m_bFileListOnly)
						DeleteMenu(hPopupMenu, ID_BROWSE_SET_HOME_FOLDER, MF_BYCOMMAND);

					TrackPopupMenu(hPopupMenu, 0, ptScreen.x, ptScreen.y, 0,
									AfxGetMainWnd()->GetSafeHwnd(), NULL);
				}
			}
		}

		return;
	}

	//
	// If we get this far, user didn't click on tree or list, so do
	// default context menu.
	//
	Default();
}

////////////////////////////////////////////////////////////////////////
// OpenItem [protected]
//
// Called when the user double-clicks or hits Enter on a ListCtrl item
////////////////////////////////////////////////////////////////////////
BOOL CFileBrowseView::OpenItem(LPLISTDATA pListData)
{
	BOOL bResult = FALSE;

	if (!pListData)
		ASSERT(0);
	else
	{
		if (m_bFileListOnly && pListData->ulAttribs & SFGAO_FOLDER)
		{
			LPSHELLFOLDER piChildShellFolder = NULL;
			HRESULT hr = pListData->piParentShellFolder->BindToObject(pListData->pIdList, 0, IID_IShellFolder, (LPVOID *) &piChildShellFolder);
			if (SUCCEEDED(hr))
			{
				LPITEMIDLIST pFQIdList = ConcatPidls(m_pCurrentIdList, pListData->pIdList);
				ResetListView(piChildShellFolder, pFQIdList);
				m_piMalloc->Free(pFQIdList);
				piChildShellFolder->Release();
				bResult = TRUE;
			}
		}
		else
		{
			TCHAR Filename[MAX_PATH + 1];
			if (GetName(m_piCurrentShellFolder, pListData->pIdList, SHGDN_FORPARSING, Filename))
				DoFileCommand(Filename, 0, false);
		}
	}

	return bResult;
}

////////////////////////////////////////////////////////////////////////
// GetListDataAtPoint [protected]
//
// Helper routine to get the list data at a particular point on the screen
////////////////////////////////////////////////////////////////////////
LPLISTDATA CFileBrowseView::GetListDataAtPoint(CPoint ptScreen)
{
	LV_HITTESTINFO lvhti;
	CPoint pt(ptScreen);

	m_ListCtrl.ScreenToClient(&pt);
	lvhti.pt = pt;
	int nIndex = m_ListCtrl.HitTest(&lvhti);
	if ((nIndex != -1) && (lvhti.flags & LVHT_ONITEM))
	{
		LV_ITEM lvi;

		m_ListCtrl.ClientToScreen(&pt);
		lvi.mask = LVIF_PARAM;
		lvi.iItem = lvhti.iItem;
		lvi.iSubItem = 0;

		if (m_ListCtrl.GetItem(&lvi))
			return (LPLISTDATA)lvi.lParam;
	}

	return NULL;
}

/****************************************************************************
*
*    FUNCTION: NotifyHandler(UINT message, WPARAM wParam, LPARAM lParam)
*
*    PURPOSE:  Handles the WM_NOTIFY and WM_CONTEXTMENU messages.
*
****************************************************************************/
LRESULT CFileBrowseView::NotifyHandler(UINT, WPARAM, LPARAM lParam) 
{
	POINT pt;
	NM_TREEVIEW *pnmtv   = (NM_TREEVIEW *)lParam;
	NM_LISTVIEW *pnmlv   = (NM_LISTVIEW *)lParam;
	LPTREEDATA   pTreeData = NULL;  //Long pointer to TreeView item data
	LPLISTDATA   pListData = NULL;  //Long pointer to ListView item data
	HRESULT        hr;
	LPSHELLFOLDER  piShellFolder2 = NULL;
	static char    szBuff[MAX_PATH];
	LV_DISPINFO    *plvdi;
	TV_SORTCB      tvscb;

	switch (pnmtv->hdr.idFrom)
	{
	case IDC_FILE_BROWSE_TREE:
		switch (pnmtv->hdr.code)   
		{
		case NM_RCLICK:
			{
				::GetCursorPos((LPPOINT)&pt);
				::ScreenToClient(m_TreeCtrl.m_hWnd, &pt);

				TV_HITTESTINFO tvhti;
				tvhti.pt = pt;
				tvhti.hItem = NULL;
				m_TreeCtrl.HitTest(&tvhti);
				if ((tvhti.flags & TVHT_ONITEM) && tvhti.hItem)
					m_TreeCtrl.SelectItem(tvhti.hItem);
			}
			break;

		case TVN_DELETEITEM:
			//Let's free the memory for the TreeView item data...
			pTreeData = LPTREEDATA(pnmtv->itemOld.lParam);
			ASSERT(pTreeData);
			if (pTreeData->piParentShellFolder)
				pTreeData->piParentShellFolder->Release();
			if (pTreeData->pIdList)
				m_piMalloc->Free(pTreeData->pIdList);  
			ASSERT(pTreeData->pFullyQualifiedIdList);
			m_piMalloc->Free(pTreeData->pFullyQualifiedIdList);
			m_piMalloc->Free(pTreeData);
			break;

		case TVN_SELCHANGED:
			//Do this only if we are not exiting the application...
			if (!m_bOutaHere)
			{
				pTreeData = LPTREEDATA(pnmtv->itemNew.lParam);
				if (pTreeData)
				{
					ASSERT(pTreeData->pFullyQualifiedIdList);
					if (pTreeData->piParentShellFolder)
					{
						//
						// Non-Desktop Folder.
						//
						ASSERT(pTreeData->pIdList);
						hr = pTreeData->piParentShellFolder->BindToObject(pTreeData->pIdList, 0, IID_IShellFolder, (LPVOID *) &piShellFolder2);
						if (SUCCEEDED(hr))
						{
							ResetListView(piShellFolder2, pTreeData->pFullyQualifiedIdList);
							piShellFolder2->Release();
						}
					}
					else
					{
						//
						// Desktop Folder.
						//
						ASSERT(NULL == pTreeData->pIdList);
						ResetListView(m_piDesktopFolder, m_pDesktopIdList);
					}
				}
			}
			break;
	
		case TVN_ITEMEXPANDING:
			if (pnmtv->itemNew.state & TVIS_EXPANDEDONCE)
				break;
			else if (m_TreeCtrl.GetChildItem(pnmtv->itemNew.hItem))
				break;

			pTreeData = LPTREEDATA(pnmtv->itemNew.lParam);
			if (pTreeData)
			{
				ASSERT(pTreeData->pFullyQualifiedIdList);
				if (pTreeData->piParentShellFolder)
				{
					//
					// Non-Desktop Folder.
					//
					ASSERT(pTreeData->pIdList);
					hr = pTreeData->piParentShellFolder->BindToObject(pTreeData->pIdList, 0, IID_IShellFolder, (LPVOID *) &piShellFolder2);
					if (SUCCEEDED(hr))
					{
						PopulateTreeFolder(piShellFolder2, pTreeData->pFullyQualifiedIdList, pnmtv->itemNew.hItem);
						piShellFolder2->Release();
					}
				}
				else
				{
					//
					// Desktop Folder.
					//
					ASSERT(NULL == pTreeData->pIdList);
					PopulateTreeFolder(m_piDesktopFolder, pTreeData->pFullyQualifiedIdList, pnmtv->itemNew.hItem);
				}

				tvscb.hParent     = pnmtv->itemNew.hItem;
				tvscb.lpfnCompare = TreeViewCompareProc;
				tvscb.lParam      = 0;

				::SendMessage(m_TreeCtrl.m_hWnd,TVM_SORTCHILDRENCB, (WPARAM)0, (LPARAM)&tvscb);
			}
			break;

		case TVN_KEYDOWN:
			{
				TV_KEYDOWN* pKeyDown = (TV_KEYDOWN *) lParam;
				if (VK_F5 == pKeyDown->wVKey)
					OnCmdRefresh();
			}
			break;

		default:
			break;
		}
		break;

	case IDC_FILE_BROWSE_LIST:
		switch (pnmlv->hdr.code)
		{
		case NM_DBLCLK:
			{
				::GetCursorPos((LPPOINT)&pt);
				LPLISTDATA pListData = GetListDataAtPoint(pt);
				if (pListData)
					OpenItem(pListData);
			}
			break;

		case LVN_DELETEITEM:
			{
				//Let's free the memory for the ListView item data...
				LV_ITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = pnmlv->iItem;
				lvi.iSubItem = 0;

				if (! m_ListCtrl.GetItem(&lvi))
					return 0;
				pListData = (LPLISTDATA)lvi.lParam;

				ASSERT(pListData && pListData->piParentShellFolder);
				pListData->piParentShellFolder->Release();
				ASSERT(pListData->pIdList);  
				m_piMalloc->Free(pListData->pIdList);  
				m_piMalloc->Free(pListData);  
			}
			break;

		case LVN_GETDISPINFO:
			{
				plvdi = (LV_DISPINFO *) lParam;
				pListData = (LPLISTDATA) plvdi->item.lParam;

				if (NULL == plvdi)
				{
					ASSERT(0);
					break;
				}
				else if (NULL == pListData)
				{
					ASSERT(0);
					break;
				}

				//
				// Form temporary fully-qualified PIDL by combining
				// parent folder info from the selected tree item and
				// relative item info from the list control item.
				//
				LPITEMIDLIST pTempIdList = GetFullyQualifiedPidlFromListItem(pListData);
				if (NULL == pTempIdList)
					break;
				
				if (plvdi->item.mask & LVIF_IMAGE)
				{
					plvdi->item.iImage = GetIcon(pTempIdList, SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

					if (pListData->ulAttribs & SFGAO_LINK)
					{
						plvdi->item.mask |= LVIF_STATE;
						plvdi->item.stateMask = LVIS_OVERLAYMASK;
						plvdi->item.state = INDEXTOOVERLAYMASK(2);
					}
				}

				if (plvdi->item.mask & LVIF_TEXT)
				{
					switch (plvdi->item.iSubItem)
					{
					case 0:
						// Display Name
						GetName(pListData->piParentShellFolder, pListData->pIdList, SHGDN_NORMAL, plvdi->item.pszText);
						break;
					case 1:
						// Type
						if (pListData->ulAttribs & SFGAO_FILESYSTEM)
						{
							SHFILEINFO info;
							::SHGetFileInfo(LPCSTR(pTempIdList), 0, &info, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_TYPENAME);
							::lstrcpy(plvdi->item.pszText, info.szTypeName);
						}
						break;
					case 2:
						// Size
						if ((pListData->ulAttribs & ~(SFGAO_FOLDER | SFGAO_FILESYSTEM)) != 0)
						{
							// Get full filesystem pathname
							char szBuff[MAX_PATH];
							if (! GetName(pListData->piParentShellFolder, pListData->pIdList, SHGDN_FORPARSING, szBuff))
								break;

							CFileStatus fileStatus;
							CFile::GetStatus(szBuff, fileStatus);
							::wsprintf(plvdi->item.pszText, "%I64uK", (fileStatus.m_size + 1023) / 1024);
						}
						break;
					case 3:
						// Modified
						if ((pListData->ulAttribs & ~(SFGAO_FOLDER | SFGAO_FILESYSTEM)) != 0)
						{
							// Get full filesystem pathname
							char szBuff[MAX_PATH];
							if (! GetName(pListData->piParentShellFolder, pListData->pIdList, SHGDN_FORPARSING, szBuff))
								break;

							CFileStatus fileStatus;
							CFile::GetStatus(szBuff, fileStatus);
							::TimeDateStringMT(plvdi->item.pszText, static_cast<time_t>(fileStatus.m_mtime.GetTime()), TRUE);
						}

						//
						// FORNOW, experimental placement... does the
						// control always ask for text in column order?
						//
						plvdi->item.mask |= LVIF_DI_SETITEM;
						break;
					default:
						ASSERT(0);
						break;
					}
				}

				if (pTempIdList)
					m_piMalloc->Free(pTempIdList);
			}
			break;

		case LVN_BEGINDRAG:
		case LVN_BEGINRDRAG:
			{
				//
				// Handle drag of a multiple selection.  Need to create
				// an array of pointers to PIDLS.
				//
				UINT uSelCount;
				LPITEMIDLIST* pPidlArray = CreateListSelectionPIDL(uSelCount);
				if (!pPidlArray)
					return 0;

				LPDATAOBJECT piDataObject = NULL;
				hr = m_piCurrentShellFolder->GetUIObjectOf(m_hWnd,
															uSelCount,  //Number of objects to get attributes of
															(const struct _ITEMIDLIST **) pPidlArray,
															IID_IDataObject,
															0,
															(LPVOID *) &piDataObject);

				if (SUCCEEDED(hr))
				{
					COleDropSource ods;
					if (ods.OnBeginDrag(this))
					{
						DWORD dwOKEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
						DWORD dwEffect = 0;

						if (::DoDragDrop(piDataObject, (LPDROPSOURCE)ods.GetInterface(&IID_IDropSource),
											dwOKEffect, &dwEffect) == DRAGDROP_S_DROP)
						{
							// Can't compare against MOVE because sometimes dwEffect gets set to zero
							if (dwEffect != DROPEFFECT_COPY && dwEffect != DROPEFFECT_LINK)
								OnCmdRefresh();
						}
					}

					piDataObject->Release();
				}

				delete [] pPidlArray;
			}
			break;

		case LVN_COLUMNCLICK:
			{
				NM_LISTVIEW* pNotifyListView = (NM_LISTVIEW *) lParam;
				ASSERT(-1 == pNotifyListView->iItem);

				LPITEMIDLIST pFQIDList;
				if (m_bFileListOnly)
					pFQIDList = m_pCurrentIdList;
				else
				{
					HTREEITEM hItem = m_TreeCtrl.GetSelectedItem();
					if (NULL == hItem)
					{
						//
						// Can't form fully qualified PIDLs without parent 
						// folder info from tree control.
						//
						::MessageBeep(MB_OK);
						break;
					}

					pTreeData = (LPTREEDATA) m_TreeCtrl.GetItemData(hItem);
					if (NULL == pTreeData)
					{
						ASSERT(0);
						break;
					}

					pFQIDList = pTreeData->pFullyQualifiedIdList;
				}

				ASSERT(pFQIDList);
				m_nListCtrlSortColumn = pNotifyListView->iSubItem;	// make it persistent
				m_bListCtrlReverseSort = ::ShiftDown();				// make it persistent
				SortListView(pFQIDList);
			}
			break;

		case LVN_KEYDOWN:
			{
				LV_KEYDOWN* pKeyDown = (LV_KEYDOWN *) lParam;
				switch (pKeyDown->wVKey)
				{
				case VK_F5:
					OnCmdRefresh();
					break;
				case VK_BACK:
					if (ShiftDown())
						OnNavForward();
					else
						OnNavBack();
					break;
				case VK_DELETE:
					OnDelete();
					break;
				case VK_RETURN:
					if (GetKeyState(VK_MENU) < 0)
						OnProperties();
					break;
				}
			}
			break;

		default:
			break;
		}
		break;
	}

	return 0;
}


/****************************************************************************
*
*    FUNCTION: ResetListView(LPTREEDATA, LPSHELLFOLDER )
*
*    PURPOSE:  Resets the list view.
*
****************************************************************************/
BOOL CFileBrowseView::ResetListView
(
	LPSHELLFOLDER piShellFolder,
	LPITEMIDLIST pFullyQualifiedIdList
)
{
	m_ListCtrl.SetRedraw(FALSE);
	m_ListCtrl.DeleteAllItems();

	ASSERT(piShellFolder);
	ASSERT(pFullyQualifiedIdList);

	if (!InitListViewImageLists() || !PopulateListView(piShellFolder))
	{
		m_ListCtrl.SetRedraw(TRUE);
		return FALSE;
	}

	if (m_piCurrentShellFolder != piShellFolder && m_pCurrentIdList != pFullyQualifiedIdList)
	{
		// We're moving to a different folder, so add to the navigation list.
		// We need to truncate the list after the current position.
		TruncateNavList(m_CurrentNavListItem + 1);

		NavItem* pNI = DEBUG_NEW NavItem;
		pNI->pIDL = ConcatPidls(NULL, pFullyQualifiedIdList);
		pNI->piShellFolder = piShellFolder;
		pNI->piShellFolder->AddRef();
		m_NavList.AddTail(pNI);
		m_CurrentNavListItem = m_NavList.GetCount() - 1;
	}

	if (m_piCurrentShellFolder != piShellFolder)
	{
		if (m_piCurrentShellFolder)
			m_piCurrentShellFolder->Release();
		m_piCurrentShellFolder = piShellFolder;
		m_piCurrentShellFolder->AddRef();
	}

	if (m_pCurrentIdList != pFullyQualifiedIdList)
	{
		if (m_pCurrentIdList)
			m_piMalloc->Free(m_pCurrentIdList);
		m_pCurrentIdList = ConcatPidls(NULL, pFullyQualifiedIdList);
	}

	if (m_ListCtrl.GetItemCount() > 1)
		SortListView(pFullyQualifiedIdList);

	m_ListCtrl.SetRedraw(TRUE);

	//
	// Set the "focus" to the first item in the list, if any.
	// 
	if (m_ListCtrl.GetItemCount() > 0)
		m_ListCtrl.SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);

	return TRUE;
}

/****************************************************************************
*
*    FUNCTION: InitListViewImageLists()
*
*    PURPOSE:  Associate the system image list with our listview.
*
****************************************************************************/
BOOL CFileBrowseView::InitListViewImageLists()
{
	SHFILEINFO sfi;
	BOOL       bSuccess=TRUE;

	//TRACE0("CFileBrowseView::InitListViewImageLists\n");
	HIMAGELIST himlSmall = (HIMAGELIST)SHGetFileInfo((LPCSTR)"C:\\", 
													   0,
													   &sfi, 
													   sizeof(SHFILEINFO), 
													   SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	HIMAGELIST himlLarge = (HIMAGELIST)SHGetFileInfo((LPCSTR)"C:\\", 
													   0,
													   &sfi, 
													   sizeof(SHFILEINFO), 
													   SHGFI_SYSICONINDEX | SHGFI_LARGEICON);

	if (himlSmall && himlLarge)
	{
		::SendMessage(m_ListCtrl.m_hWnd, LVM_SETIMAGELIST, (WPARAM)LVSIL_SMALL,
			(LPARAM)himlSmall);
		::SendMessage(m_ListCtrl.m_hWnd, LVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL,
			(LPARAM)himlLarge);
	}
	else
	   bSuccess = FALSE;

	return bSuccess;
}


////////////////////////////////////////////////////////////////////////
// PopulateListView [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CFileBrowseView::PopulateListView
(
	LPSHELLFOLDER piShellFolder 
)
{
	LV_ITEM      lvi;
	LPITEMIDLIST pIdList = NULL;
	LPENUMIDLIST piEnumList = NULL;
	ULONG        ulFetched, ulAttrs;
	HWND         hwnd=::GetParent(m_ListCtrl.m_hWnd);

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	//TRACE0("CFileBrowseView::InitListViewItems\n");

	BOOL bStatus = TRUE;
	CCursor busyCursor;
	ASSERT(piShellFolder);
	DWORD dwFlags = SHCONTF_NONFOLDERS | (m_bFileListOnly? SHCONTF_FOLDERS : 0);
	HRESULT hr = piShellFolder->EnumObjects(hwnd, dwFlags, &piEnumList);
	if (SUCCEEDED(hr))
	{
		ASSERT(piEnumList);

		int iCtr = 0;
		while (S_OK == piEnumList->Next(1, &pIdList, &ulFetched))
		{
			// Get some memory for the ITEMDATA structure.
			LPLISTDATA pListData = (LPLISTDATA) m_piMalloc->Alloc(sizeof(LISTDATA));
			if (!pListData)
			{
				bStatus = FALSE;
				goto Done;
			}

			// Since you are interested in the display attributes 
			// as well as other attributes, you need to set ulAttrs to 
			// SFGAO_DISPLAYATTRMASK before calling GetAttributesOf().
			ulAttrs = SFGAO_DISPLAYATTRMASK | SFGAO_FILESYSTEM | SFGAO_FOLDER;
			piShellFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **)&pIdList, &ulAttrs);
			pListData->ulAttribs = ulAttrs;

			lvi.iItem       = iCtr++;
			lvi.iSubItem    = 0;
			lvi.pszText     = LPSTR_TEXTCALLBACK;
			lvi.cchTextMax  = -1;		// unused
			lvi.iImage      = I_IMAGECALLBACK;
			lvi.state = 0;				// unused
			lvi.stateMask = 0;			// unused

			pListData->piParentShellFolder = piShellFolder;
			piShellFolder->AddRef();

			// Now make a copy of the ITEMIDLIST.
			pListData->pIdList = CopySinglePidl(pIdList);

			lvi.lParam = LPARAM(pListData);

			// Add the item to the list view control.   
			if (m_ListCtrl.InsertItem(&lvi) == -1)
			{
				m_piMalloc->Free(pListData);
				bStatus = FALSE;
				goto Done;
			}

			m_piMalloc->Free(pIdList);  // free PIDL the shell gave you
			pIdList = NULL;
		}
	}
	else
		bStatus = FALSE;

Done:

	if (piEnumList)  
		piEnumList->Release();

	// The following two if statements will be TRUE only if you got here
	// on an error condition from the goto statement.  Otherwise, free 
	// this memory at the end of the while loop above.
	if (pIdList)           
		m_piMalloc->Free(pIdList);

	return bStatus;
}


////////////////////////////////////////////////////////////////////////
// SortListView [protected]
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::SortListView(LPITEMIDLIST pFullyQualifiedIdList)
{
	ASSERT(pFullyQualifiedIdList);

	//
	// Sort the items based on the current column selection.
	//
	LVSORTINFO lvSortInfo;
	lvSortInfo.piMalloc = m_piMalloc;
	m_piMalloc->AddRef();
	lvSortInfo.pFullyQualifiedIdList = pFullyQualifiedIdList;
	lvSortInfo.nColumn = m_nListCtrlSortColumn;
	lvSortInfo.bReverse = m_bListCtrlReverseSort;

	CCursor waitCursor;
	m_ListCtrl.SortItems(ListViewCompareProc, DWORD(&lvSortInfo));
	m_piMalloc->Release();
}


/****************************************************************************
*
*    FUNCTION: ListViewCompareProc(LPARAM, LPARAM, LPARAM)
*
*    PURPOSE:  Callback routine for sorting list 
*
****************************************************************************/
int CALLBACK CFileBrowseView::ListViewCompareProc(LPARAM lParam1, 
								 LPARAM lParam2,
								 LPARAM lParamSort)
{
	LPLISTDATA pListData1 = (LPLISTDATA) lParam1;
	LPLISTDATA pListData2 = (LPLISTDATA) lParam2;
	LPLVSORTINFO lpSortInfo = (LPLVSORTINFO) lParamSort;

	ASSERT(lpSortInfo);

	switch (lpSortInfo->nColumn)
	{
	case 0:
		{
			// Sort by Name column
			HRESULT hr = pListData1->piParentShellFolder->CompareIDs(0, pListData1->pIdList, pListData2->pIdList);
			if (FAILED(hr))
			   return 0;

			if (lpSortInfo->bReverse)
				return -(short(HRESULT_CODE(hr)));
			else
				return short(HRESULT_CODE(hr));
		}
		break;
	case 1:

		// Sort by Type column
		if ((pListData1->ulAttribs & SFGAO_FILESYSTEM) &&
			(pListData2->ulAttribs & SFGAO_FILESYSTEM))
		{
			ASSERT(lpSortInfo->piMalloc);

			LPITEMIDLIST pFullyQualifiedIdList = lpSortInfo->pFullyQualifiedIdList;
			ASSERT(pFullyQualifiedIdList);

			//
			// Form temporary fully-qualified PIDLs by combining
			// parent folder info from the selected tree item and
			// relative item info from the list control items.
			//
			// FORNOW, this works, but it is very slow due to the
			// memory alloc/free overhead.
			//
			LPITEMIDLIST pTempIdList1 = ConcatPidls(pFullyQualifiedIdList, pListData1->pIdList);
			if (NULL == pTempIdList1)
				return 0;
			LPITEMIDLIST pTempIdList2 = ConcatPidls(pFullyQualifiedIdList, pListData2->pIdList);
			if (NULL == pTempIdList2)
			{
				lpSortInfo->piMalloc->Free(pTempIdList1);
				return 0;
			}
			
			SHFILEINFO info1;
			::SHGetFileInfo(LPCSTR(pTempIdList1), 0, &info1, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_TYPENAME);
			SHFILEINFO info2;
			::SHGetFileInfo(LPCSTR(pTempIdList2), 0, &info2, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_TYPENAME);
			int nResult = ::lstrcmpi(info1.szTypeName, info2.szTypeName);

			lpSortInfo->piMalloc->Free(pTempIdList1);
			lpSortInfo->piMalloc->Free(pTempIdList2);

			if (lpSortInfo->bReverse)
				return -(short(nResult));
			else
				return short(nResult);

		}
		return 0;
	case 2:
		// Size
		if ((pListData1->ulAttribs & ~(SFGAO_FOLDER | SFGAO_FILESYSTEM)) != 0)
		{
			// Get full filesystem pathname
			char szBuff1[MAX_PATH];
			char szBuff2[MAX_PATH];
			if ((! GetName(pListData1->piParentShellFolder, pListData1->pIdList, SHGDN_FORPARSING, szBuff1)) ||
				(! GetName(pListData2->piParentShellFolder, pListData2->pIdList, SHGDN_FORPARSING, szBuff2)))
				break;

			CFileStatus fileStatus1;
			CFile::GetStatus(szBuff1, fileStatus1);
			CFileStatus fileStatus2;
			CFile::GetStatus(szBuff2, fileStatus2);
			ULONGLONG wideResult = fileStatus1.m_size - fileStatus2.m_size;

			int		nResult = 0;
			if (wideResult > 0)
				nResult = 1;
			else if (wideResult < 0)
				nResult = -1;

			if (lpSortInfo->bReverse)
				return -nResult;
			else
				return nResult;
		}
		return 0;
	case 3:
		// Modified
		if ((pListData1->ulAttribs & ~(SFGAO_FOLDER | SFGAO_FILESYSTEM)) != 0)
		{
			// Get full filesystem pathname
			char szBuff1[MAX_PATH];
			char szBuff2[MAX_PATH];
			if ((! GetName(pListData1->piParentShellFolder, pListData1->pIdList, SHGDN_FORPARSING, szBuff1)) ||
				(! GetName(pListData2->piParentShellFolder, pListData2->pIdList, SHGDN_FORPARSING, szBuff2)))
				break;

			CFileStatus fileStatus1;
			CFile::GetStatus(szBuff1, fileStatus1);
			CFileStatus fileStatus2;
			CFile::GetStatus(szBuff2, fileStatus2);
			int nResult = static_cast<time_t>(fileStatus1.m_mtime.GetTime()) - static_cast<time_t>(fileStatus2.m_mtime.GetTime());
			if (lpSortInfo->bReverse)
				return -nResult;
			else
				return nResult;
		}
		return 0;
	default:
		ASSERT(0);			// unexpected column
		return 0;
	}

	ASSERT(0);				// should never get here...
	return 0;
}


////////////////////////////////////////////////////////////////////////
// GetFullyQualifiedListPidl [protected]
//
// Given a list item, form a fully-qualified PIDL to the list item by
// using the fully-qualified PIDL to the parent folder stored in the 
// currently selected *tree* item.  If successful, it is the caller's 
// responsibility to deallocate the returned PIDL.
////////////////////////////////////////////////////////////////////////
LPITEMIDLIST CFileBrowseView::GetFullyQualifiedPidlFromListItem
(
	LPLISTDATA pListData
)
{
	ASSERT(pListData);

	if (m_bFileListOnly)
		return ConcatPidls(m_pCurrentIdList, pListData->pIdList);

	HTREEITEM hItem = m_TreeCtrl.GetSelectedItem();
	if (NULL == hItem)
	{
		//
		// Can't form fully qualified PIDL without parent 
		// folder info from tree control.
		//
		return NULL;
	}

	LPTREEDATA pTreeData = (LPTREEDATA) m_TreeCtrl.GetItemData(hItem);
	if (NULL == pTreeData)
	{
		ASSERT(0);
		return NULL;
	}

	ASSERT(pTreeData->pFullyQualifiedIdList);
	ASSERT(pListData->pIdList);

	//
	// Form temporary fully-qualified PIDL by combining parent folder
	// info from the selected tree item and relative item info from
	// the list control item.
	//
	return ConcatPidls(pTreeData->pFullyQualifiedIdList, pListData->pIdList);
}


/****************************************************************************
*
*    FUNCTION: TreeViewCompareProc(LPARAM, LPARAM, LPARAM)
*
*    PURPOSE:  Callback routine for sorting the tree 
*
****************************************************************************/
int CALLBACK CFileBrowseView::TreeViewCompareProc(LPARAM lparam1, 
	LPARAM lparam2, LPARAM)
{
	LPTREEDATA pTreeData1=(LPTREEDATA)lparam1;
	LPTREEDATA pTreeData2=(LPTREEDATA)lparam2;
	HRESULT   hr;

	ASSERT(pTreeData1->piParentShellFolder && pTreeData2->piParentShellFolder);
	ASSERT(pTreeData1->pIdList);
	ASSERT(pTreeData2->pIdList);

	hr = pTreeData1->piParentShellFolder->CompareIDs(0,pTreeData1->pIdList,pTreeData2->pIdList);
	if (FAILED(hr))
	   return 0;

	return short(HRESULT_CODE(hr));
}

// 
// FUNCTIONS THAT DEAL WITH PIDLs
//
/****************************************************************************
*
*    FUNCTION: Next [public, static]
*
*    PURPOSE:  Gets the next PIDL in the list 
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::Next(LPCITEMIDLIST pIdList)
{
   LPSTR lpMem=(LPSTR)pIdList;

   lpMem+=pIdList->mkid.cb;

   return (LPITEMIDLIST)lpMem;
}


/****************************************************************************
*
*    FUNCTION: GetSize [public, static]
*
*    PURPOSE:  Gets the size of the PIDL 
*
****************************************************************************/
UINT CFileBrowseView::GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl)
	{
		cbTotal += sizeof(pidl->mkid.cb);       // Null terminator
		while (pidl->mkid.cb)
		{
			cbTotal += pidl->mkid.cb;
			pidl = Next(pidl);
		}
	}

	return cbTotal;
}


/****************************************************************************
*
*    FUNCTION: Create [public, static]
*
*    PURPOSE:  Allocates a PIDL 
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::AllocPidl(UINT cbSize)
{
	LPMALLOC piMalloc = NULL;
	HRESULT hr = ::SHGetMalloc(&piMalloc);
	if (FAILED(hr))
		return NULL;

	LPITEMIDLIST pIdList = LPITEMIDLIST(piMalloc->Alloc(cbSize));
	if (pIdList)
		memset(pIdList, 0, cbSize);      // zero-init for external task alloc

	piMalloc->Release();

	return pIdList;
}


/****************************************************************************
*
*    FUNCTION: ConcatPidls [public, static]
*
*    PURPOSE:  Concatenates two PIDLs 
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::ConcatPidls
(
	LPCITEMIDLIST pIdList1, 
	LPCITEMIDLIST pIdList2
)
{
	UINT cb1;

	if (pIdList1)  //May be NULL
	   cb1 = GetSize(pIdList1) - sizeof(pIdList1->mkid.cb);
	else
	   cb1 = 0;

	UINT cb2 = GetSize(pIdList2);

	LPITEMIDLIST pNewIdList = AllocPidl(cb1 + cb2);
	if (pNewIdList)
	{
		if (pIdList1)
			memcpy(pNewIdList, pIdList1, cb1);
		memcpy(((LPSTR)pNewIdList) + cb1, pIdList2, cb2);
	}
	return pNewIdList;
}


/****************************************************************************
*
*    FUNCTION: CopySinglePidl [public, static]
*
*    PURPOSE:  Copies the ITEMID 
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::CopySinglePidl
(
	LPITEMIDLIST pIdList
)
{
	ASSERT(pIdList);

	UINT uSize = pIdList->mkid.cb + sizeof(pIdList->mkid.cb);

	LPITEMIDLIST pNewIdList = AllocPidl(uSize);
	if (pNewIdList)
		CopyMemory((PVOID)pNewIdList, (CONST VOID *)pIdList, uSize);

	return pNewIdList;
}


/****************************************************************************
*
*    FUNCTION: CopyFullPidlExceptForLast [public, static]
*
*    PURPOSE:  Copies the PIDL, truncating the last entry.
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::CopyFullPidlExceptForLast
(
	LPITEMIDLIST pIdList
)
{
	ASSERT(pIdList);

	//
	// Step 1.  Clone the existing list.
	//
	LPITEMIDLIST pNewIdList = ConcatPidls(NULL, pIdList);

	//
	// Step 2.  Truncate the existing list at the last entry.
	// Yeah, this wastes some memory, but hey, memory's cheap.  :-)
	//
	if (pNewIdList)
	{
		LPITEMIDLIST pCurrentPidl = pNewIdList;
		LPITEMIDLIST pLastPidl = pNewIdList;
		while (pCurrentPidl->mkid.cb)
		{
			pLastPidl = pCurrentPidl;
			pCurrentPidl = Next(pCurrentPidl);
		}

		if (pLastPidl->mkid.cb)
			pLastPidl->mkid.cb = 0;		// do the truncation
	}

	return pNewIdList;
}


/****************************************************************************
*
*    FUNCTION: GetLastPidl [public, static]
*
*    PURPOSE:  Return pointer to last PIDL.
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::GetLastPidl
(
	LPITEMIDLIST pIdList
)
{
	ASSERT(pIdList);

	LPITEMIDLIST pCurrentPidl = pIdList;
	LPITEMIDLIST pLastPidl = pIdList;
	while (pCurrentPidl->mkid.cb)
	{
		pLastPidl = pCurrentPidl;
		pCurrentPidl = Next(pCurrentPidl);
	}

	return pLastPidl;
}


/****************************************************************************
*
*    FUNCTION: GetName [public, static]
*
*    PURPOSE:  Gets the friendly name for the folder 
*
****************************************************************************/
BOOL CFileBrowseView::GetName
(
	LPSHELLFOLDER piShellFolder, 
	LPITEMIDLIST pIdList, 
	DWORD dwFlags, 
	LPSTR pszFriendlyName
)
{
	ASSERT(piShellFolder);
	ASSERT(pIdList);
	ASSERT(pszFriendlyName);

	BOOL   bSuccess=TRUE;
	STRRET str;

	if (!piShellFolder || !pIdList || !pszFriendlyName)
		return FALSE;

	if (NOERROR == piShellFolder->GetDisplayNameOf(pIdList, dwFlags, &str))
	{
		switch (str.uType)
		{
		case STRRET_WSTR:
			WideCharToMultiByte(CP_ACP,             // code page
								0,                  // dwFlags
								str.pOleStr,        // lpWideCharStr
								-1,                 // cchWideCharStr
								pszFriendlyName,    // lpMultiByteStr
								MAX_PATH,           // cchMultiByte
								NULL,               // lpDefaultChar
								NULL);              // lpUsedDefaultChar
			break;

		case STRRET_OFFSET:
			lstrcpy(pszFriendlyName, (LPSTR)pIdList+str.uOffset);
			break;

		case STRRET_CSTR:             
			lstrcpy(pszFriendlyName, (LPSTR)str.cStr);
			break;

		default:
			ASSERT(0);
			bSuccess = FALSE;
			break;
		}
	}
	else
		bSuccess = FALSE;

	return bSuccess;
}

/****************************************************************************
*
*    FUNCTION: GetFullyQualPidlFromPath [protected]
*
*    PURPOSE:  Gets the Fully qualified Pidls for the given full pathname 
*
****************************************************************************/
LPITEMIDLIST CFileBrowseView::GetFullyQualPidlFromPath
(
	LPCTSTR lpszFullPathName
)
{
	ASSERT(lpszFullPathName);

	if (!lpszFullPathName)
		return NULL;

	LPITEMIDLIST pFullyQualifiedIdList = NULL;
	OLECHAR szOleChar[MAX_PATH];
	MultiByteToWideChar(CP_ACP,
						MB_PRECOMPOSED,
						lpszFullPathName,
						-1,
						(USHORT *)szOleChar,
						sizeof(szOleChar));

	//
	// Use Desktop folder since we want a path relative to the root.
	//
	ULONG ulEaten = 0;
	ULONG ulAttribs = 0;
	HRESULT hr = m_piDesktopFolder->ParseDisplayName(NULL,
													NULL,
													szOleChar,
													&ulEaten,
													&pFullyQualifiedIdList,
													&ulAttribs);
	if (FAILED(hr))
		return NULL;

	ASSERT(pFullyQualifiedIdList);

	return pFullyQualifiedIdList;
}

////////////////////////////////////////////////////////////////////////
// CreateListSelectionPIDL [protected]
//
// Create a list of PIDLs out of the items currently selected in the list control
////////////////////////////////////////////////////////////////////////
LPITEMIDLIST* CFileBrowseView::CreateListSelectionPIDL(UINT& Count)
{
	Count = m_ListCtrl.GetSelectedCount();
	if (!Count)
		return NULL;

	LPITEMIDLIST* pPidlArray = (LPITEMIDLIST *) DEBUG_NEW_NOTHROW int[Count];
	if (!pPidlArray)
	{
		Count = 0;
		return NULL;
	}

	int nIndex = -1;
	for (UINT i = 0; i < Count; i++)
	{
		nIndex = m_ListCtrl.GetNextItem(nIndex, LVIS_SELECTED);
		ASSERT(nIndex != -1);

		LPLISTDATA pListData = LPLISTDATA(m_ListCtrl.GetItemData(nIndex));
		ASSERT(pListData);
		pPidlArray[i] = pListData->pIdList;
		ASSERT(pPidlArray[i]);
	}

	return pPidlArray;
}

void CFileBrowseView::OnDestroy() 
{
	//
	// Save column widths to INI file.
	//
	ASSERT(::IsWindow(m_ListCtrl.GetSafeHwnd()));

	LV_COLUMN col;
	col.mask = LVCF_WIDTH;
	for (int i = 0; m_ListCtrl.GetColumn(i, &col); i++)
	{
		ASSERT(col.cx > 0 && col.cx < 30000);
		switch (i)
		{
		case 0:
			// Name column
			SetIniShort(IDS_INI_FILE_BROWSE_NAME_WIDTH, short(col.cx));
			break;
		case 1:
			// Type column
			SetIniShort(IDS_INI_FILE_BROWSE_TYPE_WIDTH, short(col.cx));
			break;
		case 2:
			// column
			SetIniShort(IDS_INI_FILE_BROWSE_SIZE_WIDTH, short(col.cx));
			break;
		case 3:
			// Modified column
			SetIniShort(IDS_INI_FILE_BROWSE_MODIFIED_WIDTH, short(col.cx));
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	C3DFormView::OnDestroy();

	m_bOutaHere = TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetResizeBarRect [private]
//
// Return the client rect of the "resize bar".
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::GetResizeBarRect(CRect& rectClient) const
{
	//
	// Determine whether or not the mouse is over the resize bar.
	// If we're displaying the list only, then return an empty rect.
	// 
	if (m_bFileListOnly)
		rectClient.SetRect(-1, -1, -1, -1);
	else
	{
		CRect rectTree;
		m_TreeCtrl.GetWindowRect(rectTree);
		ScreenToClient(rectTree);

		rectClient.SetRect(rectTree.left, rectTree.bottom, rectTree.right, rectTree.bottom + TRACKER_HEIGHT);
	}
}


////////////////////////////////////////////////////////////////////////
// IsOnResizeBar [private]
//
// Do hit test to see if we're over the "resize bar" area of the
// file browser window.
////////////////////////////////////////////////////////////////////////
BOOL CFileBrowseView::IsOnResizeBar(CPoint ptClient) const
{
	//
	// Determine whether or not the mouse is over the
	// resize bar.
	// 
	CRect rectResize;
	GetResizeBarRect(rectResize);

	return rectResize.PtInRect(ptClient);
}


////////////////////////////////////////////////////////////////////////
// DrawTrackerGhost [private]
//
// Draw the ghosted docked resize tracker image.  Stolen from MFC's
// CSplitterWnd::OnInvertTracker() method.
//
////////////////////////////////////////////////////////////////////////
void CFileBrowseView::DrawTrackerGhost(const CRect& rectClient)
{
	ASSERT_VALID(this);
	ASSERT(!rectClient.IsRectEmpty());

	ASSERT((GetStyle() & WS_CLIPCHILDREN) == 0);

	// pat-blt without clip children on
	CDC* pDC = GetDC();
	// invert the brush pattern (looks just like frame window sizing)
	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = HBRUSH(::SelectObject(pDC->m_hDC, pBrush->m_hObject));
	pDC->PatBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);
	ReleaseDC(pDC);
}


//--------------------------------------------------------------------//
//--------------------------------------------------------------------//
//                                                                    //
//                  C D r i v e C o m b o B o x                       //
//                                                                    //
//--------------------------------------------------------------------//
//--------------------------------------------------------------------//


/////////////////////////////////////////////////////////////////////////////
// CDriveComboBox

BEGIN_MESSAGE_MAP(CDriveComboBox, CComboBox)
	//{{AFX_MSG_MAP(CDriveComboBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// For shell icons
// Can't use CImageList because it expects only one object to attach to an imagelist
HIMAGELIST	CDriveComboBox::s_hImageList = NULL;

CDriveComboBox::CDriveComboBox()
{
	SHFILEINFO sfi;			// unused

	if (!s_hImageList)
	{
		s_hImageList = HIMAGELIST(SHGetFileInfo((LPCSTR) "C:\\", 
												0,
												&sfi, 
												sizeof(SHFILEINFO), 
												SHGFI_SYSICONINDEX | SHGFI_SMALLICON));
		ASSERT(s_hImageList);
	}
}

CDriveComboBox::~CDriveComboBox()
{
}


/////////////////////////////////////////////////////////////////////////////
// CDriveComboBox message handlers

void CDriveComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (NULL == pDC || NULL == pDC->m_hDC)
		return;
	CRect rect(lpDrawItemStruct->rcItem);
	
	LPCOMBODATA pComboData = NULL;
	if (int(lpDrawItemStruct->itemID) >= 0)
		pComboData = LPCOMBODATA(lpDrawItemStruct->itemData);

	//
	// Set up some color values
	//
	COLORREF ForeColor;
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		ForeColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	else
		ForeColor = ::GetSysColor(COLOR_WINDOWTEXT);

	if (pComboData)
	{  
		// Draw background
		CBrush BackBrush(::GetSysColor(COLOR_WINDOW));
		pDC->FillRect(&rect, &BackBrush);
	
		// Draw icon
		int nIconLeft = rect.left + 1;
		if ((lpDrawItemStruct->itemState & ODS_COMBOBOXEDIT) == 0)
		 	nIconLeft += (10 * pComboData->nIndentLevel);

		{
			// Acquire handle to icon for this item.
			HICON hIcon = ImageList_ExtractIcon(NULL, s_hImageList, pComboData->nIconIndex);
			if (hIcon)
				::DrawIconEx(pDC->GetSafeHdc(), nIconLeft, rect.top, hIcon, 
								16, 16, 0, 0, DI_NORMAL);
		}

		// Draw label
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(ForeColor);
		CString strLabel;
		GetLBText(lpDrawItemStruct->itemID, strLabel);
		CSize sizeText(pDC->GetTextExtent(strLabel));

		CRect rectText;
		rectText.left = nIconLeft + 20;
		rectText.top = rect.top;
		rectText.right = rectText.left + sizeText.cx + 5;
		rectText.bottom = rect.bottom;
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			CBrush TextBrush(::GetSysColor(COLOR_HIGHLIGHT));
			pDC->FillRect(&rectText, &TextBrush);
		}
		pDC->ExtTextOut(rectText.left + 2, rectText.top + 2, 0, &rectText, 
						strLabel, strLabel.GetLength(), 0);

		if (lpDrawItemStruct->itemState & ODS_FOCUS)
			pDC->DrawFocusRect(&rectText);
	}
}
