// GUIUTILS.H
//
// Various interface utilities
//
// Copyright (c) 1991-2001 by QUALCOMM, Incorporated
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

#include <stdarg.h>
#include <afxcoll.h>
#include "resource.h"

class CTocDoc;
class QCMailboxCommand;

/////////////////////////////////////////////////////////////////////////////
// Mailbox Score object for Transfer to Selection with fuzzy matches
class CMailboxScore : public CObject
{
public:
	CMailboxScore(QCMailboxCommand *pCommand, int iScore);
	~CMailboxScore();

	QCMailboxCommand		*m_pCommand;
	int						 m_iScore;
};

/////////////////////////////////////////////////////////////////////////////
// AlertDialog() displays an message and asks the user to hit a button
//
void CheckForAutoOk( BOOL bOn );
int AlertDialog(UINT DialogID, ...);

/////////////////////////////////////////////////////////////////////////////
// ErrorDialog() displays an error message and asks the user to hit the OK button
//
void ErrorDialog(UINT StringID, ...);

/////////////////////////////////////////////////////////////////////////////
// WarnDialog() displays a warning and asks the user whether or not they want
// to continue.  There's also a checkbox in the dialog allowing the user to
// never see the warning again.
//
int WarnDialog(UINT IniID, UINT StringID, ...);
int WarnUrlDialog(CString szTitle, CString szText, CString szUrlTxt, CString szUrlSite, int iIcon = IDD_SETTINGS_GET_ATTEN, CWnd* pParent = NULL);
int WarnYesNoDialog(UINT IniID, UINT StringID, ...);
int WarnOneOptionCancelDialog(UINT IniID, UINT StringID, UINT BtnTxtID, ...);
int WarnOneOptionCancelDialogWithDefButton(UINT IniID, UINT StringID, UINT BtnTxtID, UINT nDefaultButtonID, ...);
int WarnTwoOptionsCancelDialog(UINT IniID, UINT StringID, UINT button1, UINT button2,  ...);
int WarnMoodDialog(UINT IniID,UINT nNeverID, CString Message, int nMoodScore, ...);


/////////////////////////////////////////////////////////////////////////////
// NoteDialog() displays a message, giving the user the chance to
// confirm the message.  There's also a checkbox in the dialog
// allowing the user to never see the message again.
//
int NoteDialog(UINT IniID, UINT StringID, ...);

/////////////////////////////////////////////////////////////////////////////
// YesNoDialog() displays a message, giving the user the chance to answer yes or no
// to the message. There's also a checkbox in the dialog
// allowing the user to never see the message again.  The response is stored if the
// latter case is used
//
int YesNoDialog(UINT WarnID, UINT responseID, UINT StringID, ...);

// A Yes/no dialog without an INI dependancies or checkbox
int YesNoDialogPlain(UINT StringID, ...);

// Is the file in the attachment directory?
bool	IsInAttachmentDir(
			LPCTSTR		Filename);


// Is the file a program or other file type that can contain executable content?
bool	IsProgram(
			LPCTSTR		Filename,
			UINT		nINIAddExtensions = IDS_INI_WARN_PROGRAM_EXTENSIONS,	// the add string now
			UINT		nINIRemoveExtensions = IDS_INI_WARN_LAUNCH_REMOVE);

// Do a file command like opening the given filename
// (with its associated app, users choice, or an Explorer window),
// or deleting the given filename.
BOOL		DoFileCommand(
					const char *		Filename,
				    int					command = 0,
					bool				bWarnIfOpeningAndNotInAttachmentDir = true,
					int					extraFlags = 0 );
BOOL SaveAttachmentSomewhere(const char *Filename, char *newPath=NULL );

// Some functions for telling us about keyboard presses
//
inline BOOL ShiftDown()
	{ return (GetKeyState(VK_SHIFT) < 0? TRUE : FALSE); }
inline BOOL AsyncShiftDown()
	{ return (GetAsyncKeyState(VK_SHIFT) < 0? TRUE : FALSE); }
inline BOOL CtrlDown()
	{ return (GetKeyState(VK_CONTROL) < 0? TRUE : FALSE); }
inline BOOL AsyncCtrlDown()
	{ return (GetAsyncKeyState(VK_CONTROL) < 0? TRUE : FALSE); }
int EscapePressed(int Repost = FALSE);
BOOL AsyncEscapePressed(BOOL bReset = FALSE);

void HesiodErrorDialog(int nHesiodError);

