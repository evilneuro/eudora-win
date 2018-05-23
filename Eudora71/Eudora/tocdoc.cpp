// TOCDOC.CPP
//
// Routines for handling the data of mailbox TOCs
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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
#include <afxcmn.h>
#include <afxrich.h>
#include <dos.h>

#include "QCUtils.h"

#include "eudora.h"
#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "3dformv.h"
#include "tocview.h"
#include "tocframe.h"
#include "mdichild.h"
#include "msgframe.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "guiutils.h"
#include "cursor.h"
#include "progress.h"
#include "SearchView.h"
#include "debug.h"

#include "pop.h"
#include "MsgRecord.h"
#include "HostList.h"
#include "msgutils.h"
#include "address.h"
#include "eumapi.h"
#include "trnslate.h"
#include "SearchManager.h"

#include "QCImapMailboxCommand.h"
#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "Automation.h"

#ifdef COMMERCIAL
#include "persona.h"
#endif // COMMERCIAL

#ifdef IMAP4
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "ImapTypes.h"
#endif

// Include the files in the correct order to allow leak checking with malloc.
// CRTDBG_MAP_ALLOC already defined in stdafx.h
#include <stdlib.h>
#include <crtdbg.h>

#include "DebugNewHelpers.h"

extern QCMailboxDirector	g_theMailboxDirector;


// The current TOC file format only leaves room to store 8 column widths,
// so we don't save the column widths for the mood and junk columns.
#define NUM_SAVED_COLS_WIDTHS	8

// The current TOC file format only leaves room to store 9 column sort
// values so if the user chooses to sort on all 11 columns only the first
// 9 will be remembered.
#define NUM_SAVED_SORT_COLS		9


