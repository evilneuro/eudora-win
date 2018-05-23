// FILEUTIL.CPP
//
// File utility routines
//
// Copyright (c) 1991-2000 by QUALCOMM, Incorporated
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

#include "stdafx.h"

// Support for GetLongPathName on Windows95
#define COMPILE_NEWAPIS_STUBS
#define WANT_GETLONGPATHNAME_WRAPPER
#include "NewAPIs.h"

#include <QCUtils.h>

#include "fileutil.h"
#include "resource.h"
#include "guiutils.h"

#include "rs.h"  //for CRString, EudoraEnvironVar, SetupINIFilename

#include "DebugNewHelpers.h"


JJFile::JJFile(int _size /*= BUF_SIZE*/, BOOL bDisplayErrors /*= TRUE*/) :
	JJFileMT(_size),
	m_bDisplayErrors(bDisplayErrors)
{
	//ASSERT(::IsMainThreadMT());
	ErrorCallback = 0;
}


JJFile::JJFile(char* _buf, int _size, BOOL bDisplayErrors /*= TRUE*/) :
	JJFileMT(_buf, _size),
	m_bDisplayErrors(bDisplayErrors)
{
	//ASSERT(::IsMainThreadMT());
		ErrorCallback = 0;
}


JJFile::~JJFile()
{
	//ASSERT(::IsMainThreadMT());
}



////////////////////////////////////////////////////////////////////////
// Open [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Open(const char* pszFilename, int mode)
{
	//ASSERT(::IsMainThreadMT());

	// We'll repeat the operation if we get the "right" sort of error;
	// Setup the repeat count & interval SD 3/10/99
	int repeats = min(10L,max(1L,GetIniLong(IDS_INI_OPEN_RETRY_COUNT)));
	int interval = min(1000L,max(10L,GetIniLong(IDS_INI_OPEN_RETRY_INTERVAL)));

	HRESULT hr = JJFileMT::Open(pszFilename, mode, repeats, interval);

	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// Flush [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Flush()
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Flush();
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// Seek [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Seek(long lOffset, int nMode /*= SEEK_SET*/, long* plNewOffset /*= NULL*/)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Seek(lOffset, nMode, plNewOffset);
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// Delete [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Delete()
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Delete();
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}



////////////////////////////////////////////////////////////////////////
// Rename [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Rename(const char* NewName, BOOL bReplace /*= TRUE*/)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Rename(NewName, bReplace);
	if (FAILED(hr))
		DisplayErrorDialog_(hr, NewName);
	return hr;
}




////////////////////////////////////////////////////////////////////////
// JJBlockMove [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::JJBlockMove(long Start, long Length, JJFileMT* Target)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::JJBlockMove(Start, Length, Target);
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// GetLine [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::GetLine(char* pBuffer, long lBufferSize, long* plNumBytesRead /*= NULL*/)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::GetLine(pBuffer, lBufferSize, plNumBytesRead);
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}



////////////////////////////////////////////////////////////////////////
// Read [public, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Read(char* pBuffer, long lSize)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Read(pBuffer, lSize);
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// Write_ [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
HRESULT JJFile::Write_(const char* pBuffer, long lNumBytesToWrite)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Write_(pBuffer, lNumBytesToWrite);
	if (FAILED(hr))
		DisplayErrorDialog_(hr);
	return hr;
}





void JJFile::ReportError(UINT StringID, ...)
{
	
	if(!m_bDisplayErrors)
		return;

	char buf[1024] = {0};
		
	va_list argList;
	va_start(argList, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), argList);
	va_end(argList);

	if( ErrorCallback)
		ErrorCallback(buf);
	else
	{
		ASSERT(::IsMainThreadMT());
		AlertDialog(IDD_ERROR_DIALOG, buf);
	}
}


