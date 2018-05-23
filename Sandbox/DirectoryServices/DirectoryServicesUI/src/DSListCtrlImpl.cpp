 //////////////////////////////////////////////////////////////////////////////
// DSListCtrlImpl.cpp
// 
//
// Created: 09/13/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_LIST_CTRL_INTERFACE_IMPLEMENTATION_
#include "DSListCtrlImpl.h"

IMPLEMENT_DYNCREATE(DSListCtrl, C3DFormView)

BEGIN_MESSAGE_MAP(DSListCtrl, C3DFormView)
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_NOTIFY(LVN_COLUMNCLICK, IDD_NDS_LC_RESULTS_TOC, OnColumnClick)
    ON_WM_KEYUP()
//    ON_NOTIFY(NM_CLICK, IDD_NDS_LC_RESULTS_TOC, OnResultsTocClk)
END_MESSAGE_MAP()

DSListCtrl::DSListCtrl()
    : C3DFormView(IDD_NDS_DIRSERV_RESULTS_LIST), customToc(0), m_cxChar(0),
      widths(0), sizeCustomTable(0), sizeDefTable(0), pDS(0), shiftDown(FALSE),
      nCurHeight(INT_MIN)
{
    sizeCustomTable  = sizeof(fieldTbl) / sizeof(fieldTbl[0]);
    widths           = new int[sizeCustomTable];
    for (uint i = 0; i < sizeCustomTable; i++) {
	widths[i]  = -1;
    }
    sizeDefTable   = sizeof(m_pszText) / sizeof(m_pszText[0]);
}

DSListCtrl::~DSListCtrl()
{
    if (widths) {
	delete [] widths;
	widths = 0;
    }
}

BOOL
DSListCtrl::SerializeSerializables()
{
    BOOL        bWritten   = FALSE;

    // Write the widht of the right pane.
    CRString    dsSect(IDS_NDS_DIRSERV_SECTION);

    char        buffer[32] = { '\0' };
    const char *section    = dsSect;
    const char *topPaneY   = "TopPaneY";
    const char *itoaBuf    = ::itoa(nCurHeight, buffer, 10);
    const char *iniPath    = INIPath;

    if (section && topPaneY && itoaBuf && iniPath) {
	bWritten = ::WritePrivateProfileString(section, topPaneY, itoaBuf,
					       iniPath);
    }

    return bWritten;
}

void
DSListCtrl::PrepareForDestruction()
{
    ASSERT(m_LC_ResultsToc.m_hWnd);
    int         iCount       = m_LC_ResultsToc.GetItemCount();
    IDSPRecord *pRec         = NULL;

    for (int i = 0; i < iCount; i++) {
	pRec = (IDSPRecord *)
	    GetListItemLPARAM(i);
	if (pRec) {
	    pRec->Release(); 
	}
    }
}

void
DSListCtrl::Reset()
{
    int         iCount       = m_LC_ResultsToc.GetItemCount();
    IDSPRecord *pRec         = NULL;

    for (int i = 0; i < iCount; i++) {
	pRec = (IDSPRecord *)
	    GetListItemLPARAM(i);
	if (pRec) {
	    pRec->Release(); 
	}
    }
    
    m_LC_ResultsToc.DeleteAllItems();
    // Apprise the parent that items have changed.
    m_LC_ResultsToc.AppriseChangeView();

    uint idx = 0;
    if (customToc == 0) {
	for (idx = 0; idx < sizeDefTable; idx++) {
	    TocSetColumnWidth(idx, (m_cxChar * strlen(m_pszText[idx])) + 20);
	}
    }
    else {
	for (uint i = 0; i < sizeCustomTable; i++) {
	    if (widths[i] != -1) {
		TocSetColumnWidth(idx, widths[i]);
		idx++;
	    }
	}
    }
}

void
DSListCtrl::DoDataExchange(CDataExchange *pDX)
{
    C3DFormView::DoDataExchange(pDX);

    // Results
    DDX_Control(pDX, IDD_NDS_LC_RESULTS_TOC, m_LC_ResultsToc);
}

void
DSListCtrl::OnInitialUpdate()
{
    C3DFormView::OnInitialUpdate();

    SetScaleToFitSize(CSize(1, 1));
    RECT rect;
    GetClientRect(&rect);
    OnSize(SIZE_RESTORED, rect.right - rect.left, rect.bottom - rect.top);

    TEXTMETRIC tm;
    CClientDC  dc(&m_LC_ResultsToc);
    dc.GetTextMetrics(&tm);
    m_cxChar = tm.tmAveCharWidth;

    customToc = IsCustomToc();
    if (customToc == 0) {
	SetupDefaultColumns();
    }
    else {
	SetupCustomColumns();
    }
}

