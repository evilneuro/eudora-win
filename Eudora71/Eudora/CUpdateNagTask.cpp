// CUpdateNagTask.cpp: implementation for the update nagging classes.
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
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "eudora.h"

#include "resource.h"
#include "CUpdateNagTask.h"
#include "afxinet.h"
#include "intshcut.h"
#include "debug.h"

#include "fileutil.h"
#include "guiutils.h"
#include "utils.h"
#include "md5.h"
#include "eulang.h"


#include "DebugNewHelpers.h"


const int		nDownloadBufferSize = 10000;


// A thread cannot use objects created in a different thread
// so resort to globals.
CUpdateNagTask::BackgroundStatus CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_IDLE;
CString CUpdateNagTask::m_strURLfilename = _T("");
CString CUpdateNagTask::m_strUpdateHost = _T("");
CString CUpdateNagTask::m_strUpdatePath = _T("");
CString CUpdateNagTask::m_strURLCheckSum = _T("");


///////////////////////////////////////////////////////////////////////////////
// santa's little helpers

// add an http header to the request -- if header exists, it is replaced
void add_http_header( CHttpFile* req, LPTSTR nam, LPTSTR val )
{
	TCHAR hdr[1025] = "";

	_sntprintf( hdr, sizeof(hdr) - 1, "%s: %s", nam, val );

	req->AddRequestHeaders( hdr, HTTP_ADDREQ_FLAG_REPLACE |
				HTTP_ADDREQ_FLAG_ADD );
}

// Check if the update page is newer, and download it if
// necessary, in a separate thread.
UINT CheckForDownloadPage(LPVOID lpParameter)
{
	CInternetSession	sess;

	// OK, I'm told we can't do a HEAD on the URL page, so just download
	// the page and checksum it to see if it has changed and we should show it.
	try
	{
		// Download the update page and save it to a local file.

		// Establish an internet connection.
		DWORD				 dwStatusCode;
		CHttpConnection		*pHttpConnect = sess.GetHttpConnection(CUpdateNagTask::m_strUpdateHost);
		CHttpFile			*pHttpFile = pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, CUpdateNagTask::m_strUpdatePath);

		// Open a local file for output.
		CFile				 fileout;
		if (fileout.Open(CUpdateNagTask::m_strURLfilename, CFile::modeCreate | CFile::modeWrite) == 0)
		{
			// If we can't open the output file, bail out.
			CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_ERROR_CONNECTING;
			pHttpFile->Close();
			pHttpConnect->Close();
			delete pHttpFile;
			delete pHttpConnect;
			return 0;
		}

		// additional request headers
		TCHAR langCode[6] = "";
		LNG_GetLanguageInfo( LANG_INFO_RFC1766, langCode );
		add_http_header( pHttpFile, _T("Content-Language"), langCode );

		// Request the URL info
		pHttpFile->SendRequest();
		pHttpFile->QueryInfoStatusCode(dwStatusCode);
		if (dwStatusCode != HTTP_STATUS_OK && 
			dwStatusCode != HTTP_STATUS_NO_CONTENT)
		{
			// If we can't read the update page, bail out.
			CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_ERROR_CONNECTING;
			pHttpFile->Close();
			pHttpConnect->Close();
			delete pHttpFile;
			delete pHttpConnect;
			return 0;
		}

		// Get Content-Length:
		DWORD dwContentLength = 0xFFFFFFFF;
		if (dwStatusCode == HTTP_STATUS_NO_CONTENT)
			dwContentLength = 0;
		else
		{
			ULONG len = sizeof(dwContentLength);
			if (pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
									&dwContentLength, &len) == FALSE)
			{
				dwContentLength = 0xFFFFFFFF;
			}
		}

		// Read pbuf from pHttpFile and write it to fileout.
		char		pbuf[nDownloadBufferSize];
		CString		strCompleteFile;
		int			nBytesRead = 0;
		CTime		tStartTime(CTime::GetCurrentTime());

		while (dwContentLength != 0 &&
				strCompleteFile.Find("</html>") == -1 &&
				strCompleteFile.Find("</HTML>") == -1)
		{
			nBytesRead = pHttpFile->Read(pbuf, nDownloadBufferSize - 1);
			if (nBytesRead > 0)
			{
				pbuf[nBytesRead] = 0;
				fileout.Write(pbuf, nBytesRead);
				strCompleteFile += pbuf;
				dwContentLength -= nBytesRead;
			}
			// If 60 seconds have elapsed and we still haven't found the end
			// just give it up.
			if (CTime::GetCurrentTime() - tStartTime > 60)
			{
				nBytesRead = -1;
				break;
			}
		}
		fileout.Close();

		// Close the internet connection.
		pHttpFile->Close();
		pHttpConnect->Close();
		delete pHttpFile;
		delete pHttpConnect;

		// Entire file was not downloaded so bail.
		if (nBytesRead == -1)
		{
			CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_ERROR_CONNECTING;
			return 0;
		}

		// Checksum the file.
		LPCTSTR		ptstr = strCompleteFile;
		MD5_CTX			mdContext;
		MD5Init(&mdContext);
		MD5Update(&mdContext, (unsigned char *)ptstr, strCompleteFile.GetLength());
		MD5Final(&mdContext);

		// Encode the checksum as a string for the INI file.
		CString			strCheckSum(_T(""));
		char			szEntry[16];
		for (int i = 0; i < 16; ++i)
		{
			sprintf(szEntry, "%d ", mdContext.digest[i]);
			strCheckSum += szEntry;
		}

		// Compare the checksum to the last checksum
		if (CUpdateNagTask::m_strURLCheckSum != strCheckSum)
		{
			// Note the new URL checksum.
			CUpdateNagTask::m_strURLCheckSum = strCheckSum;
			// Note that we have successfully downloaded the page.
			CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_PAGE_DOWNLOADED;
		}
		else
		{
			// Note that the page isn't newer.
			CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_PAGE_NOT_NEWER;
		}
		return 0;
	}
	catch (CInternetException *pEx)
	{
		// Quietly exit, returning false so we no nag didn't actually happen.
		pEx->Delete();
		CUpdateNagTask::m_BackgroundStatus = CUpdateNagTask::BG_ERROR_CONNECTING;
		return 0;
	}
}

