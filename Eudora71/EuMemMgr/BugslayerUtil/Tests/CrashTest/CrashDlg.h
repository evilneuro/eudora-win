/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#pragma once
#include "afxwin.h"


// CCrashDlg dialog

class CCrashDlg : public CDialog
{
    DECLARE_DYNAMIC(CCrashDlg)

public:
    CCrashDlg(CWnd* pParent = NULL);
    virtual ~CCrashDlg();

    // Dialog Data
    enum { IDD = IDD_CRASHDLG };

    // Set's the exception pointers.
    void SetExceptionPointers ( EXCEPTION_POINTERS * pPtrs )
    {
        m_pExptPtrs = pPtrs ;
    }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    virtual BOOL OnInitDialog ( ) ;
    DECLARE_MESSAGE_MAP()


protected       :
    // The exception pointers.
    EXCEPTION_POINTERS * m_pExptPtrs ;


public:


    CStatic m_cFaultReason;
    CStatic m_cRegisters;
    CListBox m_cCallStackList;
    CButton m_cMiniDumpBtn;
    afx_msg void OnBnClickedMinidump();
};
