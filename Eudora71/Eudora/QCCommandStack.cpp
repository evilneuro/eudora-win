// QCCommandStack.cpp: implementation of the QCCommandStack class.
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

#include "stdafx.h"

#include "resource.h"

#include "QCCommandActions.h"
#include "QCCommandObject.h"
#include "QCCommandStack.h"

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QCCommandStack::QCCommandStack()
{
	m_theSelectedCommand.m_wCommandID = 0;
	m_theSelectedCommand.m_pObject = NULL;		
	m_theSelectedCommand.m_theAction = CA_NONE;		
}

QCCommandStack::~QCCommandStack()
{
	QCCommandMap*	pMap;

	while( GetCount() )
	{
		// Somebody didn't clean up
		ASSERT(0);

		pMap = ( QCCommandMap* ) m_theStack.RemoveTail();	
		delete pMap;
	}
}

void QCCommandStack::NewCommandMap()
{
	QCCommandMap* pMap = DEBUG_NEW_NOTHROW QCCommandMap();

	if (pMap)
		m_theStack.AddTail(pMap);
}

void QCCommandStack::Pop()
{
	if (GetCount() == 0)
		ASSERT(0);
	else
	{
		QCCommandMap* pMap = (QCCommandMap*)m_theStack.RemoveTail();	
		delete pMap;
	}
}

WORD QCCommandStack::AddCommand(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction)
{
	if (GetCount() == 0)
	{
		ASSERT(0);
		return 0;
	}
	
	QCCommandMap* pMap = (QCCommandMap*)m_theStack.GetTail();
	return pMap->Add(pObject, theAction);	
}

void QCCommandStack::DeleteCommand( WORD wID, COMMAND_ACTION_TYPE theAction, bool bDeleteCommandObject)
{
	if (GetCount() == 0)
		ASSERT(0);
	else
	{
		QCCommandMap* pMap = (QCCommandMap*)m_theStack.GetTail();

		pMap->Delete(wID, theAction, bDeleteCommandObject);
	}
}

WORD QCCommandStack::FindCommandID(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction)
{
	if (GetCount() == 0)
	{
		ASSERT(0);
		return 0;
	}
	
	QCCommandMap* pMap = (QCCommandMap*)m_theStack.GetTail();
	return pMap->FindCommandID(pObject, theAction);
}

WORD QCCommandStack::FindOrAddCommandID( QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction)
{
	WORD wID = FindCommandID(pObject, theAction);

	if (wID)
		return wID;

	return AddCommand(pObject, theAction);
}


BOOL QCCommandStack::Lookup(WORD wCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction)
{
	if (GetCount() == 0)
	{
		ASSERT(0);
		return FALSE;
	}
	
	QCCommandMap* pMap = (QCCommandMap*)m_theStack.GetTail();
	return pMap->Lookup(wCommandID, ppObject, pAction);
}

void QCCommandStack::SaveCommand(WORD wCommandID)
{
	if (GetCount() == 0)
	{
		ASSERT(0);
		return;
	}
	
	QCCommandMap* pMap = (QCCommandMap*)m_theStack.GetTail();
	QCCommandObject* pObject;
	COMMAND_ACTION_TYPE	theAction; 
	
	if (pMap->Lookup(wCommandID, &pObject, &theAction))
	{
		m_theSelectedCommand.m_wCommandID = wCommandID;
		m_theSelectedCommand.m_pObject = pObject;		
		m_theSelectedCommand.m_theAction = theAction;		
	}	
	else
	{
		m_theSelectedCommand.m_wCommandID = 0;
		m_theSelectedCommand.m_pObject = NULL;		
		m_theSelectedCommand.m_theAction = CA_NONE;		
	}
}

BOOL QCCommandStack::GetCommand(WORD uCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction)
{
	if (uCommandID == m_theSelectedCommand.m_wCommandID)
	{		
		*ppObject = m_theSelectedCommand.m_pObject;		
		*pAction = m_theSelectedCommand.m_theAction;		
		return TRUE;
	}
	
	return Lookup(uCommandID, ppObject, pAction);
}

QCCommandStack::QCCommandMap::QCCommandMap()
{
	m_wLastCommand = QC_FIRST_COMMAND_ID - 1;
}

