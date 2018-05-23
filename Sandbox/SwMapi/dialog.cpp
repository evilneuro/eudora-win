// dialog.cpp :	implementation file
//

#include "stdafx.h"
#include "swmapi.h"
#include "dialog.h"
#include "mapiinst.h"
#include "status.h"
#include "aboutdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char	BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSwitchDialog dialog

CSwitchDialog::CSwitchDialog(CWnd* pParent /*=NULL*/) :	
	CDialog(CSwitchDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSwitchDialog)
	//}}AFX_DATA_INIT
}

void CSwitchDialog::DoDataExchange(CDataExchange*	pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSwitchDialog)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSwitchDialog, CDialog)
	//{{AFX_MSG_MAP(CSwitchDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ABOUT, OnClickedAbout)
	ON_BN_CLICKED(IDC_INSTALL, OnClickedInstall)
	ON_BN_CLICKED(IDC_UNINSTALL, OnClickedUninstall)
	ON_BN_CLICKED(IDC_INFO, OnClickedInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwitchDialog message handlers

////////////////////////////////////////////////////////////////////////
// OnInitDialog
//
////////////////////////////////////////////////////////////////////////
BOOL CSwitchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//
	// Disable the Size and Maximize menu items on the System Menu.
	//
	CMenu* p_sysmenu = GetSystemMenu(FALSE);
	ASSERT(p_sysmenu != NULL);
	p_sysmenu->EnableMenuItem(SC_SIZE, MF_GRAYED);
	p_sysmenu->EnableMenuItem(SC_MAXIMIZE, MF_GRAYED);

	return TRUE;	// return TRUE  unless you set the focus to a control
}


////////////////////////////////////////////////////////////////////////
// OnClose
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::OnClose()
{
	EndDialog(IDCANCEL);
	CDialog::OnClose();
}


