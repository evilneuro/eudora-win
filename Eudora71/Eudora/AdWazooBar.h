// AdWazooBar.h : header file
//
// CAdWazooBar
// Class that handles the particular wazoo container that holds the Ad window
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

#ifndef __ADWAZOOBAR_H__
#define __ADWAZOOBAR_H__


#include "WazooBar.h"
#include "QCCommandActions.h"
#include "QICommandClient.h"

///////////////////////////////////////////////////////////////////////
// CAdWazooBar
//
///////////////////////////////////////////////////////////////////////
class QCCommandObject;

class CAdWazooBar : public CWazooBar, public QICommandClient
{
	DECLARE_DYNCREATE(CAdWazooBar)

// Construction
protected:
	CAdWazooBar();
public:
	CAdWazooBar(CWazooBarMgr* pManager);
	virtual ~CAdWazooBar();

// Attributes
public:

// Operations
public:
	BOOL Create(CWnd* pParentWnd);
	virtual BOOL SetDefaultWazooConfig(int nBar);
	virtual BOOL LoadWazooConfigFromIni();
	virtual BOOL SaveWazooConfigToIni();
	int ImageWidth()	{ return m_ImageWidth; }
	int ImageHeight()	{ return m_ImageHeight; }
	int GetMinWidth();
	int GetMinHeight();
	void NewAdSize(int Width, int Height);
	BOOL RemoveSystemMenu();

// Overrides
public:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual void Notify(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction, void* pData = NULL);

	virtual void OnExtendContextMenu(CMenu* pMenu);

	virtual BOOL ActivateWazooWindow(CRuntimeClass* pRuntimeClass);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdWazooBar)
protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);

	// Generated message map functions
protected:
	//{{AFX_MSG(CAdWazooBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDisabled();
	afx_msg void OnUpdateDisable(CCmdUI* pCmdUI);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void UpdateState();

	int m_ImageWidth, m_ImageHeight;
};

#endif //__ADWAZOOBAR_H__
