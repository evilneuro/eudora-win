// EUDORA.CPP
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

#include "stdafx.h"

#include <afxcmn.h>
#include <afxrich.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <afxmt.h>
#include <time.h>

#include "QCUtils.h"

#include "resource.h"
#include "mainfrm.h"
#include "mdichild.h"
#include "mdidrag.h"
#include "doc.h" 
#include "textfile.h"
#include "rs.h"
#include "debug.h"
#include "cursor.h"
#include "fileutil.h"
#include "guiutils.h"
#include "QCWorkerSocket.h"
#include "summary.h"
#include "tocdoc.h"
#include "tocview.h"
#include "font.h"
#include "nickdoc.h"
#include "signatur.h"
#include "compmsgd.h"
#include "utils.h"
#include "TipOfDayDlg.h"
#include "MoodWatch.h"

#include "RegInfoReader.h"
#include "RegistrationCodeDlg.h"

#include "TridentPreviewView.h"	// Just so I can use IsKindOf() on a window when sending mousewheels.
#include "CompMessageFrame.h"
#include "ReadMessageFrame.h"

#include "pop.h"
#include "HostList.h"

#include "headervw.h"
#include "controls.h"
#include "filtersd.h"
#include "filtersv.h"

#include "SearchDoc.h"
#include "SearchView.h"

#include "address.h"

#include "eumapi.h"
#include "ddeclien.h"
#include "ddeserve.h"

#include "ems-wglu.h"
#include "trnslate.h"

#include "RInstDlg.h"
#include "persona.h"

#include "mtransop.h"

#include "AboutDlg.h"
#include "ClipboardMgr.h"

#include "ReadMessageDoc.h"

// OG
#include "PaigeEdtView.h"
#include "TextFileView.h"
#include "PgCompMsgView.h"
#include "PgFixedCompMsgView.h"
#include "PgDocumentFrame.h"
#include "eudora.h"

// IMAP
#include "ImapAccountMgr.h"
#include "ImapMailMgr.h"
// END IMAP

#include "QCSharewareManager.h"
#include "QCLabelDirector.h"
#include "QCCommandStack.h"
#include "QCAutomationDirector.h"
#include "QCMailboxDirector.h"
#include "QCPluginDirector.h"
#include "QCRecipientDirector.h"
#include "QCSignatureDirector.h"
#include "QCStationeryDirector.h"
#include "QCPersonalityDirector.h"
#include "Automation.h"
#include "TraceFile.h"
#include "WizardPropSheet.h"
#include "QCGraphics.h"
#include "QCImageList.h"
#include "TocFrame.h"
#include "AutoCompleteSearcher.h"
#include "ExceptionHandler.h"
#include "JunkMail.h"

#include "QCTaskManager.h"
#include "SpoolMgr.h"
#include "QCRas.h"

#include "AuditEvents.h"
#include "AuditLog.h"
#include "CNagManager.h"
#include "..\Version.h"

#include "ContentConcentrator.h"
#include "SearchManager.h"
#include "LinkHistoryManager.h"
#include "FaceTime.h"

#include "MoreHelpWnd.h"
#include "CUpdateNagTask.h"

#include "nsprefs.h"

//Usage Stats
#include "StatMng.h"
#include "UsgStatsView.h"
#include "UsgStatsDoc.h"

#include "ExtLaunchMgr.h"

#include "PaymentAndRegistrationDlg.h"

#include "EmoticonManager.h"
#include "GoogleDesktopComponentRegistration.h"
#include "BossProtector.h"

#include "DebugNewHelpers.h"


// the command directors
QCCommandStack			g_theCommandStack;
QCAutomationDirector	g_theAutomationDirector;
QCMailboxDirector		g_theMailboxDirector;
QCPluginDirector		g_thePluginDirector;
QCRecipientDirector		g_theRecipientDirector;
QCSignatureDirector		g_theSignatureDirector;
QCCommandDirector		g_theSpellingDirector;
QCStationeryDirector	g_theStationeryDirector;
QCPersonalityDirector	g_thePersonalityDirector;
EmoticonDirector		g_theEmoticonDirector;
QCCommandDirector		g_theSearchDirector;


#ifdef EXPIRING
	// The Evaluation Time Stamp object
	#include "timestmp.h"
	CTimeStamp	g_TimeStamp;
#endif

CLIPFORMAT cfEmbeddedObject;
CLIPFORMAT cfRTF;
CLIPFORMAT cfRTO;
CLIPFORMAT cfRTWO; // without objects


// The Eudora Personality object
CPersonality	g_Personalities;


// overwrite mode for editing is global state; we make this a simple global,
// so we're not calling AfxGetApp for every single keystroke the user types.
bool g_bOverwriteMode = false;


//DEBUG: to help with LMOS testing.
//Set this to true to test non-uidl servers; Eudora would not use UIDL cmd if this flag is set
bool g_bDisableUIDL = false;

// quicktime shutdown (so we don't have to include the header)
extern "C" void PgMultimediaShutdown();


//	By not making this method a member of CEudoraApp, we avoid needing to include
//	QCSharewareManager.h in eudora.h (for SharewareModeType enum).
void AskUserToConfirmRegCodeAtStartup(
	const char *		szFirstName,
	const char *		szLastName,
	const char *		szNewRegCode,
	SharewareModeType	regMode,
	bool				bEudoraNeedsPaidRegistration);


//
// The Eudora DDE Server object.
//
static CDDEServer theDDEServer;

// Support for the Eudora.ini reset.
static void ResetEudoraIni(void);	// called at shutdown
long ResetIniFileOnExit = 0;		// gets set to flag that Eudora.ini should be reset

bool g_bDefaultMailto = FALSE;


/////////////////////////////////////////////////////////////////////////////
// CEudoraApp commands


// App command to run the dialog
void CEudoraApp::OnAppAbout()
{
	CAboutDlg aboutDlg(FALSE);
	
	aboutDlg.DoModal();
}

void CEudoraApp::OnTipOfTheDay()
{
	if (gbAutomationExternalStart) return;

	short nShowTipOfTheDay = ::GetIniShort(IDS_INI_SHOW_TIP_OF_THE_DAY);
	short nCurrentTip = ::GetIniShort(IDS_INI_CURRENT_TIP_OF_THE_DAY);

	CString strTipFilename(::ExecutableDir);
	ASSERT(strTipFilename.GetLength() > 0);
	ASSERT(strTipFilename[strTipFilename.GetLength() - 1] == '\\');
	strTipFilename += "eudora.tip";

	QCTipOfDayDlg dlg(strTipFilename, ++nCurrentTip, nShowTipOfTheDay);
	dlg.DoModal();
	
	::SetIniShort(IDS_INI_SHOW_TIP_OF_THE_DAY, short(dlg.GetShowAtStartup()));
	::SetIniShort(IDS_INI_CURRENT_TIP_OF_THE_DAY, short(dlg.GetCurrentTip()));
}

//	Displays dialog asking user to confirm the reg code that was found in
//	RegCode.dat at startup.
void AskUserToConfirmRegCodeAtStartup(
	const char *		szFirstName,
	const char *		szLastName,
	const char *		szNewRegCode,
	SharewareModeType	regMode,
	bool				bEudoraNeedsPaidRegistration)
{
	CRegistrationCodeDlg	dlg( szFirstName, szLastName, szNewRegCode,
								 IDS_REG_DIALOG_MSG_TITLE_THANKS, IDS_REG_DIALOG_MSG_AUTO );
	int		nResult = dlg.DoModal();
	if ( (nResult == IDOK) && (regMode == SWM_MODE_PRO) &&
		 QCSharewareManager::AreRegCodesEqual(szNewRegCode, dlg.m_Code) )
	{
		//	The user confirmed the paid reg code that we read from RegCode.dat.
		//	Set the paid registration nag flag appropriately
		//	(i.e. on if "Eudora-Needs-Registration: yes" was present) so that
		//	we know whether or not to nag paid users.
		QCSharewareManager::SetNeedsPaidRegistrationNag(bEudoraNeedsPaidRegistration);
	}
}

