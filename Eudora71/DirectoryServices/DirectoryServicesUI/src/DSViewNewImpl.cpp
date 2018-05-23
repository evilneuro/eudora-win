//////////////////////////////////////////////////////////////////////////////
// DSViewNewImpl.cpp
// 
//
// Created: 10/09/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DIRECTORYSERVICES_VIEWNEW_INTERFACE_IMPLEMENTATION_
#include "DSViewNewImpl.h"

#include "DebugNewHelpers.h"

IMPLEMENT_DYNCREATE(DirectoryServicesViewNew, C3DFormView)

BEGIN_MESSAGE_MAP(DirectoryServicesViewNew, C3DFormView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_SPECIAL_MAKENICKNAME, OnMakeNickname)
    ON_COMMAND(ID_NEXT_PANE, OnNextPane)
    ON_UPDATE_COMMAND_UI(ID_SPECIAL_MAKENICKNAME, OnUpdateMakeNickname)
    ON_REGISTERED_MESSAGE(umsgActivateWazoo, OnActivateWazoo)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateNeedActiveFocus)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateNeedActiveFocus)
    ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrintOne)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

DirectoryServicesViewNew::DirectoryServicesViewNew()
    : C3DFormView(IDD_NDS_DIRSERV),
      XBaseUnit(LOWORD(::GetDialogBaseUnits())),
      YBaseUnit(HIWORD(::GetDialogBaseUnits())), m_PrintPreView(NULL),
      panesCreated(false), printing_or_previewing(false)
{
    // Control metrics.
    // Horizontal and vertical space.
    xSpace                 = (XBaseUnit);
    ySpace                 = (YBaseUnit)      / 2;
}

DirectoryServicesViewNew::~DirectoryServicesViewNew()
{
}

BOOL
DirectoryServicesViewNew::PreTranslateMessage(MSG *pMsg)
{
    BOOL retVal = FALSE;
    ASSERT_VALID(this);

    switch(pMsg->message) {
    case WM_SYSCHAR:
	// A shortcut key.
	if (pMsg->lParam & 0x20000000) {
	    // Do we want this shortcut.
	    if (panesCreated && IsThisAShortcutKeyWeWant(pMsg->wParam)) {
		::SendMessage(m_SW_Main.m_hWnd, pMsg->message,
			      pMsg->wParam, pMsg->lParam);
		retVal = TRUE;
	    }
	}
	break;
    default:
	break;
    }

    return(retVal);
}

void
DirectoryServicesViewNew::PrepareForDestruction()
{
}

int
DirectoryServicesViewNew::OnCreate(LPCREATESTRUCT lpcs)
{
    if (C3DFormView::OnCreate(lpcs) == -1)           
	return -1;

    ASSERT(this->m_hWnd);
    InitializeSplitterWindow();

    return(0);
}

BOOL
DirectoryServicesViewNew::InitializeSplitterWindow()
{
    BOOL fRet = TRUE;
    CCreateContext crCtxt;
    UINT paneX = 0, panesY = 0;
    CRString dsSection(IDS_NDS_DIRSERV_SECTION);

    fRet = m_SW_Main.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE);

	CRect rectClient;
	GetClientRect(rectClient);

	UINT nStaticallyAssignedPaneX = 200;
	UINT nStaticallyAssignedPanesY = 200;
	UINT nDynamicallyAssignedPaneX = rectClient.Width()/2; 
	UINT nDynamicallyAssignedPanesY = rectClient.Height()/2; 


    if (fRet) {
	
	paneX  = ::GetPrivateProfileInt(dsSection,
					   CRString(IDS_NDS_DIRSERV_LEFTPANE_X),	//LeftPaneX
					   nStaticallyAssignedPaneX, INIPath);
	panesY = ::GetPrivateProfileInt(dsSection,
					   CRString(IDS_NDS_DIRSERV_PANES_Y),		// PanesY
					   nStaticallyAssignedPanesY, INIPath);
		
	// If user is not forcing us to use his splitter window position setting, then lets set the default value
	if (FALSE == GetIniShort(IDS_INI_USE_MY_DIRSERV_WINDOW_POSITION))
	{
		// The max of these values
		paneX = __max(paneX,nStaticallyAssignedPaneX);
		// It's better to also compare with width of the client's rectangular area to have better size allocation
		paneX = __max(paneX,nDynamicallyAssignedPaneX);
		
		// The max of these values
		panesY = __max(panesY,nStaticallyAssignedPanesY);
		// It's better to also compare with width of the client's rectangular area to have better size allocation
		panesY = __max(panesY,nDynamicallyAssignedPanesY);
		

	}				

				       
 	fRet = m_SW_Main.CreateView(0, 0, RUNTIME_CLASS(DSLeftView),	
				    CSize(paneX, panesY), &crCtxt);
    }

    if (fRet) {

	paneX = ::GetPrivateProfileInt(dsSection,
					   CRString(IDS_NDS_DIRSERV_RIGHTPANE_X),		//RightPaneX
					   200, INIPath);
		
 	fRet = m_SW_Main.CreateView(0, 1, RUNTIME_CLASS(DSRightView),
				    CSize(paneX, panesY), &crCtxt);
    }

    if (fRet) {
	panesCreated = true;
    }

    return fRet;
}