CTocDoc* CTocList::IndexData(int nIndex) const
{
	POSITION pos = FindIndex(nIndex);
	
	return (pos? GetAt(pos) : NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CTocDoc

IMPLEMENT_DYNCREATE(CTocDoc, CDoc)

CTocDoc::CTocDoc()
	:	m_UndoXferTo(CSumList::kOwnsSums, CSumList::kDontUseMap),
		m_UndoXferFrom(CSumList::kOwnsSums, CSumList::kDontUseMap)
{
	m_SavedPos.SetRectEmpty();
	m_bNeedMoodCheck = 1;
	m_iApproxSums = -1;

	// Default to allowing auto-delete
	m_nPreventAutoDelete = 0;
	m_bSaveAutoDelete = FALSE;
}

BOOL CTocDoc::OnNewDocument()
{
	int i;

	if (!CDoc::OnNewDocument())
		return (FALSE);
	
	m_ReadSums = FALSE;

	Name().Empty();
	m_strPathName.Empty();
	m_MBFileName.Empty();
	m_Version = TocVersion;
	m_NeedsCompact = FALSE;
	m_UnreadStatus = US_UNKNOWN;
	for (i = 0; i < NumFields - 1; i++)
		m_FieldWidth[i] = -1;
	m_bGroupBySubject = FALSE;
	m_bNeedsSorting = FALSE;
	m_bNeedsSortingNOW = FALSE;
	m_bNeedsFumlub = false;
	for (i = 0; i < NumSortColumns; i++)
		m_SortMethod[i] = NOT_SORTED;
	m_TopSum = m_CurrentSum = 0;
	m_NumShowing = 0;
	m_TotalSpace = m_MesSpace = m_DelSpace = 0L;
	m_NextUniqueMessageId = 0L;
	m_PluginID = 0L;
	m_PluginTag = 0L;
	m_ShowFileBrowser = FALSE;
	m_FileBrowserViewState = 0;
	m_SplitterPos = 0;
	m_UndoToToc = m_UndoFromToc = NULL;

	// Default to allowing auto-delete
	m_nPreventAutoDelete = 0;

#ifdef IMAP4
	m_pImapMailbox = NULL;
	m_bHideDeletedIMAPMsgs = FALSE;
#endif

	return (TRUE);
}

CTocDoc::~CTocDoc()
{
	ClearUndo();

#ifdef IMAP4
	if (m_pImapMailbox)
	{
		// Copy the mailbox, set the member variable to 
		// NULL before we delete it. This fixes a re-entrancy bug if the 
		// network connection takes a while to terminate.
		//
		CImapMailbox *pMailbox = m_pImapMailbox;

		m_pImapMailbox = NULL;

		// NULL out the mailbox's TocDoc reference.
		pMailbox->SetTocDoc(NULL);

		delete pMailbox;
	}
#endif

}


BEGIN_MESSAGE_MAP(CTocDoc, CDoc)
	//{{AFX_MSG_MAP(CTocDoc)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	//}}AFX_MSG_MAP
	ON_COMMAND_EX(ID_PREVIOUS_MESSAGE, OnSwitchMessage)
	ON_COMMAND_EX(ID_NEXT_MESSAGE, OnSwitchMessage)
	ON_UPDATE_COMMAND_UI(ID_PREVIOUS_MESSAGE, OnUpdateSwitchMessage )
	ON_UPDATE_COMMAND_UI(ID_NEXT_MESSAGE, OnUpdateSwitchMessage )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTocDoc diagnostics

#ifdef _DEBUG
void CTocDoc::AssertValid() const
{
	CDoc::AssertValid();
}

void CTocDoc::Dump(CDumpContext& dc) const
{
	CDoc::Dump(dc);
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CTocDoc commands


const short CTocDoc::DefaultFieldWidth[CTocDoc::NumFields - 1]
    = {1, 4, 1, 1, 8, 16, 16, 2, 1, 1};


// Build
// Rebuild a .TOC file for a mailbox
//
BOOL CTocDoc::Build(long lMessageIDStart /*= 0L*/)
{
	JJFile in;

	if (FAILED(in.Open(GetMBFileName(), O_RDONLY)))
		return (FALSE);

	m_Version = TocVersion;

	m_Type = MBT_REGULAR;
	if (!strchr(((const char*)GetFileName()) + EudoraDirLen, SLASH))
	{
		CString filename(((const char*) GetFileName()) + EudoraDirLen);

		if (!filename.CompareNoCase(CRString(IDS_IN_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_IN;
		else if (!filename.CompareNoCase(CRString(IDS_OUT_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_OUT;
		else if (!filename.CompareNoCase(CRString(IDS_TRASH_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_TRASH;
		else if (!filename.CompareNoCase(CRString(IDS_JUNK_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_JUNK;
	}
	if (m_Type != MBT_REGULAR)
		m_bAutoDelete = FALSE;

#ifdef IMAP4
	// Always remove document when view is closed.
	if (g_ImapAccountMgr.FindOwningAccount (GetMBFileName()) != 0)
	{
		m_Type = MBT_IMAP_MAILBOX;
		m_bAutoDelete = FALSE;
		m_bHideDeletedIMAPMsgs = TRUE;
	}
#endif
	
	CString ProgressMessage;
	ProgressMessage.Format(CRString(IDS_TOC_BUILD_PROGRESS), (LPCTSTR)Name());
	long lTotalBytes;
	in.Seek(0, SEEK_END, &lTotalBytes);
	in.Seek(0);
	PushProgress();
	Progress(0, ProgressMessage, lTotalBytes);

	CSummary::m_lBegin = CSummary::m_lLastTime = 0L;
	m_TotalSpace = m_MesSpace = m_DelSpace = 0L;
	m_NextUniqueMessageId = lMessageIDStart;
	m_PluginID = 0L;
	m_PluginTag = 0L;
	int ReadStatus = 1;
	for (int i = 0; ReadStatus == 1; i++)
	{
		CSummary* Sum = DEBUG_NEW CSummary;
		Sum->m_TheToc = this;
		ReadStatus = Sum->Build(&in, TRUE);
		if (!Sum->m_Length)
			delete Sum;
		else
		{
			AddExistingSum(Sum);
			ProgressAdd(Sum->m_Length);
		}
		if (EscapePressed())
		{
			ReadStatus = -1;
			break;
		}
	}
	CSummary::m_lLastTime = 0L;

	in.Close();

	// Adding sum above used to set modified flag, but that didn't make sense
	// in every case that a sum is added, so instead do it here.
	SetModifiedFlag();

	BOOL bReturn = FALSE;
	if (!ReadStatus)
	{
		Progress(lTotalBytes, NULL, lTotalBytes);
		bReturn = Write();
	}

	// If an IMAP toc, inform IMAP that we just rebuilt it.
	if (m_pImapMailbox)
		m_pImapMailbox->SetTocJustRebuilt();

	PopProgress();

	return (bReturn);
}


// Corrupt
// Check to see if TOC has been corrupted
// 
BOOL CTocDoc::Corrupt()
{
	POSITION pos = m_Sums.GetHeadPosition();
	CSummary* Sum;
	char buf[512]={0};
	long Off;
	long Len;

	while (pos)
	{
		Sum = m_Sums.GetNext(pos);
		if (!Sum)
		{
			// This should not happen. But if does, lets ASSERT
			ASSERT(0);
		}
		else
		{
			Off = Sum->m_Offset;
			Len = Sum->m_Length;
			if (Off < 0)
			{
				strcpy(buf, "Off < 0");
				goto fail;
			}

			if (Len < 0)
			{
				strcpy(buf, "Len < 0");
				goto fail;
			}
			
			if (Len == 0)
			{
#ifdef IMAP4
				//  BEGIN $_IMAP4_$ (JOK, 9/23/97).

				// A summary in an IMAP mailbox can now have zero length, implying that 
				// only minimal headers were downloaded and nothing was stored in the MBX
				// file. Determine if we think we downloaded the message body, but it's
				// missing in which case we should remove the summary or if this is a
				// case where only minimal headers downloaded so we shouldn't be surprised
				// that there's no body.
				if ( !IsImapToc() || Sum->IsIMAPMessageBodyDownloaded() )
				{
#endif // IMAP4
					// Empty summary that never got saved
					RemoveSum(Sum);
#ifdef IMAP4
				}
#endif // IMAP4
				// END $_IMAP4_$ (JOK 9/23/97).
			}

			if (Off + Len > m_TotalSpace)
			{
				strcpy(buf, "Off + Len > TotalSpace");
				goto fail;
			}
			
			if (Sum->GetUniqueMessageId() > m_NextUniqueMessageId)
			{
				strcpy(buf, "message id out-of-range");
				goto fail;
			}
		}
	}

	return (FALSE);

fail:
	if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
	{
		sprintf(buf + strlen(buf), ": Off=%ld,Len=%ld,TotalSpace=%ld,NumSums=%d: %s,%s,%s",
			Off, Len, m_TotalSpace, NumSums(), Sum->m_From, Sum->m_Subject, Sum->m_Date);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, buf);
	}

    return (TRUE);
}

// Salvage
// Try to save as many summaries as possible from a corrupt TOC file
//
BOOL CTocDoc::Salvage(BOOL Ask)
{
	BOOL bBotherOnTocRebuild = GetIniShort(IDS_INI_BOTHER_ON_TOC_REBUILD) != 0;

	if (Ask && bBotherOnTocRebuild)
	{
		if (AlertDialog(IDD_TOC_SALVAGE, (const char*)Name()) != IDOK)
			return (FALSE);
	}
	
	{
		CCursor cursor;
		int TotalSaved = 0;
		int OldNumSums = NumSums();

		// Get the largest message ID already in the toc so that when we
		// rebuild it we can start new IDs greater than the largest so we
		// know it won't conflict with any existing IDs
		long lMaxMessageID = 0L;
		for (POSITION pos = m_Sums.GetHeadPosition(); pos; )
		{
			CSummary* pSum = m_Sums.GetNext(pos);
			if (pSum)
			{
				long id = pSum->GetUniqueMessageId();
				if (id > lMaxMessageID)
					lMaxMessageID = id;
			}
		}

		// Transfer ownership of sums in m_Sums to OldSums
		CSumList OldSums;
		m_Sums.MoveSumsToOtherList(&OldSums);

		// We can't do reconciliation if we didn't fully rebuild the TOC
		if (!Build(lMaxMessageID))
			return (FALSE);

		if (m_Type != MBT_OUT)
		{
			// Set state of all new toc's to recovered
			// If we later find the matching ones in the
			// old toc, this will (properly) be ignored
			POSITION NextNewPos, NewPos = m_Sums.GetHeadPosition();
			for (NextNewPos = NewPos; NewPos; NewPos = NextNewPos)
			{
				CSummary* NewSum = m_Sums.GetNext(NextNewPos);
				if (!NewSum)
				{
					// This should not happen. but if does lets ASSERT
					ASSERT(0);
				}
				else
					NewSum->m_State = MS_RECOVERED;
			}
		}

		// Display progress in case this takes a while. Makes sure user doesn't
		// do anything else while we process events in EscapePressed.
		CString		strProgressMessage;
		strProgressMessage.Format( CRString(IDS_TOC_SALVAGE_PROGRESS), (LPCTSTR) Name() );

		PushProgress();
		Progress(0, strProgressMessage, OldNumSums);

		// Loop through old toc, looking for matches in new toc
		POSITION NextOldPos, OldPos = OldSums.GetHeadPosition();
		for (NextOldPos = OldPos; OldPos; OldPos = NextOldPos)
		{
			CSummary* OldSum = OldSums.GetNext(NextOldPos);
			if (!OldSum)
			{
				// This should not happen. but if does lets ASSERT
				ASSERT(0);
			}
			else
			{
				POSITION NextNewPos, NewPos = m_Sums.GetHeadPosition();

				for (NextNewPos = NewPos; NewPos; NewPos = NextNewPos)
				{
					CSummary* NewSum = m_Sums.GetNext(NextNewPos);
					if (!NewSum)
					{
						// This should not happen. but if does lets ASSERT
						ASSERT(0);
					}
					else
					{

						if (NewSum->m_Offset == OldSum->m_Offset)
						{
							if (NewSum->m_Length == OldSum->m_Length)
							{
								// We don't want the rebuilt one, we want the old one.
								// Get back the old one.
								m_Sums.SetAt(NewPos, OldSum);

								// Delete the rebuilt one, because we're using the old one.
								delete NewSum;
								
								// Erase the old pointer, otherwise it will get deleted when the
								// old array gets deleted
								OldSums.SetAt(OldPos, NULL);
								TotalSaved++;
							}
							break;
						}
					}
				} 
			}
			
			ProgressAdd(1);
			EscapePressed(1);
		}

		// Update progress to indicate that we're done
		Progress(OldNumSums, strProgressMessage, OldNumSums);

		// Remove the progress dialog
		PopProgress();

		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char buf[256];
			sprintf(buf, "%s .toc salvage: %d of %d saved, %d new", (LPCTSTR)Name(), TotalSaved, OldNumSums, NumSums() - TotalSaved);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, buf);
		}

		if (bBotherOnTocRebuild)
			ErrorDialog(IDS_TOC_SALVAGE_RESULT, TotalSaved, OldNumSums, NumSums() - TotalSaved);
	}

	// Notify Search Manager that it needs to reindex this mailbox
	SearchManager::Instance()->NotifyReindexMailbox( GetMBFileName() );

	return (Write());
}

// Read
// Read in a .TOC file for a mailbox
//
BOOL CTocDoc::Read(BOOL HeaderOnly /*= FALSE*/)
{
	JJFile in;
	CFileStatus MBStatus, TocStatus;
	char buf[_MAX_PATH + 64];
	unsigned short SumCount;
	short temp;
	QCMailboxCommand* pCommand;
	CRString szInBoxName( IDS_IN_MBOX_NAME );
	CRString szOutBoxName( IDS_OUT_MBOX_NAME );
	CRString szTrashBoxName( IDS_TRASH_MBOX_NAME );
	CRString szJunkBoxName( IDS_JUNK_MBOX_NAME );
	
	//
	// Helper flag to handle cases where (tsk, tsk) the user renames
	// or copies a System TOC file (In/Out/Trash/Junk) to be a regular
	// mailbox file.  If set TRUE, then set the dirty bit on the
	// TOC to force the repaired TOC to be written out to disk.
	//
	BOOL needed_repair = FALSE;

	// Don't do anything if the summaries are already read
	if (m_Sums.GetCount())
		return (TRUE);

	if (!CFile::GetStatus(GetMBFileName(), MBStatus))
		return (FALSE); 
	if (!CFile::GetStatus(GetFileName(), TocStatus))
		return Build();
	if (FAILED(in.Open(GetFileName(), O_RDONLY)))
		return Build();

	time_t MBTime = static_cast<time_t>( MBStatus.m_mtime.GetTime() );
	time_t TocTime = static_cast<time_t>( TocStatus.m_mtime.GetTime() );


	while (1)
	{
		if (FAILED(in.Get(&m_Version))) 
			break;
		// If the read version is newer than this app's current version
		// don't go any further.
		if (m_Version > TocVersion)
			break;
		if (FAILED(in.Read((char *)m_UsedVersion, UsedVersion)))
			break;
	
		char *		szTitle = m_strTitle.GetBuffer(NameSize+1);
		if (FAILED(in.Read(szTitle, NameSize)))
			break;
		// Reading in NameSize worth of characters sometimes resulted in a buffer
		// with no NULL termination character, which in turn made bad things
		// happen in the call to ReleaseBuffer. NULL terminate the sucker.
		szTitle[NameSize] = '\0';
		m_strTitle.ReleaseBuffer();
	
		// Mailbox type could also be corrupted on disk, don't trust it
		if (FAILED(in.Get(&m_Type))) 
			break;
	
		m_Type = MBT_REGULAR;
		if (!strchr(((const char*)GetFileName()) + EudoraDirLen, SLASH))
		{
			bool	bJunkFeatureInitialized = (GetIniShort(IDS_INI_ASKED_ABOUT_JUNK) != 0);
			CString basename(((const char *) GetFileName()) + EudoraDirLen);
			int dot_idx = basename.Find('.');
			if (dot_idx != -1)
				basename = basename.Left(dot_idx);
			else
				ASSERT(0);

			//
			// Since the In/Out/Trash/Junk mailbox names can be translated 
			// to non-english names, we rely on the *filenames*
			// IN.MBX, OUT.MBX, JUNK.MBX, and TRASH.MBX to determine whether or
			// not we're dealing with a system mailbox.
			//
			if (!basename.CompareNoCase(CRString(IDS_IN_MBOX_FILENAME)))
			{
				//
				// Given the filename IN.MBX, this is fer sure fer sure
				// the System In Box.  Make sure the internal name for
				// the mailbox is correct for the locale.
				//
				m_Type = MBT_IN;
				if( Name().CompareNoCase( szInBoxName ) != 0 )
				{
					//
					// Repair the locale-specific In box name to be the
					// proper locale-specific In box name.
					//
					ASSERT(0);
					Name() = szInBoxName;
					m_strTitle = szInBoxName;
					needed_repair = TRUE;
				}
			}
			else if (!basename.CompareNoCase(CRString(IDS_OUT_MBOX_FILENAME)))
			{
				//
				// Given the filename OUT.MBX, this is fer sure fer sure
				// the System Out Box.  Make sure the internal name for
				// the mailbox is correct for the locale.
				//
				m_Type = MBT_OUT;
				if (Name().CompareNoCase( szOutBoxName ) != 0)
				{
					//
					// Repair the locale-specific Out box name to be the
					// proper locale-specific Out box name.
					//
					ASSERT(0);
					Name() = szOutBoxName;
					m_strTitle = szOutBoxName;
					needed_repair = TRUE;
				}
			}
			else if (!basename.CompareNoCase(CRString(IDS_TRASH_MBOX_FILENAME)))
			{
				//
				// Given the filename TRASH.MBX, this is fer sure fer sure
				// the System Trash Box.  Make sure the internal name for
				// the mailbox is correct for the locale.
				//
				m_Type = MBT_TRASH;
				if (Name().CompareNoCase( szTrashBoxName ) != 0)
				{
					//
					// Repair the locale-specific Trash box name to be the
					// proper locale-specific Trash box name.
					//
					ASSERT(0);
					Name() = szTrashBoxName;
					m_strTitle = szTrashBoxName;
					needed_repair = TRUE;
				}
			}
			else if (bJunkFeatureInitialized && !basename.CompareNoCase(CRString(IDS_JUNK_MBOX_FILENAME)))
			{
				//
				// Given the filename JUNK.MBX, this is fer sure fer sure
				// the System Junk Box.  Make sure the internal name for
				// the mailbox is correct for the locale.
				//
				// Note that there is now a case where we don't do this here:
				// if this is the first time Eudora has been run with the Junk
				// feature and it hasn't yet checked for an existing Junk mailbox
				// don't do anything with the Junk mailbox just yet.
				//
				m_Type = MBT_JUNK;
				if (Name().CompareNoCase( szJunkBoxName ) != 0)
				{
					//
					// Repair the locale-specific Junk box name to be the
					// proper locale-specific Junk box name.
					//
					ASSERT(0);
					Name() = szJunkBoxName;
					m_strTitle = szJunkBoxName;
					needed_repair = TRUE;
				}
			}
			else
			{
				//
				// Okay, now we are fer sure fer sure dealing with a
				// normal, non-system mailbox.  Check to see if the
				// internal localized name conflicts with a localized
				// system mailbox name.  If so, then repair it by changing
				// it to the file's basename.
				//
				// See note above about first time handling of the Junk mailbox.
				//
				if (!Name().CompareNoCase( szInBoxName ) ||
					!Name().CompareNoCase( szOutBoxName ) ||
					!Name().CompareNoCase( szTrashBoxName ) ||
					(bJunkFeatureInitialized && !Name().CompareNoCase( szJunkBoxName )))
				{
					//
					// Repair the conflicting localized system mailbox
					// name to be the filename of the mailbox.
					//
					ASSERT(0);
					Name() = basename;
					m_strTitle = basename;
					needed_repair = TRUE;
				}
			}
		}
	
		if (m_Type != MBT_REGULAR)
			m_bAutoDelete = FALSE;

		// May need to reset type here.
		if (g_ImapAccountMgr.FindOwningAccount (GetMBFileName()) != 0)
		{
			m_Type = MBT_IMAP_MAILBOX;
			m_bAutoDelete = FALSE;
		}

		if (FAILED(in.Get((short *)&temp))) break;
		if (temp & 0x01)
			m_bGroupBySubject = TRUE;
		if (temp & 0x02)
			m_bNeedsSorting = m_bNeedsSortingNOW = TRUE;
		if (temp & 0x04)
			m_ShowFileBrowser = TRUE;
		m_FileBrowserViewState = (temp & 0x18) >> 3;
		if ((temp & 0x20) || (m_Type != MBT_IMAP_MAILBOX))
			m_bHideDeletedIMAPMsgs = FALSE;
		else
			m_bHideDeletedIMAPMsgs = TRUE;

		if (FAILED(in.Get((short *)&m_NeedsCompact))) break;
		if (FAILED(in.Get(&temp))) break;
		m_SavedPos.left = temp;
		if (FAILED(in.Get(&temp))) break;
		m_SavedPos.top = temp;
		if (FAILED(in.Get(&temp))) break;
		m_SavedPos.right = temp;
		if (FAILED(in.Get(&temp))) break;
		m_SavedPos.bottom = temp;

		int i;
		for (i = 0; i < NUM_SAVED_COLS_WIDTHS; i++)
			if (FAILED(in.Get(&m_FieldWidth[i]))) break;
	
		// Earlier version of TOC had a different format for the version, and a different
		// order for the field widths. The old TOC had the first field the width of the status,
		// priority, and attachment combined, whereas the new format has fields for each one,
		// and an extra field for the label.
		if (m_Version == 0x2E31)
		{
			for (i = 3; i >= 1 ; i--)
				m_FieldWidth[i + 3] = m_FieldWidth[i];
			m_FieldWidth[0] = m_FieldWidth[1] = -1;
			m_FieldWidth[2] = m_FieldWidth[0];
			if (m_FieldWidth[2] > 0)
			{
				m_FieldWidth[0] = 1;
				if (--m_FieldWidth[2])
				{
					m_FieldWidth[1] = 1;
					m_FieldWidth[2]--;
				}
			}
		}
		else if (m_Version == 0x30)
		{
			// Pre junk (up to 6.0.0.5) had a different column order.
			for (i = NUM_SAVED_COLS_WIDTHS - 1; i > 1; --i)
			{
				m_FieldWidth[i] = m_FieldWidth[i - 1];
			}
			// Junk column width isn't saved due to lack of space.
			m_FieldWidth[1] = -1;
		}
		
		if (FAILED(in.Get(&temp))) break;
		m_UnreadStatus = (UnreadStatusType)temp;
		if (FAILED(in.Get(&m_NextUniqueMessageId))) break;
		if (FAILED(in.Get(&m_PluginID))) break;
		if (FAILED(in.Get(&m_PluginTag))) break;

		if (!IsPeanutTagged() && m_ShowFileBrowser)
		{
			// This shouldn't happen, although somehow it did for Matt
			ASSERT(0);

			// To the best of my knowledge we shouldn't be trying to show
			// the file browser when the mailbox isn't tagged by ESP.
			// It certainly caused bad behavior for Matt. Correct this
			// (although it would be nice to know how it happened).
			m_ShowFileBrowser = FALSE;

			// Make sure that we write out the correction
			SetModifiedFlag(TRUE);
		}

		if (FAILED(in.Get(&m_SplitterPos))) break;
		if (!m_SplitterPos)
		{
			m_SplitterPos = GetIniShort(IDS_INI_PREVIEW_SPLITTER_POS);
			if (!m_SplitterPos)
				m_SplitterPos = (short)((m_SavedPos.bottom - 50) / 2);
		}

		for (i = 0; i < NUM_SAVED_SORT_COLS; i++)
		{
			char c;
			if (FAILED(in.Get(&c))) break;
			m_SortMethod[i] = (SortType)(unsigned char)c;
		}
		if (m_Version == 0x30)
		{
			// Pre junk (up to 6.0.0.5) had a different sort order.
			for (i = 0; i < NUM_SAVED_SORT_COLS; ++i)
			{
				if ((m_SortMethod[i] > 1) && (m_SortMethod[i] <= 9))
				{
					// Bump the previous values for priority through mood up by one
					// to account for junk.
					m_SortMethod[i] = (SortType)(m_SortMethod[i] + 1);
				}
				else if (m_SortMethod[i] == 10)
				{
					// Junk score column is now in position 2.
					m_SortMethod[i] = BY_JUNK;
				}
				else if ((m_SortMethod[i] > 129) && (m_SortMethod[i] <= 138))
				{
					// Bump the previous values for reverse priority through mood up by one
					// to account for junk.
					m_SortMethod[i] = (SortType)(m_SortMethod[i] + 1);
				}
				else if (m_SortMethod[i] == 130)
				{
					// Junk score column is now in position 2.
					m_SortMethod[i] = BY_JUNK;
				}

			}
		}

		if (FAILED(in.Get((char *)&m_AdFailure))) break;
		unsigned long OldMbxSize;
		if (FAILED(in.Get(&OldMbxSize))) break;
		if (FAILED(in.Read(buf, UnusedDiskSpace))) break;
		if (FAILED(in.Get(&SumCount))) break;
		
		if (HeaderOnly)
			return (TRUE);
		
		long FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
		UnreadStatusType NewUnreadStatus = US_NO;
		int ReadStatus = -1;
		m_TotalSpace = m_MesSpace = 0L;

		while (1)
		{
			CSummary* Sum = DEBUG_NEW CSummary;
			Sum->m_TheToc = this;
			ReadStatus = Sum->Read(&in);
			if (ReadStatus <= 0)
			{
				delete Sum;
				break;
			}
			AddExistingSum(Sum);

			if (NewUnreadStatus == US_NO && Sum->m_State == MS_UNREAD &&
				(Sum->m_Seconds + Sum->m_TimeZoneMinutes * 60) > FreshnessDate)
			{
				NewUnreadStatus = US_YES;
			}
		}

		if (ReadStatus < 0)
			break;
			
		in.Close();
		m_TotalSpace = static_cast<long>(MBStatus.m_size);

		if (NewUnreadStatus == US_YES && TocTime < FreshnessDate)
			NewUnreadStatus = US_NO;
		
		if (m_UnreadStatus != NewUnreadStatus)
		{
			// If we've changed unread status then save the new status by writing out the header
			m_UnreadStatus = NewUnreadStatus;
			if (!WriteHeader())
				break;
		}

		pCommand = g_theMailboxDirector.FindByPathname( GetMBFileName() );
		ASSERT( pCommand );
		if( pCommand )
		{
			pCommand->Execute( CA_UPDATE_STATUS, ( void* ) m_UnreadStatus );
		}

		if (needed_repair)
			SetModifiedFlag(TRUE);		// mark repaired TOC as dirty to force disk write
		else
			SetModifiedFlag(FALSE);
		
		// Note that NumSums really returns an int, which of course can exceed
		// the 65535 max value that we saved in the TOC and loaded into SumCount.
		// Compare NumSums as an unsigned short, so that we won't think that the
		// TOC is corrupt just because it exceeded 65535 messages. This means that
		// our checking for more than 65535 isn't perfect as it only checks the
		// lower 2 bytes, but it should be rare for us to have missing messages
		// and still match those 2 bytes exactly.
		if ( SumCount != static_cast<unsigned short>(NumSums()) )
		{
//			NumSums = SumCount;
			break;
		}
		if (Corrupt())
			break;
		
		long leeway = GetIniLong(IDS_INI_TOC_DATE_LEEWAY);

		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char buf[ 80 ];

			sprintf( buf, "Leeway %ld, %s: MBX %ld, TOC %ld", leeway, (LPCTSTR)Name(), MBTime, TocTime );
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, buf );
			
			sprintf( buf, "%s .mbx size: MBX %I64u, TOC %lu", (LPCTSTR)Name(), MBStatus.m_size, OldMbxSize );
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, buf );
		}

		BOOL bNeedsSalvage = FALSE;
		if ( leeway && ( MBTime > ( TocTime + leeway ) ) )
		{
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, "Failed leeway test" );
			bNeedsSalvage = TRUE;
		}
		if (OldMbxSize && OldMbxSize - 1 != MBStatus.m_size)
		{
			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, "Failed size test" );
			bNeedsSalvage = TRUE;
		}
		if (bNeedsSalvage)
		{
			BOOL bBotherOnTocRebuild = GetIniShort(IDS_INI_BOTHER_ON_TOC_REBUILD) != 0;
			int Result = IDOK;
			
			if (bBotherOnTocRebuild)
			{
				// The Carol Brummage Memorial Alert Cascade
				Result = AlertDialog(IDD_TOC_TIME, (const char*)Name());

				if (Result == IDC_USE_OLD)
				{
					Result = AlertDialog(IDD_TOC_TIME1);
					if (Result == IDC_USE_OLD)
						Result = AlertDialog(IDD_TOC_TIME2);
				}
			}

			switch (Result)
			{
			case IDOK:
				return (Salvage(FALSE));
			case IDC_USE_OLD:
				// Touch TOC so next time it is opened the date is later than the mailbox date
				Write();
				break;
			default:
				return (FALSE);
			}
		}
		
		m_DelSpace = m_TotalSpace - m_MesSpace;
		m_NeedsCompact = (m_DelSpace != 0L);
		m_ReadSums = TRUE;

		return (TRUE);
	}

	return (Salvage(TRUE));
}

// Write
// Write out a .TOC file for a mailbox
//
BOOL CTocDoc::Write(BOOL bHeaderOnly /*= FALSE*/, BOOL bSkipUnreadNotification /*= FALSE*/ )
{
	JJFile out;
	char buf[64];
	short temp;
	int i;
	QCMailboxCommand*	pCommand;

	if (bHeaderOnly)
	{
		if (FAILED(out.Open(GetFileName(), O_CREAT | O_RDWR)))
			return (FALSE);
	}
	else
	{
		if (FAILED(out.Open(GetFileName(), O_CREAT | O_TRUNC | O_WRONLY)))
			return (FALSE);
	}

	m_Version = TocVersion;
	if (FAILED(out.Put(m_Version))) return (FALSE);
	if (FAILED(out.Put((const char *)m_UsedVersion, UsedVersion))) return (FALSE);

	memset(buf, 0, NameSize);
	strcpy(buf, Name());
	if (FAILED(out.Put(buf, NameSize))) return (FALSE);

	if (FAILED(out.Put(m_Type))) return (FALSE);

	temp = 0;
	if (m_bGroupBySubject)
		temp |= 0x01;
	if (m_bNeedsSorting || m_bNeedsSortingNOW)
		temp |= 0x02;
	if (m_ShowFileBrowser)
		temp |= 0x04;
	temp |= ((m_FileBrowserViewState & 0x03) << 3);
	if (!m_bHideDeletedIMAPMsgs)
		temp |= 0x20;

	if (FAILED(out.Put((short)temp))) return (FALSE);

	if (FAILED(out.Put((short)m_NeedsCompact)))  return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.left))) return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.top))) return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.right))) return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.bottom))) return (FALSE);

	for (i = 0; i < NUM_SAVED_COLS_WIDTHS; i++)
		if (FAILED(out.Put(m_FieldWidth[i]))) return (FALSE);

	long FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
	UnreadStatusType NewUnreadStatus = US_NO;
	long lUnreadOffset = 0;
	out.Tell(&lUnreadOffset);
	ASSERT(lUnreadOffset >= 0);
	if (FAILED(out.Put((short)m_UnreadStatus))) return (FALSE);
	if (FAILED(out.Put(m_NextUniqueMessageId))) return (FALSE);

	if (FAILED(out.Put(m_PluginID))) return (FALSE);
	if (FAILED(out.Put(m_PluginTag))) return (FALSE);
	if (FAILED(out.Put(m_SplitterPos))) return (FALSE);

	for (i = 0; i < NUM_SAVED_SORT_COLS; i++)
		if (FAILED(out.Put((char)m_SortMethod[i]))) break;

	// We stash ad failure here
	if (FAILED(out.Put((char)m_AdFailure))) return FALSE;

	// Write the size of the .mbx file so that we can see if the .mbx file has been changed
	// behind our back.  The +1 is a hack because older .tocs without this field would have a
	// default value of zero, which we don't want to consider a mismatch
	CFileStatus MBStatus;
	unsigned long lMbxFileSize = m_TotalSpace;
	if (CFile::GetStatus(GetMBFileName(), MBStatus))
	{
		lMbxFileSize = static_cast<unsigned long>(MBStatus.m_size);
		if (m_TotalSpace != static_cast<long>(lMbxFileSize))
		{
			// Somehow m_TotalSpace got set incorrectly.
			// Warn the developer, and set it back on track.
			// It really only affects the display in the compact button.
			ASSERT(IsImapToc());
			m_TotalSpace = lMbxFileSize;
		}

	}
	if (FAILED(out.Put(lMbxFileSize + 1))) return FALSE;
	
	// Clear buffer for writing empty space to disk
	memset(buf, 0, UnusedDiskSpace);
	if (FAILED(out.Put(buf, UnusedDiskSpace))) return (FALSE);

	// Note that NumSums really returns an int, which of course can exceed the
	// 65535 max value that we can store in the 2 bytes of an unsigned short.
	// However we still write it out as an unsigned short because:
	// * We only use it for a sanity check to check for TOC corruption
	// * We can't change the file format without using up the rest of
	//	 UnusedDiskSpace, and I'm not sure that it's worth it
	if (FAILED(out.Put(static_cast<unsigned short>(NumSums())))) return (FALSE);
	if (!bHeaderOnly)
	{
		POSITION pos = m_Sums.GetHeadPosition();
		while (pos)
		{
			CSummary* Sum = m_Sums.GetNext(pos);
			if (!Sum)
			{
				// This should not happen. But if does, lets ASSERT
				ASSERT(0);
			}
			else
			{
				ASSERT(Sum->GetUniqueMessageId() > 0);
				ASSERT(Sum->GetUniqueMessageId() <= m_NextUniqueMessageId);
				if (Sum->Write(&out) < 0)
					return (FALSE);
					
				// Do we got a recent unread message here?
				if (NewUnreadStatus == US_NO && Sum->m_State == MS_UNREAD &&
					(Sum->m_Seconds + Sum->m_TimeZoneMinutes * 60) > FreshnessDate)
				{
					NewUnreadStatus = US_YES;
				}
			}
		}

		if (m_UnreadStatus != NewUnreadStatus)
		{
			m_UnreadStatus = NewUnreadStatus;
			if (FAILED(out.Seek(lUnreadOffset)) || FAILED(out.Put((short)m_UnreadStatus)))
				return (FALSE);
		}

		if ( ! bSkipUnreadNotification )
		{
			pCommand = g_theMailboxDirector.FindByPathname( GetMBFileName() );

			// ASSERT only if this is not a .tbx mailbox.
			if ( !ImapIsTmpMbx (GetMBFileName()) )
				ASSERT( pCommand );

			if( pCommand )
				pCommand->Execute( CA_UPDATE_STATUS, ( void* ) m_UnreadStatus );
		}
	}
	
	if (!bHeaderOnly)
		SetModifiedFlag(FALSE);

	return (TRUE);
}

