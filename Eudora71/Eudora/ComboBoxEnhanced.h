//	ComboBoxEnhanced.h
//
//	Provides enhanced CComboBoxEx.
//
//	Copyright (c) 2003 by QUALCOMM, Incorporated
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



#ifndef __ComboBoxEnhanced_H__
#define __ComboBoxEnhanced_H__

//	Forward declarations
class CComboBoxEnhanced;


class CListBoxEncapsulated : public CWnd
{
  public:
								CListBoxEncapsulated(CComboBoxEnhanced * in_pParent);
								~CListBoxEncapsulated();

  protected:
	CComboBoxEnhanced *			m_pParent;

	bool						PointIsInDisabledItem(CPoint in_point);

	//	Overrides
	afx_msg void				OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void				OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void				OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};


class CComboBoxEnhanced : public CComboBoxEx
{
  public:
	enum
	{
		kDisabled = 0x0001,
		kDividerLine = 0x0002
	};

	//	Constants
	static const int			kDefaultPadding;
	static const int			kDroppedPadding;
	static const int			kIndentPadding;


								CComboBoxEnhanced();
	virtual						~CComboBoxEnhanced();

	//	Accessors
	void						ResetMaxWidth() { m_nMaxWidth = 0; }

	//	Operations
	void						ResetContent();
	void						InsertItem(
										UINT				in_nStringResID,
										int					in_nItemID = 0,
										bool				in_bIsDisabled = false,
										bool				in_bIsDividerLine = false);
	void						InsertItem(
										const char *		in_szString,
										int					in_nItemID = 0,
										int					in_nStringLength = -1,
										bool				in_bIsDisabled = false,
										bool				in_bIsDividerLine = false);

	//	Item state
	bool						IsItemEnabled(
										COMBOBOXEXITEM &	itemInfo);
	bool						IsItemEnabled(
										int					in_nItemIndex);

  protected:
	CListBoxEncapsulated		m_ListBoxEncapsulated;
	int							m_nLastSelectedItem;
	int							m_nMaxWidth;

	//	Overrides
	afx_msg void				OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL				OnComboBoxItemSelected();
	LRESULT						OnListBoxCtlColor(WPARAM,LPARAM lParam);

	DECLARE_DYNCREATE(CComboBoxEnhanced)
	DECLARE_MESSAGE_MAP()
};


#endif // __ComboBoxEnhanced_H__
