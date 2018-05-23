// QCHesiodLib.h
// 
// Manages dynamic loading of RAS Library DLL and establishment and
// shutdown of a RAS connection.
//

#ifndef __QCHESIODLIB_H__
#define __QCHESIODLIB_H__

#include "afxmt.h"  //for CCriticalSection
//#include <ras.h>


//
// Typedefs for Hesiod API functions.
//
//FORNOWchar ** (CALLBACK* fnhes_resolve)(char* HesiodName, char* HesiodNameType);
//FORNOWint (CALLBACK* fnhes_error)();
typedef char** (CALLBACK HES_RESOLVE)(char* pszHesiodName, char* pszHesiodNameType);
typedef int (CALLBACK HES_ERROR)();

class AFX_EXT_CLASS QCHesiodLibMT
{
public:
	static HRESULT LoadHesiodLibrary();
	static HRESULT FreeHesiodLibrary();

	static HRESULT GetHesiodServer(const CString& strServiceName, const CString& strPOPUserName, char* pszBuffer, int nBufferSize, int* pnErrorCode);

private:
	QCHesiodLibMT();							// not implemented
	~QCHesiodLibMT();							// not implemented
	QCHesiodLibMT(const QCHesiodLibMT&);		// not implemented
	void operator=(const QCHesiodLibMT&);		// not implemented

	//
	// Function pointer declarations.
	//
	static HES_RESOLVE* m_pfnhes_resolve;
	static HES_ERROR* m_pfnhes_error;

	//
	// Reference count for outstanding LoadHesiodLibrary() calls.
	//
	static int m_nRefCount;
	static CCriticalSection m_Guard;

	//
	// HINSTANCE for dynamically-loaded DLL.
	//
	static HINSTANCE m_hHesiodLibrary;
};

#endif // __QCHESIODLIB_H__
