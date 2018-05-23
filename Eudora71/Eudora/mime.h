// MIME.H
//
// Routines for MIME
//
// Copyright (c) 1993-2000 by QUALCOMM, Incorporated
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

#ifndef _MIME_H_
#define _MIME_H_

#include "MIMEReader.h"

class LineReader;
class JJFile;
class HeaderDesc;
class Decoder;
class CTLAddress;



class CTranslator;
/*
 * MIME converter state structure
 */
class MIMEState : public CObject
{
public:
	MIMEState(LineReader* lr, int depth,HeaderDesc* hd = NULL);
	~MIMEState();

	int Init(HeaderDesc* hd,int depth);

	void SetLineReader(LineReader* lr) {m_LineReader = lr;}
	LineReader* GetLineReader() {return m_LineReader;}

	BoundaryType IsBoundaryLine(const char* buf) const;
	BoundaryType HuntBoundary(char* buf, LONG bSize);
	int WriteBoundary(BoundaryType boundaryType) const;

	HeaderDesc*	m_hd;
	Decoder*	m_Coder;
	MIMEReader*	m_Reader;
	LineReader*	m_LineReader;
	
	BOOL IsGrokked();
	CString		m_boundary;

	CTranslator* m_Translator;
};


// this class is used to keep track of MHTML embedded
// objects that are part of a multipart/related
// all non text/xxx parts are stored in attachment files
// the filename and content-id content-base/content-location
// are cached up via a list of these elements
// if a text/html part is found while parsing the multipart/related
// then the list of attachments is added to the message as
// "Embedded Content" lines, else the list of attachments is
// added as "Attachment Converted" lines.
class EmbeddedElement : public CObject
{
public:
	EmbeddedElement();

	void Init( const char* path, DWORD MRI, const char* CID, const char* CB, const char* CL );

	bool WasReferenced() const { return m_bWasReferenced; }
	bool IsEmoticon() const { return m_bIsEmoticon; }
	CString & GetPath() { return m_Path; }
	CString & GetCID() { return m_CID; }
	DWORD GetMRI() { return m_MRI;}
	DWORD GetCIDHash() { return m_CIDHash; }
	DWORD GetCLHash() { return m_CLHash; }
	DWORD GetMRI_CB_CL_Hash() { return m_MRI_CB_CL_Hash; }

	void SetWasReferenced(bool in_bWasReferenced) { m_bWasReferenced = in_bWasReferenced; }
	void SetIsEmoticon(bool in_bIsEmoticon) { m_bIsEmoticon = in_bIsEmoticon; }
	void SetPath( const char* Path ) { m_Path = Path; }
	void SetCID( const char* CID );
	void SetMRI( DWORD MRI ) { m_MRI = MRI;}
	void SetCIDHash( DWORD CIDHash ) { m_CIDHash = CIDHash; }
	void SetCLHash( DWORD CLHash ) { m_CLHash = CLHash; }
	void SetMRI_CB_CL_Hash( DWORD Hash ) { m_MRI_CB_CL_Hash = Hash; }

private:
	bool m_bWasReferenced;
	bool m_bIsEmoticon;
	CString m_Path;		// where the content was stored
	CString m_CID;		// for comp message stuff

	DWORD m_MRI;		// multipart/related id (0-indexed order it appears in the message)
	DWORD m_CIDHash;
	DWORD m_CLHash;
	DWORD m_MRI_CB_CL_Hash;
};

int SaveEmbeddedElement( const char * filename );
int WriteEmbeddedElements( BOOL isMHTML );
void FlushEmbeddedElements();



// Function Prototypes

void mimeInit();
CTLAddress* GetEMSHeaders(long diskStart, JJFileMT * in_mbox = NULL);
void HexToString(char* dest, const char* src, int Num = 4);
JJFile* OpenAttachFile(LPTSTR Name, BOOL bAllowRead = FALSE);
JJFile* OpenEmbeddedFile(LPTSTR Name);
int WriteAttachNote(JJFile* AttachFile, UINT labelStrID, LPCTSTR params = NULL);
Decoder* FindMIMEDecoder(LPCTSTR encoding, BOOL AllocateCoder);
int FindMIMECharset(LPCTSTR charSet);
void DeMacifyName32(LPTSTR Name);
char * GetMessageSeparatorLine();
HRESULT PutWithEscapedFileMarkers(const char* pBuffer = NULL, long lNumBytesToWrite = -1L, bool bIsHTML = false);

#define MAX_MIME_DEPTH	100	// don't deal with MIME nested more deeply than this

#endif // #ifndef _MIME_H_