BOOL CTocDoc::WriteSum(CSummary* Sum)
{
    JJFile out;
    int Index = m_Sums.GetIndex(Sum);
    
    if (Index < 0)
    	return (FALSE);

	if (FAILED(out.Open(GetFileName(), O_CREAT | O_WRONLY)))
        return (FALSE);
    if (FAILED(out.Seek(104 + 218L * Index)))
        return (FALSE);
    return (Sum->Write(&out));
}

BOOL CTocDoc::OnSaveDocument(const char* /*pszPathName*/)
{
	if (!Write())
		return (FALSE);

	SetModifiedFlag(FALSE);
	
	return (TRUE);
}

BOOL CTocDoc::SaveModified()
{
	if (!IsModified())
		return (TRUE);

	return (DoSave(m_strPathName, FALSE));
}

CSummary* CTocDoc::UpdateSum(JJFile* MBox, BOOL First)
{
	CSummary* Sum = DEBUG_NEW_MFCOBJ_NOTHROW CSummary;

    if (!Sum)
        return (NULL);

	// Turn off notifying search manager (don't need to save previous value
	// because the summary was just created)
	Sum->SetNotifySearchManager(false);

    if (First)
    {
        MBox->Tell(&CSummary::m_lBegin);
        ASSERT(CSummary::m_lBegin >= 0);
    }

    Sum->m_TheToc = this;
    Sum->Build(MBox);

	// Restore notifying search manager. Don't need to notify search manager
	// of the above changes, because we haven't added it yet. Adding the
	// summary below will notify the search manager.
	Sum->SetNotifySearchManager(true);

    if (!Sum->m_Length)
    {
        delete Sum;
        return (NULL);
    }

	// Prevent unnecessary screen repaints
	if (GetView())
		GetView()->m_SumListBox.SetRedraw(FALSE);

    AddSum(Sum);
    SetModifiedFlag();

    return (Sum);
}


void CTocDoc::CheckNumSums()
{
	int			 iNumSums = NumSums();

	if ((m_iApproxSums != -1) && (abs(m_iApproxSums - iNumSums) > GetIniShort(IDS_INI_DEBUG_TOC_DISC_LEVEL)))
	{
		if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
		{
			char			 szBuf[128];
			sprintf(szBuf, "TOC size change: %s was %d now %d", GetMBFileName(), m_iApproxSums, iNumSums);
			PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szBuf);
		}
	}

	m_iApproxSums = iNumSums;
}


BOOL CTocDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	int MSC = GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE);
	POSITION pos = m_Sums.GetHeadPosition();

	// Check to see if window position has changed, and save position if it has
	WINDOWPLACEMENT wp;
	CRect rect;
	
	wp.length = sizeof(WINDOWPLACEMENT);
	pFrame->GetWindowPlacement(&wp);
	rect = wp.rcNormalPosition;
	if (rect != m_SavedPos)
	{
		m_SavedPos = rect;
		SetModifiedFlag();
	}

	ASSERT_KINDOF(CTocFrame, pFrame);

	// We get the last splitter pos that the TOC frame stored, because when dealing
	// with maximized windows this value is only accurate when the window is active
	// and when Eudora is quitting we'll be called with lots of windows that are not
	// active. The TOC frame stores the last good information for us to retrieve here.
	//
	// If GetLastSplitterPos returns -1, then the last splitter pos is invalid.
	INT iPos = ( ( CTocFrame* )pFrame )->GetLastSplitterPos();

	if (UsingPreviewPane() && (iPos != -1) && (m_SplitterPos != iPos) )
	{
		m_SplitterPos = (short)iPos;
		SetModifiedFlag();
	}

	// Are there messages open that belong to this toc?
	// Assume auto delete provided we're dealing with a regular (non-system) mailbox
	if (m_Type == MBT_REGULAR)
		m_bAutoDelete = (m_nPreventAutoDelete == 0);
	
	while (pos)
	{
		CSummary* Sum = m_Sums.GetNext(pos);
		CFrameWnd* Frame;
		
		if (Sum && (Frame = Sum->m_FrameWnd))
		{
			if (!MSC)
			{
				m_bAutoDelete = FALSE;
				break;
			}
			else
			{
				CDocument* doc = Frame->GetActiveDocument();
				if (doc && !doc->CanCloseFrame(Frame))
				{
					m_bAutoDelete = FALSE;
					return (FALSE);
				}
			}
		}
	}
	
	BOOL Result = CDoc::CanCloseFrame(pFrame);

	if (Result)
	{
		PreCloseFrame(pFrame);
	
		// The TOC may not be destroyed, but we want to close the IMAP connection
		// in any case:
		//
		if (IsImapToc() && (NULL != m_pImapMailbox) )
			m_pImapMailbox->Close();
	}
	
	return (Result);
}

// --------------------------------------------------------------------------
//
// NotifyFrameClosing [PUBLIC]
//
// When the CTocFrame goes away we need to tell the CTocDoc, even
// tho for In,Out,Trash,Junk the CTocDoc doesn't actually die. This is
// where we will do IDS_INI_MAILBOX_SUPERCLOSE
//
void CTocDoc::NotifyFrameClosing()
{
	if (GetIniShort(IDS_INI_MAILBOX_SUPERCLOSE))
	{
		// When Mailbox Superclose is on, close down all the message windows belonging to this
		// TOC because if we got here then they all agreed it was okay to be closed.
		POSITION pos = m_Sums.GetHeadPosition();
		while (pos)
		{
			CSummary* Sum = m_Sums.GetNext(pos);
			CFrameWnd* Frame;
			CDocument* MsgDoc;
			
			if (Sum && (Frame = Sum->m_FrameWnd) && (MsgDoc = Frame->GetActiveDocument()))
				Frame->SendMessage(WM_CLOSE);
		}
	}
}


void CTocDoc::PreCloseFrame(CFrameWnd* pFrame)
{
	// Compact if need be
	DWORD dwFreeSpace = GetAvailableSpaceMT( GetMBFileName() );
	DWORD dwCompactMailboxPercent = GetIniLong(IDS_INI_COMPACT_MAILBOX_PERCENT);
	DWORD dwCompactDiskPercent = GetIniLong(IDS_INI_COMPACT_DISK_PERCENT);

	// Do divide before multiply so that we don't get an overflow
	if ((DWORD)m_DelSpace > ((m_TotalSpace / 100) * dwCompactMailboxPercent) ||
		(DWORD)m_DelSpace > ((dwFreeSpace  / 100) * dwCompactDiskPercent))
	{
		g_theMailboxDirector.CompactAMailbox( this );
	}
	
	CDoc::PreCloseFrame(pFrame);
}


POSITION CTocDoc::InternalAddSum(CSummary* pSum, bool bSelectIt /*= false*/)
{
	POSITION	posSum = NULL;
	
	if (pSum)
	{
		m_MesSpace += pSum->m_Length;
		m_TotalSpace += pSum->m_Length;
		pSum->m_TheToc = this;

		//
		// Mark the new summary object with a unique message id.  In
		// theory, the message id for this TOC could roll over a
		// 32-bit unsigned integer, but we're not doing anything to
		// prevent the rollover from happening.  We just have to hope
		// that users will never process more than 4 billion messages
		// through a given Eudora mailbox.  :-)
		//
		if (pSum->GetUniqueMessageId() == 0)
		{
			// never allow id to be zero or negative when valid
			// this is similar to coding for beyond the year 2000,
			// as if that will ever happen ...
			// actually this in here to support automation where such
			// a thing is slightly more likely
			m_NextUniqueMessageId++;
			if (m_NextUniqueMessageId <= 0) m_NextUniqueMessageId = 1L;
			pSum->SetUniqueMessageId(m_NextUniqueMessageId);
		}
		ASSERT(pSum->GetUniqueMessageId() <= m_NextUniqueMessageId);

		// Now that we've made sure that it has a unique message ID, add it
		// to the list of summaries.
		posSum = m_Sums.AddTail(pSum);

		CTocView* View = GetView();
		if (View)
		{
			// Add the new summary to the view unless we are hiding deleted IMAP messages and
			// this is a deleted IMAP message.
			if (!HideDeletedIMAPMsgs() || !pSum->IsIMAPDeleted())
			{
				int Index = View->m_SumListBox.AddString((LPCSTR)pSum);
				if (bSelectIt && Index >= 0)
					View->m_SumListBox.SetSel(Index);
			}
			View->SetCompact();
		}
				
		if (pSum->m_nMood <= 0)
			m_bNeedMoodCheck = 1;
	}

	return posSum;
}


void CTocDoc::AddExistingSum(CSummary* pSum)
{
	InternalAddSum(pSum, false);
}


void CTocDoc::AddSum(CSummary* pSum, bool bSelectIt /*= false*/)
{
	InternalAddSum(pSum, bSelectIt);

	if (pSum)
	{
		SetModifiedFlag();

		m_bNeedsSorting = TRUE;
		m_bNeedsSortingNOW = TRUE;
	}

	// A general notification mechanism would be cooler, but for now
	// we'll just call the one manager that cares about added sums.
	SearchManager::Instance()->NotifySumAddition(this, pSum);
}


void CTocDoc::SubtractSum(POSITION SumPos, int Index /*= -1*/)
{
	if (SumPos)
	{
		CSummary* Sum = m_Sums.GetAt(SumPos);

		if (!Sum)
		{
			ASSERT(0);
		}
		else
		{	
			// A general notification mechanism would be cooler, but for now
			// we'll just call the one manager that cares about deleted sums.
			SearchManager::Instance()->NotifySumRemoval(this, SumPos, Sum);
			
			m_MesSpace -= Sum->m_Length;
			m_DelSpace += Sum->m_Length;
			
			// If we're getting rid of an unsent message, check to see if we can undo the
			// state change to the original message it was a repsonse to
			if (m_Type == MBT_OUT && Sum->m_State != MS_SENT && Sum->m_RespondingToSum)
			{
				Sum->m_RespondingToSum->SetState((char)Sum->m_RespondingToOldState);

	#ifdef IMAP4 // IMAP4
				// Unset the remote IMAP answered flag if that's the current
				// local value of m_State.
				if ( Sum->m_RespondingToSum->m_TheToc && 
					 Sum->m_RespondingToSum->m_TheToc->IsImapToc() )
				{
					Sum->m_RespondingToSum->m_TheToc->ImapSetMessageState (
									Sum->m_RespondingToSum, MS_REPLIED, TRUE);
				}
	#endif // END IMAP4

				if (Sum->m_RespondingToSum != NULL)
				{
					Sum->m_RespondingToSum->m_ResponseSum = NULL;
					Sum->m_RespondingToSum = NULL;
				}
			}

			if (Index < 0)
				Index = m_Sums.GetIndex(Sum);
			
			m_NeedsCompact = TRUE;
			m_Sums.RemoveAt(SumPos);

			Sum->SetUniqueMessageId(0);		// orphans don't have a message id.

			SetModifiedFlag();

			CTocView* View = GetView();
			if (View)
			{
				View->m_SumListBox.DeleteString(Index);
				View->SetCompact();

				// Make sure we are not about to preview the summary we are about to delete.
				InvalidateCachedPreviewSummary(Sum, true);
			}
		}
	}  
}

void CTocDoc::RemoveSum(POSITION SumPos, int Index /*= -1*/)
{
	if (SumPos)
	{
		CSummary* Sum = m_Sums.GetAt(SumPos);
		if (!Sum)
		{
			// This should not happen. But if does, lets ASSERT
			ASSERT(0);
		}
		else
		{			
			SubtractSum(SumPos, Index);

			/*// KCM - possible GPF unless MsgDoc->m_Sum is NULL'd
			CMessageDoc* MsgDoc = Sum->FindMessageDoc();
			if (MsgDoc && AfxIsValidAddress(MsgDoc, sizeof(CMessageDoc)))
			{
	`			if ( MsgDoc->m_Sum == Sum )
					MsgDoc->m_Sum = NULL;
				else
					ASSERT( 0 );
			}*/

			CMessageDoc* MsgDoc = Sum->FindMessageDoc();
			if (MsgDoc && AfxIsValidAddress(MsgDoc, sizeof(CMessageDoc)))
			{
				if ( MsgDoc->m_Sum == Sum )
					Sum->NukeMessageDocIfUnused();
			}

			delete Sum;
		}
		
	}
}

void CTocDoc::MoveSumBefore(POSITION SumPosToMove, POSITION SumPosToStay)
{
	CSummary* Sum = m_Sums.GetAt(SumPosToMove);
	if (!Sum)
		ASSERT(0);
	else
	{
		m_Sums.RemoveAt(SumPosToMove);
		m_Sums.InsertBefore(SumPosToStay, Sum);
	}
}

void CTocDoc::MoveSumAfter(POSITION SumPosToMove, POSITION SumPosToStay)
{
	CSummary* Sum = m_Sums.GetAt(SumPosToMove);
	if (!Sum)
		ASSERT(0);
	else
	{
		m_Sums.RemoveAt(SumPosToMove);
		m_Sums.InsertAfter(SumPosToStay, Sum);
	}
}

// OnEditUndo
//
// Perform the Undo involving this mailbox
//
void CTocDoc::OnEditUndo()
{
	if (m_UndoFromToc)
	{
		CTocView* View = GetView();
		CSummary* Sum;
		POSITION pos;

		// We're going to select the undone summaries, so unselect what's selected now
		if (View)
			View->SelectAll(FALSE, FALSE);

		// Add summaries back in this mailbox
		while (pos = m_UndoXferFrom.GetHeadPosition())
		{
			Sum = m_UndoXferFrom.RemoveHead();
			if (!Sum)
			{
				// This should not happen. But if does, lets ASSERT
				ASSERT(0);
			}
			else
			{
				m_TotalSpace -= Sum->m_Length;
				m_DelSpace -= Sum->m_Length;
				AddSum(Sum, true);
				m_NeedsCompact = (m_DelSpace != 0);
				// If the transfer was due to a junk related action, revert to the saved junk values.
				if (Sum->LastXferWasJunkAction())
				{
					Sum->RevertJunkValues();
				}
			}
		}

		// Take summaries out of mailbox that they were transferred to
		while (m_UndoFromToc->m_UndoXferTo.GetHeadPosition())
		{
			Sum = m_UndoFromToc->m_UndoXferTo.RemoveHead();
			if (!Sum)
			{
				// This should not happen. But if does, lets ASSERT
				ASSERT(0);
			}
			else
			{

				m_UndoFromToc->SubtractSum(Sum);
            
				// If the summary we are removing is also on the undo list to be
				// returned to the mailbox, then don't delete the storage
				if (m_UndoFromToc->m_UndoXferFrom.GetIndex(Sum) < 0)
				{
					/*// KCM - verified GPF unless MsgDoc->m_Sum is NULL'd
					CMessageDoc* MsgDoc = Sum->GetMessageDoc();
					if (MsgDoc && AfxIsValidAddress(MsgDoc, sizeof(CMessageDoc)))
					{
						if ( MsgDoc->m_Sum == Sum )
							MsgDoc->m_Sum = NULL;
						else
							ASSERT( 0 );
					}*/

					CMessageDoc* MsgDoc = Sum->FindMessageDoc();
					if (MsgDoc && AfxIsValidAddress(MsgDoc, sizeof(CMessageDoc)))
					{
						if ( MsgDoc->m_Sum == Sum )
							Sum->NukeMessageDocIfUnused();
					}

					delete Sum;
				}
			}
		}
        
		// Clear out undo lists
		m_UndoFromToc->ClearUndoTo();
		ClearUndoFrom();

		// Enable Redraw since SelectAll() left it disabled
		if (View)
		{
			View->m_SumListBox.SetRedraw(TRUE);
		}
	}
	else if (m_UndoToToc)
		m_UndoToToc->OnEditUndo();
}

// OnUpdateEditUndo
//
// Sets the Undo menu item to the proper state for this mailbox
//
void CTocDoc::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	char UndoText[256];
	const char* From = NULL;
	const char* To = NULL;
	
	if (m_UndoFromToc)
	{
		From = Name();
		To = m_UndoFromToc->Name();
	}
	else if (m_UndoToToc)
	{
		From = m_UndoToToc->Name();
		To = Name();
	}

	if (From)
	{
		sprintf(UndoText, CRString(IDS_UNDO_TRANSFER), From, To);
		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(UndoText);
		return;
	}
	pCmdUI->SetText(CRString(IDS_EDIT_UNDO));

	pCmdUI->Enable(FALSE);
}

// ClearUndoTo
//
// Clears the structure that holds information about undoing a transfer
// that occurred from another mailbox to this mailbox
//
void CTocDoc::ClearUndoTo()
{
	if (m_UndoToToc)
	{
		CTocDoc* toc = m_UndoToToc;
		m_UndoToToc = NULL;
		toc->ClearUndoFrom();

		while (m_UndoXferTo.GetHeadPosition())
			m_UndoXferTo.RemoveHead();
	}   
}

