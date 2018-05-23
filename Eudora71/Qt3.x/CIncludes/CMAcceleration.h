/*
 	File:		CMAcceleration.h
 
 	Contains:	ColorSync Acceleration Component API
 
 	Version:	Technology:	ColorSync 2.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1995-1998 by Apple Computer, Inc., all rights reserved.
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __CMACCELERATION__
#define __CMACCELERATION__

#ifndef __COMPONENTS__
#include <Components.h>
#endif
#ifndef __CMAPPLICATION__
#include <CMApplication.h>
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

/* ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ version info */

enum {
	cmAccelerationInterfaceVersion = 1
};

/* ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ Component Type */

enum {
	cmAccelerationComponentType	= FOUR_CHAR_CODE('csac')
};

/* ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ Required Component function selectors */

enum {
	cmLoadTables				= 0,
	cmCalculateData				= 1
};

/* ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ table data for acceleration component */

struct CMAccelerationTableData {
	long 							inputLutEntryCount;			/* count of entries for input lut for one dimension*/
	long 							inputLutWordSize;			/* count of bits of each entry ( e.g. 16 for WORD )*/
	Handle 							inputLut;					/* handle to input lut*/
	long 							outputLutEntryCount;		/* count of entries for output lut for one dimension	*/
	long 							outputLutWordSize;			/* count of bits of each entry ( e.g. 8 for BYTE )*/
	Handle 							outputLut;					/* handle to output lut*/
	long 							colorLutInDim;				/* input dimension  ( e.g. 3 for LAB ; 4 for CMYK )*/
	long 							colorLutOutDim;				/* output dimension ( e.g. 3 for LAB ; 4 for CMYK )*/
	long 							colorLutGridPoints;			/* count of gridpoints for color lut ( for one Dimension )	*/
	long 							colorLutWordSize;			/* count of bits of each entry ( e.g. 8 for BYTE )*/
	Handle 							colorLut;					/* handle to color lut*/
	CMBitmapColorSpace 				inputColorSpace;			/* packing info for input*/
	CMBitmapColorSpace 				outputColorSpace;			/* packing info for output*/
	void *							userData;
	unsigned long 					reserved1;
	unsigned long 					reserved2;
	unsigned long 					reserved3;
	unsigned long 					reserved4;
	unsigned long 					reserved5;
};
typedef struct CMAccelerationTableData	CMAccelerationTableData;
typedef CMAccelerationTableData *		CMAccelerationTableDataPtr;
typedef CMAccelerationTableDataPtr *	CMAccelerationTableDataHdl;
/* ÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐÐ calc data for acceleration component */

struct CMAccelerationCalcData {
	long 							pixelCount;					/* count of input pixels*/
	Ptr 							inputData;					/* input array*/
	Ptr 							outputData;					/* output array*/
	unsigned long 					reserved1;
	unsigned long 					reserved2;
};
typedef struct CMAccelerationCalcData	CMAccelerationCalcData;
typedef CMAccelerationCalcData *		CMAccelerationCalcDataPtr;
typedef CMAccelerationCalcDataPtr *		CMAccelerationCalcDataHdl;
/*
   ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ 
  				A c c e l e r a t i o n   C o m p o n e n t   I n t e r f a c e s
   ÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑÑ 
*/
EXTERN_API( CMError )
CMAccelerationLoadTables		(ComponentInstance 		CMSession,
								 CMAccelerationTableDataPtr  tableData)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0000, 0x7000, 0xA82A);


EXTERN_API( CMError )
CMAccelerationCalculateData		(ComponentInstance 		CMSession,
								 CMAccelerationCalcDataPtr  calcData)						FIVEWORDINLINE(0x2F3C, 0x0004, 0x0001, 0x7000, 0xA82A);



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

#endif /* __CMACCELERATION__ */

