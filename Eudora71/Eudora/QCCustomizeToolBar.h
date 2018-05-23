// QCCustomizeToolBar.h: interface for the QCCustomizeToolBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCCUSTOMIZETOOLBAR_H__E4CE4467_ECC1_11D0_97FF_00805FD2F268__INCLUDED_)
#define AFX_QCCUSTOMIZETOOLBAR_H__E4CE4467_ECC1_11D0_97FF_00805FD2F268__INCLUDED_

#include "tbarpage.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class QCCustomizeToolBar : public SECCustomizeToolBar  
{

public:
	DECLARE_DYNAMIC(QCCustomizeToolBar);

	QCCustomizeToolBar();
	virtual ~QCCustomizeToolBar();

	virtual BOOL DragButton(int nIndex, CPoint point);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnMyQueryNewPalette(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSysColorChange();
	virtual void NewToolBar( CPoint pt, SECStdBtn& btn, BOOL bPrompt );

protected:
	CSafetyPalette m_Palette;
};

#endif // !defined(AFX_QCCUSTOMIZETOOLBAR_H__E4CE4467_ECC1_11D0_97FF_00805FD2F268__INCLUDED_)
