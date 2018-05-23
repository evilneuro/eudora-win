/*
 	File:		MacWindows.r
 
 	Contains:	Window Manager Interfaces.
 
 	Version:	Technology:	Mac OS 8.1
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/

#ifndef __MACWINDOWS_R__
#define __MACWINDOWS_R__

#ifndef __CONDITIONALMACROS_R__
#include "ConditionalMacros.r"
#endif

#define kWindowDefProcType 				'WDEF'
#define kStandardWindowDefinition 		0					/*  for document windows and dialogs */
#define kRoundWindowDefinition 			1					/*  old da-style window */
#define kFloatingWindowDefinition 		124					/*  for floating windows */

															/*  for use with kStandardWindowDefinition  */
#define kDocumentWindowVariantCode 		0
#define kModalDialogVariantCode 		1
#define kPlainDialogVariantCode 		2
#define kShadowDialogVariantCode 		3
#define kMovableModalDialogVariantCode 	5
#define kAlertVariantCode 				7
#define kMovableAlertVariantCode 		9					/*  for use with kFloatingWindowDefinition  */
#define kSideFloaterVariantCode 		8

															/*  Resource IDs for theme-savvy window defprocs  */
#define kWindowDocumentDefProcResID 	64
#define kWindowDialogDefProcResID 		65
#define kWindowUtilityDefProcResID 		66
#define kWindowUtilitySideTitleDefProcResID  67

															/*  Proc IDs for theme-savvy windows  */
#define kWindowDocumentProc 			1024
#define kWindowGrowDocumentProc 		1025
#define kWindowVertZoomDocumentProc 	1026
#define kWindowVertZoomGrowDocumentProc  1027
#define kWindowHorizZoomDocumentProc 	1028
#define kWindowHorizZoomGrowDocumentProc  1029
#define kWindowFullZoomDocumentProc 	1030
#define kWindowFullZoomGrowDocumentProc  1031

															/*  Proc IDs for theme-savvy dialogs  */
#define kWindowPlainDialogProc 			1040
#define kWindowShadowDialogProc 		1041
#define kWindowModalDialogProc 			1042
#define kWindowMovableModalDialogProc 	1043
#define kWindowAlertProc 				1044
#define kWindowMovableAlertProc 		1045

															/*  Proc IDs for top title bar theme-savvy floating windows  */
#define kWindowFloatProc 				1057
#define kWindowFloatGrowProc 			1059
#define kWindowFloatVertZoomProc 		1061
#define kWindowFloatVertZoomGrowProc 	1063
#define kWindowFloatHorizZoomProc 		1065
#define kWindowFloatHorizZoomGrowProc 	1067
#define kWindowFloatFullZoomProc 		1069
#define kWindowFloatFullZoomGrowProc 	1071

															/*  Proc IDs for side title bar theme-savvy floating windows  */
#define kWindowFloatSideProc 			1073
#define kWindowFloatSideGrowProc 		1075
#define kWindowFloatSideVertZoomProc 	1077
#define kWindowFloatSideVertZoomGrowProc  1079
#define kWindowFloatSideHorizZoomProc 	1081
#define kWindowFloatSideHorizZoomGrowProc  1083
#define kWindowFloatSideFullZoomProc 	1085
#define kWindowFloatSideFullZoomGrowProc  1087

#define kWindowNoPosition 				0x0000
#define kWindowDefaultPosition 			0x0000
#define kWindowCenterMainScreen 		0x280A
#define kWindowAlertPositionMainScreen 	0x300A
#define kWindowStaggerMainScreen 		0x380A
#define kWindowCenterParentWindow 		0xA80A
#define kWindowAlertPositionParentWindow  0xB00A
#define kWindowStaggerParentWindow 		0xB80A
#define kWindowCenterParentWindowScreen  0x680A
#define kWindowAlertPositionParentWindowScreen  0x700A
#define kWindowStaggerParentWindowScreen  0x780A


/*--------------------------wctb ¥ Window Color Lookup Table--------------------------*/
/*
	wctb_RezTemplateVersion:
		0 - original 
		1 - more color parts and implicit header	<-- default
		2 - addition index table at end
*/
#ifndef wctb_RezTemplateVersion
	#ifdef oldTemp							/* grandfather in use of ÒoldTempÓ */
		#define wctb_RezTemplateVersion 0
	#elif defined(evenNewerTemp)			/* grandfather in use of ÒevenNewerTempÓ */
		#define wctb_RezTemplateVersion 2
	#else
		#define wctb_RezTemplateVersion 1
	#endif
#endif


type 'wctb' {
#if wctb_RezTemplateVersion == 0
			unsigned hex longint;									/* ctSeed				*/
			integer;												/* ctFlags				*/
#elif wctb_RezTemplateVersion == 1
			unsigned hex longint = 0;								/* ctSeed				*/
			integer = 0;											/* ctFlags				*/
#endif
			integer = $$Countof(ColorSpec) - 1;						/* ctSize				*/
			wide array ColorSpec {
					integer		wContentColor,						/* value				*/
								wFrameColor,
								wTextColor,
								wHiliteColor,
								wTitleBarColor,
								wHiliteLight,
								wHiliteDark,
								wTitleBarLight,
								wTitleBarDark,
								wDialogLight,
								wDialogDark,
								wTingeLight,
								wTingeDark;
					unsigned integer;								/* RGB:	red				*/
					unsigned integer;								/*		green			*/
					unsigned integer;								/*		blue			*/
			};
	};


/*----------------------------WIND ¥ Window Template------------------------------------*/
/*
	WIND_RezTemplateVersion:
		0 - original 							<-- default
		1 - additional positioning info at end	
*/
#ifndef WIND_RezTemplateVersion
	#define WIND_RezTemplateVersion 1
#endif


type 'WIND' {
		rect;													/* boundsRect			*/
		integer 		documentProc,							/* procID				*/
						dBoxProc,
						plainDBox,
						altDBoxProc,
						noGrowDocProc,
						movableDBoxProc,
						zoomDocProc = 8,
						zoomNoGrow = 12,
						rDocProc = 16;
		byte			invisible, visible; 					/* visible				*/
		fill byte;
		byte			noGoAway, goAway;						/* goAway				*/
		fill byte;
		unsigned hex longint;									/* refCon				*/
		pstring 		Untitled = "Untitled";				/* title				*/
		
#if WIND_RezTemplateVersion == 1
	/*	The following are window positioning options used by System 7.0 and later */
		align word;
		unsigned integer				noAutoCenter = 0x0000,
										centerMainScreen = 0x280a,
										alertPositionMainScreen = 0x300a,
										staggerMainScreen = 0x380a,
										centerParentWindow = 0xa80a,
										alertPositionParentWindow = 0xb00a,
										staggerParentWindow = 0xb80a,
										centerParentWindowScreen = 0x680a,
										alertPositionParentWindowScreen = 0x700a,
										staggerParentWindowScreen = 0x780a;
#endif
};


#endif /* __MACWINDOWS_R__ */

