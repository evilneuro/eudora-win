// SMTPSession.cpp
//
// Copyright (c) 1997-2001 by QUALCOMM, Incorporated
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

#include <qcutils.h>
#include <afxmt.h>

#include "EudError.h"
#include "summary.h"
#include "persona.h"
#include "SpoolMgr.h"
#include "QCWorkerThreadMT.h"

#include "QCNetSettings.h"
#include "QCSMTPSettings.h"

#include "SMTPSession.h"
#include "resource.h"
#include "QCWorkerSocket.h"
#include "FileLineReader.h"

#include "summary.h"  // sendmail.h
#include "sendmail.h" // g_spoolmgr
#include "rs.h"		// FOR CRString

#include "QCPOPSettings.h"  //For POP send
#include "POPSession.h"   //todo POPSend
#include "QCRas.h"

#include "tocdoc.h"
#include "tocview.h"
#include "filtersd.h" //for CFilterActions

// For ESMTP:
#include "ehlo.h"
#include "sasl.h"
#include "Base64.h"
#include "ImapAccountMgr.h"
#include "ImapAccount.h"

#include "DebugNewHelpers.h"


// Statics:
static BOOL smtp_response (void *s, char *response, unsigned long size);
int			smtp_challenge (void *s, char *outbuf, int bufsize);
int			B64DecodeString(char* s);
int			B64EncodeString (char* s, int sLen, char *out, int outsize);


/* ============= QCSMTPThreadMT ================= */
QCSMTPThreadMT::QCSMTPThreadMT(const char *szPersona, const char *szSpoolPersona, QCSMTPSettings* pMailSettings) :
	m_strPersona(szPersona), m_strSpoolPersona(szSpoolPersona), m_Settings(pMailSettings), m_bTaskStarted(false),
	m_POPSession(NULL), m_pNetConnection(NULL), m_nTotalSize(0), m_pEhlo(NULL), m_pSasl(NULL)
{
	
	m_pTaskInfo->SetPersona(m_strPersona);
	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);

	//set the task title for the view 
	m_pTaskInfo->SetTitle(CRString(IDS_SENDING_MESSAGES));
	
	//Set the error callback function for Network to report error back to us
	(m_Settings->GetNetSettings())->SetReportErrorCallback(  
		makeCallback( (QCNetworkSettings::ErrorCallback *)0, *m_pTaskInfo, &CTaskInfoMT::CreateError) );

	(m_Settings->GetNetSettings())->SetProgressCallback(
		makeCallback( (Callback1<const char *> *)0, (*m_pTaskInfo), &CTaskInfoMT::SetMainText) );

	if(m_Settings->GetNetSettings()->IsDialupConnection())
	{
		m_pTaskInfo->SetDialupEntryName( m_Settings->GetNetSettings()->GetRASEntryName() );
		m_pTaskInfo->SetScheduleTypes( TT_USES_DIALUP);
	}
	
	m_pTaskInfo->SetScheduleTypes(TT_USES_SMTP);
	if(m_Settings->UsePOPSend())
		m_pTaskInfo->SetScheduleTypes(TT_USES_POP);

	m_Settings->GetNetSettings()->SetTaskInfo(m_pTaskInfo);

	// Collect the messages to send
	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(m_strSpoolPersona);
	
	int nTaskID = m_pTaskInfo->GetUID();
	m_nMsgsToProcess = SpoolMgr->m_Out->GetCountAndOwnership(SS_SEND, nTaskID, &m_nTotalSize);
		
	//Ignore idle time if there are no outgoing filters.. 
	if(m_nMsgsToProcess)
	{
		CFilterActions filt;
		if (filt.StartFiltering(WTA_OUTGOING) == FALSE) // if no outgoing filters
			m_pTaskInfo->IgnoreIdle();
	}

}

QCSMTPThreadMT::~QCSMTPThreadMT()
{
	if(!m_bTaskStarted)
	{
		//if task never got a chance to start, we need to move remove the spool files and change the status 
		//back to Queued state.  To make it painless, we reuse the PostProcessOutgoingMessages fn.
		ASSERT(::IsMainThreadMT());
		PostProcessOutgoingMessages((void *)this, true);
	}

	if(m_POPSession)
	{
		delete m_POPSession->GetTaskInfo();
		delete m_POPSession;
		m_POPSession = NULL;
	}

	if(m_Settings)
		delete m_Settings;

	if (m_pEhlo)
		delete m_pEhlo;
	if (m_pSasl)
		delete m_pSasl;
}

void QCSMTPThreadMT::RequestThreadStop()
{
	CSingleLock lock(&m_Guard, TRUE);

	//set this falg, so that we know that we are stopped
	m_bStopThread = true;

	//set the escapePressed variable to true in QCWorkerSocket object
	//NetWork functions fail after setting this
	if(m_pNetConnection)
		m_pNetConnection->RequestStop();
	
}



