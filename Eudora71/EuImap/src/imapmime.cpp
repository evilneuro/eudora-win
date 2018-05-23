// IMAPMIME.CPP
//
// MIME File utility routines
//


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.


#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <direct.h>

#include <QCUtils.h>

#include "resource.h"
#include "rs.h"
#include "eudora.h"

#include "ImapExports.h"
#include "ImapMime.h"

#include "DebugNewHelpers.h"

// Statics:
static void LOCAL_DeMacifyName(char* Name);
static void LOCAL_DeMacifyName32(char* Name);
static int LOCAL_CheckName(const char* Dir, const char* FName, char* FinalFname, BOOL bLongFileNameSupport);
//=================================================================================/


// DeMacifyName
//
// Take the name given to us and figure out how to map it
// to something more useful.
// Mac names can contain funny characters, spaces, be up to 32
// characters long etc, etc. We need 8 plus an extention.
//

static void LOCAL_DeMacifyName(char* Name)
{
	static const char Illegal[] = " \t\r\n\".*+,/:;<=>?[\\]|";
	char pcname[15];
	int len = 8;
	char *s, *d, *ext;
	
	if (s = strpbrk(Name, "\r\n"))
		*s = 0;
	ext = strrchr(Name, '.');

	for (d = pcname, s = Name; *s && s != ext && len; s++)
	{
		if ((*s & 0x80) || (*s < 32) || strchr((LPCSTR)Illegal, *s))
			continue;
		*d++ = *s;
		len--;
	}
	*d = 0;

	if (ext)
	{
		len = 8;
		*d++ = *ext++;
		for (s = ext; *s && len; s++)
		{
			if ((*s & 0x80) || (*s < 32) || strchr((LPCSTR)Illegal, *s))
				continue;
			*d++ = *s;
			len--;
		}
		*d = 0;
	}

	strcpy(Name, pcname);
}

static void LOCAL_DeMacifyName32(char* Name)
{
	static const char Illegal[] = "\t\r\n\"\\/:*?<>|";

	char pcname[255];
	int len = 255;
	char *s, *d;
	
	if (s = strpbrk(Name, "\r\n"))
		*s = 0;

	for (d = pcname, s = Name; *s  && len; s++)
	{
		if (/*(*s & 0x80) ||*/ ((unsigned char)*s < 32) || strchr((LPCSTR)Illegal, *s))
			continue;
		*d++ = *s;
		len--;
	}
	*d = 0;

	strcpy(Name, pcname);
}



