// PropertyPageRead.h : header file
//

#ifndef __PROPERTYPAGEREAD_H__
#define __PROPERTYPAGEREAD_H__

#include "controls.h"

/*/////////////////////////////////////////////////////////////////////////////
// CLabelButton button
class CLabelButton : public CButton
{
// Construction
public:
	CLabelButton();

// Attributes
public:
	int m_Label;

// Operations
public:

// Implementation
public:
	~CLabelButton();
	void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	// Generated message map functions
	//{{AFX_MSG(CLabelButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
*/
/////////////////////////////////////////////////////////////////////////////

#include "PropertyPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageRead1 dialog

class CPropertyPageRead1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageRead1)

// Construction
public:
	CPropertyPageRead1();
	~CPropertyPageRead1();

	void Apply();

	CDocument* m_pDoc;
	CString m_sPersona;
	CLabelMenu* m_pLabelMenu;
// Dialog Data
	//{{AFX_DATA(CPropertyPageRead1)
	enum { IDD = IDD_PROPPAGEREAD1 };
	CComboBox	m_cbPersonality;
	CLabelButton	m_LabelButton;
	CString	m_sMailbox;
	int		m_iPriority;
	int		m_iPersonality;
	int		m_iStatus;
	CString	m_sSubject;
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA



// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPageRead1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL SetLabel(UINT nID);

	// Generated message map functions
	//{{AFX_MSG(CPropertyPageRead1)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePersonality();
	afx_msg void OnSelchangeStatus();
	afx_msg void OnSelchangePriority();
	afx_msg void OnChangeSubject();
	afx_msg void OnLabel();
	afx_msg void OnUpdateMessageLabel(CCmdUI* pCmdUI);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageRead2 dialog

class CPropertyPageRead2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageRead2)

// Construction
public:
	CPropertyPageRead2();
	~CPropertyPageRead2();

	int GetServerStatus();
	void Apply();

	CDocument* m_pDoc;

// Dialog Data
	//{{AFX_DATA(CPropertyPageRead2)
	enum { IDD = IDD_PROPPAGEREAD2 };
	CButton	m_LockButton;
	CButton	m_EditButton;
	CButton	m_BlahButton;
	CButton	m_DelServerButton;
	int m_iServerStatus;
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPageRead2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropertyPageRead2)
	virtual BOOL OnInitDialog();
	afx_msg void OnLock();
	afx_msg void OnEdit();
	afx_msg void OnBlah();
	afx_msg void OnDelServer();
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


#endif // __PROPERTYPAGEREAD_H__