HRESULT QCSMTPThreadMT::DoAllMessages()
{
	m_bTaskStarted = true;
	
	//	Begin search for all the .SND files
	COutSpoolInfo *pSpoolInfo = NULL;
	SPOOLSTATUS spStatus = SS_UNKNOWN;
	HRESULT hr = S_OK;
		
	QCSpoolMgrMT* SpoolMgr  = QCSpoolMgrMT::CreateSpoolMgr(m_strSpoolPersona);
	
	int nTaskID = m_pTaskInfo->GetUID();
	//int nTotalMsgs = SpoolMgr->m_Out->GetCountAndOwnership(SS_SEND, nTaskID);
	//m_NumMessagesProcessed = nTotalMsgs;

	int nTotalMsgs = m_nMsgsToProcess;
	
	//m_pTaskInfo->SetTotal(nTotalMsgs);
	m_pTaskInfo->SetTotal(m_nTotalSize);
	CString strMainText; 
	
	
	//do 
	while(nTotalMsgs)
	{
		pSpoolInfo = SpoolMgr->m_Out->Get(SS_SEND, SS_SENDING_INPROGRESS, nTaskID);
		if(!pSpoolInfo)
		{
			ASSERT(0);
			break;
		}
	
		if( IsStoppingThread())
		{
			//Go through the list and conver all .SND messages back to QUEUES status and delete the .SND files
			// CleanUp();
			m_pTaskInfo->SetMainText(CRString(IDS_TASK_STOPPED));
			break;
		}
	
		//strMainText.Format("Messages left to send: %d", nTotalMsgs);
		strMainText.Format(CRString(IDS_SMTP_MESSAGES_LEFT_TO_SEND), nTotalMsgs);
		m_pTaskInfo->SetMainText(strMainText);
		
		hr = DoOneMessage(pSpoolInfo);
		
		nTotalMsgs--;
		//m_pTaskInfo->ProgressAdd(1);
	
				
		// SetState(GetHash(szFilename));

		if (SUCCEEDED(hr)){
			spStatus = SS_SENT;			// Spool file was sent successfully
		}
		else
		{
			if ( HRESULT_CODE(hr) == SS_UNSENDABLE ){
				spStatus = SS_UNSENDABLE;			// Spool file was unsendable because of SMTP error.
			}
			else{
				spStatus = SS_UNSENT;				// Spool file was not sent because of unknown error.
			}
		}
		

		//
		//	Transfer the file from the SEND spool to the SENT spool.
		//	The extension of the file in the SENT spool is dependent
		//	upon the status of the message.
		//
		pSpoolInfo->SetSpoolStatus(spStatus);
		
		if( IsStoppingThread())
		{
			m_pTaskInfo->SetMainText(CRString(IDS_TASK_STOPPED));
			break;
		}
	
	}

	//Finish up the left over progress if any
	m_pTaskInfo->SetSoFar(m_nTotalSize);

	return hr;
}

enum { VER_OK, VER_NOMATCH, VER_UNVER, VER_DONE };

#define BUF_SIZE 8216
HRESULT QCSMTPThreadMT::DoOneMessage( COutSpoolInfo *pSpoolInfo)     //CString szFilename)
{
	CString szFilename = pSpoolInfo->GetFileName();
	FileLineReader reader;
	char buf[BUF_SIZE];
	JJFileMT jjFile;
	long NumRead;	
	int status;
	HRESULT hr;
	bool bDone;
	CString str;

	
	hr = FileExistsMT(szFilename);
	if ( FAILED(hr) )
		return hr;


	jjFile.Open(szFilename,_O_RDWR );

	hr = ReadLine(&jjFile,buf,BUF_SIZE-1,NumRead);
	if ( FAILED(hr) )
		goto fail;

	int cmd;
	
	if (!m_bUsingPOPSend)		//if (TRUE) 
	{
		if ( IsCmd(buf) != IDS_SMTP_RESET ) 
		{
			hr = E_FAIL;
			goto fail;
		}
		hr = SMTPCmdGetReply( buf, NULL, buf, sizeof(buf), status);
		if ( FAILED( hr ) ) goto fail;
	
		hr = ReadLine(&jjFile,buf,BUF_SIZE-1,NumRead);
		if ( FAILED(hr) ) goto fail;

		if ( IsCmd(buf) != IDS_SMTP_FROM ) 
		{
			hr = E_FAIL;
			goto fail;
		}
		hr = SMTPCmdGetReply( buf, NULL, buf, sizeof(buf), status);
		if ( FAILED( hr ) )goto fail;

		hr = ReadLine(&jjFile,buf,BUF_SIZE-1,NumRead);
		if ( FAILED(hr) )goto fail;

		while ( (cmd=IsCmd(buf)) == IDS_SMTP_RECIPIENT )
		{
			char ResponseBuf[BUF_SIZE];
			hr = SMTPCmdGetReply( buf, NULL, ResponseBuf, sizeof(ResponseBuf), status, true, true);
			status = status/100;
			if (status != 2)
			{
				if (status != 5)
					SMTPError(buf, ResponseBuf);
				else
				{
					CString Recipient;
					char* p;
					
					// Strip off trailing CRLF
					p = ResponseBuf + strlen(ResponseBuf) - 1;
					while (p >= ResponseBuf && (*p == '\r' || *p == '\n'))
						*p-- = 0;

					// To get address, remove angle brackets and 550 error code
					p = strchr(ResponseBuf, '<');
					if (p)
					{
						Recipient = ++p;
						int CloseAddress = Recipient.Find('>');
						if (CloseAddress > 0)
							Recipient.ReleaseBuffer(CloseAddress);
					}
					
					//LogError(&jjFile, IDS_ERR_BAD_RECIPIENT, (LPCTSTR)Recipient, ResponseBuf);
					//ErrorDialog(IDS_ERR_BAD_RECIPIENT, ResponseBuf, (LPCTSTR)Recipient);
					
					CString eStr;
					eStr.Format( CRString(IDS_ERR_BAD_RECIPIENT), (LPCTSTR)Recipient, ResponseBuf);
					QCError *pErr = DEBUG_NEW QCSmtpMsgError(pSpoolInfo->GetUniqueID());
					m_pTaskInfo->CreateErrorContext( eStr, (TaskErrorType)(TERR_SMTP | TERR_MSG), pErr);
					
					//devise the return code
					hr = MAKE_HRESULT(1, FACILITY_ITF, SS_UNSENDABLE);
				}
				goto fail; 
			}
			hr = ReadLine(&jjFile,buf,BUF_SIZE-1,NumRead);
			if ( FAILED(hr) )
				goto fail;	
		} 

		if ( cmd != IDS_SMTP_DATA ) goto fail;
		hr = SMTPCmdGetReply( buf, NULL, buf, sizeof(buf), status);
		if ( FAILED( hr ) ) 
		{
			if (status / 100 == 5)
				hr = MAKE_HRESULT(1, FACILITY_ITF, SS_UNSENDABLE);
			goto fail;
		}
	}
	else
	{
		if ( IsCmd(buf) != IDS_POP_SEND )  goto fail;
		if (m_POPSession->POPCmdGetReply(IDS_POP_SEND, NULL, buf, BUF_SIZE-1) < 0)
			goto fail;
	}
	bDone = false;
	while (SUCCEEDED(hr =  ReadLine(&jjFile,buf,sizeof(buf),NumRead)) && !bDone ) //jjFile.RawRead(buf, sizeof(buf), &NumRead)) && !bDone)
	{
#ifdef _DEBUG
		for (int i = 0; i < NumRead; i++)
		{
			// We better not get any NULL bytes being sent out
			ASSERT(buf[i] != 0);
		}
#endif
		if (hr == S_FALSE )
			bDone = true;

		if (NumRead > 0)
		{
			if ( (status = m_pNetConnection->PutDirect(buf, NumRead)) < 0 ) goto fail;
		}
		
		m_pTaskInfo->ProgressAdd(NumRead);
	}
	jjFile.Close();


	//
	//	Reply for SMTP_MESSAGE_END
	//
	//GetSMTPReply(buf,sizeof(buf),status);

	
	if (m_bUsingPOPSend)
	{
		status = m_POPSession->POPCmdGetReply(IDS_SMTP_MESSAGE_END, NULL, buf, BUF_SIZE-1, FALSE);

		// This is a bit of a heuristic.  Berkeley's popper returns an error
		// to the XTND XMIT command when a recpient is invalid.  But since
		// POP has such simplistic error reporting, we can't tell if it was
		// because the message was bad or something went wrong with the
		// network.  99% of the time it's because of an invalid recipient, so
		// let's take that course and de-queue the message.
		//if (status < 0)
		//	SMTPmsg->m_Sum->SetState(MS_UNSENDABLE);
		if(status < 0)
		{
			CRString eStr(IDS_ERR_POP_SEND);
			QCError *pErr = DEBUG_NEW QCSmtpMsgError(pSpoolInfo->GetUniqueID());
			m_pTaskInfo->CreateErrorContext( eStr, (TaskErrorType)(TERR_SMTP | TERR_MSG), pErr);
			hr = MAKE_HRESULT(1, FACILITY_ITF, SS_UNSENDABLE);
			goto fail;
		}
	
	}
	else
	{
		//status = SMTPCmdGetReply(IDS_SMTP_MESSAGE_END, NULL, buf, BUF_SIZE-1, FALSE);
		strcpy(buf, "\r\n.\r\n");
		hr = SMTPCmdGetReply(buf, NULL, buf, sizeof(buf), status);
		if ( FAILED( hr ) )
		{
			if (status / 100 == 5)
				hr = MAKE_HRESULT(1, FACILITY_ITF, SS_UNSENDABLE);
			goto fail;
		}
	}
 
	return S_OK;

fail:

	jjFile.Close();
	if(SUCCEEDED(hr))
		hr = E_FAIL;
	
	return hr;
}


