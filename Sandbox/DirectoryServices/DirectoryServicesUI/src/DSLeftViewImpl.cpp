///////////////////////////////////////////////////////////////////////////////
// DSLeftViewImpl.cpp
// 
// 
// Created: 10/09/97 smohanty
//
// Another test bogus line in the main codeline.
// Hey there.
// One.
// Two.
// Three.
// Four.
// Five.
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_LEFT_VIEW_INTERFACE_IMPLEMENTATION_
#include "DSLeftViewImpl.h"


IMPLEMENT_DYNCREATE(DSLeftView, C3DFormView)

BEGIN_MESSAGE_MAP(DSLeftView, C3DFormView)
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_EN_CHANGE(IDD_NDS_EB_QUERY_INPUT, OnQueryInputChange)
    ON_BN_CLICKED(IDD_NDS_PB_QUERY_STARTSTOP, OnQueryStartStop)
    ON_BN_CLICKED(IDD_NDS_PB_MAIL_TO, OnMailTo)
    ON_BN_CLICKED(IDD_NDS_PB_MAIL_CC, OnMailCc)
    ON_BN_CLICKED(IDD_NDS_PB_MAIL_BCC, OnMailBcc)
    ON_BN_CLICKED(IDC_QUERY_DROPDOWN, OnQueryDropDown)
END_MESSAGE_MAP()

DSLeftView::DSLeftView()
    : C3DFormView(IDD_NDS_DIRSERV_LEFTVIEW), imgCache(NULL),
      pDS(NULL),
      XBaseUnit(LOWORD(::GetDialogBaseUnits())),
      YBaseUnit(HIWORD(::GetDialogBaseUnits())), theHits(0),
      queryState(_STOPPED), resultsSWPanesCreated(false),
      nCurWidth(INT_MIN)
{
    int halfYSpace;

    // Control metrics.
    // Horizontal and vertical space.
    xSpace                 = (XBaseUnit);
    ySpace                 = (YBaseUnit)      / 2;
    halfYSpace             = (YBaseUnit)      / 4;
    pushButtonWidth        = (30 * XBaseUnit) / 4;
    pushButtonHeight       = (14 * YBaseUnit) / 8 - ySpace;
    textHeight             = (14 * YBaseUnit) / 8 - ySpace - (ySpace / 2);
    queryLabelWidth        = (14 * XBaseUnit) / 4;
    keepOnTopWidth         = (40 * XBaseUnit) / 4;
    keepOnTopHeight        = (10 * YBaseUnit) / 8;
    minQueryRowWidth       = (4 * xSpace) + queryLabelWidth +
                             (2 * pushButtonWidth);
    
    // 3 pushbuttons, 1 check-box and the 5 spaces.
    minWidth               = (5 * xSpace) + (3 * pushButtonWidth) +
                             keepOnTopWidth;
    // Query-input row, results row, and the mail row.
    minHeight              = static_cast<int>(((3 * ySpace) + halfYSpace) +
                                              (4 * pushButtonHeight) +
                                              textHeight);
}

BOOL
DSLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
    return(C3DFormView::PreCreateWindow(cs));
}

DSLeftView::~DSLeftView()
{
}

BOOL
DSLeftView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
                   DWORD dwRequestedStyle, const RECT& rect,
                   CWnd *pParentWnd, UINT nID,
                   CCreateContext *pContext)
{
    BOOL retVal = C3DFormView::Create(lpszClassName, lpszWindowName,
                                      dwRequestedStyle, rect, pParentWnd, nID,
                                      pContext);

    if (retVal == TRUE) {
        retVal = OnCreateClient();
    }

    return(retVal);
        
}

BOOL
DSLeftView::SerializeSerializables()
{
    BOOL        bWritten   = FALSE;

    // Write the widht of the right pane.
    CRString    dsSect(IDS_NDS_DIRSERV_SECTION),
                dsLeftPaneX(IDS_NDS_DIRSERV_LEFTPANE_X);

    char        buffer[32] = { '\0' };
    const char *section    = dsSect;
    const char *leftPaneX  = dsLeftPaneX;
    const char *itoaBuf    = ::itoa(nCurWidth, buffer, 10);
    const char *iniPath    = INIPath;

    if (section && leftPaneX && itoaBuf && iniPath) {
        bWritten = ::WritePrivateProfileString(section, leftPaneX, itoaBuf,
                                               iniPath);
    }
    
    return bWritten;
}

void
DSLeftView::PrepareForDestruction()
{
    CRString dsSection(IDS_NDS_DIRSERV_SECTION);
    CRString dsKOT(IDS_NDS_DIRSERV_KOT);

    char buffer[8] = { '\0' };

    ::WritePrivateProfileString(dsSection, dsKOT,
                                itoa((m_CB_KeepOnTop.GetState() & 0x0003),
                                     buffer, 10), INIPath);
	m_EB_QueryInput.DestroyListElements();
}

