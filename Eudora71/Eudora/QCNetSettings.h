// QCNetSettings.h
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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


#ifndef _QCNetSettings_h_
#define _QCNetSettings_h_

#define QCMS_BUF_SIZE 256
#define QC_NETWORK_BUF_SIZE 3072

#include "Callbacks.h"
#include <ras.h>
#include "TaskErrorTypes.h"
class CTaskInfoMT;

typedef void (* ReportErrorFnType)(void *, UINT StringID, ...);

class QCNetworkSettings {
public:
	QCNetworkSettings(const char* persona) : m_strPersona(persona), ClosedByPeer(0), m_pRasDialParams(NULL), m_pTaskInfo(NULL)
					{ GrabSettings(); }
	
	~QCNetworkSettings();
	bool DoCaching(){ return m_bDoCaching; }
	bool DoConnectOffline() { return m_bConnectOffline;}
	bool DoAuthenticateKerberos(){ return m_bAuthenticateKerberos; }
	bool DoAuthenticateKerberos4(){ return m_bAuthenticateKerberos4; }
	bool IsDialupConnection() { return m_bDialupConnection;}

	const CString& GetPersona(){ return m_strPersona; }
	const CString& GetRASEntryName(){ return m_strRASEntryName; }
	RASDIALPARAMS *GetRasDialParams(){ return m_pRasDialParams; }
	bool GetSavePasswordFlag(){ return m_bRASSavePassword; }
	bool GetRasPassword();
	
	int GetNetworkBufferSize()  { return m_nNetworkBufferSize; }
	int GetNetworkOpenTimeout() { return m_nNetworkOpenTimeout; }
	int GetNetworkTimeout() { return m_nNetworkTimeout; }
	bool GetNetImmediateClose(){ return m_bNetImmediateClose; }
	
	
	typedef Callback2<const char *, TaskErrorType> ErrorCallback;
	typedef Callback1<const char *> ProgressCallback;

	void SetReportErrorCallback(ErrorCallback func){ ReportError = func; }
	void SetProgressCallback(ProgressCallback func){ Progress = func; }
	void SetCloseCallback(Callback1<int> func){ ClosedByPeer = func; }
	void SetTaskInfo(void * pTaskInfo) { m_pTaskInfo = (CTaskInfoMT*)pTaskInfo;}
	void* GetTaskInfo(){ return m_pTaskInfo;}
	void SetNetworkBufferSize(int size){m_nNetworkBufferSize = size;}

private:
	int GrabSettings();
    CTaskInfoMT *m_pTaskInfo;
	CString m_strPersona;
	bool m_bDoCaching;

	//RAS
	bool m_bDialupConnection;
	RASDIALPARAMS *m_pRasDialParams;
	CString m_strRASEntryName;
	
	bool m_bRASSavePassword;

	//Network
	bool m_bConnectOffline;
	int m_nNetworkBufferSize;
	int m_nNetworkOpenTimeout;
	int m_nNetworkTimeout;

	bool m_bNetImmediateClose;		//IDS_INI_NET_IMMEDIATE_CLOSE
	
	//Authentication
	bool m_bAuthenticateKerberos;
	bool m_bAuthenticateKerberos4;
	

public:
	//To report errors
	//ReportErrorFnType ReportError;
	Callback2<const char *, TaskErrorType> ReportError;
	
	//To report progress 
	Callback1<const char *> Progress;

	//To Report Closes by the server
	Callback1<int> ClosedByPeer;
		
};


#endif