CString QCSMTPThreadMT::GetGSSLibraryName()
{
	if (m_POPSession)
	{
		return m_POPSession->GetGSSLibraryName();
	}
	return CRString(IDS_GSSAPI_LIBNAME);
}


// SMTPError
// Show the user a dialog box with the error message on command
void QCSMTPThreadMT::SMTPError(const char* Command, const char* Message)
{
	ErrorDialog(IDS_ERR_TRANSFER_MAIL, 
					Command, 
					"SMTP", 
					( const char* ) CurSMTPServer, 
					Message);
}


void QCSMTPThreadMT::ErrorDialog(const char *error_buf)
{
	MessageBeep(MB_ICONEXCLAMATION);
	m_pTaskInfo->CreateError(error_buf);
}


void QCSMTPThreadMT::ErrorDialog(UINT StringID, ...)
{
	char buf[1024] = {0};
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	m_pTaskInfo->CreateError(buf);
}

/*
void QCSMTPThreadMT::ErrorDialog(UINT StringID, va_list argList)
{
	char buf[1024] = {0};
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list argSave = argList;
	
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), argList);
	
	va_end(argSave);

	m_pTaskInfo->CreateError(buf);
}*/


HRESULT QCSMTPThreadMT::LogError(JJFileMT* file, UINT StringID, ...)
{
	char buf[1024];
	
	MessageBeep(MB_ICONEXCLAMATION);
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);

	m_pTaskInfo->CreateError(buf);
	
	file->Reset();
	file->PutLine(buf);
	long lOffset;
	file->Tell(&lOffset);
	file->Truncate(lOffset);

	return S_OK;
}


/*
HRESULT QCSMTPThreadMT::LogError(JJFileMT* file, UINT , const char* buf)
{
	char buf[1024];
	ASSERT(file->IsOpen() == S_OK);
	
	file->Reset();
	file->PutLine(CRString(errmsg));
	file->PutLine(buf);
	long lOffset;
	file->Tell(&lOffset);
	file->Truncate(lOffset);

	return S_OK;
}
 */
int QCSMTPThreadMT::IsCmd(const char* buf)
{

	if ( strncmp(buf,"DATA"		,4)		== 0 )	return IDS_SMTP_DATA;
	if ( strncmp(buf,"MAIL FROM:",10)	== 0 )	return IDS_SMTP_FROM;
	if ( strncmp(buf,"RCPT TO:"	,8)		== 0 )	return IDS_SMTP_RECIPIENT;
	if ( strncmp(buf,"QUIT"		,4)		== 0 )	return IDS_SMTP_QUIT;
	if ( strncmp(buf,"RSET"		,4)		== 0 )	return IDS_SMTP_RESET;
	if ( strncmp(buf,"XTND XMIT",9)		== 0 )	return IDS_POP_SEND;

	return 0;
}

