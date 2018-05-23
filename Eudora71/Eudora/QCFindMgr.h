// QCFindMgr.h: interface for the QCFindMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCFINDMGR_H__FCF87B16_8DF5_11D2_9518_00805F9BF4D7__INCLUDED_)
#define AFX_QCFINDMGR_H__FCF87B16_8DF5_11D2_9518_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

//#include "stdafx.h"
#include <afxdlgs.h> // CFindReplaceDialog
#include "FindDlg.h"

// --------------------------------------------------------------------------

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);
static UINT WM_RESTOREFOCUS = ::RegisterWindowMessage("RestoreFocus");

// Return values for the message handler
#define EuFIND_OK         (0)
#define EuFIND_ERROR      (1)
#define EuFIND_NOTFOUND   (2)
#define EuFIND_ACK_YES    (3) // Acknowledge for support of find
#define EuFIND_ACK_NO     (4) // Acknowledge for support of find

// --------------------------------------------------------------------------

class QCFindState
{
public:
	QCFindState()
		: m_FindStr(""), m_bMatchCase(false), m_bWholeWord(false) { }
	QCFindState(const QCFindState &copy)
		: m_FindStr(copy.m_FindStr), m_bMatchCase(copy.m_bMatchCase), m_bWholeWord(copy.m_bWholeWord) { }
	QCFindState &operator=(const QCFindState &copy)
		{ m_FindStr = copy.m_FindStr; m_bMatchCase = copy.m_bMatchCase; m_bWholeWord = copy.m_bWholeWord; return (*this); }
	virtual ~QCFindState() { }

	LPCTSTR GetFindText() const { return (m_FindStr); }
	int GetFindTextLen() const { return (m_FindStr.GetLength()); }
	bool IsMatchCase() const { return (m_bMatchCase); }
	bool IsWholeWord() const { return (m_bWholeWord); }

	void SetState(CFindReplaceDialog *pDlg);
	void SetState(LPCTSTR sFindStr, bool bMatchCase, bool bWholeWord)
		{ m_FindStr = sFindStr; m_bMatchCase = bMatchCase; m_bWholeWord = bWholeWord; }

protected:
	CString m_FindStr;
	bool m_bMatchCase, m_bWholeWord;
};

// --------------------------------------------------------------------------

class QCFindMgr  
{
public:
	QCFindMgr();
	virtual ~QCFindMgr();

	static QCFindMgr *GetFindMgr();
	CFindDlg * GetFindDlg();

	bool Create(); // Brings up dlg (init dlg)
	bool Destroy(); // Call when dlg is going away (saves state)

	bool UpdateLastFindState(LPCTSTR sFindStr, bool bMatchCase, bool bWholeWord);
	bool UpdateLastFindState(); // Call this with each find -- to save state
	bool GetLastFindState(QCFindState &) const; // Call this to get Find Again info (F3)
	bool CanFindAgain() const;

	bool SetFindWnd(CWnd *pWnd);
	CWnd *GetFindWnd() const;

	bool DoFind(WPARAM wParam, LPARAM lParam);
	bool DoFindAgain();

	void UpdateFindFocus();


	static bool IsAck(WPARAM wParam, LPARAM lParam);
	static LONG SendAck(CWnd *pWnd);

protected:
	void DisplayNotFound(LPCTSTR pStr);
	bool SendFindMsg(WPARAM wParam, LPARAM lParam);

	static QCFindMgr *m_pTheFindMgr;

	CFindDlg *m_pDlg;

	// Last dialog state -- used to init next Find dlg
	QCFindState m_DlgState;
	CRect m_DlgRct; // Screen coord of last FindDlg

	// Last find state -- used for Find Again (F3)
	bool m_bValidFindState;
	QCFindState m_FindState;

	CWnd *m_pWnd;
};

#endif // !defined(AFX_QCFINDMGR_H__FCF87B16_8DF5_11D2_9518_00805F9BF4D7__INCLUDED_)
