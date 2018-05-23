////////////////////////////////////////////////////////////////////////
// MAPIFUNC.CPP
//
// MAPI entry points for 16-bit/32-bit Eudora MAPI DLL.
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
//#include <afxwin.h>		// FORNOW, might be better to use precompiled AFX headers
#include <shellapi.h>
#ifndef WIN32
#include <time.h>
#endif //!WIN32

#include "mapires.h"
#include "..\Eudora\eumapi.h"	// includes the hacked MAPI.H
#include "addrdlg.h"
#include "reciplst.h"
#include "sesmgr.h"
#include "recip.h"
#include "message.h"

#include "..\Eudora\ddeclien.h"


//
// The OUTPUTDEBUGSTRING macro is pretty obvious, but the SAFESTRING
// macro works around a bug in MFC CString classes which assert if
// given NULL char* values for a %s field.  The C runtime sprintf()
// handles NULL char* args properly by quietly inserting a "(null)"
// string.
//
#define OutputDebugString(_dbg_Msg_) ::MessageBox(NULL, _dbg_Msg_, "Eudora MAPI Debug Output", MB_OK)

#ifdef _DEBUG
#define OUTPUTDEBUGSTRING(_dbg_Msg_) OutputDebugString(_dbg_Msg_)
#define SAFESTRING(_p_dbg_Str_) ((_p_dbg_Str_ != NULL) ? (_p_dbg_Str_) : "(null)")
#else
#define OUTPUTDEBUGSTRING(_dbg_Msg_)
#define SAFESTRING(_p_dbg_Str_) "(bogus)"
#endif // _DEBUG

//
// Statics...
//
static CMapiSessionMgr s_SessionMgr;


////////////////////////////////////////////////////////////////////////
// EnumWindowsProc [static]
//
// Global callback entry point for Windows EnumWindows() API call.
// We assume that the caller stuffs a pointer to a CDWordArray
// object in the LPARAM.  We stuff this CDWordArray with all of the
// HWNDs for all running instances of Eudora.
////////////////////////////////////////////////////////////////////////
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	CDWordArray* p_array = (CDWordArray *) lParam;		// type cast
	if ((NULL == p_array) || (! p_array->IsKindOf(RUNTIME_CLASS(CDWordArray))))
	{
		ASSERT(0);
		return FALSE;		// inform EnumWindows() that we don't want to continue
	}

	char classname[100];
	if (::GetClassName(hWnd, classname, sizeof(classname)))
	{
		if (stricmp(classname, EudoraMainWindowClass) == 0)
		{
			TRACE1("EnumWindowsProc: Found a running instance of Eudora (HWND=0x%08X)\n", hWnd);
			p_array->Add(DWORD(hWnd));
		}
	}

	return TRUE;
}

static BOOL GetCommandLine(CString& CommandLine)
{
	// The 32-bit Eudora stores the command line identically under
	// HKEY_CLASSES_CURRENT_USER and HKEY_CLASSES_ROOT, while the
	// 16-bit Eudora can only store the command line under
	// HKEY_CLASSES_ROOT.  The 32-bit MAPI looks for the command line
	// under "CURRENT_USER" first, then under "ROOT" if it is not
	// found (e.g.  user is running 16-bit Eudora).  The 16-bit MAPI
	// is restricted to look only under "ROOT".
	//
	HKEY hkey_eudora_cmdline = 0;
	const char* P_CLASSES_ROOT_KEY = "Software\\Qualcomm\\Eudora\\CommandLine\\Current";
	const char* P_CURRENT_USER_KEY = "Software\\Qualcomm\\Eudora\\CommandLine";

	//
	// First try ...  look in HKEY_CLASSES_ROOT first, so that we are
	// more likely to find the last run instance of 16-bit Eudora in
	// the screwy case where you run both 16-bit and 32-bit Eudora on
	// the same system.  Remember that 16-bit Eudora doesn't have the
	// ability to access HKEY_CURRENT_USER.
	//
	if (::RegOpenKey(HKEY_CLASSES_ROOT,
	        	   	 NULL,
	        	   	 &hkey_eudora_cmdline) == ERROR_SUCCESS)
	{
		//
		// Registry opened successfully, so get the data for the key.
		//
		OUTPUTDEBUGSTRING("GetCommandLine() - opened ROOT key...\n");
		LONG bufsize = _MAX_PATH * 2;	// big buffer
		if (::RegQueryValue(HKEY_CLASSES_ROOT, 
		                    P_CLASSES_ROOT_KEY,
		                    CommandLine.GetBuffer(int(bufsize)),
		                    &bufsize) == ERROR_SUCCESS)
		{
			OUTPUTDEBUGSTRING("GetCommandLine() - got ROOT command line!\n");
			CommandLine.ReleaseBuffer();
		}
		else
		{
			CommandLine.ReleaseBuffer();
			CommandLine.Empty();			// good hygiene
		}
		::RegCloseKey(hkey_eudora_cmdline);
		hkey_eudora_cmdline = 0;
	}

#ifdef WIN32
	if (CommandLine.IsEmpty())
	{
		//
		// 2nd try.  Look in HKEY_CURRENT_USER.
		//
		if (::RegOpenKeyEx(HKEY_CURRENT_USER,
					       P_CURRENT_USER_KEY,
						   0,
						   KEY_READ,
		        	       &hkey_eudora_cmdline) == ERROR_SUCCESS)
		{
			//
			// Seek to key was successful, so get the data.
			//
			OUTPUTDEBUGSTRING("GetCommandLine() - got CURRENT_USER key\n");
			DWORD bufsize = _MAX_PATH * 2;		// big buffer
			DWORD keytype;
			if (::RegQueryValueEx(hkey_eudora_cmdline, 
			                      "Current",
								  NULL,
								  &keytype,
			                      LPBYTE(CommandLine.GetBuffer(bufsize)), 
			                      &bufsize) == ERROR_SUCCESS)
			{
				CommandLine.ReleaseBuffer();
				ASSERT(REG_SZ == keytype);
				OUTPUTDEBUGSTRING("GetCommandLine() - got CURRENT_USER command line!\n");
			}
			else
			{
				CommandLine.ReleaseBuffer();
				CommandLine.Empty();				// good hygiene
			}
			::RegCloseKey(hkey_eudora_cmdline);
			hkey_eudora_cmdline = 0;
		}
	}
#endif // WIN32

	return (!CommandLine.IsEmpty());
}


