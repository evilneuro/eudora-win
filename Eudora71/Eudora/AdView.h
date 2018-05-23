// AdView.h : header file
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

#ifndef ADVIEW_H
#define ADVIEW_H

#include "PaigeEdtView.h"

struct ENTRY_INFO;

/////////////////////////////////////////////////////////////////////////////
// CAdView view

class CAdView : public CPaigeEdtView
{
protected:
	DECLARE_DYNCREATE(CAdView)

// Attributes
public:
	static void PlaylistCallback( int cmd, long userRef, long lParam );
	static BOOL OneAdShown() { return (s_pOneAndOnlyView? s_pOneAndOnlyView->m_bOneAdShown : FALSE); }

// Operations
public:
	//
	// Normally, the constructor and destructor are protected since
	// the MFC document template stuff takes care of object creation
	// and destruction.  However, since this is now owned by a Wazoo
	// window, these need to be public.
	//
	CAdView();           
	virtual ~CAdView();

private:
	void loadEntry( ENTRY_INFO* pe );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual bool NewPaigeObject(long AddFlags = 0, long AddFlags2 = 0);
	virtual BOOL LaunchURL(LPCSTR szURL);

	void	InitBlankBitmap();
	CBitmap	m_BlankTimeBitmap;
	BOOL	m_bIsBlankTime;
	BOOL	m_bOneAdShown;
	CString	m_CurrentAdID;

	static CAdView* s_pOneAndOnlyView;
	BOOL	m_bWasInitialized;

	// Generated message map functions
protected:
	//{{AFX_MSG(CAdView)
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnSysColorChange();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif //ADVIEW_H