// ClearUndoFrom
//
// Clears the structure that holds information about undoing a transfer
// that occurred from this mailbox to another mailbox
//
void CTocDoc::ClearUndoFrom()
{
	if (m_UndoFromToc)
	{
		CTocDoc* toc = m_UndoFromToc;
		m_UndoFromToc = NULL;
		toc->ClearUndoTo();

		while (m_UndoXferFrom.GetHeadPosition())
		{
			CSummary* Sum = m_UndoXferFrom.RemoveHead();
		
			if (m_UndoXferTo.GetIndex(Sum) < 0)
			{
				/*// KCM - verified GPF unless MsgDoc->m_Sum is NULL'd
				CMessageDoc* MsgDoc = Sum->GetMessageDoc();
				if (MsgDoc && AfxIsValidAddress(MsgDoc, sizeof(CMessageDoc)))
				{
					if ( MsgDoc->m_Sum == Sum )
						MsgDoc->m_Sum = NULL;
					else
						ASSERT( 0 );
				}*/

				CMessageDoc* MsgDoc = Sum->FindMessageDoc();
				if (MsgDoc && AfxIsValidAddress(MsgDoc, sizeof(CMessageDoc)))
				{
					if ( MsgDoc->m_Sum == Sum )
						Sum->NukeMessageDocIfUnused();
				}

				delete Sum;				
			}
		}
	}   
}


