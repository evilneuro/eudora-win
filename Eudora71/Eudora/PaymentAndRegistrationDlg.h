// PaymentAndRegistrationDlg.h : header file
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

#if !defined(AFX_PAYMENTANDREGISTRATIONDLG_H__158FC03B_76B2_11D3_B8E3_00805F9BF82F__INCLUDED_)
#define AFX_PAYMENTANDREGISTRATIONDLG_H__158FC03B_76B2_11D3_B8E3_00805F9BF82F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QCSharewareManager.h"


/////////////////////////////////////////////////////////////////////////////
// CPaymentAndRegWebStepInfoDlg dialog

class CPaymentAndRegWebStepInfoDlg : public CDialog
{
public:
							//	Constructor with message string IDs specified
							CPaymentAndRegWebStepInfoDlg(
								UINT			nDialogResID,
								UINT			nDialogTitleStringID,
								UINT			nMessageTitleStringID,
								UINT			nMessageStringID,
								CWnd *			pParent = NULL);

protected:
	CRString				m_DialogTitle;
	CRString				m_MessageTitle;
	CRString				m_Message;

	virtual BOOL			OnInitDialog();
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
};


/////////////////////////////////////////////////////////////////////////////
// CRepayDlg dialog

class CRepayDlg : public CDialog
{
public:
	CRepayDlg(CWnd *pParent = NULL);

protected:
	afx_msg void OnSponsoredModeButton();
	afx_msg void OnPayNowButton();
	afx_msg void OnShowVersionsButton();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

	CString					m_Message;
	virtual void			DoDataExchange(CDataExchange* pDX);
};

/////////////////////////////////////////////////////////////////////////////
// CJunkDownDlg dialog

class CJunkDownDlg : public CDialog
{
public:
	CJunkDownDlg(CWnd *pParent = NULL);

protected:
	afx_msg void OnTellMeMoreButton();
	afx_msg void OnPayNowButton();
	afx_msg void OnOK();
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CPaymentAndRegistrationDlg dialog

class CPaymentAndRegistrationDlg : public CDialog
{
// Construction
public:
	CPaymentAndRegistrationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPaymentAndRegistrationDlg)
	enum { IDD = IDD_PAYMENT_REGISTRATION };

	SECBitmapButton	m_RegisterButton;
	SECBitmapButton	m_DemographicsButton;
	SECBitmapButton	m_FindVersionsButton;
	SECBitmapButton	m_ChangeRegistrationButton;
	CString m_ProfileYourself;
	CString m_Register;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaymentAndRegistrationDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);	
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPaymentAndRegistrationDlg)
	afx_msg void OnAdwareButton();
	afx_msg void OnPaidButton();
	afx_msg void OnFreeButton();
	afx_msg void OnRegisterButton();
	afx_msg void OnDemographicsButton();
	afx_msg void OnFindVersionsButton();
	afx_msg void OnChangeRegistrationButton();
	afx_msg void OnTellMeMoreButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBitmap					m_AdwareBitmap;
	CBitmap					m_PaidBitmap;
	CBitmap					m_FreeBitmap;
	SharewareModeType		m_ApparentMode;
	bool					m_AlreadyRegistered;
	bool					m_AlreadyProfiled;

	void					InitBitmapRadioButton(
								int			nRadioButtonID,
								UINT		nBitmapIDResource,
								CBitmap &	bitmapMember);

	void ChangeMode(SharewareModeType NewMode);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAYMENTANDREGISTRATIONDLG_H__158FC03B_76B2_11D3_B8E3_00805F9BF82F__INCLUDED_)
