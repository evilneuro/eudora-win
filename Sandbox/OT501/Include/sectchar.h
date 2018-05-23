//
// sectchar.h - definitions for generic international text functions
//

#ifndef __SECTCHAR_H__
#define __SECTCHAR_H__

#ifdef _WIN32

// use original tchar.h header file
#include <tchar.h>

#else

// following code is copied from the tchar.h header
// file of the Win32 SDK

#include <string.h>


#define _TEOF       EOF

#define __T(x)      x


/* Program */

#define _tmain      main
#define _tWinMain   WinMain
#ifdef  _POSIX_
#define _tenviron   environ
#else
#define _tenviron  _environ
#endif
#define __targv     __argv


/* Formatted i/o */

#define _tprintf    printf
#define _ftprintf   fprintf
#define _stprintf   sprintf
#define _sntprintf  _snprintf
#define _vtprintf   vprintf
#define _vftprintf  vfprintf
#define _vstprintf  vsprintf
#define _vsntprintf _vsnprintf
#define _tscanf     scanf
#define _ftscanf    fscanf
#define _stscanf    sscanf


/* Unformatted i/o */

#define _fgettc     fgetc
#define _fgettchar  _fgetchar
#define _fgetts     fgets
#define _fputtc     fputc
#define _fputtchar  _fputchar
#define _fputts     fputs
#define _gettc      getc
#define _gettchar   getchar
#define _getts      gets
#define _puttc      putc
#define _puttchar   putchar
#define _putts      puts
#define _ungettc    ungetc


/* String conversion functions */

#define _tcstod     strtod
#define _tcstol     strtol
#define _tcstoul    strtoul

#define _itot       _itoa
#define _ltot       _ltoa
#define _ultot      _ultoa
#define _ttoi       atoi
#define _ttol       atol


/* String functions */

#define _tcscat     strcat
#define _tcscpy     strcpy
#define _tcslen     strlen
#define _tcsxfrm    strxfrm
#define _tcsdup     _strdup


/* Execute functions */

#define _texecl     _execl
#define _texecle    _execle
#define _texeclp    _execlp
#define _texeclpe   _execlpe
#define _texecv     _execv
#define _texecve    _execve
#define _texecvp    _execvp
#define _texecvpe   _execvpe

#define _tspawnl    _spawnl
#define _tspawnle   _spawnle
#define _tspawnlp   _spawnlp
#define _tspawnlpe  _spawnlpe
#define _tspawnv    _spawnv
#define _tspawnve   _spawnve
#define _tspawnvp   _spawnvp
#define _tspawnvpe  _spawnvpe

#define _tsystem    system


/* Time functions */

#define _tasctime   asctime
#define _tctime     ctime
#define _tstrdate   _strdate
#define _tstrtime   _strtime
#define _tutime     _utime
#define _tcsftime   strftime


/* Directory functions */

#define _tchdir     _chdir
#define _tgetcwd    _getcwd
#define _tgetdcwd   _getdcwd
#define _tmkdir     _mkdir
#define _trmdir     _rmdir


/* Environment/Path functions */

#define _tfullpath  _fullpath
#define _tgetenv    getenv
#define _tmakepath  _makepath
#define _tputenv    _putenv
#define _tsearchenv _searchenv
#define _tsplitpath _splitpath


/* Stdio functions */

#ifdef  _POSIX_
#define _tfdopen    fdopen
#else
#define _tfdopen    _fdopen
#endif
#define _tfsopen    _fsopen
#define _tfopen     fopen
#define _tfreopen   freopen
#define _tperror    perror
#define _tpopen     _popen
#define _ttempnam   _tempnam
#define _ttmpnam    tmpnam


/* Io functions */

#define _tchmod     _chmod
#define _tcreat     _creat
#define _tfindfirst _findfirst
#define _tfindfirsti64  _findfirsti64
#define _tfindnext  _findnext
#define _tfindnexti64   _findnexti64
#define _tmktemp    _mktemp

#ifdef _POSIX_
#define _topen      open
#define _taccess    access
#else
#define _topen      _open
#define _taccess    _access
#endif

#define _tremove    remove
#define _trename    rename
#define _tsopen     _sopen
#define _tunlink    _unlink

#define _tfinddata_t    _finddata_t
#define _tfinddatai64_t _finddatai64_t


/* ctype functions */

#define _istascii   isascii
#define _istcntrl   iscntrl
#define _istxdigit  isxdigit


/* Stat functions */

#define _tstat      _stat
#define _tstati64   _stati64


/* Setlocale functions */

#define _tsetlocale setlocale

/* ++++++++++++++++++++ SBCS ++++++++++++++++++++ */


#ifndef __TCHAR_DEFINED
typedef char            _TCHAR;
typedef signed char     _TSCHAR;
typedef unsigned char   _TUCHAR;
typedef char            _TXCHAR;
typedef int             _TINT;
#define __TCHAR_DEFINED
#endif

