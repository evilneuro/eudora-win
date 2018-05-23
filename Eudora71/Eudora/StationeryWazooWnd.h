// StationeryWazooWnd.h : header file
//
// CStationeryWazooWnd
// Specific implementation of a CWazooWnd.
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

#ifndef __STATIONERYWAZOOWND_H__
#define __STATIONERYWAZOOWND_H__


#include "WazooWnd.h"
#include "StationeryTree.h"


/////////////////////////////////////////////////////////////////////////////
// CStationeryWazooWnd window

class CContainerView;
class QCStationeryCommand;
extern enum COMMAND_ACTION_TYPE;

class CStationeryWazooWnd : public CWazooWnd
{
	DECLARE_DYNCREATE(CStationeryWazooWnd)
	
	QCStationeryCommand* GetSelected();

	void DoReply(COMMAND_ACTION_TYPE theAction);
	void DoUpdateReply(COMMAND_ACTION_TYPE theAction, CCmdUI* pCmdUI);

public:
	CStationeryWazooWnd();
	virtual ~CStationeryWazooWnd();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStationeryWazooWnd)
public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnActivateWazoo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStationeryWazooWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	afx_msg void OnUpdateNew(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEdit(CCmdUI* pCmdUI);
	afx_msg void EnableIfSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReplyWith(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReplyToAllWith(CCmdUI* pCmdUI);
	afx_msg void OnNewStationery();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnNewMessageWith();	
	afx_msg void OnReplyWith();	
	afx_msg void OnReplyToAllWith();
	DECLARE_MESSAGE_MAP()

	BOOL CreateView();

	CContainerView*	m_pStationeryView;
	CStationeryTree	m_StationeryTree;
};


#endif //__STATIONERYWAZOOWND_H__
