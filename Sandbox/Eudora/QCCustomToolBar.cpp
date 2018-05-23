// QCCustomToolBar.cpp: implementation of the QCCustomToolBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QCCustomToolBar.h"
#include "QCToolBarManager.h"

#include "helpcntx.h"
#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCPluginCommand.h"
#include "QCPluginDirector.h"
#include "QCRecipientCommand.h"
#include "QCRecipientDirector.h"
#include "QCSignatureCommand.h"
#include "QCSignatureDirector.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"
#include "QCPersonalityCommand.h"
#include "QCPersonalityDirector.h"
#include "resource.h"
#include "rs.h"
#include "SafetyPal.h"
#include "MainFrm.h"
#include "utils.h"
#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern QCCommandStack			g_theCommandStack;
extern QCMailboxDirector		g_theMailboxDirector;
extern QCPluginDirector			g_thePluginDirector;
extern QCRecipientDirector		g_theRecipientDirector;
extern QCSignatureDirector		g_theSignatureDirector;
extern QCStationeryDirector		g_theStationeryDirector;
extern QCPersonalityDirector	g_thePersonalityDirector;

//			if (ID_APP_EXIT == lID)
//				lID = ID_FAKE_APP_EXIT;
#define IDOLD_EDIT_TEXT_SMALLER            32877
#define IDOLD_EDIT_TEXT_BIGGER             32878
#define IDOLD_EDIT_TEXT_HANG_INDENT_IN     32891
#define IDOLD_EDIT_TEXT_HANG_INDENT_OUT    32892
#define IDOLD_SERVER_RETRIEVE              33997
#define IDOLD_SERVER_DELETE                33998
#define IDOLD_SHOW_ALL_HEADERS             33999
#define IDOLD_POPUP_ADD_RECIPIENT          34000
#define IDOLD_POPUP_REMOVE_RECIPIENT       34001
#define IDOLD_EDIT_MESSAGE                 34002
#define IDOLD_PRIORITY_HIGHEST             40031
#define IDOLD_PRIORITY_HIGH                40032
#define IDOLD_PRIORITY_NORMAL              40033
#define IDOLD_PRIORITY_LOW                 40034
#define IDOLD_PRIORITY_LOWEST              40035
#define IDOLD_PRIORITY_LOWER               40036
#define IDOLD_PRIORITY_RAISE               40037
#define IDOLD_MESSAGE_FCC                  40042

#define DIM( a ) ( sizeof( a )/sizeof( a[0] ) )

static LONG theMap[][2] =
{
	{ IDOLD_EDIT_TEXT_SMALLER, 0 },
	{ IDOLD_EDIT_TEXT_BIGGER, 0 },
	{ IDOLD_EDIT_TEXT_HANG_INDENT_IN, },
	{ IDOLD_EDIT_TEXT_HANG_INDENT_OUT, 0 },
	{ IDOLD_SERVER_RETRIEVE, ID_MESSAGE_SERVER_FETCH },
	{ IDOLD_SERVER_DELETE, ID_MESSAGE_SERVER_DELETE },
	{ IDOLD_SHOW_ALL_HEADERS, ID_BLAHBLAHBLAH },
	{ IDOLD_POPUP_ADD_RECIPIENT, 0 },
	{ IDOLD_POPUP_REMOVE_RECIPIENT, 0 },
	{ IDOLD_EDIT_MESSAGE, ID_EDIT_MESSAGE },
	{ IDOLD_PRIORITY_HIGHEST, ID_PRIORITY_HIGHEST },
	{ IDOLD_PRIORITY_HIGH, ID_PRIORITY_HIGH },
	{ IDOLD_PRIORITY_NORMAL, ID_PRIORITY_NORMAL },
	{ IDOLD_PRIORITY_LOW, ID_PRIORITY_LOW },
	{ IDOLD_PRIORITY_LOWEST, ID_PRIORITY_LOWEST },
	{ IDOLD_PRIORITY_LOWER, ID_PRIORITY_LOWER },
	{ IDOLD_PRIORITY_RAISE, ID_PRIORITY_RAISE },
	{ IDOLD_MESSAGE_FCC, 0 }
};

static const int nMinHorzDrag = 1;				// Min. drag distance before we

IMPLEMENT_DYNCREATE( QCCustomToolBar, SECCustomToolBar )

BEGIN_MESSAGE_MAP(QCCustomToolBar, SECCustomToolBar)
	//{{AFX_MSG_MAP(QCCustomToolBar)
	ON_MESSAGE(WM_USER_QUERY_PALETTE, OnMyQueryNewPalette)
	ON_WM_PALETTECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCCustomToolBar::QCCustomToolBar()
{
	// register as a command client
	g_theMailboxDirector.Register( this );
	g_theRecipientDirector.Register( this );
	g_theStationeryDirector.Register( this );
	g_theSignatureDirector.Register( this );
	g_thePersonalityDirector.Register( this );
}

QCCustomToolBar::~QCCustomToolBar()
{

}


BOOL  QCCustomToolBar::AcceptDrop() const
{
	return SECCustomToolBar::AcceptDrop();
}


