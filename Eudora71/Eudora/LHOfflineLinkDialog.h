//	LHOfflineLinkDialog.h
//
//	Specific implementation of a CDialog for asking the user what to
//	do when a link is clicked while the computer is offline.
//
//	Copyright (c) 1999 by QUALCOMM, Incorporated
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



#ifndef __LHOfflineLinkDialog_H__
#define __LHOfflineLinkDialog_H__


#include "LinkHistoryManager.h"
#include "resource.h"


class CLHOfflineLinkDialog : public CDialog
{
public:
	// Dialog Data
	enum { IDD = IDD_OFFLINE_LINK };
	
													CLHOfflineLinkDialog(CWnd * pParent = NULL);
	LinkHistoryManager::OfflineLinkActionEnum		GetUserChoice() const { return m_mlaUserChoice; }

protected:
	//	DDX/DDV support
	virtual void									DoDataExchange(CDataExchange* pDX);

	//	Message mapped	
	afx_msg void									OnVisitNowButton();
	afx_msg void									OnBookmarkButton();
	afx_msg void									OnRemindMeButton();
	DECLARE_MESSAGE_MAP()

	BOOL											m_bRememberChoice;
	LinkHistoryManager::OfflineLinkActionEnum		m_mlaUserChoice;
};


#endif	// __LHOfflineLinkDialog_H__
