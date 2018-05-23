// AdWazooBar.cpp : implementation file
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

#include "stdafx.h"

#include "mainfrm.h"

#include "AdWazooBar.h"
#include "AdWazooWnd.h"
#include "QCSharewareManager.h"


#include "DebugNewHelpers.h"

IMPLEMENT_DYNCREATE(CAdWazooBar, CWazooBar)

BEGIN_MESSAGE_MAP(CAdWazooBar, CWazooBar)
	//{{AFX_MSG_MAP(CAdWazooBar)
	ON_WM_CREATE()

	ON_COMMAND(ID_SEC_MDIFLOAT, OnDisabled)
	ON_COMMAND(ID_WAZOO_TAB_SHOW, OnDisabled)
	ON_COMMAND(ID_SEC_HIDE, OnDisabled)

	ON_UPDATE_COMMAND_UI(ID_SEC_MDIFLOAT, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI(ID_WAZOO_TAB_SHOW, OnUpdateDisable)
	ON_UPDATE_COMMAND_UI(ID_SEC_HIDE, OnUpdateDisable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAdWazooBar
CAdWazooBar::CAdWazooBar()
{
	m_ImageWidth = 144;
	m_ImageHeight = 144;

	m_rcBorderSpace.SetRect(5, 5, 5, 5);
}

CAdWazooBar::CAdWazooBar(CWazooBarMgr* pManager) : CWazooBar(pManager)
{
	m_ImageWidth = 144;
	m_ImageHeight = 144;

	m_rcBorderSpace.SetRect(5, 5, 5, 5);
}


CAdWazooBar::~CAdWazooBar()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM)
		pSWM->UnRegister(this);
}


