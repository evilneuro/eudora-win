//	ConConProfile.cpp
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
#include "ConConProfile.h"
#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"


//	Static element map
ElementMap						ContentConcentrator::CCXMLParser::elementMapArr[] =
										{
											id_baseContainer,				"CCProfiles",
											id_keyProfile,					"profile",
											id_keyElement,					"element",
											id_keyName,						"name",
											id_keyOutput,					"output",
											id_limitBytes,					"Bytes",
											id_limitHeight,					"Height",
											id_limitWidth,					"Width",
											id_modOutputTrim,				"Trim",
											id_modOutputFlatten,			"Flatten",
											id_modOutputQuoteIncrement,		"QuoteIncrement",
											id_none,						"Always the last one"
										};


// ------------------------------------------------------------------------------------------
//		* Profile::Rule::Rule														[Public]
// ------------------------------------------------------------------------------------------
//	Profile::Rule constructor.

ContentConcentrator::Profile::Rule::Rule()
	:	m_type(0), m_szHeaderName(), m_nOutputType(0), m_nOutputBytes(0),
		m_bShouldTrimOutput(false), m_bShouldFlattenOutput(false),
		m_nOutputQuoteIncrement(0)
{

}


// ------------------------------------------------------------------------------------------
//		* Profile::Profile															[Public]
// ------------------------------------------------------------------------------------------
//	Profile constructor.
//
//	Parameters:
//		in_szName:			Name of profile

ContentConcentrator::Profile::Profile(
	const char *			in_szName)
	:	m_szName(in_szName), m_rules()
{
}


// ------------------------------------------------------------------------------------------
//		* Profile::~Profile															[Public]
// ------------------------------------------------------------------------------------------
//	Profile destructor.

ContentConcentrator::Profile::~Profile()
{
	//	Delete m_rules items, but don't bother erasing the list because it is
	//	about to be destroyed anyway.
	Rule *		pRule;
	for ( vector<Rule *>::iterator ruleIterator = m_rules.begin();
		  ruleIterator != m_rules.end();
		  ruleIterator++ )
	{
		pRule = *ruleIterator;
		delete pRule;
	}
}


// ------------------------------------------------------------------------------------------
//		* Profile::AddRule															[Public]
// ------------------------------------------------------------------------------------------
//	Adds rule to profile.
//
//	Parameters:
//		in_szName:			Name of rule
//
//	Returns:
//		Pointer to the rule added. Note that Profile continues to own this object,
//		and that it must *not* be deleted.

ContentConcentrator::Profile::Rule *
ContentConcentrator::Profile::AddRule()
{
	Rule *	pRule = DEBUG_NEW_NOTHROW Rule();

	if (pRule)
		m_rules.push_back(pRule);

	return pRule;
}


// ------------------------------------------------------------------------------------------
//		* Profile::GetCurrentRule													[Public]
// ------------------------------------------------------------------------------------------
//	Retrieves the current rule for use during loading.
//
//	Returns:
//		A pointer to the current rule.

ContentConcentrator::Profile::Rule *
ContentConcentrator::Profile::GetCurrentRule()
{
	return m_rules.back();
}



// ------------------------------------------------------------------------------------------
//		* CCXMLParser::CCXMLParser													[Public]
// ------------------------------------------------------------------------------------------
//	CCXMLParser constructor.

ContentConcentrator::CCXMLParser::CCXMLParser()
	:	XmlParser(), m_elementIDsQueue(), m_szElementData(),
		m_bIgnoreUntilNextProfile(false)
{

}


// ------------------------------------------------------------------------------------------
//		* CCXMLParser::initElementMap											   [Private]
// ------------------------------------------------------------------------------------------
//	Inits the element map for the base class XmlParser to use.
//
//	Parameters:
//		out_pMap:			Place to put our pre-existing element map

//	Returns:
//		true for success every time - providing the pre-existing element map is easy.

bool
ContentConcentrator::CCXMLParser::initElementMap(
	ElementMap **			out_pMap)
{
	*out_pMap = elementMapArr;
	
	return true;
}


// ------------------------------------------------------------------------------------------
//		* CCXMLParser::handleData												   [Private]
// ------------------------------------------------------------------------------------------
//	Accumulates data for use in endElement.
//
//	Parameters:
//		in_nID:				ID for current element
//		in_pData:			Pointer to data to accumulate
//		in_nDataLength:		Length of data to accumulate

