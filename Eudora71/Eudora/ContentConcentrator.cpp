//	ContentConcentrator.cpp
//
//	Concentrates messages into a shorter more easily readable form.
//
//	Copyright (c) 2003 by QUALCOMM, Incorporated
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
#include "ContentConcentrator.h"
#include "ConConMessage.h"
#include "ConConProfile.h"

#include <deque>

#include "fileutil.h"
#include "summary.h"
#include "tocdoc.h"
#include "QCSharewareManager.h"

#include "DebugNewHelpers.h"


//	Static pointer to the one and only instance of the Content Concentrator
ContentConcentrator *			ContentConcentrator::s_pContentConcentrator = NULL;


// ------------------------------------------------------------------------------------------
//		* Initialize														 [Static Public]
// ------------------------------------------------------------------------------------------
//	Use to create and initialize the ContentConcentrator.

bool
ContentConcentrator::Initialize()
{
	TRACE("ContentConcentrator::Initialize()\n");
	ASSERT(!s_pContentConcentrator);

	if (s_pContentConcentrator)
		delete s_pContentConcentrator;

	s_pContentConcentrator = DEBUG_NEW_NOTHROW ContentConcentrator;

	if (s_pContentConcentrator)
		s_pContentConcentrator->Load();

	return (s_pContentConcentrator != NULL);
}


// ------------------------------------------------------------------------------------------
//		* Destroy															 [Static Public]
// ------------------------------------------------------------------------------------------
//	Use to destroy the ContentConcentrator before quitting.

bool
ContentConcentrator::Destroy()
{
	TRACE("ContentConcentrator::Destroy()\n");

	if (s_pContentConcentrator)
	{
		delete s_pContentConcentrator;
		s_pContentConcentrator = NULL;
	}

	return true;
}


// ------------------------------------------------------------------------------------------
//		* ContentConcentrator													 [Protected]
// ------------------------------------------------------------------------------------------
//	ContentConcentrator constructor, protected so the client programmer is forced
//	to use Initialize.

ContentConcentrator::ContentConcentrator()
	:	m_profiles(), m_pLastTocDoc(NULL), m_pLastMailboxCommand(NULL)
{

}


// ------------------------------------------------------------------------------------------
//		* ~ContentConcentrator													 [Protected]
// ------------------------------------------------------------------------------------------
//	ContentConcentrator destructor, protected so the client programmer is forced
//	to use Destroy.

ContentConcentrator::~ContentConcentrator()
{
	//	Delete m_profiles items, but don't bother erasing the list because it is
	//	about to be destroyed anyway.
	Profile *		pProfile;
	for ( ProfileIteratorT profileIterator = m_profiles.begin();
		  profileIterator != m_profiles.end();
		  profileIterator++ )
	{
		pProfile = (*profileIterator).second;
		delete pProfile;
	}
}


// ------------------------------------------------------------------------------------------
//		* GetProfileNames															[Public]
// ------------------------------------------------------------------------------------------
//	Fills a CStringList with our profile names.
//
//	Parameters:
//		out_profileNames:		Place to return our profile names

void
ContentConcentrator::GetProfileNames(
	CStringList &			out_profileNames) const
{
	Profile *		pProfile;
	for ( ProfileIteratorT profileIterator = m_profiles.begin();
		  profileIterator != m_profiles.end();
		  profileIterator++ )
	{
		pProfile = (*profileIterator).second;

		//	We are careful to avoid empty members, but make sure that we don't have
		//	one just to be sure
		ASSERT(pProfile);
		if (pProfile)
			out_profileNames.AddHead( pProfile->GetName() );
	}
}


// ------------------------------------------------------------------------------------------
//		* GetIniIDForContext													 [Public]
// ------------------------------------------------------------------------------------------
//	Gets the profile INI ID for the specified context.
//
//	Parameters:
//		in_context:						Concentration context
//
//	Returns:
//		INI ID for the profile string for the specified context

UINT
ContentConcentrator::GetIniIDForContext(
	ContextT				in_context) const
{
	UINT				nProfileNameIniID = 0;
	
	//	Determine correct profile cache and Ini ID for given context
	switch (in_context)
	{
		case kCCPreviewContext:
			nProfileNameIniID = IDS_INI_CC_PREVIEW_PROFILE;
			break;
			
		case kCCFullViewContext:
			nProfileNameIniID = IDS_INI_CC_MESSAGE_WIN_PROFILE;
			break;
			
		case kCCMultipleContext:
			nProfileNameIniID = IDS_INI_CC_MULTIPLE_PROFILE;
			break;
			
		case kCCMultipleReplyContext:
			nProfileNameIniID = IDS_INI_CC_MULTIPLE_REPLY_PROFILE;
			break;
		
		//	No profile for kCCFullViewContextWithoutConcentration or kCCNoConcentrationContext
	}
	
	ASSERT(nProfileNameIniID != 0);

	return nProfileNameIniID;
}