////////////////////////////////////////////////////////////////////////
// DisplayErrorDialog_ [protected]
//
////////////////////////////////////////////////////////////////////////
void JJFile::DisplayErrorDialog_(HRESULT hr, const char* pszExtra /*= NULL*/)
{
	//ASSERT(::IsMainThreadMT());

	if (! m_bDisplayErrors)
		return;
	
	//
	// Get the filename.
	//
	CString strFilename("???");
	{
		BSTR bstrFilename = NULL;
		if (SUCCEEDED(GetFName(&bstrFilename)))
		{
			strFilename = bstrFilename;
			::SysFreeString(bstrFilename);
		}
	}

	CString strError("???");
	switch (HRESULT_CODE(hr))
	{
	case QCUTIL_E_FILE_WRITE:
		::AfxFormatString1(strError, IDS_ERR_FILE_WRITE, (const char *) strFilename);
		break;
	case QCUTIL_E_FILE_READ:
		::AfxFormatString1(strError, IDS_ERR_FILE_READ, (const char *) strFilename);
		break;
	case QCUTIL_E_FILE_OPEN_WRITING:
		::AfxFormatString2(strError, IDS_ERR_FILE_OPEN, (const char *) strFilename, (const char *) CRString(IDS_ERR_FILE_OPEN_WRITING));
		break;
	case QCUTIL_E_FILE_OPEN_READING:
		::AfxFormatString2(strError, IDS_ERR_FILE_OPEN, (const char *) strFilename, (const char *) CRString(IDS_ERR_FILE_OPEN_READING));
		break;
	case QCUTIL_E_FILE_RENAME:
		ASSERT(pszExtra != NULL);
		::AfxFormatString2(strError, IDS_ERR_FILE_RENAME, (const char *) strFilename, (const char *) pszExtra);
		break;
	case QCUTIL_E_FILE_DELETE:
		::AfxFormatString1(strError, IDS_ERR_FILE_DELETE, (const char *) strFilename);
		break;
	case QCUTIL_E_FILE_CLOSE:
		break;
	default:
		LPTSTR lpBuffer = NULL;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL, hr, 0, (LPTSTR)&lpBuffer, 0, NULL);
		::AfxFormatString2(strError, IDS_ERR_FILE_UNKNOWN, (const char *) strFilename, lpBuffer);
		LocalFree(lpBuffer);
		break;
	}

	//
	// GetLastError() is presumed to return the value of the global
	// ::errno (assumed to be in the range 0 thru 36) for displaying
	// extra information about WHY the file I/O call failed.
	//
	int nError = GetLastError_();
	if (nError < 0 || nError > 36)
		nError = 37;		// unknown error

	const int IDS_FILE_BASE_ERROR = 8500;
	ReportError(IDS_DOS_ERR_FORMAT, 
					(const char *) strError,
					(const char*) CRString(IDS_FILE_BASE_ERROR + nError), nError);
}




int EudoraDirLen = 0;
CString EudoraDir;
CString EudoraDirNoBackslash;
CString ExecutableDir;
CString TempDir;

BOOL CheckMailDirectory(const char* dir, const char* Location)
{
	char szShortEDir[_MAX_PATH + 1];
	char Edir[_MAX_PATH + 1];

	// fully qualify the path
	if ( ( dir[ 1 ] != ':' ) && ( strstr( dir, "\\\\" ) != dir ) )
	{
		int drive = _getdrive();
		szShortEDir[ 0 ] = (char)(drive + 'A' - 1);	// drive 1 = A, 2 = B, etc.
		szShortEDir[ 1 ] = ':';
		szShortEDir[ 2 ] = '\0';
	}
	else
	{
		szShortEDir[ 0 ] = '\0';
	}

	strcat(szShortEDir, dir);
	::TrimWhitespaceMT(szShortEDir);

	GetLongPathName(szShortEDir, Edir, _MAX_PATH);

	EudoraDirLen = strlen(Edir);
	if (Edir[EudoraDirLen - 1] == SLASH)
       	Edir[EudoraDirLen - 1] = 0;

	// Check for root directories because access() will barf otherwise
	if (Edir[0] == 0)
		;
	else if (isalpha((int)(unsigned char)(Edir[0])) && Edir[1] == ':' && Edir[2] == 0)
	{	
		Edir[2] = SLASH; 
		Edir[3] = 0;	
	}
        
    //
    // Check to make sure that the directory exists and is *writable*.
	//
    if (!::FileExistsMT(Edir))	
	{
		ErrorDialog(IDS_ERR_BAD_EUDORA_DIR, Location, dir);
		return (FALSE);
    }
		  
	// If this is a directory, then add the trailing backslash
	if (strlen(Edir) > 3)
	{
		struct stat FileAttr;
		int status = stat(Edir, &FileAttr);
		if ( status == 0 && FileAttr.st_mode & S_IFDIR )
		{
			// Add trailing backslash
			strcat(Edir, SLASHSTR);
		}
	}
	
	EudoraDir = Edir;
	EudoraDirLen = EudoraDir.GetLength();
	return (TRUE);
}