// SMTPCmdGetReply
//
// Send a command to the server and get a reply
//
HRESULT QCSMTPThreadMT::SMTPCmdGetReply(const char* cmd, const char* arg, char* buf, int size, int&status,
								bool ShowProgress/*= true*/, bool IgnoreError/*=false*/)
{
	HRESULT		hr = S_OK;

	// Create the command to send to the server
	CString		szCommandBuff = cmd;

	if (arg)
	{
		szCommandBuff += arg;
		szCommandBuff += "\r\n";
	}
	
	if (ShowProgress)
		m_pTaskInfo->SetSecondaryText(szCommandBuff);

	// the command buffer is now ready, send the command to the server...
	status = m_pNetConnection->PutDirect(szCommandBuff);
	if (status < 0)
	{
		m_pNetConnection->Close();
		return (-1);
	}
	
	if(ShowProgress)
		m_pTaskInfo->ProgressAdd(status);

	while (1)
	{
		hr = GetSMTPReply(buf, size, status);
		if (status < 0 || status / 100 >= 4)
		{
			//if (IgnoreError)	// FORNOW			
			//	continue;		// FORNOW			
			//if (!HandleError)

			//The return value is failed code; say so in the return value
			hr = E_FAIL;

			if (!IgnoreError)
			{
				if (status >= 0)
					SMTPError(szCommandBuff, buf);
			}
		}
		break;
	}

	return (hr);	
}


HRESULT QCSMTPThreadMT::GetSMTPReply(char* buf, int size, int& status)
{
	unsigned char *ptr;

	do
	{
		status = m_pNetConnection->GetLine(buf, size);
		if (status < 0) 
			return (E_FAIL);
		for (ptr = (unsigned char*)buf; *ptr && (*ptr < ' ' || *ptr > '~'); ptr++){}

		// If this was an EHLO command, look for a 250 response:
		// (JOK - 12/13/98).
		if (IsEhlo() && ptr[0]=='2' && ptr[1]=='5' && ptr[2]=='0')
		{
			EhloLine(buf);
		}
	} while (!isdigit(ptr[0]) || !isdigit(ptr[1]) || !isdigit(ptr[2]) ||
		ptr[3] == '-');

	if (ptr != (unsigned char*)buf)
		strcpy(buf, (char*)ptr);

	status = atoi((char*)ptr);

	return (S_OK);	
}

HRESULT QCSMTPThreadMT::ReadLine(JJFileMT* file, char* buf, int bSize, 
								  long& lNumBytesRead )
{
	HRESULT hr;

	lNumBytesRead = 0;
	// Send in bSize -1 to make sure there is room for NULL
	if (FAILED(hr=(file->GetLine(buf, bSize-1, &lNumBytesRead))) || (hr == S_FALSE) )
		return hr;

	// Add back the stripped off CRLF and NULL terminate
	char* End = buf;
	if (lNumBytesRead >= 2)
		End += lNumBytesRead - 2;
	End += strlen(End);
	strcpy(End, "\r\n");

	lNumBytesRead = strlen(buf);

	return S_OK;
}

HRESULT QCSMTPThreadMT::StartSMTP(const char* SMTPServer)
{
	int Status = 1;
 	char Server[256];

	SMTPConnected = FALSE;	  // DITCH
	m_bUsingPOPSend = FALSE;  // DITCH

	strncpy(Server, SMTPServer, sizeof(Server));
	CurSMTPServer = SMTPServer;	 // DITCH

	if (!m_pNetConnection)			// DITCH
	{
		//TODO : dont show progress bar when doing threads until we have thread-safe calls
		if (!(m_pNetConnection = CreateNetConnectionMT(m_Settings->m_pNetSettings, FALSE, FALSE)))	   // DITCH
			return E_FAIL;			   // DITCH
	}

	// Hesiod Kludge code.  There should really be a userinterface for this
	if (!stricmp(Server, "hesiod") && SUCCEEDED(QCHesiodLibMT::LoadHesiodLibrary()))
	{
		CString LoginName(m_Settings->GetPOPAccount());
		int At = LoginName.ReverseFind('@');
		if (At > 0)
			LoginName.ReleaseBuffer(At);
		int nHesiodError = -1;
		if (FAILED(QCHesiodLibMT::GetHesiodServer(CRString(IDS_POP_SERVICE), LoginName, Server, sizeof(Server), &nHesiodError)))
			*Server = 0;
		QCHesiodLibMT::FreeHesiodLibrary();
		if (!*Server)
		{
			ErrorDialog(IDS_ERR_HESIOD_HOST_LOOKUP);
			return E_FAIL;
		}
	}
	// End Hesiod kludge

	if (!m_bUsingPOPSend)
	{
		
		if(m_Settings->m_SSLSettings.GetSSLSendUsage() == SSLSettings::SSLUseAlternatePort)
		{
			short port = m_Settings->m_SSLSettings.m_SMTPAlternatePort;
			Status = m_pNetConnection->Open(Server, IDS_SMTP_SERVICE, port, 465);
		}
		else
		{
			int port = m_Settings->UseSubmissionPort()? m_Settings->GetSmtpSubmissionPort() : m_Settings->GetSmtpPort();
			Status = m_pNetConnection->Open(Server, IDS_SMTP_SERVICE, port, 25);  // DITCH
		}
		if(Status >= 0)
			SMTPConnected = TRUE;	//FORNOW		
	}

	if (Status < 0)
		return E_FAIL;
    if(m_Settings->m_SSLSettings.GetSSLSendUsage() == QCSSLSettings::SSLUseAlternatePort)
		{
		m_pNetConnection->SetSSLMode(TRUE,  m_strPersona, &m_Settings->m_SSLSettings, "SMTP");
		}

	return S_OK;
}


