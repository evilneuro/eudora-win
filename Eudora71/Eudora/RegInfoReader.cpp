// RegInfoReader.cpp
//
// MIMEReader class for handling EMSAPI-like registration code
// attachment translations. Parsing of information is done
// utilizing rfc 822 code.
//
// Copyright (c) 2000 by QUALCOMM, Incorporated
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

#include "resource.h"
#include "rs.h"
#include "fileutil.h"
#include "lex822.h"
#include "header.h"
#include "LineReader.h"
#include "address.h"
#include "mime.h"
#include "msgutils.h"
#include "persona.h"	//	g_Personalities
#include "doc.h"
#include "nickdoc.h"
#include "pop.h"
#include "guiutils.h"
#include "trnslate.h"

#include "RegInfoReader.h"
#include "QCSharewareManager.h"
#include "RegistrationCodeDlg.h"


class MIMEState;

#include "DebugNewHelpers.h"

CProfileConfirmationDialog *		RegInfoReader::s_pProfileConfirmationDlg = NULL;


class CProfileConfirmationDialog : public CDialog
{
public:
	enum { IDD = IDD_PROFILE_CONFIRMATION };
	
							CProfileConfirmationDialog(
								CWnd *			pParentWnd,
								UINT			nMessageID,
								const char *	szFirstName,
								const char *	szLastName,
								const char *	szMailedTo,
								const char *	szProfileID);
	virtual					~CProfileConfirmationDialog();

	static void				FormatMessageString(
								UINT			nMessageID,
								const char *	szFirstName,
								const char *	szLastName,
								const char *	szMailedTo,
								CString &		szMessageString);
	
	virtual void			DoDataExchange(CDataExchange * pDX);
	
	virtual void			OnOK();
	virtual void			OnCancel();
	virtual BOOL			OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	virtual void			PostNcDestroy();

	CString					m_szMessage;
	CString					m_szProfileID;
};


CProfileConfirmationDialog::CProfileConfirmationDialog(
	CWnd *			pParentWnd,
	UINT			nMessageID,
	const char *	szFirstName,
	const char *	szLastName,
	const char *	szMailedTo,
	const char *	szProfileID)
	:	CDialog(CProfileConfirmationDialog::IDD, pParentWnd), m_szProfileID(szProfileID)
{
	FormatMessageString(nMessageID, szFirstName, szLastName, szMailedTo, m_szMessage);

	Create(CProfileConfirmationDialog::IDD);

	//	Center it in the main window (or screen in app is minimized)
	CenterWindow(AfxGetMainWnd() && AfxGetMainWnd()->IsIconic()? GetDesktopWindow() : NULL);

	ShowWindow(SW_SHOW);
}


CProfileConfirmationDialog::~CProfileConfirmationDialog()
{
	if (RegInfoReader::s_pProfileConfirmationDlg == this)
		RegInfoReader::s_pProfileConfirmationDlg = NULL;
}


void CProfileConfirmationDialog::PostNcDestroy()
{
	delete this;
}


void CProfileConfirmationDialog::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_PROFILE_CONFIRMATION_TEXT, m_szMessage);
}


void CProfileConfirmationDialog::FormatMessageString(
	UINT			nMessageID,
	const char *	szFirstName,
	const char *	szLastName,
	const char *	szMailedTo,
	CString &		szMessageString)
{
	CString		szNameAndAddress = "";

	if (szFirstName && *szFirstName)
	{
		szNameAndAddress += szFirstName;
		szNameAndAddress += " ";
	}

	if (szLastName && *szLastName)
	{
		szNameAndAddress += szLastName;
		szNameAndAddress += " ";
	}

	if (szMailedTo && *szMailedTo)
	{
		szNameAndAddress += "<";
		szNameAndAddress += szMailedTo;
		szNameAndAddress += ">";
	}

	AfxFormatString1(szMessageString, nMessageID, szNameAndAddress);
}


void CProfileConfirmationDialog::OnOK()
{
	//	Save the profile info. This assumes that it's a string with no CRLF.
	SetIniString(IDS_INI_PROFILE, m_szProfileID);

	// Make sure profile is saved in ini right now.
	FlushINIFile();
	
	DestroyWindow();
}


