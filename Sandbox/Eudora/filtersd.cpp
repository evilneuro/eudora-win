// FILTERSD.CPP
//

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>
#include <mmsystem.h>

#include "QCUtils.h"

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "mdichild.h"
#include "fileutil.h"
#include "summary.h"
#include "doc.h"
#include "tocdoc.h"
#include "controls.h"
#include "filtersd.h"
#include "guiutils.h"
#include "3dformv.h"
#include "tocview.h"
#include "filtersv.h"
#include "msgutils.h"
#include "msgdoc.h"
#include "eudora.h"
#include "progress.h"
#include "pop.h"
#include "MsgRecord.h"	//for CMsgRecord
//#include "saveas.h"
#include "nickdoc.h"
#include "address.h"
#include "ReadMessageDoc.h"
#include "saveas.h"

#include "mainfrm.h"		// FORNOW A TOTAL HACK
#include "FilterReportView.h"

#include "QCCommandActions.h"
#include "QCMailboxCommand.h"
#include "QCMailboxDirector.h"
#include "QCStationeryCommand.h"
#include "QCStationeryDirector.h"
#include "Automation.h"
#include "TextToSpeech.h"
#include "hsregex.h" //Regex support
#include "QCSharewareManager.h"

#ifdef IMAP4
	#include "imapfol.h"
#endif

extern QCMailboxDirector	g_theMailboxDirector;
extern QCStationeryDirector	g_theStationeryDirector;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef EXPIRING
#include "timestmp.h"
extern CTimeStamp	g_TimeStamp;
#endif

static CFilterActions g_FiltActions;

/////////////////////////////////////////////////////////////////////////////
CFilter::CFilter()
{
	m_WhenToApply = m_Verb[0] = m_Verb[1] = m_Conjunction = m_PriorityChange = m_Transfer = 0;
	m_Print = m_SkipRest= 0;
	m_strVoiceGUID.Empty();
	m_Label = m_OpenMM = m_NotfyUserNR = m_ServerOpt = m_MakeStatus = m_MakePriority = m_SpeakOptions = -1;
	m_Mailbox.Empty();
	m_MakeSubject.Empty();
	m_DoPersonality = FALSE;
	m_ChangeSubject = FALSE;
	m_Personality.Empty();
	
	for (int i = 0; i < 5; i++)
	{
		m_Actions[i] = ID_FLT_NONE;
		m_CopyTo[i].Empty();
		m_Sound[i].Empty();
		m_NotifyApp[i].Empty();
		m_Forward[i].Empty();
		m_Redirect[i].Empty();
		m_Reply[i].Empty();
		m_DoForward[i] = m_DoRedirect[i] = m_DoReply[i] = m_DoCopyTo[i] = m_PlaySound[i] = m_DoNotifyApp[i] = FALSE;
	}
	SetName();
}


