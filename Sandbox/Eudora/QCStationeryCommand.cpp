// QCStationeryCommand.cpp: implementation of the QCStationeryCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "STDAFX.H"
#include <QCUtils.h>

#include "guiutils.h"
#include "QCCommandActions.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"
#include "rs.h"

#include "compmsgd.h"
#include "summary.h"
#include "eudora.h"
#include "WorkBook.h"
#include "TocDoc.h"

// these will all go away
#include <afxrich.h>
#include "resource.h" // for ID_MESSAGE_REPLY
#include "msgdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

extern CString			EudoraDir;

IMPLEMENT_DYNAMIC(QCStationeryCommand, QCCommandObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCStationeryCommand::QCStationeryCommand(
QCStationeryDirector*	pDirector,
LPCSTR					szName,
LPCSTR					szPathname ) : QCCommandObject( pDirector )
{
	m_szName = szName;
	m_szPathname = szPathname;
}

QCStationeryCommand::~QCStationeryCommand()
{

}


///////////////////////////////////////////////////////////////////////////////
//
//
void	QCStationeryCommand::Execute(
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{

	POSITION			pos;
	CCompMessageDoc*	pCompMessage;
	CView*				pView;
	QCWorksheet*		pFrame;
	CSummary*			pSummary;
	CString				szNewPath;
	CString				szNewName;
		
	if( theAction == CA_RENAME )
	{
		szNewName = ( LPCSTR ) pData;
		szNewPath = EudoraDir + CRString( IDS_STATIONERY_FOLDER ) + "\\"; 
		szNewPath += szNewName + "." + CRString( IDS_STATIONERY_EXTENSION );

		try
		{
			// attempt the reneme
			CFile::Rename( m_szPathname, szNewPath );
		}
		catch( CFileException* pException )
		{		
			switch( pException->m_cause )
			{
				case CFileException::badPath:
					ErrorDialog( IDS_ERR_BAD_STATIONERY_NAME, szNewName );
				break;
				
				case CFileException::accessDenied:
					ErrorDialog( IDS_ERR_STATIONERY_EXISTS, szNewName );
				break;
				
				default:
					ErrorDialog( IDS_ERR_RENAME_STATIONERY );
				break;

			}			
			return;
		}
	}
	else if (CA_DELETE == theAction)
	{
		//
		// Warn the user that we're about to delete a stationery
		// file and that this cannot be undone.
		//
		CString msg;
		msg.Format(CRString(IDS_STATIONERY_PERMANENT_DELETE), (const char *) m_szName);
		if (IDYES != ::AfxMessageBox(msg, MB_ICONQUESTION | MB_YESNO))
			return;
	}

	NotifyDirector( theAction, pData );	

	switch( theAction )
	{
		case CA_NEW_MESSAGE_WITH:

			pCompMessage = NewCompDocumentWith( m_szPathname );
		
			if( pCompMessage != NULL )
			{
				pCompMessage->m_Sum->Display();
			}
		break;
		
		case CA_REPLY_WITH:
			pSummary = ( CSummary* ) pData;
			pSummary->ComposeMessage( ID_MESSAGE_REPLY, NULL, FALSE, m_szPathname );
		break;
		
		case CA_REPLY_TO_ALL_WITH:
			pSummary = ( CSummary* ) pData;
			pSummary->ComposeMessage( ID_MESSAGE_REPLY_ALL, NULL, FALSE, m_szPathname );
		break;
		
		case CA_AUTO_REPLY_WITH:
			pSummary = ( CSummary* ) pData;
			pSummary->ComposeMessage( ID_MESSAGE_REPLY, NULL, TRUE, m_szPathname );
		break;

		case CA_RENAME:
			m_szName = szNewName;
			m_szPathname = szNewPath;
		break;

		case CA_DELETE:
			//TODO If it is open, delete the window and the summary entry
			// see if it's open
	
			for(	pos = CompMessageTemplate->GetFirstDocPosition(), pCompMessage = NULL;
					pos != NULL; )
			{
				pCompMessage = ( CCompMessageDoc* ) CompMessageTemplate->GetNextDoc( pos );

				CString currentDocTitle;

				if( m_szPathname.CompareNoCase( pCompMessage->GetPathName() ) == 0 )
				{
					CTocDoc* OutToc;

					pCompMessage->OnCloseDocument();

					if (!pCompMessage->m_Sum || !(OutToc = GetOutToc()))
					break;

					OutToc->RemoveSum(pCompMessage->m_Sum);
					break;
				}
			}

			if(FAILED(::FileRemoveMT( m_szPathname )))
			{
				ErrorDialog( IDS_ERR_FILE_DELETE, (const char*) m_szPathname );
			}	
		break;		

		case CA_OPEN:

			// see if it's already open
	
			for(	pos = CompMessageTemplate->GetFirstDocPosition(), pCompMessage = NULL;
					pos != NULL; )
			{
				pCompMessage = ( CCompMessageDoc* ) CompMessageTemplate->GetNextDoc( pos );
		
				//Since a composition msg and the stationery are both CompDocs,
				//check the stationery flag to see if the stationery is actually
				//open or merely being used by another composition msg.
				if (( m_szPathname.CompareNoCase( pCompMessage->GetPathName() ) == 0 ) && 
					( pCompMessage->IsStationery() ))
				{
					pos = pCompMessage->GetFirstViewPosition();

					if( pos )
					{
						pView = pCompMessage->GetNextView(pos);

						if( pView )
						{
							pFrame = ( QCWorksheet* ) pView->GetParentFrame();
							ASSERT_KINDOF( QCWorksheet, pFrame );

							if( pFrame->IsIconic() )
							{
								pFrame->MDIRestore();
							}
							else
							{
								pFrame->MDIActivate();
							}
						}
					}

					return;
				}
			}

			// open a new one
			//CompMessageTemplate->OpenDocumentFile( m_szPathname, TRUE );

			CCompMessageDoc *pCompMsg = NewCompDocumentWith(m_szPathname);

			if (pCompMsg)
			{
				pCompMsg->SetIsStationery();
				//The comp doc knows where to get the title from and what format to set it in,
				//But pass the m_szPathname to be safe
				pCompMsg->SetTitle(m_szPathname);
				pCompMsg->m_Sum->Display();
			}

			break;
	}

}


UINT QCStationeryCommand::GetFlyByID(
COMMAND_ACTION_TYPE theAction )
{
	switch( theAction )
	{
		case CA_NEW_MESSAGE_WITH:
			return ID_MESSAGE_NEWMESSAGE;
		break;
		case CA_REPLY_WITH:
			return ID_MESSAGE_REPLY_WITH;
		break;
		case CA_REPLY_TO_ALL_WITH:
			return ID_MESSAGE_REPLY_WITH;
		break;
	}

	return 0;
}


CString	QCStationeryCommand::GetToolTip(
COMMAND_ACTION_TYPE theAction)
{
	CString szToolTip( "" );

	switch( theAction )
	{
		case CA_NEW_MESSAGE_WITH:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_NEWMESSAGE_WITH );
			szToolTip += " ";
			szToolTip += m_szName;
		break;
		case CA_REPLY_WITH:
		case CA_REPLY_TO_ALL_WITH:
			szToolTip.LoadString( IDS_USERDEF_TOOLTIP_REPLY_WITH );
			szToolTip += " ";
			szToolTip += m_szName;
		break;
		case CA_OPEN:
			szToolTip = m_szName;
		break;
	}

	return szToolTip;
}
