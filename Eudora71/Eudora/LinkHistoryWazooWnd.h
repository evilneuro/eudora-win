//	LinkHistoryWazooWnd.h
//
//	Specific implementation of a CWazooWnd for Link History information.
//
//	Copyright (c) 1999 by QUALCOMM, Incorporated
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



#ifndef __LINKHISTORYWAZOOWND_H__
#define __LINKHISTORYWAZOOWND_H__


#include "WazooWnd.h"
#include "QICommandClient.h"


class CLinkHistoryList;

class CLinkHistoryWazooWnd : public CWazooWnd, public QICommandClient
{
	DECLARE_DYNCREATE(CLinkHistoryWazooWnd)

public:
	enum WindowIDs { id_LinkHistoryList = 10, id_ViewButton, id_RemoveButton };
	
						CLinkHistoryWazooWnd();
	virtual				~CLinkHistoryWazooWnd();


	virtual void		OnActivateWazoo();
	virtual BOOL		OnCmdMsg(
							UINT					nID,
							int						nCode,
							void *					pExtra,
							AFX_CMDHANDLERINFO *	pHandlerInfo);
	virtual BOOL		PreTranslateMessage(
							MSG *					pMsg);
	void				UpdateButtonsEnabledStates();
	virtual void		Notify(
							QCCommandObject *		pObject,
							COMMAND_ACTION_TYPE		theAction,
							void *					pData = NULL);

protected:
	afx_msg int			OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnDestroy();
	afx_msg BOOL		OnEraseBkgnd(CDC *pDC);
	afx_msg void		OnSize(UINT nType, int cx, int cy);
	afx_msg void		OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL		OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );
    afx_msg void		OnItemChange(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

	void				OnViewLink();
	void				OnRemoveLink();

	CLinkHistoryList *	m_pLinkHistoryList;
	CButton				m_ViewButton;
	CButton				m_RemoveButton;
	int					m_iButtonHeight;
};


#endif //__LINKHISTORYWAZOOWND_H__