/////////////////////////////////////////////////////////////////////////////
const char* CFilter::SetName()
{
	if (m_Header[0].IsEmpty() && m_Value[0].IsEmpty())
		m_Name.LoadString(IDS_UNTITLED);
	else
	{
		if (m_Header[0].IsEmpty())
			m_Name = ':';
		else
			m_Name = m_Header[0];
		m_Name += m_Value[0];
	}

	return (m_Name);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFilter::SetTransferMBOX(const char * name, const char * pathName, BOOL skipRest /*= TRUE*/)
{
	BOOL bSUCCESS = FALSE;
	int i = 0;

	// Insert this is the first empty action
	for ( ;i < NUM_FILT_ACTS  && !bSUCCESS; i++)
	{
		if (m_Actions[i] == ID_FLT_NONE)
		{
			SetTransfer();
			SetTransferTo();
			m_Actions[i] = ID_FLT_TRANSFER;
			m_Desc[i] = name;
			m_Mailbox = pathName;
			bSUCCESS = TRUE;
		}
	}

	if (skipRest)
	{
		bSUCCESS = FALSE;
		// Insert this is the first empty action
		for ( ;i < NUM_FILT_ACTS  && !bSUCCESS; i++)
		{
			if (m_Actions[i] == ID_FLT_NONE)
			{
				m_Actions[i] = ID_FLT_SKIP_REST;
				m_SkipRest = TRUE;
				bSUCCESS = TRUE;
			}
		}
	}

	return bSUCCESS;
}

bool MatchesRegex(const char *szText, const char *szRegex, int nFlags)
{
	regex_t Reg;
	int nRes = regcomp(&Reg, szRegex, REG_EXTENDED | REG_NOSUB | (nFlags?REG_ICASE:0));
	if(nRes != 0)
	{
		ASSERT(0);
		return FALSE;
	}

	nRes = regexec(&Reg, szText, 0, NULL, 0);
	if(nRes == 0) 
		return true;

	return false;
}

	
/////////////////////////////////////////////////////////////////////////////
BOOL CFilter::MatchValue(int NumCondition, const char* Contents) const
{
	int verb = VERB_FIRST + m_Verb[NumCondition];
	CString value(m_Value[NumCondition]);
	int len1, len2;

	//for non-regex filters
	if( !((verb == IDS_MATCHES_REGEX) || (verb == IDS_MATCHES_REGEX)))
	{
		//Removing const_cast here is OK as we gonna throw away the Contents once we are done matching
		strlwr(const_cast<char *>(Contents));
	
		// Filter comparisons are case insensitive, so convert to lowercase
		value.MakeLower();
	}

	switch (verb)
	{
	case IDS_CONTAINS:
	case IDS_DOESNT_CONTAIN:
		if (strstr(Contents, value))
			return (TRUE);
		break;

	case IDS_MATCHES_REGEX:
		return MatchesRegex(Contents, value, 0)?TRUE:FALSE;
		break;

	case IDS_MATCHES_REGEX_ICASE:
		return MatchesRegex(Contents, value, 1)?TRUE:FALSE;
		break;


	case IDS_IS:
	case IDS_IS_NOT:
		if (!strcmp(Contents, value))
			return (TRUE);
		break;
	case IDS_STARTS_WITH:
		if (!strncmp(Contents, value, value.GetLength()))
			return (TRUE);
		break;

	case IDS_ENDS_WITH:
		len1 = value.GetLength();
		len2 = strlen(Contents);
		if (len1 < len2 && !strcmp(Contents + len2 - len1, value))
			return (TRUE);
		break;

	case IDS_APPEARS:
	case IDS_DOESNT_APPEAR:
		return (TRUE);
	case IDS_INTERSECTS:
		{
			BOOL foundMatch = FALSE;

			// Get the Alias
			char* exAls = ExpandAliases(value, FALSE, FALSE, FALSE);

			// Get out of here if there are no addresses or alias
			if (!exAls || !*exAls || !Contents || !*Contents) 
			{
				if (exAls) delete []exAls;
				return (FALSE);
			}

			// copy the contents because StripAddress will modify string
			char* addContents = new char[strlen(Contents)+1];
			strcpy(addContents, Contents);
			
			char* Aliases = new char[strlen(exAls)+1];
			strcpy(Aliases, exAls);

			// walk through each address from field 
			char *curAddress = addContents;
			while(!foundMatch && curAddress && *curAddress)
			{
				char *EndOfAddress = FindAddressEnd(curAddress, FALSE);
				if (EndOfAddress && *EndOfAddress)
					*EndOfAddress++ = 0;

				// Get the address from the field
				StripAddress(curAddress);
				
				// Get a fresh copy of the alias before looping through and modifying
				strcpy(Aliases, exAls);
				char *curAlias = Aliases;
				
				// walk through each alias address
				while(!foundMatch && curAlias && *curAlias)
				{
					char *EndOfAliasAddress = FindAddressEnd(curAlias, FALSE);
					if (EndOfAliasAddress && *EndOfAliasAddress)
						*EndOfAliasAddress++ = 0;
					
					StripAddress(curAlias);
					
					if( !stricmp(curAlias, curAddress))
					{
						foundMatch = TRUE;
						break;
					}
					curAlias = EndOfAliasAddress;
				}
				curAddress = EndOfAddress;
			}
			
			if (exAls) delete []exAls;
			if (Aliases) delete []Aliases;
			if (addContents) delete []addContents;
			return (foundMatch);
		}
		break;
	}

	return (FALSE);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFilter::MatchCondition(int NumCondition, const char* text) const
{
	BOOL Matches = FALSE;
	const char* t = text;

	// Shoud we search the body?
	if (!CompareRStringI(IDS_FHEADER_BODY, m_Header[NumCondition]))
	{
		if (t = FindBody(text))
			Matches = MatchValue(NumCondition, t);
	}
	else
	{
		BOOL AnyHeader = FALSE;
		BOOL AnyAddressee = FALSE;
		BOOL matchPersonality = FALSE;
		
		if (!CompareRStringI(IDS_FHEADER_ANY, m_Header[NumCondition]))
			AnyHeader = TRUE;
		else if (!CompareRStringI(IDS_FHEADER_ADDRESSEE, m_Header[NumCondition]))
			AnyAddressee = TRUE;
		else if (!CompareRStringI(IDS_FHEADER_PERSONALITY, m_Header[NumCondition]))
			matchPersonality= TRUE;
		
		while (1)
		{
			// Found blank line separator between headers and body, so stop
			if (*t == '\r' || *t == '\n')
				break;

			// Find the end of this header
			const char* header = t;

// this crashes Eudora when filtering certain messages:
//			while ((t = strchr(t, '\n')) && (*++t == ' ' || *t == '\t'))
//				;
			while( ( t = strchr( t, '\n' ) ) != NULL )
			{
				++t;
				if( ( *t != ' ' ) && ( *t != '\t' ) )
				{
					break;
				}
			}

			// Did we get something
			if (!t)
				break;
			for (const char* s = t - 1; s[-1] == '\r' || s[-1] == '\n'; s--)
				;
			const char* colon = strchr(header, ':');
			if (!colon)
				continue;
			const char* contents = colon + 1;
			while (*contents == ' ' || *contents == '\t')
				contents++;
  			
			// Are there any addressee fields
			BOOL Addressee = FALSE;
			if (AnyAddressee && 
					(!CompareRStringI(IDS_FHEADER_TO, header, colon - header) ||
					!CompareRStringI(IDS_FHEADER_CC, header, colon - header) ||
					!CompareRStringI(IDS_HEADER_BCC, header, colon - header)))
					Addressee = TRUE;
			
			// Has this message been marked for personality?
			BOOL personMarked = FALSE;
			if (matchPersonality && 
				(!CompareRStringI(IDS_PERSONA_XHEADER, header, colon - header)))
			{
				// remove the <..> so it'll compare personas as the UI presents it, without the <..>
				if (*contents == '<')
					contents++;
				if (*(s - 1) == '>')
					s -= 1;
				personMarked = TRUE;
			}


			if ((AnyHeader || Addressee || personMarked || !strnicmp(header, m_Header[NumCondition], colon - header)))
			{
				char SaveChar = *s;
				char* SaveSpot = (char*)s;
				*SaveSpot = 0;

				Matches = MatchValue(NumCondition, contents);
				*SaveSpot = SaveChar;
				if (Matches)
					break;
			}
		}
	}

	switch (VERB_FIRST + m_Verb[NumCondition])
	{
	case IDS_DOESNT_CONTAIN:
	case IDS_IS_NOT:
	case IDS_DOESNT_APPEAR:
		Matches = !Matches;
	}

	return (Matches);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFilter::Match(const char* text) const
{
	BOOL Matches = FALSE;
	
	if (text)
	{
		EscapePressed(1);
		Matches = MatchCondition(0, text);
		EscapePressed(1);
		switch (CONJUNCTION_FIRST + m_Conjunction)
		{
		case IDS_FIO_IGNORE:
			break;

		case IDS_FIO_AND:
			if (Matches)
				Matches = MatchCondition(1, text);
			break;

		case IDS_FIO_OR:
			if (!Matches)
				Matches = MatchCondition(1, text);
			break;

		case IDS_FIO_UNLESS:
			if (Matches)
				Matches = !MatchCondition(1, text);
			break;
		}
		EscapePressed(1);
	}

	return (Matches);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFilter::PreventAutoRepsonse(CSummary* Sum)
{
	// Force the message to be re-read so lower case version no longer cached
	Sum->GetMessageDoc()->Read();

	char *fullM = Sum->GetMessageDoc()->GetFullMessage();
	if (!fullM)
		return TRUE;

	BOOL bNoResponse = FALSE;
	CRString Precedence(IDS_HEADER_PRECEDENCE);
	CString NoAutoResponseHeaders(GetIniString(IDS_INI_NO_AUTO_RESPONSE_HEADERS));
	CString precLabels(GetIniString(IDS_INI_PRECEDENCE));

	for (const char* header = fullM; !bNoResponse && header && *header && *header != '\r'; )
	{
		const BOOL bIsPrecedence = (strnicmp(Precedence, header, Precedence.GetLength()) == 0);

		// Certain headers (default is "Errors-to:" and "List-") mean no auto-response
		const char* narh = NoAutoResponseHeaders;
		while (*narh)
		{
			const char* comma = strchr(narh, ',');
			if (!comma)
				comma = narh + strlen(narh);
			if (strnicmp(header, narh, comma - narh) == 0)
			{
				bNoResponse = TRUE;
				break;
			}
			if (!*comma)
				break;
			narh = comma + 1;
		}

		if (bIsPrecedence)
			header += Precedence.GetLength();

		do
		{
			if (bIsPrecedence)
			{
				while (*header == ' ' || *header == '\t')
					header++;

				// Do not auto-respond to Precedence: bulk or list
				const char* filtPrec = precLabels;
				while (*filtPrec)
				{
					const char* comma = strchr(filtPrec, ',');
					if (!comma)
						comma = filtPrec + strlen(filtPrec);
					if (strnicmp(header, filtPrec, comma - filtPrec) == 0)
					{
						bNoResponse = TRUE;
						break;
					}
					if (!*comma)
						break;
					filtPrec = comma + 1;
				}
			}

			header = strchr(header, '\n');
			if (!header)
				break;
			header++;
		} while (*header == ' ' || *header == '\t');
	}
	
	delete [] fullM;

	return bNoResponse;
}

// --------------------------------------------------------------------------

//
// IsRestrictedFilterAction [PUBLIC]
//
// Returns true if the action is restricted.
//
/* static */ bool CFilter::IsRestrictedFilterAction(UINT nAction)
{
	// Shareware: Restrict some filters in REDUCED FEATURE mode
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		// All filter actions are allowed, so return non-restricted
		return (false);
	}

	// REDUCED FEATURE mode
	switch (nAction)
	{
		// These are the only actions permitted in REDUCED FEATURE mode
		case ID_FLT_NONE:
		case ID_FLT_STATUS:
		case ID_FLT_PRIORITY:
		case ID_FLT_SUBJECT:
		case ID_FLT_NOTIFY_APP:
		case ID_FLT_COPY:
		case ID_FLT_TRANSFER:
		case ID_FLT_SKIP_REST:
			return (false);

		// These actions are not allowed in REDUCED FEATURE mode
		case ID_FLT_LABEL:
		case ID_FLT_PERSONALITY:
		case ID_FLT_SOUND:
		case ID_FLT_SPEAK:
		case ID_FLT_OPEN:
		case ID_FLT_PRINT:
		case ID_FLT_NOTIFY_USER:
		case ID_FLT_FORWARD:
		case ID_FLT_REDIRECT:
		case ID_FLT_REPLY:
		case ID_FLT_SERVER_OPT:
			return (true);
	}

	// Uh-oh!  We should never get here.
	// Just assume the action is restricted.
	ASSERT(0);
	return (true);
}

/////////////////////////////////////////////////////////////////////////////
int CFilter::Action(CSummary*& Sum, CFilterActions* fltAct)
{
	char Scratch[_MAX_PATH + 1];
	int FiltActs = FA_NORMAL;

	// Shareware: Only check for IDS_INI_USE_FILTERS in REDUCED FEATURE mode
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		if ( ! GetIniShort( IDS_INI_USE_FILTERS ) )
			return (FiltActs);
	}

	// First Level
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_NONE:
				break;
			case ID_FLT_PRIORITY:
				// Change priority
				if (IsRaise())
					Sum->SetPriority(short(__max(Sum->m_Priority - 1, MSP_HIGHEST)));
				else if (IsLower())
					Sum->SetPriority(short(__min(Sum->m_Priority + 1, MSP_LOWEST)));
				else if (m_MakePriority >= 0)
  					Sum->SetPriority(short(m_MakePriority + 1));
				break;

			case ID_FLT_NOTIFY_APP:
				{
					CString cmdLine = SetupCommandLine(Sum, m_NotifyApp[i]);
					if (fltAct->m_NotifyApp.Find(cmdLine) == NULL)
						fltAct->m_NotifyApp.AddTail(cmdLine); 
				}
				break;

			case ID_FLT_LABEL:
				if (m_Label >= 0)
					Sum->SetLabel(short(m_Label));
				break;
			
			case ID_FLT_SERVER_OPT:
				{
					if (m_ServerOpt > 0)
					{
						//CMsgRecord* LMOSRecord = GetMsgByHash( Sum );
						CLMOSRecord LMOSRecord(Sum);
						CMsgRecord *pMsgRecord = LMOSRecord.GetMsgRecord();
	
						if ( pMsgRecord )
						{
							if(m_ServerOpt & SO_DELETE)
								pMsgRecord->SetDeleteFlag(LMOS_DELETE_MESSAGE);
							if (m_ServerOpt& SO_FETCH)
								pMsgRecord->SetRetrieveFlag(LMOS_RETRIEVE_MESSAGE);
						}
						LMOSRecord.WriteLMOS();
					}
				}
				break;
			case ID_FLT_SOUND:
				if (fltAct->m_SoundList.Find(m_Sound[i]) == NULL)
					fltAct->m_SoundList.AddTail(m_Sound[i]);
				break;
			
			case ID_FLT_SPEAK:
				if(m_SpeakOptions > 0)
				{
					SpeakFilterText(m_SpeakOptions, m_strVoiceGUID, 
						(m_SpeakOptions & SPEAK_WHO)?Sum->GetFrom():"", 
						(m_SpeakOptions & SPEAK_SUBJECT)?Sum->GetSubject():"");
					
					/*if(m_SpeakOptions & SPEAK_WHO)
					{
						char szFrom[512];
						strcpy(szFrom, "From: ");
						strcat(szFrom, Sum->GetFrom()); 
						TextToSpeech(szFrom, m_strVoiceGUID);
					}
					
					if(m_SpeakOptions & SPEAK_SUBJECT)
						TextToSpeech( Sum->GetSubject(), m_strVoiceGUID);*/
				}
				break;
			
			case ID_FLT_PERSONALITY:
				if (m_DoPersonality)
				{
					Sum->SetPersona(m_Personality);
					// Figure out what to really do here!!! JAB!
				}
				break;

			default:
				break;
		}
	}

	// Second Level
	for (i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_SUBJECT:
				// Change subject
				if (m_ChangeSubject)
				{
					int index = m_MakeSubject.Find('&');
					if (index < 0)
						Sum->SetSubject(m_MakeSubject);
					else
					{
						strncpy(Scratch, m_MakeSubject, index);
						Scratch[index] = 0;
						strcat(Scratch, Sum->m_Subject);
						strcat(Scratch, ((const char*)m_MakeSubject) + index + 1);
						Sum->SetSubject(Scratch);
					}
				}
				break;

			case ID_FLT_FORWARD:
			case ID_FLT_REDIRECT:
			case ID_FLT_REPLY:
				if (((m_Actions[i] == ID_FLT_REPLY) && m_DoReply[i]) ||
					((m_Actions[i] == ID_FLT_FORWARD) && m_Forward[i]) ||
					((m_Actions[i] == ID_FLT_REDIRECT) && m_Redirect[i]) )
				{
					if (PreventAutoRepsonse(Sum))
						break;

					// Make the Precedence: bulk
					CString oldPrec = Sum->GetPrecedence();
					BOOL replySetting = GetIniShort(IDS_INI_REPLY_TO_ALL);
					SetIniShort(IDS_INI_REPLY_TO_ALL, FALSE);

					Sum->SetPrecedence("bulk");
					
					if (m_Actions[i] == ID_FLT_REPLY)
					{
						QCStationeryCommand*	pCommand;
						
						pCommand = g_theStationeryDirector.Find( m_Reply[i] );
						
						if ( pCommand != NULL )
						{
							pCommand->Execute( CA_AUTO_REPLY_WITH, Sum );
						}
					}
					else if(m_Actions[i] == ID_FLT_FORWARD)
						Sum->ComposeMessage(ID_MESSAGE_FORWARD,m_Forward[i], TRUE);
					else if(m_Actions[i] == ID_FLT_REDIRECT)
						Sum->ComposeMessage(ID_MESSAGE_REDIRECT,m_Redirect[i], TRUE);

					Sum->SetPrecedence(oldPrec);
					SetIniShort(IDS_INI_REPLY_TO_ALL, short(replySetting));
				}
				break;

			default:
				break;
		}
	}
  	
	// Third Level
	//
	// Change status after replying/forwarding/redirecting because
	// those actions will change the status of the message.
	for (i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_STATUS:
				// Status
				if (m_MakeStatus >= 0)
					Sum->SetState((char)m_MakeStatus);
				break;
		}
	}
  	
	// Fourth Level
	for (i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_SKIP_REST:
				FiltActs |= FA_SKIPREST;
				break;
			case ID_FLT_COPY:
			case ID_FLT_TRANSFER:
				// Transfer/Copy to another mailbox
				while (!m_CopyTo[i].IsEmpty() || IsTransfer())
				{
					CTocDoc* toc = Sum->m_TheToc;
					CString mboxPath;

					if (!toc)
						break;

					if (m_Actions[i] == ID_FLT_TRANSFER)
					{
						mboxPath = m_Mailbox;
						SetTransferTo();
					}
					else
					{
						mboxPath = m_CopyTo[i];
						SetCopyTo();
					}

					strcpy(Scratch, mboxPath);
					char* slash = strrchr(Scratch, SLASH);
					if (slash)
						slash[1] = 0;
					
					
					CTocDoc* DestToc = GetToc(mboxPath);
					if (!DestToc)
						break;

					// Need to turn off Easy Open during transfer
					short EO = GetIniShort(IDS_INI_EASY_OPEN);
					SetIniShort(IDS_INI_EASY_OPEN, FALSE);
					CSummary *XferSum = toc->Xfer(DestToc, Sum, TRUE, IsCopyTo(), FALSE);
					// If we transfered, update our summary 
					if (!IsCopyTo()){
						 if(XferSum) // make sure we are not storing a NULL
						 {
							 //update sums in the lists
							 POSITION pos;
							 if ( (pos=fltAct->m_FilterOpenSumList.Find(Sum)) != NULL)
							 {
								 fltAct->m_FilterOpenSumList.RemoveAt(pos);
								 fltAct->m_FilterOpenSumList.AddTail(XferSum);
							 }
							
							 if ( (pos=fltAct->m_FilterPrintSum.Find(Sum)) != NULL)
							 {
								 fltAct->m_FilterPrintSum.RemoveAt(pos);
								 fltAct->m_FilterPrintSum.AddTail(XferSum);
							 }
						 }

						 CMessageDoc *pDoc = Sum->FindMessageDoc();
						 if(pDoc)
							 pDoc->m_Sum = XferSum;

						 // Now we are safe to copy
					 	 Sum = XferSum;
						 
					
					}

					SetIniShort(IDS_INI_EASY_OPEN, EO);

					// If user specifically said not to notify as normal, don't add
					if ((IsIncoming() && m_NotfyUserNR < 0) || m_NotfyUserNR & NO_NORMAL)
					{
						// Add this mailbox to the list of mailboxes that have had messages filtered into them
						if (fltAct->m_FilterTocList.Find(DestToc) == NULL)
							fltAct->m_FilterTocList.AddTail(DestToc);
						FiltActs |= FA_NORMAL;
					}
					else
						FiltActs &= ~FA_NORMAL;

					FiltActs |= FA_TRANSFER;
					if (!Sum)
					{
						FiltActs |= FA_SKIPREST;
						return FiltActs;
					}
				break;
				}
				break;
			default:
				break;
		}
	}

	// Now do things that depend on the summary
 	
	// Add entry to report either 'option' report or specified filter
	if(Sum->m_TheToc->m_Type == MBT_REGULAR  && 
		(m_NotfyUserNR < 0 && GetIniShort(IDS_INI_FILTER_REPORT)) || 
		(m_NotfyUserNR > 0 && m_NotfyUserNR & NO_REPORT ))
	{
			CString				szFullMailboxName;
			QCMailboxCommand*	pCommand;

			pCommand = g_theMailboxDirector.FindByPathname( Sum->m_TheToc->MBFilename() );
			
			ASSERT( pCommand );		
			
			if( pCommand != NULL ) 
			{
				szFullMailboxName = g_theMailboxDirector.BuildNamedPath( pCommand );
				fltAct->m_NotifyReportList.Add( szFullMailboxName );
			}
		}


	// Shareware: REDUCED FEATURE mode only has one global setting for opening mbxs
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Open Mailbox by either "option" or specified Filter
		if((m_OpenMM < 0 && GetIniShort(IDS_INI_OPEN_IN_MAILBOX)) || 
			(m_OpenMM > 0 && m_OpenMM & OM_MAILBOX ))
		{
			if (fltAct->m_FilterOpenMBoxTocList.Find(Sum->m_TheToc) == NULL)
				fltAct->m_FilterOpenMBoxTocList.AddTail(Sum->m_TheToc);
		}
	}
	else
	{
		// REDUCED FEATURE
		if( GetIniShort(IDS_INI_OPEN_IN_MAILBOX) )
		{
			if (fltAct->m_FilterOpenMBoxTocList.Find(Sum->m_TheToc) == NULL)
				fltAct->m_FilterOpenMBoxTocList.AddTail(Sum->m_TheToc);
		}
	}

	// Shareware: Open & Print msg are FULL FEATURE mode only
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Open Message by specified Filter
		if (m_OpenMM > 0 && m_OpenMM & OM_MESSAGE)
		{	
			// Add this mailbox to the list of mailboxes that have had messages filtered into them
			if (fltAct->m_FilterOpenSumList.Find(Sum) == NULL)
				fltAct->m_FilterOpenSumList.AddTail(Sum);
		}
		
		if (m_Print)
		{
			if (fltAct->m_FilterPrintSum.Find(Sum) == NULL)
				fltAct->m_FilterPrintSum.AddTail(Sum);
		}
	}

	// Only return TRUE if we are transferring so that we know to stop
	// looking at further filters
	return (FiltActs);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////////////// IMAP CFilter::ImapAction class method. ////////////////////////////////

