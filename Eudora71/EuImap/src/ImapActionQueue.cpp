// ImapActionQueue.cpp
//
// Implementation of queue for IMAP actions.
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

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "ImapActionQueue.h"

#include "POPSession.h"
#include "QCTaskManager.h"
#include "ImapMailMgr.h"
#include "ImapAccount.h"
#include "ImapAccountMgr.h"
#include "QCMailboxDirector.h"
#include "QCImapMailboxCommand.h"
#include "XMLWriter.h"
#include "xml_parser_base.h"
#include "fileutil.h"


//	Defines and other Constants	=============================
#include "DebugNewHelpers.h"

const int		 kQueueStateIdle				= 0;
const int		 kQueueStateProcessing			= 1;
const int		 kQueueStateSaving				= 2;
const int		 kQueueStateBlocked				= 3;

const int		 kActionStateSucceeded			= 0;
const int		 kActionStateWaiting			= 1;
const int		 kActionStateWorking			= 2;
const int		 kActionStateBusy				= 3;
const int		 kActionStateCancel				= 4;
const int		 kActionStateLoginFailed		= 5;
const int		 kActionStateOffline			= 6;
const int		 kActionStateFailed				= 7;


const int		 kDataFormatVersion = 1;

class CImapActionXMLParser : public XmlParser
{
  public:
	//	Constants
	static const long				kDataFormatVersion = 1;

	static const char *				kXMLBaseContainer;
	static const char *				kKeyDataFormatVersion;
	static const char *				kActionContainer;
	static const char *				kKeyActionType;
	static const char *				kKeyMailbox;
	static const char *				kKeyDelimiter;
	static const char *				kKeyMessages;
	static const char *				kKeyStatus;
	static const char *				kKeyStatusSet;
	static const char *				kKeyDestAccount;
	static const char *				kKeyDestMailbox;
	static const char *				kKeyDestDelimiter;
	static const char *				kKeyNewMailboxName;
	static const char *				kKeyDownloadAttachments;
	static const char *				kKeyOnlyIfNotDownloaded;
	static const char *				kKeyClearCacheOnly;
	static const char *				kKeyInvalidateCachedPreviewSums;
	static const char *				kKeyNormalCheck;
	static const char *				kKeyBitflags;
	static const char *				kKeyUsesNotifier;
	static const char *				kKeyDownloadedOnly;

									CImapActionXMLParser(CActionQueue *pActionQueue);

  private:

	enum XMLActionElementIDs
	{
		id_baseContainer,
		id_keyDataFormatVersion,
		id_keyContainer,
		id_keyType,
		id_keyMailbox,
		id_keyDelimiter,
		id_keyMessageIDs,
		id_keyStatus,
		id_keyStatusSet,
		id_keyDestAccount,
		id_keyDestMailbox,
		id_keyDestDelimiter,
		id_keyNewMbxName,
		id_keyDownloadAttachments,
		id_keyOnlyIfNotDownloaded,
		id_keyClearCacheOnly,
		id_keyInvalidateCachedPreviewSums,
		id_keyNormalCheck,
		id_keyBitflags,
		id_keyUsesNotifier,
		id_keyDownloadedOnly
	};

	static ElementMap				elementMapArr[];

	bool							initElementMap(
										ElementMap **			out_pMap);
	void							handleData(
										int						in_nID,
										const char *			in_pData,
										int						in_nDataLength);
	void							ProcessDataForID(
										int						in_nID);
	int								startElement(
										int						in_nID,
										const char *			in_szName,
										const char **			in_AttributeArr);
	int								endElement(
										int						in_nID,
										const char *			in_szName);

	deque<int>						m_elementIDsQueue;
	CString							m_szElementData;

	CActionQueue					*m_pActionQueue;
	CImapAction						*m_pImapAction;
};


const char		*CImapActionXMLParser::kXMLBaseContainer = "IMAPQueueData";
const char		*CImapActionXMLParser::kKeyDataFormatVersion = "DataFormatVersion";
const char		*CImapActionXMLParser::kActionContainer = "Action";
const char		*CImapActionXMLParser::kKeyActionType = "ActionType";
const char		*CImapActionXMLParser::kKeyMailbox = "Mailbox";
const char		*CImapActionXMLParser::kKeyDelimiter = "Delimiter";
const char		*CImapActionXMLParser::kKeyMessages = "Messages";
const char		*CImapActionXMLParser::kKeyStatus = "Status";
const char		*CImapActionXMLParser::kKeyStatusSet = "StatusSet";
const char		*CImapActionXMLParser::kKeyDestAccount = "DestAccount";
const char		*CImapActionXMLParser::kKeyDestMailbox = "DestMailbox";
const char		*CImapActionXMLParser::kKeyDestDelimiter = "DestDelimiter";
const char		*CImapActionXMLParser::kKeyNewMailboxName = "NewName";
const char		*CImapActionXMLParser::kKeyDownloadAttachments = "Attach";
const char		*CImapActionXMLParser::kKeyOnlyIfNotDownloaded = "OnlyIfNot";
const char		*CImapActionXMLParser::kKeyClearCacheOnly = "ClearCache";
const char		*CImapActionXMLParser::kKeyInvalidateCachedPreviewSums = "InvalidateCache";
const char		*CImapActionXMLParser::kKeyNormalCheck = "NormalCheck";
const char		*CImapActionXMLParser::kKeyBitflags = "BitFlags";
const char		*CImapActionXMLParser::kKeyUsesNotifier = "Notifier";
const char		*CImapActionXMLParser::kKeyDownloadedOnly = "DownloadOnly";

const char		*kDataActionTypeStatus = "status";
const char		*kDataActionTypeDelete = "delete";
const char		*kDataActionTypeUnDelete = "undelete";
const char		*kDataActionTypeTransferSame = "transfersame";
const char		*kDataActionTypeCopySame = "copysame";
const char		*kDataActionTypeTransferCross = "transfercross";
const char		*kDataActionTypeCopyCross = "copycross";
const char		*kDataActionTypeTransferToLocal = "transfertolocal";
const char		*kDataActionTypeCopyToLocal = "copytolocal";
const char		*kDataActionTypeTransferFromLocal = "transferfromlocal";
const char		*kDataActionTypeCopyFromLocal = "copyfromlocal";
const char		*kDataActionTypeCreateMbx = "creatembx";
const char		*kDataActionTypeRenameMbx = "renamembx";
const char		*kDataActionTypeDeleteMbx = "deletembx";
const char		*kDataActionTypeRefresh = "refresh";
const char		*kDataActionTypeExpunge = "expunge";
const char		*kDataActionTypeResync = "resync";
const char		*kDataActionTypeDownload = "download";


//	Static element map
ElementMap CImapActionXMLParser::elementMapArr[] =
{
	id_baseContainer,					const_cast<char *>(kXMLBaseContainer),
	id_keyDataFormatVersion,			const_cast<char *>(kKeyDataFormatVersion),
	id_keyContainer,					const_cast<char *>(kActionContainer),
	id_keyType,							const_cast<char *>(kKeyActionType),
	id_keyMailbox,						const_cast<char *>(kKeyMailbox),
	id_keyDelimiter,					const_cast<char *>(kKeyDelimiter),
	id_keyMessageIDs,					const_cast<char *>(kKeyMessages),
	id_keyStatus,						const_cast<char *>(kKeyStatus),
	id_keyStatusSet,					const_cast<char *>(kKeyStatusSet),
	id_keyDestAccount,					const_cast<char *>(kKeyDestAccount),
	id_keyDestMailbox,					const_cast<char *>(kKeyDestMailbox),
	id_keyDestDelimiter,				const_cast<char *>(kKeyDestDelimiter),
	id_keyNewMbxName,					const_cast<char *>(kKeyNewMailboxName),
	id_keyDownloadAttachments,			const_cast<char *>(kKeyDownloadAttachments),
	id_keyOnlyIfNotDownloaded,			const_cast<char *>(kKeyOnlyIfNotDownloaded),
	id_keyClearCacheOnly,				const_cast<char *>(kKeyClearCacheOnly),
	id_keyInvalidateCachedPreviewSums,	const_cast<char *>(kKeyInvalidateCachedPreviewSums),
	id_keyNormalCheck,					const_cast<char *>(kKeyNormalCheck),
	id_keyBitflags,						const_cast<char *>(kKeyBitflags),
	id_keyUsesNotifier,					const_cast<char *>(kKeyUsesNotifier),
	id_keyDownloadedOnly,				const_cast<char *>(kKeyDownloadedOnly),
	id_none,							"Always the last one"
};




CImapActionXMLParser::CImapActionXMLParser(CActionQueue *pActionQueue)
	:	XmlParser(), m_elementIDsQueue(), m_szElementData(),
	m_pActionQueue(pActionQueue),
	m_pImapAction(NULL)
{

}


// ------------------------------------------------------------------------------------------
//		* XMLParser::initElementMap												   [Private]
// ------------------------------------------------------------------------------------------
//	Inits the element map for the base class XmlParser to use.
//
//	Parameters:
//		out_pMap:			Place to put our pre-existing element map

//	Returns:
//		true for success every time - providing the pre-existing element map is easy.

bool
CImapActionXMLParser::initElementMap(
	ElementMap **			out_pMap)
{
	*out_pMap = elementMapArr;
	
	return true;
}


// ------------------------------------------------------------------------------------------
//		* XMLParser::handleData													   [Private]
// ------------------------------------------------------------------------------------------
//	Accumulates data for use in endElement.
//
//	Parameters:
//		in_nID:				ID for current element
//		in_pData:			Pointer to data to accumulate
//		in_nDataLength:		Length of data to accumulate

void
CImapActionXMLParser::handleData(
	int						in_nID,
	const char *			in_pData,
	int						in_nDataLength)
{
	int		nNewDataLength = m_szElementData.GetLength() + in_nDataLength;
	
	char *	pElementData = m_szElementData.GetBuffer(nNewDataLength);
	
	strncat(pElementData, in_pData, in_nDataLength);

	m_szElementData.ReleaseBuffer(nNewDataLength);
}


// ------------------------------------------------------------------------------------------
//		* XMLParser::ProcessDataForID											   [Private]
// ------------------------------------------------------------------------------------------
//	Processes accumulated data for a given ID.
//
//	Parameters:
//		in_nID:				ID for element to process