void QCCustomToolBar::DropButton(CRect& dropRect, SECStdBtn* pDragBtn, BOOL bAdd)
{
	// A button has been dropped over me!

	int		nRowStart;
	int		nRowEnd;
	int		nIndex;
	UINT	u;

	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, (m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE,
				   (m_dwStyle & CBRS_ORIENT_VERT) ? TRUE : FALSE);

	UINT nID = pDragBtn->m_nID;
	ULONG ulData = pDragBtn->m_ulData;

	if( ( pDragBtn->m_ulData != 0 ) && ( pDragBtn->m_ulData != nID ) )
	{
		// see if this button already exists	
		for ( u = 0; u < (UINT)m_nBmpItems; u++ )
		{
			if( nID == m_pBmpItems[ u ] )
			{
				pDragBtn->m_nImage = u;
				break;
			}
		}

		if( u == (UINT) m_nBmpItems )
		{
			// the button doesn't exist

			// find the index of the button in the toolbar bitmap using the orignal id
			for ( u = 0; u < (UINT)m_nBmpItems; u++ )
			{
				if( pDragBtn->m_ulData == m_pBmpItems[ u ] )
				{
					break;
				}
			}

			// if found, add the button 
			if( u < (UINT) m_nBmpItems )
			{	
				( ( QCToolBarManager* ) m_pManager )->CopyButtonImage( u, nID );
//				( ( QCToolBarManager* ) m_pManager )->SetToolBarInfo( this );
			}
		}
	}


	SECCustomToolBar* pConfigFocus = GetConfigFocus();

	// Find where to add the button
	if(GetBtnCount() == 0)
	{
		// Dropping on an empty toolbar
		nIndex = 0;

// Do we need this?
//		if(!bAdd && pConfigFocus != NULL)
//			pConfigFocus->RemoveButton(pConfigFocus->m_nDown);
	}

	else if(GetBtnCount() == 1 && pConfigFocus == this && !bAdd)
	{
		// Button dropped on same toolbar, when only one button on toolbar.
		// Leave all alone.
		return;
	}
	else if(dropRect.bottom < rect.top)
	{
		// Button dropped above all current buttons. Add at beginning of
		// buttons with a separator on right.
		AddButton(0, 0, TRUE, TRUE);			// Add sep.
		nIndex = 0;
	}
	else if(dropRect.top > rect.bottom)
	{
		// Button dropped below all buttons. Add at end of buttons with
		// a separator on left.
		AddButton(GetBtnCount(), 0, TRUE, TRUE);// Add sep.
		nIndex = GetBtnCount();
	}
	else
	{
		// Get the row start and end which the button was dropped in.
		FindRow(rect, dropRect, nRowStart, nRowEnd);
		
		// Deal with the case of a slight drag to left/right - remove/add
		// separators.
		if(SmallDrag(dropRect, nRowStart, nRowEnd, bAdd))
			// It was a small drag - so nothing else to do.
			return;

		// Find the index of the button to insert before.
		int nRight = rect.left;
		nIndex = nRowStart;
		if(dropRect.left >= rect.left)
		{
			for( ; nIndex<nRowEnd; nIndex++)
			{
				nRight += m_btns[nIndex]->m_cx;
				if(dropRect.left < nRight)
					break;
			}

			nIndex++;
		}

		if(!bAdd)
		{
			// We're not adding, so need to remove the button from the
			// old toolbar.
			if(pConfigFocus != this && pConfigFocus != NULL)
			{
// do we need this?
//				pConfigFocus->RemoveButton(pConfigFocus->m_nDown);	
			}
			else if(pConfigFocus == this)
			{
				// Special case, if the button is being dragged within me.
				// Need to adjust indexes if they have been affected.
				if(nIndex == m_nDown)
					return;

				RemoveButton( m_nDown, TRUE, &nIndex );
			}
		}

		// Are we dropping on top of a separator		
		if(nIndex < GetBtnCount())
		{
			if((m_btns[nIndex]->m_nStyle & TBBS_SEPARATOR) &&
			   !(m_btns[nIndex]->m_nStyle & TBBS_WRAPPED))
			{
				CRect itemRect;
				GetItemRect(nIndex, itemRect);
				if((dropRect.left + nMinHorzDrag) < itemRect.left &&
				   (dropRect.right + nMinHorzDrag) > itemRect.right)
				{
					// Completely over separator, so button will replace separator
					RemoveButton(nIndex, FALSE);
				}
			}
		}
	}

	// Add the button at the calculated location - and give it the config focus.
	AddButton(nIndex, nID);
	
	m_btns[nIndex]->m_ulData = ulData;

	if(!m_bAltDrag)
		// Note that when perform an "ALT" drag operation, when the operation
		// is complete no button has the config focus since we are not in
		// customize mode.
		SetConfigFocus(nIndex, TRUE);
}



void QCCustomToolBar::GetBarInfoEx(SECControlBarInfo* pInfo)
{
	SECCustomToolBarInfoEx* pEx;
	pEx = new SECCustomToolBarInfoEx;
	ASSERT(pEx != NULL);
	GetWindowText(pEx->m_strBarName);			// My title


	if(GetBtnCount())
	{
		// Build an array describing all buttons
		for(int i=0; i<GetBtnCount(); i++)
		{
			if( ( m_btns[i]->m_ulData == 0 ) ||
				( m_btns[i]->m_nID == m_btns[i]->m_ulData ) )
			{
				// only process non-customized buttons
				SECCustomToolBarInfoEx::BtnInfo* pBtnInfo = 
					new SECCustomToolBarInfoEx::BtnInfo;
				pBtnInfo->m_nID = (m_btns[i]->m_nStyle & TBBS_SEPARATOR) ? 
					ID_SEPARATOR : m_btns[i]->m_nID;

				ASSERT(pBtnInfo->m_pExtraInfo == NULL);

				// Get the button to add in any state info.
				m_btns[i]->GetBtnInfo(&pBtnInfo->m_nSizeExtraInfo, 
									  &pBtnInfo->m_pExtraInfo);

				pEx->m_btnInfo.Add(pBtnInfo);
			}
		}		
	}
	pInfo->m_pBarInfoEx = pEx;
}


