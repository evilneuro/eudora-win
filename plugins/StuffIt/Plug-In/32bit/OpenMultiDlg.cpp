// OpenMultiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "attach.h"
#include "OpenMultiDlg.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ---------------------------------------------------------------------------

#define nTemplateID    (IDD_OPENMULTITEMPLATE_DLG)
#define nTitleID       (IDS_OPENMULTITITLE_STR)

const DWORD dwFlags = OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_SHAREAWARE | OFN_FILEMUSTEXIST | OFN_EXPLORER;
const LPCTSTR lpszFilter = "All Files (*.*)|*.*||";
const LPCTSTR pTitle = "Attach Multiple Files";

// ---------------------------------------------------------------------------

UINT APIENTRY OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (uiMsg)
	{
		case WM_INITDIALOG:
		{
			i = 1;
		}
		break;

		case WM_NOTIFY:
		{
			OFNOTIFY *pNotify = (OFNOTIFY *) lParam;
			UINT nCode = (pNotify->hdr).code;

			switch (nCode)
			{
				case CDN_INITDONE:
				{
					HWND pParentWnd = GetParent(hdlg);
					SetDlgItemText(pParentWnd, IDOK, "&Add");

					CWnd window;
					window.Attach(hdlg);

					CComboBox *pCompCombo = (CComboBox *) window.GetDlgItem(IDC_COMPRESSION_COMBO);

					pCompCombo->ResetContent();
					pCompCombo->InsertString(-1, "(none)");
					pCompCombo->InsertString(-1, "Zip");
					pCompCombo->InsertString(-1, "Stuffit");
					pCompCombo->InsertString(-1, "ARC");
					pCompCombo->SetCurSel(0);

					window.Detach();
				}
				break;

				case CDN_FILEOK:
				{
					i = 4;
				}
				break;
			}
		}
		break;
	}

	return (0);
}

/////////////////////////////////////////////////////////////////////////////
// COpenMultiDlg

IMPLEMENT_DYNAMIC(COpenMultiDlg, CFileDialog)

COpenMultiDlg::COpenMultiDlg() :
	CFileDialog(TRUE, NULL, NULL, dwFlags, lpszFilter, NULL)
{
}


BEGIN_MESSAGE_MAP(COpenMultiDlg, CFileDialog)
	//{{AFX_MSG_MAP(COpenMultiDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