//////////////////////////////////////////////////////////////
BOOL GetDirs(char* CmdLine)
{
	int	done = 0;
	char buf[_MAX_PATH + 1]; 
	int status = FALSE;
	BOOL validTempDir = TRUE;
	char *Ini = NULL, *temp;
	CRString Temp1(IDS_TEMP1_ENVIRON_VAR), Temp2(IDS_TEMP2_ENVIRON_VAR);
	CRString EudoraEnvironVar(IDS_EUDORA_ENVIRON_VAR);

	//The Win32 call GetTempPath returns the directory specified by TMP or TEMP or 
	//the Windows dir if neither tmp or temp env vars were found. But the directories
	//are not validated on WinNT. So we see if they exist ourselves, else create a
	//TEMP dir in the Mail directory since we don't want to leave files lying around
	//in the Windows dir incase Eudora crashed.
	if ( ! GetEnvironmentVariable(Temp1, buf, sizeof(buf)) || (!::FileExistsMT(buf, TRUE)) )
	{
		if ( ! GetEnvironmentVariable(Temp2, buf, sizeof(buf)) || (!::FileExistsMT(buf, TRUE)) )
			validTempDir = FALSE;
	}

	//Found a valid TempDir? Then copy it.
	if (validTempDir == TRUE)
		TempDir = buf;

	if (GetModuleFileName(AfxGetInstanceHandle(), buf, sizeof(buf)))
	{
		temp = strrchr(buf, SLASH);
		if (temp)
		{
			temp[1] = 0;
			ExecutableDir = buf;
		}
    }

	if (CmdLine && *CmdLine)
    {
		// If long filename, arguments will be separated by quotes
		if (Ini = strstr(CmdLine, "\" "))
			*(++Ini)++ = 0;
		else if ((*(CmdLine + strlen(CmdLine) - 1) != '\"') && (Ini = strchr(CmdLine, ' ')))
			*Ini++ = 0;

#ifdef WIN32
		::StripQuotesMT(CmdLine);
		if (Ini)
		{
			::TrimWhitespaceMT(Ini);
			::StripQuotesMT(Ini);
			GetShortPathName(Ini,Ini, strlen(Ini));
		}
#endif

		// If the directory doesn't have any path, then assume it's an INI
		if (!Ini && (!strchr(CmdLine, SLASH) && CmdLine[1] != ':'))
		{
			Ini = CmdLine;
		}
		else
		{
			status = CheckMailDirectory(CmdLine, CRString(IDS_FILE_COMMAND_LINE));
		 	done = 1;
		}
	}

	if (!done && GetEnvironmentVariable(EudoraEnvironVar, buf, sizeof(buf)))
	{
		temp = buf;
		// If the directory doesn't have any path, then assume it's an INI
		if (!strchr(temp, SLASH) && temp[1] != ':')
		{
			Ini = temp;
		}
		else
		{
			status = CheckMailDirectory(temp, CRString(IDS_FILE_EUDORA_ENV_VAR));
			done = 1;
		}
	}

	if (!done && EudoraDir.IsEmpty())
	{
		EudoraDir = ExecutableDir;
		status = TRUE;
	}

	if (status)
	{
		// If we don't have an Ini file, but the EudoraDir actually points
		// to a file, then assume it is an Ini file and set the
		// Eudora directory to the directory the file is in.
		struct stat FileAttr;
		if (!Ini && EudoraDir[EudoraDir.GetLength() - 1] != SLASH &&
			(stat(EudoraDir, &FileAttr) == 0) && !(FileAttr.st_mode & S_IFDIR))
		{
			Ini = ::SafeStrdupMT(EudoraDir);
			int Slash = EudoraDir.ReverseFind(SLASH);
			if (Slash >= 0)
				EudoraDir.ReleaseBuffer(Slash + 1);
			SetupINIFilename(Ini);
			delete [] Ini;
			Ini = NULL;
		}
		else
			SetupINIFilename(Ini);
	}

	// If there was no command-line specified, and there is a DEudora.ini file, then look
	// to see if the DEudora.ini file has a DataFolder entry in the [Settings] section.
	if ((!CmdLine || !*CmdLine) && !DefaultINIPath.IsEmpty() &&
		GetPrivateProfileString(CRString(IDS_INISN_SETTINGS), "DataFolder", "", buf, sizeof(buf), DefaultINIPath))
	{
		CheckMailDirectory(buf, CRString(IDS_DEUDORA_DATA_FOLDER));
	}

	EudoraDirLen = EudoraDir.GetLength();
	EudoraDirNoBackslash = EudoraDir.Left(EudoraDirLen - 1);

	//If we did't find a valid TempDir then create a TEMP dir in the Eudora directory
	if ((validTempDir == FALSE) || (TempDir.IsEmpty()) )
	{
		TempDir = EudoraDir + CRString( IDS_TEMP2_ENVIRON_VAR );
		//The following call will create the dir if it does not exist
		CreateDirectoryMT(TempDir);
	}

	if (TempDir.Right(1) != SLASH)
		TempDir += SLASH;

	return (status);
}



