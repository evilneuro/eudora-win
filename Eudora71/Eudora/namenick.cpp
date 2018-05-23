// NAMENICK.CPP
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


#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>
#include <ctype.h>

#include <QCUtils.h>

#include "resource.h"
#include "rs.h"
#include "cursor.h"
#include "fileutil.h"
#include "doc.h"
#include "nickdoc.h"
#include "3dformv.h"
#include "namenick.h"
#include "guiutils.h"
#include "tocview.h"
#include "msgdoc.h"
#include "compmsgd.h"
#include "address.h"
#include "msgutils.h"
#include "mainfrm.h"

//#include "urledit.h"	// for LOOKUP.H
//#include "lookup.h"

//#include "LookupWazooWnd.h"
#include "DSWazooWndNewIface.h"
#include "NicknamesWazooWnd.h"
#include "PgReadMsgPreview.h"
#include "TridentPreviewView.h"
#include "TocFrame.h"

#include "QCSharewareManager.h"

#include "DebugNewHelpers.h"


#define NUM_QUALIFIERS		11
char		*qualifierList[NUM_QUALIFIERS] = {"Jr.","Jr","Sr.","Sr","I","II","III","IV","Esq.","Esq","PhD"};

#define NUM_HONORIFICS		11
char		*honorificList[NUM_HONORIFICS] = {"Dr.","Dr","Mr.","Mr","Mrs.","Mrs","Ms.","Ms","Miss","Mme","M"};


/////////////////////////////////////////////////////////////////////////////
// CNameNickDialog dialog


CNameNickDialog::CNameNickDialog(UINT Type, CNicknameFile* pSelectedNicknameFile /*= NULL*/, CWnd* pParent /*= NULL*/)
	: CHelpxDlg(CNameNickDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameNickDialog)
	//}}AFX_DATA_INIT
	m_NewName = "";
	m_PutOnRecipientList = FALSE;
	m_PutOnBPList = FALSE;
	m_CreateNicknameFile = FALSE;
	m_Type = Type;
	m_pNicknameFile = NULL;
	m_MakeNickAdd = FALSE;

	//
	// Caller is giving us a hint which nickname file item is selected
	// in the view.
	//
	m_pSelectedNicknameFile = pSelectedNicknameFile;	// can be NULL
}

BOOL CNameNickDialog::OnInitDialog()
{
	CHelpxDlg::OnInitDialog();
	
	ASSERT(NULL == m_pNicknameFile);
	ASSERT(g_Nicknames);

//FORNOW	if (! LoadNicknames())
//FORNOW	{
//FORNOW		OnCancel();
//FORNOW		return (FALSE);
//FORNOW	}
	
	// Set the title of the dialog depending on what action is being done
	UINT TitleID;
	switch (m_Type)
	{
	case IDC_NEW:					
		TitleID = IDS_NICK_NEW_TITLE;		
		break;
	case ID_SPECIAL_MAKENICKNAME:
		TitleID = IDS_NICK_MAKE_TITLE;
		break;
	case IDCANCEL:
		TitleID = IDS_NICK_NEW_TITLE;		
		if (NULL == m_pSelectedNicknameFile)
		{
			ASSERT(0);		// caller must specify a target nickname file
			return FALSE;
		}
		break;
	default:
		ASSERT(FALSE);
		return (FALSE);
	}
	SetWindowText(CRString(TitleID));
	
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// New and Make Nickname get list of nickname files
		if (m_Type == IDC_NEW || m_Type == ID_SPECIAL_MAKENICKNAME)
		{
			int select_idx = 0;
			for (CNicknameFile* NickFile = g_Nicknames->GetFirstNicknameFile();
				 NickFile != NULL;
				 NickFile = g_Nicknames->GetNextNicknameFile())
			{
				//
				// Don't list read-only or invisible(peanut) nickname files as candidates for
				// new nicknames.
				//
				if ((NickFile->m_ReadOnly == FALSE) && (NickFile->m_Invisible == FALSE))
				{
					int idx = m_Filename.AddString(NickFile->m_Name);

					if (m_pSelectedNicknameFile == NickFile)
						select_idx = idx;
				}
			}

			m_Filename.SetCurSel(select_idx);
			m_Filename.ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->ShowWindow(SW_SHOWNA);
		}
		else if (m_Type == IDCANCEL)
		{
			//
			// Add the one and only nickname file as the target in the combo box.
			//
			m_Filename.AddString(m_pSelectedNicknameFile->m_Name);
			m_Filename.SetCurSel(0);
			m_Filename.ShowWindow(SW_SHOWNA);
			m_Filename.EnableWindow(FALSE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_FULLNAME_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_FULLNAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_FIRSTNAME_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_FIRSTNAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_LASTNAME_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_LASTNAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADDRESS_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_ADDRESS)->EnableWindow(FALSE);
			GetDlgItem(IDC_SWAPNAMES)->EnableWindow(FALSE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CREATE_NICKNAME_FILE)->EnableWindow(FALSE);
			GetDlgItem(IDC_NICK_FILENAME_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECIPIENT_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BOSS_PROTECTOR_LIST)->EnableWindow(FALSE);
		}

		if (m_Type == IDC_NEW)
		{
			// Unhide the "make it a file" checkbox.
			CButton* p_checkbox = (CButton *) GetDlgItem(IDC_CREATE_NICKNAME_FILE);
			ASSERT(p_checkbox != NULL);
			p_checkbox->ShowWindow(SW_SHOWNA);
		}
	}
	//make the BossWatch checkbox visible only in paid mode
	if (!UsingPaidFeatureSet()) 
		GetDlgItem(IDC_BOSS_PROTECTOR_LIST)->ShowWindow(FALSE);

	CenterWindow();
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}

void CNameNickDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameNickDialog)
	DDX_Control(pDX, IDC_NICK_FILENAME_COMBO, m_Filename);
	DDX_Text(pDX, IDC_NEW_NAME, m_NewName);
	DDV_MaxChars(pDX, m_NewName, CNickname::MAX_NAME_LENGTH);
	DDX_Text(pDX, IDC_NEW_FULLNAME, m_NewFullName);
	DDX_Text(pDX, IDC_NEW_FIRSTNAME, m_NewFirstName);
	DDX_Text(pDX, IDC_NEW_LASTNAME, m_NewLastName);
	DDX_Text(pDX, IDC_NEW_ADDRESS, m_NewAddress);
	DDX_Check(pDX, IDC_RECIPIENT_LIST, m_PutOnRecipientList);
	DDX_Check(pDX, IDC_BOSS_PROTECTOR_LIST, m_PutOnBPList);
	DDX_Check(pDX, IDC_CREATE_NICKNAME_FILE, m_CreateNicknameFile);
	//}}AFX_DATA_MAP
}


