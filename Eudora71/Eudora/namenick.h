// NAMENICK.H
//
// Code for handling the Nickname naming dialog, and Make Nickname
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


#include "helpxdlg.h"


/////////////////////////////////////////////////////////////////////////////
// CNameNickDialog dialog

class CNicknameFile;
class CNickname;

class CNameNickDialog : public CHelpxDlg
{
// Construction
public:
	CNameNickDialog(UINT Type,
					CNicknameFile* pSelectedNicknameFile = NULL,
					CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CNameNickDialog)
	enum { IDD = IDD_NICKNAME_NAME };
	CComboBox	m_Filename;
	CString	m_NewName;
	CString	m_NewFullName;
	CString	m_NewFirstName;
	CString	m_NewLastName;
	CString	m_NewAddress;
	BOOL	m_PutOnRecipientList;
	BOOL	m_PutOnBPList;
	BOOL	m_CreateNicknameFile;
	//}}AFX_DATA
	
	UINT			m_Type;
	CNicknameFile*	m_pNicknameFile;
	BOOL			m_MakeNickAdd;

// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void OnOK();
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    
	// Generated message map functions
	//{{AFX_MSG(CNameNickDialog)
	afx_msg BOOL OnInitDialog();
	afx_msg void OnMakeFileClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CNicknameFile*	m_pSelectedNicknameFile;
};


/////////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg dialog

class CChooseNicknameFileDlg : public CHelpxDlg
{
// Construction
public:
	CChooseNicknameFileDlg(CNicknameFile* pSourceNicknameFile = NULL,
							CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CNameNickDialog)
	enum { IDD = IDD_NICKNAME_CHOOSE_FILE };
	CListBox	m_FileListBox;
	//}}AFX_DATA
	
	//
	// Caller uses this to determine which nickname file was chosen
	// by the user.
	//
	CNicknameFile* GetTargetNicknameFile() const
		{ return m_pTargetNicknameFile; }

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnOK();
    
	// Generated message map functions
	//{{AFX_MSG(CChooseNicknameFileDlg)
	afx_msg BOOL OnInitDialog();
	afx_msg void OnFileListBoxDblClk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	//
	// The 'pSourceNicknameFile' gives us a hint which nickname file
	// contains the singly-selected "source" nickname.  That way, we
	// can inhibit the display of the source nickname file -- i.e., it
	// doesn't make sense to list the source nickname file as a
	// choosable target.
	//
	CNicknameFile*	m_pSourceNicknameFile;		// single-selected source nickname file
	CNicknameFile*	m_pTargetNicknameFile;		// target nickname file chosen by user
};

// Function Prototypes

void MakeAutoNickname(CString& address,
							  CString& strAutoFullName,
							  CString& strAutoFirstName,
							  CString& strAutoLastName);
BOOL MakeNickname();
BOOL AddNickname(CNicknameFile *pNicknameFile,
				 CString &strNickname,
				 CString &strFullName,
				 CString &strFirstName,
				 CString &strLastName,
				 CString &strAddress,
				 BOOL bPutOnRecipientList = FALSE,
				 BOOL bPutOnBPList = FALSE,
 				 bool bWriteNotes = true,
				 BOOL bNicknameWndIsActive = FALSE);

BOOL NicknameFileExists(const char* nickFilename);
BOOL CreateNewNicknameFile(const char* newFilename);