void QCCustomToolBar::SaveCustomInfo(
LPCSTR	szSection )
{
	CString				szValue;
	INT					iCurrentButton;
	UINT				nCustomButtons = 0;
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;
	LPCTSTR				szCommandType;
	LPCTSTR				szName;
	CString				szEntry;
	CWinApp*			pApp = AfxGetApp();

	for( iCurrentButton = 0; iCurrentButton < GetBtnCount(); iCurrentButton ++ )
	{	
		if( ( m_btns[ iCurrentButton ]->m_ulData != 0 ) &&
			( m_btns[ iCurrentButton ]->m_nID != m_btns[ iCurrentButton ]->m_ulData ) )
		{
#ifdef _DEBUG
			// For debugging, grab a copy of the ID
			WORD tmp = ( WORD ) ( m_btns[ iCurrentButton ]->m_nID );
#endif // _DEBUG

			if( g_theCommandStack.Lookup( ( WORD ) ( m_btns[ iCurrentButton ]->m_nID ), &pCommand, &theAction ) == FALSE )
			{
				// You're here because this button looks to be custom (m_ulData != 0) && (m_nID != m_ulData), but
				// we cannot find the ID in our command stack.
				ASSERT(0);
				continue;
			}
			
			if( pCommand->IsKindOf( RUNTIME_CLASS( QCMailboxCommand ) ) )
			{
				szCommandType = "Mailbox";
				szName = ( ( QCMailboxCommand* ) pCommand )->GetPathname();
			}
			else if( pCommand->IsKindOf( RUNTIME_CLASS( QCRecipientCommand ) ) )
			{
				szCommandType = "Recipient";
				szName = ( ( QCRecipientCommand* ) pCommand )->GetName();
			}
			else if( pCommand->IsKindOf( RUNTIME_CLASS( QCSignatureCommand ) ) )
			{
				szCommandType = "Signature";
				szName = ( ( QCSignatureCommand* ) pCommand )->GetPathname();
			}
			else if( pCommand->IsKindOf( RUNTIME_CLASS( QCStationeryCommand ) ) )
			{
				szCommandType = "Stationery";
				szName = ( ( QCStationeryCommand* ) pCommand )->GetPathname();
			}
			else if( pCommand->IsKindOf( RUNTIME_CLASS( QCPluginCommand ) ) )
			{
				switch( ( ( QCPluginCommand* ) pCommand )->GetType() )
				{
					case CA_ATTACH_PLUGIN:
						szCommandType = "AttachmentPlugin";
					break;
					case CA_SPECIAL_PLUGIN:
						szCommandType = "SpecialPlugin";
					break;
					case CA_TRANSLATE_PLUGIN:
						szCommandType = "TranslatePlugin";
					break;
					case CA_TRANSMISSION_PLUGIN:
						szCommandType = "TransmissionPlugin";
					break;
					default:
						ASSERT( 0 );
						continue;
					break;
				}
				szName = ( ( QCPluginCommand* ) pCommand )->GetName();				
			}
			else if( pCommand->IsKindOf( RUNTIME_CLASS( QCPersonalityCommand ) ) )
			{
				szCommandType = "Personality";
				szName = ( ( QCPersonalityCommand* ) pCommand )->GetName();
			}
			else
			{
				ASSERT( 0 );
				continue;
			}
			
			szValue.Format( "%lu,%u,%s,%u,%s",	m_btns[ iCurrentButton ]->m_ulData,
												iCurrentButton,
												szCommandType,
												theAction,
												szName );
			
			szEntry.Format( "cmd%u", nCustomButtons );
			
			if (pApp->WriteProfileString(szSection, szEntry, szValue))
				nCustomButtons++;

			szValue.Format( "-2" );
			szEntry.Format( "btn%u", iCurrentButton );
			pApp->WriteProfileString(szSection, szEntry, szValue);
		}
		else
		{
			szValue.Format( "%ld",	m_btns[ iCurrentButton ]->m_nID );
			szEntry.Format( "btn%u", iCurrentButton );
			pApp->WriteProfileString(szSection, szEntry, szValue);
		}	
	}
	
	szEntry.Format( "btn%u", iCurrentButton );
	pApp->WriteProfileString(szSection, szEntry, NULL);

	szValue.Format( "%u", nCustomButtons );
	pApp->WriteProfileString(szSection, "CustomButtonCount", szValue);
}