static int LOCAL_CheckName(const char* Dir, const char* FName, char* FinalFname, BOOL bLongFileNameSupport)
{
	char DirName[_MAX_PATH + 1];

	strcpy(DirName, Dir);
	if (DirName[strlen(DirName) - 1] == '\\')
		DirName[strlen(DirName) - 1] = NULL;

	int TryNum = 1, len;
	char tmp[255], *ptr, *dot;
	int BaseLen;
	
	//gets the string of DOS reserved device names
	CString sReservedWords;
	char *pReserved = sReservedWords.GetBuffer(1024);
	GetIniString(IDS_INI_DOS_RESERVED, pReserved, 1024);
	sReservedWords.ReleaseBuffer();

	if ((strlen(DirName) + strlen(FName)) > _MAX_PATH-2)	// The file is too big to create...
	{
		strncpy(FinalFname, DirName, strlen(DirName)+1);
		int endofFname = strlen(FinalFname);
		FinalFname[endofFname] = SLASH;
		FinalFname[endofFname+1] = 0;

		int Difference = (strlen(DirName) + strlen(FName))- (_MAX_PATH - 2);
		strncat(FinalFname, FName, (strlen(FName)-Difference));

		char * floater;
		floater = strrchr(FName, '.');
		
		if (floater)	// There's a period, so there's an extension.
		{
			char *leader;
			leader = FinalFname + (((strlen(DirName) + strlen(FName))-Difference) - strlen(floater));
			while (*floater)
			{
				*leader = *floater;
				leader++;
				floater++;
			}
			*leader = 0;
		}
	}

	else
	{
		wsprintf(FinalFname, "%s%c%s", DirName, SLASH, FName);
	}

	//variables definition
	BOOL isReserved = FALSE;
	pReserved = (char *)(LPCTSTR)sReservedWords;
	CString sResWord;
	char *DOSptr;
	//store in tmp the file name without the extension
	DOSptr = strchr(FName, '.');
	len = (DOSptr? DOSptr - FName : strlen(FName));
	strncpy(tmp, FName, len);
	tmp[len] = 0;

	while (1)
	{
		//loop to see if tmp has one of the DOS reserved device names
		while (*pReserved != NULL)
		{
			if ((DOSptr = strchr(pReserved, ',')) != NULL)
			{
				//store in sResWord one DOS device name at a time so it can be compared
				len = DOSptr - pReserved;
				ASSERT( len < 9);
//				strncpy((char*)(LPCTSTR)sResWord, pReserved, len);
				sResWord = CString(pReserved);
				char* pResWord = (char*)(LPCTSTR)sResWord;
				pResWord[len]='\0';
			}
			else
				//Last device name so copy it
//				strcpy((char*)(LPCTSTR)sResWord, pReserved);
				sResWord = CString(pReserved);
		
			if (stricmp(tmp, sResWord) == 0)	//if match
			{
				isReserved = TRUE;
				break;
			}
			else
				isReserved = FALSE;

			//if there are still device names to compare
			if (DOSptr)
				pReserved = DOSptr + 1;
			else
				break;
		}

		if (!isReserved && !::FileExistsMT(FinalFname))
        	break;

		// point to the beginning of the device string just in case the new namebecomes
		// one of the reserved DOS device names
		pReserved = (char *)(LPCTSTR)sReservedWords;

		ptr = strrchr(FName, '.');
		len = (ptr? ptr - FName : strlen(FName));
		if (bLongFileNameSupport)
			BaseLen = 255;
		else
			BaseLen = 8;
			
		BaseLen -= (TryNum < 10? 1 : (TryNum < 100? 2 : 3));
		len = min(len, BaseLen);
		strncpy(tmp, FName, len);
		tmp[len] = 0;

		char tmp2[255];
		// do the following to add the number to the reserved name and the following
		// dot seperated section of the name.  THIS IS A CRUCIAL IF statement;
		// IF DELETED IT WILL INFINITLY LOOP 
		if ((dot = strchr(tmp, '.')) && isReserved)
		{
			strcpy(tmp2, dot);
			*dot = 0;
			wsprintf(FinalFname, "%s\\%s%d%s%0.4s", DirName, tmp, TryNum++, tmp2, (ptr? ptr : ""));
		}
		else
			wsprintf(FinalFname, "%s\\%s%d%0.4s", DirName, tmp, TryNum++, (ptr? ptr : ""));
	
		wsprintf(tmp, "%s%d", tmp, TryNum);
	}

	return (TRUE);
}



// OpenLocalAttachFile
//
// Legalizes the file name and opens the attachment file
//
//IMAPFile* OpenLocalAttachFile(char *pParentDir, char* Name)
//
// If bCreateStub, we're creating an attachment stub that belongs 
// in the IMAP mailbox' Attach directory, NOT the attachment folder. -jdboyd
//

JJFileMT* OpenLocalAttachFile(char *pParentDir, char* Name, BOOL bCreateStub)
{
	JJFileMT* ATFile;
	char buf[_MAX_PATH+1];
    char FinalFname[_MAX_PATH+1];
    char DirName[_MAX_PATH+1];
	CString attachDir;
	char* FileName = NULL;

	// Sanity:	
	if (!(pParentDir && Name))
		return NULL;

    *buf = 0;
	GetIniString(IDS_INI_AUTO_RECEIVE_DIR, DirName, sizeof(DirName));
	if (/*(bCreateStub==TRUE) ||*/ (DirName[0] == 0) || !FileExistsMT(DirName, TRUE))
	{ 
		WIN32_FIND_DATA wfd;
		attachDir = pParentDir + CRString(IDS_ATTACH_FOLDER); 
		attachDir.MakeLower();
		strcpy(DirName, attachDir);

		// see if we need to create "attach" directory
		HANDLE findHandle;
		findHandle = FindFirstFile((const char *)attachDir,&wfd);
		if ( findHandle == INVALID_HANDLE_VALUE)
		{
			// if we can't make the directory, reset it to to old way
			if (mkdir(attachDir) < 0)
				if (errno != EACCES)
					strcpy(DirName, pParentDir);
		}
		FindClose(findHandle);
	}


	// Turn the macintosh name into something more useful
	BOOL bLongFileNameSupport = ::LongFileSupportMT(DirName);
	if (bLongFileNameSupport)
		LOCAL_DeMacifyName32(Name);
	else
		LOCAL_DeMacifyName(Name);

	if (LOCAL_CheckName(DirName, Name, FinalFname, bLongFileNameSupport))
	{
		FileName = strrchr(FinalFname, SLASH) + 1;
		wsprintf(buf, "%s%s%s", DirName,
		((DirName[strlen(DirName) - 1] == SLASH)? "" : SLASHSTR), FileName);
	}

	// Let's get the file
	ATFile = DEBUG_NEW_NOTHROW JJFileMT; 
	if (!ATFile || (!SUCCEEDED (ATFile->Open(buf, O_CREAT | O_WRONLY) ) ) ) 
	{
		if (ATFile)
			delete ATFile;
        ATFile = NULL;
	}

	return (ATFile);
}



