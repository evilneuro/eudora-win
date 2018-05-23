// LDAP Directory Services 
//
// File:     pldap.h    
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
#ifndef PLDAP_H
#define PLDAP_H

#include <dspapi.h>
//#include "pldapv2.h"
#include "ServConfig.h"
#include "config.h"
#include "log.h"



// Item in list of IPConfig Objects which represent configured LDAP servers.
//
typedef struct ExistingServer {
    // -> pointers required by IList object
	struct ExistingServer*    pPrev;        // 
	struct ExistingServer*    pNext;        // 
                                            //
    LPSRVCRG                  ConfigObj;    // IPConfig COM object filled with data
} Existing_t;

typedef Existing_t* LPEXISTING;



// 
//
typedef class LDAP* LPLDAP;
class LDAP : public IDSPProtocol 
{
      public:
	  //  -> standard IUnknown interface methods
	  virtual HRESULT   __stdcall QueryInterface( REFIID refid, LPVOID* lpObj );
	  virtual ULONG		__stdcall AddRef();
	  virtual ULONG		__stdcall Release();

   	  virtual HRESULT	__stdcall Initialize( IUnknown* pUnkCaller );
  	  virtual HRESULT 	__stdcall CreateObject( REFIID riid, LPVOID* ppv );
	  virtual HRESULT 	__stdcall DeleteObject( IUnknown* punkObject);
	  virtual HRESULT	__stdcall EnumDatabases( IDSPEnumConfig **ppenum );
	  virtual HRESULT	__stdcall GetProtocolName( LPSTR lpszBuffer, UINT nBufferSize );
	  virtual HBITMAP	__stdcall GetProtocolImage( int iImage );
	  virtual DWORD		__stdcall GetProtocolFlags();

      LDAP();
      virtual ~LDAP();

      //  -> is the object in a proper state for exectution ?
      virtual BOOL IsObjectValid();         
      static  void ListFreeCB( LPITEM pItem, LPVOID pUserData );


      protected:
      virtual HRESULT   __stdcall NewConfigObj( LPVOID* ppv, LPSTR ServerName, LPSTR ConfigKey );
      virtual BOOL Free_ExistingConfig( IUnknown* punkObject );

      // Internal Data & Type Members
      ULONG        m_Ref;                    //
	  IUnknown*	   m_pUnkCaller;			 // IUnknown of our Creator so we may kneel and worship
      HBITMAP      m_hProtocolImage;         //
                                             //
	  LPLOG        m_LogObj;                 // For debug and in the field monitoring
      IDirConfig*  m_ConfigIO;               // COM object that performs configuration IO for us---hides where the configuration data is actually stored
	  LPCONFIG     m_ConfigObj;              // Obtain all configuration data from this object
											 //
      LPILISTMAN   m_Servers;                // A list of existing LDAP servers
};


#endif