void CProfileConfirmationDialog::OnCancel()
{
	DestroyWindow();
}


BOOL CProfileConfirmationDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_FAQ_WEBSITE)
	{
		LaunchURLWithQuery(NULL, ACTION_PROFILE_FAQ);
	}

	return CDialog::OnCommand(wParam, lParam);
}


RegInfoReader::EudoraInfoEnum
RegInfoReader::GetRegInfoFromLineReader(
	LineReader *			pLineReader,
	bool					shouldWriteOutput,
	JJFile *				pOutputFile,
	CString &				outFirstName,
	CString &				outLastName,
	CString &				outRegCode,
	CString *				outProfileID,
	CString *				outMailedTo,
	bool *					outDeleteProfileID,
	CString *				outDistributorID,
	bool *					outEudoraNeedsRegistration)
{
	static const char * RegInfoAttributeStrings[] = 
		{
			"Eudora-File-Type",
			"Eudora-Last-Name",
			"Eudora-First-Name",
			"Eudora-Reg-Code",
			"Profile",
			"Eudora-Needs-Registration",
			"Eudora-Distributor-ID",
			"Mailed-To",
			"Delete"
		};
	typedef enum
	{
		RI_FIRST_ATTR,
		RI_FILE_TYPE = RI_FIRST_ATTR,
		RI_LAST_NAME,
		RI_FIRST_NAME,
		RI_REG_CODE,
		RI_PROFILE_STRING,
		RI_NEEDS_REGISTRATION,
		RI_DISTRIBUTOR_ID,
		RI_MAILED_TO,
		RI_DELETE_PROFILE,
		RI_LAST_ATTR = RI_DELETE_PROFILE,
		RI_UNKNOWN
	} RegInfoAttributeIDs;

	// States of the reg info interpreter
	typedef enum
	{
		ristate_ExpectHeaderName,		// next token should be a header name
		ristate_ExpectColon,			// next token should be a colon
		ristate_ExpectText,				// we're looking for an unstructured field
	} RIStateEnum;


	RIStateEnum				riState = ristate_ExpectHeaderName;		// state of reg info interpreter
	RegInfoAttributeIDs		riFound = RI_UNKNOWN;
	short					riIndex;
	Token822Enum			tokenType;
	Lex822State				l822s(pLineReader, shouldWriteOutput, pOutputFile);
	CString					szFileType;
	CString					szTemp;
	EudoraInfoEnum			eudoraInfoType = rs_Unknown;


#define EXPECT_TEXT l822s.State = CollectText; riState = ristate_ExpectText;

	// Initialize all strings in case caller doesn't check return value
	outLastName = "";
	outFirstName = "";
	outRegCode = "";
	if (outProfileID != NULL)
		*outProfileID = "";
	if (outMailedTo != NULL)
		*outMailedTo = "";
	if (outDistributorID != NULL)
		*outDistributorID = "";

	if (outDeleteProfileID != NULL)
		*outDeleteProfileID = false;
	if (outEudoraNeedsRegistration != NULL)
		*outEudoraNeedsRegistration = false;

	// Parse the registration information in a robust fashion with a rfc 822 parser.
	for ( tokenType = l822s.Lex822(); (tokenType != EndOfMessage); tokenType = l822s.Lex822() )
	{
		if (shouldWriteOutput)
		{
			// Write the token to disk
			tokenType = l822s.WriteHeaderToken(tokenType);
		}
		
		//	Ignore comments
		if (tokenType == Comment)
			tokenType = LinearWhite;
		
		// First, we handle a few special token types
		switch (tokenType)
		{
			case EndOfHeader:
				//	Ignore
				break;

			case ErrorToken:
				return rs_Unknown;
				break;
			
			// If we have a return, we've hit the end of a header field the header
			// can only legally end in ristate_ExpectText or ExpectSem or ExpectVersion
			case Special:
				if (l822s.Token[0] == '\r')
					break;		// Skip first part of newline
				else if (l822s.Token[0] == '\n')
				{
					riState = ristate_ExpectHeaderName;
					break;
				}
				// else fall through
				
			default:
				// We handle the remaining tokens on a state-by-state basis
				switch (riState)
				{
					case ristate_ExpectHeaderName:
						switch (tokenType)
						{
							case LinearWhite:
								break;	// Ignore
										
							case Atom:
								for (riIndex = RI_FIRST_ATTR; riIndex <= RI_LAST_ATTR; riIndex++)
									if (!stricmp(RegInfoAttributeStrings[riIndex], (char*)l822s.Token))
										break;
								riFound = (RegInfoAttributeIDs) riIndex;
								if (riFound == RI_UNKNOWN)
								{
									// Just allow the rest of the line after the unknown tag
									EXPECT_TEXT;
								}
								else
								{
									riState = ristate_ExpectColon;
								}
								break;
										
							default:
								EXPECT_TEXT;
								break;
						}
						break;
								
					case ristate_ExpectColon:
						switch (tokenType)
						{
							case LinearWhite:
								break; // ignore
									
							case Special:
								if (l822s.Token[0] == ':')
									EXPECT_TEXT;
								break;
										
							default:
								EXPECT_TEXT;
								break;
						}
						break;

					case ristate_ExpectText:
						// Store information for all known cases
						switch (riFound)
						{
							case RI_FILE_TYPE:
								szFileType = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;
						
							case RI_LAST_NAME:
								outLastName = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;

							case RI_FIRST_NAME:
								outFirstName = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;
							
							case RI_REG_CODE:
								outRegCode = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;

							case RI_PROFILE_STRING:
								if (outProfileID != NULL)
									*outProfileID = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;

							case RI_NEEDS_REGISTRATION:
								if (outEudoraNeedsRegistration != NULL)
								{
									szTemp = ::TrimWhitespaceMT( (char*) l822s.Token );
									*outEudoraNeedsRegistration = (szTemp.CompareNoCase("yes") == 0);
								}
								break;

							case RI_DISTRIBUTOR_ID:
								if (outDistributorID != NULL)
									*outDistributorID = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;

							case RI_MAILED_TO:
								if (outMailedTo != NULL)
									*outMailedTo = ::TrimWhitespaceMT( (char*) l822s.Token );
								break;

							case RI_DELETE_PROFILE:
								if (outDeleteProfileID != NULL)
								{
									szTemp = ::TrimWhitespaceMT( (char*) l822s.Token );
									*outDeleteProfileID = (szTemp.CompareNoCase("yes") == 0);
								}
								break;
						}
				}
		}
	}

	if (szFileType.CompareNoCase(CRString(IDS_INFO_ATTACH_TYPE_REG)) == 0)
	{
		eudoraInfoType = rs_Reg;
	}
	else if (szFileType.CompareNoCase(CRString(IDS_INFO_ATTACH_TYPE_PROFILE)) == 0)
	{
		eudoraInfoType = rs_Profile;
	}


	return eudoraInfoType;
}


