#ifndef _EUSHLEXT_REG_DEFINES_H_
#define _EUSHLEXT_REG_DEFINES_H_

/*********************************************************************************************
//IMPORTANT : If contents of this file are changed, please recompile Eudora.exe & EuShlExt.dll
**********************************************************************************************/

// All these registry keys are under HKEY_CURRENT_USER
const char LM_REG_APP_KEY[]					=	"Software\\Qualcomm\\Eudora\\LaunchManager";
const char LM_REG_PATHCOUNT_KEY[]			=	"PathCount";
const char LM_REG_ALREADYWARNED_KEY[]		=	"AlreadyWarned#%d";
const char LM_REG_WARN_KEY[]				=	"WarnExtensions#%d";
const char LM_REG_DONOTWARN_KEY[]			=	"DoNotWarnExtensions#%d";
const char LM_REG_PATH_KEY[]				=	"Path#%d";
const char LM_REG_DLLPATH_KEY[]				=	"DLLPath";
const char LM_REG_ENABLELOGGING_KEY[]		=	"EnableLogging";

// These are under HKEY_CLASSES_ROOT
const char LM_REG_REGISTRATION_CID_KEY[]	=	"CLSID\\%s";
const char LM_REG_REGISTRATION_CID_I_KEY[]	=	"CLSID\\%s\\InProcServer32";
const char LM_REG_REGISTRATION_E_SE_KEY[]	=	"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks";
const char LM_REG_REGISTRATION_SE_A_KEY[]	=	"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved";

// Shell Extension's Name
const char gszShellExtensionDLLName[]	=	"EuShlExt.dll";

#endif // _EUSHLEXT_REG_DEFINES_H_