BOOL
DSLeftView::OnCreateClient()
{
    BOOL           fRet;
    UINT           topPaneY = 0, bottomPaneY = 0, panesX = 0;
    CRString       dsSection(IDS_NDS_DIRSERV_SECTION);
    CCreateContext crCtxt;

    fRet = m_SW_Results.CreateStatic(this, 2, 1, WS_CHILD | WS_VISIBLE);

    // We need to get the position of the splitter bar.

    if (fRet) {
        topPaneY  = ::GetPrivateProfileInt(dsSection, "TopPaneY", 150,
                                           INIPath);
        panesX = ::GetPrivateProfileInt(dsSection, "PanesX",
//                                      minWidth - (2 * xSpace), INIPath);
                                        minWidth, INIPath);
        fRet = m_SW_Results.CreateView(0, 0, RUNTIME_CLASS(DSListCtrl),
                                       CSize(panesX, topPaneY), &crCtxt);
    }

    if (fRet) {
        bottomPaneY  = ::GetPrivateProfileInt(dsSection, "BottomPaneY", 0,
                                              INIPath);
        fRet = m_SW_Results.CreateView(1, 0, RUNTIME_CLASS(DSRECtrl),
                                       CSize(panesX, bottomPaneY), &crCtxt);
    }

    if (fRet) {
        resultsSWPanesCreated = true;
        dsbm.pv    = static_cast<void *>(this);
        dsbm.rlr   = DSResetLongResults;
        dsbm.aslr  = DSAddStringLongResults;
        dsbm.aslrf = DSAddStringLongResultsFormatted;
        dsbm.asi   = DSAppriseItemChanged;
        dsbm.htfrt = DSHandleTabFromResultsToc;
        dsbm.qpqip = DSQueryParentQueryInProgress;
        dsbm.asnnl = DSAddStringNoNewLine;
	dsbm.grl   = DSGetResultsLong;

        (static_cast<DSListCtrl *>(m_SW_Results.GetPane(0,0)))->
            InitResultsTOC(&dsbm, 0 /*imgCache = 0*/);

        (static_cast<DSRECtrl *>(m_SW_Results.GetPane(1,0)))->
            InitializeHandleTab(DSHandleTabFromResultsLong,
                                static_cast<void *>(this));

    }

    return fRet;
}

void
DSLeftView::DoDataExchange(CDataExchange *pDX)
{
    C3DFormView::DoDataExchange(pDX);

    // Query input.
    DDX_Control(pDX, IDD_NDS_ST_QUERY_LABEL, m_ST_QueryLabel);
    DDX_Control(pDX, IDD_NDS_PB_QUERY_STARTSTOP, m_PB_QueryStartStop);
    DDX_Control(pDX, IDD_NDS_ST_QUERY_RESULTS_LABEL, m_ST_QueryResults);
    DDX_Control(pDX, IDD_NDS_PB_MAIL_TO, m_PB_MailTo);
    DDX_Control(pDX, IDD_NDS_PB_MAIL_CC, m_PB_MailCc);
    DDX_Control(pDX, IDD_NDS_PB_MAIL_BCC, m_PB_MailBcc);
    DDX_Control(pDX, IDD_NDS_CB_KEEPONTOP, m_CB_KeepOnTop);
}

void
DSLeftView::OnInitialUpdate()
{
    C3DFormView::OnInitialUpdate();
    CRString dsSection(IDS_NDS_DIRSERV_SECTION);
    CRString dsKOT(IDS_NDS_DIRSERV_KOT);
    CRString dsQueryInput(IDS_NDS_DIRSERV_QUERYSTRING);

	m_EB_QueryInput.SubclassDlgItem(IDD_NDS_EB_QUERY_INPUT, this);
    m_EB_QueryInput.SetKeyCB(static_cast<void *>(this), DSKeyCB,
                             DSActivateQueryInput);
	m_EB_QueryInput.InitListElements();

    m_ST_QueryResults.SetWindowText("Status: Ready");
    m_EB_QueryInput.EnableWindow(TRUE);
    m_EB_QueryInput.SetFocus();
	m_EB_QueryInput.AssocDropDownBtn(&m_QueryBtn);
	m_QueryBtn.SubclassDlgItem(IDC_QUERY_DROPDOWN, this);
	m_QueryBtn.SetAssocField(&m_EB_QueryInput);

    m_PB_QueryStartStop.EnableWindow(FALSE);
    m_PB_MailTo.EnableWindow(FALSE);
    m_PB_MailCc.EnableWindow(FALSE);
    m_PB_MailBcc.EnableWindow(FALSE);

    if (::GetPrivateProfileInt(dsSection, "OldKeepOnTopConverted", 0,
                               INIPath)) {
        m_CB_KeepOnTop.SetCheck(::GetPrivateProfileInt((dsSection), dsKOT, 0,
                                                       INIPath));
    }
    else {
        m_CB_KeepOnTop.SetCheck(::GetPrivateProfileInt("Settings",
                                                       "AddressKeepForeground",
                                                       0, INIPath));
        ::WritePrivateProfileString(dsSection, "OldKeepOnTopConverted",
                                    "1", INIPath);
    }


    SetScaleToFitSize(CSize(1, 1));
    RECT rect;
    GetClientRect(&rect);
    OnSize(SIZE_RESTORED, rect.right - rect.left, rect.bottom - rect.top);
    CWazooWnd::SaveInitialChildFocus(&m_EB_QueryInput);
}

void
DSLeftView::GetQueryInput(CString& str)
{
    m_EB_QueryInput.GetWindowText(str);
}

void
DSLeftView::EnableStartStop(BOOL bEnable)
{
    m_PB_QueryStartStop.EnableWindow(bEnable);
}

void
DSLeftView::OnSize(UINT nType, int cx, int cy)
{
    C3DFormView::OnSize(nType, cx, cy);

    // Don't resize if the controls aren't created yet, or the window
    // is being minimized.

    if ((m_EB_QueryInput.m_hWnd == NULL) || (nType == SIZE_MINIMIZED)) {
        return;
    }

    bool _x, _y;
    _x = (cx <= minWidth);
    _y = (cy <= minHeight);

    SmartSize(cx, _x, cy, _y);
    
    if (::ShouldUpdateIni() && (nType != SIZE_MINIMIZED)) {
        nCurWidth = cx;
        SerializeSerializables();
    }
}

////////////////////////////////////////////////////////////////////////
// OnDestroy [protected]
//
////////////////////////////////////////////////////////////////////////
void
DSLeftView::OnDestroy()
{
    PrepareForDestruction();
    C3DFormView::OnDestroy();
}

