// WebBrowserView.cpp : implementation file
//

#include "stdafx.h"

#ifdef WEB_BROWSER_WAZOO

#include "resource.h"
#include "rs.h"
#include "site.h"
#include "mshtmcid.h"
#include "WebBrowserView.h"
#include "mainfrm.h"
#include "eudora.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CWebBrowserView

IMPLEMENT_DYNCREATE(CWebBrowserView, CTridentView)

CWebBrowserView::CWebBrowserView()
{	
}

CWebBrowserView::~CWebBrowserView()
{
}

BOOL CWebBrowserView::DoOnDisplayPlugin(IHTMLElement* pElemen)
{
	return FALSE;
}

BOOL CWebBrowserView::WriteTempFile(CFile& theFile, CString& szStyleSheetFormat)
{
	ASSERT(0);
	
	return FALSE;
}

BOOL CWebBrowserView::LoadMessage()
{
#ifdef ADWARE_PROTOTYPE
	char url[255];
	static int sizes[] = {
		468,60,
		234,60,
		245,96,
		125,125,
		120,90,
		115,90,
		88,31,
		0
	};
	int *sizePtr;
	CRect rectClient;
	int wi, hi;

	// get our rectangle
	GetClientRect(rectClient);
	wi = rectClient.Width();
	hi = rectClient.Height();

	// find the first rect that fits
	for (sizePtr=sizes;*sizePtr;sizePtr+=2)
		if (sizePtr[0]<wi && sizePtr[1]<hi) break;
	if (!*sizePtr) sizePtr = sizes;

	// form the url
	sprintf(url,"%s?get=%dx%d",
		"http://lazlo.qualcomm.com/cgi-bin/adtest.cgi",
		sizePtr[0],sizePtr[1]);

	if (FAILED(m_pSite->Load((LPTSTR)url)))
		return FALSE;
#else
	if (FAILED(m_pSite->Load((LPTSTR)(LPCTSTR)GetIniString(IDS_INI_WEB_BROWSER_PAGE))))
		return FALSE;
#endif

	return TRUE;
}

#ifdef ADWARE_PROTOTYPE
void CWebBrowserView::OnTimer(UINT)
{
	// see if we are the active app
	CWnd* Active = CWnd::GetForegroundWindow();
	const BOOL bWeBeActive = (Active->GetParentFrame() == AfxGetMainWnd() || Active == AfxGetMainWnd());
	char admessage[80];
	BOOL enableMe = TRUE;

	if (bWeBeActive)
	{
		long idleTime = time(NULL) - ((CEudoraApp *)AfxGetApp())->GetLastIdle();

		// has the mouse moved?
		CPoint pt;
		static CPoint lastPt;
		::GetCursorPos(&pt);
		if ((lastPt.x==pt.x && lastPt.y==pt.y) && (idleTime > GetIniLong(IDS_INI_AD_KFI)))
			strcpy(admessage,"AP - Time doesn't count, user idle");
		else
		{
			// Get a new ad if we need one
			static int intervalCount;

			if (++intervalCount >= GetIniLong(IDS_INI_AD_KFI_COUNT))
			{
				intervalCount = 0;
				enableMe = FALSE;
				LoadMessage();	// refresh window every time timer goes off
			}
			sprintf(admessage,"AP - Ad showing for %d intervals.",intervalCount);
		}
		lastPt = pt;
	}
	else strcpy(admessage,"AP - time doesn't count, app not active.");
	// Get main window so that we can access the global status bar.
	CFrameWnd* p_mainframe = (CFrameWnd *) AfxGetMainWnd();
	p_mainframe->SetMessageText(admessage);
	p_mainframe->GetMessageBar()->UpdateWindow();
	EnableWindow(enableMe);
}
#endif

BEGIN_MESSAGE_MAP(CWebBrowserView, CTridentView)
	//{{AFX_MSG_MAP(CWebBrowserView)
	//}}AFX_MSG_MAP
#ifdef ADWARE_PROTOTYPE
    ON_WM_TIMER()
#endif //ADWARE_PROTOTYPE
	END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebBrowserView drawing

void CWebBrowserView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CWebBrowserView diagnostics

#ifdef _DEBUG
void CWebBrowserView::AssertValid() const
{
	CTridentView::AssertValid();
}

void CWebBrowserView::Dump(CDumpContext& dc) const
{
	CTridentView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CWebBrowserView message handlers

void CWebBrowserView::OnInitialUpdate() 
{
	CTridentView::SetDocHostUIFlags(CTridentView::GetDocHostUIFlags()|DOCHOSTUIFLAG_SCROLL_NO);
	CTridentView::OnInitialUpdate();
	
	ExecCommand(IDM_BROWSEMODE);
#ifdef ADWARE_PROTOTYPE
	SetTimer(0xadadadad,GetIniLong(IDS_INI_AD_KFI)*1000,NULL);
#endif
} 

#endif //WEB_BROWSER_WAZOO
