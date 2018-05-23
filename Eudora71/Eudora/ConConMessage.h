//	ConConMessage.h
//
//	Handles parsing and output of messages.
//	Only for use in ContentConcentrator files.
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



#ifndef __ConConMessage_H__
#define __ConConMessage_H__


#include "ContentConcentrator.h"
#include "ConConProfile.h"
#include "resource.h"
#include "rs.h"
#include "utils.h"

#include <deque>


class ContentConcentrator::Message
{
  public:
									Message(
											const CSummary *		in_pSummary,			
											const char *			in_szOriginalMessage);

	//	Accessors
	bool							IsHTML() const { return (m_styleType == IS_HTML); }
	bool							IsFlowed() const { return (m_styleType == IS_FLOWED); }
	bool							IsRich() const { return (m_styleType == IS_RICH); }
	bool							IsFixedPlain() const { return (m_styleType == IS_ASCII); }
	bool							IsPlain() const { return IsFlowed() || IsFixedPlain(); }
	bool							IsStyled() const { return IsHTML() || IsRich(); }

	bool							ParseMessage();

	bool							OutputMessage(
											Profile *				in_pProfile,
											CString &				out_szConcentratedMessage);

  protected:
	//	Typedef to hoist Rule into our namespace
	typedef ContentConcentrator::Profile::Rule Rule;

	//	Constants
	static const bool		kStripContent;
	static const bool		kCopyContent;
	static const char		kQuoteChar;
	static const char *		kSigSeparator;
	static const short		kSigSeparatorLength;
	static const char *		kMinDigestSeparator;
	static const short		kMinDigestSeparatorLength;
	static const short		kMaxDigestSeparatorLength;
	static const bool		kAllowReturns;
	static const bool		kDontAllowReturns;
	static const bool		kReverse;
	static const bool		kForward;
	static const bool		kStartTruncation;
	static const bool		kEndTruncation;
	static const ULONG		kStyledBody;

	class ParagraphInfo
	{
	  public:
		typedef enum
		{
			//	First the ones that don't correspond to anything
			type_Ordinary = 0,
			type_EntireMessageTags,
			type_Blank,
			type_White,

			//	Now the ones that correspond to a state or profile element name
			type_Attribution = IDS_CON_CON_TYPE_ATTRIBUTION,
			type_ForwardOn = IDS_CON_CON_TYPE_FORWARD_ON,
			type_ForwardOff = IDS_CON_CON_TYPE_FORWARD_OFF,
			type_QuoteOn = IDS_CON_CON_TYPE_QUOTE_ON,
			type_QuoteOff = IDS_CON_CON_TYPE_QUOTE_OFF,
			type_SignatureIntro = IDS_CON_CON_TYPE_SIG_INTRO,
			type_Attachment = IDS_CON_CON_TYPE_ATTACHMENT,
			type_Complete = IDS_CON_CON_TYPE_COMPLETE
		} ParaType;

										ParagraphInfo();
								
		//	Accessors			
		const char *					GetStart() const { return m_pStart; }
		void							SetStart(const char * in_pStart) { m_pStart = in_pStart; }
		const char *					GetStop() const { return m_pStop; }
		void							SetStop(const char * in_pStop) { m_pStop = in_pStop; }
		ParaType						GetType() const { return m_type; }
		void							SetType(ParaType in_type) { m_type = in_type; }
		bool							IsMundaneType() const { return (m_type == type_Ordinary) || (m_type == type_Blank) || (m_type == type_White); }
		short							GetNumQuoteChars() const { return m_nQuoteChars; }
		void							SetNumQuoteChars(short in_nNumQuoteChars) { m_nQuoteChars = in_nNumQuoteChars; }
		short							GetExcerptQuoteLevel() const { return m_nExcerptQuoteLevel; }
		void							SetExcerptQuoteLevel(short in_nExcerptQuoteLevel) { m_nExcerptQuoteLevel = in_nExcerptQuoteLevel; }
		short							GetTotalQuoteLevel() const { return static_cast<short>(m_nExcerptQuoteLevel + m_nQuoteChars); }
		bool							IsQuoted() const { return (GetTotalQuoteLevel() > 0); }
								
