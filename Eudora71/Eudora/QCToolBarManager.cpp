// QCToolBarManager.cpp: implementation of the QCToolBarManager class.
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "mainfrm.h"
#include "utils.h"
#include "QCToolBarManager.h"
#include "QCCustomToolbar.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "QCMbxCustomizeToolBarPage.h"
#include "QCCustomizeGeneralPage.h"
#include "QCCustomizePluginsPage.h"
#include "QCCustomizeRecipientsPage.h"
#include "QCCustomizeStationeryPage.h"
#include "QCCustomizePersPage.h"
#include "rs.h"
#include "QCCommandStack.h"
#include "QCSharewareManager.h"


extern QCCommandStack	g_theCommandStack;
extern QCPluginDirector	g_thePluginDirector;


#include "DebugNewHelpers.h"

#define SMALL_TOOLBARS		IDR_MAINFRAME16
#define SMALL_TOOLBARS_A	IDR_MAINFRAME16A
#define SMALL_TOOLBARS_B	IDR_MAINFRAME16B
#define LARGE_TOOLBARS		IDR_MAINFRAME32
#define LARGE_TOOLBARS_A	IDR_MAINFRAME32A
#define LARGE_TOOLBARS_B	IDR_MAINFRAME32B


static UINT BASED_CODE theFileButtons[] =
{
	ID_FILE_NEW,
	ID_FILE_OPEN,
	ID_FILE_OPEN_ATTACHMENT,
	ID_FILE_CLOSE,
	ID_FILE_SAVE,
	ID_FILE_SAVE_AS,
	ID_FILE_SAVE_AS_STATIONERY,
	ID_FILE_SENDQUEUEDMESSAGES,
	ID_FILE_CHECKMAIL,
	ID_FILE_PRINT_SETUP,
	ID_FILE_PRINT_PREVIEW,
	ID_FILE_PRINT,
	ID_FILE_PRINT_DIRECT,
	IDM_IMPORT_MAIL,
	ID_FAKE_APP_EXIT
};


static UINT BASED_CODE theEditButtons[] =
{
	ID_EDIT_UNDO,
	ID_EDIT_REDO,
	ID_EDIT_CUT,
	ID_EDIT_COPY,
	ID_EDIT_PASTE,
	ID_EDIT_PASTE_SPECIAL,
	ID_EDIT_PASTEASQUOTATION,
	ID_EDIT_CLEAR,
	ID_EDIT_INSERT_PICTURE,
	ID_EDIT_INSERT_HR,
	ID_EDIT_SELECT_ALL,
	ID_EDIT_WRAPSELECTION,
	ID_EDIT_FINISHNICKNAME,
	ID_EDIT_FIND_FINDTEXT,
	ID_EDIT_FIND_FINDTEXTAGAIN,
	ID_EDIT_FIND_FINDMSG,
	ID_EDIT_CHECKSPELLING,
	ID_SEARCH_WEB_FOR_SEL,
	ID_SEARCH_EUDORA_FOR_SEL,
	ID_SEARCH_MAILBOX_FOR_SEL,
	ID_SEARCH_MAILFOLDER_FOR_SEL
};


static UINT BASED_CODE theEditTextButtons[] =
{
	ID_EDIT_TEXT_PLAIN,
	ID_EDIT_TEXT_BOLD,
	ID_EDIT_TEXT_ITALIC,
	ID_EDIT_TEXT_UNDERLINE,
	ID_EDIT_TEXT_STRIKEOUT,
	ID_EDIT_TEXT_TT,
	ID_EDIT_TEXT_ADD_QUOTE,
	ID_EDIT_TEXT_REMOVE_QUOTE,
	ID_EDIT_TEXT_FONT,
	ID_EDIT_TEXT_MARGINS_NORMAL,
	ID_EDIT_TEXT_INDENT_IN,
	ID_EDIT_TEXT_INDENT_OUT,
	ID_EDIT_TEXT_LEFT,
	ID_EDIT_TEXT_CENTER,
	ID_EDIT_TEXT_RIGHT,
	ID_BLKFMT_BULLETTED_LIST,
	ID_EDIT_INSERT_LINK,
	ID_EDIT_TEXT_CLEAR,
	ID_EDIT_TEXT_FORMAT_PAINTER
};


static UINT BASED_CODE theMessageButtons[] =
{
	ID_MESSAGE_NEWMESSAGE,
	ID_MESSAGE_REPLY,
	ID_MESSAGE_REPLY_ALL,
	ID_MESSAGE_FORWARD,
	ID_MESSAGE_REDIRECT,
	ID_MESSAGE_SENDAGAIN,
	ID_MESSAGE_ATTACHFILE,
	ID_MESSAGE_SENDIMMEDIATELY,
	ID_MESSAGE_DELETE,
	ID_MESSAGE_UNDELETE,
	ID_MESSAGE_PURGE,
	ID_PREVIOUS_MESSAGE,
	ID_NEXT_MESSAGE,
	ID_JUNK,
	ID_NOT_JUNK
};


static UINT BASED_CODE theMessageChangeButtons[] =
{
	ID_MESSAGE_CHANGEQUEUEING,
	ID_MESSAGE_STATUS_UNREAD,
	ID_MESSAGE_STATUS_READ,
	ID_MESSAGE_STATUS_REPLIED,
	ID_MESSAGE_STATUS_FORWARDED,
	ID_MESSAGE_STATUS_REDIRECTED,
	ID_MESSAGE_STATUS_RECOVERED,
	ID_MESSAGE_STATUS_UNSENDABLE,
	ID_MESSAGE_STATUS_SENDABLE,
	ID_MESSAGE_STATUS_QUEUED,
	ID_MESSAGE_STATUS_TIME_QUEUED,
	ID_MESSAGE_STATUS_SENT,
	ID_MESSAGE_STATUS_UNSENT,
	ID_MESSAGE_PRIOR_HIGHEST,
	ID_MESSAGE_PRIOR_HIGH,
	ID_MESSAGE_PRIOR_NORMAL,
	ID_MESSAGE_PRIOR_LOW,
	ID_MESSAGE_PRIOR_LOWEST,
	ID_MESSAGE_LABEL_NONE,
	ID_MESSAGE_LABEL_1,
	ID_MESSAGE_LABEL_2,
	ID_MESSAGE_LABEL_3,
	ID_MESSAGE_LABEL_4,
	ID_MESSAGE_LABEL_5,
	ID_MESSAGE_LABEL_6,
	ID_MESSAGE_LABEL_7,
	ID_MESSAGE_SERVER_LEAVE,
	ID_MESSAGE_SERVER_FETCH,
	ID_MESSAGE_SERVER_DELETE,
	ID_MESSAGE_SERVER_FETCH_DELETE
};