void
DSLeftView::SmartSize(int cx, bool xIsMin, int cy, bool yIsMin)
{
    // Size the top row.

    // Size the query label.
    CRect rectQueryLabel;
    rectQueryLabel.TopLeft().x     = xSpace;
    rectQueryLabel.TopLeft().y     = ySpace;
    rectQueryLabel.BottomRight().y = ySpace + pushButtonHeight;
    rectQueryLabel.BottomRight().x = xSpace + queryLabelWidth;
    m_ST_QueryLabel.MoveWindow(rectQueryLabel);

    // Size query input
    CRect rectQueryInput;
    rectQueryInput.TopLeft().x     = xSpace + rectQueryLabel.BottomRight().x;
    rectQueryInput.TopLeft().y     = ySpace;
    rectQueryInput.BottomRight().x = ((cx > minQueryRowWidth) ?
                                     (cx - (2 * xSpace) - pushButtonWidth) :
                                     rectQueryInput.TopLeft().x +
                                     pushButtonWidth) - 20;
    rectQueryInput.BottomRight().y = ySpace + pushButtonHeight;
    m_EB_QueryInput.MoveWindow(rectQueryInput);

	int QueryDropDownBtnwidth = rectQueryInput.bottom - rectQueryInput.top;
	int ThreeDBorderwidth = GetSystemMetrics(SM_CXEDGE);

	m_QueryBtn.SetWindowPos(&m_EB_QueryInput, rectQueryInput.right - ThreeDBorderwidth,
		rectQueryInput.top, QueryDropDownBtnwidth, QueryDropDownBtnwidth, SWP_NOOWNERZORDER);

    // Size the start/stop button.
    CRect rectStartStop;
    rectStartStop.TopLeft().x      = (cx > minQueryRowWidth) ?
                                     cx - xSpace - pushButtonWidth :
                                     rectQueryInput.BottomRight().x + xSpace;
    rectStartStop.TopLeft().y      = ySpace;
    rectStartStop.BottomRight().x  = (cx > minQueryRowWidth) ?
                                     cx - xSpace :
                                     rectStartStop.TopLeft().x +
                                     pushButtonWidth;
    rectStartStop.BottomRight().y  = ySpace + pushButtonHeight;
    m_PB_QueryStartStop.MoveWindow(rectStartStop);

    // Size the Results Count Label.
    CRect rectResultsLabel;
    rectResultsLabel.TopLeft().x     = xSpace;
    rectResultsLabel.TopLeft().y     = 
        static_cast<int>(ySpace + (ySpace / 2) + pushButtonHeight);
    rectResultsLabel.BottomRight().x = (cx > minWidth) ? 
                                       (cx - xSpace) : (minWidth - xSpace);
    rectResultsLabel.BottomRight().y = rectResultsLabel.TopLeft().y +
                                       textHeight;
    m_ST_QueryResults.MoveWindow(rectResultsLabel);

    // Size the Lowest Row.
    int mailComplementWidth = (5 * xSpace) + keepOnTopWidth;
    int mailWidth           = (cx > minWidth) ?
        static_cast<int>((static_cast<double>(cx - mailComplementWidth)) / 3) :
        pushButtonWidth;

    // Size the To button.
    CRect rectMail;
    rectMail.TopLeft().x     = xSpace;
    rectMail.TopLeft().y     = (cy > minHeight) ?
                               (cy - ySpace - pushButtonHeight) : 
                               (minHeight - ySpace - pushButtonHeight);
    rectMail.BottomRight().x = rectMail.TopLeft().x + mailWidth;
    rectMail.BottomRight().y = rectMail.TopLeft().y + pushButtonHeight;
    m_PB_MailTo.MoveWindow(rectMail);

    // Size the Cc button.
    rectMail.TopLeft().x     = rectMail.BottomRight().x + xSpace;
    rectMail.BottomRight().x = rectMail.TopLeft().x + mailWidth;
    m_PB_MailCc.MoveWindow(rectMail);

    // Size the Bcc button.
    rectMail.TopLeft().x     = rectMail.BottomRight().x + xSpace;
    rectMail.BottomRight().x = rectMail.TopLeft().x + mailWidth;
    m_PB_MailBcc.MoveWindow(rectMail);

    // Size the KeepOnTop Check-box.
    rectMail.TopLeft().x     = rectMail.BottomRight().x + xSpace;
    rectMail.BottomRight().x = rectMail.TopLeft().x + keepOnTopWidth;
    m_CB_KeepOnTop.MoveWindow(rectMail);

    // Size the splitter.
    CRect rectSWResults;
    rectSWResults.TopLeft().x     = xSpace;
    rectSWResults.TopLeft().y     = static_cast<int>(ySpace + (ySpace / 2) +
                                                     pushButtonHeight +
                                                     textHeight);

    rectSWResults.BottomRight().x = (cx > minWidth) ?
                                    (cx - xSpace) : (minWidth - xSpace);
    rectSWResults.BottomRight().y = (cy > minHeight) ?
                                    rectMail.TopLeft().y - ySpace :
                                    rectSWResults.TopLeft().y + 
                                    (2 * pushButtonHeight);

    m_SW_Results.MoveWindow(rectSWResults, TRUE);
}

void
DSLeftView::DisplayHits()
{
    if (theHits == 0) {
        m_ST_QueryResults.SetWindowText("Status: Connecting...");
    }
    else {
        char szHits[64];
        wsprintf(szHits, "Status: Getting Data... Found %ld", theHits);
        m_ST_QueryResults.SetWindowText(szHits);
    }
}