////////////////////////////////////////////////////////////////////////
// OnOK [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CNameNickDialog::OnOK()
{
	if (!UpdateData(TRUE) || m_NewName.IsEmpty())
		return;
	
	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		//
		// Are we just creating a new nickname file?  If so, then do some
		// error checking, then pass it off to the nickname file creation
		// routine.
		//
		if ((IDC_NEW == m_Type) && m_CreateNicknameFile)
		{
			// Check for illegal characters in base part of filename
			int idx = m_NewName.FindOneOf("/\\:*?\"<>|");
			if (idx >= 0)
			{
				ErrorDialog(IDS_NICKFILE_BAD_CHARACTER, char(m_NewName[idx]));
				return;
			}
			else if (NicknameFileExists(m_NewName + CRString(IDS_NICK_FILE_EXTENSION)))
			{
				ErrorDialog(IDS_ERR_NICKFILE_EXISTS, (const char*) (m_NewName + CRString(IDS_NICK_FILE_EXTENSION)));
				return;
			}
			else if (! CreateNewNicknameFile(m_NewName + CRString(IDS_NICK_FILE_EXTENSION)))
				return;

			CHelpxDlg::OnOK();		// dismiss dialog
			return;
		}
	}

	CNickname nickname(m_NewName);
	if (! nickname.HasValidName())
	{
		ErrorDialog(IDS_ERR_NICK_INVALID_CHAR);
		return;
	}

	//
	// Determine which target Nickname file we're dealing with, then
	// initialize 'm_pNicknameFile' to point to the target Nickname file.
	//
	switch (m_Type)
	{
	case IDC_NEW:
	case ID_SPECIAL_MAKENICKNAME:
		{
			//
			// Figure out the Nickname file index from the combo box index.
			// Non-trivial because the combo box listing doesn't include
			// readonly nickname files.
			//
			ASSERT(g_Nicknames != NULL);
			int combo_idx = m_Filename.GetCurSel();
			for (m_pNicknameFile = g_Nicknames->GetFirstNicknameFile();
			     m_pNicknameFile != NULL;
				 m_pNicknameFile = g_Nicknames->GetNextNicknameFile())
			{
				if (m_pNicknameFile->m_ReadOnly == FALSE && m_pNicknameFile->m_Invisible == FALSE && combo_idx-- <= 0)
					break;
			}
		}
		break;
	case IDCANCEL:
		m_pNicknameFile = m_pSelectedNicknameFile;
		break;
	default:
		ASSERT(0);
		return;
	}
	
	if (! m_pNicknameFile)
	{
		ASSERT(0);
		return;
	}

	//
	// Depending on which "type" of make nickname operation we're
	// doing, either check for a duplicate nickname in the target file
	// or not.  If we do check for and detect a duplicate, then
	// display an error message, then bail without dismissing the
	// dialog.
	//
	switch (m_Type)
	{
	case IDC_NEW:
	case IDCANCEL:
		if (m_pNicknameFile->FindNickname(m_NewName))
		{
			m_pNicknameFile = NULL;
			ErrorDialog(IDS_ERR_NICK_NAME_USED);
			return;
		}
		break;
	case ID_SPECIAL_MAKENICKNAME:
		break;
	default:
		ASSERT(0);
		return;
	}

	CHelpxDlg::OnOK();
}


////////////////////////////////////////////////////////////////////////
// OnCommand [protected]
//
// Handles other commands (in this case only the Swap button.
////////////////////////////////////////////////////////////////////////
BOOL CNameNickDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam == IDC_SWAPNAMES))
	{
		UpdateData(TRUE);
		CString		temp = m_NewFirstName;
		m_NewFirstName = m_NewLastName;
		m_NewLastName = temp;
		UpdateData(FALSE);
		return TRUE;
	}
	return CHelpxDlg::OnCommand(wParam, lParam);
}


////////////////////////////////////////////////////////////////////////
// OnMakeFileClicked [protected]
//
// Handles state changes for "make it a file" check box.
////////////////////////////////////////////////////////////////////////
void CNameNickDialog::OnMakeFileClicked()
{

	// Shareware: Reduced feature mode only allows one nickname file
	if (UsingFullFeatureSet())
	{
		// FULL-FEATURE

		// Retrieve "make it a file" button state.
		if (! UpdateData(TRUE))
			return;
		
		if (m_CreateNicknameFile)
		{
			//
			// Checked, so disable nickname "file selection" combo box,
			// its corresponding label, and the "put on recipient list"
			// checkbox.
			//
			GetDlgItem(IDC_NICK_FILENAME_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECIPIENT_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BOSS_PROTECTOR_LIST)->EnableWindow(FALSE);
		}
		else
		{
			//
			// Unchecked, so enable nickname "file selection" combo box,
			// its corresponding label, and the "put on recipient list"
			// checkbox.
			//
			GetDlgItem(IDC_NICK_FILENAME_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_NICK_FILE_PROMPT)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECIPIENT_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_BOSS_PROTECTOR_LIST)->EnableWindow(TRUE);
		}
	}
	else
	{
		// REDUCED FEATURE
		ASSERT(0); // this should only occur in full feature mode
	}
}


BEGIN_MESSAGE_MAP(CNameNickDialog, CHelpxDlg)
	//{{AFX_MSG_MAP(CNameNickDialog)
	ON_BN_CLICKED(IDC_CREATE_NICKNAME_FILE, OnMakeFileClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg dialog

////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg [public, constructor]
//
// The 'pSourceNicknameFile' gives us a hint which nickname file
// contains the singly-selected "source" nickname.  That way, we can
// inhibit the display of the source nickname file -- i.e., it doesn't
// make sense to list the source nickname file as a choosable target.
////////////////////////////////////////////////////////////////////////
CChooseNicknameFileDlg::CChooseNicknameFileDlg
(
	CNicknameFile* pSourceNicknameFile /*=NULL*/, 
	CWnd* pParent /*=NULL*/
) :
	CHelpxDlg(CChooseNicknameFileDlg::IDD, pParent),
	m_pSourceNicknameFile(pSourceNicknameFile),			// can be NULL
	m_pTargetNicknameFile(NULL)
{
	//{{AFX_DATA_INIT(CChooseNicknameFileDlg)
	//}}AFX_DATA_INIT
}

////////////////////////////////////////////////////////////////////////
// OnInitDialog [protected]
//
////////////////////////////////////////////////////////////////////////
BOOL CChooseNicknameFileDlg::OnInitDialog()
{
	// Shareware: Reduced feature mode only allows one nickname file, so no CChooseNicknameFileDlg
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		ASSERT(0);
		return (TRUE);
	}

	CHelpxDlg::OnInitDialog();
	
//FORNOW	if (! LoadNicknames())
//FORNOW	{
//FORNOW		OnCancel();
//FORNOW		return (FALSE);
//FORNOW	}
	
	//
	// Populate list box with all writable nickname files.
	//
	ASSERT(g_Nicknames);
	for (CNicknameFile* p_nickfile = g_Nicknames->GetFirstNicknameFile();
	     p_nickfile != NULL;
		 p_nickfile = g_Nicknames->GetNextNicknameFile())
	{
		//
		// Don't add read-only nickname files and don't add the source
		// nickname file which is already selected.
		//
		if (p_nickfile->m_ReadOnly == FALSE && p_nickfile->m_Invisible == FALSE &&
			m_pSourceNicknameFile != p_nickfile)
		{
			m_FileListBox.AddString(p_nickfile->m_Name);
		}
	}

	m_FileListBox.SetCurSel(0);
	
	CenterWindow();
	
	return (TRUE);  // return TRUE  unless you set the focus to a control
}


void CChooseNicknameFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpxDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseNicknameFileDlg)
	DDX_Control(pDX, IDC_LIST_NICKNAME_FILE, m_FileListBox);
	//}}AFX_DATA_MAP
}

