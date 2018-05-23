/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					factory.cpp
Description:		Generic class factory implementation
Date:						7/19/97
Version:  			1.0 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma warning(disable : 4514)
#include <afx.h>
#include <objbase.h>
#include "DebugNewHelpers.h"
#include "registry.h"
#include "factory.h"


///////////////////////////////////////////////////////////
// Static variables

LONG 		CFactory::s_cServerLocks = 0;    	// Count of locks on this server
HMODULE CFactory::s_hModule = NULL;				// DLL module handle


///////////////////////////////////////////////////////////
// CFactory methods

CFactory::CFactory(const CFactoryData* pFactoryData) : m_cRef(1)
{
	m_pFactoryData = pFactoryData;
}

HRESULT	__stdcall CFactory::QueryInterface(REFIID iid, void** ppv)
{
	if (iid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else
	if (iid == IID_IClassFactory)
		*ppv = static_cast<IClassFactory*>(this);
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();

	return S_OK;
}

ULONG __stdcall CFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG	__stdcall CFactory::Release()
{
	if (0 == InterlockedDecrement(&m_cRef)) {
		delete this;
		return 0;
	}
	return m_cRef;
}

// IClassFactory
HRESULT __stdcall CFactory::CreateInstance(IUnknown * pUnknownOuter, REFIID iid, void** ppv)
{
	HRESULT hr;
	
	if (pUnknownOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	IUnknown * po = m_pFactoryData->CreateInstance();
	if (!po)
		return E_OUTOFMEMORY;

	hr = po->QueryInterface(iid, ppv);
	po->Release();

	return hr;
}


HRESULT __stdcall CFactory::LockServer(BOOL bLock)
{
	if (bLock)
		InterlockedIncrement(&s_cServerLocks);
	else
		InterlockedDecrement(&s_cServerLocks);

	return S_OK;
}


///////////////////////////////////////////////////////////
// GetClassObject - Create a class factory based on a CLSID.
//
HRESULT CFactory::GetClassObject(const CLSID& clsid, const IID& iid, void** ppv)
{
	if ((iid != IID_IUnknown) && (iid != IID_IClassFactory))
		return E_NOINTERFACE;
	
	// Traverse the array of data looking for this class ID.
	for (int i = 0; i < g_cFactoryDataEntries; i++) {
		const CFactoryData* pData = &g_FactoryDataArray[i];
		if (pData->IsClassID(clsid)) {
			// Found the ClassID in the array of components we can
			// create.  So create a class factory for this component.
			// Pass the CFactoryData structure to the class factory
			// so that it knows what kind of components to create.
			*ppv = (IUnknown*) DEBUG_NEW_NOTHROW CFactory(pData);
			if (*ppv == NULL) {
				return E_OUTOFMEMORY;
			}
			    
			return NOERROR;
		}
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}


///////////////////////////////////////////////////////////
// Registration
//
HRESULT CFactory::RegisterAll()
{
	for (int i=0; i < g_cFactoryDataEntries; i++)
		RegisterServer(s_hModule,
										*(g_FactoryDataArray[i].m_pCLSID),
										g_FactoryDataArray[i].m_RegistryName,
										g_FactoryDataArray[i].m_szVerIndProgID,
										g_FactoryDataArray[i].m_szProgID,
										g_FactoryDataArray[i].m_szExtraInfo);
	return S_OK;
}

HRESULT CFactory::UnregisterAll()
{
	for (int i=0; i < g_cFactoryDataEntries; i++)
		UnregisterServer(*(g_FactoryDataArray[i].m_pCLSID),
											g_FactoryDataArray[i].m_szVerIndProgID,
											g_FactoryDataArray[i].m_szProgID,
											g_FactoryDataArray[i].m_szExtraInfo);

	return S_OK;
}


HRESULT CFactory::InstallServer(HKEY hKey, LPSTR lpSubkey)
{
	for (int i=0; i < g_cFactoryDataEntries; i++)
		SetKeyAsCLSID(hKey, lpSubkey, *(g_FactoryDataArray[i].m_pCLSID), 
									g_FactoryDataArray[i].m_RegistryName);
	return S_OK;
}



HRESULT CFactory::UninstallServer(HKEY hKey, LPSTR lpSubkey)
{
	for (int i=0; i < g_cFactoryDataEntries; i++)
		DeleteKeyAsCLSID(hKey, lpSubkey, *(g_FactoryDataArray[i].m_pCLSID));
	
	return S_OK;
}




