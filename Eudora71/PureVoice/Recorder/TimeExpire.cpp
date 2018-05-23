#include <winreg.h>

#define LS_PUREVOICE_BETA_EXPIRED \
				"This beta version has expired. Please download the latest one from http://www.eudora.com. Would you like to go there?"

#define LS_PUREVOICE_BETA_DAYS_LEFT \
				"This beta version will expire in %d day(s). Please download the latest one from http://www.eudora.com. Would you like to go there?"

#define YEAR	1997
#define MONTH	8
#define DAY		1

class TimeExpire
{
public:
	BOOL IsValidDate();
	int GetDaysLeft();
	void GotoOurWebPage();
};

// returns number [0,11]
int TimeExpire::GetDaysLeft()
{
	FILETIME current;
	FILETIME expire;
    //1111 1100 0000 0000
	//5432 1098 7654 3210
    //yyyy yyym mmmd dddd
	WORD d = WORD( (0xFE00 & WORD((YEAR-1980)<<9))
					| (0x01E0 & WORD(MONTH<<5))
					| (0x001F & WORD(DAY)) );

	DosDateTimeToFileTime( d, 0, &expire);
	GetSystemTimeAsFileTime( &current );

	if ( current.dwHighDateTime >= expire.dwHighDateTime )
		return 0;
	float days = (expire.dwHighDateTime - current.dwHighDateTime)
				*(429.4967296/86400);
	if ( days > 10 ) return 11;
	return int(days);
}

BOOL TimeExpire::IsValidDate()
{
	if ( GetDaysLeft() < 1 )
	 {
		if ( AfxMessageBox( LS_PUREVOICE_BETA_EXPIRED , MB_YESNO ) == IDYES )
			GotoOurWebPage();
		return FALSE;
	 }
	else if ( GetDaysLeft() < 6 )
	 {
		char buf[256];
		wsprintf( buf, LS_PUREVOICE_BETA_DAYS_LEFT, GetDaysLeft() );
		if ( AfxMessageBox( buf, MB_YESNO ) == IDYES )
			GotoOurWebPage();
	 }
	return TRUE;
}

void TimeExpire::GotoOurWebPage()
{
	// find default browser in registery
	char exeFile[2*_MAX_PATH];
	LONG size;
	LONG err = RegQueryValue( HKEY_CLASSES_ROOT,
			"http\\shell\\open\\command", exeFile, &size );

	if ( err != ERROR_SUCCESS )
	 {
		err = RegQueryValue( HKEY_CLASSES_ROOT,
			"htmlfile\\shell\\open\\command", exeFile, &size );
	 }
	if ( err != ERROR_SUCCESS )
	 {
		err = RegQueryValue( HKEY_CLASSES_ROOT,
			"html_auto_file\\shell\\open\\command", exeFile, &size );
	 }
	if ( err != ERROR_SUCCESS )
	 {
		err = RegQueryValue( HKEY_CLASSES_ROOT,
			"HtmlDefBrowser\\shell\\open\\command", exeFile, &size );
	 }		
	if ( err != ERROR_SUCCESS )
		return;

	STARTUPINFO si;				// pointer to STARTUPINFO 
	PROCESS_INFORMATION pi; 	// pointer to PROCESS_INFORMATION  

	memset( &si, 0, sizeof(si) );
	si.cb = sizeof(si);

	lstrcat( exeFile, " http://www.eudora.com" );

	BOOL ret = ::CreateProcess(
		NULL,					// pointer to name of executable module 
		LPTSTR(exeFile),		// pointer to command line string
		NULL,					// pointer to process security attributes 
		NULL,					// pointer to thread security attributes 
		FALSE,					// handle inheritance flag 
		0,						// creation flags 
		NULL,					// pointer to new environment block 
		NULL,					// pointer to current directory name 
		&si,					// pointer to STARTUPINFO 
		&pi 					// pointer to PROCESS_INFORMATION  
	   );

}