void
CImapActionXMLParser::ProcessDataForID(
	int						in_nID)
{
	switch (in_nID)
	{
		case id_keyDataFormatVersion:
			{
				//	atol will return 0 if there's an error parsing the data.
				//	That's why we started our data version numbering with 1.

				//	We don't currently use the data format version, and we may never
				//	need to (part of the point of XML is to avoid the need for this
				//	sort of thing). That said it never hurts to allow for future needs.
				//	
				//	Uncomment if information is needed in the future.
				// long		nDataFormatVersion = atol(m_szElementData);
			}
			break;
	
		case id_keyContainer:
			{
			}
			break;
	
		case id_keyType:
			{
				if (strcmp(m_szElementData, kDataActionTypeStatus) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapChangeMsgStatusAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeDelete) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapDeleteMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeUnDelete) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapUnDeleteMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeTransferSame) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapSameServerTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeCopySame) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapSameServerTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeTransferCross) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapCrossServerTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeCopyCross) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapCrossServerTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeTransferToLocal) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapToLocalTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeCopyToLocal) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapToLocalTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeTransferFromLocal) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapFromLocalTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeCopyFromLocal) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapFromLocalTransferMsgAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeCreateMbx) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapCreateMailboxAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeRenameMbx) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapRenameMailboxAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeDeleteMbx) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapDeleteMailboxAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeRefresh) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapRefreshMailboxesAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeExpunge) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapExpungeAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeResync) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapResyncAction();
				}
				else if (strcmp(m_szElementData, kDataActionTypeDownload) == 0)
				{
					m_pImapAction = DEBUG_NEW_NOTHROW CImapDownloadAction();
				}
			}
			break;
	
		case id_keyMailbox:
			{
				m_pImapAction->SetMailbox(m_szElementData);
			}
			break;
	
		case id_keyDelimiter:
			{
				m_pImapAction->SetDelimiter(m_szElementData[0]);
			}
			break;
	
		case id_keyMessageIDs:
			{
				m_pImapAction->SetMessageIDs(m_szElementData);
			}
			break;
	
		case id_keyStatus:
			{
				((CImapChangeMsgStatusAction*)m_pImapAction)->SetState((char)(atoi(m_szElementData)));
			}
			break;
	
		case id_keyStatusSet:
			{
				((CImapChangeMsgStatusAction*)m_pImapAction)->SetStateSet(m_szElementData[0] == '0' ? FALSE : TRUE);
			}
			break;

		case id_keyDestAccount:
			{
				((CImapTransferMsgAction*)m_pImapAction)->SetDestAccountID(atol(m_szElementData));
			}
			break;

		case id_keyDestMailbox:
			{
				((CImapTransferMsgAction*)m_pImapAction)->SetDestMailbox(m_szElementData);
			}
			break;

		case id_keyDestDelimiter:
			{
				((CImapTransferMsgAction*)m_pImapAction)->SetDestDelimiter(m_szElementData[0]);
			}
			break;

		case id_keyNewMbxName:
			{
				((CImapRenameMailboxAction*)m_pImapAction)->SetNewName(m_szElementData);
			}
			break;

		case id_keyDownloadAttachments:
			{
				if (m_szElementData[0] == '1')
				{
					((CImapDownloadAction*)m_pImapAction)->SetDownloadAttachments();
				}
			}
			break;


		case id_keyOnlyIfNotDownloaded:
			{
				if (m_szElementData[0] == '1')
				{
					((CImapDownloadAction*)m_pImapAction)->SetOnlyIfNotDownloaded();
				}
			}
			break;

		case id_keyClearCacheOnly:
			{
				if (m_szElementData[0] == '1')
				{
					((CImapDownloadAction*)m_pImapAction)->SetClearCacheOnly();
				}
			}
			break;

		case id_keyInvalidateCachedPreviewSums:
			{
				if (m_szElementData[0] == '1')
				{
					((CImapDownloadAction*)m_pImapAction)->SetInvalidateCachedPreviewSums();
				}
			}
			break;
	}
	
	m_szElementData.Empty();
}


// ------------------------------------------------------------------------------------------
//		* XMLParser::startElement												   [Private]
// ------------------------------------------------------------------------------------------
//	Called when we hit the start tag for a given element.
//
//	Parameters:
//		in_nID:				ID for current element
//		in_szName:			Name of element
//		in_AttributeArr:	Attributes found inside the start tag

int
CImapActionXMLParser::startElement(
	int						in_nID,
	const char *			in_szName,
	const char **			in_AttributeArr)
{
	if ( !m_szElementData.IsEmpty() )
	{
		//	We have some data accumulated from inside an XML element, but now we're
		//	starting a new element - so process the data now.
		ProcessDataForID( m_elementIDsQueue.front() );
	}

	m_elementIDsQueue.push_front(in_nID);
	
	return 0;
}


// ------------------------------------------------------------------------------------------
//		* XMLParser::endElement													   [Private]
// ------------------------------------------------------------------------------------------
//	Called when we hit the end tag for a given element.
//
//	Parameters:
//		in_nID:				ID for current element
//		in_szName:			Name of element

int
CImapActionXMLParser::endElement(
	int						in_nID,
	const char *			in_szName)
{
	if ( !m_szElementData.IsEmpty() )
	{
		//	We have some data accumulated from inside an XML element, but now we're
		//	ending the current element - so process the data now.
		ProcessDataForID(in_nID);
	}

	if (in_nID == id_keyContainer)
	{
		ASSERT(m_pImapAction);
		if (m_pImapAction)
		{
			m_pActionQueue->AddFromFile(m_pImapAction);
			m_pImapAction = NULL;
		}
	}

	m_elementIDsQueue.pop_back();
	
	return 0;
}


extern QCMailboxDirector g_theMailboxDirector;

static void DoPostProcessing(void *pv, bool bLastTime);

void DoPostProcessing(void *pv, bool bLastTime)
{
	if (pv)
	{
		((CImapAction*)pv)->DoPostProcessing();
	}
}


CImapAction::CImapAction(unsigned long lAccountID,
						 const char *szMailbox,
						 TCHAR cDelimiter,
						 const char *szMessageIDs,
						 CString strLogin,
						 CString strPassword) :
	m_iPriority(kImapActionPriorityNormal),
	m_lAccountID(lAccountID),
	m_szMailbox(NULL),
	m_cDelimiter(cDelimiter),
	m_szMessageIDs(NULL),
	m_lMailboxHash(0),
	m_strLogin(strLogin),
	m_strPassword(strPassword),
	m_pAccount(NULL),
	m_pCommand(NULL),
	m_pTocDoc(NULL),
	m_pImapMailbox(NULL),
	m_pActionQueue(NULL),
	m_bIsLastOnMailbox(false),
	m_iState(kActionStateWaiting)
{
	if (szMailbox && *szMailbox)
	{
		// Duplicate the message ID list string because the source string is almost certainly a local.
		m_szMailbox = strdup(szMailbox);
		m_lMailboxHash = HashMT(m_szMailbox);
	}
		
	if (szMessageIDs && *szMessageIDs)
	{
		// Duplicate the message ID list string because the source string is almost certainly a local.
		m_szMessageIDs = strdup(szMessageIDs);
	}
}

CImapAction::CImapAction() :
	m_iPriority(kImapActionPriorityNormal),
	m_lAccountID(0),
	m_szMailbox(NULL),
	m_cDelimiter('\0'),
	m_szMessageIDs(NULL),
	m_lMailboxHash(0),
	m_strLogin(""),
	m_strPassword(""),
	m_pAccount(NULL),
	m_pCommand(NULL),
	m_pTocDoc(NULL),
	m_pImapMailbox(NULL),
	m_pActionQueue(NULL),
	m_bIsLastOnMailbox(false),
	m_iState(kActionStateWaiting)
{
}

CImapAction::~CImapAction()
{
	if (m_iState == kActionStateOffline)
	{
		CImapAction		*pImapAction = Duplicate();
		m_pActionQueue->ReAdd(pImapAction);
	}
	else
	{
		m_pActionQueue->ActionDone();
	}

	if (m_szMailbox)
	{
		free(m_szMailbox);
	}
	if (m_szMessageIDs)
	{
		free(m_szMessageIDs);
	}
}

//
//	CImapAction::DoPreThreadAction()
//
//	Do work in main thread to prepare for DoWork() in the secondary thread.
//
bool CImapAction::DoPreThreadAction()
{
	m_iState = kActionStateWorking;

	m_pAccount = g_ImapAccountMgr.FindAccount(m_lAccountID);
	if (!m_pAccount)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetPersona(m_pAccount->GetPersona());

	return true;
}

HRESULT CImapAction::DoWork()
{
	m_iState = kActionStateFailed;
	return E_FAIL;
}

void CImapAction::DoPostProcessing()
{
}

void CImapAction::RequestThreadStop()
{
}

//
//	CImapAction::Write()
//
//	Write the data common to all actions.
//
bool CImapAction::Write(XMLWriter *xmlWriter)
{
	if (xmlWriter)
	{
		//	Write Mailbox tag
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyMailbox, true, "%s", m_szMailbox);

		//	Write Delimiter tag
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyDelimiter, true, "%c", m_cDelimiter);

		if (m_szMessageIDs && *m_szMessageIDs)
		{
			//	Write Messages tag
			xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyMessages, true, "%s", m_szMessageIDs);
		}

		return true;
	}
	return false;
}


//
//	CImapAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapAction::Duplicate()
{
	ASSERT(0);
	return NULL;
}

//
//	CImapAction::Copy()
//
//	Copy the relevant data fields from the specified object.
//
void CImapAction::Copy(CImapAction *pImapAction)
{
	if (pImapAction)
	{
		m_iPriority = pImapAction->m_iPriority;
		m_lAccountID = pImapAction->m_lAccountID;
		m_szMailbox = strdup(pImapAction->m_szMailbox);
		m_cDelimiter = pImapAction->m_cDelimiter;
		m_szMessageIDs = strdup(pImapAction->m_szMessageIDs);
		m_lMailboxHash = pImapAction->m_lMailboxHash;
		m_strLogin = pImapAction->m_strLogin;
		m_strPassword = pImapAction->m_strPassword;
		m_pActionQueue = pImapAction->m_pActionQueue;
		m_iState = pImapAction->m_iState;
	}
}

void CImapAction::SetAccountID(unsigned long lAccountID)
{
	m_lAccountID = lAccountID;
}

void CImapAction::SetMailbox(const char *szMailbox)
{
	if (szMailbox && *szMailbox)
	{
		m_szMailbox = strdup(szMailbox);
		m_lMailboxHash = HashMT(m_szMailbox);
	}
}

void CImapAction::SetDelimiter(char cDelimiter)
{
	m_cDelimiter = cDelimiter;
}

void CImapAction::SetMessageIDs(const char *szMessageIDs)
{
	if (szMessageIDs && *szMessageIDs)
	{
		m_szMessageIDs = strdup(szMessageIDs);
	}
}