void QCCustomToolBar::LoadCustomInfo(
LPCSTR	szSection )
{
	CString				szKeyName;
	char				szValue[1024];
	INT					i;
	CString				szINIFileName;
	char*				p;
	INT					nFailed;
	LONG				lOrigCmdID;
	char*				pNext;
	INT					iPosition;
	CString				szCommandType;
	INT					iAction;
	CString				szCommandName;
	INT					iCustomCount;
	QCCommandObject*	pCommand;
	UINT				uCommandID;
	UINT				uSrcIndex;

	i = 0;
	szINIFileName = AfxGetApp()->m_pszProfileName;
	nFailed = 0;
	iCustomCount = AfxGetApp()->GetProfileInt( szSection, "CustomButtonCount", 0 );
		
	for( i = 0; i < iCustomCount; i++ )
	{
		szKeyName.Format( "cmd%d", i );
		szValue[0] = '\0';

		::GetPrivateProfileString( szSection, szKeyName, "", szValue, 1024, szINIFileName );
 
 		if( szValue[0] == '\0' )
		{
			break;
		}
		
		// parse the string
		// bitmap command id, position
		
		// get the bitmap command id
		p = strchr( szValue, ',' );
		
		if( p == NULL )
		{
			// invalid entry
			nFailed ++;
			continue;
		}

		*p = '\0';
		
		lOrigCmdID = atol( szValue );

		if( lOrigCmdID <= 0 )
		{
			nFailed++;
			continue;
		}

		for( uSrcIndex = 0; uSrcIndex < m_nBmpItems; uSrcIndex ++ )
		{
			if( m_pBmpItems[ uSrcIndex ] == ( UINT ) lOrigCmdID )
			{
				break;
			}
		}

		if( uSrcIndex == m_nBmpItems )
		{
			nFailed++;
			continue;			
		}

		// get the postion on the toolbar
		pNext = p + 1;
		p = strchr( pNext, ',' );

		if( p == NULL )
		{
			// invalid entry
			nFailed ++;
			continue;
		}

		*p = '\0';
		
		iPosition = atoi( pNext );

		if( iPosition < 0 )
		{
			nFailed++;
			continue;
		}

		iPosition -= nFailed;

		// get the command type
		pNext = p+1;

		p = strchr( pNext, ',' );

		if( p == NULL )
		{
			// invalid entry
			nFailed ++;
			continue;
		}

		*p = '\0';

		szCommandType = pNext;
		szCommandType.TrimLeft();
		szCommandType.TrimRight();

		// get the action type
		pNext = p+1;

		p = strchr( pNext, ',' );

		if( p == NULL )
		{
			// invalid entry
			nFailed ++;
			continue;
		}

		*p = '\0';

		iAction = atoi( pNext );

		if( iAction <= 0 )
		{
			nFailed++;
			continue;
		}

		
		// get the name

		// TO DO - this may need to be in quotes
		szCommandName = p+1;
		szCommandName.TrimLeft();
		szCommandName.TrimRight();
		

		// find the command
		pCommand = NULL;		
			
		if( szCommandType.CompareNoCase( "Mailbox" ) == 0 )
		{
			// it's a mailbox command
			pCommand = ( QCCommandObject* ) g_theMailboxDirector.FindByPathname( szCommandName );
		}
		else if( szCommandType.CompareNoCase( "Recipient" ) == 0 )
		{
			// it's a recipient command
			pCommand = ( QCCommandObject* ) g_theRecipientDirector.Find( szCommandName );
		}
		else if( szCommandType.CompareNoCase( "Signature" ) == 0 )
		{
			// it's a signature command
			pCommand = ( QCCommandObject* ) g_theSignatureDirector.FindByPathname( szCommandName );
		}
		else if( szCommandType.CompareNoCase( "Stationery" ) == 0 )
		{
			// it's a stationery command
			pCommand =  ( QCCommandObject* ) g_theStationeryDirector.FindByPathname( szCommandName );
		}
		else if( szCommandType.CompareNoCase( "AttachmentPlugin" ) == 0 )
		{
			// it's an attachment plugin command
			pCommand = ( QCCommandObject* )g_thePluginDirector.Find( szCommandName, CA_ATTACH_PLUGIN );
		}
		else if( szCommandType.CompareNoCase( "SpecialPlugin" ) == 0 )
		{
			// it's an special plugin command
			pCommand = ( QCCommandObject* )g_thePluginDirector.Find( szCommandName, CA_SPECIAL_PLUGIN );
		}
		else if( szCommandType.CompareNoCase( "TranslatePlugin" ) == 0 )
		{
			// it's a translate plugin command
			pCommand = ( QCCommandObject* )g_thePluginDirector.Find( szCommandName, CA_TRANSLATE_PLUGIN );
		}
		else if( szCommandType.CompareNoCase( "TransmissionPlugin" ) == 0 )
		{
			// it's an transmission plugin command
			pCommand = ( QCCommandObject* )g_thePluginDirector.Find( szCommandName, CA_TRANSMISSION_PLUGIN );
		}
		else if( szCommandType.CompareNoCase( "Personality" ) == 0 )
		{
			// it's a personality command
			pCommand = ( QCCommandObject* ) g_thePersonalityDirector.Find( szCommandName );
		}

		if( pCommand == NULL )
		{
			nFailed++;
			continue;
		}

		uCommandID = g_theCommandStack.FindCommandID( pCommand, ( COMMAND_ACTION_TYPE ) iAction );

		if( uCommandID == 0 )
		{
			nFailed++;
			continue;
		}

		// copy the image
		( ( QCToolBarManager* ) m_pManager )->CopyButtonImage( uSrcIndex, uCommandID );
		
		AddButton( iPosition, uCommandID );
		m_btns[ iPosition ]->m_ulData = lOrigCmdID;
	}

	if (GetIniShort(IDS_INI_ALLOW_IN_YOUR_FACE))
		LoadInYourFacePlugins();
}

