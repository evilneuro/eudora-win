//	XMLWriter.cpp
//
//	Simple class to aid writing basic XML files.
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


#include "stdafx.h"
#include "XMLWriter.h"

#include "fileutil.h"


// ------------------------------------------------------------------------------------------
//		* XMLWriter::XMLWriter														[Public]
// ------------------------------------------------------------------------------------------
//	XMLWriter constructor.

XMLWriter::XMLWriter(
	JJFileMT &				in_refFile)
	:	m_refFile(in_refFile), m_nIndentLevel(0), m_bLastWriteEndedLine(false)
{

}


// ------------------------------------------------------------------------------------------
//		* XMLWriter::~XMLWriter														[Public]
// ------------------------------------------------------------------------------------------
//	XMLWriter destructor.

XMLWriter::~XMLWriter()
{
	//	Sanity check our tag closing
	ASSERT(m_nIndentLevel == 0);
}


// ------------------------------------------------------------------------------------------
//		* XMLWriter::WriteIndentation												[Public]
// ------------------------------------------------------------------------------------------
//	Writes out current indentation if appropriate.

void
XMLWriter::WriteIndentation()
{
	//	Write out indentation if:
	//	* Our last write ended a line
	//	* Our indentation level is greater than 0
	if ( m_bLastWriteEndedLine && (m_nIndentLevel > 0) )
	{
		CString		szIndentation;

		szIndentation.Preallocate(m_nIndentLevel + 1);
		
		for(int i = 0; i < m_nIndentLevel; i++)
			szIndentation += "\t";

		m_refFile.Put(szIndentation);
	}
}


// ------------------------------------------------------------------------------------------
//		* XMLWriter::WriteTagStart													[Public]
// ------------------------------------------------------------------------------------------
//	Writes start XML tag.

void
XMLWriter::WriteTagStart(
	const char *			in_szTag,
	bool					in_bEndsLine)
{
	//	Write out any appropriate indentation
	WriteIndentation();
	
	//	Write out the start tag
	CString		szTagStart;

	szTagStart.Format("<%s>", in_szTag);
	
	if (in_bEndsLine)
		m_refFile.PutLine(szTagStart);
	else
		m_refFile.Put(szTagStart);

	//	Increase indentation level (doesn't take effect unless we end a line)
	m_nIndentLevel++;

	//	Remember whether or not we ended the line
	m_bLastWriteEndedLine = in_bEndsLine;
}


// ------------------------------------------------------------------------------------------
//		* XMLWriter::WriteTagEnd													[Public]
// ------------------------------------------------------------------------------------------
//	Writes end XML tag.

void
XMLWriter::WriteTagEnd(
	const char *			in_szTag,
	bool					in_bEndsLine)
{
	//	Decrease indentation level (doesn't take effect unless we end a line)
	m_nIndentLevel--;
	
	//	Write out any appropriate indentation
	WriteIndentation();
	
	//	Write out the end tag
	CString		szTagEnd;

	szTagEnd.Format("</%s>", in_szTag);
	
	if (in_bEndsLine)
		m_refFile.PutLine(szTagEnd);
	else
		m_refFile.Put(szTagEnd);

	//	Sanity check our tag closing
	ASSERT(m_nIndentLevel >= 0);

	//	Remember whether or not we ended the line
	m_bLastWriteEndedLine = in_bEndsLine;
}


// ------------------------------------------------------------------------------------------
//		* XMLWriter::WriteTaggedData												[Public]
// ------------------------------------------------------------------------------------------
//	Writes end XML tag.

void
XMLWriter::WriteTaggedData(
	const char *			in_szTag,
	bool					in_bEndsLine,
	const char *			in_szFormat,
	...)
{
	//	Format the tag data using the variable arguments
	CString		szTagData;

	va_list argList;
	va_start(argList, in_szFormat);
	szTagData.FormatV(in_szFormat, argList);
	va_end(argList);

	//	Encode any characters that could prove problematic when reading in the XML.
	//	Expat automatically handles decoding these for us, so we don't need to
	//	worry about handling them when reading the XML.
	szTagData.Replace("&", "&amp;");
	szTagData.Replace("<", "&lt;");
	szTagData.Replace(">", "&gt;");

	//	Write out any appropriate indentation
	WriteIndentation();
	
	//	Write out the entire tag
	CString		szTag;

	szTag.Format("<%s>%s</%s>", in_szTag, szTagData, in_szTag);
	
	if (in_bEndsLine)
		m_refFile.PutLine(szTag);
	else
		m_refFile.Put(szTag);

	//	Remember whether or not we ended the line
	m_bLastWriteEndedLine = in_bEndsLine;
}