void
DirectoryServicesViewNew::DoDataExchange(CDataExchange *pDX)
{
    C3DFormView::DoDataExchange(pDX);
}

void
DirectoryServicesViewNew::OnInitialUpdate()
{
    C3DFormView::OnInitialUpdate();

    SetScaleToFitSize(CSize(1, 1));
    RECT rect;
    GetClientRect(&rect);
    OnSize(SIZE_RESTORED, rect.right - rect.left, rect.bottom - rect.top);

//    CWazooWnd::SaveInitialChildFocus(&m_SW_Main);
}

void
DirectoryServicesViewNew::OnSize(UINT nType, int cx, int cy)
{
    C3DFormView::OnSize(nType, cx, cy);

    // Don't resize if the controls aren't created yet, or the window
    // is being minimized.

    if ((m_SW_Main.m_hWnd == NULL) || (nType == SIZE_MINIMIZED)) {
	return;
    }

    // Size the splitter.
    CRect rectSWDialog;
    rectSWDialog.TopLeft().x = 0;
    rectSWDialog.TopLeft().y = 0;
    rectSWDialog.BottomRight().x = cx;
    rectSWDialog.BottomRight().y = cy;
    m_SW_Main.MoveWindow(rectSWDialog, TRUE);
}

////////////////////////////////////////////////////////////////////////
// OnUpdateMakeNickname [protected]
//
// CCmdUI handler for ID_SPECIAL_MAKENICKNAME command.
////////////////////////////////////////////////////////////////////////
void
DirectoryServicesViewNew::OnUpdateMakeNickname(CCmdUI* pCmdUI)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    pCmdUI->Enable(pDSLV->GetResultsTocSelectedCount());
}

////////////////////////////////////////////////////////////////////////
// OnMakeNickname [protected]
//
// Handler for ID_SPECIAL_MAKENICKNAME command.
////////////////////////////////////////////////////////////////////////
void
DirectoryServicesViewNew::OnMakeNickname()
{
    ::MakeNickname();
}

void
DirectoryServicesViewNew::OnNextPane()
{
    int row, column;
    CWnd *pWnd = m_SW_Main.GetActivePane(&row, &column);

    if (pWnd != NULL) {
	m_SW_Main.SetActivePane(row, (column == 0) ? 1 : column, NULL);
    }

}


////////////////////////////////////////////////////////////////////////
// OnActivateWazoo [protected]
//
// Simply forward message to right hand side view of DS splitter.
////////////////////////////////////////////////////////////////////////
long
DirectoryServicesViewNew::OnActivateWazoo(WPARAM wParam, LPARAM lParam)
{
    CWnd *pWnd = m_SW_Main.GetPane(0, 1);
    if (pWnd)
        return pWnd->SendMessage(umsgActivateWazoo);
    return 0;
}


bool
DirectoryServicesViewNew::GetSelectedAddresses(CStringList& addressList)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    return(pDSLV->GetSelectedAddresses(addressList));
}

bool
DirectoryServicesViewNew::IsQueryInProgress()
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    return(pDSLV->IsQueryInProgress());
}


void
DirectoryServicesViewNew::OnUpdateNeedActiveFocus(CCmdUI* pCmdUI)
{
    // Only support this command if this view is the active
    // Wazoo window AND it owns the keyboard focus.
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    CWazooWnd* pWazooWnd = (CWazooWnd *) GetParent();
    ASSERT_KINDOF(CWazooWnd, pWazooWnd);
    pCmdUI->Enable(pWazooWnd->HasActiveFocus() && pDSLV->IsPrintable());
}

void
DirectoryServicesViewNew::OnFilePrint()
{
    DSPrintEditView *View = PreparePrint();
    ASSERT(View);
    if (View) {
	View->SendMessage(WM_COMMAND, ID_FILE_PRINT);
    }
}

void
DirectoryServicesViewNew::OnFilePrintOne()
{
    DSPrintEditView *View = PreparePrint();
    ASSERT(View);
    if (View) {
	View->SendMessage(WM_COMMAND, ID_FILE_PRINT_DIRECT);
    }
}

void
DirectoryServicesViewNew::OnFilePrintPreview()
{
    DSPrintEditView *View = PreparePrint();
    ASSERT(View);
    if (View) {
	View->SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
    }
}

