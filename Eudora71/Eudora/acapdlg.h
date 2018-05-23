// AcapDlg.h
//
// Copyright (c) 1995-2000 by QUALCOMM, Incorporated
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


static BOOL	DeleteNetObj;
static BOOL Connected;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
typedef struct
{
	int nStringID;
	CString strSetting;

}SEARCH_STRING;

class CAcapSettings
{
// Construction
public:
	CAcapSettings();	// standard constructor
	~CAcapSettings();

	CString	m_strUserID;
	CString	m_strServerName;
	CString	m_strPassword;
	char *m_szStamp;
	//}}AFX_DATA


// Implementation
	public:

		enum Authentication
		{
 
            AUTH_XCRAM,
            AUTH_CRAM,
			AUTH_APOP,
			AUTH_PLAIN,
			AUTH_NONE
		} m_Authentication;

		CObList *m_pStringIDList;
		CObArray *m_pPositions;
	public:
	BOOL RetrieveSettings(const char* strServer, const char* strUserID, const char* strPass);

	protected:
		int ConnectAcapServer(void);
		int GetGreeting(void);
		int ConfigureClient(void);
		int AuthenticateAcap(void);
		int LoginAcap(void);
		int LogoutAcap(void);
		void ReceiveSearch(char* );
		void SetFields(int nID, char * strSetting);
		int RetrieveID(CString strSetting);
		void LoadStrings();
		int AuthenticatePlainAcap(void);
		//int AuthenticateAPOPAcap();
        int AuthenticateAPOPCRAMAcap(int nType);
		CString GetAcapServer(CString strHostName);
		BOOL CloseConnection();
		
};
//{{AFX_INSERT_LOCATION}}

