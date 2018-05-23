// CUpdateNagTask.h: interface for the update nagging classes.
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

#if !defined(_CUPDATENAGTASK__INCLUDED_)
#define _CUPDATENAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "mdichild.h"
#include "CNagTask.h"
#include "PaigeEdtView.h"
#include "doc.h"

class CUpdateNagTask;


//////////////////////////////////////////////////////////////////////
// CUpdatePaigeView: Paige view for update nag
//////////////////////////////////////////////////////////////////////

class CUpdatePaigeView : public CPaigeEdtView
{
	DECLARE_DYNCREATE(CUpdatePaigeView)

public:
	CUpdatePaigeView();
	virtual ~CUpdatePaigeView();

	virtual void			Clear();
	virtual void			ResetCaret();
	virtual void			HideCaret();
	void					Home();

	virtual void			OnInitialUpdate();
};


//////////////////////////////////////////////////////////////////////
// CUpdateNagDoc: Document for update nag window
//////////////////////////////////////////////////////////////////////

class CUpdateNagDoc : public CDoc
{
	DECLARE_DYNCREATE(CUpdateNagDoc)

public:
	CUpdateNagDoc();
	~CUpdateNagDoc();

	virtual BOOL			OnNewDocument();
};

//////////////////////////////////////////////////////////////////////
// CUpdateNagWnd: Window for update nag
//////////////////////////////////////////////////////////////////////

class CUpdateNagWnd : public CMDIChild
{
protected:
	DECLARE_DYNCREATE(CUpdateNagWnd)

public:
	CUpdateNagWnd(CUpdateNagTask* pntNagTask = NULL);
	virtual ~CUpdateNagWnd();

	virtual void				RefreshWindow();
	virtual void				SaveWindowRect();
	virtual void				SetNagTask(CUpdateNagTask *lpntNagTask);
	virtual CUpdateNagTask	*	GetNagTask() const;
	virtual void				SetLatestPageDisplayed(BOOL bLatestPageDisplayed);
	virtual void				SetPaigeView(CUpdatePaigeView *pPaige);
	virtual void				SetUpdateDoc(CDocument *pDoc);

private:
	CUpdateNagTask			*	m_pntNagTask;
	CUpdatePaigeView		*	m_pPaige;
	CDocument				*	m_pDoc;
	BOOL						m_bLatestPageDisplayed;

protected:
    afx_msg void				OnSize(UINT, int, int);
	afx_msg void				OnClose();
	DECLARE_MESSAGE_MAP()
};


//////////////////////////////////////////////////////////////////////
// CUpdateNagTask: Particular behavior for the update task
//////////////////////////////////////////////////////////////////////

class CUpdateNagTask : public CNagTask
{
public:

	CUpdateNagTask(CNagTaskEntryP initEntry);
	virtual ~CUpdateNagTask();

	virtual bool				Save();

	virtual bool				ShouldNag(NagTimes context);
		// return whether or not conditions are right to nag

	virtual bool				NagInProgress(NagTimes context);
		// overriden to restore the nag if minimized

	virtual bool				DoNag(NagTimes context);
		// overriden to do the actual registration

	virtual CWnd*				CreateNagWindow(void);
		// create the appropriate nag window for the given nag

	virtual void				SetURLCheckSumString(CString& strCheckSum);
		// set the checksum for the URL page

	virtual CString				GetURLCheckSumString();
		// return the checksum for the URL page

	virtual CString				GetURLFilename();
		// return the URL filename

	virtual void				SetNagDone(bool bNagDone);
		// set whether or not the nag actually happened

	virtual time_t				LastNagTime();
		// Return time of last nag.

	typedef enum {
		// Status values for what is going on in the update thread.
		BG_IDLE,
		BG_STILL_TRYING,
		BG_ERROR_CONNECTING,
		BG_PAGE_NOT_NEWER,
		BG_PAGE_DOWNLOADED
	} BackgroundStatus;
	static BackgroundStatus		m_BackgroundStatus;

	static CString				m_strURLfilename;
	static CString				m_strUpdateHost;
	static CString				m_strUpdatePath;
	static CString				m_strURLCheckSum;

private:
	CString						m_strUpdateURL;
	bool						m_bNagDone;
};

#endif // !defined(_CUPDATENAGTASK__INCLUDED_)