RegInfoReader::EudoraInfoEnum
RegInfoReader::InternalGetInfoFromFile(
	const char *			szFilePath,
	CString &				outFirstName,
	CString &				outLastName,
	CString &				outRegCode,
	CString *				outProfileID,
	CString *				outMailedTo,
	bool *					outDeleteProfileID,
	CString *				outDistributorID,
	bool *					outEudoraNeedsRegistration)
{
	//	Reread registration information
	JJFile			regInfoFile;
	EudoraInfoEnum	eudoraInfoType = rs_Invalid;

	if ( SUCCEEDED(regInfoFile.Open(szFilePath, O_RDONLY)) )
	{
		RegInfoFileLineReader		regInfoReader;
		regInfoReader.SetFile(&regInfoFile);

		//	Call GetRegInfoFromLineReader passing false so that it doesn't
		//	write out to a file as it parses the information.
		eudoraInfoType = GetRegInfoFromLineReader( &regInfoReader, false, NULL, outFirstName,
												   outLastName, outRegCode, outProfileID,
												   outMailedTo, outDeleteProfileID,
												   outDistributorID, outEudoraNeedsRegistration );
		regInfoFile.Close();
	}

	return eudoraInfoType;
}


bool RegInfoReader::GetInfoFromRegCodeStartupFile(
	const char *			szFilePath,
	CString &				outFirstName,
	CString &				outLastName,
	CString &				outRegCode,
	CString &				outDistributorID,
	SharewareModeType &		outRegMode,
	bool &					outEudoraNeedsRegistration,
	int *					outRegCodeMonth)
{
	bool		bRegCodeGood = false;

	EudoraInfoEnum		eudoraInfoType =
			InternalGetInfoFromFile( szFilePath, outFirstName, outLastName, outRegCode,
									 NULL, NULL, NULL, &outDistributorID, &outEudoraNeedsRegistration );

	if (outRegCodeMonth)
		*outRegCodeMonth = 0;
	if (eudoraInfoType == rs_Reg)
	{
		bRegCodeGood = QCSharewareManager::IsValidRegistrationInfo( outFirstName, outLastName, outRegCode,
																	&outRegMode, outRegCodeMonth );
	}

	return bRegCodeGood;
}


