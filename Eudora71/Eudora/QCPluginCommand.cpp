// QCPluginCommand.cpp: implementation of the QCPluginCommand class.
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <afxrich.h>

#include "eudora.h"
#include "msgutils.h"
#include "compmsgd.h"
#include "QCCommandActions.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "QCProtocol.h"
#include "QCSharewareManager.h"
#include "trnslate.h"


#include "DebugNewHelpers.h"

// "Magic" ROP code clipped from an MSDN article. Used in
// DrawNeedsFullFeatureSetBitmap.
//
// This is the "magic" ROP code used to generate the embossed look for
// a disabled button. It's listed in Appendix F of the Win32 Programmer's
// Reference as PSDPxax (!) which is a cryptic reverse-polish notation for
//
// ((Destination XOR Pattern) AND Source) XOR Pattern
//
// which I leave to you to figure out. In the case where I apply it,
// Source is a monochrome bitmap which I want to draw in such a way that
// the black pixels get transformed to the brush color and the white pixels
// draw transparently--i.e. leave the Destination alone.
//
// black ==> Pattern
// white ==> Destintation (ie, transparent)
//
// 0xb8074a is the ROP code that does this. For more info, see Charles
// Petzold, _Programming Windows_, 2nd Edition, p 622-624.
//
const DWORD		MAGICROP		= 0xb8074a;


