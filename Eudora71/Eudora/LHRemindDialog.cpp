//	LHRemindDialog.cpp
//
//	Specific implementation of a CDialog for asking the user whether
//	they want to display remind links now that the computer is
//	online.
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

#include "LHRemindDialog.h"
#include "rs.h"


#include "DebugNewHelpers.h"


// ---------------------------------------------------------------------------
//		* CLHRemindDialog								[Public]
// ---------------------------------------------------------------------------
//	CLHRemindDialog constructor

CLHRemindDialog::CLHRemindDialog(CWnd * pParent /*=NULL*/)
	:	CDialog(CLHRemindDialog::IDD, pParent),
		m_raUserChoice(LinkHistoryManager::ra_Show)
{

}


BEGIN_MESSAGE_MAP(CLHRemindDialog, CDialog)
	ON_BN_CLICKED(IDC_REMIND_SHOW, OnShowButton)
	ON_BN_CLICKED(IDC_REMIND_SNOOZE, OnSnoozeButton)
	ON_BN_CLICKED(IDC_REMIND_FORGET, OnForgetButton)
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
//		* OnShowButton												[Public]
// ---------------------------------------------------------------------------
//	Message mapped method to handle click on the Show Them button.

void CLHRemindDialog::OnShowButton() 
{
	m_raUserChoice = LinkHistoryManager::ra_Show;
	EndDialog(IDC_REMIND_SHOW);
}


// ---------------------------------------------------------------------------
//		* OnSnoozeButton											[Public]
// ---------------------------------------------------------------------------
//	Message mapped method to handle click on the Remind Later button.

void CLHRemindDialog::OnSnoozeButton() 
{
	m_raUserChoice = LinkHistoryManager::ra_Snooze;
	EndDialog(IDC_REMIND_SNOOZE);
}


// ---------------------------------------------------------------------------
//		* OnForgetButton											[Public]
// ---------------------------------------------------------------------------
//	Message mapped method to handle click on the Don't Remind button.

void CLHRemindDialog::OnForgetButton() 
{
	m_raUserChoice = LinkHistoryManager::ra_Forget;
	EndDialog(IDC_REMIND_FORGET);
}