void
DSLeftView::OnMailTo()
{
    OnAddress(HEADER_TO);
}

void
DSLeftView::OnMailCc()
{
    OnAddress(HEADER_CC);
}

void
DSLeftView::OnMailBcc()
{
    OnAddress(HEADER_BCC);
}

void
DSLeftView::OnQueryDropDown()
{
	if (!m_EB_QueryInput.PoppedUpState())
	{
		m_EB_QueryInput.DoQueryAutoComplete(true);
		m_EB_QueryInput.SetPoppedUpState(true);
	}
	else
	{
		m_EB_QueryInput.KillACList();
		m_EB_QueryInput.SetPoppedUpState(false);
	}
	m_EB_QueryInput.SetFocus();
}


UINT
DSLeftView::GetResultsTocSelectedCount()
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    return(pLC->GetSelectedCount());
}

int
DSLeftView::GetResultsTocItemCount()
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    return(pLC->GetItemCount());
}

UINT
DSLeftView::GetResultsTocItemState(int i, UINT state)
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    return(pLC->GetItemState(i, state));
}

CString
DSLeftView::GetResultsTocItemText(int item, int subitem)
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    return(pLC->GetItemText(item, subitem));
}

bool
DSLeftView::GetResultsTocEmailList(int item, CStringList& curEmailList)
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    return(pLC->GetResultsTocEmailList(item, curEmailList));
}


// This is used for MakeAddressBookEntry.
bool
DSLeftView::GetSelectedAddresses(CStringList& addressList)
{
    bool found      = false;
    int  nItemCount = GetResultsTocItemCount();

    // For each item
    for (int i = 0; i < nItemCount; i++) {
        CString strMailees;
        strMailees.Empty();
        // determine if it is selected
        if (GetResultsTocItemState(i, LVIS_SELECTED) & LVIS_SELECTED) {
            bool        anyEmail;
            CStringList curEmailList;
            anyEmail = GetResultsTocEmailList(i, curEmailList);
            // and it has an email...
            if (anyEmail == true) {
                CString curName;
                curName.Empty();
                curName = GetResultsTocItemText(i, 0);
                int count = 0;
                CString newAddress, emptyName;
                emptyName.Empty();
                while (!curEmailList.IsEmpty()) {
                    newAddress.Empty();
                    CString curEmail = curEmailList.RemoveHead();
                    dsMail.CreateAddress(newAddress, (count == 0)
                                                     ? curName : emptyName,
                                         curEmail);
                    if (strMailees.IsEmpty()) {
                        strMailees = newAddress;
                    }
                    else {
                        strMailees += ",";
                        strMailees += newAddress;
                    }
                    count++;
                }
                if (!strMailees.IsEmpty()) {
                    addressList.AddTail(strMailees);
                    found = true;
                }
            } // item has an email
        } // item is selected
    } // for-loop looping over all items.

    return(found);
}

void
DSLeftView::OnAddress(UINT HeaderNum)
{
    CString strMailees;

    UINT nSelected = GetResultsTocSelectedCount();
    // We should never have come here if nothing is selected.
    ASSERT(nSelected);
    // but if we have due to some insane reason, make sure we can't
    // get here and return.
    if (!nSelected) {
        m_PB_MailTo.EnableWindow(FALSE);
        m_PB_MailCc.EnableWindow(FALSE);
        m_PB_MailBcc.EnableWindow(FALSE);       
        return;
    }

    strMailees.Empty();
    int nItemCount = GetResultsTocItemCount();

    // For each item
    for (int i = 0; i < nItemCount; i++) {
        // determine if it is selected
        if (GetResultsTocItemState(i, LVIS_SELECTED) & LVIS_SELECTED) {
            bool        anyEmail;
            CStringList curEmailList;
            anyEmail = GetResultsTocEmailList(i, curEmailList);
            // and it has an email...
            if (anyEmail == true) {
                CString curName;
                curName.Empty();
                curName = GetResultsTocItemText(i, 0);
                int count = 0;
                CString newAddress, emptyName;
                emptyName.Empty();
                while (!curEmailList.IsEmpty()) {
                    newAddress.Empty();
                    CString curEmail = curEmailList.RemoveHead();
                    dsMail.CreateAddress(newAddress, (count == 0)
                                                     ? curName : emptyName,
                                         curEmail);
                    if (strMailees.IsEmpty()) {
                        strMailees = newAddress;
                    }
                    else {
                        strMailees += ",";
                        strMailees += newAddress;
                    }
                    count++;
                }
            } // item has an email
        } // item is selected
    } // for-loop looping over all items.

    BOOL bKeepOnTop = ((m_CB_KeepOnTop.GetState() & 0x0003) == 1);
    dsMail.OnAddress(HeaderNum, strMailees,
                     GetParent()->GetParent()->GetParent(),
                     GetParentFrame(), bKeepOnTop);
}

void
DSLeftView::OnQueryInputChange()
{
    CString wndStr;
    m_EB_QueryInput.GetWindowText(wndStr);
    int nText = wndStr.GetLength();

    if (!IsQueryInProgress()) {
        if (nText > 0) {
            EnableStartStop(IsAnyDatabaseChecked());
        }
        else {
            m_PB_QueryStartStop.EnableWindow(FALSE);
        } 
    }
}

BOOL
DSLeftView::IsAnyDatabaseChecked()
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *> (GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));
    return(pDSRV->IsAnyDatabaseChecked());
}

void
DSLeftView::OnQueryStartStop()
{
    if (IsQueryInProgress()) {
        m_EB_QueryInput.SetFocus();
        QuitQuery();
    }
    else {
        QueryStart();
    }
}