static UINT BASED_CODE theSpecialButtons[] =
{
	ID_SPECIAL_FILTERMESSAGES,
	ID_RECHECK_JUNK,
	ID_SPECIAL_MAKE_FILTER,
	ID_SPECIAL_MAKENICKNAME,
	ID_SPECIAL_ADDASRECIPIENT,
	ID_SPECIAL_EMPTYTRASH,
	ID_SPECIAL_TRIMJUNK,
	ID_SPECIAL_COMPACTMAILBOXES,
	ID_SPECIAL_FORGETPASSWORD,
	ID_SPECIAL_CHANGEPASSWORD,
	ID_ABOUT_EMS_PLUGINS
};

static UINT BASED_CODE theToolsButtons[] =
{
	ID_WINDOW_FILTERS,
	IDM_VIEW_FILTER_REPORT,
	ID_WINDOW_MAILBOXES,
	IDM_VIEW_FILE_BROWSER,
	IDM_VIEW_STATIONERY,
	ID_TOOLS_VIEW_SIGNATURES,
	IDM_VIEW_PERSONALITIES,
	IDM_VIEW_TASK_STATUS,
	IDM_VIEW_TASK_ERROR,
	ID_WINDOW_NICKNAMES,
	ID_WINDOW_PH,
	ID_WINDOW_LINK_HISTORY,
	ID_WINDOW_USAGE_STATS,
	ID_SPECIAL_SETTINGS
};

static UINT BASED_CODE theWindowButtons[] =
{
	ID_WINDOW_CASCADE,
	ID_WINDOW_TILE_HORZ,
	ID_WINDOW_TILE_VERT,
	ID_WINDOW_ARRANGE,
	ID_WINDOW_SENDTOBACK
};

static UINT BASED_CODE theHelpButtons[] =
{
	ID_HELP_TOPICS,
	ID_CONTEXT_HELP,
	ID_HELP_TECHNICAL_SUPPORT,
	ID_TIP_OF_THE_DAY,
	ID_HELP_PAYMENT_AND_REGISTRATION,
	IDM_SYSTEM_CONFIGURATION,
	ID_APP_ABOUT
};

IMPLEMENT_DYNAMIC( QCToolBarManagerWithBM, SECToolBarManager )
IMPLEMENT_DYNAMIC( QCToolBarManager, QCToolBarManagerWithBM )
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCToolBarManagerWithBM::QCToolBarManagerWithBM( CFrameWnd*	pFrameWnd ) : SECToolBarManager(pFrameWnd) 
{

}

