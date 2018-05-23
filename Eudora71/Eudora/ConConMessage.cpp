//	ConConMessage.cpp
//
//	Keeps track of profile information for use in ContentConcentrator.
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



#include "stdafx.h"

#include "ConConMessage.h"
#include "ConConProfile.h"

#include "summary.h"

#include "DebugNewHelpers.h"


//	Constants
const bool			ContentConcentrator::Message::kStripContent = true;
const bool			ContentConcentrator::Message::kCopyContent = false;
const char			ContentConcentrator::Message::kQuoteChar = '>';
const char *		ContentConcentrator::Message::kSigSeparator = "-- \r\n";
const short			ContentConcentrator::Message::kSigSeparatorLength = 5;
const char *		ContentConcentrator::Message::kMinDigestSeparator = "----";
const short			ContentConcentrator::Message::kMinDigestSeparatorLength = 4;
const short			ContentConcentrator::Message::kMaxDigestSeparatorLength = 40;
const bool			ContentConcentrator::Message::kAllowReturns = true;
const bool			ContentConcentrator::Message::kDontAllowReturns = false;
const bool			ContentConcentrator::Message::kReverse = true;
const bool			ContentConcentrator::Message::kForward = false;
const bool			ContentConcentrator::Message::kStartTruncation = true;
const bool			ContentConcentrator::Message::kEndTruncation = false;
const ULONG			ContentConcentrator::Message::kStyledBody = 100;


// ------------------------------------------------------------------------------------------
//		* MessageInfo::ParagraphInfo::ParagraphInfo									[Public]
// ------------------------------------------------------------------------------------------
//	ParagraphInfo constructor.

ContentConcentrator::Message::ParagraphInfo::ParagraphInfo()
	:	m_pStart(NULL), m_pStop(NULL), m_nExcerptQuoteLevel(0), m_nQuoteChars(0),
		m_type(type_Ordinary)
{

}


// ------------------------------------------------------------------------------------------
//		* Message::Element::Element													[Public]
// ------------------------------------------------------------------------------------------
//	Element constructor.

ContentConcentrator::Message::Element::Element()
	:	m_pStart(NULL), m_pStop(NULL), m_pTrimmedStart(NULL), m_pTrimmedStop(NULL),
		m_type(0), m_nExcerptQuoteLevel(0), m_nQuoteChars(0),
		m_szHeaderName(), m_pRule(NULL), m_bTrimmedTrailingParBreak(false)
{

}


// ------------------------------------------------------------------------------------------
//		* Message::Element::InitFromParagraph										[Public]
// ------------------------------------------------------------------------------------------
//	Initializes Element to paragraph and default values.

void
ContentConcentrator::Message::Element::InitFromParagraph(
	const ParagraphInfo &		in_paragraph)
{
	//	Initialize to paragraph values
	m_pStart = in_paragraph.GetStart();
	m_pTrimmedStart = m_pStart;
	m_pStop = in_paragraph.GetStop();
	m_pTrimmedStop = m_pStop;
	m_nExcerptQuoteLevel = in_paragraph.GetExcerptQuoteLevel();
	m_nQuoteChars = in_paragraph.GetNumQuoteChars();

	//	Initialize to default values
	m_type = 0;
	m_szHeaderName.Empty();
	m_pRule = NULL;
	m_bTrimmedTrailingParBreak = false;
}


// ------------------------------------------------------------------------------------------
//		* Message::Element::IsAddressHeader											[Public]
// ------------------------------------------------------------------------------------------
//	Determines whether or not the element is an address header.
//
//	Returns:
//		true:	Element is an address header
//		false:	Element is not an address header

bool
ContentConcentrator::Message::Element::IsAddressHeader() const
{
	bool	bIsAddressHeader = false;

	if (GetType() == IDS_CON_CON_TYPE_HEADER)
	{
		bIsAddressHeader = (m_szHeaderName.CompareNoCase("To") == 0);

		if (!bIsAddressHeader)
			bIsAddressHeader = (m_szHeaderName.CompareNoCase("From") == 0);

		if (!bIsAddressHeader)
			bIsAddressHeader = (m_szHeaderName.CompareNoCase("Cc") == 0);
	}

	return bIsAddressHeader;
}


// ------------------------------------------------------------------------------------------
//		* Message::Message															[Public]
// ------------------------------------------------------------------------------------------
//	Message constructor.
//
//	Parameters:
//		in_pSummary:			Summary of message to concentrate
//		in_szOriginalMessage:	Original message to concentrate

ContentConcentrator::Message::Message(
	const CSummary *		in_pSummary,			
	const char *			in_szOriginalMessage)
	:	m_styleType(IS_ASCII), m_szStartOriginalMessage(in_szOriginalMessage),
		m_szEndOriginalMessage( in_szOriginalMessage + strlen(in_szOriginalMessage) ),
		m_lastParagraph(), m_currentParagraph(), m_nextParagraph(),
		m_bShouldPresumeForward( SubjectStartsWithFwd(in_pSummary) ),
		m_bIsDigest( SubjectContainsDigest(in_pSummary) ),
		m_nState(IDS_CON_CON_TYPE_HEADER), m_pStartXFlowed(NULL), m_pEndXFlowed(NULL),
		m_szACTrailer(IDS_ATTACH_CONVERTED), m_szECTrailer(IDS_EMBEDDED_CONTENT),
		m_szFlowed(IDS_MIME_XFLOWED)
{
	m_styleType = IsFancy(in_szOriginalMessage);

	//	Setup information for next paragraph start and stop so that the first call
	//	to ContentConcentrator::Message::AdvanceParagraph will work correctly.
	m_nextParagraph.SetStart(in_szOriginalMessage);
	m_nextParagraph.SetStop(in_szOriginalMessage);
}


// ------------------------------------------------------------------------------------------
//		* Message::ParseMessage														[Public]
// ------------------------------------------------------------------------------------------
//	Parses message into paragraphs.

bool
ContentConcentrator::Message::ParseMessage()
{	
	//	Prime by loading up the first paragraph as the next paragraph to be processed
	AdvanceParagraph();

	bool				bParsingGood = true;
	bool				bNeedRescan = false;
	bool				bNeedQueue;

	//	Element currently being parsed
	Element				currentElement;

	while ( bParsingGood && (bNeedRescan || AdvanceParagraph()) )
	{
		bNeedRescan = false;
		bNeedQueue = true;

		//	Assume this element is this paragraph
		currentElement.InitFromParagraph(m_currentParagraph);

		//	Adjust the element
		switch (m_nState)
		{
			case IDS_CON_CON_TYPE_HEADER:
				{
					if ( (currentElement.GetLength() <= 2) &&
						 (m_currentParagraph.GetType() == ParagraphInfo::type_Blank) )
					{
						//	Message separator found
						currentElement.SetType(IDS_CON_CON_TYPE_SEPARATOR);

						if ( IsPlain() )
						{
							//	We're at the start of the body
							m_nState = IDS_CON_CON_TYPE_BODY_INITIAL;

							//	If we're dealing with Format=Flowed, then prepare the next
							//	paragraph again, now that we know that we're in the body.
							if ( IsFlowed() )
								PrepareNextParagraph();
						}
						else
						{
							//	Special case the handling of styled bodies
							m_nState = kStyledBody;

							//	Prepare the next paragraph again, now that we know that we're
							//	in the styled body.
							PrepareNextParagraph();
						}
					}
					else
					{
						//	We're in the headers. The next paragraph may belong with us, since the headers may be folded.
						//	Advance past any paragraphs that start with whitespace.
						while ( (m_nextParagraph.GetStop() > m_nextParagraph.GetStart()) &&
								IsWhiteSpace(*m_nextParagraph.GetStart(), kDontAllowReturns) )
						{
							//	Include it
							if ( !AdvanceParagraph() )
								break;	//	Stop if we run out of paragraphs
						}

						//	Point the end of the element at the end of the current paragraph
						currentElement.SetStop( m_currentParagraph.GetStop() );

						//	Ok, the entire header is gathered. Let's see what we have.
						currentElement.SetType(IDS_CON_CON_TYPE_HEADER);
						const char *	pHeaderColon = static_cast<const char *>( memchr(currentElement.GetStart(), ':', currentElement.GetLength()) );

						if ( pHeaderColon && (pHeaderColon < currentElement.GetStop()) )
						{
							//	We have a header name!
							CString		szHeaderName( currentElement.GetStart(), pHeaderColon - currentElement.GetStart() );
							szHeaderName.TrimLeft();
							szHeaderName.TrimRight();

							currentElement.SetHeaderName(szHeaderName);
						}
						else
						{
							//	Uh oh. No colon found, so this isn't really a header. We should doing something
							//	clever here to figure out what this is, like look ahead, behind, or over our right
							//	shoulder. For now, however, we're just going to whimper and drool on ourselves.
							ASSERT(0);
							currentElement.SetHeaderName(IDS_UNTITLED);

							//	It's possible that we've somehow accidently run into the body. That would be
							//	disastrous for styled messages for which we only parse the headers. Bail
							//	if we're styled.
							if ( IsStyled() )
								bParsingGood = false;
						}
					}
				}
				break;
			
			case IDS_CON_CON_TYPE_BODY_INITIAL:
				{
					if ( currentElement.IsQuoted() )
						m_nState = m_bShouldPresumeForward ? IDS_CON_CON_TYPE_FORWARD : IDS_CON_CON_TYPE_QUOTE;
					else if ( IsCurrentParagraphAttribution() )
						m_nState = IDS_CON_CON_TYPE_ATTRIBUTION;
					else if ( IsCurrentParagraphForwardOn() )
						m_nState = IDS_CON_CON_TYPE_FORWARD_ON;
					else if ( IsCurrentParagraphQuoteOn() )
						m_nState = IDS_CON_CON_TYPE_QUOTE_ON;
					else if ( IsCurrentParagraphSigIntro() )
						m_nState = IDS_CON_CON_TYPE_SIG_INTRO;
					else
						m_nState = IDS_CON_CON_TYPE_BODY;
					
					bNeedRescan = true;
					bNeedQueue = false;
				}
				break;

			case IDS_CON_CON_TYPE_PLAIN_FORWARD:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_PLAIN_FORWARD);

					//	We're in a plaintext quote - collect until we're not
					do
					{
						if ( IsCurrentParagraphForwardOff() )
						{
							m_nState = IDS_CON_CON_TYPE_FORWARD_OFF;
							bNeedRescan = true;
							break;
						}
					} while( AdvanceParagraph() );

					//	Current message element ends with the previous paragraph
					currentElement.SetStop( m_lastParagraph.GetStop() );

					//	If we never found the end of the forward section
					//	then we hit the end of the message
					if (m_nState == IDS_CON_CON_TYPE_PLAIN_FORWARD)
						m_nState = IDS_CON_CON_TYPE_COMPLETE;
				}
				break;

			case IDS_CON_CON_TYPE_OUTLOOK_QUOTE:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_OUTLOOK_QUOTE);
					
					//	We're in an outlook quote - collect until we're not
					do
					{
						if ( IsCurrentParagraphQuoteOff() )
						{
							m_nState = IDS_CON_CON_TYPE_QUOTE_OFF;
							bNeedRescan = true;
							break;
						}
						else if ( IsCurrentParagraphAttachment() )
						{
							m_nState = IDS_CON_CON_TYPE_BODY_INITIAL;
							bNeedRescan = true;
							break;
						}
					} while( AdvanceParagraph() );

					//	Current message element ends with the previous paragraph
					currentElement.SetStop( m_lastParagraph.GetStop() );

					//	If we never found the end of the quote section
					//	then we hit the end of the message
					if (m_nState == IDS_CON_CON_TYPE_OUTLOOK_QUOTE)
						m_nState = IDS_CON_CON_TYPE_COMPLETE;
				}
				break;

			case IDS_CON_CON_TYPE_FORWARD:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_FORWARD);

					//	Collect until the quote level falls below the current message element
					while ( (m_nextParagraph.GetTotalQuoteLevel() >= currentElement.GetTotalQuoteLevel()) &&
							AdvanceParagraph() )
					{
					}

					if (m_currentParagraph.GetType() == ParagraphInfo::type_Complete)
					{
						currentElement.SetStop( m_lastParagraph.GetStop() );
						m_nState = IDS_CON_CON_TYPE_COMPLETE;
					}
					else
					{
						currentElement.SetStop( m_currentParagraph.GetStop() );
						m_nState = IDS_CON_CON_TYPE_BODY;
					}
				}
				break;

			case IDS_CON_CON_TYPE_SIGNATURE:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_SIGNATURE);

					//	We're in a signature - collect until something happens
					while ( m_nextParagraph.IsMundaneType() && !m_nextParagraph.IsQuoted() && AdvanceParagraph() )
					{
					}

					if (m_currentParagraph.GetType() == ParagraphInfo::type_Complete)
					{
						currentElement.SetStop( m_lastParagraph.GetStop() );
						m_nState = IDS_CON_CON_TYPE_COMPLETE;
					}
					else
					{
						currentElement.SetStop( m_currentParagraph.GetStop() );
						m_nState = IDS_CON_CON_TYPE_QUOTE;
					}
				}
				break;

			case kStyledBody:
				{
					//	Set the current element to kStyledBody so we know to just output it
					currentElement.SetType(kStyledBody);
					
					//	The paragraph encompases the entire body of the styled message and
					//	we've already initialized the element from it, so we're done.
					m_nState = IDS_CON_CON_TYPE_COMPLETE;
				}
				break;

			case IDS_CON_CON_TYPE_BODY:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_BODY);

					//	We're in the body, in the original text. Collect original text forever.
					do
					{
						if ( IsCurrentParagraphForwardOn() )
						{
							m_nState = IDS_CON_CON_TYPE_FORWARD_ON;
							break;
						}
						if ( IsCurrentParagraphQuoteOn() )
						{
							m_nState = IDS_CON_CON_TYPE_QUOTE_ON;
							break;
						}
						if ( IsCurrentParagraphSigIntro() )
						{
							m_nState = IDS_CON_CON_TYPE_SIGNATURE;
							break;
						}
						if ( IsCurrentParagraphAttribution() )
						{
							m_nState = IDS_CON_CON_TYPE_ATTRIBUTION;
							break;
						}
					}
					while ( !m_nextParagraph.IsQuoted() && AdvanceParagraph() );

					if (m_nState == IDS_CON_CON_TYPE_BODY)
					{
						//	m_nState is still IDS_CON_CON_TYPE_BODY, so we stopped either because
						//	we hit quoted material or because we hit the end of the message.
						if ( m_nextParagraph.IsQuoted() )
						{
							currentElement.SetStop( m_currentParagraph.GetStop() );
							m_nState = m_bShouldPresumeForward ? IDS_CON_CON_TYPE_FORWARD : IDS_CON_CON_TYPE_QUOTE;
						}
						else
						{
							currentElement.SetStop( m_lastParagraph.GetStop() );
							m_nState = IDS_CON_CON_TYPE_COMPLETE;
						}
					}
					else
					{
						currentElement.SetStop( m_lastParagraph.GetStop() );
						bNeedRescan = true;
					}
				}
				break;


			case IDS_CON_CON_TYPE_ATTRIBUTION:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_ATTRIBUTION);
					m_nState = IDS_CON_CON_TYPE_QUOTE;
				}
				break;

			case IDS_CON_CON_TYPE_FORWARD_ON:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_FORWARD_ON);
					m_nState = IDS_CON_CON_TYPE_PLAIN_FORWARD;
				}
				break;

			case IDS_CON_CON_TYPE_QUOTE_ON:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_QUOTE_ON);
					m_nState = IDS_CON_CON_TYPE_OUTLOOK_QUOTE;
				}
				break;

			case IDS_CON_CON_TYPE_FORWARD_OFF:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_FORWARD_OFF);
					m_nState = IDS_CON_CON_TYPE_BODY_INITIAL;
				}
				break;

			case IDS_CON_CON_TYPE_QUOTE_OFF:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_QUOTE_OFF);
					m_nState = IDS_CON_CON_TYPE_BODY_INITIAL;
				}
				break;

			case IDS_CON_CON_TYPE_SIG_INTRO:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_SIG_INTRO);
					m_nState = IDS_CON_CON_TYPE_SIGNATURE;
				}
				break;

			case IDS_CON_CON_TYPE_QUOTE:
				{
					currentElement.SetType(IDS_CON_CON_TYPE_QUOTE);

					//	Advance until we're past the quoted section or we hit the end of the message
					while ( m_nextParagraph.IsQuoted() && AdvanceParagraph() )
					{
					}

					//	Did we hit the end of the quoted material or the end of the message?
					if (m_currentParagraph.GetType() == ParagraphInfo::type_Complete)
					{
						currentElement.SetStop( m_lastParagraph.GetStop() );
						m_nState = IDS_CON_CON_TYPE_COMPLETE;
					}
					else
					{
						currentElement.SetStop( m_currentParagraph.GetStop() );
						m_nState = IDS_CON_CON_TYPE_BODY_INITIAL;
					}
				}
				break;

			default:
				{
					//	We weren't supposed to get here
					ASSERT(0);
					bParsingGood = false;
				}
				break;
		}

		if ( bNeedQueue && (currentElement.GetStart() != currentElement.GetStop())  )
		{
			//	Put the current element on the queue
			m_elementQueue.push_back(currentElement);
		}
	}

	return bParsingGood;
}


