// NSPrefs.cpp : implementation file
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
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
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _EUDORA_INSTALLER_
	#include "fileutil.h"
#endif

#include "nsprefs.h"
#include <io.h>


#include "DebugNewHelpers.h"


// This preprocessor directive can be used in cases where the user wants to limit the association of other files in the Eudora branch
// Please see SwMapi project where the same is demonstrated
#ifndef _EUDORA_INSTALLER_
	extern CString ExecutableDir;
#else
	CString ExecutableDir;
#endif //_EUDORA_INSTALLER_


// Function to Update the Netscape's Prefs.js file
// BOOL bAddEudoraAsDefaultEmailer : TRUE - add the 2 lines in prefs.js to configure Eudora as the Default Mailto handler
//                                 : FALSE - remove the 2 lines in prefs.js which configured Eudora as the Default Mailto handler
//
// Function to locate the prefs.js file on a user's machine
// This uses 2 ways to locate the prefs.js file & the second logic needs to be revisted
// for better performance. 

BOOL LocateNetscapePrefsFile(CStringList& FileList)
{
	HKEY		hKey;
	DWORD		dwIndex=0;
	
	TCHAR		szTempBuffer[512];
	DWORD		dwTempBufSize = 512;

	HKEY		hChildKey;
	FILETIME	fileTime;

	TCHAR		szData[1024];
	DWORD		dwDataBufSize=1024;
	TCHAR		szPrefsFileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT];

	DWORD		dwKeyDataType;
	static		BOOL	bDoItOnce = FALSE;

	static	BOOL	bRet = FALSE;
	BOOL		bFoundPrefsPathInUserRegKey = FALSE;

	static	CStringList csPrefsFileList;	
	if (bDoItOnce == FALSE)
	{
		csPrefsFileList.RemoveAll();
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			_T("Software\\NetScape\\Netscape Navigator\\Users"),
			0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{	
			
			while (RegEnumKeyEx(hKey, dwIndex++, szTempBuffer, &dwTempBufSize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS)
			{
				if(RegOpenKeyEx(hKey, szTempBuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
				{
					if (RegQueryValueEx(hChildKey, _T("DirRoot"), NULL, &dwKeyDataType,
						(LPBYTE) szData, &dwDataBufSize) != ERROR_SUCCESS)
					{

						continue;
					}
					
					// Open File
					_tcscpy(szPrefsFileName, szData);
					_tcscat(szPrefsFileName, _T("\\prefs.js"));

					csPrefsFileList.AddTail((const TCHAR *)szPrefsFileName);
					bFoundPrefsPathInUserRegKey = TRUE;

					bRet = TRUE;
					
				}
				dwTempBufSize = 512;				
				RegCloseKey(hChildKey);
			}
			RegCloseKey(hKey);
		}

		if (FALSE == bFoundPrefsPathInUserRegKey)
		{
			// The reason we have to search for the file is because of the registry entries made by 4.3 & above are differnt 
			// than that of 4.0* - 02/14/2000

			dwIndex = 0;

			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				_T("Software\\NetScape\\Netscape Navigator"),
				0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{	
				while ( (bDoItOnce == FALSE) && (RegEnumKeyEx(hKey, dwIndex++, szTempBuffer, &dwTempBufSize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS) )
				{
					_tcscat(szTempBuffer,_T("\\Main"));
					if(RegOpenKeyEx(hKey, szTempBuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
					{
						if (RegQueryValueEx(hChildKey, _T("Install Directory"), NULL, &dwKeyDataType,
							(LPBYTE) szData, &dwDataBufSize) != ERROR_SUCCESS)
						{
							continue;
						}

						TCHAR *szTemp = _tcsstr(_tcslwr(szData),"\\netscape");

						if (szTemp != NULL)
						{
							TCHAR *szSlash = NULL;
							szSlash = _tcschr(szTemp+1,'\\');
							if (szSlash)								
								szData[szSlash - szData] = '\0';							
							else
								szData[szTemp - szData + strlen(szTemp) + 1] = '\0';			
							_tcscat(szData,"\\Users");
										
						}
						else if (NULL != (szTemp = _tcsstr(_tcslwr(szData),"\\communicator")) )
						{
							szData[szTemp - szData] = '\0';
							_tcscat(szData,"\\Users");

						}

						// Save the current dir so that it can be restored later.
						if (szTemp != NULL && bDoItOnce == FALSE)
						{
							char oldDir[1024]; 							
							GetCurrentDirectory(1023, oldDir); 
						
							if (TRUE == SetCurrentDirectory(szData))
							{
								CStringList	csFilesList;
								GetFilesInDirectory("prefs.js", TRUE, &csFilesList);
								while (! csFilesList.IsEmpty())
								{
									CString strName = csFilesList.RemoveHead();
									csPrefsFileList.AddTail((const TCHAR *)strName);								
									bRet = TRUE;								
								}

								SetCurrentDirectory(oldDir);
							}
						}
						RegCloseKey(hChildKey);
					}
					dwTempBufSize = 512;
				}
				RegCloseKey(hKey);
			}

			// The following if loop is the addition for Netscape 6.0 compatibility
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				_T("Software\\NetScape\\Netscape 6"),
				0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{	
				dwIndex = 0;
				while ( (bDoItOnce == FALSE) && (RegEnumKeyEx(hKey, dwIndex++, szTempBuffer, &dwTempBufSize, 0, NULL, NULL, &fileTime) == ERROR_SUCCESS) )
				{
					_tcscat(szTempBuffer,_T("\\Main"));
					if(RegOpenKeyEx(hKey, szTempBuffer, 0, KEY_READ, &hChildKey) == ERROR_SUCCESS)
					{
						if (RegQueryValueEx(hChildKey, _T("Install Directory"), NULL, &dwKeyDataType,
							(LPBYTE) szData, &dwDataBufSize) != ERROR_SUCCESS)
						{
							continue;
						}

						if(szData[_tcslen(szData) - 1] == '\\')
							szData[_tcslen(szData) - 1] = '\0';

						TCHAR *szUserString = _tcsrchr(szData,'\\');
						if (szUserString)
						{
							*szUserString = '\0';
							_tcscat(szData,"\\Users50");
						}

						// Save the current dir so that it can be restored later.
						if (bDoItOnce == FALSE)
						{
							char oldDir[1024]; 							
							GetCurrentDirectory(1023, oldDir); 
						
							if (TRUE == SetCurrentDirectory(szData))
							{
								CStringList	csFilesList;
								GetFilesInDirectory("prefs.js", TRUE, &csFilesList);
								while (! csFilesList.IsEmpty())
								{
									CString strName = csFilesList.RemoveHead();
									csPrefsFileList.AddTail((const TCHAR *)strName);								
									bRet = TRUE;
									
								}
								
								SetCurrentDirectory(oldDir);
							}
						}
						RegCloseKey(hChildKey);
					}
					dwTempBufSize = 512;
				}
				RegCloseKey(hKey);
			}

		}

		bDoItOnce = TRUE;
	}

	POSITION	nPos = NULL;
	CString strName;
	nPos = csPrefsFileList.GetHeadPosition();
	FileList.RemoveAll();
	while ((! csPrefsFileList.IsEmpty()) && (nPos != NULL))
	{
		strName = csPrefsFileList.GetNext(nPos);
		FileList.AddTail((const TCHAR *)strName);
	}
	

	return bRet;
}

// Function to search for a particular file in the current directory
// Altho' the main idea is from OT Toolkit's GetDirectory(), there function does not
// work completely correct (or as expected) & hence the modification.
CStringList* GetFilesInDirectory(const CString& SearchString, const BOOL bRecurseSubDirs /* = FALSE */, CStringList *pStringList /* = NULL */)
{
	// If they don't pass in a list, create one.
	if (pStringList == NULL)
	{
		pStringList = DEBUG_NEW CStringList();
	} // if

	CStringList *pFileList = DEBUG_NEW CStringList();
	CString csFName;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(SearchString);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();		
		csFName += finder.GetFilePath();
		pFileList->AddTail ((const TCHAR *)(csFName));
	}

	pStringList->AddTail(pFileList);
	delete pFileList;
	pFileList = NULL;

	if (bRecurseSubDirs)
	{
		CString FileName = SearchString; // Copy to modify;
		CString  CurDir = "";
		CString  csDirName = "";

		// Find the last "\" in the string and return everything up to and including it.
		int nIndex = FileName.Find('\\');
		while(nIndex != -1)
		{
			CurDir = CurDir + FileName.Left(nIndex + 1);
			FileName = FileName.Right(FileName.GetLength() - nIndex - 1);

			nIndex = FileName.Find('\\');
		} // while

		CStringList *  pDirList = DEBUG_NEW CStringList();
		
		struct _finddata_t c_file;
		long hFile;

		if( (hFile = _findfirst( CurDir + "*.*", &c_file )) != -1L )
		{
				if ( c_file.attrib & _A_SUBDIR ) 
				{
					OutputDebugString(c_file.name);
					OutputDebugString("\n");
					if ( (FALSE == strcmp(c_file.name,_T(".")) )  &&  
						 (FALSE == strcmp(c_file.name,_T("..")) ) )
					{
						csDirName = CurDir + c_file.name;
						pDirList->AddTail((const TCHAR *)csDirName);
					}
					
				}
        
				while( _findnext( hFile, &c_file ) == 0 )
				{
					if ( c_file.attrib & _A_SUBDIR ) 
					{					
						OutputDebugString(c_file.name);
						OutputDebugString("\n");
						if ( ( !strcmp(c_file.name,_T(".")) ) ||
							 ( !strcmp(c_file.name,_T("..")) ) )
						{
							 continue;
						}
						else
						{
							csDirName = CurDir + c_file.name;
							pDirList->AddTail((const TCHAR *)csDirName);
						}

					}
    
				}

		   _findclose( hFile );
		}

		// Go through the directories we just got and recurse through them too.
		for (POSITION pos=pDirList->GetHeadPosition(); pos != 0; )
		{
		  CString String = pDirList->GetNext(pos);

		  // Get file name part of search path
		  FileName = SearchString;
		  nIndex = FileName.Find('\\');
		  while(nIndex != -1)
		  {
			FileName = FileName.Right(FileName.GetLength() - nIndex - 1);

			nIndex = FileName.Find('\\');
		  } // while

		  CString  SearchSpec = FileName;

		  // Do the recursion.
		  GetFilesInDirectory(String + "\\" + SearchSpec, bRecurseSubDirs, pStringList);
		} // for

		delete pDirList;
		pDirList = NULL;
	} // if

  return pStringList;
} // GetDirectory


BOOL UpdateNetscapePrefsFile(BOOL bAddEudoraAsDefaultEmailer)
{
	TCHAR		szTempBuffer[512];
	TCHAR		szPrefsFileName[_MAX_PATH + _MAX_FNAME + _MAX_EXT];

	long		lNumBytesRead = 0;

	BOOL		bRet = FALSE;
	BOOL		bFoundPrefsPathInUserRegKey = FALSE;
		

	BOOL		bAltMailOption = FALSE;
	BOOL		bAltMailDllOption = FALSE;
	BOOL		bAltMailNewsOption = FALSE;


	// Even before we start updating the prefs.js file, just make sure that EudoraNS.dll is present in "ExecutableDir" directory
	// The chances of it not being there are minimal but it can happen when a user for some reason knowingly or by mistake deletes it
	// or Developers who are debugging do not have EudoraNS.dll yet in there Eudora installation directory.
	
	struct _tfinddata_t strFileData;
	TCHAR	szPostalDLLPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];							

#ifdef _EUDORA_INSTALLER_	

	// Grab the Eudora's executable directory from registry. This could be done always rather than assuming the extern value of ExecutableDir
	// sometimes & grabbing it from registry at others, but lets leave it as it is.
	TCHAR		szData[1024];
	DWORD		dwDataBufSize=1024;
	
	DWORD		dwKeyDataType;	
	HKEY		hKey;

	if ( RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Qualcomm\\Eudora\\CommandLine"),0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, _T("Current"), NULL, &dwKeyDataType,(LPBYTE) szData, &dwDataBufSize) == ERROR_SUCCESS)
		{
			ExecutableDir = szData;

			struct stat FileAttr;
			TCHAR *cLine = DEBUG_NEW TCHAR[ExecutableDir.GetLength()+1];
			_tcscpy(cLine,ExecutableDir);

			// parse off firstArg
			// If long filename, arguments will be separated by quotes
			TCHAR *firstArg = cLine;
			{
				TCHAR *Ini = NULL;
				if (Ini = _tcsstr(cLine, "\" "))
					 *(++Ini) = 0;
				else if ( (*(cLine + strlen(cLine) - 1) != '\"') && (Ini = _tcschr(cLine, ' ')))
					 *Ini = 0;
			}

			// strip quotes off firstArg.  stat() doesn't expect them.  Further
			// the code that build up the ini path doesn't seem to strip any leading quotes
			if ( firstArg[ 0 ] == '\"' )
			{
				TCHAR * p = _tcschr( &firstArg[ 1 ], '\"' );
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
				TCHAR lastchar = firstArg[ len - 1 ];
				if ( lastchar == '\\' )
				{
					if ( ! ( len == 3 && firstArg[ 1 ] == ':' ) )
						firstArg[ len - 1 ] = ' ';
				}
			}

			TCHAR *pSlash = _tcsrchr(firstArg,'\\');
			*pSlash = '\0';

			// Check if we got an existing file/directory
			int status = stat(firstArg, &FileAttr);
			if (status == 0 && FileAttr.st_mode & S_IFDIR)
			{
				// This is where the copying of the file should be done
				// We already have the path, so just append the filename & copy it.
				_tcscat(firstArg,"\\");		

			}		
			
			ExecutableDir = (char *)firstArg;

			if (cLine)
				delete [] cLine;
		}
	}

	else return FALSE;
	
#endif	// _EUDORA_INSTALLER_

	_tcscpy(szPostalDLLPath,ExecutableDir);
	_tcscat(szPostalDLLPath,"EudoraNS.dll");
	long hFile = _tfindfirst( szPostalDLLPath, &strFileData );
	if ( hFile == -1L )
		return FALSE;		// Could not find EudoraNS.dll in the ExecutableDir.

	static	CStringList csPrefsFileList;	
	try
	{
		LocateNetscapePrefsFile(csPrefsFileList);
		POSITION	nPos = NULL;
		nPos = csPrefsFileList.GetHeadPosition();
		while ((! csPrefsFileList.IsEmpty()) && (nPos != NULL))
		{
			CString strName = csPrefsFileList.GetNext(nPos);
			_tcscpy(szPrefsFileName,strName);	
			std::fstream fsPrefsFile(szPrefsFileName, std::ios::in | std::ios::out);

			if ( fsPrefsFile.is_open() )
			{
				while ( (fsPrefsFile.getline(szTempBuffer, 510)) )
				{
					lNumBytesRead = fsPrefsFile.gcount(); 
					if (_tcsstr( _tcslwr(szTempBuffer), _T("user_pref(\"mail.")))
					{
						if ( (FALSE == bAltMailOption) && 
							(_tcsstr(_tcslwr(szTempBuffer), _T("user_pref(\"mail.use_altmail\""))) )
						{
							// Make sure it's user_pref("mail.use_altmail", true);
							if (_tcsstr(_tcslwr(szTempBuffer), _T("true")) )
							{
								
								// If bAddEudoraAsDefaultEmailer is set to FALSE, comment the two line in prefs.js which were responsible 
								// for setting Eudora as the default mailto handler so that when netscape exits, it will automatically 
								// delete them 
								if (FALSE == bAddEudoraAsDefaultEmailer)
								{										
									fsPrefsFile.seekg(-(lNumBytesRead + 1),std::ios::cur);
									fsPrefsFile.put('/');
									fsPrefsFile.put('/');
									fsPrefsFile.put(' ');
									memset(szTempBuffer,' ',lNumBytesRead - 3);
									fsPrefsFile.write(szTempBuffer,lNumBytesRead - 3);

								}

								bAltMailOption = TRUE;
								bRet = TRUE;

							}

						}
						else if ( (FALSE == bAltMailDllOption) && 
								  (_tcsstr(_tcslwr(szTempBuffer), _T("user_pref(\"mail.altmail_dll"))) )
						{
							// Make sure it points to our dll
							if (_tcsstr(_tcslwr(szTempBuffer), _T("eudorans.dll")) )
							{

								// If bAddEudoraAsDefaultEmailer is set to FALSE, comment this line so that when netscape exits, it will automatically delete it
								if (FALSE == bAddEudoraAsDefaultEmailer)
								{											
									fsPrefsFile.seekg(-(lNumBytesRead + 1),std::ios_base::cur);
									fsPrefsFile.put('/');
									fsPrefsFile.put('/');
									fsPrefsFile.put(' ');
									memset(szTempBuffer,' ',lNumBytesRead - 3);
									fsPrefsFile.write(szTempBuffer,lNumBytesRead - 3);
								}
								
								bAltMailDllOption = TRUE;
								bRet &= TRUE;

							}
						}
						else if ( (FALSE == bAltMailNewsOption) && 
								  (_tcsstr(_tcslwr(szTempBuffer), _T("user_pref(\"mail.use_altmail_for_news\""))) )
						{
							if (_tcsstr(_tcslwr(szTempBuffer), _T("false")) )
							{
								if (FALSE == bAddEudoraAsDefaultEmailer)
								{										
									fsPrefsFile.seekg(-(lNumBytesRead + 1),std::ios_base::cur);
									fsPrefsFile.put('/');
									fsPrefsFile.put('/');
									fsPrefsFile.put(' '); 
									memset(szTempBuffer,' ',lNumBytesRead - 3);
									fsPrefsFile.write(szTempBuffer,lNumBytesRead - 3);
								}


								bAltMailNewsOption = TRUE;
								bRet &= TRUE;

							}
						}
						
					}						
				}

				
				fsPrefsFile.close();

				if (TRUE == bAddEudoraAsDefaultEmailer)
				{
					// Write to the prefs.js files
					std::ofstream oPrefsFile(szPrefsFileName, std::ios::in | std::ios::ate);
						
					if (FALSE == bAltMailOption)
					{
						oPrefsFile.write(_T("user_pref(\"mail.use_altmail\", true);\n"),_tcslen(_T("user_pref(\"mail.use_altmail\", true);\n")));							

					}
					if (FALSE == bAltMailNewsOption)
					{
						// If Eudora is default emailer, THEN make sure that we do not handle the Netscape Messenger compose mail
						// At the end of Prefs.js insert this : user_pref("mail.use_altmail_for_news", false);					
						oPrefsFile.write(_T("user_pref(\"mail.use_altmail_for_news\", false);\n"),_tcslen(_T("user_pref(\"mail.use_altmail_for_news\", false);\n")));
					}
					if (FALSE == bAltMailDllOption)
					{
						TCHAR	szBuffer[512];
						_tcscpy(szPostalDLLPath,ExecutableDir);
						_tcscat(szPostalDLLPath,"EudoraNS.dll");

						// Replace the '\' by '/' in the directory name
						for (unsigned int i=0; i < strlen(szPostalDLLPath); i++)
						{
							if (szPostalDLLPath[i] == '\\')
								szPostalDLLPath[i] = '/';
						}
						
						sprintf(szBuffer,_T("user_pref(\"mail.altmail_dll\", \"%s\");\n"),szPostalDLLPath);

						// Write to the prefs.js files																
						oPrefsFile.write(szBuffer,_tcslen(szBuffer));							

					}
					oPrefsFile.close();							
				}					


				// We were able to find Prefs.js file.
				bFoundPrefsPathInUserRegKey = TRUE;
				
			}
		}
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in UpdateNetscapePrefsFile" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in UpdateNetscapePrefsFile" );
		bRet = FALSE;
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in UpdateNetscapePrefsFile" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in UpdateNetscapePrefsFile" );
		bRet = FALSE;
	}

	return bRet;

}


//
// Function to Check if the the Netscape's Prefs.js is updated to configure Eudora as the Default Mailto handler
//
BOOL CheckIfNetscapePrefsFileUpdated()
{

	TCHAR		szTempBuffer[512];
	//long		lNumBytesRead = 0;

	BOOL		bRet = FALSE;
	
	BOOL		bAltMailOption = FALSE;
	BOOL		bAltMailDllOption = FALSE;
	BOOL		bAltMailNewsOption = FALSE;


	// Lets see, even if the Netscape Prefs File is updated, make sure that EudoraNS.dll is indeed present in the ExecutableDir
	// The chances of it not being there are minimal but it can happen when a user for some reason knowingly or by mistake deletes it
	// or Developers who are debugging do not have EudoraNS.dll yet in there Eudora installation directory.
	
	struct _tfinddata_t strFileData;
	TCHAR	szPostalDLLPath[_MAX_PATH + _MAX_FNAME + _MAX_EXT];							
	_tcscpy(szPostalDLLPath,ExecutableDir);
	_tcscat(szPostalDLLPath,"EudoraNS.dll");

	long hFile = _tfindfirst( szPostalDLLPath, &strFileData );
	if ( hFile == -1L )
	{
		// Could not find EudoraNS.dll in the ExecutableDir. It's better we return FALSE .
		// Altho' technically returning FALSE does not do justice with the function's immediate & actual purpose, but what
		// we are looking at is to decide whether to use DDE or Postal API to communicate with Netscape to handle mailto
		// Since EudoraNS.dll is not present we cannot use Postal API regardless of whether prefs file is updated or not & 
		// hence just bail out with a FALSE.

		return FALSE;		
	}

	static	CStringList csPrefsFileList;	
	try
	{
		LocateNetscapePrefsFile(csPrefsFileList);
		POSITION	nPos = NULL;
		nPos = csPrefsFileList.GetHeadPosition();
		while ((! csPrefsFileList.IsEmpty()) && (nPos != NULL))
		{
			CString strName = csPrefsFileList.GetNext(nPos);
			//JJFile	jjPrefsFile(JJFile::BUF_SIZE,FALSE);

			std::fstream fsPrefsFile(strName, std::ios::in | std::ios::out);

			if ( fsPrefsFile.is_open() )
			{
				while ( (fsPrefsFile.getline(szTempBuffer, 510)) )
				{
					if (_tcsstr( _tcslwr(szTempBuffer), _T("user_pref(\"mail.")))
					{
						if ( (FALSE == bAltMailOption) && 
							(_tcsstr(_tcslwr(szTempBuffer), _T("user_pref(\"mail.use_altmail\""))) )
						{
							// Make sure it's user_pref("mail.use_altmail", true);
							if (_tcsstr(_tcslwr(szTempBuffer), _T("true")) )
							{
								bAltMailOption = TRUE;
								bRet = TRUE;									
							}

						}
						else if ( (FALSE == bAltMailDllOption) && 
								  (_tcsstr(_tcslwr(szTempBuffer), _T("user_pref(\"mail.altmail_dll"))) )
						{
							// Make sure it points to our dll
							if (_tcsstr(_tcslwr(szTempBuffer), _T("eudorans.dll")) )
							{
								bAltMailDllOption = TRUE;
								bRet &= TRUE;
							}
						}
						else if ( (FALSE == bAltMailNewsOption) && 
								  (_tcsstr(_tcslwr(szTempBuffer), _T("user_pref(\"mail.use_altmail_for_news\""))) )
						{
							if (_tcsstr(_tcslwr(szTempBuffer), _T("false")) )
							{

								bAltMailNewsOption = TRUE;
								bRet &= TRUE;

							}
						}
						
					}						
				}

				//jjPrefsFile.Close();					
				fsPrefsFile.close();
			}
			
		}
	}
	catch (CMemoryException * /* pMemoryException */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing CMemoryException in CheckIfNetscapePrefsFileUpdated" );
		throw;
	}
	catch (CException * pException)
	{
		// Other MFC exception
		pException->Delete();
		ASSERT( !"Caught CException (not CMemoryException) in CheckIfNetscapePrefsFileUpdated" );
		bRet = FALSE;
	}
	catch (std::bad_alloc & /* exception */)
	{
		// Catastrophic memory exception - rethrow
		ASSERT( !"Rethrowing std::bad_alloc in CheckIfNetscapePrefsFileUpdated" );
		throw;
	}
	catch (std::exception & /* exception */)
	{
		ASSERT( !"Caught std::exception (not std::bad_alloc) in CheckIfNetscapePrefsFileUpdated" );
		bRet = FALSE;
	}

	return bRet;

}
