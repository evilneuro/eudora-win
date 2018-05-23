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
	afx_msg void OnTimer(UINT nIDEvent);

    DECLARE_MESSAGE_MAP()

public:
    CTridentPreviewView();
    virtual ~CTridentPreviewView();

	void					DeleteAssociatedFiles();

    void OnInitialUpdate();

    virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual BOOL GetMessageAsText(CString&	szMsg,BOOL bIncludeHeaders);
	virtual BOOL SetSelectedText( const char* szText, bool bSign );
	virtual BOOL SetAssociatedFiles(CStringList * in_pAssociatedFiles);

	virtual CTocDoc *		GetTocDoc();

	virtual int				GetNumMessageDocs() const;
	virtual CSummary *		GetSummary(int in_nMessageIndex);
	virtual CMessageDoc *	GetMessageDoc(int in_nMessageIndex = 0, bool * out_bLoadedDoc = NULL, CSummary ** out_ppSummary = NULL);
	virtual bool			IsNoMessageToDisplayAllowed() const;
	virtual BOOL			UseFixedFont() const;
	virtual bool			ShouldShowAllHeaders() const;
	virtual ContentConcentrator::ContextT	GetContentConcentratorContext(bool in_bIsBlahBlahBlah = false) const;

	virtual void			WriteHeaders(
									CFile &				theFile,
									CMessageDoc *		pDoc,
									const CString &		szHeaders,
									BOOL				bShowTabooHeaders);

	virtual void OnActivateView( BOOL, CView*, CView* );

	virtual BOOL			OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

    void					EnableWindowWithDelay();

private:
	static const UINT	kEnableWindowWithDelay;

	UINT				m_nEnableWindowWithDelayTimerID;
	CStringList *		m_pAssociatedFiles;
};

#endif // !defined(AFX_TRIDENTPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_)
