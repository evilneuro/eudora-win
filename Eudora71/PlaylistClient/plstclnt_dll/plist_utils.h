// plist_utils.h

#ifndef _PLIST_UTILS_H_
#define _PLIST_UTILS_H_

#include <afx.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(max)
#undef max
#endif
#define max(a,b) ((a > b) ? a : b)
#define make_word(a, b) ((unsigned short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b))) << 8))
#define make_long(a, b) ((long)(((unsigned short)(a)) | ((unsigned long)((unsigned short)(b))) << 16))
#define lo_word(l) ((unsigned short)(l))
#define hi_word(l) ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#define lo_byte(w) ((unsigned char)(w))
#define hi_byte(w) ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

void MunchURL( char* url, char** host, char** uri, int* port );
char* DupeString( const char* str );
char* MimeString( const char* url );
char* TempFile();
void UnescapeUrl( char* url );
char *GetIniStringNotStatic( void *inProc, int num );

#define A_MINUTE	(60)
#define AN_HOUR		(60*A_MINUTE)
#define A_DAY		(24*AN_HOUR)
#define A_MONTH		(30*A_DAY)
#define A_YEAR		(365*A_DAY)

#ifdef __cplusplus
}

CString Secs2Duration( int secs, int sigdigs=1 );
#define S2D (LPCTSTR)Secs2Duration

#endif   // __cplusplus

#endif   // _PLIST_UTILS_H_
