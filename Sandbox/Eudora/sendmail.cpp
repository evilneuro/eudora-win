// SENDMAIL.CPP
//
// Sendmail/SMTP routines
//

#include "stdafx.h"

#include "QCWorkerSocket.h"		
#include "QCFileIO.h"          //CFileIO
#include "summary.h"	// CSummary
#include "sendmail.h"
#include "progress.h"
#include "resource.h"
#include "rs.h"
#include "compmsgd.h"
#include "utils.h"
#include "guiutils.h"
#include "Base64.h"
#include "QP.h"
#include "MIMEMap.h"
#include "Encode.h"
#include "uudecode.h"
#include "binhex.h"
#include "address.h"
#include "nickdoc.h"
#include "password.h"
#include "etf2html.h"
#include "Text2Html.h"
#include "Html2Text.h"
#include "trnslate.h"
#include "tocdoc.h"
#include "tocview.h"

// The following are all for SpoolOne/SendQMsgs2
#include "msgframe.h"	// CmessageDoc
#include "eudora.h"		// Add GetTrans()
#include "persona.h"	// Add g_Personalities
#include "QCWorkerThreadMT.h"
#include "QCTaskManager.h"
#include "SpoolMgr.h"	// QCSpoolMgrMT
#include "QCSMTPSettings.h"
#include "SMTPSession.h" // QCSMTPThreadMT

// The following are all for ProcessSpooledMsgs
#include "filtersd.h"
#include "QCMailBoxCommand.h"
#include "QCMailboxDirector.h"  //for g_theMailboxDirector

#include "QCSharewareManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define		PREFERED_BUFFER_SIZE	10000
#define		MINIMUM_BUFFER_SIZE		2048

// Statics
static CCompMessageDoc* SMTPmsg;
static char*	Signature;
static char*	buf;
static LONG		lBufSize;

void DisplayError(const char* szFilename);
void DoFCC(CTocDoc* OutToc, CSummary *Sum, char *bccheader);


void PostProcessOutgoingMessages(void *pv, bool bEnteringLastTime)
{
	ASSERT(::IsMainThreadMT());

	QCSMTPThreadMT *smtpThread = static_cast<QCSMTPThreadMT *>(pv);
	
	CString szFilename;
	CSummary* Sum;

	//
	// Initialize Persona list, Out TOC pointer and filters
	//
	CString	homie =	g_Personalities.GetCurrent();

	CTocDoc* OutToc = GetOutToc();
	if (!OutToc)
		return ; //return E_FAIL;


	CFilterActions filt;
	BOOL bDoFilters = FALSE;
	if (filt.StartFiltering(WTA_OUTGOING))
	{
		//	display progress w/ count of filterable items(QCSpoolMgrMT::GetFilterableMsgCount()
		//Progress("Filtering Sent Messages");
		Progress(CRString(IDS_FILTERING_SENT_MESSAGES));
		bDoFilters = TRUE;
	}

	CString strPersona = smtpThread->GetPersona();
	
	//  Grab the Spool Manager associated with this peronsa
	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(strPersona);
	COutSpoolInfo *pSpoolInfo = NULL;
	
	BOOL bQueued = FALSE;

	int nTaskID = (smtpThread->GetTaskInfo())->GetUID();

	//Cycle through the SENT Spool grabbing SNT,UNS and UNL files
	while( pSpoolInfo = SpoolMgr->m_Out->Get(SS_DONE_SEND, SS_REMOVAL_INPROGRESS, nTaskID) )
	{
		szFilename = pSpoolInfo->GetFileName();
		//Sum = GetSummaryByUniqueID(pSpoolInfo->GetUniqueID());
		Sum = pSpoolInfo->GetSummary();
		SPOOLTYPE spType = pSpoolInfo->GetSpoolType();

		switch ( spType )
		{
		case SPOOL_TYPE_SENT:
			
			SetIniShort(IDS_INI_SUCCESSFUL_SEND, 1);
			
			ASSERT(Sum != NULL);
			if(Sum)
			{
				Sum->SetState(MS_SENT);
				if (OutToc->GetView())
					OutToc->GetView()->UpdateWindow();
				
				//do we have s doc opened? if not open one
				BOOL bCreatedDoc = FALSE;
				CCompMessageDoc* doc = NULL;

				if (!(doc = (CCompMessageDoc*)Sum->FindMessageDoc()))
				{
					bCreatedDoc = TRUE;
					//doc = (CCompMessageDoc*)Sum->GetMessageDoc();
					doc = dynamic_cast<CCompMessageDoc*>(Sum->GetMessageDoc());
				}
				ASSERT(doc->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)));
				

				CString AttachStr;
				char *bcc_header = NULL;
				bool bDeleteAttachments = false;
				if ((Sum->IsMAPI() && GetIniShort(IDS_INI_MAPI_DELETE_SEND)) ||
					(Sum->IsAutoAttached() && GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_SEND)))
				{
					bDeleteAttachments = true;
				}

				if (doc && doc->GetText())
				{
					if(bDeleteAttachments)
						AttachStr = doc->GetHeaderLine(HEADER_ATTACHMENTS);
					bcc_header = (char *)::SafeStrdupMT(doc->GetHeaderLine(HEADER_BCC));
				}
				if (doc && bCreatedDoc)
					Sum->NukeMessageDocIfUnused();

				//do fcc here
				DoFCC(OutToc, Sum, bcc_header);
				
				// Do filters
				if ( (!bDoFilters || !(filt.FilterOne(Sum, WTA_OUTGOING) & FA_TRANSFER )) &&
					 !Sum->KeepCopies())
				{
					OutToc->Xfer(GetTrashToc(), Sum, TRUE);
				}
				OutToc->SetModifiedFlag();

				if(!doc){
					ASSERT(0);
					return;
				}
				
				
				// Delete attachments that should be deleted after sending
				//if ((Sum->IsMAPI() && GetIniShort(IDS_INI_MAPI_DELETE_SEND)) ||
				//	(Sum->IsAutoAttached() && GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_SEND)))
				if(bDeleteAttachments)
				{
					//CString AttachStr(doc->GetHeaderLine(HEADER_ATTACHMENTS));
					char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
					char AttachDir[_MAX_PATH + 1];
					int ADLen;

					GetIniString(IDS_INI_AUTO_RECEIVE_DIR, AttachDir, sizeof(AttachDir));
					if (!*AttachDir)
						wsprintf(AttachDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_ATTACH_FOLDER));
					ADLen = strlen(AttachDir);

					while (Attach && *Attach)
					{
						char *t = strchr(Attach, ';');
						if (t)
						{
							*t++ = 0;
							if (*t == ' ')
								t++;
						}
						
						if (strnicmp(AttachDir, Attach, ADLen) == 0)
							::FileRemoveMT(Attach);

						Attach = t;
					}
				}

				//DoFCC(OutToc, Sum, bcc_header);
				

			}
			break;

		case SPOOL_TYPE_UNSENDABLE:
			if(Sum)
			{
				Sum->SetState(MS_UNSENDABLE);
				//Sum->Display();
				//DisplayError(szFilename);
			}
			
			break;
		case SPOOL_TYPE_UNSENT:
			// Do I want to change state here?

			//FORNOW @@@@@@@@@
			if(Sum)
			{
				Sum->SetState(MS_QUEUED);
				bQueued = TRUE;
			}
			break;
		
		case SPOOL_TYPE_SEND:
			//If a sending thread is stopped, all spooled files should be moved back to the QUEUED state
			if(Sum)
			{
				Sum->SetState(MS_QUEUED);
				bQueued = TRUE;
			}
			break;
		
		default:
			ASSERT(0);
			break;
		
		}
	
		//  Done with spool file so delete it.
		pSpoolInfo->SetSpoolStatus(SS_REMOVED);
		SpoolMgr->m_Out->RemoveFromSpool(pSpoolInfo);
	}

	if (bDoFilters)
		filt.EndFiltering();

	if(bQueued)
		SetQueueStatus();

	g_Personalities.SetCurrent( homie );

	CloseProgress();


	return;

}



void DoFCC(CTocDoc* OutToc, CSummary *Sum, char *OldLine)
{
	extern QCMailboxDirector	g_theMailboxDirector;

	// Shareware: Only allow FCC in FULL FEATURE version.
	if (!UsingFullFeatureSet())
	{
		ASSERT(0);
		return;
	}

	// FULL FEATURE mode

	// save the BCC line for processessing	
	//char *OldLine = (char *)::SafeStrdupMT(doc->GetHeaderLine(HEADER_BCC));

	//FCC Stuff
	if (OldLine)
	{
		int len = ::SafeStrlenMT(OldLine);
		CString sOldLine(OldLine);
		CString sMbox;
		BOOL endOfLine=FALSE;
		int ch=131;
		int beginPos=0;
		int endPos=0;
		BOOL bFound=FALSE;
		CString sTemp;
		CString sPath;
		CString sName;

		

		while (!endOfLine)
		{
			ch = 131;

			beginPos = sOldLine.Find((unsigned char)ch);
			if (beginPos == -1)
			{
				endOfLine=TRUE;
				continue;
			}
			beginPos++;
			ch = sOldLine[beginPos];
			beginPos++;
			sOldLine = sOldLine.Mid(beginPos);
			endPos = sOldLine.Find(',');
			if (endPos == -1)
			{
				endOfLine=TRUE;
				sMbox = sOldLine;
			}
			else
			{
				sMbox = sOldLine.Mid(0, endPos);
			}
			QCMailboxCommand* pCommand = g_theMailboxDirector.FindByNamedPath( sMbox );

			if( pCommand != NULL )
			{
				OutToc->Xfer(	GetToc(	pCommand->GetPathname(), 
										pCommand->GetName(),
										FALSE, 
										FALSE), 
								Sum, 
								FALSE, 
								TRUE);					
			}
			else
			{
				char sText[256];
				sprintf(sText, CRString(IDS_FCC_LOST_MBOX), sMbox);
				AfxMessageBox(sText);
			}
		}
		delete [] OldLine;

		//if (doc && bCreatedDoc)
		//	Sum->NukeMessageDocIfUnused();

	}
}



// SMTPCmdGetReply
//
// Send a command to the server and get a reply
//
int SMTPCmdGetReply(UINT cmd, const char* arg, char* buf, int size,
	BOOL ShowProgress = TRUE, BOOL HandleError = FALSE, BOOL IgnoreError = FALSE)
{
	char cmdbuf[256];
	int status;

	// Create the command to send to the server
	strcpy(cmdbuf, CRString(cmd));
	if (arg)
		strcat(cmdbuf, arg);

	if (ShowProgress)
		Progress(cmdbuf);

	strcat(cmdbuf, "\r\n");

	// the command buffer is now ready, send the command to the server...
	status = NetConnection->PutDirect(cmdbuf);
	if (status < 0)
	{
		SMTPConnected = FALSE;
		NetConnection->Close();
		return (-1);
	}

/*
#ifndef THREADED

	while (1)
	{
		status = GetSMTPReply(buf, size);
		if (status < 0 || status / 100 >= 4)
		{
			if (IgnoreError)
				continue;

			if (!HandleError)
			{
				if (status >= 0)
					SMTPError(cmdbuf, buf);
				status = -1;
			}
		}
		break;
	}
#else
	status = 200;
#endif;
	*/
	
	status = 200;
	return (status);	
}


////////////////////////////////////////////////////////////////////////
// POPCmdGetReply_ [public]
//
// Takes an argument request, and a place to store the
// result and does the footwork
////////////////////////////////////////////////////////////////////////
int POPCmdGetReply(UINT uCmdID, const char* pszArgs, char* pszBuffer, int nBufferSize, 
								BOOL bReportError = TRUE )
{
	//
	// Format the command string, with optional arguments.  Update
	// the progress display with the outgoing command string.
	//
	char szCommand[128]; 
	strcpy(szCommand, CRString(uCmdID));

	if (uCmdID == IDS_POP_PASS || uCmdID == IDS_POP_APOP)
		;//Progress(szCommand);			// Don't show arguments to some commands
	if (pszArgs)
		strcat(szCommand, pszArgs);		// Add arguments
		
	if (uCmdID != IDS_POP_PASS && uCmdID != IDS_POP_APOP)
		; //Progress(szCommand);
	strcat(szCommand, "\r\n");

	//
	// Send the command to the server.
	//
	int nStatus = NetConnection->PutDirect(szCommand);

	if(nStatus  < 0)
		return -1;
	else
		return 200;
	
/*	
	if (nStatus < 0)
		return nStatus;


	//
	// Read back the results, until we see either +OK or -ERR.
	//
	do
	{
		nStatus = m_pNetConnection->GetLine(pszBuffer, nBufferSize);
		if (nStatus < 0)
			return nStatus;
		if (nStatus > 1)
			pszBuffer[nStatus - 2] = 0;
	} while (strncmp(pszBuffer, "+OK", 3) != 0 && strncmp(pszBuffer, "-ERR", 4) != 0);

	if (*pszBuffer == '-')
	{
		//
		// POP command resulted in error.
		//
		if (bReportError)
		{
			if ((IDS_POP_PASS == uCmdID) || (IDS_POP_APOP == uCmdID))
			{
				const char* pszBadPass = m_Settings->GetBadPass();  //GetIniString(IDS_INI_BAD_PASSWORD_STRING);
				if (!*pszBadPass || strstr(pszBuffer, pszBadPass))
				{
					//
					// Got a "bad password" indication from the server,
					// so clear the password.
					//
					//::ClearPassword();
					m_bCleanup = true;
				}

				wsprintf(szCommand, "%s%s", (const char *) CRString(uCmdID),
						(const char *) CRString(IDS_ERR_POP_PASSWORD_HIDE));
			}

			//
			// Display error message for this POP session.
			//
			//char szPOPAccount[128];
			//GetIniString(IDS_INI_POP_ACCOUNT, szPOPAccount, sizeof(szPOPAccount));
			//ErrorDialog(IDS_ERR_TRANSFER_MAIL, szCommand, "POP", szPOPAccount, pszBuffer + 1);
			ErrorDialog(IDS_ERR_TRANSFER_MAIL, szCommand, "POP", m_Settings->GetPOPAccount(), pszBuffer + 1);
		}

		return -1;	// failure
	}

	return 1;		// success
	*/
}



// Close out session
int FinishSMTP()
{
	int status = 1;

	if (Signature)			
	{
		delete [] Signature;
		Signature = 0;
	}

	/*
	if (UsingPOPSend)
	{
		status = POPCmdGetReply(IDS_SMTP_MESSAGE_END, NULL, buf, lBufSize);

		// This is a bit of a heuristic.  Berkeley's popper returns an error
		// to the XTND XMIT command when a recpient is invalid.  But since
		// POP has such simplistic error reporting, we can't tell if it was
		// because the message was bad or something went wrong with the
		// network.  99% of the time it's because of an invalid recipient, so
		// let's take that course and de-queue the message.
		if (status < 0)
			SMTPmsg->m_Sum->SetState(MS_UNSENDABLE);
	}
	else
		status = SMTPCmdGetReply(IDS_SMTP_MESSAGE_END, NULL, buf, lBufSize, FALSE);

	*/
	return (status);

}

// EndSMTP
// 
// Hang up the SMTP connection
//
int EndSMTP(BOOL abort)
{
	char buf[256];

	if (SMTPConnected)
	{
		SMTPConnected = FALSE;

		if (!UsingPOPSend)
		{
			if (!abort)
				SMTPCmdGetReply(IDS_SMTP_QUIT, NULL, buf, sizeof(buf), (ProgWin? TRUE : FALSE));
			NetConnection->Close();
		}
	}

	// It's possible to be offline and get here because of a kludge added to jjnet
	if (GetIniShort(IDS_INI_NET_IMMEDIATE_CLOSE) || 
		GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		if (DeleteNetObj == TRUE)
		{
			delete NetConnection;
			NetConnection = NULL;
		}
	}

	return (1);
}	



// DoRcptLine
//
// Take a line containing addresses to folks who are supposed to
// get this message and send it to the server if we're doing SMTP
//
int DoRcptLine(const char *RcptLine)
{
	CString copy(RcptLine);
	char* line = copy.GetBuffer(copy.GetLength() + 1);
	char* end = line + ::SafeStrlenMT(line);
	char address[258];
	char* start;

	if (UsingPOPSend || !RcptLine || !*RcptLine || !line)
		return (1);

	for (; line < end; line++)
	{
		start = line;
		line = FindAddressEnd(start);
		if (!line)
		{
			ErrorDialog(IDS_ERR_ADDRESS_COMMENT, start);
			return (-1);
		}
		*line = 0;

		StripAddress(start);
		if (strlen(start) > 255)
		{
			ErrorDialog(IDS_ERR_ADDRESS_TOO_LONG, RcptLine + (start - (const char *)copy));
			return (-1);
		}

		// Get rid of comments in group syntax
		char LastChar = 0;
		BOOL InQuote = FALSE;
		for (char* s = start; *s; LastChar = *s++)
		{
			if (*s == '"')
				InQuote = !InQuote;
			else if (!InQuote && LastChar == ':')
			{
				if (*s != ':')
					start = s;
				else if (*++s == 0)
					break;
			}
		}
		strcpy(address, start);

		int len = strlen(address);
		if (address[len - 1] == ';')
			address[--len] = 0;
		TrimWhitespaceMT(address);
		if (*address && (unsigned char)*address != 131)
		{
			memmove(address + 1, address, len + 1);
			*address = '<';
			strcat(address, ">");
			int status = SMTPCmdGetReply(IDS_SMTP_RECIPIENT, address, buf, lBufSize, TRUE, TRUE) / 100;
			if (status != 2)
			{
				if (status == 5)
				{
					SMTPmsg->m_Sum->SetState(MS_UNSENDABLE);

					// Remove angle brackets
					strcpy(address, address + 1);
					address[strlen(address) - 1] = 0;

					ErrorDialog(IDS_ERR_BAD_RECIPIENT, address);
				}
				return (-1);
			}
		}
	}

	return (1);
}

// DoHeader
//
// Expand aliases, auto qualify addresses, and send the recipients
//
int DoHeader(int HeaderNum)
{
	char* ExpandedLine;
	const char* Header;
	
	Header = SMTPmsg->GetHeaderLine(HeaderNum);
	if (Header && *Header)
	{
		if (!(ExpandedLine = ExpandAliases(Header, TRUE, TRUE, TRUE, TRUE)))
			return (-1);
		
		//SMTPmsg->SetHeaderLine(HeaderNum, ExpandedLine);
		
		//if (DoRcptLine(SMTPmsg->GetHeaderLine(HeaderNum)) < 0)
		if (DoRcptLine(ExpandedLine) < 0)
		{
			SMTPmsg->SetHeaderAsInvalid(HeaderNum, TRUE);
			delete [] ExpandedLine;
			return (-1);
		}
		delete [] ExpandedLine;
	}

	SMTPmsg->SetHeaderAsInvalid(HeaderNum, FALSE);
	return (1);
}

