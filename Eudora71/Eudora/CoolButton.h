//	CoolButton.h
//
//	Button that supports either split button functionality (i.e. part button
//	part drop down menu) or straight drop down menu functionality. Supports
//	optional icon-sized bitmap. Supports XP themes.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#ifndef __CCoolButton_H__
#define __CCoolButton_H__


#pragma once

#include "XPThemedAppearance.h"


class CCoolButton : public CButton
{
  public:
	//	Constants
	static const long						kMenuButtonPadding = 4;
	static const long						kIconTextPadding = 6;

	//	Constructor and destructor
											CCoolButton();
	virtual									~CCoolButton();

	//	Accessors
	CMenu &									GetMenu() { return m_menu; }

	//	Menu item manipulation
	BOOL									AddMenuItem(
												UINT						in_nMenuID,
												const char *				in_szMenuItem,
												UINT						in_nMenuFlags);
	BOOL									RemoveMenuItem(
												UINT						in_nMenuID,
												UINT						in_nMenuFlags);
	void									ResetMenu();
	BOOL									EnableMenuItem(
												UINT						in_nMenuID,
												BOOL						in_bEnable);
	UINT									CheckMenuItem(
												UINT						in_nMenuID,
												UINT						in_nMenuFlags);

	//	Setting button image either as bitmap or as an icon
	void									SetButtonBitmap(
												UINT						in_nResourceID,
												COLORREF					in_crMask);
	void									SetButtonIcon(
												UINT						in_nResourceID);


	//	Behavior control
	void									SetIsSplit(bool in_bIsSplit) { m_bIsSplit = in_bIsSplit; }
	void									SetIsDefaultButton(bool in_bIsDefaultButton) { m_bIsDefaultButton = in_bIsDefaultButton; }
	void									SetLeftAlignMenu(bool in_bLeftAlignMenu) { m_bLeftAlignMenu = in_bLeftAlignMenu; }

	//	Overrides
	virtual void							PreSubclassWindow();
	virtual void							DrawItem(
												LPDRAWITEMSTRUCT			lpDrawItemStruct);

  protected:
	void									DrawArrow(
												CDC *						in_pDC,
												CPoint						in_ptArrowCenter);
	BOOL									PtInMenuRect(
												CPoint						in_point);
	void									OnMenuButton();
	void									CommandRouteMenu(
												CWnd *						in_pWnd,
												CMenu *						in_pPopupMenu);

	afx_msg UINT							OnGetDlgCode();
	afx_msg LRESULT							OnSetStyle(
												WPARAM						wParam,
												LPARAM						lParam);
	afx_msg void							OnSetFocus(
												CWnd *						pOldWnd);
	afx_msg void							OnKillFocus(
												CWnd *						pNewWnd);
	afx_msg void							OnSysColorChange();
	afx_msg void							OnSysKeyDown(
												UINT						nChar,
												UINT						nRepCnt,
												UINT						nFlags);
	afx_msg void							OnMouseMove(
												UINT						nFlags,
												CPoint						point);
	afx_msg LRESULT							OnMouseHover(
												WPARAM						wparam,
												LPARAM						lparam);
	afx_msg LRESULT							OnMouseLeave(
												WPARAM						wparam,
												LPARAM						lparam);
	afx_msg void							OnLButtonDown(
												UINT						nFlags,
												CPoint						point);
	afx_msg void							OnLButtonUp(
												UINT						nFlags,
												CPoint						point);
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CCoolButton)

	CMenu									m_menu;
	CBitmap									m_btnImage;
	HICON									m_hiconButton;
	HICON									m_hiconArrow;
	CImageList								m_ilButton;
	CSize									m_sizeIcon;
	CSize									m_sizeArrow;
	long									m_nMenuButtonWidth;
	bool									m_bIsButtonPushed;
	bool									m_bIsMenuPushed;
	bool									m_bMenuInitialized;
	bool									m_bIsDefaultButton;
	bool									m_bActAsDefaultBtn;
	bool									m_bIsSplit;
	bool									m_bLeftAlignMenu;
	CXPThemedAppearance						m_xpThemedApppearance;
	bool									m_bIsMouseOverButton;
	BOOL									m_bTracking;	// are we tracking the mouse
};


#endif		//	__CCoolButton_H__
