// Directory Services 
//
// File:     asntags.h
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 04/24/1997 Qualcomm Inc.  All Rights Reserved.
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
// References:
// (1) Douglas Steedman, ASN.1 The Tutorial & Reference,ISBN 1 871802 06 7
// (2) Marshall Rose, The Open Book
//
// Description:
// Below are the definitions of those ASN.1 tag values that LDAP uses
// in RFC 1777.
//
// BER format of the Tag Octet:   8 7 6 5 4 3 2 1
//                               +---+-+---------+
// c = tag class                 | c |f| number  |
//     00 is universal           +---+-+---------+
//     01 is application
//     10 is context-specific
//     11 is private-use
// f = 0  is primitive,                             
//     1  is constructed
//
#ifndef ASNTAGS_H
#define ASNTAGS_H


// [A] LDAP Module Related Constants
//
#define MAX_INTEGER          65535      // LDAP restriction on max value of MsgId
#define LDAP_VERSION2        0x02       // expected octet value 
#define LDAP_VERSION3        0x03       // expected octet value
#define BER_TRUE             0x01       // anything > 0 is TRUE in BER
#define BER_FALSE            0x00       // expected octet value

//  -> enumerated values for the scope of a LDAP search 
#define SCOPE_BASEOBJECT     0x00
#define SCOPE_SINGLELEVEL    0x01
#define SCOPE_WHOLESUBTREE   0x02

//  -> used to encode long definite form length fields 
#define LENGTH_IN1_OCTET     0x81       //   8        7 6 5 4 3 2 1  <--- bit index
#define LENGTH_IN2_OCTET     0x82       // +---+---------------------------+
#define LENGTH_IN3_OCTET     0x83       // | 1 | number of bytes to follow |
                                        // +---+---------------------------+

// [B] Constructed Tags
//
#define SEQUENCE_TAG         0x30       // same as SEQUENCE OF
#define SET_TAG              0x31       // same as SET OF
                                        //
#define C_APPL0_TAG          0x60       // BindRequest --- Construted Application tag 0
#define C_APPL1_TAG          0x61		// BindResponse
#define C_APPL2_TAG          0x62		// 
#define C_APPL3_TAG          0x63		// SearchRequest
#define C_APPL4_TAG          0x64		// Search Entry Response (database data returned)
#define C_APPL5_TAG          0x65       // Search Result Code Response (marker for end of search reached)
#define C_APPL6_TAG          0x66		// ModifyRequest
#define C_APPL7_TAG          0x67		// ModifyResponse
#define C_APPL8_TAG          0x68		// AddRequest
#define C_APPL9_TAG          0x69		// AddResponse
#define C_APPL10_TAG         0x6A		// 
#define C_APPL11_TAG         0x6B		// DeleteResponse
#define C_APPL12_TAG         0x6C		// ModifyRDNRequest
#define C_APPL13_TAG         0x6D		// ModifyRDNResponse
#define C_APPL14_TAG         0x6E		// CompareRequest
#define C_APPL15_TAG         0x6F		// CompareResponse
							    		//
#define C_CONTEXT0_TAG       0xA0		// Constructed Context-Specific tag 0
#define C_CONTEXT1_TAG       0xA1		//
#define C_CONTEXT2_TAG       0xA2		//
#define C_CONTEXT3_TAG       0xA3		//
#define C_CONTEXT4_TAG       0xA4		//
#define C_CONTEXT5_TAG       0xA5		//
#define C_CONTEXT6_TAG	     0xA6		//
#define C_CONTEXT7_TAG       0xA7		//
#define C_CONTEXT8_TAG       0xA8       //


// [C] Primitive Tags
//
#define BOOLEAN_TAG          0x01		//
#define INTEGER_TAG          0x02		//
#define OCTETSTRING_TAG      0x04		//
#define ENUMERATED_TAG       0x0A		//
                                        //
#define P_APPL2_TAG          0x42       // UnbindRequest  --- Primitive Application tag 2
#define P_APPL10_TAG         0x4A       // DeleteRequest  --- Primitive Application tag 10
#define P_APPL16_TAG         0x50       // AbandonRequest --- Primitive Application tag 16
                                        //
#define P_CONTEXT0_TAG       0x80       // Primitive Context-Specific tag 0
#define P_CONTEXT1_TAG       0x81		//
#define P_CONTEXT2_TAG       0x82		//
#define P_CONTEXT7_TAG       0x87       //


// [D] Tags for internal Use only
#define PRIVATE_OPEN_TAG     0xFF       // Constructed Private-Use tag 31
#define PRIVATE_UNBIND_TAG   0xFE       // Constructed Private-Use tag 30
#define PRIVATE_ABANDON_TAG  0xFD       // Constructed Private-Use tag 29


#endif