// ComposeDate
// Get the info together to compose an RFC-822 happy date stamp
//
int ComposeDate(char* DateBuf, unsigned long GMTTime, int TimeZoneMinutes /*= -1*/, CSummary* SumToSetDate/* = NULL*/)
{
	*DateBuf = 0;

	// Date must be in the format of
	// "Date: <weekday> , <date> <month> <year> HH:MM:SS <gmt offset>"
	// where <gmt offset> = [+|-]HHMM for current time zone

	if (TimeZoneMinutes == -1)
		TimeZoneMinutes = -GetGMTOffset();
	else
	{
		GMTTime += TimeZoneMinutes * 60;
	}

	CTime Time = GMTTime;
	if (Time.GetTime() < 0)
		Time = 0;
	strcpy(DateBuf, ::FormatTimeMT(Time.GetTime(), CRString(IDS_SMTP_DATE_FORMAT)));
	::TimeDateStringFormatMT(DateBuf + strlen(DateBuf), 0, TimeZoneMinutes, " %4");

	if (SumToSetDate)
	{
		SumToSetDate->m_Seconds = Time.GetTime();
		SumToSetDate->m_TimeZoneMinutes = TimeZoneMinutes;
		SumToSetDate->FormatDate(DateBuf);
	}

	return (TRUE);
}

// LongerThan
// Is there a line longer than some number of chars?
BOOL LongerThan(const char* text, short len)
{
	int i = len;

	if (!text)
		return (FALSE);

	for (; *text && i; i--, text++)
	{
		// Have to find CR LF pair to be considered the end of a line
		if (*text == '\r' && text[1] == '\n')
		{
			text++;
			i = len;
		}
	}
	if (i)
		return (FALSE);

	return (TRUE);
}

// AnyFunny
// Does a block of text contain funny chars?
BOOL AnyFunny(const char* text)
{
	int FCQ = GetIniShort(IDS_INI_FIX_CURLY_QUOTES);

	if (!text)
		return (FALSE);

	for (BYTE *t = (BYTE*)text; *t; t++)
	{
		if (*t > 126)
		{
			if (!FCQ || *t < 145 || *t > 148)
				return (TRUE);
		}
	}

	return (FALSE);
}

// UseQP
// Should quoted printable encoding be done?
BOOL UseQP( const char* text1, const char* text2, int etid, BOOL bIsRich )
{
	if(	etid || 
		AnyFunny(text1) || 
		AnyFunny(text2) ||
		IsFromLine(text1) ||
		IsFromLine(text2) ||
		(	(	bIsRich || !SMTPmsg->m_Sum->WordWrap() ) &&
			(	LongerThan(text1, 1000) || LongerThan(text2, 1000))))
	{
		return (TRUE);
	}
	
	return (FALSE);
}

// NameCharset
// Build a charset= parameter
void NameCharset(char* charset, int tid)
{
	CRString Name(tid? IDS_MIME_ISO_LATIN1 : IDS_MIME_US_ASCII);
	CRString MIMECharset(IDS_MIME_CHARSET);

	sprintf(charset, "; %s=\"%s\"", (const char*)MIMECharset, (const char*)Name);
}

// SendContentType
// deduce and send the appropriate content-type
// (and CTE) for two blocks of text (body and signature, typically)
//	text1 - one block of text (may be null)
//  text2 - second block of text (may be null)
//  tableID - xlate table id
// Returns -1 for error, 0 for normal text, 1 for QP text
int SendContentType(const char* text1, const char* text2, int tableID, int flag, MIMEMap* mm = NULL)
{
	int etid = tableID;
	char scratch[128];
	
	if ( UseQP(text1, text2, etid, ( flag & MSF_XRICH ) ? TRUE : FALSE ) )
		etid = 1;

	NameCharset(scratch, etid);
	
	// send the content type
	CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
	CRString MimeText(IDS_MIME_TEXT);
	CRString MimePlain(IDS_MIME_PLAIN);
	CRString MimeHTML(IDS_MIME_HTML);
	bool bFormatFlowed = FALSE;

	const char* Type = MimeText;
	const char* Subtype;
	if (flag & MSF_XRICH)
		Subtype = MimeHTML;
	else
	{
		Subtype = MimePlain;
		if (SMTPmsg->m_Sum->WordWrap() && GetIniShort(IDS_INI_SEND_FORMAT_FLOWED))
			bFormatFlowed = TRUE;
	}

	if (mm)
	{
		Type = mm->m_Mimetype;
		Subtype = mm->m_Subtype;
		bFormatFlowed = FALSE;
	}
	if (bFormatFlowed)
	{
		sprintf(scratch + strlen(scratch), "; %s=%s", (LPCTSTR)CRString(IDS_MIME_FORMAT), (LPCTSTR)CRString(IDS_MIME_FLOWED));
	}
	sprintf(buf, "%s %s/%s%s", (const char*)ContentType, Type, Subtype, scratch);
	if (NetConnection->PutLine(buf) < 0) return (-1);

	if (!etid)
		return (0);

	// content-transfer-encoding
	CRString ContentEncoding(IDS_MIME_HEADER_CONTENT_ENCODING);
	CRString CTEName(SMTPmsg->m_Sum->UseQP()? IDS_MIME_QP : IDS_MIME_8BIT);
	sprintf(buf, "%s %s", (const char*)ContentEncoding, (const char*)CTEName);
	if (NetConnection->PutLine(buf) < 0) return (-1);

	return (SMTPmsg->m_Sum->UseQP());
}


typedef enum
{
	MT_NONE,
	MT_MIXED,
	MT_RELATED,
	MT_ALTERNATIVE
} MultipartType;

// SendMultipart
//
// Send the multipart/* header, a blank line, and then the starting boundary
//
int SendMultipart(char boundary[], MultipartType MultType)
{
	ASSERT (MultType != MT_NONE);

	const BOOL bUseAlternative = SMTPmsg->m_Sum->SendPlainAndStyled();
	const BOOL bPlainOnly = SMTPmsg->m_Sum->SendPlainOnly();

	sprintf(boundary, CRString(IDS_MIME_BOUNDARY_FORMAT), GetTickCount());

	UINT TypeID = 0;
	switch (MultType)
	{
	case MT_MIXED:
		TypeID = IDS_MIME_MIXED;
		break;

	case MT_RELATED:
		TypeID = IDS_MIME_RELATED;
		strcat(boundary, ".REL");
		break;

	case MT_ALTERNATIVE:
		TypeID = IDS_MIME_ALTERNATIVE;
		strcat(boundary, ".ALT");
		break;

	default:
		// Shouldn't get any other type here!
		ASSERT(0);
	}

	// "Content-Type: multipart/<blah>"
	sprintf(buf, "%s %s/%s;",
		(const char*)CRString(IDS_MIME_HEADER_CONTENT_TYPE),
		(const char*)CRString(IDS_MIME_MULTIPART),
		(const char*)CRString(TypeID));
	if (NetConnection->PutLine(buf) < 0)
		return -1;

// Not doing this anymore because the draft may be going away,
// or at a minimum changing.
//
//	if (MultType == MT_ALTERNATIVE)
//	{
//		// For Laurence's IETF draft: draft-lundblade-1pass-mult-alt-00.txt
//		// Specify alternative parts to come
//		// '<tab>types="text/plain,text/html";'
//		sprintf(buf, "\ttypes=\"%s/%s,%s/%s\";",
//			(const char*)CRString(IDS_MIME_TEXT),
//			(const char*)CRString(IDS_MIME_PLAIN),
//			(const char*)CRString(IDS_MIME_TEXT),
//			(const char*)CRString(IDS_MIME_HTML));
//		if (NetConnection->PutLine(buf) < 0)
//			return -1;
//	}

	if (MultType == MT_RELATED)
	{
		UINT RootTypeID = IDS_MIME_TEXT;
		UINT RootSubTypeID = IDS_MIME_HTML;
		if (bPlainOnly)
		{
			RootTypeID = IDS_MIME_TEXT;
			RootSubTypeID = IDS_MIME_PLAIN;
		}
		else if (bUseAlternative)
		{
			RootTypeID = IDS_MIME_MULTIPART;
			RootSubTypeID = IDS_MIME_ALTERNATIVE;
		}

		// '<tab>type="text/html"' or '<tab>type="text/plain"' or '<tab>type="multipart/alternative"'
		sprintf(buf, "\ttype=\"%s/%s\";",
			(const char*)CRString(RootTypeID),
			(const char*)CRString(RootSubTypeID));
		if (NetConnection->PutLine(buf) < 0)
			return -1;
	}

	// "<tab>boundary="<boundary string>"
	sprintf(buf, "\t%s=\"%s\"",
		(const char*)CRString(IDS_MIME_BOUNDARY),
		boundary + 4);
	if (NetConnection->PutLine(buf) < 0)
		return -1;

	// Blank line and starting boundary (boundary buffer starts with CRLF)
	return NetConnection->PutLine(boundary);
}

// SendDataText
// Writes out text during the DATA command.  Need to check for things
// like leading periods, and very long lines (> 1000 chars).
//
// text: text to be sent
// length: length of the text (-1 if NULL terminated)
// InitCharsOnLine: initializes the CharsOnLine static variable
//                  (-1 means no initialization)
//
// Returns -1 on error, positive otherwise
//
int SendDataText(const char* text, int length = -1, int InitCharsOnLine = -1)
{
	static int CharsOnLine = 0;

	if (InitCharsOnLine != -1)
		CharsOnLine = InitCharsOnLine;

	if (!text || !*text)
		return (1);

	if (length == -1)
		length = strlen(text);

	while (length)
	{
		const char* start = text;

		// Leading periods must be quoted, i.e. an extra period must be added
		if (!CharsOnLine && *text == '.')
		{
			if (NetConnection->Put('.') < 0)
				goto fail;
		}

		int count = 0;
		const int MagicLength = 995;
		while (length && *text != '\n' && count < MagicLength)
		{
			text++; length--; count++;
		}
		CharsOnLine = count;

		if (length)
		{
			// Special case of truncating a long line at a CR, let it pass
			if (count == MagicLength && *text == '\r')
			{
				text++; length--; count++;
			}
			if (*text == '\n')
			{
				CharsOnLine = 0;
				text++; length--; count++;
			}
		}

		// Send what we got
		if (NetConnection->Put(start, count) < 0) goto fail;

		// Line was too long, so insert newline to break it up
		if (count >= MagicLength && text[-1] != '\n')
		{
			if (NetConnection->PutLine() < 0) goto fail;
			CharsOnLine = 0;
		}
	}

	return (1);

fail:
	return (-1);
}


typedef enum {
	k2047LWSP,
	k2047Word,
	k2047Plain,
	k2047End
} Enum2047;

typedef struct {
	char word[256];
	Enum2047 wordType;
} Token2047, *Token2047Ptr;

#define RingNext(pointer,array,size) ((array) + (((pointer)-(array))+1)%(size))

/************************************************************************
 * AnyHighBits - any high bits in a string?
 ************************************************************************/
BOOL AnyHighBits(const char* s, long len)
{
	while (len-- > 0)
	{
		if (*s++ & 0x80)
			return (TRUE);
	}
		
	return(FALSE);
}

/************************************************************************
 * Next2047Word - parse a word from a 2047 stream
 ************************************************************************/
void Next2047Word(const char*& startP, const char* end, Token2047Ptr current, const char* delim,
	BOOL& wasQuote, BOOL& encQuote)
{
	char word[64];
	char* w = word;
	short wordLim = 48;
	Enum2047 wordType;
	char c;
	const char* source = startP;
	BOOL justSpace;
	const char* qSpot;
			
	// are we off the end?
	if (source >= end)
	{
		wordType = k2047End;
		*w = 0;
	}
	else
	{
		c = *source++;
		*w++ = c;
		if (wasQuote || c=='"')			// collect a quote
		{
			if (!wasQuote)
			{	// search to end of quote to see if quote contains any high bits
				for (qSpot = source; qSpot < end; qSpot++)
					if (qSpot[0] == '"' && qSpot[-1] != '\\') break;
				encQuote = AnyHighBits(source, qSpot - source);
			}
			wasQuote = TRUE;
			while (source < end && w - word < wordLim)
			{
				*w++ = *source;
				if (*source & 0x80) wordLim -= 2;	// allow space for encoding
				if (*source++ == '"' && w[-1] != '\\')
				{
					wasQuote = FALSE;
					break;	// closing "
				}
			}
			wordType = encQuote? k2047Word : k2047Plain;
		}
		else if (strchr(delim, c))	// collect a string of delimiters
		{
			justSpace = (c == ' ');
			while (source < end && w - word < wordLim)
			{
				if (*source != '"' && strchr(delim, *source))
				{
					*w++ = *source;
					if (*source != ' ') justSpace = FALSE;
					source++;
				}
				else break;
			}
			wordType = justSpace? k2047LWSP : k2047Plain;
		}
		else	// collect a regular word
		{
			while (source < end && w - word < wordLim)
			{
				if (strchr(delim, *source)) break;
				else
				{
					if (*source & 0x80) wordLim -= 2;	// allow space for encoding
					*w++ = *source;
					source++;
				}
			}
			wordType = AnyHighBits(word, w - word)? k2047Word : k2047Plain;
		}
	}
	
	*w = 0;
	strcpy(current->word, word);			// copy word and type into current buffer
	current->wordType = wordType;
	startP = source;								// mark new position in string
}

/************************************************************************
 * Encode2047String - encode a string in 2047-speak
 ************************************************************************/
void Encode2047String(char* s, BOOL StructuredHeader)
{
	char encoded[256];
	char *from, *to;
	unsigned char c;
	static const char hex[] = "0123456789ABCDEF";
	static const char OkChars[] = "!*+-/";
		
	// now, we encode
	to = encoded;
	for (from = s; *from; from++)
	{
		c = *from;

		if (isalnum(c) || strchr(OkChars, c))
			*to++ = c;
		else if (c == ' ')
			*to++ = '_';
		else if (!StructuredHeader && isprint(c) && c != '=' && c != '?' && c != '_')
			*to++ = c;
		else
		{
			*to++ = '=';
			*to++ = hex[(c >> 4) & 0xF];
			*to++ = hex[c & 0xF];
		}

		if (to > encoded + sizeof(encoded) - 20)
		{
			ASSERT(FALSE); // OVERFLOW
			return;
		}
	}
	
	*to = 0;
	
	sprintf(s, CRString(IDS_RFC1342_FMT), (const char*)CRString(IDS_MIME_ISO_LATIN1), encoded);
}

/************************************************************************
 * Encode2047 - encode a header line ala RFC 2047
 ************************************************************************/
CString Encode2047(const char* source, int len, int lineLimit, int& charsOnLine, BOOL StructuredHeader)
{
	Token2047 tokens[3];
	Token2047Ptr prev, curr, next;
	static const char dl2047[] = " \t()<>@,;:\"/[]?.=";
	BOOL continueQuote = FALSE;
	BOOL encQuote = FALSE;
	const char* spot = source;
	CString encoded;
	BOOL wrapped;
	char c;

//	// Check a simple case first: will the entire thing encoded fit on the line?
//	if (!StructuredHeader && charsOnLine + len < lineLimit)
//	{
//		strncpy(tokens[0].word, source, len);
//		tokens[0].word[len] = 0;
//		Encode2047String(tokens[0].word, StructuredHeader);
//		if ((short)(charsOnLine + strlen(tokens[0].word)) <= lineLimit)
//		{
//			encoded = tokens[0].word;
//			charsOnLine += encoded.GetLength();
//			return (encoded);
//		}
//		encoded.Empty();
//	}
	
	// initialize token buffer
	tokens[0].wordType = k2047End;
	*tokens[0].word = 0;
	
	// read first token
	Next2047Word(spot, source + len, tokens + 1 , dl2047, continueQuote, encQuote);
	
	// main loop
	for (curr = tokens + 1; curr->wordType != k2047End; curr = next)
	{
		next = RingNext(curr, tokens, 3);
		prev = RingNext(next, tokens, 3);
		Next2047Word(spot, source + len, next, dl2047, continueQuote, encQuote);
		wrapped = FALSE;

		while (1)
		{
			// whitespace between two encoded words gets elided.  Therefore, if we
			// are looking at whitespace and both the previous and next words are
			// encoded, we just add the whitespace to the beginning of the next word.
			//
			// if either the previous or next words are not encoded, then we don't
			// need to do anything special
			if (curr->wordType == k2047LWSP && prev->wordType == k2047Word && next->wordType == k2047Word)
			{
				strcat(curr->word, next->word);
				curr->wordType = k2047Word;
				Next2047Word(spot, source + len, next, dl2047, continueQuote, encQuote);
			}

			// See if the next word can be combined with the current word.
			// Unstructured headers can have any types of words combined.
			// Structured headers can combine words, as long as it's not a combination
			// of one word needing encoded and the other not.
			if (next->wordType != k2047End && (!StructuredHeader ||
				(curr->wordType == k2047Word && next->wordType == k2047Word) ||
				(curr->wordType == k2047Plain && next->wordType == k2047Plain)))
			{
				char EncodeBuf[256];
				strcpy(EncodeBuf, curr->word);
				strcat(EncodeBuf, next->word);
				if (curr->wordType == k2047Word || next->wordType == k2047Word)
					Encode2047String(EncodeBuf, StructuredHeader);
				if ((short)(charsOnLine + strlen(EncodeBuf)) > lineLimit)
				{
					// Two plain words together in a structured header could be an address,
					// so we don't want to break them up, but instead put them together
					// in the next line, hence the break before the current plain word
					if (prev->wordType != k2047End && StructuredHeader &&
						curr->wordType == k2047Plain && next->wordType == k2047Plain)
					{
						wrapped = TRUE;
					}
				}
				else
				{
					// Yes, it will fit, so append the next word to the current word,
					// and grab the following word and make it the "next" word
					strcat(curr->word, next->word);
					if (next->wordType == k2047Word)
						curr->wordType = k2047Word;
					Next2047Word(spot, source + len, next, dl2047, continueQuote, encQuote);
					continue;
				}
			}
			break;
		}
		
		// if we are encoding, get with it
		if (curr->wordType == k2047Word)
			Encode2047String(curr->word, StructuredHeader);
		
		// worry about line wrapping
		if (!wrapped)
		{
			wrapped = (prev->wordType != k2047Plain || curr->wordType != k2047Plain) &&
				(lineLimit && (short)(charsOnLine + strlen(curr->word)) >= lineLimit);
		}
		if (wrapped)
		{
			encoded += "\r\n ";
			charsOnLine = 1;
		}
		
		// if we are outputting an encoded word and the previous thing was an
		// encoded word, or if the previous thing was a regular word that did
		// NOT end with a space or a ')', then we must prepend a space to the
		// encoded word
		c = prev->word[strlen(prev->word) - 1];
		if (!wrapped && curr->wordType == k2047Word &&
		    (prev->wordType == k2047Word ||
				 prev->wordType == k2047Plain && c != ' ' && c != ')'))
		{
			memmove(curr->word + 1, curr->word, strlen(curr->word) + 1);
			curr->word[0] = ' ';
		}
		
		// if we are outputting an encoded word and the next thing is a
		// plain word and doesn't start with a space, then we must append
		// a space to the encoded word
		c = next->word[0]; // ?
		if (curr->wordType == k2047Word && next->wordType == k2047Plain && c != ' ')
			strcat(curr->word, " ");
			
		// stick the word on the end
		encoded += curr->word;
		charsOnLine += strlen(curr->word);
	}

	return (encoded);
}