// ------------------------------------------------------------------------------------------
//		* OutputMessage															 [Protected]
// ------------------------------------------------------------------------------------------
//	Outputs parsed message in concentrated form.
//
//	Parameters:
//		in_pProfile:					Profile to use for output
//		out_szConcentratedMessage:		Concentrated version of message

bool
ContentConcentrator::Message::OutputMessage(
	Profile *				in_pProfile,
	CString &				out_szConcentratedMessage)
{
	const Rule *				pRule;
	Element *					pPreviousElement = NULL;
	deque<Element>::iterator	elmIterator;
	bool						bFoundAllElements = true;
	bool						bFoundOneElement = false;
	ULONG						elementType;
	
	//	Look for a profile rule for each element
	for ( elmIterator = m_elementQueue.begin();
		  elmIterator != m_elementQueue.end();
		  elmIterator++ )
	{
		if ( (*elmIterator).GetType() == kStyledBody )
		{
			//	Styled bodies are a special case (for now). Don't bother trying
			//	to find a rule for them - we don't want anything to match them
			//	(like IDS_CON_CON_TYPE_ANY). We just want to output them.
			pRule = NULL;

			//	We also special case that we definitely found one element and
			//	we leave bFoundAllElements alone.
			bFoundOneElement = true;
		}
		else
		{
			//	Normal case - find the profile rule that corresponds to this element
			pRule = FindProfileRuleForElement(*elmIterator, in_pProfile);
			if (pRule)
				bFoundOneElement = true;
			else
				bFoundAllElements = false;
		}

		(*elmIterator).SetRule(pRule);
	}

	//	What should we do if we didn't always find a matching profile rule?!?
	//	Maybe we should display the element, but for now we'll just whine to the developer.
	ASSERT(bFoundAllElements);

	//	Continue provided we found at least one element, otherwise bail - there's something
	//	wrong with us or the current profile.
	if (bFoundOneElement)
	{
		//	Output each element according to profile rules
		for ( elmIterator = m_elementQueue.begin();
			  elmIterator != m_elementQueue.end();
			  elmIterator++ )
		{
			elementType = (*elmIterator).GetType();

			//	Set the state as we output so that methods like FindParBreak can know
			//	whether or not we're in headers.
			m_nState = (*elmIterator).GetType();

			if (m_nState == kStyledBody)
			{
				//	Styled bodies are a special case. We don't bother with any rule for them.
				//	We just output them in their entirety.
				OutputElement(pPreviousElement, *elmIterator, NULL, out_szConcentratedMessage);
			}
			else
			{
				pRule = (*elmIterator).GetRule();
				
				//	What do we do if we didn't find a matching profile rule?!?
				//	Maybe we should display the element, but for now we'll just whine to the developer.
				ASSERT(pRule);

				if (pRule)
				{
					OutputElement(pPreviousElement, *elmIterator, pRule, out_szConcentratedMessage);
					
					//	Remember the last element that we actually output
					if (pRule->GetOutputType() != IDS_CON_CON_OUT_TYPE_REMOVE)
						pPreviousElement = &(*elmIterator);
				}
			}
		}

		if ( IsPlain() )
		{			
			//	Trim whitespace off the end of the message (for better multiple message display
			//	and to avoid unnecessary scrolling area). Make sure that we don't trim
			//	off the header separator if the body is empty or all whitespace.
			int		nEndHeaders = out_szConcentratedMessage.Find("\r\n\r\n", 0);

			if (nEndHeaders > 0)
			{
				out_szConcentratedMessage.TrimRight();
				
				if (out_szConcentratedMessage.GetLength() <= nEndHeaders)
					out_szConcentratedMessage += "\r\n\r\n";
			}
		}
	}

	return bFoundOneElement;
}


// ------------------------------------------------------------------------------------------
//		* Message::DetermineQuoteLevelForNextParagraph								[Public]
// ------------------------------------------------------------------------------------------
//	Determines the quote level for the paragraph for either plain text or HTML.
//	This method is here rather than in ParagraphInfo because with HTML we need to track
//	the quote level spanning paragraphs.

void
ContentConcentrator::Message::DetermineQuoteLevelForNextParagraph()
{
	if ( IsStyled() )
	{
		//	To do: handle looking for start and end of HTML excerpt style quoting.
		//	So this means tracking whether or not we're inside of one or more levels
		//	of BLOCKQUOTE tags.
	}
	else
	{
		//	Look for quote characters. What does it mean if we find quote characters
		//	when we're dealing with HTML? Dunno, but Steve looks for quote characters
		//	even in presence of excerpting so I should too. Steve says:
		//		ok, now here is an ugliness; we may have quote characters on top of excerpt bars
		//		These may simply be characters the user has chosen to put in, or they may
		//		indicate that excerpt was not used consistently.  Bleah.
		//		in fullness of time, we may pay attention to the "lay of the land"
		//		in terms of whether or not this number of quote chars is used consistently
		//		from one para to the next.  For now, we're keeping it simple.
		const char *		pStop = m_nextParagraph.GetStop();
		const char *		pStartQuotes = SkipTags(m_nextParagraph.GetStart(), pStop);
		const char *		pEndQuotes = pStartQuotes;
		
		while ( (*pEndQuotes == kQuoteChar) && (pEndQuotes < pStop) )
			++pEndQuotes;
		
		m_nextParagraph.SetNumQuoteChars( static_cast<short>(pEndQuotes - pStartQuotes) );
	}
}


// ------------------------------------------------------------------------------------------
//		* Message::DetermineEndOfNextParagrah										[Public]
// ------------------------------------------------------------------------------------------
//	Find the end of the paragraph for either plain text or HTML.
//
//	Parameters:
//		in_szEndEntireMessage:	End of entire message

void
ContentConcentrator::Message::DetermineEndOfNextParagrah()
{
	const char *	pStop = m_nextParagraph.GetStart();
	
	if ( IsStyled() && (m_nState != IDS_CON_CON_TYPE_HEADER) )
	{
		//	We aren't handling the bodies of HTML messages yet, so suck up the entire
		//	message into this "paragraph". Eventaully we'll support HTML messages
		//	properly, but this is probably how we should always handle IsRich().
		pStop = m_szEndOriginalMessage;

		//	To do: handle looking for HTML paragraph breaks
		//	Off the top of my head this will include:
		//	BR P - what else?
	}
	else //	We're dealing with plain text, or we're still headers
	{
		//	Look for paragraph break accounting for Format=Flowed if necessary
		pStop = FindParBreak(m_nextParagraph.GetStart(), m_szEndOriginalMessage);

		if (pStop)	//	Skip past entire CRLF
			pStop += 2;
		else		//	Didn't find paragraph end so we're at the end of the message
			pStop = m_szEndOriginalMessage;
	}

	m_nextParagraph.SetStop(pStop);
}