IMPLEMENT_DYNAMIC(QCPluginCommand, QCCommandObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCPluginCommand::QCPluginCommand(
QCPluginDirector*	pDirector,
void*				pPluginObject,
LPCSTR				szName,
COMMAND_ACTION_TYPE	theType ) : QCCommandObject( pDirector )
{
	ASSERT( pPluginObject );

	m_ImageOffset = -1;
	m_pPluginObject = pPluginObject;
	m_szName = szName;
	m_theType = theType;
}

QCPluginCommand::~QCPluginCommand()
{

}


void	QCPluginCommand::Execute(
COMMAND_ACTION_TYPE	theAction,
void*				pData ) 
{
	CTLAddress*				pTAddress;
	CWnd*					pFocusWnd;
	struct TRANSLATE_DATA*	pTranslateData;
	struct TOGGLE_DATA*		pToggleData;
	CView*					pView;
	UINT					uState;
	QCProtocol*				pProtocol;

	if( theAction == CA_ATTACH_PLUGIN )
	{
		if( ( pData == NULL ) || ( m_theType != CA_ATTACH_PLUGIN ) )
		{
			ASSERT( 0 );
			return;
		}

//		unfortunately, the plugins don't support type checking 
//		ASSERT_KINDOF( CAttacher, ( CAttacher* ) m_pPluginObject );
		ASSERT_KINDOF( CCompMessageDoc, ( CCompMessageDoc* ) pData );

		( ( CAttacher* ) m_pPluginObject )->MenuHook( ( CCompMessageDoc* ) pData );
	}

	if( theAction == CA_SPECIAL_PLUGIN )
	{
		( ( CSpecial* ) m_pPluginObject )->MenuHook( NULL );
	}

	if( theAction == CA_MEASURE ) 
	{
		MeasureItem( ( LPMEASUREITEMSTRUCT ) pData );
	}
		
	if( theAction == CA_DRAW )
	{
		DrawItem( ( LPDRAWITEMSTRUCT ) pData );
	}

	// It looks like you can come here by opening a message and selecting an on request plugin from paige or trident
	// If you see some other way, please document it here. --Josh
	if( theAction == CA_TRANSLATE_PLUGIN )
	{
		ASSERT( pData );

		pTranslateData = ( struct TRANSLATE_DATA* ) pData;

		BOOL bBasic = ((CEudoraApp *)AfxGetApp())->GetTranslators()->GetHeaderFlag() & EMSAPI_REQUEST_BASIC;
		BOOL bRaw	= ((CEudoraApp *)AfxGetApp())->GetTranslators()->GetHeaderFlag() & EMSAPI_REQUEST_RAW;

		// This is executed if you have an ONREQUEST plugin and it requires the headers. (MailJail) 
		// The else part is what was here before.
		if (pTranslateData->m_pProtocol && !pTranslateData->m_pView && (bBasic || bRaw))
		{
			pProtocol = pTranslateData->m_pProtocol;
			pView = NULL;

			CString szTheMsg;
			char *pTheMsg;
			char *floater;

			pTAddress = DEBUG_NEW CTLAddress();
			pProtocol->GetMessageAsText(szTheMsg, TRUE);
			pTheMsg = DEBUG_NEW_NOTHROW char[szTheMsg.GetLength()+1];
			if (!pTheMsg)
				return;
			strcpy(pTheMsg, szTheMsg);
			floater = strstr(pTheMsg, "\r\n\r\n");
			if (floater)
			{
				*floater = 0;
			}

			char *to = NULL, *from = NULL, *cc = NULL, *subject = NULL, *bcc = NULL;
			if (bBasic)
			{
				to = HeaderContents(IDS_HEADER_TO, pTheMsg);	
				from = HeaderContents(IDS_HEADER_FROM, pTheMsg);
				cc = HeaderContents(IDS_HEADER_CC, pTheMsg);	
				subject = HeaderContents(IDS_HEADER_SUBJECT, pTheMsg);
			}
			
			pTAddress->CreateAddressList(to, 
										from, 
										subject, 
										cc, 
										bcc,
										bRaw ? pTheMsg : NULL);
			if (to) delete []to;
			if (from) delete []from;
			if (cc) delete []cc;
			if (subject) delete []subject;
			delete[] pTheMsg;


			( ( CTranslator* ) m_pPluginObject )->TranslateMessage( pProtocol, 
																	pTAddress );
			if( pTAddress )
			{
				delete pTAddress;
				pTAddress  = NULL;
			}
		}
		else	// This happens when you call an ONREQUEST plugin (UpperLower). Don't know what happens if an ONREQUEST
		{		// Plugin asks for headers...
			pProtocol = pTranslateData->m_pProtocol;
			pView = pTranslateData->m_pView;
			pFocusWnd = pTranslateData->m_pView->GetFocus();

			if( pTranslateData->m_bBuildAddresses )
			{
				pTAddress = DEBUG_NEW_NOTHROW CTLAddress();

				if( !pTAddress )
				{
					ASSERT( 0 );
					return;
				}

				pTAddress->CreateAddressList( pView->GetDocument() );		
			}
			else
			{
				pTAddress = NULL;
			}

			if( pProtocol != NULL )
			{
				( ( CTranslator* ) m_pPluginObject )->TranslateMessage( pProtocol, 
																		pTAddress );
			}
			else if( pView->IsKindOf( RUNTIME_CLASS( CRichEditView ) ) )
			{
				( ( CTranslator* ) m_pPluginObject )->TranslateMessage( &( ( ( CRichEditView* ) pView )->GetRichEditCtrl() ), 
																		pTAddress );
			}
			else if( pView->IsKindOf( RUNTIME_CLASS( CEditView ) ) )
			{
				( ( CTranslator* ) m_pPluginObject )->TranslateMessage( &( ( (CEditView* ) pView)->GetEditCtrl() ), 
																		pTAddress );
			}
			else if(	pFocusWnd->IsKindOf( RUNTIME_CLASS( CRichEditCtrl ) ) ||
						pFocusWnd->IsKindOf( RUNTIME_CLASS( CEdit ) ) )
			{
				( ( CTranslator* ) m_pPluginObject )->TranslateMessage( pFocusWnd, 
																		pTAddress );
			}

			if( pTAddress )
			{
				delete pTAddress;
			}
		}
	}

	if( theAction == CA_TOGGLE )
	{
		pToggleData = ( struct TOGGLE_DATA* ) pData;

		ASSERT( m_theType == CA_TRANSMISSION_PLUGIN );

		if( ( pToggleData == NULL ) || ( pToggleData->m_pMenu == NULL ) )
		{
			ASSERT( 0 );
			return;
		}
		if( pToggleData->m_ForceCheck)
		{
			pToggleData->m_uFlags &= ~MF_UNCHECKED;
			pToggleData->m_uFlags |= MF_CHECKED;
		}
		else
		{
			uState = pToggleData->m_pMenu->GetMenuState( pToggleData->m_uID, pToggleData->m_uFlags );		

			if( uState & MF_CHECKED )
			{
				pToggleData->m_uFlags &= ~MF_CHECKED;
				pToggleData->m_uFlags |= MF_UNCHECKED;
			}
			else 
			{
				pToggleData->m_uFlags &= ~MF_UNCHECKED;
				pToggleData->m_uFlags |= MF_CHECKED;
			}

			CString Dummy;
			CString* pStrProps = pToggleData->m_pszProperties;
			if (!pStrProps)
				pStrProps = &Dummy;
			if( ! ( ( CTranslator* ) m_pPluginObject )->QueueTrans(	EMSF_Q4_COMPLETION, 
																	*pStrProps, 
																	( pToggleData->m_uFlags & MF_CHECKED ) ? 0 : 1 ) )
			{
				pToggleData->m_uFlags &= ~MF_CHECKED;
				pToggleData->m_uFlags |= MF_UNCHECKED;
			}
			else
			{
				pToggleData->m_uFlags &= ~MF_UNCHECKED;
				pToggleData->m_uFlags |= MF_CHECKED;
			}
		}

		pToggleData->m_pMenu->CheckMenuItem( pToggleData->m_uID, pToggleData->m_uFlags );
	}
}


//	Draw the "full featured only" bitmap from QCSharewareManager. Draw it the same way
//	a system drawn menu would - disabled embossed.
bool QCPluginCommand::DrawNeedsFullFeatureSetBitmap(bool bSelected, CPoint & ptDrawHere, CDC & memDC, CDC * pdcDraw)
{
	bool		drawSucceeded = false;
	CBitmap *	pBitmap = GetFullFeatureBitmap();

	if (pBitmap)
	{
		HBITMAP		hbmSrc = static_cast<HBITMAP>(*pBitmap);
		BITMAP		bm;
		::GetObject( hbmSrc, sizeof(BITMAP), &bm );
		
		CBrush *	pOldBrush = NULL;
		CBitmap *	pOldBitmap = memDC.SelectObject( pBitmap );
		
		// Draw using hilite offset by (1,1), then shadow
		CBrush		brShadow(GetSysColor(COLOR_3DSHADOW));
		if (bSelected)
		{
			//	Just select the object to draw the gray icon
			pOldBrush = pdcDraw->SelectObject(&brShadow);
		}
		else
		{
			//	Draw the highlight
			CBrush	brHilite(GetSysColor(COLOR_3DHIGHLIGHT));
			pOldBrush = pdcDraw->SelectObject(&brHilite);
			pdcDraw->BitBlt(ptDrawHere.x+1, ptDrawHere.y+1, bm.bmWidth, bm.bmHeight, &memDC, 0, 0, MAGICROP);
			pdcDraw->SelectObject(&brShadow);
		}

		//	Draw the gray shadow
		pdcDraw->BitBlt(ptDrawHere.x, ptDrawHere.y, bm.bmWidth, bm.bmHeight, &memDC, 0, 0, MAGICROP);
		if (pOldBrush)
			pdcDraw->SelectObject(pOldBrush);

		if (pOldBitmap)
			memDC.SelectObject(pOldBitmap);

		drawSucceeded = true;
	}

	return drawSucceeded;
}


void QCPluginCommand::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	BOOL		bChecked;
	CDC*		pDC;
	CRect		theRect;
	BOOL		bGrayed;
	COLORREF	crBackColor;
	COLORREF	crForeColor;
	BITMAP		theBitmap;
	CDC			theMemDC;
	CBitmap*	pOldBitmap;
	DWORD		dwMode;
	
	ASSERT( ( m_theType == CA_TRANSLATE_PLUGIN ) || 
			( m_theType == CA_TRANSMISSION_PLUGIN ) ||
			( m_theType == CA_ATTACH_PLUGIN ) || 
			( m_theType == CA_SPECIAL_PLUGIN) );

	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);	

	if( ( NULL == pDC ) || ( NULL == pDC->m_hDC) )
	{
		return;
	}

	bChecked = FALSE;
	theRect = lpDrawItemStruct->rcItem;

	// Draw focus rect
	if( ( lpDrawItemStruct->itemAction & ODA_FOCUS ) == ODA_FOCUS )
	{
		pDC->DrawFocusRect( &theRect );
		return;
	}

	// Is this grayed?
	bGrayed = ( ( lpDrawItemStruct->itemState & ODS_GRAYED ) == ODS_GRAYED );

	// Set up some color values
	bool	bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED);
	if(bSelected)
	{
		crBackColor = GetSysColor( COLOR_HIGHLIGHT );
		crForeColor = bGrayed? GetSysColor( COLOR_GRAYTEXT ) : GetSysColor( COLOR_MENU );
	}
	else
	{
		crBackColor = GetSysColor( COLOR_MENU );
		crForeColor = bGrayed? GetSysColor( COLOR_GRAYTEXT ) : GetSysColor( COLOR_WINDOWTEXT );
	}
	
	// Draw background
	CBrush BackBrush( crBackColor );

	pDC->FillRect( &theRect, &BackBrush );

	// Are we drawing the checked bitmap?
	if( ( lpDrawItemStruct->itemState & ODS_CHECKED ) == ODS_CHECKED )
	{
		bChecked = TRUE;
	}

	theMemDC.CreateCompatibleDC( pDC );
	
	if( bChecked )
	{
		if( theMemDC.GetSafeHdc() )
		{
			// The Checked Bitmap
			CBitmap chkBM;
			chkBM.LoadOEMBitmap( OBM_CHECK );
			chkBM.GetObject( sizeof( BITMAP ), &theBitmap );

			theRect.left += 1;
			theRect.top += 1;

			pOldBitmap = theMemDC.SelectObject( &chkBM );
			dwMode = SRCAND;
			
			if( lpDrawItemStruct->itemState & ODS_SELECTED )
			{
				dwMode = NOTSRCCOPY;
			}
			
			pDC->BitBlt(	theRect.left, 
							theRect.top + 2, 
							theBitmap.bmWidth, 
							theBitmap.bmHeight, 
							&theMemDC, 
							0, 
							0, 
							dwMode);
			
			if( pOldBitmap != NULL )
			{
				theMemDC.SelectObject( pOldBitmap );
			}
		}
	}
	
	// Scoot over -- it can be checked
	
	if( m_theType == CA_TRANSMISSION_PLUGIN )
	{
		theRect.left += LOWORD( GetMenuCheckMarkDimensions() ) + 2;
	}
 	
	theRect.top += 2;
	
	//	Draw the "full featured only" bitmap from QCSharewareManager if this is a menu
	//	item for a CSpecial that only runs with full feature set and Eudora is not
	//	currently running with the full feature set.
	//	This is the only case that we need to handle for ESP. For MLM in the future
	//	we may need to slightly generalize this trick.
	CSpecial *	pSpecial = GetSpecialObject();
	bool		drewFullFeatureOnlyBitmap = false;
	if (pSpecial)
	{
		CTransAPI *		pTransAPI = pSpecial->GetTransAPI();
		if ( pTransAPI && pTransAPI->IsFullFeatureSetOnly() && !UsingFullFeatureSet() )
		{
			CPoint	pt(theRect.left, theRect.top);
			drewFullFeatureOnlyBitmap = DrawNeedsFullFeatureSetBitmap(bSelected, pt, theMemDC, pDC);
		}
	}

	// Draw the normal icon if we didn't draw the "full featured only" bitmap
	// Use the imagelist icon because it pulls out the correct 16x16 icon
	if ( !drewFullFeatureOnlyBitmap && (m_ImageOffset >= 0) )
	{
		extern QCPluginDirector	g_thePluginDirector;
		CPoint pt(theRect.left,theRect.top);
		g_thePluginDirector.m_ImageList.Draw(pDC, m_ImageOffset, pt, ILD_TRANSPARENT);
	}
	
	// Set text colors
	pDC->SetBkMode( TRANSPARENT );

	if( m_ImageOffset >= 0 )
	{
		theRect.left +=16;
	}
	
	theRect.left += 4;
	
	if (bGrayed && !bSelected)
	{
		//	It's disabled and not selected - draw with the disabled embossed shadow effect
		//	First draw the light highlight portion of the embossed effect
		pDC->SetTextColor( GetSysColor(COLOR_3DHILIGHT) );
		pDC->DrawText( m_szName, -1, &theRect, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX );
		
		//	Draw again shifted up and to the left for embossed shadow effect
		theRect += CPoint(-1,-1);
	}
	
	pDC->SetTextColor( crForeColor );

	pDC->DrawText( m_szName, -1, &theRect, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX );
}