#ifdef IMAP4  // IMAP4

#include "ImapFiltersd.h"

/////////////////////////////////////////////////////////////////////////////
// ImapAction [PUBLIC]
//
// Perform non-server IMAP filter actions on the given summary.
// Copy/Transfers between mailboxes on the same server are not performed here,
//
// "Make Status: Read, Unread, Replied" are handled by calling back to the 
// CImapFilterActions object to add this summary to the appropriate list.
// The actual status change is done only after the filter is applied to all
// messages.
//
// This is similar to the main Action() method but because we may need to 
int CFilter::ImapLocalAction(CSummary*& Sum, CFilterActions* fltAct)
{
	char Scratch[_MAX_PATH + 1];
	int FiltActs = FA_NORMAL;
	CTocDoc *pSrcToc = NULL;

	// Must have a proper fltAct:
	if (!fltAct)
	{
		ASSERT (0);
		return (FiltActs);
	}

	if (!Sum)
	{
		ASSERT (0);
		return (FiltActs);
	}

	// Note: Could be NULL!!
	//
	pSrcToc = Sum->m_TheToc;

	// Shareware: Only check for IDS_INI_USE_FILTERS in REDUCED FEATURE mode
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		if ( ! GetIniShort( IDS_INI_USE_FILTERS ) )
			return (FiltActs);
	}

	// First Level
	for (int i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Quit if user pressed escape:
		//
		if ( EscapePressed(1) )
		{
			break;
		}

		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_NONE:
				break;
			case ID_FLT_PRIORITY:
				// Change priority
				if (IsRaise())
					Sum->SetPriority(short(__max(Sum->m_Priority - 1, MSP_HIGHEST)));
				else if (IsLower())
					Sum->SetPriority(short(__min(Sum->m_Priority + 1, MSP_LOWEST)));
				else if (m_MakePriority >= 0)
  					Sum->SetPriority(short(m_MakePriority + 1));
				break;

			case ID_FLT_STATUS:
				// For IMAP, Call back to the CImapFilterActions object
				// to add this UID to a list.
				//
				if (m_MakeStatus >= 0)
					((CImapFilterActions *)fltAct)->SetImapMsgStatus(Sum, (char)m_MakeStatus);

				break;

			case ID_FLT_LABEL:
				if (m_Label >= 0)
					Sum->SetLabel(short(m_Label));
				break;
			
			case ID_FLT_SERVER_OPT:
				// For IMAP, Call back to the CImapFilterActions object
				// to add this UID to a list.
				//
				if (m_ServerOpt > 0)
				{
					// Add to the m_szDeletedUid's list.
					((CImapFilterActions *)fltAct)->ImapSetServerOpt (Sum, m_ServerOpt);
				}
				break;

			case ID_FLT_SOUND:
				if (fltAct->m_SoundList.Find(m_Sound[i]) == NULL)
					fltAct->m_SoundList.AddTail(m_Sound[i]);
				break;
			case ID_FLT_NOTIFY_APP:
				{
					CString cmdLine = SetupCommandLine(Sum, m_NotifyApp[i]);
					if (fltAct->m_NotifyApp.Find(cmdLine) == NULL)
						fltAct->m_NotifyApp.AddTail(cmdLine); 
				}
				break;
			case ID_FLT_PERSONALITY:
				if (m_DoPersonality)
				{
					Sum->SetPersona(m_Personality);
					// Figure out what to really do here!!! JAB!
				}
				break;

			default:
				break;
		}
	}

	// Second Level
	for (i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Quit if user pressed escape:
		//
		if ( EscapePressed(1) )
		{
			break;
		}

		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_SUBJECT:
				// Change subject
				if (m_ChangeSubject)
				{
					int index = m_MakeSubject.Find('&');
					if (index < 0)
						Sum->SetSubject(m_MakeSubject);
					else
					{
						strncpy(Scratch, m_MakeSubject, index);
						Scratch[index] = 0;
						strcat(Scratch, Sum->m_Subject);
						strcat(Scratch, ((const char*)m_MakeSubject) + index + 1);
						Sum->SetSubject(Scratch);
					}
				}
				break;

			case ID_FLT_FORWARD:
			case ID_FLT_REDIRECT:
			case ID_FLT_REPLY:
				if (((m_Actions[i] == ID_FLT_REPLY) && m_DoReply[i]) ||
					((m_Actions[i] == ID_FLT_FORWARD) && m_Forward[i]) ||
					((m_Actions[i] == ID_FLT_REDIRECT) && m_Redirect[i]) )
				{
					if (PreventAutoRepsonse(Sum))
						break;

					// Make the Precedence: bulk
					CString oldPrec = Sum->GetPrecedence();
					BOOL replySetting = GetIniShort(IDS_INI_REPLY_TO_ALL);
					SetIniShort(IDS_INI_REPLY_TO_ALL, FALSE);

					Sum->SetPrecedence("bulk");
					
					if (m_Actions[i] == ID_FLT_REPLY)
					{
						QCStationeryCommand*	pCommand;
						
						pCommand = g_theStationeryDirector.Find( m_Reply[i] );
						
						if ( pCommand != NULL )
						{
							pCommand->Execute( CA_AUTO_REPLY_WITH, Sum );
						}
					}
					else if(m_Actions[i] == ID_FLT_FORWARD)
						Sum->ComposeMessage(ID_MESSAGE_FORWARD,m_Forward[i], TRUE);
					else if(m_Actions[i] == ID_FLT_REDIRECT)
						Sum->ComposeMessage(ID_MESSAGE_REDIRECT,m_Redirect[i], TRUE);

					Sum->SetPrecedence(oldPrec);
					SetIniShort(IDS_INI_REPLY_TO_ALL, short(replySetting));
				}
				break;

			default:
				break;
		}
	}
  	
  	
	// Third Level
	//
	// Change status after replying/forwarding/redirecting because
	// those actions will change the status of the message.
	for (i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_STATUS:
				// For IMAP, Call back to the CImapFilterActions object
				// to add this UID to a list.
				//
				if (m_MakeStatus >= 0)
					((CImapFilterActions *)fltAct)->SetImapMsgStatus(Sum, (char)m_MakeStatus);
				break;
		}
	}
  	
	// Fourth Level. Note: Handles Copy/Transfer to LOCAL mbox or to another IMAP server.
	//
	for (i = 0; i < NUM_FILT_ACTS; i++)
	{
		// Quit if user pressed escape:
		//
		if ( EscapePressed(1) )
		{
			break;
		}

		// Skip restricted actions
		if (IsRestrictedFilterAction(m_Actions[i]))
			continue;

		switch(m_Actions[i])
		{
			case ID_FLT_SKIP_REST:
				FiltActs |= FA_SKIPREST;
				break;
			case ID_FLT_COPY:
			case ID_FLT_TRANSFER:
				// Transfer/Copy to another mailbox on a different server or to a 
				// local mailbox.
				while (!m_CopyTo[i].IsEmpty() || IsTransfer())
				{
					CTocDoc* toc = Sum->m_TheToc;
					CString mboxPath;

					if (!toc)
						break;

					if (m_Actions[i] == ID_FLT_TRANSFER)
					{
						mboxPath = m_Mailbox;
						SetTransferTo();
					}
					else
					{
						mboxPath = m_CopyTo[i];
						SetCopyTo();
					}

					strcpy(Scratch, mboxPath);
					char* slash = strrchr(Scratch, SLASH);
					if (slash)
						slash[1] = 0;
					
					
					CTocDoc* DestToc = GetToc(mboxPath);
					if (!DestToc)
						break;

					// Need to turn off Easy Open during transfer
					short EO = GetIniShort(IDS_INI_EASY_OPEN);
					SetIniShort(IDS_INI_EASY_OPEN, FALSE);

					// (JOK - 4/15/98 - If user has IDS_INI_IMAP_REMOVE_ON_DELETE
					// set, don't do an EXPUNGE here after EACH xfer!!!
					//
					short bAlwaysExpunge = GetIniShort (IDS_INI_IMAP_REMOVE_ON_DELETE);
					if (bAlwaysExpunge)
						SetIniShort (IDS_INI_IMAP_REMOVE_ON_DELETE, FALSE);

					CSummary *XferSum = toc->Xfer(DestToc, Sum, TRUE, IsCopyTo(), FALSE);

					// If we transfered, update our summary 
					if (!IsCopyTo())
						 Sum = XferSum;

					SetIniShort(IDS_INI_EASY_OPEN, EO);

					// Reset!
					if (bAlwaysExpunge)
						SetIniShort (IDS_INI_IMAP_REMOVE_ON_DELETE, bAlwaysExpunge);

					// If user specifically said not to notify as normal, don't add
					if ((IsIncoming() && m_NotfyUserNR < 0) || m_NotfyUserNR & NO_NORMAL)
					{
						// Add this mailbox to the list of mailboxes that have had messages filtered into them
						if (fltAct->m_FilterTocList.Find(DestToc) == NULL)
							fltAct->m_FilterTocList.AddTail(DestToc);
						FiltActs |= FA_NORMAL;
					}
					else
						FiltActs &= ~FA_NORMAL;

					FiltActs |= FA_TRANSFER;
					if (!Sum)
					{
						FiltActs |= FA_SKIPREST;
						return FiltActs;
					}
				break;
				}
				break;
			default:
				break;
		}
	}

	// Now do things that depend on the summary
 	
	// Add entry to report either 'option' report or specified filter
	if(Sum && 
		(m_NotfyUserNR < 0 && GetIniShort(IDS_INI_FILTER_REPORT)) || 
		(m_NotfyUserNR > 0 && m_NotfyUserNR & NO_REPORT ))
		{
			CString				szFullMailboxName;
			QCMailboxCommand*	pCommand;

			pCommand = g_theMailboxDirector.FindByPathname( Sum->m_TheToc->MBFilename() );
			
			ASSERT( pCommand );		
			
			if( pCommand != NULL ) 
			{
				szFullMailboxName = g_theMailboxDirector.BuildNamedPath( pCommand );
				fltAct->m_NotifyReportList.Add( szFullMailboxName );
			}
		}


	// Shareware: REDUCED FEATURE mode only has one global setting for opening mbxs
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Open Mailbox by either "option" or specified Filter
		if((m_OpenMM < 0 && GetIniShort(IDS_INI_OPEN_IN_MAILBOX)) || 
			(m_OpenMM > 0 && m_OpenMM & OM_MAILBOX ))
		{
			if (fltAct->m_FilterOpenMBoxTocList.Find(Sum->m_TheToc) == NULL)
				fltAct->m_FilterOpenMBoxTocList.AddTail(Sum->m_TheToc);		
		}
	}
	else
	{
		// REDUCED FEATURE
		if( GetIniShort(IDS_INI_OPEN_IN_MAILBOX) ) 
		{
			if (fltAct->m_FilterOpenMBoxTocList.Find(Sum->m_TheToc) == NULL)
				fltAct->m_FilterOpenMBoxTocList.AddTail(Sum->m_TheToc);		
		}
	}

	// Shareware: Open & Print msg are FULL FEATURE mode only
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Open Message by specified Filter
		if (m_OpenMM > 0 && m_OpenMM & OM_MESSAGE)
		{	
			// Add this mailbox to the list of mailboxes that have had messages filtered into them
			if (fltAct->m_FilterOpenSumList.Find(Sum) == NULL)
				fltAct->m_FilterOpenSumList.AddTail(Sum);
		}

		if (m_Print)
		{
			if (fltAct->m_FilterPrintSum.Find(Sum) == NULL)
				fltAct->m_FilterPrintSum.AddTail(Sum);
		}
	}

	// Only return TRUE if we are transferring so that we know to stop
	// looking at further filters
	return (FiltActs);
}


