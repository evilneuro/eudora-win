/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#ifndef __ABOUTDLG_H__
#define __ABOUTDLG_H__

#include "swres.h"

class CAboutDlg : public CDialog
{
public:
    CAboutDlg(CWnd* pParentWnd = NULL);

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    
    // Generated message map functions
    //{{AFX_MSG(CAboutDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // __ABOUTDLG_H__
