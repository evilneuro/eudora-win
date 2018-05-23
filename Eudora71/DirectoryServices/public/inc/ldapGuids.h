// Directory Services  
//
// File:     ldapGuids.h
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 04/22/1997 Qualcomm Inc.  All Rights Reserved.
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
// Object IDs for LDAP directory protocol objects.
//
//
// GUID for LDAPV2 protocol engine object
// {8B38ED72-D3BF-11d0-893A-0020AF9BB32F}
DEFINE_GUID( CLSID_ILdapv2, 0x8b38ed72, 0xd3bf, 0x11d0, 0x89, 0x3a, 0x0, 0x20, 0xaf, 0x9b, 0xb3, 0x2f);
DEFINE_GUID( IID_LDAPV2,    0x8b38ed72, 0xd3bf, 0x11d0, 0x89, 0x3a, 0x0, 0x20, 0xaf, 0x9b, 0xb3, 0x2f);

// GUID for LDAP wrapper object
// {9D459211-E75E-11d0-885A-00805F8A0D74}
DEFINE_GUID( CLSID_ILdap,   0x9d459211, 0xe75e, 0x11d0, 0x88, 0x5a, 0x0, 0x80, 0x5f, 0x8a, 0xd, 0x74);
DEFINE_GUID( IID_LDAP,      0x9d459211, 0xe75e, 0x11d0, 0x88, 0x5a, 0x0, 0x80, 0x5f, 0x8a, 0xd, 0x74);

