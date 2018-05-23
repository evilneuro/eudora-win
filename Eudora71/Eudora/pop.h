// pop.h
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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


#ifndef _POP_H_
#define _POP_H_


class CSummary;	 
class JJFile;
class CHostList;
class CMsgRecord;
class CPOPSession;
 
// Data Items
extern JJFile* 	g_pPOPInboxFile;	// FORNOW, ugly global shared with HEADER.CPP, HEXBIN.CPP, LEX822.CPP, MIME.CPP, TRNSLATE.CPP, UUDECODE.CPP
extern long 	g_lEstimatedMsgSize;// FORNOW, ugly global shared with MIME.CPP, TRNSLATE.CPP, UUDECODE.CPP
extern BOOL 	g_bHasAttachment;	// FORNOW, ugly global shared with MIME.CPP
extern BOOL		g_bAttachmentDecodeEnabled;	// FORNOW, ugly global shared with MIME.CPP, HEADER.CPP
extern long		g_lBadCoding;		// FORNOW, ugly global shared with MIME.CPP, HEXBIN.CPP, and UUDECODE.CPP

extern int		g_nAttachmentCount;	//Shared with MIME.CPP for keeping track of number of attachments in a message


// Bit flags for mail checks...
const	unsigned long	kRetrieveNewBit			= 0x00000001;
const	unsigned long	kRetrieveMarkedBit		= 0x00000002;
const	unsigned long	kFetchHeadersBit		= 0x00000004;
const	unsigned long	kSendMailBit			= 0x00000008;
const	unsigned long	kSendIfSendOnCheckBit	= 0x00000010;
const	unsigned long	kLMOSBit				= 0x00000020;
const	unsigned long	kDeleteMarkedBit		= 0x00000040;
const	unsigned long	kDeleteRetrievedBit		= 0x00000080;
const	unsigned long	kDeleteAllBit			= 0x00000100;
const	unsigned long	kSilentCheckBit			= 0x00000200;
const	unsigned long	kValidateLoginOnly		= 0x00000400;

// These are temporary until I can fix the other modules...
const	unsigned long	kCheckPassword			= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kNetworkBit				= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kPOPBit					= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kLogonBit				= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kNormalMailCheckBits	= 0;

const	unsigned long	kManualMailCheckBits	= kRetrieveNewBit + kRetrieveMarkedBit + kSendIfSendOnCheckBit + kLMOSBit + kDeleteMarkedBit;
const	unsigned long	kAutomaticMailCheckBits	= kManualMailCheckBits + kSilentCheckBit;

// Test bits to see if we need to do a check at all
const	unsigned long	kNeedMailCheckBits		= 	kRetrieveNewBit +
													kRetrieveMarkedBit +
													kFetchHeadersBit +
													kDeleteMarkedBit +
													kDeleteRetrievedBit +
													kDeleteAllBit +
													kValidateLoginOnly;

//Keeps the state of GetMail notifications back to the caller
enum CHECK_MAIL_FLAGS { CMF_RECEIVED_MAIL = 0x0001, 
						CMF_DELETED_MAIL  = 0x0002, 
						CMF_NOTIFY_NOMAIL = 0x0004, 
						CMF_ALERT_NEWMAIL = 0x0008, 
						CMF_PLAY_SOUND    = 0x0010 
};


class QCMailNotifier {
	long m_nRefCount;
	int m_Notifications;


public:
	QCMailNotifier(long count) : m_nRefCount(count), m_Notifications(0){}
	virtual ~QCMailNotifier(){ Decrement(); }

	void AddNotifications(int notes){ InterlockedExchange((LONG *)&m_Notifications, m_Notifications | notes); }

	void Increment();
	void Decrement(int NoteFlags = 0);
};


class QCPOPNotifier : public QCMailNotifier 
{

public:
	int m_bNotifyNewMail;
	int m_nTrashedMessageCount;
	int m_nJunkedMessageCount;
	int	m_nDownloadCount;
	bool m_bOpenInbox;

	QCPOPNotifier(long count) : QCMailNotifier(count), m_bNotifyNewMail(-1), 
		m_nTrashedMessageCount(0), m_nJunkedMessageCount(0),
		m_nDownloadCount(0), m_bOpenInbox(false){}
	virtual ~QCPOPNotifier(){ Decrement(); }

	void SetNotifyNewMail(bool nval){ if(!m_bNotifyNewMail) m_bNotifyNewMail = nval; }
	void AddTrashCount(int count){ m_nTrashedMessageCount += count; }
	void AddJunkCount(int count){ m_nJunkedMessageCount += count; }
	void AddDownloadCount(int count){ m_nDownloadCount += count; }
};






//
// Global Function Prototypes
//
//int POPCmdGetReply(UINT uCmdId, const char* pszArgs, char* pszBuffer, int nBufferSize);
void GetMail(unsigned long bitflags, LPSTR CheckMailList = NULL, BOOL bFullMailCheck = TRUE);



//
// FORNOW, ugly globals...
//
extern	CPOPSession*			gPOP;  //fornow

//
// Global functions.
//
//extern	void			ClearKerberosTicket();


#endif // _POP_H_
