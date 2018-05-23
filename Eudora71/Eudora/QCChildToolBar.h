// QCChildToolBar.h: interface for the QCChildToolBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCHILDTOOLBAR_H__577A4910_1351_11D1_8415_00805FD2F268__INCLUDED_)
#define AFX_QCCHILDTOOLBAR_H__577A4910_1351_11D1_8415_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "tbarcust.h"
#include "SafetyPal.h"

class QCChildToolBar : public SECCustomToolBar  
{
protected:

    //{{AFX_MSG(QCChildToolBar)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
	afx_msg LRESULT OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
	
	DECLARE_DYNCREATE(QCChildToolBar)

	virtual BOOL InitButtons();
	
	virtual BOOL AcceptDrop() const;
	
	QCChildToolBar();
	
	virtual ~QCChildToolBar();
	
	void EnableDocking(DWORD dwDockStyle);
	
	void* GetButton(
	INT iIndex );
	
	INT Height() { return m_szTbarDockHorz.cy; }

	BOOL GetDragMode() const;
	BOOL DragButton(int nIndex, CPoint point);

	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	void AddTranslatorButtons(int &startPos);

	BOOL CreateEx(DWORD dwExStyle, CWnd* pParentWnd, DWORD dwStyle, UINT nID, LPCTSTR lpszTitle);
protected:
	CSafetyPalette m_Palette;
	DWORD			m_nLastIdleProcessing;
};

#endif // !defined(AFX_QCCHILDTOOLBAR_H__577A4910_1351_11D1_8415_00805FD2F268__INCLUDED_)
