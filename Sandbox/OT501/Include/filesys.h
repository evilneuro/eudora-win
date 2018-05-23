// Stringray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc,
// All rights reserved.
//
// SEC FileSystem file format header
//
// This source code is only intended as a supplement to the
// Stingray Extension Classes
// See the Stingray Extension Classes help files for
// detailed information regarding using SEC classes.
//
// CHANGELOG:
//
//    SPL       11/30/95 Started
//
//
//

#ifndef __SECFILESYSTEM_H__
#define __SECFILESYSTEM_H__


//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#ifdef WIN32

#include <direct.h>
// winbase.h defines this to be (0x100000L) for some reason.
#ifdef GetFreeSpace
#undef GetFreeSpace
#endif

#else

#include "dos.h"
#include "direct.h"

#endif


//***************************************************************************
// Definitions
//***************************************************************************
#define DRIVE_UNDETERMINED 0


class SECFileSystem : public CObject
{

// Constructors
public:
  SECFileSystem();        // Constructor

// Attributes
private:

#ifdef WIN32

  WIN32_FIND_DATA  m_FileInfo;
  HANDLE           m_hFind;

#else

	struct _find_t	m_FileInfo;

#endif

  int              m_nMaxFileNameLength;


// Implementation
public:
  virtual ~SECFileSystem();        // Destructor

  // File/Directory Attributes

#ifndef WIN32
#define FILE_ATTRIBUTE_NORMAL CFile::normal
#endif

  enum Attribute {
    normal    = FILE_ATTRIBUTE_NORMAL,  // Normal Files (Win32 API version 0x80)
    readOnly  = CFile::readOnly,        // Read-Only Files
    hidden    = CFile::hidden,          // Hidden Files
    system    = CFile::system,          // System Files
    volume    = CFile::volume,          // Volume Label
    directory  = CFile::directory,      // Subdirectory Entry
    archive    = CFile::archive,        // Archive File

    allfiles  = SECFileSystem::normal    |    // All "files"
                SECFileSystem::readOnly  |    // Used to determine "size" of directory.
                SECFileSystem::hidden    |
                SECFileSystem::system    |
                SECFileSystem::archive,
  }; // Attribute


// Operations

  // Operations - Directory
  CString       GetCurrentDirectory(const CString& FileSystem = "");
  BOOL          ChangeDirectory(const CString& NewDirectory);
  BOOL          RenameDirectory(const CString& OldName, const CString& NewName);
  BOOL          MakeDirectory(const CString& NewDirectory);
  BOOL          MakePath(const CString& NewDirectory);
  BOOL          DeleteDirectory(const CString& Directory, const BOOL bDeleteFilesAndDirs = FALSE);

  LONG          GetDirectorySize(const CString& Directory = "", const CString& WildCard = "*.*", const BOOL bRecurseSubdirs = FALSE);

  // Operations - FileSystem

  CString       GetCurrentFileSystem();
  BOOL          ChangeFileSystem(const CString& FileSystem);
  CStringList * GetFileSystemList();
  CString       GetVolumeLabel(const CString& FileSystem);
  LONG          GetFileSystemType(const CString& FileSystem);
  BOOL          IsReadOnlyFileSystem(const CString& FileSystem);
  LONG          GetFreeSpace(const CString& FileSystem);
  LONG          GetTotalSpace(const CString& FileSystem);

  // Operations - File

  BOOL          GetFileStatus(const CString& FileName, CFileStatus& FileStatus);
  BOOL          GetFileCreateTime(const CString& FileName, CTime& time);
  BOOL          GetFileModifyTime(const CString& FileName, CTime& time);
  BOOL          GetFileAccessTime(const CString& FileName, CTime& time);
  BOOL          GetFileSize(const CString& FileName, unsigned long& lSize);
  BOOL          GetFileAttribute(const CString& FileName, BYTE& Attribute);

  BOOL          RenameFile(const CString& OldFileName, const CString& NewFileName);
  BOOL          DeleteFile(const CString& FileName);
  BOOL          DeleteFiles(const CString& FileSpec, const unsigned long eFileAttrib = normal);

  BOOL          CopyFile(const CString& SourceFileName, const CString& DestFileName, const unsigned long lBuffSize = 10240);
  BOOL          CopyFiles(const CString& FileSpec, const CString& DestPath, const BOOL bIgnoreErrors = FALSE, const unsigned long eFileAttrib = normal);

  // Operations - Comparison

  BOOL          CompareFiles(const CString& FileName1, const CString& FileName2, const unsigned long lBuffSize = 10240);
  BOOL          CompareDirectories(const CString& PathName1, const CString& PathName2, const BOOL bRecurseSubdirs = FALSE, const BOOL bCompareFileContents = FALSE, const unsigned long eFileAttrib = normal);

  // Operations - Existance
  
  BOOL          FileExists(const CString& PathAndFileName);
  BOOL          FileSystemExists(const CString& FileSystemName);
  BOOL          DirectoryExists(const CString& Path);

  // Operations - File/Path Name

  CString       GetFullPathName(const CString& PathAndFileName);
  CString       GetFileName(const CString& PathAndFileName);
  CString       GetPath(const CString& PathAndFileName, const BOOL bIncludeFileSystem = TRUE);
  CString       GetExtension(const CString& PathAndFileName);
  CString       GetFileSystem(const CString& PathAndFileName);
  CString       GetBaseFileName(const CString& PathAndFileName);

  CString       AppendWildcard(const CString& Path, const CString& Wildcard);

  // Operations - Directory Listing

  CStringList * GetDirectory(const CString& SearchString, const unsigned long eFileAttrib, const BOOL bRecurseSubDirs=FALSE, CStringList *pStringList = NULL);

  CStringList * GetSubdirList(const CString& SearchDir, const BOOL bPathInName = TRUE);
  CStringList * GetFileList(const CString& SearchString, const unsigned long eFileAttrib);

  CString *     GetDirectoryEntry(const CString& Wildcard = "", const unsigned long eFileAttrib = normal);


  // Operations - MFC CStringList

  static void   Sort(CStringList *pStringList, BOOL bCase = TRUE);

  // Operations - MFC GUI

  static void   LoadListBox(CListBox *pListBox, const CStringList * pStringList);
  static void   LoadComboBox(CComboBox *pComboBox, const CStringList * pStringList);

// Overridables


// Implementation

protected:
  BOOL          ChangeFileSystem(const char cFileSystem);
  BOOL          CloseFile(CFile *pFile) const;
  BOOL          GetADirectoryEntry(CString &fileName, const CString& Wildcard = "", const unsigned long eFileAttrib = normal);

}; // SECFileSystem


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // _SECFILESYSTEM_H_

