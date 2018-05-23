//////////////////////////////////////////////////////////////////////////////
// DMMDumpLogDlgImpl.cpp
// 
//
// Created: 11/18/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define __DMM_DUMP_LOG_INTERFACE_IMPLEMENTATION_

#include "DMMDumpLogDlgImpl.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(DMMDumpLogDialog, CDialog)

BEGIN_MESSAGE_MAP(DMMDumpLogDialog, CDialog)
END_MESSAGE_MAP()

BOOL
DMMDumpLogDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    return(true);
}

void
DMMDumpLogDialog::OnCancel()
{
    EndDialog(IDCANCEL);
}

void
DMMDumpLogDialog::OnOK()
{
    //AutoSerializedRegionMT<MemMgrSerializedRegionMT> aco;
    
	m_EB_Label.GetWindowText(m_strLabel);
    m_EB_File.GetWindowText(m_strFile);
	
	char szSize[64] = {0};
	m_EB_Size.GetWindowText(szSize, 63);
	if(strlen(szSize) > 0)
		m_Size = atoi(szSize);

    //DumpBlocksLogByLabelAndFileMT(static_cast<void *>(&bo), pDBLCB, 0,
	//			((strlen(label) > 0) ? label : 0),
	//			((strlen(file) > 0)  ? file : 0));
    //_heapmin();
    CDialog::OnOK();
}

void
DMMDumpLogDialog::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DMM_EB_QUERY_DUMP_BY_LABEL, m_EB_Label);
    DDX_Control(pDX, IDC_DMM_EB_QUERY_DUMP_BY_FILE, m_EB_File);
	DDX_Control(pDX, IDC_DMM_EB_QUERY_DUMP_BY_SIZE, m_EB_Size);
}

#endif // _DEBUG

