// QCSignatureCommand.cpp: implementation of the QCSignatureCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "eudora.h"
#include "guiutils.h"
#include "rs.h"

#include "QCCommandActions.h"
#include "QCSignatureDirector.h"
#include "QCSignatureCommand.h"

#include "Signatur.h"
#include "WorkBook.h"

#include "DebugNewHelpers.h"

extern CString			EudoraDir;

IMPLEMENT_DYNAMIC(QCSignatureCommand, QCCommandObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCSignatureCommand::QCSignatureCommand(
QCSignatureDirector*	pDirector,
LPCSTR					szName,
LPCSTR					szPathname ) : QCCommandObject( pDirector )
{
	m_szName = szName;
	m_szPathname = szPathname;
}

QCSignatureCommand::~QCSignatureCommand()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
void	QCSignatureCommand::Execute(
COMMAND_ACTION_TYPE	theAction,
void*				pData )
{
	POSITION		pos;
	CSignatureDoc*	pDoc;
	CView*			pView;
	QCWorksheet*	pFrame;
	CString			szNewPathname;

	if( theAction == CA_RENAME )
	{
		// see if it's open

		for(	pos = SignatureTemplate->GetFirstDocPosition(), pDoc = NULL;
				pos != NULL; )
		{
			pDoc = ( CSignatureDoc* ) SignatureTemplate->GetNextDoc( pos );

			if( m_szName.CompareNoCase( pDoc->GetTitle() ) == 0 )
			{
				pDoc->OnCloseDocument();
				break;
			}
		}

		szNewPathname = EudoraDir + CRString( IDS_SIGNATURE_FOLDER ) + '\\'; 
		szNewPathname += ( LPCSTR ) pData;
		szNewPathname += '.';
		szNewPathname += CRString( IDS_SIGNATURE_EXTENSION );
		
		try
		{
			CFile::Rename( m_szPathname, szNewPathname );
		}
		catch( CFileException* pExp )
		{
			ErrorDialog( IDS_UNABLE_TO_RENAME_FILE );
			pExp->Delete();
			return;
		}

	}
	else if (CA_DELETE == theAction)
	{
			//
			// Warn the user that we're about to delete a signature
			// file and that this cannot be undone.
			//
			CString msg;
			msg.Format(CRString(IDS_SIGNATURE_PERMANENT_DELETE), (const char *) m_szName);
			if (IDYES != ::AfxMessageBox(msg, MB_ICONQUESTION | MB_YESNO))
				return;
	}

	NotifyDirector( theAction, pData );	

	switch( theAction )
	{
		case CA_DELETE:
		
			// see if it's open
	
			for(	pos = SignatureTemplate->GetFirstDocPosition(), pDoc = NULL;
					pos != NULL; )
			{
				pDoc = ( CSignatureDoc* ) SignatureTemplate->GetNextDoc( pos );

				if( m_szName.CompareNoCase( pDoc->GetTitle() ) == 0 )
				{
					pDoc->OnCloseDocument();
					break;
				}
			}

			CFile::Remove( m_szPathname );
			
			delete this;

		break;

		case CA_RENAME:
		
			m_szPathname = szNewPathname;
			m_szName = ( LPCSTR ) pData;

		break;
					
		case CA_OPEN:

			// see if it's already open
	
			for(	pos = SignatureTemplate->GetFirstDocPosition(), pDoc = NULL;
					pos != NULL; )
			{
				pDoc = ( CSignatureDoc* ) SignatureTemplate->GetNextDoc( pos );
		
				if( m_szName.CompareNoCase( pDoc->GetTitle() ) == 0 )
				{
					pos = pDoc->GetFirstViewPosition();

					if( pos )
					{
						pView = pDoc->GetNextView(pos);

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
			SignatureTemplate->OpenDocumentFile( m_szPathname, TRUE );

		break;
	}
}


CString	QCSignatureCommand::GetToolTip(
COMMAND_ACTION_TYPE theAction)
{
	CString szToolTip( "" );

	switch( theAction )
	{
		case CA_OPEN:
			szToolTip = m_szName;
		break;
	}
	
	return szToolTip;
}
