// PropertyPanel.h : header file
//
// CPropertyPanel is a modeless property sheet that is 
// created once and not destroyed until the application
// closes.  It is initialized and controlled from
// CPropertyFrame.
 // CPropertyPanel has been customized to include
// a preview window.

#ifndef __PROPERTYPAGEEMPTY_H__
#define __PROPERTYPAGEEMPTY_H__

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageEmpty dialog

class CPropertyPageEmpty : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageEmpty)

// Construction
public:
	CPropertyPageEmpty();
	~CPropertyPageEmpty();

// Dialog Data
	//{{AFX_DATA(CPropertyPageEmpty)
	enum { IDD = IDD_PROPPAGEEMPTY };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPageEmpty)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropertyPageEmpty)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif	// __PROPERTYPAGEEMPTY_H__
 
#ifndef __PROPERTYPANEL_H__
#define __PROPERTYPANEL_H__

#include "PropertyPageMbx.h"
#include "PropertyPageRead.h"
#include "controls.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyPanel

class CPropertyPanel : public CPropertySheet
{
	DECLARE_DYNAMIC(CPropertyPanel)

// Construction
public:
	CPropertyPanel(CWnd* pWndParent = NULL, CDocument* pDoc = NULL);

// Attributes
public:

	CWnd* m_pWndParent;
	CDocument * m_pDoc;

	CPropertyPageMbx1 m_PageMbx1;
	CPropertyPageMbx2 m_PageMbx2;
	CPropertyPageMbx3 m_PageMbx3;

	CPropertyPageRead1 m_PageReadMessage1;
	CPropertyPageRead2 m_PageReadMessage2;

//	CPropertyPageComp1 m_PageComposition1;
//	CPropertyPageComp2 m_PageComposition2;
//	CPropertyPageComp3 m_PageComposition3;

//	CPropertyPageAttach1 m_PageAttachment1;

	CPropertyPageEmpty m_EmptyPage;

// Operations
private:
	CButton m_cbOK;
	CButton m_cbCancel;
	CButton m_cbApply;

	CMyBitmapButton m_cbTack;
	CMyBitmapButton m_cbWin;

public:
	int m_bPressedTack;
	static int m_bMultiWin;
	BOOL m_bPropertyChanged;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPanel)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyPanel();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	void UpdateApply();
	CFont * GetSysFont(int whichFont =-1);


// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyPanel)
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnApply();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnPropTack();
	afx_msg void OnPropMultiWin();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

//	afx_msg void OnTack();
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __PROPERTYPANEL_H__
