// Directory Services 
//
// File:     dir_config.h
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0
// Copyright 07/17/1997 Qualcomm Inc.  All Rights Reserved.
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
// Virtualize all access to Directory Service configuration data.  All protocol
// DLLs will be given this COM object to access its necessary configuration data.
// This data could be stored in an INI file, the Win32 Registry, or even via 
// the ACAP (or like) protocol.
//
#ifndef DIRCONFIG_H
#define DIRCONFIG_H

#include <ole2.h>



// Virtual Configuration COM Object
//
// The scheme below is very simple.  The goal is that it map well to storing
// configuration data in to ".ini" files, the Win32 Registry, and onto an 
// ACAP server.
//
// Data Model:
// ----------
//
// There is a main location where all configuration data is stored.  This can
// be set by the parameter in Initialize method or defaults to 
// HKEY_CURRENT_USER\Software\Qualcomm\Shared\DirServices\1.0 for the registry.
// This registry path is stored as a string resouce in this DLL.
//
// Under this main location are a number of sections each given a string section
// name (e.g., "LDAP", "PH", etc.).  Note that sections cannot be nested in sections.
// There can be any number of sections.
//
// Each section can have any number of keys.  Where a key consists of a string name
// (e.g., server-name) and a key value (attribute -- value pair).  The value can be
// one of the following data types: { Integer, Boolean, DWORD, String, Binary data block }.
//
// As an example:
// Section: LDAP
// Key: ldap.four11.com  value: "Port=389 Timeout=3 Seconds"
//
//
typedef class DirConfig* LPDIRCONFIG;
class DirConfig : public IUnknown 
{
    public:
    // Interface Constants and Types
    enum DataType {
         DIR_UNKNOWN = 0,       // -> encoundered data written by other program ?
         DIR_INT     = 1,       // -> a 32 bit signed value
         DIR_BOOL    = 2,       // -> a 16 bit UINT
         DIR_DWORD   = 3,       // -> a 32 bit unsigned value
         DIR_STR     = 4,       // -> an ASCII NULL terminated string
         DIR_BINARY  = 5,       // -> a block of binary data of unlimited size
    };

	// [A] Class Management Methods
	virtual BOOL Initialize( LPSTR ConfigStoredAt ) = 0;

    // [B] Creation Methods
    //  -> creates a new section 
    virtual HRESULT STDMETHODCALLTYPE CreateDirSection( LPSTR szSection ) = 0;
    //  -> creates a new key under the section 
    virtual HRESULT STDMETHODCALLTYPE CreateDirKey( LPSTR szSection, LPSTR szKey ) = 0;     
    //  -> deletes a section, all of its keys, and all of their values
    virtual HRESULT STDMETHODCALLTYPE DeleteDirSection( LPSTR szSection ) = 0;
    //  -> deletes a key and all of its value 
    virtual HRESULT STDMETHODCALLTYPE DeleteDirKey( LPSTR szSection, LPSTR szKey ) = 0;


    // [C] Accessing Methods
    // A key can have only one value, thus the <key, value> pair is an "attribute=value" relationship.
    //
    // Both the section and keys must be created before these functions can be used (i.e., these functions
    // do not automatically create a section that does not exist).
    //
    //  -> return an integer value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE GetDirInteger( LPSTR szSection, LPSTR szKey, int    default_value, int*    returned_value ) = 0;
    //  -> return a Boolean value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE GetDirBool(    LPSTR szSection, LPSTR szKey, BOOL   default_value, BOOL*   returned_value ) = 0;
    //  -> return a DWORD value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE GetDirDword(   LPSTR szSection, LPSTR szKey, DWORD  default_value, DWORD*  returned_value ) = 0;
    //  -> return a string value stored at <szSection, szKey>, into the callers allocated buffer (truncating if necessary), cbReturned indicates size of return_value buffer
    virtual HRESULT STDMETHODCALLTYPE GetDirStr(     LPSTR szSection, LPSTR szKey, LPSTR  default_value, DWORD cbDefault, LPSTR  returned_value, DWORD* cbReturned ) = 0;
    //  -> return a block of binary data stored at <szSection, szKey>, cbReturned modified to indicate size of block (on input it indicates size of return_value buffer)
    virtual HRESULT STDMETHODCALLTYPE GetDirBinary(  LPSTR szSection, LPSTR szKey, LPBYTE default_value, DWORD cbDefault, LPBYTE returned_value, DWORD* cbReturned ) = 0;

    //  -> return the names of each section one at a time (buffer is caller's allocated memory)
    virtual HRESULT STDMETHODCALLTYPE EnumDirSections(  LPSTR szSection, DWORD* cbBuffer ) = 0;     
    //  -> return the names of each key, under the given section, one at a time (buffer is callers allocated memory), also returned is the type of data stored
    virtual HRESULT STDMETHODCALLTYPE EnumDirKeys(      LPSTR szSection, LPSTR szKey, DWORD* cbBuffer, DataType* type_of_stored_value ) = 0;
    //  -> reset internal state so that next call to EnumDirKeys starts from the beginning again
    virtual HRESULT STDMETHODCALLTYPE EndEnumDirKeys(   LPSTR szSection ) = 0;
    

    // [D] Modification Methods
    // A key can have only one value, thus the <key, value> pair is an "attribute=value" relationship.
    //
    // Both the section and keys must be created before these functions can be used (i.e., these functions
    // do not automatically create a section that does not exist).
    //
    //  -> sets an integer value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE SetDirInteger( LPSTR szSection, LPSTR szKey, int    new_value ) = 0;
    //  -> sets a Boolean value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE SetDirBool(    LPSTR szSection, LPSTR szKey, BOOL   new_value ) = 0;
    //  -> sets a DWORD value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE SetDirDword(   LPSTR szSection, LPSTR szKey, DWORD  new_value ) = 0;
    //  -> sets a string value stored at <szSection, szKey>
    virtual HRESULT STDMETHODCALLTYPE SetDirStr(     LPSTR szSection, LPSTR szKey, LPSTR  new_value ) = 0;
    //  -> sets a block of binary data stored at <szSection, szKey>, cbReturned contains the count of byes in new_value
    virtual HRESULT STDMETHODCALLTYPE SetDirBinary(  LPSTR szSection, LPSTR szKey, LPBYTE new_value, DWORD cbReturned ) = 0;


    // [E] Misc Methods
    //
    //  -> Called when one of the above functions returns an "E_" HRESULT (e.g., E_FAIL)
    //  -> returns "0" if no error, otherwise negative number indicates problem (e.g., no such section defined)
    virtual HRESULT STDMETHODCALLTYPE GetDirLastError( LONG* ResultCode ) = 0;
};

 
// GUIDs for DirConfig COM Object
// {ECE47D70-FEC3-11d0-8864-00805F8A0D74}
DEFINE_GUID( CLSID_DirConfig, 0xece47d70, 0xfec3, 0x11d0, 0x88, 0x64, 0x0, 0x80, 0x5f, 0x8a, 0xd, 0x74);
DEFINE_GUID( IID_DirConfig,   0xece47d70, 0xfec3, 0x11d0, 0x88, 0x64, 0x0, 0x80, 0x5f, 0x8a, 0xd, 0x74);

#endif      








