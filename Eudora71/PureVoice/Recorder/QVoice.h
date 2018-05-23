// QVoice.h : main header file for the QVOICE application
//
// 02.10.97 lsong CSingleInstance 

#ifndef _QVOICE_H_
#define _QVOICE_H_

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

// DEFINES
#define APP_ID_STR "C311FA70DC7311d0BA9300805FB4B97E"

class CSingleInstance 
{
public:
//	LIFECYCLE
	CSingleInstance();
	~CSingleInstance();
                
//	OPERATIONS
	BOOL CheckInstance( LPCTSTR, CCommandLineInfo& cmdLineInfo );
	BOOL SetProperty( UINT nID );
	BOOL RemoveProperty( UINT nID );

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	void SendFileName( CWnd* wnd, CString& fileName );

//	ATTRIBUTES
	UINT m_nID;

private:
	static BOOL CALLBACK PropEnumFuncEx( HWND, LPTSTR, HANDLE, DWORD );
};

class CLsCommandLineInfo : public CCommandLineInfo
{
public:
//	LIFECYCLE
	CLsCommandLineInfo() :
	  m_strOutFileName(_T("")), m_strOutPathName(_T("")), 
	  m_strDllPathName(_T("")), m_hWndParent(0),
	  mb_OutFileName(FALSE), mb_DllPathName(FALSE), mb_UseMapi(TRUE),
	  mb_Record(FALSE), mb_RegisterOnly(FALSE), mb_hWndParent(FALSE),
	  mb_EudoraFileNameReq(FALSE) {};

//	OPERATIONS	  
	virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

//	ACCESS
	CString m_strOutFileName;
	CString m_strOutPathName;
	CString m_strDllPathName;
	BOOL	mb_UseMapi;
	BOOL	mb_Record;
	BOOL	mb_RegisterOnly;
	BOOL	mb_EudoraFileNameReq;
	HWND	m_hWndParent;

//////////////////////////////////////////////////////////////////////////////
private:
//	ATTRIBUTES

	BOOL	mb_OutFileName;
	BOOL	mb_DllPathName;
	BOOL	mb_hWndParent;
};

/////////////////////////////////////////////////////////////////////////////
// CQVoiceApp:
// See QVoice.cpp for the implementation of this class
//

class CQVoiceApp : public CWinApp
{
public:
//	LIFECYCLE
	CQVoiceApp();

//	OPERATIONS
	void YieldEvents();

//	ACCESS
	CLsCommandLineInfo	m_cmdLineInfo;
	CSingleInstance		m_singleInstance;
	DWORD				m_OsPlatform;

//////////////////////////////////////////////////////////////////////////////
private:
//	ATTRIBUTES
	CSingleDocTemplate* mp_DocTemplate;

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQVoiceApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CQVoiceApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
