// SSLConnectionManager.cpp
//
// Implementation of ConnectionInfoMap object.
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

#include "stdafx.h"

#include "SSLConnectionManager.h"

#include "DebugNewHelpers.h"

ConnectionInfoMap::~ConnectionInfoMap()
{
	m_Mutex.Lock();
	POSITION pos;	
	CString csKey;
    for (pos = m_PersonaMap.GetStartPosition(); pos != NULL;)
	{
		ConnectionInfo *pInfo = NULL;
		m_PersonaMap.GetNextAssoc(pos, csKey, (void*&)pInfo);
		if (pInfo)
		{
			pInfo->DeleteCertFiles();
			delete pInfo;
		}
	}
    m_PersonaMap.RemoveAll();
	m_Mutex.Unlock();
}

//
//	ConnectionInfoMap::AddConnectionInfo()
//
//	Create a ConnectionInfo object for the connection specified by uniquename
//	and add it to the map of connection objects.  If the protocol specified
//	in uniquename is not IMAP delete any pre-existing ConnectionInfo object
//	in the map with the same uniquename.  There might be any number of
//	IMAP/SSL connections for the same personality going at the same time
//	and they need to share the same connection object so that accessing the
//	"Last SSL Info" for this personality/protocol pair will yield the expected
//	data.
//
//	Parameters:
//		uniquename [in] - Unique name to specify a given ConnectionInfo object.
//
//	Note: For this to work with multiple IMAP sessions, we assume uniquename
//	consists of "<personality>\n<protocol>".  If this assumption ever changes
//	this code will need to change as well.  All in all, this technique is
//	probably a little too fragile, but for now it will do. -dwiggins
//
ConnectionInfo *ConnectionInfoMap::AddConnectionInfo(const CString &uniquename)
{
	m_Mutex.Lock();
	ConnectionInfo *pConnectionInfo = NULL;
	// If a ConnectionInfo object exists matching uniquename deal with it.
	if (m_PersonaMap.Lookup(uniquename, (void*&)pConnectionInfo))
	{
		// IMAP might have multiple connections at the same time and they
		// can share the ConnectionInfo object so don't delete the object
		// if this is an IMAP connection.
		bool	 bIsIMAP = false;
		char	*strProtocol = strchr(uniquename, '\n');
		if (strProtocol)
		{
			++strProtocol;
			if (strcmp(strProtocol, "IMAP") == 0)
			{
				bIsIMAP = true;
			}
		}
		// If this isn't an IMAP connection delete the object now.
		if (!bIsIMAP)
		{
			pConnectionInfo->DeleteCertFiles();
			delete pConnectionInfo;
			pConnectionInfo = NULL;
		}
	}
	// If we don't have a ConnectionInfo object (because none exists in map
	// or a non-IMAP connection did exist but was deleted) create one and
	// add it to the map.
	if (!pConnectionInfo)
	{
		pConnectionInfo = DEBUG_NEW ConnectionInfo();
		pConnectionInfo->m_UniqueName = uniquename;
		m_PersonaMap.SetAt(uniquename, (void*)pConnectionInfo);
	}
	m_Mutex.Unlock();
	return pConnectionInfo;
}

//
//	ConnectionInfoMap::RemoveConnectionInfo()
//
//	Delete the ConnectionInfo object specified by uniquename.
//
//	Parameters:
//		uniquename [in] - Unique name to specify a given ConnectionInfo object.
//
//	Note: This method is currently unused.  The only place ConnectionInfo
//	objects are deleted are in the destructor and in AddConnectionInfo().
//	I'll leave this method here in case somebody needs it in the future. -dwiggins
//
bool ConnectionInfoMap::RemoveConnectionInfo(const CString &uniquename)
{
	m_Mutex.Lock();
	ConnectionInfo *pConnectionInfo = NULL;
	if (m_PersonaMap.Lookup(uniquename, (void*&)pConnectionInfo))
	{
		 pConnectionInfo->DeleteCertFiles();
		 delete pConnectionInfo;
	}	
	m_Mutex.Unlock();
	return true;
}

//
//	ConnectionInfoMap::GetConnectionInfo()
//
//	Find the ConnectionInfo object specified by uniquename in the map.
//
//	Parameters:
//		uniquename [in] - Unique name to specify a given ConnectionInfo object.
//
ConnectionInfo *ConnectionInfoMap::GetConnectionInfo(const CString &uniquename)
{
	m_Mutex.Lock();
	ConnectionInfo *pTemp = NULL;
	m_PersonaMap.Lookup(uniquename, (void*&)pTemp);
	m_Mutex.Unlock();
	return pTemp;
}
