// CertManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "rs.h"
#include "utils.h"
#include "CertManagerDlg.h"
#include "qcssl.h"
#include "QCWorkerSocket.h"


CString MakeReadable(const CString& unreadable)
{
	CString readable = unreadable;
	int length = unreadable.GetLength();
	int insertno = length/4;
	for(int i=4; i< length+insertno; i+=4)
		readable.Insert(i++," ");
	return readable;
}

#include "DebugNewHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDlg dialog


CCertManagerDlg::CCertManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCertManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCertManagerDlg)
	m_IssuedBy = _T("");
	m_Subject = _T("");
	m_ValidFrom = _T("");
	m_ValidTo = _T("");
	m_SerialNo = _T("");
	m_ThumbPrint = _T("");
	//}}AFX_DATA_INIT
}


void CCertManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCertManagerDlg)
	DDX_Control(pDX, IDC_COMBO_THUMBALGO, m_AlgoList);
	DDX_Control(pDX, IDC_BUTTON_IMPORTCERT, m_BtnImportCert);
	DDX_Control(pDX, IDC_BUTTON_EXPORTCERT, m_BtnExportCert);
	DDX_Control(pDX, IDC_TREE_SERVERCERTS, m_TreeServerCerts);
	DDX_Control(pDX, IDC_BUTTON_REMOVE_CERT, m_BtnRemoveCert);
	DDX_Control(pDX, IDC_BUTTON_CERTDETAILS, m_BtnCertDetails);
	DDX_Control(pDX, IDC_BUTTON_ADDCERT, m_BtnAddCert);
	DDX_Text(pDX, IDC_EDIT_CERT_ISSUEDBY, m_IssuedBy);
	DDX_Text(pDX, IDC_EDIT_CERT_SUBJECT, m_Subject);
	DDX_Text(pDX, IDC_EDIT_VALIDFROM, m_ValidFrom);
	DDX_Text(pDX, IDC_EDIT_VALIDTO, m_ValidTo);
	DDX_Text(pDX, IDC_EDIT_SERIALNO, m_SerialNo);
	DDX_Text(pDX, IDC_EDIT_THUMBPRINT, m_ThumbPrint);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCertManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CCertManagerDlg)
	ON_NOTIFY(NM_CLICK, IDC_TREE_SERVERCERTS, OnClickTreeServercerts)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_SERVERCERTS, OnSetfocusTreeServercerts)
	ON_BN_CLICKED(IDC_BUTTON_CERTDETAILS, OnButtonCertdetails)
	ON_BN_CLICKED(IDC_BUTTON_ADDCERT, OnButtonAddcert)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CERT, OnButtonRemoveCert)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_SERVERCERTS, OnDblclkTreeServercerts)
	ON_NOTIFY(NM_KILLFOCUS, IDC_TREE_SERVERCERTS, OnKillfocusTreeServercerts)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_SERVERCERTS, OnRclickTreeServercerts)
	ON_NOTIFY(NM_RETURN, IDC_TREE_SERVERCERTS, OnReturnTreeServercerts)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_TREE_SERVERCERTS, OnItemexpandedTreeServercerts)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_SERVERCERTS, OnItemexpandingTreeServercerts)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_SERVERCERTS, OnKeydownTreeServercerts)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_SERVERCERTS, OnSelchangedTreeServercerts)
	ON_NOTIFY(TVN_SETDISPINFO, IDC_TREE_SERVERCERTS, OnSetdispinfoTreeServercerts)
	ON_BN_CLICKED(IDC_BUTTON_EXPORTCERT, OnButtonExportcert)
	ON_BN_CLICKED(IDC_BUTTON_IMPORTCERT, OnButtonImportcert)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_THUMBALGO, OnSelchangeComboThumbalgo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDlg message handlers

