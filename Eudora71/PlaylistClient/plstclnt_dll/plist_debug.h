// plist_mgr.h -- the playlist manager. user level access and scheduling.
//
// Copyright (c) 1999 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//


#ifndef _PLIST_DEBUG_H_
#define _PLIST_DEBUG_H_

#ifdef PLIST_LOGGING
#ifndef _PLIST_XMLRPC_H_
#ifndef _PLIST_CACHE_H_
// basic mfc, and mfc/wininet
#include "afx.h"
#include "afxwin.h"
#include "afxinet.h"
#endif
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PLIST_LOGGING

#ifdef PLIST_LOGGING
#define SDTrace(a) do{LogAString(a);}while(0)
#define SDTrace1(fmt,a1) do{char s[1024];sprintf(s,fmt,a1);LogAString(s);}while(0)
#define SDTrace2(fmt,a1,a2) do{char s[1024];sprintf(s,fmt,a1,a2);LogAString(s);}while(0)
#define SDTrace3(fmt,a1,a2,a3) do{char s[1024];sprintf(s,fmt,a1,a2,a3);LogAString(s);}while(0)
#define SDTrace4(fmt,a1,a2,a3,a4) do{char s[1024];sprintf(s,fmt,a1,a2,a3,a4);LogAString(s);}while(0)
#define SDTrace5(fmt,a1,a2,a3,a4,a5) do{char s[1024];sprintf(s,fmt,a1,a2,a3,a4,a5);LogAString(s);}while(0)
#define SDTrace6(fmt,a1,a2,a3,a4,a5,a6) do{char s[1024];sprintf(s,fmt,a1,a2,a3,a4,a5,a6);LogAString(s);}while(0)
#define SDTrace7(fmt,a1,a2,a3,a4,a5,a6,a7) do{char s[1024];sprintf(s,fmt,a1,a2,a3,a4,a5,a6,a7);LogAString(s);}while(0)
void OutputDebugStrings(const char *pfx,const char *s);
#define SDCharify(x)	(x>>24)&0xff,(x>>16)&0xff,(x>>8)&0xff,(x)&0xff
#define SDTraceLastError(c)	do{if (c) SDTrace(GetLastErrorStr());}while(0)
char *GetLastErrorStr(void);
void LogAString(const char *logMe);
#else
#define SDTrace(s)
#define SDTrace1(fmt,a1)
#define SDTrace2(fmt,a1,a2)
#define SDTrace3(fmt,a1,a2,a3)
#define SDTrace4(fmt,a1,a2,a3,a4)
#define SDTrace5(fmt,a1,a2,a3,a4,a5)
#define SDTrace6(fmt,a1,a2,a3,a4,a5,a6)
#define SDTrace7(fmt,a1,a2,a3,a4,a5,a6,a7)
#define OutputDebugStrings(p,s)
#define SDTraceLastError(c)
#endif

#ifdef PLIST_VERBOSE_LOGGING
#define VSDTrace(a) SDTrace(a)
#define VSDTrace1(fmt,a1) SDTrace1(fmt,a1)
#define VSDTrace2(fmt,a1,a2) SDTrace2(fmt,a1,a2)
#define VSDTrace3(fmt,a1,a2,a3) SDTrace3(fmt,a1,a2,a3)
#define VSDTrace4(fmt,a1,a2,a3,a4) SDTrace4(fmt,a1,a2,a3,a4)
#define VSDTrace5(fmt,a1,a2,a3,a4,a5) SDTrace5(fmt,a1,a2,a3,a4,a5)
#define VSDTrace6(fmt,a1,a2,a3,a4,a5,a6) SDTrace6(fmt,a1,a2,a3,a4,a5,a6)
#define VSDTrace7(fmt,a1,a2,a3,a4,a5,a6,a7) SDTrace7(fmt,a1,a2,a3,a4,a5,a6,a7)
#else
#define VSDTrace(s)
#define VSDTrace1(fmt,a1)
#define VSDTrace2(fmt,a1,a2)
#define VSDTrace3(fmt,a1,a2,a3)
#define VSDTrace4(fmt,a1,a2,a3,a4)
#define VSDTrace5(fmt,a1,a2,a3,a4,a5)
#define VSDTrace6(fmt,a1,a2,a3,a4,a5,a6)
#define VSDTrace7(fmt,a1,a2,a3,a4,a5,a6,a7)
#endif

#ifdef __cplusplus
}
#endif

#endif    // _PLIST_DEBUG_H_
