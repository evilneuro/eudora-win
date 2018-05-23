//	LHOfflineLinkDialog.cpp
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



#include "stdafx.h"

#include "LHOfflineLinkDialog.h"
#include "rs.h"


#include "DebugNewHelpers.h"


// ---------------------------------------------------------------------------
//		* CLHOfflineLinkDialog								[Public]
// ---------------------------------------------------------------------------
//	CLHOfflineLinkDialog constructor

CLHOfflineLinkDialog::CLHOfflineLinkDialog(CWnd * pParent /*=NULL*/)
	:	CDialog(CLHOfflineLinkDialog::IDD, pParent),
		m_bRememberChoice(FALSE), m_mlaUserChoice(LinkHistoryManager::ola_Cancel)
{

}


BEGIN_MESSAGE_MAP(CLHOfflineLinkDialog, CDialog)
	ON_BN_CLICKED(IDC_OFFLINE_LINK_VISIT_NOW, OnVisitNowButton)
	ON_BN_CLICKED(IDC_OFFLINE_LINK_BOOKMARK, OnBookmarkButton)
	ON_BN_CLICKED(IDC_OFFLINE_LINK_REMIND_ME, OnRemindMeButton)
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
//		* DoDataExchange										[Protected]
// ---------------------------------------------------------------------------
//	Save the IDS_INI_LH_OFFLINE_LINK_ACTION setting if appropriate.

void CLHOfflineLinkDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLHOfflineLinkDialog)
	DDX_Check(pDX, IDC_REMEMBER_CHECKBOX, m_bRememberChoice);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_bRememberChoice)
	{
		//	If user checked the "Remember my choice for next time" checkbox,
		//	then set the setting provided the choice makes sense
		if ( (m_mlaUserChoice == LinkHistoryManager::ola_Launch) ||
			 (m_mlaUserChoice == LinkHistoryManager::ola_Bookmark) ||
			 (m_mlaUserChoice == LinkHistoryManager::ola_Remind) )
		{
			SetIniLong(IDS_INI_LH_OFFLINE_LINK_ACTION, m_mlaUserChoice);
		}
	}
}


// ---------------------------------------------------------------------------
//		* OnVisitNowButton											[Public]
// ---------------------------------------------------------------------------
//	Message mapped method to handle click on the Visit Now button.

void CLHOfflineLinkDialog::OnVisitNowButton() 
{
	m_mlaUserChoice = LinkHistoryManager::ola_Launch;
	UpdateData(TRUE);
	EndDialog(IDC_OFFLINE_LINK_VISIT_NOW);
}


// ---------------------------------------------------------------------------
//		* OnVisitNowButton											[Public]
// ---------------------------------------------------------------------------
//	Message mapped method to handle click on the Bookmark button.

void CLHOfflineLinkDialog::OnBookmarkButton() 
{
	m_mlaUserChoice = LinkHistoryManager::ola_Bookmark;
	UpdateData(TRUE);
	EndDialog(IDC_OFFLINE_LINK_BOOKMARK);
}


// ---------------------------------------------------------------------------
//		* OnVisitNowButton											[Public]
// ---------------------------------------------------------------------------
//	Message mapped method to handle click on the Remind Me button.

void CLHOfflineLinkDialog::OnRemindMeButton() 
{
	m_mlaUserChoice = LinkHistoryManager::ola_Remind;
	UpdateData(TRUE);
	EndDialog(IDC_OFFLINE_LINK_REMIND_ME);
}
