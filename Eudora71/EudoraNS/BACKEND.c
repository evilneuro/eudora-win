//------------------------------------------------
//  Back end implementation 
//
//------------------------------------------------

#include <windows.h>
#include <string.h>
#include "backend.h"
#include "postal.h"
#include <stdio.h>
#include "process.h"
#include <stdlib.h>
#include "tchar.h"

#include <mapi.h>

#include <sys/types.h>
#include <sys/stat.h>


extern HMODULE g_hModule;

BOOL IsMailConnectionOpen(VOID)
{
    THREADINFO *pti;    

	THREADINFO ti;
	pti = &ti;

    
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	//-----------------------------------------------------------------------

	return TRUE;
}

VOID UnRegisterWithMailEngine(VOID)
{
    THREADINFO *pti;
	THREADINFO ti;
	pti = &ti;
    
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	//-----------------------------------------------------------------------

	pti->hWndClient = 0;

}

BOOL RegisterWithMailEngine(HWND hwnd, UINT uMessageID)
{
	BOOL fRet = TRUE;
    THREADINFO *pti;

	THREADINFO ti;
	pti = &ti;

    pti = (THREADINFO *)TlsGetValue(TlsIndex);

		//-----------------------------------------------------------------------

	pti->hWndClient = hwnd;
	pti->uMessageID = uMessageID;
	
	return fRet;
}

BOOL IsMailClientRegistered(VOID)
{
   	THREADINFO *pti;

	THREADINFO ti;
	pti = &ti;

    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	//-----------------------------------------------------------------------

    return TRUE;
}

BOOL ConnectToMail()
{
    THREADINFO *pti;
	BOOL fRet = TRUE;


	THREADINFO ti;
	pti = &ti;
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	//-----------------------------------------------------------------------
	
  return fRet;   
}

BOOL DisconnectFromMail()
{
    THREADINFO *pti;

	THREADINFO ti;
	pti = &ti;

    pti = (THREADINFO *)TlsGetValue(TlsIndex);
	//-----------------------------------------------------------------------
    
    return(TRUE);
}

BOOL LaunchMailFE(VOID)
{
	TCHAR	szEudoraPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT] = {'\0'};
	BOOL bRet = FALSE;
	
	THREADINFO *pti;
	THREADINFO ti;
	pti = &ti;
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	__try
	{
		GetEudoraRelatedPath(szEudoraPath,EUDORA_INI_PATH);
		if (_tcslen(szEudoraPath))
		{
			// Set "EXT_EudoraNS_NoWarn" INI Setting in Eudora so that it does not warn about not being the default mailer.
			// Netscape, even when Eudora's configured as the default mailer overwrites the above mentioned registry with 
			// it's own executable path after it returns from this Postal DLL Hook functions.
			// Eudora, when this INI setting is set does not warn about it not being the default mailer & updates the registry
			// entries without letting the user know.
			// Also see CEudoraApp::ProcessAttachmentWEI() function where a similar addition has been done.
			
			bRet = WritePrivateProfileString(_T("Settings"),_T("EXT_EudoraNS_NoWarn"),_T("1"),szEudoraPath);
			GetEudoraRelatedPath(szEudoraPath,EUDORA_EXE_PATH);							
			ShellExecute(NULL, _T("open"), szEudoraPath, NULL, NULL, SW_SHOWNORMAL);			
		}
	}
	__except(1)
	{
		// This makes sure that even if something goes wrong in this function, Netscape does not crash
		TRACE("WARNING : Exception occurred in LaunchMailFE\n");
	}
	
	return FALSE;
}


// typedef for MAPISendMail() function pointer
typedef ULONG (PASCAL* LPFNSENDMAIL)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);

BOOL ComposeMail (LPCSTR pszTo, LPCSTR pszReference, LPCSTR pszOrganization,
                                     LPCSTR pszXURL, LPCSTR pszSubject, LPCSTR pszPage, LPCSTR pszCc, LPCSTR pszBcc)
{
	TCHAR	szEudoraPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
	TCHAR	szMAPIDLLPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];
	TCHAR*	pszChar = NULL;

	MapiMessage		message;
	MapiRecipDesc	recipDesc[3];
		
	LPFNSENDMAIL lpfnSendMail = NULL;	
	HINSTANCE hInstMAPIDLL = NULL;

	THREADINFO *pti;
	THREADINFO ti;
	pti = &ti;
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	UNREFERENCED_PARAMETER(pszReference);
	UNREFERENCED_PARAMETER(pszOrganization);
	UNREFERENCED_PARAMETER(pszXURL);

	__try
	{
		GetEudoraRelatedPath(szEudoraPath,EUDORA_EXE_PATH);
		
		_tcscpy(szMAPIDLLPath, szEudoraPath);
		pszChar = _tcsrchr(szMAPIDLLPath,'\\');

		if (pszChar)
		{
			*pszChar = '\0';
			_tcscat(szMAPIDLLPath,"\\EuMapi32.dll");
			TRACE(szMAPIDLLPath);
			
			memset(&message, 0, sizeof(message));
			hInstMAPIDLL = LoadLibrary(szMAPIDLLPath);				

			if (hInstMAPIDLL)
			{
				// Get the Proc Address for MAPISendMail.
				lpfnSendMail = (LPFNSENDMAIL)GetProcAddress(hInstMAPIDLL, _T("MAPISendMail"));

				// Subject
				message.lpszSubject = (LPTSTR)pszSubject;

				// Message Body
				message.lpszNoteText = (LPTSTR)pszPage;

				// Either we can check for empty To,CC & BCC string & not pass the MapRecipDesc for the empty ones to the MAPISendMail function
				// or let EuMapi32.dll figure that out. Since it(EuMapi32.dll) already does that, we do not do it here. We always pass the 
				// MapRecipDesc for TO, CC & BCC

				// TO :
				memset(&recipDesc[0], 0, sizeof(MapiRecipDesc));
				recipDesc[0].ulRecipClass = MAPI_TO;
				if (pszTo)
					recipDesc[0].lpszName = (LPTSTR)pszTo;
				
				// CC :
				memset(&recipDesc[1], 0, sizeof(MapiRecipDesc));
				recipDesc[1].ulRecipClass = MAPI_CC;
				if (pszCc)
					recipDesc[1].lpszName = (LPTSTR)pszCc;
				
				// BCC :
				memset(&recipDesc[2], 0, sizeof(MapiRecipDesc));
				recipDesc[2].ulRecipClass = MAPI_BCC;
				if (pszBcc)
					recipDesc[2].lpszName = (LPTSTR)pszBcc;

				message.nRecipCount = 3;
				message.lpRecips = &recipDesc[0];		
				
				if (lpfnSendMail != NULL)
					lpfnSendMail(0, 0,	&message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

				FreeLibrary( hInstMAPIDLL );
			}
		}
	}
	__except(1)
	{
		// This makes sure that even if something goes wrong in this function, Netscape does not crash
		TRACE("WARNING : Exception occurred in ComposeMail\n");
	}
	return TRUE;
}