int SendAddressHeader(int HeaderNum)
{
	int WrapLimit = GetIniShort(IDS_INI_SMTP_RECIPIENT_WRAP);
	const char* Text;
	CString strTemp;
	if( HeaderNum == HEADER_TO || HeaderNum == HEADER_CC || HeaderNum == HEADER_BCC)
	{
		char *szExpanded = ExpandAliases(SMTPmsg->GetHeaderLine(HeaderNum), TRUE, TRUE);
		strTemp = szExpanded;
		delete [] szExpanded;
		Text = strTemp;
	}
	else
		Text = SMTPmsg->GetHeaderLine(HeaderNum);

	BOOL AllowQP = SMTPmsg->m_Sum->UseQP();
	BOOL InGroup = FALSE;
	BOOL HadHigh = FALSE;
	CString Encoded2047;
	int CharsOnLine = 0;
	char LastChar = 0;
	const char *Begin;
	int Paren = 0;
	int Quote = 0;
	char NoTo[128];
	
	if (!Text || !*Text)
	{
		if (HeaderNum != HEADER_TO)
			return (1);
		Text = GetIniString(IDS_SMTP_NO_TO, NoTo, sizeof(NoTo));
	}
	
	if (NetConnection->Put(CRString(IDS_HEADER_TO + HeaderNum)) < 0) goto fail;
	if (NetConnection->Put(' ') < 0) goto fail;

	for (Begin = Text; 1; LastChar = *Text++)
	{
		if (!*Text)
		{
			InGroup = FALSE;
			Paren = Quote = 0;
		}
		if (InGroup && !Paren && !Quote && *Text == ';')
			InGroup = FALSE;
		if (InGroup)
			Begin++;
		else
		{
			// Are we going to need to do RFC 2047 encoding?
			if (AllowQP && (*Text & 0x80))
				HadHigh = TRUE;
			// To support the DecNet addressing scheme "<node>::<username>", and so as not to confuse
			// it with the group syntax notation "<group-name>:<address(es)>;", only stop at a colon
			// if another colon is not immediately after it.
			if (!*Text || (!Paren && !Quote && (*Text == ',' ||
				(*Text == ':' && Text[1] != ':' && LastChar != ':'))))
			{
				char *GroupName;
				int len = Text - Begin;
				GroupName = new char[len+1];
				BOOL PeanutWorkgroup = FALSE;

				strncpy(GroupName, Begin, len);
				GroupName[len] = 0;			// Darn strncpy

				_strlwr(GroupName);

				if (strstr(GroupName, "-workgroup"))
				{
					PeanutWorkgroup = TRUE;
				}

				delete [] GroupName;

				if (*Text)
				{
					// Start of the group syntax?
					// Don't check for group syntax when doing a POP send because
					// we don't want the addresses to be stripped out.
					if (!UsingPOPSend && *Text == ':' && !PeanutWorkgroup)
						InGroup = TRUE;
					len++;
				}

				// Need to use RFC 2047
				if (HadHigh)
				{
					Encoded2047 = Encode2047(Begin, len, WrapLimit, CharsOnLine, TRUE);
					Begin = Encoded2047;
					len = Encoded2047.GetLength();
				}
	
				// If the address won't fit on this line, then start a new line.
				// If we've already gone through RFC 2047 encoding, then the address
				// is already broken up in to the correct minimum line lengths.
				if (!HadHigh && CharsOnLine && CharsOnLine + len > WrapLimit)
				{
					if (NetConnection->PutLine() < 0) goto fail;
					if (NetConnection->Put(' ') < 0) goto fail;
					CharsOnLine = 0;
				}
	
				// Send out the address
				if (NetConnection->Put(Begin, len) < 0) goto fail;
				CharsOnLine += len;
	
				if (!*Text)
					break;
	
				// Skip spaces and tabs
				while (Text[1] == ' ' || Text[1] == '\t')
					Text++;
				if (!Text[1])
					break;
				Begin = Text + 1;
			}
			else if (!Paren && LastChar != '\\' && *Text == '"')
				Quote = !Quote;
			else if (!Quote)
			{
				if (*Text == '(')
					Paren++;
				else if (*Text == ')')
					Paren--;
			}
		}
	}

	if (NetConnection->PutLine() < 0) goto fail;

	return (1);

fail:
	return (-1);
}

int SendNormalHeader(int HeaderNum)
{
	int WrapLimit = GetIniShort(IDS_INI_SMTP_RECIPIENT_WRAP);
	const char* Text = SMTPmsg->GetHeaderLine(HeaderNum);
	BOOL AllowQP = SMTPmsg->m_Sum->UseQP();
	CRString HeaderText(IDS_HEADER_TO + HeaderNum);
	int CharsOnLine;
	int len;
	
	// Want to send a Subject: header, even if it's blank
	if (HeaderNum != HEADER_SUBJECT && (!Text || !*Text))
		return (1);
	
	if (NetConnection->Put(HeaderText) < 0) goto fail;

	CharsOnLine = HeaderText.GetLength() + 1;
	len = strlen(Text);
	
	// Is there RFC 2047 encoding to do?
	if (AllowQP && AnyHighBits(Text, len))
	{
		// Need to have a space between the colon of the header name and its contents 
		if (NetConnection->Put(' ') < 0) goto fail;
		CharsOnLine++;

		CString Encoded2047 = Encode2047(Text, len, WrapLimit, CharsOnLine, FALSE);
		if (NetConnection->Put(Encoded2047, Encoded2047.GetLength()) < 0) goto fail;
	}
	else
	{
		while (*Text)
		{
			const char* Limit = Text + WrapLimit - CharsOnLine; CharsOnLine = 0;	// CharsOnLine used only for header label
			const char* End = Text;
			const char* Space = Text;
			for (; End < Limit && *End && *End != '\r' && *End != '\n'; End++)
			{
				if (*End == ' ')
					Space = End;
			}
			if (Space > Text && End >= Limit && *End)
				End = Space;
			if (NetConnection->Put(' ') < 0) goto fail;
			if (NetConnection->Put(Text, End - Text) < 0) goto fail;
			if (*End == '\r')
				End++;
			if (*End == '\n')
				End++;
			if (*End && NetConnection->PutLine() < 0) goto fail;
			Text = End;
		}
	}

	if (NetConnection->PutLine() < 0) goto fail;

	return (1);

fail:
	return (-1);
}

int SendReferencesHeader(int HeaderNum)
{
	const char* Text = SMTPmsg->GetHeaderLine(HeaderNum);
	CRString HeaderText(IDS_HEADER_TO + HeaderNum);

	if ( !Text || *Text == '\0' )
		return(1);		// nothing to do

	if (NetConnection->Put(HeaderText) < 0) goto fail;

	while ( Text )
	{
		// write one <message id> string per line
		char * pEnd = strchr( Text, '>' );
		if (pEnd)
			*pEnd = '\0';

		if (NetConnection->Put(" ") < 0) goto fail;
		if ( Text[ 0 ] != '<' )	// badly formed message-id - try to clean it up
			if (NetConnection->Put("<") < 0) goto fail;
		if (NetConnection->Put(Text) < 0) goto fail;
		if (NetConnection->Put(">") < 0) goto fail;
		if (NetConnection->PutLine() < 0) goto fail;

		if ( pEnd )
		{
			*pEnd = '>';
			Text = strchr( pEnd, '<' );	// advance to the next message id
		}
		else
			break;	// badly formed message-id - we did our best...
	}

	return (1);

fail:
	return (-1);
}

// EscapeFormatFlowed
//
// Makes sure that special lines (ones that start with space, '>', or "From ")
// get space-stuffed so that they aren't misinterpretted by format=flowed
//
// Returns an allocated buffer, so remember to delete when finished.
//
char* EscapeFormatFlowed(const char* Text)
{
	if (!Text)
		return NULL;

	int NumLines = 1;
	for (const char* t = Text; t; t = strchr(t, '\n'))
	{
		NumLines++;
		t++;
	}

	// Escaped text will add at most one extra character per line
	char* EscapedText = new char[strlen(Text) + NumLines + 1];
	char* EscapedTextSpot = EscapedText;

	const char* LineEnd;
	for (const char* LineStart = Text; *LineStart; LineStart = LineEnd + 1)
	{
		if (*LineStart == ' ' || *LineStart == '>' || strncmp(LineStart, "From ", 5) == 0)
			*EscapedTextSpot++ = ' ';
		LineEnd = strchr(LineStart, '\n');
		if (!LineEnd)
			LineEnd = LineStart + strlen(LineStart) - 1;
		int CharsToCopy = LineEnd - LineStart + 1;
		strncpy(EscapedTextSpot, LineStart, CharsToCopy);
		EscapedTextSpot += CharsToCopy;
	}
	*EscapedTextSpot = 0;

	return EscapedText;
}

// SendPlain
// Writes out some text to the message
//
int SendPlain(const char* text)
{	
	QPEncoder	TheQPEncoder;
	LONG		OutLen;
	int			EncodeFlag;
	const BOOL	bFCQ = GetIniShort(IDS_INI_FIX_CURLY_QUOTES);
	const BOOL	bSendFormatFlowed = GetIniShort(IDS_INI_SEND_FORMAT_FLOWED);
	const BOOL	bWordWrap = SMTPmsg->m_Sum->WordWrap();
	int			WWColumn = GetIniShort(IDS_INI_WORD_WRAP_COLUMN);
	int			WWMax = GetIniShort(IDS_INI_WORD_WRAP_MAX);
	LPCSTR		pSrcLine;
	int			iLineLen;
	int			iNextLineOffset;
	int			i;
	BOOL		bAddSoftReturn;
	BOOL		bAddSoftReturnNow;
	LONG		lSrcLen;
	CString		PropigatedQuote;

	// Init CharsOnLine variable of SendDataText()
	SendDataText(NULL, -1, 0);

	if ((EncodeFlag = SendContentType(text, NULL, 0, 0)) < 0 ||
		NetConnection->PutLine() < 0)
	{
		return -1;
	}

	if (EncodeFlag)
	{
		if (TheQPEncoder.Init(buf, OutLen, EncodeFlag) || SendDataText(buf, OutLen) < 0)
			return (-1);
	}

	if (bWordWrap/* && !(encode & ENCODE_RICH)*/ )
	{
		// initialize
		pSrcLine = text;
		lSrcLen = strlen( text );
		bAddSoftReturn = FALSE;
		
		//Sanity check
		if(WWMax < WWColumn)
		{
			WWMax = WWColumn + 4;  //otherwise the email will get chopped
			SetIniShort(IDS_INI_WORD_WRAP_MAX, (short)WWMax);
			SetIniShort(IDS_INI_WORD_WRAP_COLUMN, (short)WWColumn);
		}

		// Add one to WordWrapMax to prevent off-by-one errors below.
		// Basically, lines should be wrapped if they are WWMax characters,
		// so we can put at most WWMax - 1 characters on a line.  But we also
		// need to include space for the CR and LF, so that makes
		// (WWMax - 1) + 2 = WWMax + 1.
		WWMax++;

		while( lSrcLen > 0 )
		{
			int CharPrependCount = 0;

			bAddSoftReturnNow = bAddSoftReturn;

			// get at least one line
			CString	szLine( pSrcLine, ( WWMax < lSrcLen ) ? WWMax : lSrcLen );

			if (bAddSoftReturn)
			{
				// If there was a quote to propigate, add it now before we calculate
				// the number of characters allowed on a line
				if (PropigatedQuote.IsEmpty() == FALSE)
				{
					szLine = PropigatedQuote + szLine;
					CharPrependCount += PropigatedQuote.GetLength();
				}

				// For format=flowed, do space-stuffing of continuation lines that begin with space, '>', or "From "
				if (bSendFormatFlowed)
				{
					if (*pSrcLine == ' ' || *pSrcLine == '>' || strncmp(pSrcLine, "From ", 5) == 0)
					{
						szLine = ' ' + szLine;
						CharPrependCount++;
					}
				}
			}
							
			// see if the line actually ends before WWMax
			iLineLen = szLine.Find( "\r\n" );
			
			if ( iLineLen >= 0 )
			{
				// set the next line offset
				iNextLineOffset = iLineLen + 2 - CharPrependCount;

				if (bSendFormatFlowed)
				{
					while (iLineLen && szLine[iLineLen - 1] == ' ')
						iLineLen--;
					szLine.SetAt(iLineLen++, '\r');
					szLine.SetAt(iLineLen++, '\n');
					szLine.ReleaseBuffer(iLineLen);
				}
				else
				{
					// yep -- add the \r\n to the length and truncate the rest
					iLineLen += 2;
					szLine.ReleaseBuffer(iLineLen);
				}

				// no soft return will be necessary next time
				bAddSoftReturn = FALSE;
				PropigatedQuote.Empty();
			}
			else
			{
				// get the length of the string
				iLineLen = szLine.GetLength();

				// see if we need to wrap
				if ( iLineLen < WWMax )
				{
					// set the next line length
					iNextLineOffset = iLineLen - CharPrependCount;

					// no wrapping is necessary
					bAddSoftReturn = FALSE;
					PropigatedQuote.Empty();
				}
				else
				{
					char StartCharAtWrappedWord = szLine[WWColumn];

					// setup for wrapping at the column postion 
					szLine.ReleaseBuffer(WWColumn);
					iLineLen = WWColumn;				

					// set the next line offset ( if there is no good place to break )
					iNextLineOffset = iLineLen - CharPrependCount;
					
					// See if we can find a good place to break the line.
					// Don't go in to the quote though because it contains a space.
					int WrapHere = 0;
					while( iLineLen > CharPrependCount)
					{
						if ((szLine[iLineLen - 1] == ' ' || szLine[iLineLen - 1] == '\t') &&
							StartCharAtWrappedWord != '>')
						{		
							// set the next line offset
							int LastSpace = iLineLen - CharPrependCount;

							// skip trailing whitespace
							while(	iLineLen && 
									(	( szLine[ iLineLen - 1 ] == ' ' ) || 
										( szLine[ iLineLen - 1 ] == '\t' ) ) )
							{
								iLineLen --;
							}
							if (!bSendFormatFlowed ||
								(iLineLen == WWColumn - 1 && StartCharAtWrappedWord != ' '  && StartCharAtWrappedWord != '\t' ||
								 iLineLen < WWColumn - 1 &&  szLine[iLineLen + 1]   != ' ' &&  szLine[iLineLen + 1]   != '\t'))
							{
								WrapHere = 0;
								iNextLineOffset = LastSpace;
								break;
							}
							if (!WrapHere)
							{
								WrapHere = iLineLen;
								iNextLineOffset = LastSpace;
							}
						}
				
						// keep looking
						iLineLen --;
						StartCharAtWrappedWord = szLine[iLineLen];
					}

					if (WrapHere)
					{
						iLineLen = WrapHere;
					}
					if (iLineLen > CharPrependCount)
						szLine.ReleaseBuffer(iLineLen);


					if (PropigatedQuote.IsEmpty())
					{
						int NumQuoteChars = 0;
						while ('>' == szLine[NumQuoteChars])
							NumQuoteChars++;
						if (NumQuoteChars)
						{
							if (' ' == szLine[NumQuoteChars])
								NumQuoteChars++;
							strncpy(PropigatedQuote.GetBuffer(NumQuoteChars), szLine, NumQuoteChars);
							PropigatedQuote.ReleaseBuffer(NumQuoteChars);
						}
					}

					// format=flowed requires one trailing space
					if (bSendFormatFlowed)
						szLine += ' ';

					// get the final length of the string
					iLineLen = szLine.GetLength();

					// we'll need an extra return next time
					bAddSoftReturn = TRUE;
				}
			}

			// prepend a soft return & update the length, if necessary
			if ( bAddSoftReturnNow )
			{
				szLine = "\r\n" + szLine;
				iLineLen += 2;
			}

			if (EncodeFlag)
			{
				// do the encoding
				TheQPEncoder.Encode( szLine, iLineLen, buf, OutLen);

				// set szLine
				szLine = CString( buf, OutLen );
			}
			else if (bFCQ)
			{
				// Change curly quotes into normal quotes

				i = iLineLen;

				while (--i >= 0)
				{
					if (((BYTE)szLine[i]) == 145 || ((BYTE)szLine[i]) == 146)
					{
						szLine.SetAt(i, '\'');
					}
					else if (((BYTE)szLine[i]) == 147 || ((BYTE)szLine[i]) == 148)
					{
						szLine.SetAt(i, '"');
					}
				}
			}


			if( SendDataText( szLine ) < 0 )
			{
				// send the line
				return (-1);
			}

			
			// increment the line pointer
			pSrcLine += iNextLineOffset;

			// decrement the length
			lSrcLen -= iNextLineOffset;

			// update the guage
			ProgressAdd( iNextLineOffset );
		}
	}
	else
	{
		while (*text)
		{
			LONG len = strcspn(text, "\n");
			if (text[len] == '\n')
            	len++;
			if (EncodeFlag)
			{
				len = min(len, (LONG)(lBufSize / 3));
				TheQPEncoder.Encode(text, len, buf, OutLen);
				if (SendDataText(buf, OutLen) < 0) return (-1);
			}
			else
			{
				if (SendDataText(text, len) < 0) return (-1);
			}
			text += len;
			ProgressAdd(len);
		}
	}

	if (EncodeFlag)
	{
		if (TheQPEncoder.Done(buf, OutLen) || SendDataText(buf, OutLen) < 0)
			return (-1);
	}

	return (1);
}


