//	SearchManagerPersistence.cpp
//
//	Writes out and reads in SearchManager's data.
//	Only for use in SearchManager files.
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
#include "SearchManagerPersistence.h"
#include "SearchManagerInfo.h"
#include "X1EmailScanner.h"
#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"

//	Constants
const char *				SearchManager::XMLParser::kXMLBaseContainer = "IndexedSearchData";
const char *				SearchManager::XMLParser::kKeyDataFormatVersion = "DataFormatVersion";
const char *				SearchManager::XMLParser::kKeyX1SchemaVersion = "SchemaVersion";
const char *				SearchManager::XMLParser::kKeyIndexingScanType = "IndexingScanType";
const char *				SearchManager::XMLParser::kKeyIndexingNextScanType = "IndexingNextScanType";
const char *				SearchManager::XMLParser::kKeyIndexingMailbox = "IndexingMailbox";
const char *				SearchManager::XMLParser::kKeyIndexingCompletionTime = "IndexingCompletionTime";
const char *				SearchManager::XMLParser::kKeySortColumns = "SortColumns";
const char *				SearchManager::XMLParser::kKeySortBy = "SortBy";
const char *				SearchManager::XMLParser::kKeyIndexUpdateActions = "IndexUpdateActions";
const char *				SearchManager::XMLParser::kKeyUpdateAction = "UpdateAction";
const char *				SearchManager::XMLParser::kKeyActionType = "Type";
const char *				SearchManager::XMLParser::kKeyMailboxRelativePath = "MailboxPath";
const char *				SearchManager::XMLParser::kKeyMessageID = "MessageID";

//	Static element map
ElementMap					SearchManager::XMLParser::elementMapArr[] =
								{
									id_baseContainer,				const_cast<char *>(kXMLBaseContainer),
									id_keyDataFormatVersion,		const_cast<char *>(kKeyDataFormatVersion),
									id_keyX1SchemaVersion,			const_cast<char *>(kKeyX1SchemaVersion),
									id_keyIndexingScanType,			const_cast<char *>(kKeyIndexingScanType),
									id_keyIndexingNextScanType,		const_cast<char *>(kKeyIndexingNextScanType),
									id_keyIndexingMailbox,			const_cast<char *>(kKeyIndexingMailbox),
									id_keyIndexingCompletionTime,	const_cast<char *>(kKeyIndexingCompletionTime),
									id_keySortColumns,				const_cast<char *>(kKeySortColumns),
									id_keySortBy,					const_cast<char *>(kKeySortBy),
									id_keyIndexUpdateActions,		const_cast<char *>(kKeyIndexUpdateActions),
									id_keyUpdateAction,				const_cast<char *>(kKeyUpdateAction),
									id_keyActionType,				const_cast<char *>(kKeyActionType),
									id_keyMailboxRelativePath,		const_cast<char *>(kKeyMailboxRelativePath),
									id_keyMessageID,				const_cast<char *>(kKeyMessageID),
									id_none,						"Always the last one"
								};


// ------------------------------------------------------------------------------------------
//		* XMLParser::XMLParser														[Public]
// ------------------------------------------------------------------------------------------
//	XMLParser constructor.

SearchManager::XMLParser::XMLParser()
	:	XmlParser(), m_elementIDsQueue(), m_szElementData(),
		m_eUpdateAction(ua_Invalid), m_szActionMailboxRelativePath(),
		m_nActionMessageID(0)
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
SearchManager::XMLParser::initElementMap(
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
SearchManager::XMLParser::handleData(
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
SearchManager::XMLParser::ProcessDataForID(
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
				//long		nDataFormatVersion = atol(m_szElementData);
			}
			break;
	
		case id_keyX1SchemaVersion:
			{
				//	atol will return 0 if there's an error parsing the data.
				//	That's why we started our schema version numbering with 1.
				long		nSchemaVersion = atol(m_szElementData);

				//	Notify the Search Manager of the schema version
				//	number that we read
				SearchManager::Instance()->NotifyX1SchemaVersion(nSchemaVersion);
			}
			break;

		case id_keyIndexingScanType:
		case id_keyIndexingNextScanType:
			{
				X1EmailScanner::ScanTypeEnum	eScanType = static_cast<X1EmailScanner::ScanTypeEnum>( atoi(m_szElementData) );

				if (in_nID == id_keyIndexingScanType)
					SearchManager::Instance()->m_pInfo->m_eScanType = eScanType;
				else	//	in_nID == id_keyIndexingNextScanType
					SearchManager::Instance()->m_pInfo->m_eNextScanType = eScanType;
			}
			break;

		case id_keyIndexingMailbox:
			SearchManager::Instance()->m_pInfo->m_szIndexingMailboxRelativePath = m_szElementData;
			break;

		case id_keyIndexingCompletionTime:
			SearchManager::Instance()->m_pInfo->m_nLastScanCompletionTime = atol(m_szElementData);
			break;

		case id_keySortBy:
			{
				int			nSortBy = atoi(m_szElementData);
				
				if (nSortBy)
					SearchManager::Instance()->AddSortColumn(nSortBy);
			}
	
		case id_keyActionType:
			m_eUpdateAction = static_cast<UpdateActionEnum>( atoi(m_szElementData) );
			break;
			
		case id_keyMailboxRelativePath:
			m_szActionMailboxRelativePath = m_szElementData;
			break;

		case id_keyMessageID:
			m_nActionMessageID = atol(m_szElementData);
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
SearchManager::XMLParser::startElement(
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

	//	Create pending update action if indicated
	if (in_nID == id_keyUpdateAction)
	{
		//	Reset previously gathered info (redundant since we do the same thing
		//	in endElement, but probably more robust in the face of bad XML.
		m_eUpdateAction = ua_Invalid;
		m_szActionMailboxRelativePath.Empty();
		m_nActionMessageID = 0;
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
SearchManager::XMLParser::endElement(
	int						in_nID,
	const char *			in_szName)
{
	if ( !m_szElementData.IsEmpty() )
	{
		//	We have some data accumulated from inside an XML element, but now we're
		//	ending the current element - so process the data now.
		ProcessDataForID(in_nID);
	}

	//	Done processing current index update action
	if (in_nID == id_keyUpdateAction)
	{
		//	Add index update action with gathered info
		SearchManager::Instance()->AddIndexUpdateAction(
							m_eUpdateAction,
							m_szActionMailboxRelativePath,
							m_nActionMessageID );

		//	Reset previously gathered info
		m_eUpdateAction = ua_Invalid;
		m_szActionMailboxRelativePath.Empty();
		m_nActionMessageID = 0;
	}

	m_elementIDsQueue.pop_back();
	
	return 0;
}
