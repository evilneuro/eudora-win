// BlahBlahBlahView.cpp : implementation file
//

#include "stdafx.h"
#include "eudora.h"

#include "BlahBlahBlahView.h"
#include "font.h"
#include "ReadMessageDoc.h"
#include "ReadMessageFrame.h"
#include "rs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlahBlahBlahView

IMPLEMENT_DYNCREATE(CBlahBlahBlahView, CEditView)

CBlahBlahBlahView::CBlahBlahBlahView()
{
}

CBlahBlahBlahView::~CBlahBlahBlahView()
{
}


BEGIN_MESSAGE_MAP(CBlahBlahBlahView, CEditView)
	//{{AFX_MSG_MAP(CBlahBlahBlahView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_BN_CLICKED( ID_EDIT_MESSAGE, OnEditMode )
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlahBlahBlahView drawing

void CBlahBlahBlahView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CBlahBlahBlahView diagnostics

#ifdef _DEBUG
void CBlahBlahBlahView::AssertValid() const
{
	CEditView::AssertValid();
}

void CBlahBlahBlahView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBlahBlahBlahView message handlers

void CBlahBlahBlahView::OnInitialUpdate() 
{
	CReadMessageFrame*	pParent;
	CReadMessageDoc*	pDoc;
	char*				szFullMessage;

	CEditView::OnInitialUpdate();

	pDoc = ( CReadMessageDoc* ) GetDocument();
	
	if( pDoc == NULL )
	{
		return;
	}
	
	ASSERT_KINDOF( CReadMessageDoc, pDoc );

	// Read up the full message
	pDoc->Read();
	
	// get the whole message
	szFullMessage = pDoc->GetFullMessage();
	
	GetEditCtrl().SetFont( &GetMessageFont() );
	GetEditCtrl().SetWindowText( szFullMessage );

	pParent = ( CReadMessageFrame* ) GetParentFrame();

	ASSERT_KINDOF( CReadMessageFrame, pParent );
		
	GetEditCtrl().SetReadOnly( TRUE );

	if( pParent->GetCheck( ID_EDIT_MESSAGE ) )
	{
		GetEditCtrl().SetReadOnly( FALSE );
	}

	delete [] szFullMessage;
}


void CBlahBlahBlahView::OnEditMode()
{
	CReadMessageFrame*	pParent;

	pParent = ( CReadMessageFrame* ) GetParentFrame();

	ASSERT_KINDOF( CReadMessageFrame, pParent );

	GetEditCtrl().SetReadOnly( TRUE );

	if( pParent->GetCheck( ID_EDIT_MESSAGE ) )
	{
		GetEditCtrl().SetReadOnly( FALSE );
	}
}


void CBlahBlahBlahView::OnFilePrintPreview()
{
	// MFC now uses the frame window as the frame for the preview view.
	// In order to get around it, you can minimize the frame.  MFC will
	// then use the main frame (same as the old behavior).  So, we save
	// the window state and restore in OnEndPrintPreview.

	m_bIsIconized = GetParentFrame()->IsIconic();
	m_bIsMaximized = GetParentFrame()->IsZoomed();

	if( !m_bIsIconized )
	{
		SetRedraw( FALSE );
		GetParentFrame()->ShowWindow( SW_MINIMIZE );
	}

	CEditView::OnFilePrintPreview();
}


void CBlahBlahBlahView::OnEndPrintPreview( 
CDC*			pDC, 
CPrintInfo*		pInfo, 
POINT			point, 
CPreviewView*	pView )
{
	CView::OnEndPrintPreview( pDC, pInfo, point, pView );

	if( !m_bIsIconized )
	{	
		if( m_bIsMaximized )
		{
			( ( CMDIChildWnd* ) GetParentFrame() )->MDIMaximize();
		}
		else
		{
			( ( CMDIChildWnd* ) GetParentFrame() )->MDIRestore();
		}
	
		SetRedraw( TRUE );
	}
}