void QCPluginCommand::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	CDC	theDC;

	ASSERT( ( m_theType == CA_TRANSLATE_PLUGIN ) || 
			( m_theType == CA_TRANSMISSION_PLUGIN ) ||
			( m_theType == CA_ATTACH_PLUGIN ) || 
			( m_theType == CA_SPECIAL_PLUGIN) );

	theDC.CreateCompatibleDC( NULL );
		
	CSize	theSize(theDC.GetTextExtent( m_szName, m_szName.GetLength()));

	if( m_theType == CA_TRANSMISSION_PLUGIN )
	{
		lpMeasureItemStruct->itemWidth = theSize.cx + LOWORD( GetMenuCheckMarkDimensions() ) + 20;
	}
	else
	{
		lpMeasureItemStruct->itemWidth = theSize.cx + 20; 
	}

	// Let's decide the Height of Menu rather than just making it 20 units. The fixed length will clip
	// the height of Menu Item under some display mode (High Contrast Black schemes etc) .- 02/04/00
	lpMeasureItemStruct->itemHeight = __max(20,GetSystemMetrics(SM_CYMENUSIZE ));
}


void QCPluginCommand::GetTransmissionID( 
LONG*	plModuleID, 
LONG*	plTranslatorID)
{
	ASSERT( m_pPluginObject );		
	if ( m_theType == CA_TRANSMISSION_PLUGIN || m_theType == CA_TRANSLATE_PLUGIN)		
		( ( CTranslator* ) m_pPluginObject ) ->GetID( *plModuleID, *plTranslatorID );
	else if ( m_theType == CA_ATTACH_PLUGIN || m_theType == CA_SPECIAL_PLUGIN)	
		( ( CAttacher* ) m_pPluginObject ) ->GetID( *plModuleID, *plTranslatorID );
}


