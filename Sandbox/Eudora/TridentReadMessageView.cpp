// TridentReadMessageView.cpp : implementation of the CTridentReadMessageView class
//

#include "stdafx.h"
#include "eudora.h"

#ifdef IDM_PROPERTIES
#undef IDM_PROPERTIES
#endif

#include <AFXPRIV.H>
#include <afxdisp.h>

#include "bstr.h"
#include "ColorLis.h"
#include "ReadMessageFrame.h"
#include "ReadMessageDoc.h"
#include "TridentReadMessageView.h"
#include "site.h"
#include "mshtmcid.h"
#include "mainfrm.h"
#include "Text2HTML.h"
#include "FaceNameDialog.h"
#include "trnslate.h"

// delete 
#include "newmbox.h"

#include "fileutil.h"
#include "guiutils.h"
#include "rs.h"
#include "msgutils.h"
#include "summary.h"

#include "QCSharewareManager.h"

#include "QCOleDropSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CTridentReadMessageView

IMPLEMENT_DYNCREATE(CTridentReadMessageView, CTridentView)

BEGIN_MESSAGE_MAP(CTridentReadMessageView, CTridentView)
	//{{AFX_MSG_MAP(CTridentReadMessageView)
	ON_COMMAND(ID_EDIT_TEXT_BOLD, OnEditBold)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_BOLD, OnUpdateEditBold)
	ON_COMMAND(ID_EDIT_TEXT_ITALIC, OnEditItalic)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_ITALIC, OnUpdateEditItalic)
	ON_COMMAND(ID_EDIT_TEXT_UNDERLINE, OnEditUnderline)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_UNDERLINE, OnUpdateEditUnderline)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_EDIT_CHECKSPELLING, OnEditCheckspelling)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CHECKSPELLING, OnUpdateEditCheckspelling)
	ON_COMMAND(ID_EDIT_TEXT_FONT, OnEditTextFont)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_FONT, OnUpdateEditTextFont)
	ON_COMMAND(ID_EDIT_TEXT_PLAIN, OnEditTextPlain)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_PLAIN, OnUpdateEditTextPlain)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_HUMONGOUS, OnEditTextSizeHumongous)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_LARGE, OnEditTextSizeLarge)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_LARGER, OnEditTextSizeLarger)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_MEDIUM, OnEditTextSizeMedium)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_SMALL, OnEditTextSizeSmall)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_VERY_LARGE, OnEditTextSizeVeryLarge)
	ON_COMMAND(ID_EDIT_TEXT_SIZE_VERY_SMALL, OnEditTextSizeVerySmall)
	ON_COMMAND(ID_COLOR0, OnColor0)
	ON_COMMAND(ID_COLOR1, OnColor1)
	ON_COMMAND(ID_COLOR10, OnColor10)
	ON_COMMAND(ID_COLOR11, OnColor11)
	ON_COMMAND(ID_COLOR12, OnColor12)
	ON_COMMAND(ID_COLOR13, OnColor13)
	ON_COMMAND(ID_COLOR14, OnColor14)
	ON_COMMAND(ID_COLOR15, OnColor15)
	ON_COMMAND(ID_COLOR16, OnColor16)
	ON_COMMAND(ID_COLOR2, OnColor2)
	ON_COMMAND(ID_COLOR3, OnColor3)
	ON_COMMAND(ID_COLOR4, OnColor4)
	ON_COMMAND(ID_COLOR5, OnColor5)
	ON_COMMAND(ID_COLOR6, OnColor6)
	ON_COMMAND(ID_COLOR7, OnColor7)
	ON_COMMAND(ID_COLOR8, OnColor8)
	ON_COMMAND(ID_COLOR9, OnColor9)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_BLKFMT_BULLETTED_LIST, OnBlkfmtBullettedList)
	ON_UPDATE_COMMAND_UI(ID_BLKFMT_BULLETTED_LIST, OnUpdateBlkfmtBullettedList)
	ON_COMMAND(ID_EDIT_INSERT_LINK, OnEditInsertLink)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_LINK, OnUpdateEditInsertLink)
	ON_COMMAND(ID_EDIT_INSERT_PICTURE, OnEditInsertPicture)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_PICTURE, OnUpdateEditInsertPicture)
	ON_COMMAND(ID_EDIT_INSERT_HR, OnEditInsertHr)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT_HR, OnUpdateEditInsertHr)
	ON_COMMAND(ID_EDIT_PASTEASQUOTATION, OnEditPasteAsQuotation)
	//}}AFX_MSG_MAP

//	ON_COMMAND(ID_EDIT_TEXT_TT, OnBlkfmtFixedwidth)
//	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_TT, OnUpdateBlkfmtFixedwidth)

	ON_COMMAND(ID_EDIT_TEXT_LEFT, OnLeftAlign)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_LEFT, OnUpdateLeftAlign)
	ON_COMMAND(ID_EDIT_TEXT_RIGHT, OnRightAlign)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_RIGHT, OnUpdateRightAlign)
	ON_COMMAND(ID_EDIT_TEXT_CENTER, OnCenterAlign)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_CENTER, OnUpdateCenterAlign)

	ON_COMMAND( ID_EDIT_TEXT_INDENT_IN, OnIndent )
	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_INDENT_IN, OnUpdateIndent )
	ON_COMMAND( ID_EDIT_TEXT_INDENT_OUT, OnOutdent )
	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_INDENT_OUT, OnUpdateOutdent )

	ON_COMMAND( ID_EDIT_TEXT_CLEAR, OnClearFormatting )
	ON_UPDATE_COMMAND_UI( ID_EDIT_TEXT_CLEAR, OnUpdateClearFormatting )

	ON_COMMAND( ID_EDIT_CLEAR, OnDelete )
	ON_UPDATE_COMMAND_UI( ID_EDIT_CLEAR, OnUpdateDelete )

	ON_COMMAND( ID_EDIT_UNDO, OnUndo )
	ON_UPDATE_COMMAND_UI( ID_EDIT_UNDO, OnUpdateUndo )

	ON_COMMAND( ID_EDIT_REDO, OnRedo )
	ON_UPDATE_COMMAND_UI( ID_EDIT_REDO, OnUpdateRedo )

	ON_COMMAND( ID_EDIT_CUT, OnCut )
	ON_UPDATE_COMMAND_UI( ID_EDIT_CUT, OnUpdateCut )
	ON_COMMAND( ID_EDIT_PASTE, OnPaste )
	ON_UPDATE_COMMAND_UI( ID_EDIT_PASTE, OnUpdatePaste )
	ON_UPDATE_COMMAND_UI( ID_EDIT_PASTEASQUOTATION, OnUpdatePaste )

	ON_BN_CLICKED( ID_EDIT_MESSAGE, OnEditMode )
	ON_UPDATE_COMMAND_UI( ID_EDIT_MESSAGE, OnUpdateEditMode )

	ON_CBN_SELENDOK( IDC_FONT_COMBO, OnFontChange )
	ON_UPDATE_COMMAND_UI( IDC_FONT_COMBO, OnUpdateFont )

	ON_BN_CLICKED( ID_BLAHBLAHBLAH, OnBlahBlahBlah )
	ON_BN_CLICKED( ID_USE_FIXED_FONT, OnFixedFont )

	ON_UPDATE_COMMAND_UI(ID_COLOR0, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR1, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR2, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR3, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR4, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR5, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR6, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR7, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR8, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR9, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR10, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR11, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR12, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR13, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR15, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR14, OnUpdateFgcolor)
	ON_UPDATE_COMMAND_UI(ID_COLOR16, OnUpdateFgcolor)

	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_HUMONGOUS, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_LARGE, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_LARGER, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_MEDIUM, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_SMALL, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_VERY_LARGE, OnUpdateSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_SIZE_VERY_SMALL, OnUpdateSize)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTridentReadMessageView construction/destruction

CTridentReadMessageView::CTridentReadMessageView() 
{
	m_pParentFrame = NULL;
	m_bstrBookmark = NULL;
}


CTridentReadMessageView::~CTridentReadMessageView()
{
	CMessageDoc *pDoc = (CMessageDoc*) GetDocument();
	if (pDoc)
	{
		ASSERT_KINDOF(CMessageDoc, pDoc);
		if (pDoc->m_Sum)
			pDoc->m_Sum->SetShowAllHeaders( FALSE );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CTridentReadMessageView diagnostics

#ifdef _DEBUG
void CTridentReadMessageView::AssertValid() const
{
	CView::AssertValid();
}

void CTridentReadMessageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CReadMessageDoc* CTridentReadMessageView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReadMessageDoc)));
	return (CReadMessageDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CTridentReadMessageView message handlers


void CTridentReadMessageView::OnEditItalic() 
{
	ExecCommand( IDM_ITALIC );
	GetDocument()->SetModifiedFlag();
}

void CTridentReadMessageView::OnUpdateEditItalic(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;

	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_ITALIC );
	pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}

void CTridentReadMessageView::OnEditUnderline() 
{
	ExecCommand( IDM_UNDERLINE );
	GetDocument()->SetModifiedFlag();
}

void CTridentReadMessageView::OnUpdateEditUnderline(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;

	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_UNDERLINE );
	pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnLeftAlign() 
{
	ExecCommand( IDM_JUSTIFYLEFT );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateLeftAlign(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_JUSTIFYLEFT );
	pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnRightAlign() 
{
	ExecCommand( IDM_JUSTIFYRIGHT );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateRightAlign(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_JUSTIFYRIGHT );
	pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnCenterAlign() 
{
	ExecCommand( IDM_JUSTIFYCENTER );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateCenterAlign(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_JUSTIFYCENTER );
	pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnIndent() 
{
	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		ExecCommand( IDM_INDENT );
		GetDocument()->SetModifiedFlag();
	}
}


void CTridentReadMessageView::OnUpdateIndent(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		DWORD dwStatus = 0;
		
		if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
		{
			// we're not in editing mode
			pCmdUI->Enable( FALSE );
			return;
		}
 
		dwStatus = GetCommandStatus( IDM_INDENT );
		pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE);
	}
}



void CTridentReadMessageView::OnOutdent() 
{
	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		ExecCommand( IDM_OUTDENT );
		GetDocument()->SetModifiedFlag();
	}
}


void CTridentReadMessageView::OnUpdateOutdent(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot change margins
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		DWORD dwStatus = 0;
		
		if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
		{
			// we're not in editing mode
			pCmdUI->Enable( FALSE );
			return;
		}

		dwStatus = GetCommandStatus( IDM_OUTDENT );
		pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE);
	}
}



void CTridentReadMessageView::OnClearFormatting() 
{
	ExecCommand( IDM_REMOVEFORMAT );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateClearFormatting(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_REMOVEFORMAT );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnDelete() 
{
	ExecCommand( IDM_DELETE );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateDelete(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_DELETE );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnCut() 
{
	ExecCommand( IDM_CUT );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateCut(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}
 
	dwStatus = GetCommandStatus( IDM_CUT );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}



void CTridentReadMessageView::OnPaste() 
{
	ExecCommand( IDM_PASTE );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdatePaste(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_PASTE );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}



void CTridentReadMessageView::OnUndo() 
{
	ExecCommand( IDM_UNDO );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateUndo(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	pCmdUI->SetText( CRString( IDS_EDIT_UNDO ) );

	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_UNDO );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::OnRedo() 
{
	ExecCommand( IDM_REDO );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateRedo(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_REDO );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


BOOL CTridentReadMessageView::PreTranslateMessage(MSG* pMsg) 
{
	//
	// Dang, whatta waste.  The IOleIPActiveObject
	// TranslateAccelerator() method eats arrow keys, so we gotta
	// process the arrow keystrokes before they get eaten.
	//
	// This duplicates code in the base class, which is used by
	// read-only implementations of Trident like the preview pane.
	//
	if ((! IsEditing()) && (WM_KEYDOWN == pMsg->message))
	{
		//
		// Look for plain Arrow keystrokes and Ctrl+Arrow
		// keystrokes.  If setting is enabled, translate those
		// keystrokes into "next/previous message" commands.  Also,
		// look for Shift+Space keystrokes and translate those
		// into "change status to unread" commands.
		//
		switch (pMsg->wParam)
		{
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			{
				BOOL bCtrlDown = (GetKeyState(VK_CONTROL) < 0);
				BOOL bUsePlainArrows = GetIniShort(IDS_INI_PLAIN_ARROWS) != 0;
				BOOL bUseCtrlArrows = GetIniShort(IDS_INI_CONTROL_ARROWS) != 0;

				if ((bUsePlainArrows && !bCtrlDown) ||
					(bUseCtrlArrows && bCtrlDown))
				{
					SendMessage(WM_COMMAND, (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP) ? ID_PREVIOUS_MESSAGE : ID_NEXT_MESSAGE);
					return TRUE;	// don't let Trident control see this message
				}
			}
			break;
		case VK_SPACE:
			// handled in base class
			break;
		case 'F':
			// TranslateAccellerator below will eat control-F
			// However, if we return FALSE here and allow event
			// dispatching to happen WITHOUT TranslateAccellerator,
			// all will be well.  SD with much help from JB
			if (GetKeyState(VK_CONTROL) < 0)
				return FALSE;
			break;
		case VK_TAB:
			// Trident is stealing Ctrl+Tab, which disallows cycles through MDI windows
			return FALSE;
			break;
		default:
			break;
		}
	}

	if ( NULL != m_pIOleIPActiveObject )
    {
        HRESULT     hr;

        hr = m_pIOleIPActiveObject->TranslateAccelerator( pMsg );

        //If the object translated the accelerator, we're done
        if ( NOERROR == hr )
		{
            return TRUE;
		}
    }

	
	if( ::IsChild( GetSafeHwnd(), pMsg->hwnd ) && ( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) != 0 ) )
	{
		if( ( pMsg->message == WM_CHAR ) ||
			( ( pMsg->message == WM_KEYUP ) && 
			  ( ( pMsg->wParam == VK_BACK ) || ( pMsg->wParam == VK_DELETE ) ) ) )
		{
			GetDocument()->SetModifiedFlag();
		}
	}

	return CTridentView::PreTranslateMessage(pMsg);
}


void CTridentReadMessageView::OnBgcolor() 
{
	CColorDialog* pColorDlg = new CColorDialog;
	if ( pColorDlg->DoModal() == IDOK )
	{
		SetBgColor( pColorDlg->GetColor() );
	}
	delete pColorDlg;
}


void CTridentReadMessageView::OnUpdateBgcolor(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) );
}


void CTridentReadMessageView::OnFgcolor() 
{
	CColorDialog* pColorDlg = new CColorDialog;
	if ( pColorDlg->DoModal() == IDOK )
	{
		SetFgColor( pColorDlg->GetColor() );
	}
	
	delete pColorDlg;
}


void CTridentReadMessageView::OnUpdateFgcolor(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) );
}