// ------------------------------------------------------------------------------------------
//		* GetProfileIniString													 [Public]
// ------------------------------------------------------------------------------------------
//	Gets the profile INI string for the specified context.
//
//	Parameters:
//		in_context:						Concentration context
//		out_szProfileIniString			CString storage in which to return the profile name
//
//	Returns:
//		Whether or not it found a non-empty profile string for the specified context

bool
ContentConcentrator::GetProfileIniString(
	ContextT				in_context,
	CString &				out_szProfileIniString) const
{
	UINT				nProfileNameIniID = GetIniIDForContext(in_context);

	if (nProfileNameIniID != 0)
	{
		//	Retrieve Ini setting for specfied context
		GetIniString(nProfileNameIniID, out_szProfileIniString);
	}

	return (nProfileNameIniID != 0) && !out_szProfileIniString.IsEmpty();
}


// ------------------------------------------------------------------------------------------
//		* Load																	 [Protected]
// ------------------------------------------------------------------------------------------
//	Loads the Content Concentrator profiles from "EudoraCCProfiles.xml".
//	Note that the failure of this method will not prevent Initialize
//	from suceeding (Initialize currently ignores the return value).

bool
ContentConcentrator::Load()
{
	//	Form the path to our file in our data directory
	CString		szCCFilePathName = EudoraDir + CRString(IDS_CC_PROFILES_FILENAME);
	
	//	Check to see if our file exists
	if ( !::FileExistsMT(szCCFilePathName) )
	{
		//	It wasn't in our data directory
		if (EudoraDir.CompareNoCase(ExecutableDir) == 0)
		{
			//	Our app directory is the same - bail
			return false;
		}
		else
		{
			//	Our app directory is different - let's try there
			szCCFilePathName = ExecutableDir + CRString(IDS_CC_PROFILES_FILENAME);

			//	Ok last chance - does our file exist?
			if ( !::FileExistsMT(szCCFilePathName) )
				return false;
		}
	}

	//	Open our file
	int		hFile = open(szCCFilePathName, _O_RDONLY | _O_TEXT);
	if (hFile == -1)
		return false;

	bool	bLoadGood = false;

	//	Allocated a buffer big enough for the entire file contents (only a few K)
	long	nLength = lseek(hFile, 0, SEEK_END);
	char *	szCCFileBuf = DEBUG_NEW_NOTHROW char [nLength+1];

	if (szCCFileBuf)
	{
		//	Load the entire file contents into our buffer
		lseek(hFile, 0, SEEK_SET);
		read(hFile, szCCFileBuf, nLength);
		szCCFileBuf[nLength] = 0;

		//	Parse the entire file in one fell swoop
		CCXMLParser		ccXMLParser;
		bLoadGood = (ccXMLParser.Parse(szCCFileBuf, nLength, true) == 0);

		delete [] szCCFileBuf;
	}

	close(hFile);

	return bLoadGood;
}


// ------------------------------------------------------------------------------------------
//		* AddProfile															 [Protected]
// ------------------------------------------------------------------------------------------
//	Adds new profile to list of profiles.
//
//	Parameters:
//		in_szName:					Name of new profile
//
//	Returns:
//		Pointer to the profile added. Note that ContentConcentrator continues to own this
//		object, and that it must *not* be deleted.

ContentConcentrator::Profile *
ContentConcentrator::AddProfile(
	const char *			in_szName)
{
	Profile *	pProfile = DEBUG_NEW_NOTHROW Profile(in_szName);

	if (pProfile)
		m_profiles[ CString(in_szName) ] = pProfile;

	return pProfile;
}


// ------------------------------------------------------------------------------------------
//		* GetProfileForContext													 [Protected]
// ------------------------------------------------------------------------------------------
//	Outputs the message.
//
//	Parameters:
//		in_context:						Concentration context
//		in_pSummary:					Pointer to CSummary so that we can determine
//										if the default profile for the given context
//										has been overridden
//		in_pTocDoc:						Pointer to CTocDoc, which can function as an
//										alternate way to determine if the default profile
//										for the given context has been overridden. (Only
//										good for preview contexts).

