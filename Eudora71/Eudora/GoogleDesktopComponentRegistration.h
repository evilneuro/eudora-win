// Copyright 2004 Google Inc
// All rights reserved
// 
#ifndef __GOOGLE_DESKTOP_COMPONENT_REGISTRATION_H__
#define __GOOGLE_DESKTOP_COMPONENT_REGISTRATION_H__

// Attempts to register the provided CLSID with GDS
// if num_suffixes is zero does not attempt to register for crawl 
// events
HRESULT RegisterComponentHelper(REFCLSID clsid,
                                const wchar_t *title, 
                                const wchar_t *description,
                                const wchar_t *icon,
                                int num_suffixes,
                                const wchar_t *suffixes[]);

HRESULT UnregisterComponentHelper(REFCLSID clsid);


#endif  // __GOOGLE_DESKTOP_COMPONENT_REGISTRATION_H__
