/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					bufferio.cpp
Description:		Buffered Network I/O object
Date:						8/8/97
Version:  			1.0 
Module:					PH.DLL (PH protocol directory service object)
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include <objbase.h>
#include <stdlib.h>
#include <stdio.h>
#include "DebugNewHelpers.h"
#include "bufferio.h"


///////////////////////////////////////////////////////////////////////////////
// CBufferNetIO

CBufferNetIO::CBufferNetIO()
{
	m_pISock 				= NULL;
	m_bConnected 		= 0;
	m_pWritePos 		= m_achWriteBuf;
	m_nBytesToWrite = 0;
	m_nBytesRead		= 0;
	memset(m_achWriteBuf, 0, sizeof(m_achWriteBuf));
	memset(m_achReadBuf, 0, sizeof(m_achReadBuf));
}


CBufferNetIO::~CBufferNetIO()
{
	if (m_pISock)
		m_pISock->Release();
}


HRESULT CBufferNetIO::Create()
{
	return CoCreateInstance(CLSID_ISock, NULL, CLSCTX_INPROC_SERVER, IID_ISock, (LPVOID *)&m_pISock);
}


HRESULT CBufferNetIO::Connect(LPSTR lpszHost, UINT nPort, LPFNNETIOCB pfnCallback, LPVOID pData)
{
	if (!m_pISock || !pfnCallback)
		return S_FALSE;

	m_bConnected = 0;
	m_bBusy = 0;
	HRESULT hResult = m_pISock->Initialize(lpszHost, nPort, ISOCK_FLAG_NOFLAGS);
	if (!FAILED(hResult))	{
		m_pfnCompleteCB = pfnCallback;
		m_pUserData = pData;
		m_bBusy = 1;
		m_pISock->Connect(ISOCKCALLBACK(ConnectCB), (LPVOID)this);
		return S_OK;
	}

	return S_FALSE;
}

void CBufferNetIO::ConnectCB()
{
	m_pISock->GetSocketError(&m_nSocketError);
	m_bConnected = (0 == m_nSocketError);
	Complete(0);
}


BOOL CBufferNetIO::IsConnected()
{
	return m_bConnected;
}

BOOL CBufferNetIO::IsBusy()
{
	return m_bBusy;
}

int CBufferNetIO::GetSocketError()
{
	return m_nSocketError;
}

inline void CopyCRLF(LPSTR p)
	{ *p++ = '\r'; *p++ = '\n'; *p++ = 0; }


int CBufferNetIO::FixLine(LPSTR lpszBuffer, int nLen)
{
	if (lpszBuffer[nLen-1] != '\n'){
		CopyCRLF(lpszBuffer+nLen);
		return strlen(lpszBuffer);
	}
	else if (lpszBuffer[nLen-2] != '\r') {
		CopyCRLF(lpszBuffer+nLen-1);
		return strlen(lpszBuffer);
	}

	return nLen;
}


void CBufferNetIO::Complete(BOOL bError/*=0*/)
{
	m_bBusy = 0;
	m_nSocketError = 0;

	if (bError)
		m_pISock->GetSocketError(&m_nSocketError);

	// call completion function
	if (m_pfnCompleteCB)
		(*m_pfnCompleteCB)(m_pUserData);
}


/* 
	returns:	S_OK 		- completion callback will be called
						S_FALSE - completion callback can not be called
*/
HRESULT CBufferNetIO::WriteLine(LPSTR lpsz, LPFNNETIOCB pfnCompleteCB, LPVOID pData)
{
	int			nNewLen = 0;
	int			nLen;

	if (m_bBusy || !lpsz || !pfnCompleteCB)
		return S_FALSE;

	// get length of string to write
	const nMaxLen = sizeof(m_achWriteBuf)-3;
	nLen = strlen(lpsz);
	if (nLen > nMaxLen)
		return S_FALSE;

	strncpy(m_achWriteBuf, lpsz, nMaxLen);				// leave room for cr/lf/null
	nNewLen 				= FixLine(m_achWriteBuf, nLen);
	m_pWritePos 		= m_achWriteBuf;
	m_nBytesToWrite = nNewLen;
	m_pfnCompleteCB = pfnCompleteCB;
	m_pUserData 		= pData;
	m_bBusy 				= 1;
	
	if (FAILED(m_pISock->Writable(ISOCKCALLBACK(WritableCB), this)))
		Complete(1);

	return S_OK;
}


void CBufferNetIO::WritableCB()
{
	int	nBytesWritten	= 0;
	
	if (FAILED(m_pISock->Write(m_pWritePos, m_nBytesToWrite, &nBytesWritten)))
		Complete(1);
	else {
		m_nBytesToWrite -= nBytesWritten;
		m_pWritePos += nBytesWritten;
		if (m_nBytesToWrite <= 0)
			Complete();
		else
			if (FAILED(m_pISock->Writable(ISOCKCALLBACK(WritableCB), this)))
				Complete(1);
	}
}


// Returns pointer to end line if '\n' found in lpsz, otherwise returns NULL
LPSTR BufferFindEOL(LPSTR lpsz, UINT nMaxLen, UINT *pnLength)
{
	LPSTR 	p;
	UINT		i;

	for (p=lpsz,i=0; i < nMaxLen; i++, p++)
		if (*p == '\n') {
			if (pnLength) 
				*pnLength = i+1;
			return p+1;
		}
	
	return NULL;
}



void CBufferNetIO::GetLine()
{
	LPSTR 	pEnd;
	UINT		nLineLength;
	 
	pEnd = BufferFindEOL(m_achReadBuf, m_nBytesRead, &nLineLength);
	if (pEnd) {
		int nNullTerm = min(m_nReadLineSize, nLineLength);
		strncpy(m_lpszReadLine, m_achReadBuf, nNullTerm);			// copy string into user line buffer
		m_lpszReadLine[nNullTerm-1] = 0;											// make sure user line buffer is null terminated
		// move buffer contents up to front...
		m_nBytesRead -= nLineLength;
		memmove(m_achReadBuf, pEnd, m_nBytesRead);
		// signal completion
		Complete();
	}
	else 
		if (FAILED(m_pISock->Readable(ISOCKCALLBACK(ReadableCB), this)))
			Complete(1);
}


/* 
	returns:	S_OK 		- completion callback will be called
						S_FALSE - completion callback can not be called
*/
HRESULT CBufferNetIO::ReadLine(LPSTR lpszBuffer, UINT nBufferSize, LPFNNETIOCB pfnCompleteCB, LPVOID pData)
{
	if (m_bBusy || !lpszBuffer || !nBufferSize || !pfnCompleteCB)
		return S_FALSE;

	m_lpszReadLine 		= lpszBuffer;
	m_nReadLineSize 	= nBufferSize;
	m_pfnCompleteCB 	= pfnCompleteCB;
	m_pUserData 			= pData;
	m_lpszReadLine[0] = 0;							// NULL terminate line to start
	
	GetLine();

	return S_OK;
}


void CBufferNetIO::ReadableCB()
{
	int nBytesRead = 0;

	if (FAILED(m_pISock->Read(m_achReadBuf+m_nBytesRead, sizeof(m_achReadBuf)-m_nBytesRead, &nBytesRead)))
		Complete(1);
	else {
		m_nBytesRead += nBytesRead;
		if (nBytesRead)
			GetLine();
		else
			Complete();
	}
}