// This function is kinda weird.  
//
// It's used is to determine if Eudora needs to be launched with an old command line 
// (as saved in the registry).  This happens if there is an attachment,
// stationery (.sta), URL (/m), or a message (.msg) specified on the cmdline.
//
// As soon as one of the above types is detected, this function bails.
//
// Determining if an attachment is specified requires elimination of all other 
// possibilites.  This is due to the fact that attachments can be named anything,
// as long as they don't end in .msg or .sta.  So the code has to eliminate the
// possibility of directory or ini file before concluding an attachment has been 
// specified.

COMMANDLINE_TYPE GetCommandType(LPCTSTR cmdLine)
{
	// Did we get anything on the command line?
	if (!cmdLine || !*cmdLine)
		return CL_EMPTY;

	// Check for URL, which is a /m option
	if (!strnicmp(cmdLine, "/m ", 3) || !strnicmp(cmdLine, "mailto:", 7))
		return CL_URL;

	// Look for old command lines:
	// 	 Directory 
	// 	 Directory + Ini File
	// 	 Ini File
	// If it's none of the above, assume it's an attachment

	COMMANDLINE_TYPE retVal = CL_ATTACH;
	struct stat FileAttr;
	char test[255];
	char *Line = DEBUG_NEW char[strlen(cmdLine)+1];
	strcpy(Line,cmdLine);
	
	// parse off firstArg
	// If long filename, arguments will be separated by quotes
	char *firstArg = Line;
	{
		char *Ini = NULL;
		if (Ini = strstr(Line, "\" "))
			 *(++Ini) = 0;
		else if ( (*(Line + strlen(Line) - 1) != '\"') && (Ini = strchr(Line, ' ')))
			 *Ini = 0;
	}

	// strip quotes off firstArg.  stat() doesn't expect them.  Further
	// the code that build up the ini path doesn't seem to strip any leading quotes
	if ( firstArg[ 0 ] == '\"' )
	{
		char * p = strchr( &firstArg[ 1 ], '\"' );
		if ( p )
		{
			*p = 0;		// nix trailing quote
			firstArg++;	// skip leading quote
		}
	}


	// if firstArg ends in '\\' (I.E. C:\APPS\EUDORA\ ) remove the trailing '\\'
	// stat() won't see it as a directory with the trailing '\\', except for "x:\"
	int len = strlen( firstArg );
	if ( len )
	{
		char lastchar = firstArg[ len - 1 ];
		if ( lastchar == '\\' )
		{
			if ( ! ( len == 3 && firstArg[ 1 ] == ':' ) )
				firstArg[ len - 1 ] = ' ';
		}
	}

	// Check if we got an existing file/directory
	int status = stat(firstArg, &FileAttr);

	CString StaExt = '.' + CRString(IDS_STATIONERY_EXTENSION);
	CString MsgExt = '.' + CRString(IDS_MESSAGE_EXTENSION);
	CString MbxExt = CRString(IDS_REG_MAILBOX_EXT);

	if (status == 0 && FileAttr.st_mode & S_IFDIR)
		retVal = CL_DIR; 
	else if (status == 0 && len > 4 && !stricmp(StaExt, firstArg + len - 4))
		retVal = CL_STATIONERY;
	else if (status == 0 && len > 4 && !stricmp(MsgExt, firstArg + len - 4))
		retVal = CL_MESSAGE;
	else if(status == 0 && len > 4 && !stricmp(MbxExt, firstArg + len - 4))
		retVal = CL_MAILBOX;
	else
	{
		// it's not a directory, see if it's an ini file
		char app[_MAX_PATH + 1];
		char val[_MAX_PATH + 1];
		char *c = NULL;
		lstrcpy(val,firstArg);
		
		// If it's not a full path, see if this is a ini file local to app
		c = strrchr(firstArg, SLASH);
		if (!c)
		{
			GetModuleFileName(AfxGetInstanceHandle(), app, sizeof(app)); 
 			GetShortPathName(app,val, sizeof(val));
			c = strrchr(val,SLASH);
			if (c)
				*++c = 0;
			lstrcat(val,firstArg);
		}
	
		// Test to see if this is an ini file
		GetPrivateProfileString("Settings", "POPAccount",",",test, 255, val);
		if (test[0] != ',')
			retVal = CL_INI;
	}
	
	delete [] Line;
	return	retVal;
}


