// Directory Services 
//
// File:     standard.h    
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
// Types common to all LDAP code.
//
#ifndef STANDARD_H
#define STANDARD_H

#include <ole2.h>
#include <isock.h>
#include <IList.h>
#include <ildapv2.h>



typedef unsigned char OCTET;

// -> overlay on top of a DWORD -- is this ordering correct ?
typedef struct {
        OCTET  byte0;  // LSB 
        OCTET  byte1;  //
        OCTET  byte2;  //
        OCTET  byte3;  // MSB
} DWOverlay_t;

// -> overlay on top of a WORD 
typedef struct {
        OCTET  byte0;  // LSB
        OCTET  byte1;  // MSB
} WOverlay_t;



// -> pre-define before classes are defined
typedef class AsnDecoder*  LPDECODER;
typedef class AsnEncoder*  LPENCODER;

// -> items to be placed into ILists
typedef struct PendingItem {
    // -> pointers required by IList object
	struct PendingItem*    pPrev;           // 
	struct PendingItem*    pNext;           // 
                                            //
    // -> common to all Pending actions     //
    UINT                   MsgId;           // LDAP request ID
    OCTET                  RespOpCode;      // Expected LDAP response BER code
    LPVOID                 lpCompletion;    // caller's callback address on completion
    LPVOID                 UserData;        //
                                            //
    // -> used for outgoing Requests only   //
    LPENCODER              Encoder;         // An AsnEncoder object
    BOOL                   sending;         // it may take several socket writes to send the entire PDU
    BOOL                   abandoned;       // what if caller abandones a request that is currently being sent ?
                                            //
    // -> used for incoming Responses only  //
    LPDECODER              Decoder;         // An AsnDecoder object
    OCTET*                 ldap_pdu;        // if not NULL, then incomplete PDU filling up
    DWORD                  next_octet;      // offset of next free octet to write to
	DWORD                  pdu_size;        // expected number of octets that make up this PDU
    int                    ParseAttrib;     // At the begining or middle of a list of attributes
    int                    ParseValues;     // At the begining or middle of a list of attribute values
                                            //
} Pending_t;

typedef Pending_t* LPPENDING;


#endif

