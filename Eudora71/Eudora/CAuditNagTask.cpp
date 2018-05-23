// CAuditNagTask.cpp: implementation for the audit nagging classes.
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "CAuditNagTask.h"
#include "compmsgd.h"
#include "summary.h"
#include "rs.h"
#include "debug.h"


#include "DebugNewHelpers.h"


// Constants, perhaps they should be in a resource, but I'm not sure we want anyone to change them...
#ifdef _DEBUG
const	char	*	const	kAuditEmailAddress	= "eudusage@qualcomm.com";
const	char	*	const	kAuditEmailSubject	= "Eudora usage statistics (debug)";
const	int					kAuditChances		= 10;	// 1 in 10 chance
#else
const	char	*	const	kAuditEmailAddress	= "eudusage@qualcomm.com";
const	char	*	const	kAuditEmailSubject	= "Eudora usage statistics";
const	int					kAuditChances		= 8000;	// 1 in 8000 chance
#endif

const	int		kMaxAuditLinesToSend	= 20000;	// maximum number of audit lines to send in an e-mail message
const	int		kMinAuditBytesToSend	= 5000;		// minimum bytes of audit data before we will try and send anything
const	int		kMaxAuditBytesToKeep	= 500000;	// maximum bytes of audit data before we truncate the log and start over


//////////////////////////////////////////////////////////////////////
// CAuditNagDialog: Dialog for audit nag
//////////////////////////////////////////////////////////////////////

CAuditNagDialog::CAuditNagDialog(CNagTask* pntNagTask, UINT nIDTemplate, CWnd* pParentWnd) :
	CNagDialog(pntNagTask, nIDTemplate, pParentWnd),
	m_iSendDemo(1),
	m_iSendAdInfo(1),
	m_iSendNonPersonal(1),
	m_iSendUsage(1),
	m_iSendFeatures(1)
{
	m_lpszText.LoadString(IDS_NAG_AUDIT_TEXT);
}

CAuditNagDialog::~CAuditNagDialog()
{
}

BOOL CAuditNagDialog::OnInitDialog()
{
	CNagDialog::OnInitDialog();

	// Hide the demographic data and ad info checkboxes if we
	// aren't in adware mode.
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		if (GetSharewareMode() != SWM_MODE_ADWARE)
		{
			CWnd	*pWndDemo = GetDlgItem(IDC_AUDIT_SENDDEMO);
			if (pWndDemo)
			{
				pWndDemo->ShowWindow(SW_HIDE);
			}
			CWnd	*pWndAdInfo = GetDlgItem(IDC_AUDIT_SENDADINFO);
			if (pWndAdInfo)
			{
				pWndAdInfo->ShowWindow(SW_HIDE);
			}
		}
	}
	return TRUE;
}

void CAuditNagDialog::OnOK()
{
	// "Generate Info" button was pressed.  Inform the nagtask what info to
	// send, then dismiss the dialog.

	m_bDoingTask = TRUE;

	UpdateData(TRUE);

	if (m_iSendDemo || m_iSendAdInfo || m_iSendNonPersonal || m_iSendUsage || m_iSendFeatures)
	{
		// Proceed only if there is some data to send.
		if (m_pntNagTask)
		{
			m_pntNagTask->DoAction();
		}
	}

	// CNagDialog::OnOK() will handle NULLing the nagtask's window reference.
	CNagDialog::OnOK();
}

void CAuditNagDialog::DoDataExchange(CDataExchange* pDX)
{
	CNagDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_AUDIT_STATIC, m_lpszText);
	DDX_Check(pDX, IDC_AUDIT_SENDDEMO, m_iSendDemo);
	DDX_Check(pDX, IDC_AUDIT_SENDADINFO, m_iSendAdInfo);
	DDX_Check(pDX, IDC_AUDIT_SENDNONSETTINGS, m_iSendNonPersonal);
	DDX_Check(pDX, IDC_AUDIT_SENDUSAGE, m_iSendUsage);
	DDX_Check(pDX, IDC_AUDIT_SENDFEATURES, m_iSendFeatures);

	if (GetSharewareMode() != SWM_MODE_ADWARE)
	{
		m_iSendDemo = FALSE;
		m_iSendAdInfo = FALSE;
	}
}

void CAuditNagDialog::UpdateState()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
	{
		int				nCmdShow = SW_SHOW;
		if (GetSharewareMode() != SWM_MODE_ADWARE)
		{
			nCmdShow = SW_HIDE;
		}
		CWnd	*pWndDemo = GetDlgItem(IDC_AUDIT_SENDDEMO);
		if (pWndDemo)
		{
			pWndDemo->ShowWindow(nCmdShow);
		}
		CWnd	*pWndAdInfo = GetDlgItem(IDC_AUDIT_SENDADINFO);
		if (pWndAdInfo)
		{
			pWndAdInfo->ShowWindow(nCmdShow);
		}
	}
}