//	Process RegCode.dat at startup
//	RegCode processing:
//	* Prefers a RegCode.dat in the EudoraDir (where the INI file, etc. are located)
//	* Requires paid reg code if there was a bad one at startup
//	* Only allows reg code if the month value is greater than *or equal* the
//	  reg code, if any, that the user already has for the current mode
//
//	Distributor ID processing prefers the RegCode.dat in the ExecutableDir (if present).
void CEudoraApp::OnCheckRegCodeFile()
{
	//	Register from "RegCode.dat" if it exists, its information is good, and
	//	its information differs from the current registration information
	CString		szRegInfoStartupFileName;
	GetRegInfoFilePath(szRegInfoStartupFileName);

	bool				bWasBadProRegCodeAtStartup = QCSharewareManager::WasBadProRegCodeAtStartup();
	bool				bWasExpiredProRegCodeAtStartup = QCSharewareManager::WasExpiredProRegCodeAtStartup();
	bool				bGoodRegInfo = false;
	bool				bGotGoodRegInfo = false;
	SharewareModeType	regMode = SWM_MODE_ADWARE;
	CString				szFirstName, szLastName, szNewRegCode;
	CString				szDistributorID = "";
	CString				szPrevDistributorID = "";
	bool				bEudoraNeedsPaidRegistration;
	int					newRegCodeMonth, currentRegCodeMonth;
	SharewareModeType	regModeInRegistry = SWM_MODE_ADWARE;
	CString				szFirstNameFromRegistry, szLastNameFromRegistry, szRegCodeFromRegistry;
	bool				bRegCodeFromRegistryGood = false;

	if ( !QCSharewareManager::IsPaidModeOK() )
	{
		HKEY	hKey;

		//	Check the registry for a paid mode reg code
		if ( ::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Qualcomm\\Eudora\\Check"), 0, KEY_READ, &hKey) == ERROR_SUCCESS )
		{
			char	szData[512];
			DWORD	dwKeyDataType;
			DWORD	dwDataBufSize;

			//	Get the first name
			dwDataBufSize = sizeof(szData);
			bRegCodeFromRegistryGood = 
				( ::RegQueryValueEx(hKey, _T("FName"), NULL, &dwKeyDataType, (LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS ) &&
				(dwKeyDataType == REG_SZ);
			if (bRegCodeFromRegistryGood)
				szFirstNameFromRegistry = szData;

			//	Get the last name
			if (bRegCodeFromRegistryGood)
			{
				dwDataBufSize = sizeof(szData);
				bRegCodeFromRegistryGood = 
					( ::RegQueryValueEx(hKey, _T("LName"), NULL, &dwKeyDataType, (LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS ) &&
					(dwKeyDataType == REG_SZ);
				if (bRegCodeFromRegistryGood)
					szLastNameFromRegistry = szData;
			}

			//	Get the reg code
			if (bRegCodeFromRegistryGood)
			{
				dwDataBufSize = sizeof(szData);
				bRegCodeFromRegistryGood = 
					( ::RegQueryValueEx(hKey, _T("RCode"), NULL, &dwKeyDataType, (LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS ) &&
					(dwKeyDataType == REG_SZ);
				if (bRegCodeFromRegistryGood)
					szRegCodeFromRegistry = szData;
			}

			if (bRegCodeFromRegistryGood)
			{
				//	Check to see if the information in the registry is valid and if it's a paid reg code
				bRegCodeFromRegistryGood =
					QCSharewareManager::IsValidRegistrationInfo( szFirstNameFromRegistry, szLastNameFromRegistry,
																 szRegCodeFromRegistry, &regModeInRegistry, NULL) &&
					(regModeInRegistry == SWM_MODE_PRO);
			}

			::RegCloseKey(hKey);
		}
	}

	//	Check for RegCode.dat in at most two locations, the EudoraDir (where the INI file, etc. are
	//	located) and ExecutableDir. The below loop will execute 1-2 times (depending on whether or
	//	not the ExecutableDir is different from the EudoraDir). A second pass will run even if
	//	we found a good reg code in the first pass, because we prefer distributor IDs from the
	//	second location.
	for (short i = 0; i < 2; i++)
	{
		if ( ::FileExistsMT(szRegInfoStartupFileName) )
		{
			//	Get the reg code, and distributor ID
			bGoodRegInfo = RegInfoReader::GetInfoFromRegCodeStartupFile(
								szRegInfoStartupFileName, szFirstName, szLastName, szNewRegCode,
								szDistributorID, regMode, bEudoraNeedsPaidRegistration, &newRegCodeMonth );
			
			//	Only bother continuing with reg code processing if a
			//	previous pass didn't come up with good reg info
			if (!bGotGoodRegInfo)
			{
				//	If the reg code is good, check to see if we really want it.
				if (bGoodRegInfo)
				{
					if (bWasBadProRegCodeAtStartup || bWasExpiredProRegCodeAtStartup || bRegCodeFromRegistryGood)
					{
						//	We only want a paid reg code (bad or expired paid reg code at startup or we're
						//	not paid registered and the registry has a paid reg code).
						bGoodRegInfo = (regMode == SWM_MODE_PRO);
					}
					else if ( QCSharewareManager::IsRegisteredForMode(regMode, &currentRegCodeMonth) )
					{
						//	There's already a good reg code for this mode. Only register with the new reg
						//	code if the month is greater than *or equal* to the reg code we already have.
						//	Anything can override 255, because it indicates a 4.x upgrade. Any other
						//	month value will be valid for at least as long as an upgrade reg code.
						bGoodRegInfo = (newRegCodeMonth >= currentRegCodeMonth) || (currentRegCodeMonth == 255);
					}
				}

				if (bGoodRegInfo)
				{
					bGotGoodRegInfo = true;

					//	Verify that the reg code is new to us. If it's not we don't display the dialog,
					//	but we still won't use the other RegCode.dat (bGotGoodRegInfo = true above)
					//	for it's reg code (if any) because we don't want a reg code in the executable
					//	directory to override the reg code in the INI directory.
					LPCTSTR		szCurrentRegCode = QCSharewareManager::GetRegCodeForMode(regMode);
					if ( (szCurrentRegCode == NULL) || !QCSharewareManager::AreRegCodesEqual(szNewRegCode, szCurrentRegCode) )
					{
						AskUserToConfirmRegCodeAtStartup( szFirstName, szLastName, szNewRegCode,
														  regMode, bEudoraNeedsPaidRegistration );
					}
					else if ( (regMode == SWM_MODE_PRO) && bEudoraNeedsPaidRegistration &&
						      !QCSharewareManager::GetCheckedForMissingCRLF() )
					{
						//	We now handle files that don't end with CRLF. It previously was a problem
						//	for box pro users - we were erroneously getting a false value for
						//	bEudoraNeedsPaidRegistration (the yes in "Eudora-Needs-Registration: yes"
						//	couldn't be recognized).
						//
						//	If we're here it's an issue and we haven't checked for the problem yet.
						//	Check for a trailing CRLF, and if there isn't any then turn on the
						//	NeedsPaidRegistrationNag flag.
						JJFile			regInfoFile;

						if ( SUCCEEDED(regInfoFile.Open(szRegInfoStartupFileName, O_RDONLY)) )
						{
							HRESULT		result = regInfoFile.Seek(-2, SEEK_END);

							if (result == S_OK)
							{
								long	lNumBytesRead;
								char	szLastTwoChars[2];
								
								result = regInfoFile.RawRead(szLastTwoChars, 2, &lNumBytesRead);

								if ( (result == S_OK) && (lNumBytesRead == 2) )
								{
									//	If the last two characters are not CRLF,
									//	then turn on the NeedsPaidRegistrationNag flag
									if ( (szLastTwoChars[0] != '\r') || (szLastTwoChars[1] != '\n') )
										QCSharewareManager::SetNeedsPaidRegistrationNag(bEudoraNeedsPaidRegistration);
								}
							}

							regInfoFile.Close();

							QCSharewareManager::SetCheckedForMissingCRLF(true);
						}
					}
				}
			}
		}

		if (i == 0)
		{
			//	If the alternate path is not different, then don't bother with a second pass.
			if ( !GetRegInfoFileAlternatePath(szRegInfoStartupFileName) )
				break;

			//	We're doing a second pass, so save the first distributor ID
			szPrevDistributorID = szDistributorID;
		}
	}

	//	For the purposes of distributor ID, we don't care if the reg code was correct
	//	or not (or even if the "Eudora-File-Type" line was missing or incorrect).
	bool	bCurrentDistributorIDGood = !szDistributorID.IsEmpty();

	if ( bCurrentDistributorIDGood || !szPrevDistributorID.IsEmpty() )
	{
		//	One of the two distributor IDs was good. Prefer the current distributor ID,
		//	which prefers the 
		if (bCurrentDistributorIDGood)
			SetIniString(IDS_INI_DISTRIBUTOR, szDistributorID);
		else
			SetIniString(IDS_INI_DISTRIBUTOR, szPrevDistributorID);

		// Make sure distributor ID is saved in ini right now.
		FlushINIFile();
	}

	if (!bGotGoodRegInfo)
	{
		if (bWasExpiredProRegCodeAtStartup)
		{
			// There was an expired paid mode reg code at startup, and we didn't get a
			// replacement from RegCode.dat or the registry. Display the Repay to allow
			// advise the user of this situation.
			CRepayDlg dlg;
			dlg.DoModal();
		}
		else if (bRegCodeFromRegistryGood)
		{
			//	We didn't get any other reg code, but there was a good one in the registry
			AskUserToConfirmRegCodeAtStartup( szFirstNameFromRegistry, szLastNameFromRegistry, szRegCodeFromRegistry,
											  regModeInRegistry, false );
		}
		else if (bWasBadProRegCodeAtStartup)
		{
			//	There was a bad pro reg code at startup, and we didn't get a replacement from
			//	RegCode.dat or the registry. Display the registation code dialog to allow the
			//	user to correct this.
			CRegistrationCodeDlg	dlg( QCSharewareManager::GetFirstNameForMode(SWM_MODE_PRO),
										 QCSharewareManager::GetLastNameForMode(SWM_MODE_PRO),
										 QCSharewareManager::GetRegCodeForMode(SWM_MODE_PRO),
										 IDS_REG_DIALOG_MSG_TITLE_INVALID, IDS_REG_DIALOG_MSG_INVALID );
			dlg.DoModal();
		}
	}
}

void CEudoraApp::OnGetHelp()
{
	NewChildDocFrame(MoreHelpTemplate);	
}

void CEudoraApp::OnReportABug()
{
	CString		strFileName		= GetTmpFileNameMT("bugreport");
	bool		bMadeTempFile	= false;
	JJFileMT	jjTmpFile;
	
	// Write to the temp file the contents of the message.
	bMadeTempFile = SUCCEEDED(jjTmpFile.Open(strFileName, O_WRONLY));
	if (bMadeTempFile)
	{
		char		szTemp[256];
		CString		strLine;

		// Write the email address.
		QCLoadString(IDS_REPORT_BUG_EMAIL_ADDRESS, szTemp, sizeof(szTemp));
		strLine.Format("To: %s", szTemp);
		jjTmpFile.PutLine(strLine);

		// Write the subject line.
		// It looks like "Subject: Bug Report [Joe Blow 50011.2.38E91C31]", where the
		// Joe Blow part comes from the current RealName and the number is the build
		// number followed by the current mode followed by the current time (seconds since
		// whenever Microsoft's t=0 was).
		strLine.Format("Subject: %s [%s %d%d%d%d.%d.%lX]",
						(LPCTSTR) CRString(IDS_REPORT_BUG_EMAIL_SUBJECT),
						(LPCTSTR) GetIniString(IDS_INI_REAL_NAME),
						EUDORA_VERSION1,
						EUDORA_VERSION2,
						EUDORA_VERSION3,
						EUDORA_VERSION4,
						GetSharewareMode(),
						time(NULL));
		jjTmpFile.PutLine(strLine);

		// Blank header/body separator line.
		jjTmpFile.PutLine();

		// Flush the headers
		jjTmpFile.Flush();

		// Insert a friendly message.
		char szText[1024];
		QCLoadString(IDS_REPORT_BUG_MSG_TEXT, szText, sizeof(szText));
		jjTmpFile.PutLine(szText);

		jjTmpFile.Close();
	}

	// Add extra headers and show the message.
	CCompMessageDoc* compDoc = NewCompDocumentWith(strFileName);
	if (compDoc && compDoc->m_Sum)
	{
		// Add any user-specified flags.
		if (GetIniShort(IDS_INI_SEND_MIME))		compDoc->m_Sum->SetFlag(MSF_MIME);
		if (GetIniShort(IDS_INI_SEND_UUENCODE))	compDoc->m_Sum->SetFlag(MSF_UUENCODE);
		if (GetIniShort(IDS_INI_USE_QP))		compDoc->m_Sum->SetFlag(MSF_QUOTED_PRINTABLE);
		if (GetIniShort(IDS_INI_WORD_WRAP))		compDoc->m_Sum->SetFlag(MSF_WORD_WRAP);
		if (GetIniShort(IDS_INI_TABS_IN_BODY))	compDoc->m_Sum->SetFlag(MSF_TABS_IN_BODY);
		if (GetIniShort(IDS_INI_KEEP_COPIES))	compDoc->m_Sum->SetFlag(MSF_KEEP_COPIES);

		// Add the reg code to the extra headers.
		compDoc->m_ExtraHeaders += "eudora-reg-code: ";
		compDoc->m_ExtraHeaders += QCSharewareManager::GetRegCodeForCurrentMode();
		compDoc->m_ExtraHeaders += "\r\n";

		// Add the reg first name to the extra headers.
		compDoc->m_ExtraHeaders += "eudora-reg-first-name: ";
		compDoc->m_ExtraHeaders += QCSharewareManager::GetFirstNameForCurrentMode();
		compDoc->m_ExtraHeaders += "\r\n";

		// Add the reg last name to the extra headers.
		compDoc->m_ExtraHeaders += "eudora-reg-last-name: ";
		compDoc->m_ExtraHeaders += QCSharewareManager::GetLastNameForCurrentMode();
		compDoc->m_ExtraHeaders += "\r\n";

		// Show the message.
		compDoc->m_Sum->Display();
	}

	// If we created a temp file, delete it.
	if (bMadeTempFile)
	{
		DeleteFile(strFileName);
	}
}

void CEudoraApp::OnUpdateTipOfTheDay(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}




/////////////////////////////////////////////////////////////////////////////
// CEudoraApp

BEGIN_MESSAGE_MAP(CEudoraApp, CWinApp)
	//{{AFX_MSG_MAP(CEudoraApp)
	ON_COMMAND(ID_FILE_CHECKMAIL, OnCheckMail)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TIP_OF_THE_DAY, OnTipOfTheDay)
	ON_COMMAND(ID_CHECK_REG_CODE_FILE, OnCheckRegCodeFile)
	ON_COMMAND(ID_HELP_TECHNICAL_SUPPORT, OnGetHelp)
	ON_COMMAND(ID_REPORT_A_BUG, OnReportABug)
	ON_UPDATE_COMMAND_UI(ID_TIP_OF_THE_DAY, OnUpdateTipOfTheDay)
	ON_UPDATE_COMMAND_UI(ID_FILE_CHECKMAIL, OnUpdateCheckMail)
	//}}AFX_MSG_MAP

	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)

	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)

// Debug Memory manager Command handlers.
// smohanty: 05/29/97.
#ifdef __DEBUG__
        ON_COMMAND(ID_DMM_DUMPLOG, DMMDumpLog)
        ON_COMMAND(ID_DMM_HIGHWATERMARK, DMMHighWatermark)
        ON_COMMAND(ID_DMM_CURRENT_ALLOC, DMMCurrentAlloc)
        ON_COMMAND(ID_DMM_HEAP_TOGGLE,  DMMHeapToggle)
        ON_UPDATE_COMMAND_UI(ID_DMM_DUMPLOG, UpdateDMMDumpLog)
        ON_UPDATE_COMMAND_UI(ID_DMM_HIGHWATERMARK, UpdateDMMHighWatermark)
        ON_UPDATE_COMMAND_UI(ID_DMM_CURRENT_ALLOC, UpdateDMMCurrentAlloc)
        ON_UPDATE_COMMAND_UI(ID_DMM_HEAP_TOGGLE, UpdateDMMHeapToggle)
#endif // __DEBUG__
END_MESSAGE_MAP()


extern BOOL  g_bPopupOn;

/////////////////////////////////////////////////////////////////////////////
// CEudoraApp construction

CEudoraApp::CEudoraApp() :
	m_LastMouseMovePoint(0,0)
{
	m_pResList = NULL;
	m_hFindHandle = INVALID_HANDLE_VALUE;
	m_fdOwner = -1;
	m_hOwner = NULL;
	m_hOurWnd = NULL;
	m_TransManager = NULL;
	m_LastOverwritePressed = 0;
	m_FaceTime = NULL;
	m_iJunkMbxStatus = JUNK_MBOX_NONE;
	m_bDoingIdleTimerProcessing = FALSE;
	m_bCallCWinAppIdle = TRUE;
	m_bRestartIdleProcessing = TRUE;

	//
	// We use this as a clue to see whether we've run our
	// CEudoraApp::InitInstance().  It's possible that the outer
	// CEudoraApp wrapper shuts things down before we ever get to
	// run our CEudoraApp::InitInstance().
	//
	::memset(&m_OSVersion, 0, sizeof(m_OSVersion));
}

CEudoraApp::~CEudoraApp()
{
}

// Debug Memory Manager command and update handlers.
#ifdef __DEBUG__
void
CEudoraApp::DMMDumpLog()
{
}

void
CEudoraApp::DMMHighWatermark()
{
	// Check if the app hasn't allocated any tocs yet
	if (!TocTemplate)
		return;

	POSITION pos = TocTemplate->GetFirstDocPosition();
	while (pos)
	{
		char buf[512];
		CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(pos);
		if (doc)
		{
			sprintf(buf, "%4d Sums\t%s\n", doc->NumSums(), doc->Name());
			OutputDebugString(buf);
		}
	}
}

void
CEudoraApp::DMMCurrentAlloc()
{
}

void
CEudoraApp::DMMHeapToggle()
{
}

void
CEudoraApp::UpdateDMMDumpLog(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void
CEudoraApp::UpdateDMMHighWatermark(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void
CEudoraApp::UpdateDMMCurrentAlloc(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void
CEudoraApp::UpdateDMMHeapToggle(CCmdUI *pCmdUI)
{
    pCmdUI->SetText((fIsHeapSolventMT() == TRUE)
		    ? "&FakeOutOfMemory" : "&UnfakeOutOfMemory");
}

#endif // __DEBUG__

void CEudoraApp::OnUpdateCheckMail(CCmdUI* pCmdUI)
{
	CString CheckMailText;
	
	GetMailCheckTime(CheckMailText);

	// Get rid of Virtual Key text
	int nLen = CheckMailText.GetLength();
	int nPos;
	if ( g_bPopupOn )	
		if ( ( nPos = CheckMailText.Find('\t')) != -1)
			nLen= nPos;

	pCmdUI->SetText(CheckMailText.Left(nLen));
}

void CEudoraApp::UpdateMailCheckLabel()
{
	CString CheckMailText;

	GetMailCheckTime(CheckMailText);

	ModifyMenu(CMainFrame::QCGetMainFrame()->GetFileMenu(), ID_FILE_CHECKMAIL,
				MF_BYCOMMAND | MF_STRING, ID_FILE_CHECKMAIL, CheckMailText);
}

void CEudoraApp::GetMailCheckTime(CString &CheckMailText)
{
	
	if (m_NextCheckMail)
	{
		char Time[64];
		::TimeDateStringMT(Time, m_NextCheckMail, FALSE);
		AfxFormatString1(CheckMailText, IDS_FILE_CHECK_MAIL_TIME, Time);
	}
	else
		CheckMailText.LoadString(IDS_FILE_CHECK_MAIL);
}

void CEudoraApp::OnCheckMail()
{
	if (ShiftDown())
	{
		CMailTransferOptionsDialog dlg;
		dlg.DoModal();
	}
	else
	{
		GetMail(kManualMailCheckBits);

		UpdateMailCheckLabel();
	}
}

void CEudoraApp::OnAppExit()
{
	// If Shift was held down while doing a File->Exit, then just save the
	// positions of the currently open windows, but don't quit
	if (ShiftDown())
		((CMainFrame*)AfxGetMainWnd())->SaveOpenWindows(FALSE);
	else
		CWinApp::OnAppExit();
}

/////////////////////////////////////////////////////////////////////////////
// CEudoraApp initialization

CMultiDocTemplate* TextFileTemplate = NULL;
CMultiDocTemplate* TocTemplate = NULL;
CMultiDocTemplate* ReadMessageTemplate = NULL;
CMultiDocTemplate* CompMessageTemplate = NULL;
CMultiDocTemplate* FindTemplate = NULL;
CMultiDocTemplate* SearchTemplate = NULL;
CMultiDocTemplate* FiltersTemplate = NULL;
CMultiDocTemplate* NicknamesTemplate = NULL;
CMultiDocTemplate* SignatureTemplate = NULL;
CMultiDocTemplate* MoreHelpTemplate = NULL;
CMultiDocTemplate* UpdateNagTemplate = NULL;
CMultiDocTemplate* UsageStatsTemplate = NULL;

//	=========================================================================================
BOOL 
CEudoraApp::InitApplication()
{
	return CWinApp::InitApplication();
}

//	=========================================================================================

BOOL CEudoraApp::InitInstance()
{
	// Guard against re-entrancy of multiple copies on the same set of mailboxes
	// by using a named mutex
	CMutex StartupMutex(FALSE, "EudoraStartupMutex");
	CSingleLock StartupMutexLock(&StartupMutex, TRUE);
	ASSERT(StartupMutexLock.IsLocked());

	//
	// Make sure we're running on a compatible OS.  
	// If not, we're outta here.
	//
	m_OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&m_OSVersion);

	if (m_OSVersion.dwMajorVersion < 4)
	{
		::AfxMessageBox(IDS_NEED_VERSION4, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	// Initialize the OLE DLLs
	if (!AfxOleInit())
	{
		::AfxMessageBox(IDP_OLE_INIT_FAILED, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	ResourceStatus resourceStatus = RESOURCE_OKAY;

// This preprocessor symbol is not used any more. The
// resource DLL is *always* used. Subhashis Mohanty. 04/03/97.

// #ifdef USE_RESOURCE_DLL
	// if release version add the default resource dll
	// ( contains resources that can be overidden via plug-ins )

	resourceStatus = LoadResourceDLL();
	switch (resourceStatus)
	{
	case RESOURCE_OKAY:
		break;
	case RESOURCE_VERSION_MISMATCH:
		if (IDNO == ::AfxMessageBox(IDS_RESDLL_MISMATCH, MB_YESNO | MB_ICONQUESTION))
			return FALSE;
		break;
	case RESOURCE_NOT_LOADED:
		::AfxMessageBox( IDS_NO_RESDLL );
		break;
	case RESOURCE_UNKNOWN:
	default:
		ASSERT(0);
		resourceStatus = RESOURCE_UNKNOWN;
		break;
	}

// #endif // USE_RESOURCE_DLL

	g_theCommandStack.NewCommandMap();

	// Check if application was run with -Embedding set by system
	if (RunEmbedded())
	{
		// Remote application has started Eudora as Automation Server

		gbAutomationRunning = true;
		gbAutomationExternalStart = true;
		
	}

	// Check if application was run with -Automation set by system
	if (RunAutomated())
	{
		// Remote application is querying Eudora interfaces
		return TRUE;
	}
	
	// Uncomment this to turn on lots of heap checking code:
	 //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_CHECK_ALWAYS_DF);

	// Enable ActiveX controls in dialogs
	// do this prior to creating any windows
	AfxEnableControlContainer();

	// Initialize the rich edit control (version 2.0 and later) before creating
	// any windows that use it. Specifically fixes bug where address book
	// tabs were missing on Windows 98 (because apparently the newer rich edit
	// control won't load correctly on Windows 98 without initializing first -
	// whereas Windows XP seemed just fine even without this call).
	AfxInitRichEdit2();

	// Set Eudora palette to the colors specified
	// in the resources plus colors for the current color sceme
	SetSafetyPalette(IDB_SAFETY_PALETTE);

	// Initialize ATL OLE Module
	_Module.InitObjectMap(m_hInstance);
	
	// Store the original command line 'coz GetDirs() in the following switch modifies it - 02/10/00
	CString csCmdLine = m_lpCmdLine;

	COMMANDLINE_TYPE CmdType = GetCommandType(m_lpCmdLine);

	switch( CmdType )
	{
		case CL_ATTACH:
		case CL_STATIONERY:
		case CL_URL:
		case CL_MESSAGE:
		case CL_MAILBOX:
			// can an existing instance handle it?
			if ( ProcessAttachmentWEI( CmdType ) )
				return FALSE;

			// set up the command line based on the registry
			if (!StartEudoraWithDefaultLocation())
				return FALSE;

			// there could be a remote instance running...
			if (IsEudoraRunning())
				return FALSE;
			break;

		default:
			
			if( gbAutomationRunning )
			{
				if( !StartEudoraWithDefaultLocation() )
				{
					return FALSE;
				}
				
				// it's not clear to me that this is really the right thing to do.
				// But, it's always worked this way so ???
				if (IsEudoraRunning())
					return FALSE;
			}
			else
			{
				// use the command line to establish the working environment
				if (!GetDirs(m_lpCmdLine))
					return FALSE;

				// If there's nothing on the command line, then look to see if there's
				// a Eudora.ini file in the same directory as the executable.  If so,
				// then use that directory, otherwise look up in the registry to see
				// what was the last Eudora directory used.
				CString CommandLine(m_lpCmdLine);
				CommandLine.TrimLeft();
				if (CommandLine.IsEmpty())
				{
					if (!FileExistsMT(INIPath))
					{
						if (!StartEudoraWithDefaultLocation())
							return FALSE;
					}
				}

				if (IsEudoraRunning())
					return FALSE;
	
				// Verify &/or create Plugins Dir & Filters Dir
				CreateDirectoryMT(EudoraDir + CRString(IDS_TRANS_DIRECTORY));
				CreateDirectoryMT(EudoraDir + CRString(IDS_FILTER_DIRECTORY));

				RegisterCommandLine(m_lpCmdLine);
			}
			break;
	}

	// Pop back the original command line in m_lpCmdLine for further usage - 02/10/00
	_tcscpy(m_lpCmdLine, csCmdLine);

	// Add Eudora resource plugins that are in our directory or directories
	LoadEPIs();

	// Load shared Eudora images lists
	LoadImageLists();

	// Now that Eudora resource plugins are loaded and in effect, initialize
	// g_Personalities so that the dominant personality name (g_Personalities.m_Dominant) and
	// the string for the personality settings section of the INI
	// (g_Personalities.m_szPersonality) are loaded.
	//
	// The fields of g_Personalities that are loaded by Init aren't used until later.
	// The above calls to IsEudoraRunning do access g_Personalities, but not in any way
	// that requires m_Dominant and m_szPersonality to be loaded (*).
	//
	// (*) IsEudoraRunning->GetIniShort->GetIniString, which calls g_Personalities.GetCurrent and
	// g_Personalities.GetProfileString. g_Personalities.GetCurrent returns an empty m_Current
	// (completely normal even after g_Personalities.Init), and g_Personalities.GetProfileString
	// recognizes that the personality being requested is by the fact that it is an empty string.
	g_Personalities.Init();

	char buf[MAX_PATH + 16];

	// Set crash dump name for both crash handler and buffer overflow handler
	sprintf(buf, "%sEudoraCrashDump.dmp", (LPCTSTR)EudoraDir);
	g_QCExceptionHandler.SetMiniDumpFileName(buf);
	g_QCExceptionHandler.EnableBufferOverflowHandler();

	if (GetIniShort(IDS_INI_EXCEPTION_HANDLING))
	{		
		// Change where the exception log will go to be the file Exception.log in the mail directory
		sprintf(buf, "%sException.log", (LPCTSTR)EudoraDir);
		g_QCExceptionHandler.SetExceptionLogFileName(buf);

		g_QCExceptionHandler.EnableCrashHandler();
	}

	// Initialize the Audit log file manager -- must be destroyed when done
	CAuditLog*	auditlogfile = DEBUG_NEW CAuditLog(EudoraDir, 
											GetIniString(IDS_INI_AUDIT_LOG_FILE_NAME)
											);
	
	ASSERT(auditlogfile != NULL);
	if (!CAuditEvents::CreateAuditEvents(auditlogfile)) {
		return (FALSE);
	}
	CAuditEvents::GetAuditEvents()->StartupEvent (	1 /* platform, 1==WINDOWS */,
													(EUDORA_VERSION1 * 100) + (EUDORA_VERSION2 * 10) + (EUDORA_VERSION3)/* version in BCD */,
													EUDORA_VERSION4 /* buildNumber as long */);

	m_FaceTime = QCFaceTime::Create();
	if (m_FaceTime)
		m_FaceTime->Start();


	// Start Collectig Stats here
	//AKN:UsgStats
	InitStats();


	// Check ini file to see if automation is enabled
	if (GetIniShort(IDS_INI_AUTOMATION_ENABLED))
	{
		AutomationEnable();
//	 	gbAutomationEnabled = true;
	}

	// Check to see if command line is "/EnableAutomation"
	CString str = m_lpCmdLine;
	if (str.CompareNoCase("/EnableAutomation") == 0)
	{
		AutomationEnable();
		return FALSE;
	}

	// Check to see if command line is "/DisableAutomation"
	if (str.CompareNoCase("/DisableAutomation") == 0)
	{
		AutomationDisable();
		return FALSE;
	}

	// make sure the Embedded dir exists
	MakeEmbeddedDir();

	RegisterIcon();
	RegisterStationery();
	RegisterMailbox();
	RegisterFormats();
	m_pszProfileName = strdup( INIPath );

#ifdef FORNOW
	// Google Desktop Search registration
	wchar_t app[_MAX_PATH + 1];
	GetModuleFileNameW(AfxGetInstanceHandle(), app, sizeof(app));
	RegisterComponentHelper(CLSID_EuApplication, L"Eudora", L"Eudora Email Application", app, 0, NULL);
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Initialize the Shareware Manager -- must be destroyed when done.
	// This needs to happen before creating the splash screen because the
	// splash screen displays what mode the user is in.
	if (!QCSharewareManager::Initialize())
		return (FALSE);

	//
	// Splash screen if wanted.
	//
	// Now we give the splash screen "child" status, so we can't create it until
	// after the MainFrame is valid.
	//
	CWnd *pAboutDlg = NULL;
	if (GetIniShort(IDS_INI_NO_SPLASH_SCREEN) == FALSE)
		pAboutDlg = DEBUG_NEW_MFCOBJ_NOTHROW CAboutDlg(TRUE/*, pMainFrame*/);

	// Load up the user-defined fonts
	SetupFonts();

	// Get the debug process going
	//if (!InitDebug())
	if (! QCLogFileMT::InitDebug( GetIniLong(IDS_INI_DEBUG_LOG_LEVEL),
								  GetIniLong(IDS_INI_DEBUG_LOG_FILE_SIZE),
								  static_cast<short>( GetSharewareMode() ),
								  EudoraDir,
								  GetIniString(IDS_INI_DEBUG_LOG_FILE_NAME)) )
	{
		//
		// Still have a big problem.  Could not open the debug logfile for writing.
		// The sane thing is to bail so that subsequent logging calls don't
		// misbehave.
		//
		return FALSE;
	}

		// Load up Translation DLL's
//	InitTranslators();	// This used to be here, but we moved it so that plugins would get the right value for Eudora's HWnd

	// Lets init our variables here...
	m_nStartIdle			= 0;			// Our last state execution time holder
	m_nStartIdleFacetime	= 0;
	m_NextCheckMail 		= 0;
	m_NextSendQueued 		= 0;
	m_ForceCheckMail 		= FALSE;

	
	// Possibly disable Save Password feature
	if (GetIniShort(IDS_DISABLE_SAVE_PASSWORD))
		SetIniShort(IDS_INI_SAVE_PASSWORD, FALSE);
		
	// limit Check Mail frequency
	short CMI = GetIniShort(IDS_INI_CHECK_FOR_MAIL);
	if (CMI)
	{
		SetIniShort( IDS_INI_CHECK_FOR_MAIL,
					 _MAX(CMI, GetIniShort(IDS_MIN_MAIL_CHECK_INTERVAL)) );
	}
   	
   	// Make sure only one encoding type is selected
   	if (GetIniShort(IDS_INI_SEND_UUENCODE))
   	{
   		SetIniShort(IDS_INI_SEND_MIME, FALSE);
   		SetIniShort(IDS_INI_SEND_BINHEX, FALSE);
   	}
   	else if (GetIniShort(IDS_INI_SEND_BINHEX))
   	{
   		SetIniShort(IDS_INI_SEND_MIME, FALSE);
   		SetIniShort(IDS_INI_SEND_UUENCODE, FALSE);
   	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	// Text File class
	//03-17-2000 Change CPaigeEdtView to new CTextFileView class
	/*
	TextFileTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_NEWFILE,
		RUNTIME_CLASS(CTextFileDoc),
		RUNTIME_CLASS(PgDocumentFrame),
		RUNTIME_CLASS(CPaigeEdtView));
	AddDocTemplate(TextFileTemplate);*/
	
	TextFileTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_NEWFILE,
		RUNTIME_CLASS(CTextFileDoc),
		RUNTIME_CLASS(PgDocumentFrame),
		RUNTIME_CLASS(CTextFileView));
	AddDocTemplate(TextFileTemplate);

	// Toc class
	TocTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_TOC,
		RUNTIME_CLASS(CTocDoc),
		RUNTIME_CLASS(CTocFrame),
		RUNTIME_CLASS(CTocView));
	AddDocTemplate(TocTemplate);

	// Read message class
	ReadMessageTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_READMESS,
		RUNTIME_CLASS(CReadMessageDoc),
		RUNTIME_CLASS(CReadMessageFrame),
		RUNTIME_CLASS(CEditView));
	AddDocTemplate(ReadMessageTemplate);

	// Composition message class
	CompMessageTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_COMPMESS,
		RUNTIME_CLASS(CCompMessageDoc),
		RUNTIME_CLASS(CCompMessageFrame),
		RUNTIME_CLASS(CPaigeEdtView));
	AddDocTemplate(CompMessageTemplate);

	// Search class
	SearchTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_SEARCH_WND,
		RUNTIME_CLASS(CSearchDoc),
		RUNTIME_CLASS(CMDIChild),
		RUNTIME_CLASS(CSearchView));
	AddDocTemplate(SearchTemplate);

	// Filters class
	FiltersTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_FILTERS,
		RUNTIME_CLASS(CFiltersDoc),
		RUNTIME_CLASS(CFrameWnd),		// dummy value
		RUNTIME_CLASS(CView));			// dummy value
	AddDocTemplate(FiltersTemplate);

	// Nicknames class
	NicknamesTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_NICKNAMES,
		RUNTIME_CLASS(CNicknamesDoc),
		RUNTIME_CLASS(CFrameWnd),		// dummy value
		RUNTIME_CLASS(CView));			// dummy value
	AddDocTemplate(NicknamesTemplate);

	// Signature class
	SignatureTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_TEXTFILE,
		RUNTIME_CLASS(CSignatureDoc),
		RUNTIME_CLASS(PgDocumentFrame),
		RUNTIME_CLASS(CSignatureView)); 
	AddDocTemplate(SignatureTemplate);

	// MoreHelp class
	MoreHelpTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_MOREHELP,
		RUNTIME_CLASS(CMoreHelpDoc),
		RUNTIME_CLASS(CMDIChild),
		RUNTIME_CLASS(CMoreHelpPaigeView));
	AddDocTemplate(MoreHelpTemplate);

	// Update nag class
	UpdateNagTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_UPDATE_NAG,
		RUNTIME_CLASS(CUpdateNagDoc),
		RUNTIME_CLASS(CUpdateNagWnd),
		RUNTIME_CLASS(CUpdatePaigeView));
	AddDocTemplate(UpdateNagTemplate);

	// Usage Statistics class
	UsageStatsTemplate = DEBUG_NEW CMultiDocTemplate(
		IDR_USAGE_STATISTICS,
		RUNTIME_CLASS(CUsageStatisticsDoc),
		RUNTIME_CLASS(CUsageStatisticsFrame),
		RUNTIME_CLASS(CUsageStatisticsView));
	AddDocTemplate(UsageStatsTemplate);

	// Initialize the Search Manager -- must be destroyed when done.
	// This needs to happen before g_theMailboxDirector.Build gets called so
	// that if mailboxes are found and added the Search Manager can be notified.
	if ( !SearchManager::Initialize() )
		return FALSE;

	// build all the command objects
	if (g_theStationeryDirector.Build() == FALSE)
		return FALSE;
	
	if (g_theMailboxDirector.Build() == FALSE)
		return FALSE;
	
	if (g_theRecipientDirector.Build() == FALSE)
		return FALSE;
	
	if (g_theSignatureDirector.Build() == FALSE)
		return FALSE;
	
	if (g_thePersonalityDirector.Build() == FALSE)
		return FALSE;
	
	// Initialize the label director -- must be destroyed when done
	if (!QCLabelDirector::Initialize())
		return (FALSE);

	if( !g_theEmoticonDirector.Build() )
		return false;

	//initialize the Boss Protector domain warning lists
	CBossProtector::Build();

	// Because this init takes so long, we should force window updates throughout
	// In particular force an update right after QCSharewareManager has loaded in 
	// in case it displayed an error dialog about the pro reg code (very rare,
	// but possible).
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// Initialize the Content Concentrator -- must be destroyed when done
	if ( !ContentConcentrator::Initialize() )
		return FALSE;

	// Initialize the Link History Manger -- must be destroyed when done
	if ( !LinkHistoryManager::Initialize() )
		return FALSE;

	// Initialize the Nag Manager -- must be destroyed when done
	if (!CNagManager::Initialize())
	{
		return (FALSE);
	}
	//
	// Create and initialize the One True CFiltersDoc Object(tm).
	//
	VERIFY(CreateFiltersDoc());

	//
	// Create and initialize the One True CNicknamesDoc Object(tm).
	//
	ASSERT(NULL == g_Nicknames);
	g_Nicknames = (CNicknamesDoc *) NewChildDocument(NicknamesTemplate);
	if (! g_Nicknames->ReadTocs())
	{
		g_Nicknames->OnCloseDocument();
		return FALSE;
	}

	//
	// Load up some bitmaps into global variables for faster performance
	//
	VERIFY( g_AttachmentBitmap.LoadBitmap(IDB_ATTACHMENT) );

	VERIFY( g_PriorityHighestMenuBitmap.LoadBitmap(IDB_PRIOR_MENU_HIGHEST) );
	VERIFY( g_PriorityHighMenuBitmap.LoadBitmap(IDB_PRIOR_MENU_HIGH) );
	VERIFY( g_PriorityNormalMenuBitmap.LoadBitmap(IDB_PRIOR_MENU_NORMAL) );
	VERIFY( g_PriorityLowMenuBitmap.LoadBitmap(IDB_PRIOR_MENU_LOW) );
	VERIFY( g_PriorityLowestMenuBitmap.LoadBitmap(IDB_PRIOR_MENU_LOWEST) );

	VERIFY( g_StatusUnreadBitmap.LoadBitmap(IDB_STATUS_UNREAD) );
	VERIFY( g_StatusReadBitmap.LoadBitmap(IDB_STATUS_READ) );
	VERIFY( g_StatusRepliedBitmap.LoadBitmap(IDB_STATUS_REPLIED) );
	VERIFY( g_StatusForwardedBitmap.LoadBitmap(IDB_STATUS_FORWARDED) );
	VERIFY( g_StatusRedirectBitmap.LoadBitmap(IDB_STATUS_REDIRECT) );
	VERIFY( g_StatusUnsendableBitmap.LoadBitmap(IDB_STATUS_UNSENDABLE) );
	VERIFY( g_StatusSendableBitmap.LoadBitmap(IDB_STATUS_SENDABLE) );
	VERIFY( g_StatusQueuedBitmap.LoadBitmap(IDB_STATUS_QUEUED) );
	VERIFY( g_StatusSentBitmap.LoadBitmap(IDB_STATUS_SENT) );
	VERIFY( g_StatusUnsentBitmap.LoadBitmap(IDB_STATUS_UNSENT) );
	VERIFY( g_StatusTimeQueuedBitmap.LoadBitmap(IDB_STATUS_TIME_QUEUED) );
	VERIFY( g_StatusRecoveredBitmap.LoadBitmap(IDB_STATUS_RECOVERED) );

	VERIFY( g_ServerLeaveBitmap.LoadBitmap(IDB_SERVER_LEAVE) );
	VERIFY( g_ServerFetchBitmap.LoadBitmap(IDB_SERVER_FETCH) );
	VERIFY( g_ServerDeleteBitmap.LoadBitmap(IDB_SERVER_DELETE) );
	VERIFY( g_ServerFetchDeleteBitmap.LoadBitmap(IDB_SERVER_FETCH_DELETE) );

	if (GetIniShort(IDS_INI_CENTER_UNREAD_STATUS))
	{
		//
		// Since these bitmaps are used on menu items, center the 
		// bitmaps vertically in the space available.
		//
		int nCheckMenuHeight = ::GetSystemMetrics(SM_CYMENUCHECK);

		::FitTransparentBitmapToHeight(g_PriorityHighestMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityHighMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityNormalMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityLowMenuBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_PriorityLowestMenuBitmap, nCheckMenuHeight);

		::FitTransparentBitmapToHeight(g_StatusUnreadBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusReadBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusRepliedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusForwardedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusRedirectBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusUnsendableBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusSendableBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusQueuedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusSentBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusUnsentBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusTimeQueuedBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_StatusRecoveredBitmap, nCheckMenuHeight);

		::FitTransparentBitmapToHeight(g_ServerLeaveBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_ServerFetchBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_ServerDeleteBitmap, nCheckMenuHeight);
		::FitTransparentBitmapToHeight(g_ServerFetchDeleteBitmap, nCheckMenuHeight);
	}

#ifdef IMAP4
	// Initialize the IMAP service provider.
	//
	CImapMailMgr::InitializeImapProvider();

#endif

	
	// create main MDI Frame window
	CMainFrame* pMainFrame = DEBUG_NEW CMainFrame;
		m_pMainWnd = pMainFrame;

		// Load up Translation DLL's We have to do this here because InitTranslators requires the Mainframe exists.
		// We need it to be after the new mainframe is created so we can get a pointer to the window, but BEFORE
		// The frame is loaded because that's where the buttons and menu items loaded by the plugin director are
		// umm... loaded.
	InitTranslators();
	if ( g_thePluginDirector.Build() == FALSE )
		return FALSE;
		/////////////////////////////

#ifdef THREADED
	if ( !QCTaskManager::Create() )  //Create the task manager for handling the background tasks
		return FALSE;
#endif

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Oh sure, I bet you're going to try and paste something here too. Everyone wants to be right after the creation
	// of the mainframe. GET IN LINE BUDDY!

#ifdef EXPIRING
	// initialize the evaluation timestamp
	char szPath[ 256 ];
	if ( GetModuleFileName(AfxGetInstanceHandle(), szPath, sizeof(szPath) ))
		if ( g_TimeStamp.Read( szPath ) )
			g_TimeStamp.ReadRegistry();

	DWORD dwDaysLeft = g_TimeStamp.GetDaysRemaining();
	if ( dwDaysLeft <= 0 )
	{
		AfxGetMainWnd()->PostMessage(WM_USER_EVAL_EXPIRED);
	}
	else if ( dwDaysLeft < 7 )
	{
		if (WarnYesNoDialog(0, IDS_TIME_BOMB_DAYS_LEFT_BETA, (int)dwDaysLeft) == IDOK)
			LaunchURLWithQuery(NULL, ACTION_UPDATE);
	}
#endif

	// Initialize RAS library
	QCRasLibrary::Init( GetIniShort(IDS_INI_RAS_USE_EXISTING_CONN)?true:false,
						GetIniShort(IDS_INI_RAS_CHOOSE_BEFORE_EVERY_CONNECTION)?true:false );

	// Because this init takes so long, we should force window updates throughout
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// We need to wait until the OWNER.LOK file is created until we can let go of the
	// startup mutex, which allows other instances to see if they can start up.
	RegisterOwnership();
	VERIFY(StartupMutexLock.Unlock());

	// Because this init takes so long, we should force window updates throughout
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	//
	// Customized toolbars can contain user-defined command id's which
	// correspond to user-defined menu items.  Therefore, we must
	// postpone the load of the toolbar buttons until *after* the
	// user-defined menus are built.
	//
#ifdef CTB
	if (! pMainFrame->LoadCustomToolbar())
		return FALSE;
#endif

	// Make sure main window is behind the splash screen
	if (pAboutDlg)
		pMainFrame->SetWindowPos(pAboutDlg, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);

	// If the passed in show state is normal, then get the saved state from the last session
	int			nWindowState = m_nCmdShow;
	if (m_nCmdShow == SW_NORMAL)
		nWindowState = GetIniShort(IDS_INI_MAIN_WINDOW_STATE);

	if ( !pMainFrame->FinishInitAndShowWindow(nWindowState, pAboutDlg) )
		return FALSE;

	pMainFrame->m_bStartedIconic = pMainFrame->IsIconic();
	
	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// We need LMOS hosts ready so we can recover from leftover .RCV files
	CHostList::CreateNewHostList();

	// Recover and delete any left over spool files - must be done after
	// FinishInitAndShowWindow so that task progress can be shown and task
	// can complete correctly (task completion requires the task window).
	QCSpoolMgrMT::Init(g_Personalities);

	// Check if there are any queued messages
	SetQueueStatus();

	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

#ifdef DELPHI
	// verify required fields are present
	if (!CheckDelphiConfig())
		return (FALSE);
#endif

	//
	// Initialize Eudora's DDE Server.  This should be done before we
	// do the MAPI swapperoo.  Otherwise, the initialization could
	// stall while displaying the "could not swap" error dialog before
	// the DDE stuff has a chance to initialize.  If this stall
	// occurs, we have a deadlock situation where the MAPI DLL which
	// launched this copy of Eudora were expecting Eudora's DDE server
	// to be up and running.  How's that for twisted?
	//
	theDDEServer.Initialize();

	// Check out which MAPI DLLs should be where
	if (GetIniShort(IDS_INI_MAPI_USE_NEVER))
		MAPIUninstall();
	else
		MAPIInstall();

	/* Commented the code below which would send a message to mainframe to register Eudora as the Mailto Handler .Now this is handled
     * in by RegisterURLschemes function. Will remove this code once the new code (EudoraNS) works as expected - Apul 01/25 */
	// Register Eudora as Netscape's mailto URL handler
	// pMainFrame->SendMessage(WM_USER_REGISTER_MAILTO);

	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();
	if (pAboutDlg)
		pAboutDlg->UpdateWindow();

	// Moved here from just before "theDDEServer.Initialize()" above, to avoid
	// IMAP conflicts with Netscape!! (JOK - 7/17/98).
	//
	// Load up windows from previous session
	pMainFrame->LoadOpenWindows();

	// Because this init takes so long, we should force window updates throughout
	pMainFrame->UpdateWindow();

	// Destroy the splash screen -- if needed
	if (pAboutDlg)
		pAboutDlg->DestroyWindow();

	// We do this here rather than inside InitMenus() (which is called from
	// FinishInitAndShowWindow above) because this might result in a dialog
	// dialog being shown and we don't want to display a dialog before the
	// splash screen is removed.
	pMainFrame->InitJunkMenus();


	// ==================== Now do all the interactive checking ====================

	RegisterURLSchemes();

	if (!GetIniShort(IDS_INI_SEEN_INTRO))
	{
		// Put up the intro dialog IF it's not the box/ESD build (heaven
		// forbid they see any mention of ads!).
		if (!QCSharewareManager::IsBoxBuild())
		{
			switch (AlertDialog(IDD_INTRODUCTION, PAID_MODE_DEMO_DAYS))
			{
				case IDC_ABOUT:
					LaunchURLWithQuery(NULL, ACTION_INTRO, NULL);
					break;
				case IDC_PAR_CHANGE_REGISTRATION:
					CRegistrationCodeDlg ecDlg;
					ecDlg.DoModal() ; 
					break;
 			}
		}

		SetIniShort(IDS_INI_SEEN_INTRO, TRUE);
		FlushINIFile();
	}

	// Check RegCode.dat after we're all started up
	::PostMessage(pMainFrame->GetSafeHwnd(), WM_COMMAND, ID_CHECK_REG_CODE_FILE, 0);

	// Put up New Account Wizard if no return address specified
	const char* ra = GetReturnAddress();
	if (!ra || !*ra)
	{
		SetIniShort(IDS_INI_LAST_SETTINGS_CATEGORY, 0);
		pMainFrame->PostMessage(WM_COMMAND, ID_SPECIAL_NEWACCOUNT);
	}

	InitJunkFeature();

	if (ra && *ra)
	{
		// Show the tip-o-day only if the Wizard is not shown.
		if (::GetIniShort(IDS_INI_SHOW_TIP_OF_THE_DAY))
			::PostMessage(pMainFrame->GetSafeHwnd(), WM_COMMAND, ID_TIP_OF_THE_DAY, 0);
	}
	
	// now that Eudora's running, handle command line operations
	DoCommandLineActions(m_lpCmdLine);

	g_bDisableUIDL = GetIniShort(IDS_INI_DEBUG_DISABLE_UIDL)?true:false;


	// Begin IMAP4 stuff:

	// Tell IMAP that the app is now initialized:
	//
	CImapMailMgr::NotifyAppInitialized();

	// And resync any IMAP mailboxes that may have been opened:
	pMainFrame->PostMessage(WM_COMMAND, ID_MBOX_IMAP_RESYNC_OPENED);

// End IMAP4 stuff:

	// Let the NagManager know we've started up
	// This could also be called from the very first OnIdle
	CNagManager::GetNagManager()->DoCheckNags(NAG_AT_STARTUP);

	// Call QCUtil's function to pass the Eudora's Data Directory
	SetTempFileDirectory(EudoraDir);

	// Initialize the External Launch Manager, which would register the "EuShlExt" Shell Extension (if reqd)
	CExternalLaunchManager::Initialize();

	//check if INI IDS_INI_HIDE_ABOOK_FROM_VIRUS setting exists in the INI file
	//if setting exists - 1 - make sure all address books are nnt
	//if setting exists - 0 - make sure all address books are txt
	//if INI file setting does not exist and u find any nnt files then add the setting
	//INI IDS_INI_HIDE_ABOOK_FROM_VIRUS with value = 1 and rename all the address book extensions if required
	
	CString strKey;
	TCHAR szValue[32];
	CRString tmpDominant = CRString( IDS_DOMINANT );
	strKey = g_Personalities.GetIniKeyName(IDS_INI_HIDE_ABOOK_FROM_VIRUS);
	g_Personalities.GetProfileString(tmpDominant, strKey, "", szValue, sizeof(szValue));
	switch(szValue[0]) {
		case '0':
		RenameABookExtensions(CRString(IDS_NICK_SAFE_DBASE_NAME), CRString(IDS_NICK_DBASE_NAME), CRString(IDS_SAFE_NICK_FILE_EXTENSION), CRString(IDS_NICK_FILE_EXTENSION));
		break;

		case '1':
		RenameABookExtensions(CRString(IDS_NICK_DBASE_NAME), CRString(IDS_NICK_SAFE_DBASE_NAME), CRString(IDS_NICK_FILE_EXTENSION), CRString(IDS_SAFE_NICK_FILE_EXTENSION));
		break;

		case '\0':
		bool bFileExists;
		bFileExists = DoesABookExtnExist(CRString(IDS_NICK_SAFE_DBASE_NAME), CRString(IDS_SAFE_NICK_FILE_EXTENSION));
		if(bFileExists) {
			RenameABookExtensions(CRString(IDS_NICK_DBASE_NAME), CRString(IDS_NICK_SAFE_DBASE_NAME), CRString(IDS_NICK_FILE_EXTENSION), CRString(IDS_SAFE_NICK_FILE_EXTENSION));
			g_Personalities.WriteProfileString(tmpDominant, strKey, "1");
		}
		break;

		default:
		ASSERT(0);
		break;
	}
	//end testing
	
	return (TRUE);
}



//
//	CEudoraApp::InitJunkFeature()
//
//	The first time Junk-enabled Eudora runs we need to check with the user
//	to see whether or not they want to use this feature and if they already
//	have a mailbox named "Junk" we need to ask about that.
//
void CEudoraApp::InitJunkFeature()
{
	// Convert the old asked about junk preference.  We used to set it to 1 (bit 0), assuming
	// we'd done everything.  Now, we'll use the 1st, 2nd, and 3rd bits to indicate whether
	// or not we've given the general intro (1), which we only do when we've got a junk
	// plug-in, and whether or not we've asked about the Junk mailbox (which is special
	// regardless of whether or not they have plug-ins) (bit 2), and whether or not
	// we've warned cheapskates about missing plug-ins
	//
	// If the zeroth bit is set, we know we've done both warnings, we'll skip them.
	const short introAsked = 1<<1;
	const short boxAsked = 1<<2;
	const short downAsked = 1<<3;

	short askedBits = GetIniShort(IDS_INI_ASKED_ABOUT_JUNK);
	if (askedBits==1) askedBits = introAsked + boxAsked;

	// Grab the current list of junkers
	CTranslatorSortedList *pJunkList = m_TransManager->GetSortedTranslators(EMSF_JUNK_MAIL);

	// We do the intro if there are any junkers and we haven't done it before
	if (!pJunkList->IsEmpty() && (askedBits&introAsked)==0)
	{
		int iResult = AlertDialog(IDD_USE_JUNK_MAILBOX);
		if (iResult == IDOK)
		{
			SetIniShort(IDS_INI_USE_JUNK_MAILBOX, 1);
		}
		else
		{
			// User chose to disable the Junk feature.
			SetIniShort(IDS_INI_USE_JUNK_MAILBOX, 0);
		}
		askedBits |= introAsked;
	}

	// On the other hand, we ALWAYS ask about the mailbox, so long as we haven't
	// asked before, whether or not there are junkers.
	if ((askedBits&boxAsked)==0)
	{
		if (m_iJunkMbxStatus == JUNK_MBOX_REGULAR)
		{
			CMainFrame	*pMainFrame = (CMainFrame*)m_pMainWnd;
			// This user already has a mailbox named "Junk" so ask them what
			// Eudora should do about that.
			int iResult = AlertDialog(IDD_JUNK_MAILBOX_EXISTS);
			if (iResult == IDOK)
			{
				// User chose to have Eudora take over the regular Junk mailbox.
				g_theMailboxDirector.BlessJunkMailbox(pMainFrame->GetActiveMboxTreeCtrl());
			}
			else
			{
				// Use chose to rename their regular Junk mailbox.
				g_theMailboxDirector.RenameJunkMailbox(pMainFrame->GetActiveMboxTreeCtrl());
				// Create a brand new junk mailbox.
				g_theMailboxDirector.CreateJunkMailbox();
			}
			m_iJunkMbxStatus = JUNK_MBOX_SYSTEM;
		}
		else if (m_iJunkMbxStatus == JUNK_MBOX_SYSTEM)
		{
			// Don't need to do anything here.  This should only happen if the user
			// launched Eudora with a locked INI file.  In this case, Eudora will create
			// a Junk system mailbox but won't be able to set the INI setting to indicate
			// that the user has already been asked about the Junk mailbox.
		}
		else
		{
			// This user doesn't have a mailbox named "Junk" so just create it.
			g_theMailboxDirector.CreateJunkMailbox();
		}
		askedBits |= boxAsked;
	}

	// if the user could have more junkers if they paid, tell them so, but only once
	if (GetCurrentPaidMode()==EMS_ModePaid)
		askedBits &= ~downAsked;	// if we unpay, we'll tell them again about missing junkers

	else if ((askedBits&downAsked)==0)
	{
		int nJunk = pJunkList->GetCount();
		pJunkList = m_TransManager->GetSortedTranslators(EMSF_JUNK_MAIL,EMS_ModePaid);

		if (nJunk < pJunkList->GetCount())
		{
			CJunkDownDlg dlg;
			dlg.DoModal();
			askedBits |= downAsked;
		}
	}
	SetIniShort(IDS_INI_ASKED_ABOUT_JUNK, askedBits);
}


void CEudoraApp::ResetStartIdle()
{
	m_nStartIdle = time(NULL);
}

unsigned long CEudoraApp::GetStartIdle()
{
	return m_nStartIdle;
}

unsigned long CEudoraApp::GetIdlePeriod()
{
	return time(NULL) - m_nStartIdle;
}

// Facetime versions of the above functions.
// Facetime is slightly different from normal consideration of whether the user is there or not.
// Mouse movements indicate that the user is present for facetime, but not normally.
void CEudoraApp::ResetStartIdleFacetime()
{
	m_nStartIdleFacetime = time(NULL);
}

unsigned long CEudoraApp::GetStartIdleFacetime()
{
	return m_nStartIdleFacetime;
}

unsigned long CEudoraApp::GetIdlePeriodFacetime()
{
	return time(NULL) - m_nStartIdleFacetime;
}


BOOL CEudoraApp::PreTranslateMessage(MSG* pMsg)
{
    // If the user is doing something (which we define as pressing a key or
    // pressing a mouse button) then record the current time so that we can
    // check for idle time down in CEudoraApp::OnIdle()
    UINT mes = pMsg->message;

    if ( mes == WM_KEYDOWN )	// Switched form Keyup because people were letting go of the shift key too soon.
    {
#ifdef _DEBUG
// Exception generator
// Hold down Shift, Ctrl, Alt, and Caps Lock to cause an exception
if (ShiftDown() && CtrlDown() && GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CAPITAL) & 1)
{
	int* pi = NULL;
	*pi = 0;
}
#endif
		bool bOptionKeysPressed = ((::GetAsyncKeyState( VK_SHIFT ) < 0) ||
									(::GetAsyncKeyState( VK_CONTROL ) < 0));

		unsigned long thetime = time(NULL);
		if (pMsg->wParam == VK_INSERT)
		{	
			if (!bOptionKeysPressed && GetIniShort(IDS_INI_ALLOW_OVERWRITE_MODE) && (thetime - m_LastOverwritePressed > 1)) // One second
			{
				g_bOverwriteMode = !g_bOverwriteMode;
			}
			else if(bOptionKeysPressed)
				m_LastOverwritePressed = thetime;	
		}
	}

	if (mes == WM_MOUSEMOVE)
	{
		// Mouse movements are considered signs of the user being present for facetime,
		// but only if Eudora is the foreground application.
		if (IsForegroundApp())
		{
			// ...and only if the mouse position is significantly different from before.
			int x = LOWORD(pMsg->lParam);
			int y = HIWORD(pMsg->lParam);
			if (abs(x - m_LastMouseMovePoint.x) > 3 || abs(y - m_LastMouseMovePoint.y) > 3)
			{
				m_LastMouseMovePoint.x = x;
				m_LastMouseMovePoint.y = y;
				ResetStartIdleFacetime();
			}
		}
	}
	else if ( ((mes >= WM_KEYFIRST && mes <= WM_KEYLAST) && (mes != WM_KEYUP) && (mes != WM_SYSKEYUP))
		|| (mes >= WM_MOUSEMOVE + 1 && mes <= WM_MOUSELAST)
		|| (mes >= WM_NCLBUTTONDOWN && mes <= WM_NCMBUTTONDBLCLK)
		|| mes == WM_MOUSEWHEEL )
	{
		ResetStartIdle();
		ResetStartIdleFacetime();

		if (IsVersion4() && ((CMainFrame *)AfxGetMainWnd())->GetCurrentIconID() >= IDI_HAVE_MAIL)
			((CMainFrame *)AfxGetMainWnd())->SetIcon(FALSE,0);

		if (mes == WM_MOUSEWHEEL)
		{
			POINT point;
			point.x = (short) LOWORD(pMsg->lParam);    // horizontal position of pointer
			point.y = (short) HIWORD(pMsg->lParam);    // vertical position of pointer

			CWnd *TheWindow = CWnd::WindowFromPoint(point);

			if (!(TheWindow->IsKindOf(RUNTIME_CLASS(CView))))	// The window the mouse was over is not a Cview based class. 
																// We're going to try to pass it to the trident preview pane.
																// If it were a CView based class then we just bail, it's not
																// What we want.
			{
				CWnd *ParentWnd = TheWindow->GetParent();
				if ( ParentWnd && ParentWnd->IsKindOf(RUNTIME_CLASS( CTridentPreviewView)) )
				{

					CWnd *ChildWnd = ParentWnd->GetDescendantWindow(0);
					if (ChildWnd)
						ChildWnd->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
					return TRUE;
				}
		
			}
		}
	}
        
    return (CWinApp::PreTranslateMessage(pMsg));
}

