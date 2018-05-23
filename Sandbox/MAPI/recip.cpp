////////////////////////////////////////////////////////////////////////
//
// CMapiRecipDesc
//
// Intelligent C++ object wrapper for dumb C-based MapiRecipDesc structure.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <afxwin.h>		// FORNOW, should probably use precompiled header

#include "recip.h"

const long ENTRY_ID = 42L;



CString MakeFullAddress(MapiRecipDesc* RecipDesc)
{
	CString FullAddress;

	if (!RecipDesc)
		ASSERT(0);
	else
	{
		FullAddress = RecipDesc->lpszName;
		LPCTSTR addr = RecipDesc->lpszAddress;

		if (addr && *addr)
		{
			// Have to quote specials
			if (FullAddress.FindOneOf("()<>[]:;@\\,.") >= 0)
				FullAddress = '"' + FullAddress + '"';

			const BOOL bHasName = (FullAddress.IsEmpty() == FALSE);
			if (bHasName)
				FullAddress += " <";

			// MAPI allows addressing on the form [SMTP:a@foo.com] and SMTP:a@foo.com
			if ('[' == *addr)
				addr++;
			if (strnicmp(addr, "SMTP:", 5) == 0)
				addr += 5;

			FullAddress += addr;

			int LastCharLen = FullAddress.GetLength() - 1;
			if (LastCharLen >= 0 && FullAddress[LastCharLen] == ']')
				FullAddress.ReleaseBuffer(LastCharLen);

			if (bHasName)
				FullAddress += '>';
		}
	}

	return FullAddress;
}


////////////////////////////////////////////////////////////////////////
// CMapiRecipDesc [private, constructor]
//
////////////////////////////////////////////////////////////////////////
CMapiRecipDesc::CMapiRecipDesc(void)
{
	ulReserved = 0L;
	ulRecipClass = ULONG(-1L);
	lpszName = NULL;
	lpszAddress = NULL;
	ulEIDSize = 0L;
	lpEntryID = NULL;
}


////////////////////////////////////////////////////////////////////////
// CMapiRecipDesc [public, destructor]
//
////////////////////////////////////////////////////////////////////////
CMapiRecipDesc::~CMapiRecipDesc(void)
{
	delete [] lpszName;
	delete [] lpszAddress;
	if (lpEntryID)
		lpEntryID = NULL;		// don't delete!
}


////////////////////////////////////////////////////////////////////////
// SetClass [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiRecipDesc::SetClass(ULONG recipClass)
{
	switch (recipClass)
	{
	case MAPI_ORIG:
	case MAPI_TO:
	case MAPI_CC:
	case MAPI_BCC:
		ulRecipClass = recipClass;
		return TRUE;
	default:
		ASSERT(0);
		return FALSE;
	}
}


////////////////////////////////////////////////////////////////////////
// SetName [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiRecipDesc::SetName(const char* pName)
{
	ASSERT(pName != NULL);
	ASSERT(NULL == lpszName);
	lpszName = new char[strlen(pName) + 1];
	if (NULL == lpszName)
		return FALSE;

	strcpy(lpszName, pName);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SetAddress [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiRecipDesc::SetAddress(const char* pAddress)
{
	ASSERT(pAddress != NULL);
	ASSERT(NULL == lpszAddress);
	lpszAddress = new char[strlen(pAddress) + 1];
	if (NULL == lpszAddress)
		return FALSE;

	strcpy(lpszAddress, pAddress);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SetNameAndAddress [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiRecipDesc::SetNameAndAddress(const char* pName, const char* pAddress)
{
	if (! SetName(pName))
		return FALSE;

	if (! SetAddress(pAddress))
	{
		delete [] lpszName;
		return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// ResolveName [public]
//
// The MAPIDetails() function expects that MAPIRecipDesc items returned
// by MAPIAddress() and MAPIResolveName() contain a non-NULL lpEntryId
// pointer and a non-zero ulEIDSize value designating the size of the
// entry ID data.
//
// This is just a stub implementation that resolves all entry IDs to 
// point to an innocuous 4-byte value.
////////////////////////////////////////////////////////////////////////
BOOL CMapiRecipDesc::ResolveName()
{
	lpEntryID = (void *) &ENTRY_ID;
	ulEIDSize = sizeof(ENTRY_ID);

	return TRUE;
}
