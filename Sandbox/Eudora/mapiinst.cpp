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

#include "stdafx.h"
#include "mapiinst.h"

#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>

#ifndef WIN32
#include <dos.h>
#include <toolhelp.h>
#include <ver.h>
#endif // WIN32

#ifdef _DEBUG
#undef THIS_FILE
static char	BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//
// Statics, initialized by Initialize().
//
BOOL CMapiInstaller::m_IsInitialized = FALSE;	// one-time init flag
CString CMapiInstaller::m_SystemDir; 			// pathname to Windows system directory
CString CMapiInstaller::m_AppDir;				// pathname to application directory

//
// Local constants.
//
const int MAX_SUFFIX_NUM = 10;	// upper bound on DLL suffix range (000 to nnn).
const CString EUDORA16DLLNAME("EUMAPI.DLL");
const CString EUDORA32DLLNAME("EUMAPI32.DLL");
const CString MAPI16DLLNAME("MAPI.DLL");
const CString MAPI32DLLNAME("MAPI32.DLL");

////////////////////////////////////////////////////////////////////////
// CMapiInstaller [default constructor]
//
// Not used.
//
////////////////////////////////////////////////////////////////////////
CMapiInstaller::CMapiInstaller(void)
{
	//
	// This is a static object that is never constructed.
	//
	ASSERT(0);
}

////////////////////////////////////////////////////////////////////////
// CMapiInstaller [copy constructor]
//
// Not used.
//
////////////////////////////////////////////////////////////////////////
CMapiInstaller::CMapiInstaller(const CMapiInstaller&)
{
	//
	// This is a static object that is never constructed.
	//
	ASSERT(0);
}

////////////////////////////////////////////////////////////////////////
// ~CMapiInstaller [destructor]
//
// Not used.
//
////////////////////////////////////////////////////////////////////////
CMapiInstaller::~CMapiInstaller(void)
{
	//
	// This is a static object that is never destroyed.
	//
	ASSERT(0);
}

