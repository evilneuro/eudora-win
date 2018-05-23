// LDAP Directory Services 
//
// File:     main.cpp
// Module:   LDAP.DLL
// Author:   Mark K. Joseph, Ph.D.
// Version:  1.0 
// Copyright 05/12/1997 Qualcomm Inc.  All Rights Reserved.
/* Copyright (c) 2016, Computer History Museum 
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
DAMAGE. */

//
// Description:
// Handle all DLL wide initialization and clean up.
//
#pragma warning(disable : 4514)
#include <afx.h>
#include <afxwin.h>
#include <ole2.h>
#include <commctrl.h>
#include "DebugNewHelpers.h"
#include "standard.h"
#include "factory.h"
#include "registry.h"


HINSTANCE g_hInstance        = NULL;
DWORD	  g_dwComponentCount = 0;


// Note: DllMain is now handled by MFC because we define _USRDLL.
// As an MFC DLL we need a CWinApp subclass. Conveniently enough,
// we can override InitInstance and ExitInstance to handle anything
// that we used to do in DllMain.

class CLDAPDLL : public CWinApp
{
  public:
								CLDAPDLL();

	virtual BOOL				InitInstance();
	virtual int					ExitInstance( );

  protected:
	HRESULT						m_hrOle;

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CLDAPDLL, CWinApp)
END_MESSAGE_MAP()

// The one and only CLDAPDLL object
CLDAPDLL		theLDAPDLL;


CLDAPDLL::CLDAPDLL()
{

}


BOOL
CLDAPDLL::InitInstance()
{
	m_hrOle = OleInitialize(NULL);

	CFactory::s_hModule = g_hInstance = AfxGetInstanceHandle();

	BOOL	bResult = FALSE;

	if ( SUCCEEDED(m_hrOle) )
		bResult = CWinApp::InitInstance();

	return bResult;
}

int
CLDAPDLL::ExitInstance()
{
	if ( SUCCEEDED(m_hrOle) )
		OleUninitialize();

	return CWinApp::ExitInstance();
}

// COM Entry point called by CoGetClassObject() to get our IClassFactory
//
STDAPI
DllGetClassObject( REFCLSID rclsid,   // in:
                          REFIID   refid,    // in:
                          LPVOID*  lpObj     // out:
                        )
{      return( CFactory::GetClassObject( rclsid, refid, lpObj )); }

// Are their any objects still around, is a class factory locked ?
// If so, then this DLL cannot be unloaded from memory.
//
STDAPI
DllCanUnloadNow( void )
{
	if ( g_dwComponentCount == 0 && !CFactory::IsLocked())
		 return( ResultFromScode( S_OK    ));	
	else return( ResultFromScode( S_FALSE ));
}

extern "C" __declspec(dllexport)
HRESULT DllRegisterServer()
{      return( CFactory::RegisterAll()); }

extern "C" __declspec(dllexport)
HRESULT DllUnregisterServer()
{  
       if (g_dwComponentCount != 0) {
//           MessageBox( NULL, "Warning: several COM objects left in memory", "LDAP.DLL", MB_ICONWARNING | MB_OK );
       }    
       return( CFactory::UnregisterAll()); 
}

extern "C" __declspec(dllexport)
HRESULT DllInstallServer( HKEY registryKey, LPSTR subKey )
{      return( CFactory::InstallDirCLSIDs( registryKey, subKey )); }

extern "C" __declspec(dllexport)
HRESULT DllUninstallServer(HKEY registryKey, LPSTR subKey )
{      return( CFactory::UninstallServer(  registryKey, subKey ));  }
