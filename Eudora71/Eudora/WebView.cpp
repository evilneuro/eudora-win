// WebView.cpp : implementation of the CWebView class
//

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "WebView.h"
#include "regif.h"
#include "urledit.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CWebView

IMPLEMENT_DYNCREATE(CWebView, CHtmlView)

BEGIN_MESSAGE_MAP(CWebView, CHtmlView)
	//{{AFX_MSG_MAP(CWebView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CHtmlView::OnFilePrint)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebView construction/destruction

CWebView::CWebView()
{
	// TODO: add construction code here

}

CWebView::~CWebView()
{
}

BOOL CWebView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CHtmlView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CWebView drawing

void CWebView::OnDraw(CDC* pDC)
{
//	CWebDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CWebView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

//	REGIF IERegKey;
//	char IELocation[MAX_PATH + 1];
//
//	IERegKey.Attach("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE");
//	IERegKey.Read(NULL, IELocation, sizeof (IELocation));
//
//	ShellExecute(*::AfxGetMainWnd(), "open", IELocation, "https://mymail.qualcomm.com/inbox/?cmd=navbar", NULL, SW_SHOWNORMAL);

	// TODO: This code navigates to a popular spot on the web.
	//  change the code to go where you'd like.
//	Navigate2(_T("https://mymail.qualcomm.com/inbox/beckley/Calendar/"), NULL, NULL);
//	Navigate2(_T("https://mymail.qualcomm.com/inbox/beckley/Calendar/?cmd=contents"), NULL, NULL);
	Navigate2(CURLEdit::DecodeMailtoURLChars(GetIniString(IDS_INI_WEB_WAZOO_HOME)), NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CWebView printing


/////////////////////////////////////////////////////////////////////////////
// CWebView diagnostics

#ifdef _DEBUG
void CWebView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CWebView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWebView message handlers

void CWebView::OnNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	// Don't open a new window, but instead navigate inside the same window
	if (Cancel)
		*Cancel = TRUE;

	Navigate2(m_NavURL);
}

HRESULT CWebView::OnTranslateUrl(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
	HRESULT result = CHtmlView::OnTranslateUrl(dwTranslate, pchURLIn, ppchURLOut);

	m_NavURL = (ppchURLOut && *ppchURLOut)? *ppchURLOut : pchURLIn;

	return result;
}
	