QCCommandStack::QCCommandMap::~QCCommandMap()
{
	POSITION pos = m_theMap.GetStartPosition();
	
	while( pos != NULL )
	{
		QCCommandMapEntry*	pEntry;
		WORD wCommand;

		m_theMap.GetNextAssoc(pos, wCommand, (void*&)pEntry);
		delete pEntry;			
	}
}

WORD QCCommandStack::QCCommandMap::Add(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction)
{
	POSITION pos = m_theMap.GetStartPosition();
	QCCommandMapEntry* pEntry;
	
	while (pos)
	{
		WORD wID;

		m_theMap.GetNextAssoc(pos, wID, (void*&)pEntry);
		
		if (pEntry->m_theAction == theAction && pEntry->m_pObject == pObject)
		{
			pEntry->m_uCount += 1;
			return wID;
		}
	}

	pEntry = DEBUG_NEW_MFCOBJ_NOTHROW QCCommandMapEntry;
	if (!pEntry)
		return 0;		

	if (m_theDeletedIDArray.GetUpperBound() == -1)
	{
		if (m_wLastCommand + 1 == QC_LAST_COMMAND_ID)
		{
			delete pEntry;
			return 0;
		}
	
		pEntry->m_wCommandID = ++m_wLastCommand;
	}
	else
	{
		pEntry->m_wCommandID = m_theDeletedIDArray[m_theDeletedIDArray.GetUpperBound()];
		m_theDeletedIDArray.RemoveAt(m_theDeletedIDArray.GetUpperBound());
	}
		
	pEntry->m_theAction = theAction;
	pEntry->m_pObject = pObject;
	pEntry->m_uCount = 1;

	m_theMap.SetAt(pEntry->m_wCommandID, pEntry);
	return pEntry->m_wCommandID;
}

void QCCommandStack::QCCommandMap::Delete(WORD wID, COMMAND_ACTION_TYPE theAction, bool bDeleteCommandObject)
{
	QCCommandMapEntry*	pEntry;

	if (m_theMap.Lookup(wID, (void*&)pEntry) && pEntry->m_theAction == theAction)
	{		
		pEntry->m_uCount -= 1;

		if (pEntry->m_uCount == 0)
		{
			m_theMap.RemoveKey(wID);

			if (bDeleteCommandObject)
				delete pEntry->m_pObject;

			delete pEntry;
			m_theDeletedIDArray.Add(wID);
		}
	}
}

BOOL QCCommandStack::QCCommandMap::Lookup(WORD wCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE*	pAction)
{
	QCCommandMapEntry*	pEntry;

	if (m_theMap.Lookup(wCommandID, (void*&)pEntry))
	{
		*ppObject = pEntry->m_pObject;
		*pAction = pEntry->m_theAction;
		return TRUE;
	}
	
	return FALSE;		
}

WORD QCCommandStack::QCCommandMap::FindCommandID(QCCommandObject* pObject, COMMAND_ACTION_TYPE theAction)
{
	POSITION pos = m_theMap.GetStartPosition();
	
	while (pos)
	{
		QCCommandMapEntry* pEntry;
		WORD wID;

		m_theMap.GetNextAssoc(pos, wID, (void*&)pEntry);
		
		if (pEntry->m_theAction == theAction && pEntry->m_pObject == pObject)
			return wID;
	}

	return 0;
}

void QCCommandStack::DeleteAllMenuCommands(CMenu* pMenu, COMMAND_ACTION_TYPE theAction, bool bDeleteCommandObjects)
{
	if (!pMenu)
	{
		ASSERT(0);
		return;
	}

	for (int i = pMenu->GetMenuItemCount() - 1; i >= 0; i--)
	{
		UINT uID = pMenu->GetMenuItemID(i);

		if ((int)uID == -1) 
			DeleteAllMenuCommands(pMenu->GetSubMenu(i), theAction, bDeleteCommandObjects);
		else
			DeleteCommand(WORD(uID), theAction, bDeleteCommandObjects);		
	}
}

void QCCommandStack::GetSavedCommand(WORD* pCommandID, QCCommandObject** ppObject, COMMAND_ACTION_TYPE* pAction)
{
	*pCommandID = m_theSelectedCommand.m_wCommandID;
	*ppObject = m_theSelectedCommand.m_pObject;
	*pAction = m_theSelectedCommand.m_theAction;
}
