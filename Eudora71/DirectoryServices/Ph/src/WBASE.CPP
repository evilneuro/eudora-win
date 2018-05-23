//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Name: 					WBASE.CPP
// Date:					8/1/97
// Description:		Application framework base classes
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma warning(disable : 4514)
#include <afx.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include "DebugNewHelpers.h"

#include "wbase.h"
#include "QCUtils.h"


//////////////////////////////////////////////////////////////////////////////
// CPhWnd methods


CPhWnd::CPhWnd()
{
	m_hWnd					= NULL;
	m_hInstance			= NULL;
	m_lpszClassName = NULL;
}

CPhWnd::~CPhWnd()
{
	delete [] m_lpszClassName;
}

// registers this objects window class
BOOL 	CPhWnd::Register(HINSTANCE hInstance, LPSTR lpszClassName, UINT style/*=0*/, 
										 LPSTR lpszMenuName/*=NULL*/, HICON hIcon/*=NULL*/, HCURSOR hCursor/*=LoadCursor(NULL, IDC_ARROW)*/, 
										 HBRUSH hbrBackground/*=(HBRUSH)(COLOR_WINDOW+1)*/)
{
 WNDCLASS	wc = {0};
 
 wc.style 				= style;
 wc.lpfnWndProc 	= (WNDPROC)CPhWnd::ExportedWndProc;
 wc.cbClsExtra 		= 0;
 wc.cbWndExtra 		= sizeof(CPhWnd*);
 wc.hInstance 		= m_hInstance = hInstance;
 wc.hIcon 				= hIcon;
 wc.hCursor 			= hCursor;
 wc.hbrBackground = hbrBackground;
 wc.lpszMenuName 	= lpszMenuName;

 wc.lpszClassName = m_lpszClassName = SafeStrdupMT(lpszClassName);

 return (0 != RegisterClass(&wc));
}

// unregisters this objects window class
void CPhWnd::Unregister()
{
	UnregisterClass(m_lpszClassName, m_hInstance);
}


BOOL CPhWnd::Create(LPCSTR lpszWindowName, DWORD dwStyle, 
									const RECT& rect, HWND hwndParent/*=NULL*/, HMENU hMenu/*=NULL*/)
{
  return (NULL != CreateWindow(m_lpszClassName, lpszWindowName, dwStyle, 
															 rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
															 hwndParent, hMenu, m_hInstance, this));
}


LRESULT EXPFUNC CPhWnd::ExportedWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CPhWnd *pWnd = (CPhWnd*)GetWindowLong(hwnd, 0);

	if (uMsg == WM_NCCREATE) {
		if (!pWnd){
			pWnd = (CPhWnd*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			SetWindowLong(hwnd, 0, (LONG)pWnd);
			pWnd->m_hWnd = hwnd;
		}
	}
	else
	if (uMsg == WM_NCDESTROY) {
		SetWindowLong(hwnd, 0, NULL);
		return 0;
	}

	return (pWnd) ? pWnd->WndProc(uMsg, wParam, lParam) 
								: DefWindowProc(hwnd, uMsg, wParam, lParam);
}



//////////////////////////////////////////////////////////////////////////////
// CPhDialog methods

void CPhDialog::CommonConstruct(HINSTANCE hInstance/*=NULL*/)
{
	m_hInstance 				= hInstance;
	m_hDlg 							= NULL;
	m_hwndParent				= NULL;
	m_lpszTemplateName 	= NULL;
	m_nIDTemplate				= 0;
	m_bModal						= 0;
}

CPhDialog::CPhDialog()
{
	CommonConstruct();
}
	
CPhDialog::CPhDialog(HINSTANCE hInstance, LPCTSTR lpszTemplateName, HWND hwndParent/*=NULL*/)
{
	CommonConstruct(hInstance);

	m_lpszTemplateName = SafeStrdupMT(lpszTemplateName);

	m_hwndParent = hwndParent;
}


CPhDialog::CPhDialog(HINSTANCE hInstance, UINT nIDTemplate, HWND hwndParent/*=NULL*/)
{
	CommonConstruct(hInstance);
	m_nIDTemplate	= nIDTemplate;
	m_hwndParent = hwndParent;
}
  
	
CPhDialog::~CPhDialog()
{
	delete [] m_lpszTemplateName;
}


BOOL CPhDialog::Create(HINSTANCE hInstance, LPCTSTR lpszTemplateName, HWND hwndParent/*=NULL*/)
{
	m_hwndParent = hwndParent;

	m_lpszTemplateName = SafeStrdupMT(lpszTemplateName);

	m_hInstance = hInstance;
	return (NULL != CreateDialogParam(m_hInstance, m_lpszTemplateName, m_hwndParent, (DLGPROC)ExportedDialogProc, (LONG)this));
}

BOOL CPhDialog::Create(HINSTANCE hInstance, UINT nIDTemplate, HWND hwndParent/*=NULL*/)
{
	m_hwndParent = hwndParent;
	m_hInstance = hInstance;
	m_nIDTemplate = nIDTemplate;
	return (NULL != CreateDialogParam(m_hInstance, MAKEINTRESOURCE(nIDTemplate), m_hwndParent, (DLGPROC)ExportedDialogProc, (LONG)this));
}


int CPhDialog::DoModal()
{
	m_bModal = 1;
	return DialogBoxParam(m_hInstance, 
												m_lpszTemplateName ? m_lpszTemplateName : MAKEINTRESOURCE(m_nIDTemplate), 
												m_hwndParent, (DLGPROC)ExportedDialogProc, (LONG)this);
}

void CPhDialog::EndDialog(int nResult)
{
	if (m_bModal)
		::EndDialog(m_hDlg, nResult);
	else
		DestroyWindow(m_hDlg);
}

// static exported dialog proc
BOOL CALLBACK CPhDialog::ExportedDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CPhDialog *pDlg = NULL;

	if (uMsg == WM_INITDIALOG) {
		SetWindowLong(hwndDlg, DWL_USER, lParam);
		pDlg = (CPhDialog*)lParam;
		pDlg->m_hDlg = hwndDlg;
	}
		
	pDlg = (CPhDialog*)GetWindowLong(hwndDlg, DWL_USER);
		
	return (pDlg) ? pDlg->DlgProc(uMsg, wParam, lParam) : FALSE;
}