int SendStyled(const char* text)
{
	QPEncoder	TheQPEncoder;
	LONG		OutLen;
	int			EncodeFlag;
	const BOOL	bUseAlternative = SMTPmsg->m_Sum->SendPlainAndStyled();
	char		Boundary[128];

	// Init CharsOnLine variable of SendDataText()
	SendDataText(NULL, -1, 0);

	if (bUseAlternative)
	{
		CString PlainText(Html2Text(text, GetIniShort(IDS_INI_SEND_FORMAT_FLOWED)));

		if (SendMultipart(Boundary, MT_ALTERNATIVE) < 0 ||
			SendPlain(PlainText) < 0 ||
			NetConnection->PutLine(Boundary) < 0)
		{
			return -1;
		}
	}

	if ((EncodeFlag = SendContentType(text, NULL, 0, MSF_XRICH)) < 0 ||
		NetConnection->PutLine() < 0)
		return -1;

	if (EncodeFlag)
	{
		if (TheQPEncoder.Init(buf, OutLen, EncodeFlag) || SendDataText(buf, OutLen) < 0)
			return (-1);
	}

	const char *start, *space, *spot, *end;
	long soft = GetIniShort(IDS_INI_ENRICHED_SOFT_LINE);
	CString szXrich = CRString(IDS_MIME_XRICH);
	CString szNofill = CRString(IDS_MIMERICH_NOFILL);
	int  nXlen = szXrich.GetLength();
	int nNeg=0;
	char NewLine[4] = "\r\n";

	int		nofill=0;
	LONG	len;
	int		foundXrich=-1;
	int		nudge;	// this is a hack
	BOOL	InsideTag = FALSE;
	
	end = text + strlen(text);
		
	for (start=text;start<end;start = spot + 1)
	{
		for (space=spot=start;spot<end;spot++)
		{
			nudge=0;
			if (*spot=='\n')
			{
				nudge=1;
				break;
			}

			// break on spaces, but not if it's inside a tag
			if (*spot == ' ' && !InsideTag)
			{
				space = spot;
			}
			else if (*spot == '<')
			{
				InsideTag = TRUE;
			}
			else if (*spot == '>')
			{
				InsideTag = FALSE;
			}
			
			if (!nofill && space>start && spot-start>soft) break;
			
			if ((*spot=='<') && (spot[1] != '<'))	
			{
				nNeg = spot[1]=='/' ? 1 : 0;
				foundXrich=strncmp(spot+1+nNeg,szXrich,nXlen);
				if ( foundXrich==0 ) break;
			}
		}
		if ( spot==start )
			goto skipXrich;

		if (!nofill && spot-start>soft && space>start) spot = space;

		if( ( *spot != '\r' ) && ( spot[1] == '\r' ) && ( spot[2] == '\n' ) )
		{
			// if the place we're about to wrap at is followed by a newline,
			// don't bother inserting a soft return
			spot += 2;
			nudge = 1;
		}

		if (EncodeFlag)
		{
			len = min(len, (LONG)(lBufSize / 3));
			TheQPEncoder.Encode(start, spot-start+nudge, buf, OutLen);
			if (SendDataText(buf, OutLen) < 0) return (-1);
		}
		else
		{
			if (SendDataText(start, spot-start+nudge) < 0) return (-1);
		}
		
		if (*start=='<' && spot[-1]=='>')
		{
			if (start[1]=='/') 
			{
				if( ( spot - start - 3 ) >= lBufSize )
				{
					// this message line is too long - bail
					return -1 ;
				}

				strncpy(buf,start+2,spot-start-3);
			}
			else 
			{
				if( ( spot - start - 2 ) >= lBufSize )
				{
					// this message line is too long - bail
					return -1 ;
				}

				strncpy(buf,start+1,spot-start-2);
			}

			if (strcmp(buf,szNofill) )
			{
				if (start[1]=='/') 
					nofill = max(nofill-1,0);
				else 
					nofill++;
			}
		}

		if (	( *spot != '\n' ) && 
				( ( spot[1] != '/' ) || strncmp( spot+2, szXrich, nXlen) ) )
		{
			if (EncodeFlag)
			{
				TheQPEncoder.Encode(NewLine, 2, buf, OutLen);
				
				if (SendDataText(buf, OutLen) < 0) return (-1);
			}
			else
			{
				if (SendDataText(NewLine, 2) < 0) return (-1);
			}		

		}	
skipXrich:	
		if ( foundXrich==0 )
		{
			spot += nXlen + 1 + nNeg;
			foundXrich=-1;
		}
	}

	if (EncodeFlag)
	{
		if (TheQPEncoder.Done(buf, OutLen) || SendDataText(buf, OutLen) < 0)
			return (-1);
	}

	if (bUseAlternative)
	{
		// Finish up multipart/alternative by sending closing boundary
		if (NetConnection->Put(Boundary) < 0 ||
			NetConnection->PutLine("--") < 0)
		{
			return -1;
		}
	}

	return (1);
}


enum {
	ET_BINARY = 0,
	ET_ASCII,
	ET_QP
};

// EncodingType
//
// Reads over the file and does some manipulation on it to determine
// what encoding type is needed.
// Returns -1 for error, ET_BINARY for binary, ET_ASCII for regular text,
// ET_QP for quoted-printable, and if the message is being sent as BinHex
// the return is the number of LFs that need to be stripped
//
long EncodingType(const char* fname, MIMEMap& mm, BOOL bIsInline)
{
	JJFile InFile;
	long ByteCnt = 0L;
	int QPable = FALSE;
	BOOL IsTypeText = FALSE;
	unsigned long NumCharsOnLine = 0;
	BYTE ch, LastCh;
	char* p;

	p = strrchr(fname, '.');
	if (p)
		mm.Find(p + 1);
	if (!strcmp(mm.m_Mimetype, "text") || !strncmp(mm.m_Type, CRString(IDS_ATTACH_TEXT_TYPE), 4))
		IsTypeText = TRUE;
	else if (mm.m_Mimetype[0] || mm.m_Type[0])
	{
		// Assume binary on known mappings that aren't type TEXT
		return (ET_BINARY);
	}

	const BOOL bIsInlineText = (bIsInline && IsTypeText);

	if (FAILED(InFile.Open(fname, O_RDONLY)))
		return (-1L);

	BYTE FirstLineBuffer[256];
	BOOL bOnFirstLine = TRUE;
	HRESULT hrGet = S_OK;
	for (LastCh = 0; SUCCEEDED(hrGet = InFile.Get((char *)&ch)) && (hrGet != S_FALSE); LastCh = ch, NumCharsOnLine++)
	{
		// CRLF is end-of-line for all attachments.
		// CR only and LF only are end-of-line for inline text attachments.
		if ((ch == '\n' && LastCh == '\r') || (bIsInlineText && (ch == '\r' || ch == '\n')))
		{
			// If the first line is a "From" envelope, then this is probably a mailbox
			// and sending it as QP will help to protect it from "From "-munging MTAs
			if (bOnFirstLine)
			{
				if (NumCharsOnLine < sizeof(FirstLineBuffer))
				{
					FirstLineBuffer[NumCharsOnLine] = 0;
					if (IsFromLine((const char*)FirstLineBuffer))
						QPable = TRUE;
				}
				bOnFirstLine = FALSE;
			}

			// Only count it when we see a CR-LF pair, since that's the
			// only time a character will get stripped when doing BinHex
			if (ch == '\n' && LastCh == 'r')
				ByteCnt++;

			if (NumCharsOnLine >= 1000)
				QPable = TRUE;
			NumCharsOnLine = 0;
			continue;
		}

		// Treat as binary if we find any CRs or LFs in attachments that are not inline text
		if (!bIsInlineText && (LastCh == '\r' || ch == '\n'))
		{
			mm.m_Type[0] = 0;
			return (ET_BINARY);
		}

		// Check for binary data
		if (ch < 32 && ch != 0x0A && ch != 0x0D &&
			ch != 0x0C && ch != 0x09 && ch != 0x1A && ch != 0x04)
		{
			mm.m_Type[0] = 0;
			return (ET_BINARY);
		}
		if (ch == 0x04 || ch & 0x80)
			QPable = TRUE;

		if (bOnFirstLine && NumCharsOnLine < sizeof(FirstLineBuffer))
			FirstLineBuffer[NumCharsOnLine] = ch;
	}
	if (NumCharsOnLine >= 1000)
		QPable = TRUE;

	// Check for CR without LF in attachments that are not inline text
	if (!bIsInlineText && LastCh == '\r')
	{
		mm.m_Type[0] = 0;
		return (ET_BINARY);
	}

	if (!mm.m_Mimetype[0] || !mm.m_Subtype[0])
	{
		GetIniString(IDS_MIME_TEXT, mm.m_Mimetype, sizeof(mm.m_Mimetype));
		GetIniString(IDS_MIME_PLAIN, mm.m_Subtype, sizeof(mm.m_Subtype));
	}
	GetIniString(IDS_ATTACH_TEXT_TYPE, mm.m_Type, sizeof(mm.m_Type));
	if (!mm.m_Creator[0])
		GetIniString(IDS_CREATOR_TEXT, mm.m_Creator, sizeof(mm.m_Creator));

	if (SMTPmsg->m_Sum->TextAsDoc() && SMTPmsg->m_Sum->Encoding() == MSF_BINHEX)
		return (ByteCnt + 1);
	else
		return (QPable? ET_QP : ET_ASCII);
}


class EncodeCopy : public Encoder
{
public:
	int Encode(const char* In, LONG InLen, char* Out, LONG &OutLen)
		{ memcpy(Out, In, InLen); OutLen = InLen; return (0); }
};


int AttachMIME(const char* Filename, MIMEMap& mm, long EncoType, const char* ContentID, BOOL IsInline)
{
	Encoder* TheEncoder = NULL;
	JJFile InFile;
	long lNumRead;
	int BlockSize;
	BOOL IsQP = FALSE;
	const char* fname = strrchr(Filename, '\\');

	char* FileBuf = NULL;
	InFile.GetBuf(&FileBuf);
	ASSERT(FileBuf != NULL);

	while (1)
	{
		if (fname)
			fname++;
		else
			fname = Filename;
		
		// If there are funny characters, check whether mostly text, so use QP
		if (EncoType == ET_ASCII)
		{
			TheEncoder = new EncodeCopy;
			if (!TheEncoder || SendContentType(NULL, NULL, FALSE,0, &mm) < 0) break;
		}
		else
		{
			if (EncoType == ET_QP &&
				(!SMTPmsg->m_Sum->TextAsDoc() || SMTPmsg->m_Sum->Encoding()== MSF_MIME))
			{
				// SendContentType returns:
				//	-1 : error
				//	 0 : no QP
				//	 1 : QP
				int Result = SendContentType(NULL, NULL, TRUE, 0,&mm);
				if (Result < 0) break;
				if (Result)
				{
					TheEncoder = new QPEncoder;
					IsQP = TRUE;
				}
				else
					TheEncoder = new EncodeCopy;
			}
			else
			{
				if (SMTPmsg->m_Sum->Encoding() == MSF_UUENCODE)
					TheEncoder = new EncodeUU;
				else
					TheEncoder = new Base64Encoder;

				if (!TheEncoder) break;
		
				// If we don't have a full mime type/subtype, then assume application/octet-stream
				if (!mm.m_Mimetype[0] || !mm.m_Subtype[0])
				{
					GetIniString(IDS_MIME_APPLICATION, mm.m_Mimetype, sizeof(mm.m_Mimetype));
					GetIniString(IDS_MIME_OCTET_STREAM, mm.m_Subtype, sizeof(mm.m_Subtype));
				}
					
				// Content-Type
				CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
				CRString MIMEName(IDS_MIME_NAME);
				sprintf(buf, "%s %s/%s; %s=\"%s\"", (const char*)ContentType,
					mm.m_Mimetype, mm.m_Subtype, (const char*)MIMEName, fname);
				if (NetConnection->Put(buf) < 0) break;

				// Add Mac info if present
				if ((mm.m_Type[0] || mm.m_Creator[0]) &&
					(strcmp(mm.m_Type, CRString(IDS_TYPE_UNKNOWN)) ||
					strcmp(mm.m_Creator, CRString(IDS_CREATOR_UNKNOWN))))
				{
					if (NetConnection->PutLine(';') < 0) break;

					CRString MacType(IDS_MIME_MAC_TYPE);
					CRString MacCreator(IDS_MIME_MAC_CREATOR);
					sprintf(buf, " %s=\"%02X%02X%02X%02X\"; %s=\"%02X%02X%02X%02X\"",
						(const char*)MacType, mm.m_Type[0], mm.m_Type[1], mm.m_Type[2], mm.m_Type[3],
						(const char*)MacCreator, mm.m_Creator[0], mm.m_Creator[1], mm.m_Creator[2], mm.m_Creator[3]);
					if (NetConnection->Put(buf) < 0) break;
				}

				if (NetConnection->PutLine() < 0) break;
				
				// Content-ID: header (if there is one)
				if (ContentID && *ContentID)
				{
					sprintf(buf, "%s <%s>",
						(const char*)CRString(IDS_MIME_HEADER_CONTENT_ID),
						ContentID);
					if (NetConnection->PutLine(buf) < 0) break;
				}

				// Content-Transfer-Encoding
				CRString ContentEncoding(IDS_MIME_HEADER_CONTENT_ENCODING);
				CRString CTEName(SMTPmsg->m_Sum->Encoding() == MSF_MIME? IDS_MIME_BASE64 : IDS_MIME_UUENCODE);
				sprintf(buf, "%s %s", (const char*)ContentEncoding, (const char*)CTEName);
				if (NetConnection->PutLine(buf) < 0) break;
			}
		}
		
		// Content-Disposition
		if (EncoType == ET_BINARY || SMTPmsg->m_Sum->TextAsDoc())
		{
			CRString ContentDisp(IDS_MIME_HEADER_CONTENT_DISP);
			CRString ContentDispType(IsInline? IDS_MIME_CONTENT_DISP_INLINE : IDS_MIME_CONTENT_DISP_ATTACH);
			CRString ContentDispFilename(IDS_MIME_CONTENT_DISP_FILENAME);
			sprintf(buf, "%s %s; %s=\"%s\"",
				(const char*)ContentDisp,
				(const char*)ContentDispType,
				(const char*)ContentDispFilename,
				fname);
			if (NetConnection->PutLine(buf) < 0) break;
		}
		
		// Blank line between MIME header and body
		if (NetConnection->PutLine() < 0) break;
		
		// Put out beginning bytes.  Put filename in buffer for those encodings that need it.
		strcpy(buf, fname);
		lNumRead = 0;
		int flag = IsQP ? ENCODE_QP : ENCODE_NONE;
		if (TheEncoder && TheEncoder->Init(buf, lNumRead, flag)) break;
		
		// Init CharsOnLine variable of SendDataText()
		SendDataText(NULL, -1, 0);
		
		if (SendDataText(buf, lNumRead) < 0) break;
		
		// Now on to the attachment
		if (FAILED(InFile.Open(Filename, O_RDONLY)))
			break;

		if (IsQP)
		{
			char* buf2 = new char[lBufSize * 3];
			if (!buf2) break;
			buf[lBufSize - 1] = 0;
			ASSERT( lBufSize > 0 && lBufSize < 32767L );
			while (SUCCEEDED(InFile.RawRead(buf, int(lBufSize - 1), &lNumRead)) && (lNumRead > 0))
			{
				LONG OutLen;
				TheEncoder->Encode(buf, lNumRead, buf2, OutLen);
				if (SendDataText(buf2, OutLen) < 0)
				{
					delete [] buf2;
					lNumRead = -1;
					break;
				}
				ProgressAdd(lNumRead);
			}
			delete [] buf2;
			if (lNumRead < 0) break;
		}
		else
		{
			BlockSize = lBufSize;
			if (EncoType != ET_ASCII)
				BlockSize /= 2;
			while (SUCCEEDED(InFile.GetNextBlock(&lNumRead)) && (lNumRead > 0))
			{
				for (int i = 0; i < lNumRead; i += BlockSize)
				{
					LONG InLen = min(BlockSize, lNumRead - i);
					LONG OutLen;
					TheEncoder->Encode(FileBuf + i, InLen, buf, OutLen);
					if (SendDataText(buf, OutLen) < 0)
						lNumRead = -1;
					else
						ProgressAdd(InLen);
				}
				if (lNumRead < 0)
					break;
			}
			if (lNumRead < 0)
			{
				ErrorDialog(IDS_ERR_BINHEX_READ, Filename);
				break;
			}
		}
		
		// Finish up encoding and write out remaining bytes
		if (TheEncoder->Done(buf, lNumRead)) break;
		if (SendDataText(buf, lNumRead) < 0) break;
		
		delete TheEncoder;
		return (1);
	}

	delete TheEncoder;
	return (-1);
}

// SendAttachment
int SendAttachment(const char* Filename, const char* boundary, const char* ContentID = NULL, BOOL IsInline = FALSE)
{
	CFileStatus Status;
	long et;
	MIMEMap mm;

	if (!Filename)
	{
		ASSERT(0);
		return (-1);
	}

	ASSERT(boundary != NULL && *boundary != 0);

	while (1)
	{
		PushProgress();
		sprintf(buf, CRString(IDS_SMTP_SENDING_FILE), Filename);
		Progress(0, buf, -1);
		if (!CFile::GetStatus(Filename, Status))
		{
			ErrorDialog(IDS_ERR_OPEN_ATTACH, Filename);
			break;
		}

		if (boundary && NetConnection->PutLine(boundary) < 0) break;

		// Special .MIM extension attachment.  If we find a Content-Type: header in the file,
		// then assume it's already a MIME part, and send it out as is.
		if (stricmp(Filename + strlen(Filename) - 4, ".MIM") == 0)
		{
			CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
			JJFile RawFile;
			long NumBytes;
			if (FAILED(RawFile.Open(Filename, O_RDONLY)))
				return (-1);
			while (SUCCEEDED(RawFile.GetLine(buf, lBufSize, &NumBytes)) && NumBytes > 0)
			{
				if (strnicmp(buf, ContentType, ContentType.GetLength()) == 0)
				{
					RawFile.Close();
					return SendRawMIME(Filename);
				}
				// Found a blank line or a non-header-looking line,
				// then we know it's not a MIME part.
				if (0 == *buf || (!isspace((int)(unsigned char)*buf) && !strchr(buf, ':')))
					break;
			}
			RawFile.Close();
		}

		Progress(0, NULL, Status.m_size);
		
		if ((et = EncodingType(Filename, mm, IsInline || !SMTPmsg->m_Sum->TextAsDoc())) < 0) break;
		
		if (!mm.m_Creator[0])
			strcpy(mm.m_Creator, CRString(et? IDS_CREATOR_TEXT : IDS_CREATOR_UNKNOWN));
		if (!mm.m_Type[0])
			strcpy(mm.m_Type, CRString(et? IDS_ATTACH_TEXT_TYPE : IDS_TYPE_UNKNOWN));
		
		if (SMTPmsg->m_Sum->Encoding() == MSF_BINHEX &&
			(et == ET_BINARY || SMTPmsg->m_Sum->TextAsDoc()))
		{
			CBinHexMT BinHex(NetConnection);
			
			if (BinHex.SendBinHex(Filename, mm, et, Status.m_size - (et? et - 1 : 0), ContentID, IsInline) < 0) break;
		}
		else if (AttachMIME(Filename, mm, et, ContentID, IsInline) < 0)
			break;
		
		PopProgress();
		return (1);
	}

	PopProgress();
	return (-1);
}