// TODO: Move these to a more appropriate location

struct ActionInfo {
	ActionInfo (const TCHAR* const name, const unsigned long flags )
		: name (name), flags (flags)
	{};

	const TCHAR* const name;		// The Action's name
	const unsigned long flags;		// The bit flags that determine the query parts
};

enum QUERY_PARTS	{ QUERY_PART_PLATFORM		 = 0x00000001,
					  QUERY_PART_PRODUCT		 = 0x00000002,
					  QUERY_PART_VERSION		 = 0x00000004,
					  QUERY_PART_DISTRIBUTOR_ID	 = 0x00000008,
					  QUERY_PART_EUDORA_MODE	 = 0x00000010,
					  QUERY_PART_REAL_NAME		 = 0x00000020,
					  QUERY_PART_EMAIL_ADDRESS	 = 0x00000040,
					  QUERY_PART_REG_FIRST_NAME	 = 0x00000080,
					  QUERY_PART_REG_LAST_NAME	 = 0x00000100,
					  QUERY_PART_REG_CODE		 = 0x00000200,
					  QUERY_PART_OLD_REG_CODE	 = 0x00000400,
					  QUERY_PART_PROFILE		 = 0x00000800,
					  QUERY_PART_AD_ID			 = 0x00001000,
					  QUERY_PART_DESTINATION_URL = 0x00002000,
					  QUERY_PART_TOPIC			 = 0x00004000,
					  QUERY_PART_REG_LEVEL		 = 0x00008000,
					  QUERY_PART_LANGUAGE		 = 0x00010000
					};

#define ACTION_PAY_STRING				_T("pay")
#define ACTION_REGISTER_FREE_STRING		_T("register-free")
#define ACTION_REGISTER_AD_STRING		_T("register-ad")
#define ACTION_REGISTER_PAID_STRING		_T("register-box")
#define ACTION_REGISTER_50BOX_STRING	_T("register-50box")
#define ACTION_LOST_CODE_STRING			_T("lostcode")
#define ACTION_UPDATE_STRING			_T("update")
#define ACTION_ARCHIVED_STRING			_T("archived")
#define ACTION_PROFILE_STRING			_T("profile")
#define ACTION_PROFILE_FAQ_STRING		_T("profileidFAQ")
#define ACTION_SUPPORT_STRING			_T("support")
#define ACTION_SUPPORT_NO_TOPIC_STRING	_T("support")
#define ACTION_INTRO_STRING				_T("intro")
#define ACTION_SEARCH_STRING			_T("search")
#define ACTION_SEARCH_PARAM_LABEL		_T("&query=")
#define ACTION_SETTINGSHELP_STRING		_T("helptext")
#define ACTION_SETTINGSHELP_PARAM_LABEL	_T("&panel=")
#define ACTION_SITE_STRING				_T("site")

