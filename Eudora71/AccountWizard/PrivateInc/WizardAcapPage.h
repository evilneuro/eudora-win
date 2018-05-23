// WizardAcapPage.h

#ifndef _WIZARDACAPPAGE_H_
#define _WIZARDACAPPAGE_H_

#include "resource.h"

class CWizardPropSheet ;

/////////////////////////////////////////////////////////////////////////
/**** If ACAP is specified configured ***/

class CWizardAcapPage : public CPropertyPage
{
// Construction
public:
	CWizardAcapPage( CWizardPropSheet*);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWizardAcapPage)
	enum { IDD = IDD_DIALACAP };
	CString m_stracapserver ;
	CString m_stracapusername ;
	CString m_stracappassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardAcapPage)
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	private:
		CWizardPropSheet *m_pParent;

public:
#ifdef _DEBUG
        void * __cdecl
	operator new(size_t size, const char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, int, const char *file,
				    int line)
	{
	    return ::new(file, line) char[size];
	}
        void * __cdecl
	operator new(size_t size, int, char *file, int line)
	{
	    return ::new(file, line) char[size];
	}
#else
        void * __cdecl operator new(size_t size)
	{
	    return ::new char[size];
	}
#endif
        void operator delete(void *pv)
	{
	    delete [] ((char *) pv);
	}

// Implementation
protected:
	void UpdateButtons();

	// Generated message map functions
	//{{AFX_MSG(CWizardAcapPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeIniAcapPassword();
	afx_msg void OnChangeIniAcapServer();
	afx_msg void OnChangeIniAcapUserId();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // _WIZARDACAPPAGE_H_
