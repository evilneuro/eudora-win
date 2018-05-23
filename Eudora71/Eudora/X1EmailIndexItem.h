//	X1EmailIndexItem.h
//
//	Interprets email message information for X1 indexing.
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



#ifndef __X1EmailIndexItem_H__
#define __X1EmailIndexItem_H__

//	X1 Include
#include "X1Wrapper.h"


//	Forward declarations
class CSummary;


//	
class X1EmailIndexItem : public X1::IX1IndexItem
{
  public:
	static const long					kSecondsInADay = 60 * 60 * 24;

										X1EmailIndexItem(
											CSummary *					in_pSummary,
											const char *				in_szFullMessage);

	virtual								~X1EmailIndexItem();

	virtual HRESULT __stdcall			QueryInterface(
											const IID &					in_iid,
											void **						out_ppvObject);
		//	Standard IUnknown::QueryInterface() implementation

	virtual ULONG __stdcall				AddRef();
		//	Standard IUnknown::AddRef() implementation

	virtual ULONG __stdcall				Release();
		//	Standard IUnknown::Release() implementation

    virtual HRESULT __stdcall			raw_GetFieldTextForStorage(
											long						in_nFieldID,
											BSTR *						out_pText);
		//	Returns the field data as text appropriate for storage. For example,
		//	when storing a date value, this function should return the text form
		//	of a double representation of a date (e.g. "35065.0").


    virtual HRESULT __stdcall			raw_IsDifferentFieldTextForIndexing(
											long						in_nFieldID,
											unsigned char *				out_pbIsDifferent);
		//	Returns true if the storage form of the field text is the same as the
		//	indexing form.

	virtual HRESULT __stdcall			raw_GetFieldTextForIndexing(
											long						in_nFieldID,
											BSTR *						out_pText);
		//	Returns the field data as text appropriate for indexing. For example,
		//	when storing a date value, this function should return the string
		//	representation of a date (e.g. "1/1/1996 12:00 am").

	virtual HRESULT __stdcall			raw_GetMaxWordBytesToIndex(
											long *						out_pnMaxWordBytes);
		//	Returns the maximum number of bytes to index. Used to keep the size of
		//	the index down. -1 means unlimited.

  protected:
										X1EmailIndexItem();	//	Disallow default constructor
										X1EmailIndexItem(X1EmailIndexItem & in_rhs);	//	Disallow copy construction

	long								m_nRefCount;
	CString								m_szMailboxRelativePath;
	CString								m_szMailboxRelativePathForIndexing;
	CString								m_szMailboxName;
	unsigned long						m_nUniqueMessageId;
	CString								m_szWho;
	CString								m_szSubject;
	CString								m_szFromHeader;
	CString								m_szToHeader;
	CString								m_szCcHeader;
	CString								m_szBccHeader;
	CString								m_szAllHeaders;
	CString								m_szBody;
	CString								m_szAttachments;
	long								m_Seconds;
	int									m_TimeZoneMinutes;
	long								m_Days;
	short								m_State;
	short								m_Label;
	long								m_nAttachments;
	unsigned long						m_nPersonaHash;
	char								m_Priority;
	CString								m_szPriority;
	char								m_nMood;
	unsigned char						m_ucJunkScore;
	int									m_nSize;
	unsigned long						m_Imflags;
	CString								m_szSubjectPrefix;
	CString								m_szSubjectMain;
};


#endif	//	__X1EmailIndexItem_H__
