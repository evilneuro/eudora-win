#ifndef _SHELL_HOOK_IMPLEMENTATION_
#define _SHELL_HOOK_IMPLEMENTATION_

/*********************************************************************************************
// This file includes all the core classes required for the implementation of this Shell 
// Extension
**********************************************************************************************/

#include <windows.h>

// Required Shell includes
#include <shellapi.h>
#include <shlobj.h>

#include "RegDefines.h"

// Name of this Shell Extension. This is the name for the below defined Class ID
#define	SHELLEXNAME				"Eudora's Shell Extension"

// EuShlExt ClassID - this is how the system refers to our shell extension.
// {EDB0E980-90BD-11d4-8599-0008C7D3B6F8}
DEFINE_GUID(CLSID_EudoraShellExtension, 
0xedb0e980, 0x90bd, 0x11d4, 0x85, 0x99, 0x0, 0x8, 0xc7, 0xd3, 0xb6, 0xf8);

// Data structure for the storing Thread Local Storage associated value.
typedef struct THREADINFO_TAG
{
	HWND hWndClient;	// we don't need this one in here, but it's better to consider it now.
	UINT uMessageID;
} THREADINFO;

// Data structure for keeping in memory data
typedef struct LM_REG_DATA_TAG
{
	DWORD	dwAlreadyWarned;
	TCHAR	szPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT + sizeof(TCHAR)];	
	TCHAR	szWarnExtensions[1024];
	TCHAR	szDoNotWarnExtensions[1024];
} LM_REG_DATA;

// Data structure used while registering/unregistering
typedef struct REGSTRUCT_TAG
{
   HKEY  hRootKey;
   LPCTSTR lpszSubKey;
   LPCTSTR lpszValueName;
   LPTSTR lpszData;
}REGSTRUCT, *LPREGSTRUCT;

#define SIZEOF_NULL_TERMINATOR	(sizeof(TCHAR)/sizeof(char))

class CShellExtClassFactory : public IClassFactory
{
protected:
	ULONG        m_cRef;


public:
	CShellExtClassFactory();
	~CShellExtClassFactory();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
	STDMETHODIMP		LockServer(BOOL);

};
typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;

// This is the actual OLE Shell Extension handler
class CShellExt : public IShellExecuteHook
{
public:
  

protected:
	ULONG			m_cRef;
	LPDATAOBJECT	m_pDataObj;
	
	int				m_nPathCount;
	LM_REG_DATA		*m_pLMRegData;
   
protected:

public:
	// Generic shell ext functions
	CShellExt();
	~CShellExt();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	// Overide for Execute method
   	STDMETHODIMP		Execute( LPSHELLEXECUTEINFO pei);
	
	// Loads the data from the registry in the internal data structure.
	STDMETHODIMP_(VOID)	LoadConfiguration();

	// Performs the necessary checks to see if a specified FileName has Executable Contents, that could be considered malicious.
	STDMETHODIMP_(BOOL) IsExecutable(LPCTSTR Filename, int nPathOffset);
   
};
typedef CShellExt *LPCSHELLEXT;

void DumpLogInformation(const TCHAR *szFormat ... );

#endif // _SHELL_HOOK_IMPLEMENTATION_