int CEudoraApp::Run()
{
	// Override of CWinApp::Run to provide a place to catch most exceptions
	// that could occur in the main thread before exiting Eudora.

	bool	bExceptionOccurred = false;
	int		nResult = 0;
	
	try
	{
		nResult = CWinApp::Run();
	}
	catch (CException * pException)
	{
		// MFC currently makes operator new throw a CMemoryException
		pException->Delete();

		bExceptionOccurred = true;
	}
	catch (std::exception & /* exception */)
	{
		// I don't think we can currently get this given MFC's overriding of
		// operator new, but perhaps they'll be standards compliant in the future.

		bExceptionOccurred = true;
	}

	// We'll want some sort of kiss of death dialog here.
	ASSERT(!bExceptionOccurred);

	return nResult;
}

void CEudoraApp::SaveStdProfileSettings()
{
	CWinApp::SaveStdProfileSettings();
}

int CEudoraApp::ExitInstance()
{

	// Clean up the directors/managers
	VERIFY(QCLabelDirector::Destroy());
	VERIFY(QCSharewareManager::Destroy());
	VERIFY( ContentConcentrator::Destroy() );
	VERIFY( SearchManager::Destroy() );
	VERIFY( LinkHistoryManager::Destroy() );
	VERIFY(CNagManager::Destroy());

	CAuditEvents* AE = CAuditEvents::GetAuditEvents();
	if (AE)
	{
		if (m_FaceTime)
		{
			AE->ShutdownEvent (	m_FaceTime->FaceTime(),
								m_FaceTime->RearTime(),
								m_FaceTime->ConnectTime(),
								m_FaceTime->TotalTime());
		}
		CAuditEvents::DestroyAuditEvents();
	}
	if (m_FaceTime)
	{
		m_FaceTime->Dispose();
		m_FaceTime = NULL;
	}

	//AKN:UsgStats
	ShutdownStats();

	POSITION pos;

	if (ReadMessageTemplate)
	{
		pos = ReadMessageTemplate->GetFirstDocPosition();
		while (pos)
		{
			// If we get here, it means that we didn't clean up correctly
			ASSERT(FALSE);
			CMessageDoc* doc = (CMessageDoc*)ReadMessageTemplate->GetNextDoc(pos);
			if (doc && doc->m_Sum)
			{
				// This prevents a crash on exit because the summary
				// will have been deleted by this point
				doc->m_Sum = NULL;
			}
		}
	}

	if (CompMessageTemplate)
	{
		pos = CompMessageTemplate->GetFirstDocPosition();
		while (pos)
		{
			// If we get here, it means that we didn't clean up correctly
			ASSERT(FALSE);
			CMessageDoc* doc = (CMessageDoc*)CompMessageTemplate->GetNextDoc(pos);
			if (doc && doc->m_Sum)
			{
				// This prevents a crash on exit because the summary
				// will have been deleted by this point
				doc->m_Sum = NULL;
			}
		}
	}

	pos = m_FilterList.GetHeadPosition();
	while( pos )
	{
		delete m_FilterList.GetNext( pos );
	}
	

	DeleteAutoCompList();

	if (g_Nicknames)
	{
		g_Nicknames->m_bAutoDelete = TRUE;
		g_Nicknames->OnCloseDocument();
		ASSERT(NULL == g_Nicknames);
	}

	if (m_OSVersion.dwMajorVersion)
	{
		//
		// Only run the following if it looks like we actually ran
		// through the InitInstance() method.
		//
		SaveStdProfileSettings();
	    IniStringCleanUp();
		OwnershipCleanup();

		// Eudora.ini shouldn't be changed after this (except possibly some MFC stuff like
		// the recent file list, so if it's slated to be reset, let's go ahead and do it. We
		// need to do this before the CleanupDebug() because it writes to the log file.
		//
		// The other branch of this if statement is only taken if Eudora was never really
		// fully initialized, so we don't worry about the Eudora.ini reset there.
		// 
		if (ResetIniFileOnExit == 0xCAFEB0EF)
			ResetEudoraIni();	

		//Flush the log file and cleanup
		QCLogFileMT::CleanupDebug();

		// Shutdown Eudora's DDE Server.
		theDDEServer.Uninitialize();
	}

	delete m_TransManager;

	// perform fabulous silly shutdown procedure
	PgSharedAccess::SillyShutdown();

	// nuke quicktime
	PgMultimediaShutdown();

#ifdef IMAP4
	// Terminate services of the IMAP service provider.
	CImapMailMgr::Shutdown ();
#endif


#ifdef THREADED
	// Delete all the spool managers
	QCSpoolMgrMT::Shutdown();
	QCTaskManager::Destroy();  //Create the task manager for handling the background tasks
#endif

	// remove temp icon files create when viewing images
	CleanupIcons();

	// Free global palettes
	FreePalettes();

    // This should really be done in OnAppExit() 'cept it doesn't get called.
    // Since we never have multiple instances this is equivilent...
	if (m_OSVersion.dwMajorVersion)
		FreeResources();

	// Cleanup ATL Module
	_Module.Term();
//	ShutdownDebugMemoryManagerMT();	
	
	g_theCommandStack.Pop();
	
	//This function will close the Mood Mail Dictionary if it was open.
	CMoodWatch::CloseMoodDictionary();

	return (CWinApp::ExitInstance());
}


