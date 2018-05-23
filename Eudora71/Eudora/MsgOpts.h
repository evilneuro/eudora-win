// MsgOpts.h : header file
//

#include "helpxdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageOptions dialog

class CMessageOptions : public CHelpxDlg
{
// Construction
public:
	CMessageOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMessageOptions)
	enum { IDD = IDD_MESSAGE_OPTIONS };
	CComboBox	m_StationeryCombo;
	CComboBox	m_PersonaCombo;
	CString	m_Persona;
	CString	m_Stationery;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FillInPersonalities();
	void FillInStationery();

	// Generated message map functions
	//{{AFX_MSG(CMessageOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePersonaName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
