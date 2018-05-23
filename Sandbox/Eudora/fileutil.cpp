// FILEUTIL.CPP
//
// File utility routines
//

#include "stdafx.h"


#include <QCUtils.h>

#include "fileutil.h"
#include "resource.h"
#include "guiutils.h"

#include "rs.h"  //for CRString, EudoraEnvironVar, SetupINIFilename

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


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
// We retry opens for write if the error is an access error.
// The value of 13 was obtained by experiment (queasy) SD 3/10/99
#define FAILED_BUT_RETRY(hr) \
	(FAILED(hr) && \
	(HRESULT_CODE(hr)==QCUTIL_E_FILE_OPEN_WRITING) && \
	(GetLastError_()==13))
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
HRESULT JJFile::Rename(const char* NewName, BOOL DeleteFirst /*= TRUE*/)
{
	//ASSERT(::IsMainThreadMT());

	HRESULT hr = JJFileMT::Rename(NewName, DeleteFirst);
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

	CString strError;
	switch (HRESULT_CODE(hr))
	{
	case QCUTIL_E_FILE_WRITE:
		::AfxFormatString1(strError, IDS_ERR_FILE_WRITE, (const char *) strFilename);
		break;
	case QCUTIL_E_FILE_READ:
		::AfxFormatString1(strError, IDS_ERR_FILE_READ, (const char *) strFilename);
		break;
	case QCUTIL_E_FILE_OPEN_WRITING:
		strError.Format(CRString(IDS_ERR_FILE_OPEN), (const char *) strFilename, (const char *) CRString(IDS_ERR_FILE_OPEN_WRITING));
		break;
	case QCUTIL_E_FILE_OPEN_READING:
		strError.Format(CRString(IDS_ERR_FILE_OPEN), (const char *) strFilename, (const char *) CRString(IDS_ERR_FILE_OPEN_READING));
		break;
	case QCUTIL_E_FILE_RENAME:
		ASSERT(pszExtra != NULL);
		//ErrorDialog(IDS_ERR_FILE_RENAME, (const char *) strFilename, (const char *) pszExtra);
		ReportError(IDS_ERR_FILE_RENAME, (const char *) strFilename, (const char *) pszExtra);
		return;
	case QCUTIL_E_FILE_DELETE:
		ASSERT(pszExtra != NULL);
		//ErrorDialog(IDS_ERR_FILE_DELETE, (const char *) strFilename);
		ReportError(IDS_ERR_FILE_DELETE, (const char *) strFilename);
		return;
	default:
		ASSERT(0);
		strError = "???";
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
CString ExecutableDir;
CString TempDir;

BOOL CheckMailDirectory(const char* dir, const char* Location)
{
	char Edir[_MAX_PATH + 1];

	// fully qualify the path
	if ( ( dir[ 1 ] != ':' ) && ( strstr( dir, "\\\\" ) != dir ) )
	{
		int drive = _getdrive();
		Edir[ 0 ] = (char)(drive + 'A' - 1);	// drive 1 = A, 2 = B, etc.
		Edir[ 1 ] = ':';
		Edir[ 2 ] = '\0';
	}
	else
	{
		Edir[ 0 ] = '\0';
	}

	strcat(Edir, dir);
	::TrimWhitespaceMT(Edir);
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
    if (!::FileExistsMT(Edir, TRUE))	
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
void VerifyDir(const char *DirName, BOOL bCreate)
{
	if (!::FileExistsMT(DirName, TRUE) && bCreate)
		mkdir(DirName);
	
}

//////////////////////////////////////////////////////////////
BOOL GetDirs(char* CmdLine)
{
	int	done = 0;
	char buf[_MAX_PATH + 1]; 
	int status = FALSE;
	BOOL validTempDir = TRUE;
	char *Dir = NULL, *Ini = NULL, *temp;
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
		EudoraDirLen = EudoraDir.GetLength();
	}

	//If we did't find a valid TempDir then create a TEMP dir in the Eudora directory
	if ((validTempDir == FALSE) || (TempDir.IsEmpty()) )
	{
		TempDir = EudoraDir + CRString( IDS_TEMP2_ENVIRON_VAR );
		//The following call will create the dir if it does not exist
		VerifyDir(TempDir, TRUE);
	}

	if (TempDir.Right(1) != SLASH)
		TempDir += SLASH;
	
	// Verify &/or create Plugins Dir & Filters Dir
	VerifyDir((EudoraDir + CRString( IDS_TRANS_DIRECTORY )), TRUE);
	VerifyDir((EudoraDir + CRString( IDS_FILTER_DIRECTORY )), TRUE);

	return (status);
}



// This function is kinda weird.  
//
// It's used is to determine if Eudora needs to be launched with an old command line 
// (as saved in the registry).  This happens if there is an attachment,
// stationery (.sta), mailto directive (/m), or a message (.msg) specified on the cmdline.
//
// As soon as one of the above types is detected, this function bails.
//
// Determining if an attachment is specified requires elimination of all other 
// possibilites.  This is due to the fact that attachments can be named anything,
// as long as they don't end in .msg or .sta.  So the code has to eliminate the
// possibility of directory or ini file before concluding an attachment has been 
// specified.

COMMANDLINE_TYPE GetCommandType(char* cmdLine)
{
	// Did we get anything on the command line?
	if (!cmdLine || !*cmdLine)
		return CL_EMPTY;

	// Check for mailto: URL, which is a /m option
	if (!strnicmp(cmdLine, "/m ", 3) || !strnicmp(cmdLine, "mailto:", 7))
		return CL_MAILTO;

	// Look for old command lines:
	// 	 Directory 
	// 	 Directory + Ini File
	// 	 Ini File
	// If it's none of the above, assume it's an attachment

	COMMANDLINE_TYPE retVal = CL_ATTACH;
	struct stat FileAttr;
	char test[255];
	char *Line = new char[strlen(cmdLine)+1];
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

	if (status == 0 && FileAttr.st_mode & S_IFDIR)
		retVal = CL_DIR; 
	else if (status == 0 && len > 4 && !stricmp(StaExt, firstArg + len - 4))
		return CL_STATIONERY;
	else if (status == 0 && len > 4 && !stricmp(MsgExt, firstArg + len - 4))
		return CL_MESSAGE;
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
	const CString& destFilename)	//(i) optional filename for dest file
{
	//
	// Determine the "candidate" filename for the copied attachment
	// file.  If this filename conflicts with an existing file in the
	// Eudora attachments directory, then the file copy routine will
	// automatically rename it.
	//
	CString filename(destFilename);
	if (destFilename.IsEmpty())
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
		
	JJFile* OpenAttachFile(char*);		// prototype for extern function
	JJFile* p_destfile = NULL;
	if (! (p_destfile = OpenAttachFile((char *) ((const char *) filename))))
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