// NewChildDocument
// Create a new document based on the given template
//
CDocument* CEudoraApp::NewChildDocument(CDocTemplate* templ)
{
	CDocument* doc = templ->CreateNewDocument();
	
	ASSERT_VALID(doc);
	
	VERIFY(doc->OnNewDocument());
	
	return (doc);
}

// NewChildFrame
// Create a new frame based on the template specified by the given template,
// and associate it to the document pDOC
//
CFrameWnd* CEudoraApp::NewChildFrame(CDocTemplate* templ, CDocument* pDoc, BOOL bDoIntialUpdate /* TRUE */)
{
	CFrameWnd* frame = templ->CreateNewFrame(pDoc, NULL);
	
	ASSERT_VALID(frame);
	
	if (frame)
		templ->InitialUpdateFrame(frame, pDoc, bDoIntialUpdate);
		
	return (frame);
}

// NewChildDocFrame
// Create a new document and frame based on the given template
//
CDocument* CEudoraApp::NewChildDocFrame(CDocTemplate* templ, BOOL OnlyOne /*= TRUE*/, BOOL bDoIntialUpdate /* TRUE */)
{
	CDocument* doc = NULL;
	
	if (OnlyOne)
	{
		POSITION pos = templ->GetFirstDocPosition();
		if (pos)
		{
			doc = TocTemplate->GetNextDoc(pos);
			pos = doc->GetFirstViewPosition();
			if (pos)
			{
				CView* View = doc->GetNextView(pos);
				QCWorksheet* Frame;
				if (View && (Frame = (QCWorksheet *) View->GetParentFrame()))
				{
					ASSERT_KINDOF(QCWorksheet, Frame);
					if (Frame->IsIconic())
						Frame->MDIRestore();
					else
						Frame->MDIActivate();
				}
				return (doc);
			}
		}
	}

	if (!doc)
		doc = NewChildDocument(templ);
	
	if (doc)
		NewChildFrame(templ, doc, bDoIntialUpdate);

	return (doc);
}                       

BOOL CEudoraApp::IsIdleMessage( MSG* pMsg )
{
	// The MFC idle processing loop will break out of idle processing when it
	// sees *any* message. Rather than waiting for the next timer, we want faster
	// and more continuous extended idle processing.
	//
	// Therefore if we're currently doing processing because the idle timer fired,
	// treat *any* message as if it is an idle message. That will cause OnIdle to
	// be called again as soon as MFC finishes processing the event queue.
	//
	// In each case where we would normally return FALSE, we instead return
	// m_bDoingIdleTimerProcessing.
	CMainFrame *	pMainFrame = reinterpret_cast<CMainFrame *>( AfxGetMainWnd() );
	
	// Return FALSE if the message just dispatched should _not_
	// cause OnIdle to be run.	Messages which do not usually
	// affect the state of the user interface and happen very
	// often are checked for.
	if (!CWinApp::IsIdleMessage(pMsg))
		return m_bDoingIdleTimerProcessing;

	UINT	message = pMsg->message;

	// WM_TIMER messages that aren't the general one shouldn't cause idle processing
	if (pMsg && message == WM_TIMER)
	{
		if ( pMainFrame && (pMsg->wParam == pMainFrame->GetGeneralTimerID()) )
		{
			// Remember that we saw an actual idle message
			m_bRestartIdleProcessing |= TRUE;

			// Remember that we got a general timer and want to do idle
			// timer processing.
			m_bDoingIdleTimerProcessing = TRUE;

			// Reset the general timer back to the default time period so that it
			// doesn't continue to fire at a more frequent rate.
			CMainFrame *	pMainFrame = reinterpret_cast<CMainFrame *>( AfxGetMainWnd() );
			if (pMainFrame)
				pMainFrame->SetGeneralTimer();

			return TRUE;
		}
		else
		{
			return m_bDoingIdleTimerProcessing;
		}
	}

	// App messages shouldn't cause idle processing. Fixes bug where
	// MSHTML was somehow constantly generating WM_APP+2 messages for
	// a particular message, even when Eudora was in the background,
	// which caused Eudora to hog the CPU doing idle processing.
	//
	// To keep idle processing from being interrupted we've extended the range
	// that we don't allow to initiate idle processing to include all messages
	// used by private window classes (i.e. starting at WM_USER) through all
	// messages that are defined via RegisterWindowMessage. Any messages that
	// need idle to occur sooner than the general idle timer should post a
	// WM_KICKIDLE message.
	if ( (message >= WM_USER) && (message <= 0xFFFF) )
		return m_bDoingIdleTimerProcessing;

	if ( (message == WM_MOUSEMOVE) || (message == WM_NCMOUSEMOVE) )
	{
		// Mouse movement shouldn't cause idle processing if Eudora
		// is in the background.
		if ( !IsForegroundApp() )
			return m_bDoingIdleTimerProcessing;
	}

	// Remember that we saw an actual idle message
	m_bCallCWinAppIdle |= TRUE;
	m_bRestartIdleProcessing |= TRUE;

	return TRUE;
}


