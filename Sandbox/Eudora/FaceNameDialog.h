#if !defined(AFX_FACENAMEDIALOG_H__949F9BC1_4689_11D1_A7BC_0060972F7459__INCLUDED_)
#define AFX_FACENAMEDIALOG_H__949F9BC1_4689_11D1_A7BC_0060972F7459__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FaceNameDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFaceNameDialog dialog

class CFaceNameDialog : public CFontDialog
{
	DECLARE_DYNAMIC(CFaceNameDialog)

public:
	CFaceNameDialog(LPLOGFONT lplfInitial = NULL,
		DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
#ifndef _AFX_NO_RICHEDIT_SUPPORT
	CFaceNameDialog(const CHARFORMAT& charformat,
		DWORD dwFlags = CF_SCREENFONTS,
		CDC* pdcPrinter = NULL,
		CWnd* pParentWnd = NULL);
#endif

protected:
	//{{AFX_MSG(CFaceNameDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FACENAMEDIALOG_H__949F9BC1_4689_11D1_A7BC_0060972F7459__INCLUDED_)
