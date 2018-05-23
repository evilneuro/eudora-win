/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					configdata.cpp
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
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include "DebugNewHelpers.h"
#include "configdata.h"


CConfigData 	g_ConfigData;
CValueType		g_ValueTypes;

///////////////////////////////////////////////////////////////////////////////

// (static)
CONFIGDATA CConfigData::m_aConfigData[] = {
// field identifier  attribute(save) 	display
	DS_NAME						,"NAME"						,"<Name>"                         ,"name: "				,
	DS_COMPANY				,"COMPANY"				,"<Company Name>"                 ,NULL						,
	DS_DIVISION				,"DIVISION"				,"<Division of company>"          ,"division: "		,
	DS_POSITION				,"POSITION"				,"<Work Title>"                   ,"title: "			,
	DS_LOCATION				,"LOCATION"				,"<Work Location>"                ,NULL						,
  DS_ADDRESS				,"ADDRESS"				,"<Work Address>"                 ,"office_address: ",
	DS_CITY						,"CITY"						,"<Work City>"                    ,NULL						,
	DS_STATE					,"STATE"					,"<Work State>"                   ,NULL						,
	DS_ZIPCODE				,"ZIPCODE"				,"<Work Zip Code>"                ,NULL						,
	DS_PHONE					,"PHONE"					,"<Work Phone Number>"            ,NULL						,
	DS_CELL						,"CELL"						,"<Work Cellular Number>"         ,"cell_phone: "	,
	DS_PAGER					,"PAGER"					,"<Work Pager Number>"            ,NULL						,
	DS_PEMAIL					,"PEMAIL"					,"<Work Pager Email Address>"     ,NULL						,
	DS_FAX						,"FAX"						,"<Work Facsimile>"               ,"fax: "				,
	DS_EMAIL					,"EMAIL"					,"<Work Email Address>"           ,"email: "			,
	DS_PGPKEY					,"PGPKEY"					,"<Work PGP Key>"                 ,NULL						,
	DS_PAGERTYPE			,"PAGERTYPE"			,"<Work Pager Type>"              ,"pager: "			,
	DS_PAGERPIN				,"PAGERPIN"				,"<Work Pager Pin>"               ,"pager_pin: "	,
	DS_HADDRESS				,"HADDRESS"				,"<Home Address>"                 ,"home_address: ",
	DS_HCITY					,"HCITY"					,"<Home City>"                    ,NULL						,
	DS_HSTATE					,"HSTATE"					,"<Home State>"                   ,NULL						,
	DS_HZIPCODE				,"HZIPCODE"				,"<Home Zip Code>"                ,NULL						,
	DS_HPHONE					,"HPHONE"					,"<Home Phone Number>"            ,"home_phone: "	,
	DS_HCELL					,"HCELL"					,"<Home Cellular Number>"         ,NULL						,
	DS_HPAGER					,"HPAGER"					,"<Home Pager Number>"            ,NULL						,
	DS_HPEMAIL				,"HPEMAIL"				,"<Home Pager Email Address>"     ,NULL						,
	DS_HFAX						,"HFAX"						,"<Home Facsimile>"               ,NULL						,
	DS_HEMAIL					,"HEMAIL"					,"<Home Email Address>"           ,"email2: "			,
	DS_HPGPKEY				,"HPGPKEY"				,"<Home PGP Key>"                 ,NULL						,
	DS_HPAGERTYPE			,"HPAGERTYPE"			,"<Home Pager Type>"              ,NULL						,
  DS_HPAGERPIN			,"HPAGERPIN"			,"<Home Pager Pin>"               ,NULL						,
	DS_USERDEFINED		,"USERDEFINED"		,"<User Defined>"									,NULL						,
};
const int CConfigData::m_nNumEntries = sizeof(CConfigData::m_aConfigData)/sizeof(CONFIGDATA);


///////////////////////////////////////////////////////////////////////////////


int CConfigData::IndexToFieldID(int nIndex)
{
	if (nIndex < 0 || nIndex > m_nNumEntries-1)
		return -1;
	return m_aConfigData[nIndex].nFieldID;
}

