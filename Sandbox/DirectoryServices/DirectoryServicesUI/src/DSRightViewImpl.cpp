//////////////////////////////////////////////////////////////////////////////
// DSRightViewImpl.cpp
// 
//
// Created: 10/09/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_RIGHT_VIEW_INTERFACE_IMPLEMENTATION_
#include "DSRightViewImpl.h"

#include "QCSharewareManager.h"

#ifndef BASED_CODE
#define BASED_CODE
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(DSRightView, C3DFormView)

BEGIN_MESSAGE_MAP(DSRightView, C3DFormView)
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_NOTIFY(NM_CLICK, IDD_NDS_LC_DATABASES, OnDatabasesClk)
    ON_NOTIFY(NM_DBLCLK, IDD_NDS_LC_DATABASES, OnDatabasesDblClk)
    ON_NOTIFY(LVN_ITEMCHANGED, IDD_NDS_LC_DATABASES, OnDatabasesItemChange)
    ON_BN_CLICKED(IDD_NDS_PB_MODIFYDATABASE, OnModifyDatabase)
    ON_BN_CLICKED(IDD_NDS_PB_DELETEDATABASE, OnDeleteDatabase)
    ON_BN_CLICKED(IDD_NDS_PB_NEWDATABASE, OnNewDatabase)
    ON_NOTIFY(NM_DBLCLK, IDD_NDS_LC_PROTOCOLS, OnProtocolsDblClk)
    ON_NOTIFY(LVN_COLUMNCLICK, IDD_NDS_LC_DATABASES, OnColumnClickDatabases)
    ON_NOTIFY(LVN_COLUMNCLICK, IDD_NDS_LC_PROTOCOLS, OnColumnClickProtocols)
    ON_REGISTERED_MESSAGE(umsgActivateWazoo, OnActivateWazoo)
END_MESSAGE_MAP()

DSRightView::DSRightView()
    : C3DFormView(IDD_NDS_DIRSERV_RIGHTVIEW), pDS(NULL),
      XBaseUnit(LOWORD(::GetDialogBaseUnits())),
      YBaseUnit(HIWORD(::GetDialogBaseUnits())), shiftDown(FALSE),
      imgCache(NULL), nCurWidth(INT_MIN)
{
    // Control metrics.
    // Horizontal and vertical space.
    xSpace                 = (XBaseUnit);
    ySpace                 = (YBaseUnit)      / 2;
    tcSpace                = (YBaseUnit);
    pushButtonWidth        = (30 * XBaseUnit) / 4;
    pushButtonWidthEx      = (50 * XBaseUnit) / 4;
    pushButtonHeight       = (14 * YBaseUnit) / 8 - ySpace;

    // Horizontal metrics
    // We impose a minimum width for the protocols group box.
    minProtocolsWidth      = (2 * xSpace) + pushButtonWidthEx;
    // We also impose a minimum width for the databases group box.
    minDatabasesWidth      = (3 * xSpace) + (2 * pushButtonWidth);
    // Thus, the min Dialog width is the larger of the two.
    minDialogWidth         = ((minProtocolsWidth <= minDatabasesWidth) ?
	                     minDatabasesWidth : minProtocolsWidth) +
                             (2 * xSpace);

    // Vertical metrics
    // We impose a minimum height for the protocols list --- roughly
    // twice the pushbutton height.
    minLCProtocolsHeight   = (25 * YBaseUnit) / 8;
    // We also impose a maximum height for the protocols group box.
    maxProtocolsHeight     = 180;
    // The min height of the protocols group box is ...
    minProtocolsHeight     = tcSpace + (2 * ySpace) + minLCProtocolsHeight +
	                     pushButtonHeight;

    minLCDatabasesHeight   = (50 * YBaseUnit) / 8;
    minDatabasesHeight     = tcSpace + (2 * ySpace) + minLCDatabasesHeight +
	                     pushButtonHeight;

    minDialogHeight        = (3 * ySpace) + minProtocolsHeight +
	                     minDatabasesHeight;
}

DSRightView::~DSRightView()
{
    if (imgCache) {
	delete imgCache;
	imgCache = 0;
    }

    if (pDS != NULL) {
	if (pDS->pDirServ) {
	    pDS->pDirServ->Release();
	    pDS->pDirServ = NULL;
	}
	delete pDS;
	pDS = NULL;
    }
}

BOOL
DSRightView::SerializeSerializables()
{
    BOOL        bWritten   = FALSE;

    // Write the widht of the right pane.
    CRString    dsSect(IDS_NDS_DIRSERV_SECTION),
	        dsRightPane(IDS_NDS_DIRSERV_RIGHTPANE_X);

    char        buffer[32] = { '\0' };
    const char *section    = dsSect;
    const char *rightPane  = dsRightPane;
    const char *itoaBuf    = ::itoa(nCurWidth, buffer, 10);
    const char *iniPath    = INIPath;

    if (section && rightPane && itoaBuf && iniPath) {
	bWritten = ::WritePrivateProfileString(section, rightPane, itoaBuf,
					       iniPath);
    }
    
    return bWritten;
}

void
DSRightView::PrepareForDestruction()
{
    ASSERT(m_LC_Databases.m_hWnd);
    SerializeDatabasesState();
    ResetDatabases();
    ASSERT(m_LC_Protocols.m_hWnd);
    ResetProtocols();
}