void
DSLeftView::QueryStart()
{
    m_EB_QueryInput.SetFocus();
    m_EB_QueryInput.SetSel(0, -1);
    StartQuery();
}

void
DSLeftView::QuitQuery()
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>
                                (GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));

    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);

    if (pDS && pDS->pQuery) {
        pDS->pQuery->Cancel();
        pDS->pQuery->Release();
        pDS->pQuery = NULL;
        ResetQueryState(_STOPPED);
    }

    CWazooWnd *pParentWazoo
        = static_cast<CWazooWnd *>(GetParent()->GetParent()->GetParent());
    ASSERT_KINDOF(CWazooWnd, pParentWazoo);

    ::SendMessage(pParentWazoo->m_hWnd, WM_SETCURSOR, (WPARAM) HTCLIENT,
                (LPARAM) WM_MOUSEMOVE);
    ::SendMessage(m_EB_QueryInput.m_hWnd, WM_SETCURSOR, (WPARAM) HTCLIENT,
                (LPARAM) WM_MOUSEMOVE);
}

void
DSLeftView::ResetQueryState(_DS_QUERY_STATE _dsqst)
{
    queryState = _dsqst;

    switch(queryState) {
    case _STARTED:
        theHits = 0;
        DisplayHits();
        m_PB_QueryStartStop.SetWindowText("&Stop");
        break;
    case _STOPPED:
        m_PB_QueryStartStop.SetWindowText("&Start");
        char szHits[128];
        wsprintf(szHits, 
                 "Status: Ready - Last found %ld", theHits);
        m_ST_QueryResults.SetWindowText(szHits);
        break;
    default:
        break;
    }
}

void
DSLeftView::StartQuery()
{
    HRESULT hErr;
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>
                                (GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));
    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);

    if (pDS == NULL)
        return;

    if (pDS->pQuery) {
        if (IDYES == MessageBox("Cancel Current Query (Y/N)?",
                                "Query in Progress",
                                MB_YESNO | MB_ICONQUESTION)) {
            QuitQuery();
        }
        else {
            return;
        }
    }

    ResetResultsToc();

    ASSERT(pDS->pDirServ);
    hErr =  pDS->pDirServ->CreateObject(IID_IDSQuery, (void **)(&pDS->pQuery));

    if (SUCCEEDED(hErr)) {
        char strQuery[512] = {'\0'};

        m_EB_QueryInput.GetWindowText(strQuery, 511);
        lastQuery = strQuery;

        hErr =  pDS->pQuery->Initialize(strQuery,
                                        (pDSRV->OnlyPhSelected())
                                        ? QUERY_FLG_RAWQUERY : QUERY_FLG_NONE,
                                        DS_NAME);

        if (FAILED(hErr)) {
            MessageBox("Failed to Initialize Query", "Query Failed",
                       MB_OK|MB_ICONERROR);
            pDS->pQuery->Release();
            pDS->pQuery = NULL;
            ResetQueryState(_STOPPED);
        }

        hErr = pDSRV->AddDatabases();
        if (SUCCEEDED(hErr)) {
            ClearContents();
            ResetQueryState(_STARTED);
            pDS->pQuery->Start(StartQueryCB, static_cast<LPVOID>(this));
        }
        else {
            MessageBox("Could not add any database", "Query Failed",
                       MB_OK|MB_ICONERROR);
            pDS->pQuery->Release();
            pDS->pQuery = NULL;
            ResetQueryState(_STOPPED);
        }
    }
    else {
        MessageBox("Unable to Create Query Object", "Query Failed",
                   MB_OK|MB_ICONERROR);
        ResetQueryState(_STOPPED);
    }   
}

void
DSLeftView::ReflectQueryInTitle(const char *query)
{
    char titleBuf[56];
    int queryLen = strlen(query), val;
    
    CWazooWnd *pParentWazoo = static_cast<CWazooWnd *>
        (GetParent()->GetParent()->GetParent());
    ASSERT_KINDOF(CWazooWnd, pParentWazoo);

    strcpy(titleBuf, "Directory Services:");
    val = strlen(titleBuf) + queryLen + 1;

    if (val > 56) {
        int len = strlen(titleBuf);
        strncpy(titleBuf + len, query, 56 - len - 3 - 1);
        titleBuf[52] = '\0';
        strcat(titleBuf, "...");
    }
    else {
        strcat(titleBuf, query);
    }
    ::SetWindowText(pParentWazoo->m_hWnd, titleBuf);
}

void
DSLeftView::ResetResultsToc()
{
    (static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0)))->Reset();
}

void
DSLeftView::ClearContents()
{
    (static_cast<DSRECtrl *>(m_SW_Results.GetPane(1,0)))->ClearContents();
}

void
DSLeftView::InitializeExportToBuffer(void *pRecord)
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));

    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);
    ASSERT(pDS->pDirServ);
    IDSPRecord *pRec = (IDSPRecord *)(pRecord);
    pDS->pDirServ->ExportToBuffer((IDSPRecord *)pRecord, ExportToBufferCB,
                                  (void *)(this));
}

void
DSLeftView::InitializeExportToBufferFormatted(void *data)
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));

    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);
    ASSERT(pDS->pDirServ);
    pDS->pDirServ->ExportToBufferFormatted(data, ExportToBufferCB,
                                           (void *)(this));
}