void CTridentReadMessageView::OnEditBold() 
{
	ExecCommand( IDM_BOLD );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateEditBold(CCmdUI* pCmdUI) 
{
	DWORD dwStatus = 0;

	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	dwStatus = GetCommandStatus( IDM_BOLD );
	pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
	pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
}


void CTridentReadMessageView::SetBgColor( COLORREF color )
{
//!! tagged out while object model is changing
::MessageBox(NULL, "Currently not supported", "", MB_OK);
/*
	IUnknown* lpUnk = m_pSite->GetObjectUnknown();

	if ( lpUnk != NULL )
	{
	    HRESULT         hr;

		// Then, request the "document" object from the object
		IHTMLDocument2* pHTMLDocument2;
		hr = lpUnk->QueryInterface(IID_IHTMLDocument2, (void **)&pHTMLDocument2);
		if ( SUCCEEDED( hr ) )
		{

			// Then, get the document element from the document
			IHTMLBodyElement* pBody;
			hr = pHTMLDocument2->get_body(&pBody);
			if ( SUCCEEDED( hr ) )
			{
		
				// translate the hex color into a string without using the CRT
				int idx;
				WCHAR			buff[8]=L"#000000";
				WCHAR           HEX[17]=L"0123456789ABCDEF";

				for (int i=0;i<3;i++)
				{
					idx = color&0xF;
					buff[2*i+2] = HEX[idx];
					color >>= 4;
					idx = color&0xF;
					buff[2*i+1] = HEX[idx];
					color >>= 4;
				}

				// Now, set the background color of the body to be equal to something
				hr = pBody->put_bgColor(buff);

				// cleanup
				pBody->Release();
			}
			pHTMLDocument2->Release();
		}
	}
*/
}


void CTridentReadMessageView::SetFgColor( COLORREF color )
{
	VARIANT vColor; // input arguments

	VariantInit(&vColor);

	V_VT(&vColor) = VT_I4;
	V_I4(&vColor) = color;

	ExecCommand( IDM_FORECOLOR, &vColor );

	VariantClear(&vColor);
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnFilePrint() 
{
	CString	szFontName;
	INT		iSize;

	CTridentView::OnFilePrint();

	if( m_pParentFrame->UseFixedWidthFont() )
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FIXED_FONT );
	}
	else
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FONT );
	}

	iSize = GetIniShort( IDS_INI_MESSAGE_FONT_SIZE );

	SetFont( szFontName, iSize );
}



void CTridentReadMessageView::OnInitialUpdate() 
{
	m_pParentFrame = ( CReadMessageFrame* ) GetParentFrame();

	ASSERT_KINDOF( CReadMessageFrame, m_pParentFrame );

	CTridentView::OnInitialUpdate();	
		
    //We created the thing, now activate it with "Show"
    if( GetStyle() & WS_VISIBLE )
	{
		m_pSite->Activate( OLEIVERB_SHOW );
	}
	else
	{
		m_pSite->Activate( OLEIVERB_HIDE );
	}
	
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) )
	{
		// turn edit mode on
		ExecCommand( IDM_EDITMODE );				
	}
	else
	{
		ExecCommand( IDM_BROWSEMODE );
	}

	
	FillBlockFormatCombo();
}


void CTridentReadMessageView::OnEditMode()
{
	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) )
	{
		ExecCommand( IDM_EDITMODE );	
	}
	else
	{
		ExecCommand( IDM_BROWSEMODE );
	}

#ifdef OK1
	IUnknown* lpUnk = m_pSite->GetObjectUnknown();

	if ( lpUnk != NULL )
	{
		HRESULT         hr = 0;

		IOleControl* pOleControl= NULL;
		hr = lpUnk->QueryInterface(IID_IOleControl, (void **)&pOleControl);

		if ( SUCCEEDED( hr ) )
		{
			hr = pOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_USERMODE);
		}
	}
#endif
}


void CTridentReadMessageView::OnUpdateEditMode(CCmdUI* pCmdUI) 
{
	if( ( m_pParentFrame != NULL ) &&
		(	( m_pParentFrame->GetMajorVersion() > 4 ) ||
			(	( m_pParentFrame->GetMajorVersion() == 4 ) && 
				( m_pParentFrame->GetMinorVersion() >= 71 ) ) ) )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}


void CTridentReadMessageView::OnFilePrintPreview()
{
}


BOOL CTridentReadMessageView::IsEditing()
{
	return ( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) != 0 );
}


void CTridentReadMessageView::OnEditCheckspelling() 
{
#ifdef OLDSTUFF
#ifdef OLDSTUFF
	HRESULT hr;
	CComVariant v;
	IHTMLDocument2* pDoc = NULL;
	IHTMLBodyElement* pBody = NULL;
	IHTMLTxtRange* pRange = NULL;
	IHTMLTxtRange* pSrcRange = NULL;
	CBstr type;
	CBstr text;
	
	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if (!pDoc) return;

	// Get selection
	hr = pDoc->get_body( ( IHTMLElement ** ) &pBody );

	if( SUCCEEDED( hr ) &&  pBody ) 
	{
		// Get type
//			hr = pSel->get_type(&type);

//			if ( ( S_OK == hr )  && type )
//			{
//				CString strType = type;

			// If type isnt text, bail
//				if( strType == CRString( IDS_TEXT ) )
//				{

				// Create range
				hr = pBody->createTextRange( &pRange );
				
				if( ( S_OK == hr ) && pRange )
				{
					// Get text
				//		if (bHtml)
				//			hr = pRange->get_htmlText(&text);
				//		else
							hr = pRange->get_text( BSTRARG( text ) );
					if ( S_OK == hr && ( ( ( BSTR) text ) != NULL ) )
					{
						AfxMessageBox
					}

					pRange->Release();
				}
		pBody->Release();
	}
#else
	IHTMLElementCollection* pColl;
	hr = pDoc->get_all( &pColl );
	if (hr == S_OK)
	{
		LONG celem;
		hr = pColl->get_length( &celem );

		if ( hr == S_OK )
		{
			for ( int i=0; i< celem; i++ )
			{
				VARIANT varIndex;
				varIndex.vt = VT_UINT;
				varIndex.lVal = i;
				VARIANT var2;
				VariantInit( &var2 );
				IDispatch* pDisp; 

				hr = pColl->item( varIndex, var2, &pDisp );

				if ( hr == S_OK )
				{
					IHTMLElement* pElem;

					hr = pDisp->QueryInterface( IID_IHTMLElement, (void **)&pElem );

					if ( hr == S_OK )
					{

						BSTR bstr;
						hr = pElem->get_tagName(&bstr);
						CString strTag = bstr;
						VARIANT_BOOL	vb;
						
						hr = pElem->get_isTextEdit( &vb );

						if( ( hr == S_OK ) && ( vb == VARIANT_TRUE ) && ( strTag.CompareNoCase( CRString( IDS_HTML_BODY ) ) == 0 ) )
						{			
							hr = pDisp->QueryInterface( IID_IHTMLBodyElement, (void **)&pBody );
						
							if( ( hr == S_OK ) && pBody ) 
							{
								hr = pBody->createTextRange ( &pSrcRange ) ;

								if( ( hr == S_OK ) && pSrcRange ) 
								{
									pSrcRange->duplicate( &pRange );
									pRange->get_text( &bstr );
									
									strTag = bstr;
									long i = strTag.Find( "August 01, 1997" );
									wchar_t	szCharacter[] = L"character";
									bstr = ::SysAllocString( szCharacter );
									BSTR bstr2;
									i += 5; // ???
									pRange->moveStart( bstr, i, &i );
									pRange->get_text( &bstr2 );
									strTag = bstr2;
									i =  strTag.GetLength();
									i = -1 * ( i - 16 + 5 );
									pRange->move( bstr, i, &i );

									::SysFreeString( bstr );

									pRange->get_text( &bstr2 );
									strTag = bstr2;

									wchar_t	szChanged[] = L"Changed this";
									bstr = ::SysAllocString( szChanged );
									pRange->put_text( bstr );
									::SysFreeString( bstr );

									pRange->Release();
									pSrcRange->Release();
								}
								
								pBody->Release();
							}
						}

//							IHTMLImgElement* pImgElem;
//							hr = pDisp->QueryInterface( IID_IHTMLImgElement, (void **)&pImgElem );
//							if ( hr == S_OK )
//							{
//								pImgElem->get_href(&bstr);
//								strTag += " - ";
//								strTag += bstr;
//								pImgElem->Release();
//							}

//							m_listTags.AddString( strTag );
						
						pElem->Release();
					}
					pDisp->Release();
				}
			}
		}

		pColl->Release();
	}

#endif
#endif

}


void CTridentReadMessageView::OnUpdateEditCheckspelling(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( FALSE );
}


void CTridentReadMessageView::OnEditTextFont() 
{
	LOGFONT		lf;
	CString		szString;
	VARIANT		v;
	DWORD		dwStatus;

	USES_CONVERSION;

	memset( &lf, 0, sizeof( LOGFONT ) );
	
	lf.lfHeight = 17 ;

	dwStatus = GetCommandStatus( IDM_BOLD );	
	lf.lfWeight = ( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED ) ? 700 : 400; 
	dwStatus = GetCommandStatus( IDM_ITALIC );
	lf.lfItalic = ( UCHAR ) ( ( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED ) ? TRUE : FALSE );   
	dwStatus = GetCommandStatus( IDM_UNDERLINE );
	lf.lfUnderline = ( UCHAR ) ( ( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED ) ? TRUE : FALSE );   
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;   
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;   
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;   

	VariantInit( &v );
	ExecCommand( IDM_FONTNAME, NULL, &v );

	if( v.vt & VT_BSTR )
	{
		szString = v.bstrVal;
		strncpy( lf.lfFaceName, szString, LF_FACESIZE );
	}

	VariantClear( &v );

	CFaceNameDialog theDlg( &lf );

	if( theDlg.DoModal() != IDOK )
	{
		return;
	}

	SetFgColor( theDlg.GetColor() );

	ExecCommand( IDM_BOLD );
	ExecCommand( IDM_ITALIC );
	ExecCommand( IDM_UNDERLINE );

	dwStatus = GetCommandStatus( IDM_BOLD );
	
	if( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED )
	{
		ExecCommand( IDM_BOLD );
	}
	
	if( theDlg.IsBold() )
	{
		ExecCommand( IDM_BOLD );
	}
	
	dwStatus = GetCommandStatus( IDM_ITALIC );
	if( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED )
	{
		ExecCommand( IDM_ITALIC );
	}

	if( theDlg.IsItalic() )
	{
		ExecCommand( IDM_ITALIC );
	}

	dwStatus = GetCommandStatus( IDM_UNDERLINE );
	if( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED )
	{
		ExecCommand( IDM_UNDERLINE );
	}

	if( theDlg.IsUnderline() )
	{
		ExecCommand( IDM_UNDERLINE );
	}

	if( strnicmp ( lf.lfFaceName, szString, LF_FACESIZE ) )
	{
		VariantInit( &v );
		V_VT(&v) = VT_BSTR;
		V_BSTR(&v) = A2BSTR( lf.lfFaceName );
		
		ExecCommand( IDM_FONTNAME, &v );

		VariantClear( &v );
	}

	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateEditTextFont(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) );
}


void CTridentReadMessageView::OnEditTextPlain() 
{
	DWORD	dwStatus;

	dwStatus = GetCommandStatus( IDM_BOLD );
	if( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED )
	{
		ExecCommand( IDM_BOLD );
	}
	
	dwStatus = GetCommandStatus( IDM_ITALIC );
	if( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED )
	{
		ExecCommand( IDM_ITALIC );
	}

	dwStatus = GetCommandStatus( IDM_UNDERLINE );
	if( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED )
	{
		ExecCommand( IDM_UNDERLINE );
	}

	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateEditTextPlain(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) );
}


void CTridentReadMessageView::DoBlockFmtChange(
LPCSTR szBlockFmt )
{
	VARIANT				v;

	USES_CONVERSION;

	VariantInit( &v );
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v) = A2BSTR( szBlockFmt );
	
	ExecCommand( IDM_BLOCKFMT, &v );

	VariantClear( &v );

	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::UpdateBlockFmt(
CCmdUI* pCmdUI,
LPCSTR	szBlockFmt ) 
{
	CString				szString;
	VARIANT				v;

	USES_CONVERSION;

	if(  m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		pCmdUI->Enable( FALSE );
		return;
	}

	pCmdUI->Enable( TRUE );
	
	VariantInit( &v );

	v.vt = VT_BSTR;
	v.bstrVal = A2BSTR( szBlockFmt );

	ExecCommand( IDM_BLOCKFMT, NULL, &v );

	pCmdUI->SetCheck( FALSE );

	if( v.vt & VT_BSTR )
	{
		szString = v.bstrVal;
		
		if( szString.CompareNoCase( szBlockFmt ) == 0 )
		{
			pCmdUI->SetCheck( TRUE );
		}
	}

	VariantClear( &v );
}