void CAuditNagDialog::GetDialogText(CString & out_szDialogText)
{
	out_szDialogText = m_lpszText;
}


//////////////////////////////////////////////////////////////////////
// CAuditNagTask: Particular behavior for the audit task
//////////////////////////////////////////////////////////////////////

CAuditNagTask::CAuditNagTask(CNagTaskEntryP initEntry)
:	CNagTask(initEntry)
{
	m_rand_seed = 0;
}

CAuditNagTask::~CAuditNagTask()
{
}

void CAuditNagTask::MaybeCancelNag(void *pData)
{
	if (m_active_window)
	{
		static_cast<CAuditNagDialog *>(m_active_window)->UpdateState();
	}
}

bool CAuditNagTask::DoNag(NagTimes context)
{
// No audit nagging anymore
//	if (GetIniShort(IDS_INI_NO_AUDIT_NAG) != 0)
//	{
//		// User doesn't want to see audit nag.
		return false;
//	}
//	else if (m_active_window)
//	{
//		// If we already have a window for this nag bring it to the front.
//		m_active_window->SetFocus();
//		return true;
//	}
//	else
//	{
//		ULONGLONG			fbytesize	= 0;
//
//		CAuditEvents*		auditor = CAuditEvents::GetAuditEvents();
//		CAuditLog*			auditlog = (auditor ? auditor->GetAuditLog() : NULL);
//
//		if (auditlog)
//		{
//			CFileStatus fileStatus;
//			CString		logfname;
//
//			auditlog->GetLogFileName(logfname);
//			CFile::GetStatus(logfname, fileStatus);
//			fbytesize = fileStatus.m_size;
//		}
//
//		#ifdef _DEBUG
//		// See if the nag is being forced.
//		if (GetIniLong(IDS_INI_DEBUG_FORCE_NAG))
//		{
//			// Don't actually have to check the return value for the force nag
//			// INI entry.  If the entry exists and we got here, it is us.
//			return CNagTask::DoNag(context);
//		}
//		#endif
//
//		if (fbytesize < kMinAuditBytesToSend)
//		{
//			// aww, you're not big enough said the grouchy ladybug
//			return false; // return false so that the last nag date isn't reset, and we try again later.
//		}
//
//		// Randomly pick if we should audit
//		if (m_rand_seed == 0)
//		{
//			m_rand_seed = (unsigned int)time(NULL);
//		    srand(m_rand_seed);
//		}
//		long	ticket = ((rand() * kAuditChances) / RAND_MAX) + 1;
//
//		if (ticket >= (kAuditChances - 1))
//		{
//			return CNagTask::DoNag(context);
//		}
//		else
//		{
//			TRACE1("CAuditNagTask: no nag, checking size to truncate", ticket);
//
//			if (fbytesize > kMaxAuditBytesToKeep) 
//			{
//				auditlog->CloseLog();	// Close the log so it flushes, etc.
//				auditlog->OpenLog(false /* doRead */, true /* doTruncate */);	// reopen the log for more auditing fun, truncating it as we go.
//			}		
//		}
//	}
//	return false;
}