////////////////////////////////////////////////////////////////////////
// GetNextMessageId [public]
//
// Given a 'messageId', find and return the messageId for the next
// message in this TOC which matches the 'sortByDate' and 'unreadOnly'
// criteria, returning TRUE if successful.  If the incoming
// 'messageId' is 0xFFFFFFFF, this means we should return the id for
// the "first" message which matches the criteria.
//
// This routine supports the MAPIFindNext() function in Simple MAPI,
// and it is meant to be used with the Eudora Inbox TOC, in
// particular, although it can apply equally well to any TOC.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::GetNextMessageId(unsigned long& messageId, BOOL sortByDate, BOOL unreadOnly)
{
	if (0x0 == messageId)
	{
		ASSERT(0);			// zero message id's are not allowed
		return FALSE;
	}

	CSumList* p_sumlist = NULL;
	CSumList sorted_list;	// temp list used only when we want items sorted by date
	if (sortByDate)
	{
		//
		// Ouch.  We need to make a sorted list of TOC items that
		// are sorted by date.
		// 
		if (! GetMessagesSortedByDate(sorted_list, unreadOnly))
		{
			while (! sorted_list.IsEmpty())
				sorted_list.RemoveHead();
			return FALSE;
		}

		p_sumlist = &sorted_list;
	}
	else
	{
		p_sumlist = &m_Sums;	// just use the raw unsorted message list
	}

	//
	// Find the place in the list to start looking for the next item
	// which matches the search criteria.
	//
	ASSERT(p_sumlist != NULL);
	POSITION pos = p_sumlist->FindNextByMessageId(messageId, unreadOnly);

	while (pos != NULL)
	{
		CSummary* p_sum = p_sumlist->GetAt(pos);
		if (!p_sum)
		{
			ASSERT(0);
		}
		else
		{
			if (unreadOnly && (MS_UNREAD != p_sum->m_State))
				p_sumlist->GetNext(pos);		// skip it since we want only unread messages
			else
			{
				while (! sorted_list.IsEmpty())
					sorted_list.RemoveHead();
				messageId = p_sum->GetUniqueMessageId();
				TRACE3("CTocDoc::GetNextMessageId() - %s,%s,%lu\n", p_sum->GetFrom(), p_sum->GetDate(), messageId);
				return TRUE;
			}
		}
	}

	//
	// If we get this far, we fell off the list without finding a
	// suitable item.
	//
	while (! sorted_list.IsEmpty())
		sorted_list.RemoveHead();
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetMessagesSortedByDate [private]
//
// Return a sorted list of CSummary object pointers, sorted by date in
// ascending order.  As an option, return only those CSummary objects
// which are marked as "unread".
//
// The returned 'summaryList' contains *copies* of pointers to objects
// in the m_Sums list, so don't delete any of the CSummary objects
// through these pointers.
//
// Return TRUE if everything went okay.
//
// This routine supports the MAPIFindNext() function in Simple MAPI.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::GetMessagesSortedByDate(CSumList& sortedList, BOOL unreadOnly)
{
	ASSERT(sortedList.IsEmpty());

	//
	// First, gather up all the items to be sorted.
	//
	POSITION pos = m_Sums.GetHeadPosition();
	if (NULL == pos)
		return TRUE;		// no need to sort an empty list

	while (pos != NULL)
	{
		CSummary* p_sum = m_Sums.GetNext(pos);
		if (!p_sum)		
			ASSERT(0);
		else
		{
			if (unreadOnly && (p_sum->m_State != MS_UNREAD))
				continue;		// skip message since it is not unread

			sortedList.AddTail(p_sum);
		}
	}

	if (sortedList.GetCount() > 1)
	{
		pos = sortedList.GetHeadPosition();
		sortedList.GetNext(pos);
		while (pos != NULL)
		{
			POSITION compare_pos = pos;
			POSITION next_pos = pos;
			sortedList.GetPrev(compare_pos);
			CSummary* p_sum = sortedList.GetNext(next_pos);
			if (!p_sum)
				ASSERT(0);
			else
			{
			
			// First check to see if it's already in the right place
				if ((p_sum->m_Seconds - sortedList.GetAt(compare_pos)->m_Seconds) < 0)
				{
					compare_pos = sortedList.GetHeadPosition();
					for (; compare_pos != pos; sortedList.GetNext(compare_pos))
					{
						if ((p_sum->m_Seconds - sortedList.GetAt(compare_pos)->m_Seconds) < 0)
						{
							sortedList.RemoveAt(pos);
							sortedList.InsertBefore(compare_pos, p_sum);
							break;
						}
					}
				}
			}

			pos = next_pos;
		}

	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// GetMessageById [public]
//
// Given a 'messageId', find the message and return the message data
// in the caller-provided string buffer (messageData).  The data
// format is the same as the WM_COPYDATA format created by
// MAPISendMail() and MAPISendDocuments().
//
// This routine supports the MAPIReadMail() function in Simple MAPI,
// and it is meant to be used with the Eudora Inbox TOC, in
// particular, although it can apply equally well to any TOC.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::GetMessageById(
	CString& messageData, 		//(o) returned message data
	unsigned long messageId, 	//(i) message id
	BOOL bodyAsFile, 			//(i) return body text as attachment
	BOOL envelopeOnly,			//(i) just return header info, minus body text and attachments
	BOOL markAsRead,			//(i) mark message as being read
	BOOL wantAttachments)		//(i) want the attachments returned
{
	CSummary* p_sum = m_Sums.GetByMessageId(messageId);
	if (NULL == p_sum)
		return FALSE;

	//
	// Get a copy of the full message so headers can be dealt with.
	//
	bool		bCreatedDoc = false;
	CMessageDoc* p_doc = p_sum->GetMessageDoc(&bCreatedDoc);
	if (NULL == p_doc)
		return FALSE;
	if (NULL == p_doc->GetText())				// force message to be read up from mailbox
	{
		if (bCreatedDoc)
			p_sum->NukeMessageDocIfUnused(p_doc);
		return FALSE;
	}
	char *p_fullmsg = p_doc->GetFullMessage();	// get full text of message, including all headers
	if (NULL == p_fullmsg)
	{
		if (bCreatedDoc)
			p_sum->NukeMessageDocIfUnused(p_doc);
		return FALSE;
	}

	//
	// Always get the "envelope" (header) stuff and place the info
	// in the caller's data buffer.
	//
	messageData.Empty();

	char* p_to = HeaderContents(IDS_HEADER_TO, p_fullmsg);
	if (p_to)
	{
		AddAddressData(messageData, p_to, "TO__: ");
		delete [] p_to;
		p_to = NULL;
	}

	char* p_cc = HeaderContents(IDS_HEADER_CC, p_fullmsg);
	if (p_cc)
	{
		AddAddressData(messageData, p_cc, "CC__: ");
		delete [] p_cc;
		p_cc = NULL;
	}

	char* p_bcc = HeaderContents(IDS_HEADER_BCC, p_fullmsg);
	if (p_bcc)
	{
		AddAddressData(messageData, p_bcc, "BCC_: ");
		delete [] p_bcc;
		p_bcc = NULL;
	}

	char* p_from = HeaderContents(IDS_HEADER_FROM, p_fullmsg);
	if (p_from)
	{
		AddAddressData(messageData, p_from, "FROM: ");
		delete [] p_from;
		p_from = NULL;
	}

	CString subject(p_sum->GetSubject());
	ASSERT(! subject.IsEmpty());
	ASSERT(subject.Find('\n') == -1);
	messageData += "SUBJ: " + subject + "\n";

	//
	// Form a date string in the MAPI standard date format of
	// "YYYY/MM/DD HH:MM".  By executive decision, we're always
	// normalizing the message time (stored in GMT) to the local
	// timezone.
	//
	time_t timeLocal = p_sum->m_Seconds - (::GetGMTOffset() * 60);
	struct tm* p_tm = localtime(&timeLocal);
	if (p_tm)
	{
		char date[80];
		strftime(date, sizeof(date), "%Y/%m/%d %H:%M", p_tm);
		messageData += "DATE: ";
		messageData += date;
		messageData += "\n";
	}

	//
	// Send flag values for "unread" status and "return receipt" status.
	// Simple MAPI also supports a "sent" status, but that doesn't make
	// sense for Eudora In boxes.
	//
	if (MS_UNREAD == p_sum->m_State)
		messageData += "UNRD: Unread\n";
	if (p_sum->ReadReceipt())
		messageData += "RCPT: Return Receipt\n";

	//
	// Deal with message body and attachments.
	//
	if (envelopeOnly)
	{
		//
		// When requesting envelope info only, the Microsoft 
		// implementation supposedly returns attachment
		// file info, but does *not* make a temp copies of the actual
		// attachment files.  Note that the 'envelopeOnly' flag overrides the
		// 'wantAttachments' flag, so we always return the attachment
		// info, even if 'wantAttachments' is FALSE.
		//
		//
		// Scan the message body for "Attachment Converted" lines.
		//
		LPCTSTR body = p_doc->GetText();
		CRString keyword(IDS_ATTACH_CONVERTED);
		const int KEYWORD_LEN = keyword.GetLength();
		while (body && *body)
		{
			LPCTSTR EndLine = _tcschr(body, '\n');

			if (_tcsncmp(body, keyword, KEYWORD_LEN) == 0)
			{
				int LineLen;
				if (!EndLine)
					LineLen = _tcslen(body);
				else
				{
					LineLen = EndLine - body;
					if (body[LineLen - 1] == '\r')
						LineLen--;
				}

				CString AttachFilename(body + KEYWORD_LEN, LineLen - KEYWORD_LEN);

				AddAttachmentData(messageData, AttachFilename, FALSE);
			}
			
			if (!EndLine)
				break;
			body = EndLine + 1;
		}
	}
	else
	{
		//
		// Make working copy of the message, then scan past the
		// headers until we hit the first empty line, marking the
		// start of the body text.
		//
		const char* OriginalBody = p_doc->GetText();		// make working copy of body text
		OriginalBody = FindBody(OriginalBody);
		const char* pszBody = OriginalBody;
		CString ConvertedBody;

		// Take out hard wraps in format=flowed messages.
		// This also will remove the <x-flowed> and </x-flowed> tags.
		if (strnicmp(OriginalBody, "<x-flowed>", 10) == 0)
		{
			char* Flowed = FlowText(OriginalBody);
			ConvertedBody = Flowed;
			delete [] Flowed;
			pszBody = ConvertedBody;
		}
		
		if (bodyAsFile)
		{
			//
			// According to the MAPIReadMail() docs, this *must*
			// be the first attachment.
			//
			AddBodyAttachmentData(messageData, pszBody);
		}
		else
		{
			//
			// Estimate how big to make the 'messageData' buffer.
			//
			int nLength = messageData.GetLength();
			for (const char* b = pszBody; *b; b++, nLength++)
			{
				// Each line gets replaced with "BODY: <original line>"
				if (*b == '\n')
					nLength += 6;
			}
			// Account for last line that's not LF-delimited
			if (b != pszBody && b[-1] != '\n')
				nLength += 6;

			messageData.GetBufferSetLength(nLength);
			messageData.ReleaseBuffer();

			while (*pszBody)
			{
				char* pszNewline = strchr(pszBody, '\n');
				if (pszNewline)
				{
					// temporary NULL termination
					if (pszNewline[-1] == '\r')
						pszNewline[-1] = 0;
					else
						pszNewline[0] = 0;

					messageData += "BODY: ";
					messageData += pszBody;
					messageData += "\n";

					// undo temporary NULL termination
					if (pszNewline[0] == 0)
						pszNewline[0] = '\n';
					else
						pszNewline[-1] = '\r';

					*(char*)(pszNewline - 1) = '\r';
					pszBody = pszNewline + 1;
				}
				else
				{
					// output last line
					messageData += "BODY: ";
					messageData += pszBody;
					messageData += "\n";
					break;
				}
			}
		}

		if (wantAttachments)
		{
			//
			// Ouch, we need to scan the message body again for
			// "Attachment Converted" lines.
			//
			CRString keyword(IDS_ATTACH_CONVERTED);
			const int KEYWORD_LEN = keyword.GetLength();

			CString line;
			while (*OriginalBody)
			{
				BOOL bFoundAttachLine = (strncmp(OriginalBody, keyword, KEYWORD_LEN) == 0);
				const char* pszNewline = strchr(OriginalBody, '\n');
				if (pszNewline)
				{
					if (bFoundAttachLine)
					{
						ASSERT(*(pszNewline - 1) == '\r');
						*(char*)(pszNewline - 1) = '\0';		// temporary NULL termination
						line = OriginalBody + KEYWORD_LEN;
						*(char*)(pszNewline - 1) = '\r';		// undo temporary NULL termination
					}
					OriginalBody = pszNewline + 1;
				}
				else
				{
					// handle last line
					if (bFoundAttachLine)
						line = OriginalBody + KEYWORD_LEN;
					OriginalBody += strlen(OriginalBody);
				}

				if (!line.IsEmpty())
				{
					if (bFoundAttachLine)
						AddAttachmentData(messageData, line, TRUE);
					line.Empty();
				}
			}
		}
	}

	//
	// Add blank line as terminator.
	//
	messageData += "\n";

	if (markAsRead)
	{
		if (MS_UNREAD == p_sum->m_State)
			p_sum->SetState(MS_READ);
	}

	if (bCreatedDoc)
		p_sum->NukeMessageDocIfUnused(p_doc);

	delete [] p_fullmsg;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// PutMessageById [public]
//
// Append a new message to the mailbox and return the non-zero message
// id for the newly added message.  The data format is the same as the
// WM_COPYDATA format created by MAPISendMail() and
// MAPISendDocuments().
//
// This routine supports the MAPISaveMail() function in Simple MAPI,
// and it is meant to be used with the Eudora Inbox TOC, in
// particular, although it can apply equally well to any TOC.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::PutMessageById(
	const char* messageData,	//(i) encoded message data
	unsigned long& messageId)	//(io) message id
{
	ASSERT(0 == messageId);

	//
	// Append new message.
	//
	if ((messageId = ::SaveMAPIMessage(messageData)) != 0)
		return TRUE;

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// DeleteMessageById [public]
//
// Given a 'messageId', find the message and "delete" the message
// (really, just transfers it to the trash).
//
// This routine supports the MAPIDeleteMail() function in Simple MAPI,
// and it is meant to be used with the Eudora Inbox TOC, in
// particular, although it can apply equally well to any TOC.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::DeleteMessageById(unsigned long messageId)
{
	CSummary* p_sum = m_Sums.GetByMessageId(messageId);
	if (NULL == p_sum)
		return FALSE;

	CTocDoc* p_trash_toc = GetTrashToc();
	if (NULL == p_trash_toc)
	{
		ASSERT(0);
		return FALSE;
	}

	Xfer(p_trash_toc, p_sum);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// AddAddressData [private]
//
// Given raw address data from a message header, break up the string
// of recipients such that there is one recipient per line in the
// specially-formatted data buffer for MAPIReadMail().  The given
// 'pRawAddresses' string is modified such that it is broken up into
// individual strings with NULL separators.
//
// This routine supports the GetMessageById() method.
//
////////////////////////////////////////////////////////////////////////
void CTocDoc::AddAddressData(
	CString& messageData,		//(io) specially-formatted message data
	char* pRawAddresses,		//(i) raw address string
	const char* pKeyword)	 	//(i) message data keyword
{
	ASSERT(pRawAddresses != NULL);
	ASSERT(pKeyword != NULL);

	char* p_addr = pRawAddresses;							// for walking address string
	char* p_end = pRawAddresses + strlen(pRawAddresses);	// points to NULL terminator
	while (p_addr < p_end)
	{
		char* p_addr_end = ::FindAddressEnd(p_addr);	// find end of address string

		if (!p_addr_end)
			break;

		*p_addr_end = '\0';								// terminate address string
		CString addr(p_addr);
		addr.TrimLeft();
		addr.TrimRight();
		if (! addr.IsEmpty())
		{
			ASSERT(addr.Find('\n') == -1);
			messageData += pKeyword + addr + "\n";
		}
		p_addr = p_addr_end + 1;						// skip past NULl to next address
	}
}


////////////////////////////////////////////////////////////////////////
// AddBodyAttachmentData [private]
//
// Given the body text in a CString object, create a temporary file in
// the Windows TEMP directory containing the body text.  Then, add the
// pathname to the temp file to the specially-formatted data buffer
// for MAPIReadMail().
//
// This routine supports the GetMessageById() method.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::AddBodyAttachmentData(CString& messageData, const CString& bodyText)
{
	//
	// Write a temp file into the Windows TEMP directory, using
	// a randomly generated name.
	//
	char* p_dest_pathname = _tempnam(".", "EUD");
	if (NULL == p_dest_pathname)
		return FALSE;		// couldn't create temp filename
	CString dest_pathname(p_dest_pathname);		// convert to easier-to-manage CString object
	::free(p_dest_pathname);	// _tempnam() uses malloc()
	p_dest_pathname = NULL;

	CString pathline("PATH: " + dest_pathname + "\n");

	JJFile destfile;
	// Don't display errors when attachments can't be found
	destfile.DisplayErrors(FALSE);
	if (FAILED(destfile.Open(dest_pathname, O_CREAT | O_WRONLY)))
		return FALSE;

	if (FAILED(destfile.Put(bodyText, bodyText.GetLength())))
		return FALSE;

	//
	// If we get this far, the file write was successful, so add the
	// pathname to caller-provided data buffer.
	//
	messageData += pathline;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// AddAttachmentData [private]
//
// For the given Eudora attachment file, make a copy of the file in
// the Windows TEMP directory, then add the pathname to the copied
// file to the specially-formatted data buffer for MAPIReadMail().
//
// This routine supports the GetMessageById() method.
//
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::AddAttachmentData
(
	CString& messageData,			//(io) specially-formatted message data
	const char* attachPathname,		//(i) full pathname to Eudora attachment file
	BOOL bMakeCopyOfFile			//(i) TRUE if we make a temp copy of the original file
)
{
	CString src_pathname(attachPathname);		// make working copy
	src_pathname.TrimLeft();
	src_pathname.TrimRight();
	if (src_pathname.IsEmpty())
		return FALSE;

	// Strip off leading and trailing quotes, if any
	if ('"' == src_pathname[0])
		src_pathname = src_pathname.Right(src_pathname.GetLength() - 1);
	if ('"' == src_pathname[src_pathname.GetLength() - 1])
		src_pathname = src_pathname.Left(src_pathname.GetLength() - 1);

	CString filename = src_pathname.Right(src_pathname.GetLength() - (src_pathname.ReverseFind(SLASH) + 1));
	ASSERT(! filename.IsEmpty());

	if (bMakeCopyOfFile)
	{
		//
		// Make a copy of the file into the Windows TEMP directory, using
		// a randomly generated name.
		//
		char* p_dest_pathname = _tempnam(".", "EUD");
		if (NULL == p_dest_pathname)
			return FALSE;		// couldn't create temp filename
		CString dest_pathname(p_dest_pathname);		// convert to easier-to-manage CString object
		::free(p_dest_pathname);	// _tempnam() uses malloc()
		p_dest_pathname = NULL;

		CString fileline("FILE: " + filename + "\n");
		CString pathline("PATH: " + dest_pathname + "\n");

		JJFile srcfile;
		// Don't display errors when attachments can't be found
		srcfile.DisplayErrors(FALSE);
		if (FAILED(srcfile.Open(src_pathname, O_RDONLY)))
			return FALSE;

		struct stat stat_info;
		if (FAILED(srcfile.Stat(&stat_info)))
			return FALSE;

		JJFile destfile;
		if (FAILED(destfile.Open(dest_pathname, O_CREAT | O_WRONLY)))
			return FALSE;

		if (FAILED(srcfile.JJBlockMove(0L, stat_info.st_size, &destfile)))
			return FALSE;

		//
		// If we get this far, the file copy was successful, so add
		// pathname to caller-provided data buffer.  Since we're using
		// funky temp filenames, let's be nice to the caller and provide
		// the "readable" original filename.
		//
		messageData += fileline;
		messageData += pathline;
	}
	else
	{
		//
		// Don't make a copy of the file.  Just return the pathname and
		// filename of the original file.
		//
		messageData += "FILE: " + filename + "\n";
		messageData += "PATH: " + src_pathname + "\n";
	}
	return TRUE;
}


BOOL CTocDoc::IsPeanutTagged()
{ 
	return (m_PluginID == 850 && m_PluginTag);
}



CSummary* CTocDoc::Xfer(CTocDoc* TargetToc, CSummary* SingleSum /*= NULL*/,
	BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/, BOOL RedisplayTOCs /*= TRUE*/)
{
	static BOOL bXferActive = FALSE;			// reentrancy semaphore

	CTocView *		View = GetView();
	int				SelCount = 1;
	CTempSumList	sumlist;

	if (!SingleSum)
	{
		if (!View || !(SelCount = View->GetSelCount()))
		{
			// We'd better not be trying to transfer selected messages when
			// there's no view, or no messages in the view selected
			ASSERT(0);
			return NULL;
		}
	}

	if (bXferActive)
	{
		ASSERT(0);								// whoops, somehow got here reentrantly
		return NULL;
	}
	else
		bXferActive = TRUE;						// don't leave unless you reset this flag!

	CTocView *TargetView;
	CSummary *Sum, *CopySum = NULL;
	int HighlightIndex = -1;
	BOOL Purging = FALSE;
	JJFile TargetMBox;
	JJFile SrcMBox;

	TargetView = TargetToc->GetView();
	
	if (TargetToc == this)
	{
		// If trying to delete a message from the mailbox, get rid of it
		if (m_Type == MBT_TRASH)
			Purging = TRUE;
		else
		{
			// transfering to the same TOC - probably a filter action (or silly user)
			ASSERT(bXferActive);
			bXferActive = FALSE;
			return SingleSum;		// nothing much to do, it's already here...
		}
	}

	if (GetIniShort(IDS_INI_WARN_DELETE_MANY_MESSAGES) && TargetToc->m_Type == MBT_TRASH &&
		SelCount >= GetIniShort(IDS_INI_WARN_DELETE_MANY_MESSAGES_THRESHOLD))
	{
		if (!gbAutomationCall)
		{
			if (WarnYesNoDialog(IDS_INI_WARN_DELETE_MANY_MESSAGES, IDS_WARN_DELETE_MANY_MESSAGES, SelCount) != IDOK)
			{
				bXferActive = FALSE;
				return (FALSE);
			}
		}
	}


	// Some optimizations here for purging
	BOOL ServerDelete = GetIniShort(IDS_INI_SERVER_DELETE);
	BOOL Tidy = GetIniShort(IDS_INI_TIDY_ATTACH);
	BOOL DeleteMAPI = (GetIniShort(IDS_INI_MAPI_DELETE_TRASH) != FALSE);
	BOOL DeleteAutoAttached = (GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_TRASH) != FALSE);
	char AttachDir[_MAX_PATH + 1];
	char ACStr[64];
	int ACLen = 0;
	int ADLen = 0;
	if (Purging	&& ( Tidy || DeleteMAPI ) )
	{
		GetIniString(IDS_ATTACH_CONVERTED, ACStr, sizeof(ACStr));
		ACLen = strlen(ACStr);
		GetIniString(IDS_INI_AUTO_RECEIVE_DIR, AttachDir, sizeof(AttachDir));
		if (!*AttachDir)
			wsprintf(AttachDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_ATTACH_FOLDER));
		ADLen = strlen(AttachDir);
	}

	if (Purging)
	{
		ClearUndoTo();
	}
	else
	{
		if (!Copy)
		{
			ClearUndoFrom();
			m_UndoFromToc = TargetToc;                                       
			TargetToc->ClearUndoTo();
			TargetToc->m_UndoToToc = this;
		}
		if (FAILED(TargetMBox.Open(TargetToc->GetMBFileName(), O_RDWR | O_CREAT | O_APPEND)) ||
			FAILED(SrcMBox.Open(GetMBFileName(), O_RDONLY)))
		{
			ASSERT(bXferActive);
			bXferActive = FALSE;
			return NULL;
		}
	}

	// Is this from or going to the Out mailbox?
	BOOL FromOut = (m_Type == MBT_OUT);
	BOOL ToOut = (TargetToc->m_Type == MBT_OUT);
	BOOL TransferredQueued = FALSE;

	// Set redraw to FALSE so we don't get flicker
	if (View && !Copy)
		View->m_SumListBox.SetRedraw(FALSE);
	if (!Purging && TargetView)
		TargetView->m_SumListBox.SetRedraw(FALSE);

	BOOL FoundSelected = FALSE;
	BOOL StripHeaders = FALSE;
	BOOL ClosedOpenWindow = FALSE;
	POSITION pos, NextPos;
	BOOL bGoingBackward = FALSE;
	int i;

	// If we're only doing a single summary, it's more likely that we're going to find that
	// summary near the end of the list, so start searching at the end and work backward.
	// If there's more than one summary selected, then we have to do them in normal order
	// so they get transfered to the destination mailbox in the right order.
	// If this is an IMAP mailbox and deleted messages are being hidden, go in normal order
	// so we can adjust the selection for items which do not appear.  This is perhaps not the
	// most efficient solution to this but it'll do for now. -dwiggins
	if ((SelCount == 1) && !m_bHideDeletedIMAPMsgs)
	{
		i = NumSums() - 1;
		pos = m_Sums.GetTailPosition();
		bGoingBackward = TRUE;
	}
	else
	{
		i = 0;
		pos = m_Sums.GetHeadPosition();
	}

	::AsyncEscapePressed(TRUE);
	for (NextPos = pos; SelCount && pos; pos = NextPos, bGoingBackward? i-- : i++)
	{
		CSummary* Sum;
		if (bGoingBackward)
			Sum = m_Sums.GetPrev(NextPos);
		else
			Sum = m_Sums.GetNext(NextPos);

		if (Sum->IsIMAPDeleted() && m_bHideDeletedIMAPMsgs)
		{
			// Don't count this summary because it belongs to a deleted IMAP summary and we are
			// not showing deleted summaries so this item can't be selected.
			bGoingBackward ? ++i : --i;
			continue;
		}

		// Give some time back, and let user cancel out by hitting Esc
		if (AsyncEscapePressed())
			break;

		if (SingleSum)
		{
			// Keep searching for SingleSum if we haven't hit it yet
			if (SingleSum != Sum)
				continue;

			// This is kind of tricky.  If we're getting rid of a single summary, then
			// we may be opening the message right after it down below.  But since
			// we've optimized searching for that single summary by starting at the
			// end of the list and working backwards, NextPos points to the summary
			// *before* SingleSum.  So get NextPos to point to the summary *after*
			// SingleSum.
			NextPos = pos;
			m_Sums.GetNext(NextPos);
		}
			
		if (!SingleSum)
		{
			if (View->m_SumListBox.GetSel(i) > 0)
			{
				SelCount--;
			}
			else
			{
				// See if we found the first unselected message after a selected message
				// and note its index for later selection
				if (!Copy && FoundSelected && HighlightIndex < 0)
					HighlightIndex = i;
				
				// Not selected, so we're not going to transfer it, continue on to the next summary
				continue;
			}
		}

		// Will some headers be stripped when sent to the Out mailbox?
		if (ToOut && !Sum->IsComp() && !StripHeaders)
		{
			if (AlertDialog(IDD_STRIP_HEADERS) != IDOK)
			{
				if (HighlightIndex < 0)
					HighlightIndex = i;
				break;
			}
			StripHeaders = TRUE;
		}
		
		// Close window if need be
		CFrameWnd* Frame = Sum->m_FrameWnd;
		if (CloseWindow && Frame && !Copy)
		{
			if (!Frame->GetActiveDocument()->CanCloseFrame(Frame))
			{
				if (HighlightIndex < 0)
					HighlightIndex = i;
				break;
			}

			//
			// Well, dang.  After adding drag and drop (tow truck) transfers
			// to read messages and comp messages, we can no longer blow away
			// the message window first and ask questions later.  We must 
			// close the window only *after* we're through with the tranfer 
			// processing to allow the window to clean up its OLE processing.
			// Otherwise, all hell breaks loose if the OLE data source object
			// contained in the read/comp message window gets blown away 
			// before it gets a chance to clean up.
			// 
			//Frame->GetActiveDocument()->OnCloseDocument();	// (old way no worky anymore)

			Frame->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);		// this is the new way
			ClosedOpenWindow = TRUE;
		}

		//
		// Unfortunately, a TocFrame has to keep a cached Summary pointer
		// for the message being displayed in the preview pane.  However,
		// this Xfer() method can nuke such Summary objects, leaving
		// the TocFrame with an invalid pointer.  So, before we get around
		// to nuking the Summary object below, we need to inform the frame
		// that the Summary is about to go away.
		//
		InvalidateCachedPreviewSummary(Sum, true);

		FoundSelected = TRUE;
		if (Purging)
		{
			bool			bCreatedDoc;
			CMessageDoc *	pMsgDoc = Sum->GetMessageDoc(&bCreatedDoc);
			
			if (pMsgDoc)
			{
				// Get rid of attachment if switch is on and it is still in the
				// auto-receive attachment directory
				if (Sum->IsComp())
				{
					CCompMessageDoc *	pCompMsgDoc = reinterpret_cast<CCompMessageDoc *>(pMsgDoc);
					
					if (DeleteMAPI && Sum->IsMAPI())
					{
						pCompMsgDoc->GetText();
						CString AttachStr(pCompMsgDoc->GetHeaderLine(HEADER_ATTACHMENTS));
						char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
						
						while (Attach && *Attach)
						{
							char *t = strchr(Attach, ';');
							if (t)
							{
								*t++ = 0;
								if (*t == ' ')
									t++;
							}
							if (!strnicmp(AttachDir, Attach, ADLen))
								::FileRemoveMT(Attach);
							Attach = t;
						}
					}
					if (DeleteAutoAttached && Sum->IsAutoAttached())
					{
						pCompMsgDoc->GetText();
						CString AttachStr(pCompMsgDoc->GetHeaderLine(HEADER_ATTACHMENTS));
						
						char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
						
						while (Attach && *Attach)
						{
							char *t = strchr(Attach, ';');
							if (t)
							{
								*t++ = 0;
								if (*t == ' ')
									t++;
							}
							
							::FileRemoveMT(Attach);
							
							Attach = t;
						}
					}
				}
				else if (Tidy && Sum->HasAttachment())
				{
					for (char* t = pMsgDoc->GetText(); t; t = strchr(t, '\n'))
					{
						t++;

						if ( !strncmp(ACStr, t, ACLen) )
						{
							CString	szAttachmentPathname;
							short	tNext;

							// get the full path name to the attachment
							tNext = StripAttachmentName( t + ACLen, szAttachmentPathname, AttachDir );

							if( tNext )
							{
								// See if it is a mbx file, we may have generated a toc ourselves
								char * lpszExt = strrchr(szAttachmentPathname, '.');

								if(lpszExt)
								{
									if(stricmp(lpszExt, CRString(IDS_MAILBOX_EXTENSION)) == 0)
									{
										char szToc[_MAX_PATH];

										szToc[0] = 0;

										strcpy(szToc, szAttachmentPathname);
										lpszExt = strrchr(szToc, '.');
										if(lpszExt)
										{
											*lpszExt = 0;
											// add the toc extension
											strcat(szToc, CRString(IDS_TOC_EXTENSION));

											// see if the file exists then delete it
											CFileStatus cfStatus;

											if(CFile::GetStatus(szToc, cfStatus))
											{
												// delete the toc of the mailbox, mbox to be
												// removed below
												::FileRemoveMT( szToc );
											}
										}
									}
								}

								if ( FindAttachment(szAttachmentPathname) )
								{
									// Attempt to delete the current attachment by moving it to the recycle bin.
									// If that fails then just delete it permanently.
									if ( !DoFileCommand(szAttachmentPathname, ID_DELETE_ATTACHMENT, true, FOF_NOCONFIRMATION | FOF_NOERRORUI) )
										::FileRemoveMT( szAttachmentPathname );
								}
							}
							
							// set the pointer to the first character after this attachment 
							t += tNext + ACLen;
						}
					}
				}

				// get rid of embedded objects
				char* buf = pMsgDoc->GetFullMessage(); // gots ta get da URIMap
				CString Paths;
				pMsgDoc->m_QCMessage.GetEmbeddedObjectPaths( Paths );
				DeleteEmbeddedObjects( Paths );
				delete [] buf;

				if (bCreatedDoc)
					Sum->NukeMessageDocIfUnused(pMsgDoc);
			}

			if ( ServerDelete )
			{
				CLMOSRecord LMOSRecord(Sum);
				CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
				if (pMsgRecord && pMsgRecord->GetDeleteFlag() == LMOS_DONOT_DELETE/*1*/)
				{
					pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE/*0*/);
					LMOSRecord.WriteLMOS();
				}
			}

			// If this isn't an IMAP TOC get rid of the summary now.
			if (!IsImapToc())
			{
				RemoveSum(pos, i);
			}
		}
		else
		{
			if (!ToOut)
			{
				long lMStart = 0;
				TargetMBox.Tell(&lMStart);
				ASSERT(lMStart >= 0);
				if (FAILED(SrcMBox.JJBlockMove(Sum->m_Offset, Sum->m_Length, &TargetMBox)))
					break;
				CopySum = DEBUG_NEW CSummary;
				CopySum->Copy(Sum);
				// If transferring from an IMAP mailbox to a local mailbox, don't copy the IMAP
				// deleted flag.  Currently all other flags make sense in a POP mailbox, but
				// local mailboxes have no notion of a summary being deleted.
				if (IsImapToc() && !TargetToc->IsImapToc())
				{
					CopySum->m_Imflags &= ~IMFLAGS_DELETED;
				}
				TargetToc->AddSum(CopySum);
				CopySum->SetOffset(lMStart);
				if (m_Type == MBT_OUT && CopySum->m_State != MS_SENT)
					CopySum->SetState(MS_UNSENT);
				sumlist.AddTail(Sum);
			}
			else
			{
				CCompMessageDoc* comp = Sum->SendAgain(FALSE);
				if (!comp)
					continue;
				comp->m_Sum->SetLabel(Sum->GetLabel());
				if ( FAILED(comp->Write(&TargetMBox)) )
					break;
				if (Sum->m_State == MS_SENT)
				{
					comp->m_Sum->SetState(MS_SENT);
					comp->m_Sum->SetDate(Sum->m_Seconds);
					comp->m_Sum->m_Seconds = Sum->m_Seconds;
				}
				CopySum = comp->m_Sum;
				comp->OnCloseDocument();
			}
			if (!Copy)
			{
				if (!IsImapToc() && !TargetToc->IsImapToc())
				{
					// Remove summary from source TOC, but don't delete it
					// since it will be added to the undo list.
					SubtractSum(pos, i);
				}

				if (FromOut && Sum->IsQueued())
					TransferredQueued = TRUE;

				if (!IsImapToc() && !TargetToc->IsImapToc())
				{
					// Add to the UndoFrom list of this mailbox
					m_UndoXferFrom.AddTail(Sum);

					// Set the junk action flag to its default value.
					Sum->SetLastXferWasJunkAction(false);

					// Add to the UndoTo list of the target mailbox
					TargetToc->m_UndoXferTo.AddTail(CopySum);
				}
			}
		}
		if (SingleSum)
			break;

		// We've removed a summary, so the index of the next summary is
		// now the same as the index of what this summary was, but only
		// if this isn't an IMAP TOC.
		if (!Copy && !bGoingBackward && !IsImapToc())
			i--;	
	}

	if (View)
	{
		if (!Copy && HighlightIndex < 0)
		{
			// If there isn't a selected summary, select the next one
			if (!SingleSum && bGoingBackward)
				i++;
			if (View->GetSelCount() == 0)
				HighlightIndex = i;
		}
			
		if (HighlightIndex >= 0)
		{
			if (HighlightIndex >= NumSums())
				HighlightIndex = NumSums() - 1;
			View->m_SumListBox.SetSel(HighlightIndex);
		}

		// Set redraw back to TRUE
		if (RedisplayTOCs && View && !Copy)
		{
			View->m_SumListBox.SetRedraw(TRUE);
		}
	}
	
	if (RedisplayTOCs && !Purging && TargetView)
	{
		TargetView->m_SumListBox.SetRedraw(TRUE);
		TargetView->m_SumListBox.UpdateWindow();
	}

	if (!Purging)
	{
		TargetMBox.Close();
		SrcMBox.Close();
		if (SingleSum && !RedisplayTOCs)
		{
			// If we get here, it means that we're doing filtering.  To optimize the performance
			// of filtering, only the new summary will be written to disk (not the whole TOC).
			// Later the whole TOC be written out (since CTocDoc::WriteSum() doesn't clear the
			// modified flag), which will correctly update the TOC header.
			ASSERT(CopySum != NULL);
			TargetToc->WriteSum(CopySum);
		}
		else
			TargetToc->Write();
	}

	if (!Copy)
	{
		// Only open next window if:
		//  1. Open message window was closed
		//  2. This is only happening on one message
		//  3. There is another message in the mailbox to open
		//  4. The switch enabling this feature is turned on
		//  5. The message to open is unread
		if (ClosedOpenWindow && SingleSum && NextPos && GetIniShort(IDS_INI_EASY_OPEN))
		{
			Sum = m_Sums.GetAt(NextPos);
			if (Sum && Sum->m_State == MS_UNREAD)
				Sum->Display();
		}

		//While filtering, we dont RedisplayTocs and do one summary at a time
		bool bFiltering = SingleSum && !RedisplayTOCs;
		if( !bFiltering)
		{
			// If we get here, it means that we're *not* doing filtering.  To optimize the performance
			// of filtering, the source mailbox (i.e. the In mailbox) TOC won't get written out until
			// after all filtering is done.
			Write();
		}
		if (RedisplayTOCs && View)
			View->m_SumListBox.UpdateWindow();
	}

#ifdef IMAP4
	// If one of the mailboxes is an IMAP mailbox, handle it specially.
	if ( IsImapToc() || (TargetToc && TargetToc->IsImapToc()) && sumlist.GetCount() > 0 )
	{
		if (this == TargetToc)
			{
			//filter action or a silly user
			 return SingleSum;
			}
		CSummary	*pCopySum = ImapXfer(TargetToc, &sumlist, CloseWindow, Copy, RedisplayTOCs);
		if (pCopySum)
		{
			return pCopySum;
		}
	}
#endif

	// If we transferred a queued message out of the the Out mailbox,
	// then we need to see if the there's still any queued messages left
	if (TransferredQueued)
		SetQueueStatus();

	ASSERT(bXferActive);
	bXferActive = FALSE;
	return CopySum;
}

BOOL CTocDoc::ChangeName(const char* NewName, const char* NewFilename)
{
	char TocName[_MAX_PATH + 1];

	ASSERT(MBT_REGULAR == m_Type);		// can only rename "regular" mailboxes
	
	strcpy(TocName, NewFilename);
	::SetFileExtensionMT(TocName, CRString(IDS_TOC_EXTENSION));
	
	// Rename toc file, but only if filenames are differnet1
	if (GetMBFileName().CompareNoCase(NewFilename) && FAILED(::FileRenameMT(GetFileName(), TocName)))
		return (FALSE);
	
	SetFileName(TocName);

	CSearchView::UpdateMailboxNamesInAllSearchResults(GetMBFileName(), NewFilename, NewName);
    SetMBFileName(NewFilename);

    if (Name() != NewName)
    {
		// Name is really m_strTitle
        ReallySetTitle(NewName);
        return (WriteHeader());
    }
    
    return (TRUE);
}

void CTocDoc::OnUpdateSwitchMessage(CCmdUI* pCmdUI)
{
	CTocView* pView;
	
	pView = GetView();
	
	ASSERT_VALID( pView );
	
	if( pView && pView->m_SumListBox.GetCount() )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

BOOL CTocDoc::OnSwitchMessage(UINT Direction)
{
	CTocView* View = GetView();
	
	ASSERT_VALID(View);
	if (View)
	{
		POSITION pos;
		
		// If no summaries selected, use first or last message
		if (View->GetSelCount() <= 0)
			pos = (Direction == ID_NEXT_MESSAGE? m_Sums.GetHeadPosition() : m_Sums.GetTailPosition());
		else
		{
			CSummary	*pSum = NULL;
			pos = m_Sums.GetHeadPosition();
			for (int i = 0; pos; i++)
			{
				if (m_bHideDeletedIMAPMsgs)
				{
					pSum = m_Sums.GetAt(pos);
					if (pSum && pSum->IsIMAPDeleted())
					{
						--i;
					}
				}
				if (View->m_SumListBox.GetSel(i) > 0)
					break;
				m_Sums.GetNext(pos);
			}
		}

		if (pos)
			DoSwitchMessage(Direction, pos);
	}
	
	return (TRUE);
}

void CTocDoc::DoSwitchMessage(UINT Direction, POSITION OpenPos)
{
	CCursor wait;

	ASSERT(OpenPos);
	
	if (OpenPos)
	{
		//
		// Check to see if there is an existing message window.
		//
		CSummary* pOldSummary = NULL;
		CFrameWnd* pOldFrame = NULL;		// non-NULL if existing message window
		{
			POSITION OldPos = OpenPos;
			CSummary	*pSum = NULL;
			if (Direction == ID_NEXT_MESSAGE)
			{
				m_Sums.GetPrev(OldPos);
				if (OldPos)
				{
					pSum = m_Sums.GetAt(OldPos);
				}
				while (m_bHideDeletedIMAPMsgs && OldPos && pSum && pSum->IsIMAPDeleted())
				{
					m_Sums.GetPrev(OldPos);
					if (OldPos)
					{
						pSum = m_Sums.GetAt(OldPos);
					}
				}
			}
			else
			{
				m_Sums.GetNext(OldPos);
				if (OldPos)
				{
					pSum = m_Sums.GetAt(OldPos);
				}
				while (m_bHideDeletedIMAPMsgs && OldPos && pSum && pSum->IsIMAPDeleted())
				{
					m_Sums.GetNext(OldPos);
					if (OldPos)
					{
						pSum = m_Sums.GetAt(OldPos);
					}
				}
			}

			if (OldPos)
			{
				pOldSummary = m_Sums.GetAt(OldPos);
				pOldFrame = pOldSummary->m_FrameWnd;
			}
		}

		CTocView* pTocView = GetView();
		CView* pPreviewView = GetPreviewView();			// NULL if no preview

		if (pOldFrame || (NULL == pPreviewView))
		{
			//
			// Found an existing message window or we're not in
			// preview mode.  In these cases, the NextMsg/PrevMsg
			// command is supposed to open another message window.
			//
			if (pTocView && OpenPos)
			{
				// Open the message and select the summary in the toc window
				CSummary* Sum = m_Sums.GetAt(OpenPos);
				pTocView->SelectAll(FALSE);

				if (Sum)
				{
					if (!m_bHideDeletedIMAPMsgs || !Sum->IsIMAPDeleted())
					{
						// Passing true for the last parameter is equivalent to calling
						// MailboxChangedByHuman, but slightly more efficient.
						pTocView->Select(Sum, TRUE, true);
						Sum->Display();
					}
				}
			}
		}
		else
		{
			//
			// We must be in preview mode.
			//
			ASSERT(pPreviewView != NULL);
			POSITION OldPos;
			POSITION NewPos = OldPos = OpenPos;

			// Get the "next" message appropriate for the direction that we're
			// being told to go so that we can switch the selection.
			if (Direction == ID_NEXT_MESSAGE)
				m_Sums.GetNext(NewPos);
			else
				m_Sums.GetPrev(NewPos);

			if (NewPos)
			{
				CSummary* pNewSummary = m_Sums.GetAt(NewPos);
				CSummary* pLastSummary = NULL;
				
				if (OldPos)
					pLastSummary = m_Sums.GetAt(OldPos);

				if (pTocView)
				{
					pTocView->SelectAll(FALSE);

					if (!m_bHideDeletedIMAPMsgs || !pNewSummary->IsIMAPDeleted())
					{
						// Passing true for the last parameter is equivalent to calling
						// MailboxChangedByHuman, but slightly more efficient.
						pTocView->Select(pNewSummary, TRUE, true);
					}

					// Mark the "previous" message as read if appropriate
					if ( pLastSummary && MS_UNREAD == pLastSummary->m_State )
						pLastSummary->SetState( MS_READ );

					// Set the focus so that it's clear what's going on (i.e. if the
					// focus was in the preview pane setting the focus will help
					// clue the user in to the fact that the list selection has
					// changed). The other reason to do this is to maintain
					// previous behavior. It's not needed to kick off previewing -
					// the preview code already recognized the change up in the
					// Select calls.
					pTocView->SetFocus();
				}
			}
		}

		//
		// Close the old message window, if any.
		//
		if (pOldFrame)
		{
			if ( IsImapToc() )
			{
				// SendMessage causes problems w/IMAP - looks like a timing issue.
				pOldFrame->PostMessage(WM_CLOSE);
			}
			else
			{
				pOldFrame->SendMessage(WM_CLOSE);
			}
		}
	}
}

CTocView* CTocDoc::GetView()
{
	POSITION pos = GetFirstViewPosition();
	
	if (pos)
	{
		CTocView* view = (CTocView*)GetNextView(pos);
		if (view && view->IsKindOf(RUNTIME_CLASS(CTocView)))
			return (view);
		ASSERT(FALSE);
	}
		
	return (NULL);
}

CView* CTocDoc::GetPreviewView()
{
	if (!UsingPreviewPane())
		return NULL;

	POSITION pos = GetFirstViewPosition();
	while (pos)
	{
		CView* pPreviewView = GetNextView(pos);
		if (pPreviewView && !pPreviewView->IsKindOf(RUNTIME_CLASS(CTocView)))
		{
			//
			// FORNOW, Christie won't let me do explicit IsKindOf()
			// tests for the preview view types, so we just have to
			// assume that any view that's not a TocView is a 
			// preview view.
			//
			return pPreviewView;
		}
	}

	ASSERT(0);
	return NULL;
}

BOOL CTocDoc::SortNowIfNeeded()
{
	CTocView *	pView = GetView();
	BOOL		bSorted = FALSE;
	
	if (pView)
	{
		if (m_bNeedsSortingNOW)
		{
			pView->SortNow();
			bSorted = TRUE;
		}
		
		if (m_bNeedsFumlub)
		{
			//	Do Fumlub now after we sorted above (if appropriate) so that the
			//	correct message is selected and it's correctly scrolled into view.
			//	See CTocView::Fumlub for more details about why we need to do Fumlub
			//	after sorting.
			pView->Fumlub();
			m_bNeedsFumlub = false;
		}		
	}

	return bSorted;
}

void CTocDoc::UpdateAllServerStatus()
{
	POSITION pos = TocTemplate->GetFirstDocPosition();
	
	while (pos != NULL)
	{
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc && doc->IsKindOf(RUNTIME_CLASS(CTocDoc)))
			doc->UpdateAllViews(NULL, FW_SERVER, NULL);
		else
			ASSERT(FALSE);
    }

}

void CTocDoc::ReformatDateFields()
{
	POSITION pos = m_Sums.GetHeadPosition();
	while (pos)
	{
		CSummary* Sum = m_Sums.GetNext(pos);
		if (Sum && Sum->IsKindOf(RUNTIME_CLASS(CSummary)))
			Sum->SetDate();
		else
			ASSERT(FALSE);
	}
}

BOOL CTocDoc::Display()
{
	CView* View = GetView();
	QCWorksheet* Frame;

	if (View && (Frame = (QCWorksheet*)View->GetParentFrame()))
	{
		ASSERT_KINDOF(QCWorksheet, Frame);
		if (Frame->IsIconic())
			Frame->MDIRestore();
		else
			Frame->MDIActivate();
		return (TRUE);
	}

	BOOL Result = NewChildFrame(TocTemplate, this)? TRUE : FALSE;
	
	//SK legacy code removed
	//if (Result)
	//	m_bAutoDelete = FALSE;

#ifdef IMAP4
	// If we just created a new frame, go connect to the imap server.
	if (IsImapToc())
	{
		if (m_pImapMailbox)
		{
			m_pImapMailbox->OpenOnDisplay (this);
		}
	}
#endif
	
	return (Result);
}

BOOL CTocDoc::CalculateMood()
{
	if (!m_bNeedMoodCheck)
		return FALSE;

	JJFile MBX;
	CSummary* pSum;
	POSITION pos = m_Sums.GetTailPosition();
// We're going to try out just doing one message and then giving up,
// in order to see whether that lessens the load on the disk
//	const DWORD TimeIsUp = GetTickCount() + timeRemaining;
	while (pos)
	{
		pSum = m_Sums.GetPrev(pos);
		if (pSum->m_nMood <= 0)
		{
			if (MBX.IsOpen() == S_FALSE)
				MBX.Open(GetMBFileName(), O_RDONLY);
//			if (GetTickCount() >= TimeIsUp)
		//We will break the loop only if it does MoodCheck check on at least one message
		// or if reaches the head of toc
			pSum->CalculateMood(&MBX);
			if ((pSum->m_nMood > 0) && (pSum->m_nMood < 5))
				break;
		}
	}
	if (!pos)
		m_bNeedMoodCheck = 0;
	MBX.Close();

	return TRUE;
}

BOOL CTocDoc::DisplayBelowTopMostMDIChild()
{
	//Find the top level window and save it
	CMDIFrameWnd* pMainFrame = (CMDIFrameWnd *) ::AfxGetMainWnd();
	ASSERT_KINDOF(CMDIFrameWnd, pMainFrame);

	BOOL bIsMaximized = FALSE;
	CMDIChildWnd* pActiveMDIChild = pMainFrame->MDIGetActive(&bIsMaximized);

	if(!pActiveMDIChild || (pActiveMDIChild && !pActiveMDIChild->IsWindowVisible()) )
	{
		Display();
		return TRUE;
	}

	
	CMDIChild::m_nCmdShow = SW_HIDE;  //start hiding

	CView* View = GetView();
	QCWorksheet* Frame = NULL;

	if (View && (Frame = (QCWorksheet*)View->GetParentFrame()))
	{
		ASSERT_KINDOF(QCWorksheet, Frame);
		
		if (Frame->IsIconic())
		{
			Frame->ShowWindow(SW_HIDE);
			Frame->ShowWindow(SW_RESTORE);
		}
	}
	else
	{
		NewChildFrame(TocTemplate, this);
		View = GetView();
		if (View)
			Frame = (QCWorksheet*)View->GetParentFrame();
			
	
#ifdef IMAP4
		// If we just created a new frame, go connect to the imap server.
		if (IsImapToc())
		{
			if (m_pImapMailbox)
			{
				m_pImapMailbox->OpenOnDisplay (this);
			}
		}
#endif
	}
	
	if(Frame)
		Frame->SetWindowPos(pActiveMDIChild, 0,0,0,0, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );
	
	CMDIChild::m_nCmdShow = -1;  //reset hiding

	return TRUE;
}



// GetToc
// Returns a pointer to the specified toc by either grabbing it from memory,
// reading it from disk, or building a new one.
//
CTocDoc* GetToc(
			const char *			Filename,
			const char *			Name /*= NULL*/,
			BOOL					HeaderOnly /*= FALSE*/,
			BOOL					OnlyIfLoaded /*= FALSE*/,
			BOOL					bDisplayErrors /*= TRUE*/)
{
	CTocDoc* doc = NULL;
	char TheName[32];
	
	// Save the Name parameter in its own buffer space because sometimes this parameter points to
	// memory that gets overwritten when reading in a Toc because menus get rebuilt
	if (!Name)
		*TheName = 0;
	else
	{
		strncpy(TheName, Name, sizeof(TheName));
		TheName[sizeof(TheName) - 1] = 0;
	}

	// Is the mailbox already in the list?
	POSITION pos = TocTemplate->GetFirstDocPosition();
	while (pos != NULL)
	{
		doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc->GetMBFileName().CompareNoCase(Filename) == 0)
		{
			if ((HeaderOnly) || (OnlyIfLoaded) || (doc->m_ReadSums))
				return (doc);
			break;
		}
		doc = NULL;
    }

	// We didn't find a loaded TOC, so stop now if we were told to do so
	if (OnlyIfLoaded)
		return (NULL);

	if (!::FileExistsMT(Filename))
	{
		if (!gbAutomationCall && bDisplayErrors)
		{
			ErrorDialog(IDS_ERR_MAILBOX_EXIST, Filename);
		}
		return (NULL);
	}

	BOOL Existing = TRUE;
	if (!doc)
	{
		doc = (CTocDoc*)NewChildDocument(TocTemplate);
		if (*TheName)
			doc->Name() = TheName;

		doc->SetMBFileName(Filename);
        Existing = FALSE;
	}

	char TocName[_MAX_PATH + 1];
	strcpy(TocName, Filename);
	::SetFileExtensionMT(TocName, CRString(IDS_TOC_EXTENSION));
	doc->SetFileName(TocName);

#ifdef IMAP4
	// If this is an IMAP mailbox, create the contained CImapMailbox object and
	// go read the IMAP mailbox's info file.
	ACCOUNT_ID AccountID;
	
	AccountID = g_ImapAccountMgr.FindOwningAccount (doc->GetMBFileName());
	if (AccountID)
	{
		// Yep. It's IMAP. Create the contained IMAP mailbox object if
		// it hasn't yet been created.
		if (!doc->m_pImapMailbox)
		{
			if (! doc->ImapInitialize (AccountID))
				goto fail;
		}
	}

	// NOTE: m_Type is set to  here to MBT_IMAP_MAILBOX.
#endif

	if (!::FileExistsMT(TocName))
	{
		if (!doc->Build())
			goto fail;

		//	Notify Search Manager to reindex this mailbox
		SearchManager::Instance()->NotifyReindexMailbox( doc->GetMBFileName() );
	}
	else
	{
		if (!doc->Read(HeaderOnly))
			goto fail;
	}         

// IMAP4 - set the doc's title here to contain the personality's name as well:
//
	if (doc && doc->IsImapToc() && doc->m_pImapMailbox )
	{
		doc->m_pImapMailbox->SetFrameName (doc);
	}

	// This prevents corrupt Toc names from spreading
	if (*TheName)
		doc->Name() = TheName;

	return (doc);

fail:
	if (!Existing && doc)
	{
		// We want this toc to get removed from memory since the toc didn't
		// get read in properly, so we set the AutoDelete flag to true.
		doc->m_bAutoDelete = TRUE;
		doc->OnCloseDocument();
	}
	return (NULL);
}


