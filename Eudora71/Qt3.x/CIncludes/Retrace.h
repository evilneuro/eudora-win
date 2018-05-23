/*
 	File:		Retrace.h
 
 	Contains:	Vertical Retrace Interfaces.
 
 	Version:	Technology:	System 7.5
 				Release:	QuickTime 3.0
 
 	Copyright:	© 1985-1993, 1995-1998 by Apple Computer, Inc., all rights reserved
 
 	Bugs?:		Please include the the file and version information (from above) with
 				the problem description.  Developers belonging to one of the Apple
 				developer programs can submit bug reports to:
 
 					devsupport@apple.com
 
*/
#ifndef __RETRACE__
#define __RETRACE__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif
#ifndef __OSUTILS__
#include <OSUtils.h>
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

typedef struct VBLTask 					VBLTask;
typedef VBLTask *						VBLTaskPtr;
typedef CALLBACK_API( void , VBLProcPtr )(VBLTaskPtr vblTaskPtr);
/*
	WARNING: VBLProcPtr uses register based parameters under classic 68k
			 and cannot be written in a high-level language without 
			 the help of mixed mode or assembly glue.
*/
typedef REGISTER_UPP_TYPE(VBLProcPtr) 							VBLUPP;

struct VBLTask {
	QElemPtr 						qLink;
	short 							qType;
	VBLUPP 							vblAddr;
	short 							vblCount;
	short 							vblPhase;
};

enum { uppVBLProcInfo = 0x00009802 }; 							/* register no_return_value Func(4_bytes:A0) */
#define NewVBLProc(userRoutine) 								(VBLUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppVBLProcInfo, GetCurrentArchitecture())
#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
	#pragma parameter CallVBLProc(__A1, __A0)
	void CallVBLProc(VBLUPP routine, VBLTaskPtr vblTaskPtr) = 0x4E91;
#else
	#define CallVBLProc(userRoutine, vblTaskPtr) 				CALL_ONE_PARAMETER_UPP((userRoutine), uppVBLProcInfo, (vblTaskPtr))
#endif
EXTERN_API( QHdrPtr )
GetVBLQHdr						(void)														THREEWORDINLINE(0x2EBC, 0x0000, 0x0160);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 SlotVInstall(__A0, __D0)
																							#endif
EXTERN_API( OSErr )
SlotVInstall					(QElemPtr 				vblBlockPtr,
								 short 					theSlot)							ONEWORDINLINE(0xA06F);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 SlotVRemove(__A0, __D0)
																							#endif
EXTERN_API( OSErr )
SlotVRemove						(QElemPtr 				vblBlockPtr,
								 short 					theSlot)							ONEWORDINLINE(0xA070);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 AttachVBL(__D0)
																							#endif
EXTERN_API( OSErr )
AttachVBL						(short 					theSlot)							ONEWORDINLINE(0xA071);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 DoVBLTask(__D0)
																							#endif
EXTERN_API( OSErr )
DoVBLTask						(short 					theSlot)							ONEWORDINLINE(0xA072);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 VInstall(__A0)
																							#endif
EXTERN_API( OSErr )
VInstall						(QElemPtr 				vblTaskPtr)							ONEWORDINLINE(0xA033);

																							#if TARGET_OS_MAC && TARGET_CPU_68K && !TARGET_RT_MAC_CFM
																							#pragma parameter __D0 VRemove(__A0)
																							#endif
EXTERN_API( OSErr )
VRemove							(QElemPtr 				vblTaskPtr)							ONEWORDINLINE(0xA034);




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

#endif /* __RETRACE__ */

