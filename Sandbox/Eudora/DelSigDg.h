// DelSigDg.h : header file
//
#ifndef DELSIGDG_H
#define DELSIGDG_H

/////////////////////////////////////////////////////////////////////////////
// CDeleteDialog dialog

class CDeleteDialog : public CDialog
{
	BOOL	m_bDeleteStationery;

// access functions
	CListBox&	SignatureLB(){ return * ( ( CListBox* ) GetDlgItem( IDC_SIGNATURE_LIST ) ); }
// Construction
public:
	CDeleteDialog(
	BOOL	bDeleteStationery,
	CWnd*	pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeleteDialog)
	enum { IDD = IDD_DELETE_SIGNATURE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CDeleteDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
