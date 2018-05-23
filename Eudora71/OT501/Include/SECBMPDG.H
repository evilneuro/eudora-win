// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detaild information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	SECBitmapDialog definition
//  Created:		Sun Jun 25 22:54:12 1995
// CHANGELOG: 
//
//
//
//
//

#ifndef __SECBMPDG_H__
#define __SECBMPDG_H__

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

/////////////////////////////////////////////////////////////////////////////
// SECBitmapDialog dialog

#define SEC_BITMAP_TILE 	0
#define SEC_BITMAP_CENTER	1
#define SEC_BITMAP_FILL     2

class SECBitmapDialog : public CDialog
{
// Construction
public:
    SECBitmapDialog (UINT nDialogD, CWnd* pParent = NULL);
    SECBitmapDialog (LPCTSTR lpcDialogName, CWnd* pParent = NULL);
    
    BOOL SetBitmap (UINT nResBitmapID, int nType = SEC_BITMAP_TILE);
    BOOL SetBitmap (LPCTSTR lpcResBitmapName, int nType = SEC_BITMAP_TILE);
    
// Attributes
private:
    CBitmap m_currentBitmap;
    CBrush  m_hollowBrush;
    int     m_nType; //Tiled, centered or filled?
    
// Implementation
protected:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
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


#endif // __SECBMPDG_H__