CTocDoc* GetInToc(BOOL HeaderOnly /*= FALSE*/)
{
	return GetToc(	EudoraDir + CRString( IDS_IN_MBOX_FILENAME ) + CRString( IDS_MAILBOX_EXTENSION ), 
					CRString( IDS_IN_MBOX_NAME ), 
					HeaderOnly );
}

CTocDoc* GetOutToc()
{
	return GetToc(	EudoraDir + CRString( IDS_OUT_MBOX_FILENAME ) + CRString( IDS_MAILBOX_EXTENSION ),
					CRString( IDS_OUT_MBOX_NAME ) );
}

CTocDoc* GetTrashToc()
{
	return GetToc(	EudoraDir + CRString( IDS_TRASH_MBOX_FILENAME ) + CRString( IDS_MAILBOX_EXTENSION ),
					CRString( IDS_TRASH_MBOX_NAME ) );
}

CTocDoc* GetJunkToc()
{
	return GetToc(	EudoraDir + CRString( IDS_JUNK_MBOX_FILENAME ) + CRString( IDS_MAILBOX_EXTENSION ),
					CRString( IDS_JUNK_MBOX_NAME ) );
}


// DisplayToc
// Load up a Toc in to memory, and then display it
BOOL DisplayToc(const char* Filename, const char* Name)
{
	CTocDoc* TocDoc = GetToc(Filename, Name);
	
	return (TocDoc? TocDoc->Display() : FALSE);
}