void
ContentConcentrator::CCXMLParser::handleData(
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
//		* CCXMLParser::ProcessDataForID											   [Private]
// ------------------------------------------------------------------------------------------
//	Processes accumulated data for a given ID.
//
//	Parameters:
//		in_nID:				ID for element to process

void
ContentConcentrator::CCXMLParser::ProcessDataForID(
	int						in_nID)
{
	if (!m_bIgnoreUntilNextProfile && m_pCurrentRule)
	{
		switch (in_nID)
		{
			case id_keyElement:
				{
					//	See if it matches any of the known output types - passing -1 for the
					//	last parameter specfies that we want a match for the entire string.
					int		nFoundStringIndex = FindRStringIndexI( IDS_CON_CON_TYPE_ANY, IDS_CON_CON_TYPE_COMPLETE,
																   m_szElementData, -1 );

					//	We found a match if the returned index is not -1.
					if (nFoundStringIndex != -1)
						m_pCurrentRule->SetType(IDS_CON_CON_TYPE_ANY + nFoundStringIndex);
				}
				break;

			case id_keyName:
				m_pCurrentRule->SetHeaderName(m_szElementData);
				break;
				
			case id_keyOutput:
				{
					//	See if it matches any of the known output types - passing -1 for the
					//	last parameter specfies that we want a match for the entire string.
					int		nFoundStringIndex = FindRStringIndexI( IDS_CON_CON_OUT_TYPE_RAW, IDS_CON_CON_OUT_TYPE_SUMMARY,
																   m_szElementData, -1 );
					
					//	We found a match if the returned index is not -1.
					if (nFoundStringIndex != -1)
						m_pCurrentRule->SetOutputType(IDS_CON_CON_OUT_TYPE_RAW + nFoundStringIndex);
				}
				break;

			case id_limitBytes:
				{
					int		nOutputBytes = atoi(m_szElementData);
	
					//	Make sure that it's not 0 which indicates an error in the conversion.
					//	Note that 0 is thus an illegal and ignored value.
					if (nOutputBytes != 0)
						m_pCurrentRule->SetOutputBytes(nOutputBytes);
				}
				break;

			case id_limitHeight:
				//	Not handled yet
				break;

			case id_limitWidth:
				//	Not handled yet
				break;

			case id_modOutputQuoteIncrement:
				{
					short		nQuoteIncrement = static_cast<short>( atoi(m_szElementData) );
					
					//	Make sure that it's not 0 which indicates an error in the conversion.
					//	Note that 0 is thus an illegal and ignored value.
					if (nQuoteIncrement != 0)
						m_pCurrentRule->SetOutputQuoteIncrement(nQuoteIncrement);
				}
				break;
		}
	}
	
	m_szElementData.Empty();
}


// ------------------------------------------------------------------------------------------
//		* CCXMLParser::startElement												   [Private]
// ------------------------------------------------------------------------------------------
//	Called when we hit the start tag for a given element.
//
//	Parameters:
//		in_nID:				ID for current element
//		in_szName:			Name of element
//		in_AttributeArr:	Attributes found inside the start tag

int
ContentConcentrator::CCXMLParser::startElement(
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

	//	Create profile or element if indicated
	if (in_nID == id_keyProfile)
	{
		int		i;

		//	Look for the profile name
		for (i = 0; in_AttributeArr[i] != NULL; i++)
		{
			if (stricmp(in_AttributeArr[i], "name") == 0)
			{
				//	Actual name value is the next attribute in the array
				i++;
				break;
			}
		}

		if (in_AttributeArr[i] != NULL)
		{
			m_pCurrentProfile = ContentConcentrator::Instance()->AddProfile(in_AttributeArr[i]);
			m_bIgnoreUntilNextProfile = (m_pCurrentProfile == NULL);
		}
		else
		{
			//	Argh! No profile name! Well fine, I'm ignoring this profile then!
			ASSERT(0);
			m_bIgnoreUntilNextProfile = true;
		}
	}
	else if ( (in_nID == id_keyElement) && !m_bIgnoreUntilNextProfile )
	{
		m_pCurrentRule = m_pCurrentProfile->AddRule();
	}
	
	m_elementIDsQueue.push_front(in_nID);
	
	return 0;
}


// ------------------------------------------------------------------------------------------
//		* CCXMLParser::endElement												   [Private]
// ------------------------------------------------------------------------------------------
//	Called when we hit the end tag for a given element.
//
//	Parameters:
//		in_nID:				ID for current element
//		in_szName:			Name of element

int
ContentConcentrator::CCXMLParser::endElement(
	int						in_nID,
	const char *			in_szName)
{
	if ( !m_szElementData.IsEmpty() )
	{
		//	We have some data accumulated from inside an XML element, but now we're
		//	ending the current element - so process the data now.
		ProcessDataForID(in_nID);
	}

	if (!m_bIgnoreUntilNextProfile && m_pCurrentRule)
	{
		if (in_nID == id_modOutputTrim)
			m_pCurrentRule->SetShouldTrimOutput(true);
		else if (in_nID == id_modOutputFlatten)
			m_pCurrentRule->SetShouldFlattenOutput(true);
	}
	
	m_elementIDsQueue.pop_back();
	
	return 0;
}
