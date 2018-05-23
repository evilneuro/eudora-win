// LDAP Directory Services 
//
// File:     ildap.h    
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 06/17/1997 Qualcomm Inc.  All Rights Reserved.
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
// This file provides the standard Directory Services API to the LDAP protocol engine.
//
#ifndef ILDAP_H
#define ILDAP_H

#include <ildapv2.h>
#include <dspapi.h>


// 
//
typedef class ILDAP* LPILDAP;
class ILDAP : public IUnknown 
{
      public:
  	  virtual HRESULT	__stdcall Initialize( IUnknown *pUnkCaller ) = 0;
  	  virtual HRESULT 	__stdcall CreateObject( REFIID riid,LPVOID *ppv ) = 0;
	  virtual HRESULT 	__stdcall DeleteObject(IUnknown * punkObject) = 0;
	  virtual HRESULT	__stdcall EnumDatabases( IDSPEnumConfig **ppenum ) = 0;
	  virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	  virtual HBITMAP	__stdcall GetProtocolImage( int iImage ) = 0;
	  virtual DWORD		__stdcall GetProtocolFlags() = 0;
};

#endif