void RegInfoReader::ProcessEudoraInfoAttachment(
	const char *		szAttachmentPath)
{
	CString				szFirstName, szLastName, szRegCode, szProfileID, szMailedTo;
	bool				bDeleteProfileID;
	EudoraInfoEnum		eudoraInfoType =
			InternalGetInfoFromAttachment(szAttachmentPath, szFirstName, szLastName, szRegCode, szProfileID, szMailedTo, bDeleteProfileID);

	switch (eudoraInfoType)
	{
		case rs_Profile:
			//	Currently do nothing when the user clicks if it's a delete profile ID attachment.
			if ( !bDeleteProfileID && !szProfileID.IsEmpty() )
			{
				const char *	szPrevProfileID = GetIniString(IDS_INI_PROFILE);
				
				if (szProfileID != szPrevProfileID)
				{
					DisplayProfileIDConfirmationDialog( IDS_PROFILE_CLICK_CONFIRMATION_MSG,
														szFirstName, szLastName, szMailedTo, szProfileID );
				}
				else
				{
					CString		szMessage;

					CProfileConfirmationDialog::FormatMessageString( IDS_PROFILE_CLICK_NOT_NEW,
																	 szFirstName, szLastName, szMailedTo, szMessage);
					AfxMessageBox(szMessage);
				}
			}
			break;

		case rs_Reg:
			{
				SharewareModeType	regMode;
				bool				bRegCodeGood =
						QCSharewareManager::IsValidRegistrationInfo( szFirstName, szLastName, szRegCode, &regMode );

				UINT		nMessageTitleStringID = bRegCodeGood ?
													IDS_REG_DIALOG_MSG_TITLE_THANKS : IDS_REG_DIALOG_MSG_TITLE_INVALID;
				UINT		nMessageStringID = bRegCodeGood ?
											   IDS_REG_DIALOG_MSG_AUTO : IDS_REG_DIALOG_MSG_INVALID;

				CRegistrationCodeDlg	dlg(szFirstName, szLastName, szRegCode, nMessageTitleStringID, nMessageStringID);
				int			nResult = dlg.DoModal();

				if ( bRegCodeGood && (nResult == IDOK) && (regMode == SWM_MODE_PRO) &&
					 QCSharewareManager::GetNeedsPaidRegistrationNag() &&
					 (dlg.m_FirstName == szFirstName) && (dlg.m_LastName == szLastName) && (dlg.m_Code == szRegCode) )
				{
					//	This shouldn't happen (because we already turn off the flag when the email message
					//	is received), but if it does, we should turn off the ini flag so that we stop
					//	nagging pro users.
					QCSharewareManager::SetNeedsPaidRegistrationNag(false);
				}
			}
			break;

		case rs_Unknown:
		default:
			//	The type is unknown to us, this shouldn't happen!
			ASSERT(0);
			break;
	}
}


void RegInfoReader::DisplayProfileIDConfirmationDialog(
	UINT				nMessageID,
	const char *		szFirstName,
	const char *		szLastName,
	const char *		szMailedTo,
	const char *		szProfileID)
{
	//	Ask the user if we should set the profile info
	if (s_pProfileConfirmationDlg != NULL)
		s_pProfileConfirmationDlg->DestroyWindow();

	s_pProfileConfirmationDlg =
		DEBUG_NEW CProfileConfirmationDialog( AfxGetMainWnd(), nMessageID, szFirstName, szLastName, szMailedTo, szProfileID );
}