// Returns CSpecial pointer if we're the correct type.
CSpecial * QCPluginCommand::GetSpecialObject() const
{
	if (m_theType == CA_SPECIAL_PLUGIN)
		return reinterpret_cast<CSpecial *>(m_pPluginObject);
	else
		return NULL;
}


	
CString	QCPluginCommand::GetToolTip(
COMMAND_ACTION_TYPE)
{
	return m_szName;
}

BOOL QCPluginCommand::GetIcon(HICON &icn)
{
	if (m_theType == CA_ATTACH_PLUGIN || m_theType == CA_SPECIAL_PLUGIN)
		return (((CAttacher *) m_pPluginObject)->GetIcon(icn));
	else if (m_theType == CA_TRANSLATE_PLUGIN || m_theType == CA_TRANSMISSION_PLUGIN)
		return (((CTranslator *) m_pPluginObject)->GetIcon(icn));
	else 
		return FALSE;
}

BOOL QCPluginCommand::InYourFace()
{
	if (m_theType == CA_ATTACH_PLUGIN || m_theType == CA_SPECIAL_PLUGIN)
		return (((CAttacher *) m_pPluginObject)->IsInYerFace());
	else if (m_theType == CA_TRANSLATE_PLUGIN || m_theType == CA_TRANSMISSION_PLUGIN)
		return (((CTranslator *) m_pPluginObject)->IsInYerFace());
	else 
		return FALSE;
}

BOOL QCPluginCommand::IsDefaultOn()
{
	if (m_theType == CA_TRANSMISSION_PLUGIN)
		return (((CTranslator *) m_pPluginObject)->GetFlags() & EMSF_DEFAULT_Q_ON);
	else 
		return FALSE;
}


BOOL QCPluginCommand::IsOnCompletion()
{
	if (m_theType == CA_TRANSMISSION_PLUGIN)
		return (((CTranslator *) m_pPluginObject)->GetFlags() & EMSF_Q4_COMPLETION);
	else 
		return FALSE;
}
