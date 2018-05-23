// QCCustomToolBar.h: interface for the QCCustomToolBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCUSTOMTOOLBAR_H__6AF0C4E2_F166_11D0_9801_00805FD2F268__INCLUDED_)
#define AFX_QCCUSTOMTOOLBAR_H__6AF0C4E2_F166_11D0_9801_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "QICommandClient.h"
#include "SafetyPal.h"

class QCCustomToolBar : public SECCustomToolBar, public QICommandClient 
{
public:
	QCCustomToolBar();
	virtual ~QCCustomToolBar();

	DECLARE_DYNCREATE( QCCustomToolBar )

	// Returns TRUE if this toolbar accepts dropped buttons.
	virtual BOOL  AcceptDrop() const;

	// Handles a button drop
	virtual void DropButton(CRect& dropRect, SECStdBtn* pDragBtn, BOOL bAdd);

	virtual BOOL RemoveButton(int nIndex, BOOL bNoUpdate = FALSE, INT* pIndex = NULL );
	virtual BOOL DestroyWindow();

	virtual void GetBarInfoEx(SECControlBarInfo* pInfo);

	void SetSource( UINT iPosition, UINT uSrcCmdID )
	{
		m_btns[ iPosition ]->m_ulData = uSrcCmdID;
	}

	void SetButtonData(ULONG ulData, int nIndex = 0)
	{
		m_btns[nIndex]->m_ulData = ulData;
	}

	void SaveCustomInfo(
	LPCSTR	szSectionName );

	void LoadCustomInfo(
	LPCSTR	szSection );

	void LoadInYourFacePlugins();

	BOOL SpecialToolTipCaseHandled(WORD wCommandID, CString& strToolTip);

	virtual void Notify( 
	QCCommandObject*	pObject,
	COMMAND_ACTION_TYPE	theAction,
	void*				pData = NULL );

	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );

	virtual BOOL ConvertOldStuff();

	BOOL CreateEx(DWORD dwExStyle, CWnd* pParentWnd, DWORD dwStyle = 
				WS_VISIBLE | WS_CHILD | CBRS_TOP, UINT nID = AFX_IDW_TOOLBAR, 
				LPCTSTR lpszTitle = NULL);

	void ResetImageOffsets();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

protected:
	CSafetyPalette m_Palette;
protected:
	//{{AFX_MSG(QCCustomToolBar)
	afx_msg LRESULT OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual SECStdBtn* CreateSeparator(SECCustomToolBar* pToolBar = NULL);
};

#endif // !defined(AFX_QCCUSTOMTOOLBAR_H__6AF0C4E2_F166_11D0_9801_00805FD2F268__INCLUDED_)
