// LDAP Directory Services 
//
// File:     readme.h
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 08/13/1997 Qualcomm Inc.  All Rights Reserved.
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
// This file contains a description of the structure of the LDAP.DLL as well
// as a history of major changes to the DLL.
//
#ifndef README_H
#define README_H



/*******************************************************
//
// Structure of LDAP.DLL
//
// 1) IDSPProtocol interface is implemented in the following files: ldap_protocol.cpp, and pldap.h
//
// 2) The IDSPConfig interface is implemented in the following files: ServConfig.cpp, ServConfig.h.
//    This COM object contains the C++ object "class Config" which handles ALL access to configuration
//    data including implementing dialog boxes to allow a user to define new LDAP servers and modify
//    existing configuration data.  This C++ object is implemented in the following files: config.cpp, 
//    and config.h 
//
// 3) The IDSPEnumConfig interface is implemented in the following files: EnumConfig.cpp, and EnumConfig.h.
//
// 4) The IDSPQuery interface is implemented in the following files: query.cpp, query.h.
//    This COM object contains the LDAPV2 COM object which performs all protocol specific actions
//    to satisfy the user's query.  The LDAPV2 COM object is implemented in the following files:
//    ldapv2.cpp, ldapv2_local.cpp, pldapv2.h, decoder.cpp, decoder.h, encoder.cpp, encoder.h, asntags.h.
//    filter.cpp, modify.cpp, netio.cpp, and standard.h.
//
//    In the LDAPV2 COM object ALL ASN.1 encoding is implemented by the C++ object "class AsnEncoder" as 
//    implemented in encoder.cpp, encoder.h, and asntags.h.
//    In the LDAPV2 COM object ALL ASN.1 decoding is implemented by the C++ object "class AsnDecoder" as 
//    implemented in decoder.cpp, decoder.h, and asntags.h.
//
//    Each outgoing LDAP PDU gets its own AsnEncoder object, as well as each incoming PDU gets its own
//    AsnDecoder object.  This with this implementation multiple incoming and outgoing PDUs could be in
//    transit at any one time, and any number of incoming PDUs can be parsed at the same time. 
//    (Currently the implementation of IDSPQuery only uses a single outstanding request at a time.)
//
//   
// 5) The IDSPRecord interface is implemented in the following files: records.cpp, and records.h.
//
//
// 6) The following files support the loading and unloading of the LDAP DLL and its COM components: main.cpp, 
//    registry.cpp, registry.h, factory.cpp, factory_private.cpp, factory.h.
//
// 7) The log.cpp and log.h files support run-time, in the field logging of protocol interactions as well
//    as displaying the loading of all configuration data into the IDSPCOnfig COM Objects.
//
// 8) The util.cpp and util.h files define general purpose functions shared over the entire DLL.
//
// 
// Dependencies
//
// 1) This DLL requires the following COM objects to be available: CLSID_ISock (used for all network IO), 
//    CLSID_ISchedule (used to schedule asynch events), CLSID_IListMan (defines a linked list), and
//    the IID_IDirConfig object obtained from the creator of our IDSPProtocol COM object (used to read/write
//    configuration data to the registry or anywhere else).  
//
//
// Main References
// 1) D.Rogerson, Inside COM, Microsoft Press, 1997.
// 2) RFCs: 1777, 1779, 1778, 1823
// 3) The Directory: Selected Attribute Syntaxes, CITT Recommendation X.520.
// 4) M.Rose, The Open Book: A practical Perspective on OSI, Chapter 8 Abstract Syntax (i.e. ASN.1), Prentice Hall
// 5) D.Steedman, Abstract Syntax Notation One (ASN.1), The Tutorial & Reference, Technology Appraisals Ltd.,
//    1993, ISBN 1 871802 06 7.
// 
// Other References:
// 1) M.Rose, The Little Black Book: Mail Bonding with OSI Directory Services, Prentice Hall, 1992.
// 2) T.Howes, & M.Smith, LDAP: Programming Directory-Enabled Applications with Lightweight Directory Access Protocol,
//    Macmillan Technology Series, 1997. 
//
*******************************************************/


#endif


