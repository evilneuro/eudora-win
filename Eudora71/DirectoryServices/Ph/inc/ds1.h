/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					ds1.h
Description:		COM interfaces for directory services protocol servers
Date:						7/18/97
Version:  			1.0
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <windows.h>
#include <ole2.h>


// (The following should be in DSAPI.H)
///////////////////////////////////////////////////////////////////////////////
// Example interfaces for Protocol component...

// forward references
interface IConfig;
interface IEnumConfig;
interface IProtocol;


interface IConfig : IUnknown
{
	virtual HRESULT	__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HICON		__stdcall GetImage() = 0;
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HICON		__stdcall GetProtocolImage() = 0;
	virtual int			__stdcall GetPropPageCount() = 0;
	virtual HRESULT	__stdcall GetPropPages(HPROPSHEETPAGE * pHPropPages) = 0;
};


interface IEnumConfig : IUnknown
{
	virtual HRESULT __stdcall Next(ULONG celt, IConfig ** rgelt, ULONG * pceltFetched) = 0;
	virtual HRESULT __stdcall Skip(ULONG celt) = 0;
	virtual HRESULT __stdcall Reset(void) = 0;
	virtual HRESULT __stdcall Clone(IEnumConfig ** ppenum) = 0;
};


interface IProtocol : IUnknown
{
	// ----------------------------------------------------------------------- 
	// Initialize()
	// Initialize will QueryInterface() the caller for IID_IConfigData so that
	// the protocol and its components can read and write configuration
	// information in an abstracted manner through this interface.
	// -----------------------------------------------------------------------
	virtual HRESULT	__stdcall Initialize(IUnknown *pUnkCaller) = 0;
	virtual HRESULT __stdcall CreateObject(REFIID riid,LPVOID *ppv) = 0;
	virtual HRESULT __stdcall EnumDatabases(IEnumConfig **ppenum) = 0;
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize) = 0;
	virtual HICON		__stdcall GetProtocolImage() = 0;
};


