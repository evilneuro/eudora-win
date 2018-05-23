/*
 	File:		StringCompare.h
 
 	Contains:	Public interfaces for String Comparison and related operations
 
 	Version:	Technology:	Mac OS 8
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __STRINGCOMPARE__
#define __STRINGCOMPARE__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __TEXTCOMMON__
#include <TextCommon.h>
#endif
#ifndef __SCRIPT__
#include <Script.h>
#endif



#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

/*

	Here are the current System 7 routine names and the translations to the older forms.
	Please use the newer forms in all new code and migrate the older names out of existing
	code as maintenance permits.
	
	NEW NAME					OLD NAME					OBSOLETE FORM (no handle)
	
	CompareString (Str255)		IUCompPString (hp only)		IUCompString (hp only)
	CompareText (ptr/len)		IUMagPString				IUMagString
	IdenticalString (Str255)	IUEqualPString (hp only)	IUEqualString  (hp only)
	IdenticalText (ptr/len)		IUMagIDPString				IUMagIDString
	LanguageOrder				IULangOrder
	ScriptOrder					IUScriptOrder
	StringOrder (Str255)		IUStringOrder (hp only)
	TextOrder (ptr/len)			IUTextOrder

	RelString
	CmpString (a only)					
	EqualString (hp only)
	
	ReplaceText

*/


enum {
																/* Special language code values for Language Order*/
	systemCurLang				= -2,							/* current (itlbLang) lang for system script*/
	systemDefLang				= -3,							/* default (table) lang for system script*/
	currentCurLang				= -4,							/* current (itlbLang) lang for current script*/
	currentDefLang				= -5,							/* default lang for current script*/
	scriptCurLang				= -6,							/* current (itlbLang) lang for specified script*/
	scriptDefLang				= -7							/* default language for a specified script*/
};

/* obsolete names*/

enum {
	iuSystemCurLang				= -2,
	iuSystemDefLang				= -3,
	iuCurrentCurLang			= -4,
	iuCurrentDefLang			= -5,
	iuScriptCurLang				= -6,
	iuScriptDefLang				= -7
};


/*
   The following functions are old names, but are required for System 7 PowerPC builds
   because InterfaceLib exports these names, instead of the new ones.
*/

EXTERN_API( short )
IUMagPString					(const void *			aPtr,
								 const void *			bPtr,
								 short 					aLen,
								 short 					bLen,
								 Handle 				itl2Handle)							THREEWORDINLINE(0x3F3C, 0x001A, 0xA9ED);

EXTERN_API( short )
IUMagIDPString					(const void *			aPtr,
								 const void *			bPtr,
								 short 					aLen,
								 short 					bLen,
								 Handle 				itl2Handle)							THREEWORDINLINE(0x3F3C, 0x001C, 0xA9ED);

EXTERN_API( short )
IUTextOrder						(const void *			aPtr,
								 const void *			bPtr,
								 short 					aLen,
								 short 					bLen,
								 ScriptCode 			aScript,
								 ScriptCode 			bScript,
								 LangCode 				aLang,
								 LangCode 				bLang)								THREEWORDINLINE(0x3F3C, 0x0022, 0xA9ED);

EXTERN_API( short )
IULangOrder						(LangCode 				language1,
								 LangCode 				language2)							THREEWORDINLINE(0x3F3C, 0x0020, 0xA9ED);

EXTERN_API( short )
IUScriptOrder					(ScriptCode 			script1,
								 ScriptCode 			script2)							THREEWORDINLINE(0x3F3C, 0x001E, 0xA9ED);

EXTERN_API( short )
IUCompPString					(ConstStr255Param 		aStr,
								 ConstStr255Param 		bStr,
								 Handle 				itl2Handle);

EXTERN_API( short )
IUEqualPString					(ConstStr255Param 		aStr,
								 ConstStr255Param 		bStr,
								 Handle 				itl2Handle);

EXTERN_API( short )
IUStringOrder					(ConstStr255Param 		aStr,
								 ConstStr255Param 		bStr,
								 ScriptCode 			aScript,
								 ScriptCode 			bScript,
								 LangCode 				aLang,
								 LangCode 				bLang);

/*
   The following new function name IS exported by InterfaceLib and works on both 68k and
   System 7 PowerPC.
*/
EXTERN_API( short )
ScriptOrder						(ScriptCode 			script1,
								 ScriptCode 			script2)							THREEWORDINLINE(0x3F3C, 0x001E, 0xA9ED);

