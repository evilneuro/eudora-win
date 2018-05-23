// PropPageGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "qvoice32.h"
#include "PropPageGeneral.h"

#include "DebugNewHelpers.h"

//////////////////////////////////////////////////////////////////////////////
// CDirDialog from Eudora modified by lsong
// 05.21.97

// Needed to get the control IDs for the the controls to hide
#include <dlgs.h>

class CDirDialog : public CFileDialog
{
// Construction
public:
	CDirDialog(UINT nID, CWnd* pParentWnd);

// Attributes
public:
		
// Operations
public:
	BOOL OnInitDialog();

// Implementation
protected:
	CString m_Title;
	CString m_StartDir;
	CString m_Filter;
};

CDirDialog::CDirDialog(UINT nID, CWnd* pParentWnd) :
	CFileDialog(TRUE, NULL, "A", OFN_HIDEREADONLY, NULL, pParentWnd)
{
#ifdef WIN32
	m_ofn.Flags &= ~ ( OFN_SHOWHELP | OFN_EXPLORER );
#else
	m_ofn.Flags &= ~OFN_SHOWHELP;
#endif
	
	m_Title = _T("Select a directory");//.LoadString(IDS_FILE_DIR_TITLE);
	m_ofn.lpstrTitle = m_Title;
	
	CWnd* Ctrl = pParentWnd->GetDlgItem(nID);
	if (Ctrl)
		Ctrl->GetWindowText(m_StartDir);
//	if (m_StartDir.IsEmpty())
//		m_StartDir = EudoraDir;
	m_ofn.lpstrInitialDir = m_StartDir;
	
//	m_Filter.LoadString(IDS_ALL_FILES_FILTER);
	// "All Files (*.*)"
	m_Filter += _T("All Files (*.*)|*.*|");

	for (char* s = m_Filter.GetBuffer(1); *s; s++)
	{
		if (*s == '|')
			*s = 0;
	}
	m_ofn.lpstrFilter = m_Filter;
}

BOOL CDirDialog::OnInitDialog()
{
#ifdef WIN32    
  //  Ctl3dSubclassDlgEx(m_hWnd, CTL3D_ALL);
#endif
	if (!CFileDialog::OnInitDialog())
		return (FALSE);
		
	CWnd* FileEdit = GetDlgItem(edt1);
	if (FileEdit)
		FileEdit->ShowWindow(SW_HIDE);
	SetDlgItemText(IDOK, "Use Directory" );
	
	return (TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CPropPageGeneral property page

IMPLEMENT_DYNCREATE(CPropPageGeneral, CPropertyPage)

CPropPageGeneral::CPropPageGeneral() : CPropertyPage(CPropPageGeneral::IDD)
{
	m_strOutboxDir.Empty();
	//{{AFX_DATA_INIT(CPropPageGeneral)
	mb_ImmediateRecord = FALSE;
	mb_AttachReadMe = FALSE;
	//}}AFX_DATA_INIT
}

CPropPageGeneral::~CPropPageGeneral()
{
}

void CPropPageGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageGeneral)
	DDX_Check(pDX, IDC_RECORD, mb_ImmediateRecord);
	DDX_Check(pDX, IDC_ATTACH_README, mb_AttachReadMe);
	//}}AFX_DATA_MAP
	SetDlgItemText( IDC_OUTBOX_DIR,	m_strOutboxDir );
}

BEGIN_MESSAGE_MAP(CPropPageGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageGeneral)
	ON_BN_CLICKED(IDC_OUTBOX_DIR, OnOutboxDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageGeneral message handlers

void CPropPageGeneral::OnOutboxDir() 
{
	CDirDialog dlg( IDC_OUTBOX_DIR, this );
	// We want to set only the path, not including the file name
	if (dlg.DoModal() == IDOK)
	 {
		m_strOutboxDir = dlg.GetPathName();
		SetDlgItemText( IDC_OUTBOX_DIR,
			m_strOutboxDir.GetBufferSetLength(dlg.m_ofn.nFileOffset - 1) );
	 }
}

