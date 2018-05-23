// CompMessageFrame.h: interface for the CCompMessageFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPMESSAGEFRAME_H__7EA9D343_E0EE_11D0_97ED_00805FD2F268__INCLUDED_)
#define AFX_COMPMESSAGEFRAME_H__7EA9D343_E0EE_11D0_97ED_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "msgframe.h"
#include "QICommandClient.h"
#include "QCProtocol.h"

#define WM_USER_UPDATE_IMMEDIATE_SEND WM_USER+222

class CHeaderView;
class PgCompMsgView;
class QCChildToolBar;
class SECToolBarManager;
class QCToolBarManagerWithBM;

class CCompMessageFrame : public CMessageFrame, public QICommandClient, public QCProtocol
{
	QCChildToolBar*		m_pFormattingToolBar;
	QCChildToolBar*		m_pToolBar;
	QCToolBarManagerWithBM*	m_pToolBarManager;
	CMenu				m_theTranslatorMenu;
	CStringArray		m_thePropertyList;
	int					m_nHeaderHeight;
	bool				m_bBPWarning;

	void SelectTranslators(const char *hdr);

	DECLARE_DYNCREATE(CCompMessageFrame)

// Generated message map functions
protected:
	//{{AFX_MSG(CCompMessageFrame)
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles( HDROP );
	afx_msg void OnUpdateFileSaveAsStationery(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCheckSpelling(CCmdUI* pCmdUI);
	afx_msg void OnCheckSpelling();
	afx_msg void OnUpdateInsertSystemConfiguration(CCmdUI* pCmdUI);
	afx_msg void OnInsertSystemConfiguration();
	afx_msg void OnUpdateEditFindFindText(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindFindTextAgain(CCmdUI* pCmdUI);
	//}}AFX_MSG

	afx_msg LONG OnFindReplace(WPARAM wParam, LPARAM lParam);

	afx_msg void OkIfEditable(CCmdUI* pCmdUI);
	afx_msg void OnMoodMailUI(CCmdUI* pCmdUI);

	afx_msg void OnPriorityChange();
	afx_msg void OnSignatureChange();
	afx_msg void OnEncodingChange();
	afx_msg void OnQuotedPrintable();
	afx_msg void OnTextAsDocument();
	afx_msg void OnWordWrap();
	afx_msg void OnTabsInBody();
	afx_msg void OnKeepCopies();
	afx_msg void OnReturnReceipt();
	afx_msg long OnAddAttachment( WPARAM	wParam, LPARAM	lParam );
	afx_msg long OnSetSignature( WPARAM	wParam, LPARAM	lParam );
	afx_msg long OnSetPriority( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUserUpdateImmediateSend( WPARAM wParam, LPARAM lParam );
	afx_msg void OnToggleHeader();
//	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:

	DECLARE_MESSAGE_MAP()

// Quick! What is the access specifier of the following methods?
// Here's a hint: it's not public.

	CCompMessageFrame();
	virtual ~CCompMessageFrame();

	virtual BOOL OnCreateClient( 
	LPCREATESTRUCT	lpcs,
	CCreateContext* pContext);

public:
	CHeaderView* GetHeaderView();
	PgCompMsgView *			GetPgCompMsgView() const;
	BOOL AddAttachment( const char* Filename );

	// oooh, a public afx_msg!!
	afx_msg void OnAttachFile();
	void EnableTooltips( BOOL bEnable );

	void SetCheck( UINT nCommandID, BOOL bChecked );
	BOOL GetCheck( UINT nCommandID );

	void SetBPWarning(bool bBPWarning) { m_bBPWarning = bBPWarning; }
	bool IsBPWarning() { return m_bBPWarning; }

	void Notify( 
	QCCommandObject*	pCommand,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData); 

	BOOL OnDynamicCommand(
	UINT uID );

	void OnUpdateDynamicCommand(
	CCmdUI* pCmdUI);
	
	CString GetSelectedTranslators();
	QCChildToolBar* GetFormatToolbar(); 
	QCChildToolBar* GetToolbar(); 

	//Printing releated
	BOOL QuietPrint();

};

#endif // !defined(AFX_COMPMESSAGEFRAME_H__7EA9D343_E0EE_11D0_97ED_00805FD2F268__INCLUDED_)
