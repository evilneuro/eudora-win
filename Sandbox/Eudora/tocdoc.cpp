// TOCDOC.CPP
//
// Routines for handling the data of mailbox TOCs
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
#include "debug.h"

#include "pop.h"
#include "MsgRecord.h"
#include "HostList.h"
#include "msgutils.h"
#include "address.h"
#include "eumapi.h"
#include "trnslate.h"

#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "Automation.h"

#ifdef COMMERCIAL
#include "persona.h"
#endif // COMMERCIAL

#ifdef IMAP4
	#include "imapfol.h"
	#include "imapactl.h"
#endif

extern QCMailboxDirector	g_theMailboxDirector;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CTocDoc* CTocList::IndexData(int nIndex) const
{
	POSITION pos = FindIndex(nIndex);
	
	return (pos? GetAt(pos) : NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CTocDoc

IMPLEMENT_DYNCREATE(CTocDoc, CDoc)

CTocDoc::CTocDoc()
{
	m_SavedPos.SetRectEmpty();
}

BOOL CTocDoc::OnNewDocument()
{
	int i;

	if (!CDoc::OnNewDocument())
		return (FALSE);
	
	m_ReadSums = FALSE;

	Name().Empty();
	Filename().Empty();
	MBFilename().Empty();
	m_Version = TocVersion;
	m_NeedsCompact = FALSE;
	m_UnreadStatus = US_UNKNOWN;
	for (i = 0; i < NumFields; i++)
		m_FieldWidth[i] = -1;
	m_bGroupBySubject = FALSE;
	m_bNeedsSorting = FALSE;
	m_bNeedsSortingNOW = FALSE;
	for (i = 0; i < NumSortColumns; i++)
		m_SortMethod[i] = NOT_SORTED;
	m_TopSum = m_CurrentSum = 0;
	m_NumShowing = 0;
	m_TotalSpace = m_MesSpace = m_DelSpace = 0L;
	m_NextUniqueMessageId = 0L;
	m_PeanutID = 0L;
	m_PeanutProp = 0L;
	m_SplitterPos = 0;
	m_UndoToToc = m_UndoFromToc = NULL;
	m_pPreviewableSummary = NULL;

#ifdef IMAP4
	m_pImapMailbox = NULL;
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


const short CTocDoc::DefaultFieldWidth[CTocDoc::NumFields]
    = {1, 1, 1, 8, 16, 16, 2, 1};

// StripName
// Set the name of the mailbox, stripping out menu-oriented characters if necessary
//
#ifdef OLDSTUFF
const char* CTocDoc::StripName(const char* NewName)
{
	Name() = NewName;
	StripMailboxName(Name().GetBuffer(Name().GetLength()));
	Name().ReleaseBuffer();
	
	return (Name());
}
#endif

// Build
// Rebuild a .TOC file for a mailbox
//
BOOL CTocDoc::Build()
{
	JJFile in;

	if (FAILED(in.Open(MBFilename(), O_RDONLY)))
		return (FALSE);

	m_Version = TocVersion;

	m_Type = MBT_REGULAR;
	if (!strchr(((const char*)Filename()) + EudoraDirLen, SLASH))
	{
		CString filename(((const char*) Filename()) + EudoraDirLen);

		if (!filename.CompareNoCase(CRString(IDS_IN_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_IN;
		else if (!filename.CompareNoCase(CRString(IDS_OUT_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_OUT;
		else if (!filename.CompareNoCase(CRString(IDS_TRASH_MBOX_FILENAME) + CRString(IDS_TOC_EXTENSION)))
			m_Type = MBT_TRASH;
	}
	if (m_Type != MBT_REGULAR)
		m_bAutoDelete = FALSE;

#ifdef IMAP4
	// Always remove document when view is closed.
	if (g_ImapAccountMgr.FindOwningAccount (MBFilename()) != 0)
	{
		m_Type = MBT_IMAP_MAILBOX;
		m_bAutoDelete = FALSE;
	}
#endif

	CSummary::m_lBegin = CSummary::m_lLastTime = 0L;
	m_TotalSpace = m_MesSpace = m_DelSpace = 0L;
	m_NextUniqueMessageId = 0L;
	m_PeanutID = 0L;
	m_PeanutProp = 0L;
	int ReadStatus = 1;
	for (int i = 0; ReadStatus == 1; i++)
	{
		CSummary* Sum = new CSummary;
		Sum->m_TheToc = this;
		ReadStatus = Sum->Build(&in, TRUE);
		if (Sum->m_Length != 0L)
			AddSum(Sum);
		else
			delete Sum;
		EscapePressed(1);
	}
	CSummary::m_lLastTime = 0L;
    
	in.Close();

	if (!ReadStatus)
		return (Write());

#ifdef IMAP4	// If an IMAP toc, inform IMAP that we just rebuilt it.
	if (m_pImapMailbox)
		m_pImapMailbox->SetTocJustRebuilt();
#endif

	return (FALSE);
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
		Off = Sum->m_Offset;
		Len = Sum->m_Length;
		if (Off < 0)
		{
			strcpy(buf, "Off < 0");
			goto fail;
		}
		else if (Len <= 0)
		{
			if (Len < 0)
			{
				strcpy(buf, "Len <= 0");
				goto fail;
			}

#ifdef IMAP4
			//  BEGIN $_IMAP4_$ (JOK, 9/23/97).

			// A summary in an IMAP mailbox can now have zero length, implying that 
			// only minimal headers were downloaded and nothing was stored in the MBX
			// file. Determine if this is such a header and if to delete it.
			if ( ( !IsImapToc() ) || ( IsImapToc() && !IsNotDownloaded(Sum) ) )
			{
				// Empty summary that never got saved
				POSITION NextPos = pos;
				if (NextPos)
					m_Sums.GetNext(NextPos);
				RemoveSum(Sum);
			}
#endif // IMAP4
			// END $_IMAP4_$ (JOK 9/23/97).
		}
		else if (Off + Len > m_TotalSpace)
		{
			strcpy(buf, "Off + Len > TotalSpace");
			goto fail;
		}
		else if (Sum->GetUniqueMessageId() > m_NextUniqueMessageId)
		{
			strcpy(buf, "message id out-of-range");
			goto fail;
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
	if (Ask)
	{
		if (AlertDialog(IDD_TOC_SALVAGE, (const char*)Name()) != IDOK)
			return (FALSE);
	}
	
	{
		CCursor cursor;
		int TotalSaved = 0;
		int OldNumSums = NumSums();
		CSumList OldSums(m_Sums);
		
		m_Sums.RemoveAll();
		Build();

		if (m_Type != MBT_OUT)
		{
			// Set state of all new toc's to recovered
			// If we later find the matching ones in the
			// old toc, this will (properly) be ignored
			POSITION NextNewPos, NewPos = m_Sums.GetHeadPosition();
			for (NextNewPos = NewPos; NewPos; NewPos = NextNewPos)
			{
				CSummary* NewSum = m_Sums.GetNext(NextNewPos);
				NewSum->m_State = MS_RECOVERED;
			}
		}

		// Loop through old toc, looking for matches in new toc
		POSITION NextOldPos, OldPos = OldSums.GetHeadPosition();
		for (NextOldPos = OldPos; OldPos; OldPos = NextOldPos)
		{
			CSummary* OldSum = OldSums.GetNext(NextOldPos);
			POSITION NextNewPos, NewPos = m_Sums.GetHeadPosition();

			for (NextNewPos = NewPos; NewPos; NewPos = NextNewPos)
			{
				CSummary* NewSum = m_Sums.GetNext(NextNewPos);

				if (NewSum->m_Offset == OldSum->m_Offset)
				{
					if (NewSum->m_Length == OldSum->m_Length)
					{
						// We don't want the rebuilt one, we want the old one, so delete the rebuilt one
						delete NewSum;

						//
						// When we rebuild a TOC, the Build() routine
						// resets m_NextUniqueMessageId to zero, then
						// increments it up to the total number of
						// summary objects generated from the MBX
						// file.  However, we're restoring an old
						// summary that has an old non-zero message
						// id, which is likely to be *greater* than
						// the new m_NextUniqueMessageId upper bound.
						// If we kept the old message id value, this
						// would cause mucho problemas later, so we
						// must reset the summary id to make sense in
						// the newly rebuilt TOC.
						//
						OldSum->SetUniqueMessageId(++m_NextUniqueMessageId);
						
						// Get back the old one
						m_Sums.SetAt(NewPos, OldSum);
						
						// Erase the old pointer, otherwise it will get deleted when the
						// old array gets deleted
						OldSums.SetAt(OldPos, NULL);
						TotalSaved++;
					}
					break;
				}
			}
			EscapePressed(1);
		}

		ErrorDialog(IDS_TOC_SALVAGE_RESULT, TotalSaved, OldNumSums, NumSums() - TotalSaved);
	}

	return (Write());
}

// Read
// Read in a .TOC file for a mailbox
//
BOOL CTocDoc::Read(BOOL HeaderOnly /*= FALSE*/)
{
	JJFile in;
	CFileStatus MBStatus, TocStatus;
	time_t MBTime, TocTime;
	int ChangeDate = FALSE;
	char buf[_MAX_PATH + 64];
	short SumCount;
	short temp;
	QCMailboxCommand* pCommand;
	CRString szInBoxName( IDS_IN_MBOX_NAME );
	CRString szOutBoxName( IDS_OUT_MBOX_NAME );
	CRString szTrashBoxName( IDS_TRASH_MBOX_NAME );
	
	//
	// Helper flag to handle cases where (tsk, tsk) the user renames
	// or copies a System TOC file (In/Out/Trash) to be a regular
	// mailbox file.  If set TRUE, then set the dirty bit on the
	// TOC to force the repaired TOC to be written out to disk.
	//
	BOOL needed_repair = FALSE;

	// Don't do anything if the summaries are already read
	if (m_Sums.GetCount())
		return (TRUE);

	if (!CFile::GetStatus(MBFilename(), MBStatus))
		return (FALSE); 
	if (!CFile::GetStatus(Filename(), TocStatus))
		return Build();
	if (FAILED(in.Open(Filename(), O_RDONLY)))
		return Build();
	MBTime = MBStatus.m_mtime.GetTime();
	TocTime = TocStatus.m_mtime.GetTime();

	while (1)
	{
		if (FAILED(in.Get(&m_Version))) 
			break;
		if (FAILED(in.GetLine((char *)m_UsedVersion, UsedVersion))) 
			break;
	
		if (FAILED(in.GetLine(m_strTitle.GetBuffer(NameSize), NameSize)))
			break;
		m_strTitle.ReleaseBuffer();
	
		// Mailbox type could also be corrupted on disk, don't trust it
		if (FAILED(in.Get(&m_Type))) 
			break;
	
		m_Type = MBT_REGULAR;
		if (!strchr(((const char*)Filename()) + EudoraDirLen, SLASH))
		{
			CString basename(((const char *) Filename()) + EudoraDirLen);
			int dot_idx = basename.Find('.');
			if (dot_idx != -1)
				basename = basename.Left(dot_idx);
			else
				ASSERT(0);

			//
			// Since the In/Out/Trash mailbox names can be translated 
			// to non-english names, we rely on the *filenames*
			// IN.MBX, OUT.MBX, and TRASH.MBX to determine whether or
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
			else
			{
				//
				// Okay, now we are fer sure fer sure dealing with a
				// normal, non-system mailbox.  Check to see if the
				// internal localized name conflicts with a localized
				// system mailbox name.  If so, then repair it by changing
				// it to the file's basename.
				//
				if (!Name().CompareNoCase( szInBoxName ) ||
					!Name().CompareNoCase( szOutBoxName ) ||
					!Name().CompareNoCase( szTrashBoxName ))
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
		if (g_ImapAccountMgr.FindOwningAccount (MBFilename()) != 0)
		{
			m_Type = MBT_IMAP_MAILBOX;
			m_bAutoDelete = FALSE;
		}

		if (FAILED(in.Get((short *)&temp))) break;
		if (temp & 0x01)
			m_bGroupBySubject = TRUE;
		if (temp & 0x02)
			m_bNeedsSorting = m_bNeedsSortingNOW = TRUE;

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
		for (i = 0; i < NumFields; i++)
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
		
		if (FAILED(in.Get(&temp))) break;
		m_UnreadStatus = (UnreadStatusType)temp;
		if (FAILED(in.Get(&m_NextUniqueMessageId))) break;
		if (FAILED(in.Get(&m_PeanutID))) break;
		if (FAILED(in.Get(&m_PeanutProp))) break;
		if (FAILED(in.Get(&m_SplitterPos))) break;
		if (!m_SplitterPos)
		{
			m_SplitterPos = GetIniShort(IDS_INI_PREVIEW_SPLITTER_POS);
			if (!m_SplitterPos)
				m_SplitterPos = (short)((m_SavedPos.bottom - 50) / 2);
		}

		for (i = 0; i < NumSortColumns; i++)
		{
			char c;
			if (FAILED(in.Get(&c))) break;
			m_SortMethod[i] = (SortType)(unsigned char)c;
		}

		if (FAILED(in.GetLine(buf, UnusedDiskSpace))) break;
		if (FAILED(in.Get(&SumCount))) break;
		
		if (HeaderOnly)
			return (TRUE);
		
		long FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
		UnreadStatusType NewUnreadStatus = US_NO;
		int ReadStatus;
		m_TotalSpace = m_MesSpace = 0L;
		const BOOL bNeedsSorting = m_bNeedsSorting;
		const BOOL bNeedsSortingNOW = m_bNeedsSortingNOW;
		while (1)
		{
			CSummary* Sum = new CSummary;
			Sum->m_TheToc = this;
			ReadStatus = Sum->Read(&in);
			if (ReadStatus <= 0)
			{
				delete Sum;
				break;
			}
			AddSum(Sum);

			if (NewUnreadStatus == US_NO && Sum->m_State == MS_UNREAD &&
				(Sum->m_Seconds + Sum->m_TimeZoneMinutes * 60) > FreshnessDate)
			{
				NewUnreadStatus = US_YES;
			}
		}
		// AddSum() sets these, so we need to reset them to their previous state
		m_bNeedsSorting = bNeedsSorting;
		m_bNeedsSortingNOW = bNeedsSortingNOW;

		if (ReadStatus < 0)
			break;
			
		in.Close();
		m_TotalSpace = MBStatus.m_size;

		if (NewUnreadStatus == US_YES && TocTime < FreshnessDate)
			NewUnreadStatus = US_NO;
		
		if (m_UnreadStatus != NewUnreadStatus)
		{
			// If we've changed unread status then save the new status by writing out the header
			m_UnreadStatus = NewUnreadStatus;
			if (!WriteHeader())
				break;
		}

		pCommand = g_theMailboxDirector.FindByPathname( MBFilename() );
		ASSERT( pCommand );
		if( pCommand )
		{
			pCommand->Execute( CA_UPDATE_STATUS, ( void* ) m_UnreadStatus );
		}

		if (needed_repair)
			SetModifiedFlag(TRUE);		// mark repaired TOC as dirty to force disk write
		else
			SetModifiedFlag(FALSE);
		
		if (SumCount != NumSums())
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
		}

		if ( leeway && ( MBTime > ( TocTime + leeway ) ) )
		{

			if (QCLogFileMT::DebugMaskSet(DEBUG_MASK_TOC_CORRUPT))
			{
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT, "Failed leeway test" );
			}

			switch (AlertDialog(IDD_TOC_TIME, (const char*)Name()))
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
		if (FAILED(out.Open(Filename(), O_CREAT | O_RDWR)))
			return (FALSE);
	}
	else
	{
		if (FAILED(out.Open(Filename(), O_CREAT | O_TRUNC | O_WRONLY)))
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
	if (FAILED(out.Put((short)temp))) return (FALSE);

	if (FAILED(out.Put((short)m_NeedsCompact)))  return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.left))) return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.top))) return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.right))) return (FALSE);
	if (FAILED(out.Put((short)m_SavedPos.bottom))) return (FALSE);

	for (i = 0; i < NumFields; i++)
		if (FAILED(out.Put(m_FieldWidth[i]))) return (FALSE);

	long FreshnessDate = time(NULL) - GetIniLong(IDS_INI_UNREAD_EXPIRES) * 60 * 60 * 24;
	UnreadStatusType NewUnreadStatus = US_NO;
	long lUnreadOffset = 0;
	out.Tell(&lUnreadOffset);
	ASSERT(lUnreadOffset >= 0);
	if (FAILED(out.Put((short)m_UnreadStatus))) return (FALSE);
	if (FAILED(out.Put(m_NextUniqueMessageId))) return (FALSE);

	if (FAILED(out.Put(m_PeanutID))) return (FALSE);
	if (FAILED(out.Put(m_PeanutProp))) return (FALSE);
	if (FAILED(out.Put(m_SplitterPos))) return (FALSE);

	for (i = 0; i < NumSortColumns; i++)
		if (FAILED(out.Put((char)m_SortMethod[i]))) break;

	// Clear buffer for writing empty space to disk
	memset(buf, 0, UnusedDiskSpace);
	if (FAILED(out.Put(buf, UnusedDiskSpace))) return (FALSE);

	if (FAILED(out.Put((short)NumSums()))) return (FALSE);
	if (!bHeaderOnly)
	{
		POSITION pos = m_Sums.GetHeadPosition();
		while (pos)
		{
			CSummary* Sum = m_Sums.GetNext(pos);
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

		if (m_UnreadStatus != NewUnreadStatus)
		{
			m_UnreadStatus = NewUnreadStatus;
			if (FAILED(out.Seek(lUnreadOffset)) || FAILED(out.Put((short)m_UnreadStatus)))
				return (FALSE);
		}

		if ( ! bSkipUnreadNotification )
		{
			pCommand = g_theMailboxDirector.FindByPathname( MBFilename() );

			// ASSERT only if this is not a .tbx mailbox.
			if ( !ImapIsTmpMbx (MBFilename()) )
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

	if (FAILED(out.Open(Filename(), O_CREAT | O_WRONLY)))
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
    CSummary* Sum = new CSummary;

    if (!Sum)
        return (NULL);

    if (First)
    {
        MBox->Tell(&CSummary::m_lBegin);
        ASSERT(CSummary::m_lBegin >= 0);
    }

    Sum->m_TheToc = this;
    Sum->Build(MBox);
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

	INT iPos = ( ( CTocFrame* )pFrame )->GetSplitterPos();

	if (UsingPreviewPane() && ( m_SplitterPos != iPos ) )
	{
		m_SplitterPos = (short)iPos;
		SetModifiedFlag();
	}
	
	
	BOOL bAutoDelete = m_bAutoDelete;
	// Are there messages open that belong to this toc?
	m_bAutoDelete = TRUE;
	
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

	//SK Preserve the state of m_bAutoDelete from the previous while loop
	if(m_bAutoDelete == TRUE)
		m_bAutoDelete = bAutoDelete;
	
	BOOL Result = CDoc::CanCloseFrame(pFrame);
	
	if (Result)
	{
		// Compact if need be
		struct _diskfree_t FreeInfo;
		
		if (m_DelSpace > (m_TotalSpace * GetIniShort(IDS_INI_COMPACT_MAILBOX_PERCENT)) / 100 ||
			(!_getdiskfree(toupper(EudoraDir[0]) - 'A' + 1, &FreeInfo) && 
			(unsigned long)m_DelSpace > 
				((((unsigned long)FreeInfo.avail_clusters) * 
				FreeInfo.sectors_per_cluster *
				FreeInfo.bytes_per_sector) / 100 * 
				GetIniShort(IDS_INI_COMPACT_DISK_PERCENT))))
		{
		    g_theMailboxDirector.CompactAMailbox( this );
		}
	}

#ifdef IMAP4
	// The TOC may not be destroyed, but we want to close the IMAP connection
	// in any case:
	//
	if (Result && IsImapToc() && (NULL != m_pImapMailbox) )
	{
		m_pImapMailbox->Close();

	}
#endif // IMAP4
	
	return (Result);
}

// --------------------------------------------------------------------------
//
// NotifyFrameClosing [PUBLIC]
//
// When the CTocFrame goes away we need to tell the CTocDoc, even
// tho for In,Out,Trash the CTocDoc doesn't actually die. This is
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
	struct _diskfree_t FreeInfo;
		
	if (m_DelSpace > (m_TotalSpace * GetIniShort(IDS_INI_COMPACT_MAILBOX_PERCENT)) / 100 ||
		(!_getdiskfree(toupper(EudoraDir[0]) - 'A' + 1, &FreeInfo) && 
		 (unsigned long)m_DelSpace > 
			((((unsigned long)FreeInfo.avail_clusters) * 
			FreeInfo.sectors_per_cluster *
			FreeInfo.bytes_per_sector) / 100 * 
			GetIniShort(IDS_INI_COMPACT_DISK_PERCENT))))
	{
		g_theMailboxDirector.CompactAMailbox( this );
	}
	
	CDoc::PreCloseFrame(pFrame);
}

void CTocDoc::AddSum(CSummary* Sum, BOOL SelectIt /*= FALSE*/)
{
	if (Sum)
	{
		m_MesSpace += Sum->m_Length;
		m_TotalSpace += Sum->m_Length;
		Sum->m_TheToc = this;
		m_Sums.Add(Sum);
		m_bNeedsSorting = TRUE;
		m_bNeedsSortingNOW = TRUE;

		//
		// Mark the new summary object with a unique message id.  In
		// theory, the message id for this TOC could roll over a
		// 32-bit unsigned integer, but we're not doing anything to
		// prevent the rollover from happening.  We just have to hope
		// that users will never process more than 4 billion messages
		// through a given Eudora mailbox.  :-)
		//
		if (Sum->GetUniqueMessageId() == 0)
		{
			// never allow id to be zero or negative when valid
			// this is similar to coding for beyond the year 2000,
			// as if that will ever happen ...
			// actually this in here to support automation where such
			// a thing is slightly more likely
			m_NextUniqueMessageId++;
			if (m_NextUniqueMessageId <= 0) m_NextUniqueMessageId = 1L;
			Sum->SetUniqueMessageId(m_NextUniqueMessageId);
		}
		ASSERT(Sum->GetUniqueMessageId() <= m_NextUniqueMessageId);

		CTocView* View = GetView();
		if (View)
		{
			int Index = View->m_SumListBox.AddString((LPCSTR)Sum);
			if (SelectIt && Index >= 0)
				View->m_SumListBox.SetSel(Index);
			View->SetCompact();
		}
		
		SetModifiedFlag();
	}
}

void CTocDoc::SubtractSum(POSITION SumPos, int Index /*= -1*/)
{
	if (SumPos)
	{
		CSummary* Sum = m_Sums.GetAt(SumPos);
		m_MesSpace -= Sum->m_Length;
		m_DelSpace += Sum->m_Length;

		Sum->SetUniqueMessageId(0);		// orphans don't have a message id.
		
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

			Sum->m_RespondingToSum->m_ResponseSum = NULL;
			Sum->m_RespondingToSum = NULL;
		}

		if (Index < 0)
			Index = m_Sums.GetIndex(Sum);
		
		m_NeedsCompact = TRUE;
		m_Sums.RemoveAt(SumPos);
		SetModifiedFlag();

		CTocView* View = GetView();
		if (View)
		{
			View->m_SumListBox.DeleteString(Index);
			View->SetCompact();
		}
	}
}

void CTocDoc::RemoveSum(POSITION SumPos, int Index /*= -1*/)
{
	if (SumPos)
	{
		CSummary* Sum = m_Sums.GetAt(SumPos);
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
			m_TotalSpace -= Sum->m_Length;
			m_DelSpace -= Sum->m_Length;
			AddSum(Sum, TRUE);
			m_NeedsCompact = (m_DelSpace != 0);
		}

		// Take summaries out of mailbox that they were transferred to
		while (m_UndoFromToc->m_UndoXferTo.GetHeadPosition())
		{
			Sum = m_UndoFromToc->m_UndoXferTo.RemoveHead();
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
		ASSERT(p_sum != NULL);

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
		ASSERT(p_sum != NULL);

		if (unreadOnly && (p_sum->m_State != MS_UNREAD))
			continue;		// skip message since it is not unread

		sortedList.AddTail(p_sum);
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
	CMessageDoc* p_doc = p_sum->GetMessageDoc();
	if (NULL == p_doc)
		return FALSE;
	if (NULL == p_doc->GetText())				// force message to be read up from mailbox
		return FALSE;
	char *p_fullmsg = p_doc->GetFullMessage();	// get full text of message, including all headers
	if (NULL == p_fullmsg)
		return FALSE;

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
		CString body(p_doc->GetText());   	// make a working copy of body text
		CRString keyword(IDS_ATTACH_CONVERTED);
		const int KEYWORD_LEN = keyword.GetLength();
		while (! body.IsEmpty())
		{
			int idx = body.Find('\n');
			CString line;
			if (idx != -1)
			{
				// get next line
				if ((idx > 0) && (body[idx - 1] == '\r'))
					line = body.Left(idx - 1);		// don't want CR in front of LF
				else
					line = body.Left(idx);
				body = body.Right(body.GetLength() - idx - 1);
			}
			else
			{
				// get last line
				line = body;
				body.Empty();
			}

			if ((! line.IsEmpty()) && (strncmp(line, keyword, KEYWORD_LEN) == 0))
				AddAttachmentData(messageData, line.Right(line.GetLength() - KEYWORD_LEN), FALSE);
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
		if (p_sum->IsFlowed())
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
			int nLength = messageData.GetLength() + strlen(pszBody) + (7 * (strlen(pszBody) / 60));
			char* pszMessageData = messageData.GetBufferSetLength(nLength);
			messageData.ReleaseBuffer();

			while (*pszBody)
			{
				const char* pszNewline = strchr(pszBody, '\n');
				if (pszNewline)
				{
					ASSERT(pszNewline > pszBody);
					ASSERT(*(pszNewline - 1) == '\r');
					*(char*)(pszNewline - 1) = '\0';		// temporary NULL termination
					messageData += "BODY: ";
					messageData += pszBody;
					messageData += "\n";
					*(char*)(pszNewline - 1) = '\r';		// undo temporary NULL termination
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
				line.Empty();
				BOOL bFoundAttachLine = (strncmp(OriginalBody, keyword, KEYWORD_LEN) == 0);
				const char* pszNewline = strchr(OriginalBody, '\n');
				if (pszNewline)
				{
					ASSERT(pszNewline > OriginalBody);
					ASSERT(*(pszNewline - 1) == '\r');
					if (bFoundAttachLine)
					{
						*(char*)(pszNewline - 1) = '\0';		// temporary NULL termination
						line = OriginalBody;
						*(char*)(pszNewline - 1) = '\r';		// undo temporary NULL termination
					}
					OriginalBody = pszNewline + 1;
				}
				else
				{
					// handle last line
					if (bFoundAttachLine)
						line = OriginalBody;
					OriginalBody += strlen(OriginalBody);
				}

				if ((! line.IsEmpty()) && bFoundAttachLine)
					AddAttachmentData(messageData, line.Right(line.GetLength() - KEYWORD_LEN), TRUE);
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
	const CString& messageData, //(i) encoded message data
	unsigned long& messageId) 	//(io) message id
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
	const CString& attachPathname,	//(i) full pathname to Eudora attachment file
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


BOOL CTocDoc::ShowPeanutIcon()
{ 

	return ( ( m_PeanutID != 0 ) && (((CEudoraApp *)AfxGetApp())->GetTranslators()->GetModule(850) != NULL) ); 
}



CSummary* CTocDoc::Xfer(CTocDoc* TargetToc, CSummary* SingleSum /*= NULL*/,
	BOOL CloseWindow /*= TRUE*/, BOOL Copy /*= FALSE*/, BOOL RedisplayTOCs /*= TRUE*/)
{
	static BOOL bXferActive = FALSE;			// reentrancy semaphore

	CTocView *View = GetView();
	int SelCount = 1;
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

#ifdef IMAP4
	// If one of the mailboxes is an IMAP mailbox, handle it specially.
	if ( IsImapToc() || (TargetToc && TargetToc->IsImapToc()) )
		return ImapXfer ( TargetToc, SingleSum, CloseWindow, Copy, RedisplayTOCs);
#endif

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
		// If trying to delete a message from the Trash mailbox, get rid of it
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


	// Some optimizations here for purging
	BOOL ServerDelete = GetIniShort(IDS_INI_SERVER_DELETE);
	BOOL MarkedSomeForServerDelete = FALSE;
	BOOL Tidy = GetIniShort(IDS_INI_TIDY_ATTACH);
	BOOL DeleteMAPI = (GetIniShort(IDS_INI_MAPI_DELETE_TRASH) != FALSE);
	BOOL DeleteAutoAttached = (GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_TRASH) != FALSE);
	char AttachDir[_MAX_PATH + 1];
	char ACStr[64];
	int ACLen;
	int ADLen;
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
		if (FAILED(TargetMBox.Open(TargetToc->MBFilename(), O_RDWR | O_CREAT | O_APPEND)) ||
			FAILED(SrcMBox.Open(MBFilename(), O_RDONLY)))
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
	if (SelCount == 1)
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

		if (View && !Copy && Sum == GetPreviewableSummary())
		{
			//
			// Unfortunately, a TocFrame has to keep a cached Summary pointer
			// for the message being displayed in the preview pane.  However,
			// this Xfer() method can nuke such Summary objects, leaving
			// the TocFrame with an invalid pointer.  So, before we get around
			// to nuking the Summary object below, we need to inform the frame
			// that the Summary is about to go away.
			//
			InvalidatePreviewableSummary(Sum);

			CView* pPreviewView = GetPreviewView();
			if (pPreviewView)
			{
				//
				// Check to see if preview pane has focus.  If so, then
				// just tweak the preview without opening a new
				// message window.
				//
				CWnd* pFocusWnd = CWnd::GetFocus();
				if (pPreviewView == pFocusWnd || pPreviewView->IsChild(pFocusWnd))
				{
					//
					// FORNOW, the big assumption here is that the change
					// focus event will be enough to kick off the normal
					// preview-the-summary-with-focus logic in the TocView.
					// This may change if we get away from a focus-based
					// preview scheme.
					//
					ASSERT(View);
					View->SetFocus();
				}
			}
		}

		FoundSelected = TRUE;
		if (Purging)
		{
			// Get rid of attachment if switch is on and it is still in the
			// auto-receive attachment directory
			if (Sum->IsComp())
			{
				if (DeleteMAPI && Sum->IsMAPI())
				{
					CCompMessageDoc* MsgDoc = (CCompMessageDoc*)Sum->GetMessageDoc();
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
				if (DeleteAutoAttached && Sum->IsAutoAttached())
				{
					CCompMessageDoc* MsgDoc = (CCompMessageDoc*)Sum->GetMessageDoc();
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
			}
			else if (Tidy && Sum->HasAttachment())
			{
				CMessageDoc* MsgDoc = Sum->GetMessageDoc();
				if (MsgDoc)
				{
					for (char* t = MsgDoc->GetText(); t; t = strchr(t, '\n'))
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
								// delete the current attachment
								::FileRemoveMT( szAttachmentPathname );
							}
							
							// set the pointer to the first character after this attachment 
							t += tNext + ACLen;
						}
					}
				}
			}

			// get rid of embedded objects
			CMessageDoc* MsgDoc = Sum->GetMessageDoc();
			if (MsgDoc)
			{
				char* buf = MsgDoc->GetFullMessage(); // gots ta get da URIMap
				CString Paths;
				MsgDoc->m_QCMessage.GetEmbeddedObjectPaths( Paths );
				DeleteEmbeddedObjects( Paths );
				delete [] buf;
			}
			
			if ( ServerDelete )
			{
				//CMsgRecord* LMOSRecord = GetMsgByHash( Sum );
				CLMOSRecord LMOSRecord(Sum);
				CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
				if (pMsgRecord && pMsgRecord->GetDeleteFlag() != LMOS_DELETE_MESSAGE/*0*/)
				{
					MarkedSomeForServerDelete = TRUE;
					pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE/*0*/);
				}
			}

			// Now get rid of it for good
			RemoveSum(pos, i);
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
				CopySum = new CSummary;
				CopySum->Copy(Sum);
				TargetToc->AddSum(CopySum);
				CopySum->SetOffset(lMStart);
				if (m_Type == MBT_OUT && CopySum->m_State != MS_SENT)
					CopySum->SetState(MS_UNSENT);
			}
			else
			{
				CCompMessageDoc* comp = Sum->SendAgain(FALSE);
				if (!comp)
					continue;
				if (comp->Write(&TargetMBox) == FALSE)
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
				// Remove summary from source TOC, but don't delete it
				// since it will be added to the undo list.
				SubtractSum(pos, i);

				if (FromOut && Sum->IsQueued())
					TransferredQueued = TRUE;

				// Add to the UndoFrom list of this mailbox
				m_UndoXferFrom.AddTail(Sum);

				// Add to the UndoTo list of the target mailbox
				TargetToc->m_UndoXferTo.AddTail(CopySum);
			}
		}
		if (SingleSum)
			break;

		// We've removed a summary, so the index of the next summary is
		// now the same as the index of what this summary was
		if (!Copy && !bGoingBackward)
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

	if (Purging)
	{
		if (MarkedSomeForServerDelete)
			CHostList::WriteLMOSFile();
	}
	else
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
	if (MBFilename().CompareNoCase(NewFilename) && FAILED(::FileRenameMT(Filename(), TocName)))
		return (FALSE);
	
	Filename() = TocName;
    MBFilename() = NewFilename;

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
			pos = m_Sums.GetHeadPosition();
			for (int i = 0; pos; i++)
			{
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
			if (Direction == ID_NEXT_MESSAGE)
				m_Sums.GetPrev(OldPos);
			else
				m_Sums.GetNext(OldPos);

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
			if (pTocView)
			{
				// Open the message and select the summary in the toc window
				CSummary* Sum = m_Sums.GetAt(OpenPos);
				pTocView->SelectAll(FALSE);
				pTocView->Select(Sum);
				MailboxChangedByHuman();
				Sum->Display();
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
			{
				//
				// Only switch the selection if the focus is in the
				// TOC view.  The CMessageDoc implementation calls
				// here directly *after* switching the selection for
				// us.
				//

				// BOG: If the above comment doesn't make you want to puke,
				// your application to the SCA (Sane Coder's Assoc.) is
				// hereby denied!
				//
				// I'll will now pretend I don't notice, and hack this thing
				// to mark the previous message as read (no matter who calls us).

				CWnd* pActiveView = pTocView->GetParentFrame()->GetActiveView();
				if (pPreviewView != pActiveView)
				{
					if (Direction == ID_NEXT_MESSAGE)
						m_Sums.GetNext(NewPos);
					else
						m_Sums.GetPrev(NewPos);
				}
				else {
					if (Direction == ID_NEXT_MESSAGE)
						m_Sums.GetPrev(OldPos);
					else
						m_Sums.GetNext(OldPos);
				}
			}

			if (NewPos)
			{
				CSummary* pNewSummary = m_Sums.GetAt(NewPos);
				CSummary* pLastSummary = NULL;
				
				if (OldPos)
					pLastSummary = m_Sums.GetAt(OldPos);

				if (pTocView)
				{
					pTocView->SelectAll(FALSE);
					pTocView->Select(pNewSummary);
					MailboxChangedByHuman();

					if ( pLastSummary && MS_UNREAD == pLastSummary->m_State )
						pLastSummary->SetState( MS_READ );

					//
					// FORNOW, the big assumption here is that the change
					// focus event will be enough to kick off the normal
					// preview-the-summary-with-focus logic in the TocView.
					// This may change if we get away from a focus-based
					// preview scheme.
					//

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
	if (m_bNeedsSortingNOW)
	{
		CTocView* pView = GetView();
		if (pView)
		{
			pView->SortNow();
			return TRUE;
		}
	}

	return FALSE;
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
		BOOL Result = NewChildFrame(TocTemplate, this)? TRUE : FALSE;
	
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
CTocDoc* GetToc(const char* Filename, const char* Name /*= NULL*/, BOOL HeaderOnly /*= FALSE*/, BOOL OnlyIfLoaded /*= FALSE*/)
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
		if (doc->MBFilename().CompareNoCase(Filename) == 0)
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
		if (!gbAutomationCall)
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
#ifdef OLDSTUFF
			doc->StripName(TheName);
#else
		doc->Name() = TheName;
#endif
		doc->MBFilename() = Filename;
        Existing = FALSE;
	}

	char TocName[_MAX_PATH + 1];
	strcpy(TocName, Filename);
	::SetFileExtensionMT(TocName, CRString(IDS_TOC_EXTENSION));
	doc->Filename() = TocName;

#ifdef IMAP4
	// If this is an IMAP mailbox, create the contained CImapMailbox object and
	// go read the IMAP mailbox's info file.
	ACCOUNT_ID AccountID;
	
	AccountID = g_ImapAccountMgr.FindOwningAccount (doc->MBFilename());
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
#ifdef OLDSTUFF
		doc->StripName(TheName);
#else
		doc->Name() = TheName;
#endif

	return (doc);

fail:
	if (!Existing && doc)
		doc->OnCloseDocument();
	return (NULL);
}

CTocDoc* GetToc(UINT nID, BOOL HeaderOnly /*= FALSE*/, BOOL OnlyIfLoaded /*= FALSE*/)
{
#ifdef OLDSTUFF
	CUserMenuItem* item;
	
	if ((item = g_MailboxMenu.GetItem(nID)) || (item = g_TransferMenu.GetItem(nID)))
	{
		CMailboxesMenuItem* MItem = (CMailboxesMenuItem*)item;
		return (GetToc(MItem->m_Filename, MItem->m_Name, HeaderOnly, OnlyIfLoaded));
	}
#endif	
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


// DisplayToc
// Load up a Toc in to memory, and then display it
BOOL DisplayToc(const char* Filename, const char* Name)
{
	CTocDoc* TocDoc = GetToc(Filename, Name);
	
	return (TocDoc? TocDoc->Display() : FALSE);
}

BOOL DisplayToc(UINT nID)
{
#ifdef OLDSTUFF
	CUserMenuItem* item;

	if ((item = g_MailboxMenu.GetItem(nID)) || (item = g_TransferMenu.GetItem(nID)))
	{
		CMailboxesMenuItem* MItem = (CMailboxesMenuItem*)item;
		return (DisplayToc(MItem->m_Filename, MItem->m_Name));
	}
#endif	
	return (FALSE);
}

#ifdef OLDSTUFF
static JJFile* TempFile = NULL;
static JJFile* MBoxFile = NULL;

// InitCompact
//
// Initializes some variables for compaction
//
static int InitCompact()
{
	TempFile = new JJFile;
	MBoxFile = new JJFile;

	if (!TempFile || !MBoxFile)
    	return (FALSE);

	MainProgress(CRString(IDS_COMPACTING));
	Progress(-1, NULL, -1);

	return (TRUE);
}

// EndCompact
//
// Clean up after compaction
//
static void EndCompact()
{
	if (TempFile)
	{
		delete TempFile;
		TempFile = NULL;
    }
	if (MBoxFile)
	{
		delete MBoxFile;
		MBoxFile = NULL;
    }
	CloseProgress();
}


// CompactOneMailbox
//
// Compacts only one mailbox 
//
CompactResultType CompactOneMailbox( LPCSTR szPathname, CTocDoc* TheToc /*= NULL*/)
{
	BOOL OnlyOne = (TempFile? FALSE : TRUE);
	CompactResultType Status = CR_MILD_ERROR;
    char FullPath[_MAX_PATH + 1];
	char buf[_MAX_PATH + 1];
	CSummary* Sum;
	POSITION pos;
	char* ptr;

	if (!TheToc)
    {
		TheToc = GetToc( szPathname, NULL, TRUE);
		if (!TheToc)
			goto done;
    }
	if (!TheToc->m_NeedsCompact)
	{
		Status = CR_COMPACTED;
		goto done;
	}

	if (OnlyOne && !InitCompact())
    {
        Status = CR_DRASTIC_ERROR;
		goto done;
    }
	if (!TheToc->Read())
		goto done;

	strcpy(buf, TheToc->MBFilename());
	ptr = strrchr(buf, '.');
	if (!ptr)
		goto done;
	*ptr = 0;
    ptr = strrchr(buf, SLASH);
	if (!ptr)
		goto done;
	sprintf(FullPath, "%s%s.tmp", (const char*)TempDir, ptr + 1);
	if (FAILED(MBoxFile->Open(TheToc->MBFilename(), O_RDWR)))
		goto done;

	// Let's see if compaction can be reduced to truncating the file
	long LastOffset;
	int i;
	pos = TheToc->m_Sums.GetHeadPosition();
	for (LastOffset = 0L, i = 0; pos; i++)
	{
		Sum = TheToc->m_Sums.GetNext(pos);
		if (Sum->m_Offset != LastOffset)
			break;
		LastOffset += Sum->m_Length;
	}
	if (i == TheToc->NumSums())
	{
		if (FAILED(MBoxFile->ChangeSize(LastOffset)))
			MBoxFile->Close();
		else
		{
			MBoxFile->Close();
			TheToc->m_NeedsCompact = FALSE;
			TheToc->ClearDelSpace();
			TheToc->WriteHeader();
			TheToc->ClearUndo();
			Status = CR_COMPACTED;
		}
		goto done;
	}

	Progress(0, TheToc->Name(), TheToc->NumSums());

	if (FAILED(TempFile->Open(FullPath, O_WRONLY | O_CREAT | O_TRUNC)))
	{
		Status = CR_DRASTIC_ERROR;
		goto done;
    }

	pos = TheToc->m_Sums.GetHeadPosition();
	for (i = 0; pos; i++)
	{
		Sum = TheToc->m_Sums.GetNext(pos);
		long lOffset = 0;
		TempFile->Tell(&lOffset);
		ASSERT(lOffset >= 0);

		Sum->m_Temp = Sum->m_Offset;
		if (FAILED(MBoxFile->JJBlockMove(Sum->m_Offset, Sum->m_Length, TempFile)))
			break;
		Sum->m_Offset = lOffset;
		ProgressAdd(1);

		// If the user presses Esc, then cancel this compaction
		if (EscapePressed())
		{
			Status = CR_DRASTIC_ERROR;
			break;
		}
	}
	// If we didn't get all the way through, something went wrong so clean up
	if (i < TheToc->NumSums())
	{
		pos = TheToc->m_Sums.GetHeadPosition();
		for (; pos && i; i--)
		{
			Sum = TheToc->m_Sums.GetNext(pos);
			Sum->m_Offset = Sum->m_Temp;
		}
		TempFile->Delete();
		MBoxFile->Close();
		goto done;
	}
	
	TempFile->Close();
	MBoxFile->Close();
	TempFile->Rename(TheToc->MBFilename());
	TheToc->m_NeedsCompact = FALSE;
	TheToc->ClearDelSpace();
	TheToc->Write();
	TheToc->ClearUndo();
	Status = CR_COMPACTED;

done:
	if (OnlyOne)
		EndCompact();
	else if (TheToc)
	{
		if (TheToc->GetView())
			TheToc->GetView()->SetCompact();
		if (TheToc->m_bAutoDelete)
			TheToc->OnCloseDocument();
    }

    return (Status);
}

// CompactMailboxes
//
// Goes through the entire directory of mailboxes and compacts each one
//

int CompactMailboxes(CUserMenu* Menu /*= NULL*/)
{
	static BOOL NoDrasticError;
	BOOL GetOut = FALSE;
	BOOL TopLevel = FALSE;

	if (!Menu)
		Menu = &g_MailboxMenu;
	if (Menu == &g_MailboxMenu)
	{
		TopLevel = TRUE;
		NoDrasticError = FALSE;
		if (!InitCompact())
			GetOut = 1;
	}

	POSITION pos = Menu->m_Items.GetHeadPosition();
	while (pos)
	{
		CMailboxesMenuItem* Item = (CMailboxesMenuItem*)Menu->m_Items.GetNext(pos);
		
		if (Item->m_Popup)
		{
			if (CompactMailboxes(Item->m_Popup) == CR_DRASTIC_ERROR)
            	GetOut = TRUE;
		}
		else
		{
			// Compact only mailboxes (no New... or separators)
			if (Item->m_Filename.IsEmpty() == FALSE && CompactOneMailbox( (LPCSTR) ( Item->m_Filename ) ) == CR_DRASTIC_ERROR)
            	GetOut = TRUE;
		}
	}
	if (!GetOut && TopLevel)
		NoDrasticError = TRUE;

	if (TopLevel)
    	EndCompact();

    return (NoDrasticError);
}

#endif

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
	BOOL MarkedSomeForServerDelete = FALSE;
	BOOL Tidy = GetIniShort(IDS_INI_TIDY_ATTACH);
	BOOL DeleteMAPI = (GetIniShort(IDS_INI_MAPI_DELETE_TRASH) != FALSE);
	BOOL DeleteAutoAttached = (GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_TRASH) != FALSE);
	char AttachDir[_MAX_PATH + 1];
	char ACStr[64];
	int ACLen;
	int ADLen;
	
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
	
	if (TrashToc->m_Sums.IsEmpty() == FALSE)
	{
		Progress(0, CRString(IDS_EMPTYING_TRASH), TrashToc->m_Sums.GetCount() - 1);

#ifdef COMMERCIAL
		CString CurPersona = g_Personalities.GetCurrent();
#endif // COMMERCIAL
		
		POSITION pos;
		for (int i = 0; pos = TrashToc->m_Sums.GetHeadPosition(); i++)
		{
			CSummary* Sum = TrashToc->m_Sums.GetAt(pos);

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

							if( tNext )
							{
								// delete the current attachment
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
				CMsgRecord* LMOSRecord = GetMsgByHash( Sum );
				if (LMOSRecord && LMOSRecord->GetDeleteFlag() != LMOS_DELETE_MESSAGE/*0*/)
				{
					MarkedSomeForServerDelete = TRUE;
					LMOSRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE/*0*/);
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
	
	if (TrashToc->m_Sums.IsEmpty())
	{
		// Get rid of Trash.MBX file
		JJFile TmpFile;
		if (SUCCEEDED(TmpFile.Open(TrashToc->MBFilename(), O_RDWR | O_CREAT | O_TRUNC)))
			TmpFile.Close();		// successfully nuked Trash MBX
		
		TrashToc->ClearDelSpace();
	}

	TrashToc->ClearUndoFrom();
	TrashToc->ClearUndoTo();
	
	TrashToc->Write();

	if (MarkedSomeForServerDelete)
		CHostList::WriteLMOSFile();

	if (View)
		View->m_SumListBox.SetRedraw(TRUE);

	CloseProgress();

	return (TRUE);
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



	// Remove the TOCs that are currently in memory
void ReclaimTOCs()
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	POSITION pos = TocTemplate->GetFirstDocPosition();
	
	while (pos)
	{
		bool bReclaim = true;
	
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc)
		{
			// write it to disk if need be
			if ( doc->IsModified() )
				doc->Write();
			
			if( !doc->GetView() && doc->m_bAutoDelete)
			{
				//there is no view

				//is one of Sums has a view?
				POSITION sum_pos = doc->m_Sums.GetHeadPosition();
				while (sum_pos)
				{
					CSummary* Sum = doc->m_Sums.GetNext(sum_pos);
					if (Sum && Sum->IsKindOf(RUNTIME_CLASS(CSummary)))
					{
						if(Sum->FindMessageDoc())
						{
							bReclaim = false;
							break;
						}
					}
				}

				if(bReclaim)
					doc->OnCloseDocument();
				
				break; //do one at a time
			
			}
			
		}

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
		CString mbfilename(p_tocdoc->MBFilename());
		ASSERT(! mbfilename.IsEmpty());

		//
		// Be sure to just compare to the "folder part" of the 
		// existing paths.
		//
		if (oldPathname.CompareNoCase(mbfilename.Left(oldPathname.GetLength())) == 0)
		{
			p_tocdoc->MBFilename() = newPathname + "\\" + mbfilename.Right(mbfilename.GetLength() - oldPathname.GetLength() - 1);

			CString filename(p_tocdoc->Filename());
			p_tocdoc->Filename() = newPathname + "\\" + filename.Right(filename.GetLength() - oldPathname.GetLength() - 1);
		}
	}
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
	if ( strnicmp( szDest, szAttachmentDir, strlen( szAttachmentDir ) ) )
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

#ifdef IMAP4
	if (IsImapToc() && !GetIniShort(IDS_INI_IMAP_PREVIEW_PANE))
		return FALSE;
#endif

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// InvalidatePreviewableSummary [public]
//
// Performs the internal cleanup necessary when a Summary object is
// being destroyed or otherwise invalidated.  That is, we invalidate
// the cached "previewable" Summary object and notify the Frame, if any,
// that a Summary object that it caches separately is being invalidated.
////////////////////////////////////////////////////////////////////////
void CTocDoc::InvalidatePreviewableSummary(CSummary* pPreviewableSummary)
{
	ASSERT(pPreviewableSummary);
	ASSERT_KINDOF(CSummary, pPreviewableSummary);

	BOOL bFrameNeedsPreviewUpdate = FALSE;
	CTocView* pTocView = GetView();
	
	if (GetPreviewableSummary() == pPreviewableSummary)
	{
		//Don't invalidate the cache already, because we need to know 
		//the current previewable summary when updating the frame else
		//the frame will delete the Msgdoc while we are still using it
		
		//bFrameNeedsPreviewUpdate = TRUE;	// for notify Frame, if any
		if(pTocView)
			bFrameNeedsPreviewUpdate = TRUE;	// for notify Frame, if any
		else
		{
			SetPreviewableSummary(NULL);		// invalidate local cache
			pPreviewableSummary->NukeMessageDocIfUnused();
		}
		
	}

	if (pTocView)
	{
		//
		// Unfortunately, a TocFrame has to keep a cached Summary pointer
		// for the message being displayed in the preview pane.  However,
		// since we're in the middle of nuking a Summary object, leaving
		// the TocFrame with an invalid pointer.  So, before we get around
		// to nuking ourselves, we need to inform the frame
		// that we are going away.
		//
		CTocFrame* pTocFrame = (CTocFrame *) pTocView->GetParentFrame();
		if (pTocFrame)
		{
			ASSERT_KINDOF(CTocFrame, pTocFrame);
			if (pTocFrame->GetPreviewSummary() == pPreviewableSummary)
			{
				//
				// First, tell the frame to nuke its cached
				// summary pointer and delete its associated
				// view-less CMessageDoc.
				// 
				extern UINT umsgPurgeSummaryCache;
				pTocFrame->SendMessage(umsgPurgeSummaryCache);
			}

			if (bFrameNeedsPreviewUpdate)
			{
				//
				// Notify the frame's Preview logic that the
				// message it was previewing (or was about to
				// preview) has been invalidated.
				//
				SetPreviewableSummary(NULL);	// invalidate local chache, for the TocDoc
				extern UINT umsgPreviewMessage;
				pTocFrame->PostMessage(umsgPreviewMessage);
			}
		}
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


#ifdef IMAP4
	// Include imap-specific stuff.
	#include "imaptoc.cpp"
#endif  // END IMAP4


void DeleteEmbeddedObjects( CString Paths )
{
	const char* Index = Paths;
	const char* End;
	while ( ( End = strchr(Index, '\n') ) != NULL )
	{
		CString Object(Index, End - Index);
		if ( InEmbeddedDir( Object ) )
			::FileRemoveMT(Object);

		Index = ++End;
	}
}

BOOL InEmbeddedDir( CString path )
{
	// does path appear to be located in our Embedded directory?
	char EmbeddedDir[_MAX_PATH + 1];
	wsprintf(EmbeddedDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_EMBEDDED_FOLDER));

	if ( path.GetLength() == 0 )
		return FALSE;

	if ( ! strchr( path, '\\' ) )
		return TRUE;	// no directories specified - must be in the embedded dir

	if ( strnicmp( EmbeddedDir, path, strlen( EmbeddedDir ) ) == 0 )
		return TRUE;

	return FALSE;
}