#ifndef _TCHAR_DEFINED
#if     !__STDC__
typedef char            TCHAR;
#endif
#define _TCHAR_DEFINED
#endif


/* String functions */

#define _tcschr     strchr
#define _tcscspn    strcspn
#define _tcsncat    strncat
#define _tcsncpy    strncpy
#define _tcspbrk    strpbrk
#define _tcsrchr    strrchr
#define _tcsspn     strspn
#define _tcsstr     strstr
#define _tcstok     strtok

#define _tcsnset    _strnset
#define _tcsrev     _strrev
#define _tcsset     _strset

#define _tcscmp     strcmp
#define _tcsicmp    _stricmp
#define _tcsnccmp   strncmp
#define _tcsncmp    strncmp
#define _tcsncicmp  _strnicmp
#define _tcsnicmp   _strnicmp

#define _tcscoll    strcoll
#define _tcsicoll   _stricoll
#define _tcsnccoll  _strncoll
#define _tcsncoll   _strncoll
#define _tcsncicoll _strnicoll
#define _tcsnicoll  _strnicoll


/* "logical-character" mappings */

#define _tcsclen    strlen
#define _tcsnccat   strncat
#define _tcsnccpy   strncpy
#define _tcsncset   _strnset


/* MBCS-specific functions */

#define _tcsdec     _strdec
#define _tcsinc     _strinc
#define _tcsnbcnt   _strncnt
#define _tcsnccnt   _strncnt
#define _tcsnextc   _strnextc
#define _tcsninc    _strninc
#define _tcsspnp    _strspnp

#define _tcslwr     _strlwr
#define _tcsupr     _strupr
#define _tcsxfrm    strxfrm

#define _istlead(_c)    (0)
#define _istleadbyte(_c)    (0)

#if     (__STDC__ || defined(_NO_INLINING)) && !defined(_M_M68K)
#define _tclen(_pc) (1)
#define _tccpy(_pc1,_cpc2) (*(_pc1) = *(_cpc2))
#define _tccmp(_cpc1,_cpc2) (((unsigned char)*(_cpc1))-((unsigned char)*(_cpc2)))
#else   /* __STDC__ */
__inline size_t __cdecl _tclen(const char *_cpc) { return (_cpc,1); }
__inline void __cdecl _tccpy(char *_pc1, const char *_cpc2) { *_pc1 = *_cpc2; }
__inline int __cdecl _tccmp(const char *_cpc1, const char *_cpc2) { return (int) (((unsigned char)*_cpc1)-((unsigned char)*_cpc2)); }
#endif  /* __STDC__ */


/* ctype-functions */

#define _istalnum   isalnum
#define _istalpha   isalpha
#define _istdigit   isdigit
#define _istgraph   isgraph
#define _istlower   islower
#define _istprint   isprint
#define _istpunct   ispunct
#define _istspace   isspace
#define _istupper   isupper

#define _totupper   toupper
#define _totlower   tolower

#define _istlegal(_c)   (1)


/* the following is optional if functional versions are available */

/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#if     (__STDC__ || defined(_NO_INLINING)) && !defined(_M_M68K)
#define _strdec(_cpc1, _cpc2) ((_cpc2)-1)
#define _strinc(_pc)    ((_pc)+1)
#define _strnextc(_cpc) ((unsigned int) *(_cpc))
#define _strninc(_pc, _sz) (((_pc)+(_sz)))
#define _strncnt(_cpc, _sz) ((strlen(_cpc)>_sz) ? _sz : strlen(_cpc))
#define _strspnp(_cpc1, _cpc2) ((*((_cpc1)+strspn(_cpc1,_cpc2))) ? ((_cpc1)+strspn(_cpc1,_cpc2)) : NULL)
#else   /* __STDC__ */
__inline char * __cdecl _strdec(const char * _cpc1, const char * _cpc2) { return (char *)(_cpc1,(_cpc2-1)); }
__inline char * __cdecl _strinc(const char * _pc) { return (char *)(_pc+1); }
__inline unsigned int __cdecl _strnextc(const char * _cpc) { return (unsigned int)*_cpc; }
__inline char * __cdecl _strninc(const char * _pc, size_t _sz) { return (char *)(_pc+_sz); }
__inline size_t __cdecl _strncnt( const char * _cpc, size_t _sz) { size_t len; len = strlen(_cpc); return (len>_sz) ? _sz : len; }
__inline char * __cdecl _strspnp( const char * _cpc1, const char * _cpc2) { return (*(_cpc1 += strspn(_cpc1,_cpc2))!='\0') ? (char*)_cpc1 : NULL; }
#endif  /* __STDC__ */

/* Generic text macros to be used with string literals and character constants.
   Will also allow symbolic constants that resolve to same. */

#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)


#endif

#endif // __SECTCHAR_H__