void QCCustomToolBar::LoadInYourFacePlugins()
{
	LONG				lType;
	char				modStr[10];
	CString				szSeenPlugin	= GetIniString(IDS_INI_SEEN_PLUGINS);
	CString				szNewPlugins;	
	QCPluginCommand*	pCommand		= NULL;
	int					startPos		= GetBtnCount();
	UINT				uSrcIndex;
	long				lastModID		= 0;

	POSITION pos = g_thePluginDirector.GetFirstPluginPosition();
	while( pos != NULL )
	{
		pCommand = g_thePluginDirector.GetNextPlugin( pos );

		switch(pCommand->GetType())
		{
			case CA_TRANSLATE_PLUGIN:
				lType = ID_TRANSLATOR;
				break;
			case CA_ATTACH_PLUGIN:
				lType = ID_MESSAGE_ATTACH_PLUGIN;
				break;
			case CA_SPECIAL_PLUGIN:
				lType = ID_SPECIAL_PLUGIN;
				break;
			default:
				continue;
		}
	
		if (pCommand->InYourFace())
		{
			long modID = 0, transID = 0;
			// see if it's already been seen
			pCommand->GetTransmissionID(&modID, &transID);
			sprintf(modStr,"x%dx",modID);

			if ( szSeenPlugin.Find(modStr) < 0)
			{
				// okay, add it in
				UINT uCommandID = g_theCommandStack.FindCommandID( pCommand, pCommand->GetType() );
	
				for( uSrcIndex = 0; uSrcIndex < m_nBmpItems; uSrcIndex ++ )
				{
					if( m_pBmpItems[ uSrcIndex ] == ( UINT ) lType )
					{
						break;
					}
				}

				if( uSrcIndex == m_nBmpItems )
				{
					continue;			
				}

				if( uCommandID == 0 )
				{
					continue;
				}
				
				// ASSERTing here means the button is already on the toolbar, but it's not in the
				//  INI entry (IDS_INI_SEEN_PLUGINS).
				ASSERT(CommandToIndex(uCommandID) == (-1));
				
				// Double-check that no button has this command ID
				// It is possible that OT has saved and restored the button, so we want to avoid
				//  placing a duplicate on the toolbar.

				if (CommandToIndex(uCommandID) == (-1))
				{
					//Add separators between plugin groups
					if (modID != lastModID)
						AddButton( startPos++, 0, TRUE, TRUE );

					( ( QCToolBarManager* ) m_pManager )->CopyButtonImage( uSrcIndex, uCommandID );
					AddButton( startPos, uCommandID );
					m_btns[ startPos++ ]->m_ulData = lType;
				}

				if ( szNewPlugins.Find(modStr) < 0)
				{
					if (!szNewPlugins.IsEmpty())
						szNewPlugins += ',';
					szNewPlugins += modStr;
				}
				lastModID = modID;
			}
		}
	}
	if (!szNewPlugins.IsEmpty())
	{
		if(!szSeenPlugin.IsEmpty())
			szSeenPlugin += ',';
		szSeenPlugin += szNewPlugins;
		SetIniString( IDS_INI_SEEN_PLUGINS, szSeenPlugin);
	}
}

void QCCustomToolBar::Notify(
QCCommandObject*	pObject,
COMMAND_ACTION_TYPE	theAction,
void*				pData)
{
	QCCommandObject*	pCurrentObject;
	COMMAND_ACTION_TYPE	anAction;
	INT					iCurrentButton;
	WORD				wCommandID;

	if( theAction != CA_DELETE ) 
	{
		return;
	}

// warning :
// the command has already been removed from the command stack at this point
// so the best we can do here is delete all the commands that we can't find 
// anymore.

	for( iCurrentButton = GetBtnCount() - 1; iCurrentButton >= 0; iCurrentButton-- )
	{
		if( ( m_btns[ iCurrentButton ]->m_ulData != 0 ) &&
			( m_btns[ iCurrentButton ]->m_nID != m_btns[ iCurrentButton ]->m_ulData ) )
		{
			wCommandID = ( WORD ) ( m_btns[ iCurrentButton ]->m_nID );

			if( ( wCommandID >= QC_FIRST_COMMAND_ID ) && ( wCommandID <= QC_LAST_COMMAND_ID ) )
			{
				if( g_theCommandStack.Lookup( wCommandID, &pCurrentObject, &anAction ) == FALSE )
				{
					// delete the button bitmap -- which will also delete the button
					( ( QCToolBarManager* ) m_pManager )->DeleteBitmapAt( m_btns[ iCurrentButton ]->m_nImage );
					RemoveButton( iCurrentButton );
				}
			}
		}
	}
}



