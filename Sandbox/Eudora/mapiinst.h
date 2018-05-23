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
	static BOOL IsDLLInUse(const CString& dllName);

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
	static BOOL DoInstallEudoraDLL(const CString& sourceDLLName, const CString& targetDLLName);
	static BOOL DoUninstallEudoraDLL(const CString& dllName);

	//
	// Generic, low-level helper functions.
	//
	static BOOL FileExists(const CString& fileName);
	static BOOL IsEudoraDLL(const CString& fileName);
	static Status CompareEudoraDLLs(const CString& dllName1, const CString& dllName2);

	//
	// Commonly used cached values that are initialized by Initialize().
	//
	static BOOL m_IsInitialized;  	// initialization flag to insure one-time initialization
	static CString m_SystemDir;		// pathname to Windows system directory
	static CString m_AppDir;		// pathname to application directory
};


#endif