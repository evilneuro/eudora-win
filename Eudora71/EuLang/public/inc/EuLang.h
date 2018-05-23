// lang.h -- language & country codes, etc.

#if !defined(_LANG_H_)
#define _LANG_H_ 

#if defined(__cplusplus)
extern "C" {
#endif

// things that GetLanguageInfo knows about
enum
{
	LANG_INFO_ISO639FULL,
	LANG_INFO_ISO639ABBREV,
	LANG_INFO_ISO3166A2,
	LANG_INFO_ISO3166A3,
	LANG_INFO_RFC1766,
	LANG_INFO_LOCALNAME,

	LANG_INFO_MAX   // always last
};

// returns length of out param. pass null to test length.
int LNG_GetLanguageInfo( int what, LPTSTR out );

#if defined(__cplusplus)
}   // end of extern "C"
#endif

#endif   // _LANG_H_
