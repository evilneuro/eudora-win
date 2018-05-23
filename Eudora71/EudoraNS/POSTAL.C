//-----------------------------------------------------------
//	Netscape Postal API DLL 
//	
//	The functions exported in this DLL are the ones Navigator
//	looks for on startup and invokes appropriately.
//
//------------------------------------------------------------

#include <windows.h>
#include <stdlib.h>
#include "postal.h"
#include "backend.h"
//#include "resource.h"


HINSTANCE hInst;
HMODULE g_hModule;

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD  fdwReason, LPVOID lpvReserved)
{
    THREADINFO *pti;

	UNREFERENCED_PARAMETER(lpvReserved);

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			  hInst = hinstDLL;
			  TlsIndex = TlsAlloc();

			  g_hModule = hinstDLL;
		case DLL_THREAD_ATTACH:
			pti = (THREADINFO *)LocalAlloc(LPTR, sizeof(THREADINFO));
			if (!pti) 
			{
				return(FALSE);
			}
			TlsSetValue(TlsIndex, pti);
		  break;

		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			pti = TlsGetValue(TlsIndex);
			LocalFree(pti);
			if (fdwReason == DLL_PROCESS_DETACH) 
			{
				TlsFree(TlsIndex);
			}
	      break;
   }

   return TRUE;
}

EXPORT32 POSTCODE EXPORT16 RegisterMailClient (HWND hwnd, LPSTR szMessage)
{
	POSTCODE pRet = POST_OK;

    UINT uMessageID = RegisterWindowMessage(szMessage);

	TRACE("RegisterMailClient");

    if (!IsWindow(hwnd)) 
		pRet = POST_E_BAD_HWND;

    if (!uMessageID)
        pRet = POST_E_BAD_SZMSG;

    if (pRet == POST_OK)
	{
        if(! RegisterWithMailEngine(hwnd, uMessageID))
			pRet = POST_E_FAILED;
	}

	return pRet;
}

 
EXPORT32 POSTCODE EXPORT16 UnRegisterMailClient ()
{
	POSTCODE pRet = POST_OK;

	TRACE("UnRegisterMailClient");
	
    if (!IsMailClientRegistered())
        pRet = POST_E_UNREGISTERED;

	if (pRet == POST_OK)
		UnRegisterWithMailEngine();

	return pRet;
}
 
EXPORT32 POSTCODE EXPORT16 OpenMailSession (LPSTR pszProfile, LPSTR pszPassword)
{
	UNREFERENCED_PARAMETER(pszProfile);
	UNREFERENCED_PARAMETER(pszPassword);

	TRACE("OpenMailSession");

    if (!IsMailClientRegistered())
        return POST_E_UNREGISTERED;

	return POST_OK;
}

 
EXPORT32 POSTCODE EXPORT16 CloseMailSession ()
{
	TRACE("CloseMailSession");

    if (!IsMailClientRegistered())
        return POST_E_UNREGISTERED;

	if(!IsMailConnectionOpen())
		return POST_E_NO_SESSION;

	return POST_OK;
}

 
EXPORT32 POSTCODE EXPORT16 ComposeMailMessage (LPCSTR pszTo, LPCSTR pszReference, LPCSTR pszOrganization,
                                     LPCSTR pszXURL, LPCSTR pszSubject, LPCSTR pszPage, LPCSTR pszCc, LPCSTR pszBcc)
{
	TRACE("ComposeMailMessage");

    if (!IsMailClientRegistered())
        return POST_E_UNREGISTERED;

	if(!ConnectToMail())
		return POST_E_FAILED;

	if(! ComposeMail(pszTo, pszReference, pszOrganization, pszXURL, pszSubject, pszPage, pszCc, pszBcc))
		return POST_E_FAILED;

	if(!DisconnectFromMail())
		return POST_E_FAILED;

	return POST_OK;
}

EXPORT32 POSTCODE EXPORT16 ShowMailBox()
{
	TRACE("LaunchMailReader");

    if (!IsMailClientRegistered())
        return POST_E_UNREGISTERED;

	if(!ConnectToMail())
		return POST_E_FAILED;

	if(! LaunchMailFE())
		return POST_E_CANT_LAUNCH_UI;

	if(!DisconnectFromMail())
		return POST_E_FAILED;

	return POST_OK;
}

EXPORT32 POSTCODE EXPORT16 ShowMessageCenter()
{
	return ShowMailBox();
}

EXPORT32 POSTCODE EXPORT16 GetMenuItemString(ALTMAIL_MENU_ID menuID, LPSTR lpszReturnBuffer, int cbBufLen)
{
    if (!IsMailClientRegistered())
        return POST_E_UNREGISTERED;

	if(!lpszReturnBuffer)
		return POST_E_BAD_BUFFER;

	if(cbBufLen <= 0)
		return POST_E_BAD_LEN;

	switch (menuID)
	{
		case ALTMAIL_MenuMailBox:
			*lpszReturnBuffer = '\0'; 
			break;
		case ALTMAIL_MenuMessageCenter:
			//Since we do not support MessageCenter UI, set buffer to zero so 
			//that this menu item is deleted.
			*lpszReturnBuffer = '\0'; 
			break;
		default:
			break;
	}

	return POST_OK;
}