void
DSListCtrl::OnResultsTocClk(NMHDR *pNMHDR, LRESULT *pResult)
{
//    m_LC_ResultsToc.OnResultsTocClk(pNMHDR, pResult);
}

void
DSListCtrl::OnKeyUp(UINT nChar, UINT x, UINT y)
{
    m_LC_ResultsToc.OnKeyUp(nChar, x, y);
}

int
DSListCtrl::TocInsertColumn(UINT mask, int fmt, int cx, LPTSTR pszText,
			    int item, int iSubItem)
{
    LV_COLUMN lvc;

    memset(&lvc, 0, sizeof(LV_COLUMN));

    lvc.mask     = mask;
    lvc.fmt      = fmt;
    lvc.cx       = cx;
    lvc.pszText  = pszText;
    lvc.iSubItem = iSubItem;

    return(m_LC_ResultsToc.InsertColumn(item, &lvc));
}

void
DSListCtrl::SetupCustomColumns()
{
    UINT mask      = LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_TEXT;
    int  fmt       = LVCFMT_LEFT;
    int  cx        = 100;
    uint keyVal    = 0;
    uint idx       = 0;

    for (uint i = 0; i < sizeCustomTable; i++) {
	uint defKeyVal = 0;
	if (!strcmp(fieldTbl[i], "Name") || !strcmp(fieldTbl[i], "Email") ||
	    !strcmp(fieldTbl[i], "Database")) {
	    defKeyVal = 1;
	}
	keyVal = GetPrivateProfileInt("DirectoryServicesResultsToc",
				      fieldTbl[i], defKeyVal, INIPath);
	if (keyVal == 1) {
	    TocInsertColumn(mask, fmt, cx, fieldTbl[i], idx, 0);
	    widths[i] = (m_cxChar * strlen(fieldTbl[i])) + 20;
	    TocSetColumnWidth(idx, widths[i]);
	    idx++;
	}
    }
}

void
DSListCtrl::SetupDefaultColumns()
{
    static bool     bSetup                 = false;
    UINT   mask                            = LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM|
	                                     LVCF_TEXT;
    int    fmt                             = LVCFMT_LEFT;
    int    cx                              = 100;
    int    iSubItem                        = 0;

    if (bSetup == false) {
	bSetup = true;
	for (uint idx = 0; idx < sizeDefTable; idx++) {
	    TocInsertColumn(mask, fmt, cx, m_pszText[idx], idx, iSubItem);
	    TocSetColumnWidth(idx, (m_cxChar * strlen(m_pszText[idx])) + 20);
	}
    }
}

BOOL
DSListCtrl::TocSetColumnWidth(int idx, int width)
{
    return(SetColumnWidth(idx, width));
}

uint
DSListCtrl::IsCustomToc()
{
    return(GetPrivateProfileInt("DirectoryServicesResultsToc", "CustomToc",
				0, INIPath));
}

void
DSListCtrl::InitTOCImageList(CImgCache *imgCache)
{
    static bool visited = false;

    if (visited == false) {
	visited = true;
        if (imgCache) {
	    ListView_SetImageList(GetDlgItem(IDD_NDS_LC_RESULTS_TOC)->m_hWnd,
				  imgCache->GetImageList(), LVSIL_SMALL);
	}
    }
}

void
DSListCtrl::InitResultsTOC(DSBM *pDSBM, CImgCache *)
{
    m_LC_ResultsToc.InitResultsTOC(pDSBM);
}

void
DSListCtrl::OnSize(UINT nType, int cx, int cy)
{
    C3DFormView::OnSize(nType, cx, cy);

    // Don't resize if the controls aren't created yet, or the window
    // is being minimized.
    if ((m_LC_ResultsToc.m_hWnd == NULL) || (nType == SIZE_MINIMIZED ||
	 cx == -1 || cy == -1)) {
	return;
    }

    // Size the TOC
    CRect rectLCResultsToc;
    rectLCResultsToc.TopLeft().x = 0;
    rectLCResultsToc.TopLeft().y = 0;
    rectLCResultsToc.BottomRight().x = cx;
    rectLCResultsToc.BottomRight().y = cy;
    m_LC_ResultsToc.MoveWindow(rectLCResultsToc);

    if (::ShouldUpdateIni() && (nType != SIZE_MINIMIZED)) {
	nCurHeight = cy;
	SerializeSerializables();
    }
}