// GetUniqueDir()
// NOTES:
// Get a name for a directory that doesn't match one that already exists.
// Base the name on the given string. Use the string as-is if possible.
// Set the new name into the referenced CString.
// The resulting name must be at most MaxNameLength long.
// The name cannot contain dots or directory delimiters.
// If "bCreate" is TRUE, attempt to create the directory as well.
//
// The returned result is the RELATIVE name, NOT the full pathname.
//
// END NOTES
//

BOOL GetUniqueDir (LPCSTR pParentDir, LPCSTR pSuggestedName, CString &NewName, short MaxNameLength, BOOL bCreate)
{
	CString	Name, BasePath, Path;
	int i;
	BOOL	bResult = FALSE;

	if (!(pSuggestedName && pParentDir))
		return FALSE;

	// Create a base path for use in trying several pathnmames.
	FormatBasePath (pParentDir, BasePath);

	// Truncate the Suggested Name
	// If name begins with a dot, keep it a part of the name,
	// otherwise use only the chars before the dot.
	// Make sure it's at most MaxNameLength characters long.

	Name = pSuggestedName;
	i = Name.Find('.');
	if (i == 0)
	{
		Name = Name.Mid ( 1 );
	}
	else if (i > 1)	
	{
		Name = Name.Left (i);
	}

	// Extract basename if contains \\.

	i = Name.ReverseFind (DirectoryDelimiter);
	if (i >= 0)
	{
		Name = Name.Mid (i + 1);
	}
	// Make sure it's at least of maximum length
	Name = Name.Left (MaxNameLength);

	// Only allow alpha-numeric chars in the name.
	CString tmpName;
	for (LPCSTR p = (LPCSTR) Name; p && *p; p++)
	{
		if ( isalnum (*p) )
			tmpName += *p;
	}

	Name = tmpName;

	// Do we have anything left?
	if (Name.IsEmpty())
		Name = '0';

	// See if the directory or a file by that name exists and find another name if necessary.
	Path = BasePath + Name;
	DWORD Attributes = GetFileAttributes ((LPCSTR)Path);
	if (Attributes == 0xFFFFFFFF)
	{
		// Found a valid name.
		if (!bCreate)
		{
			NewName = Name;
			bResult = TRUE;
		}
		else if ( CreateDirectory (Path, NULL) )
		{
			NewName = Name;
			bResult = TRUE;
		}
		else
			bResult = FALSE;
	}

	// Try some heuristics.
	if (!bResult)
	{
		if (Name.GetLength() == MaxNameLength)
			Name = Name.Left (MaxNameLength - 1);

		CString BaseName = Name;

		// Form a name by appending a single digit. If we still can't find a non-existing
		// name, then fail.
		// BUG: We should try harder than this!!
		for (i = 1; (i <= 100) && !bResult; i++)
		{
			Name.Format ("%s%d", (LPCSTR)BaseName, i);
			Path = BasePath + Name;
		
			Attributes = GetFileAttributes ((LPCSTR)Path);
			if (Attributes == 0xFFFFFFFF)	// Doesn't exist. Use it.
			{

				// Found a valid name.
				if (!bCreate)
				{
					NewName = Name;
					bResult = TRUE;
				}
				else if ( CreateDirectory (Path, NULL) )
				{
					NewName = Name;
					bResult = TRUE;
				}
				else
					bResult = FALSE;
			}
		}
	}
	
	return bResult;
}		


#endif // IMAP4
