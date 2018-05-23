// QVoiceDoc.h : interface of the CQVoiceDoc class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _QVOICEDOC_H_
#define _QVOICEDOC_H_

#include "lscommon.h"
#include "LsStateEvent.h"
#include "LsStateMachine.h"
#include "LsStateAction.h"

class CQVoiceDoc : public CDocument
{
// LIFECYCLE
protected: // create from serialization only
	CQVoiceDoc();
	DECLARE_DYNCREATE(CQVoiceDoc)
public:
	virtual ~CQVoiceDoc();

public:
//	OPERATIONS

//	ACCESS
	LsStateAction* StateAction();
	LsStateEvent* StateEvent();

protected:
	LsStateEvent	m_Event;
	LsStateMachine	m_State;
	LsStateAction	m_Action;

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	BOOL DoPromptFileName(CString& fileName, LPCTSTR sTitle, DWORD lFlags, BOOL bOpenFileDialog );
//	ATTRIBUTES
	BOOL	mb_OneTime;

//////////////////////////////////////////////////////////////////////////
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQVoiceDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	//}}AFX_VIRTUAL

	BOOL SaveModified();

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CQVoiceDoc)
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveAs();
	afx_msg void OnAppExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
/////////////////////////////////////////////////////////////////////////////
