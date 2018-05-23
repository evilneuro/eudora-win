// ReadMessageFrame.h: interface for the CReadMessageFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READMESSAGEFRAME_H__7EA9D342_E0EE_11D0_97ED_00805FD2F268__INCLUDED_)
#define AFX_READMESSAGEFRAME_H__7EA9D342_E0EE_11D0_97ED_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "msgframe.h"
#include "QCOleDropSource.h"
#include "QCProtocol.h"

class SECToolBarManager;
class QCChildToolBar;
class CReadMessageToolBar;

class CReadMessageFrame : public CMessageFrame, public QCProtocol
{
    COleDataSource			m_OleDataSource;
	QCChildToolBar*			m_pFormattingToolBar;
	CReadMessageToolBar*	m_pToolBar;
	SECToolBarManager*		m_pToolBarManager;
	INT						m_iMajorVersion;
	INT						m_iMinorVersion;
	
	DECLARE_DYNCREATE(CReadMessageFrame)

protected:

	//{{AFX_MSG(CReadMessageFrame)
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	//}}AFX_MSG

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	afx_msg void OnDeleteFromServer();
	afx_msg void OnDownloadFromServer();
	afx_msg void OnBlahBlahBlah();
	afx_msg void OnEditMode();
	afx_msg void OnChangeSubject();
	afx_msg void OnPriorityChange();
	afx_msg void OnDelete();
	afx_msg long OnSetPriority(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptScreen);
	afx_msg long OnButtonSetCheck(WPARAM wParam, LPARAM lParam);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual BOOL OnCmdMsg(
	UINT				nID, 
	int					nCode, 
	void*				pExtra, 
	AFX_CMDHANDLERINFO*	pHandlerInfo );

	afx_msg BOOL OnDynamicCommand(
	UINT uID );
	
	afx_msg void OnUpdateDynamicCommand(
	CCmdUI* pCmdUI);
	
	afx_msg void OnUpdateAlwaysEnabled(
	CCmdUI*	pCmdUI);

#ifdef IMAP4
	afx_msg void OnUpdateUndelete(CCmdUI* pCmdUI);
	afx_msg void UnDelete();
#endif // IMAP4

public:

	DECLARE_MESSAGE_MAP()

	CReadMessageFrame();
	virtual ~CReadMessageFrame();

	virtual BOOL OnCreateClient( 
	LPCREATESTRUCT	lpcs,
	CCreateContext* pContext);

	QCChildToolBar* GetFormatToolbar();
	QCChildToolBar* GetToolbar();

public:
	BOOL UseFixedWidthFont();
	
	INT	GetMajorVersion() const { return m_iMajorVersion; }
	INT	GetMinorVersion() const { return m_iMinorVersion; }

	BOOL GetCheck( UINT nCommandID );
	BOOL GetFormattingCheck( UINT nCommandID );

	CComboBox* GetFontCombo();

	BOOL QuietPrint();

	void EnableTooltips( BOOL bEnable );

	BOOL GetSubject(
	CString& szText );

	void ZoomWindow();

	CView* KludgeInAFindView();
	void KillTheKludgyView();
	void KillOriginalView();
};

#endif // !defined(AFX_READMESSAGEFRAME_H__7EA9D342_E0EE_11D0_97ED_00805FD2F268__INCLUDED_)
