// QCPluginCommand.cpp: implementation of the QCPluginCommand class.
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
#include "trnslate.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


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

			pTAddress = new CTLAddress();
			pProtocol->GetMessageAsText(szTheMsg, TRUE);
			pTheMsg = new char[szTheMsg.GetLength()+1];
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
				pTAddress = new CTLAddress();

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

		if( ( pToggleData == NULL ) || ( pToggleData->m_pMenu == NULL ) || ( pToggleData->m_pszProperties == NULL ) )
		{
			ASSERT( 0 );
			return;
		}
		UINT flags = MF_CHECKED;
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

			if( ! ( ( CTranslator* ) m_pPluginObject )->QueueTrans(	EMSF_Q4_COMPLETION, 
																	*( pToggleData->m_pszProperties ), 
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
	if( lpDrawItemStruct->itemState & ODS_SELECTED )
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
	
	// Use the imagelist icon because it pulls out the correct 16x16 icon
	if (m_ImageOffset >= 0)
	{
		extern QCPluginDirector	g_thePluginDirector;
		CPoint pt(theRect.left,theRect.top);
		g_thePluginDirector.m_ImageList.Draw(pDC, m_ImageOffset, pt, ILD_TRANSPARENT);
	}
	
	// Set text colors
	pDC->SetTextColor( crForeColor );
	pDC->SetBkMode( TRANSPARENT );

	if( m_ImageOffset >= 0 )
	{
		theRect.left +=16;
	}
	
	theRect.left += 4;
	
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

	lpMeasureItemStruct->itemHeight = 20;
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