ContentConcentrator::Profile *
ContentConcentrator::GetProfileForContext(
	ContextT				in_context,
	CSummary *				in_pSummary,
	CTocDoc *				in_pTocDoc) const
{
	Profile *		pProfile = NULL;
	
	if ( (in_context != kCCNoConcentrationContext) &&
		 (in_context != kCCFullViewContextWithoutConcentration) )
	{
		//	Load the string "None" so that we can check to see if that's the current profile
		//	(we don't rely on not finding the profile name because we want to make sure that
		//	"None" is special no matter what - i.e. even if a profile with that name is added)
		CRString			szNoneProfileName(IDS_CON_CON_PROFILE_NONE);
		CTocDoc *			pTocDoc = in_pTocDoc;
		bool				bOverridingProfileSpecified = false;

		//	Toc doc for specified summary overrides any specified otherwise
		if (in_pSummary && in_pSummary->m_TheToc)
			pTocDoc = in_pSummary->m_TheToc;
		
		if (pTocDoc)
		{
			bool	bIsMultipleSelection = false;
			
			//	Check for overriding profile specified by TOC
			if (in_context == kCCPreviewContext)
			{
				bOverridingProfileSpecified = pTocDoc->IsSingleSelectionProfileOverridden();
			}
			else if (in_context == kCCMultipleContext)
			{
				bOverridingProfileSpecified = pTocDoc->IsMultipleSelectionProfileOverridden();
				bIsMultipleSelection = true;
			}
				
			if (bOverridingProfileSpecified)
			{
				const CString &		szOverridingProfileName = bIsMultipleSelection ?
															  pTocDoc->GetMultipleSelectionProfileName() :
															  pTocDoc->GetSingleSelectionProfileName();

				if (szOverridingProfileName.CompareNoCase(szNoneProfileName) == 0)
				{
					//	No profile because "None" is specified
					pProfile = NULL;
				}
				else
				{
					pProfile = m_profiles[szOverridingProfileName];
					if (!pProfile)
					{
						//	Checking for a non-existent profile will add an empty member - remove it
						m_profiles.erase(szOverridingProfileName);
						
						if (in_context == kCCPreviewContext)
							pTocDoc->ResetSingleSelectionProfileName();
						else	//	(in_context == kCCMultipleContext)
							pTocDoc->ResetMultipleSelectionProfileName();
						bOverridingProfileSpecified = false;
					}
				}
			}
		}

		if (!bOverridingProfileSpecified)
		{
			//	Retrieve current Ini setting
			CString			szCurrentProfileName;

			if ( GetProfileIniString(in_context, szCurrentProfileName) )
			{
				if (szCurrentProfileName.CompareNoCase(szNoneProfileName) == 0)
				{
					//	No profile because "None" is specified
					pProfile = NULL;
				}
				else
				{
					//	Find the named profile
					pProfile = m_profiles[szCurrentProfileName];
					
					if (!pProfile)
					{
						//	User somehow set a profile that we don't know about - bounce the setting to "None"
						ASSERT(0);

						//	Checking for a non-existent profile will add an empty member - remove it
						m_profiles.erase(szCurrentProfileName);

						UINT	nProfileNameIniID = GetIniIDForContext(in_context);

						if (nProfileNameIniID != 0)
							SetIniString(nProfileNameIniID, szNoneProfileName);
					}
				}
			}
		}
	}

	return pProfile;
}


// ------------------------------------------------------------------------------------------
//		* ConcentrateMessage														[Public]
// ------------------------------------------------------------------------------------------
//	Attempts to concentrate a message.
//
//	Parameters:
//		in_context:						Concentration context
//		in_pSummary:					Summary of message to concentrate
//		in_szOriginalMessage:			Original message to concentrate
//		out_szConcentratedMessage:		Concentrated version of message
//
//	Returns:
//		Whether or not concentration was performed.

bool
ContentConcentrator::ConcentrateMessage(
	ContextT				in_context,
	CSummary *				in_pSummary,
	const char *			in_szOriginalMessage,
	CString &				out_szConcentratedMessage)
{
	//	Assume that we didn't concentrate anything
	bool	bConcentrated = false;
	
	//	If our context is not to concentrate or we're running in Light mode
	//	- then just return that we didn't concentrate
	if ( (in_context == kCCNoConcentrationContext) || !UsingFullFeatureSet() )
		return false;

	Profile *	pProfile = GetProfileForContext(in_context, in_pSummary);

	if (pProfile)
	{
		//	Make sure that we were given an original message and
		//	that the user is in paid or sponsored mode
		if ( !in_szOriginalMessage || !UsingFullFeatureSet() )
			return false;
		
		Message		mess(in_pSummary, in_szOriginalMessage);
		
		if ( mess.ParseMessage() )
		{
			//	Pre-allocate enough space for the original message for better efficiency
			out_szConcentratedMessage.GetBuffer( strlen(in_szOriginalMessage) );

			//	Output the message
			bConcentrated = mess.OutputMessage(pProfile, out_szConcentratedMessage);

			//	Free any extra buffer space that we have leftover
			out_szConcentratedMessage.FreeExtra();
		}
	}

	return bConcentrated;
}
