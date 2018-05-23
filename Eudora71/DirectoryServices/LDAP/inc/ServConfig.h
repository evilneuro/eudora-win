// LDAP Directory Services 
//
// File:     ServConfig.h
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 07/25/1997 Qualcomm Inc.  All Rights Reserved.
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
// Defines the COM Object and interfaces to support IPConfig directory services.
//
//
#ifndef SERVCONFIG_H
#define SERVCONFIG_H

#include "config.h"
#include "log.h"



typedef class IExistingConfig* LPIEXISTCFG;
// Private interface on ServerConfig Object used to initialize with existing configuration data
//
class IExistingConfig : public IUnknown
{
      public:
      virtual HRESULT __stdcall Initialize( IDirConfig* ConfigIO, LPSTR ServerKey, LPSTR SectionKey ) = 0;
      // -> remove all configuration data from its persistent storage
      virtual HRESULT __stdcall DeleteConfiguration() = 0;
      // -> access loaded configuration data (NOTE: don't free returned pointer in properties)
	  virtual HRESULT __stdcall GetServerProperties( Server_t** properties ) = 0; 
      // -> translate an unreadble X.500 like name to a user friendly string  (NOTE: don't free returned pointer in szUserDefined)
      virtual HRESULT __stdcall MapAttribName( LPSTR szName, DS_FIELD* internal_name, DS_TYPE* data_type, LPSTR* szUserDefined ) = 0;
};
// {F16429E1-054B-11d1-8866-00805F8A0D74}
DEFINE_GUID( IID_ExistConfig, 0xf16429e1, 0x54b, 0x11d1, 0x88, 0x66, 0x0, 0x80, 0x5f, 0x8a, 0xd, 0x74);



// COM object to all configuration data for LDAP servers.
//
typedef class CServerConfig* LPSRVCRG;
class CServerConfig : public IDSPConfig, public IExistingConfig
{
      public:
	  //  -> standard IUnknown interface methods
	  virtual HRESULT   __stdcall QueryInterface( REFIID refid, LPVOID* lpObj );
	  virtual ULONG		__stdcall AddRef();
	  virtual ULONG		__stdcall Release();

      // -> IDSPConfig Interface
	  virtual HRESULT	__stdcall GetName( LPSTR lpszBuffer, UINT nBufferSize );
	  virtual HBITMAP	__stdcall GetImage( int iImage );
	  virtual LPSTR	    __stdcall GetID();
	  virtual int		__stdcall GetPropPageCount();
	  virtual HRESULT	__stdcall GetPropPages( HPROPSHEETPAGE *pPropPages );
	  virtual HRESULT	__stdcall GetProtocolName( LPSTR lpszBuffer, UINT nBufferSize );
	  virtual HBITMAP	__stdcall GetProtocolImage( int iImage );
	  virtual DWORD		__stdcall GetProtocolFlags();
	  virtual HRESULT 	__stdcall CreateObject( REFIID riid,LPVOID *ppv );

      // -> IExistingConfig Interface
      virtual HRESULT   __stdcall Initialize( IDirConfig* ConfigIO, LPSTR ServerName, LPSTR ConfigKey );
      virtual HRESULT   __stdcall DeleteConfiguration();
	  virtual HRESULT   __stdcall GetServerProperties( Server_t** properties ); 
      virtual HRESULT   __stdcall MapAttribName( LPSTR szName, DS_FIELD* internal_name, DS_TYPE* data_type, LPSTR* szUserDefined );

      CServerConfig( HBITMAP hImage );
      virtual ~CServerConfig();
      virtual BOOL IsObjectValid() { return( m_ConfigObj != NULL && m_ConfigIO != NULL ); };


      protected:
      // Internal Data & Type Members
      LONG         m_Ref;                    //
      HBITMAP      m_hProtocolImage;         // Shared with LDAP object (so we do not free this resource)
      LPLOG        m_LogObj;                 // For debug and in the field monitoring
      IDirConfig*  m_ConfigIO;               // COM object that performs configuration IO for us---hides where the configuration data is actually stored
	  LPCONFIG     m_ConfigObj;              // C++ object obtains all configuration data from this server
};

#endif

