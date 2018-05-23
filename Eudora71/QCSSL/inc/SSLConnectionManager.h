// SSLConnectionManager.cpp
//
// Implementation of ConnectionInfoMap object.  This class maintains
// a collection of ConnectionInfo objects, each specified by a
// unique name, that store information about SSL connections.
//
// Copyright (c) 2001-2003 by QUALCOMM, Incorporated
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

#ifndef _SSLCONNECTIONMANAGER_H_
#define _SSLCONNECTIONMANAGER_H_
#include <afx.h>

#include "cert.h"
#include <windows.h> 
#include <stdio.h>
#include "qcthread.h"
#include <WinCrypt.h>
#include "qcssl.h"

/*
This class is not derived from CMapStringToPtr but rather contains 
CMapStringToPtr object although it is supposed to inherit its functionality
This is containment rather than inheritance. But this is done on purpose because
we want only certain functions to be exposed so that we dont have to add synchronization
to almost every function implemented by CMapStringToPtr-sagar.
*/
class ConnectionInfoMap
{
public:
	~ConnectionInfoMap();

	ConnectionInfo	*AddConnectionInfo(const CString &uniquename);
	bool			 RemoveConnectionInfo(const CString &uniquename);
	ConnectionInfo	*GetConnectionInfo(const CString &uniquename);

	CMapStringToPtr	 m_PersonaMap;
	QCMutex			 m_Mutex;
};

#endif // #ifdef _SSLCONNECTIONMANAGER_H_
