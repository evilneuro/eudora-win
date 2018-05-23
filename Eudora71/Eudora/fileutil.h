// FILEUTIL.H
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

#ifndef _FILEUTIL_H_
#define _FILEUTIL_H_

#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <QCUtils.h>

#include "cursor.h"
#include "Callbacks.h"


class JJFile : public JJFileMT
{
public:
	JJFile(char* buf, int size, BOOL bDisplayErrors = TRUE);
	JJFile(int _size = BUF_SIZE, BOOL bDisplayErrors = TRUE);
	virtual ~JJFile();

	virtual HRESULT Open(const char* filename, int mode);
	virtual HRESULT Flush();
	virtual HRESULT Seek(long lOffset, int nMode = SEEK_SET, long* plNewOffset = NULL);
	
	void DisplayErrors(BOOL bDisplayErrors){ m_bDisplayErrors = bDisplayErrors; }

	virtual HRESULT Delete();
	virtual HRESULT Rename(const char* NewName, BOOL bReplace = TRUE);
	virtual HRESULT JJBlockMove(long Start, long Length, JJFileMT* Target);
	virtual HRESULT GetLine(char* pBuffer, long lBufferSize, long* plNumBytesRead = NULL);
	virtual HRESULT Read(char* pBuffer, long size);

protected:
	virtual HRESULT Write_(const char* pBuffer, long lNumBytesToWrite);

	void ReportError(UINT StringID, ...);
	void DisplayErrorDialog_(HRESULT hr, const char* pszExtra = NULL);
	void SetErrorCallback(Callback1<const char *> functor){ ErrorCallback = functor; } 

private:
	BOOL m_bDisplayErrors;	// ErrorMessage Flag
	//CCursor cur;	// Have wait cursor on during entire lifetime of this object
	Callback1<const char *> ErrorCallback;
};


// Data Items
extern int EudoraDirLen;
extern CString EudoraDir;
extern CString EudoraDirNoBackslash;
extern CString ExecutableDir;
extern CString TempDir;


// Function Prototypes
BOOL GetDirs(char* CmdLine);
BOOL CopyAttachmentFile(
	CString& srcPathname,			//(io) on input, pathname to source
									// file; on output, pathname to
									// file in Eudora attachment directory
	const char* destFilename);		//(i) optional filename for dest file

// enums
typedef enum
{
	CL_EMPTY,
	CL_DIR,
	CL_INI,
	CL_DIR_INI,
	CL_MESSAGE,
	CL_ATTACH,
	CL_STATIONERY,
	CL_URL,
	CL_MAILBOX
} COMMANDLINE_TYPE;

COMMANDLINE_TYPE GetCommandType(LPCTSTR CmdLine);

void	GetRegInfoFilePath(CString & szRegInfoFilePath);
bool	GetRegInfoFileAlternatePath(CString & szRegInfoFilePath);

bool
ChangeFileNameExtension(
	char *					in_szFileName,
	const char *			in_szNewExtension,
	long					in_nFileNameBufferSize);

HRESULT CascadeBackupFile(const char *in_szFileName, const int nBackups);

bool ConvertToLongPathName(CString &csPathName);

#endif