char* GetSignature(CCompMessageDoc* CompDoc, BOOL ConvertRichToHTML)
{
	char SigFilename[_MAX_PATH + 1];
	char* TheSig = NULL;
	struct stat st;

	int WhichSig = CompDoc->m_Sum->UseSignature();
	switch (WhichSig)
	{
		case 0:
			return NULL;	// Do nothing if signature is not used
		case MSF_USE_SIGNATURE:
		case MSF_USE_SIGNATURE | MSF_ALT_SIGNATURE:
			strcpy(SigFilename, EudoraDir);
			strcat(SigFilename, CRString(IDS_SIG_FILENAME_FIRST + (WhichSig == MSF_USE_SIGNATURE? 0 : 1)));
			break;
		case MSF_ALT_SIGNATURE:
			// build the folder name
			strcpy(SigFilename, EudoraDir);
			strcat(SigFilename, CRString( IDS_SIGNATURE_FOLDER ) + "\\" + CompDoc->m_Sum->m_SigSelected + "." + CRString(IDS_SIGNATURE_EXTENSION));
			break;
	}

	if (!stat(SigFilename, &st) && (TheSig = new char[st.st_size + 1]))
	{
		JJFile in(TheSig, (int)st.st_size);

		long lBytesRead = 0;

		if (FAILED(in.Open(SigFilename, O_RDONLY)))
		{
			delete [] TheSig;
			TheSig = NULL;
		}
		else if (FAILED(in.GetNextBlock(&lBytesRead)))
		{
			ASSERT(lBytesRead < 0);
			delete [] TheSig;
			TheSig = NULL;
		}
		else
			TheSig[st.st_size] = 0;
	}

	if (TheSig && ConvertRichToHTML && IsFancy(TheSig) == IS_RICH)
	{
		int NewSize = Etf2Html_Measure(TheSig);
		char* NewSignature = new char[NewSize + 1];
		Etf2Html_Convert(NewSignature, TheSig);
		delete [] TheSig;
		TheSig = NewSignature;
	}

	return TheSig;
}


// XmitMessageHi
int XmitMessageHi()
{
	CString XLate;
	XLate = SMTPmsg->m_Sum->GetTranslators();
	if (!XLate.IsEmpty())
		return (TranslateLiveOut(SMTPmsg));
	else 
		return XmitMessage(TRUE);

}

int XMitHeaders()
{
	CRString Version(IDS_VERSION);
	CRString csMimeVersion( IDS_MIME_HEADER_VERSION );
	csMimeVersion.MakeUpper();
	CString tmpHeaders;

	char* Header = NULL;
	char Space = ' ';

	// We use to build up and send the message-id header here.
	// Now the Message-Id is in the MBX file so we don't have to do
	// anything special.  I left the code below just in case we somehow
	// send a message that does not have a message id assigned.  (Like maybe
	// something from MAPI of command line...

	if ( SMTPmsg->m_MessageId.IsEmpty() )
	{
		if (NetConnection->Put(CRString(IDS_HEADER_MESSAGE_ID)) < 0) goto fail;
		if (NetConnection->Put('<') < 0) goto fail;
		strcpy(buf, strchr(strchr(Version, ' ') + 1, ' ') + 1);
		*strchr(buf, ' ') = 0;
		if (NetConnection->Put(buf) < 0) goto fail;
		if (NetConnection->Put('.') < 0) goto fail;
		if (NetConnection->Put(::FormatTimeMT((time_t)SMTPmsg->m_Sum->m_Seconds, "%Y%m%d%H%M%S")) < 0) goto fail;
		if (NetConnection->Put('.') < 0) goto fail;
		sprintf(buf, "%08lx", (DWORD)SMTPmsg);
		if (NetConnection->Put(buf) < 0) goto fail;
		const char* At = strrchr(GetIniString(IDS_INI_POP_ACCOUNT), '@');
		if (NetConnection->Put(At) < 0) goto fail;
		if (NetConnection->PutLine('>') < 0) goto fail;
	}

	// Extra headers
	GetIniString(IDS_INI_EXTRA_HEADERS, buf, lBufSize);
	if (*buf)
	{
		// 
		char* src = buf;
		char* dst = buf;
		while (*src)
		{
			if (*src != '\\' || !src[1])
				*dst++ = *src++;
			else
			{
				switch (*++src)
				{
				case 'r': *dst++ = '\r'; break;
				case 'n': *dst++ = '\n'; break;
				case 't': *dst++ = '\t'; break;
				default: *dst++ = *src;
				}
				src++;
			}
		}
		*dst = 0;
		if (dst[-1] != '\n')
		{
			// Add trailing <CR><LF> if not there
			if (dst[-1] != '\r')
				*dst++ = '\r';
			*dst++ = '\n';
			*dst = 0;
		}
		if (NetConnection->Put(buf) < 0) goto fail;
	}

	if ( ! SMTPmsg->m_ExtraHeaders.IsEmpty() )
	{
		if (NetConnection->Put( SMTPmsg->m_ExtraHeaders ) < 0) goto fail;
	}
	
	if (!SMTPmsg->m_Sum->GetPrecedence().IsEmpty())
	{
		if (NetConnection->Put(CRString(IDS_PRECEDENCE_HEADER)) < 0) goto fail;
		if (NetConnection->PutLine(SMTPmsg->m_Sum->GetPrecedence()) < 0) goto fail;

	}
	
	// X-Sender
//	if (!UsingPOPSend)
	{
		strcpy(buf + 128, GetIniString(IDS_INI_POP_ACCOUNT));
		StripAddress(buf + 128);
		if (POPPassword.IsEmpty())
			strcat(buf + 128, CRString(IDS_SMTP_HEADER_UNVER));
		sprintf(buf, CRString(IDS_SMTP_HEADER_SENDER), buf + 128);
		if (NetConnection->Put(buf) < 0) goto fail;
	}

	if (SMTPmsg->m_Sum->ReturnReceipt() || SMTPmsg->m_Sum->ReadReceipt())
	{
		char* ReturnAddr = StripAddress(::SafeStrdupMT(GetReturnAddress()));
		if (ReturnAddr)
		{
			// Return Receipt
			if (SMTPmsg->m_Sum->ReturnReceipt())
			{
				if (SMTPmsg->m_Sum->IsMDN())
				{
					// Naughty, naughty!
					// MDN responses shouldn't have MDN requests, otherwise you could get
					// yourself in to a nasty mail loop.
					ASSERT(0);
				}
				else
				{
					if (NetConnection->Put(CRString(IDS_SMTP_HEADER_RR)) < 0) goto fail;
					if (NetConnection->Put(Space) < 0) goto fail;
					if (NetConnection->PutLine(ReturnAddr) < 0) goto fail;
				}
			}

			// Read Receipt
			if (SMTPmsg->m_Sum->ReadReceipt())
			{
				if (SMTPmsg->m_Sum->IsMDN())
				{
					// Naughty, naughty!
					// MDN responses shouldn't have MDN requests, otherwise you could get
					// yourself in to a nasty mail loop.
					ASSERT(0);
				}
				else
				{
					if (NetConnection->Put(CRString(IDS_MIME_HEADER_READR)) < 0) goto fail;
					if (NetConnection->Put(Space) < 0) goto fail;
					if (NetConnection->Put('<') < 0) goto fail;
					if (NetConnection->Put(ReturnAddr) < 0) goto fail;
					if (NetConnection->PutLine('>') < 0) goto fail;
				}

			}

			delete [] ReturnAddr;
		}
	}

    // Version
   	sprintf(buf, CRString(IDS_SMTP_HEADER_VERSION), (const char*)Version);
	if (NetConnection->PutLine(buf) < 0) goto fail;


	// Priority
	short Prior;
	Prior = SMTPmsg->m_Sum->m_Priority;
	if (Prior != MSP_NORMAL)
	{
		sprintf(buf, CRString(IDS_HEADER_PRIORITY), Prior,
			(const char*)CRString(IDS_PRIORITY_HIGHEST + Prior - 1));
		if (NetConnection->PutLine(buf) < 0) goto fail;
    }

	// Time this message is going out
	char Date[64];
	ComposeDate(Date, SMTPmsg->m_Sum);

	// Date
	if (*Date)
		if (NetConnection->PutLine(Date) < 0) goto fail;

	// Do the headers that show up in the composition window
	if (SendAddressHeader(HEADER_TO) < 0) goto fail;
	if (SendAddressHeader(HEADER_FROM) < 0) goto fail;
	if (SendNormalHeader(HEADER_SUBJECT) < 0) goto fail;
	if (SendAddressHeader(HEADER_CC) < 0) goto fail;
	if (UsingPOPSend)
		if (SendAddressHeader(HEADER_BCC) < 0) goto fail;

	// Reply thread stuff
	if (SendNormalHeader(HEADER_IN_REPLY_TO) < 0) goto fail;
	if (SendReferencesHeader(HEADER_REFERENCES) < 0) goto fail;

	tmpHeaders = SMTPmsg->m_ExtraHeaders;
	tmpHeaders.MakeUpper();
	if( ( tmpHeaders.Find( csMimeVersion ) < 0 ) &&
		GetIniShort( IDS_INI_SEND_XATTACH_HEADER ))
	{
		if ( SendNormalHeader(HEADER_ATTACHMENTS) < 0 )
		{
			goto fail;
		}
	}

	return (0);

fail:
	delete Header;
	return (-1);
}

// SendTextContext
//
// Combine the two text parts in to just one part (this should only happen
// if there are no attachments and no embedded objects, but we don't know
// about that at this level), and send the parts by calling SendPlain() or
// SendStyled().
//
int SendTextContext(char* Text1, char* Text2)
{
	// We've got to do something!
	ASSERT(Text1 != NULL);

	const BOOL bText1HTML = (IsFancy(Text1) == IS_HTML);
	const BOOL bText2HTML = (IsFancy(Text2) == IS_HTML);
	const BOOL bPlainOnly = SMTPmsg->m_Sum->SendPlainOnly();
	const BOOL bSendFormatFlowed = GetIniShort(IDS_INI_SEND_FORMAT_FLOWED);

	if (!Text2)
	{
		if (bText1HTML == FALSE)
		{
			if (!bSendFormatFlowed)
				return SendPlain(Text1);

			char* EscapedText = EscapeFormatFlowed(Text1);
			int iRet = SendPlain(EscapedText);
			delete [] EscapedText;
			return iRet;
		}

		if (bPlainOnly)
		{
			CString PlainText(Html2Text(Text1, bSendFormatFlowed));
			return SendPlain(PlainText);
		}

		return SendStyled(Text1);
	}

	// Things get trickier here with two parts.  They're going to be joined together
	// and sent as one MIME part, so figure out which type it's going to be.  If no
	// styled are supposed to be sent, then strip HTML parts.  Otherwise, if either
	// part is HTML, then it will be sent as HTML.

	BOOL bSendHTML = !bPlainOnly && (bText1HTML || bText2HTML);

	if (!bSendHTML)
	{
		const char* szPlainText1;
		const char* szPlainText2;
		CString csPlainText1;
		CString csPlainText2;

		// Strip HTML if necessary
		if (!bText1HTML)
		{
			if (!bSendFormatFlowed)
				szPlainText1 = Text1;
			else
			{
				char* EscapedText1 = EscapeFormatFlowed(Text1);
				csPlainText1 = EscapedText1;
				szPlainText1 = csPlainText1;
				delete [] EscapedText1;
			}
		}
		else
		{
			csPlainText1 = Html2Text(Text1, bSendFormatFlowed);
			szPlainText1 = csPlainText1;
		}
		if (!bText2HTML)
		{
			if (!bSendFormatFlowed)
				szPlainText2 = Text2;
			else
			{
				char* EscapedText2 = EscapeFormatFlowed(Text2);
				csPlainText2 = EscapedText2;
				szPlainText2 = csPlainText2;
				delete [] EscapedText2;
			}
		}
		else
		{
			csPlainText2 = Html2Text(Text2, bSendFormatFlowed);
			szPlainText2 = csPlainText2;
		}

		const char SigSeparator[] = "\r\n";
		int Len1 = strlen(szPlainText1);
		int Len2 = strlen(szPlainText2);
		int SigSepLen = strlen(SigSeparator);
		char* CombinedPlain = new char[Len1 + Len2 + SigSepLen + 1];
		strcpy(CombinedPlain, szPlainText1);
		strcpy(CombinedPlain + Len1, SigSeparator);
		strcpy(CombinedPlain + Len1 + SigSepLen, szPlainText2);

		int status = SendPlain(CombinedPlain);
		delete [] CombinedPlain;
		return status;
	}
	else
	{
		const char* szHtmlText1;
		const char* szHtmlText2;
		CString csHtmlText1;
		CString csHtmlText2;

		// Generate HTML if necessary
		if (bText1HTML)
			szHtmlText1 = Text1;
		else
		{
			csHtmlText1 = "<html>";
			csHtmlText1 += Text2Html(Text1, TRUE, FALSE);
			szHtmlText1 = csHtmlText1;
		}
		if (bText2HTML)
			szHtmlText2 = Text2;
		else
		{
			csHtmlText2 = Text2Html(Text2, TRUE, FALSE);
			csHtmlText2 += "</html>";
			szHtmlText2 = csHtmlText2;
		}

		if (bText1HTML)
		{
			// Need to get rid of closing </html> of the first
			// part when combining with the second part
			ASSERT(szHtmlText1 == Text1);
			char* end = (char*)szHtmlText1 + strlen(szHtmlText1) - 1;
			while (isspace((int)(unsigned char)*end) && end > szHtmlText1)
				end--;
			if (end > szHtmlText1 + 6)
			{
				if (strnicmp(end - 6, "</html>", 7) == 0)
					end[-6] = 0;
			}
		}

		if (bText2HTML)
		{
			// Need to get rid of <html> tag
			ASSERT(szHtmlText2 == Text2);
			if (strnicmp(szHtmlText2, "<html>", 6) == 0)
				szHtmlText2 += 6;
		}

		const char SigSeparator[] = "<br>\r\n";
		int Len1 = strlen(szHtmlText1);
		int Len2 = strlen(szHtmlText2);
		int SigSepLen = strlen(SigSeparator);
		char* CombinedHtml = new char[Len1 + Len2 + SigSepLen + 1];
		strcpy(CombinedHtml, szHtmlText1);
		strcpy(CombinedHtml + Len1, SigSeparator);
		strcpy(CombinedHtml + Len1 + SigSepLen, szHtmlText2);

		int status = SendStyled(CombinedHtml);
		delete [] CombinedHtml;
		return status;
	}
}

// SendEmbeddedContext
//
// If the message has embedded objects, then set up the multipart/related type, send the
// text part, and send the embedded objects.  No embedded objects degenerates to just
// a call to SendTextContext().
//
int SendEmbeddedContext(const CString& EOHeader, char* Text1)
{
	const BOOL bHasEmbedded = (EOHeader.IsEmpty() == FALSE);
	char Boundary[128];

	if (bHasEmbedded)
	{
		// Send Content-Type: multipart/related, blank line, and multipart boundary
		if (SendMultipart(Boundary, MT_RELATED) < 0)
			return -1;
	}

	// Send text part
	if (SendTextContext(Text1, NULL) < 0)
		return -1;

	if (bHasEmbedded)
	{
		// Send each embedded object
		const char* Index = EOHeader;
		while (Index = strchr(Index, '<'))
		{
			if (strnicmp(++Index, "cid:", 4) == 0)
				Index += 4;
			const char* Equal = strchr(Index, '=');
			ASSERT(Equal != NULL);
			if (Equal)
			{
				CString CID(Index, Equal - Index);
				Index = strchr(Equal, '>');
				ASSERT(Index != NULL);
				if (Index)
				{
					Equal++;
					CString Attach(Equal, Index - Equal);
					if (SendAttachment(Attach, Boundary, CID, TRUE) < 0)
						return -1;
				}
			}
		}

		// Send closing boundary
		if (NetConnection->Put(Boundary) < 0 ||
			NetConnection->PutLine("--") < 0)
		{
			return -1;
		}
	}

	return 1;
}

// XmitMessage
// Actually processes and transmits the message
//
int XmitMessage(BOOL SendBody /*= TRUE*/, BOOL StdHdrs /*= TRUE*/)
{
	//
	// Make sure the QCMessage stuff embedded inside the CMessageDoc
	// loads the latest stuff into memory.
	//
	{
		char* pszDummy = SMTPmsg->GetFullMessage();
		if (pszDummy)
			delete [] pszDummy;
	}

	int		status;
	char*	MessageBody = ::SafeStrdupMT(SMTPmsg->GetText());
	char	boundary[128];

#ifndef THREADED	
	// Translate the message now 
	NetConnection->Reset();	// Reset data struct for sending the message body
#endif

	// Put up the subject line for progress	
	Progress(0, SMTPmsg->GetHeaderLine(HEADER_SUBJECT), ::SafeStrlenMT(MessageBody));

	if (StdHdrs)
	{
		if (XMitHeaders() < 0)
		{
			delete [] MessageBody;
			return -1;
		}
	}

	if (!SendBody)
	{
		delete [] MessageBody;
		return 1;
	}

	// Is this a MIME-encoded .msg file?
	CString		tmpHeaders;
	tmpHeaders = SMTPmsg->m_ExtraHeaders;
	tmpHeaders.MakeUpper();
	CRString csMimeVersion( IDS_MIME_HEADER_VERSION );
	csMimeVersion.MakeUpper();
	BOOL bSendRaw = FALSE;
	if (tmpHeaders.Find( csMimeVersion ) >= 0  || SMTPmsg->m_Sum->IsMimeAttached())
		bSendRaw = TRUE;
		
	CString EOHeader;
	if (FAILED(SMTPmsg->m_QCMessage.GetEmbeddedObjectHeaders(EOHeader)))
	{
		delete [] MessageBody;
		return -1;
	}
	BOOL bHasEmbedded = (EOHeader.IsEmpty() == FALSE);
	const BOOL bHasAttachments = (SMTPmsg->IsHeaderEmpty(HEADER_ATTACHMENTS) == FALSE);
	Signature = GetSignature(SMTPmsg, TRUE);
	const BOOL bIsMixed = bHasAttachments || (bHasEmbedded && Signature);

	if (!bSendRaw)
	{
		// Send "Mime-Version: 1.0"
		if (NetConnection->Put(CRString(IDS_MIME_HEADER_VERSION)) < 0 ||
			NetConnection->Put(' ') < 0 ||
			NetConnection->PutLine(CRString(IDS_MIME_VERSION)) < 0)
		{
			return -1;
		}

		// If there are attachments, or embedded objects and a signature,
		// then this message will be sent with a top-level type of multipart/mixed
		if (bIsMixed)
		{
			if (SendMultipart(boundary, MT_MIXED) < 0)
				goto fail;
		}
		else
		{
			*boundary = 0;
		}

		// If there are attachments or embedded objects, then the body and signature will
		// be sent separately.  If not, then send them at the same time.
		if (bHasAttachments || bHasEmbedded)
		{
			if (SendEmbeddedContext(EOHeader, MessageBody) < 0)
				goto fail;
		}
		else

		{
			if (SendTextContext(MessageBody, Signature) < 0)
				goto fail;

			// Get rid of signature so it's not sent later
			delete [] Signature;
			Signature = NULL;
		}
	}
		
	// Attach documents if necessary
	if (bSendRaw || bHasAttachments)
	{
		CString AttachStr(SMTPmsg->GetHeaderLine(HEADER_ATTACHMENTS));
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
			if (bSendRaw)
			{
				if (SendRawMIME( Attach ) < 0)
					goto fail;
			}
			else
			{
				if (SendAttachment(Attach, boundary) < 0)
					goto fail;
			}

			Attach = t;
		}
	}

	if (!bSendRaw)
	{
		if (Signature)
		{
			// Better be mixed here, otherwise we should have already sent the signature
			ASSERT(bIsMixed);

			// Boundary between previous parts and signature
			if (NetConnection->PutLine(boundary) < 0)
				goto fail;

			// Send the sig
			// Maybe some day we'll handle objects in signatures
			//if (SendEmbeddedContext(EmptyEOHeader, Signature) < 0)
			if (SendTextContext(Signature, NULL) < 0)
				goto fail;

			delete [] Signature;
			Signature = NULL;
		}

		// Write out final boundary
		if (bIsMixed)
		{
			if (NetConnection->Put(boundary) < 0 || NetConnection->PutLine("--") < 0)
				goto fail;
		}
	}

	NetConnection->Flush();