int CConfigData::FieldIDToIndex(int nFieldID)
{
	LPCONFIGDATA p = NULL;
	int i;
	
	for (i=0, p=m_aConfigData; p && i < m_nNumEntries; i++, p++)
		if (p->nFieldID == nFieldID)
			return i;
	
	return -1;
}


LPCONFIGDATA CConfigData::FindConfigEntry(DS_FIELD nField)
{
	LPCONFIGDATA p = NULL;
	int i;
	
	for (i=0, p=m_aConfigData; p && i < m_nNumEntries; i++, p++) 
		if (p->nFieldID == nField)
			return p;
	return NULL;
}

LPSTR CConfigData::GetDisplayName(DS_FIELD nField)
{
	LPCONFIGDATA p = FindConfigEntry(nField);
	return p ? p->pszDisplayName : NULL;
}

LPSTR CConfigData::GetAttributeName(DS_FIELD nField)
{
	LPCONFIGDATA p = FindConfigEntry(nField);
	return p ? p->pszSaveName : NULL;
}

int CConfigData::GetFieldFromAttributeName(LPSTR pszAttrib)
{
	LPCONFIGDATA p;
	int i;

	for (i=0, p=m_aConfigData; p && i < m_nNumEntries; i++, p++)
		if (0 == strcmp(p->pszSaveName, pszAttrib))
			return p->nFieldID;
	return 0;
}

int CConfigData::GetFieldFromDisplayName(LPSTR pszDisplay)
{
	LPCONFIGDATA p;
	int i;

	for (i=0, p=m_aConfigData; p && i < m_nNumEntries; i++, p++)
		if (0 == strcmp(p->pszDisplayName, pszDisplay))
			return p->nFieldID;
	return 0;
}

BOOL CConfigData::Enum(PFNENUMCONFIGCB pfnEnumCallback, LPVOID pData)
{
	LPCONFIGDATA p;
	int i;

	if (!pfnEnumCallback)
		return 0;
	for (i=0, p=m_aConfigData; p && i < m_nNumEntries; i++, p++)
		if (!(pfnEnumCallback)(p, pData))
			return 0;
	return 1;
}


///////////////////////////////////////////////////////////////////////////////


VALUETYPE CValueType::m_aValueTypes[] = {
	DST_ASCII			,"ASCII"			,"ASCII string without trailing 0"		,
	DST_ASCIIZ   	,"ASCIIZ"			,"ASCII string with trailing 0"    		,
	DST_BINARY   	,"BINARY"			,"Binary data"                      	,
	DST_GIF      	,"GIF"				,"GIF image"                       		,
	DST_JPG      	,"JPG"				,"JPEG image"                      		,
	DST_IMAGE    	,"IMAGE"			,"Unknown image format"            		,
	DST_WORDDOC  	,"WORDDOC"		,"Microsoft Word document"         		,
	DST_EXCEL    	,"EXCEL"			,"Microsoft Excel spreadsheet"     		,
	DST_URL      	,"URL"				,"URL string with trailing 0"      		
};
int CValueType::m_nNumEntries = sizeof(CValueType::m_aValueTypes)/sizeof(VALUETYPE);


LPVALUETYPE CValueType::FindValueType(DS_TYPE nType)
{
	LPVALUETYPE	p;
	int i;
	
	for (i=0, p=m_aValueTypes; p && i < m_nNumEntries; i++, p++)
		if (p->nValueID == nType)
			return p;
	return NULL;
}


LPSTR CValueType::GetTypeName(DS_TYPE nType)
{
	LPVALUETYPE p = FindValueType(nType);
	return p ? p->pszValueName : NULL;
}


LPSTR CValueType::GetTypeDescription(DS_TYPE nType)
{
	LPVALUETYPE p = FindValueType(nType);
	return p ? p->pszDescription : NULL;
}

BOOL CValueType::Enum(PFNENUMVALUECB pfnEnumCallback, LPVOID pData)
{
	LPVALUETYPE p;
	int i;

	if (!pfnEnumCallback)
		return 0;
	for (i=0, p=m_aValueTypes; p && i < m_nNumEntries-1; i++, p++)
		if (!(*pfnEnumCallback)(p, pData))
			return 0;
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
//


