///////////////////////////////////////////////////////////////////////////////
// DSRECtrlIface.h
//
// Created: 09/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_RE_CTRL_IFACE_H_
#define __DS_RE_CTRL_IFACE_H_

#include "3dformv.h"
#include "DSGenericRichEditCtrlIface.h"

typedef void (*HTFRL)(BOOL, void *);

struct _htfrl {
    HTFRL  htfrl;
    void  *data;
};


class DSRECtrl : public C3DFormView {
    DECLARE_DYNCREATE(DSRECtrl)

public:
                   DSRECtrl();
    virtual       ~DSRECtrl();
    virtual BOOL   Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
			  DWORD dwRequestedStyle, const RECT& rect,
			  CWnd *pParentWnd, UINT nID,
			  CCreateContext *pContext = NULL)
    {
	return(C3DFormView::Create(lpszClassName, lpszWindowName,
				   dwRequestedStyle, rect, pParentWnd, nID,
				   pContext));
    }
    virtual BOOL   OnCmdMsg(UINT nID, int nCode, void *pExtra,
			    AFX_CMDHANDLERINFO *pHandlerInfo)
    {

	return(C3DFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo));
    }

    void           PrepareForDestruction();
    void           AddStringNoNewLine(char *);
    void           ClearContents();
    void           NewRecord();
    void           SetRichEditFocus() { m_RE_ResultsLong.SetFocus(); }
    void           InitializeHandleTab(HTFRL htfrl_, void *data);
    void           GetResultsLong(CString& details)
    {
	m_RE_ResultsLong.GetWindowText(details);
    }

    HWND           FetchControlWindow() { return(m_RE_ResultsLong.m_hWnd); }

private:

    DSGenericRichEditCtrl  m_RE_ResultsLong;
    bool            contentCleared;
    _htfrl          htfrl;
    HCURSOR         m_hIBeam;
    HCURSOR         m_hArrowHour;
    HINSTANCE       h_inst;
    int             nCurHeight;
    BOOL            SerializeSerializables();

protected:
            void   DoDataExchange(CDataExchange *pDX);
            void   OnInitialUpdate();

    afx_msg void    OnSize(UINT, int, int);
    afx_msg void    OnDestroy();
    afx_msg BOOL    OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#endif // __DS_RE_CTRL_IFACE_H_


