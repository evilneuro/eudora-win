//	SearchManagerPersistence.h
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



#ifndef __SearchManagerPersistence_H__
#define __SearchManagerPersistence_H__

#include "SearchManager.h"
#include "SearchManagerInfo.h"
#include "xml_parser_base.h"


class SearchManager::XMLParser : public XmlParser
{
  public:
	//	Constants
	static const long				kDataFormatVersion = 1;

	static const char *				kXMLBaseContainer;
	static const char *				kKeyDataFormatVersion;
	static const char *				kKeyX1SchemaVersion;
	static const char *				kKeyIndexingScanType;
	static const char *				kKeyIndexingNextScanType;
	static const char *				kKeyIndexingMailbox;
	static const char *				kKeyIndexingCompletionTime;
	static const char *				kKeySortColumns;
	static const char *				kKeySortBy;
	static const char *				kKeyIndexUpdateActions;
	static const char *				kKeyUpdateAction;
	static const char *				kKeyActionType;
	static const char *				kKeyMailboxRelativePath;
	static const char *				kKeyMessageID;

									XMLParser();

  private:
	//	Typedef to hoist IndexUpdateAction into our namespace
	typedef SearchManager::IndexUpdateAction		IndexUpdateAction;
	typedef SearchManager::UpdateActionEnum			UpdateActionEnum;

	enum XMLElementIDs
	{
		id_baseContainer,
		id_keyDataFormatVersion,
		id_keyX1SchemaVersion,
		id_keyIndexingScanType,
		id_keyIndexingNextScanType,
		id_keyIndexingMailbox,
		id_keyIndexingCompletionTime,
		id_keySortColumns,
		id_keySortBy,
		id_keyIndexUpdateActions,
		id_keyUpdateAction,
		id_keyActionType,
		id_keyMailboxRelativePath,
		id_keyMessageID
	};

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
	IndexUpdateAction *				m_pCurrentIndexUpdateAction;
	UpdateActionEnum				m_eUpdateAction;
	CString							m_szActionMailboxRelativePath;
	long							m_nActionMessageID;

};


#endif	//	__SearchManagerPersistence_H__
