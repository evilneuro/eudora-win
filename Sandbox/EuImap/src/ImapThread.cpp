// imapthread.cpp: implementation of classes that do multithreading..
//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include <fcntl.h>

#include "stdafx.h"

#include <ctype.h>
#ifdef WIN32
#include <afxcmn.h>
#include <afxrich.h>
#endif

#include <dos.h>
#include <mmsystem.h>

#include "eudora.h"
#include "fileutil.h"
#include "rs.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "header.h"
#include "mime.h"
#include "imap.h"
#include "ImapExports.h"
#include "imapgets.h"
#include "imapfol.h"
#include "imapacct.h"
#include "imapactl.h"
#include "imapdlg.h"
#include "imaputil.h"		// Includes imapdll's imap.h
#include "imapthread.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// ========== Statics ====================//
UINT ImapFetchMessageFunc (LPVOID p);

//===================================================================//


// ================ CImapThread base class ==========================//

CImapThread::CImapThread ()
{
	// This must be set before long!!
	m_AccountId		= 0;
	m_pWinThread	= NULL;
}


// NOTES
// This is declared virtual so we call any derived destructors first.
// END NOTES
CImapThread::~CImapThread()
{
}




// ====================== CImapMessageThread class ========================/
CImapMessageThread::CImapMessageThread ()
{
}


CImapMessageThread::~CImapMessageThread()
{

}


//
// FUNCTION
// Copy uid's into the internal list.
// END FUNCTION

void CImapMessageThread::CopyUidList (CPtrList& UidList)
{
	POSITION	pos;
	CImapFlags *pOldFlags;
	CImapFlags *pNewFlags;

	// Clear our list.
	m_OldUidList.DeleteAll();

	pos = UidList.GetHeadPosition();
	while (pos)
	{
		pOldFlags = (CImapFlags *)UidList.GetNext(pos);
		if (pOldFlags)
		{
			pNewFlags = new CImapFlags (pOldFlags->m_Uid,
										pOldFlags->m_Imflags,
										pOldFlags->m_IsNew);
			if (pNewFlags)
			{
				// Insert it.
				m_OldUidList.AddTail( pNewFlags );
			}
		}
	}
}




// FetchNewMessages [public]
// FUNCTION
// Do the hard work of creating the system thread and fetching the messages.
// END FUINCTION

BOOL CImapMessageThread::FetchNewMessages(CImapMailbox *pImapMailbox)
{
	// Sanity.
	if (!pImapMailbox)
		return FALSE;

#if 0 // No longer used.
	
	// copy stuff from the given IMAP Mailbox into our own.
	m_ImapMailbox.Copy (pImapMailbox);

	// 

	// Dup the CIMAP contained in the given pImapMailbox.
	m_ImapMailbox.m_Imap.Dup (&pImapMailbox->m_Imap);

	// Create the new MFC thread in a suspended state.
	// Note: the MFC thread object is saved so that we can delete it.
	m_pWinThread = AfxBeginThread ( ImapFetchMessageFunc, 
								  (LPVOID) this, 
								  THREAD_PRIORITY_NORMAL,
								  0,
								  CREATE_SUSPENDED );
	ASSERT (m_pWinThread);

	m_pWinThread->m_bAutoDelete = FALSE;

	// Launch the thread.
	// NOTE: pWinThread is not valid after this!!

	m_pWinThread->ResumeThread();

#endif // 

	return TRUE;	
}



// __FetchNewMessages [private]
// FUNCTION
// Do the real work.
// END FUNCTION
void CImapMessageThread::DoFetchNewMessages ()
{

#ifdef OMITFORNOW  // No background checking for now.

	// We are back to the mailbox's fetch messages. 
	BOOL bResult = m_ImapMailbox.FetchNewMessages (m_OldUidList);

	// If we succeed, can we update the real mailbox?
	if (!bResult)
		return;

	// Get the real MBX filename.
	CString MbxFilePath;
	GetMbxFilePath (m_ImapMailbox.GetDirname(), MbxFilePath);

	// See if the toc is loaded. Set "OnlyIfLoaded to TRUE".
	CTocDoc *pToc = GetToc (MbxFilePath, NULL, FALSE, TRUE);

	// If it's not loaded, we can update it in the background.
	BOOL bLoaded = FALSE;
	if (!pToc)
	{
		bLoaded = FALSE;

		// Load it from disk.
		pToc = GetToc (MbxFilePath);
	}

	if (!pToc)
		return;

#if 0 // BUG - Go do the update from this thread for now!!! This MUST be changed!!
	if (bLoaded)
	{
		// Just set a flag and return.
		if (pToc->m_pImapMailbox)
			m_pImapMailbox->SetNeedsUpdateFromTemp (TRUE);
	}
	else
#endif // BUG!!!

	{
		// We can go and do the merge ourselves.
		pToc->ImapMergeTmpMbx (&m_OldUidList);
	}

#endif // OMITFORNOW

}





// ============== INTERNAL STATIC functions =====================//

// Static thread callback.
UINT ImapFetchMessageFunc (LPVOID p)
{
	if (!p)
		return 0;

	// Get the thread object.
	CImapMessageThread *pImapMessageThread = (CImapMessageThread *) p;

	// Do the real work.
	pImapMessageThread->DoFetchNewMessages ();

	return 1;
}

#endif // IMAP4