BOOL CEudoraApp::IdleSortMailboxes()
{
	BOOL	bDidSomething = FALSE;
	
	// Sort mailboxes marked to be sorted immediately
	POSITION TocPos = (TocTemplate? TocTemplate->GetFirstDocPosition() : NULL);
	while (TocPos)
	{
		CTocDoc* doc = (CTocDoc*) TocTemplate->GetNextDoc(TocPos);
		// Sort the toc and break out after doing just one because
		// we don't want to take up a lot of time
		if ( doc->SortNowIfNeeded() )
		{
			bDidSomething = TRUE;
			break;
		}
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleUpdateFaceTime()
{
	static CMDIChildWnd *	s_pLastChild = NULL;
	static FacetimeMode		s_LastChildFaceTimeMode = FACETIMEMODENULL;

	BOOL					bDidSomething = FALSE;

	if (m_nCurrentIdleFaceTime > 0)
	{
		const BOOL bIsForeground = IsForegroundApp();
		int state = ASEBackground;

		if (bIsForeground)
			state = m_nCurrentIdleFaceTime < kFaceIntervalAfter? ASEActive : ASEInactive;
		QCFaceTime::Update( state, !IsOffline() );

		// The following "if" is added to update the facetime whenever the active child frame changes within Eudora.
		// This way the facetime at any given instant will be more accurate ('coz it's updated more often).
		// Adding this code helps Usage Statistics to be more accurate about Eudora Usage. This is part of the fix
		// for VI #6210

		CMDIFrameWnd *		pFrame = reinterpret_cast<CMainFrame *>(m_pMainWnd);

		if (bIsForeground && pFrame)
		{
			// Get the active MDI child window.
			CMDIChildWnd *	pCurChild = (CMDIChildWnd *) pFrame->GetActiveFrame();

			if (pCurChild && (pCurChild != s_pLastChild))
			{
				// If here, then it means that the currently active window is not the same as the one which was, the last
				// time we processed OnIdle(). 
				// NOTE : The s_pLastChild should not be used for any manipulations because it is pointing to a window which 
				// is no longer present. I just use it here to check if the values of s_pLastChild & pCurChild are same.

				// Force the QCFaceTime::Update() to update the facetime values. The third parameter(true) does the trick.
				QCFaceTime::Update(state, !IsOffline(), true);

				// Here we should update the Usage Stats Facetime.
				UpdateFacetime(s_LastChildFaceTimeMode);
				
				// Update the s_pLastChild to pCurChild (Current Active Child Window)
				s_pLastChild = pCurChild;				
				
				// Get the facetime mode associated with the current window (which we have now made as the s_pLastChild)
				s_LastChildFaceTimeMode = USGetFacetimeMode(s_pLastChild);
			}
		}

		// Update the usage stats face time here.
		UpdateFacetime();

		bDidSomething = TRUE;
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleProcessIMAPQueues(bool bHighPriorityOnly /*=false*/)
{
	return g_ImapAccountMgr.ProcessActionQueues(bHighPriorityOnly);
}


BOOL CEudoraApp::IdleAutoSave()
{
	static unsigned long	ulCheckTimeForAutoSave = m_nStartCurrentIdleProcessing + 60;
	unsigned long			nAutoSaveEvery = GetIniLong(IDS_INI_AUTO_SAVE_MSGS_EVERY);

	BOOL					bDidSomething = FALSE;

	// Check every 1 minute for possible auto-saves
	if ( nAutoSaveEvery && (m_nStartCurrentIdleProcessing >= ulCheckTimeForAutoSave) && (m_nCurrentIdlePeriod > 1) )
	{
		// Auto save
		POSITION pos = CompMessageTemplate->GetFirstDocPosition();
		while (pos)
		{
			CCompMessageDoc * doc = (CCompMessageDoc *) CompMessageTemplate->GetNextDoc(pos);
			
			if ( doc && doc->IsModified() )
			{
				if ( (m_nStartCurrentIdleProcessing - doc->m_NextAutoSaveTime) >= nAutoSaveEvery*60 )
				{
					// Write out the message and stop after doing just one because
					// we don't want to take a lot of time
					CCursor::Ignore( TRUE );
					doc->OnSaveDocument(NULL);
					CCursor::Ignore( FALSE );
				
					doc->m_NextAutoSaveTime = m_nStartCurrentIdleProcessing;

					bDidSomething = TRUE;
					break;
				}
			}
		}

		// If we auto-saved something, then check again immediately on the next idle.
		// Otherwise check again in 1 minute.
		ulCheckTimeForAutoSave = bDidSomething ? 0 : m_nStartCurrentIdleProcessing + 60;
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleCheckOwnership()
{
	// Check for a Remote Instance
	static unsigned long		s_lastOwnerLockCheck = 0;
	unsigned long				freq = (unsigned long) GetIniShort(IDS_INI_CHECK_OWNER_FREQ);

	BOOL						bDidSomething = FALSE;		

	if ( freq > 0 && (m_nStartCurrentIdleProcessing - s_lastOwnerLockCheck) >= freq )
	{
		s_lastOwnerLockCheck = m_nStartCurrentIdleProcessing;

		CheckOwnership();

		bDidSomething = TRUE;
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleCheckMail()
{
	BOOL			bDidSomething = FALSE;
	long 			CMI = GetIniLong(IDS_INI_CHECK_FOR_MAIL);
	
	// Check mail if told to, and we've past the next time to check mail.
	// Skipping checking mail if there's no net connection, and setting to prevent offline checks in on.
	// Skipping checking mail if on a notebook in DC (battery) mode with the Setting flag on.
	SYSTEM_POWER_STATUS systemPower;
	
	if (m_ForceCheckMail ||
		((CMI || m_NextCheckMail) && m_nStartCurrentIdleProcessing >= m_NextCheckMail &&!GetIniShort(IDS_INI_CONNECT_OFFLINE) &&
			(!GetIniShort(IDS_INI_DONT_CHECK_UNCONNECTED) || IsOffline() == FALSE) &&
			(!GetIniShort(IDS_INI_DONOT_CHECK_BATTERY) || !GetSystemPowerStatus(&systemPower) || systemPower.ACLineStatus != 0)
		))
	{
		// Set next time to check mail
		m_NextCheckMail = m_nStartCurrentIdleProcessing + CMI * 60;
		// Check it
		GetMail(kAutomaticMailCheckBits);
		
		if (m_ForceCheckMail) 
			m_ForceCheckMail = FALSE;
		
		// Make sure it doesn't happen right away
		if ( CMI )
			m_NextCheckMail = max(m_NextCheckMail, (unsigned long)(time(NULL) + 60U));
		else
			m_NextCheckMail = 0;

		UpdateMailCheckLabel();

		bDidSomething = TRUE;
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleSendMail()
{
	BOOL	bDidSomething = FALSE;
	
	if ( m_NextSendQueued && m_nStartCurrentIdleProcessing >= m_NextSendQueued 
				&& m_nCurrentIdlePeriod > (unsigned long)GetIniShort(IDS_INI_IDLE_TIME) 
				&& !GetIniShort(IDS_INI_CONNECT_OFFLINE))
	{
		// if delayed messages are put in the queue, and their time has come...
	 	SendQueuedMessages();

		bDidSomething = TRUE;
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleTaskManagerPostProcessing()
{
	BOOL	bDidSomething = FALSE;
	
	if ( QCGetTaskManager()->IsPostProcessingRequested() )
	{
		// Do post processing based on the request process command queue not being empty
		QCGetTaskManager()->DoPostProcessing();

		bDidSomething = TRUE;
	}

	unsigned long	ulTaskMgrWaitTime = GetIniLong(IDS_INI_TASKMGR_WAITTIME);

	if (m_nCurrentIdlePeriod > ulTaskMgrWaitTime)
	{
		// If any of the tasks need post processing, request processing time
		if ( QCGetTaskManager()->NeedsPostProcessing() )
		{
			QCGetTaskManager()->RequestPostProcessing();

			bDidSomething = TRUE;
		}
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleWriteModifiedTocs()
{
	BOOL	bDidSomething = FALSE;
	
	if (m_nCurrentIdleFaceTime > 2)
	{
		// Periodically write out TOCs that are modified
		POSITION TocPos = (TocTemplate? TocTemplate->GetFirstDocPosition() : NULL);
		
		while (TocPos)
		{
			CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(TocPos);
			if (doc && doc->IsModified())
			{
				// Write out the toc and break out after doing just one because
				// we don't want to take up a lot of time
				CCursor::Ignore( TRUE );
				doc->OnSaveDocument(NULL);
				CCursor::Ignore( FALSE );

				bDidSomething = TRUE;
				break;
			}
		}
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleCalculateMood()
{
	BOOL	bDidSomething = FALSE;
	
	// Idle time is in Seconds whereas Threshold time is in Milliseconds.
	if( m_nCurrentIdlePeriod > (unsigned long) GetIniShort(IDS_INI_MOODWATCH_DELTATIME)
		&& UsingFullFeatureSet() && GetIniShort(IDS_INI_MOODCHECK_BACKGROUND)
		&& GetIniShort(IDS_INI_MOOD_MAIL_CHECK) ) 
	{
		POSITION TocPos = (TocTemplate? TocTemplate->GetFirstDocPosition() : NULL);

		while (TocPos)
		{
			CTocDoc* doc = (CTocDoc*)TocTemplate->GetNextDoc(TocPos);
			if (doc)
			{
				bDidSomething = doc->CalculateMood();

				if (bDidSomething)
					break;
			}
		}
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleSwitchToSponsoredMode()
{
	BOOL	bDidSomething = FALSE;
	
	// See if we have a pending switch to sponsored mode.
	time_t expTime;
	if (QCSharewareManager::GetAdwareSwitchTime(&expTime) && (time(NULL) > expTime))
	{
		// Use the same idleness criterion we use for the nags.
		if (m_nCurrentIdlePeriod > min((unsigned long) GetIniShort(IDS_INI_NAG_IDLE_SECS), (unsigned long) 120))
		{
			// Need to switch, and we've been idle long enough, so let's do it.
			QCSharewareManager* pSWM = GetSharewareManager();
			if (pSWM)
			{
				pSWM->SetMode(SWM_MODE_ADWARE, false, NULL);
				pSWM->ClearAdwareSwitchTime();

				bDidSomething = TRUE;
			}
		}
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleCheckNetscapeInteraction()
{
	BOOL	bDidSomething = FALSE;
	
	// Lets make sure that if Eudora is being launched by Netscape thru' the EudoraNS.dll, then we update the
	// HKCR\mailto\Shell\Open\Command registry setting to point back to Eudora. Netscape, even when Eudora's 
	// configured as the default mailer overwrites the above mentioned registry with it's own executable path.
	// EudoraNS.dll sets "EXT_EudoraNS_NoWarn" ini setting when Eudora is being launched by Netscape. 
	//
	// This will poll the Eudora.ini file directly (has to), so do it sparingly
	static unsigned long LastNoWarnCheck = 0;
	if (m_nCurrentIdleFaceTime > 30 && m_nStartCurrentIdleProcessing > LastNoWarnCheck + 60)
	{
		LastNoWarnCheck = m_nStartCurrentIdleProcessing;

		UINT uiHasEudoraNSSetNotToWarn = ::GetPrivateProfileInt(_T("Settings"),_T("EXT_EudoraNS_NoWarn"),0,INIPath);
		if (uiHasEudoraNSSetNotToWarn)
		{
			char app[_MAX_PATH + 1]; 
			char cmd[_MAX_PATH + 1];
			char longApp[_MAX_PATH + 1];
				 
			GetModuleFileName(AfxGetInstanceHandle(), longApp, sizeof(longApp));
			GetShortPathName(longApp,app, sizeof(app));
			
			strcpy(cmd,app);
			lstrcat(cmd," /m %1");

			CRString RegMailto(IDS_REG_MAILTO);

			// We can safely assume that Eudora is the default mailer or else it shouldn't come here.
			AddToRegistry(HKEY_CLASSES_ROOT, RegMailto, NULL, cmd);					
			::WritePrivateProfileString(_T("Settings"),_T("EXT_EudoraNS_NoWarn"),_T("0"),INIPath);	
		}

		bDidSomething = TRUE;
	}

	return bDidSomething;
}


BOOL CEudoraApp::IdleLogTocDiscrepancies()
{
	BOOL	bDidSomething = FALSE;
	
	// Maybe log discrepancies in the TOC size.  This is a measure to help us track down
	// the mysterious disappearing messages problem.  If conditions are right we will log
	// any instance where a TOC has changed size past the threshold specified in
	// IDS_INI_DEBUG_TOC_DISC_LEVEL since the last check.
	static unsigned long lLastTOCCheck = 0;
	// Only check for discrepancies if user wants to (discrepancy level > 0).
	if (GetIniShort(IDS_INI_DEBUG_TOC_DISC_LEVEL) > 0)
	{
		// See if enough time has elapsed since last check.
		if ( (lLastTOCCheck == 0) || (m_nStartCurrentIdleProcessing > lLastTOCCheck + GetIniShort(IDS_INI_DEBUG_TOC_DISC_FREQ)) )
		{
			CPtrList		*pMBList = g_theMailboxDirector.GetMailboxList();
			POSITION		 pos = pMBList->GetHeadPosition();
			CTocDoc			*pTocDoc = NULL;

			while (pos)
			{
				QCMailboxCommand *pCurrent = (QCMailboxCommand *)pMBList->GetNext(pos);

				if (pCurrent)
				{
					pTocDoc = GetToc(pCurrent->m_szPathname, NULL, TRUE, TRUE);
					if (pTocDoc)
					{
						pTocDoc->CheckNumSums();
					}
				}
			}

			lLastTOCCheck = m_nStartCurrentIdleProcessing;

			bDidSomething = TRUE;
		}
	}

	return bDidSomething;
}


// OnIdle
//
// Do things in the background, like automatic mail checks and mailbox TOC saves.
// This routine gets called as result of a WM_TIMER message in the main window.
//
BOOL CEudoraApp::OnIdle( LONG lCount )
{
	//	OnIdle is called from an MFC loop which calls us repeatedly until it
	//	either finds that the user has done something (checked via PeekMesage)
	//	or until we return FALSE.
	//
	//	lCount is incremented by MFC before each time that we're called.
	//	A value of 0 for lCount indicates that it is the first time that we're
	//	being called in a given idle session.
	if ( (lCount < 2) && m_bCallCWinAppIdle )
	{
		//	CWinApp operates when lCount is 0 and 1, but only returns TRUE when lCount
		//	is 0. Call CWinApp when lCount is less than 2 and return TRUE no matter
		//	what, because CWinApp does something for both values.
		CWinApp::OnIdle(lCount);

		//	If lCount == 1, then reset whether or not we need to call CWinApp::OnIdle.
		//	Will be set to TRUE the next time we see an actual idle message.
		if (lCount == 1)
			m_bCallCWinAppIdle = FALSE;

		return TRUE;
	}

	//	If we made it this far, then lCount >= 2.
	//	At this point we only use lCount to signal when a new idle session has begun.
	//	MFC examples demonstrate using lCount for a switch statement, but that has
	//	several limitations. Instead we use our own count to keep track of what
	//	we're doing.
	//
	//	s_nIdleTaskIndex: Keeps track of which task we're currently operating on.
	//					  Allows us to do main tasks as a circular queue (so that we
	//					  can be guaranteed that no main task could ever possibly starve).
	//					  Also allows us to do more than one task during a given OnIdle call.
	//	s_nIdleMainTaskStop: We always attempt to do all main tasks before continuing on
	//						 to "extended" idle tasks.
	//	s_bDoingMainTasks: true if we're doing main idle tasks, false if we're doing
	//					   "extended" idle tasks.
	static const long					kLastMainTask = 15;
	static const long					kNumMainTasks = kLastMainTask + 1;
	static long							s_nIdleTaskIndex = 0;
	static long							s_nIdleMainTaskStop = 0;
	static bool							s_bDoingMainTasks = true;
	static DWORD						s_nStartIdleTickCount = 0;
	static bool							s_bNeedToInitGetLastInputInfo = true;

	// Get the GetLastInputInfo function from User32.dll (available in Windows 2000 and later).
	typedef struct tagLASTINPUTINFO {
		UINT cbSize;
		DWORD dwTime;
	} LASTINPUTINFO, * PLASTINPUTINFO;
	typedef BOOL						(__stdcall *IndirectGetLastInputInfo)(PLASTINPUTINFO);
	static IndirectGetLastInputInfo		s_pGetLastInputInfo = NULL;

	if (s_bNeedToInitGetLastInputInfo)
	{
		HMODULE		hUser32 = GetModuleHandle("User32.dll");

		// We should always be able to get the User32.dll
		ASSERT(hUser32);

		if (hUser32)
			s_pGetLastInputInfo = reinterpret_cast<IndirectGetLastInputInfo>( GetProcAddress(hUser32, "GetLastInputInfo") );

		s_bNeedToInitGetLastInputInfo = false;
	}

	if (m_bRestartIdleProcessing)
	{
		//	Reset whether or not we need to call restart idle processing.
		//	Will be set to TRUE the next time we see an actual idle message.
		m_bRestartIdleProcessing = FALSE;
		
		//	We're in a new idle processing session.
		//	Recalculate all the idle processing timing information.
		m_nStartCurrentIdleProcessing = time(NULL);
		m_nCurrentIdlePeriod = GetIdlePeriod();
		m_nCurrentIdleFaceTime = GetIdlePeriodFacetime();

		//	Reset the start tick count for extended idle processing
		s_nStartIdleTickCount = GetTickCount();

		//	Start processing main tasks. Either continue where we left off, or
		//	start main tasks from the beginning.
		if (s_bDoingMainTasks)
		{
			//	Continue where we left off, and stop when we've done all main tasks.
			s_nIdleMainTaskStop = s_nIdleTaskIndex;
		}
		else
		{
			//	In our last processing session we finished doing all main tasks,
			//	and we were instead operating on extended idle tasks.
			//	Start main tasks from the beginning, and stop when we've done all
			//	main tasks.
			s_nIdleTaskIndex = 0;
			s_nIdleMainTaskStop = 0;
			s_bDoingMainTasks = true;
		}

		return TRUE;
	}

	//	Remember whether or not we did anything. If we did, then we'll stop idle
	//	processing and return so that CWinApp can check for user input via PeekMessage.
	BOOL	bDidSomething = FALSE;

	//	Remember when we started idle processing
	DWORD	nStartIdleProcessingTickCount = GetTickCount();

	//	Default loop threshold to 200 milliseconds (i.e. 0.2 seconds). Time period
	//	is short enough that if user does something Eudora should react quickly
	//	enough that the user won't "feel" much of a delay.
	DWORD	nLoopThreshold = 200;

	//	If any of the following are true:
	//	* Threaded tasks (mail checks) etc. are running
	//	* We're not the foreground app
	//	* We haven't been idle for at least a minute
	//	then set our loop threshold lower to make sure that
	//	we keep the system responsive.

	//	Set our loop threshold lower based on what the user is doing, so that
	//	Eudora and other apps remain responsive.
	//	* Set to 0 if Eudora is not foreground - this allows our idle throttling
	//	  code to throttle appropriately without the need to do all the throttle
	//	  threshold determination here.
	//	* Set to 50 if Eudora is doing something else or if the user has done
	//	  something is the last 10 seconds.
	//	* Set to 100 if the user has done something in the last 30 seconds.
	if ( !IsForegroundApp() )
		nLoopThreshold = 0;
	else if ( (QCGetTaskManager()->GetTaskCount() > 0) || (m_nCurrentIdlePeriod < 10) )
		nLoopThreshold = 50;
	else if (m_nCurrentIdlePeriod < 30)
		nLoopThreshold = 100;
	
	//	=== Main Idle Processing ===
	//
	//	Loop through all main idle tasks until one of them does something (i.e. more
	//	than just checking to see if it should act). In general the rule of thumb is
	//	that no idle processing should take more than 100-200 milliseconds.
	//	
	//	The idle tasks are in a rough order of priority. The order doesn't matter too
	//	much, however, because the processing is done in a circular queue fashion
	//	until we completely process all main tasks. (The next time we start processing
	//	tasks, we will start from the beginning, so the order does have some bearing).
	//	By doing the processing in a circular queue fashion we guarantee that no
	//	main task will be ignored.
	if (s_bDoingMainTasks)
	{
		do
		{
			switch (s_nIdleTaskIndex)
			{
				case 0:
					bDidSomething |= IdleUpdateFaceTime();
					break;

				case 1:
					bDidSomething |= IdleProcessIMAPQueues(true/*bHighPriorityOnly*/);
					break;

				case 2:
					// Call the Search Manager
					bDidSomething |= SearchManager::Instance()->Idle(m_nCurrentIdlePeriod, false);
					break;

				case 3:
					// Call the Plugin Idle Function making sure it's required first
					if (m_TransManager && CTranslatorManager::m_PluginIdle)
					{
						m_TransManager->IdleEveryone(m_nCurrentIdlePeriod, 0);
						bDidSomething |= TRUE;
					}
					break;

				case 4:
					bDidSomething |= IdleSortMailboxes();
					break;

				case 5:
					bDidSomething |= IdleWriteModifiedTocs();
					break;

				case 6:
					bDidSomething |= IdleAutoSave();
					break;

				case 7:
					bDidSomething |= IdleCheckMail();
					break;

				case 8:
					bDidSomething |= IdleSendMail();
					break;

				case 9:
					bDidSomething |= IdleTaskManagerPostProcessing();
					break;

				case 10:
					// Call the Link History Manager
					bDidSomething |= LinkHistoryManager::Instance()->Idle(m_nStartCurrentIdleProcessing, m_nCurrentIdlePeriod);
					break;

				case 11:
					// Call the Nag Manager if the appropriate number of
					// idle seconds have passed (with a 120 second maximum).
					if ( m_nCurrentIdlePeriod > min((unsigned long)GetIniShort(IDS_INI_NAG_IDLE_SECS), (unsigned long)120) )
					{
						CNagManager::GetNagManager()->DoCheckNags(NAG_AT_IDLE);
						bDidSomething |= TRUE;
					}
					break;

				case 12:
					bDidSomething |= IdleSwitchToSponsoredMode();
					break;

				case 13:
					if (m_nCurrentIdleFaceTime > 30)
					{
						ReclaimTOCs();
						bDidSomething |= TRUE;
					}
					break;

				case 14:
					bDidSomething |= IdleCheckOwnership();
					break;

				case 15:
					bDidSomething |= IdleCheckNetscapeInteraction();
					break;

				case 16:
					bDidSomething |= IdleLogTocDiscrepancies();
					break;
			}

			s_nIdleTaskIndex++;
			s_nIdleTaskIndex %= kNumMainTasks;

			//	Continue looping while:
			//	* We didn't hit the stop task (so we know we didn't idled everything yet)
			//	* Nothing returned that it did something yet OR there's still more time available
			//
			//	Important to continue looping even if there's technically no more loop time
			//	available when bDidSomething is false, because otherwise we can't be
			//	guaranteed that we'll ever loop the whole way through all the main idle tasks.
		} while ( (s_nIdleTaskIndex != s_nIdleMainTaskStop) &&
				  ( !bDidSomething || ((GetTickCount() - nStartIdleProcessingTickCount) < nLoopThreshold) ) );

		//	Check to see if we finished doing all main tasks
		s_bDoingMainTasks = (s_nIdleTaskIndex != s_nIdleMainTaskStop);

		//	If we're finished doing all main tasks then figure out whether or
		//	not we want to allow extended idle tasks.
		if (!s_bDoingMainTasks)
		{
			//	Allow extended idle tasks if we're doing idle timer processing
			BOOL		bAllowExtendedIdleTasks = m_bDoingIdleTimerProcessing;

			if (!bAllowExtendedIdleTasks)
			{
				//	We're not doing idle timer processing. If we couldn't set the
				//	general timer, then allow extended idle tasks anyway.
				CMainFrame *	pMainFrame = reinterpret_cast<CMainFrame *>( AfxGetMainWnd() );
				if (pMainFrame)
					bAllowExtendedIdleTasks = !pMainFrame->CouldSetGeneralTimer();
			}

			if (bAllowExtendedIdleTasks)
			{
				//	Update current idle period
				m_nCurrentIdlePeriod = GetIdlePeriod();
				
				//	Only allow extended idle tasks when Eudora is at least somewhat idle
				//	because we want to keep Eudora responsive for users.
				bAllowExtendedIdleTasks = (m_nCurrentIdlePeriod > 1);
			}
			
			if (!bAllowExtendedIdleTasks)
			{
				//	We already know that we're done with main tasks, by setting
				//	bDidSomething to false we make sure that we won't continue
				//	to be called.
				bDidSomething = false;

				//	Set s_bDoingMainTasks back to true so that we won't do any extended idle tasks.
				s_bDoingMainTasks = true;

				//	Set m_bDoingIdleTimerProcessing to FALSE because we're done doing the
				//	idle timer processing.
				m_bDoingIdleTimerProcessing = FALSE;
			}

			//	Otherwise we'll want to start extended idle processing with the first idle task.
			s_nIdleTaskIndex = 0;
		}
	}

	if (!bDidSomething && !s_bDoingMainTasks)
	{
		//	=== Extended Idle Processing ===
		//
		//	We made it through all the main tasks without doing anything. Now spend time
		//	repeatedly calling any routines that might still have work to do. Unlike the
		//	main tasks, here we continue calling a given routine until it returns that it
		//	has nothing else to do.
		//
		//	Although in practice extended tasks will be called eventually, it's a probably
		//	a good idea to do a small slice of work in the main idle processing where
		//	you are guaranteed that you'll be called.
		//
		//	Note that we have no worries that extended idle processing will starve main
		//	idle processing, because:
		//	* Any user events will halt extended idle processing
		//	* The GENERAL_BACKGROUND_TIMER_ID fires once a second (see mainfrm.cpp), so
		//	  at most we expect the total time for any given OnIdle loop to be limited
		//	  to about one second.
		bool	bMoreExtendedTasks = true;
		
		do
		{
			switch (s_nIdleTaskIndex)
			{
				case 0:
					bDidSomething |= IdleSortMailboxes();
					break;

				case 1:
					bDidSomething |= IdleAutoSave();
					break;

				case 2:
					bDidSomething |= IdleWriteModifiedTocs();
					break;

				case 3:
					// Process IMAP queues
					bDidSomething |= IdleProcessIMAPQueues();
					break;

				case 4:
					// Call the Search Manager
					bDidSomething |= SearchManager::Instance()->Idle(m_nCurrentIdlePeriod, true);
					break;

				case 5:
					bDidSomething |= IdleCalculateMood();
					break;

				default:
					//	Stop when we've done all extended tasks
					bMoreExtendedTasks = false;
					break;
			}
			
			if (!bDidSomething)
				s_nIdleTaskIndex++;
		} while ( bMoreExtendedTasks &&
				  ((GetTickCount() - nStartIdleProcessingTickCount) < nLoopThreshold) );

		//	If there are still more extended tasks, then set bDidSomething to true
		//	so that we'll be sure to continue extended idle processing.
		if (bMoreExtendedTasks)
			bDidSomething = true;

		if (bDidSomething)
		{
			//	Extended idle processing throttling. We want to balance getting
			//	a lot of stuff done with avoiding excessively hogging the CPU.
			//
			//	Note that none of these values should be greater than 1000 (the
			//	time period of our general timer). These values very roughly
			//	equate to percentages of the amount of CPU Eudora can use
			//	(just divide by 10 to get the rough CPU usage equivalent).

			//	We use a high value when Eudora is in front and nothing's happening
			//	because we want extended idle processing to finish its task(s)
			//	as quickly as possible.
			static const DWORD		kThrottleWhenInFront = 900;

			//	We use a fairly low value when other Eudora tasks/threads are
			//	running so that we don't slow down sending and receiving of email.
			static const DWORD		kThrottleForOtherEudoraTasks = 200;

			//	We use a medium value when Eudora is in the background and no user
			//	activity has been detected. This is meant to be high enough to make
			//	progress on extended idle processing, without being unfriendly to
			//	other processing occurring on the computer.
			static const DWORD		kThrottleWhenInBackNoUserActivity = 400;

			//	We use a low value when the user has done something fairly recently
			//	so that we don't slow down the user's experience in any other apps.
			static const DWORD		kThrottleWhenInBackSomeUserActivity = 100;

			//	We only processing one extended idle task when the user has done
			//	something very recently.
			static const DWORD		kThrottleWhenInBackUserActive = 0;

			//	Default throttle threshold to foreground high value
			DWORD					nThrottleThreshold = kThrottleWhenInFront;

			if (QCGetTaskManager()->GetTaskCount() > 0)
			{
				//	Limit our idle processing so that we don't choke threaded tasks.
				nThrottleThreshold = kThrottleForOtherEudoraTasks;
			}
			else if ( !IsForegroundApp() )
			{
				//	Eudora is in the background. Be cautious how much CPU we consume
				//	so that we don't annoy users.
				if (s_pGetLastInputInfo)
				{
					//	Attempt to get the last global input time (i.e. is the user doing
					//	anything at all while we're in the background).
					LASTINPUTINFO	info;
					info.cbSize = sizeof(LASTINPUTINFO);
					
					if ( !s_pGetLastInputInfo(&info) )
					{
						//	Couldn't determine if the user is doing anything. Use a
						//	throttle amount that is pretty low so that Eudora doesn't
						//	hog the system's CPU.
						nThrottleThreshold = kThrottleWhenInBackSomeUserActivity;
					}
					else
					{
						DWORD	dwTimeSinceLastUserActivity = GetTickCount() - info.dwTime;

						//	Gage how much we need to back off based on how recently the
						//	user has done anything. Keep in mind that GetLastInputInfo
						//	returns when the user has done *anything* (including mouse
						//	movement), so we don't want to choke extended idling too
						//	drastically unless the user has been active very recently.
						if (dwTimeSinceLastUserActivity < 500)
							nThrottleThreshold = kThrottleWhenInBackUserActive;
						else if (dwTimeSinceLastUserActivity < 10000)
							nThrottleThreshold = kThrottleWhenInBackSomeUserActivity;
						else
							nThrottleThreshold = kThrottleWhenInBackNoUserActivity;
					}
				}
				else
				{
					//	User is running older version of Windows. Can't easily determine
					//	if the user is doing anything. Use a throttle amount that is
					//	pretty low so that Eudora doesn't hog the old system's CPU.
					nThrottleThreshold = kThrottleWhenInBackSomeUserActivity;
				}
			}

			if (nThrottleThreshold > CMainFrame::kGeneralTimerPeriod)
				nThrottleThreshold = CMainFrame::kGeneralTimerPeriod;

			//	If we have threaded tasks running, and we've done "enough" extended
			//	idle processing, then stop this idle processing session so that we
			//	don't choke threaded tasks.
			if ( (GetTickCount() - s_nStartIdleTickCount) >= nThrottleThreshold )
			{
				//	Stop extended idle processing (for now) by returning that we don't
				//	want to continue processing.
				bDidSomething = false;

				//	Calculate when we next need to do extended idle processing
				UINT			nNextIdle = CMainFrame::kGeneralTimerPeriod - nThrottleThreshold;

				//	Set the timer for the next idle processing
				CMainFrame *	pMainFrame = reinterpret_cast<CMainFrame *>( AfxGetMainWnd() );
				if (pMainFrame)
					pMainFrame->SetGeneralTimer(nNextIdle);
			}
		}
		else
		{
			//	Set the timer for the next idle processing to the default value so that
			//	we use less CPU when we don't need to do extended processing at the moment.
			CMainFrame *	pMainFrame = reinterpret_cast<CMainFrame *>( AfxGetMainWnd() );
			if (pMainFrame)
				pMainFrame->SetGeneralTimer();
		}

		//	Remember whether or not we want to continue to do idle timer processing
		m_bDoingIdleTimerProcessing = bDidSomething;
	}

	return bDidSomething;
}

// --------------------------------------------------------------------------
//
// HandleRegError [PROTECTED]
//
// Call this function when any registry function fails. A dialog will be
// displayed with the error message and the full key path.
//
// Scott Manjourides, 7/28/98
//
void CEudoraApp::HandleRegError(HKEY hkey, LPCTSTR pRegPath, UINT nErrID)
{
	CRString sErrMsg(nErrID);
	CString sRegPath = pRegPath;

	if (HKEY_CLASSES_ROOT == hkey)
	{
		sRegPath = CString("HKEY_CLASSES_ROOT\\") + sRegPath;
	}
	else if (HKEY_CURRENT_USER == hkey)
	{
		sRegPath = CString("HKEY_CURRENT_USER\\") + sRegPath;
	}
	else if (HKEY_LOCAL_MACHINE == hkey)
	{
		sRegPath = CString("HKEY_LOCAL_MACHINE\\") + sRegPath;
	}
	else if (HKEY_USERS == hkey)
	{
		sRegPath = CString("HKEY_USERS\\") + sRegPath;
	}
	else if (HKEY_PERFORMANCE_DATA == hkey)
	{
		sRegPath = CString("HKEY_PERFORMANCE_DATA\\") + sRegPath;
	}
#if (WINVER >= 0x0400)
	else if (HKEY_CURRENT_CONFIG == hkey)
	{
		sRegPath = CString("HKEY_CURRENT_CONFIG\\") + sRegPath;
	}
	else if (HKEY_DYN_DATA == hkey)
	{
		sRegPath = CString("HKEY_DYN_DATA\\") + sRegPath;
	}
#endif // (WINVER >= 0x0400)

	CString ErrMsg = sErrMsg + CString("\nKey Name = \"") + sRegPath + CString("\"");
	MessageBox(NULL, (LPCTSTR) ErrMsg, AfxGetAppName(), MB_OK | MB_ICONERROR);
}

// --------------------------------------------------------------------------

BOOL CEudoraApp::AddToRegistry(HKEY hkey, const char *regPath, const char *value, const char *valueData, bool bShowError /* = false */){
    HKEY  hkResult;
	LONG err = 0; 

	DWORD dwDisposition;

	err = RegCreateKeyEx(hkey, (LPCTSTR)regPath,0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	
	if (ERROR_SUCCESS == err)
	{
		err = RegSetValueEx(hkResult, value, 0, REG_SZ, (const unsigned char*)valueData, strlen(valueData)+1);
		RegCloseKey(hkResult);

		if ((bShowError) && (ERROR_SUCCESS != err))
		{
			HandleRegError(hkey, regPath, IDS_REG_FAILED_WRITE_ERR); // Could not write to the key
		}
	}
	else
	{
		if (bShowError)
			HandleRegError(hkey, regPath, IDS_REG_FAILED_OPEN_ERR); // Could not create/open the key
	}

	return ((ERROR_SUCCESS == err) ? TRUE : FALSE);
}

BOOL CEudoraApp::RegisterIcon()
{
	char buf[_MAX_PATH + 1]; 
	
	CRString appTitle(AFX_IDS_APP_TITLE);

    BOOL err = AddToRegistry(HKEY_CLASSES_ROOT, appTitle, NULL, appTitle);

	// register the icon
	appTitle += CRString(IDS_REG_ICON);

	if (GetModuleFileName(AfxGetInstanceHandle(), buf, sizeof(buf)))
	{
 		GetShortPathName(buf,buf, sizeof(buf));
		err = AddToRegistry(HKEY_CLASSES_ROOT, appTitle, NULL, buf);
    } 
	return (err);
} 

BOOL CEudoraApp::RegisterURLSchemes()
{
	char app[_MAX_PATH + 1]; 
	char cmd[_MAX_PATH + 1];
	char longApp[_MAX_PATH + 1];
		 
	GetModuleFileName(AfxGetInstanceHandle(), longApp, sizeof(longApp));
	GetShortPathName(longApp,app, sizeof(app));
	
	strcpy(cmd,app);
	lstrcat(cmd," /m %1");

	// What's currently registered?
	CRString RegMailto(IDS_REG_MAILTO);
	HKEY hkResult;
	char defMailto[_MAX_PATH + 1]; 
	long len = sizeof(defMailto) - 1;
	defMailto[0] = 0;
	if (RegOpenKey(HKEY_CLASSES_ROOT, RegMailto, &hkResult) == ERROR_SUCCESS)	// HCR\mailto\shell\open\command
	{
		RegQueryValue(hkResult,NULL, defMailto, &len);
		RegCloseKey(hkResult);	
	}

	LPCTSTR appName = app;
	LPCTSTR slash = strrchr(app, SLASH);
	if (slash)
		appName = ++slash;

	CMainFrame*	pMainFrame;
	pMainFrame = ( CMainFrame* ) AfxGetMainWnd();

	BOOL bSetDefaultMailto = FALSE;
	BOOL bIsDefaultMailto = FALSE;
	BOOL warn = GetIniShort(IDS_INI_WARN_DEFAULT_MAILTO);

	// Lets make sure that if Eudora is being launched by Netscape thru' the EudoraNS.dll, then we do not warn
	// if Eudora doesn't find itself as the default mailer Eudora IS THE default mailer. Netscape, even when Eudora's 
	// configured as the default mailer overwrites the above mentioned registry with it's own executable path.
	// EudoraNS.dll sets "EXT_EudoraNS_NoWarn" ini setting when Eudora is being launched by Netscape. 
	// Also see CEudoraApp::ProcessAttachmentWEI() function where a similar addition has been done.
	const BOOL bHasEudoraNSSetNotToWarn = GetIniShort(IDS_INI_EXTERNAL_EUDORANS_NOWARN);
	BOOL bRet;
	if (bHasEudoraNSSetNotToWarn)
	{
		// Set Warn to FALSE, which would accomplish what we intend here.
		warn = FALSE;
		bRet = ::WritePrivateProfileString(_T("Settings"),_T("EXT_EudoraNS_NoWarn"),_T("0"),INIPath);
	}

	const BOOL overWrite = GetIniShort(IDS_INI_DEFAULT_MAILTO_OVERWRITE);

	if (defMailto[0] && !strstr(defMailto, appName)) 
	{
		if (!gbAutomationRunning)
		{
			if ((!warn && overWrite) || warn &&
				(YesNoDialog(IDS_INI_WARN_DEFAULT_MAILTO, IDS_INI_DEFAULT_MAILTO_OVERWRITE,IDS_WARN_DEFAULT_MAILTO) == IDOK))

			{
				bSetDefaultMailto = TRUE;							
			}
		}
	}
	else if (stricmp(defMailto, cmd))
	{
		// We are the default program, but if we are a new version, register this new path in place of the old one..
		bSetDefaultMailto = TRUE;
	}

	if (bSetDefaultMailto || strstr(defMailto, appName))
		bIsDefaultMailto = TRUE;

	if (!gbAutomationRunning)
	{
		if (bIsDefaultMailto)
		{
			// Set the global variable here which is then used in the ddeserve.cpp in CDDEServer::OnRegister()
			g_bDefaultMailto = TRUE;
			// Register Eudora as Netscape's mailto URL handler
			pMainFrame->SendMessage(WM_USER_REGISTER_MAILTO,MAKEWPARAM(0,1));	
		}
		else
		{
			// Eudora is not the default emailer
			pMainFrame->SendMessage(WM_USER_REGISTER_MAILTO,MAKEWPARAM(0,2));

		}
	}
	//}

	CRString MailtoURLProtocol(IDS_REG_HLM_MAILTO_PROT_URL);
	if (bSetDefaultMailto)
	{
		if (!AddToRegistry(HKEY_CLASSES_ROOT, RegMailto, NULL, cmd))		// HCR\mailto\shell\open\command
			::ErrorDialog(IDS_REG_MAILTO_ERR);
		int i = RegMailto.Find('\\');
		if (i >= 0)
			RegMailto.ReleaseBuffer(i);
		AddToRegistry(HKEY_CLASSES_ROOT, RegMailto, MailtoURLProtocol, "");	// HCR\mailto\URL Protocol
	}

	// These lines will make Eudora one of the mail programs that can be selected
	// in the Internet Options.  It doesn't actually make it the default, so we
	// can always set these entries no matter what.

	// This will work for IE 3.0+
	CRString HLMMailto(IDS_REG_HLM_MAILTO);
	AddToRegistry(HKEY_LOCAL_MACHINE, HLMMailto, NULL, CRString(IDS_EUDORA));							// HLM\Software\Clients\Mail\Eudora
	AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAIL_SHELL_COMMAND), NULL, app);				// HLM\Software\Clients\Mail\Eudora\shell\open\command
	AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_PROT), NULL, "URL:MailTo Protocol");	// HLM\Software\Clients\Mail\Eudora\Protocols\mailto
	AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_PROT), MailtoURLProtocol, "");		// HLM\Software\Clients\Mail\Eudora\Protocols\mailto\URL Protocol
	AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_PROT_COMMAND), NULL, cmd);			// HLM\Software\Clients\Mail\Eudora\Protocols\mailto\shell\open\command
	AddToRegistry(HKEY_LOCAL_MACHINE, CRString(IDS_REG_HLM_MAILTO_ICON), NULL, app);					// HLM\Software\Clients\Mail\Eudora\Protocols\mailto\DefaultIcon

	// This is for IE 4.0+
	char MapiDLL[_MAX_PATH + 1];
	strcpy(MapiDLL, app);
	char* ExeStart = strrchr(MapiDLL, '\\') + 1;
	strcpy(ExeStart, "EuMAPI32.dll");
	AddToRegistry(HKEY_LOCAL_MACHINE, HLMMailto, "DLLPath", MapiDLL);

	// Here's where we actually make Eudora the default mail program in the Internet Options
	if (bIsDefaultMailto)
	{
		CString RegClientsMail(HLMMailto);
		int LastSlash = RegClientsMail.ReverseFind('\\');
		if (LastSlash >= 0)
		{
			RegClientsMail.ReleaseBuffer(LastSlash);	// HLM\Software\Clients\Mail
			AddToRegistry(HKEY_LOCAL_MACHINE, RegClientsMail, NULL, CRString(IDS_EUDORA));							// HLM\Software\Clients\Mail\Eudora
		}
	}

	// Register the x-Eudora-option: URL scheme for the OS
	CRString EudoraOption(IDS_REG_EUDORA_OPTION);
	AddToRegistry(HKEY_CLASSES_ROOT, EudoraOption, NULL, cmd);					// HCR\x-eudora-option\shell\open\command
	int SlashIndex = EudoraOption.Find('\\');
	if (SlashIndex >= 0)
		EudoraOption.ReleaseBuffer(SlashIndex);
	AddToRegistry(HKEY_CLASSES_ROOT, EudoraOption, MailtoURLProtocol, "");	// HCR\x-eudora-option\URL Protocol

	return TRUE;
} 

BOOL CEudoraApp::RegisterStationery()
{
	char app[_MAX_PATH + 6];
	char icon[_MAX_PATH + 10];
	char	dllPath[_MAX_PATH];
	char *	temp = NULL;

	app[0] = '\0';		// good hygiene
	dllPath[0] = '\0';		// good hygiene

  	if (GetModuleFileName(AfxGetInstanceHandle(), app, sizeof(app)))
	{
		strcpy(dllPath, app);
		temp = strrchr(dllPath, SLASH);
		if (temp)
		{
			temp[1] = 0;
		}
	}

	if ( dllPath[ 0 ] == '\0' )
		strcpy( dllPath, "\\" );


	if ( dllPath[ strlen( dllPath ) - 1 ] != '\\' )
		strcat( dllPath, "\\" );

	strcat( dllPath, "EudoraRes.dll" );
	
 	GetShortPathName(app,app, sizeof(app));
	lstrcat(app," \"%1\"");

	sprintf(icon,"%s,-%d", dllPath,IDR_STATIONERY);
	CRString szStationeryName(IDS_REG_STATIONERY_NAME);
	szStationeryName.Replace(".", " ");

	
	
	BOOL bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_STATIONERY_EXT), NULL, CRString(IDS_REG_STATIONERY_NAME));
	if(bRes)
		bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_STATIONERY_NAME), NULL, szStationeryName);
	if(bRes)
		bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_STATIONERY), NULL, app);
	if(bRes)
		bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_STATIONERY_ICON), NULL, icon);


	return bRes;
} 

BOOL CEudoraApp::RegisterMailbox()
{
	char app[_MAX_PATH + 6];
	char icon[_MAX_PATH + 10];
	char	dllPath[_MAX_PATH];
	char *	temp = NULL;


	app[0] = '\0';		// good hygiene
	dllPath[0] = '\0';		// good hygiene

  	if (GetModuleFileName(AfxGetInstanceHandle(), app, sizeof(app)))
	{
		strcpy(dllPath, app);
		temp = strrchr(dllPath, SLASH);
		if (temp)
		{
			temp[1] = 0;
		}
	}

	if ( dllPath[ 0 ] == '\0' )
		strcpy( dllPath, "\\" );


	if ( dllPath[ strlen( dllPath ) - 1 ] != '\\' )
		strcat( dllPath, "\\" );

	strcat( dllPath, "EudoraRes.dll" );
	GetShortPathName(app,app, sizeof(app));
	lstrcat(app," \"%1\"");

	sprintf(icon,"%s,-%d", dllPath,IDI_MAILBOX);

	CRString szMailboxName(IDS_REG_MAILBOX_NAME);
	szMailboxName.Replace(".", " ");

	
	
	BOOL bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_MAILBOX_EXT), NULL, CRString(IDS_REG_MAILBOX_NAME));
	if(bRes)
		bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_MAILBOX_NAME), NULL, szMailboxName);
	if(bRes)
		bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_MAILBOX), NULL, app);
	if(bRes)
		bRes = AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_MAILBOX_ICON), NULL, icon);


	return bRes;
} 


