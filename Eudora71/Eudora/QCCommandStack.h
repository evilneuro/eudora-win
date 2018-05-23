// QCCommandStack.h: interface for the QCCommandStack class.
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
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCOMMANDSTACK_H__3068C5AC_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
#define AFX_QCCOMMANDSTACK_H__3068C5AC_AB8E_11D0_97B6_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class QCCommandObject;
extern enum COMMAND_ACTION_TYPE;


class QCCommandStack  
{
	class QCCommandMapEntry : public CObject
	{
	public:
		COMMAND_ACTION_TYPE	m_theAction;
		WORD				m_wCommandID;
		QCCommandObject*	m_pObject;
		UINT				m_uCount;		
	};

	class QCCommandMap
	{
		WORD			m_wLastCommand;
		CMapWordToPtr	m_theMap;
		CWordArray		m_theDeletedIDArray;

	public:
		QCCommandMap(); 
		virtual ~QCCommandMap();
		
		WORD Add(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction);
		void Delete(WORD wID, COMMAND_ACTION_TYPE theAction, bool bDeleteCommandObject = false);
		WORD FindCommandID(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction);
		BOOL Lookup(WORD wCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction);
	};

	QCCommandMapEntry	m_theSelectedCommand;
	CPtrList			m_theStack;
		
public:
	QCCommandStack();
	virtual ~QCCommandStack();
	
	UINT GetCount() const { return m_theStack.GetCount(); }	
	void NewCommandMap(); 
	void Pop();
		
	WORD AddCommand(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction);
	void DeleteCommand(WORD wID, COMMAND_ACTION_TYPE theAction, bool bDeleteCommandObject = false);
	WORD FindCommandID( QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction);
	WORD FindOrAddCommandID( QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction);
	BOOL Lookup(WORD wCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction);
	void SaveCommand(WORD wCommandID);
	BOOL GetCommand(WORD uCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction);
	void DeleteAllMenuCommands(CMenu* pMenu, COMMAND_ACTION_TYPE theAction, bool bDeleteCommandObjects = false);
	void GetSavedCommand(WORD* pCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction);
};

#endif // !defined(AFX_QCCOMMANDSTACK_H__3068C5AC_AB8E_11D0_97B6_00805FD2F268__INCLUDED_)
