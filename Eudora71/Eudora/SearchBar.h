//	SearchBar.h
//
//	Provides mini-dialog toolbar searching.
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



#ifndef __SearchBar_H__
#define __SearchBar_H__

//	Includes
#include <list>

#ifndef __SBARCORE_H__
	#include "sbarcore.h"
#endif

#include "ComboBoxEnhanced.h"


//	Forward declarations
class CSearchBar;


class CSearchBarComboBoxEdit : public CEdit
{
  public:
								CSearchBarComboBoxEdit(
										CSearchBar *		in_pSearchBar);
	virtual						~CSearchBarComboBoxEdit();

	void						SetActualText(
										const char *		in_szText,
										bool				in_bUpdateEmptyStatus = true);
	void						GetActualText(
										CString &			out_szText);
	void						UpdateGrayTextStatus();

	//	Accessors
	bool						IsEmpty() const { return m_bIsEmpty; }

	//	Overrides
	virtual BOOL				PreTranslateMessage(
										MSG *				pMsg);

  protected:
	//	Message map overrides
	afx_msg void				OnSetFocus(
										CWnd *				pOldWnd );
	afx_msg	void				OnKillFocus(
										CWnd *				pNewWnd);
	afx_msg LRESULT				OnSetText(
										WPARAM				wParam,
										LPARAM				lParam);
	afx_msg LRESULT				OnGetText(
										WPARAM				wParam,
										LPARAM				lParam);
	afx_msg LRESULT				OnGetTextLength(
										WPARAM				wParam,
										LPARAM				lParam);
	afx_msg LRESULT				OnKeyDown(
										WPARAM				wParam,
										LPARAM				lParam);

	CSearchBar *				m_pSearchBar;		//	Pointer to our parent SearchBar
	bool						m_bAllowSetText;	//	Whether or not we should allow setting our text
	bool						m_bGetActualText;	//	Whether or not we should tell the truth when asked for our text
	bool						m_bIsEmpty;			//	Whether or not we're "officially" empty - used for tracking when gray text is appropriate

	DECLARE_MESSAGE_MAP()
};


class CSearchBarComboBox : public CComboBoxEnhanced
{
  public:
								CSearchBarComboBox(
										CSearchBarComboBoxEdit *	in_pSearchBarComboBoxEdit);
	virtual						~CSearchBarComboBox();

  protected:
	//	Message map overrides
	afx_msg HBRUSH				OnCtlColor(
										CDC *				pDC,
										CWnd *				pWnd,
										UINT				nCtlColor);

	CSearchBarComboBoxEdit *	m_pSearchBarComboBoxEdit;

	DECLARE_MESSAGE_MAP()
};


class CSearchBar : public SECCustomToolBar
{
	DECLARE_DYNCREATE(CSearchBar)

  public:
	static const short			kSmallToolbarButtonHeight;
	static const short			kLargeToolbarButtonHeight;
	static const short			kSearchBarMinWidth;
	static const short			kSearchBarMaxWidth;
	static const short			kSearchComboHeight;
	static const short			kSearchComboPaddingFromGripper;
	static const short			kSearchComboPaddingFromEdgeDocked;
	static const short			kSearchComboPaddingFromEdgeFloating;

	enum
	{
		kSearchWeb = 0,
		kSearchEudora,
		kSearchMailbox,
		kSearchMailfolder,
		kSearchInvalid = SHRT_MAX
	};

	typedef enum
	{
		kDockedHorizontally = 0,
		kDockedVertically,
		kFloating
	} OrientationType;

								CSearchBar();
	virtual						~CSearchBar();

	virtual BOOL				Create(
										CWnd *					in_pParentWnd,
										DWORD					in_dwAlignStyle);

	short						GetMaxNumberRecentItems();

	void						ReadRecentList();
	void						WriteRecentList();
	
	void						UpdateSearchComboItems();

	//	Accessors
	UINT						GetCurrentSearchMenuStringID();

	//	Operations
	void						RefreshSettings();
	const char *				GetActiveMailboxName();
	void						SearchForCurrentText(
										bool					in_bBeepIfEmpty = true);

	//	Overrides
	virtual void				GetBarInfoEx(
										SECControlBarInfo *		pInfo);
	virtual void				SetBarInfoEx(
										SECControlBarInfo *		pInfo,
										CFrameWnd *				pFrameWnd);
	virtual void				OnBarStyleChange(
										DWORD					dwOldStyle,
										DWORD					dwNewStyle);
	virtual void				OnUpdateCmdUI(
										CFrameWnd *				pTarget,
										BOOL					bDisableIfNoHndler);
	virtual int					OnToolHitTest(
										CPoint					point,
										TOOLINFO *				pTI) const;
	virtual CSize				CalcFixedLayout(
										BOOL					bStretch,
										BOOL					bHorz);
	virtual CSize				CalcDynamicLayout(
										int						nLength,
										DWORD					dwMode);

  protected:
	class RecentSearch
	{
	  public:
											RecentSearch();
											RecentSearch(
													short				in_nSearchType,
													const char *		in_szSeachText);
											~RecentSearch();

		short								GetSearchType() const { return m_nSearchType; }
		const char *						GetSearchText() const { return m_szSearchText; }

	  protected:
		short								m_nSearchType;
		CString								m_szSearchText;
	};

	typedef std::list<RecentSearch>			RecentListT;
	typedef RecentListT::iterator			RecentListIteratorT;

	virtual void				DoPaint(
										CDC *					pDC);

	void						CalculateComboBoxRect(
										OrientationType			in_eOrientationType,
										int						in_nComboHeight,
										CRect &					out_rectCombo);

	virtual void				ResizeComboBox(
										OrientationType			in_eOrientationType);

	//	Overrides
								// Balance the wrapping of the toolbar.
	virtual void				BalanceWrap(
										int						nRow,
										Wrapped *				pWrap);

								// Returns TRUE if the default drag mode is add.
	virtual BOOL				GetDragMode() const;

								// Returns TRUE if this toolbar accepts dropped buttons.
	virtual BOOL				AcceptDrop() const;

	//	Message map overrides
	afx_msg void				OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL				OnEraseBkgnd(CDC* pDC);
	afx_msg void				UpdateSearchBarCombo(CCmdUI* pCmdUI);
	afx_msg void				OnComboBoxItemSelected();
	afx_msg void				OnComboBoxDropDown();

	short						m_nCurrentSearch;
	RecentListT					m_recentSearchList;
	short						m_nSearchBarWidth;
	int							m_nEditTextMinHeight;
	int							m_nToolbarHeight;
	BOOL						m_bWasFloating;
	DWORD						m_dwExOldStyle;
	CSearchBarComboBox			m_SearchCombo;
	CSearchBarComboBoxEdit		m_ComboEdit;

	DECLARE_MESSAGE_MAP()
};


#endif // __SearchBar_H__
