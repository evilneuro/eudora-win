/*
 	File:		Scrap.h
 
 	Contains:	Scrap Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __SCRAP__
#define __SCRAP__

#ifndef __MACTYPES__
#include <MacTypes.h>
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
  _________________________________________________________________________________________________________
   ¥ CLASSIC SCRAP MANAGER API
  _________________________________________________________________________________________________________
*/


struct ScrapStuff {
	SInt32 							scrapSize;
	Handle 							scrapHandle;
	SInt16 							scrapCount;
	SInt16 							scrapState;
	StringPtr 						scrapName;
};
typedef struct ScrapStuff				ScrapStuff;

typedef ScrapStuff *					PScrapStuff;
typedef ScrapStuff *					ScrapStuffPtr;
EXTERN_API( ScrapStuffPtr )
InfoScrap						(void)														ONEWORDINLINE(0xA9F9);

EXTERN_API( SInt32 )
UnloadScrap						(void)														ONEWORDINLINE(0xA9FA);

EXTERN_API( SInt32 )
LoadScrap						(void)														ONEWORDINLINE(0xA9FB);

EXTERN_API( SInt32 )
GetScrap						(Handle 				hDest,
								 ResType 				theType,
								 SInt32 *				offset)								ONEWORDINLINE(0xA9FD);

EXTERN_API( SInt32 )
ZeroScrap						(void)														ONEWORDINLINE(0xA9FC);

EXTERN_API( SInt32 )
PutScrap						(SInt32 				length,
								 ResType 				theType,
								 const void *			source)								ONEWORDINLINE(0xA9FE);


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

#endif /* __SCRAP__ */