void CAuditNagTask::DoAction(int iAction)
{
	CAuditEvents*	auditor = CAuditEvents::GetAuditEvents();
	CAuditLog*		auditlog = (auditor ? auditor->GetAuditLog() : NULL);

	if (auditlog)
	{
		#ifdef QUEUE_MSG
		CString		tmpfname		= GetTmpFileNameMT("audit");
		JJFileMT	jjTmpFile;
		#else
		CString		strBody(_T(""));
		#endif
		JJFileMT*	jjLogFileP		= NULL;
		bool		bMadeTempFile	= false;
		char		pBuffer[256];
		long		lNumBytesRead;
		HRESULT		hr;

		BuildAllowedMap();		// use the dialog items to build a map that we filter by
		auditlog->CloseLog();	// Close the log so it flushes, etc.

		if (auditlog->OpenLog(true /* doRead */))
		{
			jjLogFileP = auditlog->GetLogFile();
		}

		// Get the e-mail address and message subject from the INI file (use default if none specified).
		CString		strAddress = GetIniString(IDS_INI_NAG_EMAIL_ADDRESS);
		if (strAddress == "")	strAddress = kAuditEmailAddress;
		CString		strSubject = GetIniString(IDS_INI_NAG_EMAIL_SUBJECT);
		if (strSubject == "")	strSubject = kAuditEmailSubject;

		int count = 0;
		#ifdef QUEUE_MSG
		bMadeTempFile = SUCCEEDED( jjTmpFile.Open (tmpfname, O_WRONLY ) );
		#else
		bMadeTempFile = true;
		#endif
		if (bMadeTempFile)
		{
			#ifdef QUEUE_MSG
			jjTmpFile.Put ("To: ");
			jjTmpFile.PutLine (strAddress);
			jjTmpFile.Put ("Subject: ");
			jjTmpFile.PutLine (strSubject);
			
			// Blank header/body separator line.
			jjTmpFile.PutLine ();

			// Flush the headers
			jjTmpFile.Flush();
			#endif

			#ifdef _DEBUG
			long lCount = 0;
			#endif

			// Now append the audit lines, filtering as we go
			while (SUCCEEDED (hr = jjLogFileP->GetLine(pBuffer, sizeof(pBuffer), &lNumBytesRead)) 
					&& (hr != S_FALSE)
					&& (count < kMaxAuditLinesToSend))
			{
				if (LineShouldBeSent(pBuffer, lNumBytesRead))
				{
					#ifdef _DEBUG
					// If debugging, check if any of the data lines don't conform
					// to the expected format.
					if (strcmp(pBuffer, "INFO STARTS HERE") == 0)	lCount = 1;
					if (lCount)	++lCount;
					if ((lCount > 4) && (pBuffer[10] != ' '))
					{
						MessageBeep(0);
					}
					#endif

					#ifdef QUEUE_MSG
					jjTmpFile.PutLine (pBuffer);
					#else
					strBody += pBuffer;
					strBody += "\r\n";
					#endif
					count++;
				}
			}

			#ifdef QUEUE_MSG
			jjTmpFile.Close();
			#endif
		}

		if (count) 
		{
			#ifdef QUEUE_MSG
			// To queue the message:
			NewMessageFromFile( tmpfname );
			#else
			// To open the message without queueing:
			CCompMessageDoc* pCompDoc = NewCompDocument(strAddress,	NULL, strSubject, NULL, NULL, NULL, strBody);
			pCompDoc->m_Sum->Display();
			#endif
		}
		else
		{
			// Nothing to mail?  Tell the user?
			TRACE("CAuditNagTask:No lines in the audit file, not making a message\n");
		}

		if (bMadeTempFile)
		{
			#ifdef QUEUE_MSG
			DeleteFile(tmpfname);
			#endif
		}
		auditlog->CloseLog();	// Close the log so it flushes, etc.
		if (count)
			auditlog->OpenLog(false /* doRead */, true /* doTruncate */);	// reopen the log for more auditing fun, truncating it as we go.
		else
			auditlog->OpenLog(true /* doRead */, false /* doTruncate */);	// didn't write anything, so don't get rid of old contents
	}
}

CWnd* CAuditNagTask::CreateNagWindow()
{
	CAuditNagDialog		*	pAuditNagDialog = NULL;

	pAuditNagDialog = DEBUG_NEW CAuditNagDialog(this, IDD_NAG_AUDIT, AfxGetMainWnd());
	pAuditNagDialog->Create(IDD_NAG_AUDIT, AfxGetMainWnd());

	return pAuditNagDialog;
}

bool CAuditNagTask::LineShouldBeSent(char* pBuffer, long len)
{
	bool	shouldbeSent = true;
	int		facility;

	if (len > 14 && isdigit((unsigned char)pBuffer[0]))
	{
		facility = atoi(pBuffer + 10 + 4);
		if ((facility < CAuditEvents::MAXEVENT) && (!m_allowed_map[facility]))
		{
			shouldbeSent = false;
		}
	}

	return shouldbeSent;
}

void CAuditNagTask::BuildAllowedMap(void)
{
	int i;

	for (i = 0; i < CAuditEvents::MAXEVENT; i++)
	{
		m_allowed_map[i] = false;
	}

	CAuditNagDialog*	auditDialog = reinterpret_cast<CAuditNagDialog*>(m_active_window);

	if (auditDialog != NULL) 
	{
		if (auditDialog->m_iSendDemo)		AllowCategory(CAuditEvents::kCategoryDemographics);
		if (auditDialog->m_iSendAdInfo)		AllowCategory(CAuditEvents::kCategoryAdvertisements);
		if (auditDialog->m_iSendNonPersonal)	AllowCategory(CAuditEvents::kCategoryNonPersonalSettings);
		if (auditDialog->m_iSendUsage)		AllowCategory(CAuditEvents::kCategoryNetEudoraUsage);
		if (auditDialog->m_iSendFeatures)	AllowCategory(CAuditEvents::kCategoryEudoraFeatures);
	}
}

void CAuditNagTask::AllowCategory(int allowed)
{
	int i;
	
	for (i = 1; i < CAuditEvents::MAXEVENT; i++)
	{
		if (CAuditEvents::CategoryMap[i] == allowed)
		{
			m_allowed_map[i] = true;
		}
	}
}
