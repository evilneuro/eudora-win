/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					bufferio.h
Description:		Buffered Network I/O object
Date:						8/9/97
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
#ifndef __BUFFERIO_H__
#define __BUFFERIO_H__

#include <isock.h>


///////////////////////////////////////////////////////////////////////////////
// CBufferNetIO

const WRITEBUFSIZE = 1024;		// 1K write buffer
const READBUFSIZE = 8192;			// 8K read buffer

class CBufferNetIO;
typedef CBufferNetIO * LPNETIO;

typedef void (*LPFNNETIOCB)(LPVOID pData);

// this macro converts FuncName to xFuncName (the static callback wrapper function)
#define CPPCALLBACK(name)			(x##name)
#define ISOCKCALLBACK(name) 	(ISOCKCB)CPPCALLBACK(name)
#define DEFCPPCALLBACK(classname, name)	\
	static void CPPCALLBACK(name)(classname * pThis) {pThis->name();}	void name();
/* Usage of DEFCPPCALLBACK:

		// Define callback to non-static member function of form: void CClass::fn(void);
		DEFCPPCALLBACK(CMyClass, MyCallbackFunc)
		
		// The above expands to:
		static void 	xMyCallbackFunc(CMyClass * pThis) { pThis->MyCallbackFunc(); }
		void 					MyCallbackFunc();
		
*/
#define DEFCUSTOMCALLBACK(name)	DEFCPPCALLBACK(CBufferNetIO, name)


class CBufferNetIO
{
public:
	CBufferNetIO();
	~CBufferNetIO();

public:
	HRESULT				Create();
	HRESULT				Connect(LPSTR lpszHost, UINT nPort, LPFNNETIOCB pfnCallback, LPVOID pData);
	HRESULT 			ReadLine(LPSTR lpszBuffer, UINT nBufferSize, LPFNNETIOCB pfnCompleteCB, LPVOID pData);
	HRESULT 			WriteLine(LPSTR lpsz, LPFNNETIOCB pfnCompleteCB, LPVOID pData);
	BOOL					IsConnected();
	BOOL					IsBusy();
  int 					GetSocketError();

protected:
  void 					GetLine();
	void 					Complete(BOOL bError=0);
	int 					FixLine(LPSTR lpszBuffer, int nLen);

	DEFCUSTOMCALLBACK(ConnectCB)
	DEFCUSTOMCALLBACK(WritableCB)
	DEFCUSTOMCALLBACK(ReadableCB)

protected:
	LPISOCK				m_pISock;
	BOOL					m_bConnected;
	BOOL					m_bBusy;
	LPSTR					m_pWritePos;
	UINT					m_nBytesToWrite;
	UINT					m_nBytesRead;
	int						m_nSocketError;
	UINT					m_nReadLineSize;
	LPSTR					m_lpszReadLine;
	LPFNNETIOCB		m_pfnCompleteCB;
	LPVOID				m_pUserData;

	char					m_achWriteBuf[WRITEBUFSIZE];
	char					m_achReadBuf[READBUFSIZE];
};



#endif // __BUFFERIO_H__



