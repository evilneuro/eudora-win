// ReadMessageDoc.cpp : implementation of the CReadMessageDoc class
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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

#include "fileutil.h"
#include "MDIChild.h"
#include "ReadMessageFrame.h"
#include "ReadMessageDoc.h"
#include "msgutils.h"
#include "SaveAs.h"
#include "QCProtocol.h"
#include "tocdoc.h"
#include "rs.h"
#include "guiutils.h"
#include "utils.h"


#include "QCCommandActions.h"
#include "QCCommandStack.h"
#include "QCCommandObject.h"
#include "QCMailboxDirector.h"
#include "QCStationeryCommand.h"
#include "QCRecipientCommand.h"


#define DIM( a ) ( sizeof( a )/sizeof( a[0] ) )

//Static member -- storage allocation
CStringArray CReadMessageDoc::m_theTabooHeaderArray;

extern QCCommandStack		g_theCommandStack;
extern QCMailboxDirector	g_theMailboxDirector;

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CReadMessageDoc

IMPLEMENT_DYNCREATE(CReadMessageDoc, CMessageDoc)

BEGIN_MESSAGE_MAP(CReadMessageDoc, CMessageDoc)
	//{{AFX_MSG_MAP(CReadMessageDoc)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP

	ON_COMMAND_EX_RANGE(QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnDynamicCommand)
	ON_UPDATE_COMMAND_UI_RANGE(QC_FIRST_COMMAND_ID, QC_LAST_COMMAND_ID, OnUpdateDynamicCommand)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_UNREAD, OnUpdateStatusUnread)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_READ, OnUpdateStatusRead)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REPLIED, OnUpdateStatusReplied)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_FORWARDED, OnUpdateStatusForwarded)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_REDIRECTED, OnUpdateStatusRedirected)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_RECOVERED, OnUpdateStatusRecovered)
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_STATUS_TOGGLE, OnUpdateStatusToggle)

	ON_COMMAND_EX(ID_MESSAGE_STATUS_UNREAD, OnMessageStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_READ, OnMessageStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_REPLIED, OnMessageStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_FORWARDED, OnMessageStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_REDIRECTED, OnMessageStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_RECOVERED, OnMessageStatus)
	ON_COMMAND_EX(ID_MESSAGE_STATUS_TOGGLE, OnMessageStatus)

	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_LEAVE, OnUpdateMessageServer )
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_FETCH, OnUpdateMessageServer )
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_DELETE, OnUpdateMessageServer )
	ON_UPDATE_COMMAND_UI(ID_MESSAGE_SERVER_FETCH_DELETE, OnUpdateMessageServer )

	ON_COMMAND(ID_MESSAGE_SERVER_LEAVE, OnMessageServerLeave)
	ON_COMMAND(ID_MESSAGE_SERVER_FETCH, OnMessageServerFetch)
	ON_COMMAND(ID_MESSAGE_SERVER_DELETE, OnMessageServerDelete)
	ON_COMMAND(ID_MESSAGE_SERVER_FETCH_DELETE, OnMessageServerFetchDelete)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadMessageDoc construction/destruction

CReadMessageDoc::CReadMessageDoc()
{
	m_bInFind = FALSE;
	m_bDidFindOpenView = FALSE;
}

CReadMessageDoc::~CReadMessageDoc()
{
}


////////////////////////////////////////////////////////////////////////
// OnDynamicCommand [protected]
//
// Handles commands specific to a read message.  Generic stuff like
// the Transfer commands are handled in the base class CMessageDoc.
////////////////////////////////////////////////////////////////////////

