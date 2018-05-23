//	SearchManager.h
//
//	Manages X1 indexed search.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#ifndef __SearchManager_H__
#define __SearchManager_H__

#pragma once

#include <deque>
#include <vector>
#include "QCCommandObject.h"

//	Forward declarations
class MultSearchCriteria;
class SearchCriteria;
class CSearchView;
class CSummary;
class CTocDoc;
namespace X1
{
	struct IX1Database;
	struct IX1ItemResult;
	struct IX1Search;
	enum X1FindType;
}
class X1EmailIndexItem;
class X1EmailScanner;


class SearchManager
{
  public:
	//	Forward declarations
	class IndexUpdateAction;
	class SavedSearchCommand;

	//	Typedefs
	typedef enum
	{
		efid_Subject,
		efid_AllHeaders,
		efid_Body,
		efid_MailboxRelativePath,
		efid_MailboxName,
		efid_MessageID,
		efid_Who,
		efid_FromHeader,
		efid_ToHeader,
		efid_CcHeader,
		efid_BccHeader,
		efid_Attachments,
		efid_DateSeconds,
		efid_DateDaysRomanNumeral,
		efid_State,
		efid_Label,
		efid_PersonaHash,
		efid_PriorityNumber,
		efid_PriorityRomanNumeral,
		efid_JunkScoreNumber,
		efid_JunkScoreRomanNumeral,
		efid_AttachmentCountNumber,
		efid_AttachmentCountRomanNumeral,
		efid_SizeNumber,
		efid_SizeRomanNumeral,
		efid_ImapFlags,
		efid_Mood,
		efid_TimeZoneMinutes,

		//	efid_SubjectPrefix + efid_SubjectMain = efid_Subject
		efid_SubjectPrefix,
		efid_SubjectMain,

		efid_LastField,
		efid_X1ColumnInfo = efid_LastField,
		efid_TotalNumFields = efid_LastField + 1
	} EmailFieldIDsEnum;

	typedef enum
	{
		rn_LastDigitIsOne,
		rn_LastDigitIsTen,
		rn_LastDigitIsHundred,
		rn_LastDigitIsThousand,
		rn_LastDigitIsUnsupoorted
	} RomanNumeralsEnum;

	typedef enum
	{
		st_Numeric,
		st_Text,
		st_X1Query
	} SearchTypeEnum;

	typedef enum
	{
		ua_Invalid = 0,
		ua_FirstValidAction,
		ua_FirstMessageAction = ua_FirstValidAction,
		ua_RemoveMessage = ua_FirstMessageAction,
		ua_AddMessage,
		ua_UpdateMessage,
		ua_LastMessageAction = ua_UpdateMessage,
		ua_FirstMailboxAction,
		ua_RemoveMailbox = ua_FirstMailboxAction,
		ua_AddMailbox,
		ua_ReindexMailbox,
		ua_LastValidAction = ua_ReindexMailbox
	} UpdateActionEnum;

	typedef std::vector<SavedSearchCommand *>	SavedSearchList;

	//	Constants
	static const long					kRomanNumeralMaxPossibleValue = 39999;
	static const long					kMaxPossibleJunkScore = 100;
	static const long					kMaxNumAttachments = 200;
	static const char *					kMailboxRelativePathRootTag;

	//	Internal classes
	class SavedSearchCommand : public QCCommandObject
	{
	  public:
							SavedSearchCommand(
								QCCommandDirector *		in_pCommandDirector,
								const char *			in_szFileName,
								const char *			in_szFileTitle)
								:	QCCommandObject(in_pCommandDirector),
									m_szFileName(in_szFileName),
									m_szFileTitle(in_szFileTitle),
									m_dwCommandID(0) {}
							~SavedSearchCommand() { }

		//	Accessors
		const char *		GetFileName() const { return m_szFileName; }
		const char *		GetFileTitle() const { return m_szFileTitle; }
		WORD				GetCommandID() const { return m_dwCommandID; }
		void				SetCommandID(WORD in_dwCommandID) { m_dwCommandID = in_dwCommandID; }

		virtual void		Execute(
								COMMAND_ACTION_TYPE		theAction,
								void *					pData = NULL) {}

