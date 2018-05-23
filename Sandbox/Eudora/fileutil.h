// FILEUTIL.H
//
// File utility routines
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
	virtual HRESULT Rename(const char* NewName, BOOL DeleteFirst = TRUE);
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
extern CString ExecutableDir;
extern CString TempDir;


// Function Prototypes
BOOL GetDirs(char* CmdLine);
BOOL CopyAttachmentFile(
	CString& srcPathname,			//(io) on input, pathname to source
									// file; on output, pathname to
									// file in Eudora attachment directory
	const CString& destFilename);	//(i) optional filename for dest file

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
	CL_MAILTO
} COMMANDLINE_TYPE;

COMMANDLINE_TYPE GetCommandType(char* CmdLine);


#endif
