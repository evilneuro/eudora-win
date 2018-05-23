#ifndef __CEudoraModule_h__
#define __CEudoraModule_h__

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>

interface IEuApplication;

// -----------------------------------------------------------------------------
// Eudora OLE Module

class CEudoraModule : public CComModule
{
public:
    CEudoraModule();
    ~CEudoraModule();
	LONG Lock();
	LONG Unlock();
	void InitObjectMap(HINSTANCE hInst);

	DWORD dwThreadID;
};

extern CEudoraModule _Module;

#include <atlcom.h>

#endif // __CEudoraModule_h__