QCToolBarManagerWithBM::~QCToolBarManagerWithBM()
{

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCToolBarManager::QCToolBarManager( CFrameWnd*	pFrameWnd ) : QCToolBarManagerWithBM(pFrameWnd) 
{
	m_pToolBarClass = RUNTIME_CLASS(QCCustomToolBar);
}

QCToolBarManager::~QCToolBarManager()
{

}

BOOL QCToolBarManager::LoadToolBarResource()
{
	return QCEnableLargeBtns(GetIniShort(IDS_INI_SHOW_LARGEBUTTONS) != 0);
}


BOOL QCToolBarManager::LargeButtonsEnabled()
{
	return m_bLargeBmp;
}


// c_dalew 10/11/99 - This is a version of SECToolBarManager::EnableLargeBtns()
// that allows more than one bitmap resource.  This code is a combination of
// SECToolBarManager::EnableLargeBtns() which switches the toolbar to/from large
// buttons and QCToolBarManager::LoadToolBarResource() which loads a toolbar from
// multiple bitmap resources.
BOOL QCToolBarManager::QCEnableLargeBtns(BOOL bEnable, BOOL bForceSwitch)
{
	UINT*				pOldBmpItems;
	UINT				nOldBmpItems;
	UINT*				pBmpItems;
	CBitmap				theBitmapA;
	UINT*				pBmpItemsA;
	UINT				nBmpItemsA;
	INT					nImgWidthA;
	INT					nImgHeightA;
	CBitmap				theBitmapB;
	UINT*				pBmpItemsB;
	UINT				nBmpItemsB;
	INT					nImgWidthB;
	INT					nImgHeightB;
	CPaletteDC			theDestDC;
	CPaletteDC			theSrcDC;
	CBitmap				theNewBM;
	CBitmap*			pBmpOldDest;
	CBitmap*			pBmpOldSrc;
	BITMAP				bmp1;
	BITMAP				bmp2;
	BITMAP				bmp3;
	CDC*				pDC;
	UINT*				pOldIDs;
	POSITION			pos;
	QCPluginCommand*	pCommand;
	UINT				uID;

	// Always load if we've got no existing bitmaps
	if (m_pBmpItems && (!bForceSwitch && (bEnable == m_bLargeBmp)))
		return FALSE;

	m_bLargeBmp = bEnable;

	pOldBmpItems = m_pBmpItems;
	nOldBmpItems = m_nBmpItems;

	m_bmp.DeleteObject();

	// c_dalew 10/11/99 - Starting here we use the QCToolBarManager::LoadToolBarResource() code
	// which loads a toolbar from multiple bitmap resources.

	// c_dalew 10/11/99 - Since we have three separate bitmap resources, we can't simply
	// use m_lpszLargeBmpName.
	LPCTSTR lpszName;
	LPCTSTR lpszNameA;
	LPCTSTR lpszNameB;
	if(m_bLargeBmp)
	{
		lpszName = MAKEINTRESOURCE(LARGE_TOOLBARS);
		lpszNameA = MAKEINTRESOURCE(LARGE_TOOLBARS_A);
		lpszNameB = MAKEINTRESOURCE(LARGE_TOOLBARS_B);
	}
	else
	{
		lpszName = MAKEINTRESOURCE(SMALL_TOOLBARS);
		lpszNameA = MAKEINTRESOURCE(SMALL_TOOLBARS_A);
		lpszNameB = MAKEINTRESOURCE(SMALL_TOOLBARS_B);
	}

	if (!SECLoadToolBarResource(lpszName, m_bmp, m_pBmpItems, m_nBmpItems, m_nImgWidth, m_nImgHeight))
		return false;

	VERIFY(m_bmp.GetObject(sizeof(bmp1), &bmp1));

	int iToolbarDisplayFix = GetIniShort(IDS_INI_TOOLBAR_DISPLAY_FIX);
	if (iToolbarDisplayFix < 2)
	{
		if (!::SECLoadToolBarResource(	lpszNameA, 
										theBitmapA, 
										pBmpItemsA, 
										nBmpItemsA, 
										nImgWidthA, 
										nImgHeightA) || 
			!::SECLoadToolBarResource(	lpszNameB, 
										theBitmapB, 
										pBmpItemsB, 
										nBmpItemsB, 
										nImgWidthB, 
										nImgHeightB)	)
		{
			return FALSE;
		}

		// add in the plugins
		pos = g_thePluginDirector.GetFirstPluginPosition();
		
		while( pos )
		{
			pCommand = g_thePluginDirector.GetNextPlugin( pos );

			if( ( pCommand->GetType() == CA_ATTACH_PLUGIN ) ||
				( pCommand->GetType() == CA_SPECIAL_PLUGIN ) ||
				( pCommand->GetType() == CA_TRANSLATE_PLUGIN ) )
			{
				uID = g_theCommandStack.FindOrAddCommandID( pCommand, pCommand->GetType() );

				if( uID )
				{
					CopyButtonImage( &g_thePluginDirector.m_ImageList, pCommand->m_ImageOffset, uID );
				}
			}
		}

		// create the new bitmap
		VERIFY(m_bmp.GetObject(sizeof(bmp1), &bmp1));
		VERIFY(theBitmapA.GetObject(sizeof(bmp2), &bmp2));
		VERIFY(theBitmapB.GetObject(sizeof(bmp3), &bmp3));

		ASSERT( bmp2.bmHeight == bmp1.bmHeight );
		ASSERT( bmp3.bmHeight == bmp1.bmHeight );

		// Just checking to make sure that the bitmap has the same number of images as the list of IDs.
		ASSERT( (int)m_nBmpItems == bmp1.bmWidth / bmp1.bmHeight );
		ASSERT( (int)nBmpItemsA  == bmp2.bmWidth / bmp2.bmHeight );
		ASSERT( (int)nBmpItemsB  == bmp3.bmWidth / bmp3.bmHeight );

		theSrcDC.CreateCDC();

		pBmpOldSrc = ( CBitmap* ) theSrcDC.SelectObject( &m_bmp );	

		// create the destination bitmap
		theDestDC.CreateCDC();

		VERIFY(pDC = m_pFrameWnd->GetDC());
		theNewBM.CreateCompatibleBitmap( pDC, bmp1.bmWidth + bmp2.bmWidth + bmp3.bmWidth, bmp1.bmHeight );
		pBmpOldDest = ( CBitmap* ) theDestDC.SelectObject( &theNewBM );	
		VERIFY(m_pFrameWnd->ReleaseDC(pDC));

		if (iToolbarDisplayFix == 0)
		{
			// DRW - Some users see messed up toolbar buttons.  Try building the toolbar with
			// StretchDIBits() rather than BitBlt().  Who knows, maybe this will do the trick.
			SECDib		secDIB2;
			theSrcDC.SelectObject(&m_bmp);
			secDIB2.CreateFromBitmap(&theSrcDC, &m_bmp);
			::StretchDIBits(
				theDestDC.GetSafeHdc(),
				0, 0,
				bmp1.bmWidth, bmp1.bmHeight,
				0, 0,
				bmp1.bmWidth, bmp1.bmHeight,
				secDIB2.m_lpSrcBits,
				(LPBITMAPINFO)(&(secDIB2.m_lpBMI->bmiHeader)),
				DIB_RGB_COLORS,
				SRCCOPY);

			theSrcDC.SelectObject(theBitmapA);
			secDIB2.CreateFromBitmap(&theSrcDC, &theBitmapA);
			::StretchDIBits(
				theDestDC.GetSafeHdc(),
				bmp1.bmWidth, 0,
				bmp2.bmWidth, bmp2.bmHeight,
				0, 0,
				bmp2.bmWidth, bmp2.bmHeight,
				secDIB2.m_lpSrcBits,
				(LPBITMAPINFO)(&(secDIB2.m_lpBMI->bmiHeader)),
				DIB_RGB_COLORS,
				SRCCOPY);

			theSrcDC.SelectObject(theBitmapB);
			secDIB2.CreateFromBitmap(&theSrcDC, &theBitmapB);
			::StretchDIBits(
				theDestDC.GetSafeHdc(),
				bmp1.bmWidth + bmp2.bmWidth, 0,
				bmp3.bmWidth, bmp3.bmHeight,
				0, 0,
				bmp3.bmWidth, bmp3.bmHeight,
				secDIB2.m_lpSrcBits,
				(LPBITMAPINFO)(&(secDIB2.m_lpBMI->bmiHeader)),
				DIB_RGB_COLORS,
				SRCCOPY);
		}
		else
		{
			VERIFY(theDestDC.BitBlt(0, 0, bmp1.bmWidth, bmp1.bmHeight, &theSrcDC, 0, 0, SRCCOPY));

			theSrcDC.SelectObject(theBitmapA);
			VERIFY(theDestDC.BitBlt(bmp1.bmWidth, 0, bmp2.bmWidth, bmp1.bmHeight, &theSrcDC, 0, 0, SRCCOPY));

			theSrcDC.SelectObject(theBitmapB);
			VERIFY(theDestDC.BitBlt(bmp1.bmWidth + bmp2.bmWidth, 0, bmp3.bmWidth, bmp1.bmHeight, &theSrcDC, 0, 0, SRCCOPY));
		}

		theSrcDC.SelectObject( pBmpOldSrc );
		theDestDC.SelectObject( pBmpOldDest );

		// delete the source bitmaps
		VERIFY(m_bmp.DeleteObject());
		VERIFY(theBitmapA.DeleteObject());
		VERIFY(theBitmapB.DeleteObject());
		VERIFY(m_bmp.Attach(theNewBM.Detach()));

		// concatenate the id arrays
		pOldIDs = m_pBmpItems;
		pBmpItems = DEBUG_NEW UINT [ m_nBmpItems + nBmpItemsA + nBmpItemsB ];
		memcpy( pBmpItems, pOldIDs, m_nBmpItems * sizeof( UINT ) );
		memcpy( &( pBmpItems[m_nBmpItems] ), pBmpItemsA, nBmpItemsA * sizeof( UINT ) );
		memcpy( &( pBmpItems[m_nBmpItems+nBmpItemsA] ), pBmpItemsB, nBmpItemsB * sizeof( UINT ) );
		delete [] pOldIDs;
		delete [] pBmpItemsA;
		delete [] pBmpItemsB;

		// increment the counter
		m_nBmpItems += nBmpItemsA + nBmpItemsB;

		m_pBmpItems = pBmpItems;
	}

	// c_dalew 10/11/99 - And now back to the code taken from SECToolBarManager::EnableLargeBtns().

	if(pOldBmpItems)
	{
#ifdef _DEBUG
		// The large and small toolbar bitmap resources must 
		// have the same elements (command ids) in the same order 
		// (essentially 2 views of the same objects).
		// If an ASSERT fires here, double check your 2 toolbar bitmaps
		// for exact 1-1 correspondance.
		ASSERT(nOldBmpItems == m_nBmpItems);
		for(UINT i=0; i<m_nBmpItems; i++)
			ASSERT(pOldBmpItems[i] == m_pBmpItems[i]);
#endif

		delete[] pOldBmpItems;
		pOldBmpItems = NULL;

		pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
		BOOL bFound = FALSE;
		while(pos != NULL)
		{
			CControlBar* pBar = (CControlBar*) 
				m_pFrameWnd->m_listControlBars.GetNext(pos);

			if(pBar->IsKindOf(RUNTIME_CLASS(SECCustomToolBar)))
			{
				SECCustomToolBar* pToolBar = (SECCustomToolBar*) pBar;
				SetToolBarInfo(pToolBar);

				if(pToolBar->IsFloating())
				{
					CFrameWnd* pFrameWnd = pToolBar->GetParentFrame();
					pFrameWnd->RecalcLayout();
				}

				bFound = TRUE;
			}
		}

		if(bFound)
			m_pFrameWnd->DelayRecalcLayout();
	}

	return TRUE;
}


BOOL QCToolBarManagerWithBM::AddButtonImage( CBitmap& theNewButtonBitmap, UINT uNewID )
{
	CPaletteDC	theDestDC;
	CPaletteDC	theSrcDC;
	CBitmap		theNewBM;
	CBitmap*	pBmpOldDest;
	CBitmap*	pBmpOldSrc;
	BITMAP		bmp;
	CDC*		pDC;
	POSITION	pos;	
	UINT*		pOldIDs;

	// make a new bitmap from the current bitmap and from the 
	// theNewButtonBitmap parameter
	theSrcDC.CreateCDC();
	m_bmp.GetObject(sizeof(bmp), &bmp);
	pBmpOldSrc = ( CBitmap* ) theSrcDC.SelectObject( &m_bmp );	

	VERIFY(pDC = m_pFrameWnd->GetDC());
	theDestDC.CreateCDC();
	theNewBM.CreateCompatibleBitmap( pDC, bmp.bmWidth + m_nImgWidth, bmp.bmHeight );
	pBmpOldDest = ( CBitmap* ) theDestDC.SelectObject( &theNewBM );	
	VERIFY(m_pFrameWnd->ReleaseDC(pDC));

	SECDib		secDIB;

	secDIB.CreateFromBitmap(&theSrcDC, &m_bmp);
	::StretchDIBits(
		theDestDC.GetSafeHdc(),
		0, 0,
		bmp.bmWidth, bmp.bmHeight,
		0, 0,
		bmp.bmWidth, bmp.bmHeight,
		secDIB.m_lpSrcBits,
		(LPBITMAPINFO)(&(secDIB.m_lpBMI->bmiHeader)),
		DIB_RGB_COLORS,
		SRCCOPY);

	theSrcDC.SelectObject( theNewButtonBitmap );
	secDIB.CreateFromBitmap(&theSrcDC, &theNewButtonBitmap);
	::StretchDIBits(
		theDestDC.GetSafeHdc(),
		bmp.bmWidth, 0,
		m_nImgWidth, bmp.bmHeight,
		0, 0,
		m_nImgWidth, bmp.bmHeight,
		secDIB.m_lpSrcBits,
		(LPBITMAPINFO)(&(secDIB.m_lpBMI->bmiHeader)),
		DIB_RGB_COLORS,
		SRCCOPY);

	theSrcDC.SelectObject( pBmpOldSrc );	
	theDestDC.SelectObject( pBmpOldDest );

	m_bmp.DeleteObject();
	m_bmp.Attach( theNewBM.Detach() );

	// Array mapping Cmd IDs to bitmap indexes
	pOldIDs = m_pBmpItems;					
	m_pBmpItems = DEBUG_NEW UINT [ m_nBmpItems + 1 ];
	memcpy( m_pBmpItems, pOldIDs, m_nBmpItems * sizeof( UINT ) );
	m_pBmpItems[m_nBmpItems] = uNewID;
	delete [] pOldIDs;
	
	// increment the counter
	m_nBmpItems += 1;

	// fix up the toolbars
	
	pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	
	while(pos != NULL)
	{
		CControlBar* pBar = (CControlBar*) 
			m_pFrameWnd->m_listControlBars.GetNext(pos);

		if(pBar->IsKindOf(RUNTIME_CLASS(SECCustomToolBar)))
		{
			SetToolBarInfo( ( SECCustomToolBar* ) pBar );
		}
	}

	return TRUE;
}


void QCToolBarManagerWithBM::CopyButtonImage( UINT uBitmapIndex, UINT uID )
{
	CPaletteDC	theDestDC;
	CPaletteDC	theSrcDC;
	CBitmap		theNewBM;
	CBitmap*	pBmpOldDest;
	CBitmap*	pBmpOldSrc;
	BITMAP		bmp;
	CDC*		pDC;
	
	
	// create a new bitmap from the designated button index
	theSrcDC.CreateCDC();
	m_bmp.GetObject(sizeof(bmp), &bmp);
	pBmpOldSrc = ( CBitmap* ) theSrcDC.SelectObject( &m_bmp );	
	theDestDC.CreateCDC();
	VERIFY(pDC = m_pFrameWnd->GetDC());
	theNewBM.CreateCompatibleBitmap( pDC, m_nImgWidth, bmp.bmHeight );
	pBmpOldDest = ( CBitmap* ) theDestDC.SelectObject( &theNewBM );	
	theDestDC.BitBlt( 0, 0, m_nImgWidth, bmp.bmHeight, &theSrcDC, m_nImgWidth * uBitmapIndex, 0, SRCCOPY );
	theSrcDC.SelectObject( pBmpOldSrc );	
	theDestDC.SelectObject( pBmpOldDest );
	VERIFY(m_pFrameWnd->ReleaseDC(pDC));

	// add it in as a new image & id

	AddButtonImage( theNewBM, uID );	
}

// This will take info from an image list and put it in the toolbar
void QCToolBarManagerWithBM::CopyButtonImage( CImageList *pImageList, int imageOffset, UINT uID, BOOL drawEdge /*= FALSE*/ )
{
	CPaletteDC	theDestDC;
	CBitmap		theNewBM;
	CBitmap*	pBmpOldDest;
	CDC*		pDC;
	IMAGEINFO	ii;
	

	if (!pImageList || !pImageList->GetImageInfo(imageOffset, &ii))
	{
		ASSERT (0);
		return;
	}
	
	// create a new bitmap from the designated button index

	// Here's where we're drawing to...
	VERIFY(pDC = m_pFrameWnd->GetDC());
	theDestDC.CreateCDC();
	theNewBM.CreateCompatibleBitmap( pDC, m_nImgWidth, m_nImgHeight );
	pBmpOldDest = ( CBitmap* ) theDestDC.SelectObject( &theNewBM );	
	VERIFY(m_pFrameWnd->ReleaseDC(pDC));

	// Draw background
	COLORREF BackColor = GetSysColor(COLOR_BTNFACE);
	CBrush BackBrush(BackColor);
	CRect rect(0, 0, m_nImgWidth, m_nImgHeight);
	theDestDC.FillRect(&rect, &BackBrush);

	if (drawEdge)
	{
		COLORREF outlineColor = GetSysColor(COLOR_3DLIGHT);
		CBrush BackBrush = outlineColor;
		CRect outline(0, 0, 1, m_nImgHeight);
		theDestDC.FillRect(&outline, &BackBrush);
		outline.SetRect(0, 0, m_nImgWidth, 1);
		theDestDC.FillRect(&outline, &BackBrush);
		outline.SetRect(m_nImgWidth - 1, 0, m_nImgWidth, m_nImgHeight);
		theDestDC.FillRect(&outline, &BackBrush);
		outline.SetRect(0, m_nImgHeight - 1, m_nImgWidth, m_nImgHeight);
		theDestDC.FillRect(&outline, &BackBrush);
	}


	// Draw it in...
	CPoint pt(0,0);
	CRect imgRect = ii.rcImage;

	// Center it!
	if (rect.Width() > imgRect.Width())
		pt.x = ( (rect.Width() - imgRect.Width()) / 2 );
	if (rect.Height() > imgRect.Height())
		pt.y = ( (rect.Height() - imgRect.Height()) / 2 );
	
	pImageList->Draw(&theDestDC, imageOffset, pt, ILD_TRANSPARENT);

	// restore
	theDestDC.SelectObject( pBmpOldDest );

	// add it in as a new image & id
	AddButtonImage( theNewBM, uID );	
}



void QCToolBarManagerWithBM::DeleteAllUnusedBitmaps()
{
	CPtrList			theIDList;
	POSITION			theBarPos;
	POSITION			theIDPos;
	POSITION			theNextPos;
	QCCustomToolBar*	pToolBar;
	INT					iButton;
	BOOL				bFound;
	UINT				uButtonID;
	UINT				uID;
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;

	theBarPos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	

	while( theBarPos != NULL )
	{
		// get the next toolbar from the frame window
		pToolBar = ( QCCustomToolBar* ) m_pFrameWnd->m_listControlBars.GetNext( theBarPos );

		if( pToolBar->IsKindOf(RUNTIME_CLASS( QCCustomToolBar ) ) )
		{
			// go through the buttons and find those within the custom id range

			for( iButton = pToolBar->GetBtnCount() - 1; iButton >= 0; iButton-- )
			{
				uButtonID = pToolBar->GetItemID( iButton );
				
				if( ( uButtonID >= QC_FIRST_COMMAND_ID ) && ( uButtonID <= QC_LAST_COMMAND_ID ) )
				{
					// See if the id is in the list.	
					theIDPos = theIDList.GetHeadPosition();
					bFound = FALSE;
					theNextPos = theIDPos;
						
					while( ( theIDPos != NULL ) && !bFound )
					{
						uID = ( UINT ) theIDList.GetNext( theNextPos );
						
						if( uID > uButtonID )
						{
							break;
						}

						if( uID == uButtonID )
						{
							bFound = TRUE;
						}
						
						theIDPos = theNextPos;

					}

					if( !bFound )
					{
						// add the id
						if( theIDPos )
						{
							theIDList.InsertBefore( theIDPos, ( void* ) ( uButtonID ) );
						}
						else
						{
							theIDList.AddTail( ( void* ) ( uButtonID ) );
						}
					}
				}
			}					
		}
	}



	for( iButton = m_nBmpItems - 1; iButton >= 0; iButton-- )
	{
		// Fake maps into the director range ... but we don't want to 
		// remove it's bitmap because it's from the main toolbar
		if( ( m_pBmpItems[ iButton ] != ID_FAKE_APP_EXIT ) &&
			( m_pBmpItems[ iButton ] >= QC_FIRST_COMMAND_ID ) && 
			( m_pBmpItems[ iButton ] <= QC_LAST_COMMAND_ID ) ) 
		{

			if( g_theCommandStack.Lookup(	( unsigned short ) ( m_pBmpItems[ iButton ] ), 
											&pCommand, 
											&theAction ) )
			{
				if( ( theAction == CA_TRANSLATE_PLUGIN ) ||
					( theAction == CA_SPECIAL_PLUGIN ) ||
					( theAction == CA_ATTACH_PLUGIN ) )
				{
					continue;
				}
			}

			theIDPos = theIDList.GetHeadPosition();
			
			bFound = FALSE;

			while( theIDPos )
			{
				uID = ( UINT ) theIDList.GetNext( theIDPos );
				
				if( uID > m_pBmpItems[ iButton ] )
				{
					// don't look any further -- its not here
					break;
				}
				else if( uID == m_pBmpItems[ iButton ] )
				{
					bFound = TRUE;					
					break;
				}
			}
			
			if( !bFound )
			{
				DeleteBitmapAt( iButton );
			}
		}
	}
}



void QCToolBarManagerWithBM::DeleteBitmapAt( UINT iIndex )
{
	CPaletteDC	theDestDC;
	CPaletteDC	theSrcDC;
	CBitmap		theNewBitmap;
	CBitmap*	pBmpOldDest;
	CBitmap*	pBmpOldSrc;
	BITMAP		bmp;
	CDC*		pDC;
	UINT*		pOldIDs;
	UINT		i;
	UINT		j;
	POSITION	pos;
				
	// remove any buttons from the tool bars

	pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	
	// get the source bitmap
	theSrcDC.CreateCDC();
	m_bmp.GetObject(sizeof(bmp), &bmp);
	pBmpOldSrc = ( CBitmap* ) theSrcDC.SelectObject( &m_bmp );	

	// create the new bitmap
	VERIFY(pDC = m_pFrameWnd->GetDC());
	theDestDC.CreateCDC();
	theNewBitmap.CreateCompatibleBitmap( pDC, ( m_nBmpItems - 1 ) * m_nImgWidth, bmp.bmHeight );
	pBmpOldDest = ( CBitmap* ) theDestDC.SelectObject( &theNewBitmap );	
	VERIFY(m_pFrameWnd->ReleaseDC(pDC));
	
	if( iIndex > 0 )
	{
		// add everything before the button
		theDestDC.BitBlt( 0, 0, iIndex * m_nImgWidth, bmp.bmHeight, &theSrcDC, 0, 0, SRCCOPY );
	}
	
	if( iIndex < m_nBmpItems - 1 )
	{
		// add everything after
		theDestDC.BitBlt(	iIndex * m_nImgWidth, 
							0, 
							( m_nBmpItems - 1 - iIndex ) * m_nImgWidth, 
							bmp.bmHeight, 
							&theSrcDC, 
							( iIndex + 1 ) * m_nImgWidth, 
							0, 
							SRCCOPY );
	}

//	pDC->BitBlt( -1 *  ( ( iIndex - 5 ) * m_nImgWidth ), 100, ( m_nBmpItems - 1 ) * m_nImgWidth, bmp.bmHeight, &theDestDC, 0, 0, SRCCOPY );

	theDestDC.SelectObject( pBmpOldDest );	
	theSrcDC.SelectObject( pBmpOldSrc );	
	
	m_bmp.DeleteObject();
	m_bmp.Attach( theNewBitmap.Detach() );

	// Array mapping Cmd IDs to bitmap indexes
	pOldIDs = m_pBmpItems;					
	m_pBmpItems = DEBUG_NEW UINT [ m_nBmpItems - 1 ];
	for ( i = 0, j = 0; i < m_nBmpItems; i++ )
	{
		if( i != iIndex )
		{
			m_pBmpItems[ j++ ] = pOldIDs[ i ];
		}
	}

	delete [] pOldIDs;

	// decrement the bitmap counter
	m_nBmpItems -= 1;

	// fix up the tool bars

	pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	
	while(pos != NULL)
	{
		QCCustomToolBar* pBar = (QCCustomToolBar*) 
			m_pFrameWnd->m_listControlBars.GetNext(pos);

		if( pBar->IsKindOf(RUNTIME_CLASS( QCCustomToolBar) ) )
		{
			SetToolBarInfo( pBar );
			pBar->ResetImageOffsets();
		}
	}
}


////////////////////////////////////////////////////////////////////////
// QCPropSheetProc [static]
//
// Callback function that gets called by CPropertySheet::DoModal().  This
// allows us to hack the appearance of the toolbar customization dialog.
////////////////////////////////////////////////////////////////////////
static int CALLBACK QCPropSheetProc
(
	HWND hwndDlg,	// handle to the property sheet dialog box
	UINT uMsg,		// message identifier
	LPARAM lParam	// message parameter
)
{
	TRACE3("QCPropSheetProc: hwndDlg = %d, uMsg = %u, lParam = %ld\n", hwndDlg, uMsg, lParam);
	switch (uMsg)
	{
	case PSCB_PRECREATE:
		ASSERT(NULL == hwndDlg);
		break;
	case PSCB_INITIALIZED:
		{
			//
			// This is effectively the "OnInitDialog()" hook for a
			// property sheet, so disable the cancel button and
			// disable the system close button on the title bar.
			//
			ASSERT(hwndDlg != NULL);
			{
				// Hide OK button
				HWND hOKButton = ::GetDlgItem(hwndDlg, IDOK);
				ASSERT(hOKButton);
				::EnableWindow(hOKButton, FALSE);
				::ShowWindow(hOKButton, SW_HIDE);
			}

			POINT BtnPt = {0,0};

			{
				// Hide HELP button
				HWND hHELPbtn = ::GetDlgItem(hwndDlg, IDHELP);
				ASSERT(hHELPbtn);
				if (hHELPbtn)
				{
					::EnableWindow(hHELPbtn, FALSE);
					::ShowWindow(hHELPbtn, SW_HIDE);

					// Save the position of the button in BtnPt
					CRect BtnRct(0,0,0,0);
					::GetWindowRect(hHELPbtn, BtnRct);

					BtnPt.x = BtnRct.left;
					BtnPt.y = BtnRct.top;

					::ScreenToClient(hwndDlg, &BtnPt);
				}
			}

			{
				// Change text on Cancel button to "Close"
				HWND hCancelButton = ::GetDlgItem(hwndDlg, IDCANCEL);
				ASSERT(hCancelButton);
				::SetWindowText(hCancelButton, CRString(IDS_CLOSE));

				// Make this the default pushbutton.
				LONG lStyle = ::GetWindowLong(hCancelButton, GWL_STYLE);
				lStyle |= BS_DEFPUSHBUTTON;
				::SetWindowLong(hCancelButton, GWL_STYLE, lStyle);

				// Move the 'close' button to where the help btn was, if we saved the pos
				if (BtnPt.x > 0)
				{
					/*** This doesn't seem to work, don't know why. The btn stays were is wants and
					     ignored this MoveWindow call ***/

					CRect BtnRct(0,0,0,0);
					::GetWindowRect(hCancelButton, BtnRct);
					::MoveWindow(hCancelButton, BtnPt.x, BtnPt.y, BtnRct.Width(), BtnRct.Height(), TRUE);
				}
			}
//FORNOW			HMENU hSysMenu = ::GetSystemMenu(hwndDlg, FALSE);
//FORNOW			ASSERT(hSysMenu);
//FORNOW			::EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		}
		break;
	case PSCB_BUTTONPRESSED:
		// We don't care about which button was pressed in the dialog as there's
		// only one, "Close", and we save the changes when the dialog goes away
		break;
	default:
		ASSERT(0);
		break;
	}

	return 0;
}

void QCToolBarManager::OnCustomize() 
{
	INT								iMajorVersion;
	INT								iMinorVersion;
	CString							szDllPathname;
	SECToolBarSheet					toolbarSheet( IDS_TOOLBAR_CUSTOMIZE, ::AfxGetMainWnd() );
	QCCustomizeGeneralPage			theGeneralPage;
	QCMailboxCustomizeToolbarPage	theMailboxPage;
	QCCustomizePluginsPage			thePluginsPage;
	QCCustomizeRecipientsPage		theRecipientsPage;
	QCCustomizeStationeryPage		theStationeryPage;
	QCCustomizePersonalitiesPage	thePersonalitiesPage;
		
	GetSystemDLLVersion("comctl32.dll", iMajorVersion, iMinorVersion);

	ASSERT(NULL == toolbarSheet.m_psh.pfnCallback);
	toolbarSheet.m_psh.pfnCallback = QCPropSheetProc;
	toolbarSheet.m_psh.dwFlags |= PSH_USECALLBACK;		// use callback to disable Cancel button and system Close button

	toolbarSheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;		// nuke the Apply button
	toolbarSheet.m_psh.dwFlags &= ~(PSH_HASHELP);

	theGeneralPage.SetManager( this );
	
	// Have to make these non-temporaries because the SEC code doesn't make copies
	// of them, but instead just keeps pointers to the character buffers
	CRString FileLabel(IDS_TOOLBAR_CUSTOM_FILE);
	CRString EditLabel(IDS_TOOLBAR_CUSTOM_EDIT);
	CRString EditTextLabel(IDS_TOOLBAR_CUSTOM_EDIT_TEXT);
	CRString MessageLabel(IDS_TOOLBAR_CUSTOM_MESSAGE);
	CRString MessageChangeLabel(IDS_TOOLBAR_CUSTOM_MESSAGE_CHANGE);
	CRString SpecialLabel(IDS_TOOLBAR_CUSTOM_SPECIAL);
	CRString ToolsLabel(IDS_TOOLBAR_CUSTOM_TOOLS);
	CRString WindowLabel(IDS_TOOLBAR_CUSTOM_WINDOW);
	CRString HelpLabel(IDS_TOOLBAR_CUSTOM_HELP);

	theGeneralPage.DefineBtnGroup(FileLabel, NUMELEMENTS(theFileButtons), theFileButtons);
	theGeneralPage.DefineBtnGroup(EditLabel, NUMELEMENTS(theEditButtons), theEditButtons);
	theGeneralPage.DefineBtnGroup(EditTextLabel, NUMELEMENTS(theEditTextButtons), theEditTextButtons);
	theGeneralPage.DefineBtnGroup(MessageLabel, NUMELEMENTS(theMessageButtons), theMessageButtons );
	theGeneralPage.DefineBtnGroup(MessageChangeLabel, NUMELEMENTS(theMessageChangeButtons), theMessageChangeButtons);
	theGeneralPage.DefineBtnGroup(SpecialLabel, NUMELEMENTS(theSpecialButtons), theSpecialButtons);
	theGeneralPage.DefineBtnGroup(ToolsLabel, NUMELEMENTS(theToolsButtons), theToolsButtons);
	theGeneralPage.DefineBtnGroup(WindowLabel, NUMELEMENTS(theWindowButtons), theWindowButtons);
	theGeneralPage.DefineBtnGroup(HelpLabel, NUMELEMENTS(theHelpButtons), theHelpButtons);
	
	toolbarSheet.AddPage(&theGeneralPage);

	if( ( iMajorVersion > 4 ) || 
		( ( iMajorVersion == 4 ) && ( iMinorVersion >= 70 ) ) )
	{
		theMailboxPage.SetManager( this );
		toolbarSheet.AddPage( &theMailboxPage );

		thePluginsPage.SetManager( this );
		toolbarSheet.AddPage( &thePluginsPage );

		theRecipientsPage.SetManager( this );
		toolbarSheet.AddPage( &theRecipientsPage );

		theStationeryPage.SetManager( this );
		toolbarSheet.AddPage( &theStationeryPage );

		thePersonalitiesPage.SetManager( this );
		toolbarSheet.AddPage( &thePersonalitiesPage );
	}
	
	toolbarSheet.DoModal();

	//
	// ALWAYS save the new state after the dialog closes.
	//
	((CMainFrame*)m_pFrameWnd)->SaveBarState(_T("ToolBar"));

	DeleteAllUnusedBitmaps();
}


// c_dalew 10/11/99 - Override of SECToolBarManager::LoadState() that calls
// QCEnableLargeBtns().  I really hate doing this, but SECToolBarManager::EnableLargeBtns()
// is not virtual but I need to override it to load the toolbar from multiple resources.
void QCToolBarManager::QCLoadState(LPCTSTR lpszProfileName)
{
	CWinApp* pApp;
	TCHAR szSection[256];
	BOOL bVal;

	// c_dalew 10/11/99 - Had to add this here.
	// INI file entries.
	static const TCHAR szToolBarManagerSection[]     = _T("%s-ToolBarManager");
	static const TCHAR szToolTips[]       = _T("ToolTips");
	static const TCHAR szCoolLook[]       = _T("CoolLook");
	static const TCHAR szLargeButtons[]   = _T("LargeButtons");

	wsprintf(szSection, szToolBarManagerSection, lpszProfileName);

	pApp = AfxGetApp();
	ASSERT(pApp != NULL);

	// Get the tooltips state.
	bVal = (BOOL)pApp->GetProfileInt(szSection, 
												   szToolTips, m_bToolTips);
	EnableToolTips(bVal);
	
	// Get the cool look mode
	bVal = (BOOL)pApp->GetProfileInt(szSection,
												   szCoolLook, m_bCoolLook);
	EnableCoolLook(bVal);

	// Determine whether we should use large buttons
	bVal = (BOOL)pApp->GetProfileInt(szSection,
												   szLargeButtons, m_bLargeBmp);
	// c_dalew 10/11/99 - This is the only change I made to this method's functionality.
	QCEnableLargeBtns(bVal);

	// Iterate through all SECCustomToolBars setting
	// us as its manager.
	int nToolbars = 0;
	CPtrList& list = m_pFrameWnd->m_listControlBars;
	POSITION pos = list.GetHeadPosition();
	while(pos != NULL)
	{
		SECCustomToolBar* pBar = (SECCustomToolBar*) list.GetNext(pos);
		if(pBar->IsKindOf(RUNTIME_CLASS(SECCustomToolBar))) {
			pBar->SetManager(this);
			nToolbars++;
		}
	}

	// If no toolbars are shown, show the default ones
	if (nToolbars == 0)
		SetDefaultDockState();

}


void QCToolBarManager::LoadState(LPCTSTR lpszProfileName)
{
	CString				szSection;
	SECCustomToolBar*	pToolBar;
	POSITION			pos;

	// c_dalew 10/11/99 - Call our version of LoadState().  See note on QCLoadState()
	// for why I do this.
	QCLoadState( lpszProfileName );

	if (GetSharewareMode() != SWM_MODE_LIGHT)
	{
		pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	
		while(pos != NULL)
		{
			pToolBar = (SECCustomToolBar*) m_pFrameWnd->m_listControlBars.GetNext(pos);

			if( pToolBar->IsKindOf( RUNTIME_CLASS( QCCustomToolBar ) ) )
			{
				szSection.Format( "%s-BarID%lu", lpszProfileName, pToolBar->GetDlgCtrlID() );
				
				( ( QCCustomToolBar* ) pToolBar )->LoadCustomInfo( szSection );
			}
		}
	}
}



void QCToolBarManager::SaveState(LPCTSTR lpszProfileName)
{
	CString				szSection;
	SECCustomToolBar*	pToolBar;
	POSITION			pos;

	SECToolBarManager::SaveState( lpszProfileName );

	pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	
	while(pos != NULL)
	{
		pToolBar = (SECCustomToolBar*) m_pFrameWnd->m_listControlBars.GetNext(pos);

		if( pToolBar->IsKindOf( RUNTIME_CLASS( QCCustomToolBar ) ) )
		{
			szSection.Format( "%s-BarID%lu", lpszProfileName, pToolBar->GetDlgCtrlID() );
			
			( ( QCCustomToolBar* ) pToolBar )->SaveCustomInfo( szSection );
		}
	}
}



BOOL QCToolBarManager::ConvertOldStuff()
{
	QCCustomToolBar*	pToolBar;
	POSITION			pos;

	pos = m_pFrameWnd->m_listControlBars.GetHeadPosition();
	
	while(pos != NULL)
	{
		pToolBar = (QCCustomToolBar*) m_pFrameWnd->m_listControlBars.GetNext(pos);

		if( pToolBar->IsKindOf( RUNTIME_CLASS( QCCustomToolBar ) ) )
		{
			return pToolBar->ConvertOldStuff();
		}
	}

	return FALSE;
}


SECCustomToolBar* QCToolBarManager::CreateUserToolBar(
										LPCTSTR lpszTitle /* = NULL */,
										DWORD dwStyle,
										DWORD dwExStyle)
{
	// Creates a 'user' toolbar. First find an ID to use
	int nID = SEC_IDW_FIRST_USER_TOOLBAR;
	BOOL bFound = TRUE;
	CPtrList& list = m_pFrameWnd->m_listControlBars;
	while(bFound)
	{
		bFound = FALSE;
		POSITION pos = list.GetHeadPosition();
		while(pos != NULL)
		{
			CControlBar* pBar = (CControlBar*) list.GetNext(pos);
			if(pBar->IsKindOf(RUNTIME_CLASS(SECCustomToolBar)))
			{
				if(pBar->GetDlgCtrlID() == nID)
				{
					// This ID is in use ... try the next one.
					nID++;
					bFound = TRUE;
					break;
				}
			}
		}
	}

	if(nID >= (AFX_IDW_TOOLBAR + 0x100))
		// Too many toolbars
		return NULL;


	ASSERT(m_pToolBarClass != NULL);
	QCCustomToolBar* pToolBar = (QCCustomToolBar*) 
									m_pToolBarClass->CreateObject();
	if(!pToolBar)
		return NULL;

	ASSERT_KINDOF(SECCustomToolBar, pToolBar);

	pToolBar->SetManager(this);

	// Give the toolbar a default title.
	TCHAR szBuffer[128];
	if(!lpszTitle)
		wsprintf(szBuffer, _T("ToolBar%d"), nID - SEC_IDW_FIRST_USER_TOOLBAR + 1);
	else
		lstrcpy(szBuffer, lpszTitle);

	if (dwStyle == -1)
	{
		dwStyle = WS_VISIBLE | WS_CHILD | CBRS_SIZE_DYNAMIC | CBRS_TOP;
	}
	if (dwExStyle == -1)
	{
		dwExStyle = 0L;
		if(m_bFlyBy)
			dwStyle |= CBRS_FLYBY;
		if(m_bToolTips)
			dwStyle |= CBRS_TOOLTIPS;
		if(m_bCoolLook)
			dwExStyle |= (CBRS_EX_COOLBORDERS | CBRS_EX_GRIPPER);
	}
	
	// had to over write this function so that QCCustomToolBar::CreateEx 
	// was called instead of SECCustomToolBar::CreateEx
	pToolBar->CreateEx(dwExStyle, m_pFrameWnd, dwStyle, nID, szBuffer);
	
	SetToolBarInfo(pToolBar);

	pToolBar->EnableConfigMode(m_bConfig);
	pToolBar->EnableDocking(CBRS_ALIGN_ANY);
	pToolBar->m_bAutoDelete = TRUE;

	return pToolBar;
}


CControlBar* QCToolBarManager::DynCreateControlBar(SECControlBarInfo* pBarInfo)
{
	CControlBar* pBar = NULL;

	switch ( pBarInfo->m_dwBarTypeID )
	{
	case CBT_SECCUSTOMTOOLBAR:
		{
			// It's an SECCustomToolBar - better create it.
			ASSERT(m_pToolBarClass != NULL);
			QCCustomToolBar* pToolBar = (QCCustomToolBar*) 
											m_pToolBarClass->CreateObject();
			ASSERT_KINDOF(QCCustomToolBar, pToolBar);

			pToolBar->SetManager(this);
			pToolBar->m_bAutoDelete = TRUE;
			DWORD dwStyle = WS_VISIBLE | WS_CHILD | CBRS_TOP | 
							CBRS_SIZE_DYNAMIC;
			DWORD dwExStyle = 0L;
			if(m_bFlyBy)
				dwStyle |= CBRS_FLYBY;
			if(m_bToolTips)
				dwStyle |= CBRS_TOOLTIPS;
			if(m_bCoolLook)
				dwExStyle = (CBRS_EX_COOLBORDERS | CBRS_EX_GRIPPER);
			
			// QC change :  added the window title

			if( pBarInfo->m_pBarInfoEx != NULL ) 
			{
				ASSERT_KINDOF( SECCustomToolBarInfoEx, pBarInfo->m_pBarInfoEx );

				VERIFY(pToolBar->CreateEx(dwExStyle, m_pFrameWnd, dwStyle,
										  pBarInfo->m_nBarID,
										  ( ( SECCustomToolBarInfoEx* ) ( pBarInfo->m_pBarInfoEx ) )->m_strBarName));
			}
			else
			{
				VERIFY(pToolBar->CreateEx(dwExStyle, m_pFrameWnd, dwStyle,
										  pBarInfo->m_nBarID));
			}

			SetToolBarInfo(pToolBar);

			pToolBar->EnableDocking(CBRS_ALIGN_ANY);
			pBar = pToolBar;
		}
		break;
	default:
		pBar = SECControlBarManager::DynCreateControlBar(pBarInfo);
		break;
	}

	return pBar;
}