void CTridentReadMessageView::OnBlahBlahBlah()
{
	HRESULT							hr;
	IHTMLDocument2*					pDoc;
	IHTMLStyleSheetsCollection*		pStyleSheetCollection;
	IHTMLRuleStyle*					pRule;
	IHTMLStyleSheetRule*			pStyleSheetRule;
	IHTMLStyleSheet*				pStyleSheet;
	IHTMLStyleSheetRulesCollection*	pStyleSheetRulesCollection;
	LONG							lCount;
	LONG							lIndex;
	LONG							lRuleCount;
	LONG							lRuleIndex;
	VARIANT							varIndex;
	VARIANT							varResult;
	CBstr							cbstr;
	CString							szText;
	CRString						szSpanTabooHeader( IDS_SPAN_TABOOHEADER );
	CRString						szNone( IDS_NONE );
	
	USES_CONVERSION;

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if ( !pDoc ) 
		return;

	pStyleSheetCollection = NULL;
	hr = pDoc->get_styleSheets( &pStyleSheetCollection );

	if ( ( hr != S_OK ) || ( pStyleSheetCollection == NULL ) ) 
	{
		pDoc->Release();
		return;
	}

	lCount = 0;

	hr = pStyleSheetCollection->get_length( &lCount );

	if( ( hr != S_OK ) || ( lCount == 0 ) )
	{
		pStyleSheetCollection->Release();
		pDoc->Release();
		return;
	}


	for( lIndex = 0; lIndex < lCount; lIndex ++ )
	{
		if ( hr != S_OK )
		{
			break;
		}

		varIndex.vt = VT_UINT;
		varIndex.lVal = lIndex;
		VariantInit( &varResult );

		hr = pStyleSheetCollection->item( &varIndex, &varResult );

		if ( ( hr != S_OK ) || ( varResult.vt != VT_DISPATCH ) )
		{
			if( hr == S_OK )
			{
				VariantClear( &varResult );
			}

			break;
		}
				
		pStyleSheet = NULL;

		hr = varResult.pdispVal->QueryInterface( IID_IHTMLStyleSheet, (void **)&pStyleSheet );
		
		if ( ( hr != S_OK ) || ( pStyleSheet == NULL ) )
		{
			VariantClear( &varResult );
			break;
		}
		
		pStyleSheetRulesCollection = NULL;
		hr = pStyleSheet->get_rules( &pStyleSheetRulesCollection );

		if ( ( hr != S_OK ) || ( pStyleSheetRulesCollection == NULL ) )
		{
			pStyleSheet->Release();
			VariantClear( &varResult );
			break;
		}
		
		lRuleCount = 0;
		hr = pStyleSheetRulesCollection->get_length( &lRuleCount );					
		
		if( hr != S_OK )
		{
			pStyleSheetRulesCollection->Release();
			pStyleSheet->Release();
			VariantClear( &varResult );
			break;
		}

		for( lRuleIndex = 0; lRuleIndex < lRuleCount; lRuleIndex ++ )
		{
			pStyleSheetRule = NULL;

			hr = pStyleSheetRulesCollection->item( lRuleIndex, &pStyleSheetRule );

			if( ( hr != S_OK ) || ( pStyleSheetRule == NULL ) )
			{
				break;
			}

			hr =  pStyleSheetRule->get_selectorText( BSTRARG( cbstr ) );
			szText = cbstr;

			if( ( hr != S_OK ) || cbstr.IsNull()  )
			{
				pStyleSheetRule->Release();
				continue;
			}


			if( szText.CompareNoCase( szSpanTabooHeader ) )
			{
				pStyleSheetRule->Release();
				continue;
			}

			pRule = NULL;

			hr = pStyleSheetRule->get_style( &pRule );
			
			if( ( hr != S_OK ) || ( pRule == NULL ) )
			{
				pStyleSheetRule->Release();
				continue;
			}

			if( m_pParentFrame->GetCheck( ID_BLAHBLAHBLAH ) == FALSE )
			{
				// we're going from blahblahblah view back to normal
				cbstr = A2BSTR( szNone );
			}
			else
			{
				cbstr = A2BSTR( "" );
			}

			hr = pRule->put_display( cbstr );
			
			pRule->Release();
			pStyleSheetRule->Release();
			lCount = 0; // end the outer loop 
		}

		pStyleSheetRulesCollection->Release();
		pStyleSheet->Release();
		VariantClear( &varResult );
	}

	GetDocument()->m_Sum->ToggleFlag(MSF_SHOW_ALL_HEADERS);

	pStyleSheetCollection->Release();
}


void CTridentReadMessageView::OnFontChange()
{
	CComboBox*			pCombo;
	CString				szString;
	VARIANT				v;

	USES_CONVERSION;

	SetFocus();

	VERIFY( pCombo = ( CComboBox* ) ( m_pParentFrame->GetFontCombo() ) );
	
	if( pCombo->GetCurSel() < 0 )
	{
		return;
	}

	pCombo->GetLBText( pCombo->GetCurSel(), szString );

	VariantInit( &v );
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v) = A2BSTR( szString );
	
	ExecCommand( IDM_FONTNAME, &v );

	VariantClear( &v );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnUpdateFont(CCmdUI* pCmdUI) 
{
	CComboBox*			pCombo;
	CString				szString;
	VARIANT				v;
	INT					iNew;
	INT					iCurrent;
	
	USES_CONVERSION;

	VERIFY( pCombo = ( CComboBox* ) ( m_pParentFrame->GetFontCombo() ) );
	
	if( pCombo->GetDroppedState() == FALSE )
	{
		VariantInit( &v );
		ExecCommand( IDM_FONTNAME, NULL, &v );

		if( v.vt & VT_BSTR )
		{
			szString = v.bstrVal;
			iCurrent = pCombo->GetCurSel();
			
			iNew = pCombo->FindStringExact( -1, szString );
			
			if( iNew != iCurrent )
			{
				pCombo->SetCurSel( iNew );
			}
		}

		VariantClear( &v );			
	}

	pCmdUI->Enable( TRUE );
}


void CTridentReadMessageView::OnEditTextSizeHumongous() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 7;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnEditTextSizeVeryLarge() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 6;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnEditTextSizeLarger() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 5;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnEditTextSizeLarge() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 4;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}

void CTridentReadMessageView::OnEditTextSizeMedium() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 3;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnEditTextSizeSmall() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 2;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}


void CTridentReadMessageView::OnEditTextSizeVerySmall() 
{
	VARIANT vaFontSize; // input arguments

	VariantInit(&vaFontSize);

	V_VT(&vaFontSize) = VT_I4;
	V_I4(&vaFontSize) = 1;

	ExecCommand( IDM_FONTSIZE, &vaFontSize );
	GetDocument()->SetModifiedFlag();
}

void CTridentReadMessageView::OnColor0() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR0 ) );
}

void CTridentReadMessageView::OnColor1() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR1 ) );
}

void CTridentReadMessageView::OnColor2() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR2 ) );
}

void CTridentReadMessageView::OnColor3() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR3 ) );
}

void CTridentReadMessageView::OnColor4() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR4 ) );
}

void CTridentReadMessageView::OnColor5() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR5 ) );
}

void CTridentReadMessageView::OnColor6() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR6 ) );
}

void CTridentReadMessageView::OnColor7() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR7 ) );
}

void CTridentReadMessageView::OnColor8() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR8 ) );
}

void CTridentReadMessageView::OnColor9() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR9 ) );
}

void CTridentReadMessageView::OnColor10() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR10 ) );
}

void CTridentReadMessageView::OnColor11() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR11 ) );
}

void CTridentReadMessageView::OnColor12() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR12 ) );
}

void CTridentReadMessageView::OnColor13() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR13 ) );
}

void CTridentReadMessageView::OnColor14() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR14 ) );
}

void CTridentReadMessageView::OnColor15() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR15 ) );
}

void CTridentReadMessageView::OnColor16() 
{
	SetFgColor( CColorMenu::GetColor( ID_COLOR16 ) );
}