void CCertManagerDlg::OnClickTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	HTREEITEM hItem = NULL;//m_TreeServerCerts.GetSelectedItem();
	CertData *pData =NULL;// (CertData*)m_TreeServerCerts.GetItemData(hItem);

    UINT flags;
    POINT pt;
    ::GetCursorPos(&pt);
    CPoint point(pt);

    // Get the location of click.
    m_TreeServerCerts.ScreenToClient(&point);

    if ((hItem = m_TreeServerCerts.HitTest(point, &flags)) != NULL) 
	{
		pData = (CertData*)m_TreeServerCerts.GetItemData(hItem);
		m_IssuedBy = m_ValidFrom = m_ValidTo = m_Subject = "";
		if(pData)
		{
		m_IssuedBy = pData->m_Issuer;
		m_ValidFrom = pData->m_ValidFrom.Format("%A, %B %d, %Y %H:%M:%S" );
		m_ValidTo = pData->m_ValidTo.Format("%A, %B %d, %Y %H:%M:%S" );
		m_Subject = pData->m_Subject;
		}
		UpdateData(FALSE);
	}

*/
	*pResult = 0;
}

void CCertManagerDlg::OnSetfocusTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}
/*
void CCertManagerDlg::OnClickListEutrusted(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	m_TreeServerCerts.SelectItem(NULL);
	//m_EuTrustedList.SetSelectionMark(-1);
	m_UserCertList.SetSelectionMark(-1);

	CertData *pData =NULL;// (CertData*)m_TreeServerCerts.GetItemData(hItem);

    POINT pt;
    ::GetCursorPos(&pt);
    CPoint point(pt);
	int npos = -1;

    // Get the location of click.
    m_EuTrustedList.ScreenToClient(&point);
    UINT flags;

    if ((npos = m_EuTrustedList.HitTest(point, &flags)) != -1) 
	{
		pData = (CertData*)m_EuTrustedList.GetItemData(npos);
		m_IssuedBy = pData->m_Issuer;
		m_ValidFrom = pData->m_ValidFrom.Format("%A, %B %d, %Y %H:%M:%S" );
		m_ValidTo = pData->m_ValidTo.Format("%A, %B %d, %Y %H:%M:%S" );
		m_Subject = pData->m_Subject;
		UpdateData(FALSE);
	}



}
*/
/*
void CCertManagerDlg::OnClickListUsercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_TreeServerCerts.SelectItem(NULL);
	m_EuTrustedList.SetSelectionMark(-1);
//	m_UserCertList.SetSelectionMark(-1);
	CertData *pData =NULL;// (CertData*)m_TreeServerCerts.GetItemData(hItem);

    POINT pt;
    ::GetCursorPos(&pt);
    CPoint point(pt);
	int npos = -1;

    // Get the location of click.
    m_UserCertList.ScreenToClient(&point);
    UINT flags;

    if ((npos = m_UserCertList.HitTest(point, &flags)) != -1) 
	{
		pData = (CertData*)m_UserCertList.GetItemData(npos);
		m_IssuedBy = pData->m_Issuer;
		m_ValidFrom = pData->m_ValidFrom.Format("%A, %B %d, %Y %H:%M:%S" );
		m_ValidTo = pData->m_ValidTo.Format("%A, %B %d, %Y %H:%M:%S" );
		m_Subject = pData->m_Subject;
		UpdateData(FALSE);
	}

	*pResult = 0;
}
*/

int CCertManagerDlg::Run(const CString& persona, const CString& protocol, ConnectionInfo *pInfo)
{
	m_Persona = persona;
	m_Protocol = protocol;
	m_pConnectionInfo = pInfo;
	m_FilesToTrash.RemoveAll();//not really necessary.

	int nRet =  DoModal();
	
	POSITION pos = m_FilesToTrash.GetHeadPosition();
	while(pos)
	{
		CString csFileName = m_FilesToTrash.GetNext(pos);
		CFile::Remove(csFileName);
	}
	m_FilesToTrash.RemoveAll();
	return nRet;
}