/* For C the other new names are mapped to the old names using #define.*/
#if TARGET_OS_MAC
#define CompareString(aStr, bStr, itl2Handle) \
		 IUCompPString(aStr, bStr, itl2Handle)
#endif
#define MacCompareString(aStr, bStr, itl2Handle) \
		 IUCompPString(aStr, bStr, itl2Handle)
#define CompareText(aPtr, bPtr, aLen, bLen, itl2Handle) \
		 IUMagPString(aPtr, bPtr, aLen, bLen, itl2Handle)
#define IdenticalString(aStr, bStr, itl2Handle) \
		 IUEqualPString(aStr, bStr, itl2Handle)
#define IdenticalText(aPtr, bPtr, aLen, bLen, itl2Handle) \
		 IUMagIDPString(aPtr, bPtr, aLen, bLen, itl2Handle)
#define StringOrder(aStr, bStr, aScript, bScript, aLang, bLang) \
		 IUStringOrder(aStr, bStr, aScript, bScript, aLang, bLang)
#define TextOrder(aPtr, bPtr, aLen, bLen, aScript, bScript, aLang, bLang) \
		 IUTextOrder(aPtr, bPtr, aLen, bLen, aScript, bScript, aLang, bLang)
#define LanguageOrder(language1, language2) \
		 IULangOrder(language1, language2)
/* C glue*/

#if CGLUESUPPORTED
#if OLDROUTINENAMES
EXTERN_API_C( short )
iucomppstring					(const char *			aStr,
								 const char *			bStr,
								 Handle 				intlHandle);

EXTERN_API_C( short )
iuequalpstring					(const char *			aStr,
								 const char *			bStr,
								 Handle 				intlHandle);

EXTERN_API_C( short )
iustringorder					(const char *			aStr,
								 const char *			bStr,
								 ScriptCode 			aScript,
								 ScriptCode 			bScript,
								 LangCode 				aLang,
								 LangCode 				bLang);

#endif  /* OLDROUTINENAMES */

#endif  /* CGLUESUPPORTED */

/*
   The following new function name IS exported by InterfaceLib and works on both 68k and
   System 7 PowerPC.
*/
#if TARGET_OS_MAC
	#define MacReplaceText ReplaceText
#endif
EXTERN_API( short )
MacReplaceText					(Handle 				baseText,
								 Handle 				substitutionText,
								 Str15 					key)								FOURWORDINLINE(0x2F3C, 0x820C, 0xFFDC, 0xA8B5);

EXTERN_API( short )
IUMagString						(const void *			aPtr,
								 const void *			bPtr,
								 short 					aLen,
								 short 					bLen)								THREEWORDINLINE(0x3F3C, 0x000A, 0xA9ED);

EXTERN_API( short )
IUMagIDString					(const void *			aPtr,
								 const void *			bPtr,
								 short 					aLen,
								 short 					bLen)								THREEWORDINLINE(0x3F3C, 0x000C, 0xA9ED);

EXTERN_API( short )
IUCompString					(ConstStr255Param 		aStr,
								 ConstStr255Param 		bStr);

EXTERN_API( short )
IUEqualString					(ConstStr255Param 		aStr,
								 ConstStr255Param 		bStr);

#if CGLUESUPPORTED
#if OLDROUTINENAMES
EXTERN_API_C( short )
iucompstring					(const char *			aStr,
								 const char *			bStr);

EXTERN_API_C( short )
iuequalstring					(const char *			aStr,
								 const char *			bStr);

#endif  /* OLDROUTINENAMES */

#endif  /* CGLUESUPPORTED */

EXTERN_API( short )
RelString						(ConstStr255Param 		str1,
								 ConstStr255Param 		str2,
								 Boolean 				caseSensitive,
								 Boolean 				diacSensitive);

EXTERN_API( Boolean )
EqualString						(ConstStr255Param 		str1,
								 ConstStr255Param 		str2,
								 Boolean 				caseSensitive,
								 Boolean 				diacSensitive);

#if CGLUESUPPORTED
EXTERN_API_C( short )
relstring						(const char *			str1,
								 const char *			str2,
								 Boolean 				caseSensitive,
								 Boolean 				diacSensitive);

EXTERN_API_C( Boolean )
equalstring						(const char *			str1,
								 const char *			str2,
								 Boolean 				caseSensitive,
								 Boolean 				diacSensitive);

#endif  /* CGLUESUPPORTED */


#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STRINGCOMPARE__ */

