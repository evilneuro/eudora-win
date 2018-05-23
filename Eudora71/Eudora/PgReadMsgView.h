#if !defined(AFX_PGREADMSGVIEW_H__35E93282_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
#define AFX_PGREADMSGVIEW_H__35E93282_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PgReadMsgView.h : header file
//

#include "PgMsgView.h"

/////////////////////////////////////////////////////////////////////////////
// PgReadMsgView view

class PgReadMsgView : public PgMsgView
{
protected:
	PgReadMsgView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(PgReadMsgView)

// Attributes
public:

protected:
	short m_TabooHeadersStyleID;
	long m_HeaderBytes;
	bool m_bWasConcentrated;
	bool m_bCanConcentrate;
// Operations
public:

	virtual CTocDoc *		GetTocDoc();
	
	BOOL GetMessageAsHTML(
		CString& szText,
		BOOL includeHeaders);

	BOOL GetMessageAsText(
		CString& szText,
		BOOL includeHeaders);

	BOOL SetAllText(
		const char* szText,bool bSign );

	BOOL SetAllHTML(
		const char* szHTML,bool bSign );

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PgReadMsgView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

	virtual BOOL RefreshMessage();

// Implementation
protected:
	virtual ~PgReadMsgView();

	virtual void				SetMoreMessageStuff(
										PgStuffBucket *						pStuffBucket);

	virtual void				ImportMessage(
										ContentConcentrator::ContextT		in_context = ContentConcentrator::kCCFullViewContext,
										bool								in_bRefresh = true,
										CMessageDoc *						pMsgDoc = NULL );
	virtual void ExportMessage( CMessageDoc* pMsgDoc = NULL );
	void						ReloadAndConcentrateIfAppropriate(
										bool								in_bIsRO,
										bool								in_bRefresh = true,
										bool								in_bMustReload = false );
	void SetTabooHeadersStyle();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(PgReadMsgView)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditMode();
	afx_msg void OnBlahBlahBlah();
	afx_msg void OnFixedFont();
	//afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PGREADMSGVIEW_H__35E93282_E9CD_11D0_AC4F_00805FD2626C__INCLUDED_)
