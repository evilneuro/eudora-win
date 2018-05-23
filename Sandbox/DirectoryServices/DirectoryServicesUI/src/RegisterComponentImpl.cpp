//////////////////////////////////////////////////////////////////////////////
// RegisterComponentImpl.cpp
// 
// Created: 08/13/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegisterComponentImpl.h"

HRESULT
RegisterComponent(int comp)
{
    HRESULT    hErr             = E_FAIL;
    HINSTANCE  hLib             = NULL;
    InstallCOM pfnInstallCOM    = NULL;
    FARPROC    fP               = NULL;
    char       fqBuf[_MAX_PATH] = { '\0' };

    if (::GetFullyQualifiedDSBase(fqBuf, sizeof(fqBuf)) == true) {
	strcat(fqBuf, DS_COMP_ARR[comp]);
	if ((hLib = ::LoadLibrary(fqBuf)) != NULL) {
	    if ((fP = ::GetProcAddress(hLib, "DllRegisterServer")) != NULL) {
		pfnInstallCOM = (InstallCOM) fP;
		hErr          = (*pfnInstallCOM)();
	    }
	    ::FreeLibrary(hLib);
	}
    }

    return hErr;
}

HRESULT
UnRegisterComponent(int comp)
{
    HRESULT    hErr             = E_FAIL;
    HINSTANCE  hLib             = NULL;
    InstallCOM pfnInstallCOM    = NULL;
    FARPROC    fP               = NULL;
    char       fqBuf[_MAX_PATH] = { '\0' };

    if (::GetFullyQualifiedDSBase(fqBuf, sizeof(fqBuf)) == true) {
	strcat(fqBuf, DS_COMP_ARR[comp]);
	if ((hLib = ::LoadLibrary(fqBuf)) != NULL) {
	    if ((fP = ::GetProcAddress(hLib, "DllUnregisterServer")) != NULL) {
		pfnInstallCOM = (InstallCOM) fP;
		hErr          = (*pfnInstallCOM)();
	    }
	    ::FreeLibrary(hLib);
	}
    }

    return hErr;
}

HRESULT
RegisterDriver(int comp)
{
    HRESULT       hErr             = E_FAIL;
    HINSTANCE     hLib             = NULL;
    InstallDriver pfnInstallDriver = NULL;
    FARPROC       fP               = NULL;
    char          fqBuf[_MAX_PATH] = { '\0' };
    LPSTR         szReg = "Software\\Qualcomm\\Shared\\DirServices\\Drivers";

    if (::GetFullyQualifiedDSBase(fqBuf, sizeof(fqBuf)) == true) {
	strcat(fqBuf, DS_COMP_ARR[comp]);
	if ((hLib = ::LoadLibrary(fqBuf)) != NULL) {
	    if ((fP = ::GetProcAddress(hLib, "DllInstallServer")) != NULL) {
		pfnInstallDriver = (InstallDriver) fP;
		hErr             = (*pfnInstallDriver)(HKEY_CURRENT_USER,
						       szReg);
	    }
	    ::FreeLibrary(hLib);
	}
    }

    return hErr;
}


HRESULT
RegisterComponentAndDriver(int comp)
{
    HRESULT       hErr             = E_FAIL;
    HINSTANCE     hLib             = NULL;
    InstallCOM    pfnInstallCOM    = NULL;
    InstallDriver pfnInstallDriver = NULL;
    FARPROC       fP               = NULL;
    char          fqBuf[_MAX_PATH] = { '\0' };
    LPSTR         szReg = "Software\\Qualcomm\\Shared\\DirServices\\Drivers";

    if (::GetFullyQualifiedDSBase(fqBuf, sizeof(fqBuf)) == true) {
	strcat(fqBuf, DS_COMP_ARR[comp]);
	if ((hLib = ::LoadLibrary(fqBuf)) != NULL) {
	    if ((fP = ::GetProcAddress(hLib, "DllRegisterServer")) != NULL) {
		pfnInstallCOM = (InstallCOM) fP;
		hErr          = (*pfnInstallCOM)();
		if (SUCCEEDED(hErr)) {
		    if ((fP = ::GetProcAddress(hLib, "DllInstallServer")) !=
			NULL) {
			pfnInstallDriver = (InstallDriver) fP;
			hErr             = (*pfnInstallDriver)
			                   (HKEY_CURRENT_USER, szReg);
			if (FAILED(hErr)) {
			    if ((fP = ::GetProcAddress(hLib,
						       "DllUnregisterServer"))
				!= NULL) {
				pfnInstallCOM = (InstallCOM) fP;
				// We want to save the failed value of
				// of hErr, so no assignment to hErr here.
				(*pfnInstallCOM)();
			    }
			}
		    }
		}
	    }
	    ::FreeLibrary(hLib);
	}
    }
    
    return hErr;
}

