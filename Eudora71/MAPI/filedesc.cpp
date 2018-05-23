////////////////////////////////////////////////////////////////////////
//
// CMapiFileDesc
//
// Intelligent C++ object wrapper for dumb C-based MapiFileDesc structure.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <afxwin.h>			// FORNOW, should probably be precompiled header

#include "filedesc.h"


////////////////////////////////////////////////////////////////////////
// CMapiFileDesc [private, constructor]
//
////////////////////////////////////////////////////////////////////////
CMapiFileDesc::CMapiFileDesc(void)
{
	ulReserved = 0;      
	flFlags = 0;      
	nPosition = ULONG(-1);           
	lpszPathName = NULL;      
	lpszFileName = NULL;      
	lpFileType = NULL;      
}


////////////////////////////////////////////////////////////////////////
// ~CMapiFileDesc [public, destructor]
//
////////////////////////////////////////////////////////////////////////
CMapiFileDesc::~CMapiFileDesc(void)
{
	delete lpszPathName;      
	delete lpszFileName;      
	delete lpFileType;      
}


////////////////////////////////////////////////////////////////////////
// SetFileName [public]
//
// Allocates and initializes a new string for the 'lpszFileName' field.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiFileDesc::SetFileName(const CString& fileName)
{
	ASSERT(NULL == lpszFileName);
	ASSERT(fileName.GetLength() > 0);

	lpszFileName = new char [fileName.GetLength() + 1];
	if (NULL == lpszFileName)
		return FALSE;

	strcpy(lpszFileName, fileName);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// SetPathName [public]
//
// Allocates and initializes a new string for the 'lpszPathName' field.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiFileDesc::SetPathName(const CString& pathName)
{
	ASSERT(NULL == lpszPathName);
	ASSERT(pathName.GetLength() > 0);

	lpszPathName = new char [pathName.GetLength() + 1];
	if (NULL == lpszPathName)
		return FALSE;

	strcpy(lpszPathName, pathName);
	return TRUE;
}