////////////////////////////////////////////////////////////////////////
// Create [public]
//
// We need to specify some particular parameters to the Create() call
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::Create(CWnd* pParentWnd)
{
	if (CWazooBar::Create(pParentWnd, "AdWazooBar",
								CBRS_LEFT | CBRS_SIZE_FIXED | WS_VISIBLE,
								CBRS_EX_STDCONTEXTMENU | CBRS_EX_COOLBORDERS | CBRS_EX_GRIPPER,
								IDC_AD_WAZOO_BAR))
	{
		ASSERT(::IsWindow(m_wndTab.GetSafeHwnd()));

		CreateWazooWindow(RUNTIME_CLASS(CAdWazooWnd));

		//
		// Special case for creation of child Wazoos inside of a
		// floating window...  need to notify the child wazoos to
		// perform their OnInitialUpdate() sequence.
		//
		// We can't tell if the window is going to be floating or not,
		// so we just send the message anyway, and guard against the
		// potential double initialization in the view.
		//
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		// Hide tabs and tell tab window not to draw 3D borders
		m_wndTab.ShowTabs(FALSE);
		m_wndTab.SetDraw3DBorder(FALSE);

		QCSharewareManager* pSWM = GetSharewareManager();
		if (pSWM)
			pSWM->Register(this);

		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// SetDefaultWazooConfig [public, virtual]
//
// Set up the Ad wazoo bar in the default state
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::SetDefaultWazooConfig(int /*nBar*/)
{
	UpdateState();

	return TRUE;
}

int CAdWazooBar::GetMinWidth()
{
	// Don't ask me why the calculation is off by 11 pixels because I don't know
	return m_rcBorderSpace.left + m_rcBorderSpace.right + ImageWidth() + 11;
}

int CAdWazooBar::GetMinHeight()
{
	// Don't ask me why the calculation is off by 2 pixels because I don't know
	int MinHeight = m_rcBorderSpace.top + m_rcBorderSpace.bottom + ImageHeight() + 2;

	if (!IsFloating())
		MinHeight += m_GripperInfo.GetHeight();

	return MinHeight;
}

////////////////////////////////////////////////////////////////////////
// NewAdSize [public]
//
// Change minimum sizes for handling a new ad.  Redisplay may be needed.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::NewAdSize(int Width, int Height)
{
	// Sanity checks
	if (Width < 0 || Width > 1000 || Height < 0 || Height > 1000)
	{
		ASSERT(0);
		return;
	}

	if (Width != m_ImageWidth || Height != m_ImageHeight)
	{
		BOOL bNeedResize = FALSE;

		// If the ad size is at least 200 pixels wide, and is more than 25% wider
		// than it is tall, then dock a left/right docked ad window to the bottom
		if (Width >= 200 && Width > Height * 1.25 && !IsFloating() && (GetBarStyle() & CBRS_ORIENT_VERT))
		{
			CMainFrame *pMainFrame = (CMainFrame*)AfxGetMainWnd();
			pMainFrame->DockControlBarEx(this, AFX_IDW_DOCKBAR_BOTTOM, 5, 0, (float)0.25, Height);
			bNeedResize = TRUE;
		}
		else
		{
			// Only resize if Ad size grew
			if (Width > m_ImageWidth || Height > m_ImageHeight)
				bNeedResize = TRUE;
		}

		m_ImageWidth = Width;
		m_ImageHeight = Height;

		if (bNeedResize)
		{
			CMainFrame *pMainFrame = (CMainFrame*)AfxGetMainWnd();
			pMainFrame->DelayRecalcLayout();
		}
	}
}

////////////////////////////////////////////////////////////////////////
// LoadWazooConfigFromIni [public, virtual]
//
// We don't really load up the Ad wazoo bar from the INI file, but
// instead just create the Ad window and some default parameters.
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::LoadWazooConfigFromIni()
{
	SECDockBar* pDockBar = DYNAMIC_DOWNCAST(SECDockBar, m_pDockBar);

	if (!pDockBar)
	{
		// The Ad wazoo bar has never been docked (probably the first time we've run an adware version),
		// so dock it once, and continue
		CMainFrame *pMainFrame = (CMainFrame*)AfxGetMainWnd();
		pMainFrame->DockControlBarEx(this, AFX_IDW_DOCKBAR_LEFT, 5, 0, (float)0.25, 180);
		ASSERT(m_pDockBar);
	}

	SetDefaultWazooConfig(0);

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SaveWazooConfigToIni [public, virtual]
//
// Do nothing since we don't save the Ad wazoo container to the
// [WazooBars] section of the INI file.
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::SaveWazooConfigToIni()
{
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CalcFixedLayout [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
CSize CAdWazooBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize theSize = SECControlBar::CalcFixedLayout(bStretch, bHorz);

	// Ad window has minimum horizontal and vertical sizes
	const int MinWidth = GetMinWidth();
	const int MinHeight = GetMinHeight();

	if (IsFloating())
	{
		// When the Ad window is floating, we can always ensure the smallest size
		theSize.cx = MinWidth;
		theSize.cy = MinHeight;
	}
	else
	{
		// When the Ad window is not floating, we can only ensure a minimum size
		theSize.cx = __max(theSize.cx, MinWidth);
		theSize.cy = __max(theSize.cy, MinHeight);
	}

	return theSize;
}

////////////////////////////////////////////////////////////////////////
// UpdateState [protected]
//
// Show/Hide Ad wazoo bar based on current state of program.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::UpdateState()
{
	if (GetSharewareMode() == SWM_MODE_ADWARE)
		m_pManager->ActivateWazooWindow((CMainFrame*)AfxGetMainWnd(), RUNTIME_CLASS(CAdWazooWnd));
	else
		OnHide();
}

////////////////////////////////////////////////////////////////////////
// Notify [public, virtual]
//
// Notify us when program changes state between Light, Pro, and Adware.
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::Notify(QCCommandObject* /*pObject*/, COMMAND_ACTION_TYPE theAction, void* pData /*= NULL*/)
{
	if (theAction == CA_SWM_CHANGE_FEATURE)
	{
		const BOOL bCurrentModeIsAdware = GetSharewareMode() == SWM_MODE_ADWARE;
		const BOOL bOldModeWasAdware = pData? (*(SharewareModeType*)pData == SWM_MODE_ADWARE) : !bCurrentModeIsAdware;

		if (bOldModeWasAdware != bCurrentModeIsAdware)
			UpdateState();
	}
}


////////////////////////////////////////////////////////////////////////
// OnExtendContextMenu [protected, virtual]
//
// Override base class implementation to disable CWazooBar functionality
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnExtendContextMenu(CMenu* pMenu)
{
	// Don't allow CWazooBar to extend context menu, and hide the "Hide" menu item
	if (pMenu)
		pMenu->RemoveMenu(ID_SEC_HIDE, MF_BYCOMMAND);
}

////////////////////////////////////////////////////////////////////////
// RemoveSystemMenu [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CAdWazooBar::RemoveSystemMenu()
{
	CFrameWnd* pFrame = GetParentFrame();

	if (!pFrame || pFrame == AfxGetMainWnd())
		return FALSE;

	// Remove the system menu so that we don't get a close box
	pFrame->ModifyStyle(WS_SYSMENU, 0, SWP_FRAMECHANGED);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// ActivateWazooWindow [public, virtual]
//
// Use this to get rid of various system menu options on floating windows
////////////////////////////////////////////////////////////////////////
/*virtual*/ BOOL CAdWazooBar::ActivateWazooWindow(CRuntimeClass* pRuntimeClass)
{
	RemoveSystemMenu();

	return CWazooBar::ActivateWazooWindow(pRuntimeClass);
}

////////////////////////////////////////////////////////////////////////
// OnBarStyleChange [public, virtual]
//
// Make sure the CBRS_SIZE_DYNAMIC style doesn't get set and that
// CBRS_SIZE_FIXED does get set
////////////////////////////////////////////////////////////////////////
/*virtual*/ void CAdWazooBar::OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle)
{
	if ((dwNewStyle & CBRS_SIZE_DYNAMIC) || !(dwNewStyle & CBRS_SIZE_FIXED))
		SetBarStyle((dwNewStyle & ~CBRS_SIZE_DYNAMIC) | CBRS_SIZE_FIXED);
}


/////////////////////////////////////////////////////////////////////////////
// CAdWazooBar message handlers

////////////////////////////////////////////////////////////////////////
// OnCreate [protected]
//
// Override of the base CWazooBar class to prevent a drag-and-drop
// object from being registered.
////////////////////////////////////////////////////////////////////////
int CAdWazooBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Skip base CWazooBar class
	if (SECControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_wndTab.Create(this);

	return 0;
}


////////////////////////////////////////////////////////////////////////
// OnDisabled [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnDisabled()
{
	// Disabled function for Ad window
	ASSERT(0);
}


////////////////////////////////////////////////////////////////////////
// OnUpdateDisable [protected]
//
////////////////////////////////////////////////////////////////////////
void CAdWazooBar::OnUpdateDisable(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}