//
//	CImapTransferMsgAction::GetSummaryFromUIDString()
//
//	Extracts the first message ID from the specified string and returns a pointer to the summary
//	which matches that ID.
//
CSummary *CImapAction::GetSummaryFromUIDString(CString &strUIDList)
{
	if (!m_pTocDoc || strUIDList.IsEmpty())
	{
		return NULL;
	}

	CSummary	*pSum = NULL;
	CString		 strUID;
	int			 iPos = -1;

	iPos = strUIDList.Find(",");
	if (iPos > -1)
	{
		strUID = strUIDList.Left(iPos);
		strUIDList = strUIDList.Right(strUIDList.GetLength() - iPos - 1);
	}
	else
	{
		strUID = strUIDList;
		strUIDList = "";
	}

	pSum = m_pTocDoc->GetSummaryFromUID(atoi(strUID));

	return pSum;
}

//
//	CImapAction::UsesMailbox()
//
//	Returns true if this item uses the specified mailbox.
//
bool CImapAction::UsesMailbox(unsigned long lMailboxHash)
{
	return (m_lMailboxHash == lMailboxHash);
}

//
//	CImapAction::OpenMailbox()
//
//	Calls CImapMailbox on the specified mailbox and sets the appropriate state for the given result code.
//
bool CImapAction::OpenMailbox(CImapMailbox *pImapMailbox)
{
	if (!pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	HRESULT		 hResult = pImapMailbox->OpenMailbox(TRUE/*bSilent*/);

	if (hResult == S_OK)
	{
		return true;
	}
	else if (HRESULT_CONTAINS_LOGON_FAILURE(hResult))
	{
		m_iState = kActionStateLoginFailed;
	}
	else if (HRESULT_CONTAINS_OFFLINE(hResult))
	{
		m_iState = kActionStateOffline;
	}
	else if (HRESULT_CONTAINS_CANCEL(hResult))
	{
		m_iState = kActionStateCancel;
	}
	else if (HRESULT_CONTAINS_BUSY(hResult))
	{
		m_iState = kActionStateBusy;
	}
	else if (hResult == E_FAIL)
	{
		// A result code of E_FAIL means that the connection couldn't be made.  While technically not
		// offline for now we treat these two failures identically.
		m_iState = kActionStateOffline;
	}

	return false;
}

//
//	CImapChangeMsgStatusAction
//

CImapChangeMsgStatusAction::CImapChangeMsgStatusAction(unsigned long lAccountID,
													   const char *szMailbox,
													   TCHAR cDelimiter,
													   const char *szMessageIDs,
													   char cState,
													   BOOL bSet) : 
	CImapAction(lAccountID, szMailbox, cDelimiter, szMessageIDs, "", ""),
	m_cState(cState),
	m_bSet(bSet)
{
}

CImapChangeMsgStatusAction::CImapChangeMsgStatusAction() : 
	CImapAction(),
	m_cState('\0'),
	m_bSet(false)
{
}

CImapChangeMsgStatusAction::~CImapChangeMsgStatusAction()
{
}

//
//	CImapChangeMsgStatusAction::Write()
//
//	Write the message status change data.
//
bool CImapChangeMsgStatusAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeStatus);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	//	Write Status tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyStatus, true, "%d", m_cState);

	//	Write StatusSet tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyStatusSet, true, "%d", m_bSet ? 1 : 0);

	return true;
}

//
//	CImapChangeMsgStatusAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapChangeMsgStatusAction::Duplicate()
{
	CImapChangeMsgStatusAction		*pImapAction = DEBUG_NEW CImapChangeMsgStatusAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapChangeMsgStatusAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapChangeMsgStatusAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);

	if (pImapAction)
	{
		CImapChangeMsgStatusAction	*pImapChangeStatusAction = (CImapChangeMsgStatusAction*)pImapAction;

		m_cState = pImapChangeStatusAction->m_cState;
		m_bSet = pImapChangeStatusAction->m_bSet;
	}
}

//
//	CImapChangeMsgStatusAction::DoPreThreadAction()
//
//	Prepare to perform the online status change of the specified messages.
//
bool CImapChangeMsgStatusAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		CTocDoc			*pTocDoc = GetToc(m_pCommand->GetPathname());
		if (pTocDoc)
		{
			m_pImapMailbox = pTocDoc->m_pImapMailbox;
		}
	}

	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (!OpenMailbox(m_pImapMailbox))
	{
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	m_pTaskInfo->SetTitle(CRString(IDS_IMAP_UPDATING_STATUS));

	return true;
}

//
//	CImapChangeMsgStatusAction::DoWork()
//
//	Perform the online status change of the specified messages.
//
HRESULT CImapChangeMsgStatusAction::DoWork()
{
	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	CString		 strMessageIDs(m_szMessageIDs);
	if (m_pImapMailbox->SetRemoteState(strMessageIDs, m_cState, !m_bSet))
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}

void CImapChangeMsgStatusAction::SetState(char cState)
{
	m_cState = cState;
}

void CImapChangeMsgStatusAction::SetStateSet(BOOL bSet)
{
	m_bSet = bSet;
}


//
//	CImapDeleteMsgAction
//

CImapDeleteMsgAction::CImapDeleteMsgAction(unsigned long lAccountID,
										   const char *szMailbox,
										   TCHAR cDelimiter,
										   const char *szMessageIDs) : 
	CImapAction(lAccountID, szMailbox, cDelimiter, szMessageIDs, "", ""),
	m_bAutoExpunge(FALSE)
{
}

CImapDeleteMsgAction::CImapDeleteMsgAction() : 
	CImapAction(),
	m_bAutoExpunge(FALSE)
{
}

CImapDeleteMsgAction::~CImapDeleteMsgAction()
{
}

//
//	CImapDeleteMsgAction::Write()
//
//	Write the message delete data.
//
bool CImapDeleteMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeDelete);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapDeleteMsgAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapDeleteMsgAction::Duplicate()
{
	CImapDeleteMsgAction		*pImapAction = DEBUG_NEW CImapDeleteMsgAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapDeleteMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapDeleteMsgAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapDeleteMsgAction::DoPreThreadAction()
//
//	Prepare to perform the online deletion of the specified messages.
//
bool CImapDeleteMsgAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		m_pTocDoc = GetToc(m_pCommand->GetPathname());
		if (m_pTocDoc)
		{
			m_pImapMailbox = m_pTocDoc->m_pImapMailbox;
		}
	}

	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (!OpenMailbox(m_pImapMailbox))
	{
		return false;
	}

	m_bAutoExpunge = m_pImapMailbox->ReadyToAutoExpunge();

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	CRString		 strTitle(IDS_IMAP_DELETING_MSGS);
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}
	
//
//	CImapDeleteMsgAction::DoWork()
//
//	Perform the online deletion of the specified messages.
//
HRESULT CImapDeleteMsgAction::DoWork()
{
	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pActionQueue)
	{
		m_bIsLastOnMailbox = (m_pActionQueue->GetNextMailboxMatch(m_lMailboxHash) == NULL);
	}

	if (m_pImapMailbox->DeleteMessagesFromServer(m_szMessageIDs, m_bAutoExpunge, TRUE/*bSilent*/) == S_OK)
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}

//
//	CImapDeleteMsgAction::DoPostProcessing()
//
//	Update the contents of the view if we expunged any messages.
//
void CImapDeleteMsgAction::DoPostProcessing()
{
	if (m_bAutoExpunge && m_bIsLastOnMailbox && m_pTocDoc && m_pImapMailbox)
	{
		m_pImapMailbox->DoManualResync(m_pTocDoc, TRUE, FALSE, FALSE);

		m_pTocDoc->Write();
	}
}


//
//	CImapDeleteMsgAction
//

CImapUnDeleteMsgAction::CImapUnDeleteMsgAction(unsigned long lAccountID,
											   const char *szMailbox,
											   TCHAR cDelimiter,
											   const char *szMessageIDs) : 
	CImapAction(lAccountID, szMailbox, cDelimiter, szMessageIDs, "", "")
{
}

CImapUnDeleteMsgAction::CImapUnDeleteMsgAction() : 
	CImapAction()
{
}

CImapUnDeleteMsgAction::~CImapUnDeleteMsgAction()
{
}

//
//	CImapUnDeleteMsgAction::Write()
//
//	Write the message undelete data.
//
bool CImapUnDeleteMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeUnDelete);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapUnDeleteMsgAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapUnDeleteMsgAction::Duplicate()
{
	CImapUnDeleteMsgAction		*pImapAction = DEBUG_NEW CImapUnDeleteMsgAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapUnDeleteMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapUnDeleteMsgAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapUnDeleteMsgAction::DoPreThreadAction()
//
//	Prepare to perform the online undeletion of the specified messages.
//
bool CImapUnDeleteMsgAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		CTocDoc			*pTocDoc = GetToc(m_pCommand->GetPathname());
		if (pTocDoc)
		{
			m_pImapMailbox = pTocDoc->m_pImapMailbox;
		}
	}

	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (!OpenMailbox(m_pImapMailbox))
	{
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	m_pTaskInfo->SetTitle(CRString(IDS_IMAP_UNDELETING_MSGS));

	return true;
}
	
//
//	CImapUnDeleteMsgAction::DoWork()
//
//	Perform the online undeletion of the specified messages.
//
HRESULT CImapUnDeleteMsgAction::DoWork()
{
	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pImapMailbox->UnDeleteMessagesFromServer(m_szMessageIDs, TRUE/*bSilent*/) == S_OK)
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}


//
//	CImapTransferMsgAction
//

CImapTransferMsgAction::CImapTransferMsgAction(unsigned long lAccountID,
											   const char *szMailbox,
											   TCHAR cDelimiter,
											   const char *szMessageIDs,
											   unsigned long lDestAccountID,
											   const char *szDestMailbox,
											   TCHAR cDestDelimiter,
											   BOOL bCopy) : 
	CImapAction(lAccountID, szMailbox, cDelimiter, szMessageIDs, "", ""),
	m_lDestAccountID(lDestAccountID),
	m_szDestMailbox(NULL),
	m_cDestDelimiter(cDestDelimiter),
	m_lDestMailboxHash(0),
	m_bCopy(bCopy),
	m_bAutoExpunge(FALSE),
	m_pDestCommand(NULL),
	m_pDestTocDoc(NULL)
{
	if (szDestMailbox && *szDestMailbox)
	{
		m_szDestMailbox = strdup(szDestMailbox);
		m_lDestMailboxHash = HashMT(m_szDestMailbox);
	}
}

