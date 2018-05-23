// PropertyPageMbx.h : header file
//

#ifndef __PROPERTYPAGEMBX_H__
#define __PROPERTYPAGEMBX_H__

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMbx1 dialog

class CPropertyPageMbx1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageMbx1)

// Construction
public:
	CPropertyPageMbx1();
	~CPropertyPageMbx1();

	CDocument* m_pDoc;
// Dialog Data
	//{{AFX_DATA(CPropertyPageMbx1)
	enum { IDD = IDD_PROPPAGEMBX1 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPageMbx1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropertyPageMbx1)
	afx_msg void OnPropMbxCompact();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMbx2 dialog

class CPropertyPageMbx2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageMbx2)

// Construction
public:
	CPropertyPageMbx2();
	~CPropertyPageMbx2();

	CDocument* m_pDoc;

// Dialog Data
	//{{AFX_DATA(CPropertyPageMbx2)
	enum { IDD = IDD_PROPPAGEMBX2 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPageMbx2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropertyPageMbx2)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageMbx3 dialog

class CPropertyPageMbx3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageMbx3)

// Construction
public:
	CPropertyPageMbx3();
	~CPropertyPageMbx3();

	CDocument* m_pDoc;

// Dialog Data
	//{{AFX_DATA(CPropertyPageMbx3)
	enum { IDD = IDD_PROPPAGEMBX3 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertyPageMbx3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropertyPageMbx3)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};



#endif // __PROPERTYPAGEMBX_H__
