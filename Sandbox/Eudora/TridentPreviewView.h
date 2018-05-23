// TridentPreviewView.h: interface for the CTridentPreviewView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRIDENTPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_)
#define AFX_TRIDENTPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "TridentView.h"

class CTridentPreviewView : public CTridentView  
{
protected:
    DECLARE_DYNCREATE(CTridentPreviewView)

	//{{AFX_MSG(CTridentPreviewView)
	//}}AFX_MSG
	afx_msg LRESULT LoadNewPreview(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()

public:
    CTridentPreviewView();
    virtual ~CTridentPreviewView();

    void OnInitialUpdate();
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
			  AFX_CMDHANDLERINFO* pHandlerInfo);

    virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL GetMessageAsText(CString&	szMsg,BOOL bIncludeHeaders);
	virtual BOOL SetSelectedText( const char* szText, bool bSign );

    BOOL WriteTempFile( CFile& theFile,	CString& szStyleSheetFormat);
    virtual BOOL DoOnDisplayPlugin( IHTMLElement* pElement );
	virtual void OnActivateView( BOOL, CView*, CView* );

    void EnableWindowWithDelay( unsigned );

private:
    static void CALLBACK WMTimerProc( HWND, UINT, UINT, DWORD );
};

#endif // !defined(AFX_TRIDENTPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_)