////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void
DSListCtrl::OnDestroy()
{
    PrepareForDestruction();
    C3DFormView::OnDestroy();
}

LPARAM
DSListCtrl::GetListItemLPARAM(int nItem)
{
    return m_LC_ResultsToc.GetListItemLPARAM(nItem);
}

LPARAM
DSListCtrl::GetListFocusedItemLPARAM()
{
    int idx        = -1;
    LPARAM lParam  = 0;

    if ((idx = m_LC_ResultsToc.GetNextItem(-1, LVNI_FOCUSED)) != -1) {
	return(GetListItemLPARAM(idx));
    }

    return(0);
}

void
DSListCtrl::InsertDefaultTocItems(int item, char **table)
{
    uint idx;
    for (idx = 0; idx < sizeDefTable; idx++) {
	 if (table[idx] != 0) {
	     if (idx != 3) {
		 m_LC_ResultsToc.SetItemText(item, idx, table[idx]);
	     }
	     else {
		 if (table[idx][0] != '\0') {
		     m_LC_ResultsToc.SetItemText(item, idx, table[idx]);
		 }
	     }
	     TocSetColumnWidth(idx, LVSCW_AUTOSIZE);
	 }
    }
}


int
DSListCtrl::InsertItem(char **table, HBITMAP hBit, CImgCache *imgCache,
		       IDSPRecord *pRecord)
{
    LV_ITEM lvi;
    memset(&lvi, 0, sizeof(LV_ITEM));

    if (hBit != NULL && imgCache) {
	lvi.iImage  = imgCache->Add(hBit);
	lvi.mask   |= LVIF_IMAGE;
    }

    lvi.mask |= LVIF_TEXT;
    lvi.pszText = "";
    
    lvi.mask |= LVIF_PARAM;
    lvi.lParam = (LPARAM)(pRecord);
    pRecord->AddRef();
    lvi.iItem = m_LC_ResultsToc.GetItemCount();
    lvi.iSubItem = 0;

    int item = m_LC_ResultsToc.InsertItem(&lvi);

    if (item != -1) {
	if (customToc == 0) {
	    InsertDefaultTocItems(item, table);
	}
	else {
	}
    }

    return(item);
}

void
DSListCtrl::AllocDBEntryAscii(char **buf, DBRECENT *pEnt)
{
    int nLen = pEnt->dwSize;
    *buf     = new char[nLen + 1];
    fBaptizeBlockMT(*buf, "DSUI-TOC");
    fNoteBlockOriginMT(*buf, THIS_FILE, __LINE__ - 2);
    strncpy(*buf, (char *)(pEnt->data), nLen);
    (*buf)[nLen] = '\0';
}

void
DSListCtrl::AllocDBEntryAsciiZ(char **buf, DBRECENT *pEnt)
{
    int nLen = strlen((char *)(pEnt->data));
    *buf     = new char[nLen + 1];
    fBaptizeBlockMT(*buf, "DSUI-TOC");
    fNoteBlockOriginMT(*buf, THIS_FILE, __LINE__ - 2);
    strncpy(*buf, (char *)(pEnt->data), nLen + 1);
}

void
DSListCtrl::SetupDSBuffer(char **buf, DBRECENT *pEnt)
{
    switch(pEnt->nType) {
    case DST_ASCII:
	AllocDBEntryAscii(buf, pEnt);
	break;
    case DST_ASCIIZ:
	AllocDBEntryAsciiZ(buf, pEnt);
	break;
    }
}

// This function breaks the generalization of Toc items.
// Will come back post 4.0 and make it general.
bool
DSListCtrl::GetResultsTocEmailList(int item, CStringList& curEmailList)
{
    bool retVal = false;
    IDSPRecord *pRecord = (IDSPRecord *) GetListItemLPARAM(item);
    DBRECENT   *pDRec   = pRecord->GetRecordList();

    if (pDRec) {
	DBRECENT *pTmp = pDRec;
	while (pTmp) {
	    if (pTmp->nName == DS_EMAIL) {
		CString tmpStr((char *)(pTmp->data));
		if (!tmpStr.IsEmpty()) {
		    retVal = true;
		}
		curEmailList.AddTail(tmpStr);
	    }
	    pTmp = pTmp->pNext;
	}
    }
    return(retVal);
}
    
