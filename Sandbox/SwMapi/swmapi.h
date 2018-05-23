// swmapi.h : main header file for the SWMAPI application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "swres.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSwitchApp:
// See swmapi.cpp for the implementation of this class
//

class CSwitchApp : public CWinApp
{
public:
	CSwitchApp();

// Overrides
	virtual BOOL InitInstance();

// Implementation

	//{{AFX_MSG(CSwitchApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