////////////////////////////////////////////////////////////////////////
// CopyAttachmentFile [static]
//
// Given a pathname to a MAPI attachment file, make a copy of the
// attachment file in the Eudora attachments directory.  On input, the
// 'pathname' parameter contains the pathname to temporary the source
// file.  Upon successful return, the pathname contains the name of
// copied destination file in the Eudora attachments directory.  Cool,
// huh?
////////////////////////////////////////////////////////////////////////
BOOL CopyAttachmentFile(
	CString& srcPathname,			//(io) on input, pathname to source
									// file; on output, pathname to
									// file in Eudora attachment directory
	const char* destFilename)		//(i) optional filename for dest file
{
	//
	// Determine the "candidate" filename for the copied attachment
	// file.  If this filename conflicts with an existing file in the
	// Eudora attachments directory, then the file copy routine will
	// automatically rename it.
	//
	CString filename(destFilename);
	if (filename.IsEmpty())
	{
		//
		// No candidate filename provided, so just use the filename
		// from the given pathname string (i.e., the part of the
		// pathname string to the right of the last slash separator).
		//
		filename = srcPathname.Right(srcPathname.GetLength() - (srcPathname.ReverseFind(SLASH) + 1));
	}
    
	JJFile srcfile;
	if (FAILED(srcfile.Open(srcPathname, O_RDONLY)))
		return FALSE;
		
	struct stat stat_info;
	if (FAILED(srcfile.Stat(&stat_info)))
		return FALSE;
		
	JJFile* OpenAttachFile(LPTSTR,BOOL);		// prototype for extern function
	JJFile* p_destfile = NULL;
	if (! (p_destfile = OpenAttachFile((LPTSTR)(LPCTSTR)filename, FALSE)))
		return FALSE;
		
	if (FAILED(srcfile.JJBlockMove(0L, stat_info.st_size, p_destfile)))
	{
		p_destfile->Delete();
		delete p_destfile;
		return FALSE;
	}
	
	{
		BSTR bstrFilename = NULL;
		if (SUCCEEDED(p_destfile->GetFName(&bstrFilename)))
		{
			srcPathname = bstrFilename;
			::SysFreeString(bstrFilename);
		}
	}

	delete p_destfile;
	
	return TRUE;
}


void GetRegInfoFilePath(CString & szRegInfoFilePath)
{
	szRegInfoFilePath = EudoraDir + CRString( IDS_REG_INFO_STARTUPREG_NAME );
}


bool GetRegInfoFileAlternatePath(CString & szRegInfoFilePath)
{
	bool	bAltPathIsDifferent = (EudoraDir.Compare(ExecutableDir) != 0);

	if (bAltPathIsDifferent)
		szRegInfoFilePath = ExecutableDir + CRString( IDS_REG_INFO_STARTUPREG_NAME );

	return bAltPathIsDifferent;
}


bool
ChangeFileNameExtension(
	char *					in_szFileName,
	const char *			in_szNewExtension,
	long					in_nFileNameBufferSize)
{
	bool	bFileExtensionChanged = false;

	if (in_szFileName && in_szNewExtension && *in_szNewExtension)
	{
		char *	szFileExt = strrchr(in_szFileName, '.');
		
		// Chop off old extension - if any
		if (szFileExt)
			*szFileExt = '\0';
		
		// File extension should be just extension with no period
		ASSERT(*in_szNewExtension != '.');
		
		// Handle period just to be sure
		if (*in_szNewExtension == '.')
			++in_szNewExtension;
		
		int		nCurrentFileNameLength = strlen(in_szFileName);
		int		nFileExtLength = strlen(in_szNewExtension);
		
		// Make sure that there's enough room
		if ( (nCurrentFileNameLength + nFileExtLength + 1) < in_nFileNameBufferSize )
		{
			// Append period
			*szFileExt++ = '.';

			// Now append new extension (without period)
			strcpy(szFileExt, in_szNewExtension);
			bFileExtensionChanged = true;
		}
		else
		{
			// Notify developer when debugging
			ASSERT(0);
			
			// Unable to fit new extension - restore period
			if (szFileExt)
				*szFileExt = '.';
		}
	}

	return bFileExtensionChanged;
}

