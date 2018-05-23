// DockBar.h : header file
//

#if !defined(AFX_DOCKBAR_H__5273A2EB_8261_11D3_B8E4_00805F9BF82F__INCLUDED_)
#define AFX_DOCKBAR_H__5273A2EB_8261_11D3_B8E4_00805F9BF82F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



/////////////////////////////////////////////////////////////////////////////
// QCDockBar window

class QCDockBar : public SECDockBar
{
// Construction
public:
	QCDockBar();

// Attributes
public:
	virtual BOOL IsControlBarAtMaxWidthInRow(SECControlBar* pBar);
	virtual BOOL IsOnlyControlBarInRow(SECControlBar* pBar);
	bool			FindControlBarLocation(SECControlBar* in_pBar, int & out_nCol, int & out_nRow);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(QCDockBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~QCDockBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(QCDockBar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void NormalizeRow(int nPos, CControlBar* pBarDocked, int& nBarsBidirectional, int& nBarsUnidirectional);
	virtual void CalcTrackingLimits(Splitter* pSplitter);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCKBAR_H__5273A2EB_8261_11D3_B8E4_00805F9BF82F__INCLUDED_)