////////////////////////////////////////////////////////////////////////
// Initialize [private, static]
//
// Self-initialization routine that caches a few commonly-used values
// in member variables.  This should only be done once.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::Initialize(void)
{
	if (m_IsInitialized)
	{
		ASSERT(0);
		return TRUE;
	}

	//
	// Cache the pathname to the Windows system directory.
	//
	char buf[_MAX_PATH + 1];
	if (GetSystemDirectory(buf, sizeof(buf)))
	{
		m_SystemDir = buf;

#ifndef WIN32
		//
		// Okay, for 16-bit code running under WinNT, the
		// GetSystemDirectory() call for some reason returns
		// "c:\winnt\system" rather than "c:\winnt\system32" as you
		// might expect.  If left unchanged, this means that we would
		// be installing the Eudora DLL in the WRONG directory since
		// Win16 apps under NT actually look for DLLs only in the
		// "system32" directory and ignore any DLLs in the "system"
		// directory.  Therefore, let's do a very crude hack to see if
		// we can detect this situation.  BTW, the ::GetVersion()
		// routine is no help since Win NT 3.51 reports a Windows
		// version of 3.10 when running 16-bit code.
		//
		if (FileExists(m_SystemDir + "32"))
		{
			m_SystemDir += "32";	// brutal hack for Win NT
		}
#endif // WIN32

		if (m_SystemDir[m_SystemDir.GetLength() - 1] != '\\')
			m_SystemDir += "\\";	// make sure there's a trailing slash
	}
	else
		return FALSE;

	//
	// Cache the application directory pathname.
	//
	if (GetModuleFileName(AfxGetInstanceHandle(), buf, sizeof(buf)))
	{
		char* p_slash = strrchr(buf, '\\');
		if (p_slash)
			p_slash[1] = '\0';		// truncate the filename part
		m_AppDir = buf;
    }
	else
		return FALSE;

	m_IsInitialized = TRUE;
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// Install [public, static]
//
// Replace the Real MAPI16 and MAPI32 DLLs with the corresponding
// Eudora MAPI16 and MAPI32 DLLs.  This routine makes sure that the
// existing MAPI16 and MAPI32 DLLs are not in use and only performs
// the install if it can verify that the existing DLLs are not Eudora
// DLLs.
//
// Returns a status code to indicate what happened.
//
////////////////////////////////////////////////////////////////////////
CMapiInstaller::Status CMapiInstaller::Install(void)
{
	if (! m_IsInitialized)
	{
		if (! Initialize())
			return STATUS_INTERNAL_ERROR;
		ASSERT(m_IsInitialized);
	}

	//
	// Step 1.  Verify that the Eudora MAPI DLLs to be installed both 
	// exist.
	//
	if (! FileExists(m_AppDir + EUDORA16DLLNAME))
		return STATUS_SOURCE_DLL_NOT_FOUND;
	else if (! FileExists(m_AppDir + EUDORA32DLLNAME))
		return STATUS_SOURCE_DLL_NOT_FOUND;

	//
	// Step 2.  If both of the currently installed 16/32-bit DLLs are
	// present and both are Eudora MAPI DLLs and both Eudora MAPI DLLs
	// are identical, then we're done.
	//
	if (FileExists(m_SystemDir + MAPI16DLLNAME) &&
		IsEudoraDLL(m_SystemDir + MAPI16DLLNAME) &&
		FileExists(m_SystemDir + MAPI32DLLNAME) &&
		IsEudoraDLL(m_SystemDir + MAPI32DLLNAME))
	{
		//
		// Windows has a limitation where the version detection
		// functions do not work across platforms.  That is, 32-bit
		// OSes can only read 32-bit DLLs and 16-bit OSes can only
		// read 16-bit DLLs.  Therefore, we have to ignore bogus
		// comparisons and attempt compares on both 16-bit and 32-bit
		// DLLs until we come up with a definitive answer.
		//
		switch (CompareEudoraDLLs(m_AppDir + EUDORA32DLLNAME, m_SystemDir + MAPI32DLLNAME))
		{
		case STATUS_SUCCESSFUL:
			return STATUS_ALREADY_INSTALLED;
		case STATUS_FAILED:
			break;
		case STATUS_INTERNAL_ERROR:
			switch (CompareEudoraDLLs(m_AppDir + EUDORA16DLLNAME, m_SystemDir + MAPI16DLLNAME))
			{
			case STATUS_SUCCESSFUL:
				return STATUS_ALREADY_INSTALLED;
			case STATUS_FAILED:
			case STATUS_INTERNAL_ERROR:
				break;
			default:
				ASSERT(0);
				break;
			}
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	//
	// Step 3.  Verify that both target DLLs, if they exist, are not
	// "busy" (i.e., loaded into memory).
	//
	if ((FileExists(m_SystemDir + MAPI16DLLNAME) &&
	     IsDLLInUse(m_SystemDir + MAPI16DLLNAME)) ||
	    (FileExists(m_SystemDir + MAPI32DLLNAME) &&
	     IsDLLInUse(m_SystemDir + MAPI32DLLNAME)))
	{
		//
		// A target DLL is in use and therefore cannot be disturbed.
		//
		return STATUS_DLL_IN_USE;
	}

	//
	// Step 4.  Install the Eudora MAPI16 DLL.  If the existing MAPI16
	// DLL is an "old" Eudora MAPI16 DLL, then blow it away.
	// Otherwise, if the existing MAPI16 DLL is not a Eudora MAPI16
	// DLL, then save it to a backup name.
	//
	if (FileExists(m_SystemDir + MAPI16DLLNAME))
	{
		//
		// There are two cases -- either we have a different (probably
		// old) Eudora DLL or we have a non-Eudora DLL.
		//
		if (IsEudoraDLL(m_SystemDir + MAPI16DLLNAME))
		{
			if (! DoUninstallEudoraDLL(m_SystemDir + MAPI16DLLNAME))
				return STATUS_FAILED;
		}
		else
		{
			if (! DoBackupMapiDLL(m_SystemDir + MAPI16DLLNAME))
				return STATUS_FAILED;
		}
	}

	//
	// The 16-bit moment of truth!
	//
	if (! DoInstallEudoraDLL(m_AppDir + EUDORA16DLLNAME, m_SystemDir + MAPI16DLLNAME))
		return STATUS_FAILED;

	//
	// Step 5.  Install the Eudora MAPI32 DLL.  If the existing MAPI32
	// DLL is an "old" Eudora MAPI32 DLL, then blow it away.
	// Otherwise, if the existing MAPI32 DLL is not a Eudora MAPI32
	// DLL, then save it to a backup name.
	//
	if (FileExists(m_SystemDir + MAPI32DLLNAME))
	{
		//
		// There are two cases -- either we have a different (probably
		// old) Eudora DLL or we have a non-Eudora DLL.
		//
		if (IsEudoraDLL(m_SystemDir + MAPI32DLLNAME))
		{
			if (! DoUninstallEudoraDLL(m_SystemDir + MAPI32DLLNAME))
				return STATUS_FAILED;
		}
		else
		{
			if (! DoBackupMapiDLL(m_SystemDir + MAPI32DLLNAME))
				return STATUS_FAILED;
		}
	}

	//
	// The 32-bit moment of truth!
	//
	if (! DoInstallEudoraDLL(m_AppDir + EUDORA32DLLNAME, m_SystemDir + MAPI32DLLNAME))
		return STATUS_FAILED;

	//
	// If we get this far, then hey ... we performed a successful
	// install.  Now, update the WIN.INI file so that the MAPI stuff
	// is enabled for 16-bit MAPI clients.
	//
	::WriteProfileString("Mail", "MAPI", "1");
	return STATUS_SUCCESSFUL;
}


////////////////////////////////////////////////////////////////////////
// Uninstall [public, static]
//
// Remove one or both of the Eudora MAPI DLLs, restoring the original
// MAPI DLLs, if any.  The existing DLLs are removed only if they
// are not Eudora DLLs.
//
////////////////////////////////////////////////////////////////////////
CMapiInstaller::Status CMapiInstaller::Uninstall(void)
{
	if (! m_IsInitialized)
	{
		if (! Initialize())
			return STATUS_INTERNAL_ERROR;
		ASSERT(m_IsInitialized);
	}
		
	//
	// Step 1.  If neither of the currently installed 16/32-bit DLLs are
	// Eudora MAPI DLLs, then we're done.
	//
	if (! ((FileExists(m_SystemDir + MAPI16DLLNAME) &&
			IsEudoraDLL(m_SystemDir + MAPI16DLLNAME)) ||
		   (FileExists(m_SystemDir + MAPI32DLLNAME) &&
			IsEudoraDLL(m_SystemDir + MAPI32DLLNAME))))
	{
		return STATUS_NOT_INSTALLED;
	}

	//
	// Step 2.  Verify that both target DLLs, if they exist, are not
	// "busy" (i.e., loaded into memory).
	//
	if ((FileExists(m_SystemDir + MAPI16DLLNAME) &&
	     IsDLLInUse(m_SystemDir + MAPI16DLLNAME)) ||
	    (FileExists(m_SystemDir + MAPI32DLLNAME) &&
	     IsDLLInUse(m_SystemDir + MAPI32DLLNAME)))
	{
		//
		// A target DLL is in use and therefore cannot be disturbed.
		//
		return STATUS_DLL_IN_USE;
	}

	//
	// Step 3.  If there is a Eudora MAPI16 DLL installed, then
	// remove it and replace it with the original MAPI16 DLL, if any.
	//
	if (FileExists(m_SystemDir + MAPI16DLLNAME) &&
		IsEudoraDLL(m_SystemDir + MAPI16DLLNAME))
	{
		if (! DoUninstallEudoraDLL(m_SystemDir + MAPI16DLLNAME))
			return STATUS_FAILED;

		if (! DoRestoreMapiDLL(m_SystemDir + MAPI16DLLNAME))
			return STATUS_FAILED;
	}

	//
	// Step 4.  If there is a Eudora MAPI32 DLL installed, then
	// remove it and replace it with the original MAPI32 DLL, if any.
	//
	if (FileExists(m_SystemDir + MAPI32DLLNAME) &&
		IsEudoraDLL(m_SystemDir + MAPI32DLLNAME))
	{
		if (! DoUninstallEudoraDLL(m_SystemDir + MAPI32DLLNAME))
			return STATUS_FAILED;

		if (! DoRestoreMapiDLL(m_SystemDir + MAPI32DLLNAME))
			return STATUS_FAILED;
	}

	//
	// If we get this far, then hey ... we performed a successful
	// install.
	//
	return STATUS_SUCCESSFUL;
}


////////////////////////////////////////////////////////////////////////
// DoBackupMapiDLL [private, static]
//
// 1.  If there is no existing MAPI DLL in the Windows System directory,
// then continue to step 3.
//
// 2.  If the current MAPI DLLs are not Eudora MAPI DLLs, rename the
// Microsoft MAPI DLLs in the Windows System directory.  As a
// precaution, the renaming only occurs if we can verify that the
// existing MAPI DLL is *not* a Eudora MAPI DLL since we only want
// System (i.e, "real") MAPI DLLs as backup DLLs.  
//
// If the existing backup DLL is the same as the current DLL, then
// we're done.  Otherwise, if the existing backup DLL is different
// than the current DLL, then the user will be warned and the renamed
// DLL will have a different name than the existing backup DLLs.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::DoBackupMapiDLL(const CString& dllName)
{
	ASSERT(m_IsInitialized);

	//
	// Verify that the given DLL filename has a 3 character extension
	// (e.g., ".DLL").
	//
	ASSERT(dllName.GetLength() > 4);
	if ((dllName.ReverseFind('.') != (dllName.GetLength() - 4)) ||		// missing 3 character extension (e.g., ".DLL")
		IsDLLInUse(dllName) ||											// file in use, don't try renaming
		(! FileExists(dllName)) ||										// can't backup non-existent file
		(IsEudoraDLL(dllName)))											// Eudora DLL already installed
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Rename a file of the form "c:\winXX\system\mapi.dll" to
	// the form "c:\winXX\system\mapi.000".
	//
	for (int i = 0; i <= MAX_SUFFIX_NUM; i++)
	{
		//
		// Generate a unique numeric filename extension in the range
		// 000 to MAX_SUFFIX_NUM.
		//
		ASSERT(MAX_SUFFIX_NUM < 1000);
		char extension[20];
		wsprintf(extension, ".%03d", i);

		//
		// Create a new pathname that uses the generated extension.
		//
		CString newname(dllName.Left(dllName.GetLength() - 4) + extension);

		if (! FileExists(newname))
		{
			//
			// FORNOW, using the rename() function from the C library
			// here instead of MFC CFile::Rename() method since the
			// MFC method throws exceptions on failure.  If we decide
			// to implement C++ exception handling, then we should go
			// back and use the MFC method rather than the C library
			// function.
			//
			if (rename(dllName, newname) == 0)
				return TRUE;
			else
				return FALSE;
		}
		else
		{
			//
			// FORNOW, just silently go to the next suffix.  However,
			// it might be nice add a prompt here so that the user has
			// a clue that there are multiple MAPI backup DLLs sitting
			// in their Windows system directory.
			//
		}
	}

	//
	// If we get this far, we must have more than MAX_SUFFIX_NUM old backup
	// files.  Whoa.  I that it is okay to punt in this case.
	//
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// DoRestoreMapiDLL [private, static]
//
// Note -- this routine assumes that the file to be restored is not
// present.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::DoRestoreMapiDLL(const CString& dllName)
{
	ASSERT(m_IsInitialized);

	//
	// Verify that the given DLL filename has a 3 character extension
	// (e.g., ".DLL") and that the file to be restored doesn't exist.
	//
	ASSERT(dllName.GetLength() > 4);
	if ((dllName.ReverseFind('.') != (dllName.GetLength() - 4)) ||
		FileExists(dllName))
	{
		ASSERT(0);
		return FALSE;		// internal error
	}

	//
	// Scan the disk for an existing backup DLL file of the form
	// "c:\winXX\system\mapi.nnn", starting at MAX_SUFFIX_NUM and
	// working downwards to 000.  The newest one we find (i.e., the
	// one with the largest suffix number) is probably the best
	// candidate for the restore.
	//
	for (int i = MAX_SUFFIX_NUM; i >= 0; i--)
	{
		//
		// Generate a 3-digit numeric extension in the range
		// 000 to MAX_SUFFIX_NUM.
		//
		ASSERT(MAX_SUFFIX_NUM < 1000);
		char extension[20];
		wsprintf(extension, ".%03d", i);

		//
		// Create a new pathname that uses the generated extension.
		//
		CString backup_name(dllName.Left(dllName.GetLength() - 4) + extension);

		if (FileExists(backup_name))
		{
			//
			// To be super-paranoid, it might be a good idea to 
			// check whether the backup DLL is in use, but then
			// again ... nahhhh, never mind.  :-)
			//
			
			
			if (i != 0)
			{
				//
				// FORNOW, we are not warning the user that there are
				// multiple candidates for backup DLLs sitting in the
				// Windows system directory.
				//
			}

			//
			// FORNOW, using the rename() function from the C library
			// here instead of MFC CFile::Rename() method since the
			// MFC method throws exceptions on failure.  If we decide
			// to implement C++ exception handling, then we should go
			// back and use the MFC method rather than the C library
			// function.
			//
			if (rename(backup_name, dllName) == 0)
				return TRUE;
			else
				return FALSE;
		}
	}

	//
	// If we get this far, there are no backup files to restore.  No
	// problema ... just report this as a "successful" restore.
	//
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// DoInstallEudoraDLL [private, static]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::DoInstallEudoraDLL(
	const CString& sourceDLLName, 
	const CString& targetDLLName)
{
	ASSERT(m_IsInitialized);

	//
	// The assumption at this point is that the target DLL file has
	// safely renamed and is no longer present in the Windows 
	// System directory.
	//
	if (FileExists(targetDLLName))
	{
		ASSERT(0);
		return FALSE;		// internal error
	}

	//
	// Another assumption at this point is that the source Eudora DLL
	// is present.
	//
	if (! FileExists(sourceDLLName))
	{
		ASSERT(0);
		return FALSE;		// internal error
	}
		

#ifdef WIN32
	//
	// For Win32, the file copy is a one-liner.
	//
	return CopyFile(sourceDLLName, targetDLLName, TRUE);
#else
	//
	// For Win16, we have to do the file copy ourselves.  Start by
	// opening the source file for binary read access.
	//
	int fd_source = open(sourceDLLName, _O_BINARY | _O_RDONLY);
	if (-1 == fd_source)
		return FALSE;

	//
	// Create the target file for binary write access.
	//
	int fd_target = open(targetDLLName, _O_BINARY | _O_CREAT | _O_EXCL | _O_WRONLY, _S_IWRITE);
	if (-1 == fd_target)
	{
		close(fd_source);
		return FALSE;
	}

	//
	// Run a loop to transfer the contents of the source file to 
	// the target file.
	//
	BOOL status = TRUE;		// returned
	for (;;)
	{
		const int BUFSIZE = 1024;
		char buf[BUFSIZE];
		int numbytes = read(fd_source, buf, sizeof(buf));
		if (numbytes < 0)
		{
			//
			// Read failure... 
			//
			status = FALSE;
			break;
		}
		else if (0 == numbytes)
		{
			//
			// Just happened to hit the end-of-file.
			//
			ASSERT(TRUE == status);
			break;
		}
		
		if (write(fd_target, buf, numbytes) < numbytes)
		{
			//
			// Write failed...
			//
			status = FALSE;
			break;
		}
	}

	//
	// Close down any open files.
	//
	if (fd_source != -1)
	{
		close(fd_source);
		fd_source = NULL;
	}

	if (fd_target != -1)
	{
		close(fd_target);
		fd_target = NULL;
	}
	
	if (TRUE == status)
	{
		//
		// On a successful file copy, set the datestamp of the
		// target file to match that of the source file.
		//
		int fh;
		unsigned datestamp;
		unsigned timestamp;

		if (_dos_open(sourceDLLName, _O_RDONLY, &fh) == 0)
		{
			_dos_getftime(fh, &datestamp, &timestamp);
			_dos_close(fh);

			if (_dos_open(targetDLLName, _O_RDONLY, &fh) == 0)
			{
				_dos_setftime(fh, datestamp, timestamp);
				_dos_close(fh);
			}
		}
	}

	return status;
#endif // WIN32
}


////////////////////////////////////////////////////////////////////////
// DoUninstallEudoraDLL [private, static]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::DoUninstallEudoraDLL(const CString& dllName)
{
	ASSERT(m_IsInitialized);

	if ((! FileExists(dllName)) ||		// can't remove a non-existent file.
	    (! IsEudoraDLL(dllName)) ||		// shouldn't remove a non-Eudora DLL.
		IsDLLInUse(dllName))			// can't remove a busy file.
	{
		ASSERT(0);
		return FALSE;					// internal error
	}

	if (remove(dllName) == -1)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// FileExists [private, static]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::FileExists(const CString& fileName)
{
	CFileStatus unused;

	if (CFile::GetStatus(fileName, unused) == 0)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// IsDLLInUse [public, static]
//
// Return TRUE if the DLL appears to be in memory.  It is assumed that
// the file to be checked exists.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::IsDLLInUse(const CString& fileName)
{
	//
	// The general idea is to try to open the file in share mode such
	// that we have exclusive access.  If this fails, then that
	// implies that somebody else (most likely Windows itself) has the
	// file open.  NOTE: For Win 3.x, this assumes that SHARE.EXE is
	// running.  If it's not, then this routine is unreliable.
	//
	// FORNOW, not doing a check for Win 3.x to see whether or not
	// the SHARE program is loaded.
	//
	ASSERT(FileExists(fileName));
	int fd = sopen(fileName, _O_BINARY | _O_RDONLY, _SH_DENYRD);
	if (-1 == fd)
		return TRUE;		// somebody else has the file open
	else
	{
		close(fd);

#ifndef WIN32
		//
		// For the case where we're running the 16-bit MAPI Installer
		// under Win 3.x (or the Win 3.x emulation under Windows NT),
		// then let's do an extra check to see if a 16-bit MAPI DLL is
		// currently loaded into memory.  This covers the case where
		// SHARE.EXE is not running under Win 3.x and the sopen()
		// call succeeds even though it could not acquire exclusive
		// access to the file.
		//
		// Note that this only covers the MAPI16 DLL when running from
		// a 16-bit MAPI Installer.  It does not detect the MAPI16 DLL
		// when running from a 32-bit MAPI Installer.  Curiously, the
		// 16-bit installer is able to see the SHARE lock on a 32-bit
		// DLL since Win32 places the proper SHARE lock on the file.
		// 
		MODULEENTRY mod_entry;
		mod_entry.dwSize = sizeof(MODULEENTRY);
		if (ModuleFindName(&mod_entry, "MAPI") != NULL)
			return TRUE;;
#endif // WIN32

		return FALSE;		// nobody else has the file open
	}
}


////////////////////////////////////////////////////////////////////////
// IsEudoraDLL [private, static]
//
// The 'fileName' parameter is assumed to be a fully-qualified
// pathname.  The trick here is to look in the header section of the
// DLL file for the "stub" program.  The Eudora version of the "stub"
// program (MAPISTUB.EXE) contains the string "This program requires
// Eudora Pro" at a known offset in the file.  Nice hack, huh?  :-)
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiInstaller::IsEudoraDLL(const CString& fileName)
{
	ASSERT(FileExists(fileName));
	FILE* p_stream = fopen(fileName, "rb");
	if (NULL == p_stream)
		return FALSE;		// file open error

	//
	// Seek to offset 515 from the beginning of the file.
	//
	if (fseek(p_stream, 515, SEEK_SET))
	{
		fclose(p_stream);
		return FALSE;		// seek error
	}

	//
	// Read 32 bytes from the file.
	//
	const int BUFSIZE = 32;
	char buf[BUFSIZE + 1];
	if (fread(buf, BUFSIZE, 1, p_stream) != 1)
	{
		fclose(p_stream);
		return FALSE;		// read error
	}
	buf[BUFSIZE] = '\0';	// force null termination
	fclose(p_stream);

	if (stricmp(buf, "This program requires Eudora Pro") == 0)
 		return TRUE;

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// CompareEudoraDLLs [private, static]
//
// Peek at the VERSIONINFO resource in the DLL files and compare them.
// Return one of the following status codes:
//
// STATUS_SUCCESSFUL		-- files are identical
// STATUS_FAILED			-- files are definitely different
// STATUS_INTERNAL_ERROR	-- cannot do valid comparison
//
////////////////////////////////////////////////////////////////////////
CMapiInstaller::Status CMapiInstaller::CompareEudoraDLLs(
	const CString& dllName1, 		//(i) Full pathname of master Eudora DLL.
	const CString& dllName2)		//(i) Full pathname of installed Eudora DLL.
{
	ASSERT(FileExists(dllName1));
	ASSERT(FileExists(dllName2));
	ASSERT(IsEudoraDLL(dllName1));
	ASSERT(IsEudoraDLL(dllName2));
	
	//
	// Do setup to prepare for the incoming version data.
	//
	DWORD handle1 = 0;
	DWORD size1 = ::GetFileVersionInfoSize((char *)(const char *) dllName1, &handle1);
	if (NULL == size1)
		return STATUS_INTERNAL_ERROR;		// 1st DLL doesn't have VERSIONINFO data?

	DWORD handle2 = 0;
	DWORD size2 = ::GetFileVersionInfoSize((char *)(const char *) dllName2, &handle2);
	if (NULL == size2)
		return STATUS_INTERNAL_ERROR;		// 2nd DLL doesn't have VERSIONINFO data?

	//
	// Allocate data buffers of the proper sizes.
	//
	LPSTR p_data1 = new char[size1];
	if (NULL == p_data1)
		return STATUS_INTERNAL_ERROR;
	LPSTR p_data2 = new char[size2];
	if (NULL == p_data2)
	{
		delete [] p_data1;
		return STATUS_INTERNAL_ERROR;
	}

	//
	// Now, fetch the actual VERSIONINFO data.
	//
	if ((! ::GetFileVersionInfo((char *)(const char *) dllName1, handle1, size1, p_data1)) ||
		(! ::GetFileVersionInfo((char *)(const char *) dllName2, handle2, size2, p_data2)))
	{
		ASSERT(0);		// missing VERSIONINFO data?
		delete [] p_data1;
		delete [] p_data2;
		return STATUS_INTERNAL_ERROR;
	}

	//
	// Fetch and compare CompanyName strings from each DLL.
	//
	LPSTR p_buf = NULL;
	UINT bufsize = 0;
	CString value1;
	CString value2;
	Status status = STATUS_SUCCESSFUL;		// innocent until proven guilty

	//
	// Okay, you would think that passing a raw string literal to an
	// argument that takes a string pointer would be okay.  Well, not
	// if you're running the 16-bit VerQueryValue() function under
	// Windows 3.1/32s.  It GPFs if you pass it a string literal.  So,
	// the workaround is to copy the string to a local buffer first,
	// then pass the string in the buffer.  This, for some inexplicable
	// reason, works under all Windows OSes.
	//
	const char* FILEINFOPATH = "\\StringFileInfo\\040904E4\\";
	char kludgestr[_MAX_PATH];
	strcpy(kludgestr, FILEINFOPATH);
	strcat(kludgestr, "CompanyName");
	if (::VerQueryValue(p_data1,
						kludgestr, 
						(void **) &p_buf, 
						&bufsize))
	{
		if (p_buf && bufsize)
			value1 = p_buf;
	}
	else
	{
		ASSERT(0);			// required CompanyName string missing
		status = STATUS_INTERNAL_ERROR;
	}

	if (STATUS_SUCCESSFUL == status)
	{
		p_buf = NULL;
		bufsize = 0;
		strcpy(kludgestr, FILEINFOPATH);
		strcat(kludgestr, "CompanyName");
		if (::VerQueryValue(LPVOID(p_data2),
							kludgestr, 
							(void **) &p_buf, 
							&bufsize))
		{
			if (p_buf && bufsize)
				value2 = p_buf;
		}
		else
		{
			ASSERT(0);		// required CompanyName string missing
			status = STATUS_INTERNAL_ERROR;
		}
	}

	if (STATUS_SUCCESSFUL == status)
	{
		ASSERT(! value1.IsEmpty());
		ASSERT(! value2.IsEmpty());

		if (value1 != value2)
			status = STATUS_FAILED;
	}

	//
	// Fetch and compare FileVersion strings from each DLL.
	//
	if (STATUS_SUCCESSFUL == status)
	{
		value1.Empty();
		value2.Empty();
		p_buf = NULL;
		bufsize = 0;

		strcpy(kludgestr, FILEINFOPATH);
		strcat(kludgestr, "FileVersion");
		if (::VerQueryValue(p_data1,
							kludgestr, 
							(void **) &p_buf, 
							&bufsize))
		{
			if (p_buf && bufsize)
				value1 = p_buf;
		}
		else
		{
			ASSERT(0);			// required FileVersion string missing
			status = STATUS_INTERNAL_ERROR;
		}

		if (STATUS_SUCCESSFUL == status)
		{
			p_buf = NULL;
			bufsize = 0;
			strcpy(kludgestr, FILEINFOPATH);
			strcat(kludgestr, "FileVersion");
			if (VerQueryValue(LPVOID(p_data2),
							  kludgestr, 
							  (void **) &p_buf, 
							  &bufsize))
			{
				if (p_buf && bufsize)
					value2 = p_buf;
			}
			else
			{
				ASSERT(0);		// required FileVersion string missing
				status = STATUS_INTERNAL_ERROR;
			}
		}

		if (STATUS_SUCCESSFUL == status)
		{
			ASSERT(! value1.IsEmpty());
			ASSERT(! value2.IsEmpty());

			if (value1 != value2)
				status = STATUS_FAILED;
		}
	}

	//
	// All roads should lead here...
	//
	delete [] p_data1;
	delete [] p_data2;

	return status;
}