HRESULT QCSMTPThreadMT::DoSMTPIntro()
{
	HRESULT hr;
	//
	// Use of QCMS_BUF_SIZE assures that all our calls to 
	// CMailSettings class will not overrun these buffers.
	//
	char host[QCMS_BUF_SIZE];
	char buf[QC_NETWORK_BUF_SIZE];
	int status;

	if (m_bUsingPOPSend)
		return E_FAIL;

	m_pTaskInfo->SetMainText(CRString(IDS_SERVER_CONNECT));

	// Get the opening banner
	hr = GetSMTPReply( buf, sizeof(buf), status);
	if (status / 100 != 2)
	{
		// The return code was not 2xx
		if (status >= 0)
			ErrorDialog(IDS_ERR_SMTP_INTRO, buf);

			// No critical states to cleanup.  Just abort out of the connection
			SMTPConnected = FALSE;		//FORNOW
		return (E_FAIL);
	}

	if (QCWinSockLibMT::IsLibraryLoaded() == false || QCWinSockLibMT::gethostname(host, sizeof(host)) == SOCKET_ERROR)
	{
		// We couldn't figure anything else out for the HELO command, so
		// let's just send the name of the machine we're talking to
		strncpy(host, m_Settings->GetSMTPServer(), sizeof(host));
		::TrimWhitespaceMT(host);
		if (!*host)
		{
			strncpy(buf, m_Settings->GetPOPAccount(), sizeof(buf));
			const char* pa = buf;
			if (pa = strrchr(pa, '@'))
				strcpy(host, pa + 1);
		}
	}
	
	// if the hostname is a simple name, append the domain
	// we faked earlier
	if ( !strchr( host, '.' ) )
	{
		CString domain = m_Settings->GetBestGuessAtDomain();
		if ( !domain.IsEmpty() )
		{
			if ( domain[0] != '.' ) strcat( host, "." );	// dots are nice
			strcat( host, domain );
		}
	}
	
	// Prepare for ESMTP:
	//
	const BOOL bAuthRequired = m_Settings->AuthRequired();

	m_pEhlo = DEBUG_NEW_NOTHROW CEhlo();
	m_pSasl = DEBUG_NEW_NOTHROW CSasl();

	if (!m_pEhlo || !m_pSasl)
	{
		status = 500;
	}
	else
	{
		// Does the server support ESMTP? If so, we also grab capabilities.
		//
		// If EHLO fails, don't report to the UI as we have a backup plan of calling HELO.
		// However, if authentication is required, then a server that can't handle EHLO
		// certainly can't authenticate, so we need to report that and return failure.
		InEhlo (TRUE);
		SMTPCmdGetReply("EHLO ", host, buf, sizeof(buf), status, true, !bAuthRequired);
		InEhlo (FALSE);
	}

	hr = E_FAIL;


	if (status >= 400)
	{
		if (bAuthRequired)
			return E_FAIL;

		// Try plain old SMTP.
		SMTPCmdGetReply("HELO ", host, buf, sizeof(buf), status, true, true);

		// An error is not necessarily a catastrophic error for helo
		//
		hr = S_OK;
	}
	else if (status/100 == 2)
	{
		if ((m_Settings->m_SSLSettings.m_SSLSendAvailableOrAlways) && m_pEhlo->SupportsTLS())
		{
			HRESULT hr;
			hr = SMTPCmdGetReply("STARTTLS\r\n", NULL, buf, sizeof(buf), status, true, true);
			if (FAILED(hr))
			{
				// NAV can cause the STARTTLS command to fail even though the
				// server actually does support STARTTLS.
				if (m_pEhlo) m_pEhlo->SetTLS(FALSE);
				if (m_Settings->m_SSLSettings.m_nSSLSendUsage == SSLSettings::SSLUseAlways)
				{
					ErrorDialog(IDS_SSL_NOSUPPORT, m_Settings->GetSMTPServer());
					return E_FAIL;
				}
			}
			else
			{
				m_pNetConnection->SetSSLMode(TRUE,  m_strPersona, &m_Settings->m_SSLSettings, "SMTP");
				if (m_pEhlo) delete m_pEhlo;
				if (m_pSasl) delete m_pSasl;
				m_pEhlo = DEBUG_NEW_NOTHROW CEhlo();
				m_pSasl = DEBUG_NEW_NOTHROW CSasl();

				if (!m_pEhlo || !m_pSasl)
					return E_FAIL;

				InEhlo(TRUE);
				SMTPCmdGetReply("EHLO ", host, buf, sizeof(buf), status, true, !bAuthRequired);
				InEhlo(FALSE);
				// Forcibly set TLS support flag since the server may not advertise it again
				// after successful negotiation.
				m_pEhlo->SetTLS(TRUE);
				hr = S_OK;
			}
		}
		// some servers dont say they support SSL if they are already in the SSL mode in response to the EHLO command.-sagar
		else if (m_pEhlo && !m_pEhlo->SupportsTLS() && (m_Settings->m_SSLSettings.m_nSSLSendUsage == SSLSettings::SSLUseAlways))
		{
			ErrorDialog(IDS_SSL_NOSUPPORT, m_Settings->GetSMTPServer());
			return E_FAIL;
		}

		// We need to authenticate.
		//
		hr = DoSMTPAuth (status);
	}
	
	return hr;
}