#ifndef THREADED
	if (!NetConnection->IsFileBased())
#endif
		status = FinishSMTP();

	Progress(1, NULL, 1);

	delete [] MessageBody;
	
	return (status);

fail:
	delete [] Signature;
	Signature = NULL;

	delete [] MessageBody;
	
	return (-1);
}

// This function assumes connection is already started (such as withi XMitMessage)
int SendRawMIME(const char *FileName)
{
	JJFile rawMIME;
	const char * base;
	const UINT kbufsize = (4*1024);
	char buf[kbufsize+1];
	memset(buf,0, kbufsize + 1);
	long lNumRead = 0;

	if (FAILED(rawMIME.Open(FileName, O_RDONLY)))
		goto fail;
	
	while (SUCCEEDED(rawMIME.RawRead(buf, kbufsize, &lNumRead)) && (lNumRead > 0))
	{
		NetConnection->Put(buf, lNumRead);
	}
	
	if (lNumRead < 0)
		goto fail;
	
	NetConnection->Flush();	// Make sure all pending data has been put onto the net

	// nuke 'em return receipts
	base = strrchr( FileName, '\\' );
	if ( base ) 
		base++;					// skip the '\\'
	else
		base = FileName;
	if ( CompareRString(IDS_READ_RECEIPT_FILE, base) == 0 )
		rawMIME.Delete();		// assume it's a return receipt and dump it

	return (0);
fail:
	return (-1);
}


/*

// SendMessage
// Routine that actually sends a message, given all of the information
// regarding the text and send recipients etc
//
int SendMessage(CCompMessageDoc* msg)
{
	int retCode;

	// allocate the buffer
	for( lBufSize = PREFERED_BUFFER_SIZE; lBufSize > MINIMUM_BUFFER_SIZE; lBufSize -= 1024 )
	{
		buf = new char [ lBufSize ];
		if ( buf != NULL )
		{
			break;
		}
	}

	// check for errors
	if ( buf == NULL )
	{
		goto fail;
	}

	SMTPmsg = msg;


	if (!UsingPOPSend)
	{
		// Start by reseting the server
		if (SMTPCmdGetReply(IDS_SMTP_RESET, NULL, buf, lBufSize) < 0)
			goto fail;

        // Per the MDN RFC:
		// "The envelope sender address (i.e., SMTP MAIL FROM) of the MDN SHOULD
		// be null (<>), specifying that no Delivery Status Notification
		// messages or other messages indicating successful or unsuccessful
		// delivery are to be sent in response to an MDN."
		if (SMTPmsg->m_Sum->IsMDN())
			strcpy(buf, "<>");
		else
		{
			const char* ra = GetReturnAddress();
			if (ra)
				strcpy(buf, ra);
			else
				*buf = 0;
			StripAddress(buf);
			if (*buf != '<')
			{
				memmove(buf + 1, buf, strlen(buf) + 1);
				*buf = '<';
				strcat(buf, ">");
			}
		}

		if (SMTPCmdGetReply(IDS_SMTP_FROM, buf, buf, lBufSize) < 0)
			goto fail;
	}

	// Send the list of recipients.  Do Bcc Line first so that Bcc recipients don't
	// show up in Received: headers unless the message contains only Bcc recipients
	if (DoHeader(HEADER_BCC) < 0 || DoHeader(HEADER_TO) < 0 || DoHeader(HEADER_CC) < 0)
		goto fail;

	if (UsingPOPSend)
	{
		if (POPCmdGetReply(IDS_POP_SEND, NULL, buf, lBufSize) < 0)
			goto fail;
	}
	else
	{
		if (SMTPCmdGetReply(IDS_SMTP_DATA, NULL, buf, lBufSize) < 0)
			goto fail;
	}

	retCode = XmitMessageHi();
	if (retCode < 0)
		goto fail;

	// Get rid of MAPI attachments after sending, if told to do so	
	if (SMTPmsg->m_Sum->IsMAPI() && GetIniShort(IDS_INI_MAPI_DELETE_SEND) &&
		SMTPmsg->IsHeaderEmpty(HEADER_ATTACHMENTS) == FALSE)
	{
		CString AttachStr(SMTPmsg->GetHeaderLine(HEADER_ATTACHMENTS));
		char* Attach = AttachStr.GetBuffer(AttachStr.GetLength());
		char AttachDir[_MAX_PATH + 1];
		int ADLen;

		GetIniString(IDS_INI_AUTO_RECEIVE_DIR, AttachDir, sizeof(AttachDir));
		if (!*AttachDir)
			wsprintf(AttachDir,"%s%s",(const char *)EudoraDir, (const char *)CRString(IDS_ATTACH_FOLDER));
		ADLen = strlen(AttachDir);

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

	// handle .msg file auto attachments (MIME encoded messages)
	if (SMTPmsg->m_Sum->IsAutoAttached() && 
		GetIniShort(IDS_INI_AUTO_ATTACHED_DELETE_SEND) &&
		SMTPmsg->IsHeaderEmpty(HEADER_ATTACHMENTS) == FALSE)
	{
		// extract the attachment filename
		CString AttachStr(SMTPmsg->GetHeaderLine(HEADER_ATTACHMENTS));
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

			// expects a fully-qualified filename
			::FileRemoveMT( Attach );

			Attach = t;
		}
	}

	// free the temporary buffer
	delete [] buf;	
	buf = NULL;

	SMTPmsg = NULL;
	return (1);

fail:
	// free the temporary buffer
	delete [] buf;	
	buf = NULL;

	SMTPmsg = NULL;
	return (-1);
}

  */



/* ============================ QCSMTPMessage ========================== */

QCSMTPMessage::QCSMTPMessage(CCompMessageDoc* pMsgDoc)
{
	 Network* m_oldNetConnection= NULL;
	 CFileIO* m_fileIO = NULL;
 	 //
	 // Alert! setting global
	 //
	 SMTPmsg = pMsgDoc;

}


/* ================ Public Methods ====================*/
HRESULT QCSMTPMessage::Start(const char* filename)
{
	HRESULT hr = 0;
	int Status = 1;
 	
	DeleteNetObj = FALSE;
	SMTPConnected = FALSE;
	UsingPOPSend = FALSE;
	
	//Set UsingPOPSend
	UsingPOPSend = (GetIniShort(IDS_INI_USE_POP_SEND) != 0);


	// If a NetConnection exists here, we've got problems.
	// Odds are its a Network based NetConnection and I need a file one.
	// Just to be sure, shut down old and create a new file based one.
	//
	if (NetConnection)	 
	{
		// Someone didn't cleanup properly. Carry on in a safe way.
		ASSERT(TRUE);
		delete NetConnection;
		NetConnection = NULL;
	}
	
	if (!CreateNetConnection(TRUE,TRUE))
		return (E_FAIL);
	DeleteNetObj = TRUE;

	
	//ASSERT_KINDOF(CFileIO, ::NetConnection);
	Status = ((CFileIO*) NetConnection)->OpenFile(filename,_O_CREAT | _O_RDWR | _S_IREAD | _S_IWRITE);

	if (Status >= 0)
	{
		SMTPConnected = TRUE;
		hr = S_OK;
	}
		// allocate the buffer
	for( lBufSize = PREFERED_BUFFER_SIZE; lBufSize > MINIMUM_BUFFER_SIZE; lBufSize -= 1024 )
	{
		buf = new char [ lBufSize ];
		if ( buf != NULL )
		{
			break;
		}
	}
	if ( buf == NULL )
	{
		hr = E_FAIL;
	}


	return (hr);
}


// Close out session
HRESULT QCSMTPMessage::End()
{
	// What cleanup do I need.  Should I delete Sig?
	
	HRESULT hr=S_OK;

	if (Signature)			
	{
		delete [] Signature;
		Signature = 0;
	}
	
	if (SMTPConnected)
	{
		SMTPConnected = FALSE;
		NetConnection->Close();
	}

	if (DeleteNetObj == TRUE)
	{
		delete NetConnection;
		
		//close the progress bar as we created a progress bar

		//Let high-level layers to handle CloseProgress
		//CloseProgress();
		NetConnection = NULL;
	}
		// free the temporary buffer
	delete [] buf;	
	buf = NULL;


	return (hr);
}

HRESULT QCSMTPMessage::RedirectOutput(const char *FileName)
{
	ASSERT(!m_oldNetConnection);
		
	// This could potentially screw other globals such as Signature...

	// Save the old global
	m_oldNetConnection = NetConnection;
	::NetConnection = NULL;

	// This will set NetConnetion to a file I/O in the constructor
	m_fileIO = (CFileIO *) ::CreateNetConnection(FALSE, TRUE);
	ASSERT(::NetConnection == m_fileIO);
	//ASSERT_KINDOF(CFileIO, ::NetConnection);
	
	int err = m_fileIO->OpenFile(FileName,_O_CREAT | _O_RDWR | _S_IREAD | _S_IWRITE );

	return err;
}

HRESULT QCSMTPMessage::EndRedirect()
{
	ASSERT(NetConnection);
	HRESULT hr=S_OK;

	m_fileIO->Close();
	delete m_fileIO;

	// Reset connection back
	NetConnection = m_oldNetConnection;
	m_oldNetConnection=NULL;

	return hr;
}


HRESULT QCSMTPMessage::WriteRawMIME(const char *FileName)
{
	HRESULT hr=S_OK;
	JJFile rawMIME;
	const UINT kbufsize = (4*1024);
	char buf[kbufsize+1];
	memset(buf,0, kbufsize + 1);
	long numRead = 0;

	if (rawMIME.Open(FileName, O_RDONLY) < 0)
		return E_FAIL;
	
	while( (!FAILED(rawMIME.RawRead(buf, kbufsize, &numRead))) && numRead > 0 )
	{
		NetConnection->Put(buf, numRead);
	}
	
	if (numRead < 0)
		return(E_FAIL);
	
	NetConnection->Flush();	// Make sure all pending data has been put onto the net
	
	return (hr);
}

HRESULT QCSMTPMessage::WriteHash()
{
	return S_OK;

}


HRESULT QCSMTPMessage::WriteEnvelope()
{
	ASSERT(NetConnection);
	
	if (!UsingPOPSend)
	{
		// Start by reseting the server
		if (SMTPCmdGetReply(IDS_SMTP_RESET, NULL, buf, lBufSize) < 0)
			return E_FAIL;

        // Per the MDN RFC:
		// "The envelope sender address (i.e., SMTP MAIL FROM) of the MDN SHOULD
		// be null (<>), specifying that no Delivery Status Notification
		// messages or other messages indicating successful or unsuccessful
		// delivery are to be sent in response to an MDN."
		//
		// Some folks don't like this as it causes troubles with spam-prevention
		// SMTP server that reject messages not from the SMTP server's domain, so
		// we give the easy way out with an option to send the actual address.
		if (SMTPmsg->m_Sum->IsMDN() && GetIniShort(IDS_INI_MDN_SEND_ADDRESS) == FALSE)
			strcpy(buf, "<>");
		else
		{
			const char* ra = GetReturnAddress();
			if (ra)
				strcpy(buf, ra);
			else
				*buf = 0;
			StripAddress(buf);
			if (*buf != '<')
			{
				memmove(buf + 1, buf, strlen(buf) + 1);
				*buf = '<';
				strcat(buf, ">");
			}
		}
		if (SMTPCmdGetReply(IDS_SMTP_FROM, buf, buf, lBufSize) < 0)
			return E_FAIL;		
	}

	// Send the list of recipients.  Do Bcc Line first so that Bcc recipients don't
	// show up in Received: headers unless the message contains only Bcc recipients
	if (DoHeader(HEADER_BCC) < 0 || DoHeader(HEADER_TO) < 0 || DoHeader(HEADER_CC) < 0)
		return E_FAIL;

	return (S_OK);
}

HRESULT QCSMTPMessage::WriteHeaders()
{
	ASSERT(NetConnection);
	
	CRString Version(IDS_VERSION);
	CRString csMimeVersion( IDS_MIME_HEADER_VERSION );
	csMimeVersion.MakeUpper();
	CString tmpHeaders;

	char* Header = NULL;
	char Space = ' ';

	
	if (UsingPOPSend)
	{
		
		if (POPCmdGetReply(IDS_POP_SEND, NULL, buf, lBufSize) < 0)
			goto fail;
		
	}
	else
	{
		if (SMTPCmdGetReply(IDS_SMTP_DATA, NULL, buf, lBufSize) < 0)
			goto fail;
	}
	

	// We use to build up and send the message-id header here.
	// Now the Message-Id is in the MBX file so we don't have to do
	// anything special.  I left the code below just in case we somehow
	// send a message that does not have a message id assigned.  (Like maybe
	// something from MAPI of command line...

	if ( SMTPmsg->m_MessageId.IsEmpty() )

	{
		if (NetConnection->Put(CRString(IDS_HEADER_MESSAGE_ID)) < 0) goto fail;
		if (NetConnection->Put('<') < 0) goto fail;
		strcpy(buf, strchr(Version, ' ') + 1);
		*strchr(buf, ' ') = 0;
		if (NetConnection->Put(buf) < 0) goto fail;
		if (NetConnection->Put('.') < 0) goto fail;
		if (NetConnection->Put("32") < 0) goto fail;
		if (NetConnection->Put('.') < 0) goto fail;
		if (NetConnection->Put(FormatTimeMT((time_t)SMTPmsg->m_Sum->m_Seconds, "%Y%m%d%H%M%S")) < 0) goto fail;
		if (NetConnection->Put('.') < 0) goto fail;
		sprintf(buf, "%08lx", (DWORD)SMTPmsg);
		if (NetConnection->Put(buf) < 0) goto fail;
		const char* At = strrchr(GetIniString(IDS_INI_POP_ACCOUNT), '@');
		if (NetConnection->Put(At) < 0) goto fail;
		if (NetConnection->PutLine('>') < 0) goto fail;
	}

	// Extra headers
	GetIniString(IDS_INI_EXTRA_HEADERS, buf, lBufSize);
	if (*buf)
	{
		// 
		char* src = buf;
		char* dst = buf;
		while (*src)
		{
			if (*src != '\\' || !src[1])
				*dst++ = *src++;
			else
			{
				switch (*++src)
				{
				case 'r': *dst++ = '\r'; break;
				case 'n': *dst++ = '\n'; break;
				case 't': *dst++ = '\t'; break;
				default: *dst++ = *src;
				}
				src++;
			}
		}
		*dst = 0;
		if (dst[-1] != '\n')
		{
			// Add trailing <CR><LF> if not there
			if (dst[-1] != '\r')
				*dst++ = '\r';
			*dst++ = '\n';
			*dst = 0;
		}
		if (NetConnection->Put(buf) < 0) goto fail;
	}

	if ( ! SMTPmsg->m_ExtraHeaders.IsEmpty() )
	{
		if (NetConnection->Put( SMTPmsg->m_ExtraHeaders ) < 0) goto fail;
	}
	
	if (!SMTPmsg->m_Sum->GetPrecedence().IsEmpty())
	{
		if (NetConnection->Put(CRString(IDS_PRECEDENCE_HEADER)) < 0) goto fail;
		if (NetConnection->PutLine(SMTPmsg->m_Sum->GetPrecedence()) < 0) goto fail;

	}
	
	// X-Sender
//	if (!UsingPOPSend)
	{
		strcpy(buf + 128, GetIniString(IDS_INI_POP_ACCOUNT));
		StripAddress(buf + 128);
		if (POPPassword.IsEmpty())
			strcat(buf + 128, CRString(IDS_SMTP_HEADER_UNVER));
		sprintf(buf, CRString(IDS_SMTP_HEADER_SENDER), buf + 128);
		if (NetConnection->Put(buf) < 0) goto fail;
	}

	if (SMTPmsg->m_Sum->ReturnReceipt() || SMTPmsg->m_Sum->ReadReceipt())
	{
		char* ReturnAddr = StripAddress(::SafeStrdupMT(GetReturnAddress()));
		if (ReturnAddr)
		{
			// Return Receipt
			if (SMTPmsg->m_Sum->ReturnReceipt())
			{
				if (SMTPmsg->m_Sum->IsMDN())
				{
					// Naughty, naughty!
					// MDN responses shouldn't have MDN requests, otherwise you could get
					// yourself in to a nasty mail loop.
					ASSERT(0);
				}
				else
				{
					if (NetConnection->Put(CRString(IDS_SMTP_HEADER_RR)) < 0) goto fail;
					if (NetConnection->Put(Space) < 0) goto fail;
					if (NetConnection->PutLine(ReturnAddr) < 0) goto fail;
				}
			}

			// Read Receipt
			if (SMTPmsg->m_Sum->ReadReceipt())
			{
				if (SMTPmsg->m_Sum->IsMDN())
				{
					// Naughty, naughty!
					// MDN responses shouldn't have MDN requests, otherwise you could get
					// yourself in to a nasty mail loop.
					ASSERT(0);
				}
				else
				{
					if (NetConnection->Put(CRString(IDS_MIME_HEADER_READR)) < 0) goto fail;
					if (NetConnection->Put(Space) < 0) goto fail;
					if (NetConnection->Put('<') < 0) goto fail;
					if (NetConnection->Put(ReturnAddr) < 0) goto fail;
					if (NetConnection->PutLine('>') < 0) goto fail;
				}

			}

			delete [] ReturnAddr;
		}
	}

    // Version
   	sprintf(buf, CRString(IDS_SMTP_HEADER_VERSION), (const char*)Version);
	if (NetConnection->PutLine(buf) < 0) goto fail;


	// Priority
	short Prior;
	Prior = SMTPmsg->m_Sum->m_Priority;
	if (Prior != MSP_NORMAL)
	{
		sprintf(buf, CRString(IDS_HEADER_PRIORITY), Prior,
			(const char*)CRString(IDS_PRIORITY_HIGHEST + Prior - 1));
		if (NetConnection->PutLine(buf) < 0) goto fail;
    }

	// Time this message is going out
	char Date[64];
	ComposeDate(Date, SMTPmsg->m_Sum);

	// Date
	if (*Date)
		if (NetConnection->PutLine(Date) < 0) goto fail;

	// Do the headers that show up in the composition window
	if (SendAddressHeader(HEADER_TO) < 0) goto fail;
	if (SendAddressHeader(HEADER_FROM) < 0) goto fail;
	if (SendNormalHeader(HEADER_SUBJECT) < 0) goto fail;
	if (SendAddressHeader(HEADER_CC) < 0) goto fail;
	if (UsingPOPSend)
		if (SendAddressHeader(HEADER_BCC) < 0) goto fail;

	// Reply thread stuff
	if (SendNormalHeader(HEADER_IN_REPLY_TO) < 0) goto fail;
	if (SendReferencesHeader(HEADER_REFERENCES) < 0) goto fail;

	tmpHeaders = SMTPmsg->m_ExtraHeaders;
	tmpHeaders.MakeUpper();
	if( ( tmpHeaders.Find( csMimeVersion ) < 0 ) &&
		GetIniShort( IDS_INI_SEND_XATTACH_HEADER ) )
	{
		if ( SendNormalHeader(HEADER_ATTACHMENTS) < 0 )
		{
			goto fail;
		}
	}

	return (S_OK);

fail:
	delete Header;
	return (E_FAIL);
}


