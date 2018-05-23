//	ContentConcentrator.h
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



#ifndef __ContentConcentrator_H__
#define __ContentConcentrator_H__


#include <map>
#include <memory>		//	For auto_ptr


//	Forward declarations
class CSummary;
class CTocDoc;
class QCMailboxCommand;

class ContentConcentrator
{
  public:
	typedef enum
	{
		kCCNoConcentrationContext,
		kCCPreviewContext,
		kCCFullViewContext,
		kCCFullViewContextWithoutConcentration,
		kCCMultipleContext,
		kCCMultipleReplyContext
	} ContextT;

	static bool						Initialize();
	static bool						Destroy();
	static ContentConcentrator *	Instance()
										{
											ASSERT(s_pContentConcentrator);
											return s_pContentConcentrator;
										}

	void							GetProfileNames(
											CStringList &			out_profileNames) const;

	UINT							GetIniIDForContext(
											ContextT				in_context) const;

	bool							GetProfileIniString(
											ContextT				in_context,
											CString &				out_szProfileIniString) const;

	bool							ShouldConcentrate(
											ContextT				in_context,
											CSummary *				in_pSummary) const
												{ return (GetProfileForContext(in_context, in_pSummary) != NULL); }

	bool							ShouldConcentrate(
											ContextT				in_context,
											CTocDoc *				in_pTocDoc) const
												{ return (GetProfileForContext(in_context, NULL, in_pTocDoc) != NULL); }

	bool							ConcentrateMessage(
											ContextT				in_context,
											CSummary *				in_pSummary,			
											const char *			in_szOriginalMessage,
											CString &				out_szConcentratedMessage);

  protected:
	class CCXMLParser;
	class Profile;
	class Message;

	friend CCXMLParser;
	friend Message;
	friend Profile;

	typedef std::map<CString, Profile *>		ProfileMapT;
	typedef ProfileMapT::iterator				ProfileIteratorT;

									ContentConcentrator();
									~ContentConcentrator();

	bool							Load();

	Profile *						GetProfileForContext(
											ContextT				in_context,
											CSummary *				in_pSummary,
											CTocDoc *				in_pTocDoc = NULL) const;

	Profile *						AddProfile(
											const char *			in_szName);

	static ContentConcentrator *	s_pContentConcentrator;
	mutable ProfileMapT				m_profiles;
	CTocDoc *						m_pLastTocDoc;
	QCMailboxCommand *				m_pLastMailboxCommand;
};


#endif	//	__ContentConcentrator_H__