const ActionInfo  ACTION_PAY				= ActionInfo (  ACTION_PAY_STRING,
															(   QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_FIRST_NAME
					  										  | QUERY_PART_REG_LAST_NAME
					  										  | QUERY_PART_REG_CODE
					  										  | QUERY_PART_OLD_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														  );
const ActionInfo  ACTION_REGISTER_FREE		= ActionInfo (  ACTION_REGISTER_FREE_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_FIRST_NAME
					  										  | QUERY_PART_REG_LAST_NAME
					  										  | QUERY_PART_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_REGISTER_AD		= ActionInfo (  ACTION_REGISTER_AD_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_FIRST_NAME
					  										  | QUERY_PART_REG_LAST_NAME
					  										  | QUERY_PART_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_REGISTER_PAID		= ActionInfo (  ACTION_REGISTER_PAID_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_FIRST_NAME
					  										  | QUERY_PART_REG_LAST_NAME
					  										  | QUERY_PART_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_REGISTER_50BOX		= ActionInfo (  ACTION_REGISTER_50BOX_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_LOST_CODE			= ActionInfo (  ACTION_LOST_CODE_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_FIRST_NAME
					  										  | QUERY_PART_REG_LAST_NAME
					  										  | QUERY_PART_REG_CODE
					  										  | QUERY_PART_OLD_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_UPDATE				= ActionInfo (  ACTION_UPDATE_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REG_LEVEL
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_ARCHIVED			= ActionInfo (  ACTION_ARCHIVED_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REG_LEVEL
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_PROFILE			= ActionInfo (  ACTION_PROFILE_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EMAIL_ADDRESS
															  | QUERY_PART_PROFILE
															  | QUERY_PART_LANGUAGE
															)
														 );

const ActionInfo  ACTION_PROFILE_FAQ		= ActionInfo (  ACTION_PROFILE_FAQ_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
															  | QUERY_PART_PROFILE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_SUPPORT			= ActionInfo (  ACTION_SUPPORT_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_SUPPORT_NO_TOPIC	= ActionInfo (  ACTION_SUPPORT_NO_TOPIC_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
					  										  | QUERY_PART_EUDORA_MODE
					  										  | QUERY_PART_REAL_NAME
					  										  | QUERY_PART_EMAIL_ADDRESS
					  										  | QUERY_PART_REG_FIRST_NAME
					  										  | QUERY_PART_REG_LAST_NAME
					  										  | QUERY_PART_REG_CODE
					  										  | QUERY_PART_OLD_REG_CODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_INTRO				= ActionInfo (  ACTION_INTRO_STRING,
															(	QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_SEARCH_WEB			= ActionInfo (  ACTION_SEARCH_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
															  | QUERY_PART_EUDORA_MODE
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_SETTINGSHELP		= ActionInfo (  ACTION_SETTINGSHELP_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
															  | QUERY_PART_LANGUAGE
															)
														 );
const ActionInfo  ACTION_SITE				= ActionInfo (  ACTION_SITE_STRING,
															(	QUERY_PART_PLATFORM
															  | QUERY_PART_PRODUCT
															  | QUERY_PART_VERSION
															  | QUERY_PART_DISTRIBUTOR_ID
															  | QUERY_PART_LANGUAGE
															)
														 );

int GetGMTOffset();
BOOL IsOffline();
BOOL LaunchURL( LPCSTR in_szURL, LPCSTR in_szLinkText = NULL, int in_command = 0 );
BOOL LaunchURLNoHistory( LPCSTR szURL, LPCSTR szLinkText = NULL, int command = 0 );
BOOL LaunchURLWithQuery( LPCSTR szURL, const ActionInfo& action,
						 LPCSTR topicOrSpecialParam = NULL,
						 const char *regCode = NULL, const char *firstName = NULL,
						 const char *lastName = NULL, const char *regLevel = NULL,
						 const char *specialParamLabel = NULL );
BOOL ConstructURLWithQuery( CString& theURL, const ActionInfo& action,
							LPCSTR topicOrSpecialParam = NULL,
							const char *regCode = NULL, const char *firstName = NULL,
							const char *lastName = NULL, const char *regLevel = NULL,
							const char *specialParamLabel = NULL );
BOOL GetJumpURL(CString *theURL);
BOOL URLIsValid( LPCSTR	szURL );
CString EscapeURL( LPCSTR	szURL );
CString UnEscapeURL( LPCSTR	szURL );
CString EscapePercentsInURL( LPCTSTR szIn);
BOOL FitTransparentBitmapToHeight(CBitmap& theBitmap, int nTargetHeight);
BOOL CopyTextToClipboard(const char* String, HWND hWndNewOwner);
BOOL IsForegroundApp();
CString ExtStringArrayToString(CStringArray& mylist);
bool ExtStringToStringArray(CString Extensions, CStringArray& mylist);
bool ContainsString(CStringArray& mylist, LPCTSTR str);
bool Difference(CStringArray& list1, CStringArray& list2, CStringArray& diff);

void LeftClickAttachment(CWnd* pWnd, CPoint pt, LPCTSTR Attach);

BOOL TransferToSelection(const char *szName, CView *pView = NULL);
void GenerateTransferToMenus(CMenu* pMenu, CString &strText, CObArray *poaTransferMatches);

void				OpenFindMessagesWindow(
							const char *		szMailboxName = NULL,
							const char *		szSearchPhrase = NULL,
							bool				bSelectEvenIfPreviousSelection = false,
							bool				bSelectParentFolder = false,
							bool				bSelectAll = false,
							bool				bStartSearch = false);

BOOL SyncPlayMedia(LPCTSTR pFilename, BOOL bDisableMainWindow = FALSE);
HWND AsyncPlayMedia(LPCTSTR pFilename, HWND hwndOwner = NULL);

// MAPI install/uninstall routines with error reporting
#include "mapiinst.h"
CMapiInstaller::Status MAPIInstall();
CMapiInstaller::Status MAPIUninstall(BOOL bShuttingDown = FALSE);