////////////////////////////////////////////////////////////////////////
// GetEudoraWindowHandle [static]
//
// Returns an HWND for the running instance of Eudora.  If there is no
// running instance of Eudora, then try to auto-launch Eudora using the 
// last known good command line in the registry.
//
// If that still doesn't work and we can't find a running instance of
// Eudora, return a NULL value.
// 
////////////////////////////////////////////////////////////////////////
static HWND GetEudoraWindowHandle(HWND clientHwnd)
{
	OUTPUTDEBUGSTRING("GetEudoraWindowHandle()\n");

	static HWND hLastEudoraWnd = NULL;
	if (hLastEudoraWnd)
	{
		if (IsWindow(hLastEudoraWnd))
			return hLastEudoraWnd;
		hLastEudoraWnd = NULL;
	}

	//
	// Acquire the HWNDs of all running instances of Eudora.
	//
	CDWordArray hwnd_array;
	::EnumWindows(EnumWindowsProc, LPARAM(&hwnd_array));

	switch (hwnd_array.GetSize())
	{
	case 0:
		//
		// Need to launch Eudora ourselves, so drop through to code below...
		//
		break;
	case 1:
		hLastEudoraWnd = HWND(hwnd_array[0]);		// that was easy
		return hLastEudoraWnd;
	default:
		{
			//
			// Uh, oh.  Multiple instances of Eudora detected.  FORNOW,
			// put up an error message and bail.
			//
			CString errmsg;
			VERIFY(errmsg.LoadString(IDS_ERR_FOUND_MULTIPLE_EUDORAS));
			CString errtitle;
			VERIFY(errtitle.LoadString(IDS_EUDORA_MAPI));
			::MessageBox(clientHwnd, (const char *) errmsg, (const char *) errtitle, MB_OK | MB_ICONSTOP);
		}
		return NULL;
	}

	HWND hwnd = NULL;			// HWND for Eudora main window

	//
	// If we get this far, Eudora is not running, so attempt to fetch
	// the last known command line from the registry.
	CString cmdline;
	if (GetCommandLine(cmdline))
	{
		//
		// Got a candidate cmdline of some sort, so do the launch.
		//
		OUTPUTDEBUGSTRING("GetEudoraWindowHandle() - launching command line\n");
		switch (WinExec(cmdline, SW_SHOWNORMAL))
		{
#ifdef WIN32
		case 0:						// The system is out of memory or resources.
		case ERROR_BAD_FORMAT:		// The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).
		case ERROR_FILE_NOT_FOUND:	// The specified file was not found.
		case ERROR_PATH_NOT_FOUND:	// The specified path was not found.
#else
		case 0:		// System was out of memory, executable file was corrupt, or relocations were invalid. 
		case 2:		// File was not found. 
		case 3:		// Path was not found. 
		case 5:		// Attempt was made to dynamically link to a task, or there was a sharing or network-protection error. 
		case 6:		// Library required separate data segments for each task. 
		case 8:		// There was insufficient memory to start the application. 
		case 10:	// Windows version was incorrect. 
		case 11:	// Executable file was invalid. Either it was not a Windows application or there was an error in the .EXE image. 
		case 12:	// Application was designed for a different operating system. 
		case 13:	// Application was designed for MS-DOS 4.0. 
		case 14:	// Type of executable file was unknown. 
		case 15:	// Attempt was made to load a real-mode application (developed for an earlier version of Windows). 
		case 16:	// Attempt was made to load a second instance of an executable file containing multiple data segments that were not marked read-only. 
		case 19:	// Attempt was made to load a compressed executable file. The file must be decompressed before it can be loaded. 
		case 20:	// Dynamic-link library (DLL) file was invalid. One of the DLLs required to run this application was corrupt. 
		case 21:	// Application requires Microsoft Windows 32-bit extensions. 
#endif // WIN32
			{
				//
				// WinExec() not successful, so post error dialog.
				//
				CString errmsg;
				VERIFY(errmsg.LoadString(IDS_ERR_COULD_NOT_LAUNCH_PROGRAM));
				errmsg += ": ";
				errmsg += cmdline;

				CString errtitle;
				VERIFY(errtitle.LoadString(IDS_EUDORA_MAPI));
				::MessageBox(clientHwnd, (const char *) errmsg, (const char *) errtitle, MB_OK | MB_ICONSTOP);
			}
			return NULL;
		default:
			break;
		}
	
		//
		// If we get this far, the launch was apparently successful,
		// so let's wait for Eudora to show up.  If she doesn't show
		// up after 60 seconds, then give up.
		// 
		int retry_count = 120;
		while (retry_count > 0)
		{
			if ((hwnd = ::FindWindow(EudoraMainWindowClass, NULL)) != NULL)
				break;			// She's alive...
			else
			{
#ifdef WIN32
				Sleep(500);	// 500ms
#else
				//
				// Stall this thread for about 0.5 second (on average).  
				// The granularity of the 'time()' function is one second.
				//
				time_t start_time, now;
				time(&start_time);
				now = start_time;
				while (start_time != now)
				{
					//
					// So if you're paying attention here, you're probably
					// wondering why the hell we're making a DDE call
					// and then doing nothing with the returned string.
					// It turns out all we want to do here is to stall this
					// thread for awhile and give Eudora a chance to come
					// up, and the DDEML library has some very nice ways
					// to cleanly stall and cooperatively let other
					// applications get some processing time.  Cool, huh?
					//
					CDDEClient dde_client;
					CString unused;
					dde_client.GetEudoraMAPIServerVersion(unused);
					time(&now);
				}
#endif
			}
			OUTPUTDEBUGSTRING("GetEudoraWindowHandle() - FindWindow() failed, retrying...\n");
			retry_count--;
		}
	}
	
	if (NULL == hwnd)
	{
		CString errmsg;
		VERIFY(errmsg.LoadString(IDS_ERR_CANNOT_FIND_EUDORA));
		CString errtitle;
		VERIFY(errtitle.LoadString(IDS_WHERE_IS_EUDORA));
		::MessageBox(clientHwnd, (const char *) errmsg, (const char *) errtitle, MB_OK | MB_ICONSTOP);
	}

	hLastEudoraWnd = hwnd;

	return hLastEudoraWnd;
}


////////////////////////////////////////////////////////////////////////
// UseShortFilenames [static]
//
// Returns TRUE or FALSE as follows:
//
// MAPI DLL     Eudora      Returns
// --------     ------      -------
// 16-bit       unknown     TRUE
// 16-bit       16-bit      TRUE
// 16-bit       32-bit      TRUE
// 32-bit       unknown     TRUE
// 32-bit       16-bit      TRUE
// 32-bit       32-bit      FALSE
// 
////////////////////////////////////////////////////////////////////////
static BOOL UseShortFilenames()
{
#ifdef WIN32
	// We're just going to assume that if you're using the 32-bit MAPI client that
	// you're also using the 32-bit Eudora.  What's the likelihood that someone is
	// running a 3.x 16-bit Eudora with this 4.x 32-bit MAPI dll.  It saves us from
	// making the DDE calls to get the Eudora version, which sometimes hangs.
	return FALSE;
#else
	//
	// 16-bit DLL should always send short filename, by definition.
	//
	return TRUE;
#endif // WIN32
}


////////////////////////////////////////////////////////////////////////
// IsEudoraMapiDLL [extern]
//
////////////////////////////////////////////////////////////////////////
extern "C" BOOL FAR PASCAL IsEudoraMapiDLL(void)
{
	OUTPUTDEBUGSTRING("IsEudoraMapiDLL()\n");

	return TRUE;
}

DWORD g_dwAuthentication = 0;