HRESULT QCSMTPThreadMT::EndSMTP(BOOL abort)
{
	char buf[QC_NETWORK_BUF_SIZE];
	int status;

	if (SMTPConnected)			//FORNOW	
	{
		SMTPConnected = FALSE;	//FORNOW	

		if (!m_bUsingPOPSend)
		{
			if (!abort)
			{
				const bool bIgnoreErrors = m_Settings->IgnoreQUITErrors();
				if (bIgnoreErrors)
				{
					(m_Settings->GetNetSettings())->SetReportErrorCallback(  
						makeCallback( (QCNetworkSettings::ErrorCallback *)0, *m_pTaskInfo, &CTaskInfoMT::IgnoreError) );
				}
				SMTPCmdGetReply("QUIT\r\n", NULL, buf, sizeof(buf), status, true, bIgnoreErrors);
			}
			m_pNetConnection->Close();
		}
	}


	//Make sure we lock before deleting as this pointer can be used to request stop at any time..
	CSingleLock lock(&m_Guard, TRUE);
	if(m_pNetConnection)
	{
		delete m_pNetConnection;
		m_pNetConnection = NULL; //so that requeststop wont try to dereference it
	}
	
	return S_OK; 
}



BOOL QCSMTPThreadMT::PreProcessPOPSend()
{
	ASSERT(::IsMainThreadMT());
	BOOL bResult = TRUE;

	if(m_Settings->UsePOPSend())
	{
		m_bUsingPOPSend = true;

		//Setup for Using POP Send
		//Send mail using POP XTND XMIT
		QCPOPSettings *settings = DEBUG_NEW QCPOPSettings(m_strPersona, kSendMailBit);
		
		bResult = FALSE;
		m_POPSession = DEBUG_NEW_NOTHROW CPOPSession(settings);
		if (m_POPSession != NULL)
		{
			//Get the POP password in main thread
			bResult = m_POPSession->DoGetPassword_();
		}
	}
	else
	{
		QCNetworkSettings *pNetSettings = m_Settings->GetNetSettings();
		if( !pNetSettings->GetRasPassword())
			return FALSE;
	}

	// JOK, 12/21/98: If using ESMTP AUTH, we need to get user name and password
	// from the personality. If this fails, we need to ask the user.
	//
	bool bEsmtpAuth = m_Settings->AuthAllowed() || m_Settings->AuthRequired();
	if (bResult && bEsmtpAuth)
	{
		// Get the user's login and password for the "current" personality,
		// be it POP or IMAP. This assumes that we're going to be doing something like
		// CRAM-MD5 which requires the POP/IMAP password. For Kerberos, the password is
		// obtained elsewhere so we shouldn't do this here. That means that we need
		// to know at this point which authentication mechanism will be used.
		//
		bResult = GetLogin ();
	}

	return bResult;

}


HRESULT QCSMTPThreadMT::DoWork()
{
	HRESULT hr = E_FAIL;
	char Server[QCMS_BUF_SIZE];

	
	m_pTaskInfo->SetPostProcFn(CRString(IDS_SMTP_POST_FILTER_SENT_MESSAGES), PostProcessOutgoingMessages, (void *)this);
	
	if( IsStoppingThread())
	{
		m_pTaskInfo->SetMainText(CRString(IDS_TASK_STOPPED));
		goto DONE;
	}
	
	
	if(!m_Settings->UsePOPSend())
	{
		strncpy(Server, m_Settings->GetSMTPServer(),sizeof(Server)); 
	   	::TrimWhitespaceMT(Server);
		char* s;
		if (!*Server && (s = strrchr(m_Settings->GetPOPAccount(), '@')))
			strcpy(Server, s + 1);
 		

		// If the server name is Hesiod, StartSMTP queries the Hesiod server for the
		// users POP server.  The users SMTP Hesiod server is ignored at this time.
		if ( !FAILED( hr = StartSMTP(Server)	) &&
			 !FAILED( hr = DoSMTPIntro()		) )
		{
			hr = DoAllMessages();
		}
		
		EndSMTP(FALSE); //Result == SQR_MAJOR_ERROR);
	}
	else
	{
		//Authenticate
		BOOL bResult = FALSE;

		bResult = m_POPSession->SetupNetConnection_();
		if(!bResult) goto DONE;

		//Use POP network connection for sending the message
		m_pNetConnection = m_POPSession->GetNetConnection();
		
		bResult = m_POPSession->OpenPOPConnection_();
		if(!bResult) goto TEARDOWN;


		bResult = m_POPSession->DoLogon_();
		if(!bResult) goto CLOSE_POP;
		
		
		hr = DoAllMessages();
		

CLOSE_POP:
		bResult = m_POPSession->ClosePOPConnection_();
		//{
		//	char szBuffer[1024] = { 0 };
		//	m_POPSession->POPCmdGetReply(IDS_POP_QUIT, NULL, szBuffer, sizeof(szBuffer));
		//}


TEARDOWN:
		bResult = m_POPSession->TeardownNetConnection_();
	}
		

DONE:
	if(m_Settings->UsePOPSend())
	{
		//Set the SMTP net connection to NULL, so that smtp thread wont try to delete it
		m_pNetConnection = NULL;
		delete m_POPSession->GetTaskInfo();
		delete m_POPSession;
		m_POPSession = NULL;
	}


	//m_pTaskInfo->SetMainText("Completed. Waiting to process the results.");
	m_pTaskInfo->SetMainText(CRString(IDS_WAITING_TO_POST_PROCESS));

	//Ignore idle time if we only sent one msg..
	if(m_nMsgsToProcess == 1)
		m_pTaskInfo->IgnoreIdle();

	m_pTaskInfo->SetPostProcessing(true);  //do the post processing after here

	return hr;
}



#define IsWhite(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')