#endif // IMAP4
/////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////

CString CFilter::SetupCommandLine(CSummary* Sum, CString rawCmd)
{
	CString cmdLine;
	char tmpFile[_MAX_PATH];			
	LPCSTR rglpsz[6];
	CMessageDoc* doc = Sum->GetMessageDoc();
	
	// Read up the full message
  	doc->CMessageDoc::Read();
	char *fullMes = doc->GetFullMessage();

	rglpsz[0] = Sum->GetDate();
	rglpsz[1] = HeaderContents(IDS_HEADER_TO, fullMes);
	rglpsz[2] = Sum->GetFrom();
	rglpsz[3] = Sum->GetSubject();
	rglpsz[4] = HeaderContents(IDS_HEADER_CC, fullMes);
	
	// if there's a %6 in string, a file has to be created and message 
	// dumped in there
	if (rawCmd.Find("%6") >= 0)
	{
		//extern CString TempDir - declared in fileutil.h
		if ( (! TempDir.IsEmpty()) && GetTempFileName(TempDir, "eud", 0, tmpFile))
		{
			short ih = GetIniShort(IDS_INI_INCLUDE_HEADERS);
			
			JJFile theFile;
			
			if (SUCCEEDED(theFile.Open( tmpFile, O_CREAT | O_TRUNC | O_WRONLY)))
			{
				if (doc && Sum->IsComp())
				{
					SetIniShort(IDS_INI_INCLUDE_HEADERS, 0);
					doc->WriteAsText( &theFile );
				}
				else if (doc)
				{
					CSaveAs sa;
					if (sa.CreateFile(tmpFile))
					{
						SetIniShort(IDS_INI_INCLUDE_HEADERS, 1);
						((CReadMessageDoc *)doc)->SaveAs(sa);
					}
				}
				
 				rglpsz[5] = tmpFile;
			}
			SetIniShort(IDS_INI_INCLUDE_HEADERS, ih);
		}
	}
 	AfxFormatStrings(cmdLine, rawCmd, rglpsz, 6);

	delete [] (char *)rglpsz[1];
	delete [] (char *)rglpsz[4];
	delete [] fullMes;

	return cmdLine;

}

/////////////////////////////////////////////////////////////////////////////

CFilterList::~CFilterList()
{
	NukeAllData();
}


/////////////////////////////////////////////////////////////////////////////
void CFilterList::NukeAllData()
{
	while (!IsEmpty())
		delete RemoveHead();
}


/////////////////////////////////////////////////////////////////////////////
CFilter* CFilterList::IndexData(int nIndex) const
{
	POSITION pos = FindIndex(nIndex);
	
	return (pos? GetAt(pos) : NULL);
}



static CFiltersDoc* g_Filters = NULL;

BOOL CreateFiltersDoc()
{
	if (g_Filters)
	{
		// Now why would you want to create more than one of these, huh?!
		ASSERT(0);
		return TRUE;
	}

	g_Filters = (CFiltersDoc *) NewChildDocument(FiltersTemplate);

	return g_Filters? TRUE : FALSE;
}

CFiltersDoc* GetFiltersDoc()
{
	g_Filters->DoOneTimeInitIfNotDoneYet();
	return g_Filters;
}

