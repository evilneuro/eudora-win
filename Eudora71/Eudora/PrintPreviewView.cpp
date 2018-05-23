// PrintPreviewView.cpp : implementation file
//

#include "stdafx.h"
#include "eudoraexe.h"
#include "PrintPreviewView.h"
#include "TextFileView.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewView

IMPLEMENT_DYNCREATE(CPrintPreviewView, CPreviewView)

CPrintPreviewView::CPrintPreviewView()
{
}

CPrintPreviewView::~CPrintPreviewView()
{
	delete m_pPreviewState;//dont let the base class do this. (will result in a crash)

	m_pPreviewState = NULL; 
	CTextFileView *pTextFileView = (CTextFileView *)m_pPrintView;
	pTextFileView->DontDeactivateHighLight(TRUE);

	CFrameWnd *pFrameWnd = pTextFileView->GetParentFrame();
	pFrameWnd->PostMessage(WM_CLOSE);

}


BEGIN_MESSAGE_MAP(CPrintPreviewView, CPreviewView)
	//{{AFX_MSG_MAP(CPrintPreviewView)
	ON_COMMAND(AFX_ID_PREVIEW_PRINT, OnPreviewPrint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewView drawing


/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewView diagnostics

#ifdef _DEBUG
void CPrintPreviewView::AssertValid() const
{
	CPreviewView::AssertValid();
}

void CPrintPreviewView::Dump(CDumpContext& dc) const
{
	CPreviewView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewView message handlers
void CPrintPreviewView::OnPreviewPrint()
{
	CTextFileView *pTextFileView = (CTextFileView*)m_pPrintView;
	CView *pOrigView = (CView*)pTextFileView->m_pABOrigView;
	OnPreviewClose();               // force close of Preview

	// cause print (can be overridden by catching the command)
//	CWnd* pMainWnd = AfxGetThread()->m_pMainWnd;
//	ASSERT_VALID(pMainWnd);
	pOrigView->SendMessage(WM_COMMAND, ID_FILE_PRINT);
}
