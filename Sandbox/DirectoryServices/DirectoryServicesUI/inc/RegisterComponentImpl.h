///////////////////////////////////////////////////////////////////////////////
// RegisterComponentImpl.h
// 
// Created: 09/12/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_REGISTER_COMPONENT_IMPL_H_
#define __DS_REGISTER_COMPONENT_IMPL_H_

#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
//#include <objbase.h>
#include "rs.h"         // Resource strings methods.
#include "resource.h"   // Resource definitions.
#include "utils.h"
#include "font.h"
#include "guiutils.h"
#include "RegisterComponentIface.h"

// bool    ShouldRegisterComponent(int comp);
// bool    RegisterUpdate(char *, char *);
bool    IsComponentAvailable(int comp);
bool    DoesTheFileExist(c_char * const file);
bool    GetFullyQualifiedDSBase(char *fqBuf, int bufSize);
HRESULT RegisterComponent(int comp);
HRESULT UnRegisterComponent(int comp);
HRESULT RegisterDriver(int comp);
HRESULT RegisterComponentAndDriver(int comp);

typedef HRESULT (*InstallCOM)();
typedef HRESULT (*InstallDriver)(HKEY, LPSTR);

static char _ds_dll_base[_MAX_PATH] = { '\0' };

const char * const
_ds_comp_nf  = "%s not found in the working directory.\n%s will not be available during this session";

const char * const
_ds_comp_pnf = "No Directory Services Protocol DLLs found.\nDirectory Sergvices will be unavailable during current session.";

const char * const
_ds_comp_na  = "%s Protocol will not be available during current\nsession since it could not be registered successfully.";

enum {
    DS_COMP_DIRSERV  = 0,
    DS_COMP_ISOCK    = 1,
    DS_COMP_PH       = 2,
    DS_COMP_LDAP     = 3,
    DS_COMP_EUDORABK = 4
};

c_char * const DS_COMP_ARR[] = {
    "DirServ.dll",
    "ISock.dll",
    "Ph.dll",
    "Ldap.dll",
    "EudoraBK.dll"
};

c_char * const DS_COMP_ERR[] = {
    "Directory Services",
    "Directory Services",
    "Ph Protocol",
    "LDAP Protocol",
    "EudoraBk Protocol"
};


#endif //
