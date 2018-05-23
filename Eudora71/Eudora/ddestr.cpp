////////////////////////////////////////////////////////////////////////
//
// CDDEString
// Class wrapper for DDE string objects.  As long as the object is
// alive, it represents a valid DDE string object.  When the object
// is destroyed, it cleans up the internal DDE string object.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ddestr.h"



#include "DebugNewHelpers.h"


////////////////////////////////////////////////////////////////////////
// CDDEString	[public, constructor]
//
////////////////////////////////////////////////////////////////////////
CDDEString::CDDEString(DWORD instId, const char* pStr) :
	m_InstId(instId),
	m_HSZ(NULL),
	m_IsHandleOwner(TRUE)
{
	//
	// NULL incoming string means NULL handle.
	//
	ASSERT(instId);
	if (pStr != NULL)
	{
		m_HSZ = DdeCreateStringHandle(instId, pStr, CP_WINANSI);
		ASSERT(m_HSZ != NULL);
	}
}


////////////////////////////////////////////////////////////////////////
// CDDEString	[public, constructor]
//
////////////////////////////////////////////////////////////////////////
CDDEString::CDDEString(DWORD instId, HSZ hStr) :
	m_InstId(instId),
	m_HSZ(hStr),
	m_IsHandleOwner(FALSE)
{
	// nothing else to do!
	ASSERT(instId);
	ASSERT(hStr != NULL);
}


////////////////////////////////////////////////////////////////////////
// ~CDDEString	[public, destructor]
//
////////////////////////////////////////////////////////////////////////
CDDEString::~CDDEString(void)
{
	if (m_IsHandleOwner && m_HSZ)
	{
		VERIFY(DdeFreeStringHandle(m_InstId, m_HSZ));
	}
}


////////////////////////////////////////////////////////////////////////
// operator CString	[public]
//
// Casts a CDDEString to a temporary CString.
//
////////////////////////////////////////////////////////////////////////
CDDEString::operator CString(void) const
{
	CString str("");

	if (m_HSZ)
	{
		int len = int(GetLength());
		ASSERT(len > 0);
		DdeQueryString(m_InstId, m_HSZ, str.GetBuffer(len), len+1, CP_WINANSI);
		str.ReleaseBuffer();
		if (str.GetLength() != len)
		{
			ASSERT(0);
		}
	}
	return str;
}


////////////////////////////////////////////////////////////////////////
// GetLength [public]
//
// Returns the length of string, not counting the NULL terminator.
//
////////////////////////////////////////////////////////////////////////
int CDDEString::GetLength(void) const
{
	if (m_HSZ != NULL)
		return int(DdeQueryString(m_InstId, m_HSZ, NULL, 0, CP_WINANSI));
	else
		return 0;
}


////////////////////////////////////////////////////////////////////////
// IsEqual [public]
//
// Returns TRUE if the given DDE string has the same string value as
// this DDE string.  This is a case-insensitive test, so "FRED" and
// "fred" are considered to be equivalent.
//
////////////////////////////////////////////////////////////////////////
BOOL CDDEString::IsEqual(HSZ hStr) const
{
	return DdeCmpStringHandles(m_HSZ, hStr) == 0;
}
