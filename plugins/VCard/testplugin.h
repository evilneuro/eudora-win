// testplugin.h : main header file for the TESTPLUGIN DLL
//

#if !defined(AFX_TESTPLUGIN_H__58BE4CA5_54A1_11D2_89F6_00805F9B7487__INCLUDED_)
#define AFX_TESTPLUGIN_H__58BE4CA5_54A1_11D2_89F6_00805F9B7487__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "ems-win.h"
#include "MessagePlug.h"
#include "SettingsDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CTestpluginApp
// See testplugin.cpp for the implementation of this class
//

class EudoraFuncs{
public:
	emsAddABEntry AddNNToAB;
	emsEnumAddressBooks EnumAB;
};

class CTestpluginApp : public CWinApp
{
public:
	CTestpluginApp();
	BOOL InitInstance();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestpluginApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTestpluginApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	EudoraFuncs m_EudoraFuncs;
	CMessagePlug* m_MessagePlugin;
	HICON m_OurIcon;
	bool m_ShowingSettingsPanel;
	CSettingsDialog* m_Settings;
	char * m_INIFilename;
};

class VCEntry{
public:
	char *FieldName;
	char *Value;
	VCEntry* next;

	VCEntry() {FieldName = NULL; Value = NULL; next = NULL;}
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPLUGIN_H__58BE4CA5_54A1_11D2_89F6_00805F9B7487__INCLUDED_)