bool
GetFullyQualifiedDSBase(char *fqBuf, int bufSize)
{
    bool retVal       = true;

    if (_ds_dll_base[0] == '\0') {
	if (::GetModuleFileName(AfxGetInstanceHandle(), fqBuf, bufSize)) {
	    char *temp = strrchr(fqBuf, '\\');
	    temp[1]    = '\0';
	    strcpy(_ds_dll_base, fqBuf);
	}
	else {
	    ::MessageBox(NULL, "Could not get module name",
			 "Error!", MB_OK | MB_ICONEXCLAMATION);
	    retVal = false;
	}
    }
    else {
	strcpy(fqBuf, _ds_dll_base);
    }

    return retVal;
}


bool
IsComponentAvailable(int comp)
{
    bool retVal = true;

    if (!DoesTheFileExist(DS_COMP_ARR[comp])) {
	char buf[256];
	sprintf(buf, _ds_comp_nf, DS_COMP_ARR[comp], DS_COMP_ERR[comp]);
	::MessageBox(NULL, buf, "Error!", MB_OK | MB_ICONEXCLAMATION);
	retVal = false;
    }

    return retVal;
}

HRESULT
RegisterCOMObjects()
{
    HRESULT hErr        = E_FAIL;
    short   howManyComp = 0;
    char    buf[256]    = { '\0' };

    if (!IsComponentAvailable(DS_COMP_DIRSERV)) {
	goto ds_not_registered;
    }
    else {
	if (FAILED(::RegisterComponent(DS_COMP_DIRSERV))) {
	    goto ds_not_registered;
	}
    }

    if (!IsComponentAvailable(DS_COMP_ISOCK)) {
	goto ds_not_registered;
    }
    else {
	if (FAILED(::RegisterComponent(DS_COMP_ISOCK))) {
	    goto ds_not_registered;
	}
    }

    // Now that DirServ and ISock are successfully registered,
    // flip the switch.

    hErr = S_OK;

    if (IsComponentAvailable(DS_COMP_PH)) {
	if (FAILED(::RegisterComponentAndDriver(DS_COMP_PH))) {
	    sprintf(buf, _ds_comp_na, DS_COMP_ARR[DS_COMP_PH]);
	    ::MessageBox(NULL, buf, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
	else {
	    howManyComp++;
	}
    }

    if (IsComponentAvailable(DS_COMP_LDAP)) {
	if (FAILED(::RegisterComponentAndDriver(DS_COMP_LDAP))) {
	    sprintf(buf, _ds_comp_na, DS_COMP_ARR[DS_COMP_LDAP]);
	    ::MessageBox(NULL, buf, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
	else {
	    howManyComp++;
	}
    }

    if (IsComponentAvailable(DS_COMP_EUDORABK)) {
	if (FAILED(::RegisterComponentAndDriver(DS_COMP_EUDORABK))) {
	    sprintf(buf, _ds_comp_na, DS_COMP_ARR[DS_COMP_EUDORABK]);
	    ::MessageBox(NULL, buf, "Error!", MB_OK | MB_ICONEXCLAMATION);
	}
	else {
	    howManyComp++;
	}
    }

    if (howManyComp == 0) {
	// Since no components could be registered, flip the switch back.
	hErr = E_FAIL;
	::MessageBox(NULL, _ds_comp_pnf, "Error!", MB_OK | MB_ICONEXCLAMATION);
    }

ds_not_registered:

    return hErr;
}

// bool
// ShouldRegisterComponent(int comp)
// {
//     bool  retVal = true;
//     DWORD handle = 0;
// #ifdef __DEBUG__
//     char fileInfoBuf[512] = { '\0' }, fqBuf[_MAX_PATH] = { '\0' };
// #endif
//     DWORD size = 0;

//     bool bGFQDSP = ::GetFullyQualifiedDSPath(DS_COMP_ARR[comp], fqBuf, sizeof(fqBuf));

//     if (bGFQDSP != false) {
// #ifdef __DEBUG__
// 	sprintf(fileInfoBuf, "No version info on %s.", fqBuf);
// #endif
// 	size =  ::GetFileVersionInfoSize(fqBuf, &handle);

// 	if (size != NULL) {
// 	    char *p_data = new char[size];
// 	    if (p_data != NULL) {
// 		BOOL bFVI = ::GetFileVersionInfo(fqBuf, handle, size,
// 						 p_data);
// 		if (bFVI == FALSE) {
// 		    MAssertSzMT(bFVI, fileInfoBuf);
// 		    // We assert. Doing nothing here will
// 		    // go ahead and register the component,
// 		    // exactly the action we want here.
// 		}
// 		else {
// 		    char *p_buf   = 0;
// 		    UINT  bufsize = 0;
// 		    BOOL  bVQV    = ::VerQueryValue(p_data, "\\",
// 						    (void **) &p_buf,
// 						    &bufsize);
// 		    if (bVQV == TRUE) {
// 			if (p_buf && bufsize) {
// 			    //retVal = RegisterUpdate(p_buf,
// 			    //	DS_COMP_ARR[comp]);
// 			    retVal = true;
// 			}
// 		    }
// 		    else  {
// 			MAssertSzMT(bVQV, fileInfoBuf);
// 			// We assert. Doing nothing here will
// 			// go ahead and register the component,
// 			// the action we want here.
// 		    }
// 		}
// 		delete [] p_data;
// 	    }
// 	    else {
// 		MAssertSzMT(p_data, "Out of Memory");
// 		retVal = false;
// 	    }
// 	}
// 	else {
// 	    // if we can't get size info on the file, we go ahead and
// 	    // register, but fire an assert.
// 	    MAssertSzMT(size, fileInfoBuf);
// 	}
//     }
//     else {
// 	retVal = false;
//     }

//     return(retVal);
// }

// bool
// RegisterUpdate(char *p_buf, char *file)
// {

//     bool     retVal = false;
//     CRString dsSection(IDS_NDS_DIRSERV_SECTION);
//     char     buf[64];
//     DWORD    fileVersionMS = 0;
//     DWORD    fileVersionLS = 0;
//     UINT     iniVersionMS  = 0;
//     UINT     iniVersionLS  = 0;
//     CString  strMS(file);
//     CString  strLS(file);

//     strMS = strMS.Left(strMS.GetLength() - 4);
//     strMS += "-Major";
//     strLS = strLS.Left(strLS.GetLength() - 4);
//     strLS += "-Minor";

//     VS_FIXEDFILEINFO* p_info = (VS_FIXEDFILEINFO *) p_buf;
//     fileVersionMS            = p_info->dwFileVersionMS / 0x10000;
//     fileVersionLS            = p_info->dwFileVersionLS;

//     iniVersionMS = ::GetPrivateProfileInt(dsSection, strMS, 0, INIPath);
//     iniVersionLS = ::GetPrivateProfileInt(dsSection, strLS, 0, INIPath);

//     if (fileVersionMS > iniVersionMS) {
// 	::WritePrivateProfileString(dsSection, strMS,
// 				    itoa(fileVersionMS, buf, 10), INIPath);
// 	retVal = true;
//     }

//     if (fileVersionLS > iniVersionLS) {
// 	::WritePrivateProfileString(dsSection, strLS,
// 				    itoa(fileVersionLS, buf, 10), INIPath);
// 	retVal = true;
//     }

//     return(retVal);
// }

bool
DoesTheFileExist(c_char * const file)
{
    bool retVal      = false;
    char fqBuf[_MAX_PATH] = { '\0' };

    if (::GetFullyQualifiedDSBase(fqBuf, sizeof(fqBuf)) == true) {
	strcat(fqBuf, file);
	OFSTRUCT ofs;

	HFILE hFile = ::OpenFile(fqBuf, &ofs, OF_EXIST);

	if (hFile != HFILE_ERROR) {
	    retVal = true;
	}
    }

    return retVal;
}
