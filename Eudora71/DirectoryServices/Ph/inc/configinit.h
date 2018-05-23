/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					configinit.h
Description:		Default initialization from an ".ini" file for PH or Finger
Date:						9/4/97
Version:  			1.0 
Module:					PH.DLL (PH protocol directory service object)
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
#ifndef __CONFIGINIT_H__
#define __CONFIGINIT_H__

#include <dspapi.h>


enum DTYPE {DT_INT, DT_STRING};


typedef struct
{
	DTYPE			dt;
	LPSTR			pszKey;				// ini file key (read)
	UINT			nIDKey;				// resource ID of registry key string (write)
	UINT			nDefValue;		// default integer value or if string, the resource id to load for default value
}	IENTRY, FAR * LPIENTRY;

///////////////////////////////////////////////////////////////////////////////
// CConfigInit class (base class for PH and Finger initializations)

class CConfigInit
{
public:
	CConfigInit(IDirConfig * pIDirConfig);
	virtual ~CConfigInit();

	BOOL					Initialize(LPSTR pszFileRoot);
  void 					CreateSection(LPSTR pszWriteSection);
  void 					StoreFixed(LPIENTRY pIEntry, int nNumEntries, LPSTR pszReadSection, LPSTR pszWriteSection);
	void 					CopyString(LPSTR pszReadKey, UINT uIDKey, LPSTR pszReadSection, LPSTR pszWriteSection);
	void 					CopyInt(LPSTR pszReadKey, UINT uIDKey, LPSTR pszReadSection, LPSTR pszWriteSection, UINT nDefVal);
	virtual BOOL	LoadPreConfigData() = 0;

protected:
	IDirConfig *	m_pIDirConfig;
	LPSTR					m_pszFileName;
	char					m_szBuffer[512];
};

///////////////////////////////////////////////////////////////////////////////
// CPHInit class

class CPHInit : public CConfigInit
{
public:
	CPHInit(IDirConfig * pIDirConfig) : CConfigInit(pIDirConfig) {}
	virtual BOOL	LoadPreConfigData();
  void 					StoreAttribTable(LPSTR pszReadSection, LPSTR pszWriteSection);
};


///////////////////////////////////////////////////////////////////////////////
// CFingerInit class

class CFingerInit : public CConfigInit
{
public:
	CFingerInit(IDirConfig * pIDirConfig) : CConfigInit(pIDirConfig) {}
	virtual BOOL	LoadPreConfigData();
};


#endif  // __CONFIGINIT_H__