// EmtpyTrash
//
// Get rid of messages in the Trash mailbox
//
BOOL EmptyTrash()
{
	CTocDoc* TrashToc = GetTrashToc();

	if (!TrashToc)
	{
		ASSERT(FALSE);
		return (FALSE);
	}

	if (TrashToc->NumSums() == 0)
		return (TRUE);

	if (GetIniShort(IDS_INI_WARN_EMPTY_TRASH))
	{
		if (!gbAutomationCall)
		{
			if (WarnYesNoDialog(IDS_INI_WARN_EMPTY_TRASH, IDS_WARN_EMPTY_TRASH, TrashToc->NumSums()) != IDOK)
			{
				return (FALSE);
			}
		}
	}

	CTocView* View = TrashToc->GetView();
	if (View)
		View->m_SumListBox.SetRedraw(FALSE);

	// Some optimizations here for purging
	BOOL ServerDelete = GetIniShort(IDS_INI_SERVER_DELETE);
	BOOL Tidy = GetIniShort(IDS_INI_TIDY_ATTACH);
	BOOL DeleteMAPI = (GetIniShort(IDS_INI_MAPI_DELETE_TRASH) != FALSE);
	BOOL DeleteAutoAttached = (GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_TRASH) != FALSE);
	char AttachDir[_MAX_PATH + 1];
	char ACStr[64];
	int ACLen = 0;
	int ADLen = 0;
	
	if (Tidy || DeleteMAPI)
	{
		GetIniString(IDS_ATTACH_CONVERTED, ACStr, sizeof(ACStr));
		ACLen = strlen(ACStr);
		GetIniString(IDS_INI_AUTO_RECEIVE_DIR, AttachDir, sizeof(AttachDir));
		if (!*AttachDir)
		{
			wsprintf(AttachDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_ATTACH_FOLDER));
			//Tidy = FALSE;
		}
		ADLen = strlen(AttachDir);
	}

	CSumList &	listTrashSums = TrashToc->GetSumList();
	
	if (listTrashSums.IsEmpty() == FALSE)
	{
		Progress(0, CRString(IDS_EMPTYING_TRASH), listTrashSums.GetCount() - 1);

#ifdef COMMERCIAL
		CString CurPersona = g_Personalities.GetCurrent();
#endif // COMMERCIAL
		
		POSITION pos;
		for (int i = 0; pos = listTrashSums.GetHeadPosition(); i++)
		{
			CSummary* Sum = listTrashSums.GetAt(pos);

			CMessageDoc* doc;
			if (Tidy || DeleteMAPI || DeleteAutoAttached)
				doc = Sum->GetMessageDoc();
			else
				doc = Sum->FindMessageDoc();
		
			// don't want to crash on an 'Empty Trash'
			if (doc)
			{
				if (DeleteMAPI && Sum->IsMAPI() && Sum->IsComp())
				{
					CCompMessageDoc* MsgDoc = (CCompMessageDoc*)doc;
					MsgDoc->GetText();
					CString AttachStr(MsgDoc->GetHeaderLine(HEADER_ATTACHMENTS));
					char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
		
					while (Attach && *Attach)
					{
						char *t = strchr(Attach, ';');
						if (t)
						{
							*t++ = 0;
							if (*t == ' ')
								t++;
						}
						if (!strnicmp(AttachDir, Attach, ADLen))
							::FileRemoveMT(Attach);
						Attach = t;
					}
				}
				else if (DeleteAutoAttached && Sum->IsAutoAttached() && Sum->IsComp())
				{
					CCompMessageDoc* MsgDoc = (CCompMessageDoc*)doc;
					MsgDoc->GetText();
					CString AttachStr(MsgDoc->GetHeaderLine(HEADER_ATTACHMENTS));
					char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
		
					while (Attach && *Attach)
					{
						char *t = strchr(Attach, ';');
						if (t)
						{
							*t++ = 0;
							if (*t == ' ')
								t++;
						}
						::FileRemoveMT(Attach);
						Attach = t;
					}
				}
				else if (Tidy && !Sum->IsComp() && Sum->HasAttachment())
				{
					for (char* t = doc->GetText(); t; t = strchr(t, '\n'))
					{
						t++;
						if ( !strncmp(ACStr, t, ACLen) )
						{
							CString	szAttachmentPathname;
							short	tNext;
							
							// get the full path name to the attachment
							tNext = StripAttachmentName( t + ACLen, szAttachmentPathname, AttachDir );

							if ( tNext && FindAttachment(szAttachmentPathname) )
							{
								// Attempt to delete the current attachment by moving it to the recycle bin.
								// If that fails then just delete it permanently.
								if ( !DoFileCommand(szAttachmentPathname, ID_DELETE_ATTACHMENT, true, FOF_NOCONFIRMATION | FOF_NOERRORUI) )
									::FileRemoveMT( szAttachmentPathname );
							}
							
							// set the pointer to the first character after this attachment 
							t += tNext + ACLen;
						}

					}
				}

				// get rid of embedded objects
				char* buf = doc->GetFullMessage();		// gots ta get da URIMap
				CString Paths;
				doc->m_QCMessage.GetEmbeddedObjectPaths( Paths );
				DeleteEmbeddedObjects( Paths );
				delete [] buf;

				// Get rid of document (will close window if open)
				doc->m_bAutoDelete = TRUE;
				doc->OnCloseDocument();
			}
		
			if ( ServerDelete )
			{
				CLMOSRecord LMOSRecord(Sum);
				CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
				if (pMsgRecord && pMsgRecord->GetDeleteFlag() == LMOS_DONOT_DELETE/*1*/)
				{
					pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE/*0*/);
					LMOSRecord.WriteLMOS();
				}
			}
				
			TrashToc->RemoveSum(pos, 0);
		
			ProgressAdd(1);
			if (EscapePressed())
				break;
		}

#ifdef COMMERCIAL
		// restore the current personality
		g_Personalities.SetCurrent( CurPersona );
#endif // COMMERCIAL
	}
	
	if ( listTrashSums.IsEmpty() )
	{
		// Get rid of Trash.MBX file
		JJFile TmpFile;
		if (SUCCEEDED(TmpFile.Open(TrashToc->GetMBFileName(), O_RDWR | O_CREAT | O_TRUNC)))
			TmpFile.Close();		// successfully nuked Trash MBX
		
		TrashToc->ClearDelSpace();
	}

	TrashToc->ClearUndoFrom();
	TrashToc->ClearUndoTo();
	
	TrashToc->Write();

	if (View)
		View->m_SumListBox.SetRedraw(TRUE);

	CloseProgress();

	return (TRUE);
}


//
//	TrimJunk()
//
//	Removes messages from the junk that are older than the user-specified age.
//
//	Parameters:
//		bUserRequested[in] - If true then user requested trim so do it no matter when the
//			last trim was done.
//
//	STILL TO BE DONE: option to nuke.
//
bool TrimJunk(bool bUserRequested, bool bOnMailCheck)
{
	// Only proceed if not light and user wants to trim junk.
	if (!UsingFullFeatureSet() ||
		!GetIniShort(IDS_INI_JUNK_AGEOFF))
	{
		return true;
	}

	// Get the junk TOC.
	CTocDoc		*pJunkToc = GetJunkToc();
	if (!pJunkToc)
	{
		ASSERT(0);
		return false;
	}

	// Get the current time.
	CTime			 now = CTime::GetCurrentTime();
	time_t			 tSeconds = static_cast<time_t>( now.GetTime() );
	short			 sCurrHours = (short)(tSeconds / 3600);

	// If this is an automatic check (not user requested) see if enough
	// time has elapsed since last check.
	if (!bUserRequested)
	{
		// Check the last time we aged junk off.
		short			 sLastAgeOff = GetIniShort(IDS_INI_LAST_JUNK_AGEOFF);
		if (sLastAgeOff == 0)
		{
			// If this is the first time we are checking don't do it now.
			// Note the time and bail so that we start counting now.
			SetIniShort(IDS_INI_LAST_JUNK_AGEOFF, sCurrHours);
			return true;
		}
		
		// Bail if not enough time has elapsed since the last age-off check.
		short			 sHoursSinceLastAgeOff = (short)(sCurrHours - sLastAgeOff);
		short			 sDaysSinceLastAgeOff = (short)(sHoursSinceLastAgeOff / 24);
		short			 sDaysUntilAgeOff = GetIniShort(IDS_INI_DAYS_TIL_JUNK_AGEOFF);
		short			 sAgeOffDays = GetIniShort(IDS_INI_JUNK_AGEOFF_DAYS);
		// If this is an age-off attempt on a mail check, only do it if
		// age-off has not happened for three times the specified age-off time.
		// but if the age is only 1, the user has a bee in their ... bonnet and we should age sooner
		if (bOnMailCheck && sAgeOffDays > 1)
		{
			sDaysUntilAgeOff *= 3;
		}
		// Allow 8 hours of slop so that any day that is at least 16 hours old counts as a day.
		if ((sHoursSinceLastAgeOff % 24) >= 16)
		{
			++sDaysSinceLastAgeOff;
		}
		// Bail if not enough time has elapsed since the last age-off check.
		if (sDaysSinceLastAgeOff < sDaysUntilAgeOff)
		{
			return true;
		}
	}

	// If we go through with the check (even if there is nothing to trim)
	// record the current time in hours.
	SetIniShort(IDS_INI_LAST_JUNK_AGEOFF, sCurrHours);

	// Get the mailbox to trim to.
	CTocDoc				*pTocDest = NULL;
	QCMailboxCommand	*pCommand = NULL;
	CRString			 strJunkName(IDS_JUNK_MBOX_NAME);
	CString				 strTrimToName;
	GetIniString(IDS_INI_JUNK_TRIM_MAILBOX, strTrimToName);
	if (strTrimToName.IsEmpty() || (strTrimToName.CompareNoCase(strJunkName) == 0))
	{
		strTrimToName = GetIniString(IDS_TRASH_MBOX_NAME);
	}
	// A mailbox name of "-" means nuke, so don't bother finding a mailbox in that case.
	if (strTrimToName.Compare("-") != 0)
	{
		pCommand = g_theMailboxDirector.FindByNamedPath(strTrimToName);
		if (pCommand)
		{
			pTocDest = GetToc(pCommand->GetPathname());

			// Bail if we can't get the user-specified mailbox to trim to.
			// Perhaps we should warn the user that we can't trim with their current settings.
			if (!pTocDest)
			{
				return false;
			}
		}
	}

	CWaitCursor		 wait_cursor;

	pJunkToc->TrimJunk(pTocDest, strTrimToName);

	TrimImapJunk(strTrimToName);

	return true;
}


//
//	TrimImapJunk()
//
//	Removes from IMAP junk mailboxes messages that are older than the user-specified age.
//
//	Parameters:
//		strTrimToName [in] - Name of mailbox to trim junk to.
//
//	Note:
//		This should only be called from TrimJunk() so we assume we have already checked
//		for the appropriate time since last age off.
//
bool TrimImapJunk(CString &strTrimToName)
{
	LPSTR					 szNameList;
	CString					 strPersona;
	CImapAccount			*pAccount = NULL;
	QCImapMailboxCommand	*pCommand = NULL;
	CTocDoc					*pTocDoc = NULL;
	QCImapMailboxCommand	*pCommandDest = NULL;
	CTocDoc					*pTocDest = NULL;
	CString					 strJunkName;

	// Iterate over IMAP personalities looking for ones that have junk mailboxes.
	for (szNameList = g_Personalities.List(); szNameList && *szNameList; szNameList += strlen(szNameList) + 1)
	{
		strPersona = szNameList;
		pAccount = g_ImapAccountMgr.FindAccount(g_Personalities.GetHash(strPersona));
		if (pAccount)
		{
			// Find this account's Junk mailbox (skip this account if there is no Junk mailbox).
			pCommand = pAccount->GetJunkMailbox(strJunkName, false/*bCreate*/);
			if (pCommand)
			{
				pTocDoc = GetToc(pCommand->GetPathname(), NULL, FALSE/*HeaderOnly*/, FALSE/*OnlyIfLoaded*/);
				if (pTocDoc)
				{
					CString			 strActualTrimToName = strTrimToName;

					// Find the mailbox to trim to.
					// A mailbox name of "-" means nuke, so don't bother finding a mailbox in that case.
					if (strTrimToName.Compare("-") != 0)
					{
						// Look for the mailbox at the root of our local mailbox tree.
						pCommandDest = pAccount->FindMailboxAtRoot(strActualTrimToName);
						if (pCommandDest)
						{
							// We found an existing mailbox, get its TOC.
							pTocDest = GetToc(pCommandDest->GetPathname(), NULL/*Name*/, TRUE/*HeaderOnly*/, TRUE/*OnlyIfLoaded*/);
						}
					}

					// At this point we may not have a destination mailbox because we haven't
					// yet tried to create it but go ahead and start the trimming process.
					// If it turns out that there is something to trim and we don't have
					// a destination mailbox we will create it then.
					if (!pTocDoc->TrimJunk(pTocDest, strActualTrimToName, pCommandDest))
					{
						// User said don't trim IMAP junk.
						return false;
					}
				}
			}
		}
	}

	return true;
}


// Remove the TOCs that are currently in memory
void TocCleanup()
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	POSITION pos = TocTemplate->GetFirstDocPosition();
	
	while (pos)
	{
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc)
		{
			// write it to disk if need be
			if ( doc->IsModified() )
			{
				// Don't bother with notification of unread status changing
				// since we're shutting the whole thing down very soon
				doc->Write( FALSE, TRUE );
			}

			doc->m_bAutoDelete = TRUE;
			doc->OnCloseDocument();
		}
	}
}


bool CloseTOCIfNotInUse(CTocDoc * pTocDoc)
{
	if (!pTocDoc)
		return false;
	
	// write it to disk if need be
	if ( pTocDoc->IsModified() )
		pTocDoc->Write();

	bool	bCloseToc = !pTocDoc->GetView() && pTocDoc->m_bAutoDelete &&
						!pTocDoc->InvolvedInUndo();
	
	if (bCloseToc)
	{
		//there is no view

		//is one of Sums has a view?
		CSumList &	listSums = pTocDoc->GetSumList();
		POSITION	sum_pos = listSums.GetHeadPosition();

		while (sum_pos)
		{
			CSummary *	pSum = listSums.GetNext(sum_pos);

			if ( pSum && pSum->IsKindOf(RUNTIME_CLASS(CSummary)) )
			{
				if(pSum->FindMessageDoc())
				{
					bCloseToc = false;
					break;
				}
			}
		}

		if (bCloseToc)
			pTocDoc->OnCloseDocument();	
	}

	return bCloseToc;
}


	// Remove the TOCs that are currently in memory
void ReclaimTOCs()
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	POSITION pos = TocTemplate->GetFirstDocPosition();
	
	while (pos)
	{
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);

		if ( CloseTOCIfNotInUse(doc) )
			break;	// do one at a time

	}//while
}


