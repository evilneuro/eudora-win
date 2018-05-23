// LDAP Directory Services 
//
// File: 			factory_private.cpp
// Date:			7/19/97
// Copyright		07/19/1997 Qualcomm Inc.  All Rights Reserved.
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

// Version:  		1.0 
// Description:		Implementation Specific parts of our class factory
//
#pragma warning(disable : 4514)
#include <afx.h>

#include <objbase.h>
#include "DebugNewHelpers.h"
#include "standard.h"
#include "pldap.h"
#include "registry.h"
#include "factory.h"

// Allocate a new LDAP IProtocol Object  
//
IUnknown* Create_LDAP_DirServ()
{ 
	LPLDAP LdapObj = DEBUG_NEW_NOTHROW LDAP();
	if (LdapObj == 0) {
        return( NULL );
	}

	if (!LdapObj->IsObjectValid()) {
        delete LdapObj;
        return( NULL );        
	}
    return((IUnknown*) LdapObj ); 
}

// Allocate a new LDAPV2 Object
//
/*
IUnknown* Create_LDAPV2()
{ 
	LPLDAPV2 LdapV2 = new LDAPV2();
	if (LdapV2 == 0) {
        return( NULL );
	}
	fBaptizeBlockMT(LdapV2, "LDAP-::Create_LDAPV2");
    if (!LdapV2->IsObjectValid()) {
        delete LdapV2;
        return( NULL );        
	}
    return((IUnknown*) LdapV2 );
}
*/


// Table that tells the generic class factory code what to do
//
CFactoryData g_FactoryDataArray[1] =
{
	{ &CLSID_ILdap,   Create_LDAP_DirServ, NULL, NULL, NULL, NULL },
};

UINT g_cFactoryDataEntries = sizeof(g_FactoryDataArray) / sizeof(CFactoryData);


