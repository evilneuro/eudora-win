/*////////////////////////////////////////////////////////////////////////////

NAME:
	CNameToolBar - 

FILE:		NameToolBar.h
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	Derived from CDialogBar.

RESCRITICTIONS:

FILES:

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/05/97   lss     -Initial

/*////////////////////////////////////////////////////////////////////////////
#ifndef _NAMETOOLBAR_H_
#define _NAMETOOLBAR_H_


class CLsEdit : public CEdit
{
protected:
	void OnKillFocus(CWnd* pNewWnd);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNameToolBar ToolBar

class CNameToolBar : public CDialogBar
{
public:
//	LIFECYCLE
	CNameToolBar(); 
	~CNameToolBar();

//	OPERATIONS
	BOOL	Init(CWnd* pParentWnd);
	void	Show( BOOL bShow = TRUE );

//	ACCESS
	LPCTSTR	Name();
	LPCTSTR	Name( LPCTSTR name );

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	void		setup();

//	ATTRIBUTES
	CFrameWnd*	mp_MainFrame;
	CLsEdit		m_edtName;
	CString		m_name;
	BOOL		mb_Show;
	BOOL		mb_firstTime;
	int			m_Height;

//////////////////////////////////////////////////////////////////////////////
// ClassWizard stuff
//////////////////////////////////////////////////////////////////////////////
public:
	CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNameToolBar)
	//}}AFX_VIRTUAL
	// Generated message map functions
protected:
	//{{AFX_MSG(CNameToolBar)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
