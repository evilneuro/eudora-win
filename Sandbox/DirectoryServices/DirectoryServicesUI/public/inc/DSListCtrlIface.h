///////////////////////////////////////////////////////////////////////////////
// DSListCtrlIface.h
//
// Created: 09/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_LIST_CTRL_IFACE_H_
#define __DS_LIST_CTRL_IFACE_H_

#include "3dformv.h"
#include "DSBounceMessageImpl.h"
#include "imgcache.h"
#include "dsapi.h"
#include "DirectoryServicesBaseTypes.h"
#include "DSResultsTocIface.h"


class DSListCtrl;

struct _DS_Composite {
    DSListCtrl *pDSLC;
    int         subItem;
};

typedef void (*AHCB)(void *);
typedef Directory_Services _DS;

class DSListCtrl : public C3DFormView {
    DECLARE_DYNCREATE(DSListCtrl)

public:
                   DSListCtrl();
    virtual       ~DSListCtrl();
    virtual BOOL   Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
			  DWORD dwRequestedStyle, const RECT& rect,
			  CWnd *pParentWnd, UINT nID,
			  CCreateContext *pContext = NULL)
    {
	return(C3DFormView::Create(lpszClassName, lpszWindowName,
				   dwRequestedStyle, rect, pParentWnd, nID,
				   pContext));
    }
    virtual BOOL   OnCmdMsg(UINT nID, int nCode, void *pExtra,
			    AFX_CMDHANDLERINFO *pHandlerInfo)
    {

	return(C3DFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo));
    }

    void           PrepareForDestruction();
    void           Reset();
    void           InitResultsTOC(_ds_bounce_message *pDSBounceMessage,
				  CImgCache *imgCache);
    void           InitTOCImageList(CImgCache *imgCache);
    LPARAM         GetListItemLPARAM(int nItem);
    LPARAM         GetListFocusedItemLPARAM();

    BOOL           SetItemText(int nItem, int nSubItem, LPTSTR lpszTxt);
    BOOL           SetColumnWidth(int nCol, int cx);
    int            ImageList_Add(HBITMAP hBit);
    int            GetItemCount();
    void           SelectFirstItem();
    CString        GetItemText(int, int);
    int            GetItemText(int iItem, int iSubItem, LPTSTR lpszText,
			       int nLen);
    UINT           GetItemState(int nItem, UINT nMask)
    {
	return(m_LC_ResultsToc.GetItemState(nItem, nMask));
    }

    UINT           GetSelectedCount()
    {
	return(m_LC_ResultsToc.GetSelectedCount());
    }
    int            InsertItem(_DS *pDS, IDSPRecord *pRecord,
			      CImgCache *imgCache, AHCB addHitCB, void *data);
    bool           GetResultsTocEmailList(int item, CStringList& curEmailList);
    int            GetNextItem(int nItem, int nFlags)
    {
	return(m_LC_ResultsToc.GetNextItem(nItem, nFlags));
    }
    
    HWND           FetchControlWindow() { return(m_LC_ResultsToc.m_hWnd); }

private:

    DSResultsToc      m_LC_ResultsToc;
    HIMAGELIST     hilResultsTOC_Entry;
    uint           customToc;
    int            m_cxChar;
    int           *widths;
    uint           sizeCustomTable;
    uint           sizeDefTable;
    _DS           *pDS;
    BOOL           shiftDown;
    int            nCurHeight;
    int            TocInsertColumn(UINT, int, int, LPTSTR, int, int);
    void           SetupDefaultColumns();
    void           SetupCustomColumns();
    BOOL           TocSetColumnWidth(int idx, int width);
    uint           IsCustomToc();
    int            InsertDefaultItem(IDSPRecord  *pRecord,
				     CImgCache *imgCache, AHCB addHitCB,
				     void *data);
    int            InsertItem(char **table, HBITMAP hBit,
			      CImgCache *imgCache, IDSPRecord *pRecord);
    void           AllocDBEntryAscii(char **buf, DBRECENT *pEnt);
    void           AllocDBEntryAsciiZ(char **buf, DBRECENT *pEnt);
    void           SetupDSBuffer(char **buf, DBRECENT *pEnt);
    void           InsertDefaultTocItems(int item, char **table);
    void           GetDefaultTocItems(IDSPRecord *pRecord, char **table);
    IDSDatabase   *GetDatabaseFromRecord(IDSPRecord *pRecord);
    void           InitializeDatabaseName(IDSDatabase *pDatabase, char *buf,
					 int size);
    HBITMAP        GetBitmapFromRecord(IDSPRecord *pRecord);
    void           GetDefaultTocItemsFromRecord(IDSPRecord *pRecord,
						char ***table);
    void           DeleteDefaultResultsItemTocMemory(char **table);
    void           ResetRecordList();
    BOOL           SerializeSerializables();

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    static  int    CALLBACK CompareFunc(LPARAM, LPARAM, LPARAM);
    static  int    DefaultCompareFunc(LPARAM, LPARAM, _DS_Composite *);
            void   DoDataExchange(CDataExchange *pDX);
            void   OnInitialUpdate();

    afx_msg void   OnResultsTocClk(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void   OnKeyUp(UINT, UINT, UINT);
    afx_msg void   OnSize(UINT, int, int);
    afx_msg void   OnDestroy();
    afx_msg void   OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
    DECLARE_MESSAGE_MAP()
};

#endif // __DS_LIST_CTRL_IFACE_H_
