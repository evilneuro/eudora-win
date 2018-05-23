// DynamicMailboxMenu.cpp : implementation file
//
// Class for mailbox menus that build themselves on the fly
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include "DynamicMailboxMenu.h"
#include "QCMailboxDirector.h"

#include "QCMailboxCommand.h"
#include "QCCommandStack.h"

#include "fileutil.h"
#include "resource.h"
#include "rs.h"

#include "DebugNewHelpers.h"


extern QCMailboxDirector g_theMailboxDirector;
extern QCCommandStack g_theCommandStack;


CRecentMailboxMenu* g_RecentMailboxesMenu;
CRecentMailboxMenu* g_RecentTransferMenu;

/////////////////////////////////////////////////////////////////////////////
// CDynamicMailboxMenu

IMPLEMENT_DYNAMIC(CDynamicMailboxMenu, CDynamicMenu)

CDynamicMailboxMenu::CDynamicMailboxMenu(QCMailboxCommand* pMailboxCommand, BOOL bIsTransfer) :
	m_pMailboxCommand(pMailboxCommand),
	m_bIsTransfer(bIsTransfer)
{
}

CDynamicMailboxMenu::~CDynamicMailboxMenu()
{
}

/*virtual*/ BOOL CDynamicMailboxMenu::BuildMenu(UINT nIndex, BOOL bSysMenu)
{
	g_theMailboxDirector.NewMessageCommandsLevel(m_pMailboxCommand, m_bIsTransfer, this);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CRecentMailboxMenu

IMPLEMENT_DYNAMIC(CRecentMailboxMenu, CDynamicMailboxMenu)

CRecentMailboxMenu::CRecentMailboxMenu(BOOL bIsTransfer) : CDynamicMailboxMenu(NULL, bIsTransfer)
{
	if (bIsTransfer)
		g_RecentTransferMenu = this;
	else
		g_RecentMailboxesMenu = this;
}

CRecentMailboxMenu::~CRecentMailboxMenu()
{
}

/*virtual*/ BOOL CRecentMailboxMenu::BuildMenu(UINT nIndex, BOOL bSysMenu)
{
	for (std::list<LPCTSTR>::iterator it = QCMailboxDirector::s_RecentMailboxList.begin(); it != QCMailboxDirector::s_RecentMailboxList.end(); ++it)
	{
		QCMailboxCommand* pCommand = g_theMailboxDirector.FindByPathname(EudoraDir + *it);

		if (pCommand)
		{
			WORD wID = g_theCommandStack.FindOrAddCommandID(pCommand, m_bIsTransfer? CA_TRANSFER_TO : CA_OPEN);
			if (wID)
				AppendMenu(MF_STRING | MF_DISABLED | MF_GRAYED, wID, pCommand->GetName());
		}
	}

	return TRUE;
}
