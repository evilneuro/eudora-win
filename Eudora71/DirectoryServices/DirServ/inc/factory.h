/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					factory.h
Description:		Generic class factory
Date:						7/19/97
Version:  			1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef __FACTORY_H__
#define __FACTORY_H__

class CFactoryData;

// Global data used by CFactory
extern CFactoryData g_FactoryDataArray[];
extern int g_cFactoryDataEntries;


// Component creation function
typedef IUnknown * (*FPCREATEINSTANCE)();


///////////////////////////////////////////////////////////
// CFactoryData

class CFactoryData
{
public:
	const CLSID* 			m_pCLSID;							// The class ID for the component
	FPCREATEINSTANCE 		CreateInstance;				// Pointer to the function that creates it
	const char* 			m_RegistryName;				// Name of the component to register in the registry
	const char* 			m_szProgID;						// ProgID
	const char* 			m_szVerIndProgID;			// Version-independent ProgID
	const char*				m_szExtraInfo;				// Extra key for CLSID storage

	BOOL IsClassID(const CLSID& clsid) const	// Helper function for finding the class ID
		{ return (*m_pCLSID== clsid); }
};


///////////////////////////////////////////////////////////////////////////////
// Generic class factory class

class CFactory : public IClassFactory
{
public:
	// IUnknown
	virtual HRESULT	__stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG		__stdcall AddRef();
	virtual ULONG		__stdcall Release();
	// IClassFactory
	virtual HRESULT __stdcall CreateInstance(IUnknown * pUnknownOuter, REFIID iid, void** ppv);
	virtual HRESULT __stdcall LockServer(BOOL bLock);
	// Constructor
  CFactory(const CFactoryData* pFactoryData);
	// Destructor
	~CFactory() {}

	static BOOL 							IsLocked() { return (s_cServerLocks > 0); }
  static HRESULT 						GetClassObject(const CLSID& clsid, const IID& iid, void** ppv);
	static HRESULT 						RegisterAll();
	static HRESULT 						UnregisterAll();
	static HRESULT 						InstallServer(HKEY hKey, LPSTR lpSubkey);

public:
	long									m_cRef;
	static LONG						s_cServerLocks;
	static HMODULE				s_hModule;
	const CFactoryData *	m_pFactoryData;
};



#endif //__FACTORY_H__




