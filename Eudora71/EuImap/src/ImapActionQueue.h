// ImapActionQueue.h
//
// Header file of queue for IMAP actions.
//
// Copyright (c) 2005-2006 by QUALCOMM, Incorporated
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

#ifndef __IMAPACTIONQUEUE_H
#define __IMAPACTIONQUEUE_H

#include "QCWorkerThreadMT.h"

// Temporary switch to indicate whether to use the local/cache/online technique for IMAP operations
// or to do the operations online immediately.
const bool				bQueueAction = true;

const int		 kImapActionPriorityLow			= -1;
const int		 kImapActionPriorityNormal		= 0;
const int		 kImapActionPriorityHigh		= 1;



class CImapAccount;
class CImapMailbox;
class CSummary;
class CTocDoc;
class QCImapMailboxCommand;
class XMLWriter;

class CActionQueue;

//
// Base class for an IMAP operation to be cached.  Must be subclassed for specific use and subclass must
// provide an DoPreThreadAction() function to perform the action online.
//

class CImapAction : public QCWorkerThreadMT
{
public:
	CImapAction(unsigned long lAccountID,
				const char *szMailbox,
				TCHAR cDelimiter,
				const char *szMessageIDs,
				CString strLogin,
				CString strPassword);
	CImapAction();
	~CImapAction();

	virtual bool			 Write(XMLWriter *xmlWriter);	// Write the action to a file.
	virtual CImapAction		*Duplicate();					// Duplicate this object.
	virtual void			 Copy(CImapAction *pImapAction);	// Copy the action.

	virtual bool			 DoPreThreadAction();	// Perform actions to be done in main thread.
	virtual HRESULT			 DoWork();				// Perform the online action in the thread.
	virtual void			 DoPostProcessing();	// Perform post processing in the main thread.
	virtual void			 RequestThreadStop();	// A thread stop was requested.
	virtual bool			 UsesMailbox(unsigned long lMailboxHash);	// Does this action use this mailbox?
	virtual bool			 OpenMailbox(CImapMailbox *pImapMailbox);	// Wrapper for CImapMailbox::OpenMailbox().

	int						 m_iPriority;			// Priority of action within the queue.
	unsigned long			 m_lAccountID;			// Hash of account to act on.
	char					*m_szMailbox;			// Mailbox to act on.
	TCHAR					 m_cDelimiter;			// Mailbox delimiter.
	char					*m_szMessageIDs;		// String containing ID's of messages to act on.
	unsigned long			 m_lMailboxHash;		// Hash of mailbox name for quick comparing

	void					 SetPriority(int iPriority) {m_iPriority = iPriority;}
	int						 GetPriority() {return m_iPriority;}
	void					 SetAccountID(unsigned long lAccountID);
	void					 SetMailbox(const char *szMailbox);
	void					 SetDelimiter(char cDelimiter);
	void					 SetMessageIDs(const char *szMessageIDs);
	void					 SetActionQueue(CActionQueue *pActionQueue) {m_pActionQueue = pActionQueue;}
	CSummary				*GetSummaryFromUIDString(CString &strUIDList);
	int						 GetState() {return m_iState;}

	CString					 m_strLogin;			// User name for login.
	CString					 m_strPassword;			// Password for login.

	CImapAccount			*m_pAccount;			// Account the action belongs to.
	QCImapMailboxCommand	*m_pCommand;			// Pointer to command object for mailbox.
	CTocDoc					*m_pTocDoc;				// Pointer to TocDoc object for mailbox.
	CImapMailbox			*m_pImapMailbox;		// Pointer to mailbox object.

	CActionQueue			*m_pActionQueue;		// Queue containing this item.
	bool					 m_bIsLastOnMailbox;	// true if this is the last item acting on the mailbox
	int						 m_iState;				// State of this action.
};


//
// Subclass of CImapAction to perform status change of one or more messages.
//

