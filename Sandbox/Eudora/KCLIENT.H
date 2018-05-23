// kclient.h 

// Steve Edgar - Project Mandarin, Cornell University.

//	Mandarin-style "KClient" Kerberos extensions to MIT Windows Kerberos
// Be sure to "#include" this file after windows.h
// See the file kclient.wri for a description of each function.

BOOL CALLBACK GetTicketForService (LPSTR, LPSTR, LPDWORD) ;
BOOL CALLBACK DeleteAllSessions (void) ;
BOOL CALLBACK SetUserName (LPSTR) ;
BOOL CALLBACK ListTickets (HWND) ;
void CALLBACK SetKrbdllMode (BOOL) ;



//added for Eudora

/*
#ifdef WIN32	
BOOL Kerb16GetTicket(LPSTR commandString, LPSTR buffer, LPDWORD bufLen);	  
BOOL Kerb16SetName(LPSTR name);
BOOL Kerb16LoadLib();		  
BOOL Kerb16FreeLib();
BOOL KerbQuitApp();
//DWORD s_dwKerbTicketBufferLength = 0; 
#endif		  
*/

//static HINSTANCE s_hInstKerberosLib = NULL;

//static BOOL (CALLBACK* fnSetUserName)(LPSTR) = NULL;
//static BOOL (CALLBACK* fnGetTicketForService)(LPSTR, LPSTR, LPDWORD) = NULL;
//static BOOL (CALLBACK* fnSendTicketForService)(LPSTR service, LPSTR version, int fd) = NULL;


#include <winsock.h> //for SOCKET
#include "Callbacks.h"

class  QCKerberos
{
	static HINSTANCE s_hInstKerberosLib;
	static BOOL s_bKerbAppLoaded;

public:
	static int LoadKerberosLibrary(Callback1<const char *> ReportError);
	static int FreeLibrary();
	static BOOL IsLibraryLoaded(){ return s_bKerbAppLoaded; }

	static BOOL SetUserName(LPSTR str);
	static BOOL SendTicket(char *szCommandString, SOCKET socket, BOOL *bSentTicket);
	static const char *GetTicketBuffer(){ return s_szKerbTicketBuffer;}
	static DWORD GetTicketLength(){ return s_dwKerbTicketBufferLength; }
	//static void LoadTicket();



	//DLL
	static BOOL GetTicketForService(LPSTR s1, LPSTR s2, LPDWORD pWord);
	static BOOL SendTicketForService(LPSTR service, LPSTR version, int fd);

	//APPLICATION
	static BOOL Kerb16GetTicket(LPSTR commandString, LPSTR buffer, LPDWORD bufLen);	  
	static BOOL Kerb16SetName(LPSTR name);
	static BOOL Kerb16LoadLib();		  
	static BOOL Kerb16FreeLib();
	static BOOL KerbQuitApp();

	static BOOL SetKerb16Ticket(PCOPYDATASTRUCT pcds);
	static void ClearKerberosTicket();

	//	Kerberos constants
	enum { kTicketBufferSize = 2000};
	static BOOL s_bKerbTicket;
	static char s_szKerbTicketBuffer[QCKerberos::kTicketBufferSize];
	static DWORD s_dwKerbTicketBufferLength; 


private:	

	//DLL functions
	static BOOL (CALLBACK* m_pfnSetUserName)(LPSTR);
	static BOOL (CALLBACK* m_pfnGetTicketForService)(LPSTR, LPSTR, LPDWORD);
	static BOOL (CALLBACK* m_pfnSendTicketForService)(LPSTR service, LPSTR version, int fd);
	

};