////////////////////////////////////////////////////////////////////////
// OnOK [protected, virtual]
//
////////////////////////////////////////////////////////////////////////
void CChooseNicknameFileDlg::OnOK()
{
	if (! UpdateData(TRUE))
		return;
	
	//
	// Determine which Nickname file we're dealing with.  Set
	// 'm_pTargetNicknameFile' here.  Open up a local scope to define
	// some local variables.
	//
	{
		//
		// Figure out the Nickname file from the list box index.
		// Non-trivial because the list box doesn't include readonly
		// nickname files.
		//
		ASSERT(g_Nicknames != NULL);
		int sel_idx = m_FileListBox.GetCurSel();
		for (m_pTargetNicknameFile = g_Nicknames->GetFirstNicknameFile();
			 m_pTargetNicknameFile != NULL;
			 m_pTargetNicknameFile = g_Nicknames->GetNextNicknameFile())
		{
			if (m_pTargetNicknameFile->m_ReadOnly == FALSE && m_pTargetNicknameFile->m_Invisible == FALSE &&
			    m_pTargetNicknameFile != m_pSourceNicknameFile &&
			    sel_idx-- <= 0)
				break;
		}
	}
	
	if (! m_pTargetNicknameFile)
	{
		// nothing selected?
		ASSERT(0);
		return;
	}

	CHelpxDlg::OnOK();
}

