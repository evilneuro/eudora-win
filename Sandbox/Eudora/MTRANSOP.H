// mtransop.h : header file
//

#include "helpxdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMailTransferOptionsDialog dialog

class CMailTransferOptionsDialog : public CHelpxDlg
{
// Construction
public:
	CMailTransferOptionsDialog(BOOL Sending = FALSE, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CMailTransferOptionsDialog)
	enum { IDD = IDD_MAIL_TRANSFER_OPTIONS };
	CListBox	m_Personas;
	BOOL	m_RetrieveNew;
	BOOL	m_Send;
	BOOL	m_DeleteMarked;
	BOOL	m_RetrieveMarked;
	BOOL	m_DeleteRetrieved;
	BOOL	m_DeleteAll;
	BOOL	m_FetchHeaders;
	int		m_PersonaOptions;
	//}}AFX_DATA


// Overrides
public:
	virtual BOOL OnInitDialog();
	virtual int DoModal();
		  
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMailTransferOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	// Generated message map functions
	//{{AFX_MSG(CMailTransferOptionsDialog)
	afx_msg void OnFetchHeadersClicked();
	virtual void OnOK();
	afx_msg void OnDblclkPersonas();
	afx_msg void OnOptionsClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	BOOL	m_DeleteMarkedExist;
	BOOL	m_RetrieveMarkedExist;
	LPSTR	m_lpNameList; 
	BOOL	m_bMultiplePersonas;
	BOOL	m_SendingFlag;
};
