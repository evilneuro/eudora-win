/*
 	File:		ImageCodec.r
 
 	Contains:	QuickTime Interfaces.
 
 	Version:	Technology:	QuickTime 3.0
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1990-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __IMAGECODEC_R__
#define __IMAGECODEC_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#include "Components.r"
#include "ImageCompression.r"
#define kParameterTitleName 			'name'
#define kParameterTitleID 				1

#define kParameterWhatName 				'what'
#define kParameterWhatID 				1

#define kParameterVersionName 			'vers'
#define kParameterVersionID 			1

#define kParameterRepeatableName 		'pete'
#define kParameterRepeatableID 			1


	#define kParameterRepeatableTrue	long { "1" }
	#define kParameterRepeatableFalse	long { "0" }

#define kParameterAlternateCodecName 	'subs'
#define kParameterAlternateCodecID 		1

#define kParameterSourceCountName 		'srcs'
#define kParameterSourceCountID 		1

#define kParameterDependencyName 		'deep'
#define kParameterDependencyID 			1


	#define kParameterListDependsUponColorProfiles	OSType { "prof" }
	#define kParameterListDependsUponFonts			OSType { "font" }

#define kParameterEnumList 				'enum'
#define kParameterAtomTypeAndID 		'type'

		#define kNoAtom 				OSType { "none" }
		#define kAtomNoFlags 			long { "0" }
		#define kAtomNotInterpolated 	long { "1" }
		#define kAtomInterpolateIsOptional 	long { "2" }

#define kParameterDataType 				'data'

		#define kParameterTypeDataLong 		long { "2" }
		#define kParameterTypeDataFixed 	long { "3" }
		#define kParameterTypeDataRGBValue 	long { "8" }
		#define kParameterTypeDataDouble 	long { "11" }
		#define kParameterTypeDataText 		OSType { "text" }
		#define kParameterTypeDataEnum		OSType { "enum" }
		#define kParameterTypeDataBitField 	OSType { "bool" }
		#define kParameterTypeDataImage		OSType { "imag" }

#define kParameterDataRange 			'rang'
#define kParameterDataBehavior 			'ditl'

	// items edited via typing
		#define kParameterItemEditText 		OSType { "edit" }
		#define kParameterItemEditLong 		OSType { "long" }
		#define kParameterItemEditFixed 	OSType { "fixd" }
		#define kParameterItemEditDouble 	OSType { "doub" }

	// items edited via control(s)
		#define kParameterItemPopUp 		OSType { "popu" }
		#define kParameterItemRadioCluster 	OSType { "radi" }
		#define kParameterItemCheckBox 		OSType { "chex" }
		#define kParameterItemControl 		OSType { "cntl" }
		
	// special user items
		#define kParameterItemLine 			OSType { "line" }
		#define kParameterItemColorPicker 	OSType { "pick" }
		#define kParameterItemGroupDivider	OSType { "divi" }
		#define kParameterItemStaticText	OSType { "stat" }
		#define kParameterItemDragImage		OSType { "imag" }



		// flags valid for lines and groups
		#define kGraphicsNoFlags 			long { "0" }
		#define kGraphicsFlagsGray 			long { "1" }

		// flags valid for groups
		#define kGroupNoFlags 						long { "0" }
		#define kGroupAlignText 					long { "65536" }
		#define kGroupSurroundBox 					long { "131072" }
		#define kGroupMatrix 						long { "262144" }
		#define kGroupNoName						long { "524288" }
		#define kGroupSurroundWithGray				long { "131073" }
		#define kGroupSurroundWithGrayAlign			long { "196609" }
		#define kGroupSurroundWithGrayNoName		long { "655361" }
		#define kGroupSurroundWithGrayAlignNoName	long { "720897" }
		
		// flags valid for popup/radiocluster/checkbox/control
		#define kDisableControl			long { "1" }
		#define kDisableWhenNotEqual	long { "1" }
		#define kDisableWhenEqual		long { "17" }
		#define kDisableWhenLessThan	long { "33" }
		#define kDisableWhenGreaterThan	long { "49" }

		// flags valid for popups
		#define kPopupStoreAsString		long { "65536" }

#define kParameterDataUsage 			'use '

		#define kParameterUsagePixels		OSType { "pixl" }
		#define kParameterUsageRectangle	OSType { "rect" }
		#define kParameterUsagePoint		OSType { "xy  " }
		#define kParameterUsage3DPoint		OSType { "xyz " }
		#define kParameterUsageDegrees		OSType { "degr" }
		#define kParameterUsageRadians		OSType { "rads" }
		#define kParameterUsagePercent		OSType { "pcnt" }
		#define kParameterUsageSeconds		OSType { "secs" }
		#define kParameterUsageMilliseconds	OSType { "msec" }
		#define kParameterUsageMicroseconds	OSType { "µsec" }
		#define kParameterUsage3by3Matrix	OSType { "3by3" }

#define kParameterDataDefaultItem 		'dflt'
#define kParameterInfoLongName 			'©nam'
#define kParameterInfoCopyright 		'©cpy'
#define kParameterInfoDescription 		'©inf'
#define kParameterInfoWindowTitle 		'©wnt'
#define kParameterInfoPicture 			'©pix'
#define kParameterInfoManufacturer 		'©man'
#define kParameterInfoIDs 				1


type 'cdec' {
	hex string;
};


type 'cdci' {
	pstring[31];
	hex integer	version;
	hex integer	revlevel;
	hex longint	vendor;
	hex longint	decompressFlags;
	hex longint	compressFlags;
	hex longint	formatFlags;
	byte		compressionAccuracy;
	byte		decompressionAccuracy;
	integer		compressionSpeed;
	integer		decompressionSpeed;
	byte		compressionLevel;
	byte		resvd;
	integer		minimumHeight;
	integer		minimumWidth;
	integer		decompressPipelineLatency;
	integer		compressPipelineLatency;
	longint		privateData;
};


#define	compressorComponentType			'imco'
#define	decompressorComponentType		'imdc'


/* Resource type used for the creation of parameter description atoms */
type 'atms' {
	longint;	// root atom count
	
	array AtomArray {
		literal longint;		// atomType
		longint;				// atomID
		longint noChildren = 0;	// children
		longint =  $$CountOf(AtomData);
		array AtomData {
			switch {
			case long:
				key literal longint = 'long';
				pstring;			// data
			case short:
				key literal longint = 'shrt';
				pstring;			// data
			case noMininumFixed:
				key literal longint = 'nmiF';
				pstring = "";		// data
			case noMaximumFixed:
				key literal longint = 'nmaF';
				pstring = "";		// data
			case noMininumDouble:
				key literal longint = 'nmiD';
				pstring = "";		// data
			case noMaximumDouble:
				key literal longint = 'nmaD';
				pstring = "";		// data
			case fixed:
				key literal longint = 'fixd';
				pstring;			// data
			case double:
				key literal longint = 'doub';
				pstring;			// data
			case string:
				key literal longint = 'str ';
				pstring;			// data
			case OSType:
				key literal longint = 'osty';
				pstring;			// data
			};
			
		};
	};
};


#endif /* __IMAGECODEC_R__ */