BOOL CTridentReadMessageView::DoFindFirst( 
const CString&	szSearch, 
BOOL			bMatchCase, 
BOOL			bWholeWord, 
BOOL			bSelect )
{
	HRESULT				hr;
	IHTMLBodyElement*	pBody;
	IHTMLTxtRange*		pRange;
	IHTMLTxtRange*		pNextRange;
	BOOL				bReturn;
	VARIANT_BOOL		vbFound;
	CBstr				cbstr;
	CBstr				cbstrSearch;
	CBstr				cbstrStartToEnd( IDS_START_TO_END );
	MSG					msg;
	BOOL				bDone;
	IHTMLElement*		pParent;
	CString				strTag;
	LONG				lFlags;

	USES_CONVERSION;

	if( bSelect )
	{
		while( ( !IsReady() || ( m_nIsVisible < 2 ) ) && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
			{
				// we have to wait until the trident control is
				// is ready to go before we can do anything with it.

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
	}
	else
	{
		while( !IsReady() && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			// we have to wait until the trident control is
			// is ready to go before we can do anything with it.

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	m_bstrBookmark = NULL;
	bReturn = FALSE;

	cbstrSearch = A2BSTR( szSearch );

	if( ( ( BSTR ) cbstrSearch ) == NULL )
	{
		return FALSE;
	}

	// Get document
	pBody = m_pSite->GetBody();
	
	if( !pBody ) 
	{
		return bReturn;
	}

	pNextRange = NULL;
	pRange = NULL;
	
	// Create range
	hr = pBody->createTextRange( &pNextRange );

	if( ( hr == S_OK ) && pNextRange )
	{
		do
		{
			bDone = TRUE;
		
			if( pRange )
			{
				pRange->Release();
				pRange = NULL;
			}

			hr = pNextRange->duplicate( &pRange );
		
			ASSERT( pRange );

			vbFound = VARIANT_FALSE;
			lFlags = ( ( bMatchCase ? SEARCHFLAG_MATCHCASE : 0 ) | ( bWholeWord ? SEARCHFLAG_MATCHWHOLEWORD : 0 ) );

			hr = pRange->findText( cbstrSearch, 0, lFlags, &vbFound );
			
#pragma warning(disable : 4310)			
			if( ( vbFound == VARIANT_TRUE ) && 
				bSelect && 
				( m_pParentFrame->GetCheck( ID_BLAHBLAHBLAH ) == FALSE ) ) 
#pragma warning(default : 4310)
			{
				// make sure it's not in a taboo header

				pParent = NULL;
				
				hr = pRange->parentElement( &pParent );
				
				if( ( hr == S_OK ) && pParent )
				{
					hr = pParent->get_tagName( BSTRARG( cbstr ) );
					strTag = cbstr;

					if( strTag.CompareNoCase( CRString( IDS_HTML_SPAN ) ) == 0 )
					{
						hr = pParent->get_className( BSTRARG( cbstr ) );
						strTag = cbstr;

						if( strTag.CompareNoCase( CRString( IDS_TABOOHEADER ) ) == 0 )
						{
							// it's in a taboo header
							bDone = FALSE;
						}
					}
					pParent->Release();
				}
			}
			
#pragma warning(disable : 4310)			
			if( vbFound == VARIANT_TRUE ) 
#pragma warning(default : 4310)
			{
				// set the new range
				hr = pNextRange->setEndPoint( cbstrStartToEnd, pRange );
			}

		}
		while( !bDone );

#pragma warning(disable : 4310)
		if( vbFound == VARIANT_TRUE )
#pragma warning(default : 4310)
		{
			bReturn = TRUE;
			
			if( bSelect ) 
			{
				pRange->select();
#pragma warning(disable : 4310)
				pRange->scrollIntoView( VARIANT_TRUE );
#pragma warning(default : 4310)
			}
								
			hr = pNextRange->getBookmark( &m_bstrBookmark );
		}
		
		if( pNextRange )
		{
			pNextRange->Release();
		}

		if( pRange )
		{
			pRange->Release();
		}
	}

	pBody->Release();
	return bReturn;
}



BOOL CTridentReadMessageView::DoFindNext( 
const CString&	szSearch, 
BOOL			bMatchCase, 
BOOL			bWholeWord, 
BOOL			bSelect )
{
	HRESULT				hr;
	IHTMLBodyElement*	pBody;
	IHTMLTxtRange*		pRange;
	IHTMLTxtRange*		pNextRange;
	BOOL				bReturn;
	VARIANT_BOOL		vbFound;
	CBstr				cbstr;
	CBstr				cbstrSearch;
	CBstr				cbstrStartToEnd( IDS_START_TO_END );
	BOOL				bDone;
	IHTMLElement*		pParent;
	CString				strTag;
	LONG				lFlags;

	USES_CONVERSION;

	if( m_bstrBookmark == NULL )
	{
		return DoFindFirst( szSearch, bMatchCase, bWholeWord, bSelect );
	}

	cbstrSearch = A2BSTR( szSearch );

	if( ( ( BSTR ) cbstrSearch ) == NULL )
	{
		return FALSE;
	}

	bReturn = FALSE;
		
	// Get body
	pBody = m_pSite->GetBody();
	
	if( !pBody ) 
	{
		return bReturn;
	}

	pRange = NULL;
	pNextRange = NULL;

	// Create range
	hr = pBody->createTextRange( &pNextRange );
	pBody->Release();

	if( ( S_OK == hr ) && pNextRange )
	{
		vbFound = VARIANT_FALSE;

		hr = pNextRange->moveToBookmark ( m_bstrBookmark , &vbFound ); 					
		m_bstrBookmark = NULL;
		
		if( !vbFound )
		{
			pNextRange->Release();
			return FALSE;
		}


		do
		{	
			bDone = TRUE;
			vbFound = VARIANT_FALSE;

			if( pRange )
			{
				pRange->Release();
				pRange = NULL;
			}

			hr = pNextRange->duplicate( &pRange );
			
			if( ( hr == S_OK ) && pRange )
			{
				lFlags = ( ( bMatchCase ? SEARCHFLAG_MATCHCASE : 0 ) | ( bWholeWord ? SEARCHFLAG_MATCHWHOLEWORD : 0 ) );
			
				hr = pRange->findText( cbstrSearch, 0, lFlags, &vbFound );

#pragma warning(disable : 4310)
				if( ( vbFound == VARIANT_TRUE ) &&
					bSelect && 
					( m_pParentFrame->GetCheck( ID_BLAHBLAHBLAH ) == FALSE ) ) 
#pragma warning(default : 4310)
				{
					// make sure it's not in a taboo header

					pParent = NULL;
					
					hr = pRange->parentElement( &pParent );
					
					if( ( hr == S_OK ) && pParent )
					{
						hr = pParent->get_tagName( BSTRARG( cbstr ) );
						strTag = cbstr;
	
						if( strTag.CompareNoCase( CRString( IDS_HTML_SPAN ) ) == 0 )
						{
							hr = pParent->get_className( BSTRARG( cbstr ) );
							strTag = cbstr;
		
							if( strTag.CompareNoCase( CRString( IDS_TABOOHEADER ) ) == 0 )
							{
								// it's in a taboo header
								bDone = FALSE;
							}
						}
						pParent->Release();
					}
				}
			}

#pragma warning(disable : 4310)
			if( vbFound == VARIANT_TRUE ) 
#pragma warning(default : 4310)
			{
				// set the new range
				hr = pNextRange->setEndPoint( cbstrStartToEnd, pRange );
			}
		}
		while( !bDone );

#pragma warning(disable : 4310)
		if( vbFound == VARIANT_TRUE )
#pragma warning(default : 4310)
		{
			bReturn = TRUE;

			if( bSelect )
			{
				pRange->select();
#pragma warning(disable : 4310)
				pRange->scrollIntoView( VARIANT_TRUE );
#pragma warning(default : 4310)
			}
								
			hr = pNextRange->getBookmark( &m_bstrBookmark );
		}

		if( pNextRange )
		{
			pNextRange->Release();
		}
		
		if( pRange )
		{
			pRange->Release();
		}
	}

	return bReturn;
}


void CTridentReadMessageView::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE );
}


void CTridentReadMessageView::SaveInfo() 
{
	HRESULT					hr;
	IDispatch*				pDispatch; 	
	IHTMLDocument2*			pDoc;
	IHTMLElement*			pElement;
	IHTMLBodyElement*		pBody;
	IHTMLTxtRange*			pTempRange;
	IHTMLTxtRange*			pRange;
	IHTMLTxtRange*			pHeaderRange;
	IHTMLElement*			pSpanElement;
	CBstr					cbstr;
	CString					szText;
	CString					szText2;
	IHTMLElementCollection* pCollection;
	LONG					lIndex;
	VARIANT					varIndex;
	VARIANT					var;
	LONG					lCount;
	CBstr					cbstrEndToStart( IDS_END_TO_START );
	CBstr					cbstrStartToStart( IDS_START_TO_START );
	CBstr					cbstrStartToEnd( IDS_START_TO_END );
	CString					szHeaders;
	CString					szBody;
	VARIANT_BOOL			vb;
	CReadMessageDoc*		pMessageDoc;
	CRString				szBodyTag( IDS_HTML_BODY );
	CRString				szSpanTag( IDS_HTML_SPAN );
	CRString				szEudoraHeader( IDS_EUDORAHEADER );
	CRString				szTabooHeader( IDS_TABOOHEADER );

	pMessageDoc = ( CReadMessageDoc* ) GetDocument();
	ASSERT_KINDOF( CReadMessageDoc, pMessageDoc );
	
	USES_CONVERSION;

	pDoc = m_pSite->GetDoc2();
	
	if( !pDoc ) 
	{
		return;
	}

	pRange = NULL;
	pHeaderRange = NULL;
	pSpanElement = NULL;
	
	hr = pDoc->get_all( &pCollection );

	if( ( hr == S_OK ) && pCollection )
	{
		hr = pCollection->get_length( &lCount );

		if ( hr == S_OK )
		{
			for ( lIndex = 0; lIndex < lCount; lIndex++ )
			{
				varIndex.vt = VT_UINT;
				varIndex.lVal = lIndex;

				VariantInit( &var );

				hr = pCollection->item( varIndex, var, &pDispatch );

				if( ( hr == S_OK ) && pDispatch ) 
				{
					hr = pDispatch->QueryInterface( IID_IHTMLElement, (void **)&pElement );

					if( ( hr == S_OK ) && pElement )
					{
						hr = pElement->get_tagName( BSTRARG( cbstr ) );
						szText = cbstr;
						
						if( pRange == NULL )
						{
							// haven't found the body tag yet
							if ( szText.CompareNoCase( szBodyTag ) == 0 ) 
							{
								pBody = NULL;
								hr = pElement->QueryInterface( IID_IHTMLBodyElement, (void **)&pBody );
								ASSERT( pBody );
								hr = pBody->createTextRange( &pRange );
								ASSERT( pRange );
								pBody->Release();
								pRange->moveToElementText( pElement );
							}
						}
						else if( szText.CompareNoCase( szSpanTag ) == 0 )
						{
							if( pSpanElement )
							{
								pSpanElement->Release();
								pSpanElement = NULL;
							}

							// see if it's a header span
							hr = pElement->get_className( BSTRARG( cbstr ) );
							szText = cbstr;

							if( szText.CompareNoCase( szEudoraHeader ) &&
								szText.CompareNoCase( szTabooHeader ) )
							{
								// not in the headers anymore
								// create the header & body range								
								hr = pRange->duplicate( &pHeaderRange );
								hr = pRange->duplicate( &pTempRange );
								hr = pTempRange->moveToElementText( pElement );
								hr = pHeaderRange->setEndPoint( cbstrEndToStart, pTempRange );
								hr = pRange->setEndPoint( cbstrStartToStart, pTempRange );
								pTempRange->Release();

								// bail
								lCount = 0;
							}
							else
							{
								pSpanElement = pElement;
								pElement = NULL;
							}
						}
						else
						{
							// see if the parent is a header span
							if( pSpanElement )
							{
								hr = pSpanElement->contains( pElement, &vb );
#pragma warning(disable : 4310)
								if( vb != VARIANT_TRUE )
#pragma warning(default : 4310)
								{
									pSpanElement->Release();
									pSpanElement = NULL;
								}
							}

							if( pSpanElement == NULL )
							{
								// we aren't in a header span
								// create the header & body range

								hr = pRange->duplicate( &pHeaderRange );
								hr = pRange->duplicate( &pTempRange );
								hr = pTempRange->moveToElementText( pElement );
								hr = pHeaderRange->setEndPoint( cbstrEndToStart, pTempRange );
								
								if( szText.CompareNoCase( CRString( IDS_HTML_BREAK ) ) == 0 )
								{
									// skip the br -- it will be added to the 
									// header later
									hr = pRange->setEndPoint( cbstrStartToEnd, pTempRange );
								}
								else
								{
									hr = pRange->setEndPoint( cbstrStartToStart, pTempRange );
								}
								
								pTempRange->Release();

								// bail
								lCount = 0;
							}
						}	
								
						if( pElement )
						{
							pElement->Release();
						}
					}
					
					pDispatch->Release();
				}
			}
		}
	}

	if( pSpanElement )
	{
		pSpanElement->Release();
	}

	pCollection->Release();

	szHeaders = "";
	szBody = "";

	if( pHeaderRange )
	{
		hr = pHeaderRange->get_text( BSTRARG( cbstr ) );
		szHeaders = cbstr;
		pHeaderRange->Release();
	}
	
	if( pRange )
	{
		hr = pRange->get_htmlText( BSTRARG( cbstr ) );
		szBody = cbstr;
		pRange->Release();
	}

	// convert nbsp's back to regular spaces
	while( ( lIndex = szHeaders.Find( ( wchar_t ) 160 ) ) >= 0 )
	{
		szHeaders.SetAt( lIndex, ' ' );
	}

	if( strnicmp( szBody, "<html>", 8 ) )
	{
		szBody = "<html>" + szBody + "</html>";
	}

	// make sure there's a blank line between the headers and the body

	if ( ( szHeaders != "" ) && ( szHeaders.Right( 4 ) != "\r\n\r\n" ) )
	{
		if( szHeaders.Right( 2 ) != "\r\n" ) 
		{
			szHeaders += "\r\n";
		}
		
		szHeaders += "\r\n";
	}


	szHeaders += szBody;

//	CFile theFile( "c:\\test.txt", CFile::modeReadWrite | CFile::modeCreate );
//	theFile.Write( szHeaders, szHeaders.GetLength() );
//	theFile.Close();

	pMessageDoc->SetText( szHeaders );
	pMessageDoc->m_Sum->SetFlag(MSF_XRICH);
	pMessageDoc->m_Sum->SetFlagEx( MSFEX_HTML );
}



void CTridentReadMessageView::FillBlockFormatCombo()
{											
#ifdef OK1	
	CComboBox*	pCombo;
	HRESULT		hr;
    VARIANTARG	varRange;
    LONG		lLBound, lUBound, lIndex;
    BSTR		bstr;
    SAFEARRAY*	psa = NULL;

	pCombo = m_pParentFrame->GetBlockFmtCombo();

	if( pCombo == NULL )
	{
		return;
	}

    varRange.vt = VT_ARRAY;
    varRange.parray = NULL;

	hr = m_pSite->GetCommandTarget()->Exec(
            &CMDSETID_Forms3,
            IDM_GETBLOCKFMTS,
            MSOCMDEXECOPT_DONTPROMPTUSER,
            NULL,
            &varRange);

    if( FAILED( hr ) )
	{
       return;
	}

	while ( pCombo->GetCount() )
	{
		pCombo->DeleteString( 0 );
	}

    psa = V_ARRAY(&varRange);
    SafeArrayGetLBound(psa, 1, &lLBound);
    SafeArrayGetUBound(psa, 1, &lUBound);

    for (lIndex=lLBound; lIndex<=lUBound; lIndex++)
    {
        SafeArrayGetElement(psa, &lIndex, &bstr);

		CString str(bstr);

		pCombo->AddString( str );

       ::SysFreeString(bstr);
 		
    }

    if(psa)
        SafeArrayDestroy(psa);
#endif
}


BOOL CTridentReadMessageView::GetAllText( CString& szText )
{
    szText.Empty();

    // HTML to Text translation is not really symmetric in Trident
    // So if the document has not been modified then return the text
    // from the message doc ONLY if it was originally a plain text message

    CReadMessageDoc* pDoc = (CReadMessageDoc*) GetDocument();

    if( pDoc != NULL ) {
        if (!( pDoc->IsModified()) && 
            !(pDoc->m_Sum->IsXRich()) && !pDoc->m_Sum->IsHTML() ) {

            // make sure the message is read up
			char* FMBuffer = pDoc->GetFullMessage(RAW);
            szText = FMBuffer;
			delete [] FMBuffer;
            return TRUE;
        }
    }

    return CTridentView::GetAllText( szText );
}


BOOL CTridentReadMessageView::SetSelectedText(
const char* szText,
bool bSign )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLSelectionObject*	pSel;
	IHTMLTxtRange*			pRange;
	IHTMLTxtRange*			pBodyRange;
	CBstr					cbstrType;
	CBstr					cbstrText;
	CBstr					cbstr;
	BOOL					bReturn;
	BOOL					bDone;
	IHTMLElementCollection*	pCollection;
	IHTMLElement*			pElement;
	IDispatch*				pDispatch;
	VARIANT					varIndex;
	VARIANT					var;
	VARIANT_BOOL			vb;
	LONG					lCount;
	LONG					lIndex;
	CRString				szBody( IDS_HTML_BODY );
	CString					szTemp;

	USES_CONVERSION;

	pRange = NULL;
	pBodyRange = NULL;
	pSel = NULL;
	pDoc = NULL;
	bReturn = FALSE;
	bDone = FALSE;

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if (!pDoc) 
	{
		return FALSE;
	}

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( !SUCCEEDED( hr ) || !pSel ) 
	{
		return FALSE;
	}

	// Create range
	hr = pSel->createRange((IDispatch**)&pRange);

	if( !SUCCEEDED( hr ) || !pRange ) 
	{
		pSel->Release();
		return FALSE;
	}

	hr = pRange->duplicate( &pBodyRange );

	// see if it takes up the entire message body
	// note: this is a hack because CTRL-A run plugin caused
	// new text to put inside the taboo header span
	hr = pDoc->get_all( &pCollection );

	if( ( hr == S_OK ) && pCollection )
	{
		hr = pCollection->get_length( &lCount );

		if ( hr == S_OK )
		{
			for ( lIndex = 0; !bDone && ( lIndex < lCount ); lIndex++ )
			{
				varIndex.vt = VT_UINT;
				varIndex.lVal = lIndex;

				VariantInit( &var );

				hr = pCollection->item( varIndex, var, &pDispatch );

				if( ( hr == S_OK ) && pDispatch ) 
				{
					hr = pDispatch->QueryInterface( IID_IHTMLElement, (void **)&pElement );

					if( ( hr == S_OK ) && pElement )
					{
						hr = pElement->get_tagName( BSTRARG( cbstr ) );
						szTemp = cbstr;
						
						if( szTemp.CompareNoCase( szBody ) == 0 )
						{								
							// bail out -- even if bDone == FALSE
							lCount = 0;

							pBodyRange->moveToElementText( pElement );
							
							vb = VARIANT_FALSE;
							hr = pRange->isEqual( pBodyRange, &vb );
							
#pragma warning(disable : 4310)							
							if( SUCCEEDED( hr ) && ( vb == VARIANT_TRUE ) )
#pragma warning(default : 4310)
							{
								szTemp = Text2Html( szText, TRUE, FALSE );
								
								cbstr = A2BSTR( szTemp );
								
								if( ( ( BSTR ) cbstr ) != NULL )
								{
									hr = pElement->put_innerHTML( cbstr );
								
									if ( S_OK == hr )
									{							
										bDone = TRUE;
										GetDocument()->SetModifiedFlag();
									}
								}
							}
						}
					
						pElement->Release();	
					}
				}
			}
		}

		pCollection->Release();
	}
	
	pBodyRange->Release();

	if( bDone )
	{
		pRange->Release();
		pSel->Release();
		return TRUE;
	}

	// Get type
	hr = pSel->get_type( BSTRARG( cbstrType ) );

	if ( ( S_OK == hr )  && ( ( ( BSTR ) cbstrType ) != NULL ) )
	{
		CString strType = cbstrType;

		// If type isnt text, bail
		if( ( strType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 ) || ( strType.CompareNoCase( CRString( IDS_NONE ) ) == 0 ) )
		{
			cbstrText = A2BSTR( szText );
			
			if( ( ( BSTR ) cbstrText ) != NULL )
			{
				hr = pRange->put_text( cbstrText );
			
				if ( S_OK == hr )
				{							
					bReturn = TRUE;
					GetDocument()->SetModifiedFlag();
				}
			}
		}

		pRange->Release();
	}

	return bReturn;
}


BOOL CTridentReadMessageView::SetAllText(
const char* szText,
bool		bSign )
{
	SetBodyText( szText );
	return TRUE;
}



BOOL CTridentReadMessageView::SetSelectedHTML(
const char* szHTML,
bool bSign )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLSelectionObject*	pSel;
	IHTMLTxtRange*			pRange;
	CBstr					cbstrType;
	CBstr					cbstrText;
	BOOL					bReturn;

	USES_CONVERSION;

	pRange = NULL;
	pSel = NULL;
	pDoc = NULL;
	bReturn = FALSE;

	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if (!pDoc) 
	{
		return FALSE;
	}

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( SUCCEEDED( hr ) &&  pSel ) 
	{
		// Get type
		hr = pSel->get_type( BSTRARG( cbstrType ) );

		if ( ( S_OK == hr )  && ( ( ( BSTR ) cbstrType ) != NULL ) )
		{
			CString strType = cbstrType;

			// If type isnt text, bail
			if( ( strType.CompareNoCase( CRString ( IDS_TEXT ) ) == 0 ) || ( strType.CompareNoCase( CRString( IDS_NONE ) ) == 0 ) )
			{
				// Create range
				hr = pSel->createRange((IDispatch**)&pRange);
				
				if( ( S_OK == hr ) && pRange )
				{
					cbstrText = A2BSTR( szHTML );
					
					if( ( ( BSTR ) cbstrText ) != NULL )
					{
						hr = pRange->pasteHTML( cbstrText );
					
						if ( S_OK == hr )
						{							
							bReturn = TRUE;
							GetDocument()->SetModifiedFlag();
						}
					}

					pRange->Release();
				}
			}
		}

		pSel->Release();
	}

	return bReturn;
}