// EhloLine [PRIVATE]
//
// Process a 250 response line from an ESMTP server.
// Adapted from the MAC code.
//
void QCSMTPThreadMT::EhloLine(LPCTSTR pBuf)
{
	if (!pBuf)
		return;

	char directive [65];
	char value[256];
	const char* start, *end, *stop;

	if (m_pEhlo->SawGreeting())
	{
		return;
	}

	// Initialize these:
	//
	*directive = *value = 0;
	
	/*
	 * grab the Ehlo directive
	 */
	end = pBuf + strlen(pBuf);
	start = pBuf + 4;

	while (start < end && IsWhite(*start)) start++;

	// Old version of AUTH draft had syntax of "AUTH=<authentication list>"
	if (strnicmp(start, "AUTH=", 5) == 0)
		stop = start + 4;
	else
		for (stop = start; stop < end && !IsWhite(*stop); stop++);

	size_t len = stop - start;
	strncpy (directive, start, len);
	directive[len] = 0;

	if (directive[len-1]=='\r' || directive[len-1] == '\n')
		directive[--len] = 0;

	if (directive[len-1]=='\r' || directive[len-1] == '\n')
		directive[--len] = 0;
	
	/*
	 * and the value
	 */
	if (stop < end)
		start = stop + 1;
	else
		start = end;

	for (; start < end && IsWhite(*start); start++);

	if (start < end)
	{
		strncpy (value, start, sizeof(value));
		value[sizeof(value) - 1] = 0;
		len = strlen(value);

		if (value[len-1]=='\r' || value[len-1] == '\n')
			value[--len] = 0;

		if (value[len-1]=='\r' || value[len-1] == '\n')
			value[--len] = 0;
	}
	
	/*
	 * now what?
	 */
	switch(CEhlo::FindESMTPType (directive))
	{
		case ESMTP_SIZE:
			m_pEhlo->ParseMaxSize (value);
			break;

		case ESMTP_MIME8BIT:
			m_pEhlo->SetMime8bit (TRUE);
			break;

		case ESMTP_AUTH:
		{
			m_pEhlo->SetAuth (TRUE);
			m_pSasl->ParseAuthenticators (value);
			break;
		}

		case ESMTP_STARTTLS:
		{
			m_pEhlo->SetTLS(TRUE);
			break;
		}

#if 0 // Not yet
		case esmtpAmd5:
			SetPref(PREF_SMTP_PASS,YesStr);
			PSCopy(directive,(*CurPers)->password);
			GenDigest(value,directive,digest);
			PCopy((*Ehlo)->digest,digest);
			break;
		case esmtpPipeline:
			(*Ehlo)->pipeline = True;
			break;
#endif

	}
}



// SaslChallenge [PUBLIC]
//
// NOTE: (JOK) - This function returns a challenge from the server, already
// base64 decoded. The decoded challenge is copied to "outbuf", and the number of bytes
// in outbuf is returned.
// 
int QCSMTPThreadMT::SaslChallenge (char *outbuf, int outsize)
{
	if (!outbuf)
		return FALSE;

	char buf[QC_NETWORK_BUF_SIZE];
	*buf = 0;

	int status;
	GetSMTPReply(buf, sizeof(buf), status);
	if (status != 334)
	{
		if (status >= 0)
			SMTPError("", buf);
		return 0;
	}

	strcpy(buf, buf + 4);
	int len = B64DecodeString(buf);
	buf[len] = 0;

	// Make sure the output buffer can hold the data.
	if (len > outsize)
		return FALSE;

	// Now send the output back.
	if (len > 0)
	{
		buf[len] = 0;
		// Need to use memcpy() not strcpy() because data can contain NULLs.
		memcpy (outbuf, buf, len + 1);
	}

	return len;
}



// SaslResponse [PUBLIC]
//
// NOTE (JOK): Use this to send our resonse back to the server. Pass the response text
// via "Response".
//
BOOL  QCSMTPThreadMT::SaslResponse (char *response, unsigned long size)
{
	char *out = NULL;

	if (size)
	{
		// Encode response.
		//
		int outsize = (size * 4)/3 + 20;
		out = DEBUG_NEW char [outsize];

  		/* make CRLF-less BASE64 string */
		int len = B64EncodeString (response, size, out, outsize);

		int		j;
		char	*t, *u;

		for (t = out, u = t, j = 0; j < len; j++)
		{
			if (t[j] > ' ')
				 *u++ = t[j];
		}

		*u = '\0';

		/* append CRLF */
		*u++ = '\015'; *u++ = '\012';

		*u = '\0';
	}
	else if (response)
	{
		// Data string is empty but not NULL: send no data.
		out = DEBUG_NEW char [8];
		wsprintf (out, "\r\n");
	}
	else
	{
		// Data string is NULL: send SASL cancel.
		out = DEBUG_NEW char [8];
		wsprintf (out, "*\r\n");
	}

	// the command buffer is now ready, send the command to the server...
	int status = m_pNetConnection->PutDirect(out);

	// Cleanup:
	delete[] out;

	if (status < 0)
	{
		m_pNetConnection->Close();
		return (FALSE);
	}

	return TRUE;
}




