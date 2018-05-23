// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:			John Williston
//  Description:	Declaration for SECUserToolsDlg
//  Created:		August 1996
//

#ifndef __USRTLDLG_H__
#define __USRTLDLG_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

class SECBrowseFileEdit;
class SECBrowseDirEdit;
class SECUserTool;

/////////////////////////////////////////////////////////////////////////////
// SECUserToolsDlg dialog

class SECUserToolsDlg : public CDialog
{
// Construction
public:
	SECUserToolsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SECUserToolsDlg)
	enum { IDD = IDD_UTM_USER_TOOLS };
	CListBox	m_lbTools;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SECUserToolsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SECUserToolsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnSelchangeList();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	afx_msg void OnRemove();
	afx_msg void OnChangeMenuText();
	afx_msg void OnKillfocusMenuText();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////
//
// customizations
//
////////////////////////////////////////////////////////////////////////////////

protected:
	CObArray* m_pTools;

	afx_msg void OnUpdateAdd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRemove(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMoveUp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMoveDown(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFields(CCmdUI* pCmdUI);

	SECBrowseFileEdit* m_peditCmd;
	SECBrowseDirEdit* m_peditDir;
	SECUserTool* m_pLastTool;

	void UpdateLastTool( SECUserTool* pNewTool );

public:
	~SECUserToolsDlg();

	void SetToolsArrayPtr(CObArray* p)
		{ m_pTools = p; }
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif	// __USRTLDLG_H__
