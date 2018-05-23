////////////////////////////////////////////////////////////////////////
//
// CRecipientList
//
// A customized derivation of CStringList that provides support for
// tokenizing a comma-separated list of address names into separate
// CString objects.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <afxwin.h>			// FORNOW, should probably use precompiled header

#include "reciplst.h"

////////////////////////////////////////////////////////////////////////
// CRecipientList [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CRecipientList::CRecipientList(void)
{
	// nothing to do
}


////////////////////////////////////////////////////////////////////////
// ~CRecipientList [public, destructor]
//
////////////////////////////////////////////////////////////////////////
CRecipientList::~CRecipientList(void)
{
	// nothing to do
}


////////////////////////////////////////////////////////////////////////
// Tokenize [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CRecipientList::Tokenize(const CString& recipString)
{
	if (recipString.IsEmpty())
		return TRUE;			// nothing to do

	CString str(recipString);	// make working copy
	int idx;
	while ((idx = str.Find(',')) != -1)
	{
		//
		// Remove leading and trailing whitespace, then add string to
		// the recipient collection.
		//
		CString newstr(str.Left(idx));
		newstr.TrimLeft();
		newstr.TrimRight();
		if (! newstr.IsEmpty())
			AddTail(newstr);

		//
		// Removed processed recipient by shifting string.
		//
		str = str.Right(str.GetLength() - (idx + 1));
	}

	str.TrimLeft();
	str.TrimRight();
	if (! str.IsEmpty())
		AddTail(str);

	return TRUE;
}