void
DSListCtrl::GetDefaultTocItems(IDSPRecord *pRecord, char **table)
{
    // A hit may have more than one name, work-phone, or work-email.
    // We just show the first one we encounter in the results hit.
    bool      nameVisited   = false;
    bool      emailVisited  = false;
    bool      phoneVisited  = false;
    int       item          = 0;
    DBRECENT *pDRec         = pRecord->GetRecordList();

    if (pDRec) {
	DBRECENT *pTmp = pDRec;

	while (pTmp) {
	    if (pTmp->nName == DS_NAME) {
		if (nameVisited == false) {
		    nameVisited = true;
		}
		else {
		    pTmp = pTmp->pNext;
		    continue;
		}
		SetupDSBuffer(&(table[0]), pTmp);
	    }
	    else if (pTmp->nName == DS_EMAIL) {
		if (emailVisited == false) {
		    emailVisited = true;
		}
		else {
		    pTmp = pTmp->pNext;
		    continue;
		}
		SetupDSBuffer(&(table[1]), pTmp);
	    }
	    else if (pTmp->nName == DS_PHONE) {
		if (phoneVisited == false) {
		    phoneVisited  = true;
		}
		else {
		    pTmp = pTmp->pNext;
		    continue;
		}
		SetupDSBuffer(&(table[2]), pTmp);
	    }
		
	    pTmp = pTmp->pNext;
	}
    }
}

IDSDatabase *
DSListCtrl::GetDatabaseFromRecord(IDSPRecord *pRecord)
{
    char *szDatabaseID     = pRecord->GetDatabaseID();
    if (szDatabaseID) {
	return(pDS->pDirServ->FindDatabase(szDatabaseID));
    }
    else {
	return(0);
    }
}

void
DSListCtrl::InitializeDatabaseName(IDSDatabase *pDatabase, char *buf, int size)
{
    HRESULT hErr;
    hErr = pDatabase->GetName(buf, size);

    if (FAILED(hErr)) {
	memset(buf, '\0', size);
    }
}

HBITMAP
DSListCtrl::GetBitmapFromRecord(IDSPRecord *pRecord)
{
    HBITMAP hBit = NULL;

    IDSDatabase *pDatabase = GetDatabaseFromRecord(pRecord);
    if (pDatabase) {
	hBit = pDatabase->GetProtocolImage(IMG_SMALL);
	pDatabase->Release();
    }

    return(hBit);
}

void
DSListCtrl::GetDefaultTocItemsFromRecord(IDSPRecord *pRecord, char ***table)
{
    *table = new char *[sizeDefTable * sizeof(char *)];
	
    for (uint i = 0; i < sizeDefTable; i++) {
	(*table)[i] = 0;
    }

    GetDefaultTocItems(pRecord, *table);
    IDSDatabase *pDatabase = GetDatabaseFromRecord(pRecord);
    if (pDatabase) {
	(*table)[3] = new char[128];
	InitializeDatabaseName(pDatabase, (*table)[3], 128);
	pDatabase->Release();
    }
}

int
DSListCtrl::InsertDefaultItem(IDSPRecord *pRecord,
			      CImgCache *imgCache, AHCB addHitCB,
			      void *data)
{
    HBITMAP  hBit = NULL;
    char   **table;
    int      item;

    GetDefaultTocItemsFromRecord(pRecord, &table);
    hBit = GetBitmapFromRecord(pRecord);

    if (table[0]) {
	item = InsertItem(table, hBit, imgCache, pRecord);
    }
    else {
	char szBuf[128];
	IDSDatabase *pDatabase = GetDatabaseFromRecord(pRecord);
	pDatabase->GetProtocolName(szBuf, 128);
	if (strcmp(szBuf, "Finger") == 0) {	// Is it from finger?
	    table[0] = new char[strlen("<See Details>") + 1];
	    strcpy(table[0], "<See Details>");
	}
	item = InsertItem(table, hBit, imgCache, pRecord);
    }

    DeleteDefaultResultsItemTocMemory(table);

    // if this is the first item, splat the contents to the
    // details window.
    if (m_LC_ResultsToc.GetItemCount() == 1) {
	m_LC_ResultsToc.SplatContents(pRecord, data);
    }
    // Call the AddHit callback.
    if (addHitCB) {
	(*addHitCB)(data);
    }

    return(item);
}

int
DSListCtrl::InsertItem(_DS *pDSIn, IDSPRecord *pRecord, CImgCache *imgCache,
                       AHCB addHitCB, void *data)
{
    ASSERT(pDSIn);
    pDS = pDSIn;
    if (customToc == 0) {
	return(InsertDefaultItem(pRecord, imgCache, addHitCB, data));
    }
    else {
	return(0);
    }

}

