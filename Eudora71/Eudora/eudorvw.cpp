// eudorvw.cpp : implementation of the CEudoraView class
//

#include "stdafx.h"
#include "eudora.h"

#include "eudordoc.h"
#include "eudorvw.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CEudoraView

IMPLEMENT_DYNCREATE(CEudoraView, CView)

BEGIN_MESSAGE_MAP(CEudoraView, CView)
	//{{AFX_MSG_MAP(CEudoraView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEudoraView construction/destruction

CEudoraView::CEudoraView()
{
	// TODO: add construction code here
}

CEudoraView::~CEudoraView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CEudoraView drawing

void CEudoraView::OnDraw(CDC* pDC)
{
	CEudoraDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CEudoraView printing

BOOL CEudoraView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CEudoraView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CEudoraView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CEudoraView diagnostics

#ifdef _DEBUG
void CEudoraView::AssertValid() const
{
	CView::AssertValid();
}

void CEudoraView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CEudoraDoc* CEudoraView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEudoraDoc)));
	return (CEudoraDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEudoraView message handlers