BOOL CTridentReadMessageView::SetAllHTML(
const char* szHTML,
bool bSign )
{
	HRESULT				hr;
	IHTMLBodyElement*	pBody;
	IHTMLTxtRange*		pRange;
	BOOL				bReturn;
	CBstr				cbstrText;

	USES_CONVERSION;

	bReturn = FALSE;

	// Get body
	pBody = m_pSite->GetBody();
	
	if( !pBody ) 
	{
		return bReturn;
	}

	pRange = NULL;

	// Create range
	hr = pBody->createTextRange( &pRange );

	if( ( S_OK == hr ) && pRange )
	{
		cbstrText = A2BSTR( szHTML );
		
		if( ! cbstrText.IsNull() )
		{
			hr = pRange->pasteHTML( cbstrText );
		
			if ( S_OK == hr )
			{							
				bReturn = TRUE;
				GetDocument()->SetModifiedFlag();
			}
		}

		pRange->Release();
	}

	pBody->Release();
	return bReturn;
}


BOOL CTridentReadMessageView::WriteTempFile( 
CFile& theFile,
CString& szStyleSheetFormat )
{
	CReadMessageDoc*	pDoc;
	char*				szHeader;
	char*				szColon;
	char*				szFullMessage;
	const char*			szBody;
	char*				szEnd;	
	BOOL				bIncludeHeader;
	INT					i;
	const CStringArray*	pTabooHeaderArray;
	CString				szStyleSheet;
	CString				szLine;
	char				cSave;
	CString				szFontName;
	CString				szEudoraHeaderLine;
	CString				szTabooHeaderLine;
	CString				szBRNewLine;
	CString				szEndSpan;
	BOOL				bReturn;
	BOOL				bKludgeStartingSpace;

	pDoc = ( CReadMessageDoc* ) GetDocument();
	
	if( pDoc == NULL )
	{
		return FALSE;
	}
	
	// get the whole message
	szFullMessage = pDoc->GetFullMessage( RAW );

#ifdef IMAP4 // 
	//
	// It's possible for this to fail if the message couldn't be retrieved
	//  from the IMAP server
	//
	if (NULL == szFullMessage) 
	{
		return FALSE;
	}
#endif // IMAP4


	pDoc->ReallySetTitle( pDoc->m_Sum->MakeTitle() );
		
	// get the base font name
	if( m_pParentFrame->UseFixedWidthFont() )
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FIXED_FONT );
	}
	else
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FONT );
	}

	LPCTSTR DisplayValue = GetIniShort(IDS_INI_SHOW_ALL_HEADERS)? "" : "none";
	szStyleSheet.Format( 	szStyleSheetFormat,
							( LPCSTR )szFontName,
							( LPCSTR )GetIniString( IDS_INI_MESSAGE_FIXED_FONT ),
							( LPCSTR )GetIniString( IDS_INI_EXCERPT_BARS ),
							DisplayValue);
	
	bReturn = TRUE;

	try
	{
		theFile.Write( szStyleSheet, szStyleSheet.GetLength() );

		// convert cid's to local file URLs
		MorphMHTML( &szFullMessage );

		pDoc->m_QCMessage.Init( pDoc->m_MessageId, szFullMessage );

		// find the headers.  Wrap <SPAN> tags around each.
		// set each class to EUDORAHEADER or TABOOHEADER depending on whether or not
		// it should be shown in normal mode.

		szHeader = szFullMessage;
		szBody = FindBody( szFullMessage );
		pTabooHeaderArray = & ( pDoc->GetTabooHeaderArray() );
		szEudoraHeaderLine.Format( "<%s CLASS=%s>\r\n", ( LPCSTR ) CRString( IDS_HTML_SPAN ), ( LPCSTR ) CRString( IDS_EUDORAHEADER ) );
		szTabooHeaderLine.Format( "<%s CLASS=%s>\r\n", ( LPCSTR ) CRString( IDS_HTML_SPAN ), ( LPCSTR ) CRString( IDS_TABOOHEADER ) );
		szEndSpan.Format( "</%s>\r\n", ( LPCSTR ) CRString( IDS_HTML_SPAN ) );
		szBRNewLine.Format( "<%s>\r\n", ( LPCSTR ) CRString( IDS_HTML_BREAK ) );
		
		while( szHeader < szBody )
		{
			szColon = szHeader;
			
			while(	( *szColon != ':' ) && 
					( szColon < szBody ) && 
					( *szColon != '\0' ) &&
					( *szColon != '\n' ) )
			{
				szColon ++;
			}
			
			if( ( szColon == szBody ) || ( *szColon != ':' ) )
			{
				// no more headers
				break;
			}
		
			bIncludeHeader = TRUE;
						
			for( i = 0; i < pTabooHeaderArray->GetSize(); i ++ )
			{
				if( !strnicmp( (*pTabooHeaderArray)[i], szHeader, ( ( *pTabooHeaderArray )[i] ).GetLength() ) )
				{
					bIncludeHeader = FALSE;
        			break;
				}
			}

			if( bIncludeHeader )
			{
				// write the span tag
	//			szHeaders += "<SPAN CLASS=EUDORAHEADER>\r\n";
				theFile.Write( szEudoraHeaderLine, szEudoraHeaderLine.GetLength() );
			}
			else
			{
	//			szHeaders += "<SPAN CLASS=TABOOHEADER>\r\n";
				theFile.Write( szTabooHeaderLine, szTabooHeaderLine.GetLength() );
			}

			// write the whole header (it may be on several lines) to file, 
			// replace the \r\n with <BR>'s
			
			do
			{
				bKludgeStartingSpace = ( *szHeader == ' ' );
				szEnd = szHeader;

				while(	( *szEnd != '\0' ) && 
						( *szEnd != '\r' ) &&
						( *szEnd != '\n' ) )
				{
					szEnd ++;
				}
				
				cSave = *szEnd;

				*szEnd = '\0';

				szLine = Text2Html( szHeader, TRUE, FALSE );

				// write the line
	//			szHeaders += szLine;
				if( bKludgeStartingSpace && szLine.GetLength() && ( szLine[0] == ' ' ) )
				{
					szLine = "&nbsp;" + szLine.Right( szLine.GetLength() - 1 );
				}

				theFile.Write( szLine, szLine.GetLength() );
				
				*szEnd = cSave;

				if( *szEnd == 0 )
				{
					break;
				}

	//			szHeaders += "<BR>\r\n";			
				theFile.Write( szBRNewLine, szBRNewLine.GetLength() );

				++szEnd;			
				
				if( ( *szEnd == '\r' ) || ( *szEnd == '\n' ) )
				{
					++szEnd;
				}
				
				szHeader = szEnd;
			}
			while( ( *szHeader == ' ' ) || ( *szHeader == '\t' ) );	// if the line starts with 
																	// a space or tab, it's 
																	// still part of this 
																	// header
			// write the </SPAN>
	//		szHeaders += "</SPAN>\r\n";
			theFile.Write( szEndSpan, szEndSpan.GetLength() );
		}

	//	szHeaders += "<BR>\r\n";
		theFile.Write( szBRNewLine, szBRNewLine.GetLength() );

		// get the body as html and save it to theFile
		CString theBody;
		pDoc->m_QCMessage.GetBodyAsHTML( theBody );
		theFile.Write( theBody, theBody.GetLength() );

		theFile.Flush();
	}
	catch( CException* pExp )
	{
		pExp->Delete();
		bReturn = FALSE;
	}

	delete [] szFullMessage;

	return bReturn;
}


void CTridentReadMessageView::OnBlkfmtBullettedList() 
{
	// Shareware: In reduced feature mode, you cannot insert bullet lists
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		ExecCommand( IDM_UNORDERLIST, NULL, NULL, TRUE );
		GetDocument()->SetModifiedFlag();
	}
}

void CTridentReadMessageView::OnUpdateBlkfmtBullettedList(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot insert bullet lists
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		DWORD dwStatus = 0;

		if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
		{
			// we're not in editing mode
			pCmdUI->Enable( FALSE );
			return;
		}

		dwStatus = GetCommandStatus( IDM_UNORDERLIST );
		pCmdUI->SetCheck( (dwStatus & OLECMDF_LATCHED) == OLECMDF_LATCHED );
		pCmdUI->Enable( (dwStatus & OLECMDF_ENABLED) == OLECMDF_ENABLED );
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE);
	}
}


void CTridentReadMessageView::OnBlkfmtH1() 
{
	DoBlockFmtChange( "Heading 1" );
}

void CTridentReadMessageView::OnUpdateBlkfmtH1(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI, "Heading 1" );
}

void CTridentReadMessageView::OnBlkfmtH2() 
{
	DoBlockFmtChange( "Heading 2" );
}

void CTridentReadMessageView::OnUpdateBlkfmtH2(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI, "Heading 2" );
}

void CTridentReadMessageView::OnBlkfmtH3() 
{
	DoBlockFmtChange( "Heading 3" );
}

void CTridentReadMessageView::OnUpdateBlkfmtH3(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI, "Heading 3" );
}

void CTridentReadMessageView::OnBlkfmtH4() 
{
	DoBlockFmtChange( "Heading 4" );
}

void CTridentReadMessageView::OnUpdateBlkfmtH4(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI, "Heading 4" );
}

void CTridentReadMessageView::OnBlkfmtH5() 
{
	DoBlockFmtChange( "Heading 5" );
}

void CTridentReadMessageView::OnUpdateBlkfmtH5(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI, "Heading 5" );
}

void CTridentReadMessageView::OnBlkfmtH6() 
{
	DoBlockFmtChange( "Heading 6" );
}

void CTridentReadMessageView::OnUpdateBlkfmtH6(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI, "Heading 6" );
}

void CTridentReadMessageView::OnBlkfmtNormal() 
{
	DoBlockFmtChange( CRString( IDS_NORMAL ) );
}

void CTridentReadMessageView::OnUpdateBlkfmtNormal(CCmdUI* pCmdUI) 
{
	UpdateBlockFmt( pCmdUI,  CRString( IDS_NORMAL ) );
}

void CTridentReadMessageView::OnEditInsertLink() 
{
	// Shareware: In reduced feature mode, you cannot insert hyperlinks
	if (!UsingFullFeatureSet())
		return;

	// FULL FEATURE mode
	HRESULT					hr;
	IHTMLElement*			pElement;
	CString					szInnerHTML;
	CRString				szAnchor( IDS_HTML_ANCHOR );
	CRString				szEudora( IDS_EUDORA_TAG );
	CRString				szAutoURL( IDS_AUTOURL );
	VARIANT					var;
	VARIANT_BOOL			vb;
	CBstr					cbstr;
	
	USES_CONVERSION;


	ExecCommand( IDM_HYPERLINK, NULL, NULL, TRUE );
	
	pElement = GetSelectedElement( szAnchor );
	
	if( !pElement )
	{
		return;
	}

	GetDocument()->SetModifiedFlag();

	hr = pElement->get_innerHTML(  BSTRARG( cbstr ) );
	szInnerHTML = cbstr;

	// see if this was an auto url
	cbstr = A2BSTR( szEudora );
	VariantInit( &var );

	hr = pElement->getAttribute( cbstr, FALSE, &var );

	if( ( hr == S_OK ) && ( var.vt == VT_BSTR ) )
	{
		szInnerHTML = var.bstrVal;
		if( szInnerHTML.CompareNoCase( szAutoURL ) == 0 )
		{
			vb = FALSE;
			hr = pElement->removeAttribute( cbstr, FALSE, &vb );
		}
	}

	VariantClear( &var );

	if( !HasSelectedText() )
	{
		return;
	}

	// we may need to expand the url

	ExpandTag( pElement, IDS_HTML_ANCHOR );
}



void CTridentReadMessageView::OnUpdateEditInsertLink(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot insert hyperlinks
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		IHTMLElement*	pElement;

		if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
		{
			// we're not in editing mode
			pCmdUI->Enable( FALSE );
			return;
		}

		if( HasSelectedText() )
		{
			pCmdUI->Enable( TRUE );
			return;
		}

		// see if we're in an anchor
		pElement = GetSelectedElement( CRString( IDS_HTML_ANCHOR ) );

		if( pElement != NULL )
		{
			pCmdUI->Enable( TRUE );
			pElement->Release();
		}
		else
		{
			pCmdUI->Enable( FALSE );
		}
	}
	else
	{
		// REDUCED FEATURE mode
		pCmdUI->Enable(FALSE);
	}
}

void CTridentReadMessageView::OnEditInsertPicture() 
{
	// Shareware: In reduced feature mode, you cannot insert pictures
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		ExecCommand( IDM_IMAGE, NULL, NULL, TRUE );
		GetDocument()->SetModifiedFlag();
	}
}

void CTridentReadMessageView::OnUpdateEditInsertPicture(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot insert pictures
	pCmdUI->Enable( UsingFullFeatureSet() && m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) );
}