void
DSRightView::SerializeDatabasesState()
{
    CRString dsSection(IDS_NDS_DIRSERV_SECTION);
    int   iCount = m_LC_Databases.GetItemCount();
    void *pv     = static_cast<void *>(&m_LC_Databases);

    for (int i = 0; i < iCount; i++) {
	UINT    nState  = m_LC_Databases.GetItemState(i, LVIS_STATEIMAGEMASK);
	HRESULT hErrName, hErrProtoName;
	char    szName[128] = {'\0'}, szProtoName[128] = {'\0'},
	        iniStr[258] = {'\0'};

	IDSDatabase *pDatabase = (IDSDatabase *) ::GetListItemLPARAM(pv, i);
	ASSERT(pDatabase);

	hErrName      = pDatabase->GetName(szName, sizeof(szName));
	hErrProtoName = pDatabase->GetProtocolName(szProtoName,
						   sizeof(szProtoName));
	nState >>= 12;
	nState--;

	if (SUCCEEDED(hErrName) && SUCCEEDED(hErrProtoName)) {
	    strcpy(iniStr, szProtoName);
	    strcat(iniStr, ":");
	    strcat(iniStr, szName);
	    if (nState) { // this database is checked...
		::WritePrivateProfileString(dsSection, iniStr, "1", INIPath);
	    }
	    else { // this database is not checked...
		::WritePrivateProfileString(dsSection, iniStr, "0", INIPath);
	    }
	}
    }
}

void
DSRightView::DoDataExchange(CDataExchange *pDX)
{
    C3DFormView::DoDataExchange(pDX);

    // Database
    DDX_Control(pDX, IDD_NDS_LC_PROTOCOLS, m_LC_Protocols);
    DDX_Control(pDX, IDD_NDS_PB_NEWDATABASE, m_PB_NewDatabase);
    DDX_Control(pDX, IDD_NDS_LC_DATABASES, m_LC_Databases);
    DDX_Control(pDX, IDD_NDS_PB_MODIFYDATABASE, m_PB_ModifyDatabase);
    DDX_Control(pDX, IDD_NDS_PB_DELETEDATABASE, m_PB_DeleteDatabase);
}

void
DSRightView::OnInitialUpdate()
{
    C3DFormView::OnInitialUpdate();

	// Shareware: Register that we want to know of feature changes
	QCSharewareManager *pSWM = QCSharewareManager::GetSharewareManager();
	ASSERT(pSWM);
	if (pSWM)
	{
		pSWM->QCCommandDirector::Register((QICommandClient*)this);
	}

    m_PB_NewDatabase.EnableWindow(TRUE);
    m_PB_ModifyDatabase.EnableWindow(FALSE);
    m_PB_DeleteDatabase.EnableWindow(FALSE);

    SetScaleToFitSize(CSize(1, 1));
    RECT rect;
    GetClientRect(&rect);
    OnSize(SIZE_RESTORED, rect.right - rect.left, rect.bottom - rect.top);
}