class CImapChangeMsgStatusAction : public CImapAction
{
public:
	CImapChangeMsgStatusAction(unsigned long lAccountID,
							   const char *szMailbox,
							   TCHAR cDelimiter,
							   const char *szMessageIDs,
							   char cState,
							   BOOL bSet);
	CImapChangeMsgStatusAction();
	~CImapChangeMsgStatusAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();

	char				 m_cState;
	BOOL				 m_bSet;

	void				 SetState(char cState);
	void				 SetStateSet(BOOL bSet);
};


//
// Subclass of CImapAction to perform deletion of one or more messages.
//

class CImapDeleteMsgAction : public CImapAction
{
public:
	CImapDeleteMsgAction(unsigned long lAccountID,
						 const char *szMailbox,
						 TCHAR cDelimiter,
						 const char *szMessageIDs);
	CImapDeleteMsgAction();
	~CImapDeleteMsgAction();

	BOOL				 m_bAutoExpunge;

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
	virtual void		 DoPostProcessing();
};


//
// Subclass of CImapAction to perform undeletion of one or more messages.
//

class CImapUnDeleteMsgAction : public CImapAction
{
public:
	CImapUnDeleteMsgAction(unsigned long lAccountID,
						   const char *szMailbox,
						   TCHAR cDelimiter,
						   const char *szMessageIDs);
	CImapUnDeleteMsgAction();
	~CImapUnDeleteMsgAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
};


//
// Subclass of CImapAction to perform transfer of one or more messages.
//

class CImapTransferMsgAction : public CImapAction
{
public:
	CImapTransferMsgAction(unsigned long lAccountID,
						   const char *szMailbox,
						   TCHAR cDelimiter,
						   const char *szMessageIDs,
						   unsigned long lDestAccountID,
						   const char *szDestMailbox,
						   TCHAR cDestDelimiter,
						   BOOL bCopy);
	CImapTransferMsgAction();
	~CImapTransferMsgAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual void			 Copy(CImapAction *pImapAction);

	virtual bool			 DoPreThreadAction();
	virtual HRESULT			 DoWork() = 0;
	virtual void			 DoPostProcessing();
	virtual bool			 UsesMailbox(unsigned long lMailboxHash);
	CSummary				*GetSummaryFromUIDStringFromDestination(CString &strUIDList);

	unsigned long			 m_lDestAccountID;
	char					*m_szDestMailbox;
	TCHAR					 m_cDestDelimiter;
	unsigned long			 m_lDestMailboxHash;
	BOOL					 m_bCopy;
	BOOL					 m_bAutoExpunge;

	void					 SetDestAccountID(unsigned long lDestAccountID);
	void					 SetDestMailbox(const char *szDestMailbox);
	void					 SetDestDelimiter(char cDestDelimiter);
	void					 SetCopy(BOOL bCopy);

	CTocDoc					*m_pDestTocDoc;
	QCImapMailboxCommand	*m_pDestCommand;
	CDWordArray				 m_dwaNewUIDs;
};


//
// Subclass of CImapTransferMsgAction to perform same server transfer of one or more messages.
//

class CImapSameServerTransferMsgAction : public CImapTransferMsgAction
{
public:
	CImapSameServerTransferMsgAction(unsigned long lAccountID,
									 const char *szMailbox,
									 TCHAR cDelimiter,
									 const char *szMessageIDs,
									 unsigned long lDestAccountID,
									 const char *szDestMailbox,
									 TCHAR cDestDelimiter,
									 BOOL bCopy);
	CImapSameServerTransferMsgAction();
	~CImapSameServerTransferMsgAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual CImapAction		*Duplicate();
	virtual void			 Copy(CImapAction *pImapAction);

	virtual HRESULT			 DoWork();
};


//
// Subclass of CImapTransferMsgAction to perform cross server (both IMAP) transfer of one or more messages.
//

class CImapCrossServerTransferMsgAction : public CImapTransferMsgAction
{
public:
	CImapCrossServerTransferMsgAction(unsigned long lAccountID,
									  const char *szMailbox,
									  TCHAR cDelimiter,
									  const char *szMessageIDs,
									  unsigned long lDestAccountID,
									  const char *szDestMailbox,
									  TCHAR cDestDelimiter,
									  BOOL bCopy);
	CImapCrossServerTransferMsgAction();
	~CImapCrossServerTransferMsgAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual CImapAction		*Duplicate();
	virtual void			 Copy(CImapAction *pImapAction);