BOOL CReadMessageDoc::OnDynamicCommand(UINT uID)
{
    QCCommandObject* pCommand = NULL;
    COMMAND_ACTION_TYPE theAction;

    if( !g_theCommandStack.GetCommand( (WORD) uID, &pCommand, &theAction ) )
        return FALSE;

    if( pCommand == NULL || !theAction )
        return FALSE;

    if( theAction == CA_REPLY_WITH ||
        theAction == CA_REPLY_TO_ALL_WITH ) {

        CString selectedText = "";
        UINT uCommand = (theAction == CA_REPLY_WITH) ? ID_MESSAGE_REPLY : ID_MESSAGE_REPLY_ALL;
        const char* pSelText = GetSelectedText( selectedText ) ? (const char*)selectedText : NULL;
        ComposeMessage( uCommand, NULL, ((QCStationeryCommand*)pCommand)->GetPathname(), pSelText );
        return TRUE;
    }
    else if ( theAction == CA_FORWARD_TO ||
              theAction == CA_REDIRECT_TO ) {

        CString selectedText = "";
        UINT uCommand = (theAction == CA_FORWARD_TO) ? ID_MESSAGE_FORWARD : ID_MESSAGE_REDIRECT;
        const char* pSelText = GetSelectedText( selectedText ) ? (const char*)selectedText : NULL;
        ComposeMessage( uCommand, ((QCRecipientCommand*)pCommand)->GetName(), NULL, pSelText );
        return TRUE;
    }
    else if( theAction == CA_CHANGE_PERSONA ) {
        VERIFY( m_Sum != NULL );
        pCommand->Execute( theAction, m_Sum );
        return TRUE;
    }

    return CMessageDoc::OnDynamicCommand( uID );
}


////////////////////////////////////////////////////////////////////////
// OnUpdateDynamicCommand [protected]
//
// Handles commands specific to a read message.  Generic stuff like
// the Transfer commands are handled in the base class CMessageDoc.
////////////////////////////////////////////////////////////////////////
void CReadMessageDoc::OnUpdateDynamicCommand(CCmdUI* pCmdUI)
{
	QCCommandObject*	pCommand;
	COMMAND_ACTION_TYPE	theAction;	

	if( pCmdUI->m_pSubMenu == NULL )
	{
		if( g_theCommandStack.Lookup( ( WORD ) ( pCmdUI->m_nID ), &pCommand, &theAction ) )
		{
			if( ( theAction == CA_REPLY_WITH ) ||
				( theAction == CA_REPLY_TO_ALL_WITH ) || 
				( theAction == CA_REDIRECT_TO ) ||
				( theAction == CA_FORWARD_TO ) ||
				( theAction == CA_CHANGE_PERSONA ) )
			{
				pCmdUI->Enable( TRUE );
				return;
			}
		}
	}

	CMessageDoc::OnUpdateDynamicCommand(pCmdUI);
}