////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [protected]
//
// Perform Just In Time display initialization.
////////////////////////////////////////////////////////////////////////
long 
DSRightView::OnActivateWazoo(WPARAM, LPARAM)
{
    ::ShouldUpdateIni(1);

    if (m_PB_NewDatabase.GetSafeHwnd() && (NULL == pDS)) {
	pDS = new _directory_services;
	fBaptizeBlockMT(static_cast<void *>(pDS), "DSUI-DS");

        HRESULT hRegCOM = ::RegisterCOMObjects();

	if (SUCCEEDED(hRegCOM)) {

	    imgCache = new CImgCache;

	    if (imgCache) {
		imgCache->Initialize(RGB(255, 0, 255));
	    }

	    if (pDS) {
		HRESULT hErr;

		hErr = CoCreateInstance(CLSID_DirServ, NULL,
					CLSCTX_INPROC_SERVER,
					IID_IDirServ,
					(void **)(&(pDS->pDirServ)));

		if (SUCCEEDED(hErr)) {
		    ASSERT(pDS->pDirServ);
		    hErr = pDS->pDirServ->Initialize();
		    if (SUCCEEDED(hErr)) {
			InitProtocols();
			PopulateProtocols();
			InitDatabases();
			PopulateDatabases();
		    }
		}
	    }
	}
	else {
	    ::MessageBox(NULL, _ds_uds, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
    }

	DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
	DSLeftView        *pDSLV  = static_cast<DSLeftView *>
                                (pDSMSW->GetPane(0, 0));

   pDSLV->SetQueryInputFocus();

    return 0;
}


void
DSRightView::InitProtocols()
{
    LV_COLUMN lvc;
    memset(&lvc, 0, sizeof(LV_COLUMN));
    ListView_SetImageList(GetDlgItem(IDD_NDS_LC_PROTOCOLS)->m_hWnd,
			  imgCache->GetImageList(), LVSIL_SMALL);
    lvc.mask     = LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_TEXT;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.cx       = 300;
    lvc.iSubItem = 0;
    lvc.pszText  = "Registered Drivers";

    m_LC_Protocols.InsertColumn(0, &lvc);
}

void
DSRightView::ResetProtocols()
{
    int iCount             = m_LC_Protocols.GetItemCount();
    IDSProtocol *pProto    = NULL;
    void *pv               = static_cast<void *>(&m_LC_Protocols);

    for (int i = 0; i < iCount; i++) {
	pProto = (IDSProtocol *)  ::GetListItemLPARAM(pv, i);
	if (pProto != NULL) {
	    pProto->Release();
	}
    }
    
    m_LC_Protocols.DeleteAllItems();
}

void
DSRightView::PopulateProtocols()
{
    ASSERT(pDS);
    ASSERT(pDS->pDirServ);
    HRESULT hErr;
    IDSEnumProtocols *pEnum = NULL;

    ResetProtocols();

    hErr = pDS->pDirServ->EnumProtocols(&pEnum);

    if (SUCCEEDED(hErr)) {
	IDSProtocol *pProto;
	ULONG nProto;
	int idx = 0;

	while(pEnum->Next(1, &pProto, &nProto) != S_FALSE) {
	    char szBuf[128];
	    LV_ITEM lvi;
	    HBITMAP hBit;

	    memset(&lvi, 0, sizeof(LV_ITEM));

	    hErr = pProto->GetProtocolName(szBuf, sizeof(szBuf));
	    if (SUCCEEDED(hErr)) {
		lvi.mask |= LVIF_TEXT;
		lvi.pszText = szBuf;
	    }

	    hBit = pProto->GetProtocolImage(IMG_SMALL);
	    if (hBit != NULL) {
		lvi.iImage  = imgCache->Add(hBit);
		lvi.mask |= LVIF_IMAGE;
	    }

	    lvi.mask |= LVIF_PARAM;
	    lvi.lParam = (LPARAM) pProto;
	    lvi.iItem = idx++;
	    lvi.iSubItem = 0;
	    m_LC_Protocols.InsertItem(&lvi);
	}
	pEnum->Release();
	m_LC_Protocols.SetItemState(0, LVIS_FOCUSED|LVIS_SELECTED,
				   LVIS_FOCUSED|LVIS_SELECTED);
	m_LC_Protocols.SetColumnWidth(0, LVSCW_AUTOSIZE);
    }
}

void
DSRightView::InitDatabases()
{
    ASSERT(pDS);
    LV_COLUMN lvc;

    memset(&lvc, 0, sizeof(LV_COLUMN));

    ListView_SetImageList(GetDlgItem(IDD_NDS_LC_DATABASES)->m_hWnd,
			  imgCache->GetImageList(), LVSIL_SMALL);

    COLORREF cRF = RGB(255, 255, 255);

    pDS->ilDatabase_State.Create(MAKEINTRESOURCE(IDB_NDS_DIRSERV_STATUSUN),
				 16, 1, cRF);

    m_LC_Databases.SetImageList(&(pDS->ilDatabase_State), LVSIL_STATE);

    lvc.mask     = LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_TEXT;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.cx       = 300;
    lvc.iSubItem = 0;
    lvc.pszText  = "Configured Servers";
    m_LC_Databases.InsertColumn(0, &lvc);

    lvc.cx       = 100;
    lvc.pszText  = "Protocol";
    m_LC_Databases.InsertColumn(1, &lvc);
}

void
DSRightView::PopulateDatabases()
{
    ASSERT(pDS);
    ASSERT(pDS->pDirServ);

    IDSEnumDatabases *pEnum = NULL;
    HRESULT           hErr;

    ResetDatabases();

    hErr = pDS->pDirServ->EnumDatabases(NULL, &pEnum);

    if (SUCCEEDED(hErr)) {
	CRString dsSection(IDS_NDS_DIRSERV_SECTION);
	IDSDatabase *pDatabase;
	ULONG        nDatabase;
	int          idx = 0;

	while(pEnum->Next(1, &pDatabase, &nDatabase) != S_FALSE) {
	    char    szBuf[128]       = {'\0'}, szName[128] = {'\0'},
 		    szProtoName[128] = {'\0'}, iniStr[258] = {'\0'};

	    LV_ITEM lvi;
	    HBITMAP hBit       = NULL;

	    memset(&lvi, 0, sizeof(LV_ITEM));
	    ASSERT(pDatabase);

	    hErr = pDatabase->GetName(szBuf, sizeof(szBuf));
	    if (SUCCEEDED(hErr)) {
		lvi.mask    |= LVIF_TEXT;
		lvi.pszText  = szBuf;
	    }

	    hBit = pDatabase->GetProtocolImage(IMG_SMALL);
	    if (hBit != NULL) {
		lvi.iImage  = imgCache->Add(hBit);
		lvi.mask  |= LVIF_IMAGE;
	    }

	    lvi.mask     |= LVIF_PARAM|LVIF_STATE;
	    lvi.lParam    = (LPARAM)(pDatabase);
	    lvi.iItem     = idx;
	    lvi.iSubItem  = 0;
	    lvi.stateMask = LVIS_STATEIMAGEMASK;
	    lvi.state     = INDEXTOSTATEIMAGEMASK(1);
	    m_LC_Databases.InsertItem(&lvi);

 	    HRESULT hErrProtoName;

 	    hErrProtoName = pDatabase->GetProtocolName(szProtoName,
 						       sizeof(szProtoName));
 	    if (SUCCEEDED(hErr) && SUCCEEDED(hErrProtoName)) {
 		strcpy(iniStr, szProtoName);
 		strcat(iniStr, ":");
 		strcat(iniStr, szBuf);
 		if (::GetPrivateProfileInt(dsSection, iniStr, 0, INIPath)) {
		    m_LC_Databases.SetItemState(idx, INDEXTOSTATEIMAGEMASK(2),
						LVIS_STATEIMAGEMASK);
		}
		m_LC_Databases.SetItemText(idx, 1, szProtoName);
	    }
	    idx++;
	}

	pEnum->Release();

	m_LC_Databases.SetItemState(0, LVIS_FOCUSED|LVIS_SELECTED,
				    LVIS_FOCUSED|LVIS_SELECTED);
    }

    if (m_LC_Databases.GetItemCount()) {
	UINT nSel;
	m_PB_ModifyDatabase.EnableWindow(
	    (nSel = m_LC_Databases.GetSelectedCount()) == 1);
	m_PB_DeleteDatabase.EnableWindow((nSel >= 1));
    }

    m_LC_Databases.SetColumnWidth(0, LVSCW_AUTOSIZE);
}

void
DSRightView::ResetDatabases()
{
    int iCount             = m_LC_Databases.GetItemCount();
    IDSDatabase *pDatabase = NULL;
    void *pv               = static_cast<void *>(&m_LC_Databases);

    for (int i = 0; i < iCount; i++) {
	pDatabase = (IDSDatabase *)  ::GetListItemLPARAM(pv, i);
	if (pDatabase != NULL) {
	    pDatabase->Release();
	}
    }
    
    m_LC_Databases.DeleteAllItems();
}

void
DSRightView::OnDatabasesClk(NMHDR *pNMHDR, LRESULT *pResult)
{
    UINT flags;
    int index;
    POINT pt;
    ::GetCursorPos(&pt);
    CPoint point(pt);

    // Get the location of click.
    m_LC_Databases.ScreenToClient(&point);

	UINT nState = 0;
    // If it's on the state icon do the appropriate thing...
    if ((index = m_LC_Databases.HitTest(point, &flags)) != -1) {
	if (flags & LVHT_ONITEMSTATEICON) {
	    nState = m_LC_Databases.GetItemState(index, LVIS_STATEIMAGEMASK);
	    nState >>= 12;
	    nState--;
	    nState = !nState;
	    nState++;
	    m_LC_Databases.SetItemState(index, INDEXTOSTATEIMAGEMASK(nState),
					LVIS_STATEIMAGEMASK);
	}
    }

	// Shareware: In reduced feature mode, you can only select one database
	QCSharewareManager *pSWM = QCSharewareManager::GetSharewareManager();
	ASSERT(pSWM);
	if ((pSWM) && (pSWM->UsingFullFeatureSet() == false))
	{
		// REDUCED-FEATURE MODE

		// Allow only one database to be selected at one time
		// If an item is selected, unselect all other items
		if(nState == 2)
		{
			int nItemCnt = m_LC_Databases.GetItemCount();
			for (int cnt = 0; cnt < nItemCnt; cnt++) 
			{
				if(cnt != index) 
				{
					m_LC_Databases.SetItemState(cnt, INDEXTOSTATEIMAGEMASK(1), 
						LVIS_STATEIMAGEMASK);	//unselect
				}
			}
		}
	}
    
	DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSLeftView        *pDSLV  = static_cast<DSLeftView *>
                                (pDSMSW->GetPane(0, 0));

    // If query is not in progress, enable or disable the start button.
    if (!pDSLV->IsQueryInProgress()) {

	// Post-processing --- find the first checked database if any.
	int nItemCnt = m_LC_Databases.GetItemCount();
	UINT nSt = 0;

	for (int cnt = 0; cnt < nItemCnt; cnt++) {
	    nSt = m_LC_Databases.GetItemState(cnt, LVIS_STATEIMAGEMASK);
	    nSt >>= 12;
	    nSt--;
	    if (nSt) {
		break;
	    }	
	}

	CString wndStr;
	pDSLV->GetQueryInput(wndStr);
	int nText = wndStr.GetLength();

	if (nText && nSt) {
	    pDSLV->EnableStartStop(TRUE);
	}
	else {
	    pDSLV->EnableStartStop(FALSE);
	}
    }

    *pResult = 0;
}

void
DSRightView::OnDatabasesDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
    ModifyDatabase();
    *pResult = 0;
}

void
DSRightView::ModifyDatabase()
{
    void *pv = static_cast<void *>(&m_LC_Databases);

    IDSDatabase *pDatabase = (IDSDatabase *) ::GetListFocusedItemLPARAM(pv);

    if (pDatabase != NULL) {
	if (ConfigDatabase(pDatabase) == FALSE) {
	    UpdateListFocused(pv);
	}
    }
}

BOOL
DSRightView::ConfigDatabase(IDSDatabase *pDatabase)
{
    ASSERT(pDatabase != NULL);
 
    int iCount     = 0;
    BOOL bCanceled = FALSE;

    if ((iCount = pDatabase->GetPropPageCount()) != 0) {

	HPROPSHEETPAGE *pPages;

	pPages = (HPROPSHEETPAGE *) malloc(sizeof(HPROPSHEETPAGE *) * iCount);
	if (pPages != NULL) {
	    if (SUCCEEDED(pDatabase->GetPropPages(pPages))) {
		CWnd *cWnd = GetFocus();
		int result = DisplayPropertySheet(pPages, iCount);

		if(result == 0) {
		    bCanceled = TRUE;
		}
		cWnd->SetFocus();
	    }
	    free(pPages);
	}
    }
    return(bCanceled);
}

int
DSRightView::DisplayPropertySheet(HPROPSHEETPAGE *pPages, int iCount)
{
    int result = 0;
    PROPSHEETHEADER psh;
    memset(&psh, 0, sizeof(PROPSHEETHEADER));

    // The dwSize member should be sizeof(PROPSHEETHEADER). But this
    // causes our INetSDK compiled stuff to fail on PropertySheet()
    // if you don't have IE4.0 installed. We just use the old header
    // size as a workaround.
    psh.dwSize          = PRE_IE_FOUR_OH_HEADER_SIZE;
    psh.dwFlags         = PSH_DEFAULT;//0; //PSH_PROPSHEETPAGE;
    psh.hwndParent 	= this->m_hWnd;
    psh.hInstance	= AfxGetInstanceHandle();
    psh.pszIcon 	= NULL;
    psh.pszCaption 	= (LPSTR) "Modify Database";
    psh.nPages 	        = iCount;
    psh.nStartPage 	= 0;
    psh.phpage	        = pPages;
    psh.pfnCallback     = NULL;
				
    result = PropertySheet(&psh);

    return(result);
}

void
DSRightView::UpdateListFocused(void *pv)
{
    int idx        = -1;
    CListCtrl *pLC = static_cast<CListCtrl *>(pv);

    if ((idx = pLC->GetNextItem(-1, LVNI_FOCUSED)) != -1) {
	LV_ITEM lvi;
	memset(&lvi, 0, sizeof(LV_ITEM));
	IDSDatabase *pDatabase
	    = (IDSDatabase *) ::GetListItemLPARAM(pv, idx);
	char szBuf[256] = {'\0'};

	ASSERT(pDatabase != NULL);
	if (SUCCEEDED(pDatabase->GetName(szBuf, sizeof(szBuf)))) {
	    lvi.mask   |= LVIF_TEXT;
	    lvi.pszText = szBuf;
	    lvi.iItem    = idx;
	    pLC->SetItem(&lvi);
	}
    }
}

void
DSRightView::OnDatabasesItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
    UINT nSel;
    m_PB_ModifyDatabase.EnableWindow(
	(nSel = m_LC_Databases.GetSelectedCount()) == 1);
    m_PB_DeleteDatabase.EnableWindow((nSel >= 1));

    *pResult = 0;
}