// Assure that the given rect is within the rectangle for pWnd.
void AssureVisibleInWindow(CRect &rect, CWnd *pWnd)
{
	if (pWnd)
	{
		CRect			rectParent;
		int				iWidth = rect.Width();
		int				iHeight = rect.Height();
		pWnd->GetWindowRect(&rectParent);
		if (rect.left < 0)
		{
			rect.left = 0;
			rect.right = rect.left + iWidth;
		}
		if (rect.top < 0)
		{
			rect.top = 0;
			rect.bottom = rect.top + iHeight;
		}
 		if (rect.right > rectParent.Width())
		{
			rect.left = rectParent.Width() - rect.Width();
			rect.right = rect.left + iWidth;
		}
		if (rect.bottom > rectParent.Height())
		{
			rect.top = rectParent.Height() - rect.Height();
			rect.bottom = rect.top + iHeight;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CUpdatePaigeView: Paige view for update nag
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CUpdatePaigeView, CPaigeEdtView)

CUpdatePaigeView::CUpdatePaigeView()
{
	SetReadOnly();
}

CUpdatePaigeView::~CUpdatePaigeView()
{
}

void CUpdatePaigeView::Clear()
{
	pgSetSelection(m_paigeRef, 0, pgTextSize(m_paigeRef), 0, TRUE);
	pgDelete(m_paigeRef, NULL, best_way);
}

void CUpdatePaigeView::ResetCaret()
{
	pgSetSelection(m_paigeRef, 0, 0, 0, TRUE);
}

void CUpdatePaigeView::HideCaret()
{
	pgSetHiliteStates(m_paigeRef, deactivate_verb, no_change_verb, TRUE);
}

void CUpdatePaigeView::Home()
{
	// Set caret to the beginning.
	ResetCaret();
	HideCaret();

	// Update the scrollbars and scroll to the beginning.
	UpdateScrollBars(true);
	ScrollToCursor();
}

void CUpdatePaigeView::OnInitialUpdate()
{
	CPaigeEdtView::OnInitialUpdate();
}


//////////////////////////////////////////////////////////////////////
// CUpdateNagDoc: Document for update nag window
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CUpdateNagDoc, CDoc)

CUpdateNagDoc::CUpdateNagDoc()
{
}

CUpdateNagDoc::~CUpdateNagDoc()
{
}

BOOL CUpdateNagDoc::OnNewDocument()
{		
	if (!CDoc::OnNewDocument())
	{
		return FALSE;
	}

	SetTitle(CRString(IDR_UPDATE_NAG));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CUpdateNagWnd: Window for update nag
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CUpdateNagWnd, CMDIChild)

BEGIN_MESSAGE_MAP(CUpdateNagWnd, CMDIChild)
	ON_WM_CREATE()
    ON_WM_SIZE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

CUpdateNagWnd::CUpdateNagWnd(CUpdateNagTask* pntNagTask) :
	CMDIChild(),
	m_pntNagTask(pntNagTask),
	m_pPaige(NULL),
	m_pDoc(NULL),
	m_bLatestPageDisplayed(FALSE)
{
}

CUpdateNagWnd::~CUpdateNagWnd()
{
	// m_pDoc is deleted when RemoveView() is called in OnClose().
	if (m_pntNagTask)	m_pntNagTask->SetActiveWindow(NULL);
}

void CUpdateNagWnd::OnSize(UINT nType, int cx, int cy)
{
	CMDIChild::OnSize(nType, cx, cy);
	if (m_pPaige)
	{
		m_pPaige->MoveWindow(0, 0, cx, cy);
	}
}

void CUpdateNagWnd::OnClose()
{
	SaveWindowRect();
	if (m_pntNagTask)
	{
		m_pntNagTask->SetActiveWindow(NULL);
	}

	CMDIChild::OnClose();
}

void CUpdateNagWnd::RefreshWindow()
{
	if (m_pPaige)
	{
		if (!m_bLatestPageDisplayed)
		{
			// If the page we are displaying isn't the latest, reread the file.
			CFile		fileHTML;
			CString		strFilename(_T(""));
			if (m_pntNagTask)
			{
				strFilename = m_pntNagTask->GetURLFilename();
			}

			if (fileHTML.Open(strFilename, CFile::modeRead) == 0)
			{
				// If the file cannot be opened, just bail.
				return;
			}

			// We never expect the file to be big enough to worry about the
			// original ULONGLONG length. Just cast it to fit in a DWORD.
			DWORD		dwFileLength = static_cast<DWORD>( fileHTML.GetLength() );
			if (dwFileLength > 0)
			{
				char		*pbufHTML = DEBUG_NEW char[dwFileLength + 1];
				fileHTML.Read(pbufHTML, dwFileLength);
				fileHTML.Close();
				pbufHTML[dwFileLength] = '\0';

				// Clear the Paige view.
				m_pPaige->Clear();

				// Import HTML from the file.
				m_pPaige->SetAllHTML(pbufHTML, FALSE);

				// SetAllHTML() modifies the document, but it isn't really modified.
				if (m_pDoc)		m_pDoc->SetModifiedFlag(FALSE);

				m_pPaige->Home();

				delete [] pbufHTML;
				pbufHTML = NULL;

				m_bLatestPageDisplayed = TRUE;

				// If we already have a window for this nag bring it to the front.
				if (IsIconic())
				{
					ShowWindow(SW_RESTORE);
				}
				else
				{
					ShowWindow(SW_SHOW);
				}
			}
		}
	}
}

void CUpdateNagWnd::SaveWindowRect()
{
	if (m_pntNagTask)
	{
		CRect		rect;
		CRect		rectParent;
		CRect		rectSave;
		CWnd		*parent = GetParent();

		WINDOWPLACEMENT		wndpl;
		GetWindowPlacement(&wndpl);

		if ((wndpl.showCmd == SW_SHOWMAXIMIZED) || (wndpl.showCmd == SW_SHOWMINIMIZED))
		{
			rect = wndpl.rcNormalPosition;
			rectSave.left = rect.left;
			rectSave.top = rect.top;
		}
		else
		{
			GetWindowRect(&rect);
			parent->GetWindowRect(&rectParent);
			// Convert left and top to the window's coordinate system and
			// modify by a fudge of 2.
			rectSave.left = rect.left - rectParent.left - 2;
			rectSave.top = rect.top - rectParent.top - 2;
		}
		rectSave.right = rectSave.left + rect.Width();
		rectSave.bottom = rectSave.top + rect.Height();
		m_pntNagTask->SaveWindowRect(rectSave);
	}
}

void CUpdateNagWnd::SetNagTask(CUpdateNagTask *pntNagTask)
{
	m_pntNagTask = pntNagTask;
}

CUpdateNagTask * CUpdateNagWnd::GetNagTask() const
{
	return m_pntNagTask;
}

void CUpdateNagWnd::SetLatestPageDisplayed(BOOL bLatestPageDisplayed)
{
	m_bLatestPageDisplayed = bLatestPageDisplayed;
}

void CUpdateNagWnd::SetPaigeView(CUpdatePaigeView *pPaige)
{
	m_pPaige = pPaige;
}

void CUpdateNagWnd::SetUpdateDoc(CDocument *pDoc)
{
	m_pDoc = pDoc;
}


//////////////////////////////////////////////////////////////////////
// CUpdateNagTask: Particular behavior for the update task
//////////////////////////////////////////////////////////////////////

CUpdateNagTask::CUpdateNagTask(CNagTaskEntryP initEntry)
:	CNagTask(initEntry),
	m_strUpdateURL(_T("")),
	m_bNagDone(false)
{
	// Get temporary filename from the string resource.
	CString		strFileName;
	strFileName.LoadString(IDS_NAG_UPDATE_URL_FILE);
	m_strURLfilename = EudoraDir + strFileName;

	// Get the URL for the upload page from the string resource.
	GetJumpURL(&m_strUpdateURL);
	ConstructURLWithQuery(m_strUpdateURL, ACTION_UPDATE);

	// Derive the hostname from the URL.
	m_strUpdateHost = m_strUpdateURL;
	if (m_strUpdateHost.Left(7) == _T("http://"))
	{
		m_strUpdateHost = m_strUpdateHost.Mid(7);
	}
	int iSlashPos = m_strUpdateHost.Find('/');
	if (iSlashPos != -1)
	{
		m_strUpdatePath = m_strUpdateHost.Mid(iSlashPos);
		m_strUpdateHost = m_strUpdateHost.Left(iSlashPos);
	}

	// Retrieve the last update page mod date.
	m_strURLCheckSum = GetIniString(IDS_INI_NAG_UPDATE_PAGE_CHECKSUM);
}

CUpdateNagTask::~CUpdateNagTask()
{
}

bool CUpdateNagTask::Save()
{
	// SetIniLong() returns a BOOL, Save() returns a bool.
	return (SetIniString(IDS_INI_NAG_UPDATE_PAGE_CHECKSUM, GetURLCheckSumString()) && CNagTask::Save());
}

bool CUpdateNagTask::ShouldNag(NagTimes context)
{
	if (GetIniShort(IDS_INI_NO_UPDATE_NAG) != 0)
		return false;

	switch (CUpdateNagTask::m_BackgroundStatus)
	{
	case BG_IDLE:
		// Break out to check for new page below
		break;

	case BG_STILL_TRYING:
		// A previous attempt is still happening in the background.
		return false;

	case BG_ERROR_CONNECTING:
		// What do we do on error?
		// If we move back to the BG_IDLE state, we'll just be checking every
		// hour.  Let's just punt for now, and we'll wind up checking every
		// time the user starts up (since we start in the BG_IDLE state).
		return false;

	case BG_PAGE_DOWNLOADED:
		CUpdateNagTask::m_BackgroundStatus = BG_IDLE;
		SetURLCheckSumString(CUpdateNagTask::m_strURLCheckSum);
		return true;

	case BG_PAGE_NOT_NEWER:
		CUpdateNagTask::m_BackgroundStatus = BG_IDLE;
		struct _stat	st;
		if (_stat(GetURLFilename(), &st) != -1 && st.st_size != 0)
		{
			#ifdef _DEBUG
			// See if the nag is being forced.
			if (GetIniLong(IDS_INI_DEBUG_FORCE_NAG))
			{
				// Don't actually have to check the return value for the force nag
				// INI entry.  If the entry exists and we got here, it is us.
				return true;
			}
			#endif
			// If 30 days have passed since last nag, go ahead and nag.
			CTime		tLastNag(LastNagTime());
			CTime		tCurrTime(CTime::GetCurrentTime());
			CTimeSpan	tsTimeSinceLastNag(tCurrTime - tLastNag);

			if (tsTimeSinceLastNag.GetTotalSeconds() / NAG_SECS_PER_DAY > 30 )
			{
				return true;
			}
		}

		// We're not actually going to do a nag (returning false),
		// but we do need to update the last nag status
		m_nag_last = time(NULL);
		Save();
		return false;
	}

	if (!IsOffline())
	{
		// We want to do the update page checking and downloading in a
		// separate thread so it doesn't block the main thread.  Since a
		// second thread can't use objects created in the main thread
		// we use these variables to keep track of the status of the
		// background thread and to convey info to the other thread (e.g.,
		// the host and path names).
		CUpdateNagTask::m_BackgroundStatus = BG_STILL_TRYING;

		// Start the second thread.
		AfxBeginThread((AFX_THREADPROC) CheckForDownloadPage, 
				   NULL, 
				   THREAD_PRIORITY_NORMAL, 
				   0,
				   0);
	}

	// Don't do nag just yet.  After we get the results from the background fetch
	// we'll be back in here and will deal with what happened during the fetch.
	return false;
}

bool CUpdateNagTask::NagInProgress(NagTimes context)
{
	bool		bReturn = false;
	if (ShouldNag(context))
	{
		bReturn = CNagTask::NagInProgress(context);
		if (m_active_window) ((CUpdateNagWnd*)m_active_window)->RefreshWindow();
	}
	return bReturn;
}

bool CUpdateNagTask::DoNag(NagTimes context)
{
	bool		bReturn = false;
	if (ShouldNag(context))
	{
		bReturn = CNagTask::DoNag(context);
		if (m_active_window) ((CUpdateNagWnd*)m_active_window)->RefreshWindow();
	}
	return bReturn;
}

CWnd* CUpdateNagTask::CreateNagWindow()
{
	CFileStatus fs;
	if (!CFile::GetStatus(m_strURLfilename, fs) || fs.m_size == 0)
		return NULL;

	CUpdateNagWnd	*pUpdateNagWindow = NULL;
	CDocument		*pDoc = NewChildDocFrame(UpdateNagTemplate);

	POSITION pos = pDoc->GetFirstViewPosition();
	if (pos)
	{
		CView		*pView = pDoc->GetNextView(pos);
		if (pView)
		{
			pUpdateNagWindow = (CUpdateNagWnd*)(pView->GetParentFrame());
			if (pUpdateNagWindow)
			{
				ASSERT_KINDOF(CUpdatePaigeView, pView);
				pUpdateNagWindow->SetPaigeView((CUpdatePaigeView*)pView);
				pUpdateNagWindow->SetUpdateDoc(pDoc);
			}
		}
	}

	if (pUpdateNagWindow)
	{
		pUpdateNagWindow->SetNagTask(this);
		pUpdateNagWindow->RefreshWindow();
	}
	return pUpdateNagWindow;
}

void CUpdateNagTask::SetURLCheckSumString(CString& strCheckSum)
{
	m_strURLCheckSum = strCheckSum;
}

CString CUpdateNagTask::GetURLCheckSumString()
{
	return m_strURLCheckSum;
}

CString CUpdateNagTask::GetURLFilename()
{
	return m_strURLfilename;
}

void CUpdateNagTask::SetNagDone(bool bNagDone)
{
	m_bNagDone = bNagDone;
}

time_t CUpdateNagTask::LastNagTime()
{
	return m_nag_last;
}