bool RegInfoReader::IsMailedToUser(CString & szMailedTo)
{
	//	In the absence of the Mailed-To field, assume that the attachment was mailed to the user
	bool		isMailedToUser = (szMailedTo.IsEmpty() == TRUE);
	
	if (!isMailedToUser)
	{
		//	Grab the me alias to check
		CString			szMe = CRString(IDS_ME_NICKNAME);

		if ( !szMe.IsEmpty() )
		{
			JJFile in;
			CNickname *		pMeAlias = g_Nicknames->ReadNickname(szMe, in);
			in.Close();

			//	Check to see if the me alias indicates that it's address directly to the user
			isMailedToUser = (pMeAlias != NULL) && AddressIsMe( szMailedTo, pMeAlias->GetAddresses() );
		}
	}

	if (!isMailedToUser)
	{
		//	Save the current personality, and BE SURE TO RESTORE THIS PERSONALITY
		//	BEFORE LEAVING THIS ROUTINE!!!!!
		CString			szSaveCurrentPersona = g_Personalities.GetCurrent();

		// Enumerate the list of personalities and each corresponding account.
		for (LPSTR pszNameList = g_Personalities.List();
			 pszNameList && *pszNameList;
			 pszNameList += strlen(pszNameList) + 1)
		{
			// Temporarily switch to this personality so that we can get the return address.
			if ( g_Personalities.SetCurrent(pszNameList) )
			{
				if ( AddressIsMe(szMailedTo, GetIniString(IDS_INI_RETURN_ADDRESS)) ||
					 AddressIsMe(szMailedTo, GetIniString(IDS_INI_POP_ACCOUNT)) )
				{
					isMailedToUser = true;
					break;
				}
			}
			else
			{
				ASSERT(0);
			}
		}

		// Last but not least, restore the current personality.
		g_Personalities.SetCurrent(szSaveCurrentPersona);
	}

	return isMailedToUser;
}


