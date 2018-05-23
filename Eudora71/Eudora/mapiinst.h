// mapiinst.h
//
// Copyright (c) 1994-2000 by QUALCOMM, Incorporated
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

#ifndef H_MAPIINST_H
#define H_MAPIINST_H

////////////////////////////////////////////////////////////////////////
//
// CMapiInstaller
//
// This is a static class that provides two simple public methods:
// Install() and Uninstall().  The Install() function copies the
// Eudora MAPI DLLs to the Windows SYSTEM directory, replacing the
// existing Microsoft MAPI DLLs.  The Uninstall() function reverses
// this process, restoring the original Microsoft DLLs.
//
// To provide for reuse in multiple applications, this class purposely
// does not provide any UI or require any RC-type resources (e.g., for
// error message dialogs).  Users of this class should provide any
// desired UI components.
//
////////////////////////////////////////////////////////////////////////

class CMapiInstaller
{
// Implementation
public:
	enum Status
	{
		STATUS_SUCCESSFUL,
		STATUS_ALREADY_INSTALLED,
		STATUS_NOT_INSTALLED,
		STATUS_SOURCE_DLL_NOT_FOUND,
		STATUS_DLL_IN_USE,
		STATUS_FAILED,
		STATUS_INTERNAL_ERROR
	};

	//
	// Public interface.  Simple and Sweet.
	//
	static Status Install(void);
	static Status Uninstall(void);

	//
	// Useful query function.
	//
	static BOOL IsDLLInUse(const char* dllName);

private:
    CMapiInstaller(void);					// default constructor (never called)
    CMapiInstaller(const CMapiInstaller&);	// copy constructor (never called)
    ~CMapiInstaller(void);					// standard destructor (never called)

	static BOOL Initialize(void);

	//
	// Install/Uninstall functions.
	//
	static BOOL DoBackupMapiDLL(const CString& dllName);
	static BOOL DoRestoreMapiDLL(const CString& dllName);
	static BOOL DoInstallEudoraDLL(const char* sourceDLLName, const char* targetDLLName);
	static BOOL DoUninstallEudoraDLL(const char* dllName);

	//
	// Generic, low-level helper functions.
	//
	static BOOL FileExists(const char* fileName);
	static BOOL IsEudoraDLL(const char* fileName);
	static Status CompareEudoraDLLs(const char* dllName1, const char* dllName2);

	//
	// Commonly used cached values that are initialized by Initialize().
	//
	static BOOL m_IsInitialized;  	// initialization flag to insure one-time initialization
	static CString m_SystemDir;		// pathname to Windows system directory
	static CString m_AppDir;		// pathname to application directory
};


#endif