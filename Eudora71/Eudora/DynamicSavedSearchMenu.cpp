// DynamicRecipientMenu.cpp : implementation file
//
// Class for saved search menus that build themselves on the fly
//
// Copyright (c) 2006 by QUALCOMM, Incorporated
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



#include "stdafx.h"

#include "DynamicSavedSearchMenu.h"
#include "SearchManager.h"

#include "DebugNewHelpers.h"

IMPLEMENT_DYNAMIC(CDynamicSavedSearchMenu, CDynamicMenu)


CDynamicSavedSearchMenu::CDynamicSavedSearchMenu(COMMAND_ACTION_TYPE CommandAction)
{
	m_CommandAction = CommandAction;
}


CDynamicSavedSearchMenu::~CDynamicSavedSearchMenu()
{
}


BOOL
CDynamicSavedSearchMenu::BuildMenu(UINT nIndex, BOOL bSysMenu)
{
	RebuildMenu(true);

	return TRUE;
}


void
CDynamicSavedSearchMenu::RebuildMenu(bool bDoInitialBuild)
{
	//	If we haven't been built yet and we're not being told to do the
	//	initial build, then just bail.
	if (!HasBeenBuilt() && !bDoInitialBuild)
		return;

	//	Remove all previous items
	while ( (GetMenuItemCount() > 0) && RemoveMenu(0, MF_BYPOSITION) );
	
	//	Get the list of saved searches
	SearchManager::SavedSearchList &	listSavedSearches = SearchManager::Instance()->GetSavedSearchesList();

	//	Add saved search commands to the menu
	for ( SearchManager::SavedSearchList::iterator i = listSavedSearches.begin();
		  i != listSavedSearches.end();
		  i++ )
	{
		SearchManager::SavedSearchCommand *		pSavedSearchCommand = *i;

		if (pSavedSearchCommand)
		{
			AppendMenu( MF_STRING, pSavedSearchCommand->GetCommandID(),
						pSavedSearchCommand->GetFileTitle() );
		}
	}
}