void CTridentReadMessageView::OnFixedFont()
{
	INT		iSize;
	CString	szFontName;

	if( m_pParentFrame->UseFixedWidthFont() )
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FIXED_FONT );
	}
	else
	{
		szFontName = GetIniString( IDS_INI_MESSAGE_FONT );
	}

	iSize = GetIniShort( IDS_INI_MESSAGE_FONT_SIZE );

	SetFont( szFontName, iSize );
}


BOOL CTridentReadMessageView::GetMessageBody(
CString&	szMsg,
BOOL		bAsHTML )
{
	IDispatch*				pDisp; 
	CBstr					cbstr;
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLBodyElement*		pBody;
	IHTMLElement*			pElement;
	IHTMLTxtRange*			pRange;
	IHTMLTxtRange*			pRange2;
	IHTMLElementCollection* pColl;
	LONG					celem;
	VARIANT					varIndex;
	VARIANT					var2;
	CString					strTag;
	CBstr					cbstrStartToEnd( IDS_START_TO_END );
	CRString				szSpan( IDS_HTML_SPAN );
	CRString				szEudoraHeader( IDS_EUDORAHEADER );
	CRString				szTabooHeader( IDS_TABOOHEADER );
	
	// Get document
	pDoc = m_pSite->GetDoc2();
	
	if( !pDoc ) 
	{
		return FALSE;
	}

	// Get body
	pBody = m_pSite->GetBody();
	
	if( !pBody ) 
	{
		return FALSE;
	}

	pColl = NULL;
	pRange = NULL;
	
	// Create range
	hr = pBody->createTextRange( &pRange );

	if( ( S_OK == hr ) && pRange )
	{
		hr = pDoc->get_all( &pColl );

		if( (hr == S_OK) && pColl )
		{
			hr = pColl->get_length( &celem );

			if ( hr == S_OK )
			{
				for ( int i=0; i< celem; i++ )
				{
					varIndex.vt = VT_UINT;
					varIndex.lVal = i;
					VariantInit( &var2 );
					hr = pColl->item( varIndex, var2, &pDisp );

					if ( hr == S_OK )
					{
						hr = pDisp->QueryInterface( IID_IHTMLElement, (void **)&pElement );

						if ( hr == S_OK )
						{
							hr = pElement->get_tagName( BSTRARG( cbstr ) );
							strTag = cbstr;						

							if( strTag.CompareNoCase( szSpan ) == 0 )
							{
								// see if its a EUDORAHEADER or TABOOHEADER
								pElement->get_className( BSTRARG( cbstr ) );
								strTag = cbstr;

								if( strTag.CompareNoCase( szEudoraHeader ) ||
									strTag.CompareNoCase( szTabooHeader ) )
								{
									pRange2 = NULL;

									hr = pRange->duplicate( &pRange2 );
									
									if( ( hr == S_OK ) && pRange2 )
									{
										// get the span range
										hr = pRange2->moveToElementText( pElement );
										
										// move the start of pRange to the end of pRange2
										hr = pRange->setEndPoint( cbstrStartToEnd, pRange2 );

										pRange2->Release();
									}
								}
							}

							pElement->Release();
						}

						pDisp->Release();
					}
				}
			}
		}
	}

	if( pColl ) 
	{
		pColl->Release();
	}

	if( pRange == NULL )
	{
		return FALSE;
	}

	if( bAsHTML )
	{
		hr = pRange->get_htmlText( BSTRARG( cbstr ) );
	}
	else
	{
		hr = pRange->get_text( BSTRARG( cbstr ) );
	}

	if( ( hr == S_OK ) && ! cbstr.IsNull() )
	{
		szMsg = cbstr;
		return TRUE;
	}

	return FALSE;
}


BOOL CTridentReadMessageView::GetMessageAsHTML(
CString&	szMsg,
BOOL		bIncludeHeaders)
{
	if( bIncludeHeaders )
	{
		return GetAllHTML( szMsg );
	}
	
	return GetMessageBody( szMsg, TRUE );
}

BOOL CTridentReadMessageView::GetMessageAsText(
CString&	szMsg,
BOOL		bIncludeHeaders)
{
	if( bIncludeHeaders )
	{
		return GetAllText( szMsg );
	}
	
	return GetMessageBody( szMsg, FALSE );
}


void CTridentReadMessageView::OnEditInsertHr() 
{
	// Shareware: In reduced feature mode, you cannot insert horiz line
	if (UsingFullFeatureSet())
	{
		ExecCommand( IDM_HORIZONTALLINE, NULL, NULL, TRUE );	
		GetDocument()->SetModifiedFlag();
	}
}

void CTridentReadMessageView::OnUpdateEditInsertHr(CCmdUI* pCmdUI) 
{
	// Shareware: In reduced feature mode, you cannot insert horiz line
	if (UsingFullFeatureSet())
	{
		if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
		{
			// we're not in editing mode
			pCmdUI->Enable( FALSE );
			return;
		}
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE ); // Disable Insert Horiz Line in REDUCED FEATURE mode
	}
}


void CTridentReadMessageView::OnBlkfmtFixedwidth() 
{
	IHTMLElement*	pElement;
	CString			szOnTag;
	CString			szOffTag;

	if( m_pParentFrame->GetFormattingCheck( ID_EDIT_TEXT_TT ) )
	{
		// delete the tt tags
		pElement = GetSelectedElement( CRString( IDS_HTML_FIXED ) );
		if( pElement )
		{
			SplitTagOff( pElement, IDS_HTML_FIXED );
			pElement->Release();
		}
		return;
	}

	// build the tag strings
	szOnTag.Format( "<%s>", ( LPCSTR ) CRString( IDS_HTML_FIXED ) );
	szOffTag.Format( "</%s>", ( LPCSTR ) CRString( IDS_HTML_FIXED ) );

	if( WrapTag( szOnTag, szOffTag ) )
	{
		GetDocument()->SetModifiedFlag();
	}
}


void CTridentReadMessageView::OnUpdateBlkfmtFixedwidth(CCmdUI* pCmdUI) 
{
	IHTMLElement*	pElement;

	if( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) == 0 )
	{
		// we're not in editing mode
		pCmdUI->Enable( FALSE );
		return;
	}

	pCmdUI->Enable( TRUE );

	pElement = GetSelectedElement( CRString( IDS_HTML_FIXED ) );

	if( pElement )
	{
		pCmdUI->SetCheck( TRUE );
		pElement->Release();
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
	}
}
	


BOOL CTridentReadMessageView::DoOnDisplayPlugin(
IHTMLElement* pElement )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLElement*			pParent;
	IHTMLElement*			p;
	IHTMLAnchorElement*		pAnchor;
	CBstr					cbstr;
	CString					szValue;
	VARIANT					v;
	CBstr					cbstrEudora( IDS_EUDORA_TAG );
	IHTMLBodyElement*		pBody;
	IHTMLTxtRange*			pRange;
	CEudoraApp*				pApp;
	CString					szFixed( "" );
	INT						i;
	CString					szHRef( "" );

	USES_CONVERSION;

	// Get document
	VERIFY( pDoc = m_pSite->GetDoc2() );
	
	if ( !pDoc ) 
	{
		return FALSE;
	}

	VERIFY( pBody = m_pSite->GetBody() );

	if( !pBody )
	{
		return FALSE;
	}

	hr = pElement->get_tagName( BSTRARG( cbstr ) );
	szValue = cbstr;

	pParent = NULL;
	p = pElement;

	if( szValue.CompareNoCase( CRString( IDS_HTML_IMG ) ) == 0 )
	{
		pParent = NULL;
		hr = pElement->get_parentElement( &pParent );

		if( ( hr == S_OK ) && ( pParent != NULL ) )
		{
			hr = pParent->get_tagName( BSTRARG( cbstr ) );
			p = pParent;
			szValue = cbstr;
		}
	}

	if( szValue.CompareNoCase( CRString( IDS_HTML_ANCHOR ) ) == 0 )
	{
		// see if it's a plugin
		hr = p->getAttribute( cbstrEudora, 0, &v );

		if( ( hr == S_OK ) && ( v.vt == VT_BSTR ) )
		{
			szValue = v.bstrVal;
			
			if( szValue.CompareNoCase( CRString( IDS_PLUGIN ) ) == 0 )
			{
				// get the anchor
				
				pAnchor = NULL;
				hr = p->QueryInterface( IID_IHTMLAnchorElement, ( void** )( &pAnchor ) );
				
				if( ( hr == S_OK ) && ( pAnchor != NULL ) )
				{
					hr = pAnchor->get_href( BSTRARG( cbstr ) );
					
					if( ( hr == S_OK ) && ( ( ( BSTR ) cbstr ) != NULL ) )
					{
						szHRef = cbstr;
						hr = pBody->createTextRange( &pRange );
						hr = pRange->moveToElementText( p );
						hr = pRange->collapse( FALSE );
						hr = pRange->select();
					}

					pAnchor->Release();					
				}
			}
		}

		VariantClear( &v );
	}


	if( pParent )
	{
		pParent->Release();
	}

	pBody->Release();
	
	if( szHRef == "" )
	{
		return FALSE;
	}
	
	// don't release it if we're returning FALSE
	pElement->Release();

	// get rid of the "file:///"
	szHRef = szHRef.Right( szHRef.GetLength() - 8 ) ;

	szFixed.GetBuffer( szHRef.GetLength() + 1 );

	while( ( i = szHRef.Find( "%20" ) ) >= 0 )
	{
		szFixed += szHRef.Left( i );
		szFixed += ' ';
		szHRef = szHRef.Right( szHRef.GetLength() - i - 3 );
	}

	szFixed += szHRef;

	pApp = (CEudoraApp*) AfxGetApp();
	pApp->GetTranslators()->XLateDisplay( this, szFixed );

	return TRUE;
}


void CTridentReadMessageView::OnEditPasteAsQuotation() 
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc = NULL;
	IHTMLSelectionObject*	pSel = NULL;
	IHTMLTxtRange*			pRange = NULL;
	COleDataObject			theClipBoardData;
	HGLOBAL					hData;
	CString					szText;
	CString					szTemp;
	CBstr					cbstrText;
	CBstr					cbstrType;
	CString					szType;

	USES_CONVERSION;

	VERIFY( theClipBoardData.AttachClipboard() );

	if( ( theClipBoardData.IsDataAvailable( CF_TEXT, NULL ) == FALSE ) ||
		( ( hData = theClipBoardData.GetGlobalData( CF_TEXT, NULL ) ) == NULL ) )
	{
		return;
	}

	szTemp = ::Text2Html( ( char* ) ::GlobalLock( hData ), TRUE, FALSE );
	
	::GlobalUnlock( hData );

	if( szTemp == "" )
	{
		return;
	}
	
	szText.Format( "<%s TYPE=%s class=%s %s>%s</%s>",  ( LPCSTR )CRString( IDS_HTML_BLOCKQUOTE ),
														( LPCSTR )CRString( IDS_CITE ),
														( LPCSTR )CRString( IDS_CITE ),
														( LPCSTR )CRString( IDS_CITE ),		
														( LPCSTR )szTemp,
														( LPCSTR )CRString( IDS_HTML_BLOCKQUOTE ) );

	VERIFY( cbstrText = A2BSTR( szText ) );

	// Get document
	VERIFY( pDoc = m_pSite->GetDoc2() );

	// Get selection
	hr = pDoc->get_selection(&pSel);

	if( SUCCEEDED( hr ) &&  pSel ) 
	{
		// Get type
		hr = pSel->get_type( BSTRARG( cbstrType ) );

		if ( ( S_OK == hr )  && ( ( ( BSTR ) cbstrType ) != NULL ) )
		{
			szType = cbstrType;

			// If type isnt text, bail
			if( ( szType.CompareNoCase( CRString( IDS_TEXT ) ) == 0 ) || 
				( szType.CompareNoCase( CRString( IDS_NONE ) ) == 0 ) )
			{
				// Create range
				hr = pSel->createRange((IDispatch**)&pRange);
				
				if( ( S_OK == hr ) && pRange )
				{
					hr = pRange->pasteHTML( cbstrText );
					GetDocument()->SetModifiedFlag();
					pRange->Release();
				}
			}
		}

		pSel->Release();
	}
}


