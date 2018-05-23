// utils.cpp

#pragma warning(disable : 4514)
#include <afx.h>
#include <string.h>
#include "DebugNewHelpers.h"
#include <QCUtils.h>


// Free, then strdup a new string variable
// usage: FreeDup(m_pszMemberVar, pszNewString);
void FreeDup(LPSTR & rpszOriginal, LPSTR pszNew)
{
	delete [] rpszOriginal;
	rpszOriginal = NULL;

	if ( pszNew && strlen(pszNew) )
		rpszOriginal = SafeStrdupMT(pszNew);
}



// Load a string resource and store in allocated memory
LPSTR LoadDup(HINSTANCE hInstance, UINT uID)
{
	LPSTR psz = NULL;
	char buffer[256];
	if ( 0 != LoadString(hInstance, uID, buffer, sizeof(buffer)-1) )
		psz = SafeStrdupMT(buffer);

	return psz;
}

