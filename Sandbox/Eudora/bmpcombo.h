// BMPCOMBO.H
//
#ifndef _BMPCOMBO_H_
#define _BMPCOMBO_H_

#include "font.h"

class CBitmapComboItem
{
public:
	CBitmapComboItem(UINT BitmapID, UINT StringID, int ID = 0);
	CBitmapComboItem(UINT BitmapID, CString String, int ID = 0);
	
	CString m_Text;
	CBitmap m_Bitmap;
	int m_BitmapWidth, m_BitmapHeight;
	int m_ID;
};

/////////////////////////////////////////////////////////////////////////////
// CBitmapCombo window

class CBitmapCombo : public CComboBox
{
	DECLARE_DYNCREATE(CBitmapCombo)

	CWnd*	m_pLastFocus;

// Construction
public:
	CBitmapCombo();

// Attributes
public:
	int m_Width, m_Height, m_ItemHeight;
	UINT	m_EditWidth;

// Operations
public:

// Implementation
public:
	virtual ~CBitmapCombo();
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	int Add(CBitmapComboItem* Item);
	CBitmapComboItem* GetItemDataPtr(int nIndex) const
		{ return ((CBitmapComboItem*)CComboBox::GetItemDataPtr(nIndex)); }

	void RestoreFocus();
	void SetEditWidth( INT iWidth ) { m_EditWidth = iWidth; }

protected:
	// Generated message map functions
	//{{AFX_MSG(CBitmapCombo)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	virtual void PostNcDestroy();

protected:
	static CFontInfo m_Font;
	static int m_Count;
	enum {SPACE = 3};
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CPriorityCombo window

class CPriorityCombo : public CBitmapCombo
{
// Construction
public:
	CPriorityCombo();

// Attributes
public:

// Operations
public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Implementation
public:
	~CPriorityCombo();

protected:
	// Generated message map functions
	//{{AFX_MSG(CPriorityCombo)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif	// _BMPCOMBO_H_
