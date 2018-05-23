/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					private.h
Description:		Private interface for the config object for PH protocol server
Date:						8/11/97
Version:  			1.0 
Module:					PH.DLL (PH protocol directory service object)
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
#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include <objbase.h>


interface IPrivateConfig : IUnknown
{
	virtual LPSTR __stdcall GetHostName() = 0;
	virtual int   __stdcall GetPortNumber() = 0;
	virtual LPSTR __stdcall ParseLine(LPSTR lpsz, int * pds) = 0;
	// smohanty: 04/21/98
	virtual LPCSTR __stdcall GetMailDomain() = 0;
	virtual BOOL   __stdcall SetMailDomain(LPCSTR pszMailDomain) = 0;
};


// Private interface to our PH and Finger IDSPConfig objects
// {A15FBFE0-1281-11d1-8870-00805F4AB1BF}
#define I_STR_PRIVATECONFIG					L"{A15FBFE0-1281-11d1-8870-00805F4AB1BF}"
DEFINE_GUID(IID_IPrivateConfig, 		0xa15fbfe0, 0x1281, 0x11d1, 0x88, 0x70, 0x0, 0x80, 0x5f, 0x4a, 0xb1, 0xbf);



#endif		//__PRIVATE_H__
