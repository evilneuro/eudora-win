//	ConConProfile.h
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



#ifndef __ConConProfile_H__
#define __ConConProfile_H__


#include "ContentConcentrator.h"
#include "xml_parser_base.h"

#include <vector>


class ContentConcentrator::Profile
{
  public:
	class Rule
	{
	  public:
										Rule();

		//	Accessors
		UINT							GetType() const { return m_type; }
		void							SetType(UINT in_type) { m_type = in_type; }
		const CString &					GetHeaderName() const { return m_szHeaderName; }
		void							SetHeaderName(const char * in_szHeaderName) { m_szHeaderName = in_szHeaderName; }
		long							GetOutputType() const { return m_nOutputType; }
		void							SetOutputType(UINT in_nOutputType) { m_nOutputType = in_nOutputType; }
		UINT							GetOutputBytes() const { return m_nOutputBytes; }
		void							SetOutputBytes(long in_nOutputBytes) { m_nOutputBytes = in_nOutputBytes; }
		bool							ShouldTrimOutput() const { return m_bShouldTrimOutput; }
		void							SetShouldTrimOutput(bool in_bShouldTrimOutput) { m_bShouldTrimOutput = in_bShouldTrimOutput; }
		bool							ShouldFlattenOutput() const { return m_bShouldFlattenOutput; }
		void							SetShouldFlattenOutput(bool in_bShouldFlattenOutput) { m_bShouldFlattenOutput = in_bShouldFlattenOutput; }
		short							GetOutputQuoteIncrement() const { return m_nOutputQuoteIncrement; }
		void							SetOutputQuoteIncrement(short in_nOutputQuoteIncrement) { m_nOutputQuoteIncrement = in_nOutputQuoteIncrement; }

	  protected:
		UINT							m_type;
		CString							m_szHeaderName;
		long							m_nOutputType;
		UINT							m_nOutputBytes;
		bool							m_bShouldTrimOutput;
		bool							m_bShouldFlattenOutput;
		short							m_nOutputQuoteIncrement;
	};

									Profile(
											const char *			in_szName);
									~Profile();

	//	Accessors
	const CString &					GetName() const { return m_szName; }

	Rule *							AddRule();
	Rule *							GetCurrentRule();

	vector<Rule *> &				GetRules() { return m_rules; }

  protected:
	CString							m_szName;
	vector<Rule *>					m_rules;
};


class ContentConcentrator::CCXMLParser : public XmlParser
{
  public:
									CCXMLParser();

  private:
	enum XMLElementIDs
	{
		id_baseContainer,
		id_keyProfile,
		id_keyElement,
		id_keyName,
		id_keyOutput,
		id_limitBytes,
		id_limitHeight,
		id_limitWidth,
		id_modOutputTrim,
		id_modOutputFlatten,
		id_modOutputQuoteIncrement
	};

	//	Hoist some items into our namespace
	typedef ContentConcentrator::Profile			Profile;
	typedef ContentConcentrator::Profile::Rule		Rule;

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
	bool							m_bIgnoreUntilNextProfile;
	Profile *						m_pCurrentProfile;
	Rule *							m_pCurrentRule;
};


#endif	//	__ConConProfile_H__
