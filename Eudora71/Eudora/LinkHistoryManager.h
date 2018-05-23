//	LinkHistoryManager.h
//
//	Manages persistent list of URLs that the user has launched (or ads displayed).
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



#ifndef __LinkHistoryManager_H__
#define __LinkHistoryManager_H__


#include <set>

#include "QCGraphics.h"
#include "QCImageList.h"


class JJFile;


class LinkHistoryManager
{
public:
	static const long	kSecondsPerDay;
	static const long	kPurgeFrequencySecs;
	static const char *	kEndOfEntryString;
	static const long	kEndOfEntryStrLen;
	
	typedef enum
	{
		lt_Ad = 0,
		lt_Http,
		lt_Ftp,
		lt_Mail,
		lt_DirectoryServices,
		lt_Setting,
		lt_File,
		lt_Unknown
	} LinkTypeEnum;

	//	Note: ls_Removed is only used for ads. It allows us to not list an ad that the user
	//	has "removed", while still keeping the information around. The problem is that we only
	//	know the link name for the ad when it is added (i.e. each time it is displayed).
	//	If the user clicks on the ad after removing it from the link history, we don't know
	//	the link name at the time of the click. This is worked around by never actually
	//	removing the ad - that way we know we can match it when the user clicks on it.
	//	(The timing under which this could occur is now exceedingly rare - the user would
	//	have to remove the ad from the link history after it displays and click on it before
	//	it displays again. Since it is still possible - I think it's worth leaving the code
	//	in to deal with this.).
	typedef enum
	{
		ls_Remind = 10,
		ls_Bookmarked = 20,
		ls_Attempted = 30,
		ls_None = 40,
		ls_NotVisited = 50,
		ls_Removed = 60,	//	Only used for ads
		ls_Unknown = 70
	} LinkStatusEnum;

	typedef enum
	{
		ola_Ask = 0,
		ola_Launch,
		ola_Bookmark,
		ola_Remind,
		ola_Cancel
	} OfflineLinkActionEnum;

	typedef enum
	{
		ra_Show,
		ra_Snooze,
		ra_Forget
	} RemindActionEnum;

	enum
	{
		kLinkTypeIconSize = 32,
		kLinkTypeIconPaddedHeight = 34
	};

	class UrlItemData
	{
	public:
						UrlItemData();
						UrlItemData(
							LinkTypeEnum		type,
							LPCSTR				szFindString);
						UrlItemData(
							LPCSTR				szLinkName,
							LPCSTR				szURL,
							LPCSTR				szAdID,
							LPCSTR				szThumbnailFileName,
							long				lLastVisitTime,
							long				lLastTouchTime,
							LinkTypeEnum		type,
							LinkStatusEnum		status);
						~UrlItemData() { }

		//	Common strings (although m_szLinkName is currently just used for ads)
		CString			m_szLinkName;
		CString			m_szURL;

		//	Ad specific strings
		CString			m_szThumbnailFileName;
		CString			m_szAdID;

		//	Time values for sorting and purging
		unsigned long	m_lLastVisitTime;
		unsigned long	m_lLastTouchTime;
			//	m_lLastTouchTime is updated any time a link is modified.
			//	Used to determine which links are old enough to expire.
		LinkTypeEnum	m_type;
		LinkStatusEnum	m_status;

		//	Non-persistent data
		int				m_nIconIndex;
	};

	class LessUrlItemDataByKey
	{
	public:
		bool			operator()(const UrlItemData * lhs, const UrlItemData * rhs) const;
	};

	class LessCString
	{
	public:
		bool			operator()(const CString & lhs, const CString & rhs) const
							{
								return (lhs.CompareNoCase(rhs) < 0);
							}
	};

	typedef UrlItemData *			LPUrlItemData;

	static bool						Initialize();
	static bool						Destroy();
	static LinkHistoryManager *		Instance()
										{
											ASSERT(s_pLinkHistoryManager);
											return s_pLinkHistoryManager;
										}