bool CCertManagerDlg::CreateChainTree(ConnectionInfo *pConnectionInfo)
{
	if (m_htServerCerts && m_TreeServerCerts.ItemHasChildren(this->m_htServerCerts))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeServerCerts.GetChildItem(m_htServerCerts);
		while (hChildItem != NULL)
		{
			hNextItem = m_TreeServerCerts.GetNextItem(hChildItem, TVGN_NEXT);
			m_TreeServerCerts.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}

	POSITION pos = pConnectionInfo->m_CertDataList.GetHeadPosition();
	TVINSERTSTRUCT tvInsert;
	HTREEITEM htitem = NULL;
	HTREEITEM hParent = m_htServerCerts;

	while(pos != NULL)
		{
		CertData *pCertData = (CertData*)pConnectionInfo->m_CertDataList.GetNext(pos);
		ASSERT(pCertData);
		tvInsert.hParent = hParent;
		tvInsert.hInsertAfter = NULL;
		tvInsert.item.mask = TVIF_TEXT;
		tvInsert.item.pszText = (char*)LPCTSTR(pCertData->m_Subject);
		htitem = m_TreeServerCerts.InsertItem(&tvInsert);
		m_TreeServerCerts.SetItemData(htitem, (DWORD)pCertData);
		hParent = htitem;
		COleDateTime time = COleDateTime::GetCurrentTime();
		bool bExpired = !(pCertData->m_ValidFrom <= time) || !(pCertData->m_ValidTo >= time);
		if(pCertData->m_bTrusted && !bExpired)
			m_TreeServerCerts.SetItemImage(htitem, SC_TrustedAndNotExpired,SC_TrustedAndNotExpired );
		else if(pCertData->m_bTrusted && bExpired)
			m_TreeServerCerts.SetItemImage(htitem, SC_TrustedAndExpired, SC_TrustedAndExpired);
		else if(!pCertData->m_bTrusted && !bExpired)
			m_TreeServerCerts.SetItemImage(htitem, SC_NotTrustedAndNotExpired, SC_NotTrustedAndNotExpired);
		else if(!pCertData->m_bTrusted && bExpired)
			m_TreeServerCerts.SetItemImage(htitem, SC_NotTrustedAndExpired, SC_NotTrustedAndExpired);
		}
	m_TreeServerCerts.Expand(m_htServerCerts, TVE_EXPAND  );
	return true;
}