CImapTransferMsgAction::CImapTransferMsgAction() : 
	CImapAction(),
	m_lDestAccountID(0),
	m_szDestMailbox(NULL),
	m_cDestDelimiter('\0'),
	m_lDestMailboxHash(0),
	m_bCopy(false),
	m_bAutoExpunge(FALSE),
	m_pDestCommand(NULL),
	m_pDestTocDoc(NULL)
{
}

CImapTransferMsgAction::~CImapTransferMsgAction()
{
	if (m_szDestMailbox)
	{
		free(m_szDestMailbox);
	}
}

//
//	CImapTransferMsgAction::Write()
//
//	Write the message transfer data.
//
bool CImapTransferMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	//	Write the destination account tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyDestAccount, true, "%d", m_lDestAccountID);

	//	Write the destination mailbox tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyDestMailbox, true, "%s", m_szDestMailbox);

	//	Write the destination delimiter tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyDestDelimiter, true, "%c", m_cDestDelimiter);

	return true;
}

//
//	CImapTransferMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapTransferMsgAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapTransferMsgAction::UsesMailbox()
//
//	Returns true if this item uses the specified mailbox.
//
bool CImapTransferMsgAction::UsesMailbox(unsigned long lMailboxHash)
{
	return ((m_lMailboxHash == lMailboxHash) || (m_lDestMailboxHash == lMailboxHash));
}

//
//	CImapTransferMsgAction::DoPreThreadAction()
//
//	Prepare to perform the online transfer of the specified messages.
//
bool CImapTransferMsgAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (!m_pCommand)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	m_pDestCommand = g_theMailboxDirector.ImapFindByImapName(m_lDestAccountID,
															 m_szDestMailbox,
															 m_cDestDelimiter);

	if (!m_pDestCommand)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (m_pCommand)
	{
		m_pTocDoc = GetToc(m_pCommand->GetPathname());
		if (m_pTocDoc)
		{
			m_pImapMailbox = m_pTocDoc->m_pImapMailbox;
		}
	}
	
	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (!OpenMailbox(m_pImapMailbox))
	{
		return false;
	}

	if (m_pDestCommand)
	{
		m_pDestTocDoc = GetToc(m_pDestCommand->GetPathname());
	}

	if (!m_pDestTocDoc || !m_pDestTocDoc->m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (!OpenMailbox(m_pDestTocDoc->m_pImapMailbox))
	{
		return false;
	}

	if (!m_bCopy && m_bIsLastOnMailbox)
	{
		m_bAutoExpunge = m_pImapMailbox->ReadyToAutoExpunge();
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	CString			strTitle;
	if (m_bCopy)
	{
		strTitle = CRString(IDS_IMAP_COPYING);
	}
	else
	{
		strTitle = CRString(IDS_IMAP_TRANSFERING);
	}
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapTransferMsgAction::DoPostProcessing()
//
//	Use the UIDPLUS data to update the summary and then queue a resync to refresh the local copy of
//	the mailbox.
//
void CImapTransferMsgAction::DoPostProcessing()
{
	if (m_pTocDoc)
	{
		// Use the UIDPLUS data to move summary data between the TOC's.
		CString		 strMessageIDs(m_szMessageIDs);
		m_pTocDoc->ImapTransferSummaryData(strMessageIDs,
										   m_pDestTocDoc,
										   &m_dwaNewUIDs,
										   NULL/*pImapSum*/,
										   true/*bFindNew*/);
	}

	// Resync the target mailbox.
	// Only resync on the last item in the queue that acts on this mailbox, otherwise
	// we might enter a state where more than one message is transferred to the same mailbox via
	// different operations, then when the first transfer is performed online we sync and lose
	// any local data placed by the later transfers.
	if (m_bIsLastOnMailbox && m_pDestTocDoc && m_pDestTocDoc->m_pImapMailbox)
	{
		m_pDestTocDoc->m_pImapMailbox->DoManualResync(m_pDestTocDoc, TRUE, FALSE, FALSE);
	}
}

void CImapTransferMsgAction::SetDestAccountID(unsigned long lDestAccountID)
{
	m_lDestAccountID = lDestAccountID;
}

void CImapTransferMsgAction::SetDestMailbox(const char *szDestMailbox)
{
	if (szDestMailbox && *szDestMailbox)
	{
		m_szDestMailbox = strdup(szDestMailbox);
		m_lDestMailboxHash = HashMT(m_szDestMailbox);
	}
}

void CImapTransferMsgAction::SetDestDelimiter(char cDestDelimiter)
{
	m_cDestDelimiter = cDestDelimiter;
}

void CImapTransferMsgAction::SetCopy(BOOL bCopy)
{
	m_bCopy = bCopy;
}

//
//	CImapTransferMsgAction::GetSummaryFromUIDStringFromDestination()
//
//	Extracts the first message ID from the specified string and returns a pointer to the summary
//	which matches that ID in the destination TOC.
//
CSummary *CImapTransferMsgAction::GetSummaryFromUIDStringFromDestination(CString &strUIDList)
{
	if (!m_pDestTocDoc || strUIDList.IsEmpty())
	{
		return NULL;
	}

	CSummary	*pSum = NULL;
	CString		 strUID;
	int			 iPos = -1;

	iPos = strUIDList.Find(",");
	if (iPos > -1)
	{
		strUID = strUIDList.Left(iPos);
		strUIDList = strUIDList.Right(strUIDList.GetLength() - iPos - 1);
	}
	else
	{
		strUID = strUIDList;
		strUIDList = "";
	}

	pSum = m_pDestTocDoc->GetSummaryFromUID(atoi(strUID));

	return pSum;
}


//
//	CImapSameServerTransferMsgAction
//

CImapSameServerTransferMsgAction::CImapSameServerTransferMsgAction(unsigned long lAccountID,
																   const char *szMailbox,
																   TCHAR cDelimiter,
																   const char *szMessageIDs,
																   unsigned long lDestAccountID,
																   const char *szDestMailbox,
																   TCHAR cDestDelimiter,
																   BOOL bCopy) : 
	CImapTransferMsgAction(lAccountID, szMailbox, cDelimiter, szMessageIDs,
						   lDestAccountID, szDestMailbox, cDestDelimiter, bCopy)
{
}

CImapSameServerTransferMsgAction::CImapSameServerTransferMsgAction() : 
	CImapTransferMsgAction()
{
}

CImapSameServerTransferMsgAction::~CImapSameServerTransferMsgAction()
{
}

//
//	CImapSameServerTransferMsgAction::Write()
//
//	Write the message transfer data.
//
bool CImapSameServerTransferMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	if (m_bCopy)
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeCopySame);
	}
	else
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeTransferSame);
	}

	//	Write the tags common to all transfer actions
	CImapTransferMsgAction::Write(xmlWriter);

	return true;
}

//
//	CImapSameServerTransferMsgAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapSameServerTransferMsgAction::Duplicate()
{
	CImapSameServerTransferMsgAction		*pImapAction = DEBUG_NEW CImapSameServerTransferMsgAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapSameServerTransferMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapSameServerTransferMsgAction::Copy(CImapAction *pImapAction)
{
	CImapTransferMsgAction::Copy(pImapAction);
}

//
//	CImapSameServerTransferMsgAction::DoWork()
//
//	Perform the online transfer of the specified messages.
//
HRESULT CImapSameServerTransferMsgAction::DoWork()
{
	if (!m_pTocDoc || !m_pImapMailbox || !m_pDestTocDoc || !m_pDestTocDoc->m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pActionQueue)
	{
		m_bIsLastOnMailbox = (m_pActionQueue->GetNextMailboxMatch(m_lDestMailboxHash) == NULL);
	}

	// Now transfer the messages on the server.
	HRESULT		 hResult = m_pImapMailbox->XferMessagesOnServer(m_szMessageIDs,
																&m_dwaNewUIDs,
																m_pDestTocDoc->m_pImapMailbox->GetImapName(),
																TRUE/*Copy*/,
																TRUE/*bSilent*/);

	if ((hResult == S_OK) && !m_bCopy)
	{
		m_pImapMailbox->DeleteMessagesFromServer(m_szMessageIDs, m_bAutoExpunge, TRUE/*bSilent*/);
	}

	if (hResult == S_OK)
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}


//
//	CImapCrossServerTransferMsgAction
//

CImapCrossServerTransferMsgAction::CImapCrossServerTransferMsgAction(unsigned long lAccountID,
																	 const char *szMailbox,
																	 TCHAR cDelimiter,
																	 const char *szMessageIDs,
																	 unsigned long lDestAccountID,
																	 const char *szDestMailbox,
																	 TCHAR cDestDelimiter,
																	 BOOL bCopy) : 
	CImapTransferMsgAction(lAccountID, szMailbox, cDelimiter, szMessageIDs,
						   lDestAccountID, szDestMailbox, cDestDelimiter, bCopy)
{
}

CImapCrossServerTransferMsgAction::CImapCrossServerTransferMsgAction() : 
	CImapTransferMsgAction()
{
}

CImapCrossServerTransferMsgAction::~CImapCrossServerTransferMsgAction()
{
}

//
//	CImapCrossServerTransferMsgAction::Write()
//
//	Write the message transfer data.
//
bool CImapCrossServerTransferMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	if (m_bCopy)
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeCopyCross);
	}
	else
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeTransferCross);
	}

	//	Write the tags common to all transfer actions
	CImapTransferMsgAction::Write(xmlWriter);

	return true;
}