BOOL QCCustomToolBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	QCCommandObject*	pObject;
	COMMAND_ACTION_TYPE	theAction;
	INT					iCurrentButton;
	WORD				wCommandID;
	CString				szToolTip;

	ASSERT(pResult != NULL);
	NMHDR* pNMHDR = (NMHDR*) lParam;

	switch (pNMHDR->code)
	{
	case TTN_NEEDTEXTA:
	case TTN_NEEDTEXTW:
		{
			//
			// The toolbar button command id is stuffed into the 'idFrom' field.
			//
			wCommandID = ( WORD ) pNMHDR->idFrom;
			ASSERT( wCommandID != ID_SEPARATOR );
			for( iCurrentButton = GetBtnCount() - 1; iCurrentButton >= 0; iCurrentButton-- )
			{
				if((m_btns[ iCurrentButton ]->m_nStyle & TBBS_SEPARATOR) == 0 &&
					m_btns[ iCurrentButton ]->m_nID == wCommandID )
				{
					break;
				}
			}

			if( iCurrentButton < 0 )
			{
				break;
			}

			SECStdBtn*	pButton = m_btns[ iCurrentButton ];
			if( g_theCommandStack.Lookup( wCommandID, &pObject, &theAction ) )
			{
				if( theAction == CA_ATTACH_PLUGIN ||
					theAction == CA_SPECIAL_PLUGIN ||
					theAction == CA_TRANSLATE_PLUGIN ||
					( m_btns[ iCurrentButton ]->m_ulData != 0 ) &&
					( m_btns[ iCurrentButton ]->m_nID != m_btns[ iCurrentButton ]->m_ulData ) )
				{
					szToolTip = pObject->GetToolTip( theAction );				
					szToolTip = szToolTip.Left(79);		// truncate at 79 chars, if necessary

					if (TTN_NEEDTEXTW == pNMHDR->code)
					{
						TOOLTIPTEXTW *pTTT = (TOOLTIPTEXTW *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));

						USES_CONVERSION;
						wcscpy(pTTT->szText, A2W( szToolTip ) );
					}
					else
					{
						TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *) pNMHDR;
						ASSERT(0 == (pTTT->uFlags & TTF_IDISHWND));
						strcpy(pTTT->szText, szToolTip);
					}
					return TRUE;
				}
			}
		}
		break;
		default:
		break;
	}
	
	return SECCustomToolBar::OnNotify( wParam, lParam, pResult );
}