BOOL CCertManagerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_ImageList.Create(IDB_BITMAP_CERT_TREE, 16, 0,RGB(0,255,0));
	m_TreeServerCerts.SetImageList(&m_ImageList, TVSIL_NORMAL);
	this->m_htServerCerts = m_TreeServerCerts.InsertItem("Server Certificates",0);
	this->m_htUserCerts = m_TreeServerCerts.InsertItem("User Trusted Certificates", 0);
	this->m_htRootCerts = m_TreeServerCerts.InsertItem("Eudora Provided Trusted Certificates", 0);
	this->m_htClientCerts = m_TreeServerCerts.InsertItem("Client Certificates");

	CreateChainTree(m_pConnectionInfo);
	CreateUserList();
	CreateRootList();
	CreateClientChainTree(m_pConnectionInfo);
	m_TreeServerCerts.EnsureVisible(m_htServerCerts);
	HTREEITEM hFirstCert = m_TreeServerCerts.GetChildItem(m_htServerCerts);
	if(hFirstCert == NULL)
		hFirstCert = m_htServerCerts;
	m_TreeServerCerts.SelectItem(hFirstCert);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCertManagerDlg::OnButtonCertdetails() 
{
	 CertData *pData = GetSelectedCert();
	 if(pData == NULL)
		 return;

	 CString csTempFile;
	 CString csTempDir;
	 char path[200];

	 DWORD len = ::GetTempPath(199,path);
	 path[len] = 0;
	 csTempDir = path;
	 GetTempFileName( path, "EuCer", 0, path);
/* Passing a "1" instead of "0" will just create a temporary file name.
   Then we will not have to create two files for every cert thats displayed.
   So although this method would be more efficient, it causes minor problems when
   a user keeps clicking on the "view cert details" button very fast. This is because
   the "1" does not gaurantee a unique file name. So if the click is fast enough , 
   and the computer slow enough that even the first cert has not yet been displayed then
   there could be problems . So using this safer approach. All these temporary files
   will be cleaned up when you close cert manager. This does not cause any problems to
   any cert UI thats being displayed by OS.
*/
	 csTempFile = path;
	 csTempFile += ".crt";


	 if(pData && pData->m_CertBlob)
		{
		CFile file;
		file.Open(LPCTSTR(csTempFile), CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		file.Write(pData->m_CertBlob,pData->m_CertBlobLength);
		file.Close();
		}
	else
		csTempFile.Empty();;


	if(csTempFile.GetLength()>0)
	{

		::ShellExecute(this->m_hWnd,"open", LPCTSTR(csTempFile), NULL,LPCTSTR(csTempDir), SW_SHOWNORMAL );
		m_FilesToTrash.AddTail(csTempFile);
		m_FilesToTrash.AddTail(path);
	}
	
}

bool CCertManagerDlg::CreateRootList()
{
	if (m_htRootCerts && m_TreeServerCerts.ItemHasChildren(this->m_htRootCerts))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeServerCerts.GetChildItem(m_htRootCerts);
		while (hChildItem != NULL)
		{
			hNextItem = m_TreeServerCerts.GetNextItem(hChildItem, TVGN_NEXT);
			m_TreeServerCerts.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}

	FPNQCSSLGetRootCertList fnRootList =NULL;
	if(Network::GetQCSSLDll() == NULL)
	{
	return 0;
	}

	fnRootList = Network::GetQCSSLGetRootCertList();
	if(!fnRootList)
	{
	return 0;
	}

	CPtrList *pList = fnRootList();
	if(pList == NULL || pList->GetCount() <=0)
		return 0;

	POSITION pos = pList->GetHeadPosition();


	while(pos != NULL)
		{
		CertData *pCertData = (CertData*)pList->GetNext(pos);
		ASSERT(pCertData);
		HTREEITEM htitem = m_TreeServerCerts.InsertItem(pCertData->m_Subject,0,0,this->m_htRootCerts);
		if(htitem != NULL)
		m_TreeServerCerts.SetItemData(htitem, (DWORD) pCertData);
		COleDateTime time = COleDateTime::GetCurrentTime();
		bool bExpired = !(pCertData->m_ValidFrom <= time) || !(pCertData->m_ValidTo >= time);
		if(!bExpired)
			m_TreeServerCerts.SetItemImage(htitem, EC_TrustedAndNotExpired, EC_TrustedAndNotExpired);
		else
			m_TreeServerCerts.SetItemImage(htitem, EC_TrustedAndExpired, EC_TrustedAndExpired);

		}
		m_TreeServerCerts.Expand(m_htRootCerts, TVE_EXPAND  );

	return true;
}

bool CCertManagerDlg::CreateUserList()
{

	if (m_htUserCerts && m_TreeServerCerts.ItemHasChildren(m_htUserCerts))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeServerCerts.GetChildItem(m_htUserCerts);
		while (hChildItem != NULL)
		{
			hNextItem = m_TreeServerCerts.GetNextItem(hChildItem, TVGN_NEXT);
			m_TreeServerCerts.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}

	FPNQCSSLGetUserCertList fnUserList =NULL;
	if(Network::GetQCSSLDll() == NULL)
	{
	return 0;
	}

	fnUserList = Network::GetQCSSLGetUserCertList();
	if(!fnUserList)
	{
	return 0;
	}

	CPtrList *pList = fnUserList();
	if(pList == NULL || pList->GetCount() <=0)
		return 0;

	POSITION pos = pList->GetHeadPosition();

	while(pos != NULL)
		{
		CertData *pCertData = (CertData*)pList->GetNext(pos);
		ASSERT(pCertData);
		HTREEITEM htitem = m_TreeServerCerts.InsertItem(pCertData->m_Subject,0,0,this->m_htUserCerts);
		if(htitem != NULL)
		m_TreeServerCerts.SetItemData(htitem, (DWORD) pCertData);
		COleDateTime time = COleDateTime::GetCurrentTime();
		bool bExpired = !(pCertData->m_ValidFrom <= time) || !(pCertData->m_ValidTo >= time);
		if(!bExpired)
			m_TreeServerCerts.SetItemImage(htitem, UC_TrustedAndNotExpired, UC_TrustedAndNotExpired);
		else
			m_TreeServerCerts.SetItemImage(htitem, UC_TrustedAndExpired, UC_TrustedAndExpired);
		}
	m_TreeServerCerts.Expand(m_htUserCerts, TVE_EXPAND  );
	return true;

}

void CCertManagerDlg::OnButtonAddcert() 
{	
	HTREEITEM hItem = m_TreeServerCerts.GetSelectedItem();
	if(!this->IsServerCertItem(hItem))
		return;

	CertData *pData = GetSelectedCert();
	if(pData == NULL)
		return;
	 
	FPNQCSSLAddTrustedUserCert fnAddTrustedUserCert = NULL;

	if(Network::GetQCSSLDll() == NULL)
	{
	   return ;
	}

	fnAddTrustedUserCert = Network::GetQCSSLAddTrustedUserCert();
	if(!fnAddTrustedUserCert)
	{
		return ;
	}
	fnAddTrustedUserCert(pData, this->m_Persona,this->m_Protocol);
	CreateUserList();
	CreateChainTree(this->m_pConnectionInfo);
}

void CCertManagerDlg::OnButtonRemoveCert() 
{
	HTREEITEM hItem = m_TreeServerCerts.GetSelectedItem();
	if(!this->IsUserCertItem(hItem))
		return;

	CertData *pData = GetSelectedCert();
	if(pData == NULL)
		return;
	 
	FPNQCSSLDeleteTrustedUserCert fnDeleteTrustedUserCert = NULL;

	if(Network::GetQCSSLDll() == NULL)
	{
	   return ;
	}

	fnDeleteTrustedUserCert = Network::GetQCSSLDeleteTrustedUserCert();
	if(!fnDeleteTrustedUserCert)
	{
		return ;
	}
	fnDeleteTrustedUserCert(pData, this->m_Persona,this->m_Protocol);
	CreateUserList();
}

CertData* CCertManagerDlg::GetSelectedCert()
{
     HTREEITEM hItem = m_TreeServerCerts.GetSelectedItem();
	 CertData *pData = NULL;

	 if(hItem != NULL)
	 {
		pData = (CertData*)m_TreeServerCerts.GetItemData(hItem);
	 }
	return pData;
}

bool CCertManagerDlg::RefreshUserCertList()
{
	
	this->CreateUserList();

	return true;

}

void CCertManagerDlg::OnDblclkTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnButtonCertdetails();	
	*pResult = 0;
}

