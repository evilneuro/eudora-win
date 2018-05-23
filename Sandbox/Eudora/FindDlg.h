// FindDlg.h: interface for the CFindDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FINDDLG_H__DD46359F_8DF8_11D2_9518_00805F9BF4D7__INCLUDED_)
#define AFX_FINDDLG_H__DD46359F_8DF8_11D2_9518_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

#include <afxdlgs.h> // CFindReplaceDialog

// --------------------------------------------------------------------------

class CFindDlg : public CFindReplaceDialog
{
// Construction
public:
// Dialog Data
	//{{AFX_DATA(CFindDlg)
		// NOTE: the ClassWizard will add data members here
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CFindDlg)
        protected:
    virtual BOOL PreTranslateMessage( MSG* pMsg );
        virtual void DoDataExchange(CDataExchange* pDX);
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CFindDlg)
        afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_FINDDLG_H__DD46359F_8DF8_11D2_9518_00805F9BF4D7__INCLUDED_)