BOOL QCCustomToolBar::ConvertOldStuff()
{
	CRString			szSection(IDS_TOOLBAR_MAIN);
	CRString			szKeyFmt(IDS_TOOLBAR_ITEM_FMT);
	CString				szKey;
	CString				szValue;
	char				buf[ 1024 ];
	char*				p;
	char*				pNext;
	INT					i;
	INT					idx;
	CWinApp*			pApp;
	INT					iPosition;
	LONG				lID;
	LONG				lType;
	CString				szName;
	CString				szMailbox;
	COMMAND_ACTION_TYPE	theAction;
	QCCommandObject*	pCommand;
	WORD				wCommand;
	UINT					iSrcIndex;


	//
	// Walk the command id items in the INI szSection, until we run out
	// of command id items.
	//
	pApp = ::AfxGetApp();
	idx = 0;
	iPosition = 0;

	while( 1 )
	{
		szKey.Format( szKeyFmt, idx );
		
		szValue = pApp->GetProfileString( szSection, szKey );
		
		if ( szValue == "" )
		{
			break;
		}
		
		idx ++;
		strcpy( buf, szValue );

		//
		// The string should either be of the form:
		//
		//     12345
		//
		// where '12345' is a "hardcoded" command id, or of the form:
		//
		//     12345,54321,Menu Item String,"c:\Mail Box\Pathname\String"
		//
		// where '12345' is a "dynamic" command id assigned to a
		// user-defined menu command, '54321' is the "type" of a
		// user-defined menu command, 'Menu Item String' is the
		// menu item string which appears on the menu, and 
		// 'c:\Mail Box\Pathname\String' is the mailbox pathname (only
		// used for Mailbox and Transfer menu items).
		//
		

		p = strchr( buf, ',' );

		if( p != NULL )
		{
			*p = '\0';
		}
		
		lID = atol( buf );

		if( p == NULL )
		{
			// map to the new ids

			for( i = DIM( theMap ) - 1; i >= 0; i-- )
			{
				if( theMap[i][0] == lID )
				{
					lID = theMap[i][1];
					break;
				}
			}
			
			if( lID != 0 )
			{
				AddButton( iPosition++, lID, ( lID == 0 ) );			
			}
			continue;
		}

		pNext = strchr( p + 1, ',' );
			
		if( pNext == NULL )
		{
			ASSERT( 0 );
			continue;
		}
		
		*pNext = '\0';
		
		lType = atol( p + 1 );

		if( lType == 0 )
		{
			ASSERT( 0 );
			continue;
		}

		p = pNext + 1;
		
		pNext = strchr( p, ',' );
		
		
		if( pNext != NULL )
		{
			*pNext = '\0';
		}
		
		szName = p;
		
		if( pNext != NULL )
		{
			szMailbox = pNext + 1;
			if( ( szMailbox.GetLength() < 3 ) ||
				( szMailbox[0] != '"' ) || 
				( szMailbox[ szMailbox.GetLength() - 1 ] != '"' ) )
			{
				continue;
			}

			szMailbox = szMailbox.Mid( 1, szMailbox.GetLength() - 2 );
		}
		else
		{
			szMailbox = "";
		}

		pCommand = NULL;

		switch ( lType )
		{
			case ID_MAILBOX_IN:
			case ID_MAILBOX_OUT:
			case ID_MAILBOX_TRASH:
			case ID_MAILBOX_USER:
				theAction = CA_OPEN;
				pCommand = ( QCCommandObject* ) g_theMailboxDirector.FindByPathname( szMailbox );
			break;

			case ID_TRANSFER_IN:
				theAction = CA_TRANSFER_TO;
				pCommand =  ( QCCommandObject* ) g_theMailboxDirector.FindByPathname( szMailbox );				
			break;
			
			case ID_TRANSFER_OUT:
				theAction = CA_TRANSFER_TO;
				pCommand =  ( QCCommandObject* ) g_theMailboxDirector.FindByPathname( szMailbox );				
			break;
			
			case ID_TRANSFER_TRASH:
				theAction = CA_TRANSFER_TO;
				pCommand =  ( QCCommandObject* ) g_theMailboxDirector.FindByPathname( szMailbox );				
			break;
			
			case ID_TRANSFER_USER:
				theAction = CA_TRANSFER_TO;
				pCommand =  ( QCCommandObject* ) g_theMailboxDirector.FindByPathname( szMailbox );				
			break;
			
			case ID_RECIPIENT_INSERT:
				theAction = CA_INSERT_RECIPIENT;
				pCommand =  ( QCCommandObject* ) g_theRecipientDirector.Find( szName );				
			break;
			
			case ID_RECIPIENT_NEW_MESSAGE:
				theAction = CA_NEW_MESSAGE_TO;
				pCommand =  ( QCCommandObject* ) g_theRecipientDirector.Find( szName );				
			break;

			case ID_RECIPIENT_FORWARD:
				theAction = CA_FORWARD_TO;
				pCommand =  ( QCCommandObject* ) g_theRecipientDirector.Find( szName );				
			break;

			case ID_RECIPIENT_REDIRECT:
				theAction = CA_REDIRECT_TO;
				pCommand =  ( QCCommandObject* ) g_theRecipientDirector.Find( szName );				
			break;

			case ID_MESSAGE_NEWMESSAGE_WITH:
				theAction = CA_NEW_MESSAGE_WITH;
				pCommand =  ( QCCommandObject* ) g_theStationeryDirector.FindByPathname( szName );				
			break;

			case ID_MESSAGE_REPLY_WITH:
				theAction = CA_REPLY_WITH;
				pCommand =  ( QCCommandObject* ) g_theStationeryDirector.FindByPathname( szName );				
			break;

			case ID_TRANSLATOR:
				theAction = CA_TRANSLATE_PLUGIN;
				pCommand =  ( QCCommandObject* ) g_thePluginDirector.Find( szName, CA_TRANSLATE_PLUGIN );				
			break;

			case ID_MESSAGE_ATTACH_PLUGIN:
				theAction = CA_ATTACH_PLUGIN;
				pCommand =  ( QCCommandObject* ) g_thePluginDirector.Find( szName, CA_ATTACH_PLUGIN );				
			break;

			case ID_SPECIAL_PLUGIN:
				theAction = CA_SPECIAL_PLUGIN;
				pCommand =  ( QCCommandObject* ) g_thePluginDirector.Find( szName, CA_SPECIAL_PLUGIN );				
			break;
		}

		if( pCommand == NULL )
		{
			continue;
		}

		wCommand = g_theCommandStack.FindCommandID( pCommand, theAction );

		if( !wCommand )
		{
			continue;
		}
		
		for( iSrcIndex = 0; iSrcIndex < m_nBmpItems; iSrcIndex ++ )
		{
			if( m_pBmpItems[ iSrcIndex ] == ( UINT ) lType )
			{
				break;
			}
		}

		if( iSrcIndex == m_nBmpItems )
		{
			continue;
		}

		( ( QCToolBarManager* ) m_pManager )->CopyButtonImage( iSrcIndex, wCommand );
		
		AddButton( iPosition, wCommand );
		m_btns[ iPosition++ ]->m_ulData = m_pBmpItems[ iSrcIndex ];
	}

	if( idx > 0 )
	{
		while( idx >= 0 )
		{
			szKey.Format( szKeyFmt, idx-- );
			pApp->WriteProfileString( szSection, szKey, NULL );
		}

		if( iPosition > 0 )
		{
			while( iPosition < GetBtnCount() )
			{
				RemoveButton( GetBtnCount() - 1 );
			}

			return TRUE;
		}				
	}

	return FALSE;
}

BOOL QCCustomToolBar::CreateEx(DWORD dwExStyle, CWnd* pParentWnd, 
								DWORD dwStyle, UINT nID, LPCTSTR lpszTitle)
{
	
	m_cyTopBorder    = 1;
	m_cyBottomBorder = 1;
	return SECCustomToolBar::CreateEx(dwExStyle, pParentWnd, dwStyle, nID, lpszTitle );
}


