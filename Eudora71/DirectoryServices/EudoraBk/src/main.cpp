/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					server.cpp
Description:		Generic directory services COM component object
Date:						7/22/97
Version:  			1.0
Module:					SERVER.DLL
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma warning(disable : 4201 4514 4706)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "registry.h"


long g_cComponents = 0;				// Count of active components


///////////////////////////////////////////////////////////
// Exported DLL functions

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void * /* lpReserved */)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		CFactory::s_hModule = (HINSTANCE)hModule;

	return TRUE;
}

STDAPI DllGetClassObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	return CFactory::GetClassObject(clsid, iid, ppv);
}


STDAPI DllCanUnloadNow()
{
	if (0 == g_cComponents && !CFactory::IsLocked())
		return S_OK;
	else
		return S_FALSE;
}

//STDAPI DllRegisterServer()
extern "C" __declspec(dllexport)
HRESULT DllRegisterServer()
{
	return CFactory::RegisterAll();
}


//STDAPI DllUnregisterServer()
extern "C" __declspec(dllexport)
HRESULT DllUnregisterServer()
{
	return CFactory::UnregisterAll();
}

//STDAPI DllInstallServer(HKEY hKey, LPSTR lpSubkey)
extern "C" __declspec(dllexport)
HRESULT DllInstallServer( HKEY hKey, LPSTR lpSubkey )
{
	return CFactory::InstallServer(hKey, lpSubkey);
}


//STDAPI DllUninstallServer(HKEY hKey, LPSTR lpSubkey)
extern "C" __declspec(dllexport)
HRESULT DllUninstallServer(HKEY hKey, LPSTR lpSubkey )
{
	return CFactory::UninstallServer(hKey, lpSubkey);
}

