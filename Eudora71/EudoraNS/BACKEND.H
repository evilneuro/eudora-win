//----------------------------------------------------
//  Backend implementation 
//
//-----------------------------------------------------

#ifndef _BACKEND_H
#define _BACKEND_H

#ifdef _DEBUG
	#define	TRACE(sz)	MessageBox(NULL, sz, "Information", MB_OK);
#else
	#define	TRACE(sz)
#endif //_DEBUG

typedef struct {
	HWND hWndClient;
	UINT uMessageID;
} THREADINFO;

DWORD TlsIndex;

VOID UnRegisterWithMailEngine(VOID);
BOOL RegisterWithMailEngine(HWND hwnd, UINT uMessageID);
BOOL IsMailClientRegistered(VOID);
BOOL IsMailConnectionOpen(VOID);
BOOL ConnectToMail(VOID);
BOOL DisconnectFromMail(VOID);
BOOL ComposeMail (LPCSTR pszTo, LPCSTR pszReference, LPCSTR pszOrganization,
                                     LPCSTR pszXURL, LPCSTR pszSubject, LPCSTR pszPage, LPCSTR pszCc, LPCSTR pszBcc);
BOOL LaunchMailFE(VOID);

VOID DisplayError(UINT uMsgID, LPCSTR lpszExtra);

typedef enum {  EUDORA_EXE_PATH = 1,
				EUDORA_DATA_FOLDER_PATH,
				EUDORA_INI_PATH 
			} PATH_OPTIONS ;
		
BOOL GetEudoraRelatedPath(LPTSTR lpszPath, PATH_OPTIONS PathOption);
#endif