void CCertManagerDlg::OnKillfocusTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCertManagerDlg::OnRclickTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCertManagerDlg::OnReturnTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCertManagerDlg::OnItemexpandedTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	switch (pNMTreeView->action)
	{
	case TVE_COLLAPSE:
		if(hItem == this->m_htRootCerts || hItem == this->m_htUserCerts || hItem == this->m_htServerCerts|| hItem == this->m_htClientCerts )
			m_TreeServerCerts.SetItemImage(pNMTreeView->itemNew.hItem, SC_ClosedFolder,SC_ClosedFolder );
		else if(this->IsServerCertItem(hItem) || IsClientCertItem(hItem)) 
			{	
				CertData *pCertData =NULL;
				pCertData = (CertData*)m_TreeServerCerts.GetItemData(hItem);
				if(pCertData)
				{
				COleDateTime time = COleDateTime::GetCurrentTime();
				bool bExpired = !(pCertData->m_ValidFrom <= time) || !(pCertData->m_ValidTo >= time);
				if(pCertData->m_bTrusted && !bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_TrustedAndNotExpired,SC_TrustedAndNotExpired );
				else if(pCertData->m_bTrusted && bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_TrustedAndExpired, SC_TrustedAndExpired);
				else if(!pCertData->m_bTrusted && !bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_NotTrustedAndNotExpired, SC_NotTrustedAndNotExpired);
				else if(!pCertData->m_bTrusted && bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_NotTrustedAndExpired, SC_NotTrustedAndExpired);
				}
			}
		break;
	case TVE_EXPAND:
		if(hItem == this->m_htRootCerts || hItem == this->m_htUserCerts || hItem == this->m_htServerCerts|| hItem == this->m_htClientCerts)
			m_TreeServerCerts.SetItemImage(pNMTreeView->itemNew.hItem, SC_OpenFolder, SC_OpenFolder);
		else if(this->IsServerCertItem(hItem)|| IsClientCertItem(hItem)) 
			{
				CertData *pCertData =NULL;
				pCertData = (CertData*)m_TreeServerCerts.GetItemData(hItem);
				if(pCertData)
				{
				COleDateTime time = COleDateTime::GetCurrentTime();
				bool bExpired = !(pCertData->m_ValidFrom <= time) || !(pCertData->m_ValidTo >= time);
				if(pCertData->m_bTrusted && !bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_TrustedAndNotExpiredParent,SC_TrustedAndNotExpiredParent );
				else if(pCertData->m_bTrusted && bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_TrustedAndExpiredParent, SC_TrustedAndExpiredParent);
				else if(!pCertData->m_bTrusted && !bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_NotTrustedAndNotExpiredParent, SC_NotTrustedAndNotExpiredParent);
				else if(!pCertData->m_bTrusted && bExpired)
					m_TreeServerCerts.SetItemImage(hItem, SC_NotTrustedAndExpiredParent, SC_NotTrustedAndExpiredParent);
				}
			}

		break;
	default:
		ASSERT(0);
		break;
	}
	
	*pResult = 0;
}