BOOL CEudoraApp::RegisterCommandLine(char *cmdLine)
{
    HKEY  hkResult;
	DWORD dwDisposition;
	LONG err = 0;

	char val[_MAX_PATH + _MAX_PATH + _MAX_PATH + 3]; // 3 = 2 spaces and a NULL
	char app[_MAX_PATH + 1];
	char eDir[_MAX_PATH + 1];
	char iniPath[_MAX_PATH + 1];

	val[0] = app[0] = eDir[0] = iniPath[0] = '\0';

	//strip	tailing SLASH
	strcpy(eDir,EudoraDir);
	if (eDir[strlen(eDir)-1] == SLASH)
		eDir[strlen(eDir)-1] = 0;
		 
	GetModuleFileName(AfxGetInstanceHandle(), app, sizeof(app)); 
 	VERIFY(GetShortPathName(app,app, sizeof(app)) > 0);
 	VERIFY(GetShortPathName(eDir,eDir, sizeof(eDir)) > 0);
 	VERIFY(GetShortPathName(INIPath,iniPath, sizeof(iniPath)) > 0);
	sprintf(val, "%s %s %s", app, eDir, (const char *)iniPath);

	// register command line in the root
	AddToRegistry(HKEY_CLASSES_ROOT, CRString(IDS_REG_SOFTWARE), NULL, val);

	// register command line in the current user
	char buf[_MAX_PATH + 1];
	strcpy(buf, CRString(IDS_REG_SOFTWARE_COMMAND));
	err = RegCreateKeyEx(HKEY_CURRENT_USER, (LPCTSTR)buf,0,NULL,	
    						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
    						&hkResult,&dwDisposition);
	
	strcpy(buf, CRString(IDS_REG_SOFTWARE_CURRENT));
	err = RegSetValueEx(hkResult,buf,0,REG_SZ,(const unsigned char*)val,strlen(val)+1);
	RegCloseKey(hkResult);

	return ( (BOOL)err );	
}