void
DSLeftView::ResultsTocItemChanged(int)
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    UINT nSelected  = pLC->GetSelectedCount();

    if (nSelected > 0) {
        int nEmail =  0;
        int idx    = -1;

		AutoCompList* theList = m_EB_QueryInput.GetACList();
		CString WindowText;
		m_EB_QueryInput.GetWindowText(WindowText);
		char* temp = WindowText.GetBuffer(0);
		theList->Add(temp);
		WindowText.ReleaseBuffer();

        while((idx = pLC->GetNextItem(idx, LVNI_SELECTED)) != -1) {
            char szEmail[256];
            nEmail = pLC->GetItemText(idx, 1, szEmail, 256);
            if (nEmail > 0)
                break;
        }

        if (nEmail > 0) {
            if (m_PB_MailTo.IsWindowEnabled() == FALSE)
                m_PB_MailTo.EnableWindow(TRUE);
            if (m_PB_MailCc.IsWindowEnabled() == FALSE)
                m_PB_MailCc.EnableWindow(TRUE);
            if (m_PB_MailBcc.IsWindowEnabled() == FALSE)
                m_PB_MailBcc.EnableWindow(TRUE);
        }
        else {
            if (m_PB_MailTo.IsWindowEnabled() == TRUE)
                m_PB_MailTo.EnableWindow(FALSE);
            if (m_PB_MailCc.IsWindowEnabled() == TRUE)
                m_PB_MailCc.EnableWindow(FALSE);
            if (m_PB_MailBcc.IsWindowEnabled() == TRUE)
                m_PB_MailBcc.EnableWindow(FALSE);
        }
    }
    else {
        if (m_PB_MailTo.IsWindowEnabled() == TRUE)
            m_PB_MailTo.EnableWindow(FALSE);
        if (m_PB_MailCc.IsWindowEnabled() == TRUE)
            m_PB_MailCc.EnableWindow(FALSE);
        if (m_PB_MailBcc.IsWindowEnabled() == TRUE)
            m_PB_MailBcc.EnableWindow(FALSE);
    }
}

QUERY_STATUS
DSLeftView::GetQueryLastError()
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));

    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);
    ASSERT(pDS->pQuery);
    return(pDS->pQuery->GetLastError());
}

void
DSLeftView::SetDataState(_DS_DATA_STATE _dsDataState)
{
    dataState = _dsDataState;
}

void
DSLeftView::WaitForRecord()
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));
    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);
    ASSERT(pDS->pQuery);
    pDS->pQuery->WaitForRecord(WaitRecordCB, static_cast<LPVOID>(this));
}

void
DSLeftView::GetTheData()
{
    m_ST_QueryResults.SetWindowText("Status: Getting Data...");
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));
    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);
    ASSERT(pDS->pQuery);
    pDS->pQuery->WaitForRecord(WaitRecordCB, static_cast<LPVOID>(this));
}

void
DSLeftView::Error(char *s)
{
    MessageBox(s, "Error", MB_OK);
}

void DSLeftView::Error( QUERY_STATUS nQueryStatus )
{
    char szBuf[64] = "";

    switch( nQueryStatus ) {

    case QUERY_DONE:
    case QUERY_CANCELLED:
        break;

    case QUERY_ECONNECT:
        wsprintf( szBuf,
                  "Could not connect to database. Try again later.");
        break;

    case QUERY_EADDRESS:
        wsprintf( szBuf,
                  "Invalid hostname or file.");
        break;

    case QUERY_EREAD:
    case QUERY_EWRITE:
        wsprintf( szBuf,
                  "Network error. Try again later.");
        break;

    case QUERY_EOUTOFOMEMORY:
        wsprintf( szBuf,
                  "Out of memory. Close some applications and try again.");
        break;

    case QUERY_TIMEDOUT:
        wsprintf( szBuf,
                  "Operation timed out before completing." );
        break;


    default:
        wsprintf( szBuf, "Query failed." );
        break;
    }

    if ( strlen( szBuf ) )
//      ErrorDialog( IDR_DIRECTORY_SERVICES, szBuf );
        MessageBox( szBuf, "Directory Services", MB_OK | MB_ICONEXCLAMATION );
}

QUERY_STATUS
DSLeftView::GetQueryRecord(IDSPRecord **ppRecord)
{
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));
    pDS = pDSRV->GetDSInterface();
    ASSERT(pDS);
    ASSERT(pDS->pQuery);
    return(pDS->pQuery->GetRecord(ppRecord));
}

void
DSLeftView::AddStringNoNewLine(char *s)
{
    DSRECtrl *pRLong = static_cast<DSRECtrl *>(m_SW_Results.GetPane(1, 0));
    pRLong->AddStringNoNewLine(s);
}

void
DSLeftView::AddHit(IDSPRecord *pRecord)
{
    IDSDatabase *pDatabase    = NULL;
    DSMainSplitterWnd *pDSMSW = static_cast<DSMainSplitterWnd *>(GetParent());
    DSRightView       *pDSRV  = static_cast<DSRightView *>
                                (pDSMSW->GetPane(0, 1));
    pDS                       = pDSRV->GetDSInterface();
    ASSERT(pDS);

    char *szDatabaseID     = pRecord->GetDatabaseID();
    if (szDatabaseID &&
        (pDatabase = pDS->pDirServ->FindDatabase(szDatabaseID))) {
        static bool visited = false;
        DSListCtrl *pLC = static_cast<DSListCtrl *>
                          (m_SW_Results.GetPane(0, 0));
        imgCache                  = pDSRV->GetImageCache();
        if (visited == false) {
            visited = true;     
            pLC->InitTOCImageList(imgCache);
        }
        pLC->InsertItem(pDS, pRecord, imgCache, AddThisHitCB, (void *) this);
    }

    if (pDatabase) {
        pDatabase->Release();
    }
}

void
DSLeftView::AddHitCallback()
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));

    theHits++;
    DisplayHits();

    if (pLC->GetItemCount() == 1) {
        pLC->SelectFirstItem();
        ResultsTocItemChanged(0);
    }
}