BOOL GetEudoraRelatedPath(LPTSTR lpszPath, PATH_OPTIONS PathOption)
{
	HKEY	hKey;
	DWORD	dwKeyDataType;

	TCHAR		szData[1024];
	DWORD		dwDataBufSize=1024;

	struct stat FileAttr;
	TCHAR *firstArg = NULL;
	TCHAR * p = NULL;
	int len = 0;	

	TCHAR szParsedPaths[3][2048] = {"\0","\0","\0"};
	TCHAR szTempString[2048];
	TCHAR cLine[2048];
	int status;
	TCHAR *Ini = NULL;

	short sCount = 0;

	THREADINFO *pti;
	THREADINFO ti;
	pti = &ti;
    pti = (THREADINFO *)TlsGetValue(TlsIndex);

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Qualcomm\\Eudora\\CommandLine"),0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, _T("Current"), NULL, &dwKeyDataType,(LPBYTE) szData, &dwDataBufSize) == ERROR_SUCCESS)
		{
			_tcscpy(cLine,szData);
			Ini = cLine;

			__try
			{
				while(Ini)
				{
					// parse off firstArg
					// If long filename, arguments will be separated by quotes
					_tcscpy(szTempString,cLine);
					{
						if (Ini = _tcsstr(szTempString, "\" "))
							 *(++Ini) = 0;
						else if ( (*(szTempString + strlen(szTempString) - 1) != '\"') && (Ini = _tcschr(szTempString, ' ')))
							 *Ini = 0;
					}
					firstArg = szTempString;
			
					if (Ini)
						_tcscpy(cLine, (char *)(Ini+1));				

					// strip quotes off firstArg.  stat() doesn't expect them.  Further
					// the code that build up the ini path doesn't seem to strip any leading quotes
					if ( firstArg[ 0 ] == '\"' )
					{
						p = _tcschr( &firstArg[ 1 ], '\"' );
						if ( p )
						{
							*p = 0;		// nix trailing quote
							firstArg++;	// skip leading quote
						}
					}

					// if firstArg ends in '\\' (I.E. C:\APPS\EUDORA\ ) remove the trailing '\\'
					// stat() won't see it as a directory with the trailing '\\', except for "x:\"
					len = strlen( firstArg );
					if ( len )
					{
						TCHAR lastchar = firstArg[ len - 1 ];
						if ( lastchar == '\\' )
						{
							if ( ! ( len == 3 && firstArg[ 1 ] == ':' ) )
								firstArg[ len - 1 ] = ' ';
						}
					}

					// Check if we got an existing file/directory
					status = stat(firstArg, &FileAttr);
					if (status == 0)
					{
						// This is where the copying of the file should be done
						_tcscpy(szParsedPaths[sCount++], firstArg);
					}			
				}			
			}
			__except(1)
			{
				TRACE("WARNING : Exception caused in Parsing the HKCU\\Software\\Qualcomm\\Eudora\\Command Line parameters\n");
			}
		}

		RegCloseKey(hKey);
	}
	else 
		return FALSE;	// How come Eudora's didn't register the Command Line ?

	switch(PathOption)
	{
		case EUDORA_EXE_PATH:
			_tcscpy(lpszPath,szParsedPaths[0]);
			break;
		case EUDORA_DATA_FOLDER_PATH:
			break;
		case EUDORA_INI_PATH:
			if (!_tcslen(szParsedPaths[2]))	// If there is no third argument stored in the HKCU\S\Q\E\CommandLine
			{
				if (!_tcslen(szParsedPaths[1]))
				{
					TCHAR *pszChar = NULL;
					_tcscpy(lpszPath,szParsedPaths[0]);
					pszChar = _tcsrchr(lpszPath,'\\');
					if (pszChar)
					{
						*pszChar = '\0';
						_tcscat(lpszPath,"\\eudora.ini");
					}
				}
				else
				{
					_tcscpy(lpszPath,szParsedPaths[1]);
					_tcscat(lpszPath,"\\eudora.ini");
				}
			}
			else
				_tcscpy(lpszPath,szParsedPaths[2]);
			break;
		default:
			_tcscpy(lpszPath,_T(""));
			break;
	}

	
	return TRUE;
}

