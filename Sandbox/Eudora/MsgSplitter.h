#ifndef MSGSPLITTER_H
#define MSGSPLITTER_H

#include "SplitHlp.h"

// CMsgSplitterWnd:
// Provides splitter funtionality for Message and TOC frames

class CMsgSplitterWnd : public CSplitterHelpWnd
{
	CRuntimeClass* m_pClass;

public:
	CMsgSplitterWnd();
	~CMsgSplitterWnd();

protected:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT, short, CPoint);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()

public:

	virtual BOOL CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext);

	void SetViewType(CRuntimeClass* pClass);
	CRuntimeClass* GetViewType() { return m_pClass; }
};


#endif
