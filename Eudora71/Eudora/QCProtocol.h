// QCProtocol.h: interface for the QCProtocol class.
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#if !defined(AFX_QCPROTOCOL_H__F2AD9DE3_2874_11D1_8423_00805FD2F268__INCLUDED_)
#define AFX_QCPROTOCOL_H__F2AD9DE3_2874_11D1_8423_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCChildToolBar.h"

enum QCPROTOCOLID 
{
	QCP_FIND,
	QCP_QUIET_PRINT,
	QCP_SAVE_INFO,
	QCP_GET_MESSAGE,
	QCP_TRANSLATE,
	QCP_SPELL,
	QCP_METRICS,
	QCP_FORMAT_TOOLBAR,
	QCP_READ_TOOLBAR,
	QCP_MOODMAIL
};


class QCProtocol  
{
public:
	QCProtocol();
	virtual ~QCProtocol();
	
	virtual BOOL CheckSpelling(BOOL);

	virtual BOOL DoFindFirst(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);
	virtual BOOL DoFindNext(const char* szSearch, BOOL bMatchCase, BOOL bWholeWord, BOOL bSelect);

	virtual BOOL QuietPrint();
	virtual HRESULT SaveInfo();
		
	static QCProtocol* QueryProtocol(QCPROTOCOLID theID, CObject* pObject);

	virtual QCChildToolBar* GetFormatToolbar();
	virtual QCChildToolBar* GetToolbar();

	virtual BOOL GetSelectedText(CString& szText);

	virtual BOOL GetSelectedTextAndTrim(
						CString &		out_szText,
						bool			in_bAllowMultipleLines = true,
						bool			in_bDoSuperTrim = false,
						int				in_nMaxLength = 0,
						bool			in_bUseElipsisIfTruncating = false);

	virtual BOOL GetAllText(CString& szText);
	virtual BOOL GetSelectedHTML(CString& szHTML);
	virtual BOOL GetAllHTML(CString& szHTML);

	virtual BOOL SetSelectedText(const char* szText, bool bSign);
	virtual BOOL SetAllText(const char* szText, bool bSign);
	virtual BOOL SetSelectedHTML(const char* szHTML, bool bSign);
	virtual BOOL SetAllHTML(const char* szHTML, bool bSign);
	virtual BOOL PasteOnDisplayHTML(const char* szHTML, bool bSign);
	virtual BOOL RefreshMessage();
	virtual BOOL SetAssociatedFiles(CStringList * in_pAssociatedFiles);

	virtual BOOL GetMessageAsHTML(CString& msg, BOOL IncludeHeaders = FALSE);
	virtual BOOL GetMessageAsText(CString& msg, BOOL IncludeHeaders = FALSE); 
	virtual BOOL OnSignatureChanged();

	virtual int GetTotalHeight();

	virtual bool IsReadOnly(){ return FALSE; }
};

#endif // !defined(AFX_QCPROTOCOL_H__F2AD9DE3_2874_11D1_8423_00805FD2F268__INCLUDED_)
