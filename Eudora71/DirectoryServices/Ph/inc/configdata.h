/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					configdata.h
Description:		
Date:						7/28/97
Version:  			1.0 
Module:					
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef __CONFIGDATA_H__
#define __CONFIGDATA_H__

#include <dir_config.h>
#include <dsdefs.h>


extern const CONFIGDATAENTRIES;


///////////////////////////////////////////////////////////////////////////////

typedef struct tagCONFIGDATA
{
	int				nFieldID;
	LPSTR			pszSaveName;
	LPSTR			pszDisplayName;
	LPSTR			pszDefaultAttribute;
}	CONFIGDATA, * LPCONFIGDATA;

///////////////////////////////////////////////////////////////////////////////
// CConfigData class

class CConfigData
{
public:
	// Define CConfigData::Enum callback type
	typedef BOOL (*PFNENUMCONFIGCB)(LPCONFIGDATA pConfigData, LPVOID pData);

  static int 						IndexToFieldID(int nIndex);
	static int 						FieldIDToIndex(int nFieldID);
	static LPCONFIGDATA 	FindConfigEntry(DS_FIELD nField);
	static LPSTR 					GetDisplayName(DS_FIELD nField);
	static LPSTR 					GetAttributeName(DS_FIELD nField);
	static int			 			GetFieldFromAttributeName(LPSTR pszAttrib);
	static int			 			GetFieldFromDisplayName(LPSTR pszDisplay);
	static BOOL 					Enum(PFNENUMCONFIGCB pfnEnumCallback, LPVOID pData);

public:
	static CONFIGDATA 		m_aConfigData[];
	static const int			m_nNumEntries;
};


///////////////////////////////////////////////////////////////////////////////
// 

typedef struct tagVALUETYPE
{
	int 				nValueID;
	LPSTR				pszValueName;
	LPSTR				pszDescription;
}	VALUETYPE, FAR * LPVALUETYPE;


///////////////////////////////////////////////////////////////////////////////
// CValueType class

class CValueType
{
public:
		// Define CConfigData::Enum callback type
	typedef BOOL (*PFNENUMVALUECB)(LPVALUETYPE pValueType, LPVOID pData);

	static LPVALUETYPE 		FindValueType(DS_TYPE nType);
	static LPSTR 					GetTypeName(DS_TYPE nType);
	static LPSTR 					GetTypeDescription(DS_TYPE nType);
	static BOOL 					Enum(PFNENUMVALUECB pfnEnumCallback, LPVOID pData);

protected:
	static VALUETYPE 			m_aValueTypes[];
	static int						m_nNumEntries;
};


///////////////////////////////////////////////////////////////////////////////
// external declarations

extern CConfigData 	g_ConfigData;
extern CValueType		g_ValueTypes;


#endif	// __CONFIGDATA_H__