void QCCustomToolBar::ResetImageOffsets()
{
	INT i;

	for ( i = m_btns.GetSize() - 1; i >= 0; i-- )
	{
		if( ( m_btns[ i ]->m_nStyle & TBBS_SEPARATOR ) == 0 )
		{
			m_btns[ i ]->m_nImage = IDToBmpIndex( m_btns[ i ]->m_nID );
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CreateSeparator [protected, virutal]
//
// Need to override this base-class function to correctly initialize
// the ID field of separators.
//////////////////////////////////////////////////////////////////////
SECStdBtn* QCCustomToolBar::CreateSeparator(SECCustomToolBar* pToolBar)
{
	SECStdBtn* pBtn = SECCustomToolBar::CreateSeparator(pToolBar);

	if (pBtn)
		pBtn->m_nID = ID_SEPARATOR;

	return pBtn;
}

LRESULT QCCustomToolBar::OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam)
{            
	return m_Palette.DoQueryNewPalette((CWnd*)this);
}

void QCCustomToolBar::OnPaletteChanged(CWnd* pFocusWnd)
{
	m_Palette.DoPaletteChanged((CWnd*)this, pFocusWnd);
}

void QCCustomToolBar::OnSysColorChange()
{
	m_Palette.DoSysColorChange((CWnd*)this);
}



void QCCustomToolBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CFrameWnd*			pFrameWnd;
	CRString			szMainToolBar( IDS_TOOLBAR_TITLE );
	BOOL				bIsVisible;
	CString				szMenuName;
	CString				szName;
	QCCustomToolBar*	pToolBar;
	POSITION			pos;
	BOOL				bFirst;
	UINT				uID;


	if( ( ( QCToolBarManager* ) m_pManager )->InConfigMode() )
	{
		return;
	}

  	// Get the menu that contains all the CMboxTree context popups
	CMenu popup_menus;
	HMENU hMenu = QCLoadMenu(IDR_CTRLBAR_POPUPS);
	if ( ! hMenu || ! popup_menus.Attach( hMenu ) )
		return;
	CMenu* p_temp_popup_menu = NULL;
	VERIFY((p_temp_popup_menu = popup_menus.GetSubMenu(0)) != NULL);

	if (p_temp_popup_menu != NULL)
	{
		bFirst = TRUE;
		pFrameWnd = ( ( QCToolBarManager* ) m_pManager )->GetFrameWnd();
		
		if( pFrameWnd != NULL )
		{
			CMainFrame *pTheFrame = CMainFrame::QCGetMainFrame();
			ASSERT(pTheFrame);

			if (pTheFrame)
			{
				// Set the correct check state for the TOOLBAR
				UINT nCheck = MF_BYCOMMAND;

				if (pTheFrame->IsToolbarVisible())
					nCheck |= MF_CHECKED;
				else
					nCheck |= MF_UNCHECKED;

				VERIFY(p_temp_popup_menu->CheckMenuItem(ID_CTRLBAR_SHOW_TOOLBAR, nCheck) != (-1));

				// Set the correct check state for the STATUS BAR
				nCheck = MF_BYCOMMAND;

				if (pTheFrame->IsStatbarVisible())
					nCheck |= MF_CHECKED;
				else
					nCheck |= MF_UNCHECKED;

				VERIFY(p_temp_popup_menu->CheckMenuItem(ID_CTRLBAR_SHOW_STATUSBAR, nCheck) != (-1));

				pos = pFrameWnd->m_listControlBars.GetHeadPosition();
			}

			// Shareware: Disable customizable toolbar menu item for reduced feature set
			if (!UsingFullFeatureSet())
			{
				// Reduced feature, so gray out the customize menu item
				p_temp_popup_menu->EnableMenuItem(ID_CUSTOMIZE_LINK, MF_BYCOMMAND | MF_GRAYED);
			}


			// we don't really process the ids, so we'll reused the range for the
			// command stack
			
			uID = QC_FIRST_COMMAND_ID;

			// add the user defined toolbars to the menu

			while( pos != NULL )
			{
				pToolBar = (QCCustomToolBar*) ( pFrameWnd->m_listControlBars.GetNext(pos) );
				pToolBar->GetWindowText( szName );
				
				if( pToolBar->IsKindOf( RUNTIME_CLASS( QCCustomToolBar ) ) && 
					szName.CompareNoCase( szMainToolBar ) )
				{
					if( bFirst )
					{
						// add in the separator
						p_temp_popup_menu->AppendMenu( MF_SEPARATOR, 0, ( LPCSTR ) 0 );
						bFirst = FALSE;
					}
				
					if( pToolBar->IsVisible() )
					{
						p_temp_popup_menu->AppendMenu( MF_STRING | MF_CHECKED, uID++, szName );
					}
					else
					{
						p_temp_popup_menu->AppendMenu( MF_STRING, uID++, szName );
					}
				}
			}

			uID = p_temp_popup_menu->TrackPopupMenu( TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, pWnd );

			if( ( uID < QC_FIRST_COMMAND_ID ) || ( uID > QC_LAST_COMMAND_ID ) )
			{
				pFrameWnd->PostMessage( WM_COMMAND, uID, 0 );
			}
			else
			{
				// get the name of the toolbar
				p_temp_popup_menu->GetMenuString( uID, szMenuName, MF_BYCOMMAND );
				
				// get the toolbar
				pos = pFrameWnd->m_listControlBars.GetHeadPosition();
				
				while( pos != NULL )
				{
					pToolBar = (QCCustomToolBar*) ( pFrameWnd->m_listControlBars.GetNext(pos) );
					pToolBar->GetWindowText( szName );
					
					if( pToolBar->IsKindOf( RUNTIME_CLASS( QCCustomToolBar ) ) && 
						szName.CompareNoCase( szMenuName ) == 0 )
					{
						break;
					}
					
					pToolBar = NULL;
				}

				if( pToolBar )
				{
					// toggle the visibilty
					bIsVisible = pToolBar->IsVisible();
					pFrameWnd->ShowControlBar( pToolBar, !bIsVisible, FALSE );
					SetIniShort( IDS_INI_SHOW_TOOLBAR, !bIsVisible );
					pFrameWnd->RecalcLayout();
				}
			}
		}
	}

	popup_menus.DestroyMenu();
	return;
}