void CCertManagerDlg::OnItemexpandingTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
/*	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	switch (pNMTreeView->action)
	{
	case TVE_COLLAPSE:
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_CLOSED_FOLDER_READONLY, IMAGE_CLOSED_FOLDER_READONLY);
		else
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_CLOSED_FOLDER, IMAGE_CLOSED_FOLDER);
		DoDeselectAllChildren(p_treeview->itemNew.hItem);
		EvaluateSelection();
		break;
	case TVE_EXPAND:
		if (p_nickfile->m_ReadOnly)
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_OPEN_FOLDER_READONLY, IMAGE_OPEN_FOLDER_READONLY);
		else
			SetItemImage(p_treeview->itemNew.hItem, IMAGE_OPEN_FOLDER, IMAGE_OPEN_FOLDER);
		break;
	default:
		ASSERT(0);
		break;
	}
	*/
	*pResult = 0;
}

void CCertManagerDlg::OnKeydownTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCertManagerDlg::OnSelchangedTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	CertData *pData =NULL;
	pData = (CertData*)m_TreeServerCerts.GetItemData(hItem);
	m_IssuedBy = m_ValidFrom = m_ValidTo = m_Subject = "";
	m_SerialNo = "";
	m_AlgoList.ResetContent();
	m_AlgoList.EnableWindow(FALSE);
	this->m_ThumbPrint = "";
	if(pData)
	{
		m_IssuedBy = pData->m_Issuer;
		m_ValidFrom = pData->m_ValidFrom.Format("%A, %B %d, %Y %H:%M:%S" );
		m_ValidTo = pData->m_ValidTo.Format("%A, %B %d, %Y %H:%M:%S" );
		m_Subject = pData->m_Subject;
		m_SerialNo = MakeReadable(pData->m_SerialNumber);
		CreateHashAlgList();	
	}

	UpdateData(FALSE);
	m_BtnAddCert.EnableWindow(IsServerCertItem(hItem));
	m_BtnCertDetails.EnableWindow(pData != NULL);
	m_BtnRemoveCert.EnableWindow(IsUserCertItem(hItem));
	m_BtnExportCert.EnableWindow(pData != NULL);
	
	*pResult = 0;
}

