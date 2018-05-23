// BlahBlahBlahView.cpp : implementation file
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include "eudora.h"

#include "BlahBlahBlahView.h"
#include "font.h"
#include "ReadMessageDoc.h"
#include "ReadMessageFrame.h"
#include "rs.h"

#include "DebugNewHelpers.h"

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