void CReadMessageDoc::OnUpdateStatusUnread(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CReadMessageDoc::OnUpdateStatusRead(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CReadMessageDoc::OnUpdateStatusReplied(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CReadMessageDoc::OnUpdateStatusForwarded(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CReadMessageDoc::OnUpdateStatusRedirected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CReadMessageDoc::OnUpdateStatusRecovered(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void CReadMessageDoc::OnUpdateStatusToggle(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(MS_READ == m_Sum->m_State || MS_UNREAD == m_Sum->m_State);
}


BOOL CReadMessageDoc::OnMessageStatus(UINT StatusMenuID) 
{
	if (m_Sum)
	{
		int nNewStatus = MS_UNREAD;
		
		switch (StatusMenuID)
		{
		case ID_MESSAGE_STATUS_UNREAD:		nNewStatus = MS_UNREAD;		break;
		case ID_MESSAGE_STATUS_READ:		nNewStatus = MS_READ;		break;
		case ID_MESSAGE_STATUS_REPLIED:		nNewStatus = MS_REPLIED;	break;
		case ID_MESSAGE_STATUS_FORWARDED:	nNewStatus = MS_FORWARDED;	break;
		case ID_MESSAGE_STATUS_REDIRECTED:	nNewStatus = MS_REDIRECT;	break;
		case ID_MESSAGE_STATUS_RECOVERED:	nNewStatus = MS_RECOVERED;	break;
		case ID_MESSAGE_STATUS_TOGGLE:
			if (MS_READ == m_Sum->m_State)
				nNewStatus = MS_UNREAD;
			else if (MS_UNREAD == m_Sum->m_State)
				nNewStatus = MS_READ;
			else
			{
				::MessageBeep(MB_OK);
				return TRUE;
			}
			break;
		default: 
			ASSERT(0); 
			return FALSE;
		}

		m_Sum->SetState(char(nNewStatus));

#ifdef IMAP4
		// If an IMAP mailbox, need to set state on remote mailbox also.
		CTocDoc* toc = m_Sum->m_TheToc;

		if ( toc && toc->IsImapToc() )
		{
			toc->ImapSetMessageState ( m_Sum, (char) nNewStatus );
		}
#endif // IMAP4

	}

	return (TRUE);
}


CView* CReadMessageDoc::GetView()
{
	// read msg's have one pane
	POSITION pos = GetFirstViewPosition();
	if (pos) {
		CView* pView = (CView*) GetNextView( pos );
		VERIFY( pView );
		return pView;
	}

	return (NULL);
}

HRESULT CReadMessageDoc::Write()
{
	return (Write((JJFile*)NULL));
}

HRESULT CReadMessageDoc::Write(JJFile* mboxFile)
{
	BOOL CreatedJJFile = FALSE;
	CView* pView = GetView();
	QCProtocol*	pProtocol;

	pProtocol = QCProtocol::QueryProtocol( QCP_SAVE_INFO, pView );

	if( pProtocol == NULL )
	{
		return E_POINTER;
	}

	// Update the subject in the summary and on the frame's title bar
	{
		//
		// Need to do the dynamic downcast since this read message doc
		// is used in both the CTocFrame and in CReadMessageFrame.  The
		// following only applies to the CReadMessageFrame case.
		//
		CFrameWnd* pFrameWnd = pView->GetParentFrame();
		CReadMessageFrame* pReadMessageFrame = DYNAMIC_DOWNCAST(CReadMessageFrame, pFrameWnd);
		if (pReadMessageFrame)
		{
			CString szSubject;
			pReadMessageFrame->GetSubject( szSubject );
			m_Sum->SetSubject( szSubject );
			ReallySetTitle(m_Sum->MakeTitle());
		}
	}

	CTocDoc *curToc = m_Sum->m_TheToc;
	if (!curToc)
		return E_POINTER;

	// Save the flags & subject
	if (!mboxFile)
	{
		curToc->SetModifiedFlag();
	}

	CString		szRawMessage;

	if ( !IsModified() && WasTranslated() && m_QCMessage.IsInit() )
	{
		// The message wasn't modified, it was translated - so we don't want to call
		// SaveInfo, because that would blast over the newly translated information
		// with an HTML version of the document that's inferior to the message's
		// raw version (e.g. it would break attachment icons).
		// Since we checked that the message is initialized, we're good to go without
		// checking for any errors.
		m_QCMessage.GetRawMessage(szRawMessage);
	}
	else
	{
		// Grab the modified information the old way (it will be HTML, but that's
		// the best we can do in this case).
		HRESULT		hr = pProtocol->SaveInfo();
		if ( FAILED(hr) )
			return hr;
		szRawMessage = m_Text;
	}

	while (1)
	{
		if (!mboxFile)
		{
			mboxFile = DEBUG_NEW JJFile;
			CreatedJJFile = TRUE;
			if (FAILED(mboxFile->Open(curToc->GetMBFileName(), O_APPEND | O_RDWR)))
				break;
		}
	
		// Get the offset of the start of the message
		long lStart = 0;
		mboxFile->Tell(&lStart);
		ASSERT(lStart >= 0);
	
		// Write From line
		CTime Time(CTime::GetCurrentTime());
		if (Time.GetTime() < 0)
			Time = 0;
		CString FromString = ::FormatTimeMT( static_cast<time_t>(Time.GetTime()), GetIniString(IDS_FROM_CTIME_FORMAT) );
		if (FAILED(mboxFile->PutLine(FromString))) break;	

		if (szRawMessage.GetLength() == 0) break;
		
		const char *	Text = szRawMessage;
		do
		{
			long len = strcspn(Text, "\n");
			if (Text[len] == '\n')
				len++;
	
	        // Quote From lines
			if (IsFromLine(Text) && FAILED(mboxFile->Put('>'))) break;
	
			if (FAILED(mboxFile->Put(Text, len))) break;
			Text += len;
		} while (*Text);
		// Did something go wrong?
		if (*Text) break;


		if (Text[-1] != '\n' && FAILED(mboxFile->PutLine())) break;

		int iReturnCount = 2; //2 for \n\r of the "From ???..." line
		if (Text[-1] != '\n')
			iReturnCount += 2; //2 for \n\r of the end of themessage to align "from".
	
		if (CreatedJJFile)
		{
			mboxFile->Seek(lStart, SEEK_CUR);
			m_Sum->m_Length = szRawMessage.GetLength() + 
							FromString.GetLength() + iReturnCount;
			m_Sum->Build(mboxFile, TRUE);
			// Set the offset to the beginning of this message in the .mbx file.
			m_Sum->m_Offset = lStart;
			mboxFile->Close();
			curToc->m_NeedsCompact = TRUE;
			curToc->Write();
			SetModifiedFlag(FALSE);
			delete mboxFile;
		}

		return S_OK;
	}

	if (mboxFile && CreatedJJFile)
		delete mboxFile;

	return E_FAIL;
}

BOOL CReadMessageDoc::WriteAsText( 
JJFile*	pFile, 
BOOL ) 
{
	return FALSE;
}

void CReadMessageDoc::BuildTabooHeaderArray()
{
	CString		strTabooString;

	GetIniString(IDS_INI_USER_TABOO_HEADERS, strTabooString);
	
	char *		p;
	char *		pCurrent = strTabooString.GetBuffer( strTabooString.GetLength() );
	
	while( *pCurrent != '\0' ) 
	{
		p = strchr( pCurrent, ',' );
		
		if( p != NULL )
		{
			*p = '\0';
		}
		
		m_theTabooHeaderArray.Add( pCurrent );

		if( p != NULL )
		{
			pCurrent = p + 1;
		}
		else
		{
			break;
		}
	}
}

const CStringArray& CReadMessageDoc::GetTabooHeaderArray()
{ 
	if ( m_theTabooHeaderArray.GetSize() == 0)
		BuildTabooHeaderArray();

	return m_theTabooHeaderArray; 
}

void CReadMessageDoc::StripTabooHeaders( char* szMessage )
{
	char*	p;
	char*	t;
	BOOL	GetOut = FALSE;
	INT		i;

	//Make sure that the Taboo Header Array has been read up
	if ( m_theTabooHeaderArray.GetSize() == 0 )
	{
		BuildTabooHeaderArray();
	}

	p = szMessage;
	t = p;

	while (!GetOut && p)
	{
		char* Colon = p;
		while (!GetOut && *Colon != ':')
		{
			if (*Colon == '\0' || *Colon++ == '\n')
				GetOut = TRUE;
		}

		if (!GetOut)
		{
			// Find out if this is a header to skip
			int IncludeHeader = TRUE;
			
			for( i = 0; i < m_theTabooHeaderArray.GetSize(); i ++ )
			{
				if( !strnicmp( m_theTabooHeaderArray[i], p, m_theTabooHeaderArray[i].GetLength() ) )
				{
					IncludeHeader = FALSE;
        	        break;
				}
			}

			// Get the whole header, it may be on several lines
			char* EndHeader = strchr(Colon, '\n');
			while (EndHeader && (*++EndHeader == ' ' || *EndHeader == '\t'))
				EndHeader = strchr(EndHeader, '\n');
			if (EndHeader && IncludeHeader)
			{
				for (long NumChars = EndHeader - p; NumChars; NumChars--)
					*t++ = *p++;
			}
			p = EndHeader;
		}
	}

	if (p)
		strcpy(t, p);
	else
		*t = 0;
}


BOOL CReadMessageDoc::SaveAs(CSaveAs& SA)
{
	const char* Text = GetText();
	
	if (!GetIniShort(IDS_INI_INCLUDE_HEADERS) || m_Sum->IsSubPart())
		Text = FindBody(Text);

	if( Text == NULL )
	{
		return FALSE;
	}

	if (GetIniShort(IDS_INI_GUESS_PARAGRAPHS))
	{
		char* CopyText = ::SafeStrdupMT(Text);

		if( CopyText == NULL )
		{
			return FALSE;
		}

		BOOL Status = SA.PutText(UnwrapText(CopyText));
		delete [] CopyText;
		return (Status);
	}
	
	return (SA.PutText(Text));
}

/////////////////////////////////////////////////////////////////////////////
// CReadMessageDoc commands

void CReadMessageDoc::OnFileSaveAs() 
{
	CSaveAs SA;

	if ( ! (m_Sum && SA.Start(m_Sum->m_Subject, TRUE)) )
		return;

	SaveAsFile(SA.GetFilePtr(), SA.GetPathName());
}


BOOL CReadMessageDoc::SaveAsFile(JJFile* pFile, const char* szPathName)
{
	if ( !pFile  || !szPathName || !*szPathName )
		return (FALSE);

	CView* pView= GetView();

	QCProtocol*	view = QCProtocol::QueryProtocol( QCP_GET_MESSAGE, ( CObject* )pView );

	if (!view) 
		return (FALSE);

	CString msg;

	//Ignore Guess Paragraphs if saving as Html
	const char* Extension = strrchr(szPathName, '.');
	if (Extension && 
		(CRString(IDS_HTM_EXTENSION).CompareNoCase(Extension + 1) == 0 ||
		 CRString(IDS_HTML_EXTENSION).CompareNoCase(Extension + 1) == 0 ) )
	{
		view->GetMessageAsHTML(msg, GetIniShort( IDS_INI_INCLUDE_HEADERS ));
		if ( pFile->Put( msg ) != S_OK )
		{
			ASSERT( 0 );
			return (FALSE);
		}
	}
	else
	{
		view->GetMessageAsText(msg, GetIniShort( IDS_INI_INCLUDE_HEADERS ));

		if ( GetIniShort( IDS_INI_GUESS_PARAGRAPHS ) )
		{
			char *CopyText = ::SafeStrdupMT( msg );
			if ( pFile->Put( UnwrapText( CopyText ) ) != S_OK )
			{
				delete CopyText;
				return (FALSE);
			}
			else 
			{
				delete CopyText;
				return (TRUE);
			}
		}
		else
		{
			if ( pFile->Put(msg) != S_OK )
				return ( FALSE );
		}
	}

	return (TRUE);
}


void CReadMessageDoc::OnUpdateMessageServer(CCmdUI* pCmdUI)
{
	if (m_Sum)
	{
		if (m_Sum->IsMessageIsOnServer())
		{
			pCmdUI->Enable(TRUE);
			return;
		}
	}

	pCmdUI->Enable(FALSE);
}


void CReadMessageDoc::OnMessageServerLeave()
{
	if (m_Sum)
		m_Sum->SetMessageServerLeave();
}


void CReadMessageDoc::OnMessageServerFetch()
{
	if (m_Sum)
		m_Sum->SetMessageServerDownload();
}


void CReadMessageDoc::OnMessageServerDelete()
{
	if (m_Sum)
		m_Sum->SetMessageServerDelete();
}


void CReadMessageDoc::OnMessageServerFetchDelete()
{
	if (m_Sum)
		m_Sum->SetMessageServerDownloadDelete();
}