HRESULT QCSMTPMessage::WriteBody()
{
	// Getting rid of this code the easy way, commenting out the body.
	// We better never get here.
	
	//
	// Make sure the QCMessage stuff embedded inside the CMessageDoc
	// loads the latest stuff into memory.
	//
	{
		char* pszDummy = SMTPmsg->GetFullMessage();
		if (pszDummy)
			delete [] pszDummy;
	}

	int		status;
	char*	MessageBody = ::SafeStrdupMT(SMTPmsg->GetText());
	char	boundary[128];

	// Translate the message now 
//	NetConnection->Reset();	// Reset data struct for sending the message body

	// Put up the subject line for progress	
	Progress(0, SMTPmsg->GetHeaderLine(HEADER_SUBJECT), ::SafeStrlenMT(MessageBody));

	//if (StdHdrs)
	//{
	//	if (XMitHeaders() < 0)
	//	{
	//		delete [] MessageBody;
	//		return -1;
	//	}
	//}

	//if (!SendBody)
	//{
	//	delete [] MessageBody;
	//	return 1;
	//}

	
	// Is this a MIME-encoded .msg file?
	CString		tmpHeaders;
	tmpHeaders = SMTPmsg->m_ExtraHeaders;
	tmpHeaders.MakeUpper();
	CRString csMimeVersion( IDS_MIME_HEADER_VERSION );
	csMimeVersion.MakeUpper();
	BOOL bSendRaw = FALSE;
	if (tmpHeaders.Find( csMimeVersion ) >= 0  || SMTPmsg->m_Sum->IsMimeAttached())
		bSendRaw = TRUE;
		
	CString EOHeader;
	if (FAILED(SMTPmsg->m_QCMessage.GetEmbeddedObjectHeaders(EOHeader)))
	{
		delete [] MessageBody;
		return E_FAIL; //-1;
	}
	BOOL bHasEmbedded = (EOHeader.IsEmpty() == FALSE);
	const BOOL bHasAttachments = (SMTPmsg->IsHeaderEmpty(HEADER_ATTACHMENTS) == FALSE);
	Signature = GetSignature(SMTPmsg, TRUE);
	const BOOL bIsMixed = bHasAttachments || (bHasEmbedded && Signature);

	if (!bSendRaw)
	{
		// Send "Mime-Version: 1.0"
		if (NetConnection->Put(CRString(IDS_MIME_HEADER_VERSION)) < 0 ||
			NetConnection->Put(' ') < 0 ||
			NetConnection->PutLine(CRString(IDS_MIME_VERSION)) < 0)
		{
			return E_FAIL; //-1;
		}

		// If there are attachments, or embedded objects and a signature,
		// then this message will be sent with a top-level type of multipart/mixed
		if (bIsMixed)
		{
			if (SendMultipart(boundary, MT_MIXED) < 0)
				goto fail;
		}
		else
		{
			*boundary = 0;
		}

		// If there are attachments or embedded objects, then the body and signature will
		// be sent separately.  If not, then send them at the same time.
		if (bHasAttachments || bHasEmbedded)
		{
			if (SendEmbeddedContext(EOHeader, MessageBody) < 0)
				goto fail;
		}
		else

		{
			if (SendTextContext(MessageBody, Signature) < 0)
				goto fail;

			// Get rid of signature so it's not sent later
			delete [] Signature;
			Signature = NULL;
		}
	}
		
	// Attach documents if necessary
	if (bSendRaw || bHasAttachments)
	{
		CString AttachStr(SMTPmsg->GetHeaderLine(HEADER_ATTACHMENTS));
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
			if (bSendRaw)
			{
				if (SendRawMIME( Attach ) < 0)
					goto fail;
			}
			else
			{
				if (SendAttachment(Attach, boundary) < 0)
					goto fail;
			}

			Attach = t;
		}
	}

	if (!bSendRaw)
	{
		if (Signature)
		{
			// Better be mixed here, otherwise we should have already sent the signature
			ASSERT(bIsMixed);

			// Boundary between previous parts and signature
			if (NetConnection->PutLine(boundary) < 0)
				goto fail;

			// Send the sig
			// Maybe some day we'll handle objects in signatures
			//if (SendEmbeddedContext(EmptyEOHeader, Signature) < 0)
			if (SendTextContext(Signature, NULL) < 0)
				goto fail;

			delete [] Signature;
			Signature = NULL;
		}

		// Write out final boundary
		if (bIsMixed)
		{
			if (NetConnection->Put(boundary) < 0 || NetConnection->PutLine("--") < 0)
				goto fail;
		}
	}

	NetConnection->Flush();


//	if (!NetConnection->IsFileBased())
		status = FinishSMTP();

	Progress(1, NULL, 1);

	delete [] MessageBody;
	
	return S_OK; //(status);

fail:
	delete [] Signature;
	Signature = NULL;

	delete [] MessageBody;
	
	return E_FAIL; //return (-1);
	
	
	

/*
	
	
	if( XmitMessage())
		return S_OK;
	else
		return E_FAIL;

	//ASSERT(FALSE);
	//return (E_FAIL);
	ASSERT(NetConnection);
	//
	// Make sure the QCMessage stuff embedded inside the CMessageDoc
	// loads the latest stuff into memory.
	//
	{
		char* pszDummy = SMTPmsg->GetFullMessage();
		if (pszDummy)
			delete [] pszDummy;
	}

	int status;
	char*		MessageBody = ::SafeStrdupMT(SMTPmsg->GetText());
	
		// Get signature
		Signature = GetSignature(SMTPmsg, TRUE);

		// 
		// HTML text processing.
		// If either the sig or body is HTML then the entire content
		// type of the message is flagged as HTML.  Since this is an OR 
		// operation either the body or signature may not be HTML.  In
		// that case, the plain text must first be encoded as Text/HTML
		// before it is sent out.  These variables set up this whole process.
		//
		BOOL bBodyHTML	= SMTPmsg->m_Sum->IsHTML();
		BOOL bSigHTML	= (IsFancy(Signature) == IS_HTML);
		BOOL bStripHTML = GetIniShort( IDS_INI_SEND_PLAIN_ONLY );
		BOOL HasAttachments = (SMTPmsg->IsHeaderEmpty(HEADER_ATTACHMENTS) == FALSE);
		BOOL bSendBodyHTML = FALSE;
		BOOL bSendSigHTML = FALSE;
		int EncodeBody = ENCODE_NONE;
		int EncodeSig  = ENCODE_NONE;
		int BodyContentFlag = 0;
		int SigContentFlag = 0;
		char* MRBoundary;
		TextReader	theTextReader;
		BOOL HasEmbedded;
		CString EOHeader;
		if (FAILED(SMTPmsg->m_QCMessage.GetEmbeddedObjectHeaders(EOHeader)))
			goto fail;
		HasEmbedded = (EOHeader.IsEmpty() == FALSE);

		// If we are doing multipart, then the body and the signature get sent as different
		// parts, and so can have different MIME types.  If there are no attachments, then
		// the body and the signature get combined, and if either of them is HTML then
		// both need to be sent as HTML.
		if (!bStripHTML)
		{
			if (HasAttachments || HasEmbedded)
			{
				bSendBodyHTML = bBodyHTML;
				bSendSigHTML = bSigHTML;
			}
			else if (bSigHTML || bBodyHTML)
			{
				bSendBodyHTML = bSendSigHTML = TRUE;
				// Don't encode as Text/HTML if encoded already
				EncodeBody	|= (bBodyHTML ? ENCODE_NONE : ENCODE_RICH);
				EncodeSig	|= (bSigHTML ? ENCODE_NONE : ENCODE_RICH);
			}
		}

		if (bSendBodyHTML)
			BodyContentFlag = MSF_XRICH;
		if (bSendSigHTML)
			SigContentFlag = MSF_XRICH;

		BOOL		NeedQP;
		char		boundary[128];
		if ((NeedQP = SendMIMEHeaders(boundary, BodyContentFlag)) < 0) goto fail;
	
		// Do embedded objects with multipart/related
		char BoundaryBuf[sizeof(boundary)];
		MRBoundary = boundary;
		if (HasEmbedded)
		{
			if (HasAttachments)
			{
				// If the message also has attachments, then send the multipart/mixed
				// boundary and the the multipart/related header
				if (NetConnection->PutLine(boundary) < 0) goto fail;
				MRBoundary = BoundaryBuf;
				if (SendMultipart(MRBoundary, TRUE) < 0) goto fail;
			}
			else
			{
				// If this doesn't have attachments, then we've already sent the
				// multipart/related header, and have already generated a boundary
				MRBoundary = boundary;
			}
		}
		
		// Blank line separating headers from body
		if (NetConnection->PutLine() < 0) goto fail;

		if (XMitMIMEBoundary(MRBoundary, &NeedQP, BodyContentFlag) < 0) goto fail;
		
		// Now that all of the header records have been sent, go
		// ahead and send the body of the message

		EncodeBody |= NeedQP ? ENCODE_QP : ENCODE_NONE;
		EncodeSig  |= NeedQP ? ENCODE_QP : ENCODE_NONE;

		if (!*boundary && bBodyHTML && Signature)
		{
			// Need to get rid of closing </html> tag if we're combining
			// the body of the message along with the signature
			char* end = MessageBody + strlen(MessageBody) - 1;
			while (isspace((int)(unsigned char)*end) && end > MessageBody)
				end--;
			if (end > MessageBody + 6)
			{
				if (strnicmp(end - 6, "</html>", 7) == 0)
					end[-6] = 0;
			}
		}

		if (bBodyHTML && bSendBodyHTML)
		{
			if (SendEnriched(MessageBody, EncodeBody) < 0)
				goto fail;
		}
		else 
		{
			if (bBodyHTML && bStripHTML)
			{
				int len = strlen(MessageBody);
				len = theTextReader.StripHtml(MessageBody, len);
				MessageBody[len] = 0;
			}
			
			if (!bBodyHTML && bSendBodyHTML)
			{
				CString HTMLizedBody(Text2Html(MessageBody, TRUE, FALSE));
				if ((SendText( "<html>", EncodeSig) < 0) ||
					(SendEnriched(HTMLizedBody, EncodeSig) < 0))
				{
					goto fail;
				}
				if (*boundary)
				{
					if (SendText( "</html>", EncodeSig) < 0)
						goto fail;
				}
			}
			else
			{
				if (SendText(MessageBody, EncodeBody) < 0)
					goto fail;
			}
		}

		// If this has embedded objects (i.e. is multipart/related),
		// then send all of those embedded objects
		if (HasEmbedded)
		{
			const char* Index = EOHeader;
			while (Index = strchr(Index, '<'))
			{
				if (strnicmp(++Index, "cid:", 4) == 0)
					Index += 4;
				const char* Equal = strchr(Index, '=');
				ASSERT(Equal != NULL);
				if (Equal)
				{
					CString CID(Index, Equal - Index);
					Index = strchr(Equal, '>');
					ASSERT(Index != NULL);
					if (Index)
					{
						Equal++;
						CString Attach(Equal, Index - Equal);
						if (SendAttachment(Attach, MRBoundary, CID, TRUE) < 0) goto fail;
					}
				}
			}

			if (HasAttachments)
			{
				// Finish up a nested multipart/related by writing the closing boundary
				if (NetConnection->Put(MRBoundary) < 0) goto fail;
				if (NetConnection->PutLine("--") < 0) goto fail;
			}
			
		}

		// Attach documents if necessary
		if (HasAttachments)
		{
			CString AttachStr(SMTPmsg->GetHeaderLine(HEADER_ATTACHMENTS));
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
				if (SendAttachment(Attach, boundary) < 0)
					goto fail;

				Attach = t;
			}
		}

		// Add signature if wanted
		if (Signature)
		{
			// Need to get rid of <html> tag
			char* SignatureToSend = Signature;
			if (strnicmp(SignatureToSend, "<html>", 6) == 0)
				SignatureToSend += 6;

			if (*boundary)
			{
				if (NetConnection->PutLine(boundary) < 0) goto fail;

				// Send header for signature
				if ((NeedQP = SendContentType(SignatureToSend, NULL, 0, SigContentFlag)) < 0)
					goto fail;

				// Blank line separating MIME headers from signature
				if (NetConnection->PutLine() < 0) return (-1);
			}

			if (bSendSigHTML && *boundary)
			{
				if (SendText("<html>", EncodeSig) < 0)
					goto fail;
			}

			if (bSigHTML && !bStripHTML)
			{
				if ((SendText("<BR>\r\n", EncodeSig) < 0) ||
					(SendEnriched(SignatureToSend, EncodeSig) < 0))
				{
					goto fail;
				}
			}
			else
			{
				if (bSigHTML && bStripHTML)
				{
					int len = strlen(SignatureToSend);
					len = theTextReader.StripHtml(SignatureToSend, len);
					SignatureToSend[len] = 0;
				}
				
				if (!bSigHTML && bSendSigHTML)
				{
					CString HTMLizedSig(Text2Html(Signature, TRUE, FALSE));
					if ((SendText( "<BR>\r\n", EncodeBody) < 0) ||
						(SendEnriched(HTMLizedSig, EncodeBody) < 0) ||
						(SendText( "</html>", EncodeBody) < 0))
					{
						goto fail;
					}
				}
				else
				{
					//Plain sig, plain msg
					if( ( SendText( "\r\n", EncodeSig ) < 0 ) ||
						( SendText( SignatureToSend, EncodeSig ) < 0 ) )
					{ 
						goto fail;
					}
				}
			}

			delete [] Signature;
			Signature = NULL;
		}

		// Write out final boundary
		if (*boundary)
		{
			if (NetConnection->Put(boundary) < 0) goto fail;
			if (NetConnection->PutLine("--") < 0) goto fail;
		}
	
		NetConnection->Flush();	// Make sure all pending data has been put onto the net
		// Why isn't Craig Checking here?
		// @@@@@
#ifndef THREADED
		if (!NetConnection->IsFileBased()) 
#endif
			status = FinishSMTP();
//	}
		

	Progress(1, NULL, 1);

	delete [] MessageBody;
	
	return (status);

fail:
	delete [] Signature;
	Signature = NULL;

	delete [] MessageBody;

	return (E_FAIL);
*/
}









/* ==================================== */
void DisplayError(const char* szFilename)
{
	JJFileMT jjFile;
	char buf[1024];
	long NumRead;

	jjFile.Open(szFilename,_O_RDWR );
	HRESULT hr = jjFile.GetLine(buf, sizeof(buf), &NumRead);
	jjFile.Close();
	AlertDialog(IDD_ERROR_DIALOG, buf);

}