//
//	CImapCrossServerTransferMsgAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapCrossServerTransferMsgAction::Duplicate()
{
	CImapCrossServerTransferMsgAction		*pImapAction = DEBUG_NEW CImapCrossServerTransferMsgAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapCrossServerTransferMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapCrossServerTransferMsgAction::Copy(CImapAction *pImapAction)
{
	CImapTransferMsgAction::Copy(pImapAction);
}

//
//	CImapCrossServerTransferMsgAction::DoPreThreadAction()
//
//	Prepare to perform the online transfer of the specified messages.
//
bool CImapCrossServerTransferMsgAction::DoPreThreadAction()
{
	if (!CImapTransferMsgAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	CString			strTitle;
	if (m_bCopy)
	{
		strTitle = CRString(IDS_IMAP_COPYING);
	}
	else
	{
		strTitle = CRString(IDS_IMAP_TRANSFERING);
	}
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapCrossServerTransferMsgAction::DoWork()
//
//	Perform the online transfer of the specified messages.
//
HRESULT CImapCrossServerTransferMsgAction::DoWork()
{
	if (!m_pTocDoc || !m_pImapMailbox || !m_pDestTocDoc || !m_pDestTocDoc->m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pActionQueue)
	{
		m_bIsLastOnMailbox = (m_pActionQueue->GetNextMailboxMatch(m_lDestMailboxHash) == NULL);
	}

	return S_OK;
}

//
//	CImapCrossServerTransferMsgAction::DoPostProcessing()
//
//	Do the actual message transfer here since retrieving the message body is not thread safe.
//
void CImapCrossServerTransferMsgAction::DoPostProcessing()
{
	if (!m_pTocDoc)
	{
		return;
	}

	// Now transfer the messages on the server.
	CSummary	*pSum = NULL;
	CSummary	*pCopySum = NULL;
	CString		 strUIDList(m_szMessageIDs);
	bool		 bSomeFailed = false;
	do
	{
		pSum = GetSummaryFromUIDString(strUIDList);

		if (pSum)
		{
			if (pSum->IsNotFullyIMAPDownloaded())
			{
				CSumList		sumList;
				sumList.AddHead(pSum);
				m_pTocDoc->ImapFetchMessagesFromServer(&sumList,
														TRUE/*bDownloadAttachments*/,
														TRUE/*m_bOnlyIfNotDownloaded*/,
														FALSE/*m_bClearCacheOnly*/,
														TRUE/*m_bInvalidateCachedPreviewSums*/);
			}
			if (!m_pTocDoc->ImapCopyMsgBetweenAccounts(pSum, m_pDestTocDoc, &pCopySum))
			{
				bSomeFailed = true;
			}
		}
	} while (pSum);

	if (!m_bCopy)
	{
		m_pImapMailbox->DeleteMessagesFromServer(m_szMessageIDs, m_bAutoExpunge, TRUE/*bSilent*/);
	}

	if (bSomeFailed)
	{
		m_iState = kActionStateFailed;
	}
	else
	{
		m_iState = kActionStateSucceeded;
	}
}


//
//	CImapToLocalTransferMsgAction
//

CImapToLocalTransferMsgAction::CImapToLocalTransferMsgAction(unsigned long lAccountID,
															 const char *szMailbox,
															 TCHAR cDelimiter,
															 const char *szMessageIDs,
															 const char *szDestMailbox,
															 BOOL bCopy) : 
	CImapTransferMsgAction(lAccountID, szMailbox, cDelimiter, szMessageIDs,
						   0, szDestMailbox, '\0', bCopy)
{
}

CImapToLocalTransferMsgAction::CImapToLocalTransferMsgAction() : 
	CImapTransferMsgAction()
{
}

CImapToLocalTransferMsgAction::~CImapToLocalTransferMsgAction()
{
}

//
//	CImapToLocalTransferMsgAction::Write()
//
//	Write the message transfer data.
//
bool CImapToLocalTransferMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	if (m_bCopy)
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeCopyToLocal);
	}
	else
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeTransferToLocal);
	}

	//	Write the tags common to all transfer actions
	CImapTransferMsgAction::Write(xmlWriter);

	return true;
}

//
//	CImapToLocalTransferMsgAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapToLocalTransferMsgAction::Duplicate()
{
	CImapToLocalTransferMsgAction		*pImapAction = DEBUG_NEW CImapToLocalTransferMsgAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapToLocalTransferMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapToLocalTransferMsgAction::Copy(CImapAction *pImapAction)
{
	CImapTransferMsgAction::Copy(pImapAction);
}

//
//	CImapToLocalTransferMsgAction::DoPreThreadAction()
//
//	Prepare to perform the online transfer of the specified messages.
//
bool CImapToLocalTransferMsgAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		m_pTocDoc = GetToc(m_pCommand->GetPathname());
		if (m_pTocDoc)
		{
			m_pImapMailbox = m_pTocDoc->m_pImapMailbox;
		}
	}

	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	QCMailboxCommand	*pCommand = g_theMailboxDirector.FindByPathname(m_szDestMailbox);
	if (pCommand)
	{
		m_pDestTocDoc = GetToc(pCommand->GetPathname());
	}

	if (!m_pDestTocDoc)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	// Don't allow the TocDoc to auto-delete before we are done with it.
	m_pDestTocDoc->IncrementPreventAutoDelete();

	if (!OpenMailbox(m_pImapMailbox))
	{
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	CString			strTitle;
	if (m_bCopy)
	{
		strTitle = CRString(IDS_IMAP_COPYING);
	}
	else
	{
		strTitle = CRString(IDS_IMAP_TRANSFERING);
	}
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapToLocalTransferMsgAction::DoWork()
//
//	Perform the online transfer of the specified messages.  Well, for now message body downloading is
//	not thread safe so we don't actually do anything here.
//
HRESULT CImapToLocalTransferMsgAction::DoWork()
{
	if (!m_pTocDoc || !m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	return S_OK;
}

//
//	CImapToLocalTransferMsgAction::DoPostProcessing()
//
//	Do the actual message transfer here since retrieving the message body is not thread safe.
//
void CImapToLocalTransferMsgAction::DoPostProcessing()
{
	// If message was not fetched, download the data and do the transfer.
	// Now transfer the messages on the server.
	CSummary	*pSum = NULL;
	CSummary	*pCopySum = NULL;
	CString		 strUIDList(m_szMessageIDs);
	do
	{
		pSum = GetSummaryFromUIDString(strUIDList);

		if (pSum)
		{
			// Only proceed if the message is not fully downloaded.  If the message is already
			// downloaded, then the local transfer we did before this action was queued already
			// did all the work.
			if (pSum->IsNotFullyIMAPDownloaded())
			{
				CSumList		sumList;
				sumList.AddHead(pSum);
				m_pTocDoc->ImapFetchMessagesFromServer(&sumList,
														TRUE/*bDownloadAttachments*/,
														TRUE/*m_bOnlyIfNotDownloaded*/,
														FALSE/*m_bClearCacheOnly*/,
														TRUE/*m_bInvalidateCachedPreviewSums*/);
				if (!pSum->IsNotFullyIMAPDownloaded())
				{
					m_pTocDoc->ImapCopyMsgFromImapToLocal(pSum, m_pDestTocDoc, &pCopySum);
				}
			}
		}
	} while (pSum);

	// We are done with the TocDoc so allow it to auto-delete again.
	m_pDestTocDoc->DecrementPreventAutoDelete();

	if (!m_bCopy)
	{
		m_pImapMailbox->DeleteMessagesFromServer(m_szMessageIDs, m_bAutoExpunge, TRUE/*bSilent*/);
	}

	m_iState = kActionStateSucceeded;

}


//
//	CImapFromLocalTransferMsgAction
//

CImapFromLocalTransferMsgAction::CImapFromLocalTransferMsgAction(const char *szMailbox,
																 unsigned long lDestAccountID,
																 const char *szDestMailbox,
																 TCHAR cDestDelimiter,
																 const char *szMessageIDs,
																 BOOL bCopy) : 
	CImapTransferMsgAction(0, szMailbox, '\0', szMessageIDs,
						   lDestAccountID, szDestMailbox, cDestDelimiter, bCopy)
{
}

CImapFromLocalTransferMsgAction::CImapFromLocalTransferMsgAction() : 
	CImapTransferMsgAction()
{
}

CImapFromLocalTransferMsgAction::~CImapFromLocalTransferMsgAction()
{
}

//
//	CImapFromLocalTransferMsgAction::Write()
//
//	Write the message transfer data.
//
bool CImapFromLocalTransferMsgAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	if (m_bCopy)
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeCopyFromLocal);
	}
	else
	{
		xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeTransferFromLocal);
	}

	//	Write the tags common to all transfer actions
	CImapTransferMsgAction::Write(xmlWriter);

	return true;
}