BOOL CEudoraApp::StartEudoraWithDefaultLocation()
{
    HKEY  hkResult;
	DWORD dwType = REG_SZ;
	TCHAR buf[MAX_PATH + 1];
	TCHAR cmdLine[MAX_PATH + 1];
	LONG err = 0;

	// First look for UseAppData in the DEudora.ini file
	if (!DefaultINIPath.IsEmpty() &&
		GetPrivateProfileString(CRString(IDS_INISN_SETTINGS), "UseAppData", "", buf, sizeof(buf), DefaultINIPath) &&
		atol(buf))
	{
		// We found it, and it has a non-zero value.  Empty the buffer so that we use APPDATA below.
		buf[0] = 0;
	}
	else
	{
		// Next, look in the registry for the last place Eudora started up
		unsigned long buflen = sizeof(cmdLine);

		// HKEY_CURRENT_USER registry
		strcpy(buf, CRString(IDS_REG_SOFTWARE_COMMAND));
		err = RegOpenKeyEx(HKEY_CURRENT_USER, (LPCTSTR)buf,0,KEY_ALL_ACCESS,&hkResult);
		strcpy(buf, "Current");
		err = RegQueryValueEx(hkResult,buf, NULL, &dwType, (unsigned char*)cmdLine, &buflen);
		RegCloseKey(hkResult);
		
		if (err)
		{
			// root registry
			strcpy(buf, CRString(IDS_REG_SOFTWARE));
			err = RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)buf,0,KEY_ALL_ACCESS,&hkResult);
			err = RegQueryValueEx(hkResult,NULL, NULL, &dwType, (unsigned char*)cmdLine, &buflen);
			RegCloseKey(hkResult);
		}

		buf[0] = 0;
		char* c;
		if (*cmdLine == '"')
			c = strchr(cmdLine + 1, '"');
		else
			c = strchr(cmdLine, ' ');
		if (c)
		{
			c++;
			while (*c == ' ')
				c++;
			strcpy(buf, c);
		}
	}

	// If no data directory found yet, then default to the APPDATA directory. SHGetSpecialFolderPath()
	// and the CSIDL_APPDATA special folder only exist under the 4.71 version of the shell, which means
	// the user has to have installed IE 4.0 or greater (which is always true of Win 98 and Win 2000).
	if (!*buf)
	{
		HMODULE hShell32 = GetModuleHandle("Shell32.dll");

		if (!hShell32)
			ASSERT(0);	// Not loading Shell32 is a very bad thing
		else
		{
			typedef BOOL (__stdcall *IndirectSHGetSpecialFolderPath)(HWND, LPTSTR, int, BOOL);
#ifdef UNICODE
			IndirectSHGetSpecialFolderPath pFn = (IndirectSHGetSpecialFolderPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathW");
#else
			IndirectSHGetSpecialFolderPath pFn = (IndirectSHGetSpecialFolderPath)GetProcAddress(hShell32, "SHGetSpecialFolderPathA");
#endif
			TCHAR FullPath[_MAX_PATH + 1];
			if (pFn && (*pFn)(NULL, FullPath, CSIDL_APPDATA, TRUE))
			{
				TCHAR AppDataParent[MAX_PATH + 1];
				_tcscpy(AppDataParent, FullPath);
				*(_tcsrchr(AppDataParent, '\\') + 1) = 0;

				_tcscat(FullPath, CRString(IDS_APPDATA_SUBDIR1));
				if (CreateDirectoryMT(FullPath))
				{
					_tcscat(FullPath, CRString(IDS_APPDATA_SUBDIR2));

					// Create shortcut to data folder that won't be hidden so that
					// users can easily get to it.  Only do this if the directory
					// doesn't exist because it will cause all sorts of DLLs to be
					// loaded up.  We have to create the shortcut after the
					// directory is created, though.
					const BOOL bDirExisted = FileExistsMT(FullPath);

					if (CreateDirectoryMT(FullPath))
						GetShortPathName(FullPath, buf, sizeof(cmdLine)/sizeof(TCHAR));

					if (!bDirExisted)
						CreateShortcutMT(FullPath, AppDataParent);
				}
			}
		}
	}
	 
	if (err || !GetDirs(buf))
	{
		ErrorDialog(IDS_ERR_CANNOT_START_WITH_ATTACH);
		return FALSE;
	}
	
	return (TRUE);	
}


// ProcessAttachmentWithExistingInstance()
BOOL CEudoraApp::ProcessAttachmentWEI(int CmdType)
{
	HWND otherEudoraHWND = ::FindWindow(EudoraMainWindowClass,NULL);

	if (otherEudoraHWND)
	{ 
		// if command line contained attachment have running eudora attach it
		if (m_lpCmdLine && *m_lpCmdLine && 
				( CmdType == CL_ATTACH		|| 
				  CmdType == CL_STATIONERY	||  
				  CmdType == CL_URL			|| 
				  CmdType == CL_MESSAGE		||
				  CmdType == CL_MAILBOX ) )
		{
	    	COPYDATASTRUCT cds;
		
			cds.dwData = 1;							//  Specify this is command line
			cds.lpData = m_lpCmdLine;				//  Point to text
			cds.cbData = strlen(m_lpCmdLine) + 1;	//  Length of string

			::SendMessage(otherEudoraHWND, WM_COPYDATA, NULL, (LPARAM)&cds);
		}
		
		if (::IsIconic(otherEudoraHWND)) // Check if app is minimized
			::ShowWindow(otherEudoraHWND, SW_RESTORE); // Activate and display app, restoring from minimized
		else
			::SetForegroundWindow(otherEudoraHWND); // Bring app to front

		return TRUE;
	}
	return FALSE;
}

BOOL CEudoraApp::DoCommandLineActions(LPCTSTR CommandLine)
{
	if (!CommandLine)
		return FALSE;
	COMMANDLINE_TYPE CmdType = GetCommandType(CommandLine);

	switch (CmdType)
	{
		case CL_URL:
		{
			LPCTSTR UrlStart = CommandLine;
			if (strnicmp(UrlStart, "/m", 2) == 0)
				UrlStart += 2;
			while (*UrlStart == ' ')
				UrlStart++;
			LaunchURLNoHistory(UrlStart);
			return TRUE;
		}

		case CL_ATTACH:
			((CMainFrame*)AfxGetMainWnd())->CreateMessage(CommandLine);
			return TRUE;

		case CL_STATIONERY:
		{
			CCompMessageDoc* Comp = NewCompDocumentWith(CommandLine);
			if (Comp)
				Comp->m_Sum->Display();
			return TRUE;
		}

		case CL_MESSAGE:
			NewMessageFromFile(CommandLine);
			return TRUE;

		case CL_MAILBOX:
				{
					QCMailboxCommand * pMbox = NULL;
					char szCommand[_MAX_PATH];
					strcpy(szCommand, CommandLine);
					char * lpszCommand = strchr(szCommand, '\"');
					if(lpszCommand)
					{
						lpszCommand++;
						char * lpszTmp2 = strrchr(lpszCommand, '\"');
						if(lpszTmp2)
							*lpszTmp2 = 0;

					}
					else
					{
						lpszCommand = szCommand;
					}

					CFileStatus cfStatus;
					
					if(CFile::GetStatus(lpszCommand, cfStatus))
					{
						char szShort[_MAX_PATH];

						GetShortPathName(lpszCommand, szShort, sizeof(szShort));

						if(strlen(szShort) > 1)
							lpszCommand = szShort;

						pMbox = g_theMailboxDirector.FindByPathname(lpszCommand);
						if(pMbox)
						{
							pMbox->Execute(CA_OPEN);
							return TRUE;
						}
						else
						{
							// Temp Mailbawx
							char szName[_MAX_PATH];
							char * lpszName;
							strcpy(szName, lpszCommand);
							lpszName = strrchr(szName, '\\');

							if(!lpszName)
								return FALSE; // oh-oh

							lpszName++;

							WIN32_FIND_DATA wfd;
							CTocDoc * pTocDoc = NULL;

							HANDLE hFind = FindFirstFile(lpszCommand, &wfd);
							
							if(hFind)
							{
								FindClose(hFind);
								pTocDoc = GetToc(lpszCommand, wfd.cFileName);

							}
							else
							{
								pTocDoc = GetToc(lpszCommand, lpszName);
							}



							if(pTocDoc)
							{
								pTocDoc->Display();
								
							}

						}
					}
					else
						return FALSE; // No valid file passed on command line

					return TRUE;
				}

	}

	return FALSE;
}


BOOL CEudoraApp::IsEudoraRunning()
{
	// Allow multiple instances on different working directories
	if ( GetIniShort(IDS_INI_OWNER_LOK) && ! EudoraDir.IsEmpty() )
	{
		CString cOwnerFile(EudoraDir);
		cOwnerFile += "OWNER.LOK";
		while (1)
		{
			m_fdOwner = open( cOwnerFile, O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE );
			if ( m_fdOwner != -1 )
			{
				// try to read an owner id
				DWORD dwTemp;
				int bytes = read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
				if ( bytes == sizeof(DWORD) && dwTemp )
				{
					// someone's already using this Eudora directory - are they local?
					m_hOwner = (HWND)dwTemp;
					if ( IsLocalInstance( m_hOwner ) )
					{
						return LocalLateral();
					}

					int bytes = read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
					if ( ( bytes == sizeof(DWORD) && dwTemp == REMOTE_INSTANCE ) || 
						! IsLocalPath( cOwnerFile ) )
					{
						const BOOL bStopRunning = RemoteInstance();
						// If told to exit remote instance, then wait for
						// the remote instance to go away and try again
						if (!bStopRunning && m_hOwner == 0)
						{
							for (int i = 0; i < 10; i++)
							{
								if (!FileExistsMT(cOwnerFile))
									break;
								Sleep(2000);
							}
							continue;
						}

						return bStopRunning;
					}

					// it's not a local instance nor was it a remote instance
					// assume it's a stale local instance
					m_hOwner = NULL;
				}
			}
			break;
		}
	}

	return FALSE;	// let it rip...
}

