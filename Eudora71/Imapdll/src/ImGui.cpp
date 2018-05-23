// ImGui.CPP
//
// Various interface utilities
//
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


// #include <ctl3d.h>

#include <io.h>

#include "resource.h"
// #include "netstream.h"
// #include "imapmail.h"
#include "imGui.h"

// Include the files in the correct order to allow leak checking with malloc.
// CRTDBG_MAP_ALLOC already defined in stdafx.h
#include <stdlib.h>
#include <crtdbg.h>

#include "DebugNewHelpers.h"


// Alert Dialog extension
static BOOL g_bCheckForAutoOk = FALSE;
void CheckForAutoOk( BOOL bOn ) { g_bCheckForAutoOk = bOn; }


/////////////////////////////////////////////////////////////////////////////
// CAlertDialog dialog

class CAlertDialog : public CDialog
{
// Construction
public:
//	CAlertDialog(UINT DialogID, const char* Message, CWnd* pParent = NULL);

	CAlertDialog();

// Dialog Data
	//{{AFX_DATA(CAlertDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	const char* m_Message;
	va_list m_Args;

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CAlertDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};
/////////////////////////////////////////////////////////////////////////////
// CAlertDialog dialog


//CAlertDialog::CAlertDialog(UINT DialogID, const char* Message, CWnd* pParent /*=NULL*/)
//	: CDialog(DialogID, pParent)
CAlertDialog::CAlertDialog()
{
	//{{AFX_DATA_INIT(CAlertDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//m_Message = Message;

	m_Message = NULL;
}


BOOL CAlertDialog::OnInitDialog()
{
	char buf[1024];
	
#ifdef WIN32    
//	Ctl3dSubclassDlgEx(m_hWnd, CTL3D_ALL);
#endif
	
	CWnd* Control = GetDlgItem(IDC_ALERT_TEXT);
	*buf = 0;
	if (Control)
	{
		if (!m_Message)
		{
			CString Format;
			Control->GetWindowText(Format);
			_vsnprintf(buf, sizeof(buf) - 1, Format, m_Args);
		}
		Control->SetWindowText(m_Message? m_Message : buf);
	}
	va_end(m_Args);

	ASSERT(m_Message || *buf);
	
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CAlertDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlertDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAlertDialog, CDialog)
	//{{AFX_MSG_MAP(CAlertDialog)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlertDialog message handlers

BOOL CAlertDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// If we got a button press, then end the dialog
	if ( GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED && GET_WM_COMMAND_ID(wParam, lParam) >= 1000)
	{
		EndDialog(GET_WM_COMMAND_ID(wParam, lParam));
		return (TRUE);
	}

	return (CDialog::OnCommand(wParam, lParam));
}


void CAlertDialog::OnDestroy()
{
	CDialog::OnDestroy();
}


// ================ External interface ===============//

int AlertDialog(UINT /*DialogID*/, ...)
{
#if 0 // JOK

	HRSRC			hResInfo;
	HGLOBAL			hDlg;
	LPCDLGTEMPLATE  pDlgTemplate = NULL;
	HINSTANCE		hInst = (HINSTANCE)g_hModule;


	// Load the dialog template resource from the DLL's resources.
	if ((hResInfo = ::FindResource(hInst, (LPCSTR)DialogID, RT_DIALOG)) &&
		(hDlg = ::LoadResource(hInst, hResInfo)))
	{
		pDlgTemplate = (LPCDLGTEMPLATE) ::LockResource(hDlg);
		if (pDlgTemplate)
		{
			DWORD nResLen = ::SizeofResource(hInst, hResInfo);

			LPCDLGTEMPLATE pMemDlgTemplate = (LPCDLGTEMPLATE) malloc ( nResLen );
			if (pMemDlgTemplate)
				memcpy ( (void *) pMemDlgTemplate, (void *)pDlgTemplate, nResLen);

			// Can now free the original resource.
			::UnlockResource(hDlg);

			if (pMemDlgTemplate)
			{
				// Call the dialogs simple constructor.
				CAlertDialog dlg;

				if ( dlg.InitModalIndirect (pMemDlgTemplate, NULL) )
				{
					va_start(dlg.m_Args, DialogID);

					return dlg.DoModal();
				}
			}
		}
	}

	return IDCANCEL;

#endif 

#if 0 
	// Do this so we load our own resource.
//	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	// Call the dialogs simple constructor.
	CAlertDialog dlg(DialogID, NULL);
	
	va_start(dlg.m_Args, DialogID);

	return dlg.DoModal();
#endif

	return IDCANCEL;

}

#endif // IMAP4
