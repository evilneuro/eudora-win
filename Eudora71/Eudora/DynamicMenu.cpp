// DynamicMenu.cpp : implementation file
//
// Abstract base class for menus that build themselves on the fly
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

#include "DynamicMenu.h"
#include "utils.h"


#include "DebugNewHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// CDynamicMenu

IMPLEMENT_DYNAMIC(CDynamicMenu, CMenu)

CDynamicMenu::CDynamicMenu()
{
	m_bHasBeenBuilt = FALSE;
}

CDynamicMenu::~CDynamicMenu()
{
}

/////////////////////////////////////////////////////////////////////////////
// DeleteMenuObjects [public, static]
//
// Deletes any CDynamicMenus that are in the passed in menu.
// If bRemoveItems is TRUE, then delete the menu items as well.
/////////////////////////////////////////////////////////////////////////////
/*static*/ void CDynamicMenu::DeleteMenuObjects(CMenu* pParentMenu, BOOL bRemoveItems)
{
	if (!pParentMenu)
		return;

	for (int i = pParentMenu->GetMenuItemCount() - 1; i >= 0; i--)
	{
		CMenu* pSubMenu = pParentMenu->GetSubMenu(i);
		if (pSubMenu)
		{
			DeleteMenuObjects(pSubMenu, bRemoveItems);
			CDynamicMenu* pDynMenu = DYNAMIC_DOWNCAST(CDynamicMenu, pSubMenu);
			if (pDynMenu)
				delete pDynMenu;
		}
		if (bRemoveItems)
			pParentMenu->RemoveMenu(i, MF_BYPOSITION);
	}
}

/////////////////////////////////////////////////////////////////////////////
// OnInitMenuPopup [public, virtual]
//
// Deletes any CDynamicMenus that are in the passed in menu.
// If bRemoveItems is TRUE, then delete the menu items as well.
/////////////////////////////////////////////////////////////////////////////
/*virtual*/ CDynamicMenu::OnInitMenuPopup(UINT nIndex, BOOL bSysMenu)
{
	// If it's the first time displaying this menu we need to build the menu items.
	// Menus are either empty, or the have only a separator in them (which act as
	// a placeholder in menu resources to make sure a popup menu is there).
	int NumMenuItems = GetMenuItemCount();
	if (NumMenuItems == 0 || (NumMenuItems == 1 && GetMenuItemID(0) == 0))
	{
		if (NumMenuItems == 1)
			RemoveMenu(0, MF_BYPOSITION);
		if (BuildMenu(nIndex, bSysMenu))
		{
			// If there is nothing to put on the menu, some types will put in a placeholder
			// menu item saying something to the effect of "No items".  If that is the case
			// then consider the menu not built yet so that it will get built if something
			// gets added to the menu at a later time.
			NumMenuItems = GetMenuItemCount();
			if (NumMenuItems > 1 || (NumMenuItems == 1 && GetMenuItemID(0) != 0))
			{
				m_bHasBeenBuilt = TRUE;
				WrapMenu(GetSafeHmenu());
			}
			else
				m_bHasBeenBuilt = FALSE;
		}
	}
}
