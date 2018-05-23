// BidentPreviewView.h: interface for the CBidentPreviewView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BIDENTPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_)
#define AFX_BIDENTPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "BidentView.h"

class CBidentPreviewView : public CBidentView  
{
protected:
	DECLARE_DYNCREATE(CBidentPreviewView)

	//{{AFX_MSG(CBidentPreviewView)
	//}}AFX_MSG
	afx_msg LRESULT LoadNewPreview(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

public:
	CBidentPreviewView();
	virtual ~CBidentPreviewView();

	void OnInitialUpdate();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL WriteTempFile( 
	CFile&	theFile,
	BOOL	bShowTabooHeaders );

};

#endif // !defined(AFX_BidentPREVIEWVIEW_H__5A907A71_2B8A_11D1_8423_00805FD2F268__INCLUDED_)