BOOL CEudoraApp::IsLocalInstance( HWND hWnd )
{
	char buf[ 80 ];

	// try to find EUDORA in the window's title bar text
	int bytes;
	if ( ::IsWindow(hWnd) )
	{
		buf[ sizeof(buf) - 1 ] = '\0';
		if ( bytes = ::GetWindowText(hWnd, buf, sizeof(buf) - 1) )
		{
			strupr( buf );
			if ( strstr( buf, "EUDORA" ) )
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CEudoraApp::IsLocalPath( const char* csPath )
{
	BOOL ret = TRUE;
	char path[ _MAX_PATH ];
	int drive;

	strncpy( path, csPath, sizeof(path) - 1 );
	path[ sizeof(path) - 1 ] = 0;

	if ( strlen( path ) >= 2 )
	{
		if ( path[ 1 ] == ':' )
		{
			// non-UNC fully qualified path
			path[ 3 ] = 0;
			strupr( path );
		}
		else if ( strstr( path, "\\\\" ) == path )
		{
			// UNC fully qualified path
			// UNCs are \\machine\sharename\ - find end of machine name
			char * end = strchr( &path[ 2 ], '\\' );
			if ( end )
			{
				// now find the end of the sharename
				end = strchr( end + 1, '\\' );
				if ( end )
					*(end + 1) = 0;
			}
		}
		else
		{
			drive = _getdrive();		// 1 = 'A', 2 = 'B'
			drive -= 1;					// make em 0-indexed

			path[ 0 ] = char(drive + 'A');
			path[ 1 ] = ':';
			path[ 2 ] = '\\';
			path[ 3 ] = 0;
		}

		// path now has the drive's root path

		// GetDriveType() isn't documented as working with UNCs under 95...
		// if it fails we'll return IsLocal == TRUE which is not that bad.
		// (I'd bet the documentation is wrong; this probably does work under 95)
		UINT type = GetDriveType( path );
		if ( type == DRIVE_REMOTE || type == DRIVE_UNKNOWN  )
			ret = FALSE;
	}

	return ret;
}

BOOL CEudoraApp::LocalLateral()
{
	if ( ::IsWindow( m_hOwner )	)
		::SetForegroundWindow( m_hOwner );

	return TRUE;	// causes this instance to terminate
}

BOOL CEudoraApp::RemoteInstance()
{
	CRemoteInstance theDlg;

	int result = theDlg.DoModal();
	switch ( result )
	{
	case CRemoteInstance::TERMINATE_REMOTE :
		// causes our HWND to be written to OWNER.LOC
		// this will subsequently cause a well-behaved Eudora to teminate itself
		m_hOwner = 0;
		RegisterOwnership();
		break;

	case CRemoteInstance::IGNORE_REMOTE :
		// leaves m_hOwner set to the remote's HWND
		// returning FALSE below causes this instance to continue
		break;

	case CRemoteInstance::TERMINATE_LOCAL :
	default:
		return TRUE;	// causes this instance to terminate
		break;
	}

	return FALSE;	// causes this instance to continue
}

void CEudoraApp::RegisterOwnership( void )
{
	HWND hOurWnd = m_pMainWnd? m_pMainWnd->m_hWnd : 0;

	// m_hOwner could be a remote instance (we might be sneekin' in the backdoor)
	if ( ! m_hOwner )
	{
		if ( m_fdOwner != -1 )
		{
			// make us the owner
			lseek( m_fdOwner, 0L, SEEK_SET );
			DWORD	dwTemp = (DWORD)hOurWnd;
			write( m_fdOwner, &dwTemp, sizeof( DWORD ) );

			if (hOurWnd && IsLocalPath( EudoraDir ) )
				dwTemp = LOCAL_INSTANCE;
			else
				dwTemp = REMOTE_INSTANCE;

			write( m_fdOwner, &dwTemp, sizeof( DWORD ) );

			// make sure it gets to disk...
			if (hOurWnd)
				_commit( m_fdOwner );
			else
			{
				close(m_fdOwner);
				m_fdOwner = -1;
			}
		}
		
		m_hOwner = hOurWnd;	// we are the owner
	}

	// save our hWnd for use in OwnershipCleanup() and runtime checks
	// m_pMainWnd->m_hWnd disappears early in the shutdown process
	m_hOurWnd = hOurWnd;
}

BOOL CEudoraApp::CheckOwnership( void )
{
	// see if we're still owners of the mailbox
	HWND hOwnerWnd = NULL;

	if ( m_fdOwner != -1 )
	{
		// get the current owner
		long retl = lseek( m_fdOwner, 0L, SEEK_SET );
		if ( retl != -1 )
		{
			DWORD	dwTemp;
			int reti = read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
			if ( reti == sizeof( DWORD ) )
				hOwnerWnd = (HWND)dwTemp;
		}

		// if not, then terminate the app
		if ( m_hOwner && hOwnerWnd != m_hOwner )
		{
#ifdef _DEBUG
			AfxMessageBox("PostQuitMessage() in CheckOwnership");
			ASSERT(0);
#endif
			::PostQuitMessage( 0 );
			return FALSE;
		}
	}

	return TRUE;
}

void CEudoraApp::OwnershipCleanup( void )
{
	HWND hOwnerWnd = NULL;

	if ( m_fdOwner != -1 )
	{
		// get the current owner
		lseek( m_fdOwner, 0L, SEEK_SET );
		DWORD	dwTemp;
		read( m_fdOwner, (LPVOID)&dwTemp, sizeof( DWORD ) );
		hOwnerWnd = (HWND)dwTemp;

		// close the ownership file
		close( m_fdOwner );
		m_fdOwner = -1;

		// if we owned it, then get rid of the file
		if ( m_hOurWnd && ( !hOwnerWnd || hOwnerWnd == m_hOurWnd ) )
		{
			CString cOwnerFile = EudoraDir;
			cOwnerFile += "OWNER.LOK";
			unlink( cOwnerFile );
		}
	}
}

void CEudoraApp::WinHelp(DWORD dwData, UINT nCmd /*=NULL*/)
{	
	// Clicking Shift-F1 and selecting a menu item
	//  brings up the help topic in a secondary
	//  window.  Better to display it in a popup.
	if ( (nCmd==NULL) || ( nCmd==HELP_CONTEXT )	)
		nCmd=HELP_CONTEXTPOPUP;

	CWinApp::WinHelp(dwData, nCmd);
 	return; 
}           

void CEudoraApp::SetExternalCheckMail(BOOL bWaitForIdle)
{

	if ( bWaitForIdle )
		SetNextMailCheck(time(NULL));
	else
		m_ForceCheckMail = TRUE; 
}


void CEudoraApp::OnFileOpen()
{
	CFileDialog dlg(TRUE,
					CRString(IDS_TEXT_EXTENSION),
					NULL,
					OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT, 
					//CRString(IDS_STATIONERY_FILE_FILTER32),
					CRString(IDS_MAIN_OPENFILE_FILTER),
					AfxGetMainWnd());
	
	if (dlg.DoModal() == IDOK)
	{
		CString fileSel = dlg.GetPathName(); 
		fileSel.MakeLower();
		CString sta = "." + CRString(IDS_STATIONERY_EXTENSION);
		if (fileSel.Find(sta) > 0)
		{
			CCompMessageDoc* Comp = NewCompDocumentWith(fileSel);
			Comp->SetIsStationery();
				
			if (Comp)
				Comp->m_Sum->Display();
		}

		else
			CWinApp::OpenDocumentFile(fileSel);
	}
}

void CEudoraApp::OnFileNew()
{
	//Had to explicitly use the TextFileTemplate bcos even the
	//Signature template uses IDR_TEXT_FILE which causes the DocManager
	//to put up a Dialog box to choose between the two.
	TextFileTemplate->OpenDocumentFile(NULL, TRUE);	
}

CEudoraApp::ResourceStatus CEudoraApp::LoadResourceDLL( void )
{
	char	appPath[_MAX_PATH];
	char	dllPath[_MAX_PATH];
	char *	temp;
	ResourceStatus status = RESOURCE_NOT_LOADED;

	appPath[0] = '\0';		// good hygiene
	dllPath[0] = '\0';		// good hygiene

	if ( ! m_pResList )
		m_pResList = DEBUG_NEW_MFCOBJ_NOTHROW CPtrList;

	if ( m_pResList )
	{
  		if (GetModuleFileName(AfxGetInstanceHandle(), appPath, sizeof(appPath)))
		{
			strcpy(dllPath, appPath);
			temp = strrchr(dllPath, SLASH);
			if (temp)
			{
				temp[1] = 0;
			}
		}

		if ( dllPath[ 0 ] == '\0' )
			strcpy( dllPath, "\\" );


		if ( dllPath[ strlen( dllPath ) - 1 ] != '\\' )
			strcat( dllPath, "\\" );

		strcat( dllPath, "EudoraRes.dll" );

		HINSTANCE hDLL = LoadLibrary( dllPath );
		if ( hDLL )
		{
			FARPROC pfInitDLL = GetProcAddress( hDLL, "INITDLL" );
				
			// 32-bit resource DLLs and plug-ins do not have an INITDLL fuction
			if ( ! pfInitDLL )
			{
				m_pResList->AddHead( (LPVOID)(DWORD)hDLL );
				status = RESOURCE_OKAY;
			}

			if ((RESOURCE_OKAY == status) && (!::IsMatchingResourceDLL(appPath, dllPath)))
				status = RESOURCE_VERSION_MISMATCH;
		}
	}

	return status;
}

void CEudoraApp::LoadEPIs( void )
{
	if ( ! m_pResList )
		m_pResList = DEBUG_NEW_MFCOBJ_NOTHROW CPtrList;

	// last dll added is searched first my MFC
	if ( m_pResList )
	{
		// check the application dir
		AddEPIs( ExecutableDir );

		// check the data dir if different from the application dir
		if (_tcsicmp(ExecutableDir, EudoraDir))
			AddEPIs( EudoraDir );
	}
}

void CEudoraApp::FreeResources( void )
{
	// walk the list freeing libs as we go
	// finally free the list itself
	if ( m_pResList )
	{
		POSITION pos;

		for( pos = m_pResList->GetHeadPosition(); pos != NULL; )
		{
			HINSTANCE hInst = (HINSTANCE)m_pResList->GetNext( pos );
			if ( hInst && hInst != AfxGetInstanceHandle() )
				FreeLibrary( hInst );
		}

		delete m_pResList;
	}
}

void CEudoraApp::AddEPIs( const char* clsDirectory )
{
	if ( m_pResList )
	{
		CString clsPath;
	
		FindFirstEPI( clsDirectory, clsPath );

		while ( ! clsPath.IsEmpty() )
		{
			// add the resource dll
			HINSTANCE hDLL = ::LoadLibrary( clsPath );
			if ( hDLL )
			{
				FARPROC pfInitDLL = ::GetProcAddress( hDLL, "INITDLL" );
				
				// 32-bit resource DLLs and plug-ins do not have an INITDLL fuction
				if (!pfInitDLL)
				{
					// If we've already seen this resource plug-in DLL, then don't add it again.
					// Should never happen because we check if the program and data directories
					// are the same, but this is just in case.
					if (m_pResList->Find((LPVOID)(DWORD)hDLL))
						::FreeLibrary(hDLL);
					else
						m_pResList->AddHead((LPVOID)(DWORD)hDLL);
				}
				else
					::FreeLibrary(hDLL);
			}
			
			// look for another
			FindNextEPI( clsPath );
		}
	}
}

void CEudoraApp::FindFirstEPI( const char* clsDirectory, CString & clsEPIPath )
{
	// build up a sorted list of all EPIs
	CString	TmpPath;

	m_EPIList.RemoveAll();

	FindFirstEPIFile( clsDirectory, TmpPath );

	while ( ! TmpPath.IsEmpty() )
	{
		m_EPIList.Add( TmpPath );
		
		// look for another
		FindNextEPIFile( TmpPath );
	}

	// now start walking the sorted list 'z'..'a'
	m_Pos = m_EPIList.GetTailPosition();

	FindNextEPI( clsEPIPath );
}

void CEudoraApp::FindNextEPI( CString & clsEPIPath )
{
	if ( m_Pos )
		clsEPIPath = m_EPIList.GetPrev( m_Pos );
	else
		clsEPIPath.Empty();
}

void CEudoraApp::FindFirstEPIFile( const char* clsDirectory, CString & clsEPIPath )
{
	char	szFileSpec[ 255 ];

	// build up the wildcard specifier for Eudora Plug In's
	strcpy( szFileSpec, clsDirectory );
	if ( szFileSpec[ 0 ] == 0 )
		strcpy( szFileSpec, "\\" );

	if ( szFileSpec[ strlen( szFileSpec ) - 1 ] != '\\' )
		strcat( szFileSpec, "\\" );

	m_clsFindDir = szFileSpec;

	strcat( szFileSpec, "*.EPI" );

	// see if any exist
	if (m_hFindHandle != INVALID_HANDLE_VALUE) 
	{
		FindClose( m_hFindHandle );
		m_hFindHandle = INVALID_HANDLE_VALUE;
	}

	m_FindData.dwFileAttributes = _A_NORMAL;

	m_hFindHandle = FindFirstFile( szFileSpec, &m_FindData );
	if (m_hFindHandle == INVALID_HANDLE_VALUE) 
	{
		clsEPIPath = "";
	}
	else
	{
		clsEPIPath = m_clsFindDir;
		clsEPIPath += m_FindData.cFileName;
	}
}

void CEudoraApp::FindNextEPIFile( CString & clsEPIPath )
{
	BOOL bResult;

	bResult = FindNextFile( m_hFindHandle, &m_FindData );
	if ( !bResult )
	{
		FindClose( m_hFindHandle );
		m_hFindHandle = INVALID_HANDLE_VALUE;
		clsEPIPath = "";
	}
	else
	{
		clsEPIPath = m_clsFindDir;
		clsEPIPath += m_FindData.cFileName;
	}
}

BOOL CEudoraApp::InitTranslators()
{
	CString transDir = EudoraDir + CRString(IDS_TRANS_DIRECTORY);
	CString transExeDir = ExecutableDir + CRString(IDS_TRANS_DIRECTORY);

	// Check to see if these are one in the same
	int numDir = 2;
	if (!transDir.CompareNoCase(transExeDir))
		numDir--;

	// see if we need to create "Trans" directory
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)transExeDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
		mkdir(transExeDir);
	else
		FindClose(findHandle);

	// Load 'em up
	m_TransManager = DEBUG_NEW CTranslatorManager((short)numDir, transDir, transExeDir);

	// Dump a trace of the translators loaded for debugging
#ifdef _DEBUG
	{
		CTranslator *t;
		CString		desc;
		short index = 0;
		while( t = m_TransManager->GetNextTranslator(&index))
		{
			t->getDescription(desc);
			TRACE1("Description: %s\n", desc);
		}
	}
#endif
	
	return TRUE;
}

void CEudoraApp::RegisterFormats()
{
	cfEmbeddedObject = (CLIPFORMAT)::RegisterClipboardFormat(_T("Embedded Object"));
	cfRTF = (CLIPFORMAT)::RegisterClipboardFormat(_T(CF_RTF));
	cfRTO = (CLIPFORMAT)::RegisterClipboardFormat(_T(CF_RETEXTOBJ));
	cfRTWO= (CLIPFORMAT)::RegisterClipboardFormat(_T(CF_RTFNOOBJS));

	VERIFY(g_theClipboardMgr.RegisterAllFormats());
}


//
// FilterList functions
//
//

void CEudoraApp::AddFilterListEntry( 
CObject* pEntry )
{
	BOOL		bTryAgain;
	POSITION	pos;

	bTryAgain = FALSE;
	
	do
	{
		try
		{
			m_FilterList.AddTail( pEntry );
		}
		catch( CMemoryException*	pExp )
		{	
			pExp->Delete();
			
			bTryAgain = !bTryAgain;
			
			if( bTryAgain )
			{
				// make some room by blowing away the oldest entry
				
				pos = m_FilterList.GetHeadPosition();
				
				if ( pos )
				{
					delete m_FilterList.RemoveHead();
				}

			}
			else
			{
				ErrorDialog( AFX_IDS_MEMORY_EXCEPTION );
			}
		}
	}
	while( bTryAgain );
}



const CObList&	CEudoraApp::GetFilterList()
{
	return m_FilterList;
}


void CEudoraApp::CleanupIcons()
{
	char Filename[_MAX_PATH + 1];

	CString iconDir = EudoraDir + CRString(IDS_ICON_FOLDER); 
	iconDir.MakeLower();

	wsprintf( Filename, "%s\\%s", iconDir, "*.jpg" );

	WIN32_FIND_DATA	Find;
	HANDLE			FindHandle;

	Find.dwFileAttributes = _A_NORMAL;
	FindHandle = FindFirstFile(Filename,&Find);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		int Result = 1;
		while (Result)
		{
			wsprintf( Filename, "%s\\%s", iconDir, Find.cFileName );
			TRY
			{
				CFile::Remove( Filename );
			}
			CATCH( CFileException, e )
			{
				TRACE("Failed to delete %s\n", Filename);
			}
			END_CATCH
			Result = FindNextFile(FindHandle,&Find);
		}

		FindClose(FindHandle);
	}
}

void CEudoraApp::MakeEmbeddedDir()
{
	CString embeddedDir;

	WIN32_FIND_DATA wfd;
	embeddedDir = EudoraDir + CRString(IDS_EMBEDDED_FOLDER); 

	// see if we need to create "embedded" directory
	HANDLE findHandle;
	findHandle = FindFirstFile((const char *)embeddedDir,&wfd);
	if ( findHandle == INVALID_HANDLE_VALUE)
	{
		if (mkdir(embeddedDir) < 0)
			ASSERT( 0 );
	}
	else
		FindClose(findHandle);
}

/////////////////////
//
// Eudora.ini reset
//
/////////////////////

static void ResetEudoraIni(void)
{
	const UINT settingsToKeep[] =
	{
		// getting started
		IDS_INI_REAL_NAME,
		IDS_INI_RETURN_ADDRESS,
		IDS_INI_POP_SERVER,
		IDS_INI_POP_ACCOUNT,
		IDS_INI_LOGIN_NAME,
		IDS_INI_SMTP_SERVER,

		// ACAP
		IDS_INI_USE_ACAP_SERVER,
		IDS_INI_ACAP_SERVER,
		IDS_INI_ACAP_USER_ID,
		IDS_INI_ACAP_PASSWORD,
		IDS_INI_ACAP_PORT,

		// sending/receiving mail
		IDS_INI_USES_POP,
		IDS_INI_USE_POP_SEND,
		IDS_INI_AUTO_RECEIVE_DIR,
		IDS_INI_LEAVE_MAIL_ON_SERVER,
		IDS_INI_DELETE_MAIL_FROM_SERVER,
		IDS_INI_SAVE_PASSWORD,
		IDS_INI_SAVE_PASSWORD_TEXT,
		IDS_INI_AUTH_KERB,
		IDS_INI_AUTH_PASS,
		IDS_INI_AUTH_APOP,
		IDS_INI_AUTH_RPA,
		IDS_INI_SMTP_AUTH_REQUIRED,
		IDS_INI_SMTP_AUTH_ALLOWED,
		IDS_INI_SMTP_AUTH_BANISHED,
		IDS_INI_CHECK_FOR_MAIL,
		IDS_INI_DOMAIN_QUALIFIER,

		// IMAP
		IDS_INI_USES_IMAP,
		IDS_INI_IMAP_PREFIX,
		IDS_INI_IMAP_ACCOUNT_DIR,
		IDS_INI_IMAP_XFERTOTRASH,
		IDS_INI_IMAP_TRASH_MBOXNAME,
		
		// personalities
		IDS_INI_PERSONA_REAL_NAME,
		IDS_INI_PERSONA_RETURN_ADDR,
		IDS_INI_PERSONA_POP_SERVER,
		IDS_INI_PERSONA_POP_ACCOUNT,
		IDS_INI_PERSONA_LOGIN_NAME,
		IDS_INI_PERSONA_SMTP_SERVER,
		IDS_INI_PERSONA_IMAP_PREFIX,
		IDS_INI_PERSONA_DEFAULT_DOMAIN,
		IDS_INI_PERSONA_CHECK_MAIL,
		IDS_INI_PERSONA_LMOS,
		IDS_RI_PERSONA0,		//
		IDS_RI_PERSONA1,		//
		IDS_RI_PERSONA2,		//
		IDS_RI_PERSONA3,		//
		IDS_RI_PERSONA4,		// anything that starts with Persona followed by a digit
		IDS_RI_PERSONA5,		// (will pick up Persona10 and up too)
		IDS_RI_PERSONA6,		//
		IDS_RI_PERSONA7,		//
		IDS_RI_PERSONA8,		//
		IDS_RI_PERSONA9,		//

		// dialup networking
		IDS_INI_AUTO_CONNECTION_USERNAME,
		IDS_INI_AUTO_CONNECTION_PASSWORD,
		IDS_INI_AUTO_CONNECTION_DOMAIN,
		IDS_INI_AUTO_CONNECTION_SAVE_PASSWORD,
		IDS_INI_AUTO_CONNECTION_NAME,

		// mode/registration stuff
		IDS_INI_MODE,
		IDS_INI_REG_STATUS,
		IDS_INI_PROFILE,
		IDS_INI_DISTRIBUTOR,
		IDS_INI_REG_CODE_PRO,
		IDS_INI_REG_FIRST_NAME_PRO,
		IDS_INI_REG_LAST_NAME_PRO,
		IDS_INI_REG_CODE_AD,
		IDS_INI_REG_FIRST_NAME_AD,
		IDS_INI_REG_LAST_NAME_AD,
		IDS_INI_REG_FIRST_NAME_LIGHT,
		IDS_INI_REG_LAST_NAME_LIGHT,
		IDS_INI_REG_CODE_LIGHT,
		IDS_PRE43INI_REG_CODE,

		// features
		IDS_INI_USE_JUNK_MAILBOX,

		// potentially annoying dialogs
		IDS_INI_SEEN_INTRO,
		IDS_INI_ASKED_ABOUT_JUNK,
		IDS_INI_SHOW_TIP_OF_THE_DAY,
		IDS_INI_CURRENT_TIP_OF_THE_DAY
	};
	const nSettings = sizeof(settingsToKeep)/sizeof(settingsToKeep[0]);
	CString settings[nSettings];
	BOOL ok;
	int errStrID = 0;
	char tempIniName[_MAX_PATH + 1];

	// The FALSE arguments in the JJFile constructors below turn off
	// that class's error reporting, which depends on the message pump
	// (and it's already been shut off).

	// Existing Eudora.ini; gets renamed to EudIni.sav.
	JJFile iniFile(JJFile::BUF_SIZE, FALSE);

	// New Eudora.ini; starts out with a temp file name and gets renamed
	// to Eudora.ini.
	JJFile newIniFile(JJFile::BUF_SIZE, FALSE);
	BOOL openedNewIni = FALSE;

	// Existing EudIni.sav; if it does exist, gets renamed to one of
	// EudIni01.sav, ..., EudIni99.sav.
	JJFile saveIni(JJFile::BUF_SIZE, FALSE);

	CString sSaveIniName;    // EudIni.sav path
	CString sSaveIniNewName; // what we'll rename EudIni.sav to if we have to
	BOOL renameSavedIni = FALSE;

	ok = TRUE;

	// Open the existing Eudora.ini.
	if (ok)
	{
		if (FAILED(iniFile.Open(INIPath, O_RDWR)))
		{
			ok = FALSE;
			errStrID = IDS_RIE_NO_INI_FILE;
		}
	}

	// Create the new Eudora.ini in the same directory with a temporary name,
	// and open it.
	if (ok)
	{
		// This not only names the file but creates it.
		if (!GetTempFileName(EudoraDir, "", 0, tempIniName))
		{
			ok = FALSE;
			errStrID = IDS_RIE_FILE_CREATE_FAILED;
		}
		if (ok)
		{
			if (FAILED(newIniFile.Open(tempIniName, O_WRONLY)))
			{
				ok = FALSE;
				errStrID = IDS_RIE_FILE_CREATE_FAILED;
			}
			else
				openedNewIni = TRUE;
		}
	}

	// Put the name we'll use to save the existing Eudora.ini in sSaveIniName.
	// If there's an existing file by the same name, we'll rename it.
	if (ok)
	{
		sSaveIniName = EudoraDir + CRString(IDS_RI_SAVE_FULL_NAME);

		// If there's an existing file by the same name, determine a new name for it.
		if (!FAILED(saveIni.Open(sSaveIniName, O_WRONLY)))
		{
			CString sTemp(EudoraDir + CRString(IDS_RI_SAVE_NAME));

			// Check EudIni01.sav, EudIni02.sav, etc. until we find one that doesn't
			// already exist.
			BOOL done = FALSE;
			const CRString sExt(IDS_RI_SAVE_EXTENSION);
			for (int i = 1; (!done) && (i < 100); i++)
			{
				char tmp[10];

				wsprintf(tmp, "%02d.%s", i, (LPCTSTR) sExt);
				sSaveIniNewName = sTemp + tmp;
				if (!FileExistsMT(sSaveIniNewName))
					done = TRUE;
			}
			if (done)
				renameSavedIni = TRUE;
			else
			{	// EudIni01.sav through EudIni99.sav must already exist.
				ok = FALSE;
				errStrID = IDS_RIE_TOO_MANY_SAVED;
			}
		}
	}

	// Build the list of setting strings we'll use to filter the .ini file. For each setting, we
	// grab the string; truncate it before the newline, if there is one; and lower-case it (because
	// we need to do case-insensitive comparisons.
	if (ok)
	{
		for (int i = 0; i < nSettings; i++)
		{
			settings[i].LoadString(settingsToKeep[i]);
			int newLinePos = settings[i].Find('\n');
			if (newLinePos >= 0)
			{
				settings[i].GetBufferSetLength(newLinePos);
				settings[i].ReleaseBuffer();
			}
			settings[i].MakeLower();
		}
	}

	// Traverse the existing Eudora.ini and write out the parts we want to the
	// new file.
	if (ok)
	{
		long lNumBytesRead;
		CString sLine;
		CString sLcLine;
		const CRString sToolbar(IDS_RI_TOOLBAR_SECTION);
		const maxLineLen = 1024;

		do
		{
			char* pszLine = sLine.GetBuffer(maxLineLen);
			HRESULT hrGet = iniFile.GetLine(pszLine, maxLineLen, &lNumBytesRead);
			sLine.ReleaseBuffer(-1);
			if (SUCCEEDED(hrGet) && (lNumBytesRead > 0) && !sLine.IsEmpty())
			{
				sLine.TrimLeft();
				sLcLine = sLine;
				sLcLine.MakeLower();

				// We 'll keep all the section headers, except the [toolbar-blahblahblah]
				// ones.
				if ((sLcLine[0] == '[') && (sLcLine.Find(sToolbar) < 0))
					newIniFile.PutLine(sLine);
				else
				{
					BOOL found = FALSE;
					for (int i = 0; (!found) && (i < nSettings); i++)
					{
						if (sLcLine.Find(settings[i]) >= 0)
						{
							found = TRUE;
							newIniFile.PutLine(sLine);
						}
					}
				}
				sLine.Empty();
			}	
		} while (lNumBytesRead > 0);
	}

	// If there's already a EudIni.sav, rename it to whatever we decided on above.
	if (ok)
	{
		if (renameSavedIni)
		{
			if (FAILED(saveIni.Rename(sSaveIniNewName, FALSE)))
			{
				ok = FALSE;
				errStrID = IDS_RIE_SAVE_RENAME_FAILED;
			}
		}
	}

	// Rename the old Eudora.ini to EudIni.sav, and rename the new Eudora.ini
	// to Eudora.ini.
	BOOL renamedNewIni = FALSE;
	if (ok)
	{
		if (FAILED(iniFile.Rename(sSaveIniName, FALSE)))
		{
			ok = FALSE;
			errStrID = IDS_RIE_INI_RENAME_FAILED;
		}
	}
	if (ok)
	{
		if (FAILED(newIniFile.Rename(INIPath, FALSE)))
		{
			ok = FALSE;
			errStrID = IDS_RIE_TEMP_RENAME_FAILED;
		}
		else
			renamedNewIni = TRUE;
	}

	// Clean up. (JJFiles are closed automatically on destruction.)
	if (openedNewIni && !renamedNewIni)
		newIniFile.Delete();

	// Log what just happened
	if (ok)
		QCLogFileMT::WriteDebugLog(DEBUG_MASK_DIALOG,
		                           CRString(IDS_RI_LOG_SUCCESS), -1);
	else
		QCLogFileMT::WriteDebugLog(DEBUG_MASK_DIALOG,
		                           CRString(IDS_RI_LOG_FAILURE), -1);

	// If anything went wrong, inform the user.
	if (!ok)
	{
		if (errStrID)	// just in case we forget to set it somewhere
			AfxMessageBox(CRString(errStrID), MB_ICONEXCLAMATION | MB_OK);
	}

	// That's it!
	return;
}