////////////////////////////////////////////////////////////////////////
// IsEudoraMapi [extern]
//
// pAuthentication - Parameter which the MAPI client uses to authenticate
// with the MAPI server, and vice versa.  The client will pass in a pointer
// to a DWORD that contains a non-zero multiple of 8467 in order to
// authenticate to the server.  If the parameter has such a value, then the
// MAPI server will allow messages to be sent via MAPISendMail() without a
// warning to the user.  Also if the parameter has such a value, then the
// MAPI server will fill the value with a non-zero multiple of 9697 in
// order to authenticate to the client.
//
// pDesktopMode - If not NULL, the function will put in one of the following values:
//    0 - Desktop Eudora is in Ad/Sponsored mode
//    1 - Desktop Eudora is in Light mode
//    2 - Desktop Eudora is in Paid mode
//    0xFFFFFFFF - Desktop Eudora version is earlier than 4.3 or cannot tell mode
//
// Reserved - Not used for now.  In the future, may be a regcode structure
// that can be filled out with regcode, and first and last name of Desktop Eudora.
////////////////////////////////////////////////////////////////////////
extern "C" VOID FAR PASCAL IsEudoraMapi(LPDWORD pAuthentication, LPDWORD pDesktopMode, LPVOID Reserved)
{
	UNREFERENCED_PARAMETER(Reserved);

	OUTPUTDEBUGSTRING("IsEudoraMapi()\n");

	if (pAuthentication)
	{
		if (*pAuthentication && ((*pAuthentication % 8467) == 0))
		{
			g_dwAuthentication = *pAuthentication;

			srand(GetTickCount());
			*pAuthentication = ((rand() & 0x7FFF) + 1) * 9697;
		}
	}

	if (pDesktopMode)
	{
		// Default the value to "couldn't figure out the mode"
		*pDesktopMode = 0xFFFFFFFF;

		CString CommandLine;
		if (GetCommandLine(CommandLine))
		{
			// First command-line argument will be the program location
			// Last command-line argument will be the INI file location
			LPCTSTR Program = NULL;
			LPCTSTR IniFile = NULL;
			LPTSTR Command = CommandLine.GetBuffer(CommandLine.GetLength());

			// Need to do INI before Program because getting Program will truncate string
			if (Command[strlen(Command) - 1] == '"')
			{
				// Get rid of trailing quote and point just after previous quote
				Command[strlen(Command) - 1] = 0;
				char* Quote = strrchr(Command, '"');
				if (Quote)
					IniFile = Quote + 1;
			}
			else
			{
				char* Space = strrchr(Command, ' ');
				if (Space)
					IniFile = Space + 1;
			}

			if (Command[0] == '"')
			{
				Program = Command + 1;
				char* Quote = strchr(Command, '"');
				if (Quote)
					*Quote = 0;
			}
			else
			{
				Program = Command;
				char* Space = strchr(Command, ' ');
				if (Space)
					*Space = 0;
			}

			if (!Program || !IniFile)
				ASSERT(0);	// What happened?
			else
			{
				// We need to make sure this is at least the 4.3 version of Eudora because
				// mode was introduced in version 4.3.
				TCHAR szString[1024] = {0};
				DWORD dwHandle = 0;
				DWORD dwSize = GetFileVersionInfoSize((LPTSTR)Program, &dwHandle);
				
				if (dwSize)
				{
					LPSTR pData = new char[dwSize];
					VS_FIXEDFILEINFO* pFileInfo = NULL;
					UINT uBufSize = 0;

					if (pData && GetFileVersionInfo((LPTSTR)Program, dwHandle, dwSize, pData)) 
					{
						//
						// Okay, you would think that passing a raw string literal to an
						// argument that takes a string pointer would be okay.  Well, not
						// if you're running the 16-bit VerQueryValue() function under
						// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
						// the workaround is to copy the string to a local buffer first,
						// then pass the string in the buffer.  This, for some inexplicable
						// reason, works under all Windows OSes.
						//
						strcpy(szString, "\\");
						
						if (VerQueryValue(pData, szString, (void **)&pFileInfo, &uBufSize) && uBufSize && pFileInfo)
						{
							// now check the version
							int MajorVersion = pFileInfo->dwFileVersionMS >> 16;
							int MinorVersion = pFileInfo->dwFileVersionMS & 0xFFFF;
							if (MajorVersion > 4 || (MajorVersion == 4 && MinorVersion >= 3))
							{
								*pDesktopMode = GetPrivateProfileInt("Settings", "Mode", 0, IniFile);
							}
						}
					}

					delete [] pData;
				}
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////
// MAPILogon [extern]
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPILogon(
	ULONG ulUIParam, 
	LPSTR lpszProfileName,
	LPSTR lpszPassword, 
	FLAGS flFlags, 
	ULONG ulReserved, 
	LPLHANDLE lplhSession)
{
	OUTPUTDEBUGSTRING("MAPILogon calling MAPILogonA\n");

	return MAPILogonA(	ulUIParam,
						lpszProfileName,
						lpszPassword, 
						flFlags, 
						ulReserved, 
						lplhSession);
}


////////////////////////////////////////////////////////////////////////
// MAPILogonA [extern]
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPILogonA(
	ULONG ulUIParam, 
	LPSTR lpszProfileName,
	LPSTR lpszPassword, 
	FLAGS flFlags, 
	ULONG ulReserved, 
	LPLHANDLE lplhSession)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(lpszProfileName);
	UNREFERENCED_PARAMETER(lpszPassword);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	CString buf;
	buf.Format("MAPILogonA:\n"
			   "  ulUIParam = %lu\n"
			   "  lpszProfileName = %s\n"
			   "  lpszPassword = %s\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n"
			   "  lplhSession = <%p>\n",
			   ulUIParam, 
			   SAFESTRING(lpszProfileName), 
			   SAFESTRING(lpszPassword), 
			   flFlags,
			   ulReserved,
			   lplhSession);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
	
	if (NULL == lplhSession)
	{
		ASSERT(0);
		return MAPI_E_FAILURE;
	}

	//
	// As a precaution, start off with a bogus session ID.
	//
	*lplhSession = 0;

	//
	// Acquire HWND of running instance of Eudora, launching Eudora
	// first, if necessary.
	//
	HWND hWnd = GetEudoraWindowHandle(HWND(ulUIParam));
	if (NULL == hWnd)
		return MAPI_E_FAILURE;

	//
	// Eudora's implementation always creates a new session since it
	// doesn't do implicit logons ... only the MAPI client apps call
	// MAPILogon() ... so if that's what they want, that's what they're
	// gonna get.
	//
	*lplhSession = s_SessionMgr.CreateSession();
	ASSERT(*lplhSession > 0);

	//
	// Check for the "check mail" flag bit.
	//
	if (flFlags & MAPI_FORCE_DOWNLOAD)
	{
		//
		// Send a message to Eudora to force it to check for new mail.
		//
		ASSERT(hWnd != NULL);
		CString buf("CHEK: \n\n");
		COPYDATASTRUCT cds;
		
		cds.dwData = EUM_CHECK_MAIL;
		cds.cbData = buf.GetLength() + 1;		// add one for terminating NULL
		cds.lpData = buf.GetBuffer(int(cds.cbData));
		SendMessage(hWnd, WM_COPYDATA, 0, LPARAM(&cds));
		buf.ReleaseBuffer();
		OUTPUTDEBUGSTRING("MAPILogonA: returned from SendMessage()\n");
	}
	
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPILogoff
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPILogoff(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	FLAGS flFlags, 
	ULONG ulReserved)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulUIParam);
	UNREFERENCED_PARAMETER(flFlags);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	CString buf;
	buf.Format("MAPILogoff:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n",
			   lhSession,
			   ulUIParam, 
			   flFlags, 
			   ulReserved);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	if (lhSession)
	{
		if (! s_SessionMgr.IsValidSessionId(lhSession))
			return MAPI_E_INVALID_SESSION;
		if (! s_SessionMgr.DestroySession(lhSession))
		{
			ASSERT(0);
			return MAPI_E_FAILURE;
		}
	}
	
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPISendMail
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPISendMail(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	lpMapiMessage lpMessage, 
	FLAGS flFlags, 
	ULONG ulReserved)
{
	OUTPUTDEBUGSTRING("MAPISendMail calling MAPISendMailA\n");

	return MAPISendMailA(	lhSession, 
							ulUIParam,
							lpMessage, 
							flFlags, 
							ulReserved);
}


////////////////////////////////////////////////////////////////////////
// MAPISendMailA
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPISendMailA(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	lpMapiMessage lpMessage, 
	FLAGS flFlags, 
	ULONG ulReserved)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	{
		CString buf;
		ASSERT(lpMessage != NULL);
		buf.Format("MAPISendMailA:\n"
				   "  lhSession = %lu\n"
				   "  ulUIParam = %lu\n"
				   "  flFlags = %#010lx\n"
				   "  lpMessage = <%p>\n",
				   lhSession,
				   ulUIParam,
				   flFlags,
				   lpMessage);
		OUTPUTDEBUGSTRING(buf);
		if (lpMessage)
		{
			ULONG idx;
			CString buf;
	
			buf.Format("  lpMessage.lpszSubject = \"%s\"\n", SAFESTRING(lpMessage->lpszSubject));
			OUTPUTDEBUGSTRING(buf);
			buf.Format("  lpMessage.lpszNoteText = \"%s\"\n", SAFESTRING(lpMessage->lpszNoteText));
			OUTPUTDEBUGSTRING(buf);
			buf.Format("  lpMessage.lpszMessageType = \"%s\"\n", SAFESTRING(lpMessage->lpszMessageType));
			OUTPUTDEBUGSTRING(buf);
			buf.Format("  lpMessage.lpszDateReceived = \"%s\"\n", SAFESTRING(lpMessage->lpszDateReceived));
			OUTPUTDEBUGSTRING(buf);
			buf.Format("  lpMessage.lpOriginator = <%p>\n", lpMessage->lpOriginator);
			OUTPUTDEBUGSTRING(buf);
			if (lpMessage->lpOriginator)
			{
				buf.Format("  lpMessage.lpOriginator.ulRecipClass = %lu\n", lpMessage->lpOriginator->ulRecipClass);
				OUTPUTDEBUGSTRING(buf);
				ASSERT(lpMessage->lpOriginator->lpszName != NULL);
				buf.Format("  lpMessage.lpOriginator.lpszName = \"%s\"\n", SAFESTRING(lpMessage->lpOriginator->lpszName));
				OUTPUTDEBUGSTRING(buf);
				buf.Format("  lpMessage.lpOriginator.lpszAddress = \"%s\"\n", SAFESTRING(lpMessage->lpOriginator->lpszAddress));
				OUTPUTDEBUGSTRING(buf);
			}
	
			buf.Format("  lpMessage.nRecipCount = %lu\n", lpMessage->nRecipCount);
			OUTPUTDEBUGSTRING(buf);
			for (idx = 0; idx < lpMessage->nRecipCount; idx++)
			{
				ASSERT(lpMessage->lpRecips);
				buf.Format("    lpMessage.lpRecips[%lu].ulRecipClass = %lu\n", idx, lpMessage->lpRecips[idx].ulRecipClass);
				OUTPUTDEBUGSTRING(buf);
				ASSERT(lpMessage->lpRecips->lpszName);                                             
				buf.Format("    lpMessage.lpRecips[%lu].lpszName = \"%s\"\n", idx, SAFESTRING(lpMessage->lpRecips[idx].lpszName));
				OUTPUTDEBUGSTRING(buf);
				buf.Format("    lpMessage.lpRecips[%lu].lpszAddress = \"%s\"\n", idx, SAFESTRING(lpMessage->lpRecips[idx].lpszAddress));
				OUTPUTDEBUGSTRING(buf);
			}
	
			buf.Format("  lpMessage.nFileCount = %lu\n", lpMessage->nFileCount);
			OUTPUTDEBUGSTRING(buf);
			for (idx = 0; idx < lpMessage->nFileCount; idx++)
			{
				ASSERT(lpMessage->lpFiles);
				buf.Format("    lpMessage.lpFiles[%lu].nPosition = %lu\n", idx, lpMessage->lpFiles[idx].nPosition);
				OUTPUTDEBUGSTRING(buf);
				ASSERT(lpMessage->lpFiles[idx].lpszPathName);                                             
				buf.Format("    lpMessage.lpFiles[%lu].lpszPathName = \"%s\"\n", idx, SAFESTRING(lpMessage->lpFiles[idx].lpszPathName));
				OUTPUTDEBUGSTRING(buf);
				buf.Format("    lpMessage.lpFiles[%lu].lpszFileName = \"%s\"\n", idx, SAFESTRING(lpMessage->lpFiles[idx].lpszFileName));
				OUTPUTDEBUGSTRING(buf);
			}
		}
	
		buf.Format("  ulReserved = %lu\n", ulReserved);
		OUTPUTDEBUGSTRING(buf);
	}
#endif // _DEBUG

	if (lhSession)
	{
		if (! s_SessionMgr.IsValidSessionId(lhSession))
			return MAPI_E_INVALID_SESSION;
	}
	
	if (NULL == lpMessage)
		return MAPI_E_FAILURE;

	//
	// Check for the case where the caller wants to auto-send
	// the message, but didn't specify any recipients (duh).  In
	// this case, just punt.
	//
	if ((0 == lpMessage->nRecipCount) &&
		(0 == (flFlags & MAPI_DIALOG)))
	{
		return MAPI_E_UNKNOWN_RECIPIENT;
	}

	HWND hWnd = GetEudoraWindowHandle(HWND(ulUIParam));
	if (NULL == hWnd)
		return MAPI_E_FAILURE;		// Eudora is not running

	//
	// Determine what type of Eudora we're talking to (16 vs 32) and
	// decide whether or not to use short attachment file names.
	//
	BOOL use_short_filenames = UseShortFilenames();

	//
	// Write the MapiMessage record data into a simple, line-oriented
	// textual format that Eudora can understand.
	//
	CString buf;
	BOOL want_auto_send = (flFlags & MAPI_DIALOG) ? FALSE : TRUE;
	if (! ((CMapiMessage *) lpMessage)->WriteMessageData(buf, want_auto_send, use_short_filenames))
		return MAPI_E_TEXT_TOO_LARGE;

#ifdef _DEBUG
	OUTPUTDEBUGSTRING("MAPISendMailA: --- buf[] contents ---\n");
	OUTPUTDEBUGSTRING(buf);
	OUTPUTDEBUGSTRING("MAPISendMailA: --- buf[] contents ---\n");
#endif // _DEBUG

	//
	// Transmit the message data to Eudora via WM_COPYDATA.
	//
	COPYDATASTRUCT cds;
	cds.dwData = EUM_SEND_MAIL;
	cds.cbData = buf.GetLength() + 1;		// add one for terminating NULL
	cds.lpData = buf.GetBuffer(int(cds.cbData));
	SendMessage(hWnd, WM_COPYDATA, 0, LPARAM(&cds));
	buf.ReleaseBuffer();
	OUTPUTDEBUGSTRING("MAPISendMailA: returned from SendMessage()\n");

	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPISendDocuments
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPISendDocuments(
	ULONG ulUIParam, 
	LPSTR lpszDelimChar,
	LPSTR lpszFilePaths, 
	LPSTR lpszFileNames, 
	ULONG ulReserved)
{
	OUTPUTDEBUGSTRING("MAPISendDocuments calling MAPISendDocumentsA\n");

	return MAPISendDocumentsA(	ulUIParam, 
								lpszDelimChar,
								lpszFilePaths, 
								lpszFileNames, 
								ulReserved);
}


////////////////////////////////////////////////////////////////////////
// MAPISendDocumentsA
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPISendDocumentsA(
	ULONG ulUIParam, 
	LPSTR lpszDelimChar,
	LPSTR lpszFilePaths, 
	LPSTR lpszFileNames, 
	ULONG ulReserved)
{
	ASSERT(lpszDelimChar != NULL);
	ASSERT(lpszFilePaths != NULL);
	ASSERT(lpszFileNames != NULL);

#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulReserved);
#else
	CString buf;
	buf.Format("MAPISendDocumentsA:\n"
			   "  ulUIParam = %lu\n"
			   "  lpszDelimChar = %c\n"
			   "  lpszFilePaths = \"%s\"\n"
			   "  lpszFileNames = \"%s\"\n"
			   "  ulReserved = %ul\n",
			   ulUIParam, 
			   lpszDelimChar[0], 
			   SAFESTRING(lpszFilePaths), 
			   SAFESTRING(lpszFileNames),
			   ulReserved);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
	
	HWND hWnd = GetEudoraWindowHandle(HWND(ulUIParam));
	if (NULL == hWnd)
		return MAPI_E_FAILURE;		// Eudora is not running

	CString databuf;					// outgoing data buffer
	CString pathnames(lpszFilePaths);	// working copy of delimited pathnames string
	CString filenames(lpszFileNames);	// working copy of delimited filenames string
	char delim = lpszDelimChar[0];

	BOOL use_short_filenames = UseShortFilenames();

	//
	// Walk through the delimited pathname and filename strings
	// in lockstep, processing one pair at a time.
	//
	while (! pathnames.IsEmpty())
	{
		int pathnames_idx = pathnames.Find(delim);
		int filenames_idx = filenames.Find(delim);

		//
		// Extract the next pathname/filename pair from the
		// delimited strings.
		//
		CString pathname;
		CString filename;
		if (pathnames_idx != -1)
		{
			//
			// Found a delimiter, so copy only the part of the
			// string upto the delimiter.
			//
			ASSERT(filenames_idx != -1);
			pathname = pathnames.Left(pathnames_idx);
			filename = filenames.Left(filenames_idx);

			//
			// Remove the leading strings upto and including the
			// delimiter.
			//
			pathnames = pathnames.Right(pathnames.GetLength() - (pathnames_idx + 1));
			filenames = filenames.Right(filenames.GetLength() - (filenames_idx + 1));
		}
		else
		{
			//
			// No more delimeters, so use the whole strings.
			//
			ASSERT(filenames_idx == -1);
			pathname = pathnames;
			filename = filenames;

			pathnames = "";
			filenames = "";
		}

		//
		// At this point, 'filename' and 'pathname' contain the
		// strings we want, so add them to the outgoing data
		// buffer with appropriate keywords and syntax.
		//
		if (! filename.IsEmpty())
		{
			ASSERT(filename.Find('\n') == -1);
			ASSERT(filename.Find(';') == -1);
			databuf += "FILE: " + filename + "\n";
		}

		ASSERT(pathname.Find('\n') == -1);
		ASSERT(pathname.Find(';') == -1);

		if (use_short_filenames)
		{
#ifdef WIN32
			CString shortpath;
			const int PATHLEN = strlen(pathname) + 32;		// slop factor
			char* p_path = shortpath.GetBuffer(PATHLEN);
			if (p_path)
			{
				DWORD dwStatus = ::GetShortPathName(pathname, p_path, PATHLEN);
				shortpath.ReleaseBuffer();
				if (dwStatus && (int(dwStatus) < PATHLEN))
					pathname = shortpath;
				else
					ASSERT(0);
			}
#endif
		}
		databuf += "PATH: " + pathname + "\n";
	}

	//
	// Add a final empty line to signify the end of the data buffer.
	//
	databuf += "\n";

	COPYDATASTRUCT cds;
	
	cds.dwData = EUM_SEND_DOCUMENTS;
	cds.cbData = databuf.GetLength() + 1;		// add one for NULL terminator
	cds.lpData = databuf.GetBuffer(int(cds.cbData));
	SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
	databuf.ReleaseBuffer();
	OUTPUTDEBUGSTRING("MAPISendDocumentsA: returned from SendMessage()\n");

	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPIFindNext
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIFindNext(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	LPSTR lpszMessageType, 
	LPSTR lpszSeedMessageID, 
	FLAGS flFlags,
	ULONG ulReserved, 
	LPSTR lpszMessageID)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulUIParam);
	UNREFERENCED_PARAMETER(lpszMessageType);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	ASSERT(lpszMessageID != NULL);
	buf.Format("MAPIFindNext:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpszMessageType = \"%s\"\n"
			   "  lpszSeedMessageID = \"%s\"\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n"
			   "  lpszMessageID = <%p>\"\n",
			   lhSession,
			   ulUIParam,
			   SAFESTRING(lpszMessageType),
			   SAFESTRING(lpszSeedMessageID),
			   flFlags,
			   ulReserved,
			   lpszMessageID);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	if (NULL == lpszMessageID)
		return MAPI_E_FAILURE;		// caller blew it
	else if (0 == lhSession)
		return MAPI_E_INVALID_SESSION;
	else if (! s_SessionMgr.IsValidSessionId(lhSession))
		return MAPI_E_INVALID_SESSION;

	//
	// Set some MAPIFindNext() flags.
	//
	const BOOL SORT_BY_DATE = (flFlags & MAPI_GUARANTEE_FIFO) ? TRUE : FALSE;
	const BOOL UNREAD_ONLY = (flFlags & MAPI_UNREAD_ONLY) ? TRUE : FALSE;

	//
	// Set up the message id.  An empty message id is valid and indicates
	// that we need to retrieve the first message id.
	//
	CString message_id;
	ASSERT(message_id.IsEmpty());
	if (lpszSeedMessageID && (strlen(lpszSeedMessageID) > 0))
		message_id = lpszSeedMessageID;

	//
	// Okay, now we're all set to ask Eudora for the next Inbox message
	// identifier via DDE.
	//
	CDDEClient dde_client;
	if (! dde_client.GetNextMessageId(message_id, SORT_BY_DATE, UNREAD_ONLY))
	{
		strcpy(lpszMessageID, "");		// good hygiene
		return MAPI_E_NO_MESSAGES;
	}

	ASSERT(message_id.GetLength() > 0 && message_id.GetLength() < 64);
	strcpy(lpszMessageID, message_id);
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPIReadMail
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIReadMail(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	LPSTR lpszMessageID, 
	FLAGS flFlags, 
	ULONG ulReserved,
	lpMapiMessage FAR *lppMessage)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulUIParam);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	ASSERT(lppMessage != NULL);
	buf.Format("MAPIReadMail:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpszMessageID = \"%s\"\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n"
			   "  lppMessage = <%p>\n"
			   "  *lppMessage = <%p>\n",
			   lhSession,
			   ulUIParam,
			   SAFESTRING(lpszMessageID),
			   flFlags,
			   ulReserved,
			   lppMessage,
			   *lppMessage);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// Validate the session ID, message ID, and flags.
	//
	if (NULL == lppMessage)
		return MAPI_E_FAILURE;		// caller blew it
	else if (0 == lhSession)
		return MAPI_E_INVALID_SESSION;
	else if (! s_SessionMgr.IsValidSessionId(lhSession))
		return MAPI_E_INVALID_SESSION;
	else if (NULL == lpszMessageID)
		return MAPI_E_INVALID_MESSAGE;

	//
	// Set some MAPIReadMail() flags.
	//
	const BOOL BODY_AS_FILE = (flFlags & MAPI_BODY_AS_FILE) ? TRUE : FALSE;
	const BOOL ENVELOPE_ONLY = (flFlags & MAPI_ENVELOPE_ONLY) ? TRUE : FALSE;
	const BOOL MARK_AS_READ = (flFlags & MAPI_PEEK) ? FALSE : TRUE;
	const BOOL WANT_ATTACHMENTS = (flFlags & MAPI_SUPPRESS_ATTACH) ? FALSE : TRUE;

	//
	// Fetch the message data corresponding to the given message id via
	// DDE to Eudora.
	//
	CDDEClient dde_client;
	CString message_data;
	if (! dde_client.GetMessageById(message_data, lpszMessageID, BODY_AS_FILE, ENVELOPE_ONLY, MARK_AS_READ, WANT_ATTACHMENTS))
		return MAPI_E_FAILURE;

	//
	// Now, we need to parse the message data buffer and place the data
	// in a MapiMessage record to be returned to the caller.
	//
	CMapiMessage* p_msg = s_SessionMgr.NewMapiMessage();
	if (NULL == p_msg)
		return MAPI_E_INSUFFICIENT_MEMORY;

	if (! p_msg->ReadMessageData(message_data))
	{
		s_SessionMgr.FreeBuffer(p_msg);
		*lppMessage = NULL;
		return MAPI_E_FAILURE;
	}

	//
	// If we get this far, then the message record was setup successfully,
	// so return it to the caller.
	//
	*lppMessage = p_msg;
#ifdef _DEBUG
	buf.Format("  returning *lppMessage = <%p>\n", *lppMessage);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPISaveMail
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPISaveMail(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	lpMapiMessage lpMessage, 
	FLAGS flFlags, 
	ULONG ulReserved,
	LPSTR lpszMessageID)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(flFlags);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	ASSERT(lpMessage != NULL);
	buf.Format("MAPISaveMail:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpMessage = <%p>\n",
			   lhSession,
			   ulUIParam,
			   lpMessage);
	OUTPUTDEBUGSTRING(buf);
	if (lpMessage)
	{
		ULONG idx;

		buf.Format("  lpMessage.lpszSubject = \"%s\"\n", SAFESTRING(lpMessage->lpszSubject));
		OUTPUTDEBUGSTRING(buf);
		buf.Format("  lpMessage.lpszNoteText = \"%s\"\n", SAFESTRING(lpMessage->lpszNoteText));
		OUTPUTDEBUGSTRING(buf);
		buf.Format("  lpMessage.lpszMessageType = \"%s\"\n", SAFESTRING(lpMessage->lpszMessageType));
		OUTPUTDEBUGSTRING(buf);
		buf.Format("  lpMessage.lpszDateReceived = \"%s\"\n", SAFESTRING(lpMessage->lpszDateReceived));
		OUTPUTDEBUGSTRING(buf);

		buf.Format("  lpMessage.nRecipCount = %lu\n", lpMessage->nRecipCount);
		OUTPUTDEBUGSTRING(buf);
		for (idx = 0; idx < lpMessage->nRecipCount; idx++)
		{
			ASSERT(lpMessage->lpRecips);
			buf.Format("    lpMessage.lpRecips[%lu].ulRecipClass = %lu\n", idx, lpMessage->lpRecips[idx].ulRecipClass);
			OUTPUTDEBUGSTRING(buf);
			ASSERT(lpMessage->lpRecips->lpszName);                                             
			buf.Format("    lpMessage.lpRecips[%lu].lpszName = \"%s\"\n", idx, SAFESTRING(lpMessage->lpRecips[idx].lpszName));
			OUTPUTDEBUGSTRING(buf);
			buf.Format("    lpMessage.lpRecips[%lu].lpszAddress = \"%s\"\n", idx, SAFESTRING(lpMessage->lpRecips[idx].lpszAddress));
			OUTPUTDEBUGSTRING(buf);
		}

		buf.Format("  lpMessage.nFileCount = %lu\n", lpMessage->nFileCount);
		OUTPUTDEBUGSTRING(buf);
		for (idx = 0; idx < lpMessage->nFileCount; idx++)
		{
			ASSERT(lpMessage->lpFiles);
			buf.Format("    lpMessage.lpFiles[%lu].nPosition = %lu\n", idx, lpMessage->lpFiles[idx].nPosition);
			OUTPUTDEBUGSTRING(buf);
			ASSERT(lpMessage->lpFiles[idx].lpszPathName);                                             
			buf.Format("    lpMessage.lpFiles[%lu].lpszPathName = \"%s\"\n", idx, SAFESTRING(lpMessage->lpFiles[idx].lpszPathName));
			OUTPUTDEBUGSTRING(buf);
			buf.Format("    lpMessage.lpFiles[%lu].lpszFileName = \"%s\"\n", idx, SAFESTRING(lpMessage->lpFiles[idx].lpszFileName));
			OUTPUTDEBUGSTRING(buf);
		}
	}

	ASSERT(lpszMessageID != NULL);
	buf.Format("  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n"
			   "  lpszMessageID = \"%s\"\n",
			   flFlags,
			   ulReserved,
			   SAFESTRING(lpszMessageID));
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// Check some preconditions.
	//
	if (NULL == lpszMessageID)
		return MAPI_E_FAILURE;			// caller blew it
	else if (strlen(lpszMessageID) > 0)
		return MAPI_E_NOT_SUPPORTED;	// Eudora doesn't support overwrite of existing messages
	else if (NULL == lpMessage)
		return MAPI_E_FAILURE;			// caller blew it

	//
	// Make sure we have a valid session ID and message ID pair.  A
	// zero session ID indicates we want an implicit logon, however an
	// implicit logon is allowed only if the message ID is empty.
	//
	CString message_id(lpszMessageID);
	if (0 == lhSession)
	{
		//
		// Well, we don't do implicit logons, so let's just make sure
		// that Eudora is around.
		//
		ASSERT(message_id.IsEmpty());
		HWND hWnd = GetEudoraWindowHandle(HWND(ulUIParam));
		if (NULL == hWnd)
			return MAPI_E_FAILURE;
	}
	else if (! s_SessionMgr.IsValidSessionId(lhSession))
		return MAPI_E_INVALID_SESSION;

	//
	// Determine what type of Eudora we're talking to (16 vs 32) and
	// decide whether or not to use short attachment file names.
	//
	BOOL use_short_filenames = UseShortFilenames();

	//
	// Convert the MapiMessage data into a simple, line-oriented textual
	// format that Eudora can understand.
	//
	CString message_data;
	if (! ((CMapiMessage *) lpMessage)->WriteMessageData(message_data, FALSE, use_short_filenames))
		return MAPI_E_FAILURE;

	//
	// Send the message data with the given message id to Eudora via DDE.
	//
	CDDEClient dde_client;
	if (! dde_client.PutMessageById(message_data, message_id))
		return MAPI_E_FAILURE;

	ASSERT(! message_id.IsEmpty());
	strcpy(lpszMessageID, message_id);
#ifdef _DEBUG
	buf.Format("  returning lpszMessageID = %s\n", lpszMessageID);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPIDeleteMail
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIDeleteMail(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	LPSTR lpszMessageID, 
	FLAGS flFlags, 
	ULONG ulReserved)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulUIParam);
	UNREFERENCED_PARAMETER(flFlags);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	ASSERT(lpszMessageID != NULL);
	buf.Format("MAPIDeleteMail:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpszMessageID = \"%s\"\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n",
			   lhSession,
			   ulUIParam,
			   SAFESTRING(lpszMessageID),
			   flFlags,
			   ulReserved);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// Validate the session ID.
	//
	if (0 == lhSession)
		return MAPI_E_INVALID_SESSION;
	else if (! s_SessionMgr.IsValidSessionId(lhSession))
		return MAPI_E_INVALID_SESSION;
	else if (NULL == lpszMessageID)
		return MAPI_E_INVALID_MESSAGE;

	//
	// Use DDE to tell Eudora to delete the message.
	//
	CDDEClient dde_client;
	if (! dde_client.DeleteMessageById(lpszMessageID))
		return MAPI_E_FAILURE;

#ifdef _DEBUG
	buf.Format("  deleted lpszMessageID = %s successfully\n", lpszMessageID);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPIFreeBuffer
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIFreeBuffer(LPVOID pv)
{
#ifdef _DEBUG
	CString buf;
	buf.Format("MAPIFreeBuffer:\n"
			   "  pv = <%p>\n", pv);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// This function returns zero on success, else the value of the
	// pointer on failure.  Note that it is okay to have a NULL value
	// for 'pv' ...  this is treated as a successful no-op.
	//
	if (pv != NULL)
	{
		if (s_SessionMgr.FreeBuffer(pv))
			pv = NULL;		// success!
	}

	return ULONG(pv);
}


////////////////////////////////////////////////////////////////////////
// MAPIAddress
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIAddress(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	LPSTR lpszCaption, 
	ULONG nEditFields, 
	LPSTR lpszLabels, 
	ULONG nRecips,
	lpMapiRecipDesc lpRecips, 
	FLAGS flFlags, 
	ULONG ulReserved,
	LPULONG lpnNewRecips, 
	lpMapiRecipDesc FAR *lppNewRecips)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(lpszLabels);
	UNREFERENCED_PARAMETER(flFlags);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	buf.Format("MAPIAddress:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpszCaption = \"%s\"\n"
			   "  nEditFields = %lu\n"
			   "  lpszLabels = \"%s\"\n",
			   lhSession,
			   ulUIParam,
			   SAFESTRING(lpszCaption),
			   nEditFields,
			   SAFESTRING(lpszLabels));
	OUTPUTDEBUGSTRING(buf);

	buf.Format("  nRecips = %lu\n", nRecips);
	OUTPUTDEBUGSTRING(buf);
	for (ULONG idx = 0; idx < nRecips; idx++)
	{
		ASSERT(lpRecips);
		buf.Format("    lpRecips[%lu].ulRecipClass = %lu\n", idx, lpRecips[idx].ulRecipClass);
		OUTPUTDEBUGSTRING(buf);
		ASSERT(lpRecips->lpszName);                                             
		buf.Format("    lpRecips[%lu].lpszName = \"%s\"\n", idx, SAFESTRING(lpRecips[idx].lpszName));
		OUTPUTDEBUGSTRING(buf);
		buf.Format("    lpRecips[%lu].lpszAddress = \"%s\"\n", idx, SAFESTRING(lpRecips[idx].lpszAddress));
		OUTPUTDEBUGSTRING(buf);
	}

	ASSERT(lpnNewRecips != NULL);
	ASSERT(lppNewRecips != NULL);
	buf.Format("  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n"
			   "  lpnNewRecips = <%p>\n"
			   "  lppNewRecips = <%p>\n"
			   "  *lppNewRecips = <%p>\n",
			   flFlags,
			   ulReserved,
			   lpnNewRecips,
			   lppNewRecips,
			   *lppNewRecips);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// Validate the session ID and other stuff.
	//
	if (NULL == lpnNewRecips)
		return MAPI_E_FAILURE;		// caller blew it
	else if (NULL == lppNewRecips)
		return MAPI_E_FAILURE;		// caller blew it
	else if (lhSession && (! s_SessionMgr.IsValidSessionId(lhSession)))
		return MAPI_E_INVALID_SESSION;

	//
	// For good hygiene...
	//
	*lpnNewRecips = 0L;
	*lppNewRecips = NULL;

	//
	// Validate the nEditFields value.
	//
	switch (nEditFields)
	{
	case -1:	// use default
	case 4:		// use default
	case 0:		// allow browsing only
	case 1:		// allow TO only
	case 2:		// allow TO and CC
	case 3:		// allow TO, CC, and BCC
		break;
	default:
		return MAPI_E_INVALID_EDITFIELDS;
	}

	//
	// Create address dialog.  FORNOW, not using (or error-checking)
	// lpszCaption, nEditFields, and lpszLabels on the dialog.
	//
	AfxLockTempMaps();			// see MFC TechNote #3
	CWnd* p_parent_wnd = CWnd::FromHandle(HWND(ulUIParam));

	CString caption;
	if (lpszCaption && *lpszCaption)
		caption = lpszCaption;
	else
		caption.LoadString(IDS_DEFAULT_CAPTION);
	CAddressDlg dlg(caption, nEditFields, p_parent_wnd);

	//
	// Populate the To, Cc, and Bcc edit controls with the
	// provided recipient list.
	//
	for (ULONG i = 0; i < nRecips; i++)
	{
		ASSERT(lpRecips);

		CString FullAddress(MakeFullAddress(&lpRecips[i]));

		switch (lpRecips[i].ulRecipClass)
		{
		case MAPI_TO:
			if (! dlg.m_To.IsEmpty())
				dlg.m_To += ", ";
			dlg.m_To += FullAddress;
			break;
		case MAPI_CC:
			if (! dlg.m_Cc.IsEmpty())
				dlg.m_Cc += ", ";
			dlg.m_Cc += FullAddress;
			break;
		case MAPI_BCC:
			if (! dlg.m_Bcc.IsEmpty())
				dlg.m_Bcc += ", ";
			dlg.m_Bcc += FullAddress;
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	if (dlg.DoModal() != IDOK)
	{
		AfxUnlockTempMaps();	// see MFC TechNote #3
		return MAPI_E_USER_ABORT;
	}

	//
	// Okay, if we get this far, the user supposedly has a list of
	// addresses to return to the MAPI client.  Let's do the dirty work 
	// of parsing the raw, comma-separated strings entered by the user 
	// into an array of MapiRecipDesc records.
	//
	AfxUnlockTempMaps();		// see MFC TechNote #3

	//
	// Step 1.  Build dynamic lists of recipients, sorted into To/Cc/Bcc
	// buckets.
	//
	CRecipientList to_list;
	CRecipientList cc_list;
	CRecipientList bcc_list;
	if ((! to_list.Tokenize(dlg.m_To)) ||
		(! cc_list.Tokenize(dlg.m_Cc)) ||
		(! bcc_list.Tokenize(dlg.m_Bcc)))
	{
		return MAPI_E_INSUFFICIENT_MEMORY;
	}

	//
	// Step 2.  Allocate an array big enough to hold all of the 
	// recipients.
	//
	const ULONG RECIP_COUNT = to_list.GetCount() + cc_list.GetCount() + bcc_list.GetCount();
	ASSERT(RECIP_COUNT);
	CMapiRecipDesc* p_recip_array = s_SessionMgr.NewMapiRecipDescArray(RECIP_COUNT);
	if (NULL == p_recip_array)
		return MAPI_E_INSUFFICIENT_MEMORY;

	//
	// Step 3.  Populate the array with all the recipients.
	//
	unsigned recip_array_idx = 0;
	while (! to_list.IsEmpty())
	{
		CString recip_str = to_list.RemoveHead();
		ASSERT(recip_array_idx < RECIP_COUNT);

		p_recip_array[recip_array_idx].ulRecipClass = MAPI_TO;

		if (! p_recip_array[recip_array_idx].SetNameAndAddress(recip_str, recip_str))
		{
			s_SessionMgr.FreeBuffer(p_recip_array);
			return MAPI_E_INSUFFICIENT_MEMORY;
		}

		//
		// The doc for MAPIDetails() says that MAPIAddress() is supposed
		// to "resolve" the names before returning them.  We can do that.
		//
		p_recip_array[recip_array_idx].ResolveName();

		recip_array_idx++;
	}

	while (! cc_list.IsEmpty())
	{
		CString recip_str = cc_list.RemoveHead();
		ASSERT(recip_array_idx < RECIP_COUNT);

		p_recip_array[recip_array_idx].ulRecipClass = MAPI_CC;

		if (! p_recip_array[recip_array_idx].SetNameAndAddress(recip_str, recip_str))
		{
			s_SessionMgr.FreeBuffer(p_recip_array);
			return MAPI_E_INSUFFICIENT_MEMORY;
		}

		//
		// The doc for MAPIDetails() says that MAPIAddress() is supposed
		// to "resolve" the names before returning them.  We can do that.
		//
		p_recip_array[recip_array_idx].ResolveName();

		recip_array_idx++;
	}

	while (! bcc_list.IsEmpty())
	{
		CString recip_str = bcc_list.RemoveHead();
		ASSERT(recip_array_idx < RECIP_COUNT);

		p_recip_array[recip_array_idx].ulRecipClass = MAPI_BCC;

		if (! p_recip_array[recip_array_idx].SetNameAndAddress(recip_str, recip_str))
		{
			s_SessionMgr.FreeBuffer(p_recip_array);
			return MAPI_E_INSUFFICIENT_MEMORY;
		}

		//
		// The doc for MAPIDetails() says that MAPIAddress() is supposed
		// to "resolve" the names before returning them.  We can do that.
		//
		p_recip_array[recip_array_idx].ResolveName();

		recip_array_idx++;
	}

	ASSERT(RECIP_COUNT == recip_array_idx);
	*lpnNewRecips = RECIP_COUNT;
	*lppNewRecips = p_recip_array;
	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPIDetails
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIDetails(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	lpMapiRecipDesc lpRecip, 
	FLAGS flFlags, 
	ULONG ulReserved)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	ASSERT(lpRecip != NULL);
	buf.Format("MAPIDetails:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpRecip = <%p>\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n",
			   lhSession,
			   ulUIParam,
			   lpRecip,
			   flFlags,
			   ulReserved);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// If caller wants to use an existing session, then validate the
	// session ID and message ID, if any.
	//
	if (0 == (flFlags & MAPI_NEW_SESSION))
	{
		if (lhSession && (! s_SessionMgr.IsValidSessionId(lhSession)))
			return MAPI_E_FAILURE;
	}

	if (NULL == lpRecip)
		return MAPI_E_FAILURE;
	else if (0 == lpRecip->ulEIDSize)
		return MAPI_E_AMBIGUOUS_RECIPIENT;

	//
	// Display a bogus "details" dialog.
	//
	AfxLockTempMaps();			// see MFC TechNote #3
	CWnd* p_parent_wnd = CWnd::FromHandle(HWND(ulUIParam));
	if (NULL == p_parent_wnd)
	{
		AfxUnlockTempMaps();
		return MAPI_E_FAILURE;
	}

	CString message;
	message.LoadString(IDS_DETAILS_MESSAGE);
	CString caption;
	caption.LoadString(IDS_DETAILS_CAPTION);
	p_parent_wnd->MessageBox(message, caption, MB_OK);
	AfxUnlockTempMaps();		// see MFC TechNote #3

	return SUCCESS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////
// MAPIResolveName
//
////////////////////////////////////////////////////////////////////////
extern "C" ULONG FAR PASCAL MAPIResolveName(
	LHANDLE lhSession, 
	ULONG ulUIParam,
	LPSTR lpszName, 
	FLAGS flFlags, 
	ULONG ulReserved,
	lpMapiRecipDesc FAR *lppRecip)
{
#ifndef _DEBUG
	UNREFERENCED_PARAMETER(ulUIParam);
	UNREFERENCED_PARAMETER(flFlags);
	UNREFERENCED_PARAMETER(ulReserved);
#else
	//
	// Print lots of useful debug info...
	//
	CString buf;
	ASSERT(lpszName != NULL);
	ASSERT(lppRecip != NULL);
	buf.Format("MAPIResolveName:\n"
			   "  lhSession = %lu\n"
			   "  ulUIParam = %lu\n"
			   "  lpszName = \"%s\"\n"
			   "  flFlags = %#010lx\n"
			   "  ulReserved = %lu\n"
			   "  lppRecip = <%p>\n"
			   "  *lppRecip = <%p>\n",
			   lhSession,
			   ulUIParam,
			   SAFESTRING(lpszName),
			   flFlags,
			   ulReserved,
			   lppRecip,
			   *lppRecip);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	//
	// Validate the session ID and other stuff.
	//
	if (NULL == lppRecip)
		return MAPI_E_FAILURE;		// caller blew it
	else if (lhSession && (! s_SessionMgr.IsValidSessionId(lhSession)))
		return MAPI_E_INVALID_SESSION;

	//
	// For good hygiene...
	//
	*lppRecip = NULL;

	//
	// Allocate, populate, and return a new MapiRecipDesc containing
	// the "resolved" address name (the joke is that ALL names are
	// resolved by Eudora, even if it is total garbage).
	//
	CMapiRecipDesc* p_recip = s_SessionMgr.NewMapiRecipDescArray(1);		// array of one
	if (NULL == p_recip)
		return MAPI_E_INSUFFICIENT_MEMORY;
	p_recip->SetClass(MAPI_ORIG);
	if (! p_recip->SetNameAndAddress(lpszName, lpszName))
	{
		s_SessionMgr.FreeBuffer(p_recip);
		return MAPI_E_INSUFFICIENT_MEMORY;
	}

	//
	// The doc for MAPIDetails() says that MAPIAddress() is supposed
	// to "resolve" the names before returning them.  We can do that.
	//
	p_recip->ResolveName();

	//
	// If we get this far, then the message record was setup successfully,
	// so return it to the caller.
	//
	*lppRecip = p_recip;
#ifdef _DEBUG
	buf.Format("  returning *lppRecip = <%p>\n",
			   *lppRecip);
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
	return SUCCESS_SUCCESS;
}

