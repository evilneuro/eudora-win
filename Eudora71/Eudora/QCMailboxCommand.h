// QCMailboxCommand.h: interface for the QCMailboxCommand class.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#if !defined(AFX_QCMAILBOXCOMMAND_H__171DE302_BCD1_11D0_97C3_00805FD2F268__INCLUDED_)
#define AFX_QCMAILBOXCOMMAND_H__171DE302_BCD1_11D0_97C3_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCCommandObject.h"
#include "tocdoc.h"

class QCMailboxDirector;
class CTocDoc;

enum  UnreadStatusType
{
	US_UNKNOWN,
	US_YES,
	US_NO,
};

enum  COMPACT_RESULT_TYPE
{
    CRT_COMPACTED,
    CRT_MILD_ERROR,
    CRT_DRASTIC_ERROR
};

class QCMailboxCommand : public QCCommandObject  
{
	DECLARE_DYNAMIC(QCMailboxCommand)

	CString				m_szName;
	CString				m_szPathname;
	MailboxType			m_theType;		// Type of mailbox
	UnreadStatusType	m_theStatus;	// Does mailbox have unread messages?
	CPtrList			m_theChildList;

public:
	QCMailboxCommand(QCMailboxDirector* pDirector, LPCSTR szName, LPCSTR szPathname,
						MailboxType theType = MBT_REGULAR, UnreadStatusType theStatus = US_UNKNOWN);
	virtual ~QCMailboxCommand();

	virtual void Execute(COMMAND_ACTION_TYPE theAction, void* pData = NULL);

	LPCSTR				GetName() const { return m_szName; }
	const CString&		GetPathname() const { return m_szPathname; }
	MailboxType			GetType() const { return m_theType; }
	UnreadStatusType	GetStatus() const { return m_theStatus; }
	CPtrList&			GetChildList() { return m_theChildList; }

	BOOL				IsJunk();
	BOOL				IsTrash();

	virtual BOOL IsEmpty();
		
	virtual UINT GetFlyByID(COMMAND_ACTION_TYPE theAction);
	virtual CString	GetToolTip(COMMAND_ACTION_TYPE theAction);

	// Overrideable. Descendent classes can override,
	// e.g. IMAP mailbox command objects.
	//
	virtual BOOL CanHaveChildren() { return m_theType == MBT_FOLDER; }

	// Added to handle IMAP cases.
	virtual BOOL CanContainMessages() { return m_theType != MBT_FOLDER; }

	// Shortcut to determining if it's an IMAP command type.
	BOOL IsImapType() { return (	m_theType == MBT_IMAP_ACCOUNT ||
									m_theType == MBT_IMAP_MAILBOX ||
									m_theType == MBT_IMAP_NAMESPACE); }

protected:
	friend class QCMailboxDirector;
	
	void SetName(LPCSTR szName) { m_szName = szName; }
	void SetPathname(LPCSTR szPathname);
	void SetType(MailboxType theType) { m_theType = theType; }

	COMPACT_RESULT_TYPE CompactMailbox(CTocDoc* pTocDoc, bool bUserRequested = false);

	virtual void NotifyDirector(COMMAND_ACTION_TYPE theAction, void* pData = NULL)
					{ QCCommandObject::NotifyDirector(theAction, pData); }
};

#endif // !defined(AFX_QCMAILBOXCOMMAND_H__171DE302_BCD1_11D0_97C3_00805FD2F268__INCLUDED_)
