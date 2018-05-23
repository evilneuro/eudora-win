// HostList.h
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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

#ifndef _HOST_LIST_H_
#define _HOST_LIST_H_

//Forward declarations
class CPopHost;
class JJFile;

#include "afxmt.h"
#include "pophost.h"

class CHostList : public CList<CPopHost *, CPopHost *> //CSortedList
{
public:
	static CHostList* CreateNewHostList();
	static void WriteLMOSFile();
	static CPopHost* GetHostForCurrentPersona();
	static CPopHost* GetHost(long nLMOSBackupCount, const char* strPersona, const char* strPOPAccount = NULL);
	
	static CPopHost *GetHostWithMsgHash(long Hash);
    
	static void Cleanup();
	~CHostList();
	
protected:
	//
	// FORNOW, very bad style.  This is a non-virtual override
	// of the virtual CSortedList::Compare(CMsgRecord*, CMsgRecord*) 
	// method from the base class.  It is non-virtual since it doesn't
	// match the prototype of the base class method.  Is this really 
	// what we intended?  WKS 97.07.02.
	//
	short Compare(CPopHost *, CPopHost *);

private:
	
	static CHostList* MakeNewHostList();
	
	void InsertPopHost(CPopHost *);
	static CPopHost* MakeNewHost_(JJFile* pLMOSFile, bool *); 
	static CCriticalSection m_HostListGuard;
};



#endif