//
// CascadeBackupFile - back up a file in cascade fashion
//  in_szFilePath - the file to be backed up
//  in_nBackups - the number of backups to keep, or 0 to remove all backups
//  Each time this routine is called, the oldest backup is removed, the other
//  backups all have one digit added to their extension, and the file itself is
//  backed up as file.001
HRESULT CascadeBackupFile(const char * in_szFilePath, const int nBackup)
{
	HRESULT result = ERROR_SUCCESS;

	// sanity-check the backup value
	ASSERT(nBackup>=0 && nBackup<99);

	CString sBackupPath;	// the backup file we're working with
	int n;								// the backup number we're working with
	CString sErr;					// for convenience of logging error messages

	// first, let's count the backups we already have
	for (n=0;n<99;n++)
	{
		sBackupPath.Format(IDS_BACKUP_FILE_FMT,in_szFilePath,n+1);
		if (!FileExistsMT(sBackupPath)) break;
	}
	
	// now, let's delete the ones we don't need
	for (int i=n;i>0 && i>=nBackup;i--)
	{
		sBackupPath.Format(IDS_BACKUP_FILE_FMT,in_szFilePath,i);

		// Check for existence of file before attempting to remove
		// so that we don't log something that we expect to fail.
		if (FileExistsMT(sBackupPath))
		{
			// if the delete fails, we'll figure that out later when we rename
			// for now, LALALALALALALALALALALA
			result = FileRemoveMT(sBackupPath);

			// At least log that we failed...
			if (FAILED(result))
			{
				sErr.Format("Remove \"%s\" failed (%x)",sBackupPath,result);
				PutDebugLog(DEBUG_MASK_TOC_CORRUPT,sErr);
			}
		}
	}

	// is there anything left to do?
	if (!nBackup) return result;	// if the user passed zero, just kill backups and
													// return the error from the last deletion

	// rename files nBackupPath-1..1 to nBackupPath..2.
	if (nBackup>1 && n>0)
	{
		int		nLastFile = min(n+1, nBackup);
		
		for (i=nLastFile;i>1;i--)
		{
			CString sBackupPath2;

			sBackupPath.Format(IDS_BACKUP_FILE_FMT,in_szFilePath,i-1);

			// Check for existence of file before attempting to rename so that we
			// don't stop or log an error, when we expect the operation to fail.
			if (FileExistsMT(sBackupPath))
			{
				sBackupPath2.Format(IDS_BACKUP_FILE_FMT,in_szFilePath,i);

				result = FileRenameMT(sBackupPath,sBackupPath2);
				if (FAILED(result))
				{
					sErr.Format("Rename \"%s\" to \"%s\" failed (%x)",sBackupPath,sBackupPath2,result);
					PutDebugLog(DEBUG_MASK_TOC_CORRUPT,sErr);
					return result;
				}
			}
		}
	}

	// so far so good.  Now, copy the darn file!
	sBackupPath.Format(IDS_BACKUP_FILE_FMT,in_szFilePath,1);
	if (CopyFile(in_szFilePath,sBackupPath,FALSE))
		result = ERROR_SUCCESS;
	else
		result = GetLastError();

	sErr.Format("Backed up \"%s\" to \"%s\" (%x)",in_szFilePath,sBackupPath,result);
	PutDebugLog(DEBUG_MASK_TOC_CORRUPT,sErr);

	ASSERT(SUCCEEDED(result));

	return result;
}

bool ConvertToLongPathName(CString &csPathName)
{
	char		szLongPathName[_MAX_PATH+1];
	
	DWORD		dwResult = GetLongPathName(csPathName, szLongPathName, _MAX_PATH);

	if ( (dwResult == 0) || (dwResult > _MAX_PATH) )
		return false;

	csPathName = szLongPathName;
	return true;
}