// ------------------------------------------------------------------------------------------
//		* Message::SubjectStartsWithFwd												[Public]
// ------------------------------------------------------------------------------------------
//	Determines whether or not the subject looks like a forwarded subject.
//
//	Parameters:
//		in_pSummary:			Summary of message to concentrate
//
//	Returns:
//		Whether or not the subject starts with any string that indicates that the message
//		has been forwarded.

bool
ContentConcentrator::Message::SubjectStartsWithFwd(
	const CSummary *		in_pSummary) const
{
	if (!in_pSummary)
		return false;

	//	See if it matches any of the specified strings - passing 0 for the
	//	last parameter specifies that it only has to match up for the length
	//	of the resource string.
	int		nFoundStringIndex =
		FindRStringIndexI(IDS_CON_CON_FWD_SUBJECT_START_1, IDS_CON_CON_FWD_SUBJECT_START_2, in_pSummary->m_Subject, 0);
	
	//	We found a match if the returned index is not -1
	return (nFoundStringIndex != -1);
}


// ------------------------------------------------------------------------------------------
//		* Message::SubjectContainsDigest											[Public]
// ------------------------------------------------------------------------------------------
//	Determines whether or not the subject looks like the message is a digest.
//
//	Parameters:
//		in_pSummary:			Summary of message to concentrate
//
//	Returns:
//		Whether or not the subject contains the word "digest", which indicates that the
//		message might be a digest.