HRESULT QCSMTPThreadMT::DoSMTPAuth (int& status)
{
	const bool bAuthRequired = m_Settings->AuthRequired();
	if (!m_Settings->AuthAllowed() && !bAuthRequired)
		return S_OK;

	// We only fail if the authentication is required
	HRESULT hr = bAuthRequired? E_FAIL : S_OK;

	// If we didn't even attempt to log in, this is zero.
	status = 0;

	if (!m_pSasl)
	{
		ASSERT (0);
		return hr;
	}

	CAuthenticator *pAuth = NULL;
	CString BadAuth(m_Settings->GetAuthBanished());
	
	// The logic gets a little funky here.  The default case is to have SMTP follow
	// the POP/IMAP setting for Kerberos5/GSSAPI.  If K5 is selected in POP/IMAP this
	// is trivial -- SMTP prefers GSSAPI first so the default behavior will result in
	// GSSAPI being used if the server offers it.  If K5 is not selected we need to
	// stop SMTP from selecting it and the simplest way to do that is to add "GSSAPI"
	// to the list of banished auth mechanisms.  However, some sites want the ability
	// to have SMTP do K5 without supporting it for POP/IMAP.  If the user chooses
	// not to have SMTP follow POP/IMAP just go with the default behavior.
	if (m_Settings->AuthK5FollowsINI() && !m_Settings->m_pNetSettings->DoAuthenticateKerberos())
	{
		if (!BadAuth.IsEmpty()) BadAuth += ",";
		BadAuth += "GSSAPI";
	}

	while (pAuth = m_pSasl->GetNextAuthenticator(pAuth? pAuth->GetAuthClient() : NULL))
	{
		if (BadAuth.IsEmpty())
			break;
		
		// Get rid of authentication methods that are verboten
		const char* CompareBad = BadAuth;
		const char* Current = pAuth->GetAuthenticatorType();
		int Len = strlen(Current);
		while (1)
		{
			if (strnicmp(Current, CompareBad, Len) == 0 &&
				(!CompareBad[Len] || CompareBad[Len] == ','))
			{
				break;
			}
			if (!(CompareBad = strchr(CompareBad, ',')))
				break;
			CompareBad++;
		}

		// If we get out of this loop and CompareBad is NULL, it means
		// we didn't find a match on the list of authenitcators to ditch
		if (!CompareBad)
			break;
	}

	if (!pAuth)
	{
		if (bAuthRequired)
			ErrorDialog(IDS_ERR_SMTP_NO_AUTH_PROTOCOL);
	}
	else
	{
		// Send the initial AUTH command.
		char cmdbuf[QC_NETWORK_BUF_SIZE];
		wsprintf (cmdbuf, "AUTH %s\r\n", pAuth->GetAuthenticatorType());
		m_pTaskInfo->SetSecondaryText(cmdbuf);

		status = m_pNetConnection->PutDirect(cmdbuf);

		if (status < 0)
		{
			m_pNetConnection->Close();
			return E_FAIL;
		}

		// Initiate the SASL mechanism.
		PAuthClient pClient = pAuth->GetAuthClient();
		if (pClient)
		{
			if ( (*pClient) (smtp_challenge, smtp_response, this, m_szUser) )
			{
				char buf[QC_NETWORK_BUF_SIZE];
				*buf = 0;

				// Get the last response.
				GetSMTPReply(buf, QC_NETWORK_BUF_SIZE, status);

				if (status == 235)
				{
					hr = S_OK;
				}
				else if (status >= 0)
				{
					// Only report errors when auth is required
					if (bAuthRequired)
						SMTPError("", buf);
				}
			}
		}
	}

	return hr;
}




// GetLogin [PRIVATE]
//
// 
BOOL QCSMTPThreadMT::GetLogin ()
{
	ASSERT ( IsMainThreadMT() );
	BOOL bResult = FALSE;

	// 
	if ( g_Personalities.IsImapPersona (m_strPersona) )
	{
		CImapAccount *pAccount = GetImapAccountMgr()->FindAccount (g_Personalities.GetHash( m_strPersona ));
		if (pAccount)
			bResult = pAccount->Login(m_szUser, m_szPassword);
	}
	else
	{
		// Must be POP. Note: we may have already gotten a password if
		// we're using POP send.
		//
		if (!m_POPSession)
		{
			//Setup for Using POP Send
			//Send mail using POP XTND XMIT
			QCPOPSettings *settings = DEBUG_NEW QCPOPSettings(m_strPersona, kSendMailBit);
		
			m_POPSession = DEBUG_NEW_NOTHROW CPOPSession(settings);
			if (m_POPSession != NULL)
			{
				//Get the POP password in main thread
				bResult = m_POPSession->DoGetPassword_();

				if (bResult)
				{
					m_szUser	 = m_POPSession->ExportLogin();
					int at = m_szUser.ReverseFind('@');
					if (at >= 0)
						m_szUser = m_szUser.Left (at);
						
					m_szPassword = m_POPSession->ExportPassword();
				}
					
			}
		}
	}

	return bResult;
}





//=================================================================================//


// Callback stubs for SASL. NOTE: This should use the template-based callback
// mechanism in callbacks.h.
//
int smtp_challenge (void *s, char *outbuf, int bufsize)
{
	QCSMTPThreadMT* pSmtp = (QCSMTPThreadMT *) s;

	return pSmtp ? pSmtp->SaslChallenge (outbuf, bufsize) : NULL;
}



/* Send authenticator response in BASE64
 * Accepts: CProtocol object.
 *			string to send
 *			length of string
 * Returns: T if successful, else NIL
 */

BOOL smtp_response (void *s, char *response, unsigned long size)
{
	QCSMTPThreadMT* pSmtp = (QCSMTPThreadMT *) s;

	return pSmtp ? pSmtp->SaslResponse (response, size) : FALSE;
}



// Return decodes count, or 0 if failed.
int B64DecodeString(char* s)
{
	Base64Decoder	d64;
	LONG			len;
	int				result;
	
	d64.Init();
	result = d64.Decode(s, strlen(s), s, len);

	if (result == 0)
		return len;
	else
		return 0;
}



// Encode the string "s", of length "sLen", into the given buffer "out".
// Return the length of the encoded string as the value of the function.
// Note: "outsize" MUST be 
int B64EncodeString (char* s, int sLen, char *out, int outsize)
{
	Base64Encoder	TheEncoder;
	LONG			OutLen = 0;
	char*			OutSpot = out;

	memset(out, 0, outsize);

	TheEncoder.Init(out, OutLen, 0);
	OutSpot += OutLen;
	TheEncoder.Encode(s, sLen, OutSpot, OutLen);
	OutSpot += OutLen;
	TheEncoder.Done(OutSpot, OutLen);
	OutLen += OutSpot - out;

	out[OutLen] = 0;

	return OutLen;
}
