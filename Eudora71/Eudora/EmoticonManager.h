// EmoticonManager.h
//
// Copyright (c) 1991-2004 by QUALCOMM, Incorporated
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

//

#ifndef __EMOTICONMANAGER_H__
#define __EMOTICONMANAGER_H__

// Routines for handling emoticons. 
//

#include <afx.h>
#include <afxtempl.h>
#include <afxcoll.h>

#include "QCCommandObject.h"
#include "QCCommandDirector.h"

class EmoticonDirector;

const int INVALID_EMOTICON = 1;

typedef enum {
	IMG_SIXTEEN = 16,
	IMG_TWENTYFOUR = 24,
	IMG_THIRTYTWO = 32
} ImageSize;

typedef enum {
	FONT_SMALLEST,
	FONT_SMALLER,
	FONT_MEDIUM,
	FONT_LARGER,
	FONT_LARGEST
} FontSize;

class Emoticon : public QCCommandObject
{
	

public:

	//Emoticon();
	Emoticon(EmoticonDirector* pDirector, CString szImageFullPath16, CString szImageFullPath24, CString szImageFullPath32, CString trigger, CString meaning = "", bool bIsSynonym = false);
	virtual ~Emoticon();

	const CString& GetImageFullPath() const;
	const CString& GetImageFullPathForMenu() const;
	const CString& GetImageFullPath16() const { return m_szImageFullPath16; }
	const CString& GetImageFullPath24() const { return m_szImageFullPath24; }
	const CString& GetImageFullPath32() const { return m_szImageFullPath32; }
	void SetImageFullPath16( CString szImageFullPath16) { m_szImageFullPath16 = szImageFullPath16; }
	void SetImageFullPath24( CString szImageFullPath24) { m_szImageFullPath24 = szImageFullPath24; }
	void SetImageFullPath32( CString szImageFullPath32) { m_szImageFullPath32 = szImageFullPath32; }
	const CString& GetTrigger() const { return m_szTrigger; }
	const CString& GetMeaning() const { return m_szMeaning; }
	const int GetTriggerLength() const { return m_TriggerLength; }

	CImageList *			GetImage() const { return m_pImage; };
	void					SetImage(CImageList * in_pImage) { m_pImage = in_pImage; }
	DWORD					GetImageWidth() const { return m_nImageWidth; }
	void					SetImageWidth(DWORD in_nImageWidth) { m_nImageWidth = in_nImageWidth; }
	DWORD					GetImageHeight() const { return m_nImageHeight; }
	void					SetImageHeight(DWORD in_nImageHeight) { m_nImageHeight = in_nImageHeight; }

	void SetID(WORD ID) { m_ID = ID; }
	const WORD GetID() const { return m_ID; };
	bool IsSynonym() const { return m_bIsSynonym; };
	void SetSynonym() { m_bIsSynonym = true; };
	virtual void Execute(COMMAND_ACTION_TYPE	theAction, void* pData = NULL );
	DECLARE_DYNCREATE(Emoticon);

private:
	CString m_szImageFullPath16;
	CString m_szImageFullPath24;
	CString m_szImageFullPath32;
	CString m_szTrigger;
	CString m_szMeaning;
	int m_TriggerLength;
	CImageList *	m_pImage;
	DWORD			m_nImageWidth;
	DWORD			m_nImageHeight;
	WORD m_ID;
	bool m_bIsSynonym;
};

typedef CTypedPtrList<CObList, Emoticon*>  EmoticonList;
typedef CTypedPtrMap<CMapStringToOb, CString, EmoticonList*> EmoticonMap;
typedef CTypedPtrMap<CMapWordToOb, WORD, Emoticon*> EmoticonIDMap;
typedef CTypedPtrMap<CMapStringToOb, CString, Emoticon*> EmoticonTriggerMap;

class EmoticonDirector : public QCCommandDirector
{

public:
	EmoticonDirector();
	~EmoticonDirector();
	bool Build();
	Emoticon* GetMatchingEmoticon(const char * pCheckTextHere, const char * pStartTextBuffer, const char * pEndTextBuffer, int & nTextReplaceLength, bool bIsHTML, bool bCheckPreviousChar = true);
	bool ParseWordForEmoticons(CString &szWord, bool bIsHTML = false);

	const EmoticonList* GetEmoticonList() { return &m_EmoticonList; }

	void SetEmoticonIDMap(WORD wID, Emoticon* emoticonObj);
	void LookUpEmoticonIDMap(WORD wID, Emoticon* & emoticonObj);
	void LookUpEmoticonTriggerMap(CString &trigger, Emoticon* & emoticonObj);

private:
	static EmoticonMap m_EmoticonMap;
	static EmoticonList	m_EmoticonList;
	static EmoticonIDMap m_EmoticonIDMap;
	static EmoticonTriggerMap m_EmoticonTriggerMap;

	static CMapStringToString m_EscapeCharMap;
	static CStringList m_ImageExtensionList;
	static CMapStringToString m_HTMLEscapeCharMap;

	void TestEmoticonMap();
	void BuildEmoticonList();
	Emoticon* BuildEmoticonObject(const CString & szFileFullPath, ImageSize imgSize, const CString & trigger, const CString & meaning, bool bIsSynonym);
	Emoticon* InsertEmoticonObjectInMap(const CString & szFileName, const CString & szFileFullPath, const CString & szImgExtn, ImageSize imgSize);
	bool ParseEmoticonName(const CString & szFileName, const CString & szFileFullPath, const CString & szImgExtn, CString & trigger, CString & meaning);
	void BuildEscapeCharacterMap();
	void BuildImageExtensionList();
	void BuildHTMLEscapeCharList();
	bool CheckTextForTrigger(const char * szText, int nTextLength, const char * pStartTextBuffer, const char * pEndTextBuffer, Emoticon* pEmoticonObj, int & nTextReplaceLength, bool bIsHTML, bool bCheckPreviousChar = true );
	void BuildEmoticonList(const CString & szDirPath);
	void BuildEmoticonsFromDirectory(const CString & szDirPath, ImageSize imgSize);
	void ConvertIfHTMLEscapeChar(CString &szChar, const char * pCheckTextHere, const char * pEndTextBuffer);

	char m_emoticon_synonym;
};

class InvalidEmoticonException : public CException
{

public:
	BOOL InvalidEmoticonException::GetErrorMessage( LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL );
};

#endif // __EMOTICONMANAGER_H__