//
//	CImapFromLocalTransferMsgAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapFromLocalTransferMsgAction::Duplicate()
{
	CImapFromLocalTransferMsgAction		*pImapAction = DEBUG_NEW CImapFromLocalTransferMsgAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapFromLocalTransferMsgAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapFromLocalTransferMsgAction::Copy(CImapAction *pImapAction)
{
	CImapTransferMsgAction::Copy(pImapAction);
}

//
//	CImapFromLocalTransferMsgAction::DoPreThreadAction()
//
//	Prepare to perform the online transfer of the specified messages.
//
bool CImapFromLocalTransferMsgAction::DoPreThreadAction()
{
	// This is the only subclass of CImapAction that doesn't call CImapAction::DoPreThreadAction().
	// That is because in this one case the source account/mailbox is not IMAP.  Just make sure we
	// do any appropriate initialization from CImapAction::DoPreThreadAction() here.  As of now,
	// all that means is set our state correctly.
	m_iState = kActionStateWorking;

	m_pAccount = g_ImapAccountMgr.FindAccount(m_lDestAccountID);
	if (!m_pAccount)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	m_pDestCommand = g_theMailboxDirector.ImapFindByImapName(m_lDestAccountID,
															 m_szDestMailbox,
															 m_cDestDelimiter);

	if (m_pDestCommand)
	{
		m_pDestTocDoc = GetToc(m_pDestCommand->GetPathname());
	}

	if (!m_pDestTocDoc || !m_pDestTocDoc->m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	if (!OpenMailbox(m_pDestTocDoc->m_pImapMailbox))
	{
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	CString			strTitle;
	if (m_bCopy)
	{
		strTitle = CRString(IDS_IMAP_COPYING);
	}
	else
	{
		strTitle = CRString(IDS_IMAP_TRANSFERING);
	}
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapFromLocalTransferMsgAction::DoWork()
//
//	Do any thread safe actions to prepare to perform the online transfer of the specified messages.
//	Note that at this time retrieving the message body is not threadsafe so we can't do the actual
//	message transfer here at this time.
//
HRESULT CImapFromLocalTransferMsgAction::DoWork()
{
	if (!m_pDestTocDoc || !m_pDestTocDoc->m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pActionQueue)
	{
		m_bIsLastOnMailbox = (m_pActionQueue->GetNextMailboxMatch(m_lDestMailboxHash) == NULL);
	}

	return S_OK;
}

//
//	CImapFromLocalTransferMsgAction::DoPostProcessing()
//
//	Do the actual message transfer here since retrieving the message body is not thread safe.
//
void CImapFromLocalTransferMsgAction::DoPostProcessing()
{
	// Now transfer the messages on the server.
	CSummary		*pSum = NULL;
	CSummary		*pCopySum = NULL;
	CString			 strUIDList(m_szMessageIDs);
	bool			 bSomeFailed = false;
	do
	{
		pSum = GetSummaryFromUIDStringFromDestination(strUIDList);

		if (pSum)
		{
			if (!m_pDestTocDoc->ImapCopyMsgFromLocalToImap(pSum, m_pDestTocDoc, &pCopySum))
			{
				bSomeFailed = true;
			}
		}
	} while (pSum);

	if (bSomeFailed)
	{
		m_iState = kActionStateFailed;
	}
	else
	{
		m_iState = kActionStateSucceeded;
	}

	// Resync the target mailbox.
	// Only resync on the last item in the queue that acts on this mailbox, otherwise
	// we might enter a state where more than one message is transferred to the same mailbox via
	// different operations, then when the first transfer is performed online we sync and lose
	// any local data placed by the later transfers.
	if (m_bIsLastOnMailbox && m_pDestTocDoc && m_pDestTocDoc->m_pImapMailbox)
	{
		m_pDestTocDoc->m_pImapMailbox->DoManualResync(m_pDestTocDoc, TRUE, FALSE, FALSE);
	}
}


//
//	CImapCreateMailboxAction
//

CImapCreateMailboxAction::CImapCreateMailboxAction(unsigned long lAccountID,
												   const char *szMailbox,
												   TCHAR cDelimiter,
												   CString strLogin,
												   CString strPassword) :
	CImapAction(lAccountID, szMailbox, cDelimiter, "", strLogin, strPassword)
{
}

CImapCreateMailboxAction::CImapCreateMailboxAction() :
	CImapAction()
{
}

CImapCreateMailboxAction::~CImapCreateMailboxAction()
{
}

//
//	CImapCreateMailboxAction::Write()
//
//	Write the mailbox creation data.
//
bool CImapCreateMailboxAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeCreateMbx);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapCreateMailboxAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapCreateMailboxAction::Duplicate()
{
	CImapCreateMailboxAction		*pImapAction = DEBUG_NEW CImapCreateMailboxAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapCreateMailboxAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapCreateMailboxAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapCreateMailboxAction::DoPreThreadAction()
//
//	Prepare to perform the online creation of the specified mailbox.
//
bool CImapCreateMailboxAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	CRString		 strTitle(IDS_IMAP_CREATING);
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapCreateMailboxAction::DoWork()
//
//	Perform the online creation of the specified mailbox.
//
HRESULT CImapCreateMailboxAction::DoWork()
{
	if (!m_pAccount)
	{
		return E_FAIL;
	}

	if (m_pAccount->CreateMailboxOnServer(m_szMailbox, m_cDelimiter))
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}

//
//	CImapCreateMailboxAction::DoPostProcessing()
//
//	Update the mailbox info in case the server has different attributes than what we anticipated.
//	This must be done in the main thread.
//
void CImapCreateMailboxAction::DoPostProcessing()
{
	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	// Update the mailbox's local info.
	if (m_pCommand)
	{
		m_pCommand->Execute(CA_UPDATE_STATUS, (void*)US_NO);
	}
}


//
//	CImapRenameMailboxAction
//

CImapRenameMailboxAction::CImapRenameMailboxAction(unsigned long lAccountID,
												   const char *szMailbox,
												   TCHAR cDelimiter,
												   LPCSTR szNewName,
												   CString strLogin,
												   CString strPassword) :
	CImapAction(lAccountID, szMailbox, cDelimiter, "", strLogin, strPassword),
	m_szNewName(NULL)
{
	if (szNewName && *szNewName)
	{
		m_szNewName = strdup(szNewName);
	}
}

CImapRenameMailboxAction::CImapRenameMailboxAction() :
	CImapAction(),
	m_szNewName(NULL)
{
}

CImapRenameMailboxAction::~CImapRenameMailboxAction()
{
	if (m_szNewName)
	{
		free(m_szNewName);
	}
}

//
//	CImapRenameMailboxAction::Write()
//
//	Write the mailbox rename data.
//
bool CImapRenameMailboxAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeRenameMbx);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	//	Write the NewName tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyNewMailboxName, true, "%s", m_szNewName);

	return true;
}

//
//	CImapRenameMailboxAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapRenameMailboxAction::Duplicate()
{
	CImapRenameMailboxAction		*pImapAction = DEBUG_NEW CImapRenameMailboxAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapRenameMailboxAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapRenameMailboxAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapRenameMailboxAction::DoPreThreadAction()
//
//	Prepare to perform the online rename of the specified mailbox.
//
bool CImapRenameMailboxAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	m_pTaskInfo->SetTitle(CRString(IDS_IMAP_RENAMING));

	return true;
}

//
//	CImapRenameMailboxAction::DoWork()
//
//	Perform the online renaming of the specified mailbox.
//
HRESULT CImapRenameMailboxAction::DoWork()
{
	if (!m_pAccount)
	{
		return E_FAIL;
	}

	if (m_pAccount->RenameMailboxOnServer(m_strLogin, m_strPassword, m_szMailbox, m_szNewName, m_cDelimiter))
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}

void CImapRenameMailboxAction::SetNewName(const char *szNewName)
{
	if (szNewName && *szNewName)
	{
		m_szNewName = strdup(szNewName);
	}
}


//
//	CImapDeleteMailboxAction
//

CImapDeleteMailboxAction::CImapDeleteMailboxAction(unsigned long lAccountID,
												   const char *szMailbox,
												   TCHAR cDelimiter,
												   CString strLogin,
												   CString strPassword):
	CImapAction(lAccountID, szMailbox, cDelimiter, "", strLogin, strPassword)
{
}

CImapDeleteMailboxAction::CImapDeleteMailboxAction():
	CImapAction()
{
}

CImapDeleteMailboxAction::~CImapDeleteMailboxAction()
{
}

//
//	CImapDeleteMailboxAction::Write()
//
//	Write the mailbox deletion data.
//
bool CImapDeleteMailboxAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeDeleteMbx);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapDeleteMailboxAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapDeleteMailboxAction::Duplicate()
{
	CImapDeleteMailboxAction		*pImapAction = DEBUG_NEW CImapDeleteMailboxAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapDeleteMailboxAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapDeleteMailboxAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapDeleteMailboxAction::DoPreThreadAction()
//
//	Prepare to perform the online deletion of the specified mailbox.
//
bool CImapDeleteMailboxAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_SENDING);
	m_pTaskInfo->SetTitle(CRString(IDS_IMAP_DELETING));

	return true;
}

//
//	CImapDeleteMailboxAction::DoWork()
//
//	Perform the online deletion of the specified mailbox.
//
HRESULT CImapDeleteMailboxAction::DoWork()
{
	if (!m_pAccount)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pAccount->DeleteMailboxOnServer(m_strLogin, m_strPassword, m_szMailbox, m_cDelimiter))
	{
		m_iState = kActionStateSucceeded;
	}
	else
	{
		m_iState = kActionStateFailed;
	}

	return S_OK;
}


//
//	CImapRefreshMailboxesAction
//

CImapRefreshMailboxesAction::CImapRefreshMailboxesAction(unsigned long lAccountID,
														 const char *szMailbox,
														 TCHAR cDelimiter,
														 CString strLogin,
														 CString strPassword):
	CImapAction(lAccountID, szMailbox, cDelimiter, "", strLogin, strPassword)
{
}

CImapRefreshMailboxesAction::CImapRefreshMailboxesAction():
	CImapAction()
{
}

CImapRefreshMailboxesAction::~CImapRefreshMailboxesAction()
{
}

//
//	CImapRefreshMailboxesAction::Write()
//
//	Write the mailbox refresh data.
//
bool CImapRefreshMailboxesAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeRefresh);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapRefreshMailboxesAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapRefreshMailboxesAction::Duplicate()
{
	CImapRefreshMailboxesAction		*pImapAction = DEBUG_NEW CImapRefreshMailboxesAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapRefreshMailboxesAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapRefreshMailboxesAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapRefreshMailboxesAction::DoPreThreadAction()
//
//	Prepare to perform the mailbox list refresh.
//
bool CImapRefreshMailboxesAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	CRString		 strTitle(IDS_IMAP_REFRESHING);
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapRefreshMailboxesAction::DoWork()
//
//	Perform the mailbox list refresh.
//
HRESULT CImapRefreshMailboxesAction::DoWork()
{
	return S_OK;
}

//
//	CImapRefreshMailboxesAction::DoPostProcessing()
//
//	Do the main thread portion of the mailbox refresh.
//
void CImapRefreshMailboxesAction::DoPostProcessing()
{
	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand && m_pAccount)
	{
		if (m_szMailbox && *m_szMailbox)
		{
            m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
																 m_szMailbox,
																 m_cDelimiter);
		}
		else
		{
			CString		 strDirectory;
			m_pAccount->GetDirectory(strDirectory);

			m_pCommand = (QCImapMailboxCommand*)g_theMailboxDirector.FindByPathname(strDirectory);
		}
	}

	if (!m_pCommand)
	{
		m_iState = kActionStateFailed;
		return;
	}

	// Note: On input to Execute(), set the value of the BOOL pData points to
	// to TRUE to indicate that we want to recurse through child mailboxes.
	// It will be set by Execute() to indicate if we need to update the mailbox tree control.
	BOOL	bViewNeedsRefresh = TRUE;
	m_pCommand->Execute(CA_IMAP_REFRESH, &bViewNeedsRefresh);
	if (bViewNeedsRefresh)
	{
		// Delete all child items and force a re-fresh.
		// Note: Recurse.
		g_theMailboxDirector.UpdateImapMailboxLists(m_pCommand, TRUE, TRUE);
	}

	m_iState = kActionStateSucceeded;
}


//
//	CImapExpungeAction
//

CImapExpungeAction::CImapExpungeAction(unsigned long lAccountID,
									   const char *szMailbox,
									   TCHAR cDelimiter):
	CImapAction(lAccountID, szMailbox, cDelimiter, "", "", "")
{
}

CImapExpungeAction::CImapExpungeAction():
	CImapAction()
{
}

CImapExpungeAction::~CImapExpungeAction()
{
}