void
DSLeftView::ProcessTabIntoResultsToc()
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    HWND hWnd = pLC->FetchControlWindow();

    ::SetFocus(hWnd);
    if (pLC->GetItemCount()) {
        if (pLC->GetSelectedCount() == 0) {
            pLC->SelectFirstItem();
        }
        ResultsTocItemChanged(0);
    }
    else {
        m_PB_MailTo.EnableWindow(FALSE);
        m_PB_MailCc.EnableWindow(FALSE);
        m_PB_MailBcc.EnableWindow(FALSE);
    }
}    

void
DSLeftView::ProcessTabFromQueryInput()
{
    if (ShiftDown()) {
        m_CB_KeepOnTop.SetFocus();
    }
    else {
        ProcessTabIntoResultsToc();
    }
}

void
DSLeftView::ProcessTabFromResultsToc(BOOL shiftDown)
{
    if (shiftDown == TRUE) {
        m_EB_QueryInput.SetFocus();
    }
    else {
        HWND hWnd = (static_cast<DSRECtrl *>(m_SW_Results.GetPane(1, 0)))->
                    FetchControlWindow();
        ::SetFocus(hWnd);
    }
}

void
DSLeftView::ProcessTabFromResultsLong(BOOL shiftDown)
{
    if (shiftDown == TRUE) {
        ProcessTabIntoResultsToc();
    }
    else {
        if (m_PB_MailTo.IsWindowEnabled() == TRUE) {
            m_PB_MailTo.SetFocus();
        }
        else {
            m_CB_KeepOnTop.SetFocus();
        }
    }
        
}

bool
DSLeftView::IsQueryInputEmpty()
{
    CString wndStr;
    m_EB_QueryInput.GetWindowText(wndStr);
    int nText = wndStr.GetLength();

    if (nText <=0) {
        return(true);
    }
    else {
        return(false);
    }
}

void
DSLeftView::ActivateQueryInput()
{
    CWazooWnd *pParentWazoo
        = static_cast<CWazooWnd *>(GetParent()->GetParent()->GetParent());
    ASSERT_KINDOF(CWazooWnd, pParentWazoo);
    ::SendMessage(pParentWazoo->m_hWnd, WM_SETCURSOR, (WPARAM) HTCLIENT,
                  (LPARAM) WM_MOUSEMOVE);
}

LRESULT
DSLeftView::WindowProc(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    switch (wMessage) {
    case WM_GETDLGCODE:
        return(DLGC_WANTALLKEYS);
    case WM_CHAR:
        if (wParam == VK_TAB) {
            if (resultsSWPanesCreated == true) {
                return(ProcessTab(wMessage, wParam, lParam));
            }
        }
        break;
    case WM_SYSCHAR:
        if (lParam & 0x20000000) {
            if (IsThisALeftViewShortcutKey(wParam)) {
                if (resultsSWPanesCreated == true) {
                    if (ProcessShortcut(wParam) == true) {
                        return(0);
                    }
                }
            }
            else if (IsThisARightViewShortcutKey(wParam)) {
                ::SendMessage(GetParent()->m_hWnd, wMessage, wParam, lParam);
                return(0);
            }
        }
        break;
    default:
        break;
    }
    return(C3DFormView::WindowProc(wMessage, wParam, lParam));
}

bool
DSLeftView::ProcessShortcut(WPARAM wParam)
{
    bool retVal = false;

    switch((BYTE) wParam) {
    case 'q':
        m_EB_QueryInput.SetFocus();
        retVal = true;
        break;
    case 's':
        if (m_PB_QueryStartStop.IsWindowEnabled() == TRUE) {
            m_PB_QueryStartStop.SetFocus();
            m_PB_QueryStartStop.SendMessage(WM_LBUTTONDOWN,
                                            (WPARAM)MK_LBUTTON,
                                            MAKELPARAM(0, 0));
            m_PB_QueryStartStop.SendMessage(WM_LBUTTONUP, (WPARAM)0,
                                            MAKELPARAM(0, 0));
            retVal = true;
        }
        break;
    case 't':
        if (m_PB_MailTo.IsWindowEnabled() == TRUE) {
            m_PB_MailTo.SetFocus();
            m_PB_MailTo.SendMessage(WM_LBUTTONDOWN,
                                    (WPARAM)MK_LBUTTON,
                                    MAKELPARAM(0, 0));
            m_PB_MailTo.SendMessage(WM_LBUTTONUP, (WPARAM)0,
                                    MAKELPARAM(0, 0));
            retVal = true;
        }
        break;
    case 'c':
        if (m_PB_MailCc.IsWindowEnabled() == TRUE) {
            m_PB_MailCc.SetFocus();
            m_PB_MailCc.SendMessage(WM_LBUTTONDOWN,
                                    (WPARAM)MK_LBUTTON,
                                    MAKELPARAM(0, 0));
            m_PB_MailCc.SendMessage(WM_LBUTTONUP, (WPARAM)0,
                                    MAKELPARAM(0, 0));
            retVal = true;
        }
        break;
    case 'b':
        if (m_PB_MailBcc.IsWindowEnabled() == TRUE) {
            m_PB_MailBcc.SetFocus();
            m_PB_MailBcc.SendMessage(WM_LBUTTONDOWN,
                                     (WPARAM)MK_LBUTTON,
                                     MAKELPARAM(0, 0));
            m_PB_MailBcc.SendMessage(WM_LBUTTONUP, (WPARAM)0,
                                     MAKELPARAM(0, 0));
            retVal = true;
        }
        break;
    case 'k': // We are always enabled!
        m_CB_KeepOnTop.SetFocus();
        m_CB_KeepOnTop.SetCheck(!m_CB_KeepOnTop.GetCheck());
        retVal = true;
    default:
        break;
    }
    return(retVal);
}


