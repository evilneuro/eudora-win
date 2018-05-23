// dlghelper.h

#ifndef __DLGHELPER_H__
#define __DLGHELPER_H__		1

///////////////////////////////////////////////////////////////////////////////
// helper macros

#define EXPORTEDDLGPROC(name)		(DLGPROC)x##name

#define MAKEEXPORTEDPAGEPROC(classname, name) \
BOOL CALLBACK classname::x##name(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ if (uMsg == WM_INITDIALOG) \
	SetWindowLong(hwndDlg, DWL_USER, ((PROPSHEETPAGE*)lParam)->lParam); \
	classname *p = (classname*)GetWindowLong(hwndDlg, DWL_USER); \
	return p ? p->name(hwndDlg, uMsg, wParam, lParam) : FALSE; } 


#define MAKEEXPORTEDDLGPROC(classname, name) \
BOOL CALLBACK classname::x##name(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ if (uMsg == WM_INITDIALOG) \
	SetWindowLong(hwndDlg, DWL_USER, lParam); \
	classname *p = (classname*)GetWindowLong(hwndDlg, DWL_USER); \
	return p ? p->name(hwndDlg, uMsg, wParam, lParam) : FALSE; } 


#define DEFEXPORTEDDLGPROC(name) \
	static BOOL CALLBACK x##name(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); \
	BOOL name(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


#define EXPORTEDPAGEPROC			EXPORTEDDLGPROC
#define DEFEXPORTEDPAGEPROC		DEFEXPORTEDDLGPROC

enum IGNOREBOUNDS
{
	IGNORENONE,
	IGNOREBOTH,
	IGNOREMAX,
	IGNOREMIN
};

///////////////////////////////////////////////////////////////////////////////
// "C" function prototypes

extern void SetListViewColumn(HWND hwndList, int nColumn, UINT nIDResString, int nWidth);
extern BOOL CheckEditFieldEmpty(HWND hwndDlg, UINT nIDEdit, UINT nResStringMsg);
extern BOOL CheckEditFieldInt(HWND hwndDlg, UINT nIDEdit, int nMin, int nMax,
											IGNOREBOUNDS ignore = IGNORENONE);
extern void ListView_EditFocused(HWND hwndList);
extern int ListView_GetFocusedItem(HWND hwndList, LPARAM *plParam=NULL);
extern BOOL SetListViewEntry(HWND hwndList, LPSTR pszAttribName, LPSTR pszPresentName, int nType);
extern LPARAM ListView_GetItemData(HWND hwndList, int iItem);
extern BOOL ListView_AutoResizeLastColumn(HWND hwndListView);


#endif		//__DLGHELPER_H__


