// SENDMAIL.H
//
// Include file for the sendmail routines.
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

// Function Prototypes

class CCompMessageDoc;
class Network;
extern Network* NetConnection;

static BOOL SMTPConnected = FALSE;
static BOOL	DeleteNetObj;
static BOOL	UsingPOPSend;
static CString CurSMTPServer;


int ComposeDate(char* DateBuf, unsigned long GMTTime, int TimeZoneMinutes = -1, CSummary* SumToSetDate = NULL);
inline int ComposeDate(char* DateBuf, CSummary* SumToSetDate)
	{ return ComposeDate(DateBuf, SumToSetDate->m_Seconds, SumToSetDate->m_TimeZoneMinutes, SumToSetDate); }
int StartSMTP(const char *server);
int FinishSMTP();
int EndSMTP(BOOL abort);
int DoSMTPIntro();
int SendMessage(CCompMessageDoc* msg);
int SendRawMIME(const char *FileName);
char* GetSignature(CCompMessageDoc* CompDoc, BOOL ConvertRichToHTML);

HRESULT SpoolOne(CSummary* Sum,HRESULT InResult);
HRESULT SendQueuedMessages2(int WhichToSend /*= QS_READY_TO_BE_SENT*/,
									BOOL bMultiPersona = TRUE,
									bool bSend =TRUE);

void PostProcessOutgoingMessages(void *pv, bool bEnteringLastTime);


#ifndef _QCSMTPMessage_h_
#define _QCSMTPMessage_h_

class CFileIO;

class QCSMTPMessage 
{
public:
	QCSMTPMessage(CCompMessageDoc*);

	HRESULT Start(const char* filename);
	HRESULT End();
	HRESULT WriteHash();
	HRESULT WriteEnvelope();
	HRESULT WriteBody();
	HRESULT RedirectOutput(const char* filename);
	HRESULT EndRedirect();
	HRESULT WriteHeaders();
	HRESULT WriteRawMIME(const char *FileName);

private:
	Network* m_oldNetConnection;
	CFileIO* m_fileIO;
};

#endif

