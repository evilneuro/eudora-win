// QCHesiodLib.cpp
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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

#include "afxmt.h"    //for CCriticalSection

#include <direct.h>				// for _getdrive()

#include <QCUtils.h>
#include <hes_api.h>
#include "QCHesiodLib.h"

#include "DebugNewHelpers.h"



//
// Declaration and initialization of statics.
//
HES_RESOLVE* QCHesiodLibMT::m_pfnhes_resolve = NULL;
HES_ERROR* QCHesiodLibMT::m_pfnhes_error = NULL;
int QCHesiodLibMT::m_nRefCount = 0;
HINSTANCE QCHesiodLibMT::m_hHesiodLibrary = NULL;
CCriticalSection QCHesiodLibMT::m_Guard;

////////////////////////////////////////////////////////////////////////
// LoadWSLibrary [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCHesiodLibMT::LoadHesiodLibrary()
{
	//ASSERT(::IsMainThreadMT());			// FORNOW, needs critical section
	CSingleLock lock(&m_Guard, TRUE);

	//
	// If reference count is zero, then we need to really load the library.
	// Otherwise, just bump the reference count and return success.
	//
	if (0 == QCHesiodLibMT::m_nRefCount)
	{
		//
		// Change the current drive so that if it is set to some
		// removable media drive (like a floppy), then the
		// LoadLibrary() call won't cause that drive to be searched
		// (which causes a "Cannot read from Drive X")
		//
		{
			char szPathname[MAX_PATH];
			if (! ::GetModuleFileName(NULL, szPathname, sizeof(szPathname)))
			{
				ASSERT(0);
				return E_FAIL;
			}

			int nExeDriveNum = ::toupper(szPathname[0]) - 'A' + 1;
			if (::_getdrive() != nExeDriveNum)
				::_chdrive(nExeDriveNum);
		}

		//
		// Load the library.
		//
		ASSERT(NULL == QCHesiodLibMT::m_hHesiodLibrary);
		if ((QCHesiodLibMT::m_hHesiodLibrary = ::AfxLoadLibrary("wshelp32.dll")) == NULL)
			return E_FAIL;

		ASSERT(NULL == QCHesiodLibMT::m_pfnhes_resolve);
		ASSERT(NULL == QCHesiodLibMT::m_pfnhes_error);

		QCHesiodLibMT::m_pfnhes_resolve = (HES_RESOLVE *) ::GetProcAddress(QCHesiodLibMT::m_hHesiodLibrary, "hes_resolve");
		QCHesiodLibMT::m_pfnhes_error = (HES_ERROR *) ::GetProcAddress(QCHesiodLibMT::m_hHesiodLibrary, "hes_error");
	}

	//
	// FORNOW, not verifying that we loaded all the entry
	// points successfully.
	//
	ASSERT(QCHesiodLibMT::m_pfnhes_resolve != NULL);
	ASSERT(QCHesiodLibMT::m_pfnhes_error != NULL);

	QCHesiodLibMT::m_nRefCount++;
	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// FreeHesiodLibrary [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCHesiodLibMT::FreeHesiodLibrary()
{
	//ASSERT(::IsMainThreadMT());					// FORNOW, needs critical section
	CSingleLock lock(&m_Guard, TRUE);

	if (QCHesiodLibMT::m_nRefCount <= 0)
	{
		ASSERT(0);
		return E_FAIL;
	}
	else if (1 == QCHesiodLibMT::m_nRefCount)
	{
		//
		// Really do the unload, first making sure that we're all
		// cleaned up.
		//
		ASSERT(QCHesiodLibMT::m_hHesiodLibrary);
		::AfxFreeLibrary(QCHesiodLibMT::m_hHesiodLibrary);

		QCHesiodLibMT::m_hHesiodLibrary = NULL;

		QCHesiodLibMT::m_pfnhes_resolve = NULL;
		QCHesiodLibMT::m_pfnhes_error = NULL;
	}

	QCHesiodLibMT::m_nRefCount--;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
// GetHesiodServer [public, static]
//
////////////////////////////////////////////////////////////////////////
HRESULT QCHesiodLibMT::GetHesiodServer
(
	const CString& strServiceName, 		//(i) requested service
	const CString& strPOPUserName,		//(i) POP account name, i.e., the 'c_wsakai' part of 'c_wsakai@adept.qualcomm.com'
	char* pszBuffer, 					//(o) returned server name
	int nBufferSize,					//(i) size of 'pszBuffer'
	int* pnErrorCode					//(o) error code (only valid if return value indicates failure)
)
{
	//ASSERT(::IsMainThreadMT());			// FORNOW, needs critical section
	CSingleLock lock(&m_Guard, TRUE);

	char** ppszHesiodList = NULL;
	
	if (NULL == pszBuffer || nBufferSize <= 0 || NULL == pnErrorCode)
	{
		ASSERT(0);
		return E_INVALIDARG;
	}

	if (NULL == QCHesiodLibMT::m_pfnhes_resolve ||
		NULL == QCHesiodLibMT::m_pfnhes_error)
	{
		ASSERT(0);			// oops, forgot to call LoadHesiodLibrary() first
		return E_POINTER;
	}

	if (stricmp(strServiceName, "pop3") == 0)
	{
		if ((ppszHesiodList = QCHesiodLibMT::m_pfnhes_resolve((char *) (const char *) strPOPUserName, "pobox")) == NULL)
		{
			if ((*pnErrorCode = QCHesiodLibMT::m_pfnhes_error()) != HES_ER_NOTFOUND)
				return E_FAIL;
			else if ((ppszHesiodList = QCHesiodLibMT::m_pfnhes_resolve("pop", "sloc")) == NULL)
			{
				*pnErrorCode = QCHesiodLibMT::m_pfnhes_error();
				return E_FAIL;
			}
			else
				::strncpy(pszBuffer, *ppszHesiodList, nBufferSize);
		}
		else
		{
			// Parse the return string.  Only return the system name for now.
			char* pszStr = *ppszHesiodList;
			while (*pszStr && *pszStr != ' ' && *pszStr != '\t') 
				pszStr++;
			while (*pszStr && (*pszStr == ' ' || *pszStr == '\t')) 
				pszStr++;

			::strncpy(pszBuffer, pszStr, nBufferSize);
			pszStr = pszBuffer;
			while (*pszStr && *pszStr != ' ' && *pszStr != '\t') 
				pszStr++;
			*pszStr = '\0';
		}
	}
	else if (::stricmp(strServiceName, "smtp") == 0)
	{
		if ((ppszHesiodList = QCHesiodLibMT::m_pfnhes_resolve("smtp", "sloc")) == NULL)
		{
			*pnErrorCode = QCHesiodLibMT::m_pfnhes_error();
			return E_FAIL;
		}
	}
	else if (stricmp(strServiceName, "imap") == 0)//this is same as POP for now but a seperate case for future use.
	{
		if ((ppszHesiodList = QCHesiodLibMT::m_pfnhes_resolve((char *) (const char *) strPOPUserName, "pobox")) == NULL)
		{
			if ((*pnErrorCode = QCHesiodLibMT::m_pfnhes_error()) != HES_ER_NOTFOUND)
				return E_FAIL;
			else if ((ppszHesiodList = QCHesiodLibMT::m_pfnhes_resolve("pop", "sloc")) == NULL)//"pop" should be passed , not "imap"
			{															 
				*pnErrorCode = QCHesiodLibMT::m_pfnhes_error();
				return E_FAIL;
			}
			else
				::strncpy(pszBuffer, *ppszHesiodList, nBufferSize);
		}
		else
		{
			// Parse the return string.  Only return the system name for now.
			char* pszStr = *ppszHesiodList;
			while (*pszStr && *pszStr != ' ' && *pszStr != '\t') 
				pszStr++;
			while (*pszStr && (*pszStr == ' ' || *pszStr == '\t')) 
				pszStr++;

			::strncpy(pszBuffer, pszStr, nBufferSize);
			pszStr = pszBuffer;
			while (*pszStr && *pszStr != ' ' && *pszStr != '\t') 
				pszStr++;
			*pszStr = '\0';
		}
	}

	if (*ppszHesiodList == NULL || **ppszHesiodList == '\0')
	{
		*pnErrorCode = HES_ER_NOTFOUND;
		return E_FAIL;
	}

	return S_OK;
}