LRESULT
DSLeftView::ProcessTab(UINT wMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT retVal       = 0;
    HWND hWnd            = GetFocus()->m_hWnd;
    HWND hWndResultsToc  = (static_cast<DSListCtrl*>
                           (m_SW_Results.GetPane(0, 0)))->FetchControlWindow();
    HWND hWndResultsLong = (static_cast<DSRECtrl *>
                           (m_SW_Results.GetPane(1, 0)))->FetchControlWindow();

    if (hWnd == m_EB_QueryInput.m_hWnd) {
        ProcessTabFromQueryInput();
    }
    else if (hWnd == m_PB_QueryStartStop.m_hWnd) {
        if (ShiftDown()) {
            m_EB_QueryInput.SetFocus();
        }
        else {
            ProcessTabIntoResultsToc();
        }
    }
    else if (hWnd == hWndResultsToc) {
        ProcessTabFromResultsToc(ShiftDown());
    }
    else if (hWnd == hWndResultsLong) {
        ProcessTabFromResultsLong(ShiftDown());
    }
    else if (hWnd == m_PB_MailTo.m_hWnd) {
        if (ShiftDown()) {
            ::SetFocus(hWndResultsLong);
        }
        else {
            m_PB_MailCc.SetFocus();
        }
    }
    else if (hWnd == m_PB_MailCc.m_hWnd) {
        if (ShiftDown()) {
            m_PB_MailTo.SetFocus();
        }
        else {
            m_PB_MailBcc.SetFocus();
        }
    }
    else if (hWnd == m_PB_MailBcc.m_hWnd) {
        if (ShiftDown()) {
            m_PB_MailCc.SetFocus();
        }
        else {
            m_CB_KeepOnTop.SetFocus();
        }
    }
    else if (hWnd == m_CB_KeepOnTop.m_hWnd) {
        if (ShiftDown()) {
            if (m_PB_MailBcc.IsWindowEnabled() == TRUE) {
                m_PB_MailBcc.SetFocus();
            }
            else {
                ::SetFocus(hWndResultsLong);
            }
        }
        else {
            m_EB_QueryInput.SetFocus();
        }
    }
    else {
        return(C3DFormView::WindowProc(wMessage, wParam, lParam));    
    }
    
    return(retVal);
}

void
DSLeftView::GetResultsLong(CString& details)
{
    (static_cast<DSRECtrl *>(m_SW_Results.GetPane(1,0)))->
        GetResultsLong(details);
}

bool
DSLeftView::IsPrintable()
{
    DSListCtrl *pLC = static_cast<DSListCtrl *>(m_SW_Results.GetPane(0, 0));
    return((pLC->GetNextItem(-1, LVNI_SELECTED) != -1));
}

void
StartQueryCB( LPVOID pvContext )
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);

    QUERY_STATUS qs = pDSLV->GetQueryLastError();

    if ( qs == QUERY_OK ) {
        pDSLV->SetDataState( _DATA_BEGIN );
        pDSLV->GetTheData();
    }
    else {
        pDSLV->Error( qs );
        pDSLV->SetDataState( _DATA_END );
        pDSLV->QuitQuery();
    }
}

void
WaitRecordCB( LPVOID pvContext )
{
    IDSPRecord *pRecord = NULL;
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);

    QUERY_STATUS qs = pDSLV->GetQueryRecord( &pRecord );

    if ( qs == QUERY_OK ) {
        pDSLV->AddHit( pRecord );
        pRecord->Release();
        pDSLV->WaitForRecord();
    }
    else {
        pDSLV->Error( qs );
        pDSLV->SetDataState( _DATA_END );
        pDSLV->QuitQuery();
    }
}

void
DSResetLongResults(void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    pDSLV->ClearContents();
}

void
DSAddStringNoNewLine(char *s, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        pDSLV->AddStringNoNewLine(s);
    }
}

void
DSAddStringLongResults(void *pRecord, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    pDSLV->InitializeExportToBuffer(pRecord);
}

void
DSAddStringLongResultsFormatted(void *data, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        pDSLV->InitializeExportToBufferFormatted(data);
    }
}

void
DSAppriseItemChanged(int nItem, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    pDSLV->ResultsTocItemChanged(nItem);
}

void
ExportToBufferCB(char *s, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    pDSLV->AddStringNoNewLine(s);
}

void
DSKeyCB(void *pvContext, WPARAM wParam)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);

    if (wParam == VK_TAB) {
        pDSLV->ProcessTabFromQueryInput();
    }
    else {
        if (pDSLV->IsQueryInProgress()) {
            ::MessageBeep(MB_ICONEXCLAMATION);
        }
        else {
            if ((pDSLV->IsQueryInputEmpty()) ||
                (!pDSLV->IsAnyDatabaseChecked())) {
                ::MessageBeep(MB_ICONEXCLAMATION);
            }
            else {
                pDSLV->QueryStart();
            }
        }
    }
}

void
AddThisHitCB(void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        pDSLV->AddHitCallback();
    }
}

void
DSHandleTabFromResultsToc(BOOL shiftDown, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        pDSLV->ProcessTabFromResultsToc(shiftDown);
    }
}

void
DSHandleTabFromResultsLong(BOOL shiftDown, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        pDSLV->ProcessTabFromResultsLong(shiftDown);
    }
}

void
DSActivateQueryInput(void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        pDSLV->ActivateQueryInput();
    }
}

bool
DSQueryParentQueryInProgress(void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
        return(pDSLV->IsQueryInProgress());
    }
    return(false);
}

void
DSGetResultsLong(CString& str, void *pvContext)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(pvContext);
    if (pDSLV) {
	pDSLV->GetResultsLong(str);
    }
}