	bool					 DoPreThreadAction();
	virtual HRESULT			 DoWork();
	void					 DoPostProcessing();
};


//
// Subclass of CImapTransferMsgAction to perform IMAP to local transfer of one or more messages.
//

class CImapToLocalTransferMsgAction : public CImapTransferMsgAction
{
public:
	CImapToLocalTransferMsgAction(unsigned long lAccountID,
								  const char *szMailbox,
								  TCHAR cDelimiter,
								  const char *szMessageIDs,
								  const char *szDestMailbox,
								  BOOL bCopy);
	CImapToLocalTransferMsgAction();
	~CImapToLocalTransferMsgAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual CImapAction		*Duplicate();
	virtual void			 Copy(CImapAction *pImapAction);

	bool					 DoPreThreadAction();
	virtual HRESULT			 DoWork();
	void					 DoPostProcessing();
};


//
// Subclass of CImapTransferMsgAction to perform local to IMAP transfer of one or more messages.
//

class CImapFromLocalTransferMsgAction : public CImapTransferMsgAction
{
public:
	CImapFromLocalTransferMsgAction(const char *szMailbox,
									unsigned long lDestAccountID,
									const char *szDestMailbox,
									TCHAR cDestDelimiter,
									const char *szMessageIDs,
									BOOL bCopy);
	CImapFromLocalTransferMsgAction();
	~CImapFromLocalTransferMsgAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual CImapAction		*Duplicate();
	virtual void			 Copy(CImapAction *pImapAction);

	bool					 DoPreThreadAction();
	virtual HRESULT			 DoWork();
	virtual void			 DoPostProcessing();
};


//
// Subclass of CImapAction to perform mailbox creation.
//

class CImapCreateMailboxAction : public CImapAction
{
public:
	CImapCreateMailboxAction(unsigned long lAccountID,
							 const char *szMailbox,
							 TCHAR cDelimiter,
							 CString strLogin,
							 CString strPassword);
	CImapCreateMailboxAction();
	~CImapCreateMailboxAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual CImapAction		*Duplicate();
	virtual void			 Copy(CImapAction *pImapAction);

	virtual bool			 DoPreThreadAction();
	virtual HRESULT			 DoWork();
	virtual void			 DoPostProcessing();
};


//
// Subclass of CImapAction to perform mailbox renaming.
//

class CImapRenameMailboxAction : public CImapAction
{
public:
	CImapRenameMailboxAction(unsigned long lAccountID,
							 const char *szMailbox,
							 TCHAR cDelimiter,
							 LPCSTR szNewName,
							 CString strLogin,
							 CString strPassword);
	CImapRenameMailboxAction();
	~CImapRenameMailboxAction();

	virtual bool			 Write(XMLWriter *xmlWriter);
	virtual CImapAction		*Duplicate();
	virtual void			 Copy(CImapAction *pImapAction);

	virtual bool			 DoPreThreadAction();
	virtual HRESULT			 DoWork();

	char					*m_szNewName;

	void					 SetNewName(const char *szNewName);
};


//
// Subclass of CImapAction to perform mailbox deletion.
//

class CImapDeleteMailboxAction : public CImapAction
{
public:
	CImapDeleteMailboxAction(unsigned long lAccountID,
							 const char *szMailbox,
							 TCHAR cDelimiter,
							 CString strLogin,
							 CString strPassword);
	CImapDeleteMailboxAction();
	~CImapDeleteMailboxAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
};


//	UNUSED
//
// Subclass of CImapAction to perform mailbox list refresh.
//
//	UNUSED

class CImapRefreshMailboxesAction : public CImapAction
{
public:
	CImapRefreshMailboxesAction(unsigned long lAccountID,
								const char *szMailbox,
								TCHAR cDelimiter,
								CString strLogin,
								CString strPassword);
	CImapRefreshMailboxesAction();
	~CImapRefreshMailboxesAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
	virtual void		 DoPostProcessing();
};


