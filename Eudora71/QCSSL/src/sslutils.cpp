// sslutils.cpp
//
// Implementation file for miscellaneous SSL utility routines.
//

#include "stdafx.h"

#include <stddef.h>		/* Contains definition of size_t */
#include <stdlib.h>
#include <string.h>

#include<afx.h>

#include "DebugNewHelpers.h"

bool GetResourceString(UINT StringID, char * buffer, int maxbuf)
{
	// Set this so the resource gets loaded from the DLL's resource.
	HINSTANCE hInstance = ::GetModuleHandle("QCSll.dll");
	int nLen = ::LoadString(hInstance, StringID, buffer, maxbuf);
	if(nLen >0)
		return true;
	else
		return false;
	
}