void
DSRightView::OnModifyDatabase()
{
    ModifyDatabase();
}

void
DSRightView::OnDeleteDatabase()
{

    if (IDNO == MessageBox("Delete Selected Database(s) (Y/N)?",
			   "Delete Databases(s)",
			   MB_YESNO | MB_ICONQUESTION)) {
	return;
    }

    ASSERT(pDS);
    CRString dsSection(IDS_NDS_DIRSERV_SECTION);
    HRESULT hErrName, hErrProtoName;
    void *pv   = static_cast<void *>(&m_LC_Databases);
    int  count = m_LC_Databases.GetItemCount();

    // Serialize the states, since we are going to do a DeleteAllItems
    // and a PopulateDatabases.
    SerializeDatabasesState();

    // Delete the selected databases.
    int idx = -1;
    while((idx = m_LC_Databases.GetNextItem(idx, LVNI_SELECTED)) != -1) {
	IDSDatabase *pDatabase = (IDSDatabase *) ::GetListItemLPARAM(pv, idx);
	if (pDatabase) {
	    char            szName[128] = {'\0'}, szProtoName[128] = {'\0'},
		            iniStr[258] = {'\0'};
	    hErrName      = pDatabase->GetName(szName, sizeof(szName));
	    hErrProtoName = pDatabase->GetProtocolName(szProtoName,
						       sizeof(szProtoName));
	    if (SUCCEEDED(pDS->pDirServ->DeleteDatabase(pDatabase))) {
		// Blow away the INI entry.
		if (SUCCEEDED(hErrName) && SUCCEEDED(hErrProtoName)) {
		    strcpy(iniStr, szProtoName);
		    strcat(iniStr, ":");
		    strcat(iniStr, szName);
		    ::WritePrivateProfileString(dsSection, iniStr, NULL,
						INIPath);
		}
		pDatabase->Release();
	    }
	}
    }

    // Blow away all the items.
    m_LC_Databases.DeleteAllItems();
    // Repopulate the list.
    PopulateDatabases();
}

