// DynamicStationeryMenu.h : header file
//
// Class for stationery menus that build themselves on the fly
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

#if !defined(AFX_DYNAMICSTATIONERYMENU_H__6F8BE5F5_FF82_4EB4_A485_E93774FDE388__INCLUDED_)
#define AFX_DYNAMICSTATIONERYMENU_H__6F8BE5F5_FF82_4EB4_A485_E93774FDE388__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DynamicMenu.h"
#include "QCCommandActions.h"

/////////////////////////////////////////////////////////////////////////////
// CDynamicStationeryMenu

class CDynamicStationeryMenu : public CDynamicMenu
{
	DECLARE_DYNAMIC(CDynamicStationeryMenu)

// Construction
public:
	CDynamicStationeryMenu(COMMAND_ACTION_TYPE CommandAction);

// Attributes

// Operations

// Overrides
protected:
	virtual BOOL BuildMenu(UINT nIndex, BOOL bSysMenu);

// Implementation
public:
	virtual ~CDynamicStationeryMenu();

protected:
	COMMAND_ACTION_TYPE m_CommandAction;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DYNAMICSTATIONERYMENU_H__6F8BE5F5_FF82_4EB4_A485_E93774FDE388__INCLUDED_)