////////////////////////////////////////////////////////////////////////
// OnClickedInstall [protected]
//
// 1.  If there is no existing MAPI DLL in the Windows System directory,
// then continue to step 3.
//
// 2.  If the current MAPI DLLs are not Eudora MAPI DLLs, rename the
// Microsoft MAPI DLLs in the Windows System directory.  As a
// precaution, the renaming only occurs if we can verify that the
// existing MAPI DLL is *not* a Eudora MAPI DLL since we only want
// System DLLs as backup DLLs.  
//
// If the existing backup DLL is the same as the current DLL, then
// we're done.  Otherwise, if the existing backup DLL is different
// than the current DLL, then the user will be warned and the renamed
// DLL will have a different name than the existing backup DLLs.
//
// 3.  If we have a good backup DLL, then copy the current Eudora MAPI
// DLLs to the Windows System directory.
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::OnClickedInstall()
{
	CString csMAPIInstall;
	csMAPIInstall.LoadString( IDS_MAPI_INSTALL );
	
	CString csMAPIInstallError;
	csMAPIInstallError.LoadString( IDS_MAPI_INSTALL_ERROR );
	
	CString csStatus;
	
	//
	// Do the install...
	//
	switch (CMapiInstaller::Install())
	{
	case CMapiInstaller::STATUS_SUCCESSFUL:
		csStatus.LoadString( IDS_INSTALLED_OK );
		MessageBox(csStatus, csMAPIInstall, MB_ICONINFORMATION | MB_OK);
		break;
	case CMapiInstaller::STATUS_ALREADY_INSTALLED:
		csStatus.LoadString( IDS_ALREADY_INSTALLED );
		MessageBox(csStatus, csMAPIInstall, MB_ICONINFORMATION | MB_OK);
		break;
	case CMapiInstaller::STATUS_DLL_IN_USE:
		csStatus.LoadString( IDS_ERROR_1 );
		MessageBox(csStatus, csMAPIInstallError, MB_ICONSTOP | MB_OK);
		break;
	case CMapiInstaller::STATUS_FAILED:
		csStatus.LoadString( IDS_ERROR_2 );
		MessageBox(csStatus, csMAPIInstallError, MB_ICONSTOP | MB_OK);
		break;
	case CMapiInstaller::STATUS_SOURCE_DLL_NOT_FOUND:
		csStatus.LoadString( IDS_ERROR_3 );
		MessageBox(csStatus, csMAPIInstallError, MB_ICONSTOP | MB_OK);
		break;
	case CMapiInstaller::STATUS_INTERNAL_ERROR:		// Installer had internal error
		csStatus.LoadString( IDS_ERROR_4 );
		MessageBox(csStatus, csMAPIInstallError, MB_ICONSTOP | MB_OK);
		break;
	default:
		ASSERT(0);
		csStatus.LoadString( IDS_ERROR_5 );
		MessageBox(csStatus, csMAPIInstallError, MB_ICONSTOP | MB_OK);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnClickedUninstall [protected]
//
// 1.  If the current DLL is a Eudora DLL, then remove it.
//
// 2.  If there is no backup DLL, or if the backup DLL is the same as 
// the current DLL, then we're done.
//
// 3.  If we have a good backup DLL, then rename it as the official
// MAPI DLL.  If there are multiple backup DLLs to choose from, then
// maybe we should let the user decide?
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::OnClickedUninstall()
{
	CString csMAPIUninstall;
	csMAPIUninstall.LoadString( IDS_MAPI_UNINSTALL );
	
	CString csMAPIUninstallError;
	csMAPIUninstallError.LoadString( IDS_MAPI_UNINSTALL_ERROR );
	
	CString csStatus;
	
	//
	// Do the uninstall...
	//
	switch (CMapiInstaller::Uninstall())
	{
	case CMapiInstaller::STATUS_SUCCESSFUL:
		csStatus.LoadString( IDS_UNINSTALLED_OK );
		MessageBox(csStatus, csMAPIUninstall, MB_ICONINFORMATION | MB_OK);
		break;
	case CMapiInstaller::STATUS_NOT_INSTALLED:
		csStatus.LoadString( IDS_NOT_UNINSTALLED );
		MessageBox(csStatus, csMAPIUninstall, MB_ICONINFORMATION | MB_OK);
		break;
	case CMapiInstaller::STATUS_DLL_IN_USE:
		csStatus.LoadString( IDS_ERROR_6 );
		MessageBox(csStatus, csMAPIUninstallError, MB_ICONSTOP | MB_OK);
		break;
	case CMapiInstaller::STATUS_FAILED:
		csStatus.LoadString( IDS_ERROR_7 );
		MessageBox(csStatus, csMAPIUninstallError, MB_ICONSTOP | MB_OK);
		break;
	case CMapiInstaller::STATUS_INTERNAL_ERROR:
		csStatus.LoadString( IDS_ERROR_4 );
		MessageBox(csStatus, csMAPIUninstallError, MB_ICONSTOP | MB_OK);
		break;
	default:
		ASSERT(0);
		csStatus.LoadString( IDS_ERROR_5 );
		MessageBox(csStatus, csMAPIUninstallError, MB_ICONSTOP | MB_OK);
		break;
	}
}


////////////////////////////////////////////////////////////////////////
// OnClickedAbout
//
// Display the About dialog for this application.
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::OnClickedAbout()
{
    CAboutDlg dlg(this);
    dlg.DoModal();
}


////////////////////////////////////////////////////////////////////////
// OnClickedInfo
//
// Display the MAPI Info dialog for this application.
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::OnClickedInfo()
{
    CStatusDlg dlg(this);
    dlg.DoModal();
}


////////////////////////////////////////////////////////////////////////
// OnCancel
//
// Intercept the cancel keystroke and convert it into the equivalent of
// a WM_CLOSE so that all exit routes go through the OnClose() function.
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::OnCancel()
{
	PostMessage(WM_SYSCOMMAND, SC_CLOSE);
}
















































#ifdef FORNOW
////////////////////////////////////////////////////////////////////////
// LoadEudoraDLL
//
// There are several possibilities to consider here.
//
// 1.  Check to see if there is a MAPI module already loaded.  If so,
// then check to see if it is the Eudora MAPI DLL.  If it is the
// Eudora MAPI DLL, then execute another LoadLibrary() to increment
// the reference count.  Otherwise, leave the m_hEudoraDLLInst NULL to
// indicate that we failed to load the Eudora MAPI DLL.
//
// 2.  If there is no MAPI module already loaded, then attempt to load
// the Eudora EUMAPI.DLL from the same directory as this loader
// program.  If successful, verify the identity of the Eudora MAPI
// DLL.  Otherwise, unload the DLL and set the m_hEudoraDLLInst to
// NULL to indicate that we could not load the Eudora MAPI DLL.
//
////////////////////////////////////////////////////////////////////////
BOOL CSwitchDialog::LoadEudoraDLL(UINT& errorCode)	//(o) error code set only if returning FALSE
{
	//
	// Check for an existing MAPI module.
	//
	ASSERT(NULL == m_hEudoraDLLInst);
#ifndef WIN32
	MODULEENTRY mod_entry;
	mod_entry.dwSize = sizeof(MODULEENTRY);
	HMODULE h_module;
	if ((h_module = ModuleFindName(&mod_entry, "MAPI")) != NULL)
	{
		//
		// Could be bad news.  Somebody's MAPI DLL is already loaded,
		// so we no choice but to live with the one that somebody
		// else loaded before we got here.
		//
		m_hEudoraDLLInst = LoadLibrary(mod_entry.szExePath);
		if (m_hEudoraDLLInst < HINSTANCE_ERROR)
		{
			m_hEudoraDLLInst = NULL;
			errorCode = IDS_ERR_CANNOT_QUERY_EXISTING_DLL;
			return FALSE;
		}
		else
		{
			//
			// Check to see if we got the Eudora MAPI DLL.
			//
			if (GetProcAddress(m_hEudoraDLLInst, "IsEudoraMapiDLL"))
			{
				return TRUE;
			}
			else
			{
				FreeLibrary(m_hEudoraDLLInst);
				m_hEudoraDLLInst = NULL;
				errorCode = IDS_ERR_INCOMPATIBLE_DLL;
				return FALSE;
			}
		}
	}
#endif // WIN32

	//
	// Okay, if we get this far, then there is no existing MAPI
	// module in memory.  We can now go to town and load up the
	// EUMAPI.DLL from the same directory as this application.
	//
	ASSERT(NULL == m_hEudoraDLLInst);
	CString mapi_pathname;
    char buf[_MAX_PATH + 1];
	if (GetModuleFileName(AfxGetInstanceHandle(), buf, sizeof(buf)))
	{
		char * p_slash = strrchr(buf, '\\');
		if (p_slash)
		{
			p_slash[1] = '\0';
			mapi_pathname = buf;
		}
    }
	
	//
	// Load up the EUMAPI.DLL into system memory.
	//
#ifdef WIN32
	mapi_pathname += "EUMAPI32.DLL";		// FORNOW, hardcoded name
#else
	mapi_pathname += "EUMAPI.DLL";			// FORNOW, hardcoded name
#endif // WIN32
	m_hEudoraDLLInst = LoadLibrary(mapi_pathname);

#ifdef WIN32
	if (NULL == m_hEudoraDLLInst)
#else
	if (m_hEudoraDLLInst < HINSTANCE_ERROR)
#endif
	{
		m_hEudoraDLLInst = NULL;
		errorCode = IDS_ERR_CANNOT_LOAD_DLL;
		return FALSE;
	}
	else
	{
		//
		// Do a super-paranoid check to make sure we got the
		// Eudora MAPI DLL.
		//
		if (GetProcAddress(m_hEudoraDLLInst, "IsEudoraMapiDLL"))
		{
			return TRUE;
		}
		else
		{
			UnloadEudoraDLL();
			errorCode = IDS_ERR_INCOMPATIBLE_DLL;
			return FALSE;
		}
		return TRUE;
	}

	//
	// We should never get this far.
	//
	ASSERT(0);
	errorCode = IDS_ERR_INTERNAL;
	return FALSE;
}
#endif // FORNOW


#ifdef FORNOW
////////////////////////////////////////////////////////////////////////
// UnloadEudoraDLL
//
// If there was a previous successful load, then do the unload.
// Otherwise, do nothing.
//
////////////////////////////////////////////////////////////////////////
void CSwitchDialog::UnloadEudoraDLL(void)
{
	if (m_hEudoraDLLInst)
		FreeLibrary(m_hEudoraDLLInst);

	m_hEudoraDLLInst = NULL;
}
#endif // FORNOW


