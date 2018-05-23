#if !defined(AFX_QCTOOLBARCMDPAGE_H__E4CE4466_ECC1_11D0_97FF_00805FD2F268__INCLUDED_)
#define AFX_QCTOOLBARCMDPAGE_H__E4CE4466_ECC1_11D0_97FF_00805FD2F268__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QCToolBarCmdPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// QCToolBarCmdPage dialog

class QCToolBarCmdPage : public SECToolBarCmdPage
{
	DECLARE_DYNAMIC(QCToolBarCmdPage)

// Construction
public:
	QCToolBarCmdPage(
	UINT	uID,
	UINT	uTitleID);
	
	~QCToolBarCmdPage();

	virtual UINT TranslateID(
	UINT	uID );

// Dialog Data
	//{{AFX_DATA(QCToolBarCmdPage)
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides

	virtual BOOL OnSetActive();

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(QCToolBarCmdPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:


	// Generated message map functions
	//{{AFX_MSG(QCToolBarCmdPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QCTOOLBARCMDPAGE_H__E4CE4466_ECC1_11D0_97FF_00805FD2F268__INCLUDED_)
