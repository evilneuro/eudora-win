#if !defined(AFX_LABELCOMBOBOX_H__D1F0392A_77FB_11D2_9511_00805F9BF4D7__INCLUDED_)
#define AFX_LABELCOMBOBOX_H__D1F0392A_77FB_11D2_9511_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LabelComboBox.h : header file
//

#include "QCLabelDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CLabelComboBox window

class CLabelComboBox : public CComboBox
{
// Construction
public:
	CLabelComboBox();

// Attributes
public:

// Operations
public:
	bool InitLabels();
	unsigned int GetCurLabel() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabelComboBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLabelComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLabelComboBox)
	afx_msg void OnDropdown();
	//}}AFX_MSG

	QCLabelDirector *m_LblDir;
	COLORREF m_HighlightBkgdClr, m_HighlightTextClr, m_WindowTextClr, m_WindowBkgdClr;
	bool m_bInited;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABELCOMBOBOX_H__D1F0392A_77FB_11D2_9511_00805F9BF4D7__INCLUDED_)
