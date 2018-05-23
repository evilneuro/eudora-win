// CAuditNagTask.h: interface for the audit nagging classes.
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

#if !defined(_CAUDITNAGTASK__INCLUDED_)
#define _CAUDITNAGTASK__INCLUDED_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CNagTask.h"


//////////////////////////////////////////////////////////////////////
// CAuditNagDialog: Dialog for audit nag
//////////////////////////////////////////////////////////////////////

class CAuditNagDialog : public CNagDialog
{
public:
	CAuditNagDialog(CNagTask* lpntNagTask, UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CAuditNagDialog();

	virtual BOOL			OnInitDialog();
	virtual void			OnOK();
	virtual void			DoDataExchange(CDataExchange* pDX);
	virtual void			UpdateState();

	virtual void			GetDialogText(CString & out_szDialogText);

	int						m_iSendDemo;
	int						m_iSendAdInfo;
	int						m_iSendNonPersonal;
	int						m_iSendUsage;
	int						m_iSendFeatures;
	CString					m_lpszText;
};


//////////////////////////////////////////////////////////////////////
// CAuditNagTask: Particular behavior for the audit task
//////////////////////////////////////////////////////////////////////

class CAuditNagTask : public CNagTask
{
public:

	CAuditNagTask(CNagTaskEntryP initEntry);
	virtual ~CAuditNagTask();

	virtual void			MaybeCancelNag(void *pData);
		// may need to update state of dialog checkboxes

	virtual bool			DoNag(NagTimes context);
		// overriden to randomize, and possibly do a lot of work to send the file

	virtual CWnd*			CreateNagWindow(void);
		// create the appropriate nag window for the given nag

	virtual void			DoAction(int iAction = IDOK);
		// do the action associated with the given nag, 
		// by default this is called on "ok" exit from dialog


protected:
	virtual bool			LineShouldBeSent(char* pBuffer, long len);
		// returns true if the audit line should be e-mailed in

	virtual	void			BuildAllowedMap(void);
		// from the dialog, sets m_allowed_map true for events that user says "ok to send".

	virtual	void			AllowCategory(int allowed);
		// helper function

	bool				m_allowed_map[CAuditEvents::MAXEVENT];
	unsigned	int		m_rand_seed;
};

#endif // !defined(_CAUDITNAGTASK__INCLUDED_)