BOOL
DSListCtrl::SetItemText(int nItem, int nSubItem, LPTSTR lpszTxt)
{
    return(m_LC_ResultsToc.SetItemText(nItem, nSubItem, lpszTxt));
}

BOOL
DSListCtrl::SetColumnWidth(int nCol, int cx)
{
    return(m_LC_ResultsToc.SetColumnWidth(nCol, cx));
}

int
DSListCtrl::ImageList_Add(HBITMAP hBit)
{
    return(::ImageList_Add(hilResultsTOC_Entry, hBit, NULL));
}

int
DSListCtrl::GetItemCount()
{
    return(m_LC_ResultsToc.GetItemCount());
}

void
DSListCtrl::SelectFirstItem()
{
    m_LC_ResultsToc.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED,
				 LVIS_SELECTED|LVIS_FOCUSED);
}

CString
DSListCtrl::GetItemText(int iItem, int iSubItem)
{
    return(m_LC_ResultsToc.GetItemText(iItem, iSubItem));
}

int
DSListCtrl::GetItemText(int iItem, int iSubItem, LPTSTR lpszText, int nLen)
{
    return(m_LC_ResultsToc.GetItemText(iItem, iSubItem, lpszText, nLen));
}

void
DSListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *nLV = (NM_LISTVIEW *) pNMHDR;
    _DS_Composite dsc;

    shiftDown = ShiftDown();
    dsc.pDSLC   = this;
    dsc.subItem = nLV->iSubItem;
    m_LC_ResultsToc.SortItems(CompareFunc, (DWORD)(&dsc));
    *pResult = 0;
}

int CALLBACK
DSListCtrl::CompareFunc(LPARAM one, LPARAM two, LPARAM data)
{
    _DS_Composite *pDSC = (_DS_Composite *) data;
    if (pDSC->pDSLC->customToc == 0) {
	return(DefaultCompareFunc(one, two, pDSC));
    }
    else {
	return(0);
    }
}

int
DSListCtrl::DefaultCompareFunc(LPARAM one, LPARAM two, _DS_Composite *pDSC)
{
    int retVal;
    char **table1, **table2;
    IDSPRecord *pRecOne = (IDSPRecord *) one;
    IDSPRecord *pRecTwo = (IDSPRecord *) two;

    pDSC->pDSLC->GetDefaultTocItemsFromRecord(pRecOne, &table1);
    pDSC->pDSLC->GetDefaultTocItemsFromRecord(pRecTwo, &table2);

    int column = pDSC->subItem;

    // it is possible that the one of the two items is empty, or
    // that both are empty.

    if (table1[column] == 0 && table2[column] == 0) {
	retVal = 0;
	goto theend;
    }
    else if (table1[column] == 0) {
	retVal = 1;
	goto theend;
    }
    else if (table2[column] == 0) {
	retVal = -1;
	goto theend;
    }

    retVal = strcmp(table1[column], table2[column]);

    switch(column) {
    case 0:
	// comparison by name.
	if (retVal == 0) { // do an additional comparison by email.
	    if (table1[1] && table2[1]) {
		retVal = strcmp(table1[1], table2[1]);
	    }
	}
	break;
    case 1: case 2: case 3:
	// comparison by email, or phone or database.
	if (retVal == 0) { // do an additional comparison by name.
	    if (table1[0] && table2[0]) {
		retVal = strcmp(table1[0], table2[0]);
	    }
	}
	break;
    default:
	ASSERT(0);
	break;
    }

theend:

    pDSC->pDSLC->DeleteDefaultResultsItemTocMemory(table1);
    pDSC->pDSLC->DeleteDefaultResultsItemTocMemory(table2);

    if (pDSC->pDSLC->shiftDown == TRUE) {
	if (retVal > 0) {
	    retVal = -1;
	}
	else if (retVal < 0) {
	    retVal = 1;
	}
    }
    return(retVal);
}

void
DSListCtrl::DeleteDefaultResultsItemTocMemory(char **table)
{
    for (uint i = 0; i < sizeDefTable; i++) {
	if (table[i]) {
	    delete [] table[i];
	    table[i] = 0;
	}
    }

    if (table) {
	delete [] table;
	table = 0;
    }
}

LRESULT
DSListCtrl::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_GETDLGCODE:
	return(DLGC_WANTALLKEYS);
    case WM_CHAR:
    case WM_SYSCHAR:
	if (ShouldPropagateMessageToParent(wMessage, wParam, lParam)) {
	    ::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
	    return(0);
	}
	break;
    default:
	break;
    }

    return(C3DFormView::WindowProc(wMessage, wParam, lParam));
} 