//
//	CImapExpungeAction::Write()
//
//	Write the mailbox refresh data.
//
bool CImapExpungeAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeExpunge);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapExpungeAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapExpungeAction::Duplicate()
{
	CImapExpungeAction		*pImapAction = DEBUG_NEW CImapExpungeAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapExpungeAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapExpungeAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapExpungeAction::DoPreThreadAction()
//
//	Prepare to perform the mailbox list refresh.
//
bool CImapExpungeAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		m_pTocDoc = GetToc(m_pCommand->GetPathname());
		if (m_pTocDoc)
		{
			m_pImapMailbox = m_pTocDoc->m_pImapMailbox;
		}
	}

	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	CRString		 strTitle(IDS_IMAP_EXPUNGING);
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapExpungeAction::DoWork()
//
//	Perform the mailbox list refresh.
//
HRESULT CImapExpungeAction::DoWork()
{
	if (!m_pTocDoc || !m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	if (m_pActionQueue)
	{
		m_bIsLastOnMailbox = (m_pActionQueue->GetNextMailboxMatch(m_lMailboxHash) == NULL);
	}

	m_pTocDoc->ImapExpungeOnServer();

	m_iState = kActionStateSucceeded;

	return S_OK;
}

//
//	CImapExpungeAction::DoPostProcessing()
//
//	Do the main thread portion of the mailbox refresh.
//
void CImapExpungeAction::DoPostProcessing()
{
	if (m_bIsLastOnMailbox && m_pTocDoc && m_pImapMailbox)
	{
		m_pTocDoc->ImapDoPostExpunge();

		m_pImapMailbox->DoManualResync(m_pTocDoc, TRUE, FALSE, FALSE);

		m_pTocDoc->Write();
	}
}


//
//	CImapResyncAction
//

CImapResyncAction::CImapResyncAction(unsigned long lAccountID,
									 const char *szMailbox,
									 TCHAR cDelimiter,
									 BOOL bNormalCheck,
									 unsigned long ulBitflags,
									 BOOL bUsesNotifier,
									 BOOL bDownloadedOnly):
	CImapAction(lAccountID, szMailbox, cDelimiter, "", "", ""),
	m_bNormalCheck(bNormalCheck),
	m_ulBitflags(ulBitflags),
	m_bUsesNotifier(bUsesNotifier),
	m_bDownloadedOnly(bDownloadedOnly)
{
}

CImapResyncAction::CImapResyncAction():
	CImapAction(),
	m_bNormalCheck(TRUE),
	m_ulBitflags(0),
	m_bDownloadedOnly(TRUE)	
{
}

CImapResyncAction::~CImapResyncAction()
{
}

//
//	CImapResyncAction::Write()
//
//	Write the mailbox resync data.
//
bool CImapResyncAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeResync);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyNormalCheck,
							   true,
							   "%d",
							   m_bNormalCheck ? 1 : 0);

	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyBitflags,
							   true,
							   "%d",
							   m_ulBitflags);

	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyUsesNotifier,
							   true,
							   "%d",
							   m_bUsesNotifier ? 1 : 0);

	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyDownloadedOnly,
							   true,
							   "%d",
							   m_bDownloadedOnly ? 1 : 0);

	return true;
}

//
//	CImapResyncAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapResyncAction::Duplicate()
{
	CImapResyncAction		*pImapAction = DEBUG_NEW CImapResyncAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapResyncAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapResyncAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);

	if (pImapAction)
	{
		CImapResyncAction	*pImapResyncAction = (CImapResyncAction*)pImapAction;

		m_bNormalCheck = pImapResyncAction->m_bNormalCheck;
		m_ulBitflags = pImapResyncAction->m_ulBitflags;
		m_bUsesNotifier = pImapResyncAction->m_bUsesNotifier;
		m_bDownloadedOnly = pImapResyncAction->m_bDownloadedOnly;
	}
}

//
//	CImapResyncAction::DoPreThreadAction()
//
//	Prepare to perform the mailbox resync.
//
bool CImapResyncAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

//	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
//	{
//		// The only way the above can fail is if we can't create a new connection object.
//		// This should be extremely rare and if it happens that likely means some kind of
//		// catastrophic failure that we cannot hope to gracefully recover from.
//		m_iState = kActionStateFailed;
//		return false;
//	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		m_pTocDoc = GetToc(m_pCommand->GetPathname());
	}

	if (!m_pTocDoc)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	CRString		 strTitle(IDS_IMAP_RESYNCING);
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapResyncAction::DoWork()
//
//	Perform the mailbox resync.
//
HRESULT CImapResyncAction::DoWork()
{
	if (!m_pTocDoc)
	{
		m_iState = kActionStateFailed;
		return E_FAIL;
	}

	return S_OK;
}

//
//	CImapResyncAction::DoPostProcessing()
//
//	Do the main thread portion of the mailbox refresh.
//
void CImapResyncAction::DoPostProcessing()
{
	if (m_pTocDoc)
	{
		CImapMailMgr	*pImapMailMgr = GetImapMailMgr();
		if (pImapMailMgr)
		{
			CString		 strAccountName;
			m_pAccount->GetName(strAccountName);
			QCPOPNotifier	*pPOPNotifier = NULL;
			if (m_bUsesNotifier)
			{
				pPOPNotifier = DEBUG_NEW_NOTHROW QCPOPNotifier(1);
			}
			pImapMailMgr->CheckMailOnServer(strAccountName,
											m_pTocDoc,
											m_bNormalCheck,
											m_ulBitflags,
											pPOPNotifier,
											m_bDownloadedOnly);
			if (m_bUsesNotifier)
			{
				pPOPNotifier->Decrement();
			}
		}
	}

	m_iState = kActionStateSucceeded;
}

//
//	CImapResyncAction::UsesMailbox()
//
//	Returns false.  This function safeguards against do a resync before a given action in the
//	specified mailbox has a chance to complete.  The whole point of this action is to resync so
//	it doesn't count as using the mailbox.
//
bool CImapResyncAction::UsesMailbox(unsigned long lMailboxHash)
{
	return false;
}


//
//	CImapDownloadAction
//

CImapDownloadAction::CImapDownloadAction(unsigned long lAccountID,
										 const char *szMailbox,
										 TCHAR cDelimiter,
										 const char *szMessageIDs,
										 BOOL bDownloadAttachments,
										 BOOL bOnlyIfNotDownloaded,
										 BOOL bClearCacheOnly,
										 BOOL bInvalidateCachedPreviewSums):
	CImapAction(lAccountID, szMailbox, cDelimiter, szMessageIDs, "", ""),
	m_bDownloadAttachments(bDownloadAttachments),
	m_bOnlyIfNotDownloaded(bOnlyIfNotDownloaded),
	m_bClearCacheOnly(bClearCacheOnly),
	m_bInvalidateCachedPreviewSums(bInvalidateCachedPreviewSums)
{
}

CImapDownloadAction::CImapDownloadAction():
	CImapAction(),
	m_bDownloadAttachments(FALSE),
	m_bOnlyIfNotDownloaded(FALSE),
	m_bClearCacheOnly(FALSE),
	m_bInvalidateCachedPreviewSums(FALSE)
{
}

CImapDownloadAction::~CImapDownloadAction()
{
}

//
//	CImapDownloadAction::Write()
//
//	Write the message download data.
//
bool CImapDownloadAction::Write(XMLWriter *xmlWriter)
{
	if (!xmlWriter)
	{
		return false;
	}

	//	Write ActionType tag
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyActionType, true, "%s", kDataActionTypeDownload);

	//	Write the binary flag tags
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyDownloadAttachments,
							   true,
							   "%d",
							   m_bDownloadAttachments ? 1 : 0);
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyOnlyIfNotDownloaded,
							   true,
							   "%d",
							   m_bOnlyIfNotDownloaded ? 1 : 0);
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyClearCacheOnly,
							   true,
							   "%d",
							   m_bClearCacheOnly ? 1 : 0);
	xmlWriter->WriteTaggedData(CImapActionXMLParser::kKeyInvalidateCachedPreviewSums,
							   true,
							   "%d",
							   m_bInvalidateCachedPreviewSums ? 1 : 0);

	//	Write the tags common to all actions
	CImapAction::Write(xmlWriter);

	return true;
}

//
//	CImapDownloadAction::Duplicate()
//
//	Duplicate this object.
//
CImapAction *CImapDownloadAction::Duplicate()
{
	CImapDownloadAction		*pImapAction = DEBUG_NEW CImapDownloadAction();
	if (pImapAction)
	{
		pImapAction->Copy(this);
	}
	return pImapAction;
}

//
//	CImapDownloadAction::Copy()
//
//	Copy the relevant data fields into the specified object.
//
void CImapDownloadAction::Copy(CImapAction *pImapAction)
{
	CImapAction::Copy(pImapAction);
}

//
//	CImapDownloadAction::DoPreThreadAction()
//
//	Prepare to perform the message download.
//
bool CImapDownloadAction::DoPreThreadAction()
{
	if (!CImapAction::DoPreThreadAction())
	{
		// State was already set by CImapAction::DoPreThreadAction(), just bail.
		return false;
	}

	if (!m_pAccount->SetupControlStream(m_strLogin, m_strPassword))
	{
		// The only way the above can fail is if we can't create a new connection object.
		// This should be extremely rare and if it happens that likely means some kind of
		// catastrophic failure that we cannot hope to gracefully recover from.
		m_iState = kActionStateFailed;
		return false;
	}

	// If we don't have a pointer to the relevant command object, obtain it now.
	if (!m_pCommand)
	{
		m_pCommand = g_theMailboxDirector.ImapFindByImapName(m_lAccountID,
															 m_szMailbox,
															 m_cDelimiter);
	}

	if (m_pCommand)
	{
		m_pTocDoc = GetToc(m_pCommand->GetPathname());
		if (m_pTocDoc)
		{
			m_pImapMailbox = m_pTocDoc->m_pImapMailbox;
		}
	}

	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return false;
	}

	m_pTaskInfo->SetTaskType(TASK_TYPE_RECEIVING);
	CRString		 strTitle(IDS_IMAP_DOWNLOADING);
	m_pTaskInfo->SetTitle(strTitle);
	m_pTaskInfo->SetPostProcFn(strTitle, ::DoPostProcessing, this);

	return true;
}

//
//	CImapDownloadAction::DoWork()
//
//	Perform the mailbox list refresh.
//
HRESULT CImapDownloadAction::DoWork()
{
	return S_OK;
}

//
//	CImapDownloadAction::DoPostProcessing()
//
//	Do the main thread portion of the mailbox refresh.
//
void CImapDownloadAction::DoPostProcessing()
{
	if (!m_pImapMailbox)
	{
		m_iState = kActionStateFailed;
		return;
	}

	CSumList		 sumList;
	CSummary		*pSum = NULL;
	CString			 strUIDList(m_szMessageIDs);
	do
	{
		pSum = GetSummaryFromUIDString(strUIDList);

		if (pSum)
		{
			sumList.AddTail(pSum);
		}
	} while (pSum);

	m_pTocDoc->ImapFetchMessagesFromServer(&sumList,
										   m_bDownloadAttachments,
										   m_bOnlyIfNotDownloaded,
										   m_bClearCacheOnly,
										   m_bInvalidateCachedPreviewSums);

	m_iState = kActionStateSucceeded;
}