void
DSRightView::OnNewDatabase()
{
    void *pv            = static_cast<void *>(&m_LC_Protocols);
    IDSProtocol *pProto = (IDSProtocol *) ::GetListFocusedItemLPARAM(pv);

    if (pProto) {
	ASSERT(pDS);
	HRESULT hErr;
	IDSDatabase *pDatabase = NULL;

	hErr = pDS->pDirServ->CreateDatabase(pProto, &pDatabase);
	if (SUCCEEDED(hErr)) {
	    BOOL result = ConfigDatabase(pDatabase);
	    if (result == TRUE) {
		hErr = pDS->pDirServ->DeleteDatabase(pDatabase);
		pDatabase->Release();
	    }
	    else {
		pDatabase->Release();
		SerializeDatabasesState();
		PopulateDatabases();
	    }
	}
    }	    
}

void
DSRightView::OnProtocolsDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
    OnNewDatabase();
    *pResult = 0;
}

void
DSRightView::Error(char *s)
{
    MessageBox(s, "Error", MB_OK);
}

void
DSRightView::OnSize(UINT nType, int cx, int cy)
{
    C3DFormView::OnSize(nType, cx, cy);

    // Don't resize if the controls aren't created yet, or the window
    // is being minimized.

    if ((m_LC_Protocols.m_hWnd == NULL) || (nType == SIZE_MINIMIZED)) {
	return;
    }

    bool _x, _y;
    _x = (cx <= minDialogWidth);
    _y = (cy <= minDialogHeight);

    SmartSize(cx, _x, cy, _y);
	
    if (::ShouldUpdateIni() && nType != SIZE_MINIMIZED) {
	nCurWidth = cx;
	SerializeSerializables();
    }
}

////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void
DSRightView::OnDestroy()
{
	// Shareware: Unregister ourselves from notifications
	QCSharewareManager *pSWM = QCSharewareManager::GetSharewareManager();
	ASSERT(pSWM);
	if (pSWM)
	{
		pSWM->UnRegister(this);
	}

    PrepareForDestruction();
    C3DFormView::OnDestroy();
}