//
// Subclass of CImapAction to perform mailbox expunge.
//

class CImapExpungeAction : public CImapAction
{
public:
	CImapExpungeAction(unsigned long lAccountID,
					   const char *szMailbox,
					   TCHAR cDelimiter);
	CImapExpungeAction();
	~CImapExpungeAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
	virtual void		 DoPostProcessing();
};


//
// Subclass of CImapAction to perform mailbox resync.
//

class CImapResyncAction : public CImapAction
{
public:
	CImapResyncAction(unsigned long lAccountID,
					  const char *szMailbox,
					  TCHAR cDelimiter,
					  BOOL bNormalCheck,
					  unsigned long ulBitflags,
					  BOOL m_bUsesNotifier,
					  BOOL bDownloadedOnly);
	CImapResyncAction();
	~CImapResyncAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
	virtual void		 DoPostProcessing();
	bool				 UsesMailbox(unsigned long lMailboxHash);

	BOOL				 m_bNormalCheck;
	unsigned long		 m_ulBitflags;
	BOOL				 m_bUsesNotifier;
	BOOL				 m_bDownloadedOnly;
};


//
// Subclass of CImapAction to perform message downloading.
//

class CImapDownloadAction : public CImapAction
{
public:
	CImapDownloadAction(unsigned long lAccountID,
						const char *szMailbox,
						TCHAR cDelimiter,
						const char *szMessageIDs,
						BOOL bDownloadAttachments,
						BOOL bOnlyIfNotDownloaded,
						BOOL bClearCacheOnly,
						BOOL bInvalidateCachedPreviewSums);
	CImapDownloadAction();
	~CImapDownloadAction();

	virtual bool		 Write(XMLWriter *xmlWriter);
	virtual CImapAction	*Duplicate();
	virtual void		 Copy(CImapAction *pImapAction);

	virtual bool		 DoPreThreadAction();
	virtual HRESULT		 DoWork();
	virtual void		 DoPostProcessing();

	BOOL				 m_bDownloadAttachments;
	BOOL				 m_bOnlyIfNotDownloaded;
	BOOL				 m_bClearCacheOnly;
	BOOL				 m_bInvalidateCachedPreviewSums;

	void				 SetDownloadAttachments() {m_bDownloadAttachments = TRUE;}
	void				 SetOnlyIfNotDownloaded() {m_bOnlyIfNotDownloaded = TRUE;}
	void				 SetClearCacheOnly() {m_bClearCacheOnly = TRUE;}
	void				 SetInvalidateCachedPreviewSums() {m_bInvalidateCachedPreviewSums = TRUE;}
};


//
// Object that manages the queue of actions to be performed.  A single instance of this is created
// for a given instance of Eudora.
//

class CActionQueue : public CObject
{
public:
	CActionQueue(unsigned long lAccountID, CString &strFileName, CString &strDirName);
	~CActionQueue();

	bool			 OnIdle(bool bHighPriorityOnly, bool bOnlineStateChanged);
	bool			 IsProcessing();
	void			 FinishUp();

	void			 Add(CImapAction *pImapAction);
	void			 AddFromFile(CImapAction *pImapAction);
	void			 ReAdd(CImapAction *pImapAction);
	void			 ActionDone();
	int				 GetCount() {return m_array.GetCount();}
	bool			 HasHighPriorityItems();
	CImapAction		*GetNextMailboxMatch(unsigned long ulHashToMatch);

	void			 SetNeedsWrite(bool bNeedsWrite) {m_bNeedsWrite = bNeedsWrite;}
	bool			 NeedsWrite() {return m_bNeedsWrite;}

	bool			 WriteQueue();
	bool			 ReadQueue();

private:
	CPtrArray		 m_array;
	int				 m_iState;
	unsigned long	 m_lAccountID;
	CString			 m_strFileName;
	CString			 m_strDirName;
	bool			 m_bNeedsWrite;
};

#endif // __IMAPACTIONQUEUE_H