void CCertManagerDlg::OnSetdispinfoTreeServercerts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCertManagerDlg::OnTimeChange() 
{
	
	CDialog::OnTimeChange();
}

void CCertManagerDlg::OnOK() 
{
	m_ImageList.DeleteImageList();	
	CDialog::OnOK();
}

bool CCertManagerDlg::IsUserCertItem(HTREEITEM &hItem)
{
	HTREEITEM parentItem = 	m_TreeServerCerts.GetParentItem(hItem);
	HTREEITEM currentItem = hItem;
	bool bIsUserCert = false;
	while(parentItem != NULL)
	{
		if(parentItem == this->m_htUserCerts)
		{
		bIsUserCert = true;
		break;
		}
		currentItem = parentItem;
		parentItem = m_TreeServerCerts.GetParentItem(currentItem);
	}
	return bIsUserCert;
}

bool CCertManagerDlg::IsServerCertItem(HTREEITEM &hItem)
{
	HTREEITEM parentItem = 	m_TreeServerCerts.GetParentItem(hItem);
	HTREEITEM currentItem = hItem;
	bool bIsServerCert = false;
	while(parentItem != NULL)
	{
		if(parentItem == this->m_htServerCerts)
		{
		bIsServerCert = true;
		break;
		}
		currentItem = parentItem;
		parentItem = m_TreeServerCerts.GetParentItem(currentItem);
	}
	return bIsServerCert;

}

bool CCertManagerDlg::IsEuCertItem(HTREEITEM &hItem)
{
	HTREEITEM parentItem = 	m_TreeServerCerts.GetParentItem(hItem);
	HTREEITEM currentItem = hItem;
	bool bIsEuCert = false;
	while(parentItem != NULL)
	{
		if(parentItem == this->m_htRootCerts)
		{
		bIsEuCert = true;
		break;
		}
		currentItem = parentItem;
		parentItem = m_TreeServerCerts.GetParentItem(currentItem);
	}
	return bIsEuCert;

}
bool CCertManagerDlg::IsClientCertItem(HTREEITEM &hItem)
{
	HTREEITEM parentItem = 	m_TreeServerCerts.GetParentItem(hItem);
	HTREEITEM currentItem = hItem;
	bool bIsClientCert = false;
	while(parentItem != NULL)
	{
		if(parentItem == this->m_htClientCerts)
		{
		bIsClientCert = true;
		break;
		}
		currentItem = parentItem;
		parentItem = m_TreeServerCerts.GetParentItem(currentItem);
	}
	return bIsClientCert;
}