	  protected:				
		const char *					m_pStart;				//	Start of paragraph
		const char *					m_pStop;				//	End of paragraph
		short							m_nExcerptQuoteLevel;	//	Quote level
		short							m_nQuoteChars;			//	# of quote chars found (in addition to quote level, if any)
		ParaType						m_type;					//	Paragraph type
	};

	class Element
	{
	  public:
										Element();
								
		void							InitFromParagraph(
												const ParagraphInfo &		in_paragraph);

		bool							IsAddressHeader() const;
								
		//	Accessors			
		const char *					GetStart() const { return m_pStart; }
		void							SetStart(const char * in_pStart) { m_pStart = in_pStart; }
		const char *					GetStop() const { return m_pStop; }
		void							SetStop(const char * in_pStop) { m_pStop = in_pStop; }
		int								GetLength() const { return m_pStop - m_pStart; }
		const char *					GetTrimmedStart() const { return m_pTrimmedStart; }
		void							SetTrimmedStart(const char * in_pTrimmedStart) { m_pTrimmedStart = in_pTrimmedStart; }
		const char *					GetTrimmedStop() const { return m_pTrimmedStop; }
		void							SetTrimmedStop(const char * in_pTrimmedStop) { m_pTrimmedStop = in_pTrimmedStop; }
		ULONG							GetType() const { return m_type; }
		void							SetType(ULONG in_type) { m_type = in_type; }
		short							GetNumQuoteChars() const { return m_nQuoteChars; }
		void							SetNumQuoteChars(short in_nNumQuoteChars) { m_nQuoteChars = in_nNumQuoteChars; }
		short							GetExcerptQuoteLevel() const { return m_nExcerptQuoteLevel; }
		void							SetExcerptQuoteLevel(short in_nExcerptQuoteLevel) { m_nExcerptQuoteLevel = in_nExcerptQuoteLevel; }
		short							GetTotalQuoteLevel() const { return static_cast<short>(m_nExcerptQuoteLevel + m_nQuoteChars); }
		void							SetOutputQuoteIncrement(short in_nOutputQuoteIncrement) { m_nOutputQuoteIncrement = in_nOutputQuoteIncrement; }
		short							GetOutputQuoteLevel() const { return static_cast<short>(GetTotalQuoteLevel() + m_nOutputQuoteIncrement); }
		bool							IsQuoted() const { return (GetTotalQuoteLevel() > 0); }
		const CString &					GetHeaderName() const { return m_szHeaderName; }
		void							SetHeaderName(const CString & in_szHeaderName) { m_szHeaderName = in_szHeaderName; }
		void							SetHeaderName(UINT in_nID) { m_szHeaderName.LoadString(in_nID); }
		const Rule *					GetRule() const { return m_pRule; }
		void							SetRule(const Rule * in_pRule) { m_pRule = in_pRule; }
		bool							TrimmedTrailingParBreak() const { return m_bTrimmedTrailingParBreak; }
		void							SetTrimmedTrailingParBreak(bool in_bTrimmedTrailingParBreak) { m_bTrimmedTrailingParBreak = in_bTrimmedTrailingParBreak; }
										
	  protected:		
		const char *					m_pStart;						//	Start of message element
		const char *					m_pStop;						//	End of message element
		const char *					m_pTrimmedStart;				//	Start of message element after trimming
		const char *					m_pTrimmedStop;					//	End of message element after trimming
		ULONG							m_type;							//	Type of element found
		short							m_nExcerptQuoteLevel;			//	Quote level
		short							m_nQuoteChars;					//	# of quote chars found (in addition to quote level, if any)
		short							m_nOutputQuoteIncrement;		//	Quote level that we'll add while outputting as specified by the corresponding rule
		CString							m_szHeaderName;					//	Name of header, not including colon
		const Rule *					m_pRule;						//	Rule that corresponds to this element
		bool							m_bTrimmedTrailingParBreak;		//	During output we trimmed one or more trailing par breaks
	};

	void							DetermineEndOfNextParagrah();
	void							DetermineQuoteLevelForNextParagraph();

	bool							SubjectStartsWithFwd(
											const CSummary *		in_pSummary) const;
	bool							SubjectContainsDigest(
											const CSummary *		in_pSummary) const;