////////////////////////////////////////////////////////////////////////
// UpdatePathnamesInOpenTocs [extern]
//
// Walk through the list of open TOCs, updating the file pathnames from
// old to new.  This covers the case where you rename a mailbox *folder*.
// Use CTocDoc::ChangeName() for handling the rename and move cases
// for individual mailboxes.
////////////////////////////////////////////////////////////////////////
void UpdatePathnamesInOpenTocs(const CString& oldPathname, 
								const CString& newPathname)
{
	POSITION pos = TocTemplate->GetFirstDocPosition();
	while (pos != NULL)
	{
		CTocDoc* p_tocdoc = (CTocDoc*) TocTemplate->GetNextDoc(pos);
		ASSERT(p_tocdoc != NULL);
		ASSERT(p_tocdoc->IsKindOf(RUNTIME_CLASS(CTocDoc)));

		// Fetch existing pathname
		CString mbfilename(p_tocdoc->GetMBFileName());
		ASSERT(! mbfilename.IsEmpty());

		//
		// Be sure to just compare to the "folder part" of the 
		// existing paths.
		//
		if (oldPathname.CompareNoCase(mbfilename.Left(oldPathname.GetLength())) == 0)
		{
			CString		strMBFilename = newPathname + "\\" + mbfilename.Right(mbfilename.GetLength() - oldPathname.GetLength() - 1);
			p_tocdoc->SetMBFileName(strMBFilename);

			const CString &		strOldFileName = p_tocdoc->GetFileName();
			CString				strNewFileName = newPathname + "\\" + strOldFileName.Right(strOldFileName.GetLength() - oldPathname.GetLength() - 1);
				
			p_tocdoc->SetFileName(strNewFileName);
		}
	}

	CSearchView::UpdatePathnamesInAllSearchResults(oldPathname, newPathname);
}



///////////////////////////////////////////////////////////////////////////////
//
//	short	StripAttachmentName( 
//	LPCSTR		szSrc, 
//	CString&	szDest,
//	LPCSTR		szAttachmentDir );
//
//	Gets a pathname from a string.  The source pathname may or may not be quoted and
//	may span more than 1 line.  The destination will not contain quotes or newlines.
//
//	Input:
//		LPCSTR		szSrc			-	the source string
//		CString&	szDest			-	the destination CString
//		LPCSTR		szAttachmentDir -	the attachment directory
//
//	Return Value:
//		On success, the offset to the character following the attachment pathname is 
//		returned.  Otherwise, 0 is returned.
//
short	StripAttachmentName( 
LPCSTR		szSrc,
CString&	szDest,
LPCSTR		szAttachmentDir )
{
	short	retIndex;
	LPCSTR	pSrc;
	BOOL	bQuoted;
	int		iSpan;

	retIndex = 0;
	szDest = "";
	
	// skip spaces
	while( *szSrc == ' ' )
	{
		szSrc ++;
		retIndex ++;
	}
	
	// see if the name is in quotes
	if ( ( bQuoted = ( *szSrc == '"' ) ) == TRUE )
	{
		// skip the starting quote
		szSrc ++;
		retIndex ++;
	}

	// clear the span length and initialize the span pointer
	iSpan = 0;
	pSrc = szSrc;
	
	
	//
	//	there are 3 terminiation cases
	//	
	//	1) null char
	//	2) the string is not quoted and the end of the line is reached.
	//	3) the string is quoted and the second quote has been found.
	//
	while(	(*pSrc != '\0' ) && 
			(	( !bQuoted && ( *pSrc != '\r' ) && ( *pSrc != '\n' ) ) ||
				( bQuoted && ( *pSrc != '"' ) ) ) )
	{
		if ( *pSrc < ' ' )
		{
			// if there are characters in the current span, add them to the destination string
			
			if ( iSpan )
			{
				CString szSpan( szSrc, iSpan );

				// add the span CString
				szDest += szSpan;

				// clear the span length
				iSpan = 0;
			}
			
			// set szSrc to point to the beginning of the next span
			szSrc = pSrc + 1;			
		}
		else
		{
			// increment the span length counter
			iSpan ++;
		}
		
		// increment the current character pointer
		pSrc ++;

		// increment the index
		retIndex ++;
	}

	// if there are characters in the current span, add them to the destination string	
	if ( iSpan )
	{
		CString szSpan( szSrc, iSpan );

		// add the span CString
		szDest += szSpan;
	}

	// skip the closing quote
	if ( bQuoted && ( *pSrc == '"' ) ) 
	{
		retIndex ++;
	}

	// see if the string starts with the attachment directory
	if (strlen(szAttachmentDir)>0 && strnicmp( szDest, szAttachmentDir, strlen( szAttachmentDir ) ) )
	{
		// nope
		szDest = "";
		retIndex = 0;
	}

	return retIndex;
}	

BOOL CTocDoc::GetProductUsedFlag( int flag )	// 0 .. 47
{
	int		byte, bit;
	BYTE	mask = 0x01;

	byte = flag / 8;
	bit = flag % 8;

	for ( int i = 0; i < bit; i++ )
		mask <<= 1;

	BYTE value = (BYTE)(m_UsedVersion[ byte ] & mask );

	return value;
}

void CTocDoc::SetProductUsedFlag( int flag )	// 0 .. 47
{
	int		byte, bit;
	BYTE	mask = 0x01;

	byte = flag / 8;
	bit = flag % 8;

	for ( int i = 0; i < bit; i++ )
		mask <<= 1;
	
	m_UsedVersion[ byte ] |= mask;
	
	SetModifiedFlag( TRUE );
}


////////////////////////////////////////////////////////////////////////
// UsingPreviewPane [public]
//
// Returns wheher or not there's a preview pane in this mailbox
////////////////////////////////////////////////////////////////////////
BOOL CTocDoc::UsingPreviewPane()
{
	if (!GetIniShort(IDS_INI_MAILBOX_PREVIEW_PANE))
		return FALSE;

	// Special case for turning off preview pane for this mailbox
	if ((unsigned short) m_SplitterPos == (unsigned short)MINSHORT)
		return FALSE;

#ifdef IMAP4
	if (IsImapToc() && !GetIniShort(IDS_INI_IMAP_PREVIEW_PANE))
		return FALSE;
#endif

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// InvalidateCachedPreviewSummary [public]
//
// Notifies the TocFrame, if any, that a Summary object that it might
// be caching is being invalidated. Also tells it whether or not
// to update the preview display if the summary was in fact cached.
////////////////////////////////////////////////////////////////////////
void CTocDoc::InvalidateCachedPreviewSummary(CSummary * in_pInvalidSummary, bool in_bRefreshPreviewDisplay)
{
	ASSERT( !in_pInvalidSummary || in_pInvalidSummary->IsKindOf(RUNTIME_CLASS(CSummary)) );

	CTocView *		pTocView = GetView();
	if (pTocView)
	{
		CTocFrame *		pTocFrame = reinterpret_cast<CTocFrame *>( pTocView->GetParentFrame() );
		
		if (pTocFrame)
			pTocFrame->InvalidateCachedPreviewSummary(in_pInvalidSummary, in_bRefreshPreviewDisplay);
	}
}


////////////////////////////////////////////////////////////////////////
// MailboxChangedByHuman [public]
//
// This is called when some mailbox operation occurs due to explicit
// user action (delete, next/prev msg, transfer, etc).  We post a 
// fake LBN_SELCHANGE to the TOC list so that the auto-mark-as-read
// logic gets invoked in the TocFrame.  Got that?
////////////////////////////////////////////////////////////////////////
void CTocDoc::MailboxChangedByHuman()
{
	CTocView* pTocView = GetView();
	if (pTocView)
	{
		ASSERT_KINDOF(CTocView, pTocView);
		// Needs to be posted because the LBN_SELCHANGE normally
		// occurs *after* the preview has started.
		pTocView->m_SumListBox.PostMessage(WM_COMMAND, 
											MAKEWPARAM(pTocView->m_SumListBox.GetDlgCtrlID(), LBN_SELCHANGE), 
											LPARAM(pTocView->m_SumListBox.GetSafeHwnd()));
	}
}

//
//	CTocDoc::TrimJunk()
//
//	If this is a junk mailbox, look for old junk and trim it.
//
//	Parameters:
//		pTocDest [in] - TOC to trim to.  For IMAP this can be NULL.
//		strTrimToName [in] - Name of mailbox to trim to (needed only for IMAP).
//		pCommandDest [in] - Command object for mailbox to trim to (needed only for IMAP).
//
bool CTocDoc::TrimJunk(CTocDoc *pTocDest,
					   CString &strTrimToName,
					   QCMailboxCommand *pCommandDest)
{
	static bool			 bAskedAboutIMAP = false;

	// Bail if there is nothing to check.
	if (NumSums() == 0)
	{
		return true;
	}

	// See if there are any messages old enough to trim.
	CTime			 now = CTime::GetCurrentTime();
	time_t			 tSeconds = static_cast<time_t>( now.GetTime() );
	short			 sAgeInDays = 0;
	short			 sAgeOffDays = GetIniShort(IDS_INI_JUNK_AGEOFF_DAYS);
	unsigned long	 lTimeStamp = 0;
	unsigned char	 ucMinScore = (unsigned char)GetIniShort(IDS_INI_JUNK_MIN_AGEOFF_SCORE);
	int				 iSumsToTrim = 0;
	CSummary		*pSum = NULL;
	POSITION		 pos = m_Sums.GetHeadPosition();
	while (pos)
	{
		pSum = m_Sums.GetAt(pos);
		if (pSum)
		{
			// If the message age meets or exceeds the user specified maximum age
			// for junk and the message score meets or exceeds the minimum score
			// to junk, then mark this summary as old junk.
			lTimeStamp = pSum->m_lArrivalSeconds;
			// If this is a newer version where arrival time is supported, use that
			// time for the age of the message, otherwise use the message's timestamp.
			if (lTimeStamp == 0)
			{
				lTimeStamp = pSum->m_Seconds;
			}
			sAgeInDays = (short)((tSeconds - lTimeStamp) / 86400);
			if ((sAgeInDays >= sAgeOffDays) && (pSum->GetJunkScore() >= ucMinScore))
			{
				pSum->SetOldJunk(true);
				++iSumsToTrim;
			}
		}
		m_Sums.GetNext(pos);
	}

	// Bail if we have nothing to do.
	if (iSumsToTrim == 0)
	{
		return true;
	}

	// Warn the user if desired.
	bool		 bCancelled = false;
	if (GetIniShort(IDS_INI_JUNK_WARN_BEFORE_AGEOFF))
	{
		if (IsImapToc())
		{
			// Only ask the user the first time we attempt to age off an IMAP mailbox.
			// The message asks about all IMAP accounts, so saying yes to one is
			// saying yes to all.
			if (!bAskedAboutIMAP)
			{
				bAskedAboutIMAP = true;
				bCancelled = (WarnYesNoDialog(IDS_INI_JUNK_WARN_BEFORE_AGEOFF,
											  IDS_WARN_TRIM_IMAP_JUNK) != IDOK);
			}
		}
		else
		{
			bAskedAboutIMAP = false;
			bCancelled = (WarnYesNoDialog(IDS_INI_JUNK_WARN_BEFORE_AGEOFF,
										  IDS_WARN_TRIM_JUNK,
										  iSumsToTrim,
										  sAgeOffDays) != IDOK);
		}
	}

	// Bail if user cancelled.
	if (bCancelled)
	{
		// User said no: reset the old junk flags and return out.
		pos = m_Sums.GetHeadPosition();
		while (pos)
		{
			pSum = m_Sums.GetAt(pos);
			if (pSum)
			{
				pSum->SetOldJunk(false);
			}
			m_Sums.GetNext(pos);
		}
		return false;
	}

	// Don't update while trimming.
	CTocView	*pView = GetView();
	if (pView)
	{
		pView->m_SumListBox.SetRedraw(FALSE);
	}

	CString			 strUIDList;
	CUidMap			 uidmap;

	// Begin showing progress.
	Progress(0, CRString(IDS_TRIMMING_JUNK), m_Sums.GetCount() - 1);

	// Trim the messages starting from the end.
	// For now just trim to trash.  Eventually trim to user specified mailbox.
	pos = m_Sums.GetTailPosition();
	while (pos && (m_Sums.GetCount() > 0))
	{
		pSum = m_Sums.GetAt(pos);
		if (pSum && pSum->IsOldJunk())
		{
			if (IsImapToc())
			{
				// For IMAP accumulate UIDs in a list for mass transfer or nuke.
				char		 szUID[16];
				sprintf(szUID, "%ld", pSum->GetHash());
				if (!strUIDList.IsEmpty())
				{
					strUIDList += ",";
				}
				strUIDList += szUID;
				uidmap.OrderedInsert(pSum->GetHash(), FALSE, FALSE);
			}
			else
			{
				// If this message is not marked as deleted from the server, mark it as deleted now.
				{
					// Enter new scope here because CLMOSRecord will lock on constructor, unlock on destructor
					CLMOSRecord LMOSRecord(pSum);
					CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
		
					if (pMsgRecord && pMsgRecord->GetDeleteFlag() != LMOS_DELETE_MESSAGE/*0*/)
					{
						pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE/*0*/);
					}
				}

				// Trim the message.
				pSum->SetOldJunk(false);
				if (pTocDest)
				{
					// Trim to the desired mailbox.
					Xfer(pTocDest, pSum, FALSE/*CloseWindow*/, FALSE/*Copy*/, TRUE/*RedisplayTOCs*/);
				}
				else
				{
					// Nuke it.
					RemoveSum(pos, -1);
				}
			}

			// Update progress.  Note that this isn't really adequate for IMAP because
			// we haven't really done the actual work of moving or deleting the messages
			// but the move/delete is one mass operation so we can't really do accurate
			// progress in that case.  There might be a better way, but for now this is it.
			ProgressAdd(1);
			if (EscapePressed())
			{
				break;
			}
		}
		m_Sums.GetPrev(pos);
	}

	if (IsImapToc())
	{
		// Do a mass transfer or delete of the messages specified in the UID list.
		if (m_pImapMailbox)
		{
			if (pCommandDest)
			{
				// There is something to trim but we didn't find an existing destination mailbox:
				// try to create it now.
				CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
				if (pAccount)
				{
					pCommandDest = pAccount->CreateMailboxAtRoot(strTrimToName);
				}
			}

			if (pCommandDest)
			{
				// Transfer and expunge.
				m_pImapMailbox->XferMessagesOnServer(strUIDList,
													 NULL/*dwaNewUIDs*/,
													 strTrimToName/*pDestination*/,
													 FALSE/*Copy*/,
													 FALSE/*bSilent*/);
				m_pImapMailbox->Expunge();

				// Refresh the destination if it is open.
				if (pTocDest && pTocDest->GetView() && pCommandDest)
				{
					BOOL				 bViewNeedsUpdate = FALSE;
					pCommandDest->Execute(CA_IMAP_RESYNC, &bViewNeedsUpdate);
				}
			}
			else
			{
				// Nuke.
				m_pImapMailbox->DeleteMessagesFromServer(strUIDList,
														 TRUE/*Expunge*/,
														 TRUE/*bSilent*/);
			}
			// Remove the summaries from the TOC.
			int			 iHighlightIndex;
			ImapRemoveListedTocs(&uidmap, iHighlightIndex, FALSE/*CloseWindow*/);			
		}
	}

	ClearUndoFrom();
	ClearUndoTo();
	
	// Store the TOCs.
	Write();
	if (pTocDest)
	{
		pTocDest->Write();
	}

	// Resume updating.
	if (pView)
	{
		pView->m_SumListBox.SetRedraw(TRUE);
	}

	CloseProgress();

	return true;
}

//
//	CTocDoc::IsJunk()
//
//	Indicates whether or not this is somebody's junk mailbox (either the system
//	junk mailbox or an IMAP account's junk mailbox).
//
bool CTocDoc::IsJunk()
{
	if (IsImapToc())
	{
		// For an IMAP TOC get the account for this mailbox, find its junk mailbox and
		// see if the names match.
		if (m_pImapMailbox)
		{
			CImapAccount	*pAccount = g_ImapAccountMgr.FindAccount(m_pImapMailbox->GetAccountID());
			if (pAccount)
			{
				CString		 strJunkMBoxPath;
				CString		 strThisMBoxPath;

				// Get the account's junk mailbox path and our path.
				pAccount->GetJunkLocalMailboxPath(strJunkMBoxPath, FALSE/*MakeSureExists*/, TRUE/*bSilent*/);
				strThisMBoxPath = GetPathName();

				// Strip off the trailing toc and mbx and see if everything else matches.
				strJunkMBoxPath = strJunkMBoxPath.Left(strJunkMBoxPath.GetLength() - 3);
				strThisMBoxPath = strThisMBoxPath.Left(strThisMBoxPath.GetLength() - 3);
				if (strJunkMBoxPath.CompareNoCase(strThisMBoxPath) == 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	return (m_Type == MBT_JUNK);
}

//
//	CTocDoc::SetHideDeletedIMAPMsgs()
//
//	Set the flag to indicate whether or not deleted IMAP messages are shown for this mailbox
//	and possibly update the view to reflect the new state.
//
void CTocDoc::SetHideDeletedIMAPMsgs(BOOL bHide, BOOL bUpdateView)
{
	// Set the flag.  This flag can only be set to TRUE if this is an IMAP TOC.
	m_bHideDeletedIMAPMsgs = (bHide && IsImapToc());

	// Mark this doc as modified.
	SetModifiedFlag(TRUE);

	// Update the view if desired.
	if (bUpdateView)
	{
		CTocView	*pTocView = GetView();
		if (pTocView)
		{
			pTocView->ResetContents();
		}
	}
}

void DeleteEmbeddedObjects( const char* EOHeader )
{
	const char* Index = EOHeader;
	const char* End;
	while ( ( End = strchr(Index, '\n') ) != NULL )
	{
		CString Object(Index, End - Index);
		if ( InEmbeddedDir( Object ) )
			::FileRemoveMT(Object);

		Index = ++End;
	}
}

BOOL InEmbeddedDir( const char* path )
{
	// does path appear to be located in our Embedded directory?
	if ( !path || !*path)
		return FALSE;

	if ( ! strchr( path, '\\' ) )
		return TRUE;	// no directories specified - must be in the embedded dir

	CString EmbeddedDir(EudoraDir);
	EmbeddedDir += CRString(IDS_EMBEDDED_FOLDER);

	if ( strnicmp( EmbeddedDir, path, EmbeddedDir.GetLength() ) == 0 )
		return TRUE;

	return FALSE;
}

BOOL CTocDoc::SortedDateDescending()
{
	for (int i = 0; i < NumSortColumns; i++)
		if ( m_SortMethod[i]==BY_REVERSE_DATE ) return TRUE;
		
	return FALSE;
}