bool	CCertManagerDlg::CreateClientChainTree(ConnectionInfo *pConnectionInfo)
{
	if (m_htClientCerts && m_TreeServerCerts.ItemHasChildren(this->m_htClientCerts))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeServerCerts.GetChildItem(m_htClientCerts);
		while (hChildItem != NULL)
		{
			hNextItem = m_TreeServerCerts.GetNextItem(hChildItem, TVGN_NEXT);
			m_TreeServerCerts.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}

	POSITION pos = pConnectionInfo->m_ClientCertList.GetHeadPosition();
	TVINSERTSTRUCT tvInsert;
	HTREEITEM htitem = NULL;
	HTREEITEM hParent = m_htClientCerts;

	while(pos != NULL)
		{
		CertData *pCertData = (CertData*)pConnectionInfo->m_ClientCertList.GetNext(pos);
		ASSERT(pCertData);
		tvInsert.hParent = hParent;
		tvInsert.hInsertAfter = NULL;
		tvInsert.item.mask = TVIF_TEXT;
		tvInsert.item.pszText = (char*)LPCTSTR(pCertData->m_Subject);
		htitem = m_TreeServerCerts.InsertItem(&tvInsert);
		m_TreeServerCerts.SetItemData(htitem, (DWORD)pCertData);
		hParent = htitem;
		COleDateTime time = COleDateTime::GetCurrentTime();
		bool bExpired = !(pCertData->m_ValidFrom <= time) || !(pCertData->m_ValidTo >= time);
		if( !bExpired)
			m_TreeServerCerts.SetItemImage(htitem, SC_TrustedAndNotExpired,SC_TrustedAndNotExpired );
		else if(bExpired)
			m_TreeServerCerts.SetItemImage(htitem, SC_TrustedAndExpired, SC_TrustedAndExpired);
		}
	m_TreeServerCerts.Expand(m_htClientCerts, TVE_EXPAND  );
	return true;

}

void CCertManagerDlg::OnButtonExportcert() 
{
	CertData *pData = GetSelectedCert();
	if((pData == NULL) || (pData->m_CertBlob == NULL))
		 return;

	CFileDialog theDlg(	FALSE, "", NULL, 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
					CRString(IDS_CERT_FILE_FILTER), NULL );
	if ( theDlg.DoModal() == IDOK )
	{
		CFile file;
		file.Open(LPCTSTR(theDlg.GetPathName()), CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		file.Write(pData->m_CertBlob,pData->m_CertBlobLength);
		file.Close();
	}
}

void CCertManagerDlg::OnButtonImportcert() 
{
	CFileDialog theDlg(	TRUE, "", NULL, 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
					CRString(IDS_CERT_FILE_FILTER), NULL );
	if ( theDlg.DoModal() == IDOK )
	{	
		if(Network::GetQCSSLDll() == NULL)
		{
			return ;
		}


		FPNQCSSLAddTrustedCertFromFile fpnAdd = Network::GetQCSSLAddTrustedCertFromFile();
		if(!fpnAdd)
			return;
		fpnAdd(theDlg.GetPathName());
		CreateUserList();
		CreateChainTree(this->m_pConnectionInfo);
	}
}

void CCertManagerDlg::OnSelchangeComboThumbalgo() 
{
	int index = m_AlgoList.GetCurSel();
	Hash *pHash = (Hash*) m_AlgoList.GetItemDataPtr(index);
	m_ThumbPrint = MakeReadable(pHash->m_HashData);
	UpdateData(FALSE);
	CEdit *pEdit = (CEdit*)this->GetDlgItem(IDC_EDIT_THUMBPRINT);
	pEdit->FmtLines(TRUE);
	m_ThumbPrint = MakeReadable(pHash->m_HashData);
	pEdit->GetWindowText(m_ThumbPrint);
	UpdateData(FALSE);
}

int CCertManagerDlg::CreateHashAlgList()
{
	m_AlgoList.ResetContent();
	m_AlgoList.EnableWindow(FALSE);
	CertData *pData = GetSelectedCert();
	if(pData == NULL)
		return 0;
	m_AlgoList.EnableWindow(TRUE);
	int index =0;
	POSITION pos = pData->m_HashList.GetHeadPosition();
	while(pos != NULL)
		{
		Hash *pHash = (Hash*)pData->m_HashList.GetNext(pos);
		if(pHash->m_HashData.GetLength()>0)
			{
			index = m_AlgoList.AddString(pHash->m_AlgName);
			m_AlgoList.SetItemDataPtr(index, (void*)pHash);
			}
		}
	m_AlgoList.SetCurSel(index);
	OnSelchangeComboThumbalgo();
	return 0;
}
