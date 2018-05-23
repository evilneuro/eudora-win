#if !defined(AFX_OPENMULTIDLG_H__30C82352_E2A7_11D0_83D0_00805FD26222__INCLUDED_)
#define AFX_OPENMULTIDLG_H__30C82352_E2A7_11D0_83D0_00805FD26222__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OpenMultiDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpenMultiDlg dialog

class COpenMultiDlg : public CFileDialog
{
	DECLARE_DYNAMIC(COpenMultiDlg)

public:
	COpenMultiDlg();

protected:
	//{{AFX_MSG(COpenMultiDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENMULTIDLG_H__30C82352_E2A7_11D0_83D0_00805FD26222__INCLUDED_)
