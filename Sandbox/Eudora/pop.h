/////////////////////////////////////////////////////////////////////////////
// 	File: POP.H 
//
//  Purpose:  This file provides the declarations for the POP
//				routines.   
//
//	Author: Jeff Beckley
//	Created:   9/1/94
//  Modified:
//			  10/4/94	JCP		Added Kerberos capability
//  Modified: 3/3/98   Sudheer Koganti	for Thread-safety
//
/////////////////////////////////////////////////////////////////////////////

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


// Bit flags for mail checks...
const	unsigned long	kRetrieveNewBit			=   1;
const	unsigned long	kRetrieveMarkedBit		=   2;
const	unsigned long	kFetchHeadersBit		=   4;
const	unsigned long	kSendMailBit			=	8;
const	unsigned long	kSendIfSendOnCheckBit	=  16;
const	unsigned long	kLMOSBit				=  32;
const	unsigned long	kDeleteMarkedBit		=  64;
const	unsigned long	kDeleteRetrievedBit		= 128;
const	unsigned long	kDeleteAllBit			= 256;
const	unsigned long	kSilentCheckBit			= 512;

// These are temporary until I can fix the other modules...
const	unsigned long	kCheckPassword			= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kNetworkBit				= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kPOPBit					= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kLogonBit				= 0;	// FORNOW, obsolete ... remove from COMPMSGD.CPP
const	unsigned long	kNormalMailCheckBits	= 0;

const	unsigned long	kManualMailCheckBits	= kRetrieveNewBit + kRetrieveMarkedBit + kSendIfSendOnCheckBit + kLMOSBit + kDeleteMarkedBit;
const	unsigned long	kAutomaticMailCheckBits	= kManualMailCheckBits + kSilentCheckBit;



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

	void Decrement(int NoteFlags = 0);
};


class QCPOPNotifier : public QCMailNotifier 
{

public:
	bool m_bNotifyNewMail;
	int m_nTrashedMessageCount;
	int	m_nDownloadCount;
	bool m_bOpenInbox;

	QCPOPNotifier(long count) : QCMailNotifier(count), m_bNotifyNewMail(false), 
		m_nTrashedMessageCount(0), m_nDownloadCount(0), m_bOpenInbox(false){}
	virtual ~QCPOPNotifier(){ Decrement(); }

	void SetNotifyNewMail(bool nval){ if(!m_bNotifyNewMail) m_bNotifyNewMail = nval; }
	void AddTrashCount(int count){ m_nTrashedMessageCount += count; }
	void AddDownloadCount(int count){ m_nDownloadCount += count; }
};






//
// Global Function Prototypes
//
//int POPCmdGetReply(UINT uCmdId, const char* pszArgs, char* pszBuffer, int nBufferSize);
void GetMail(unsigned long bitflags, LPSTR CheckMailList = NULL);



//
// FORNOW, ugly globals...
//
extern	CPOPSession*			gPOP;  //fornow
extern	CHostList*		gPopHostList;

//
// Global functions.
//
extern  CMsgRecord*		GetMsgByHash( CSummary * Sum );
//extern	void			ClearKerberosTicket();



#endif // _POP_H_

