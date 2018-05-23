// LDAP Directory Services 
//
// File:     records.h
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 08/06/1997 Qualcomm Inc.  All Rights Reserved.
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
// Defines the COM Object and interfaces to support IDSPRecord COM object.
//
//
#ifndef RECORDS_H
#define RECORDS_H

#include "log.h"



typedef class ISetRecord* LPISETREC;
// Private interface on CRecord Object used to initialize the list of items in a single record of data from a search hit
//
class ISetRecord : public IUnknown
{
      public:
      // -> the "DBRECENT" structure MUST be malloced but any data pointed to was allocated with the C++ new operator.
      virtual HRESULT __stdcall AddItem( LPDBRECENT pItem ) = 0;
      // -> modify access to non-record hit data
      virtual HRESULT __stdcall SetRecordId( LPSTR szRecordID ) = 0;
      virtual HRESULT __stdcall SetDBId( LPSTR szDbID ) = 0;
      virtual HRESULT __stdcall SetErrorStr( QUERY_STATUS ResultCode, int LDAP_ResultCode ) = 0;
};
// {EA0BD571-0E88-11d1-8869-00805F8A0D74}
DEFINE_GUID( IID_ISetRecord, 0xea0bd571, 0xe88, 0x11d1, 0x88, 0x69, 0x0, 0x80, 0x5f, 0x8a, 0xd, 0x74);



// COM object holding all the data associated with one search hit.
// This solves the memory free problem in Win32 systems (i.e., he who mallocs must free his own memory)
//
typedef class CRecord* LPREC;
class CRecord : public IDSPRecord, public ISetRecord
{
      public:
	  //  -> standard IUnknown interface methods
	  virtual HRESULT      __stdcall QueryInterface( REFIID refid, LPVOID* lpObj );
	  virtual ULONG	       __stdcall AddRef();
	  virtual ULONG		   __stdcall Release();

      // -> IDSPRecord Interface
  	  virtual DBRECENT*	   __stdcall GetRecordList();
	  virtual LPSTR		   __stdcall GetRecordID();
  	  virtual LPSTR        __stdcall GetDatabaseID();
	  virtual QUERY_STATUS __stdcall GetError( LPSTR pszBuffer, int nLen );

      // -> ISetRecord Interface
      virtual HRESULT      __stdcall AddItem( LPDBRECENT pItem );
      virtual HRESULT      __stdcall SetRecordId( LPSTR szRecordID );
      virtual HRESULT      __stdcall SetDBId( LPSTR szDbID );
      virtual HRESULT      __stdcall SetErrorStr( QUERY_STATUS ResultCode, int LDAP_ResultCode );

      CRecord();
      virtual ~CRecord();


      protected:
      // Internal Data & Type Members
      LONG          m_Ref;             //
      LPDBRECENT    m_ItemHead;        // a list of items composing a single record of data from a search hit
      LPDBRECENT    m_ItemTail;        // tail of list started by m_ItemHead
      LPSTR         m_szRecordID;      // for us this is a: LDAP DN of data
      LPSTR         m_szDBID;          // UID defining a particular LDAP server configuration data
      QUERY_STATUS  m_QueryStatus;     // 
      int           m_LDAPCode;        // printable error strings are generated from the LDAP result code
};

#endif