HRESULT SpoolOne(CSummary* Sum, HRESULT InResult)
{
	CTocDoc*			OutToc = GetOutToc();
	CCompMessageDoc*	doc = NULL;
	HRESULT				hr=S_OK;
	BOOL				CreatedDoc = FALSE;
	CString				tmpHeaders;
	CRString			csMimeVersion( IDS_MIME_HEADER_VERSION );
	CString				XLate;

						
	ASSERT(::IsMainThreadMT());
	//	
	//	The active document may be an altered QUEUED message.	
	//
	if (Sum->m_FrameWnd)
	{
		//If task is opened for some reason, do not queue it as the user may be intending to edit this message
		return InResult;
	
		/*doc = (CCompMessageDoc*)Sum->m_FrameWnd->GetActiveDocument();
		if (doc->IsModified())
		{
			switch (AlertDialog(IDD_SEND_CHANGED, Sum->MakeTitle()))
			{
			case IDC_DONT_SEND:
				return (InResult);
			case IDOK:
				if (doc->OnSaveDocument(NULL) == FALSE)
					return (InResult);
				break;
			case IDC_SEND_ORIGINAL:
				// Get old contents back
				doc->SetModifiedFlag(FALSE);
				if (((CMessageDoc*)doc)->Read() == FALSE)
					return (InResult);
				break;
			}
		}
		doc->Queue(TRUE);
		*/
	}
	
	//
	//	Extract valid document from summary
	//
	if (!(doc = (CCompMessageDoc *) Sum->FindMessageDoc()))
	{
		CreatedDoc = TRUE;
		doc = (CCompMessageDoc *) Sum->GetMessageDoc();
	}
	
	if (!doc || !doc->GetText())
	{
		if (CreatedDoc)
			Sum->NukeMessageDocIfUnused();
		
		return (InResult);
	}
	ASSERT_KINDOF(CCompMessageDoc, doc);

	//
	// Create a Spool file for this message.
	//
	CString Persona = g_Personalities.GetCurrent();
	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(Persona);

	CString szFilename;
	hr = SpoolMgr->m_Out->BeginWrite(szFilename);
	if ( FAILED(hr) ) 
	{
		if (CreatedDoc)
			Sum->NukeMessageDocIfUnused();
		return hr;
	}

	//
	// Create an object to encode the message and send its
	// MIME parts to the spool file.  
	//
	QCSMTPMessage SendMsg(doc);
	hr = SendMsg.Start(szFilename);
	if ( FAILED(hr) ) goto fail;
	
	hr = SendMsg.WriteHash();
	if ( FAILED(hr) ) goto fail;

	if (!UsingPOPSend)
	{
		hr=SendMsg.WriteEnvelope();
		if ( FAILED(hr) ) goto fail;
	}
	
	hr = SendMsg.WriteHeaders();
	if ( FAILED(hr) ) goto fail;

	//
	//	In most instances we shall encode the body and write it to the 
	//	spool file.  There are a two exceptions:
	//			1) Some messages may need to be translated.
	//			2) Some messages may already be MIME encoded (.MSG files).
	//	Treat these two cases special.
	
	//
	//	Translation?
	//
	XLate = doc->m_Sum->GetTranslators();
	if (!XLate.IsEmpty())
	{
		//
		//	Save MIME encoded body to temp file
		//
		CString raw = GetTmpFileNameMT();
		// WriteBody needs an open file....
		hr = SendMsg.RedirectOutput(raw);
		if ( FAILED(hr) ) goto fail;
		
		hr= SendMsg.WriteBody();
		if ( FAILED(hr) ) goto fail;

		hr=SendMsg.EndRedirect();
		if ( FAILED(hr) ) goto fail;	//
		//	Translate it and store it in new file called COOKED.
		//
		char cooked[_MAX_PATH];
		*cooked = 0;
		int err = GetTransMan()->XLateMessageOut(EMSF_Q4_TRANSMISSION | 
													EMSF_WHOLE_MESSAGE | 
													EMSF_REQUIRES_MIME | 
													EMSF_GENERATES_MIME, 
													SMTPmsg, 
													raw, 
													cooked);
		//
		//	Add translated file to spool
		//
		if (!err && cooked)
		{
			hr = SendMsg.WriteRawMIME(cooked);
			if ( FAILED(hr) ) goto fail;
		}
		else
		{
			hr = 0x80000000 + err;
			SMTPmsg->m_Sum->SetState(MS_UNSENDABLE);
			goto fail;
		}
		DeleteFile(raw); 
		DeleteFile(cooked); 
	}
	else
	{
		hr = SendMsg.WriteBody();
		if ( FAILED(hr) ) goto fail;
	}

	hr = SendMsg.End();  
	hr = SpoolMgr->m_Out->EndWrite(szFilename, Sum->GetUniqueMessageId(), Sum);
	

	if (CreatedDoc)
			Sum->NukeMessageDocIfUnused();
	return hr;


fail:	
	
	HRESULT hr2;
	hr2 = SendMsg.End();  // what to do about return value here...
	SpoolMgr->m_Out->EndWrite();

	if (CreatedDoc)
		Sum->NukeMessageDocIfUnused();

	return hr;
}








/*
	//
	//	MIME-encoded .msg file?
	//
	tmpHeaders = doc->m_ExtraHeaders;
	tmpHeaders.MakeUpper();
	csMimeVersion.MakeUpper();
	if ( tmpHeaders.Find( csMimeVersion ) >= 0 ||
		SMTPmsg->m_Sum->IsMimeAttached() )
	{
		// extract the filename
		CString AttachStr(SMTPmsg->GetHeaderLine(HEADER_ATTACHMENTS));
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

			// expects a fully-qualified filename
			if (SendMsg.WriteRawMIME( Attach ) < 0)
				return 0;

			Attach = t;
		}
		NetConnection->Flush();	// Make sure all pending data has been put onto the net
		//if (!NetConnection->IsFileBased())
		int status = FinishSMTP();
	}
	else 
*/






/*

// SMTPError
// Show the user a dialog box with the error message on command
void SMTPError(const char* Command, const char* Message)
{
	ErrorDialog(	IDS_ERR_TRANSFER_MAIL, 
					Command, 
					"SMTP", 
					( const char* ) CurSMTPServer, 
					Message);
}

// GetSMTPReply
//
// Get the message from the server, and return the status
// code
int GetSMTPReply(char *buf, int size)
{
	int status;
	char *ptr;

	do
	{
		status = NetConnection->GetLine(buf, size);
		if (status < 0) 
			return (status);
		for (ptr = buf; *ptr && (*ptr < ' ' || *ptr > '~'); ptr++){}
	} while (!isdigit(ptr[0]) || !isdigit(ptr[1]) || !isdigit(ptr[2]) ||
		ptr[3] == '-');

	if (ptr != buf)
		strcpy(buf, ptr);

	return (atoi(ptr));
}




// 
// StartSMTP
// Routine that initiates a connection with the mail server
//
int StartSMTP(const char* SMTPServer)
{
	int Status = 1;
 	char Server[256];

	DeleteNetObj = FALSE;
	SMTPConnected = FALSE;
	UsingPOPSend = FALSE;

	strncpy(Server, SMTPServer, sizeof(Server));
	CurSMTPServer = SMTPServer;

	// If we need to login in to the POP server first before sending,
	// get the POP password before any connection starts up.
	UsingPOPSend = (GetIniShort(IDS_INI_USE_POP_SEND) != 0);

	if (!NetConnection)
	{
		if (!CreateNetConnection())
			return (-1);
		DeleteNetObj = TRUE;
	}

// This really shouldn't be here.  It was here or in compmsgd.  This
// code assumes that the service will always be POP if the host	name
// is Hesiod.
// Hesiod Kludge code.  There should really be a userinterface for this
	if (!stricmp(Server, "hesiod"))
	{
//FORNOW		(void)NetConnection->GetHesiodServer(CRString(IDS_POP_SERVICE), Server, sizeof(Server));
//FORNOW		// Could be NULL
		//
		// If we're using Hesiod, then override the 'Server' value
		// with the host name retrieved from the Hesiod server.
		//
		Server[0] = '\0';
		{
			//
			// Grab the username (c_wsakai) out of the POP account setting
			// (c_wsakai@adept.qualcomm.com).
			//
			char szPOPUserName[128];
			::GetIniString(IDS_INI_POP_ACCOUNT, szPOPUserName, sizeof(szPOPUserName));
			{
				char* pszAtSign = strrchr(szPOPUserName, '@');
				if (NULL == pszAtSign)
					return -1;
				*pszAtSign = 0;
			}

			//
			// Do the Hesiod lookup.
			//
			if (SUCCEEDED(QCHesiodLibMT::LoadHesiodLibrary()))
			{
				int nHesiodError = -1;
				if (FAILED(QCHesiodLibMT::GetHesiodServer(CRString(IDS_POP_SERVICE), szPOPUserName, Server, sizeof(Server), &nHesiodError)))
					::HesiodErrorDialog(nHesiodError);
				QCHesiodLibMT::FreeHesiodLibrary();

				if ('\0' == Server[0])
					return -1;
			}
			else
			{
				const int IDS_WINSOCK_BASE_ERROR = 8000;
				::ErrorDialog(IDS_WINSOCK_BASE_ERROR + 200 + 1);
				return -1;
			}
		}
	}
// End Hesiod kludge

	if (!UsingPOPSend)
		Status = NetConnection->Open(Server, IDS_SMTP_SERVICE, IDS_INI_SMTP_PORT, 25);

	if (Status >= 0)
		SMTPConnected = TRUE;

	return (Status);
}


// DoSMTPIntro
//
// Once the connection is open, get the opening banner and say
// hello to the server if we're actually talking to SMTP
//
int DoSMTPIntro()
{
	char host[256];
	char buf[128];
	int retCode;

	if (UsingPOPSend)
		return (1);

	MainProgress(CRString(IDS_SERVER_CONNECT));

	// Get the opening banner
	retCode = GetSMTPReply(buf, sizeof(buf));
	if (retCode / 100 != 2)
	{
		// The return code was not 2xx
		if (retCode >= 0)
			ErrorDialog(IDS_ERR_SMTP_INTRO, buf);
		// No critical states to cleanup.  Just abort out of the connection
		SMTPConnected = FALSE;
		return (-1);
	}

	//if (QCWinSockLibMT::IsLibraryLoaded() == S_FALSE || QCWinSockLibMT::gethostname(host, sizeof(host)) == SOCKET_ERROR)
	if (QCWinSockLibMT::IsLibraryLoaded() == false || QCWinSockLibMT::gethostname(host, sizeof(host)) == SOCKET_ERROR)
	{
		*host = '\0';
	}

	// We couldn't figure anything else out for the HELO command, so
	// let's just send the name of the machine we're talking to
	if (!*host)
	{
		GetIniString(IDS_INI_SMTP_SERVER, host, sizeof(host));
		if (!*host)
		{
			const char* pa = GetIniString(IDS_INI_POP_ACCOUNT);
			if (pa = strrchr(pa, '@'))
				strcpy(host, pa);

			if (!stricmp(host, "hesiod"))
			{
// Hesiod Kludge code.  There should really be a userinterface for this
//FORNOW				host[0] = '\0';
//FORNOW				(void)NetConnection->GetHesiodServer(CRString(IDS_POP_SERVICE), host, sizeof(host));
//FORNOW				// host could be NULL
				//
				// If we're using Hesiod, then override the 'host' value
				// with the host name retrieved from the Hesiod server.
				//
				host[0] = '\0';
				{
					//
					// Grab the username (c_wsakai) out of the POP account setting
					// (c_wsakai@adept.qualcomm.com).
					//
					char szPOPUserName[128];
					::GetIniString(IDS_INI_POP_ACCOUNT, szPOPUserName, sizeof(szPOPUserName));
					{
						char* pszAtSign = strrchr(szPOPUserName, '@');
						if (NULL == pszAtSign)
							return -1;
						*pszAtSign = 0;
					}

					//
					// Do the Hesiod lookup.
					//
					if (SUCCEEDED(QCHesiodLibMT::LoadHesiodLibrary()))
					{
						int nHesiodError = -1;
						if (FAILED(QCHesiodLibMT::GetHesiodServer(CRString(IDS_POP_SERVICE), szPOPUserName, host, sizeof(host), &nHesiodError)))
							::HesiodErrorDialog(nHesiodError);
						QCHesiodLibMT::FreeHesiodLibrary();

						if ('\0' == host[0])
							return -1;
					}
					else
					{
						const int IDS_WINSOCK_BASE_ERROR = 8000;
						::ErrorDialog(IDS_WINSOCK_BASE_ERROR + 200 + 1);
						return -1;
					}
				}
// End Hesiod kludge
			}
		}
	}

	// An error is not necessarialy a catastrophic error for hello
	SMTPCmdGetReply(IDS_SMTP_HELLO, host, buf, sizeof(buf), TRUE, TRUE, FALSE);

	return (0);
}


*/

HRESULT SendQueuedMessages2(int WhichToSend /*= QS_READY_TO_BE_SENT*/,
									BOOL bMultiPersona /*= TRUE*/,
									bool bSend /*=TRUE*/)
{
	
	HRESULT hr = S_OK; //SQR_ALL_OK;	// assume the best in case...
	
	// get out'a here if there aren't any queued messages
	if (QueueStatus <= QS_NONE_QUEUED)
		return (hr);

	
	CString	cachePersonality =	g_Personalities.GetCurrent();
	LPSTR lpPersonalities = g_Personalities.List();
	lpPersonalities += strlen( lpPersonalities ) + 1;	//Skip <Dominant>
	CString strPersona = "";								//Still always start with the Default personality

	//All the tasks started here will be taken as a group
	QCTaskGroup group;

	do
	{
		if ( bMultiPersona )
			g_Personalities.SetCurrent( strPersona );
		
		
		//Send for the persona using a background thread
		SendMail(strPersona, WhichToSend);

				// advance to next personality
		strPersona = lpPersonalities;
		lpPersonalities += strlen( lpPersonalities ) + 1;

	} while ( bMultiPersona && !strPersona.IsEmpty() );

	g_Personalities.SetCurrent( cachePersonality );
 
	return (hr);
}


void SignalTheEnd()
{
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "SendMailDone");
	SetEvent(hEvent);
}

CTime g_AfterTime;
void ExitAfterSend()
{
	if( QCGetTaskManager()->GetNumTaskErrors(g_AfterTime, TASK_TYPE_SENDING) == 0)
		::AfxGetMainWnd()->PostMessage(WM_CLOSE);
}


HRESULT SendQueuedMessagesAndQuit(int WhichToSend /*= QS_READY_TO_BE_SENT*/, BOOL bMultiPersona /*= TRUE*/)
{
	
	HRESULT hr = S_OK; //SQR_ALL_OK;	// assume the best in case...
	
	// get out'a here if there aren't any queued messages
	if (QueueStatus <= QS_NONE_QUEUED)
		return (hr);

	
	CString	cachePersonality =	g_Personalities.GetCurrent();
	LPSTR lpPersonalities = g_Personalities.List();
	lpPersonalities += strlen( lpPersonalities ) + 1;	//Skip <Dominant>
	CString strPersona = "";								//Still always start with the Default personality

	//HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, "SendMailDone");
	//Callback0 End = makeCallback( (Callback0 *)0, &SignalTheEnd);
	Callback0 End = makeCallback( (Callback0 *)0, &ExitAfterSend);
	g_AfterTime = CTime::GetCurrentTime();
	{
		//All the tasks started here will be taken as a group. 
		//Let this group ignore idle time
		QCTaskGroup group(End, true);
		
		do
		{
			if ( bMultiPersona )
				g_Personalities.SetCurrent( strPersona );
			
			//Send for the persona using a background thread
			SendMail(strPersona, WhichToSend);

					// advance to next personality
			strPersona = lpPersonalities;
			lpPersonalities += strlen( lpPersonalities ) + 1;

		} while ( bMultiPersona && !strPersona.IsEmpty() );
		g_Personalities.SetCurrent( cachePersonality );
	} //start tasks here 



	/*
	while(1)
	{
		if(EscapePressed())
			return E_FAIL;
		if( WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0 )
			break;
		Sleep(1);
	}*/
 
	return (hr);
}



HRESULT SendMail(CString strPersona, int WhichToSend /*= QS_READY_TO_BE_SENT*/)
{
	time_t Now = time(NULL);
	time_t Later = Now + 12 * 60L * 60L;	// 12 hours from now
	
	
	HRESULT hr = S_OK; //SQR_ALL_OK;	// assume the best in case...

	// get out'a here if there aren't any queued messages
	if (QueueStatus <= QS_NONE_QUEUED)
		return (hr);

	CTocDoc* OutToc = GetOutToc();
	if (!OutToc)
		return (hr);

	CString	homie =	g_Personalities.GetCurrent();
	g_Personalities.SetCurrent( strPersona );

	
/*		if ( UsingPOPSend )
		{
			POPAuthenticate();
			Put password in sttings object;
		}
  */

	// are there any messages for this server?
	int NumToSend = 0;
	CSummary* Sum;
	POSITION pos;
	for (pos = OutToc->m_Sums.GetHeadPosition(); pos; )
	{
		Sum = OutToc->m_Sums.GetNext(pos);
		if ( Sum->GetPersona() == strPersona )
		{
			time_t SumLocalSeconds = Sum->m_Seconds + Sum->m_TimeZoneMinutes * 60;

			if (Sum->m_State == MS_QUEUED ||
				(Sum->m_State == MS_TIME_QUEUED && (SumLocalSeconds <= Now ||
				SumLocalSeconds <= Later && WhichToSend <= QS_DELAYED_WITHIN_12 ||
				WhichToSend == QS_DELAYED_MORE_THAN_12)))
			{
				NumToSend++;
			}
		}
	}

	//MainProgress("Spooling outgoing messages");
	if (NumToSend)
	{
	    //CountdownProgress(CRString(IDS_SMTP_MESSAGES_LEFT), NumToSend);
		CountdownProgress(CRString(IDS_PREPARING_TO_SPOOL), NumToSend);
    
		BOOL bSpooledOne = FALSE;
		hr = S_OK;
		for (pos = OutToc->m_Sums.GetHeadPosition(); pos && NumToSend; )
		{
			Sum = OutToc->m_Sums.GetNext(pos);
			if ( Sum->GetPersona() == strPersona )
			{
				time_t SumLocalSeconds = Sum->m_Seconds + Sum->m_TimeZoneMinutes * 60;

				if (Sum->m_State == MS_QUEUED ||
					(Sum->m_State == MS_TIME_QUEUED && (SumLocalSeconds <= Now ||
					SumLocalSeconds <= Later && WhichToSend <= QS_DELAYED_WITHIN_12 ||
					WhichToSend == QS_DELAYED_MORE_THAN_12)))
				{
					hr = E_ABORT;
					hr = SpoolOne(Sum, hr);  // Check for success
					if ( SUCCEEDED(hr) )
					{
						Sum->SetState(MS_SPOOLED);
						bSpooledOne = TRUE;
					}
					NumToSend--;
					DecrementCountdownProgress();
				}
			}
		}

		FlushQueue = FALSE;
		if (OutToc->IsModified())
			OutToc->Write();
		SetQueueStatus();

		CloseProgress();

		// If none of the messages actually got spooled, then there's no sense
		// in starting up a background task since no messages will be sent.
		if (!bSpooledOne)
			return (hr);
		
		//
		// Store this persona's settings in the Mail
		// Settings Snapshot.  This allows the thread to access
		// INI entries without having to share the INI file
		// with other threads.
		//
		QCSMTPSettings* pSettings = new QCSMTPSettings(strPersona);
		pSettings->GrabSettings();

		//
		// Create the SMTP thread object and initialize it with
		// this persona's settings.
		//
		QCSMTPThreadMT* smtpThread = new QCSMTPThreadMT(strPersona, pSettings);
	
		if(!smtpThread->PreProcessPOPSend())
		{
			//Don't need taskinfo anymore so nuke it
			CTaskInfoMT *pInfo = smtpThread->GetTaskInfo();
			delete smtpThread;
			delete pInfo;

			hr = SQR_MAJOR_ERROR;
		}
		
		// Store POP password in object
		if (hr != SQR_MAJOR_ERROR &&  !QCGetTaskManager()->QueueWorkerThread(smtpThread))
		{
			//Don't need taskinfo anymore so nuke it
			CTaskInfoMT *pInfo = smtpThread->GetTaskInfo();
			delete smtpThread;
			delete pInfo;
			hr = SQR_MAJOR_ERROR;
		}

	}
	
	g_Personalities.SetCurrent( homie );

	return (hr);
}