bool
ContentConcentrator::Message::SubjectContainsDigest(
	const CSummary *		in_pSummary) const
{
	if (!in_pSummary || !in_pSummary->m_Subject)
		return false;

	CString		szSubject(in_pSummary->m_Subject);
	CRString	szDigest(IDS_CON_CON_DIGEST_SUBJECT);

	szSubject.MakeLower();

	//	Check to see if the word "digest" appears in the subject
	int			nDigestStringLoc = szSubject.Find(szDigest);
	bool		bSubjectContainsDigest = (nDigestStringLoc != -1);

	//	If it appears in the subject, make sure that it's not part of a longer word.
	//	If "digest" is not at the beginning of the subject string, check the character
	//	preceeding the word "digest".
	if (nDigestStringLoc > 0)
		bSubjectContainsDigest = !IsWordChar(szSubject[nDigestStringLoc-1]);

	if (bSubjectContainsDigest)
	{
		//	If "digest" is not at the end of the subject string, check the character
		//	after the word "digest".
		int		nCharLocAfterDigestString = nDigestStringLoc + szDigest.GetLength();

		bSubjectContainsDigest = !IsWordChar(szSubject[nCharLocAfterDigestString]);
	}

	return bSubjectContainsDigest;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsBlank															[Public]
// ------------------------------------------------------------------------------------------
//	Determine if the indicated text is blank - i.e. without even meaningful whitespace.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text is blank.

bool
ContentConcentrator::Message::IsBlank(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume not all blank
	bool	bIsAllBlank = false;
	
	if ( IsPlain() || (m_nState == IDS_CON_CON_TYPE_HEADER) )
	{
		//	All blank with plain is simple - one or two return chars
		long	nLength = in_pStop - in_pStart;

		if (nLength == 1)
			bIsAllBlank = IsReturn(*in_pStart);
		else if (nLength == 2)
			bIsAllBlank = IsCRLF(in_pStart);
	}
	else if ( IsHTML() )
	{
		//	To do: scan to see if paragraph contains all tags with nothing to be displayed -
		//	i.e. no non-tags, and nbsp; tags
	}
	
	return bIsAllBlank;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsAllWhiteSpace													[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text is all whitespace.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text is all whitespace.

bool
ContentConcentrator::Message::IsAllWhiteSpace(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume not all whitespace
	bool	bIsAllWhiteSpace = false;
	
	if ( IsPlain() || (m_nState == IDS_CON_CON_TYPE_HEADER) )
	{
		//	For scan assume all whitespace
		bIsAllWhiteSpace = true;
		
		for (const char * pScan = in_pStart; pScan < in_pStop; pScan++)
		{
			if ( !IsWhiteSpace(*pScan, kAllowReturns) )
			{
				//	Not whitespace
				bIsAllWhiteSpace = false;
				break;
			}
		}
	}
	else if ( IsHTML() )
	{
		//	To do: scan to see if paragraph contains all tags with nothing to be displayed -
		//	or whitespace. Note that unlike IsBlank nbsp; tags are allowed here.
	}
	
	return bIsAllWhiteSpace;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsAllQuoteChars													[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text is all quote chars.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text is all quote chars.

bool
ContentConcentrator::Message::IsAllQuoteChars(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume not all quote chars
	bool	bIsAllQuoteChars = false;
	
	if ( IsHTML() )
	{
		//	To do: ???
	}
	else
	{
		//	For scan assume all quote chars
		bIsAllQuoteChars = true;
		
		for (const char * pScan = in_pStart; pScan < in_pStop; pScan++)
		{
			if (*pScan != kQuoteChar)
			{
				//	Not quote chars
				bIsAllQuoteChars = false;
				break;
			}
		}
	}
	
	return bIsAllQuoteChars;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsTag															[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text location starts with a tag.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text starts with a tag.

short
ContentConcentrator::Message::IsTag(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume no tag
	short		nTagLength = 0;
	
	if ( IsHTML() )
	{
		//	To do: Scan for type of HTML tags and absence of any other content
	}
	else if ( IsFlowed() && (*in_pStart == '<') )
	{		
		const char *	pCompare = in_pStart + 1;
		bool			bIsEndTag = (*pCompare == '/');

		if ( (!m_pStartXFlowed && !bIsEndTag) || (!m_pEndXFlowed && bIsEndTag) )
		{
			if (bIsEndTag)
				pCompare++;		//	Advance past the '/'
			
			int			nLength = in_pStop - pCompare - 1;
			
			if ( (nLength >= m_szFlowed.GetLength()) &&
				(strncmp(pCompare, m_szFlowed, m_szFlowed.GetLength()) == 0) &&
				(pCompare[m_szFlowed.GetLength()] == kQuoteChar) )
			{
				//	We can only see the x-flowed tags once - otherwise someone used them
				//	in the body of the message for some reason.
				if (bIsEndTag)
					m_pEndXFlowed = in_pStart;
				else
					m_pStartXFlowed = in_pStart;
			}
		}

		if (in_pStart == m_pStartXFlowed)
		{
			//	Total length is x-flowed + "<>"
			nTagLength = static_cast<short>( m_szFlowed.GetLength() + 2 );
		}
		else if (in_pStart == m_pEndXFlowed)
		{
			//	Total length is x-flowed + "</>"
			nTagLength = static_cast<short>( m_szFlowed.GetLength() + 3 );
		}
	}
	
	return nTagLength;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsAllEntireMessageTags											[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text is all special entire message tags.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text is all special entire message tags.

bool
ContentConcentrator::Message::IsAllEntireMessageTags(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume not all special entire message tags
	bool	bIsAllEntireMessageTags = false;
	
	if ( IsHTML() )
	{
		//	To do: Scan for type of HTML tags and absence of any other content
	}
	else if ( IsFlowed() )
	{
		bIsAllEntireMessageTags = (IsTag(in_pStart, in_pStop) > 0);
	}
	
	return bIsAllEntireMessageTags;
}


// ------------------------------------------------------------------------------------------
//		* Message::SkipTags															[Public]
// ------------------------------------------------------------------------------------------
//	Skip past any tags.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Pointer to text after tags (start if there are no tags and stop if the text is
//		all tags).

const char *
ContentConcentrator::Message::SkipTags(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Start at the very beginning (a very good place to start)
	const char *	pAfterTags = in_pStart;

	if ( IsHTML() )
	{
		//	To do: Scan past any HTML tags
	}
	else if ( IsFlowed() )
	{
		//	Skip past x-flowed tag
		pAfterTags += IsTag(in_pStart, in_pStop);
	}
	
	return pAfterTags;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeAttribution												[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like an attribution.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like an attribution.

bool
ContentConcentrator::Message::LooksLikeAttribution(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume not attribution
	bool	bLooksLikeAttribution = false;
	
	if ( IsHTML() )
	{
		//	To do:
		//	* Check to see if it's a reasonable length (minus tags - argh)
		//	* Must end with : and paragraph ending tag - i.e. BR or P
		//	* Must "begin" with a word character - after skipping any tags other than nbsp;
	}
	else
	{
		in_pStart = SkipTags(in_pStart, in_pStop);
		long	nLength = in_pStop - in_pStart;

		//	Make sure that it's a reasonable length
		if ( (nLength > 3) && (nLength < 128) )
		{
			const char *	pScan = in_pStop-2;

			//	Must end with : and return
			if ( IsCRLF(pScan) )
			{
				--pScan;
				if (*pScan == ':')
				{
					//	Must begin with a word character
					bLooksLikeAttribution = IsWordChar(*in_pStart);
				}
			}
		}
	}
	
	return bLooksLikeAttribution;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeQuoteOrForwardDividerLine								[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like a quote or forward divider line as specified by
//	the specified resource strings.
//
//	Parameters:
//		in_pStart:				Start of text to examine
//		in_pStop:				End of text to examine
//		in_nStartStringResID	First string to look for in line
//		in_nStopStringResID		Last string to look for in line
//
//	Returns:
//		Whether or not indicated text looks like a quote or forward boundary.

bool
ContentConcentrator::Message::LooksLikeQuoteOrForwardDividerLine(
	const char *			in_pStart,
	const char *			in_pStop,
	UINT					in_nStartStringResID,
	UINT					in_nStopStringResID) const
{
	//	Assume not quote or forward divider line
	bool	bLooksLikeQuoteOrForwardDividerLine = false;
		
	if ( IsHTML() )
	{
		//	To do:
		//	* Check to see if it's a reasonable length (minus tags - argh)
		//	* Skip tags, then do basically the same thing as below
	}
	else
	{
		in_pStart = SkipTags(in_pStart, in_pStop);
		long	nLength = in_pStop - in_pStart;

		//	Make sure that it's a reasonable length
		if ( (nLength > 10) && (nLength < 40) )
		{
			//	Check to see if start looks correct:
			//	Forwarded on:
			//	-----Original Message-----
			//	--- begin forwarded text
			//
			//	Forward off:
			//	-----End Original Message-----
			//	--- end forwarded text
			//
			//	Quote on:
			//	-----Original Message-----
			//
			//	Quote off:
			//	-----End Original-----
			if ( (in_pStart[0] == '-') && (in_pStart[1] == '-') && (in_pStart[2] == '-') )
			{
				const char *		pScan = in_pStart;

				//	Skip past -'s and any whitespace
				for (pScan = in_pStart; pScan < in_pStop; pScan++)
				{
					if ( (*pScan != '-') && !IsWhiteSpace(*pScan, kDontAllowReturns) )
						break;
				}

				//	Make sure that it's still long enough
				if ( (in_pStop - pScan) > 4 )
				{
					//	See if it matches any of the specified strings - passing 0 for the
					//	last parameter specifies that it only has to match up for the length
					//	of the resource string.
					int		nFoundStringIndex =
						FindRStringIndexI(in_nStartStringResID, in_nStopStringResID, pScan, 0);

					//	We found a match if the returned index is not -1.
					bLooksLikeQuoteOrForwardDividerLine = (nFoundStringIndex != -1);
				}
			}
		}
	}
	
	return bLooksLikeQuoteOrForwardDividerLine;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeForwardOn												[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like a start forwarding divider line.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like a start forwarding divider line.

bool
ContentConcentrator::Message::LooksLikeForwardOn(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume start of forwarded section
	bool	bLooksLikeForwardOn = true;

	if (!m_bShouldPresumeForward)
	{
		//	We're not presuming forward, so check to see if it
		//	looks like the start of a quoted section
		bLooksLikeForwardOn = !LooksLikeQuoteOrForwardDividerLine(
									in_pStart, in_pStop,
									IDS_CON_CON_QUOTE_ON_1, IDS_CON_CON_QUOTE_ON_1);
	}

	if (bLooksLikeForwardOn)
	{
		//	Either we're presuming forward or it didn't look like the start
		//	of a quoted section. Check to see if it looks like the start
		//	of a forwarded section.
		bLooksLikeForwardOn = LooksLikeQuoteOrForwardDividerLine(
									in_pStart, in_pStop,
									IDS_CON_CON_FORWARD_ON_1, IDS_CON_CON_FORWARD_ON_2);
	}

	return bLooksLikeForwardOn;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeForwardOff												[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like an end forwarding divider line.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like an end forwarding divider line.

bool
ContentConcentrator::Message::LooksLikeForwardOff(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	//	Assume end of forwarded section
	bool	bLooksLikeForwardOff = true;

	if (!m_bShouldPresumeForward)
	{
		//	We're not presuming forward, so check to see if it
		//	looks like the end of a quoted section
		bLooksLikeForwardOff = !LooksLikeQuoteOrForwardDividerLine(
									in_pStart, in_pStop,
									IDS_CON_CON_QUOTE_OFF_1, IDS_CON_CON_QUOTE_OFF_1);
	}

	if (bLooksLikeForwardOff)
	{
		//	Either we're presuming forward or it didn't look like the end
		//	of a quoted section. Check to see if it looks like the end
		//	of a forwarded section.
		bLooksLikeForwardOff = LooksLikeQuoteOrForwardDividerLine(
									in_pStart, in_pStop,
									IDS_CON_CON_FORWARD_OFF_1, IDS_CON_CON_FORWARD_OFF_2);
	}

	return bLooksLikeForwardOff;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeQuoteOn													[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like a start quoting divider line.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like a start quoting divider line.

bool
ContentConcentrator::Message::LooksLikeQuoteOn(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	return LooksLikeQuoteOrForwardDividerLine(
				in_pStart, in_pStop,
				IDS_CON_CON_QUOTE_ON_1, IDS_CON_CON_QUOTE_ON_1);
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeQuoteOff												[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like an end quoting divider line.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like an end quoting divider line.

bool
ContentConcentrator::Message::LooksLikeQuoteOff(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	return LooksLikeQuoteOrForwardDividerLine(
				in_pStart, in_pStop,
				IDS_CON_CON_QUOTE_OFF_1, IDS_CON_CON_QUOTE_OFF_1);
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeAttachmentLine											[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like an attachment line.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like an attachment line.

bool
ContentConcentrator::Message::LooksLikeAttachmentLine(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	if ( IsHTML() )
	{
		//	To do:
		//	* Skip tags, then continue below
	}
	
	in_pStart = SkipTags(in_pStart, in_pStop);
	long		nLength = in_pStop - in_pStart;

	bool		bLooksLikeAttachmentLine =
						(nLength >= m_szACTrailer.GetLength()) &&
						(strnicmp(m_szACTrailer, in_pStart, m_szACTrailer.GetLength()) == 0);

	if (!bLooksLikeAttachmentLine)
	{
		//	At least for now we'll treat embedded content lines like attachment
		//	lines. The only code currently using this designation is the outlook
		//	quote parsing code, which needs to stop parsing when it reaches an
		//	"Embedded Content:" line just like it would for an "Attachment Converted"
		//	line. In the future we may want to add an extra type specifically for
		//	embedded content.
		bLooksLikeAttachmentLine =
				(nLength >= m_szECTrailer.GetLength()) &&
				(strnicmp(m_szECTrailer, in_pStart, m_szECTrailer.GetLength()) == 0);
	}
	
	return bLooksLikeAttachmentLine;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeDigestSeparator											[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks a digest separator.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like a digest separator.

bool
ContentConcentrator::Message::LooksLikeDigestSeparator(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	bool	bLooksLikeDigestSeparator = false;
	
	if ( IsHTML() )
	{
		//	To do: I'm not sure - maybe just skip tags then continue below?
	}
	else
	{
		in_pStart = SkipTags(in_pStart, in_pStop);

		short	nLength = static_cast<short>(in_pStop - in_pStart);

		bLooksLikeDigestSeparator = (nLength >= kMinDigestSeparatorLength) &&
									(nLength <= kMaxDigestSeparatorLength) &&
									(strnicmp(kMinDigestSeparator, in_pStart, kMinDigestSeparatorLength) == 0);
	}
	
	return bLooksLikeDigestSeparator;
}


// ------------------------------------------------------------------------------------------
//		* Message::LooksLikeSigIntro												[Public]
// ------------------------------------------------------------------------------------------
//	Determine if indicated text looks like the start of a signature.
//
//	Parameters:
//		in_pStart:	Start of text to examine
//		in_pStop:	End of text to examine
//
//	Returns:
//		Whether or not indicated text looks like the start of a signature.

bool
ContentConcentrator::Message::LooksLikeSigIntro(
	const char *			in_pStart,
	const char *			in_pStop) const
{
	bool	bLooksLikeSigIntro = false;
	
	if ( IsHTML() )
	{
		//	To do: I'm not sure - maybe just skip tags then continue below?
	}
	else
	{
		in_pStart = SkipTags(in_pStart, in_pStop);

		bLooksLikeSigIntro = ((in_pStop - in_pStart) == kSigSeparatorLength) &&
							 IsSigSeparator(in_pStart, in_pStop);
	}
	
	return bLooksLikeSigIntro;
}


// ------------------------------------------------------------------------------------------
//		* Message::DetermineNextParagraphType										[Public]
// ------------------------------------------------------------------------------------------
//	Determine what type of paragraph this is.

void
ContentConcentrator::Message::DetermineNextParagraphType()
{
	const char *	pStart = m_nextParagraph.GetStart();
	const char *	pStop = m_nextParagraph.GetStop();
	
	//	Default to ordinary paragraph
	ParagraphInfo::ParaType		type = ParagraphInfo::type_Ordinary;
	
	if ( IsBlank(pStart, pStop) )
		type = ParagraphInfo::type_Blank;
	else if ( IsAllWhiteSpace(pStart, pStop) )
		type = ParagraphInfo::type_White;
	else if ( LooksLikeAttribution(pStart, pStop) )
		type = ParagraphInfo::type_Attribution;
	else if ( LooksLikeForwardOn(pStart, pStop) )
		type = ParagraphInfo::type_ForwardOn;
	else if ( LooksLikeForwardOff(pStart, pStop) )
		type = ParagraphInfo::type_ForwardOff;
	else if ( LooksLikeQuoteOn(pStart, pStop) )
		type = ParagraphInfo::type_QuoteOn;
	else if ( LooksLikeQuoteOff(pStart, pStop) || (m_bIsDigest && LooksLikeDigestSeparator(pStart, pStop)) )
		type = ParagraphInfo::type_QuoteOff;
	else if ( LooksLikeSigIntro(pStart, pStop) )
		type = ParagraphInfo::type_SignatureIntro;
	else if ( LooksLikeAttachmentLine(pStart, pStop) )
		type = ParagraphInfo::type_Attachment;

	m_nextParagraph.SetType(type);
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphAttribution									[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is an attribution.
//
//	Returns:
//		Whether or not current paragraph is an attribution.

bool
ContentConcentrator::Message::IsCurrentParagraphAttribution() const
{
	bool	bIsAttribution = false;

	//	Must be less quoted than what follows
	if ( m_currentParagraph.GetTotalQuoteLevel() < m_nextParagraph.GetTotalQuoteLevel() )
	{
		//	Must have looked like attribution
		bIsAttribution = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_ATTRIBUTION);
	}

	return bIsAttribution;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphForwardOn										[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is start of forwarded section.
//
//	Returns:
//		Whether or not current paragraph is start of forwarded section.

bool
ContentConcentrator::Message::IsCurrentParagraphForwardOn() const
{
	bool	bIsForwardOn = false;

	//	Must not be quoting here
	if ( !m_currentParagraph.IsQuoted() && !m_nextParagraph.IsQuoted() )
	{
		//	Must have looked like forward on
		bIsForwardOn = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_FORWARD_ON);
	}

	return bIsForwardOn;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphForwardOff										[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is end of forwarded section.
//
//	Returns:
//		Whether or not current paragraph is end of forwarded section.

bool
ContentConcentrator::Message::IsCurrentParagraphForwardOff() const
{
	bool	bIsForwardOff = false;

	//	Must not be quoting here
	if ( !m_currentParagraph.IsQuoted() && !m_nextParagraph.IsQuoted() )
	{
		//	Must have looked like forward off
		bIsForwardOff = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_FORWARD_OFF);
	}

	return bIsForwardOff;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphQuoteOn										[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is start of forwarded section.
//
//	Returns:
//		Whether or not current paragraph is start of forwarded section.

bool
ContentConcentrator::Message::IsCurrentParagraphQuoteOn() const
{
	bool	bIsQuoteOn = false;

	//	Must not be quoting here
	if ( !m_currentParagraph.IsQuoted() && !m_nextParagraph.IsQuoted() )
	{
		//	Must have looked like quote on
		bIsQuoteOn = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_QUOTE_ON);
	}

	return bIsQuoteOn;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphQuoteOff										[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is end of forwarded section.
//
//	Returns:
//		Whether or not current paragraph is end of forwarded section.

bool
ContentConcentrator::Message::IsCurrentParagraphQuoteOff() const
{
	bool	bIsQuoteOff = false;

	//	Must not be quoting here
	if ( !m_currentParagraph.IsQuoted() && !m_nextParagraph.IsQuoted() )
	{
		//	Must have looked like quote off
		bIsQuoteOff = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_QUOTE_OFF);
	}

	return bIsQuoteOff;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphSigIntro										[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is signature intro.
//
//	Returns:
//		Whether or not current paragraph is signature intro.

bool
ContentConcentrator::Message::IsCurrentParagraphSigIntro() const
{
	bool	bIsSigIntro = false;

	//	Quoting should not vary here
	if ( m_currentParagraph.GetTotalQuoteLevel() == m_nextParagraph.GetTotalQuoteLevel() )
	{
		//	Must have looked like sig intro
		bIsSigIntro = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_SIG_INTRO);
	}

	return bIsSigIntro;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsCurrentParagraphAttachment										[Public]
// ------------------------------------------------------------------------------------------
//	Determine if current paragraph is an attachment line.
//
//	Returns:
//		Whether or not current paragraph is an attachment line.

bool
ContentConcentrator::Message::IsCurrentParagraphAttachment() const
{
	bool	bIsAttachment = false;

	//	No quoting allowed
	if ( !m_currentParagraph.IsQuoted() )
	{
		//	Must have looked like attachment
		bIsAttachment = (m_currentParagraph.GetType() == IDS_CON_CON_TYPE_ATTACHMENT);
	}

	return bIsAttachment;
}


// ------------------------------------------------------------------------------------------
//		* Message::AdvanceParagraph													[Public]
// ------------------------------------------------------------------------------------------
//	Advance paragraph records.
//
//	Returns:
//		Whether or not this paragraph is real.

bool
ContentConcentrator::Message::AdvanceParagraph()
{
	//	Switch paragraph info
	//	* To do - maybe optimize switching, also will we need to be able to look further back or forward?
	m_lastParagraph = m_currentParagraph;
	m_currentParagraph = m_nextParagraph;
	
	//	Prepare the next paragraph
	PrepareNextParagraph();
	
	//	If we have data in the current paragraph return true
	return m_currentParagraph.GetStart() < m_szEndOriginalMessage;
}


// ------------------------------------------------------------------------------------------
//		* Message::PrepareNextParagraph												[Public]
// ------------------------------------------------------------------------------------------
//	Prepares the next paragraph.

void
ContentConcentrator::Message::PrepareNextParagraph()
{
	//	Parse for the next paragraph
	m_nextParagraph.SetStart( m_currentParagraph.GetStop() );
	DetermineEndOfNextParagrah();
	DetermineQuoteLevelForNextParagraph();
	
	//	Determine the next paragraph type
	if (m_nextParagraph.GetStart() >= m_szEndOriginalMessage)
		m_nextParagraph.SetType(ParagraphInfo::type_Complete);
	else
		DetermineNextParagraphType();
}


// ------------------------------------------------------------------------------------------
//		* Message::Element::RuleMatchesElement									 [Protected]
// ------------------------------------------------------------------------------------------
//	Determines whether or not the specified rule matches the given element.
//
//	Parameters:
//		in_pRule:				Rule to compare
//		in_element:				Element to compare
//
//	Returns:
//		-1 : rule doesn't match, but matches may be found later
//		 0 : rule matches
//		 1 : rule doesn't match, and no more rules are allowed to match it

short
ContentConcentrator::Message::RuleMatchesElement(
	Rule *					in_pRule,
	Element &				in_element)
{
	//	Check for wildcard rule
	if (in_pRule->GetType() == IDS_CON_CON_TYPE_ANY)
		return 0;

	//	Check to see if rule matches us
	if ( in_element.GetType() != in_pRule->GetType() )
		return -1;

	//	OK the types are the same, now we must qualify. Assume match until proven otherwise.
	short	matchResult = 0;

	//	OK the types are the same, now we must qualify.
	if (in_element.GetType() == IDS_CON_CON_TYPE_HEADER)
	{
		const CString &		szRuleHeaderName = in_pRule->GetHeaderName();
		const CString &		szElementHeaderName = in_element.GetHeaderName();
		
		if ( !szRuleHeaderName.IsEmpty() && !szElementHeaderName.IsEmpty() )
		{
			//	Check to make sure that header name matches
			if ( szRuleHeaderName.Compare(szElementHeaderName) != 0 )
				matchResult = -1;
		}
	}

	//	Do more qualifiers here eventually

	//	We made it this far so it matches
	return matchResult;
}


// ------------------------------------------------------------------------------------------
//		* Message::Element::FindProfileRuleForElement							 [Protected]
// ------------------------------------------------------------------------------------------
//	Finds the profile rule that best matches this element.
//
//	Parameters:
//		in_element:					Element for which to find a matching rule
//		in_pProfile:				Profile in which to look for a matching rule
//
//	Returns:
//		Matching rule or NULL if none is found

ContentConcentrator::Profile::Rule *
ContentConcentrator::Message::FindProfileRuleForElement(
	Element &				in_element,
	Profile *				in_pProfile)
{
	Rule *					pMatchingRule = NULL;
	vector<Rule *> &		rules = in_pProfile->GetRules();
	short					match;
	
	for ( vector<Rule *>::iterator ruleIterator = rules.begin();
		  ruleIterator != rules.end();
		  ruleIterator++ )
	{
		match = RuleMatchesElement(*ruleIterator, in_element);

		if (match == 0)
			pMatchingRule = *ruleIterator;
		else if (match == 1)
			break;
		//	match == -1 means that we didn't match, but we should continue looking
	}

	return pMatchingRule;
}


// ------------------------------------------------------------------------------------------
//		* Message::OutputElement												 [Protected]
// ------------------------------------------------------------------------------------------
//	Outputs this element according the specified rule.
//
//	Parameters:
//		in_pPreviousElement:			Pointer to the previous element or NULL if we're
//										on the first element
//		io_element:						Element to output
//		in_pRule:						Rule for output
//		out_szConcentratedMessage:		Concentrated version of message

void
ContentConcentrator::Message::OutputElement(
	Element *				in_pPreviousElement,
	Element &				io_element,
	const Rule *			in_pRule,
	CString &				out_szConcentratedMessage)
{
	const char *	pStart = io_element.GetStart();
	const char *	pStop = io_element.GetStop();

	if (in_pRule)
	{
		ULONG			elementType = io_element.GetType();
		short			nQuoteIncrement = in_pRule->GetOutputQuoteIncrement();
		long			nOutputType = in_pRule->GetOutputType();
		
		//	Stash the output quote increment in the element so that we can make later
		//	decisions based upon it (for now it will be used in OutputLineBreakIfNeeded).
		io_element.SetOutputQuoteIncrement(nQuoteIncrement);
		
		if ( (nOutputType != IDS_CON_CON_OUT_TYPE_REMOVE) && in_pRule->ShouldTrimOutput() )
		{
			FixTrimEndpoints(io_element);
			pStart = io_element.GetTrimmedStart();
			pStop = io_element.GetTrimmedStop();
			
			if ( io_element.GetTrimmedStart() != io_element.GetStart() )
			{
				//	We trimmed the start - we need to output any tags found there
				CopyText(elementType, io_element.GetStart(), io_element.GetTrimmedStart(), kStripContent, 0, out_szConcentratedMessage);
			}
		}
		
		//	Sanity check that we still have something to output
		if (pStop <= pStart)
			return;
		
		//	One of my test messages is a reply from Outlook to a message I originally sent
		//	with Eudora. The "-----Original Message-----" portion (i.e. what I originally
		//	wrote) for some reason has x-flowed tags in it (I didn't think we sent those -
		//	testing appears to confirm that we don't, so perhaps Outlook uses them too). The
		//	message itself is *not* Format=Flowed and does *not* have x-flowed tags around
		//	the entire message, just around the body portion of my original message below
		//	the "-----Original Message-----" line.
		//
		//	Because of this the resulting weirdness is that when incrementing the quote level,
		//	the headers from my original message get interpreted as not-excerpted and the body
		//	from my original message get interpreted as excerpted.
		//
		//	Check for this very specific case and output a x-flowed tag before the section
		//	for which we're incrementing the quote level. Note that this means that we'll
		//	be leaving in the redundant x-flowed start that is inside the element. This
		//	will work because GetBodyAsHTML will be happy to skip the additional x-flowed
		//	tag and continue to interpret the content as x-flowed until it hits the
		//	x-flowed closing tag.
		if ( (nOutputType != IDS_CON_CON_OUT_TYPE_REMOVE) && (nQuoteIncrement > 0) &&
			m_pStartXFlowed && (io_element.GetStart() < m_pStartXFlowed) &&
			(io_element.GetStop() > m_pStartXFlowed) )
		{
			CString			szFlowedOn;
			
			szFlowedOn.Format(CRString(IDS_MIME_RICH_ON), m_szFlowed);
			
			out_szConcentratedMessage += szFlowedOn;
			out_szConcentratedMessage += "\r\n";
		}
		
		switch ( in_pRule->GetOutputType() )
		{
			case IDS_CON_CON_OUT_TYPE_REMOVE:
				//	Easy - output no content
				CopyText(elementType, pStart, pStop, kStripContent, 0, out_szConcentratedMessage);
				break;
				
			case IDS_CON_CON_OUT_TYPE_TRUNCATE:
				{
					OutputLineBreakIfNeeded(in_pPreviousElement, io_element, out_szConcentratedMessage);
					
					//	Copy truncated content and all tags
					const char *	pStartTruncation;
					const char *	pStopTruncation;
					CString			szEllipsis;
					
					bool	bTruncated = DetermineTruncationEndpoints(
						in_pRule->GetOutputBytes(), nQuoteIncrement,
						io_element, pStart, pStop, 
						pStartTruncation, pStopTruncation, szEllipsis );
					if (bTruncated)
					{
						//	Copy the content before the truncation
						CopyText(elementType, pStart, pStartTruncation, kCopyContent, nQuoteIncrement, out_szConcentratedMessage);
						
						//	Tack on the ellipsis indicated
						out_szConcentratedMessage += szEllipsis;
						
						//	Copy the tags (if any) in the truncated section
						CopyText(elementType, pStartTruncation+1, pStopTruncation, kStripContent, 0, out_szConcentratedMessage);
						
						//	Copy the content after the truncation
						CopyText(elementType, pStopTruncation, pStop, kCopyContent, nQuoteIncrement, out_szConcentratedMessage);
					}
					else
					{
						//	We're not truncating after all, so just copy all content and tags
						CopyText(elementType, pStart, pStop, kCopyContent, nQuoteIncrement, out_szConcentratedMessage);
					}
				}
				break;
				
			case IDS_CON_CON_OUT_TYPE_DISPLAY:
			default:
				OutputLineBreakIfNeeded(in_pPreviousElement, io_element, out_szConcentratedMessage);
				
				//	Really easy - copy all content and tags
				CopyText(elementType, pStart, pStop, kCopyContent, nQuoteIncrement, out_szConcentratedMessage);
				break;
		}
		
		if ( (nOutputType != IDS_CON_CON_OUT_TYPE_REMOVE) &&
			 (io_element.GetTrimmedStop() != io_element.GetStop()) )
		{
			//	We trimmed the stop so we need to output any tags found there
			CopyText(elementType, io_element.GetTrimmedStop(), io_element.GetStop(), kStripContent, 0, out_szConcentratedMessage);
		}
	}
	else
	{
		//	We don't have any rule - so just the entire element
		CopyTextToCString(pStart, pStop, out_szConcentratedMessage);
	}
}


// ------------------------------------------------------------------------------------------
//		* Message::OutputLineBreakIfNeeded										 [Protected]
// ------------------------------------------------------------------------------------------
//	Outputs line break for the current element if needed given the previous element.
//
//	Parameters:
//		in_pPreviousElement:			Pointer to the previous element or NULL if we're
//										on the first element
//		in_element:						Element to output
//		out_szOutput:					String onto which to append output

void
ContentConcentrator::Message::OutputLineBreakIfNeeded(
	Element *				in_pPreviousElement,
	Element &				in_element,
	CString &				out_szOutput)
{
	//	If we're on the first element then we don't need to output an extra line break
	if (!in_pPreviousElement)
		return;

	short			nPreviousQuoteLevel = in_pPreviousElement->GetOutputQuoteLevel();
	short			nCurrentQuoteLevel = in_element.GetOutputQuoteLevel();
	short			nCurrentLineBreaksBetweenElements = 0;

	//	Count the number of CRLFs that ended the previous element (after trimming, if any)
	const char *	pStopScan = in_pPreviousElement->GetTrimmedStart();
	const char *	pScan = in_pPreviousElement->GetTrimmedStop() - 2;

	while ( (pScan > pStopScan) && IsCRLF(pScan) )
	{
		nCurrentLineBreaksBetweenElements++;
		pScan -= 2;
	}

	//	Count the number of CRLFs that start the current element (after trimming, if any)
	pStopScan = in_element.GetTrimmedStop();
	pScan = in_element.GetTrimmedStart();
	while ( (pScan < pStopScan) && IsCRLF(pScan) )
	{
		nCurrentLineBreaksBetweenElements++;
		pScan += 2;
	}

	//	We may need an additional line break if we didn't already have a lot.
	if (nCurrentLineBreaksBetweenElements < 2)
	{
		short	nQuoteCharsToMaintain = 0;
		
		if (nCurrentLineBreaksBetweenElements > 0)
		{
			//	Use whatever quote level is lower
			nQuoteCharsToMaintain = in_element.GetNumQuoteChars();

			if (in_pPreviousElement->GetNumQuoteChars() < nQuoteCharsToMaintain)
				nQuoteCharsToMaintain = in_pPreviousElement->GetNumQuoteChars();
		}
		
		//	Did we trim the previous element? Trimming is agressive and trims
		//	it down to the point that there's no extra space after it. Compensate
		//	for that here by adding a line break back in if needed for
		//	aesthetics.
		if ( in_pPreviousElement->TrimmedTrailingParBreak() )
		{
			OutputLineBreak(nQuoteCharsToMaintain, out_szOutput);
		}
		
		//	Are we inserting less quoted or plain text after a quote?
		else if (nPreviousQuoteLevel > nCurrentQuoteLevel)
		{
			OutputLineBreak(nQuoteCharsToMaintain, out_szOutput);
		}
		
		//	Are we inserting a quote after non-attributional plain(er) text?
		else if ( (in_pPreviousElement->GetType() != IDS_CON_CON_TYPE_SEPARATOR) &&
			(in_pPreviousElement->GetType() != IDS_CON_CON_TYPE_ATTRIBUTION) &&
			(nPreviousQuoteLevel < nCurrentQuoteLevel) )
		{
			OutputLineBreak(nQuoteCharsToMaintain, out_szOutput);
		}
	}
}


// ------------------------------------------------------------------------------------------
//		* Message::OutputLineBreak												 [Protected]
// ------------------------------------------------------------------------------------------
//	Outputs line break.
//
//	Parameters:
//		in_nQuoteChars:			Number of quote chars to maintain
//		out_szOutput:			String onto which to append output

void
ContentConcentrator::Message::OutputLineBreak(
	short					in_nQuoteChars,
	CString &				out_szOutput)
{
	if ( IsHTML() )
	{
		//	To do: Output CRLF followed by BR tag
		//	We may also need to consider closing and reopening P tags...
	}

	OutputQuoteChars(in_nQuoteChars, out_szOutput);
	out_szOutput += "\r\n";
}


// ------------------------------------------------------------------------------------------
//		* Message::OutputQuoteChars												 [Protected]
// ------------------------------------------------------------------------------------------
//	Outputs up to 12 quote characters.
//
//	Parameters:
//		in_nQuoteChars:			Number of quote chars to output
//		out_szOutput:			String onto which to append output

void
ContentConcentrator::Message::OutputQuoteChars(
	short					in_nQuoteChars,
	CString &				out_szOutput)
{
	if (in_nQuoteChars > 0)
	{
		//	Maintain up to 12 quote characters
		in_nQuoteChars = (in_nQuoteChars < 12) ? in_nQuoteChars : static_cast<short>(12);
		
		CString		szQuoteString(kQuoteChar, in_nQuoteChars);
		
		out_szOutput += szQuoteString;
	}
}


// ------------------------------------------------------------------------------------------
//		* Message::FindCRLF														 [Protected]
// ------------------------------------------------------------------------------------------
//	Scans forward or backwards until we find a CRLF.
//
//	Parameters:
//		in_pStart:			Start of text to scan
//		in_pStop:			End of text to scan
//		in_pLastLocation:	Last location we searched or NULL to start search from endpoint
//		in_bReverse:		Search in reverse from stop to start

const char *
ContentConcentrator::Message::FindCRLF(
	const char *			in_pStart,
	const char *			in_pStop,
	const char *			in_pLastLocation,
	bool					in_bReverse)
{
	const char *	pCandidate = in_pLastLocation;

	if (in_bReverse)
	{
		if (in_pLastLocation == NULL)
			pCandidate = in_pStop-2;
		else
			pCandidate = in_pLastLocation - 1;

		while ( (pCandidate >= in_pStart) && !IsCRLF(pCandidate) )
			pCandidate--;

		if (pCandidate < in_pStart)
			pCandidate = NULL;
	}
	else
	{
		if (in_pLastLocation == NULL)
			pCandidate = in_pStart;
		else
			pCandidate = in_pLastLocation + 1;

		while ( (pCandidate < (in_pStop-1)) && !IsCRLF(pCandidate) )
			pCandidate++;

		if (pCandidate >= (in_pStop-1))
			pCandidate = NULL;
	}

	return pCandidate;
}


// ------------------------------------------------------------------------------------------
//		* Message::FindParBreak													 [Protected]
// ------------------------------------------------------------------------------------------
//	Scans forward or backwards until we find a par break.
//
//	Parameters:
//		in_pStart:			Start of text to scan
//		in_pStop:			End of text to scan
//		in_pLastLocation:	Last location we searched or NULL to start search from endpoint
//		in_bReverse:		Search in reverse from stop to start

const char *
ContentConcentrator::Message::FindParBreak(
	const char *			in_pStart,
	const char *			in_pStop,
	const char *			in_pLastLocation,
	bool					in_bReverse)
{
	const char *	pCandidate = in_pLastLocation;
	
	if ( IsPlain() || (m_nState == IDS_CON_CON_TYPE_HEADER) )
	{
		bool	bIsFlowed = IsFlowed();
		
		//	Special case for sig separator when format flowed,
		//	because it looks like a soft line break
		if ( bIsFlowed && IsSigSeparator(in_pStart, in_pStop) )
		{
			//	Start of par break is two chars before the end of "-- \r\n"
			pCandidate = in_pStart + kSigSeparatorLength - 2;
		}
		else
		{
			do
			{
				pCandidate = FindCRLF(in_pStart, in_pStop, pCandidate, in_bReverse);
				
				//	If we found a CR - look for LF as next character
				if (pCandidate)
				{
					//	Format=Flowed only applies to the body, not headers
					if ( bIsFlowed && (m_nState != IDS_CON_CON_TYPE_HEADER) )
					{
						//	It's flowed so we need to make sure that we don't have a soft
						//	line break - i.e. not SP CRLF
						if (pCandidate == in_pStart)
							break;
						else if (*(pCandidate-1) != ' ')
							break;
					}
					else
					{
						//	We're not flowed - so we're done
						break;
					}
				}

			} while (pCandidate);
		}
	}
	else if ( IsHTML() )
	{
		//	To do: Look for P and probably BR tags
		
		//	For now, we shouldn't reach this code
		ASSERT(0);
	}

	return pCandidate;
}


// ------------------------------------------------------------------------------------------
//		* Message::CountQuoteCharsForSingleLine									 [Protected]
// ------------------------------------------------------------------------------------------
//	Scans forward counting quote chars for a given line.
//
//	Parameters:
//		in_pStart:			Start of text to scan
//		in_pStop:			End of text to scan
//
//	Returns:
//		Number of quote chars between start and stop.

int
ContentConcentrator::Message::CountQuoteCharsForSingleLine(
	const char *			in_pStart,
	const char *			in_pStop)
{
	const char *	pScan = in_pStart;
	int				nQuoteChars = 0;

	while ( (pScan < in_pStop) && (*pScan == kQuoteChar) )
	{
		pScan++;
		nQuoteChars++;
	}

	return nQuoteChars;
}


// ------------------------------------------------------------------------------------------
//		* Message::CountQuoteCharsForMultipleLines								 [Protected]
// ------------------------------------------------------------------------------------------
//	Scans forward counting quote chars for each line between start and stop.
//
//	Parameters:
//		in_pStart:			Start of text to scan
//		in_pStop:			End of text to scan
//
//	Returns:
//		Number of quote chars between start and stop.

int
ContentConcentrator::Message::CountQuoteCharsForMultipleLines(
	const char *			in_pStart,
	const char *			in_pStop)
{
	int				nQuoteChars = 0;
	const char *	pScan = FindCRLF(in_pStart, in_pStop, NULL);

	while (pScan)
	{
		//	Advance past the CRLF
		pScan += 2;

		nQuoteChars += CountQuoteCharsForSingleLine(pScan, in_pStop);
		
		//	Start one early than where we stopped, because FindCRLF
		//	will increment by one and we don't need that here
		pScan = FindCRLF(in_pStart, in_pStop, pScan-1);
	}

	return nQuoteChars;
}


// ------------------------------------------------------------------------------------------
//		* Message::FixTrimEndpoints												 [Protected]
// ------------------------------------------------------------------------------------------
//	Reduce offsets that include all-white lines until we wind up with either no all-white
//	lines or the whole text is collapsed to a single all-white line.
//
//	Parameters:
//		in_bIsQuoted:				Whether or not the element is quoted
//		io_pStart:					Start of text to trim
//		io_pStop:					End of text to trim

void
ContentConcentrator::Message::FixTrimEndpoints(
	Element &				io_element)
{
	if ( IsHTML() )
	{
		//	To do: HTML implementation
	}
	else
	{
		const char *	pStart = io_element.GetStart();
		const char *	pStop = io_element.GetStop();
		bool			bIsQuoted = (io_element.GetTotalQuoteLevel() > 0);
		bool			bLastCharQuoteOrReturn = bIsQuoted;
		short			nTagLength;
		
		//	Scan for the first non-white character
		const char *	pScan = pStart;
		while ( pScan < (pStop-2) )
		{
			nTagLength = IsTag(pScan, pStop);

			if (nTagLength > 0)
			{
				//	Advance past the tag
				pScan += nTagLength;
			}
			else
			{
				//	If we're dealing with a quoted element and the last character
				//	was a quote or a return, check this character for a quote
				if ( bIsQuoted && bLastCharQuoteOrReturn )
					bLastCharQuoteOrReturn = (*pScan == kQuoteChar);
				
				if ( !bLastCharQuoteOrReturn && !IsWhiteSpace(*pScan, kAllowReturns) )
					break;

				//	If we're dealing with a quoted element and the char isn't
				//	a quote, then check to see if it's a return.
				if (bIsQuoted && !bLastCharQuoteOrReturn)
					bLastCharQuoteOrReturn = IsReturn(*pScan);

				//	Advance to the next char
				pScan++;
			}
		}
		
		//	Back up to just after the last CRLF
		while ( (pScan >= (pStart+2)) && !IsCRLF(pScan-2) )
			pScan--;
		
		if ( IsCRLF(pScan-2) )
		{
			//	Our scan worked - set the start
			io_element.SetTrimmedStart(pScan);
		}
		else
		{
			//	Our scan failed - start at the normal start
			io_element.SetTrimmedStart(pStart);
			pScan = pStart;
		}

		//	Find the last non-white character
		bLastCharQuoteOrReturn = bIsQuoted;
		const char *	pLastNonWhiteChar = pStop;
		while (pScan < pStop)
		{
			nTagLength = IsTag(pScan, pStop);

			if (nTagLength > 0)
			{
				//	Advance past the tag
				pScan += nTagLength;
			}
			else
			{
				//	If we're dealing with a quoted element and the last character
				//	was a quote or a return, check this character for a quote
				if ( bIsQuoted && bLastCharQuoteOrReturn )
					bLastCharQuoteOrReturn = (*pScan == kQuoteChar);
								
				if ( !bLastCharQuoteOrReturn && !IsWhiteSpace(*pScan, kAllowReturns) )
					pLastNonWhiteChar = pScan;

				//	If we're dealing with a quoted element and the char isn't
				//	a quote, then check to see if it's a return.
				if (bIsQuoted && !bLastCharQuoteOrReturn)
					bLastCharQuoteOrReturn = IsReturn(*pScan);

				//	Advance to the next char
				pScan++;
			}
		}

		//	Find the next CRLF after the last non-white character
		pScan = pLastNonWhiteChar;
		while ( (pScan < pStop) && !IsCRLF(pScan) )
			pScan++;

		//	Advance past the CRLF
		if ( IsCRLF(pScan) && (pScan < (pStop-1)) )
			pScan += 2;
		
		//	Set the end
		io_element.SetTrimmedStop(pScan);

		if (pScan != pStop)
		{
			//	Remember whether or not we trimmed a trailing paragraph break
			if ( FindParBreak(pScan, pStop) )
				io_element.SetTrimmedTrailingParBreak(true);
		}
	}
}


// ------------------------------------------------------------------------------------------
//		* Message::DetermineTruncationEndpoints									 [Protected]
// ------------------------------------------------------------------------------------------
//	Determines whether we should be truncating, where the start and stop
//	of truncation should be, and what the ellipsis string should be.
//
//	Parameters:
//		in_nOutputBytes:			Number of bytes of content to keep
//		in_element:					Element that we're truncating
//		in_pStart:					Start of text to examine for truncation
//		in_pStop:					End of text to examine for truncation
//		out_pStartTruncation:		Where truncation should start
//		out_pStopTruncation:		Where truncation should stop
//		out_szEllipsis:				The correct ellipsis string to concat after the
//									truncated text
//
//	Returns:
//		true:	We're truncating
//		false:	We're not truncating

bool
ContentConcentrator::Message::DetermineTruncationEndpoints(
	UINT					in_nOutputBytes,
	short					in_nQuoteIncrement,
	const Element &			in_element,
	const char *			in_pStart,
	const char *			in_pStop,
	const char *&			out_pStartTruncation,
	const char *&			out_pStopTruncation,
	CString &				out_szEllipsis)
{
	//	To do: Adapt techniques to handle HTML
	
	UINT	nLength = in_pStop - in_pStart;
	bool	bIsHeader = (in_element.GetType() == IDS_CON_CON_TYPE_HEADER);

	//	Don't truncate if we're already small enough, where "enough" is
	//	that we would be truncating less than 20% of our current size.
	if ( ((nLength*8)/10) < in_nOutputBytes )
		return false;

	//	We want to truncate from the middle, so let's setup offsets to where
	//	the truncation should start and stop.
	out_pStartTruncation = in_pStart + in_nOutputBytes/2;

	//	Adjust the offset to account for any quote characters that threw off our count
	out_pStartTruncation += CountQuoteCharsForMultipleLines(in_pStart, out_pStartTruncation);

	out_pStopTruncation = in_pStop - in_nOutputBytes/2;

	//	Adjust the offset to account for any quote characters that threw off our count
	out_pStopTruncation -= CountQuoteCharsForMultipleLines(out_pStopTruncation, in_pStop);

	//	Adjust the start of truncation to trim at a paragraph boundary if possible
	bool	bStartAtParagraphBoundary = FixTruncationEndpointByParagraph(in_pStart, in_pStop, kStartTruncation, out_pStartTruncation);

	if (!bStartAtParagraphBoundary)
	{
		FixTruncationEndpointByWord(in_pStart, in_pStop, kStartTruncation, in_element.IsAddressHeader(), out_pStartTruncation);
	}
	else if (bIsHeader)
	{
		//	Skip past any whitespace to keep the header line formatting valid
		while ( (out_pStartTruncation < in_pStop) && IsWhiteSpace(*out_pStartTruncation, false) )
			out_pStartTruncation++;
	}

	//	Adjust the stop of truncation to trim at a paragraph boundary if possible
	int		nQuoteChars = 0;
	bool	bStopAtParagraphBoundary = FixTruncationEndpointByParagraph(in_pStart, in_pStop, kEndTruncation, out_pStopTruncation);

	if (!bStopAtParagraphBoundary)
	{
		FixTruncationEndpointByWord(in_pStart, in_pStop, kEndTruncation, in_element.IsAddressHeader(), out_pStopTruncation, &nQuoteChars);
		nQuoteChars = static_cast<short>(nQuoteChars + in_nQuoteIncrement);
	}
	else if (bIsHeader)
	{
		if (bStartAtParagraphBoundary)
		{
			//	We started at a paragraph boundary - so we already have the CRLF and spaces that we need.
			//	Skip past any whitespaces because we don't need them
			while ( (out_pStopTruncation < in_pStop) && IsWhiteSpace(*out_pStopTruncation, false) )
				out_pStopTruncation++;
		}
		else
		{
			//	We need the CRLF - back up before it
			out_pStopTruncation -= 2;
		}
	}
	
	//	Another sanity check - would we still be removing at least 20%?
	if ( ((nLength*8)/10) < static_cast<UINT>((out_pStartTruncation - in_pStart) + (in_pStop - out_pStopTruncation)) )
		return false;

	CString		szINIEllipsis;

	bool	bRemovingParBreak = !bIsHeader && (FindCRLF(in_pStart, in_pStop) != NULL);

	//	If we removed a line break and truncated a paragraph at the start of the truncation,
	//	tack on an ellipsis and line break on the first paragraph
	if ( bRemovingParBreak && !bStartAtParagraphBoundary )
	{
		//	Return value for GetIniString is inconsistent - it sometimes mean the string
		//	failed to load and sometimes means the default value was used. Just
		//	empty the string and see if it gets filled.
		szINIEllipsis.Empty();
		GetIniString(IDS_INI_CC_ELLIPSIS_TRAIL, szINIEllipsis);
		if ( !szINIEllipsis.IsEmpty() )
			out_szEllipsis += szINIEllipsis;

		out_szEllipsis += "\r\n";
	}
	else if ( !IsWhiteSpace(*out_pStartTruncation, true) )
	{
		//	Make sure that there's some sort of whitespace before the ...snip...
		out_szEllipsis += " ";
	}

	//	Tack on the main ellipsis
	szINIEllipsis.Empty();
	GetIniString(IDS_INI_CC_ELLIPSIS_CENTER, szINIEllipsis);
	if ( !szINIEllipsis.IsEmpty() )
		out_szEllipsis += szINIEllipsis;

	//	If we removed a line break, we need a trailing line break, and possibly an ellipsis
	//	at the start of the paragraph after the truncation if we didn't cut cleanly there.
	if (bRemovingParBreak)
	{
		//	No matter what separate what follows from main ellipsis string
		out_szEllipsis += "\r\n";
		if ( !bStopAtParagraphBoundary )
		{
			if (nQuoteChars > 0)
			{
				//	Maintain up to 12 quote characters
				nQuoteChars = (nQuoteChars < 12) ? nQuoteChars : static_cast<short>(12);
				
				CString		szQuoteString(kQuoteChar, nQuoteChars);
				
				out_szEllipsis += szQuoteString;
			}

			//	Tack on leading ellipsis string
			szINIEllipsis.Empty();
			GetIniString(IDS_INI_CC_ELLIPSIS_LEAD, szINIEllipsis);
			if ( !szINIEllipsis.IsEmpty() )
				out_szEllipsis += szINIEllipsis;
		}
	}
	else if ( !IsWhiteSpace(*out_pStopTruncation, true) )
	{
		//	Make sure that there's some sort of whitespace after the ...snip...
		out_szEllipsis += " ";
	}

	return true;
}


// ------------------------------------------------------------------------------------------
//		* Message::IsTruncationCloseEnough										 [Protected]
// ------------------------------------------------------------------------------------------
//	Determines whether or not truncation is close enough to what we were shooting for.
//
//	Parameters:
//		in_pStart:					Start of text to examine for truncation
//		in_pStop:					End of text to examine for truncation
//		in_pTruncationLocation:		Truncation location that we wanted
//		in_pNominee:				Nominee location that we're checking for closeness
//		in_bStartTruncation:		Whether or not this is the start truncation location
//		in_nMaxVarianceMultiple:	Muliple of how great the variance can be between what
//									we wanted to remove and what we're actually looking at
//									removing (default value is 5, which corresponds to 20%)
//
//	Returns:
//		true:	It's close enough to what we wanted
//		false:	It's not close enough to what we wanted

bool
ContentConcentrator::Message::IsTruncationCloseEnough(
	const char *			in_pStart,
	const char *			in_pStop,
	const char *			in_pTruncationLocation,
	const char *			in_pNominee,
	bool					in_bStartTruncation,
	int						in_nMaxVarianceMultiple)
{
	//	How much did we want to remove?
	UINT	nLength = in_pStop - in_pStart;
	UINT	nDistance = in_bStartTruncation ?
						(in_pTruncationLocation - in_pStart) : (in_pStop - in_pTruncationLocation);
	UINT	nWantToRemove = nLength - 2 * nDistance;

	//	What did we actually end up with?
	UINT	nRemainingAmount = in_bStartTruncation ?
							   (in_pNominee - in_pStart) : (in_pStop - in_pNominee);
	UINT	nWillRemove = nLength - 2 * nRemainingAmount;

	//	How far off are we from what we wanted to end up with?
	UINT	nDiff = abs( static_cast<int>(nWantToRemove - nWillRemove) );

	//	If the variance is more than 20%, forget it
	return (nDiff*in_nMaxVarianceMultiple <= nWantToRemove);
}


// ------------------------------------------------------------------------------------------
//		* Message::FindCRLFEndpoint												 [Protected]
// ------------------------------------------------------------------------------------------
//	Adjust a truncation spot to end on a paragraph boundary.
//
//	Parameters:
//		in_pStart:					Start of text to examine
//		in_pStop:					End of text to examine
//		in_bReverse:				Whether or not we're looking in reverse
//		in_pParBreak:				Par break at which to start searching for CRLF endpoint
//
//	Returns:
//		Pointer to CRLF endpoint

const char *
ContentConcentrator::Message::FindCRLFEndpoint(
	const char *			in_pStart,
	const char *			in_pStop,
	bool					in_bReverse,
	const char *			in_pParBreak)
{
	//	Look for the first or last CRLF in a run
	const char *	pCRLF = in_pParBreak;
	const char *	pNextCRLF;
	bool			bIsAllQuoteChars;

	//	We're starting the truncation, so look for the first CRLF (ignoring quotes)
	//	in a run before the paragraph break
	while (pCRLF)
	{
		pNextCRLF = FindCRLF(in_pStart, in_pStop, pCRLF, in_bReverse);

		if (!pNextCRLF)
			break;

		bIsAllQuoteChars = in_bReverse ?
						   IsAllQuoteChars(pNextCRLF+2, pCRLF) :
						   IsAllQuoteChars(pCRLF+2, pNextCRLF);
		if (!bIsAllQuoteChars)
			break;
		
		pCRLF = pNextCRLF;
	}

	return pCRLF;
}


// ------------------------------------------------------------------------------------------
//		* Message::FixTruncationEndpointByParagraph								 [Protected]
// ------------------------------------------------------------------------------------------
//	Adjust a truncation spot to end on a paragraph boundary.
//
//	Parameters:
//		in_pStart:					Start of text to examine for truncation
//		in_pStop:					End of text to examine for truncation
//		in_bStartTruncation:		Whether or not this is the start of truncation
//		io_pTruncationLocation:		Truncation location to modify
//
//	Returns:
//		true:	We suceeded in putting it on a proper paragraph boundary
//		false:	It doesn't seem to make sense to adjust the truncation paragraphwise

bool
ContentConcentrator::Message::FixTruncationEndpointByParagraph(
	const char *			in_pStart,
	const char *			in_pStop,
	bool					in_bStartTruncation,
	const char *&			io_pTruncationLocation)
{
	//	When searching for paragraph break candidate locations we first search
	//	for a par break, which is a stricter search with Format=Flowed then simply
	//	looking for a CRLF (i.e. it avoids "soft" line breaks). Then we search
	//	for the CRLF endpoint - i.e. the start or stop of CRLFs before or after the
	//	par break. This second search allows "soft" line breaks, and is done to
	//	avoid the inclusion of all white lines just before or just after the
	//	section that is snipped.
	
	//	Look for paragraph break before us
	const char *	pLeftParBreak = FindParBreak(in_pStart, in_pStop, io_pTruncationLocation, kReverse);

	//	Look for CRLF endpoint
	pLeftParBreak = FindCRLFEndpoint(in_pStart, in_pStop, in_bStartTruncation, pLeftParBreak);

	//	Look for paragraph break after us
	const char *	pRightParBreak = FindParBreak(in_pStart, in_pStop, io_pTruncationLocation);

	//	Look for CRLF endpoint
	pRightParBreak = FindCRLFEndpoint(in_pStart, in_pStop, in_bStartTruncation, pRightParBreak);

	//	Reject the paragraph break before us if it's too close to the start.
	//	It's too close if there are less than 4 characters before the paragraph break.
	if (pLeftParBreak <= (in_pStart+4))
		pLeftParBreak = NULL;

	//	Reject the paragraph break after us if it's too close to the stop.
	//	It's too close if there are less than 4 characters after the paragraph break
	//	(we're currently pointing before the paragraph break - hence the -6).
	if (pRightParBreak >= (in_pStop-6))
		pRightParBreak = NULL;

	//	Did we get any?
	if (!pLeftParBreak && !pRightParBreak)
		return false;

	//	Pick either the only one we have or the closest one
	const char *	pNominee;

	if (!pLeftParBreak)
		pNominee = pRightParBreak;
	else if (!pRightParBreak)
		pNominee = pLeftParBreak;
	else if ( (io_pTruncationLocation - pLeftParBreak) < (pRightParBreak - io_pTruncationLocation) )
		pNominee = pLeftParBreak;
	else
		pNominee = pRightParBreak;

	//	Adjust the pointer to be after the CRLF
	pNominee += 2;

	bool	bCloseEnough = IsTruncationCloseEnough(in_pStart, in_pStop, io_pTruncationLocation, pNominee, in_bStartTruncation);

	if (bCloseEnough)
	{
		//	Hey - we found a good one!
		io_pTruncationLocation = pNominee;
	}

	return bCloseEnough;
}


// ------------------------------------------------------------------------------------------
//		* Message::FixTruncationEndpointByWord									 [Protected]
// ------------------------------------------------------------------------------------------
//	Adjust a truncation spot to end on a word boundary.
//
//	Parameters:
//		in_pStart:					Start of text to examine for truncation
//		in_pStop:					End of text to examine for truncation
//		in_bStartTruncation:		Whether or not this is the start of truncation
//		in_bIsAddressHeader:		Whether or not we're dealing with an address header
//		io_pTruncationLocation:		Truncation location to modify
//		out_nQuoteChars:			Place to return number of quote chars encountered or NULL
//
//	Returns:
//		true:	We adjusted the truncation endpoint
//		false:	We were not able to adjust the truncation endpoint

bool
ContentConcentrator::Message::FixTruncationEndpointByWord(
	const char *			in_pStart,
	const char *			in_pStop,
	bool					in_bStartTruncation,
	bool					in_bIsAddressHeader,
	const char *&			io_pTruncationLocation,
	int *					out_nQuoteChars)
{
	if ( out_nQuoteChars && !IsHTML() )
	{
		//	Look back for the start of the line
		const char *	pScan = FindCRLF(in_pStart, in_pStop, io_pTruncationLocation, kReverse);
		if (pScan)
		{
			//	Advance past the CRLF
			pScan += 2;

			//	Count quote chars for the given line
			(*out_nQuoteChars) += CountQuoteCharsForSingleLine(pScan, in_pStop);
		}
	}

	bool			bGoodLocation;
	bool			bTryAgain;
	const char *	pNominee;
	const char *	pReverseNominee;

	do
	{
		bTryAgain = false;
		bGoodLocation = true;
		
		//	Start scanning at the suggested truncation location
		pNominee = io_pTruncationLocation;
		pReverseNominee = io_pTruncationLocation;
		
		//	Are we in a word or address?
		if ( IsWordOrAddressChar(*pNominee, in_bIsAddressHeader) )
		{
			//	In a word:
			//	Scan forward until we point at a non-word/address char
			while ( (pNominee < in_pStop) && IsWordOrAddressChar(*pNominee, in_bIsAddressHeader) )
				pNominee++;

			//	Back up until we point at a non-word/address char
			while ( (pReverseNominee > in_pStart) && IsWordOrAddressChar(*pReverseNominee, in_bIsAddressHeader) )
				pReverseNominee--;
		}

		if (in_bStartTruncation)
		{
			//	Scan both nominees backward until just after word char
			while ( (pNominee > (in_pStart+1)) && !IsWordOrAddressChar(*(pNominee-1), in_bIsAddressHeader) )
				pNominee--;
			while ( (pReverseNominee > (in_pStart+1)) && !IsWordOrAddressChar(*(pReverseNominee-1), in_bIsAddressHeader) )
				pReverseNominee--;
		}
		else
		{
			//	Scan both nominees forward until we point at a word char
			while ( (pNominee < in_pStop) && !IsWordOrAddressChar(*pNominee, in_bIsAddressHeader) )
				pNominee++;
			while ( (pReverseNominee < in_pStop) && !IsWordOrAddressChar(*pReverseNominee, in_bIsAddressHeader) )
				pReverseNominee++;
		}

		//	Pick the reverse nominee if:
		//	* It's closer to where we started OR
		//	* The forward nominee got too close to the end
		if ( ((io_pTruncationLocation - pReverseNominee) < (pNominee - io_pTruncationLocation)) ||
			 (pNominee >= (in_pStop-4)) )
		{
			pNominee = pReverseNominee;
		}
		
		//	After all that work, did we just end up where we started?
		if (pNominee == io_pTruncationLocation)
			bGoodLocation = false;
		
		//	Make sure that there's "enough" text left before or after our nominee
		if ( (pNominee <= (in_pStart+4)) || (pNominee >= (in_pStop-4)) )
			bGoodLocation = false;

		//	Only check to see if it's close enough if it's been good so far, and we're willing to try again (address header)
		if (bGoodLocation && in_bIsAddressHeader)
			bGoodLocation = IsTruncationCloseEnough(in_pStart, in_pStop, io_pTruncationLocation, pNominee, in_bStartTruncation, 3);

		if (!bGoodLocation && in_bIsAddressHeader)
		{
			//	Try again - without caring that we're dealing with an address header
			bTryAgain = true;
			in_bIsAddressHeader = false;
		}
	} while (bTryAgain);

	if (bGoodLocation)
	{
		//	Cool - it worked
		io_pTruncationLocation = pNominee;
	}

	return bGoodLocation;
}


// ------------------------------------------------------------------------------------------
//		* Message::CopyText														 [Protected]
// ------------------------------------------------------------------------------------------
//	Copies content and tags, stripping content if specified
//
//	Parameters:
//		in_elementType:				Type of element that we're outputting
//		in_pStart:					Start of text to output
//		in_pStop:					End of text to output
//		in_bStripContent:			Whether or not we're only outputting tags
//		in_nQuoteIncrement:			Amount to increment quote by or 0
//		out_szOutput:				Destination for copied output

void
ContentConcentrator::Message::CopyText(
	ULONG					in_elementType,
	const char *			in_pStart,
	const char *			in_pStop,
	bool					in_bStripContent,
	short					in_nQuoteIncrement,
	CString &				out_szOutput)
{
	if (in_bStripContent)
	{
		if (in_elementType != IDS_CON_CON_TYPE_HEADER)
		{
			if ( IsHTML() )
			{
				//	To do: Output just tags - no content
				
				//	For now this is not allowed.
				ASSERT(0);
			}
			else if ( IsFlowed() )
			{
				const char *	pScan = in_pStart;
				short			nTagLength;
				int				nPotentialCRLFIndex = out_szOutput.GetLength() - 2;
				bool			bKeepCRLFBeforeTag = (nPotentialCRLFIndex >= 0) && !IsCRLF(&static_cast<LPCTSTR>(out_szOutput)[nPotentialCRLFIndex]);
				
				do
				{
					pScan = static_cast<char *>( memchr(pScan, '<', in_pStop-pScan) );
					
					if (pScan)
					{
						nTagLength = IsTag(pScan, in_pStop);
						if (nTagLength > 0)
						{
							//	Check to see if:
							//	* We want to keep a CRLF before the tag (because there isn't
							//	  already one in the output)
							//	* There was a CRLF immediately before this tag to keep
							if ( bKeepCRLFBeforeTag && ((pScan-2) >= in_pStart) && IsCRLF(pScan-2) )
							{
								//	Adjust the pointer and tag length to include the CRLF
								pScan -= 2;
								nTagLength += 2;
							}
							
							//	Include CRLF after tag, if any
							if ( ((pScan + nTagLength) <= (in_pStop-2)) && IsCRLF(pScan + nTagLength) )
							{
								//	Adjust the tag length to include the CRLF
								nTagLength += 2;
								
								//	Remember that we had a CRLF and so won't need one before another tag
								//	(probably can't happen anyway with Format=Flowed, but just in case)
								bKeepCRLFBeforeTag = false;
							}
							
							CopyTextToCString(pScan, pScan+nTagLength, out_szOutput);
							
							//	Move past the tag and search for the next potential tag
							pScan += nTagLength;
						}
						else
						{
							//	Move past the '<' and search for the next potential tag
							pScan++;
						}
					}
				} while ( pScan && (pScan < in_pStop) );
			}
		}
	}
	else
	{
		if (in_nQuoteIncrement == 0)
		{
			//	Not incrementing quote - just copy everything
			CopyTextToCString(in_pStart, in_pStop, out_szOutput);
		}
		else
		{
			if ( IsPlain() || (in_elementType == IDS_CON_CON_TYPE_HEADER) )
			{
				const char *	pLineStart = in_pStart;
				const char *	pLineEnd;
				CString			szQuoteString(kQuoteChar, in_nQuoteIncrement);
				int				nPotentialCRLFIndex = out_szOutput.GetLength() - 2;
				bool			bIsFullLine = (nPotentialCRLFIndex < 0) || IsCRLF(&static_cast<LPCTSTR>(out_szOutput)[nPotentialCRLFIndex]);

				do
				{
					//	Look for the end of the line
					pLineEnd = static_cast<char *>( memchr(pLineStart, '\r', in_pStop-pLineStart) );
					if ( pLineEnd && (pLineEnd < in_pStop) )
					{
						pLineEnd++;

						//	We really expect \r\n
						ASSERT(*pLineEnd == '\n');
						if ( *pLineEnd == '\n' )
							pLineEnd++;
					}
					else
					{
						pLineEnd = in_pStop;
					}

					//	Tack on the quote string before each full line that isn't all
					//	entire message tags (avoids quoting x-flowed line)
					if ( bIsFullLine && !IsAllEntireMessageTags(pLineStart, pLineEnd) )
						out_szOutput += szQuoteString;
					
					//	Copy the line
					CopyTextToCString(pLineStart, pLineEnd, out_szOutput);
					
					//	Prepare for the next line
					pLineStart = pLineEnd;
					bIsFullLine = true;
				} while (pLineStart < in_pStop);
			}
			else	//	Styled body
			{
				//	To do: Perhaps just copy content - quote handling might need to be done elsewhere

				//	For now this is not allowed (styled bodies are special cased in OutputElement)
				ASSERT(0);
			}
		}
	}
}


// ------------------------------------------------------------------------------------------
//		* Message::CopyTextToCString											 [Protected]
// ------------------------------------------------------------------------------------------
//	Copies text to CString
//
//	Parameters:
//		in_pStart:					Start of text to output
//		in_pStop:					End of text to output
//		out_szOutput:				Destination for copied output

void
ContentConcentrator::Message::CopyTextToCString(
	const char *			in_pStart,
	const char *			in_pStop,
	CString &				out_szOutput)
{
	//	Temporarily NULL terminate the string so that we can copy it.
	//	Gross though this is, it's the only way to copy non-NULL terminated text to a
	//	CString while still taking advantage of CStrings ability to automatically grow
	//	if necessary.
	char	cSaveChar = *in_pStop;
	
	//	Evil modification of const data
	*const_cast<char *>(in_pStop) = '\0';

	out_szOutput += in_pStart;

	//	Erase any traces of the evil we did
	*const_cast<char *>(in_pStop) = cSaveChar;
}

