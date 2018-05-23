// 
// Stingray Software Extension Classes
// Copyright (C) 1996 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the  help files for detailed information
// regarding using these classes.
// 
//  Author:		Jan


#ifdef WIN32

#ifndef __SLSTCTIP_H__
#define __SLSTCTIP_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

class SECListCtrl;

class SECListCtrlTip : public CWnd
{
// Construction
public:
	SECListCtrlTip( SECListCtrl *);

// Attributes
private:
	int m_nHitItem;
	int m_nHitSubItem;
	SECListCtrl *m_pListCtrl;
	CString       m_strText;
	CRect         m_rectItem;

	void SetWindowText( const CString &str);
public:

// Operations
public:
	BOOL HookMouseMove(UINT iHitFlags, int nHitItem, int nHitCol, CPoint pt);
	void Hide();
	void Refresh(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECListCtrlTip)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SECListCtrlTip();

	// Generated message map functions
protected:
	//{{AFX_MSG(SECListCtrlTip)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SLSTCTIP_H__

#endif // WIN32