BOOL CTridentReadMessageView::ExpandTag(
IHTMLElement*	pElement,
UINT			uStringID ) 
{
	IHTMLSelectionObject*	pSelection;
	IHTMLBodyElement*		pBody;
	IHTMLDocument2*			pDoc;
	IHTMLTxtRange*			pSelectedRange;
	IHTMLTxtRange*			pElementRange;
	IHTMLTxtRange*			pNewRange;
	IHTMLTxtRange*			pTempRange;
	VARIANT_BOOL			vb;
	CBstr					cbstr;
	CBstr					cbstrStartToStart( IDS_START_TO_START );
	CBstr					cbstrEndToEnd( IDS_END_TO_END );
	CBstr					cbstrCharacter( IDS_CHARACTER );
	LONG					l;
	CString					szOuterHTML;
	CString					szInnerHTML;
	CString					szStartTag;
	CString					szEndTag;
	CString					szText;
	HRESULT					hr;
	CRString				szTempTag( IDS_EUDORATEMP_TAG );

	USES_CONVERSION;

	ASSERT( pElement );

	// Get document
	VERIFY( pDoc = m_pSite->GetDoc2() );
	VERIFY( pBody = m_pSite->GetBody() );

	// get the element range
	pElementRange = NULL;
	hr = pBody->createTextRange( &pElementRange );
	pBody->Release();

	if( !SUCCEEDED( hr ) || !pElementRange )
	{
		return FALSE;
	}

	hr = pElementRange->moveToElementText( pElement );

	if( !SUCCEEDED( hr ) )
	{
		pElementRange->Release();
		return FALSE;
	}

	// Get selection range
	pSelection = NULL;
	hr = pDoc->get_selection( &pSelection );

	if( !SUCCEEDED( hr ) || !pSelection )
	{
		pElementRange->Release();
		return FALSE;
	}

	pSelectedRange = NULL;
	hr = pSelection->createRange( ( IDispatch** ) &pSelectedRange );
	pSelection->Release();

	if( !SUCCEEDED( hr ) || !pSelectedRange )
	{
		pElementRange->Release();
		return FALSE;
	}

	// see if they are equal 
	vb = VARIANT_FALSE;
	hr = pElementRange->isEqual( pSelectedRange, &vb );

#pragma warning(disable : 4310)
	if( !SUCCEEDED( hr ) || ( vb == VARIANT_TRUE ) )
#pragma warning(default : 4310)
	{
		// yep.
		pSelectedRange->Release();
		pElementRange->Release();
		return TRUE;
	}


	// get the strings 
	hr = pElement->get_innerHTML( BSTRARG( cbstr ) );

	if( !SUCCEEDED( hr ) || ( (BSTR) cbstr ) == NULL )
	{
		pElement->Release();
		return FALSE;
	}
	
	szInnerHTML = cbstr;

	hr = pElement->get_outerHTML( BSTRARG( cbstr ) );

	if( !SUCCEEDED( hr ) || ( (BSTR) cbstr ) == NULL )
	{
		pElement->Release();
		return FALSE;
	}

	szOuterHTML = cbstr;

	// This assumes that end tags do not have attributes

	szEndTag = "</" + CRString( uStringID ) + ">";

	ASSERT( szOuterHTML.GetLength() > szInnerHTML.GetLength() );
	szText = szOuterHTML.Right( szEndTag.GetLength() );
	ASSERT( szText.CompareNoCase( szEndTag ) == 0 );

	szStartTag = szOuterHTML.Left( szOuterHTML.GetLength() - szInnerHTML.GetLength() - szEndTag.GetLength() );

	// now construct the new range

	hr = pElementRange->compareEndPoints( cbstrStartToStart, pSelectedRange, &l );

	if( !SUCCEEDED( hr ) )
	{
		pSelectedRange->Release();
		pElementRange->Release();
		pElement->Release();
		return FALSE;
	}

	pNewRange = NULL;

	if( l >= 0 ) 
	{
		// the start of pElementRange >= start of pSelectedRange
		// use the pSelectedRange start point
		hr = pSelectedRange->duplicate( &pNewRange );
	}
	else
	{
		hr = pElementRange->duplicate( &pNewRange );
	}

	if( !SUCCEEDED( hr ) || ( pNewRange == NULL ) )
	{
		pSelectedRange->Release();
		pElementRange->Release();
		pElement->Release();
		return FALSE;
	}

	// now set the end point of new range

	hr = pElementRange->compareEndPoints( cbstrEndToEnd, pSelectedRange, &l );
	
	if( l >= 0 ) 
	{
		// the end of pElementRange >= end of pSelectedRange
		// use the pElementRange end point
		hr = pNewRange->setEndPoint( cbstrEndToEnd, pElementRange );
	}
	else
	{
		hr = pNewRange->setEndPoint( cbstrEndToEnd, pSelectedRange );
	}

	if( !SUCCEEDED( hr ) )
	{
		pNewRange->Release();
		pSelectedRange->Release();
		pElementRange->Release();
		pElement->Release();
		return FALSE;
	}

	pTempRange = NULL;

	hr = pNewRange->duplicate( &pTempRange );
	
	if( !SUCCEEDED( hr ) || !pTempRange )
	{
		pNewRange->Release();
		pSelectedRange->Release();
		pElementRange->Release();
		pElement->Release();
		return FALSE;
	}

	// insert place holders

#pragma warning(disable : 4310)
	hr = pTempRange->collapse( VARIANT_TRUE );
#pragma warning(default : 4310)
	
	cbstr = A2BSTR( '<' + szTempTag + '>' );
	hr = pTempRange->put_text( cbstr );
	pTempRange->Release();

	l = 0;
	hr = pNewRange->moveEnd( cbstrCharacter, szTempTag.GetLength() + 2, &l );

	pTempRange = NULL;
	hr = pNewRange->duplicate( &pTempRange );
	
	if( !SUCCEEDED( hr ) || !pTempRange )
	{
		pNewRange->Release();
		pSelectedRange->Release();
		pElementRange->Release();
		pElement->Release();
		return FALSE;
	}

	hr = pTempRange->collapse( VARIANT_FALSE );

	cbstr = A2BSTR( "</" + szTempTag + '>' );
	hr = pTempRange->put_text( cbstr );
	
	l = 0;
	hr = pNewRange->moveEnd( cbstrCharacter, szTempTag.GetLength() + 3, &l );

	// remove the original anchor tags
	cbstr = A2BSTR( szInnerHTML );
	hr = pElement->put_outerHTML( cbstr );

	// get the entire html fragment
	hr = pNewRange->get_htmlText( BSTRARG( cbstr ) );
	szText = cbstr;

	// replace the place holders
	l = szText.Find( "&lt;" + szTempTag + "&gt;" );
	
	szOuterHTML = szText.Left( l ) + szStartTag;
	szText = szText.Right( szText.GetLength() - l - szTempTag.GetLength() - 8 );

	l = szText.Find( "&lt;/" + szTempTag + "&gt;" );
	szOuterHTML += szText.Left( l ) + szEndTag;
	szOuterHTML += szText.Right( szText.GetLength() - l - szTempTag.GetLength() - 9 );

	cbstr = A2BSTR( szOuterHTML );
	hr = pNewRange->pasteHTML( cbstr );

	pNewRange->Release();
	pSelectedRange->Release();
	pElementRange->Release();
	pElement->Release();
	GetDocument()->SetModifiedFlag();
	return TRUE;
}


BOOL CTridentReadMessageView::SplitTagOff(
IHTMLElement*	pElement,
UINT			uStringID ) 
{
	IHTMLSelectionObject*	pSelection;
	IHTMLBodyElement*		pBody;
	IHTMLDocument2*			pDoc;
	IHTMLTxtRange*			pSelectedRange;
	IHTMLTxtRange*			pElementRange;
	IHTMLTxtRange*			pRange;
	IHTMLElement*			pParent;
	VARIANT_BOOL			vb;
	CBstr					cbstr;
	LONG					l;
	CString					szTempDeleteEndTag( "<eudoradeleteend>" );
	CString					szTempDeleteStartTag( "<eudoradeletestart>" );
	CBstr					cbstrBeforeBegin( "BeforeBegin" );
	CBstr					cbstrBeforeEnd( "BeforeEnd" );
	CBstr					cbstrTempDeleteEndTag( szTempDeleteEndTag );
	CBstr					cbstrTempDeleteStartTag( szTempDeleteStartTag );
	CBstr					cbstrTempOnTag;
	CBstr					cbstrTempOffTag;
	CBstr					cbstrEndToEnd( IDS_END_TO_END );
	CString					szTempTag;
	CString					szTempOnTag;
	CString					szTempOffTag;
	CString					szOuterHTML;
	CString					szInnerHTML;
	CString					szStartTag;
	CString					szEndTag;
	CString					szPasteHTML;
	HRESULT					hr;
	BOOL					bHasLeftRange;
	BOOL					bHasRightRange;

	USES_CONVERSION;

	ASSERT( pElement );

	if( !pElement )
	{
		return NULL;
	}

	// get the inner and out html for the element
	hr = pElement->get_innerHTML( ( BSTRARG ) cbstr );
	szInnerHTML = cbstr;

	hr = pElement->get_outerHTML( ( BSTRARG ) cbstr );
	szOuterHTML = cbstr;

	// get the start and end tags -- this assumes that end tags do not have attributes
	szEndTag = "</" + CRString( uStringID ) + ">";

	ASSERT( szOuterHTML.GetLength() > szInnerHTML.GetLength() );

	szStartTag = szOuterHTML.Left( szOuterHTML.GetLength() - szInnerHTML.GetLength() - szEndTag.GetLength() );

	// Get document
	VERIFY( pDoc = m_pSite->GetDoc2() );
	VERIFY( pBody = m_pSite->GetBody() );

	// get the element range
	pElementRange = NULL;
	hr = pBody->createTextRange( &pElementRange );
	pBody->Release();

	if( !SUCCEEDED( hr ) || !pElementRange )
	{
		return FALSE;
	}

	hr = pElementRange->moveToElementText( pElement );

	if( !SUCCEEDED( hr ) )
	{
		pElementRange->Release();
		return FALSE;
	}

	// Get selection range
	pSelection = NULL;
	hr = pDoc->get_selection( &pSelection );

	if( !SUCCEEDED( hr ) || !pSelection )
	{
		pElementRange->Release();
		return FALSE;
	}

	pSelectedRange = NULL;
	hr = pSelection->createRange( ( IDispatch** ) &pSelectedRange );
	pSelection->Release();

	if( !SUCCEEDED( hr ) || !pSelectedRange )
	{
		pElementRange->Release();
		return FALSE;
	}

	// see if they are equal 
	vb = VARIANT_FALSE;
	hr = pElementRange->isEqual( pSelectedRange, &vb );

#pragma warning(disable : 4310)
	if( !SUCCEEDED( hr ) || ( vb == VARIANT_TRUE ) )
#pragma warning(default : 4310)
	{
		// yep. just turn it off the tag and bail
		hr = pElement->get_innerHTML( ( BSTRARG) cbstr );
		hr = pElement->put_outerHTML( cbstr );
		pSelectedRange->Release();
		pElementRange->Release();
		return TRUE;
	}

	// see if part of the element range is to the right of the selected range

	VERIFY( cbstr = A2BSTR( CRString( IDS_END_TO_END ) ) );
	hr = pElementRange->compareEndPoints( cbstr, pSelectedRange, &l );

	if( !SUCCEEDED( hr ) )
	{
		pSelectedRange->Release();
		pElementRange->Release();
		return FALSE;
	}

	bHasRightRange = ( l > 0 );

	// see if part of the element range is to the left of the selected range

	// see if part of the element is left of the selected range
	VERIFY( cbstr = A2BSTR( CRString( IDS_START_TO_START ) ) );
	hr = pElementRange->compareEndPoints( cbstr, pSelectedRange, &l );
	pElementRange->Release();

	if( !SUCCEEDED( hr ) )
	{
		pSelectedRange->Release();
		return FALSE;
	}

	bHasLeftRange = ( l < 0 );

	// build the temporary tags
	szTempTag = '<' + CRString( IDS_EUDORATEMP_TAG );
	szTempOnTag = szTempTag + "ON>";
	VERIFY( cbstrTempOnTag = A2BSTR( szTempOnTag ) );
	szTempOffTag = szTempTag + "OFF>";
	VERIFY( cbstrTempOffTag = A2BSTR( szTempOffTag ) );

	if( bHasRightRange )
	{
		// ok the element is bigger than the selection on both sides
		pRange = NULL;
		hr = pSelectedRange->duplicate( &pRange );
		
		if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
		{
			pSelectedRange->Release();
			return FALSE;
		}

		// collapse to the end of the range
#pragma warning(disable : 4310)
		hr = pRange->collapse( VARIANT_FALSE );
#pragma warning(default : 4310)

		// insert the temporary on tag at the end of the selection
		hr = pRange->put_text( cbstrTempOnTag );
		pRange->Release();
	}
	else
	{
		hr = pElement->insertAdjacentText( cbstrBeforeEnd, cbstrTempDeleteEndTag );
	}
	
	if( bHasLeftRange )
	{
		// ok the element is bigger than the selection on both sides
		pRange = NULL;
		hr = pSelectedRange->duplicate( &pRange );
		
		if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
		{
			pSelectedRange->Release();
			return FALSE;
		}

		// collapse to the end of the range
#pragma warning(disable : 4310)
		hr = pRange->collapse( VARIANT_TRUE );
#pragma warning(default : 4310)

		// insert the temporary on tag at the end of the selection
		hr = pRange->put_text( cbstrTempOffTag );
		pRange->Release();
	}
	else
	{
		hr = pElement->insertAdjacentText( cbstrBeforeBegin, cbstrTempDeleteStartTag );
	}

	// this is a temporary fix for the selection problem
	// actually, it's not a fix -- it's just a more desirable bug
#pragma warning(disable : 4310)
	pSelectedRange->collapse( VARIANT_TRUE );
#pragma warning(default : 4310)
	pSelectedRange->select();

	// ok -- now we set the "selected range to be the outermost range which completely
	// encompases the element
	hr = pSelectedRange->moveToElementText( pElement );
	pRange = NULL;
	hr = pSelectedRange->duplicate( &pRange );
		
	if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
	{
		pSelectedRange->Release();
		return FALSE;
	}

	// set the start of the range
#pragma warning(disable : 4310)
	pSelectedRange->collapse( VARIANT_TRUE );
#pragma warning(default : 4310)
	pParent = NULL;
	hr = pSelectedRange->parentElement( &pParent );

	if( !SUCCEEDED( hr ) || ( pParent == NULL ) )
	{
		pSelectedRange->Release();
		pRange->Release();
		return FALSE;
	}
	
	pSelectedRange->moveToElementText( pParent );
	pParent->Release();

	// set the end of the range
	pRange->collapse( VARIANT_FALSE );
	pParent = NULL;
	hr = pRange->parentElement( &pParent );

	if( !SUCCEEDED( hr ) || ( pParent == NULL ) )
	{
		pSelectedRange->Release();
		pRange->Release();
		return FALSE;
	}
	
	pRange->moveToElementText( pParent );
	pSelectedRange->setEndPoint( cbstrEndToEnd, pRange );
	pRange->Release();


	// now get the text
	hr = pSelectedRange->get_htmlText( ( BSTRARG ) cbstr );
	szOuterHTML = cbstr;
	
	// first we're dealing with html now so convert the temporary tags
	szTempDeleteEndTag = Text2Html( szTempDeleteEndTag, FALSE, FALSE );
	szTempDeleteEndTag = szTempDeleteEndTag.Left( szTempDeleteEndTag.GetLength() - 2 ); // strip "\r\n"
	szTempDeleteStartTag = Text2Html( szTempDeleteStartTag, FALSE, FALSE );
	szTempDeleteStartTag = szTempDeleteStartTag.Left( szTempDeleteStartTag.GetLength() - 2 ); // strip "\r\n"
	szTempOnTag = Text2Html( szTempOnTag, FALSE, FALSE );
	szTempOnTag = szTempOnTag.Left( szTempOnTag.GetLength() - 2 ); // strip "\r\n"
	szTempOffTag = Text2Html( szTempOffTag, FALSE, FALSE );
	szTempOffTag = szTempOffTag.Left( szTempOffTag.GetLength() - 2 ); // strip "\r\n"

	
	// remove the start and end tags if necessary	
	szPasteHTML = "";

	l = szOuterHTML.Find( szTempDeleteEndTag );

	if( l >= 0 )
	{
		szPasteHTML += szOuterHTML.Left( l );
		szOuterHTML = szOuterHTML.Right( szOuterHTML.GetLength() - l - szTempDeleteEndTag.GetLength() - szEndTag.GetLength() );
	}

	szOuterHTML = szPasteHTML + szOuterHTML;
	szPasteHTML = "";

	l = szOuterHTML.Find( szTempDeleteStartTag );

	if( l >= 0 )
	{
		szPasteHTML += szOuterHTML.Left( l );
		szOuterHTML = szOuterHTML.Right( szOuterHTML.GetLength() - l - szTempDeleteStartTag.GetLength() - szStartTag.GetLength() );
	}

	szOuterHTML = szPasteHTML + szOuterHTML;
	szPasteHTML = "";

	// now replace any temporary tags 

	szPasteHTML = "";

	// find the temporary start tag
	l = szOuterHTML.Find( szTempOnTag );

	if( l >= 0 )
	{
		// replace it with the real thing
		szPasteHTML += szOuterHTML.Left( l ) + szStartTag;
		szOuterHTML = szOuterHTML.Right( szOuterHTML.GetLength() - l - szTempOnTag.GetLength() );
	}

	szOuterHTML = szPasteHTML + szOuterHTML;
	szPasteHTML = "";

	// find the temporary end tag
	l = szOuterHTML.Find( szTempOffTag );

	if( l >= 0 )
	{
		// replace it with the real thing
		szPasteHTML += szOuterHTML.Left( l ) + szEndTag;
		szOuterHTML = szOuterHTML.Right( szOuterHTML.GetLength() - l - szTempOffTag.GetLength() );
	}

	szPasteHTML += szOuterHTML;
	VERIFY( cbstr = A2BSTR( szPasteHTML ) );

	hr = pSelectedRange->pasteHTML( cbstr );
	ClearUndoStack();

	return TRUE;
}


