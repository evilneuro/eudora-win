// Copyright 2004 Google Inc
// All rights reserved
// 
#include "stdafx.h"
#include <atlsafe.h>

#include "GoogleDesktopSearchAPI.h"
#include "GoogleDesktopSearchAPI_i.c"
#include "GoogleDesktopComponentRegistration.h"

HRESULT RegisterComponentHelper(REFCLSID clsid,
                                const wchar_t *title, 
                                const wchar_t *description,
                                const wchar_t *icon,
                                int num_suffixes,
                                const wchar_t *suffixes[]) { 
  CComPtr<IGoogleDesktopSearchComponentRegister> spRegister;
  HRESULT hr;
  
  hr = spRegister.CoCreateInstance(CLSID_GoogleDesktopSearchRegister);
  
  if (SUCCEEDED(hr)) {
    ATLASSERT(spRegister != NULL);
    
    // Component description is 6 strings
    CComSafeArray<VARIANT> arr_descr(6);

    arr_descr.SetAt(0, CComVariant(L"Title"));
    arr_descr.SetAt(1, CComVariant(title));
    arr_descr.SetAt(2, CComVariant(L"Description"));
    arr_descr.SetAt(3, CComVariant(description));
    arr_descr.SetAt(4, CComVariant(L"Icon"));
    arr_descr.SetAt(5, CComVariant(icon));

    // our CLSID in string format
    CComBSTR clsid(clsid);
    
    // Wrap description array in variant
    CComVariant descr(arr_descr.m_psa);
    CComPtr<IGoogleDesktopSearchComponentRegistration> spRegistration;

    // and register
    hr = spRegister->RegisterComponent(clsid, descr, &spRegistration);
    if (FAILED(hr))
      return hr;

    ATLASSERT(FAILED(hr) || spRegistration);

    // success, now register extensions
    for (int i = 0; SUCCEEDED(hr) && i < num_suffixes; i++) 
      hr = spRegistration->RegisterExtension(CComBSTR(suffixes[i]));
      
    // revoke our registration in case any of our extensions fail to register
    if (FAILED(hr)) {
      // ignore the error - we've already failed
      HRESULT ignore;
      
      ignore = spRegister->UnregisterComponent(CComBSTR(clsid));
    }
  }

  return hr;
}

HRESULT UnregisterComponentHelper(REFCLSID clsid) {
   CComPtr<IGoogleDesktopSearchComponentRegister> spRegister;
  HRESULT hr;
  
  hr = spRegister.CoCreateInstance(CLSID_GoogleDesktopSearchRegister);
  
  if (SUCCEEDED(hr)) { 
    // our CLSID in string format
    CComBSTR bstrClsid(clsid);
  
    hr = spRegister->UnregisterComponent(bstrClsid);
  }
  
  return hr;
}
