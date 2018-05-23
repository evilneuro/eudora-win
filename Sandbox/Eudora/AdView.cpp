// AdView.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "AdView.h"
#include "pgosutl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CAdView

IMPLEMENT_DYNCREATE(CAdView, CPaigeEdtView)

CAdView::CAdView()
{
	SetReadOnly();
}

CAdView::~CAdView()
{
}


BEGIN_MESSAGE_MAP(CAdView, CPaigeEdtView)
	//{{AFX_MSG_MAP(CAdView)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAdView message handlers

void CAdView::OnInitialUpdate() 
{
	m_pgBorder.SetRectEmpty();

	CPaigeEdtView::OnInitialUpdate();

	paige_rec_ptr pgRecPtr = (paige_rec_ptr)UseMemory(m_paigeRef);

	// Tell Paige not to display the caret
	pgRecPtr->flags2 |= NO_CARET_BIT;

	// There are times when a paige ref needs to know that this is an ad
	PgStuffBucket* pSB = (PgStuffBucket*) pgRecPtr->user_refcon;
	pSB->kind = PgStuffBucket::kAd;

	UnuseMemory(m_paigeRef);

	COLORREF Sys3DColor = GetSysColor(COLOR_3DFACE);
	color_value BackColor;
	pgOSToPgColor(&Sys3DColor, &BackColor);
	pgSetPageColor(m_paigeRef, &BackColor);

	PgDataTranslator theTxr(m_paigeRef);

	const char* AdHTML = "<div align=center><a href=""http://www.ugo.com/welovegary/""><img src=""http://www.ugo.com/welovegary/images/garycoleman.jpg"" alt=""Dis%20eez%20da%20ad!"" width=128 height=158></a></div>";
	memory_ref impRef = CString2Memory(PgMemGlobalsPtr(), AdHTML);
	theTxr.ImportData(&impRef, PgDataTranslator::kFmtHtml);
	DisposeMemory(impRef);

	ShowScrollBar(SB_BOTH, FALSE);
}

void CAdView::OnSetFocus(CWnd* pOldWnd)
{
	// Bypass PaigeEdtView because it mucks with the caret, which we want to hide
	CView::OnSetFocus(pOldWnd);
}

void CAdView::OnKillFocus(CWnd* pNewWnd)
{
	// Bypass PaigeEdtView because it mucks with the caret, which we want to hide
	CView::OnKillFocus(pNewWnd);
}