BoundaryType RegInfoReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	// grab descriptors for our message
	if (!MimeStates.GetSize())
		return (btError);

	BoundaryType			boundaryType = btEndOfMessage;
	MIMEState *				ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];
	HeaderDesc *			hd = ms->m_hd;

	// Determine the name of the file to which we will write out the registration
	// information as we parse it.
	char					outputFileName[_MAX_PATH + 1];
	*outputFileName = 0;
	AttributeElement *		pAttrEl = hd->GetAttribute(CRString(IDS_MIME_CONTENT_DISP_FILENAME));
	if (!pAttrEl)
		pAttrEl = hd->GetAttribute(CRString(IDS_MIME_NAME));
	if (pAttrEl)
		strncpy(outputFileName, pAttrEl->m_Value, _MAX_PATH - 1);
	if (!*outputFileName)
		strncpy(outputFileName, ((MIMEState*)MimeStates[0])->m_hd->subj, _MAX_PATH - 1);
	if (!*outputFileName)
		strncpy(outputFileName, CRString(IDS_REG_INFO_NAME), _MAX_PATH - 1);

	//	Look for a preexisting .3 extension, if any
	char *	pPeriod = strrchr(outputFileName, '.');

	if ( (pPeriod == NULL) || (static_cast<size_t>(pPeriod - outputFileName) != (strlen(outputFileName) - 4)) )
	{
		//	There's no preexisting .3 extension. Default to a text extension.
		strcat(outputFileName, ".txt");
	}

	JJFile *				attachFile = OpenAttachFile(outputFileName);
	JJFile *				outputFile = (attachFile != NULL) ? attachFile : g_pPOPInboxFile;

	// Find our birth mother so that we can detect our boundary
	MIMEState* parentMS = NULL;
	if (MimeStates.GetSize() > 1)
		parentMS = (MIMEState*)MimeStates[MimeStates.GetUpperBound() - 1];

	//	Parsing the information also writes out the attachment for future use.
	RegInfoLineReader		regInfoLineReader(ms->m_LineReader, parentMS);
	Lex822State				l822s(&regInfoLineReader, true, outputFile);
	CString					firstName, lastName, regCode, szProfileID, szMailedTo;
	SharewareModeType		newRegMode;
	int						regMonthCode, newRegMonthCode;
	bool					bDeleteProfileID;
	
	EudoraInfoEnum			eudoraInfoType = GetRegInfoFromLineReader( &regInfoLineReader, true, outputFile,
																	   firstName, lastName, regCode,
																	   &szProfileID, &szMailedTo, &bDeleteProfileID,
																	   NULL, NULL );
	boundaryType = regInfoLineReader.GetBoundaryType();
	
	//	Remove the content type and header if the header was understood
	if ( (MimeStates.GetSize() > 1) && ms->IsGrokked() )
		g_pPOPInboxFile->Truncate(ms->m_hd->lDiskStart);

	switch (eudoraInfoType)
	{
		case rs_Unknown:
			//	No special icon, it's displayed as an ordinary attachment
			if (attachFile != NULL)
				WriteAttachNote(attachFile, IDS_ATTACH_CONVERTED);
			break;
		
		case rs_Profile:
			//	Mark it as a Eudora info attachment
			if (attachFile != NULL)
				WriteAttachNote(attachFile, IDS_ATTACH_EUDORA_INFO);
			
			//	If there is a profile code, the message is addressed directly to the user,
			//	and the user is in sponsored mode or a deadbeat then process the profile
			//	code automatically.
			if ( (szProfileID != "") && IsMailedToUser(szMailedTo) &&
				 ((GetSharewareMode() == SWM_MODE_ADWARE) || IsDeadbeat()) )
			{
				const char *	szPrevProfileID = GetIniString(IDS_INI_PROFILE);

				if (bDeleteProfileID)
				{
					//	We're deleting a profile ID, delete it if the profile ID we're being
					//	told to delete matches the one that we had in the INI.
					if ( szPrevProfileID && *szPrevProfileID && (szProfileID == szPrevProfileID) )
					{
						//	Clear the profile info.
						SetIniString(IDS_INI_PROFILE, "");

						// Make sure profile is saved in ini right now.
						FlushINIFile();
					}
				}
				else
				{
					//	We're adding a profile ID
					if (szPrevProfileID && *szPrevProfileID)
					{
						//	We already had a profile ID, make sure that the new profile ID is really new
						//	- otherwise silently ignore it.
						if (szProfileID != szPrevProfileID)
						{
							DisplayProfileIDConfirmationDialog( IDS_PROFILE_AUTO_CONFIRMATION_MSG,
																firstName, lastName, szMailedTo, szProfileID );
						}
					}
					else
					{
						//	No previous profile ID, silently accept the new one.
						//	Save the profile info. This assumes that it's a string with no CRLF.
						SetIniString(IDS_INI_PROFILE, szProfileID);

						// Make sure profile is saved in ini right now.
						FlushINIFile();
					}
				}
			}
			break;

		case rs_Reg:
			{
				//	It's a reg info attachment so check for the special mime parameter regcode=yes.
				pAttrEl = hd->GetAttribute( CRString(IDS_MIME_REG_VALID) );
				
				bool	bRegGood = (pAttrEl != NULL) &&
								   (pAttrEl->m_Value.CompareNoCase(CRString(IDS_REG_VALID_VALUE)) == 0);
				if (bRegGood)
				{
					//	It's a good reg info attachment, mark it as a Eudora info attachment
					if (attachFile != NULL)
						WriteAttachNote(attachFile, IDS_ATTACH_EUDORA_INFO);
					
					//	Check to see if it's addressed directly to the user and that the user
					//	hasn't turned off reg code dialog display on arrival before automatically
					//	bringing up a dialog.
					if ( IsMailedToUser(szMailedTo) &&
						 (GetIniShort(IDS_INI_NO_REG_CODE_DIALOG_ON_ARRIVAL) == 0) )
					{
						//	Check to make sure the reg info in the attachment is good
						bRegGood = QCSharewareManager::IsValidRegistrationInfo(
										firstName, lastName, regCode, &newRegMode, &newRegMonthCode);	
						if (bRegGood)
						{
							if ( (newRegMode == SWM_MODE_PRO) && QCSharewareManager::IsPaidModeOK() &&
								 QCSharewareManager::GetNeedsPaidRegistrationNag() &&
								 (firstName.CompareNoCase(QCSharewareManager::GetFirstNameForMode(SWM_MODE_PRO)) == 0) &&
								 (lastName.CompareNoCase(QCSharewareManager::GetLastNameForMode(SWM_MODE_PRO)) == 0) &&
								 QCSharewareManager::AreRegCodesEqual(regCode, QCSharewareManager::GetRegCodeForMode(SWM_MODE_PRO)) )
							{
								//	Turn off the ini flag so that we stop nagging pro users
								QCSharewareManager::SetNeedsPaidRegistrationNag(false);
							}
							else
							{
								//	Register the user automatically if the user is not registered for the
								//	mode associated with the registration code and the new registration
								//	mode is higher than the old registration mode.
								//  Show dialog if the new registration code has a higher month code than the
								//  existing registration code.
								if ( (!QCSharewareManager::IsRegisteredForMode(newRegMode, &regMonthCode)) || (newRegMonthCode >= regMonthCode) )
								{
									CRegistrationCodeDlg	dlg( firstName, lastName, regCode,
																 IDS_REG_DIALOG_MSG_TITLE_THANKS, IDS_REG_DIALOG_MSG_AUTO );
									int		nResult = dlg.DoModal();

									if ( (nResult == IDOK) && (newRegMode == SWM_MODE_PRO) &&
										 QCSharewareManager::IsPaidModeOK() &&
										 QCSharewareManager::GetNeedsPaidRegistrationNag() )
									{
										//	Turn off the ini flag so that we stop nagging pro users
										//	The difference between this and the above check is that we don't
										//	care if the reg code is the same or not, they received a pro reg
										//	code via email and accepted it - we should stop nagging them.
										QCSharewareManager::SetNeedsPaidRegistrationNag(false);
									}
								}
							}
						}
						else if ( !QCSharewareManager::IsRegisteredForCurrentMode() )
						{
							//	Report error only if they aren't already registered.
							CRegistrationCodeDlg	dlg( firstName, lastName, regCode,
														 IDS_REG_DIALOG_MSG_TITLE_INVALID, IDS_REG_DIALOG_MSG_INVALID );
							dlg.DoModal();
						}
					}
				}
				else
				{
					//	The reg info didn't have the special mime parameter.
					//	Delete the file and don't register the user.
					if (attachFile != NULL)
						attachFile->Delete();
					
					//	Write a note explaining what happened.
					g_pPOPInboxFile->EnsureNewline();
					g_pPOPInboxFile->PutLine( CRString(IDS_ERR_INVALID_REGCODE_RECEIVED_MISSING_PARAM) );
				}
			}
			break;
	}

	delete attachFile;

	return boundaryType;
}