	  protected:
		CString				m_szFileName;
		CString				m_szFileTitle;
		WORD				m_dwCommandID;
	};

	//	Static methods
	static bool							Initialize();
	static bool							Destroy();
	static SearchManager *				Instance()
	{
		ASSERT(s_pSearchManager);
		return s_pSearchManager;
	}

	//	Accessors
	X1::IX1Database *					GetX1EmailDB();
	X1::IX1Search *						GetX1Search();
	bool								IsInitialIndexingComplete() const;
	bool								IsIndexedSearchOn() const;
	bool								ShouldUseIndexedSearch() const;

	//	Saved Search List methods
	SavedSearchList &					GetSavedSearchesList();
	void								UpdateSavedSearchList();

	//	Idle methods
	BOOL								SaveIfDirty();
	BOOL								IdleStartScanningIfNeeded();
	BOOL								Idle(
											unsigned long				in_nDeltaTime,
											bool						in_bIsExtendedIdleProcessing,
											bool						in_bMustDoScanning = false);

	//	Index update action methods
	bool								HasIndexUpdateActions() const;
	long								GetNumberIndexUpdateActions() const;
	bool								HasMailboxUpdateActions() const;
	IndexUpdateAction *					GetFirstIndexUpdateAction();
	void								PopIndexUpdateAction();
	void								AddIndexUpdateAction(
											UpdateActionEnum			in_eUpdateAction,
											const CString &				in_szMailboxPath,
											long						in_nMessageID);

	void								AddSortColumn(
											int							in_nSortBy);

	//	Scanning
	bool								IsDoingFullScan() const;
	void								StartScanning();
	void								StopScanning();
	void								ReIndex();
	void								SetPauseExtendedIdleProcessing(
											bool						in_bPauseExtendedIdleProcessing);

	//	Notification methods
	void								OnActivateApp(
											BOOL						in_bActive);
	void								NotifyIniChange(
											UINT						in_nStringNum);
	void								NotifySumAddition(
											const CTocDoc *				in_pTocDoc,
											const CSummary *			in_pSum);
	void								NotifySumRemoval(
											const CTocDoc *				in_pTocDoc,
											POSITION					in_posSum,
											const CSummary *			in_pSum);
	void								NotifySumModification(
											const CTocDoc *				in_pTocDoc,
											const CSummary *			in_pSum);
	void								NotifyTemporarilyCloseMailbox(
											const char *				in_szMailboxFullPath);
	void								NotifyRestartProcessingMailbox(
											const char *				in_szMailboxFullPath);
	void								NotifyStopProcessingMailbox(
											const char *				in_szMailboxFullPath);
	void								NotifyMailboxPathNameChange(
											const char *				in_szOldMailboxPathName,
											const char *				in_szNewMailboxPathName);
	void								NotifyMailboxAdded(
											const char *				in_szMailboxFullPath);
	void								NotifyMailboxDeleted(
											const char *				in_szMailboxFullPath);
	void								NotifyReindexMailbox(
											const char *				in_szMailboxFullPath);

	void								NotifyX1Exception();
	void								NotifyIndexingStatus(
											int							in_nCurrentScanType,
											int							in_nNextScanType,
											const char *				in_szIndexingMailboxRelativePath);

	//	Searching methods
	void								DoSearch(
											MultSearchCriteria &		in_searchCriteria,
											std::deque<CString> &		in_listCheckedMailboxes,
											std::deque<CString> &		in_listUncheckedMailboxes,
											CSearchView *				in_pSearchView,
											unsigned int				in_nResultsFrequency,
											bool						in_bIncludeDeleteIMAPMessages);

  protected:
	//	Forward declarations
	class Info;
	class IndexChange;
	class UpdateIdentifier;
	class Searching;
	class Utils;
	class XMLParser;

	//	Friends
	friend X1EmailIndexItem;
	friend X1EmailScanner;
	friend XMLParser;

										SearchManager();
										~SearchManager();

	void								NotifyX1SchemaVersion(
											long						in_nSchemaVersion);

	static SearchManager *				s_pSearchManager;

	Info *								m_pInfo;
	bool								m_bInitializedSavedSearchesList;
	SavedSearchList						m_listSavedSearches;
};


#endif		//	__SearchManager_H__