DSPrintEditView *
DirectoryServicesViewNew::PreparePrint()
{
    CCreateContext  Context;
    CEdit          *Edit;

    if (!m_PrintPreView) {
	m_PrintPreView = DEBUG_NEW_MFCOBJ_NOTHROW DSPrintEditView;
	memset(&Context, 0, sizeof(Context));
 	Context.m_pCurrentDoc = &doc;
	if (!m_PrintPreView ||
	    !m_PrintPreView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
				    CRect(0, 0, 0, 0), ::AfxGetMainWnd(),
				    AFX_IDW_PANE_FIRST + 1, &Context)) {
	    ASSERT(FALSE);
	    if (m_PrintPreView)
		delete m_PrintPreView;
	    m_PrintPreView = NULL;
	    return(NULL);
 	}
	else {
	    m_PrintPreView->SetCB(DSSetPrintingPreviewingFlag,
				  static_cast<void *>(this));
	}
    }

    ASSERT_VALID(m_PrintPreView);

    Edit = &m_PrintPreView->GetEditCtrl();
    Edit->SetFont(&PrinterFont, FALSE);

    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));

    CString details;
    details.Empty();

    pDSLV->GetResultsLong(details);

    // Clear the old contents.
    Edit->SetSel(0, -1);
    Edit->ReplaceSel("");
    // Put the new contents in.
    Edit->SetSel(-1, -1);
    Edit->ReplaceSel(details);

    char titleBuf[56];
    const char *query = pDSLV->GetLastQuery();
    int queryLen = strlen(query), val;
    CString csTitle;
	csTitle.LoadString(IDS_DIRSERV_DIRSERV);
    strcpy(titleBuf, csTitle.GetBuffer(100));
	strcat(titleBuf, ": ");
    val = strlen(titleBuf) + queryLen + 1;
    csTitle.ReleaseBuffer();
    if (val > 56) {
	int len = strlen(titleBuf);
	strncpy(titleBuf + len, query, 56 - len - 3 - 1);
	titleBuf[52] = '\0';
	strcat(titleBuf, "...");
    }
    else {
	strcat(titleBuf, query);
    }
    doc.SetTitle(titleBuf);
                                   	
    return(m_PrintPreView);
}

void
DirectoryServicesViewNew::QueryStart()
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    if (pDSLV) {
	if (!pDSLV->IsQueryInputEmpty() && pDSLV->IsAnyDatabaseChecked()
	    && !pDSLV->IsQueryInProgress()) {
	    pDSLV->QueryStart();
	}
    }
}

void
DirectoryServicesViewNew::GiveQueryInputFocus()
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    if (pDSLV) {
	pDSLV->GiveQueryInputFocus();
    }
}

LRESULT
DirectoryServicesViewNew::WindowProc(UINT wMessage, WPARAM wParam,
				     LPARAM lParam)
{
    return(C3DFormView::WindowProc(wMessage, wParam, lParam));
} 

void
DirectoryServicesViewNew::CheckClosePrintPreview()
{
    if (m_PrintPreView && printing_or_previewing) {
 	CFrameWnd *pParent;
 	CWnd *pNaturalParent = m_PrintPreView->GetParentFrame();
 	pParent = DYNAMIC_DOWNCAST(CFrameWnd, pNaturalParent);
  	if (pParent == NULL || pParent->IsIconic())
 	    pParent = (CFrameWnd *)AfxGetThread()->m_pMainWnd;

 	ASSERT_VALID(pParent);
 	ASSERT_KINDOF(CFrameWnd, pParent);
	
	CPreviewView *pView =
	    (CPreviewView *)pParent->GetDlgItem(AFX_IDW_PANE_FIRST);
	ASSERT_KINDOF(CPreviewView, pView);

	pView->SendMessage(WM_COMMAND,
			   MAKEWPARAM(LOWORD(AFX_ID_PREVIEW_CLOSE),BN_CLICKED),
			   LPARAM(::GetDlgItem(pView->m_hWnd,
					     AFX_ID_PREVIEW_CLOSE)));
    }
}

void
DirectoryServicesViewNew::SetPrintState(bool state)
{
    printing_or_previewing = state;
}

bool
DirectoryServicesViewNew::IsPrintPreviewMode()
{
    return(printing_or_previewing);
}

void
DirectoryServicesViewNew::ResetResize()
{
    if (m_SW_Main.m_hWnd != NULL) {
	m_SW_Main.ResetResize();
    }
}

void
DirectoryServicesViewNew::DoQuery(LPCTSTR QueryString)
{
    DSLeftView *pDSLV = static_cast<DSLeftView *>(m_SW_Main.GetPane(0, 0));
    if (pDSLV) {
	pDSLV->DoQuery(QueryString);
    }
}

void
DSSetPrintingPreviewingFlag(void *data, bool state)
{
    DirectoryServicesViewNew *pDSVN
	= static_cast<DirectoryServicesViewNew *>(data);

    ASSERT_KINDOF(DirectoryServicesViewNew, pDSVN);

    if (pDSVN) {
	pDSVN->SetPrintState(state);
    }
}