void
DSRightView::SmartSize(int cx, bool xIsMin, int cy, bool yIsMin)
{
    // Size the top group box.
    int n;
    CRect rectGBProtocols;
    // Ok, these are easy.
    rectGBProtocols.TopLeft().x     = xSpace;
    rectGBProtocols.TopLeft().y     = ySpace;
    // Slightly involved...
    rectGBProtocols.BottomRight().x = ((xIsMin) ? 
	                               minDialogWidth : cx) - xSpace;
    // Even more...
    n = static_cast<int>(0.3 * ((cy - (7 * ySpace) - (2 * tcSpace) -
	                 (2 * pushButtonHeight))));
    n = (n < 1) ? 0 : n;
    rectGBProtocols.BottomRight().y = (yIsMin) ?
	                              (ySpace + minProtocolsHeight) : 
	                              (3 * ySpace) + tcSpace +
	                              pushButtonHeight + n;
    GetDlgItem(IDD_NDS_GB_PROTOCOLS)->MoveWindow(rectGBProtocols);

    // Size the controls in the top group-box.
    // Size the NewDatabase button.
    int offSet = (rectGBProtocols.Width() - pushButtonWidthEx) / 2;
    CRect rectPBNewDatabase;
    rectPBNewDatabase.TopLeft().x     = rectGBProtocols.TopLeft().x + offSet;
    rectPBNewDatabase.TopLeft().y     = rectGBProtocols.BottomRight().y -
	                                ySpace - pushButtonHeight;
    rectPBNewDatabase.BottomRight().x = rectPBNewDatabase.TopLeft().x +
	                                pushButtonWidthEx;
    rectPBNewDatabase.BottomRight().y = rectGBProtocols.BottomRight().y -
	                                ySpace;
    m_PB_NewDatabase.MoveWindow(rectPBNewDatabase);

    // Size the List Control.
    CRect rectLCProtocols;
    rectLCProtocols.TopLeft().x     = rectGBProtocols.TopLeft().x + xSpace;
    rectLCProtocols.TopLeft().y     = rectGBProtocols.TopLeft().y + tcSpace;
    rectLCProtocols.BottomRight().x = rectGBProtocols.BottomRight().x - xSpace;
    rectLCProtocols.BottomRight().y = rectPBNewDatabase.TopLeft().y - ySpace;
    m_LC_Protocols.MoveWindow(rectLCProtocols);

    // Size the Bottom group box.
    // Size the top group box.
    CRect rectGBDatabases;
    rectGBDatabases.TopLeft().x     = xSpace;
    rectGBDatabases.TopLeft().y     = rectGBProtocols.BottomRight().y + ySpace;
    rectGBDatabases.BottomRight().x = rectGBProtocols.BottomRight().x;
    rectGBDatabases.BottomRight().y = (yIsMin) ?
                                      (rectGBDatabases.TopLeft().y +
				       minDatabasesHeight) :
                                       rectGBDatabases.TopLeft().y +
				       cy - rectGBProtocols.Height() - 
				       (3 * ySpace);
    GetDlgItem(IDD_NDS_GB_DATABASES)->MoveWindow(rectGBDatabases);
    ASSERT(rectGBDatabases.Width() >= minDatabasesWidth);

    // Size the controls in the bottom group box.
    
    int sep = rectGBDatabases.Width() - (2 * pushButtonWidth);
        sep = static_cast<int>((static_cast<double>(sep)) / 3);
        sep = (minDatabasesWidth <= rectGBDatabases.Width()) ?
	      sep : xSpace;

    CRect rectPB;
    rectPB.TopLeft().x     = rectGBDatabases.TopLeft().x + sep;
    rectPB.TopLeft().y     = rectGBDatabases.BottomRight().y -
	                     ySpace - pushButtonHeight;
    rectPB.BottomRight().x = rectPB.TopLeft().x + pushButtonWidth;
    rectPB.BottomRight().y = rectGBDatabases.BottomRight().y - ySpace;
    m_PB_ModifyDatabase.MoveWindow(rectPB);

    rectPB.TopLeft().x     = rectPB.BottomRight().x + sep;
    rectPB.BottomRight().x = rectPB.TopLeft().x + pushButtonWidth;
    m_PB_DeleteDatabase.MoveWindow(rectPB);

    CRect rectLCDatabases;
    rectLCDatabases.TopLeft().x = rectGBDatabases.TopLeft().x + xSpace;
    rectLCDatabases.TopLeft().y = rectGBDatabases.TopLeft().y + tcSpace;
    rectLCDatabases.BottomRight().x = rectGBDatabases.BottomRight().x - 
	                              xSpace;
    rectLCDatabases.BottomRight().y = rectPB.TopLeft().y - ySpace;
    m_LC_Databases.MoveWindow(rectLCDatabases);
}

BOOL
DSRightView::IsAnyDatabaseChecked()
{
    UINT nSt = 0;
    int nItemCnt = m_LC_Databases.GetItemCount();
    for (int cnt = 0; cnt < nItemCnt; cnt++) {
	nSt = m_LC_Databases.GetItemState(cnt, LVIS_STATEIMAGEMASK);
	nSt >>= 12;
	nSt--;
	if (nSt) {
	    break;
	}
	else {
	    nSt = 0;
	}
    }

    return((nSt == 0) ? FALSE : TRUE);
}

HRESULT
DSRightView::AddDatabases()
{
    ASSERT(pDS);

    int iCount = m_LC_Databases.GetItemCount();
    int nSelected = 0;
    HRESULT hErr;
    void   *pv = static_cast<void *>(&m_LC_Databases);

    for (int i = 0; i < iCount; i++) 
	{
		UINT nState = m_LC_Databases.GetItemState(i, LVIS_STATEIMAGEMASK);
		nState >>= 12;
		nState--;
		if (nState) 
		{
			ASSERT(pDS->pQuery);
			nSelected++;

			// Shareware: In reduced feature mode, you can only select one database
			QCSharewareManager *pSWM = QCSharewareManager::GetSharewareManager();
			ASSERT(pSWM);
			if ((pSWM) && (pSWM->UsingFullFeatureSet() == true))
			{
				// FULL-FEATURE MODE
				hErr = pDS->pQuery->AddDatabase((IDSDatabase *)
								(::GetListItemLPARAM(pv, i)));
				if (FAILED(hErr))
					return(hErr);
			}
			else
			{
				// REDUCED-FEATURE MODE
				// Allow only one database; Unselected any other selected databases
				if(nSelected == 1)
				{
					hErr = pDS->pQuery->AddDatabase((IDSDatabase *)
									(::GetListItemLPARAM(pv, i)));
					if (FAILED(hErr)) 
						return(hErr);
				}
				else
				{
					m_LC_Databases.SetItemState(i, INDEXTOSTATEIMAGEMASK(1), 
							LVIS_STATEIMAGEMASK);	//unselect
				}
			}
		}
    }

    if (!nSelected) {
	return(E_FAIL);
    }

    return(S_OK);
}


