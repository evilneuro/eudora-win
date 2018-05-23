#if !defined(AFX_CERTMANAGERDLG_H__7F470EA1_B740_11D4_B57B_0008C7392A1C__INCLUDED_)
#define AFX_CERTMANAGERDLG_H__7F470EA1_B740_11D4_B57B_0008C7392A1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CertManagerDlg.h : header file
//
#include <afxcmn.h>
/////////////////////////////////////////////////////////////////////////////
// CCertManagerDlg dialog
struct ConnectionInfo;
struct CertData;
class CCertManagerDlg : public CDialog
{
// Construction
public:
	int Run(const CString& persona, const CString& protocol, ConnectionInfo *pInfo);
	CCertManagerDlg(CWnd* pParent = NULL);   // standard constructor
	enum CertBitmaps
	{
		SC_ClosedFolder = 0,
		SC_OpenFolder = 1,
		UC_ClosedFolder = 0,
		UC_OpenFolder = 1,
		EC_ClosedFolder = 0,
		EC_OpenFolder = 1,
		SC_TrustedAndNotExpired = 2,//SC = server cert
		SC_TrustedAndExpired = 9,
		SC_NotTrustedAndNotExpired = 3,
		SC_NotTrustedAndExpired = 10,
		UC_TrustedAndNotExpired = 4,//UC = user cert
		UC_TrustedAndExpired = 11,
		EC_TrustedAndNotExpired = 7,//RC = Eudora provided cert
		EC_TrustedAndExpired = 6,
		SC_TrustedAndNotExpiredParent = 13,//SC = server cert
		SC_TrustedAndExpiredParent = 12,
		SC_NotTrustedAndNotExpiredParent = 15,
		SC_NotTrustedAndExpiredParent = 14,
	};

// Dialog Data
	//{{AFX_DATA(CCertManagerDlg)
	enum { IDD = IDD_DIALOG_CERT_MANAGER };
	CComboBox	m_AlgoList;
	CButton	m_BtnImportCert;
	CButton	m_BtnExportCert;
	CTreeCtrl	m_TreeServerCerts;
	CButton	m_BtnRemoveCert;
	CButton	m_BtnCertDetails;
	CButton	m_BtnAddCert;
	CString	m_IssuedBy;
	CString	m_Subject;
	CString	m_ValidFrom;
	CString	m_ValidTo;
	CString	m_SerialNo;
	CString	m_ThumbPrint;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCertManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStringList m_FilesToTrash;
	int CreateHashAlgList();
	bool IsEuCertItem(HTREEITEM & hItem);
	bool IsServerCertItem(HTREEITEM & hItem);
	bool IsUserCertItem(HTREEITEM & hItem);
	bool IsClientCertItem(HTREEITEM & hItem);
	bool RefreshUserCertList();
	CertData* GetSelectedCert();
	bool CreateUserList();
	bool CreateRootList();
	bool CreateChainTree(ConnectionInfo *pInfo);
	bool CreateClientChainTree(ConnectionInfo *pInfo);
	CString m_Persona;
	CString m_Protocol;
	ConnectionInfo * m_pConnectionInfo;
	HTREEITEM m_htServerCerts;
	HTREEITEM m_htUserCerts;
	HTREEITEM m_htRootCerts;
	HTREEITEM m_htClientCerts;
	CImageList m_ImageList;
	// Generated message map functions
	//{{AFX_MSG(CCertManagerDlg)
	afx_msg void OnClickTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonCertdetails();
	afx_msg void OnButtonAddcert();
	afx_msg void OnButtonRemoveCert();
	afx_msg void OnDblclkTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturnTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandedTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandingTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetdispinfoTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimeChange();
	virtual void OnOK();
	afx_msg void OnButtonExportcert();
	afx_msg void OnButtonImportcert();
	afx_msg void OnSelchangeComboThumbalgo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CERTMANAGERDLG_H__7F470EA1_B740_11D4_B57B_0008C7392A1C__INCLUDED_)
