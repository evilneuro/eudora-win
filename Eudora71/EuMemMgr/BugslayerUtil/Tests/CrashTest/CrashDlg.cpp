/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "CrashTest.h"
#include "CrashDlg.h"
#include ".\crashdlg.h"


// CCrashDlg dialog

IMPLEMENT_DYNAMIC(CCrashDlg, CDialog)
CCrashDlg::CCrashDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCrashDlg::IDD, pParent)
{
}

CCrashDlg::~CCrashDlg()
{
}

void CCrashDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FAULTREASON, m_cFaultReason);
    DDX_Control(pDX, IDC_REGISTERS, m_cRegisters);
    DDX_Control(pDX, IDC_CALLSTACK, m_cCallStackList);
    DDX_Control(pDX, IDC_MINIDUMP, m_cMiniDumpBtn);
}


BEGIN_MESSAGE_MAP(CCrashDlg, CDialog)
    ON_BN_CLICKED(IDC_MINIDUMP, OnBnClickedMinidump)
END_MESSAGE_MAP()


// CCrashDlg message handlers
BOOL CCrashDlg :: OnInitDialog ( )
{
    CDialog::OnInitDialog ( ) ;
    
    ASSERT ( NULL != m_pExptPtrs ) ;
    
    LPCTSTR szStr = GetFaultReason ( m_pExptPtrs ) ;
    
    m_cFaultReason.SetWindowText ( szStr ) ;
    
    szStr = GetRegisterString ( m_pExptPtrs ) ;
    
    m_cRegisters.SetWindowText ( szStr ) ;
    
    szStr = GetFirstStackTraceString ( GSTSO_MODULE     |
                                        GSTSO_SYMBOL    |
                                        GSTSO_SRCLINE    ,
                                       m_pExptPtrs        ) ;
    while ( NULL != szStr )
    {
        m_cCallStackList.AddString ( szStr ) ;
        szStr = GetNextStackTraceString ( GSTSO_MODULE     |
                                            GSTSO_SYMBOL   |
                                            GSTSO_SRCLINE   ,
                                          m_pExptPtrs        ) ;
    }
    
    if ( FALSE == IsMiniDumpFunctionAvailable ( ) )
    {
        m_cMiniDumpBtn.EnableWindow ( FALSE ) ;
    }
    return ( FALSE ) ;
}

void CCrashDlg::OnBnClickedMinidump()
{
    BSUMDRET eRet =
            CreateCurrentProcessCrashDump ( MiniDumpWithHandleData ,
                                            _T ( "CrashTest.DMP" ) ,
                                            GetCurrentThreadId ( ) ,
                                            m_pExptPtrs              ) ;
    ASSERT ( eDUMP_SUCCEEDED == eRet ) ;
    if ( eDUMP_SUCCEEDED != eRet )
    {
        CString cPrompt ;
        cPrompt.Format ( _T ( "Dump failed : %d" ) , eRet ) ;
        AfxMessageBox ( cPrompt ) ;
    }
}