void
DSRightView::OnColumnClickDatabases(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *nLV = (NM_LISTVIEW *) pNMHDR;
    _DS_RV_Composite dscrv;

    shiftDown     = ShiftDown();
    dscrv.pDSRV   = this;
    dscrv.subItem   = nLV->iSubItem;
    m_LC_Databases.SortItems(CompareFuncDatabases, (DWORD)(&dscrv));
   *pResult = 0;
}

void
DSRightView::OnColumnClickProtocols(NMHDR *pNMHDR, LRESULT *pResult)
{
    NM_LISTVIEW *nLV = (NM_LISTVIEW *) pNMHDR;
    _DS_RV_Composite dscrv;

    shiftDown = ShiftDown();
    dscrv.pDSRV  = this;
    dscrv.subItem = nLV->iSubItem;
    m_LC_Protocols.SortItems(CompareFuncProtocols, (DWORD)(&dscrv));
    *pResult = 0;
}

int CALLBACK
DSRightView::CompareFuncDatabases(LPARAM one, LPARAM two, LPARAM data)
{
    int retVal = 0;
    char szBuf1[128], szBuf2[128];
    HRESULT hErr1, hErr2;

    IDSDatabase *pDB1 = (IDSDatabase *) one;
    IDSDatabase *pDB2 = (IDSDatabase *) two;
    _DS_RV_Composite *pDSCRV = (_DS_RV_Composite *) data;
    int column = pDSCRV->subItem;

    if (column == 0) { // comparison by name.
	hErr1 = pDB1->GetName(szBuf1, sizeof(szBuf1));
	hErr2 = pDB2->GetName(szBuf2, sizeof(szBuf2));

	if (SUCCEEDED(hErr1) && SUCCEEDED(hErr2)) { // could get both names.
	    if (szBuf1 && szBuf2) {
		retVal = strcmp(szBuf1, szBuf2);
		if (retVal == 0) { // do an additional comparison by protoname.
		    hErr1 = pDB1->GetProtocolName(szBuf1, sizeof(szBuf1));
		    hErr2 = pDB2->GetProtocolName(szBuf2, sizeof(szBuf2));
		    if (SUCCEEDED(hErr1) && SUCCEEDED(hErr2)) { 
			if (szBuf1 && szBuf2) {
			    retVal = strcmp(szBuf1, szBuf2);
			}
		    }
		}
	    }
	}
    }
    else { // comparison by protocol name.
	hErr1 = pDB1->GetProtocolName(szBuf1, sizeof(szBuf1));
	hErr2 = pDB2->GetProtocolName(szBuf2, sizeof(szBuf2));
	if (SUCCEEDED(hErr1) && SUCCEEDED(hErr2)) { // could get both protos.
	    if (szBuf1 && szBuf2) {
		retVal = strcmp(szBuf1, szBuf2);
		if (retVal == 0) { // do an additional comparison by name.
		    hErr1 = pDB1->GetName(szBuf1, sizeof(szBuf1));
		    hErr2 = pDB2->GetName(szBuf2, sizeof(szBuf2));
		    if (SUCCEEDED(hErr1) && SUCCEEDED(hErr2)) { 
			if (szBuf1 && szBuf2) {
			    retVal = strcmp(szBuf1, szBuf2);
			}
		    }
		}
	    }
	}
    }

    if (pDSCRV->pDSRV->shiftDown == TRUE) {
	if (retVal > 0) {
	    retVal = -1;
	}
	else if (retVal < 0) {
	    retVal = 1;
	}
    }

    return(retVal);
}

int CALLBACK
DSRightView::CompareFuncProtocols(LPARAM one, LPARAM two, LPARAM data)
{
    int retVal = 0;
    char szBuf1[128], szBuf2[128];
    HRESULT hErr1, hErr2;

    IDSProtocol *pPR1 = (IDSProtocol *) one;
    IDSProtocol *pPR2 = (IDSProtocol *) two;

    _DS_RV_Composite *pDSCRV = (_DS_RV_Composite *) data;
    int column = pDSCRV->subItem;

    hErr1 = pPR1->GetProtocolName(szBuf1, sizeof(szBuf1));
    hErr2 = pPR2->GetProtocolName(szBuf2, sizeof(szBuf2));

    if (SUCCEEDED(hErr1) && SUCCEEDED(hErr2)) { // could get both names.
	if (szBuf1 && szBuf2) {
	    retVal = strcmp(szBuf1, szBuf2);
	}
    }

    if (pDSCRV->pDSRV->shiftDown == TRUE) {
	if (retVal > 0) {
	    retVal = -1;
	}
	else if (retVal < 0) {
	    retVal = 1;
	}
    }

    return(retVal);
}

bool
DSRightView::OnlyPhSelected()
{
    ASSERT(pDS);

    bool    onlyPhSelected = true;
    bool    selected       = false;
    int     iCount = m_LC_Databases.GetItemCount();
    HRESULT hErr;
    void   *pv = static_cast<void *>(&m_LC_Databases);

    for (int i = 0; i < iCount; i++) {
	UINT nState = m_LC_Databases.GetItemState(i, LVIS_STATEIMAGEMASK);
	nState >>= 12;
	nState--;
	if (nState) {
	    char szBuf[128];
	    selected = true;
	    IDSDatabase *pDatabase = (IDSDatabase *)
		(::GetListItemLPARAM(pv, i));
	    hErr = pDatabase->GetProtocolName(szBuf, 128);
	    
	    int cmp = strcmp(szBuf, "Ph");

	    if (cmp) {
		onlyPhSelected = false;
		break;
	    }
	}
    }

    if (selected && onlyPhSelected) {
	return(true);
    }
    else {
	return(false);
    }
}