// RegInfoLineReader
//
// Class which provides a line reader that detects a given mime boundary
// and returns 0 bytes read to indicate this. This allows us to stop
// the Lex822State parser when it hits the mime boundary.
//
long RegInfoLineReader::ReadLine(char* buf, LONG bSize)
{
	long	lNumBytesRead = 0;
	
	if (m_LineReader != NULL)
	{
		lNumBytesRead = m_LineReader->ReadLine(buf, bSize);

		if ( (lNumBytesRead > 0) && (m_ParentMS != NULL) )
		{
			m_BoundaryType = m_ParentMS->IsBoundaryLine(buf);
			
			if (m_BoundaryType != btNotBoundary)
				lNumBytesRead = 0;
		}
	}

	return lNumBytesRead;
}


// RegInfoFileLineReader
//
// Class which provides a file line reader that returns an extra blank line.
// This avoid problems with Lex822State::LexFill adding ".CRLF" to the buffer
// to indicate the end of the header area, which previously caused the last
// line to be interpreted incorrectly when the file did not end with a CRLF.

long RegInfoFileLineReader::ReadLine(char* buf, LONG bufSize)
{
	long	lNumBytesRead = 0;
	
	if (!bReadEOF)
	{
		lNumBytesRead = FileLineReader::ReadLine(buf, bufSize);

		bReadEOF = (lNumBytesRead == 0);
		if ( bReadEOF && (bufSize > 1) )
		{
			lNumBytesRead = 2;
			buf[0] = '\r';
			buf[1] = '\n';
		}
	}

	return lNumBytesRead;
}