void CTridentReadMessageView::OnUpdateSize(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pParentFrame->GetCheck( ID_EDIT_MESSAGE ) != 0 );
}


void CTridentReadMessageView::SetBodyText(
LPCSTR	szMessage ) 
{
	HRESULT					hr;
	IDispatch*				pDispatch; 	
	IHTMLDocument2*			pDoc;
	IHTMLElement*			pElement;
	IHTMLBodyElement*		pBody;
	IHTMLTxtRange*			pTempRange;
	IHTMLTxtRange*			pRange;
	IHTMLElement*			pSpanElement;
	BSTR					bstr;
	CString					szText;
	CString					szText2;
	IHTMLElementCollection* pCollection;
	LONG					lIndex;
	VARIANT					varIndex;
	VARIANT					var;
	LONG					lCount;
	CBstr					bstrStartToStart( IDS_START_TO_START );
	CBstr					bstrStartToEnd( IDS_START_TO_END );
	VARIANT_BOOL			vb;
	CReadMessageDoc*		pMessageDoc;
	CRString				szBodyTag( IDS_HTML_BODY );
	CRString				szSpanTag( IDS_HTML_SPAN );
	CRString				szEudoraHeader( IDS_EUDORAHEADER );
	CRString				szTabooHeader( IDS_TABOOHEADER );

	pMessageDoc = ( CReadMessageDoc* ) GetDocument();
	ASSERT_KINDOF( CReadMessageDoc, pMessageDoc );
	
	USES_CONVERSION;

	pDoc = m_pSite->GetDoc2();
	
	if( !pDoc ) 
	{
		return;
	}

	pRange = NULL;
	pSpanElement = NULL;

	hr = pDoc->get_all( &pCollection );

	if( ( hr == S_OK ) && pCollection )
	{
		hr = pCollection->get_length( &lCount );

		if ( hr == S_OK )
		{
			for ( lIndex = 0; lIndex < lCount; lIndex++ )
			{
				varIndex.vt = VT_UINT;
				varIndex.lVal = lIndex;

				VariantInit( &var );

				hr = pCollection->item( varIndex, var, &pDispatch );

				if( ( hr == S_OK ) && pDispatch ) 
				{
					hr = pDispatch->QueryInterface( IID_IHTMLElement, (void **)&pElement );

					if( ( hr == S_OK ) && pElement )
					{
						hr = pElement->get_tagName( &bstr );
						szText = bstr;
						::SysFreeString( bstr );
						
						if( pRange == NULL )
						{
							// haven't found the body tag yet
							if ( szText.CompareNoCase( szBodyTag ) == 0 ) 
							{
								pBody = NULL;
								hr = pElement->QueryInterface( IID_IHTMLBodyElement, (void **)&pBody );
								ASSERT( pBody );
								hr = pBody->createTextRange( &pRange );
								ASSERT( pRange );
								pBody->Release();
								pRange->moveToElementText( pElement );
							}
						}
						else if( szText.CompareNoCase( szSpanTag ) == 0 )
						{
							if( pSpanElement )
							{
								pSpanElement->Release();
								pSpanElement = NULL;
							}

							// see if it's a header span
							hr = pElement->get_className( &bstr );
							szText = bstr;
							::SysFreeString( bstr );

							if( szText.CompareNoCase( szEudoraHeader ) &&
								szText.CompareNoCase( szTabooHeader ) )
							{
								// not in the headers anymore
								// create the body range								
								hr = pRange->duplicate( &pTempRange );
								hr = pTempRange->moveToElementText( pElement );
								hr = pRange->setEndPoint( bstrStartToStart, pTempRange );
								pTempRange->Release();

								// bail
								lCount = 0;
							}
							else
							{
								pSpanElement = pElement;
								pElement = NULL;
							}
						}
						else
						{
							// see if the parent is a header span
							if( pSpanElement )
							{
								hr = pSpanElement->contains( pElement, &vb );
#pragma warning(disable : 4310)
								if( vb != VARIANT_TRUE )
#pragma warning(default : 4310)
								{
									pSpanElement->Release();
									pSpanElement = NULL;
								}
							}

							if( pSpanElement == NULL )
							{
								// we aren't in a header span
								// create the header & body range

								hr = pRange->duplicate( &pTempRange );
								hr = pTempRange->moveToElementText( pElement );								
								hr = pRange->setEndPoint( bstrStartToStart, pTempRange );					
								pTempRange->Release();

								// bail
								lCount = 0;
							}
						}	
								
						if( pElement )
						{
							pElement->Release();
						}
					}
					
					pDispatch->Release();
				}
			}
		}
	}

	if( pSpanElement )
	{
		pSpanElement->Release();
	}

	pCollection->Release();

	if( pRange )
	{
		szText2 = FindBody( szMessage );
		szText = Text2Html( szText2, TRUE, FALSE );
		bstr = A2BSTR( szText );
		hr = pRange->pasteHTML( bstr );
		::SysFreeString( bstr );
		pRange->Release();
	}
	GetDocument()->SetModifiedFlag();
}

#if 0    // BOG
void CTridentReadMessageView::StripNBSP(
char*	szText )
{
	INT	i;

	for ( i = strlen( szText) - 1; i >= 0; i-- )
	{
		if( ( UCHAR )( szText[ i ] ) == 0xa0 )
		{
			szText[ i ] = ' ';
		}
	}
}
#endif

void CTridentReadMessageView::ClearUndoStack() 
{
	HRESULT hr = E_FAIL;
	IServiceProvider* piservProv;
	IOleUndoManager* pioleUndoManager;
	IUnknown* lpUnk = m_pSite->GetObjectUnknown();

	hr = lpUnk->QueryInterface(IID_IServiceProvider, (LPVOID*)&piservProv);

	hr = piservProv->QueryService(IID_IOleUndoManager, IID_IOleUndoManager, (LPVOID*)&pioleUndoManager);

	if (SUCCEEDED(hr))
	{
		pioleUndoManager->Enable(FALSE);
		pioleUndoManager->Enable(TRUE);
		pioleUndoManager->Release();
	}

	piservProv->Release();
}


BOOL CTridentReadMessageView::WrapTag(
LPCSTR	szOnTag,
LPCSTR	szOffTag )
{
	HRESULT					hr;
	IHTMLDocument2*			pDoc;
	IHTMLSelectionObject*	pSelection;
	IHTMLTxtRange*			pRange;
	IHTMLTxtRange*			pSelectionRange;
	IHTMLTxtRange*			pOuterRange;
	IHTMLElement*			pElement;
	CString					szText;
	CString					szPasteHTML;
	CBstr					cbstr;
	CBstr					cbstrCharacter( IDS_CHARACTER );
	CBstr					cbstrEndToEnd( IDS_END_TO_END );
	CBstr					cbstrTextedit( "Textedit" );
	CBstr					cbstrTempOnTag;
	CBstr					cbstrTempOffTag;
	CString					szTempOnTag;
	CString					szTempOffTag;
	CString					szTempTag;
	LONG					l;


	USES_CONVERSION;
	
	// Get document
	pDoc = m_pSite->GetDoc2();

	// Get selection
	pSelection = NULL;
	hr = pDoc->get_selection( &pSelection );

	if( !SUCCEEDED( hr ) || ( pSelection == NULL ) ) 
	{
		return FALSE;
	}

	// Create range
	pSelectionRange = NULL;
	hr = pSelection->createRange((IDispatch**)&pSelectionRange);
	pSelection->Release();

	if( !SUCCEEDED( hr ) || ( pSelectionRange == NULL ) ) 
	{
		return FALSE;
	}


	// build the temporary tag strings
	szTempTag = '<' + CRString( IDS_EUDORATEMP_TAG );
	szTempOnTag = szTempTag + "ON>";
	VERIFY( cbstrTempOnTag = A2BSTR( szTempOnTag ) );
	szTempOffTag = szTempTag + "OFF>";
	VERIFY( cbstrTempOffTag = A2BSTR( szTempOffTag ) );

	// paste the temporary start tag to the start
	pRange = NULL;
	hr = pSelectionRange->duplicate( &pRange );	

	if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
	{
		pSelectionRange->Release();
		return FALSE;
	}

#pragma warning(disable : 4310)
	hr = pRange->collapse( VARIANT_TRUE );
#pragma warning(default : 4310)
	hr = pRange->put_text( cbstrTempOnTag );
	pRange->Release();
	l = 0;
	hr = pSelectionRange->moveEnd( cbstrCharacter, szTempOnTag.GetLength(), &l );
		
	// paste the temporary end tag to the end
	pRange = NULL;
	hr = pSelectionRange->duplicate( &pRange );
	if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
	{
		pSelectionRange->Release();
		return FALSE;
	}
	hr = pRange->collapse( VARIANT_FALSE );
	hr = pRange->put_text( cbstrTempOffTag );
	pRange->Release();
	l = 0;
	hr = pSelectionRange->moveEnd( cbstrCharacter, szTempOffTag.GetLength(), &l );
	
	// get the minimum outer most range that encompases the selection 
	
	// get the start of the range by getting the parent of the start of the selection
	pRange = NULL;
	hr = pSelectionRange->duplicate( &pRange );	
	
	if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
	{
		pSelectionRange->Release();
		return FALSE;
	}
	
#pragma warning(disable : 4310)	
	hr = pRange->collapse( VARIANT_TRUE );
#pragma warning(default : 4310)	
	hr = pRange->duplicate( &pOuterRange );
	pRange->Release();

	if( !SUCCEEDED( hr ) || ( pOuterRange == NULL ) )
	{
		pSelectionRange->Release();
		return FALSE;
	}

	pElement = NULL;
	hr = pOuterRange->parentElement( &pElement );
	
	if( !SUCCEEDED( hr ) || ( pElement == NULL ) )
	{
		pSelectionRange->Release();
		pOuterRange->Release();
		return FALSE;
	}

	hr = pOuterRange->moveToElementText( pElement );
	pElement->Release();

	// now set the end of the range by getting the parent element at the end of the selection
	pRange = NULL;
	hr = pSelectionRange->duplicate( &pRange );	
	
	if( !SUCCEEDED( hr ) || ( pRange == NULL ) )
	{
		pSelectionRange->Release();
		pOuterRange->Release();
		return FALSE;
	}

	hr = pRange->collapse( VARIANT_FALSE );
	pElement = NULL;
	hr = pRange->parentElement( &pElement );

	if( !SUCCEEDED( hr ) || ( pElement == NULL ) )
	{
		pSelectionRange->Release();
		pOuterRange->Release();
		return FALSE;
	}
	
	hr = pRange->moveToElementText( pElement );
	hr = pOuterRange->setEndPoint( cbstrEndToEnd, pRange );
	pElement->Release();
	pRange->Release();

	// now get the complete html for the outer range	
	hr = pOuterRange->get_htmlText( ( BSTRARG ) cbstr );
	szText = cbstr;

	// replace the fake tags with the real thing

	// first we're dealing with html now so convert the temporary tags
	szTempOnTag = Text2Html( szTempOnTag, FALSE, FALSE );
	szTempOnTag = szTempOnTag.Left( szTempOnTag.GetLength() - 2 ); // strip "\r\n"
	szTempOffTag = Text2Html( szTempOffTag, FALSE, FALSE );
	szTempOffTag = szTempOffTag.Left( szTempOffTag.GetLength() - 2 ); // strip "\r\n"

	szPasteHTML = "";

	// find the temporary start tag
	l = szText.Find( szTempOnTag );

	if( l >= 0 )
	{
		// replace it with the real thing
		szPasteHTML += szText.Left( l ) + szOnTag;
		szText = szText.Right( szText.GetLength() - l - szTempOnTag.GetLength() );
	}

	szText = szPasteHTML + szText;
	szPasteHTML = "";

	// find the temporary end tag
	l = szText.Find( szTempOffTag );

	if( l >= 0 )
	{
		// replace it with the real thing
		szPasteHTML += szText.Left( l ) + szOffTag;
		szText = szText.Right( szText.GetLength() - l - szTempOffTag.GetLength() );
	}

	szPasteHTML += szText;

	// paste it into the doc
	VERIFY( cbstr = A2BSTR( szPasteHTML ) );
	hr = pOuterRange->pasteHTML( cbstr );
	
	pOuterRange->Release();
	pSelectionRange->Release();
	
	ClearUndoStack();
	return TRUE;
}
