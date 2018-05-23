// TextFileView.cpp : implementation file
//03-17-2000 Implementation of File New or File Open text Document
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
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
#include "eudoraexe.h"
#include "TextFileView.h"
#include "resource.h"
#include "rs.h"

// Paige html definitions
#include "pghtmdef.h"

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CTextFileView

IMPLEMENT_DYNCREATE(CTextFileView, CPaigeEdtView)

CTextFileView::CTextFileView()
{
}

CTextFileView::~CTextFileView()
{
}


BEGIN_MESSAGE_MAP(CTextFileView, CPaigeEdtView)
	//{{AFX_MSG_MAP(CTextFileView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(WM_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEASQUOTATION, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_PICTURE, OnUpdateInsertPicture)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextFileView drawing

void CTextFileView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
	CPaigeEdtView::OnDraw(pDC);

}

/////////////////////////////////////////////////////////////////////////////
// CTextFileView diagnostics

#ifdef _DEBUG
void CTextFileView::AssertValid() const
{
	CPaigeEdtView::AssertValid();
}

void CTextFileView::Dump(CDumpContext& dc) const
{
	CPaigeEdtView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextFileView message handlers

void CTextFileView::OnInitialUpdate() 
{
	CPaigeEdtView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	UpdateScrollBars(true);
}

// Serialize:
// Loads/saves Paige contents to/from HTML files
//
// Once this code is in Eudora, it will need to be rewritten to use streams
// instead of just passing the file name to the SGML parser.

void CTextFileView::Serialize(CArchive& ar) 
{
    PgDataTranslator theTxr( m_paigeRef );
	if( m_bAllowTables ) //on address book printing.
		{
		theTxr.m_importFlags |= IMPORT_TABLES_FLAG;
		}   
    CFile *theFile = ar.GetFile();
    CString fileName = theFile->GetFileName();

    //I guess we can just find ".ext" but this is being safe
    int dot = fileName.ReverseFind('.');
    CString fileExt = fileName.Right(fileName.GetLength() - dot - 1);

    PgDataTranslator::FormatType format;

    if ( ( fileExt.CompareNoCase(CRString(IDS_HTM_EXTENSION)) == 0 ) ||
         ( fileExt.CompareNoCase(CRString(IDS_HTML_EXTENSION)) == 0) )
        format = PgDataTranslator::kFmtHtml;
    else if ( fileExt.CompareNoCase(CRString(IDS_RTF_EXTENSION)) == 0)
        format = PgDataTranslator::kFmtRtf;
    else
        format = PgDataTranslator::kFmtText;

    if ( ar.IsStoring() ) {
        // storing code
        theTxr.ExportData( theFile, format);
        SaveChangeState();
    }
    else {
        // loading code

        pgApplyNamedStyle(m_paigeRef,NULL,body_style,best_way);

        theTxr.ImportData( theFile, format);
        pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
        UpdateScrollBars();
    }
        
    //theFile->Close();
}

void CTextFileView::OnUpdateEditPaste( CCmdUI* pCmdUI ) 
{
    if ( !m_fRO ) {
        COleDataObject clipBoardData;
        VERIFY(clipBoardData.AttachClipboard());

        if ( (clipBoardData.IsDataAvailable( CF_BITMAP, NULL)) ) {
			pCmdUI->Enable( FALSE );
			return;
		}
    }

	CPaigeEdtView::OnUpdateEditPaste( pCmdUI );
}

void CTextFileView::OnUpdateInsertPicture(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( FALSE );
}


#if 0
// a little test jig hack for the EDIT message stuff. it's too much darn
// trouble to write a separate app that does this--having to hook yourself
// into the other's process space, etc.
#include "font.h"
LRESULT CTextFileView::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	if ( message == WM_CHAR ) {
		SendMessage( WM_SETFONT, (WPARAM)(HFONT)ScreenFontBold, 0 );
		SendMessage( WM_SETTEXT, 0, (LPARAM)"Hi there ya nerd!" );

		char buf[256] = "";
		SendMessage( WM_GETTEXT, sizeof(buf), (LPARAM)&buf );
		MessageBox( buf );

		SendMessage( EM_SETSEL, 0, 10 );

		SendMessage( EM_GETSELTEXT, 0, (LPARAM)&buf );
		MessageBox( buf );

		DWORD begin, end;
		SendMessage( EM_GETSEL, (WPARAM)&begin, (LPARAM)&end );
		sprintf( buf, "Sel range: %i --> %i", begin, end );
		MessageBox( buf );

		Sleep( 3000 );
		SendMessage( EM_HIDESELECTION, 1, 0 );
		Sleep( 3000 );
		SendMessage( EM_HIDESELECTION, 0, 0 );
		Sleep( 3000 );

		SendMessage( WM_SETFONT, (WPARAM)(HFONT)ScreenFontItalic, 0 );
		SendMessage( EM_REPLACESEL, 0, (LPARAM)"Eat me!" );
		Sleep( 3000 );
		SendMessage( WM_UNDO, 0, 0 );
		SendMessage( EM_SETMODIFY, 0, 0 );
		return TRUE;
	}

	return CPaigeEdtView::WindowProc( message, wParam, lParam );
}
#endif