//
//	Global action queue object.
//

CActionQueue::CActionQueue(unsigned long lAccountID, CString &strFileName, CString &strDirName) :
	m_lAccountID(lAccountID),
	m_iState(kQueueStateIdle),
	m_bNeedsWrite(false)
{
	m_strFileName = strFileName;
	m_strDirName = strDirName;
}

CActionQueue::~CActionQueue()
{
}

//
//	CActionQueue::OnIdle()
//
//	Perform idle time processing.  At the moment this simply consists of popping the first item off the
//	queue and performing it.
//
//	Returns true if an action was performed, false if there is nothing to do.
//
bool CActionQueue::OnIdle(bool bHighPriorityOnly, bool bOnlineStateChanged)
{
	if (bOnlineStateChanged && (m_iState == kQueueStateBlocked))
	{
		// When the state changes, reset the flag if it is blocked.  No matter what failed to cause us
		// to be blocked, it is time to give it another try.
		m_iState = kQueueStateIdle;
	}

	if ((m_iState != kQueueStateIdle) || (m_array.GetSize() == 0))
	{
		return false;
	}

	m_iState = kQueueStateProcessing;

	// Pop the first item off the queue and perform it.
	CImapAction			*pImapAction = (CImapAction*)m_array.GetAt(0);
	if (pImapAction)
	{
		// Bail out if we are only doing high priority items at this time and the first item
		// in the queue is not a high priority one.
		if (bHighPriorityOnly && (pImapAction->GetPriority() != kImapActionPriorityHigh))
		{
			m_iState = kQueueStateIdle;
			return false;
		}

		// Do any actions we need to do in the main thread, including establishing a network connection.
		pImapAction->DoPreThreadAction();

		// Check the state of the entry after doing the main thread actions.
		switch(pImapAction->GetState())
		{
			case kActionStateWorking:
				// All main thread actions succeeded.
				if (QCGetTaskManager()->QueueWorkerThread(pImapAction))
				{
					m_array.RemoveAt(0);
					m_bNeedsWrite = true;
				}
				else
				{
					// Question: How exactly do we handle failure to queue the action?  Normally we would just
					// do the following and delete the thread object, however in this case we want to make
					// every effort to do the action online so we don't want to give up on the first try.
					// However, we also don't want to keep stumbling on the same action.
					// delete pImapAction->GetTaskInfo();
					// delete pImapAction;
				}
				break;
			case kActionStateFailed:
				// Complete failure.  Remove the action so we don't attempt it again.
				m_array.RemoveAt(0);
				m_bNeedsWrite = true;
				m_iState = kQueueStateIdle;
				break;
			case kActionStateBusy:
			case kActionStateCancel:
			case kActionStateLoginFailed:
			case kActionStateOffline:
				// The action couldn't proceed because of a temporary failure.  These failures would
				// apply to any further actions on this account so block any further actions for now.
				m_iState = kQueueStateBlocked;
				break;
			case kActionStateSucceeded:
			case kActionStateWaiting:
				// We shouldn't be getting here in these states.
				ASSERT(0);
				m_iState = kQueueStateIdle;
				break;
		}
	}

	return TRUE;
}

//
//	CActionQueue::IsProcessing()
//
//	Returns true if an action is currently being processed, false otherwise.
//
bool CActionQueue::IsProcessing()
{
	if (m_iState == kQueueStateProcessing)
	{
		return true;
	}

	return false;
}

//
//	CActionQueue::FinishUp()
//
//	Empty out the queue by removing and performing any actions in the queue.
//
void CActionQueue::FinishUp()
{
	while (m_array.GetSize() > 0)
	{
		CImapAction			*pImapAction = (CImapAction*)m_array.GetAt(0);
		if (pImapAction)
		{
			m_array.RemoveAt(0);
			pImapAction->DoPreThreadAction();
			delete pImapAction;
		}
	}
}

//
//	CActionQueue::Add()
//
//	Add the specified action to the end of the queue.
//
void CActionQueue::Add(CImapAction *pImapAction)
{
	if (!pImapAction)
	{
		return;
	}

	m_bNeedsWrite = true;

	pImapAction->SetActionQueue(this);

	// Add the item to the queue.  Items with a priority of immediate are placed at the front
	// of the queue.  For now all other items are added to the end of the queue.
	if (pImapAction->GetPriority() == kImapActionPriorityHigh)
	{
		m_array.InsertAt(0, pImapAction);
	}
	else
	{
		m_array.Add(pImapAction);
	}

	// If we are blocked because of a temporary failure, try again.  The rationale here is that
	// the user just did something that needs to be queued, so asking them for their password again
	// (in the case of login failure) wouldn't be out of the blue.
	if (m_iState == kQueueStateBlocked)
	{
		m_iState = kQueueStateIdle;
	}
}

//
//	CActionQueue::AddFromFile()
//
//	Add the specified action read from the file to the end of the queue.
//
void CActionQueue::AddFromFile(CImapAction *pImapAction)
{
	if (!pImapAction)
	{
		return;
	}

	// Do we maybe want to do some sanity checking to see that the item we are about to add is well-formed?

	// Since there is one queue file per account, writing out the account info for each item would be
	// redundant.  When we reach this point the account ID will be empty.  Set it now from the ID of
	// the account which owns this queue.
	pImapAction->m_lAccountID = m_lAccountID;

	pImapAction->SetActionQueue(this);

	m_array.Add(pImapAction);
}

//
//	CActionQueue::ReAdd()
//
//	Re-add the specified action to the start of the queue.
//
void CActionQueue::ReAdd(CImapAction *pImapAction)
{
	if (!pImapAction)
	{
		return;
	}

	m_bNeedsWrite = true;

	m_array.InsertAt(0, pImapAction);
}

//
//	CActionQueue::ActionDone()
//
//	Note that the action currently being processed is done.
//
void CActionQueue::ActionDone()
{
	m_iState = kQueueStateIdle;
}

//
//	CActionQueue::HasHighPriorityItems()
//
//	Returns true if there are any high priority items in the action queue, false otherwise.
//
bool CActionQueue::HasHighPriorityItems()
{
	// See if the first item in the queue is high priority.
	if (m_array.GetSize() > 0)
	{
		CImapAction			*pImapAction = (CImapAction*)m_array.GetAt(0);
		if (pImapAction)
		{
			if (pImapAction->GetPriority() == kImapActionPriorityHigh)
			{
				return true;
			}
		}
	}

	return false;
}

//
//	CActionQueue::GetNextMailboxMatch()
//
//	Returns the next item in the queue acting on the same mailbox as the specified item.
//
//	Note: For now this assumes the item to be matched has been removed from the queue.
//	At some point we may want this to be flexible enough to find the item in the list and
//	find the next match after that item.
//
CImapAction *CActionQueue::GetNextMailboxMatch(unsigned long ulHashToMatch)
{
	if (ulHashToMatch == 0)
	{
		return NULL;
	}

	int					 iSize = m_array.GetSize();
	CImapAction			*pImapAction = NULL;
	for (int i = 0; i < iSize; ++i)
	{
		pImapAction = (CImapAction*)m_array.GetAt(i);
		if (pImapAction && pImapAction->UsesMailbox(ulHashToMatch))
		{
			return pImapAction;
		}
	}

	return NULL;
}

//
//	CActionQueue::WriteQueue()
//
//	Write the queue to disk.
//
bool CActionQueue::WriteQueue()
{
	JJFileMT	file;

	CString		strTempFilePathName;
	HRESULT		hresult = GetTempFileName(m_strDirName, "queue", 0, strTempFilePathName.GetBuffer(MAX_PATH + 1));
	if (SUCCEEDED(hresult))
		strTempFilePathName.ReleaseBuffer();
	else
		strTempFilePathName = "queue-temp.tmp";

	hresult = file.Open(strTempFilePathName, O_CREAT|O_TRUNC|O_WRONLY);

	if (SUCCEEDED(hresult))
	{
		//	Open succeeded - write out the XML
		XMLWriter		xmlWriter(file);

		//	Start <IMAPQueueData>
		xmlWriter.WriteTagStart(CImapActionXMLParser::kXMLBaseContainer, true);

		//	Write DataFormatVersion tag
		xmlWriter.WriteTaggedData(CImapActionXMLParser::kKeyDataFormatVersion, true, "%d", kDataFormatVersion);

		int				 iSize = m_array.GetSize();
		CImapAction		*pImapAction = NULL;
		for (int i = 0; i < iSize; ++i)
		{
			pImapAction = (CImapAction*)m_array.GetAt(i);
			if (pImapAction)
			{
				//	Start <Action>
				xmlWriter.WriteTagStart(CImapActionXMLParser::kActionContainer, true);

				pImapAction->Write(&xmlWriter);

				//	Close </Action>
				xmlWriter.WriteTagEnd(CImapActionXMLParser::kActionContainer, true);
			}
		}

		//	Close </IMAPQueueData>
		xmlWriter.WriteTagEnd(CImapActionXMLParser::kXMLBaseContainer, true);

		//	Close the file
		hresult = file.Close();
		ASSERT(SUCCEEDED(hresult));

		hresult = file.Rename(m_strFileName);
		ASSERT(SUCCEEDED(hresult));

		m_bNeedsWrite = false;

		return true;
	}

	return false;
}

//
//	CActionQueue::ReadQueue()
//
//	Read the queue from disk.
//
bool CActionQueue::ReadQueue()
{
	//	Check to see if our file exists
	if (!::FileExistsMT(m_strFileName))
	{
		// Shoot.  We really thought we were going to find a file.
		return false;
	}

	//	Open our file
	int		hFile = open(m_strFileName, _O_RDONLY|_O_TEXT);
	if (hFile == -1)
	{
		ASSERT(!"Could not open IMAP action data file");		
		return false;
	}

	bool	bLoadGood = false;

	//	Allocated a buffer big enough for the entire file contents
	long					nLength = lseek(hFile, 0, SEEK_END);
	std::auto_ptr<char>		szFileBuf(DEBUG_NEW_NOTHROW char[nLength+1]);

	if (szFileBuf.get())
	{
		//	Load the entire file contents into our buffer
		lseek(hFile, 0, SEEK_SET);
		read(hFile, szFileBuf.get(), nLength);
		szFileBuf.get()[nLength] = 0;

		//	Parse the entire file in one fell swoop
		CImapActionXMLParser		imapActionXMLParser(this);
		bLoadGood = (imapActionXMLParser.Parse(szFileBuf.get(), nLength, true) == 0);
	}

	close(hFile);

	return true;
}

#endif // IMAP4