LRESULT
DSRightView::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_GETDLGCODE:
	return(DLGC_WANTALLKEYS);
    case WM_CHAR:
	if (wParam == VK_TAB) {
	    ProcessTab();
	    return(0);
	}
	break;
    case WM_SYSCHAR:
	if (lParam & 0x20000000) {
	    if (IsThisALeftViewShortcutKey(wParam)) {
		::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
		return(0);
	    }
	    else if (IsThisARightViewShortcutKey(wParam)) {
		if (ProcessShortcut(wParam) == true) {
		    return(0);
		}
	    }
	}
	break;
    default:
	break;
    }

    return(C3DFormView::WindowProc(wMessage, wParam, lParam));
} 

void
DSRightView::ProcessTab()
{
    HWND hWnd = GetFocus()->m_hWnd;
    if (hWnd == m_LC_Protocols.m_hWnd) {
	if (ShiftDown()) {
	    m_PB_DeleteDatabase.SetFocus();
	}
	else {
	    m_PB_NewDatabase.SetFocus();
	}
    }
    else if (hWnd == m_PB_NewDatabase.m_hWnd) {
	if (ShiftDown()) {
	    m_LC_Protocols.SetFocus();
	}
	else {
	    m_LC_Databases.SetFocus();
	}
    }
    else if (hWnd == m_LC_Databases.m_hWnd) {
	if (ShiftDown()) {
	    m_PB_NewDatabase.SetFocus();
	}
	else {
	    if (m_PB_ModifyDatabase.IsWindowEnabled() == TRUE) {
		m_PB_ModifyDatabase.SetFocus();
	    }
	    else {
		m_PB_DeleteDatabase.SetFocus();
	    }
	}
    }
    else if (hWnd ==  m_PB_ModifyDatabase.m_hWnd) {
	if (ShiftDown()) {
	    m_LC_Databases.SetFocus();
	}
	else {
	    m_PB_DeleteDatabase.SetFocus();
	}
    }
    else if (hWnd ==  m_PB_DeleteDatabase.m_hWnd) {
	if (ShiftDown()) {
	    if (m_PB_ModifyDatabase.IsWindowEnabled() == TRUE) {
		m_PB_ModifyDatabase.SetFocus();
	    }
	    else {
		m_LC_Databases.SetFocus();
	    }
	}
	else {
	    m_LC_Protocols.SetFocus();
	}
    }
}

bool
DSRightView::ProcessShortcut(WPARAM wParam)
{
    bool retVal = false;

    switch((BYTE) wParam) {
    case 'n': // New Database button.
	m_PB_NewDatabase.SetFocus();
	m_PB_NewDatabase.SendMessage(WM_LBUTTONDOWN,
				     (WPARAM)MK_LBUTTON,
				     MAKELPARAM(0, 0));
	m_PB_NewDatabase.SendMessage(WM_LBUTTONUP, (WPARAM)0,
				     MAKELPARAM(0, 0));
	retVal = true;
	break;
    case 'm': // Modify Database button.
	if (m_PB_ModifyDatabase.IsWindowEnabled() == TRUE) {
	    m_PB_ModifyDatabase.SetFocus();
	    m_PB_ModifyDatabase.SendMessage(WM_LBUTTONDOWN,
					    (WPARAM)MK_LBUTTON,
					    MAKELPARAM(0, 0));
	    m_PB_ModifyDatabase.SendMessage(WM_LBUTTONUP, (WPARAM)0,
					    MAKELPARAM(0, 0));
	    retVal = true;
	}
	break;
    case 'd': // Delete Database button
	m_PB_DeleteDatabase.SetFocus();
	m_PB_DeleteDatabase.SendMessage(WM_LBUTTONDOWN,
					(WPARAM)MK_LBUTTON,
					MAKELPARAM(0, 0));
	m_PB_DeleteDatabase.SendMessage(WM_LBUTTONUP, (WPARAM)0,
					MAKELPARAM(0, 0));
	retVal = true;
	break;
    default:
	break;
    }
    
    return(retVal);
}

bool
ShouldUpdateIni(int code)
{
    static bool bActive = false;

    switch(code) {
    case 0:
	bActive = false;
	break;
    case 1:
	bActive = true;
	break;
    default:
	break;
    }
    
    return bActive;
}

void
DSRightView::Notify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData)
{
	if (theAction == CA_SWM_CHANGE_FEATURE) 
	{

		// Shareware: In reduced feature mode, you can only select one database
		QCSharewareManager *pSWM = QCSharewareManager::GetSharewareManager();
		ASSERT(pSWM);
		if ((pSWM) && (pSWM->UsingFullFeatureSet() == false))
		{
			// So the user has just changed from FULL to REDUCED
			// Make sure only one DB is selected

			int iCount = m_LC_Databases.GetItemCount();
			bool bPrevSel = false;

			for (int i = 0; i < iCount; i++) 
			{
				UINT nState = m_LC_Databases.GetItemState(i, LVIS_STATEIMAGEMASK);
				nState >>= 12;
				nState--;
				if (nState) // If nState is > 0, then this item is selected
				{
					if (bPrevSel)
					{
						// This is the second or higher selcted, so unselect it
						m_LC_Databases.SetItemState(i, INDEXTOSTATEIMAGEMASK(1), 
								LVIS_STATEIMAGEMASK);	//unselect
					}
					else
					{
						bPrevSel = true;
					}
				}
			}
		}
	}
}
