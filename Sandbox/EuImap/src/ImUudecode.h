// ImUuDECODE.H
//
// Routines for decoding uuencoded data (generic version!!!)

#ifndef _IMUUDECODE_H_
#define _IMUUDECODE_H_

#include "Decode.h"

// Forward declarations

class JJFileMT;


// Data Types

typedef enum
{
	NotAb, AbWrite, AbDone
} AbStates;

class ImapDecodeUU : public Decoder
{
public:
	ImapDecodeUU();
	~ImapDecodeUU();
	int Decode(const char* In, LONG InLen, char* Out, LONG& OutLen);
	int Init();

	// Access to setting internal directives and data:
	//
	void SetMustOutputFile (BOOL bValue)
		{ m_bMustWriteOutput = bValue; }
	void SetMailboxDirectory (LPCSTR pStr)
		{ m_szMailboxDir = pStr; }

	LPCTSTR GetEmbeddedFilename () { return m_szEmbeddedFilename; }

protected:
	AbStates m_State;

	char m_Name[255];

	BOOL IsAbLine(const char* text, LONG size);
	int UURightLength(const char* text, LONG size, BOOL Strict = TRUE);

	AbStates UULine(const char* text, LONG size, char *Out, LONG& Outlen);

	JJFileMT* CreateAttachFilefromEmbeddedName ();

	// If set, we must creat an attach file and do the writing ourselves.
	// Initialize to FALSE.
	//
	BOOL m_bMustWriteOutput;

	// Caller must set this directory under which there's the Attach directory.
	CString		m_szMailboxDir;
	JJFileMT*	m_pATFile;
	CString		m_szEmbeddedFilename;
};

#endif // #ifndef _IMUUDECODE_H_