	bool							Save();
	bool							Load();
	bool							IsDirty() const { return m_bIsDirty; }
	void							SetDirtyState(bool bDirty = true) { m_bIsDirty = bDirty; }
	void							SetShouldRemind();
	void							CreateIconImageList();
	CImageList *					GetIconImageList();
	BOOL							Idle(
										unsigned long		currentTime,
										unsigned long		deltaTime);

	bool							IsTrackedLinkType(
										LinkTypeEnum		type);
	bool							ShouldLaunchURL(
										LPCSTR				szURL,
										bool				bIsAdURL,
										LPCSTR				szLinkText = NULL,
										const char *		szAdID = NULL);
	void							UpdateVisitedURLInfo(
										LPUrlItemData		pURL,
										BOOL				bLaunchGood);
	void							AddVisitedURL(
										const char *		szURL,
										BOOL				bLaunchGood,
										bool				bIsAdURL = false,
										const char *		szAdID = NULL);
	void							AddBookmarkedURL(
										const char *		szURL,
										LinkStatusEnum		ls_BookmarkOrRemind,
										bool				bIsAdURL,
										const char *		szAdID = NULL);
	void							AddAdToLinkHistory(
										const char *		szName,
										const char *		szImageFileURL,
										const char *		szURL,
										const char *		szAdID);
	void							RemoveURL(
										LPUrlItemData		pURLToDelete);
	void							NotifyLHViewReady();

protected:
	typedef std::set<LPUrlItemData, LessUrlItemDataByKey>	UrlListT;
	typedef std::set<CString, LessCString>					FileNameListT;
	typedef UrlListT::iterator								UrlListIteratorT;
	typedef FileNameListT::iterator							FileNameIteratorT;				

	enum
	{
		kLHOriginalFileVersionNoTouchDate = 1,
		kLHFileVersionLastPreThumbnail = 2,
		kLHFileVersionLastPreAdID = 3,
		kLHFileVersionLastPreCorrectedAdID = 4,
			//	kLHFileVersionLastPreCorrectedAdID stored the ad ID, but sometimes it was incorrectly
			//	an empty string. Read in the ad ID, but don't trust it - reparse it from the URL.
		kLHFileVersionLastPreBackwardFriendly = 5,
			//	Last version with spurious l's and no end of record marker (for downgrade support)
		kLHFileVersionFirstBackwardFriendly = 6,
		kLHFileVersionEudora43 = 6,
		kLHCurrentFileVersion = 6
	};

									LinkHistoryManager();
									~LinkHistoryManager();
	bool							ReadEntireLine(
										JJFile &			readFile,
										CString &			szRead,
										char *				szBuffer,
										long				lBufferSize);
	static LinkTypeEnum				ClassifyURL(
										LPCSTR				szURL);
	void							UpdateLHListDisplay(
										LPUrlItemData		pURLToAddOrUpdate,
										bool				bIsNewToList);
	void							LoadThumbnailIfNecessary(
										LPUrlItemData		pEntry);
	CBitmap *						MakeThumbnail(
										ImageFormatType		imageType,
										const char *		szImageFullPath,
										const char *		szThumbnailName);
	bool							FindUniqueThumbnailName(
										const char *		szImageFullPath,
										CString &			szThumbnailName);
	LPUrlItemData					InternalAddURL(
										LPCSTR				szLinkName,
										LPCSTR				szURL,
										LPCSTR				szAdID,
										long				lLastVisitTime,
										long				lLastTouchTime,
										LinkTypeEnum		type,
										LinkStatusEnum		status);
	void							InternalAddURL(
										LPUrlItemData		pURLToAdd);

	static LinkHistoryManager *		s_pLinkHistoryManager;
	UrlListT						m_UrlList;
	CString							m_szLHFilePathName;
	CString							m_szLHDirPath;
	long							m_LastPurgeTime;
	bool							m_bIsDirty;
	bool							m_bShouldRemind;
	unsigned long					m_lRemindSnoozeTime;
	QCImageList						m_IconImageList;
	bool							m_bIconImageListCreated;
};


#endif		//	__LinkHistoryManager_H__