	static inline bool				IsReturn(char in_char) { return (in_char == '\r') || (in_char == '\n'); }
	static inline bool				IsCRLF(const char * pCheckForCRLF) { return (pCheckForCRLF[0] == '\r') && (pCheckForCRLF[1] == '\n'); }
	static inline bool				IsWordChar(char in_char) { return (isalnum((unsigned char)in_char) != 0); }
	static inline bool				IsWordOrAddressChar(char in_char, bool in_bAllowAddressChars);
	static inline bool				IsWhiteSpace(
											char					in_char,
											bool					in_bAllowReturns);
	static inline bool				IsSigSeparator(
											const char *			in_pStart,
											const char *			in_pEnd);

	//	These methods use paragraph syntax to tentatively identify message elements
	bool							IsBlank(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							IsAllWhiteSpace(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							IsAllQuoteChars(
											const char *			in_pStart,
											const char *			in_pStop) const;
	short							IsTag(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							IsAllEntireMessageTags(
											const char *			in_pStart,
											const char *			in_pStop) const;
	const char *					SkipTags(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeAttribution(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeQuoteOrForwardDividerLine(
											const char *			in_pStart,
											const char *			in_pStop,
											UINT					in_nStartStringResID,
											UINT					in_nStopStringResID) const;
	bool							LooksLikeForwardOn(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeForwardOff(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeQuoteOn(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeQuoteOff(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeDigestSeparator(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeSigIntro(
											const char *			in_pStart,
											const char *			in_pStop) const;
	bool							LooksLikeAttachmentLine(
											const char *			in_pStart,
											const char *			in_pStop) const;
				
	void							DetermineNextParagraphType();

	//	These methods determine if the current paragraph is a particular interesting element.
	//	They differ from the above LooksLike... methods because they take into account the
	//	context of the parser (i.e. grammar), whereas the LooksLike... methods look only at
	//	the syntax of the paragraph itself.
	bool							IsCurrentParagraphAttribution() const;
	bool							IsCurrentParagraphForwardOn() const;
	bool							IsCurrentParagraphForwardOff() const;
	bool							IsCurrentParagraphQuoteOn() const;
	bool							IsCurrentParagraphQuoteOff() const;
	bool							IsCurrentParagraphSigIntro() const;
	bool							IsCurrentParagraphAttachment() const;
				
	bool							AdvanceParagraph();
	void							PrepareNextParagraph();

	short							RuleMatchesElement(
											Rule *					in_pRule,
											Element &				in_element);
	Rule *							FindProfileRuleForElement(
											Element &				in_element,
											Profile *				in_pProfile);
	void							OutputElement(
											Element *				in_pPreviousElement,
											Element &				io_element,
											const Rule *			in_pRule,
											CString &				out_szConcentratedMessage);
	void							OutputLineBreakIfNeeded(
											Element *				in_pPreviousElement,
											Element &				in_element,
											CString &				out_szOutput);
	void							OutputLineBreak(
											short					in_nQuoteChars,
											CString &				out_szOutput);
	void							OutputQuoteChars(
											short					in_nQuoteChars,
											CString &				out_szOutput);
	const char *					FindCRLF(
											const char *			in_pStart,
											const char *			in_pStop,
											const char *			in_pLastLocation = NULL,
											bool					in_bReverse = false);
	const char *					FindParBreak(
											const char *			in_pStart,
											const char *			in_pStop,
											const char *			in_pLastLocation = NULL,
											bool					in_bReverse = false);
	int								CountQuoteCharsForSingleLine(
											const char *			in_pStart,
											const char *			in_pStop);
	int								CountQuoteCharsForMultipleLines(
											const char *			in_pStart,
											const char *			in_pStop);
	void							FixTrimEndpoints(
											Element &				io_element);
	bool							DetermineTruncationEndpoints(
											UINT					in_nOutputBytes,
											short					in_nQuoteIncrement,
											const Element &			in_element,
											const char *			in_pStart,
											const char *			in_pStop,
											const char *&			out_pStartTruncation,
											const char *&			out_pStopTruncation,
											CString &				out_szEllipsis);
	bool							IsTruncationCloseEnough(
											const char *			in_pStart,
											const char *			in_pStop,
											const char *			in_pTruncationLocation,
											const char *			in_pNominee,
											bool					in_bStartTruncation,
											int						in_nMaxVarianceMultiple = 5);
	const char *					FindCRLFEndpoint(
											const char *			in_pStart,
											const char *			in_pStop,
											bool					in_bReverse,
											const char *			in_pParBreak);
	bool							FixTruncationEndpointByParagraph(
											const char *			in_pStart,
											const char *			in_pStop,
											bool					in_bStartTruncation,
											const char *&			io_pTruncationLocation);
	bool							FixTruncationEndpointByWord(
											const char *			in_pStart,
											const char *			in_pStop,
											bool					in_bStartTruncation,
											bool					in_bIsAddressHeader,
											const char *&			io_pTruncationLocation,
											int *					out_nQuoteChars = NULL);
	void							CopyText(
											ULONG					in_elementType,
											const char *			in_pStart,
											const char *			in_pStop,
											bool					in_bStripContent,
											short					in_nQuoteIncrement,
											CString &				out_szOutput);
	void							CopyTextToCString(
											const char *			in_pStart,
											const char *			in_pStop,
											CString &				out_szOutput);

	int								m_styleType;				//	Style type - affects parsing
	const char *					m_szStartOriginalMessage;	//	Start of original full message
	const char *					m_szEndOriginalMessage;		//	Termination of original full message (i.e. points to '\0')
	ParagraphInfo					m_lastParagraph;			//	The last paragraph we saw while parsing
	ParagraphInfo					m_currentParagraph;			//	The current paragraph that we're parsing
	ParagraphInfo					m_nextParagraph;			//	The next paragraph that we'll parse
	bool							m_bShouldPresumeForward;	//	Are first-level quotes forwards?
	bool							m_bIsDigest;				//	Is the message likely a digest?
	ULONG							m_nState;					//	Current parsing state
	mutable const char *			m_pStartXFlowed;			//	Pointer to <x-flowed> or NULL
	mutable const char *			m_pEndXFlowed;				//	Pointer to </x-flowed> or NULL
	deque<Element>					m_elementQueue;
	CRString						m_szACTrailer;				//	Attachment Converted
	CRString						m_szECTrailer;				//	Embedded Content
	CRString						m_szFlowed;					//	x-flowed marker
};


// ------------------------------------------------------------------------------------------
//		* Message::IsWordOrAddressChar												[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated char is a word char or address char.
//
//	Parameters:
//		in_char:				Character to examine
//		in_bAllowAddressChars:	Are . and @ allowed?
//
//	Returns:
//		Whether or not indicated character is a whitespace.

bool
ContentConcentrator::Message::IsWordOrAddressChar(
	char					in_char,
	bool					in_bAllowAddressChars)
{
	//	Note that this is a lame definition of what makes up an address, but
	//	it's not meant to be perfect, just to improve things slightly.
	bool	bIsAddressChar = (in_char == '@') || (in_char == '.');
	
	return ( IsWordChar(in_char) || (in_bAllowAddressChars && bIsAddressChar) );
}


// ------------------------------------------------------------------------------------------
//		* Message::IsWhiteSpace														[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated char is a whitespace character.
//
//	Parameters:
//		in_char:			Character to examine
//		in_bAllowReturns:	Are \r & \n allowed?
//
//	Returns:
//		Whether or not indicated character is a whitespace.

bool
ContentConcentrator::Message::IsWhiteSpace(
	char					in_char,
	bool					in_bAllowReturns)
{
	//	Assume not whitespace
	bool	bIsWhiteSpace = false;

	if ( in_bAllowReturns || !IsReturn(in_char) )
		bIsWhiteSpace = (isspace((unsigned char)in_char) != 0);

	return bIsWhiteSpace;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsSigSeparator													[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text is a sig separator.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated character is a whitespace.

bool
ContentConcentrator::Message::IsSigSeparator(
	const char *			in_pStart,
	const char *			in_pStop)
{
	//	Check to make sure the length is correct
	bool	bIsSigSeparator = ((in_pStop - in_pStart) >= kSigSeparatorLength);

	if (bIsSigSeparator)
		bIsSigSeparator = (strncmp(in_pStart, kSigSeparator, kSigSeparatorLength) == 0);

	return bIsSigSeparator;
}


#endif	//	__ConConMessage_H__
