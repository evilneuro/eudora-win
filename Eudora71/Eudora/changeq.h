// CHANGEQ.H
//
// Change Queueing dialog
//

#include "helpxdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CChangeQueueing dialog

class CChangeQueueing : public CHelpxDlg
{
// Construction
public:
	CChangeQueueing(long GMTSeconds, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CChangeQueueing)
	enum { IDD = IDD_CHANGE_QUEUEING };
	CString	m_Date;
	CString	m_Time;
	int m_Type;
	//}}AFX_DATA

// Attributes
	enum { RIGHT_NOW, NEXT_TIME, ON_OR_AFTER, DONT_SEND };
	time_t m_Seconds;

// Operations
	BOOL ChangeSummary(CSummary* Sum);

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CChangeQueueing)
	virtual BOOL OnInitDialog();
	afx_msg void OnSetFocusEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	time_t m_StartSeconds;
};
