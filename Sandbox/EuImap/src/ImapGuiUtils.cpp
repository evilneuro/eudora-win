// ImapGuiUtils.cpp - Implement some IMAP gui utils.
//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include <ctl3d.h>

#include <io.h>

#include "resource.h"
#include "guiutils.h"
#include "doc.h"
#include "rs.h"
#include "eudora.h"
#include "cursor.h"
#include "fileutil.h"
#include "mime.h"
#include "progress.h"

#include "imapfol.h"
#include "ImapDownload.h"

#include "ImapGuiUtils.h"

// Initialize static members:
//
short CImmediateProgress::m_sRefCount		= 0;
short CImmediateProgress::m_sProgressIdle = 0;
BOOL  CImmediateProgress::m_bBusy			= FALSE;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//=============================== CImmediateProgress =================================//
// Show progress immediately then replace INI setting:
//=========================================================================//
CImmediateProgress::CImmediateProgress ()
{
	// THis must be called only in the main thread:
	//
	if ( !IsMainThreadMT() )
	{
		ASSERT (0);
		return;
	}

	// Prevent re-entrancy:
	if (m_bBusy)
	{
		m_bWasBusy = TRUE;
		return;
	}
	else
	{
		m_bWasBusy = FALSE;
	}

	// Otherwise:
	m_bBusy = TRUE;

	// Declare that weve been here:
	m_sRefCount++;

	// Are we the first?
	// 
	if (m_sRefCount == 1)
	{
		// If someone already has the progress up, there's nothing we can do:
		//
		if (!InProgress)
		{
			// Get INI setting:
			//
			m_sProgressIdle = GetIniShort (IDS_INI_PROGRESS_IDLE);

			// Set  it to zero:
			//
			SetIniShort (IDS_INI_PROGRESS_IDLE, 0);
		}
	}

	m_bBusy = FALSE;
}


CImmediateProgress::~CImmediateProgress()
{
	// THis must be called only in the main thread:
	//
	if ( !IsMainThreadMT() )
	{
		ASSERT (0);
		return;
	}

	// Did we fail re-entrancy?
	//
	if (m_bWasBusy)
		return;

	// were we the first?
	//
	if (m_sRefCount <= 1)
	{
		// Reset progress idle:
		//
		SetIniShort (IDS_INI_PROGRESS_IDLE, m_sProgressIdle);

		m_sProgressIdle = 0;

		m_sRefCount--;

		if (m_sRefCount < 0)
		{
			ASSERT (0);
			m_sRefCount = 0;
		}
	}
	else
	{
		m_sRefCount--;
	}
}



///////////////////////////////////////////////////////////////////
// ImapGuiFetchAttachment 
//
// If this is an IMAP attachment stub file, go retrieve it's real
// contents from the server.
// Actually, this is just a stub file so we didn't have to include real IMAP stuff in
// "guiutils.cpp"
///////////////////////////////////////////////////////////////////
BOOL ImapGuiFetchAttachment (LPCSTR Filename)
{
	// Pass the buck to ImapDownloader.
	return ImapDownloaderFetchAttachment (Filename);
}



#endif // IMAp4