BOOL CanCloseFiltersFrame()
{
	return g_Filters->CanCloseFrame(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CFiltersDoc

IMPLEMENT_DYNCREATE(CFiltersDoc, CDoc)

CFiltersDoc::CFiltersDoc()
{
	m_bOneTimeInit = false;
}

bool CFiltersDoc::DoOneTimeInitIfNotDoneYet()
{

	if( m_bOneTimeInit == false)
	{
		//Read in the filters and do all checks
		if (!Read(true))
		{
			OnCloseDocument();
			return false;
		}
		LoadExtraFilters(); // Read in extra filter files

		m_bOneTimeInit = true;
		return true;
	}

	return false;
}


BOOL CFiltersDoc::OnNewDocument()
{
	if (!CDoc::OnNewDocument())
		return (FALSE);
		
	m_strPathName = EudoraDir + CRString(IDS_FIO_FILENAME);
	SetTitle(CRString(IDR_FILTERS));
	
	return (TRUE);
}

CFiltersDoc::~CFiltersDoc()
{
}


////////////////////////////////////////////////////////////////////////
// CanCloseFrame [public, virtual]
//
////////////////////////////////////////////////////////////////////////
BOOL CFiltersDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	// Give a chance to move info from controls to filter
	if (pFrame)
	{
		CView* pView = pFrame->GetActiveView();
		if (pView)
			pView->UpdateData();
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		POSITION pos = GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = GetNextView(pos);
			ASSERT_VALID(pView);

			if (pView->IsKindOf(RUNTIME_CLASS(CFiltersViewRight)))
			{
				//
				// Force RHS view to do the data exchange thing.  All we're trying
				// to do here is update the dirty bit for the underlying filters
				// document so that the IsModified() call below reflects
				// the current document state.
				//
				pView->UpdateData(TRUE);
			}
		}
		
	}

	//
	// Don't call the base class implementation here since it puts up
	// the default Yes/No/Cancel "do ya wanna save" UI.  Now that the
	// filters are Wazooed, we must do our own UI since only Yes/No
	// are applicable.
	//
//	return (CDoc::CanCloseFrame(pFrame));
	if (! IsModified())
		return TRUE;        // ok to continue

	CString strPrompt;
	AfxFormatString1(strPrompt, AFX_IDP_ASK_TO_SAVE, m_strTitle);
	if (!gbAutomatedShutdown)
	{
		switch (AfxMessageBox(strPrompt, MB_YESNO, AFX_IDP_ASK_TO_SAVE))
		{
		case IDYES:
			// Do the save-to-disk thing...
			OnSaveDocument(NULL);
			break;
		case IDNO:
			// Reload original data from disk
			m_Filters.NukeAllData();
			Read();
			UpdateAllViews(NULL, FUT_REFRESH);
			SetModifiedFlag(FALSE);
			break;
		default:
			ASSERT(0);		// Hmm.  Should never get here?
			break;
		}
	}
	else
	{
		// Do the save-to-disk thing...
		OnSaveDocument(NULL);
	}

	//
	// Always return TRUE since our Wazooed views are not housed in
	// traditional MDI child windows anymore.
	//
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// OnCloseDocument [public, virtual]
//
// The standard base implementation ends up calling CDocument::OnCloseDocument().
// This is really bad for this since it destroys the "parent frame", which
// could be main frame itself in the case of a docked filters view.
//
// So, instead, all we need to do is to divorce ourselves from all of
// our views and optionally destroy ourselves.  The Wazoo windows take 
// care of destroying the views.
////////////////////////////////////////////////////////////////////////
void CFiltersDoc::OnCloseDocument()
{
	//
	// WARNING:  Do NOT call the base class OnCloseDocument()
	// implementation since it will destroy the frame associated with
	// each view!
	//
	BOOL bAutoDelete = m_bAutoDelete;
	m_bAutoDelete = FALSE;  // prevents destruction of document while closing views

	// manually divorce ourselves from our views...
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		RemoveView(pView);
	}
	m_bAutoDelete = bAutoDelete;

	// clean up contents of document before destroying the document itself
	DeleteContents();

	// delete the document if necessary
	if (m_bAutoDelete)
	{
		delete this;
		g_Filters = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
BOOL CFiltersDoc::OnSaveDocument(const char* pszPathName)
{
	UpdateAllViews(NULL, FUT_UPDATEDOC);

	if (IsModified() && !Write()) 
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFiltersDoc::CreateFilterItem(CFilter *pFilter)
{
	UpdateAllViews(NULL, FUT_NEWITEM, (CObject*)pFilter);
	OnSaveDocument(NULL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// FilterMsg
//
// Figures out if the passed in message needs to be filtered, and does
// the filtering.  Returns the filter actions performed.
//
/////////////////////////////////////////////////////////////////////////////
int CFiltersDoc::FilterMsg(CSummary* Sum, int WhenToApply, CFilterActions *pFiltAct, FILTER_FILE_TYPE ffType /*= FF_MAIN_FILTER*/)
{
	CFilterList *pFiltList = NULL;
	if ( ffType == FF_PRE_FILTER )
		pFiltList = &m_PreFilters;
	else if( ffType == FF_MAIN_FILTER )
		pFiltList = &m_Filters;
	else if( ffType == FF_POST_FILTER )
		pFiltList = &m_PostFilters;
	else 
		return FALSE;

	CMessageDoc* doc = NULL;
	int		FiltAct = FA_NORMAL;
	UINT	retFiltAct = 0;
	BOOL	CreatedDoc = FALSE;
	char*	text = NULL;
	BOOL	TextWasAllocated = FALSE;
	POSITION pos = pFiltList->GetHeadPosition();
	

	while (pos)
	{
		CFilter* filt = pFiltList->GetNext(pos);

		EscapePressed(1);
		
		if (filt->m_WhenToApply & WhenToApply)
		{
			if (!(doc = Sum->FindMessageDoc()))
			{
				CreatedDoc = TRUE;
				doc = Sum->GetMessageDoc();
				text = NULL;
			}

			if (!doc)
			{
				ASSERT(0);
				break;
			}

			if (!TextWasAllocated)
			{
				if (Sum->IsComp() && !text)
				{
					// If this is a composition messages, then we need to reload the
					// message text because if it were previously loaded the headers
					// would have already been stripped out
					text = doc->GetFullMessage();
					TextWasAllocated = TRUE;
					//strlwr(text); //do it later
				}
				else if (!text || text != doc->GetText())
				{
					// Need to force comp messages to be read in as received messages
					// so that the headers won't get stripped
					if (!doc->CMessageDoc::Read() || !(text = doc->GetText()))
					{
						ASSERT(FALSE);
						break;
					}

					// Filter comparisons are case insensitive, so convert to lowercase
					//strlwr(text);  //do it later
					
				}
			}

			if (filt->Match(text))
			{
				Progress(-1, Sum->MakeTitle());
				
				// Stop when we've done a transfer
				if (FiltAct = filt->Action(Sum, pFiltAct))
				{
					// accumulate the action bits
					retFiltAct |= FiltAct;

					if ( !Sum || (FiltAct & FA_SKIPREST) )
						break;
				}
			}
		}
	}

	if (TextWasAllocated)
		delete [] text;

	// Xfer is nuking the doc ... when it's in preview mode
	// So we need to be careful about removing the doc
	// (JOK 10/7/97) - "Sum" may have been deleted.
	if (Sum && CreatedDoc)
		Sum->NukeMessageDocIfUnused();

	if ( !retFiltAct )
	{
		// nothing happened
		return FA_NORMAL;
	}

	return ( retFiltAct );
}

/////////////////////////////////////////////////////////////////////////////
const int KEYWORD_FIRST = IDS_FIO_RULE;
const int KEYWORD_LAST = IDS_FIO_COPY_TO;
const int NumKeywords = KEYWORD_LAST - KEYWORD_FIRST + 1;
const int NumHeaders = IDS_FIO_MATCH_PERSONALITY - IDS_FIO_ANY_HEADER + 1;


/////////////////////////////////////////////////////////////////////////////
void CFiltersDoc::LoadExtraFilters()
{
	WIN32_FIND_DATA			fd;
	HANDLE					findHandle;
	CString					szFind;
	CString					szPath;

	// Clear out lists
	m_PreFilters.NukeAllData();
	m_PostFilters.NukeAllData();
	
	// Find all the filters files and load 'em up
	for (int i = IDS_FILTER_PRE_EXT; i <= IDS_FILTER_PST_EXT; i++)
	{
		szPath = EudoraDir + CRString( IDS_FILTER_DIRECTORY ); 
		szPath += "\\";
		szFind = szPath + "*." + CRString( i );
		
		findHandle = FindFirstFile( szFind, &fd ); 

		// find all the .pre & .pst filter files
		while( findHandle != HANDLE( INVALID_HANDLE_VALUE ) )
		{
			Read( szPath + fd.cFileName );

			if ( !FindNextFile( findHandle, &fd ) )  
				break; 
		}
		if( findHandle != HANDLE( INVALID_HANDLE_VALUE ) )
			FindClose( findHandle );
	}
	
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFiltersDoc::Read(BOOL bDoChecks /* = false */ )
{
	return (Read(m_strPathName, bDoChecks));
}

/////////////////////////////////////////////////////////////////////////////

// bDoChecks is true if you want to check the filters for problems
// bDoChecks should be true for the first filter load (during startup)
BOOL CFiltersDoc::Read(const char *filtFileName, BOOL bDoChecks /* = false */ )
{
	// We're going to keep track of filter actions which
	// reference missing mailboxes. We need a pointer to the filter
	// and the action index. Quick and dirty = two arrays.
	CArray<CFilter*,CFilter*> MissingMailboxFilter;
	CArray<int,int> MissingMailboxIndex;

	CFilterList *pFilterList;
	CString	szFileName = filtFileName;
	BOOL FoundConj = FALSE;
	CFilter* filt = NULL;
	char buf[256];
	JJFile filtFile;
	short Version = 0;
	int i = 0;
	BOOL FIRST = TRUE;

	if (!::FileExistsMT(szFileName)) 
		return (TRUE);
	if (filtFile.Open(szFileName, O_RDONLY) < 0)
		return (FALSE);

	if ( szFileName.Right(3).CompareNoCase( CRString(IDS_FILTER_PRE_EXT) ) == 0 )  
		pFilterList = &m_PreFilters;
	else if ( szFileName.Right(3).CompareNoCase( CRString(IDS_FILTER_PST_EXT) ) == 0 )  
		pFilterList = &m_PostFilters;
	else
		pFilterList = &m_Filters;

	CString KeywordStrings[NumKeywords];
	LoadCStringArray(KeywordStrings, KEYWORD_FIRST, KEYWORD_LAST);

	CString VerbStrings[NumVerbs];
	//LoadCStringArray(VerbStrings, IDS_FIO_CONTAINS, IDS_FIO_INTERSECTS);
	LoadCStringArray(VerbStrings, IDS_FIO_CONTAINS, IDS_FIO_REGEX);

	CString JunctStrings[NumConjunctions];
	LoadCStringArray(JunctStrings, CONJUNCTION_FIRST, CONJUNCTION_LAST);

	CString HeaderStrings[NumHeaders];
	LoadCStringArray(HeaderStrings, IDS_FIO_ANY_HEADER, IDS_FIO_MATCH_PERSONALITY);

	long lNumBytesRead = 0;
	while (SUCCEEDED(filtFile.GetLine(buf, sizeof(buf), &lNumBytesRead)) && (lNumBytesRead > 0))
	{
		// Find end of word
		const char* space = buf;
		char *ptr = NULL;
		while (*space && *space != ' ')
			space++;

		// Did we find a matching keyword?
		int Keyword = FindCStringArrayIndexI(NumKeywords, KeywordStrings, buf, space - buf);
		
		// See if there's a version # befor getting it
		if (FIRST)
		{
			FIRST = FALSE;
			if ( (Keyword + KEYWORD_FIRST) == IDS_FIO_RULE)
				Version = 0;
			else 
			{
				Version = ( short ) atoi(buf);
				continue;
			}
		}

		if (Keyword < 0)
			continue;
		Keyword += KEYWORD_FIRST;
		if (*space == ' ')
			space++;
			 
		// Do nothing until we get a first rule
		if (!filt && Keyword != IDS_FIO_RULE)
			continue;

		switch (Keyword)
		{
		case IDS_FIO_RULE:
			filt = new CFilter;
			pFilterList->AddTail(filt);
			filt->m_Name = space;
			FoundConj = FALSE;
			i = 0;
			break;
			
		case IDS_FIO_INCOMING:	filt->SetIncoming(); 			break;
		case IDS_FIO_OUTGOING:	filt->SetOutgoing();			break;
		case IDS_FIO_MANUAL:	filt->SetManual();				break;
		case IDS_FIO_LABEL:		
			filt->m_Label = atoi(space);
			filt->m_Actions[i++] = ID_FLT_LABEL;
			break;
		case IDS_FIO_PRIORITY:	
			filt->m_MakePriority = atoi(space);	
			filt->m_Actions[i++] = ID_FLT_PRIORITY;
			if (filt->m_MakePriority == PRIOR_LOWER)
				filt->SetLower();
			if (filt->m_MakePriority == PRIOR_RAISE)
				filt->SetRaise();
			break;
		case IDS_FIO_STATUS:	
			filt->m_MakeStatus = atoi(space);	
			filt->m_Actions[i++] = ID_FLT_STATUS;
			break;
		case IDS_FIO_LOWER:		
			filt->SetLower();	
			filt->m_MakePriority = PRIOR_LOWER;	
			filt->m_Actions[i++] = ID_FLT_PRIORITY; 
			break;
		case IDS_FIO_RAISE:		
			filt->SetRaise();	
			filt->m_MakePriority = PRIOR_RAISE;	
			filt->m_Actions[i++] = ID_FLT_PRIORITY;	
			break;
		case IDS_FIO_SKIP:		
			filt->m_SkipRest = TRUE;	
			filt->m_Actions[i++] = ID_FLT_SKIP_REST; 
			break;
		case IDS_FIO_PRINT:		
			filt->m_Print = TRUE;	
			filt->m_Actions[i++] = ID_FLT_PRINT; 
			break;
			
		case IDS_FIO_SOUND:
			filt->m_Actions[i] = ID_FLT_SOUND;
			filt->m_Sound[i] = space;
			filt->m_PlaySound[i++] = TRUE;
			break;
		case IDS_FIO_SPEAK:
			filt->m_Actions[i] = ID_FLT_SPEAK;
			ptr = const_cast<char *>(space);
			while( *ptr != ' ')
				ptr++;
			*ptr = 0; ptr++; //truncate and point to the next part of the string

			filt->m_SpeakOptions = atoi(space);
			::TrimWhitespaceMT(ptr);
			filt->m_strVoiceGUID = ptr;
			i++;
			break;
		
		case IDS_FIO_NOTIFY_APP:
			filt->m_Actions[i] = ID_FLT_NOTIFY_APP;
			filt->m_NotifyApp[i] = space;
			filt->m_DoNotifyApp[i++] = TRUE;
			break;
		case IDS_FIO_COPY:
			{
				filt->m_Actions[i] = ID_FLT_COPY;
				filt->m_CopyTo[i] = EudoraDir + space;
				filt->m_DoCopyTo[i] = TRUE;
				QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( filt->m_CopyTo[i] );

				if( pCommand )
				{
					switch (GetIniShort(IDS_INI_FILTER_TRANSFER_NAME)) //seanr
					{
						case 1: // Full path using "Folder \ Folder \ Mailbox" style
						{
							filt->m_Desc[i] = g_theMailboxDirector.BuildNamedPath(pCommand);
							break;
						}


						case 2:	// Full path using ms-dos style
						{
							filt->m_Desc[i] = EudoraDir + space;
							break;
						}

						case 3: // One parent deep style (per sdorner request) "Folder\Mailbox" or just "Mailbox" if root
						{
								

							if (g_theMailboxDirector.FindParent(pCommand))  //Make sure we have a parent
							{
								filt->m_Desc[i] = (g_theMailboxDirector.FindParent(pCommand))->GetName();
								filt->m_Desc[i] += "\\";
								filt->m_Desc[i] += pCommand->GetName();
									
							}

							else // No parent?  Cool, just get the name then.
							{
								filt->m_Desc[i] = pCommand->GetName();
							}

							break;
						} 
						
						case 0: // Default path using GetName() or "Mailbox" style
						default:
						{
							filt->m_Desc[i] = pCommand->GetName();
							break;
						}
					}			
				}
				else
				{
					// This filter action points to a mailbox that does not exist, so add it to the list
					MissingMailboxFilter.Add(filt);
					MissingMailboxIndex.Add(i);
				}
				i++;
				break;
			}
		case IDS_FIO_TRANSFER_TO:
		case IDS_FIO_COPY_TO:
			{
				filt->SetTransfer();
				if (Keyword == IDS_FIO_TRANSFER_TO)
				{
					filt->SetTransferTo();
					filt->m_Actions[i] = ID_FLT_TRANSFER;
				}
				else
				{
					// This looks very strange. Why are we setting it to ID_FLT_COPY but not
					// setting m_CopyTo[] ? This looks like legacy code that never gets executed. smanjo, 3/12/98
					filt->m_Actions[i] = ID_FLT_COPY;
					filt->m_DoCopyTo[i] = TRUE;
				}

				filt->m_Mailbox = EudoraDir + space;
				QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname( filt->m_Mailbox );

				if( pCommand )
				{
					if (Keyword == IDS_FIO_TRANSFER_TO)
					{
						switch (GetIniShort(IDS_INI_FILTER_TRANSFER_NAME)) //seanr
						{
							case 1: // Full path using "Folder \ Folder \ Mailbox" style
							{
								filt->m_Desc[i] = g_theMailboxDirector.BuildNamedPath(pCommand);
								break;
							}


							case 2:	// Full path using ms-dos style
							{
								filt->m_Desc[i] = filt->m_Mailbox;
								break;
							}

							case 3: // One parent deep style (per sdorner request) "Folder\Mailbox" or just "Mailbox" if root
							{
								

								if (g_theMailboxDirector.FindParent(pCommand))  //Make sure we have a parent
								{
									filt->m_Desc[i] = (g_theMailboxDirector.FindParent(pCommand))->GetName();
									filt->m_Desc[i] += "\\";
									filt->m_Desc[i] += pCommand->GetName();
									
								}

								else // No parent?  Cool, just get the name then.
								{
									filt->m_Desc[i] = pCommand->GetName();
								}

								break;
							} 


							case 0: // Default path using GetName() or "Mailbox" style
							default:
							{
								filt->m_Desc[i] = pCommand->GetName();
								break;
							}
						}
					}
				}
				else
				{
					// This filter action points to a mailbox that does not exist, so add it to the list
					MissingMailboxFilter.Add(filt);
					MissingMailboxIndex.Add(i);
				}
				i++;
			}
			break;
			
		case IDS_FIO_PERSONALITY:
			filt->m_DoPersonality = TRUE;
			filt->m_Personality = space; 
			filt->m_Actions[i++] = ID_FLT_PERSONALITY; 
			break;
		case IDS_FIO_SUBJECT:
			filt->m_ChangeSubject = TRUE;
			filt->m_MakeSubject = space; 
			filt->m_Actions[i++] = ID_FLT_SUBJECT; 
			break;
		case IDS_FIO_FORWARD:
			filt->m_DoForward[i] = TRUE;
			filt->m_Forward[i] = space; 
			filt->m_Actions[i++] = ID_FLT_FORWARD; 
			break;
		case IDS_FIO_REDIRECT:
			filt->m_DoRedirect[i] = TRUE;
			filt->m_Redirect[i] = space; 
			filt->m_Actions[i++] = ID_FLT_REDIRECT; 
			break;
		case IDS_FIO_REPLY:
			filt->m_DoReply[i] = TRUE;
			filt->m_Reply[i] = space; 
			filt->m_Actions[i++] = ID_FLT_REPLY; 
			break;
			break;
		case IDS_FIO_OPEN: 
			filt->m_OpenMM = atoi(space); 
			filt->m_Actions[i++] = ID_FLT_OPEN; 
			break;
		case IDS_FIO_NOTIFY_USER:
			filt->m_NotfyUserNR = atoi(space); 
			filt->m_Actions[i++] = ID_FLT_NOTIFY_USER; 
			break;
		case IDS_FIO_SERVER_OPT:
			filt->m_ServerOpt = atoi(space); 
			filt->m_Actions[i++] = ID_FLT_SERVER_OPT; 
			break;
		case IDS_FIO_HEADER:
			if (!FoundConj)
			{
				filt->m_Header[0] = space;
				int i = FindCStringArrayIndex(NumHeaders, HeaderStrings, filt->m_Header[0]);
				if (i >= 0)
					filt->m_Header[0].LoadString(IDS_FHEADER_ANY + i);
			}
			else
			{
				filt->m_Header[1] = space;
				int i = FindCStringArrayIndex(NumHeaders, HeaderStrings, filt->m_Header[1]);
				if (i >= 0)
					filt->m_Header[1].LoadString(IDS_FHEADER_ANY + i);
			}
			break;
			
		case IDS_FIO_VERB:
			{
				// match correct strings
				int index = FindCStringArrayIndexI(NumVerbs, VerbStrings, space);
				if (index < 0)
					index = 0;
				if (!FoundConj)
					filt->m_Verb[0] = index;
				else
					filt->m_Verb[1] = index;
			}
			break;
			
		case IDS_FIO_VALUE:
			if (!FoundConj)
				filt->m_Value[0] = space;
			else
				filt->m_Value[1] = space;
			break;
			
		case IDS_FIO_CONJUNCTION:
			FoundConj = TRUE;
			filt->m_Conjunction = FindCStringArrayIndexI(NumConjunctions, JunctStrings, space);
			if (filt->m_Conjunction < 0)
				filt->m_Conjunction = 0;
			break;
		}

	}
	filtFile.Close();

	// Make sure the two arrays are the same size
	ASSERT(MissingMailboxFilter.GetSize() == MissingMailboxIndex.GetSize());

	// Check if we have any actions with missing mailboxes
	if (MissingMailboxFilter.GetSize() > 0)
	{
		BOOL bFix = false;

		// Ask the user if we should fix the problem actions (if we are doing the checks)
		if (bDoChecks)
			bFix = (YesNoDialogPlain(IDS_FIXMAILBOX_FILTER, ((int) MissingMailboxFilter.GetSize())) == IDOK);

		// When bDoChecks == false we continue as if the user said NO (we don't change the filters)

		CFilter *pFilter;
		int nIndex;

		// Loop through all the problem actions
		for (int i=0; i < (MissingMailboxFilter.GetSize()); i++)
		{
			// Get the filter ptr and action index
			pFilter = MissingMailboxFilter.GetAt(i);
			nIndex = MissingMailboxIndex.GetAt(i);

			if (bFix)
			{
				// Really fix it; change bad mailbox references to In box

				CTocDoc	*InTOC = ::GetInToc();
				if (pFilter->m_Actions[nIndex] == ID_FLT_TRANSFER)
				{
					pFilter->m_Mailbox = InTOC->MBFilename();
				}
				else // COPY
				{
					ASSERT(pFilter->m_Actions[nIndex] == ID_FLT_COPY);
					pFilter->m_CopyTo[ nIndex ] = InTOC->MBFilename();
				}

				pFilter->m_Desc[nIndex] = CRString( IDS_IN_MBOX_NAME );
			}
			else // !bFix
			{
				// Leave broken, change display text only

				CString str, sMissing = CRString(IDS_MISSINGMAILBOX_STR);

				// Format the string to include the mailbox name that is missing
				if (pFilter->m_Actions[nIndex] == ID_FLT_TRANSFER)
				{
					str.Format(sMissing, LPCSTR(pFilter->m_Mailbox));
				}
				else // COPY
				{
					ASSERT(pFilter->m_Actions[nIndex] == ID_FLT_COPY);
					str.Format(sMissing, LPCSTR(pFilter->m_CopyTo[nIndex]));
				}

				// Change the description string to indicate a missing mailbox (ie "<<Missing mailbox: c:\mail\test.mbx>>")
				pFilter->m_Desc[nIndex] = str;
			}
		}

		// If we fixed the problem, save it to disk
		if (bFix)
			Write();
	}

	if ((bDoChecks) && (m_Filters.GetCount() && Version < FiltVersion))
		UpdateFilterVersion(); // Add skip rest for older versions (if user wants)

	UpdateAllViews(NULL, FUT_REFRESH);

	SetModifiedFlag(FALSE);
	return (TRUE);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFiltersDoc::UpdateFilterVersion()
{
	BOOL GOT_XFER = FALSE;
	POSITION pos = m_Filters.GetHeadPosition();
	int gap = -1;

	if (AlertDialog(IDD_FILT_UPDATE) == IDOK)
	{
		while (pos)
		{
			GOT_XFER = FALSE;
			CFilter* filt = g_Filters->m_Filters.GetNext(pos);

			for (int i = 0; i < NUM_FILT_ACTS; i++)
			{
				// Figure if it's a xfer or a copy
				if (filt->m_Actions[i] == ID_FLT_TRANSFER)
					GOT_XFER = TRUE;
				if (filt->m_Actions[i] == ID_FLT_NONE)
					gap = i;
				if (gap > 0 && GOT_XFER)
					break;
			}
			
			if (GOT_XFER && !filt->m_SkipRest && gap)
			{
				filt->m_SkipRest = TRUE;	
				filt->m_Actions[gap] = ID_FLT_SKIP_REST; 
			}
		}
	}
	Write();
	return (TRUE);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFiltersDoc::Write()
{
	CString TempName;
	JJFile out;
	int	fail = 0;
	char tmp[64];
	
	// Load some strings for better performance
	CRString Rule(IDS_FIO_RULE);
	CRString Incoming(IDS_FIO_INCOMING);
	CRString Outgoing(IDS_FIO_OUTGOING);
	CRString Manual(IDS_FIO_MANUAL);
	CRString Label(IDS_FIO_LABEL);
	CRString Raise(IDS_FIO_RAISE);
	CRString Lower(IDS_FIO_LOWER);
	CRString TransferTo(IDS_FIO_TRANSFER_TO);
	CRString CopyTo(IDS_FIO_COPY_TO);
	CRString Personality(IDS_FIO_PERSONALITY);
	CRString Subject(IDS_FIO_SUBJECT);
	CRString Header(IDS_FIO_HEADER);
	CRString Verb(IDS_FIO_VERB);
	CRString Value(IDS_FIO_VALUE);
	CRString Conjunction(IDS_FIO_CONJUNCTION);
	CRString None(IDS_FIO_NONE);
	CRString Status(IDS_FIO_STATUS);
	CRString Priority(IDS_FIO_PRIORITY);
	CRString Sound(IDS_FIO_SOUND);
	CRString Speak(IDS_FIO_SPEAK);
	CRString Open(IDS_FIO_OPEN);
	CRString Print(IDS_FIO_PRINT);
	CRString NotifyUser(IDS_FIO_NOTIFY_USER);
	CRString NotifyApp(IDS_FIO_NOTIFY_APP);
	CRString Forward(IDS_FIO_FORWARD);
	CRString Redirect(IDS_FIO_REDIRECT);
	CRString Reply(IDS_FIO_REPLY);
	CRString ServerOpt(IDS_FIO_SERVER_OPT);
	CRString Copy(IDS_FIO_COPY);
	CRString Transfer(IDS_FIO_TRANSFER);
	CRString Skip(IDS_FIO_SKIP);
	
	CString Verbs[NumVerbs];
	//LoadCStringArray(Verbs, IDS_FIO_CONTAINS, IDS_FIO_INTERSECTS);
	LoadCStringArray(Verbs, IDS_FIO_CONTAINS, IDS_FIO_REGEX);

	CString Conjunctions[NumConjunctions];
	LoadCStringArray(Conjunctions, IDS_FIO_IGNORE, IDS_FIO_UNLESS);
	
	CString HeaderStrings[NumHeaders];
	LoadCStringArray(HeaderStrings, IDS_FHEADER_ANY, IDS_FHEADER_PERSONALITY);

	CString ioHeader[2];
	
	TempName = TempDir + CRString(IDS_FIO_TEMP_FILENAME);
	if (FAILED(out.Open(TempName, O_CREAT | O_TRUNC | O_WRONLY)))
		return (FALSE);

	sprintf(tmp,"%d", FiltVersion); 
	if (FAILED(out.PutLine(tmp)))
		return (FALSE);

	POSITION pos = m_Filters.GetHeadPosition();
	char tempStr[64];
	while (!fail && pos)
	{
		CFilter *filt = m_Filters.GetNext(pos);
		
		// Rule name
		if (FAILED(out.Put(Rule))) fail = 1;
		if (!fail && FAILED(out.PutLine(filt->m_Name))) fail = 1;

		// Actions
		for (int i = 0;  i < NUM_FILT_ACTS; i++)
		{
			switch(filt->m_Actions[i])
			{
				case ID_FLT_NONE:
					break;
				case ID_FLT_SKIP_REST:
					if (!fail && filt->m_SkipRest && FAILED(out.PutLine(Skip))) fail = 1;
					break;
				case ID_FLT_PRINT:
					if (!fail && filt->m_Print && FAILED(out.PutLine(Print))) fail = 1;
					break;
				case ID_FLT_LABEL:
					if (!fail && filt->m_Label >= 0)
					{
						wsprintf(tempStr, "%s%d", (const char*)Label, filt->m_Label);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
				case ID_FLT_COPY:
					if (!fail)// && !filt->m_CopyTo[i].IsEmpty())
					{
						if ( ! filt->m_CopyTo[i].IsEmpty() )
						{
							if (FAILED(out.Put(Copy))) fail = 1;
							if (!fail && FAILED(out.PutLine(((const char*)filt->m_CopyTo[i]) + EudoraDirLen))) fail = 1;
						}
					}
					break;
				case ID_FLT_TRANSFER:
					if (!fail && filt->IsTransfer())
					{
						if ( ! filt->m_Mailbox.IsEmpty() )
						{
							if (filt->IsTransferTo() && FAILED(out.Put(TransferTo))) fail = 1;
							if (!fail && FAILED(out.PutLine(((const char*)filt->m_Mailbox) + EudoraDirLen))) fail = 1;
						}
					}
					break;
				case ID_FLT_PERSONALITY:
					if (!fail )
					{
						if (!fail && FAILED(out.Put(Personality))) fail = 1;
						if (!fail && FAILED(out.PutLine(filt->m_Personality))) fail = 1;
					}
					break;
				case ID_FLT_SUBJECT:
					if (!fail )//&& !filt->m_MakeSubject.IsEmpty())
					{
						if (!fail && FAILED(out.Put(Subject))) fail = 1;
						if (!fail && FAILED(out.PutLine(filt->m_MakeSubject))) fail = 1;
					}
					break;
				case ID_FLT_FORWARD:
					if (!fail )//&& !filt->m_Forward[i].IsEmpty())
					{
						if (!fail && FAILED(out.Put(Forward))) fail = 1;
						if (!fail && FAILED(out.PutLine(filt->m_Forward[i]))) fail = 1;
					}
					break;
				case ID_FLT_REDIRECT:
					if (!fail && !filt->m_Redirect[i].IsEmpty())
					{
						if (!fail && FAILED(out.Put(Redirect))) fail = 1;
						if (!fail && FAILED(out.PutLine(filt->m_Redirect[i]))) fail = 1;
					}
					break;
				case ID_FLT_REPLY:
						if (!fail && !filt->m_Reply[i].IsEmpty())
					{
						if (!fail && FAILED(out.Put(Reply))) fail = 1;
						if (!fail && FAILED(out.PutLine(filt->m_Reply[i]))) fail = 1;
					}
					break;
				case ID_FLT_OPEN:
					if (!fail && filt->m_OpenMM >= 0)
					{
						wsprintf(tempStr, "%s%d", (const char*)Open, filt->m_OpenMM);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
				case ID_FLT_NOTIFY_USER:
					if (!fail && filt->m_NotfyUserNR >= 0)
					{
						wsprintf(tempStr, "%s%d", (const char*)NotifyUser, filt->m_NotfyUserNR);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
				case ID_FLT_SERVER_OPT:
					if (!fail && filt->m_ServerOpt >= 0)
					{
						wsprintf(tempStr, "%s%d", (const char*)ServerOpt, filt->m_ServerOpt);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
					break;
				case ID_FLT_PRIORITY:
					if (!fail && filt->m_MakePriority >= 0)
					{
						wsprintf(tempStr, "%s%d", (const char*)Priority, filt->m_MakePriority);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
				case ID_FLT_STATUS:
					if (!fail && filt->m_MakeStatus >= 0)
					{
						wsprintf(tempStr, "%s%d", (const char*)Status, filt->m_MakeStatus);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
				case ID_FLT_SOUND:
					if (!fail)
					{
						if (FAILED(out.Put(Sound))) fail = 1;
						if (!fail && FAILED(out.PutLine((const char*)filt->m_Sound[i]))) fail = 1;
					}
					break;
				case ID_FLT_SPEAK:
					if (!fail && filt->m_SpeakOptions >= 0)
					{
						wsprintf(tempStr, "%s%d %s", (const char*)Speak, filt->m_SpeakOptions, filt->m_strVoiceGUID);
						if (FAILED(out.PutLine(tempStr))) fail = 1;
					}
					break;
				
				case ID_FLT_NOTIFY_APP:
					if (!fail)
					{
						if (FAILED(out.Put(NotifyApp))) fail = 1;
						if (!fail && FAILED(out.PutLine((const char*)filt->m_NotifyApp[i]))) fail = 1;
					}
					break;
			}
		}
		// When to apply the filter
		if (!fail && filt->IsIncoming() && FAILED(out.PutLine(Incoming))) fail = 1;
		if (!fail && filt->IsOutgoing() && FAILED(out.PutLine(Outgoing))) fail = 1;
		if (!fail && filt->IsManual() && FAILED(out.PutLine(Manual))) fail = 1;

		// First Condition

	// Header
		int j = FindCStringArrayIndex(NumHeaders, HeaderStrings, filt->m_Header[0]);
		if (j >= 0)
			ioHeader[0].LoadString(IDS_FIO_ANY_HEADER + j);
		else 
			ioHeader[0] = filt->m_Header[0];

		if (!fail && FAILED(out.Put(Header))) fail = 1;
		if (!fail && FAILED(out.PutLine(ioHeader[0]))) fail = 1;

		// Verb
		if (!fail && FAILED(out.Put(Verb))) fail = 1;
		if (!fail && FAILED(out.PutLine(Verbs[filt->m_Verb[0]]))) fail = 1;

		// Value
		if (!fail && FAILED(out.Put(Value))) fail = 1;
		if (!fail && FAILED(out.PutLine(filt->m_Value[0]))) fail = 1;

		// Conjunction
		if (!fail && FAILED(out.Put(Conjunction))) fail = 1;
		if (!fail && FAILED(out.PutLine(Conjunctions[filt->m_Conjunction]))) fail = 1;

		// Second Condition
		// Header
		j = FindCStringArrayIndex(NumHeaders, HeaderStrings, filt->m_Header[1]);
		if (j >= 0)
			ioHeader[1].LoadString(IDS_FIO_ANY_HEADER + j);
		else
			ioHeader[1] = filt->m_Header[1];

		if (!fail && FAILED(out.Put(Header))) fail = 1;
		if (!fail && FAILED(out.PutLine(ioHeader[1]))) fail = 1;

		// Verb
		if (!fail && FAILED(out.Put(Verb))) fail = 1;
		if (!fail && FAILED(out.PutLine(Verbs[filt->m_Verb[1]]))) fail = 1;

		// Value
		if (!fail && FAILED(out.Put(Value))) fail = 1;
		if (!fail && FAILED(out.PutLine(filt->m_Value[1]))) fail = 1;
	}
	if (!fail && FAILED(out.Close())) fail = 1;
	if (!fail && FAILED(out.Rename(m_strPathName))) fail = 1;

	if (!fail)
	{
		SetModifiedFlag(FALSE);

		return (TRUE);
	}
//fail:
	out.Delete();
	return (FALSE);
}


BEGIN_MESSAGE_MAP(CFiltersDoc, CDoc)
	//{{AFX_MSG_MAP(CFiltersDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFiltersDoc commands


/////////////////////////////////////////////////////////////////////////////
CStringCountList::~CStringCountList()
{
	RemoveAll();
}


void CStringCountList::RemoveAll()
{
	while (!IsEmpty())
	{
		CStringCount* Head = RemoveHead();
		delete Head;
	}
}

/////////////////////////////////////////////////////////////////////////////
CStringCount* CStringCountList::Add(const char* Name )
{
	POSITION pos = GetHeadPosition();

	while (pos)
	{
		CStringCount* sc = GetNext(pos);
		
		if (sc && sc->m_Name.CompareNoCase(Name) == 0)
		{
			sc->m_Count++;
			return (sc); 
		}
	}

	CStringCount* sc = new CStringCount(Name);
	
	if (sc)
		AddHead(sc);
	return (sc);

}



/////////////////////////////////////////////////////////////////////////////
CFilterActions::CFilterActions()
{
}

/////////////////////////////////////////////////////////////////////////////
void CFilterActions::ClearAllLists()
{
	m_FilterTocList.RemoveAll();
	m_NotifyReportList.RemoveAll();
	m_FilterOpenMBoxTocList.RemoveAll();
	m_FilterOpenSumList.RemoveAll();
	m_FilterPrintSum.RemoveAll();
	m_SoundList.RemoveAll();
	m_NotifyApp.RemoveAll();	
}


/////////////////////////////////////////////////////////////////////////////
CFilterActions::~CFilterActions()
{
	ClearAllLists();
}


// HasFilterContext
//
// Returns whether or not the given filter set has at
// least one filter for the given context.
//
BOOL CFilterActions::HasFilterContext(FILTER_FILE_TYPE ffType, const int WhenToApply)
{
	ASSERT(g_Filters && g_Filters->HasDoneInit());

	CFilterList *pFiltList = NULL;
	if ( ffType == FF_PRE_FILTER )
		pFiltList = &(g_Filters->m_PreFilters);
	else if( ffType == FF_MAIN_FILTER )
		pFiltList = &(g_Filters->m_Filters);
	else if( ffType == FF_POST_FILTER )
		pFiltList = &(g_Filters->m_PostFilters);
	else
	{
		ASSERT(0);
		return FALSE;
	}

	ASSERT(	WhenToApply == WTA_INCOMING ||
			WhenToApply == WTA_OUTGOING ||
			WhenToApply == WTA_MANUAL);

	POSITION pos = pFiltList->GetHeadPosition();
	while (pos)
	{
		CFilter* filt = pFiltList->GetNext(pos);
		if (filt->m_WhenToApply & WhenToApply)
			return TRUE;
	}

	return FALSE;
}

// StartsFiltering
//
// Starts the filtering process
//
BOOL CFilterActions::StartFiltering(const int WhenToApply/* = 0*/)
{

#ifdef EXPIRING
	if ( g_TimeStamp.IsExpired1() )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
		return FALSE;
	}
#endif 

	ASSERT(g_Filters);

	if (!g_Filters)
		return FALSE;
	
	g_Filters->DoOneTimeInitIfNotDoneYet();
	
	if (WhenToApply != 0)
	{
		if (!HasFilterContext(FF_PRE_FILTER, WhenToApply) &&
			!HasFilterContext(FF_MAIN_FILTER, WhenToApply) &&
			!HasFilterContext(FF_POST_FILTER, WhenToApply))
		{
			return FALSE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// FilterOne
//
/////////////////////////////////////////////////////////////////////////////
int CFilterActions::FilterOne(CSummary* Sum, int WhenToApply)
{
	ASSERT(g_Filters && g_Filters->HasDoneInit());

	if (!Sum)
		return (FALSE);

	int filtResult = g_Filters->FilterMsg( Sum, WhenToApply, this, FF_PRE_FILTER );
	if( !( filtResult & FA_SKIPREST ) )
	{
		filtResult |= g_Filters->FilterMsg( Sum, WhenToApply, this, FF_MAIN_FILTER );
		if( !( filtResult & FA_SKIPREST ) )
		{
			filtResult |= g_Filters->FilterMsg( Sum, WhenToApply, this, FF_POST_FILTER );
		}
	}
	return filtResult;
}


#include "msgframe.h"
/////////////////////////////////////////////////////////////////////////////
// EndFiltering
//
// Finishes up the filtering after all messages have been processed
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFilterActions::EndFiltering()
{
	ASSERT(g_Filters && g_Filters->HasDoneInit());

	BOOL Success = TRUE;
	BOOL Normal = FALSE;
	
	// Open up messages that have been filtered	with "Open Message"
	POSITION pos = m_FilterOpenSumList.GetHeadPosition();
	while (pos)
	{
		CSummary* sum = m_FilterOpenSumList.GetNext(pos);
		if (sum)
		{
			sum->DisplayBelowTopMostMDIChild();
		
			if (sum->m_State == MS_UNREAD)
				sum->SetState(MS_READ);
		}
	}

	// Turn redraw back on for all mailboxes, which will get the ones that
	// temporarily had redraw turned off as a result of a transfer
	pos = TocTemplate->GetFirstDocPosition();
	while (pos)
	{
		CTocDoc* toc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (toc->GetView())
			toc->GetView()->m_SumListBox.SetRedraw(TRUE);
	}

	// Open up mailboxes that have been filtered with "Open Message"
	pos = m_FilterOpenMBoxTocList.GetHeadPosition();
	while (pos)
	{
		// Open up the mailbox if it isn't open already (except Trash),
		CTocDoc* toc = m_FilterOpenMBoxTocList.GetNext(pos);
		
		
		
		if (toc->m_Type != MBT_TRASH)
			toc->DisplayBelowTopMostMDIChild();//toc->Display();

		if (toc->GetView())
			toc->GetView()->Fumlub();
	}

	// Open up mailboxes that had message filtered into them
	pos = m_FilterTocList.GetHeadPosition();
	while (pos)
	{
		// Open up the mailbox if it isn't open already (except Trash),
		CTocDoc* toc = m_FilterTocList.GetNext(pos);

		// 
		// Based on the settings, make sure that mailboxes with
		// newly-filtered mail are opened.
		//
		if (GetIniShort(IDS_INI_OPEN_IN_MAILBOX) && (toc->m_Type != MBT_TRASH))
			toc->DisplayBelowTopMostMDIChild();//toc->Display();

		//
		// If the mailbox was just opened or was already opened, then
		// be sure to scroll to the "First Unread Message of the Last
		// Unread Block" ... you know, Fumlub.
		// 
		if (toc->GetView())
			toc->GetView()->Fumlub();
	}
	
	// Dump Report
	pos = m_NotifyReportList.GetHeadPosition();
	if (pos)
	{
		// Find active Filter Report window, if any.
		CMainFrame* pMainFrame = (CMainFrame *) AfxGetMainWnd();
		ASSERT_KINDOF(CMainFrame, pMainFrame);
		CFilterReportView* pView = pMainFrame->GetActiveFilterReportView();
		if (pView)
		{
			// Do the time stamp thang
			time_t now;
			time( &now );
			while (pos)
			{
				CStringCount *mboxName = m_NotifyReportList.GetNext(pos);
				pView->AddEntry( mboxName->m_Count, (const char*)mboxName->m_Name, now );
			}
			if (GetIniShort(IDS_INI_FILTER_REPORT))
				pMainFrame->PostMessage(WM_COMMAND, IDM_VIEW_FILTER_REPORT);	// activate the report window
		}
	}


	//PlaySounds();
	pos = m_SoundList.GetHeadPosition();
	while (pos)
	{
		CString snd = m_SoundList.GetNext(pos);
		if (!snd.IsEmpty()) 
			PlaySound(snd, NULL, SND_SYNC | SND_FILENAME | SND_NODEFAULT);
		if (EscapePressed(1))
			break;
	}

	pos = m_NotifyApp.GetHeadPosition();
	while (pos)
	{
		CString cmdLine = m_NotifyApp.GetNext(pos);
		if (!cmdLine.IsEmpty())
			::WinExec( cmdLine, SW_SHOWNA);
		if (EscapePressed(1))
			break;
	}

	CloseProgress();
	// Print Summaries
	m_FilterPrintSum.Print();
 	
	// Clear out lists
	ClearAllLists();
	return (Normal);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFilterActions::ContainsProActions()
{
	// Shareware: Should only call this in REDUCED FEATURE mode
	if (UsingFullFeatureSet())
	{
		// FULL FEATURE mode
		ASSERT(0);
		return (FALSE);
	}
	
	// Make sure filters are loaded up
	if (!g_Filters)
	{
		if (!(g_Filters = (CFiltersDoc*)NewChildDocument(FiltersTemplate)))
			return (FALSE);
	}
	if (g_Filters->m_Filters.IsEmpty() && !g_Filters->Read())
	{
		g_Filters->OnCloseDocument();
		return (FALSE);
	}

	POSITION pos = g_Filters->m_Filters.GetHeadPosition();

	while (pos)
	{
		CFilter* filt = g_Filters->m_Filters.GetNext(pos);

		for (int i = 0; i < NUM_FILT_ACTS; i++)
		{
			// is it a pro action?
			switch ( filt->m_Actions[i] )
			{
				//Light actions
				//case ID_FLT_NONE:
				//case ID_FLT_PRIORITY:
				//case ID_FLT_SUBJECT:
				//case ID_FLT_COPY:
				//case ID_FLT_TRANSFER:
				//case ID_FLT_SKIP_REST:

				//Pro actions
				//case ID_FLT_STATUS:
				case ID_FLT_LABEL:
				case ID_FLT_PERSONALITY:
				case ID_FLT_SOUND:
				case ID_FLT_OPEN:
				case ID_FLT_PRINT:
				//case ID_FLT_NOTIFY_USER:
				//case ID_FLT_NOTIFY_APP:
				case ID_FLT_FORWARD:
				case ID_FLT_REDIRECT:
				case ID_FLT_REPLY:
				case ID_FLT_SERVER_OPT:
					return TRUE;
					break;
			}
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL FilterMBoxLink(const char *MailboxName, BOOL IsFolder)
{
	char mboxName[256];
	int len = strlen(MailboxName);
	CFilterActions filtAct;
	filtAct.StartFiltering();
	POSITION pos = g_Filters->m_Filters.GetHeadPosition();
	
	while (pos)
	{
		CFilter* filt = g_Filters->m_Filters.GetNext(pos);
		
		for (int i = 0; i < NUM_FILT_ACTS; i++)
		{
			// Figure if it's a xfer or a copy
			if (filt->m_Actions[i] == ID_FLT_TRANSFER)
				strcpy(mboxName,filt->m_Mailbox);
			else if (filt->m_Actions[i] == ID_FLT_COPY)
				strcpy(mboxName, filt->m_CopyTo[i]);
			else
				continue;

			if (IsFolder)
				mboxName[len] = 0;

			if (!stricmp(mboxName, MailboxName))
				return TRUE;
		}
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
void FiltersUpdateMailbox( const char *OldName, const char *NewName, const char *NewMenuName, BOOL IsFolder)
{
	int ChangeMade = FALSE;
 	char mboxName[256];
	int len = strlen(OldName);
	BOOL xfer = TRUE;
 	CFilterActions filtAct;

	filtAct.StartFiltering();

	POSITION pos = g_Filters->m_Filters.GetHeadPosition();

	while (pos)
	{
		CFilter* filt = g_Filters->m_Filters.GetNext(pos);

		for (int i = 0; i < NUM_FILT_ACTS; i++)
		{
			// Figure if it's a xfer or a copy
			if (filt->m_Actions[i] == ID_FLT_TRANSFER)
			{
				strcpy(mboxName,filt->m_Mailbox);
				xfer = TRUE;
			}
			else if (filt->m_Actions[i] == ID_FLT_COPY)
			{
				strcpy(mboxName, filt->m_CopyTo[i]);
				xfer = FALSE;
			}
			else
				continue;

			// terminate if folder so compare to where it was
			if (IsFolder)
				mboxName[len] = 0;

			// did it change
			if (!stricmp(mboxName, OldName))
			{
				if (!IsFolder)
				{
					if (NewName && xfer)
						filt->m_Mailbox = NewName;
					else if (NewName)
						filt->m_CopyTo[i] = NewName;
					filt->m_Desc[i] = NewMenuName;
				}
				else
				{
					char tmp[255];  
					sprintf(tmp,"%s\\%s",NewName,mboxName+len+1);
					if (xfer)
						filt->m_Mailbox = tmp; 
					else
						filt->m_CopyTo[i] = tmp;
				}
				ChangeMade = TRUE;
			}
		}
	}

	if (ChangeMade)
	{
		g_Filters->Write();
		// update the filters view if its up
		g_Filters->UpdateAllViews(NULL,FUT_UPDATEVIEW);
	}
}