BEGIN_MESSAGE_MAP(CChooseNicknameFileDlg, CHelpxDlg)
	//{{AFX_MSG_MAP(CChooseNicknameFileDlg)
	ON_LBN_DBLCLK(IDC_LIST_NICKNAME_FILE, OnFileListBoxDblClk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChooseNicknameFileDlg message handlers


////////////////////////////////////////////////////////////////////////
// OnFileListBoxDblClk [protected]
//
// Shorthand for clicking the OK button.
//
////////////////////////////////////////////////////////////////////////
void CChooseNicknameFileDlg::OnFileListBoxDblClk()
{
	OnOK();
}


static void MergeAddresses(char*& Addresses, const char* From = NULL, const char* To = NULL,
	const char* Cc = NULL, const char* Bcc = NULL)
{
    const char Newline[] = "\r\n";
	int NewLength = ::SafeStrlenMT(Addresses) + ::SafeStrlenMT(From) + ::SafeStrlenMT(To) + ::SafeStrlenMT(Cc) +
		::SafeStrlenMT(Bcc) + 8;
	char* NewBuf = DEBUG_NEW_NOTHROW char[NewLength + 1];
	const char* ptr;
	
	if (!NewBuf)
		return;
		
	if (!Addresses)
		*NewBuf = 0;
	else
	{
		strcpy(NewBuf, Addresses);
		delete [] Addresses;
	}
	Addresses = NewBuf;

	if (To)
	{

		for (ptr = To; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		
		if(*ptr) {
		
			while(*To) {

				int i = 0;
				bool inQuotes = false;

				if(*To == '"')
					inQuotes = true;

				if(inQuotes) {
					do {
						i++;
					} while( *(To+i) != '"' );
					inQuotes = false;
				}

				while( (*(To+i) != ',') && (*(To+i) != '\0') )
					i++;

				if(*To == ',')
					To++;

				while( isspace((int)(unsigned char)*To) )
					To++;

				//check if Addresses already contains To. If so don't append
				bool bMatch = false;
				for (const char* address = Addresses; *address ; address++) {
					if(*address == *To) {
						if(strncmp(address, To, i) == 0) {
							bMatch = true;
							break;
						}
					}
				}

				if(!bMatch) {
					if (*Addresses)
						strcat(Addresses, Newline);
					strncat(Addresses, To, i);
				}

				To += i;
			}

		}

	}

	if (From)
	{

		for (ptr = From; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		
		if(*ptr) {
		
			while(*From) {

				int i = 0;
				bool inQuotes = false;

				if(*From == '"')
					inQuotes = true;

				if(inQuotes) {
					do {
						i++;
					} while( *(From+i) != '"' );
					inQuotes = false;
				}

				while( (*(From+i) != ',') && (*(From+i) != '\0') )
					i++;

				if(*From == ',')
					From++;

				while( isspace((int)(unsigned char)*From) )
					From++;

				//check if Addresses already contains From. If so don't append
				bool bMatch = false;
				for (const char* address = Addresses; *address ; address++) {
					if(*address == *From) {
						if(strncmp(address, From, i) == 0) {
							bMatch = true;
							break;
						}
					}
				}

				if(!bMatch) {
					if (*Addresses)
						strcat(Addresses, Newline);
					strncat(Addresses, From, i);
				}

				From += i;
			}

		}

	}

	if (Cc)
	{

		for (ptr = Cc; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		
		if(*ptr) {
		
			while(*Cc) {

				int i = 0;
				bool inQuotes = false;

				if(*Cc == '"')
					inQuotes = true;

				if(inQuotes) {
					do {
						i++;
					} while( *(Cc+i) != '"' );
					inQuotes = false;
				}

				while( (*(Cc+i) != ',') && (*(Cc+i) != '\0') )
					i++;

				if(*Cc == ',')
					Cc++;

				while( isspace((int)(unsigned char)*Cc) )
					Cc++;

				//check if Addresses already contains Cc. If so don't append
				bool bMatch = false;
				for (const char* address = Addresses; *address ; address++) {
					if(*address == *Cc) {
						if(strncmp(address, Cc, i) == 0) {
							bMatch = true;
							break;
						}
					}
				}

				if(!bMatch) {
					if (*Addresses)
						strcat(Addresses, Newline);
					strncat(Addresses, Cc, i);
				}

				Cc += i;
			}

		}

	}


	if (Bcc)
	{

		for (ptr = Bcc; *ptr && isspace((int)(unsigned char)*ptr); ptr++) {}
		
		if(*ptr) {
		
			while(*Bcc) {

				int i = 0;
				bool inQuotes = false;

				if(*Bcc == '"')
					inQuotes = true;

				if(inQuotes) {
					do {
						i++;
					} while( *(Bcc+i) != '"' );
					inQuotes = false;
				}
				while( (*(Bcc+i) != ',') && (*(Bcc+i) != '\0') )
					i++;

				if(*Bcc == ',')
					Bcc++;

				while( isspace((int)(unsigned char)*Bcc) )
					Bcc++;

				//check if Addresses already contains Bcc. If so don't append
				bool bMatch = false;
				for (const char* address = Addresses; *address ; address++) {
					if(*address == *Bcc) {
						if(strncmp(address, Bcc, i) == 0) {
							bMatch = true;
							break;
						}
					}
				}

				if(!bMatch) {
					if (*Addresses)
						strcat(Addresses, Newline);
					strncat(Addresses, Bcc, i);
				}

				Bcc += i;
			}

		}

	}


}

static void AddMessageAddresses(CSummary* Sum, char*& Addresses, BOOL DoAll)
{
	if (!Sum)
		return;

	bool NewDoc = false;

	// Find or load the message doc
	CMessageDoc* MsgDoc = Sum->GetMessageDoc(&NewDoc);
	if (!MsgDoc)
		return;
		
	const char* From = NULL;
	const char* To = NULL;
	const char* Cc = NULL;
	const char* Bcc = NULL;
	
	if (Sum->IsComp())
	{
		// Need to make sure message is read if we don't have a window
		if (!Sum->m_FrameWnd)
			MsgDoc->GetText();
		CCompMessageDoc* CompMsgDoc = (CCompMessageDoc*)MsgDoc;
			
		// I don't believe we want always the FROM field [jab 5/30/95] 
		if (DoAll)
			From = CompMsgDoc->GetHeaderLine(HEADER_FROM);
		To = CompMsgDoc->GetHeaderLine(HEADER_TO);
		Cc = CompMsgDoc->GetHeaderLine(HEADER_CC);
		Bcc = CompMsgDoc->GetHeaderLine(HEADER_BCC);
	}
	else
	{
		const char* MessageText = MsgDoc->GetText();
		From = HeaderContents(IDS_HEADER_FROM, MessageText);
		if (DoAll)
		{
			To = HeaderContents(IDS_HEADER_TO, MessageText);
			Cc = HeaderContents(IDS_HEADER_CC, MessageText);
		}
	}
	
	MergeAddresses(Addresses, From, To, Cc, Bcc);

	// Strip off "(by way of..." from address if it is there.
	unsigned int	i = 0;
	BOOL			bFound = FALSE;
	for (i = 0; (i < strlen(Addresses)) && !bFound; ++i)
	{
		if (Addresses[i] == '(')
		{
			bFound = TRUE;
		}
	}
	if (bFound && (strncmp(Addresses + i - 1, "(by way of", 10) == 0))
	{
		Addresses[i - 1] = '\0';
	}


	if (Sum->IsComp() == FALSE)
	{
		delete [] (char*)From;
		delete [] (char*)To;
		delete [] (char*)Cc;
		delete [] (char*)Bcc;
	}

	if (NewDoc)
		MsgDoc->OnCloseDocument();
}


////////////////////////////////////////////////////////////////////////
// GetNextToken [static]
//
// Get the next token (section of text surrounded by spaces) in the text
////////////////////////////////////////////////////////////////////////
static BOOL GetNextToken(CString& string, CString& token, int* idx)
{
	int		iStartIdx = *idx;
	*idx = string.Find(" ", *idx);
	if (*idx == -1)
	{
		token = string.Right(string.GetLength() - iStartIdx);
		return FALSE;
	}
	token = string.Mid(iStartIdx, (*idx - iStartIdx));
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// ParentheticalsExtractor [static]
//
// Remove any parentheses and double quotes and everything inside them.
////////////////////////////////////////////////////////////////////////
static void ParentheticalsExtractor(CString& string)
{
	int			iStart, iEnd;
	int			idx;

	do
	{
		idx = string.Find("\"");
		if (idx != -1)
		{
			iStart = idx;
			idx = string.Find("\"", idx + 1);
			if (idx != -1)
			{
				iEnd = idx;
				string = string.Left(iStart) + string.Right(string.GetLength() - iEnd - 1);
			}
			else
			{
				// Delete the solitary double quote
				string = string.Left(iStart) + string.Right(string.GetLength() - iStart - 1);
			}
		}
	} while (idx != -1);

	do
	{
		idx = string.Find("(");
		if (idx != -1)
		{
			iStart = idx;
			idx = string.Find(")", idx + 1);
			if (idx != -1)
			{
				iEnd = idx;
				string = string.Left(iStart) + string.Right(string.GetLength() - iEnd - 1);
			}
		}
	} while (idx != -1);
}

////////////////////////////////////////////////////////////////////////
// CommaExtractor [static]
//
// Remove any commas.
////////////////////////////////////////////////////////////////////////
static void CommaExtractor(CString& string)
{
	int			idx = 0;
	while ((idx = string.Find(",")) != -1)
	{
		string = string.Left(idx) + string.Right(string.GetLength() - idx - 1);
	}
}

////////////////////////////////////////////////////////////////////////
// IsQualifier [static]
//
// Return TRUE if given text is a qualifier string.
////////////////////////////////////////////////////////////////////////
static BOOL IsQualifier(CString& string)
{
	BOOL		bFound = FALSE;

	for (int i = 0; (i < NUM_QUALIFIERS) && !bFound; ++i)
	{
		bFound = (string == qualifierList[i]);
	}

	return bFound;
}

////////////////////////////////////////////////////////////////////////
// IsAllQualifier [static]
//
// Return TRUE if entire given text is composed of qualifier strings.
////////////////////////////////////////////////////////////////////////
static BOOL IsAllQualifier(CString& string)
{
	BOOL		bAllQualifier = TRUE;
	int			idx = 0;
	CString		token;

	while (GetNextToken(string, token, &idx) && bAllQualifier)
	{
		if (!IsQualifier(token))
		{
			bAllQualifier = FALSE;
		}
		++idx;
	}
	if (!IsQualifier(token))
	{
		bAllQualifier = FALSE;
	}
	return bAllQualifier;
}

////////////////////////////////////////////////////////////////////////
// QualifierExtractor [static]
//
// Remove any qualifier strings from string and place them in qualifiers.
////////////////////////////////////////////////////////////////////////
static void QualifierExtractor(CString& string, CString& qualifiers)
{
	int			idx = 0;
	CString		token;
	int			iLen = 0;
	BOOL		bMore = TRUE;

	while (bMore)
	{
		bMore = GetNextToken(string, token, &idx);
		if (IsQualifier(token))
		{
			iLen = token.GetLength();
			if (qualifiers.GetLength() > 0)		qualifiers += " ";
			qualifiers += token;
			if (idx == -1)	idx = string.GetLength();
			string = string.Left(idx - iLen) + string.Right(string.GetLength() - idx - 1);
			idx -= iLen;
		}
		else
		{
			idx += 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// IsHonorific [static]
//
// Return TRUE if given text is an honorific string.
////////////////////////////////////////////////////////////////////////
static BOOL IsHonorific(CString& string)
{
	BOOL		bFound = FALSE;

	for (int i = 0; (i < NUM_HONORIFICS) && !bFound; ++i)
	{
		bFound = (string == honorificList[i]);
	}

	return bFound;
}

////////////////////////////////////////////////////////////////////////
// HonorificsExtractor [static]
//
// Remove any honorific strings from string and place them in honorifics.
////////////////////////////////////////////////////////////////////////
static void HonorificsExtractor(CString& string, CString& honorifics)
{
	int			idx = 0;
	CString		token;
	int			iLen = 0;
	BOOL		bMore = TRUE;

	while (bMore)
	{
		bMore = GetNextToken(string, token, &idx);
		if (IsHonorific(token))
		{
			iLen = token.GetLength();
			if (honorifics.GetLength() > 0)		honorifics += " ";
			honorifics += token;
			if (idx == -1)	idx = string.GetLength();
			string = string.Left(idx - iLen) + string.Right(string.GetLength() - idx - 1);
			idx -= iLen;
		}
		else
		{
			idx += 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// FirstLastExtractor [static]
//
// Extract the first name and last name strings from string.
////////////////////////////////////////////////////////////////////////
static void FirstLastExtractor(CString& name, CString& firstName, CString& lastName)
{
	BOOL		bLikelyInAsia = FALSE;
	int			idx = -1;
	int			lastNameIdx = -1;
	CString		honorifics;
	if (bLikelyInAsia)
	{
		if ((idx = name.Find(" ")) != -1)
		{
			firstName = name.Right(name.GetLength() - idx - 1);
			lastName = name.Left(idx);
			firstName.TrimLeft();
			lastName.TrimRight();
		}
	}
	else
	{
		HonorificsExtractor(name, honorifics);
		name.TrimLeft();
		name.TrimRight();
		idx = 0;
		while ((idx = name.Find(" ", idx)) != -1)
		{
			lastNameIdx = idx;
			++idx;
		}
		if (lastNameIdx != -1)
		{
			lastName = name.Right(name.GetLength() - lastNameIdx - 1);
			firstName = name.Left(lastNameIdx);
			lastName.TrimLeft();
			firstName.TrimRight();
		}

	}	
}

////////////////////////////////////////////////////////////////////////
// MakeAutoNickname [static]
//
// Convert a string of the form:
//
//     "LastName, FirstName" <name@domain.com>
//
// to a string of the form:
//
//     LastName_FirstName
//
// that is no longer than 32 characters long
// Now we also need to fill in a full name, first name and last name
// from the same source data.
//
////////////////////////////////////////////////////////////////////////
void MakeAutoNickname(CString& address,
							  CString& strAutoFullName,
							  CString& strAutoFirstName,
							  CString& strAutoLastName)
{
	int		idx = -1;
	int		lastNameIdx = -1;

	// Strip everything after and including the first open bracket, if any
	address = address.SpanExcluding("<");

	// Strip (illegal) leading/trailing whitespace, if any
	address.TrimLeft();
	address.TrimRight();

	CString			qualifiers;
	CString			honorifics;

	// Discard parentheticals
	ParentheticalsExtractor(address);

	// Count the commas.
	int				iCommaCount = 0;
	int				iStartPos = 0;
	while ((idx = address.Find(",", iStartPos)) != -1)
	{
		++iCommaCount;
		iStartPos = idx + 1;
	}

	// If name contains only one comma
	if (iCommaCount == 1)
	{
		CString			afterComma = address.Right(address.GetLength() - iStartPos);
		CString			beforeComma = address.Left(iStartPos - 1);
		afterComma.TrimLeft();
		afterComma.TrimRight();
		beforeComma.TrimLeft();
		beforeComma.TrimRight();
		if (IsAllQualifier(afterComma))
		{
			// Note and remove any qualifiers
			qualifiers = afterComma;
			// Call FirstLastExtractor on the stuff before the comma
			FirstLastExtractor(beforeComma, strAutoFirstName, strAutoLastName);
		}
		else
		{
			// Note and remove any qualifiers
			QualifierExtractor(address, qualifiers);
			// Note the LastName as all words before the comma
			if ((idx = address.Find(",")) != -1)
			{
				address = address.Left(idx) + address.Right(address.GetLength() - idx - 1);
				afterComma = address.Right(address.GetLength() - idx);
				afterComma.TrimLeft();
				beforeComma = address.Left(idx);
				beforeComma.TrimRight();
			}
			strAutoLastName = beforeComma;
			// Note the FirstName as all words after the comma
			strAutoFirstName = afterComma;
		}
	}
	else if (iCommaCount == 0)
	{
		// Note and remove any qualifiers
		QualifierExtractor(address, qualifiers);
		// Call FirstLastExtractor on the name
		FirstLastExtractor(address, strAutoFirstName, strAutoLastName);
	}
	else if (iCommaCount > 1)
	{
		// If comma occurs after first token, assume Last, First Honorifics/Qualifiers
		int			tokenIdx = 0;
		CString		token;
		CString		afterComma;
		CString		beforeComma;

		GetNextToken(address, token, &tokenIdx);
		idx = address.Find(",");
		if ((tokenIdx - 1) == idx)
		{
			// Note and remove any honorifics and qualifiers
			HonorificsExtractor(address, honorifics);
			QualifierExtractor(address, qualifiers);
			// Note the LastName as the word before the first comma
			if ((idx = address.Find(",")) != -1)
			{
				address = address.Left(idx) + address.Right(address.GetLength() - idx - 1);
				afterComma = address.Right(address.GetLength() - idx);
				afterComma.TrimLeft();
				CommaExtractor(afterComma);
				beforeComma = address.Left(idx - 1);
				beforeComma.TrimRight();
			}
			strAutoLastName = beforeComma;
			// Note the FirstName as all words after the comma
			strAutoFirstName = afterComma;
		}
		else
		{
			// If comma occurs later, assume First Last Honorifics/Qualifiers
			// Note and remove any honorifics and qualifiers
			CommaExtractor(address);
			HonorificsExtractor(address, honorifics);
			QualifierExtractor(address, qualifiers);
			FirstLastExtractor(address, strAutoFirstName, strAutoLastName);
		}
	}

	if (qualifiers.GetLength() > 0)
	{
		if (strAutoFirstName.GetLength() > 0)
		{
			strAutoFirstName += " ";
		}
		strAutoFirstName += qualifiers;
	}

	// Shorten double-space sequences to single spaces in first and last names.
	while ((idx = strAutoFirstName.Find("  ")) != -1)
	{
		strAutoFirstName = strAutoFirstName.Left(idx) + strAutoFirstName.Right(strAutoFirstName.GetLength() - idx - 1);
	}
	while ((idx = strAutoLastName.Find("  ")) != -1)
	{
		strAutoLastName = strAutoLastName.Left(idx) + strAutoLastName.Right(strAutoLastName.GetLength() - idx - 1);
	}

	// Replace spaces and (illegal) commas with underscores
	while ((idx = address.FindOneOf(" ,")) != -1)
	{
		address.SetAt(idx, '_');
	}

	// Replace at-signs with hyphens
	while ((idx = address.FindOneOf("@")) != -1)
	{
		address.SetAt(idx, '-');
	}

	// As a final pass, shorten double-underscore sequences to single underscores.
	while ((idx = address.Find("__")) != -1)
	{
		address = address.Left(idx) + address.Right(address.GetLength() - idx - 1);
	}

	strAutoFullName = address;

	// Then make sure the auto-generated name meets the length requirements
	address = address.Left(32);

	while ((idx = strAutoFullName.FindOneOf("_")) != -1)
	{
		strAutoFullName.SetAt(idx, ' ');
		lastNameIdx = idx;
	}
}


////////////////////////////////////////////////////////////////////////
// GetAutoNicknameFromWindow [static]
//
// Formats the 'strAutoNickname' string with an "auto" nickname, based on
// the current open window.  Returns TRUE if the autoNickname was
// successfully set.
////////////////////////////////////////////////////////////////////////
static BOOL GetAutoNicknameFromWindow(UINT windowID, 
									  CString& strAutoNickname,
									  CString& strAutoFullName,
									  CString& strAutoFirstName,
									  CString& strAutoLastName)
{
	CFrameWnd* p_frame = (CFrameWnd *) AfxGetMainWnd();
	ASSERT(p_frame != NULL);
	ASSERT(p_frame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	ASSERT(strAutoNickname.IsEmpty());
	ASSERT(strAutoFullName.IsEmpty());
	ASSERT(strAutoFirstName.IsEmpty());
	ASSERT(strAutoLastName.IsEmpty());

	if (NULL == p_frame)
	{
		ASSERT(0);
		return FALSE;
	}

	switch (windowID)
	{
//FORNOW	case IDR_LOOKUP32:
//FORNOW		{
//FORNOW			//
//FORNOW			// If Make Nickname is being done on a *single*, selected item
//FORNOW			// in the PH window, then pre-populate the "new name" field of
//FORNOW			// the dialog box with a stripped down version of the name.
//FORNOW			// Don't bother with any of this if there are multiple
//FORNOW			// selected items.
//FORNOW			//
//FORNOW			CLookupView* p_lookup_view = (CLookupView *) p_frame->GetActiveFrame()->GetActiveView();
//FORNOW
//FORNOW			//
//FORNOW			// Ask the CLookupView object for a list of selected e-mail 
//FORNOW			// addresses, assumed to be of the form:
//FORNOW			//
//FORNOW			//      "Real Name" <name@domain.com>
//FORNOW			//
//FORNOW			if (p_lookup_view)
//FORNOW			{
//FORNOW				ASSERT(p_lookup_view->IsKindOf(RUNTIME_CLASS(CLookupView)));
//FORNOW
//FORNOW				CStringList address_list;
//FORNOW			 	if (p_lookup_view->GetSelectedAddresses(address_list) && 
//FORNOW					(address_list.GetCount() == 1))
//FORNOW				{
//FORNOW					CString address = address_list.RemoveHead();
//FORNOW
//FORNOW					MakeAutoNickname(address);
//FORNOW					if (! address.IsEmpty())
//FORNOW						strAutoNickname = address;
//FORNOW				}
//FORNOW			}
//FORNOW		}
//FORNOW		break;
	case IDR_TOC:
		{
			// There's only one way that we need to get the single-selected Summary these
			// days. We get the TOC frame and ask it.
			CTocFrame *		pTocFrame = DYNAMIC_DOWNCAST( CTocFrame, p_frame->GetActiveFrame() );
			ASSERT(pTocFrame);
			if (pTocFrame)
			{
				//
				// Grab single-selected Summary from TOC list box --
				// don't bother with any of this if there are multiple
				// selected items.  If Make Nickname is being done on
				// a *single*, selected item in a TOC window, then
				// pre-populate the "new name" field of the dialog box
				// with a stripped down version of the sender's "Real
				// Name", if any.
				//

				//	Get the selected summaries
				CTocFrame::SummaryArrayT		currentPreviewSummaryArray;
				pTocFrame->GetSelectedSummaries(currentPreviewSummaryArray);
				int nNumSummaries = currentPreviewSummaryArray.GetSize();
				//get the From address only if we have 1 selected message
				//if we have multiple selected messages then we leave it to the
				//user to fill in the nickname etc.
				if (nNumSummaries == 1) {
					
						CString name(currentPreviewSummaryArray[0]->m_From);

						MakeAutoNickname(name, strAutoFullName, strAutoFirstName, strAutoLastName);

						if (! name.IsEmpty())
							strAutoNickname = name;
				}

			}
		}
		break;
	case IDR_READMESS:
	case IDR_COMPMESS:
		{
			//
			// If Make Nickname is being done on a read or comp message
			// window, then pre-populate the "new name" field of the
			// dialog box with a stripped down version of the sender's
			// "Real Name" for read messages, or with the recipient's
			// name (To/Cc/Bcc) for comp messages.
			//
			CMessageDoc* p_msgdoc = (CMessageDoc *) p_frame->GetActiveFrame()->GetActiveDocument();
			if (p_msgdoc)
			{
				ASSERT(p_msgdoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)));
				if (p_msgdoc->m_Sum)
				{
					CString name;
					if (p_msgdoc->m_Sum->IsComp())
					{
						//
						// Comp message case.  The auto nickname naming
						// only applies if there is a single recipient
						// in all of the To:, Cc:, and Bcc: headers.
						//
						CCompMessageDoc* p_compdoc = (CCompMessageDoc*) p_msgdoc;
						ASSERT(p_compdoc->IsKindOf(RUNTIME_CLASS(CCompMessageDoc)));

						//
						// Build up the recipient list to see if we can
						// form an auto-nickname from it....
						//
						for (int i = 1; i <= 3; i++)
						{
							const char* p_header = NULL;
							switch (i)
							{
							case 1:
								p_header = p_compdoc->GetHeaderLine(HEADER_TO);
								break;
							case 2:
								p_header = p_compdoc->GetHeaderLine(HEADER_CC);
								break;
							case 3:
								p_header = p_compdoc->GetHeaderLine(HEADER_BCC);
								break;
							default:
								ASSERT(0);
								p_header = NULL;
								break;
							}

							if (p_header && *p_header)
							{
								if (! name.IsEmpty())
									name += ", ";
								name += p_header;
							}
						}

						if (! name.IsEmpty())
						{
							//
							// We can't use no multi-recipient addresses
							// as an auto-nickname, ya hear?
							//
							const char* p_end = ::FindAddressEnd(name);
							if (p_end && (*p_end == ','))
								name.Empty();
						}
					}
					else
					{
						//
						// Read message case.  Just look at the "From" header.
						//
						name = p_msgdoc->m_Sum->m_From;
					}

					if (! name.IsEmpty())
					{
						MakeAutoNickname(name, strAutoFullName, strAutoFirstName, strAutoLastName);

						if (! name.IsEmpty())
							strAutoNickname = name;
					}
				}
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	if (strAutoNickname.IsEmpty())
		return FALSE;
	else
		return TRUE;
}


////////////////////////////////////////////////////////////////////////
// MakeNickname [extern]
//
// Returns TRUE if nicknames were successfully added to the database.
////////////////////////////////////////////////////////////////////////
BOOL MakeNickname()
{
 	//
 	// PART ONE.  The whole idea is to extract an optional "auto
 	// nickname" and optional list of addresses from the "source
 	// window".  The source window can either be a Wazoo window with
 	// keyboard focus or the topmost MDI child window.
 	//
 	char* pszAddresses = NULL;
 	CString strAutoNickname;	// auto-generated default nickname, if any
	CString strAutoFullName;	// auto-generated default full name, if any
	CString strAutoFirstName;	// auto-generated default first name, if any
	CString strAutoLastName;	// auto-generated default last name, if any
 	BOOL DoAll = GetIniShort(IDS_INI_REPLY_TO_ALL);
 	BOOL bNicknameWndIsActive = FALSE;
	bool bWriteNotes          = true;
 	CMainFrame* pMainFrame = (CMainFrame *) ::AfxGetMainWnd();
 	ASSERT_KINDOF(CMainFrame, pMainFrame);

 	CWazooWnd* pWazooWnd = pMainFrame->GetWazooWindowWithFocus();
 	if (pWazooWnd) 	{
	    // Check to see if the Wazoo window is a valid source of
	    // nickname info.
	    DirectoryServicesWazooWndNew *pDSWazooWnd
		= DYNAMIC_DOWNCAST(DirectoryServicesWazooWndNew, pWazooWnd);
	    if (pDSWazooWnd) {
		// If Make Nickname is being done on a *single*, selected item
		// in the PH window, then pre-populate the "new name" field of
		// the dialog box with a stripped down version of the name.
		// Don't bother with any of this if there are multiple
		// selected items.
		//
		// Ask the DirectoryServicesWazooWndNew object for a
		// list of selected e-mail addresses, assumed to be of
		// the form:
		//
		//      "Real Name" <name@domain.com>
		CStringList strAddressList;
		if (pDSWazooWnd->GetSelectedAddresses(strAddressList) && 
		    (strAddressList.GetCount() == 1)) {
		    strAutoNickname = strAddressList.RemoveHead();
			CString temp = strAutoNickname;
			int nIndex = strAutoNickname.Find("<");//here is where the address starts, remove all double quotes
			//before that otherwise ParentheticalExtractor will delete the string between those quotes.
			if(nIndex >0)
			{
				temp = strAutoNickname.Left(nIndex);
				temp.Remove('\"');
			}
			MakeAutoNickname(temp, strAutoFullName, strAutoFirstName, strAutoLastName);
			strAutoNickname = temp;
		}

		strAddressList.RemoveAll();

		// Ask the DirectoryServicesWazooWndNew object for a list
		// of selected e-mail addresses.
		if (pDSWazooWnd->GetSelectedAddresses(strAddressList))
		{
		    if (strAddressList.GetCount() > 1) {
			bWriteNotes = false;
		    }
		    // Convert the string list into a single string buffer.  We
		    // do this in two passes.  First, we need to count up how
		    // much space the buffer needs to hold all the nicknames
		    // (plus commas after each one).  Then, we copy the strings
		    // into the buffer, separated by commas.
		    int Length = 0;
		    POSITION pos = strAddressList.GetHeadPosition();
		    while (pos != NULL) {
			CString strAddress = strAddressList.GetNext(pos);

			//NOTE: we are adding a comma instead of CRLF pair since
			//because CRLF pair does not display correctly in the dialog
			//so it is only length+1
			Length += strAddress.GetLength() + 1;
		    }
		    if (!Length || !(pszAddresses = DEBUG_NEW_NOTHROW char[Length + 1]))
			return FALSE;
		    *pszAddresses = 0;
				
		    // Now, we do a second pass to copy the addresses
		    // into the buffer we just allocated.
		    char* a = pszAddresses;
		    while (! strAddressList.IsEmpty()) {
			CString strAddress = strAddressList.RemoveHead();
			if (a != pszAddresses) {
				//using comma to seperate the addresses instead of CRLF pair
				*a++ = ',';
			}
			strcpy(a, strAddress);
                        // better late than never
			ASSERT(int(strlen(pszAddresses)) <= Length);

			a += strAddress.GetLength();
		    }
		    *a = 0;
		}

		// HACK ALERT...  The Evil Goto(tm).
		goto PART3;
	    }

 		CNicknamesWazooWnd* pNicknamesWazooWnd = DYNAMIC_DOWNCAST(CNicknamesWazooWnd, pWazooWnd);
 		if (pNicknamesWazooWnd)
 		{
 			//
 			// Set flag used later to determine whether or not to 
 			// auto-save the newly-added nicknames.
 			//
 			bNicknameWndIsActive = TRUE;

 			//
 			// Fetch a list of selected CNickname objects from the view.
 			//
 			ASSERT(g_Nicknames);
 			CObList nickname_list;
 			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_FETCH_SELECTED_NICKNAMES, &nickname_list);

 			// Count up how much space the buffer needs to hold all the nicknames
 			// (plus commas after each one)
 			int nLength = 0;
 			POSITION pos = nickname_list.GetHeadPosition();
 			while (pos != NULL)
 			{
 				CNickname* pNickname = (CNickname *) nickname_list.GetNext(pos);
 				ASSERT(pNickname != NULL);
 				ASSERT_KINDOF(CNickname, pNickname);
				//NOTE: we are adding a comma instead of CRLF pair since
				//because the CRLF pair does not display correctly in the dialog
				//so it is only length+1
 				nLength += pNickname->GetName().GetLength() + 1;
 			}
 			if (!nLength || !(pszAddresses = DEBUG_NEW_NOTHROW char[nLength + 1]))
 				return FALSE;
 			*pszAddresses = 0;
			
 			// Now stick the names in to the buffer
 			char* a = pszAddresses;
 			while (! nickname_list.IsEmpty())
 			{
 				CNickname* pNickname = (CNickname *) nickname_list.RemoveHead();
 				if (a != pszAddresses)
 				{
					//using comma to seperate addresses instead of CRLF pair
					*a++ = ',';
 				}
 				strcpy(a, pNickname->GetName());
 				a += pNickname->GetName().GetLength();
 				ASSERT(int(strlen(pszAddresses)) <= nLength);	// better late than never
 			}
 			*a = 0;

 			//
 			// HACK ALERT...  The Evil Goto(tm).
 			//
 			goto PART3;
 		}

 		//
 		// If we get this far, the Wazoo window is not a source of
 		// nickname info, so fall back on the topmost MDI child.
 		//
 	}

 	//
 	// PART TWO.  No usable Wazoo, so use topmost MDI child.
 	//
 	{
 		UINT WindowID = GetTopWindowID();
 		switch (WindowID)
 		{
 		case IDR_TOC:
 		case IDR_READMESS:
 		case IDR_COMPMESS:
 			break;
 		default:
// 			ASSERT(0);
 			return (FALSE);
 		}

 		ASSERT(g_Nicknames);
			
 		//
 		// To avoid a weird race condition with the nicknames child
 		// window getting activated by the CNameNickDialog, we cache
 		// the active child window doc/view pair here, BEFORE we run
 		// CNameNickDialog.  If we don't do this, then
 		// CFrameWnd::GetActiveFrame() can return the nickname child
 		// window when we expected another child window.
 		//
 		CDocument* p_active_doc = pMainFrame->GetActiveFrame()->GetActiveDocument();

 		// If Make Nickname is being done on selected nicknames in the Nicknames window,
 		// then build the list ahead of time because the Make Nickname dialog will change
 		// the selection in the list box
 		switch (WindowID)
 		{
 		case IDR_TOC:
 			{
 				GetAutoNicknameFromWindow(WindowID, strAutoNickname, strAutoFullName, strAutoFirstName, strAutoLastName);

 				CTocDoc* Toc;
 				CTocView* TocView;
 				if ((Toc = (CTocDoc*) p_active_doc) &&
 					(TocView = Toc->GetView()))
 				{
 					ASSERT(Toc->IsKindOf(RUNTIME_CLASS(CTocDoc)));
 					ASSERT(TocView->IsKindOf(RUNTIME_CLASS(CTocView)));
 					int SelCount = TocView->m_SumListBox.GetSelCount();
 					int TotalCount = TocView->m_SumListBox.GetCount();
				
 					for (int i = 0; SelCount && i < TotalCount; i++)
 					{
 						if (TocView->m_SumListBox.GetSel(i) > 0)
 						{
 							CSummary* Sum = TocView->m_SumListBox.GetItemDataPtr(i);
 							if (Sum)
 								AddMessageAddresses(Sum, pszAddresses, DoAll);
 							SelCount--;
 						}
 					}
 				}
 			}
 			break;
 		case IDR_READMESS:
 		case IDR_COMPMESS:
 			{
 				GetAutoNicknameFromWindow(WindowID, strAutoNickname, strAutoFullName, strAutoFirstName, strAutoLastName);

 				CMessageDoc* MsgDoc = (CMessageDoc *) p_active_doc;
 				if (MsgDoc)
 				{
 					ASSERT(MsgDoc->IsKindOf(RUNTIME_CLASS(CMessageDoc)));
 					AddMessageAddresses(MsgDoc->m_Sum, pszAddresses, DoAll);
 				}
 			}
 			break;
 		default:
 			ASSERT(0);
 			return FALSE;
 		}
 	}

	//
	// At this point, 'pszAddresses' points to the list of addresses
	// extracted from a source window.  If we don't include ourselves
	// in the replies, then strip addresses that intersect with the
	// "me" nickname from the address list.
	//
	if (pszAddresses && (strlen(pszAddresses) > 0))
	{
 		if (GetIniShort(IDS_INI_INCLUDE_SELF) == FALSE)
 			StripMe(pszAddresses);
	}

	//
	// Now, check to see if the address list is empty or not.  If it
	// is empty, then give the user a chance to bail.
	//
	if (pszAddresses && (strlen(pszAddresses) == 0))
	{
		if (IDNO == ::AfxMessageBox(CRString(IDS_MAKE_EMPTY_NICKNAME), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
		{
			// user wants to bail...
			delete [] pszAddresses;
			return FALSE;
		}
	}

 	//
 	// PART THREE.  If we get this far, 'strAutoNickname' contains the
 	// optional auto nickname and 'pszAddresses' points to the optional
 	// list of nicknames.
 	//
 PART3:
 	while (1)
 	{
 		CNameNickDialog dlg(ID_SPECIAL_MAKENICKNAME);

 		if (!pszAddresses)
 			break;

		dlg.m_NewName = strAutoNickname;		// can be empty
		dlg.m_NewFullName = strAutoFullName;
		dlg.m_NewFirstName = strAutoFirstName;
		dlg.m_NewLastName = strAutoLastName;
		dlg.m_NewAddress = pszAddresses;
		
 		if (dlg.DoModal() == IDCANCEL || !dlg.m_pNicknameFile)
 			break;

	 	delete [] pszAddresses;

		return AddNickname(dlg.m_pNicknameFile,
							dlg.m_NewName,
							dlg.m_NewFullName,
							dlg.m_NewFirstName,
							dlg.m_NewLastName,
							dlg.m_NewAddress,
							dlg.m_PutOnRecipientList,
							dlg.m_PutOnBPList,
							bWriteNotes,
							bNicknameWndIsActive);
	}
 	delete [] pszAddresses;
 	return (FALSE);
}

////////////////////////////////////////////////////////////////////////
// AddNickname [extern]
//
// Returns TRUE if nicknames were successfully added to the database.
////////////////////////////////////////////////////////////////////////
BOOL AddNickname(CNicknameFile *pNicknameFile,
				 CString &strNickname,
				 CString &strFullName,
				 CString &strFirstName,
				 CString &strLastName,
				 CString &strAddress,
				 BOOL bPutOnRecipientList,
				 BOOL bPutOnBPList,
				 bool bWriteNotes,
				 BOOL bNicknameWndIsActive)
{
 	CNickname nickname(strNickname);
 	ASSERT(nickname.HasValidName());

 	nickname.SetAddresses(strAddress);
 	nickname.SetAddressesLength(nickname.GetAddresses().GetLength());

	CString notes;
	
	if (strFullName.IsEmpty() && bWriteNotes == true) {
 		//
 		// If we have access to the "Real Name", then format it properly
 		// and stuff it into the Notes field of the nickname.
 		//
 		CString real_name(strAddress);

 		//
 		// We can't use no multi-recipient addresses
 		// as an auto-nickname, ya hear?
 		//
 		const char* p_end = ::FindAddressEnd(real_name);
 		if (p_end && (*p_end != ','))
 		{
 			::GetRealName(real_name.GetBuffer(real_name.GetLength()));
 			real_name.ReleaseBuffer();
 			notes.Format("<%s:%s>", (const char *) "name", (const char *) real_name);
 		}
 	}
	else	
		notes = "<name:"+ strFullName + ">";

	notes += "<first:" + strFirstName + "><last:" + strLastName + ">";
	nickname.SetRawNotes(notes);
	nickname.SetRawNotesLength(nickname.GetRawNotes().GetLength());

 	if (!pNicknameFile)
 	{
 		ASSERT(0);
 		return FALSE;
 	}

 	CNickname* p_new_nickname = pNicknameFile->AddNickname(nickname, bPutOnRecipientList, bPutOnBPList);
 	if (p_new_nickname)
 	{
 		pNicknameFile->SetModified(TRUE/*bIsModified*/, FALSE/*bForgetHashes*/);

 		// Notify everyone that there were some changes
 		//FORNOWg_Nicknames->SetModifiedFlag();

 		//
 		// Do the auto-save behavior for the Make Nickname command,
 		// if the Nicknames window is NOT the active window.
 		//
 		if (! bNicknameWndIsActive)
 			g_Nicknames->OnSaveDocument(NULL);

 		//
 		// If the Nicknames window is open, force the RHS view to
 		// update.  The NICKNAME_WAS_CHANGED message is a kludge
 		// to force the single-selection back to an existing
 		// nickname item.  It has nothing to do with renaming
 		// nicknames.
 		//
 		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_NICKNAME_WAS_CHANGED, p_new_nickname);
 		g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_DISPLAYNICKNAME, p_new_nickname);

 		return (TRUE);
 	}

	// user canceled, or ran out of memory
 	return (FALSE);
}

////////////////////////////////////////////////////////////////////////
// NicknameFileExists [extern]
//
// Returns TRUE if the user-defined nickname file exists in the user's
// nicknames folder.
////////////////////////////////////////////////////////////////////////
BOOL NicknameFileExists(const char* nickFilename)
{
	CString pathname(EudoraDir);
	pathname += CRString(IDS_NICK_DIR_NAME);
	pathname += SLASHSTR;
	pathname += nickFilename;

	return ::FileExistsMT(pathname);
}

////////////////////////////////////////////////////////////////////////
// CreateNewNicknameFile [extern]
//
// Creates a new, empty nickname file in the user's nickname folder.
////////////////////////////////////////////////////////////////////////
BOOL CreateNewNicknameFile(const char* newFilename)
{
	// Shareware: Reduced feature mode only allows one nickname file, so no CChooseNicknameFileDlg
	if (!UsingFullFeatureSet())
	{
		// REDUCED FEATURE
		ASSERT(0);
		return (FALSE);
	}

	CString pathname(EudoraDir);
	pathname += CRString(IDS_NICK_DIR_NAME);
	pathname += SLASHSTR;
	pathname += newFilename;

	// Create the nickname file
	JJFile newfile;
	if (FAILED(newfile.Open(pathname, O_WRONLY | O_CREAT | O_TRUNC)))
		return FALSE;
	newfile.Close();

	//
	// Add the new nickname file to the collection of nickname files
	// maintained by CNicknamesDoc.  Then, refresh the nicknames view.
	// 
	if (g_Nicknames)
	{
		CNicknameFile* p_nickfile = g_Nicknames->AddNicknameFile(pathname, "");

		if (p_nickfile != NULL)
		{
			//
			// Update the view.
			//
			g_Nicknames->UpdateAllViews(NULL, NICKNAME_HINT_ADDED_NEW_NICKFILE, p_nickfile);
			return TRUE;
		}
	}
	return FALSE;
}


