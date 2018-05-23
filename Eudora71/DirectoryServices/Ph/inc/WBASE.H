//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Name: 					WBASE.H
// Date:					8/1/97
// Description:		Windows C++ base classes
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef __WINBASE_H__
#define __WINBASE_H__


#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif

#ifndef EXPFUNC
#define EXPFUNC EXPORT CALLBACK
#endif

// for readablity
#define constructor
#define destructor

// forward declarations
class CPhWnd;
class CPhDialog;

//////////////////////////////////////////////////////////////////////////////
// CPhWnd class

class CPhWnd
{
public:
// Constructors and other creation
	constructor				CPhWnd();
	destructor				~CPhWnd();
  
// Attributes
	LPSTR							m_lpszClassName;
	HINSTANCE					m_hInstance;
	HWND 							m_hWnd;

public:
	BOOL 							Register(HINSTANCE hInstance, LPSTR lpszClassName, UINT style=0, 
										 LPSTR lpszMenuName=NULL, HICON hIcon=NULL, 
										 HCURSOR hCursor=LoadCursor(NULL, IDC_ARROW), 
										 HBRUSH hbrBackground=(HBRUSH)(COLOR_WINDOW+1));


	void 							Unregister();

	BOOL 							Create(LPCSTR lpszWindowName, DWORD dwStyle, 
														const RECT& rect, HWND hwndParent=NULL, HMENU hMenu=NULL);
static 
	LRESULT EXPFUNC 	ExportedWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
virtual
	LRESULT						WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

};


//////////////////////////////////////////////////////////////////////////////
// CPhDialog class

class CPhDialog
{
private:
	void							CommonConstruct(HINSTANCE hInstance=NULL);
public:
// Constructors and other creation
	constructor				CPhDialog();
	constructor				CPhDialog(HINSTANCE hInstance, LPCTSTR lpszTemplateName, HWND hwndParent=NULL);
	constructor				CPhDialog(HINSTANCE hInstance, UINT nIDTemplate, HWND hwndParent=NULL);
	destructor				~CPhDialog();
  
// Attributes
	HINSTANCE					m_hInstance;
	HWND 							m_hDlg;
	LPCTSTR						m_lpszTemplateName;
	UINT							m_nIDTemplate;
	HWND 							m_hwndParent;
	BOOL							m_bModal;

public:
	BOOL							Create(HINSTANCE hInstance, LPCTSTR lpszTemplateName, HWND hwndParent=NULL);
	BOOL							Create(HINSTANCE hInstance, UINT nIDTemplate, HWND hwndParent=NULL);
	virtual int				DoModal();
	void							EndDialog(int nResult);

	static 
	BOOL CALLBACK 		ExportedDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL			DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};


#endif // __WINBASE_H__


