//	PopupText.h
//	
//	Tooltip-like window that can be used for displaying text, but
//	with more flexibility and easier use than standard tooltips.
//
//	Originally based on code by Paul DiLascia as published in
//	MSDN Magazine November 2003, but heavily modified for our
//	purposes.
//
//	Copyright (c) 2004 by QUALCOMM, Incorporated
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



#ifndef __PopupText_H__
#define __PopupText_H__


// Get NONCLIENTMETRICS info: ctor calls SystemParametersInfo.
class CNonClientMetrics : public NONCLIENTMETRICS
{
  public:
							CNonClientMetrics()
								{
									cbSize = sizeof(NONCLIENTMETRICS);
									SystemParametersInfo(SPI_GETNONCLIENTMETRICS,0,this,0);
								}
};


// Popup text window, like tooltip.
class CPopupText : public CWnd
{
  public:

	static bool				Initialize(
								CPoint			in_pt,
								CWnd *			in_pParentWnd,
								UINT			in_nID = 0 );
	static bool				Destroy();
	static bool				WasCreated() { return (s_pPopupText != NULL); }
	static CPopupText *		Instance()
								{
									ASSERT(s_pPopupText);
									return s_pPopupText;
								}

	BOOL					Create(
								CPoint			pt,
								CWnd *			pParentWnd,
								UINT			nID = 0);
	void					SetText(
								LPCTSTR			pszText);
	void					PositionWindow(
								CRect &			rectItem,
								CWnd *			pwndKeepInsideWindow,
								CPoint &		ptCursor);
	void					ShowDelayed(
								UINT			in_nMsec);
	void					Cancel();
	void					RelayEvent(
								MSG *			in_pMsg);

  protected:
							CPopupText();
	virtual					~CPopupText();

	virtual void			PostNcDestroy();
	virtual BOOL			PreCreateWindow(CREATESTRUCT& cs);

	void					DrawText(
								CDC &			dc,
								LPCTSTR			lpText,
								CRect &			rectText,
								UINT			flags,
								CRect *			prectWindow = NULL);

	afx_msg void			OnPaint();
	afx_msg void			OnTimer(UINT nIDEvent);
	afx_msg LRESULT			OnSetText(WPARAM wp, LPARAM lp);
	
	DECLARE_DYNAMIC(CPopupText);
	DECLARE_MESSAGE_MAP();

	//	Constants
	static const int		kDisplayTimer;
	static const int		kCheckLocationTimer;
	static const int		kMaxCharactersBeforeWrapping;

	//	Statics
	static CPopupText *		s_pPopupText;
	
	//	Member variables
	CSize					m_sizeTextMargins;				// Extra space around text
	CSize					m_sizeWindowMargins;			// Space between popup text and window or other items
	short					m_nWindowWidthLeftOfCursor;		// Amount of popup text width to the left of the cursor
	short					m_nWindowWidthRightOfCursor;	// Amount of popup text width to the right of the cursor
	CRect					m_rectItem;
	CRect					m_rectText;
	CFont					m_font;							// Font: change if you like
};


#endif	//	__PopupText_H__
