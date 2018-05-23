#ifndef _MODIFYACCTSHEETH__
#define _MODIFYACCTSHEETH__

#include "ModifyAcctDlgs.h"
#include "persona.h"
#include "PersParams.h"

//The Property Sheet for the Modify Personality/Account 

/*********************** SAMEER JINDAL sometime in Late 1997 *******************/

class CModifyAcctSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CModifyAcctSheet)

// Construction
public:
	CModifyAcctSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, const CString& strName  = "<Dominant>", bool bCreateNew = false);
	CModifyAcctSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, const CString& strName = "<Dominant>", bool bCreateNew = false);
	virtual ~CModifyAcctSheet();

	void Create(); // calls DoModal internally

// Attributes
public:

	// Maintain pointers to all property pages
	CModifyAcct_Generic *m_pGenericDlg;
	CModifyAcct_Servers *m_pServersDlg;

	CWnd *m_pParent;

	bool m_bCreateNew;
	CPersParams m_params, m_original_params;

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyAcctSheet)
	//}}AFX_VIRTUAL

protected:
	void UpdateWazoo(LPCSTR sPersName);

	// Generated message map functions
	//{{AFX_MSG(CModifyAcctSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	virtual BOOL OnInitDialog() ;
	virtual void OnHelp() ;
	virtual void OnOK() ;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif _MODIFYACCTSHEETH__
