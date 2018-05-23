// Directory Services 
//
// File:     ldaperr.h    
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 04/28/1997 Qualcomm Inc.  All Rights Reserved.
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
// Description:
// result parsing function calls in order to extract query results and errors. 
//
#ifndef LDAPERR_H
#define LDAPERR_H


// -> Success codes
#define LDAP_SUCCESS					 0    // request ended sucessfully
#define LDAP_REQUEST_PENDING			 1    // request is in progress
#define LDAP_COMPAREFALSE				 2    //
#define LDAP_COMPARETRUE				 3    //

// -> failure codes
#define LDAP_OPERATIONSERROR			-1
#define LDAP_PROTOCOLERROR				-2
#define LDAP_TIMELIMITEXCEEDED			-3
#define LDAP_SIZELIMITEXCEEDED			-4
#define LDAP_AUTHMETHODNOTSUPPORTED		-5
#define LDAP_STRONGAUTHREQUIRED			-6
#define LDAP_NOSUCHATTRIBUTE			-7
#define LDAP_UNDEFINEDATTRTYPE     		-8
#define LDAP_INAPPROPIATEMATCING		-9
#define LDAP_CONSTRAINTVIOLATION		-10
#define LDAP_ATTRORVALUEEXISTS			-11
#define LDAP_INVALIDATTRSYNTAX			-12
#define LDAP_NOSUCHOBJECT				-13
#define LDAP_ALIASPROBLEM				-14
#define LDAP_INVALIDDNSYNTAX			-15
#define LDAP_ISLEAF     				-16
#define LDAP_ALIASDEREFPROBLEM			-17
#define LDAP_INAPPROPRIATEAUTH			-18
#define LDAP_INVALIDCREDENTIALS			-19
#define LDAP_INSUFFICENTRIGHTS			-20
#define LDAP_BUSY   					-21
#define LDAP_UNAVAILABLE				-22
#define LDAP_UNWILLINGTOPERFORM			-23
#define LDAP_LOOPDETECT					-24
#define LDAP_NAMINGVIOLATION			-25
#define LDAP_OBJCLASSVIOLATION			-26
#define LDAP_NOTALLOWEDONNONLEAF        -27
#define LDAP_NOTALLOWEDONRDN     		-28
#define LDAP_ENTRYALREADYEXISTS  		-29
#define LDAP_NOOBJCLASSMODS        		-30
#define LDAP_OTHER                      -31
#define LDAP_EXP_CONNECTIONCLOSED       -32   // Exception code added by us, not standard LDAP error
#define LDAP_EXP_BADHOSTADDR            -33   // Exception code added by us, not standard LDAP error
#define LDAP_EXP_CANNOTREAD             -34
#define LDAP_EXP_CANNOTWRITE            -35

#endif


