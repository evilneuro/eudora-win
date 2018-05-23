// QCSharewareManager.cpp: implementation of the QCSharewareManager class.
//
// Copyright (c) 1999-2001 by QUALCOMM, Incorporated
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

#include "fileutil.h"
#include "resource.h"
#include "guiutils.h"
#include "QCGraphics.h"

#include "QCSharewareManager.h"
#include "ProfileNag.h"
#include "mainfrm.h"
#include "regif.h"
#include "timestmp.h"

#include "RegInfoReader.h"

//	Need next two headers for NotifyPluginsOfMode
#include "eudora.h"
#include "trnslate.h"  
#include "PaymentAndRegistrationDlg.h"

#include "..\regcode\regcode_charsets.h"
#include "..\regcode\regcode_v2.h"

#include "DebugNewHelpers.h"

/*
// Uncomment this to run as a box build
#ifdef DEBUG
#ifdef DEFAULT_SWM_MODE
#undef DEFAULT_SWM_MODE
#endif
#define DEFAULT_SWM_MODE 2
#endif
*/

// *** IMPORTANT ***
// Remove or comment out the below line for international builds of Eudora.
#define SUPPORT_US_ENGLISH_ONLY_REGCODES

static int CalcRegLevel(int regPolicy, int regMonth);
static int GetRandomNumber();

// --------------------------------------------------------------------------

class CDowngradeDialog : public CDialog
{
public:
	CDowngradeDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CDowngradeDialog();

	virtual void			OnOK();
	virtual void			DoDataExchange(CDataExchange* pDX);

	BOOL					m_bUserConfirmsGoingLight;
	CString					m_lpszText;
};

// --------------------------------------------------------------------------

CDowngradeDialog::CDowngradeDialog(UINT nIDTemplate, CWnd* pParentWnd) :
	CDialog(nIDTemplate, pParentWnd),
	m_bUserConfirmsGoingLight(FALSE)
{
	m_lpszText.LoadString(IDS_DOWNGRADE_TEXT);
}

CDowngradeDialog::~CDowngradeDialog()
{
}

void CDowngradeDialog::OnOK()
{
	m_bUserConfirmsGoingLight = TRUE;

	// Dismiss the dialog
	CDialog::OnOK();
}

void CDowngradeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_MISSINGFEATURESTEXT, m_lpszText);
}

// --------------------------------------------------------------------------

BOOL UserConfirmsGoingLight(CWnd* pParentWnd = NULL)
{
	CDowngradeDialog dlg(IDD_DOWNGRADE_WINDOW, pParentWnd);

	dlg.DoModal();

	return dlg.m_bUserConfirmsGoingLight;
}

// --------------------------------------------------------------------------

class CPermanentSwitchWarningDialog : public CDialog
{
public:
							CPermanentSwitchWarningDialog(CWnd* pParentWnd = NULL);
	virtual					~CPermanentSwitchWarningDialog();

	virtual void			OnOK();

	bool					UserConfirmsPermanentSwitch() const { return m_bUserConfirmsPermanentSwitch; }

protected:
	bool					m_bUserConfirmsPermanentSwitch;
};

// --------------------------------------------------------------------------

CPermanentSwitchWarningDialog::CPermanentSwitchWarningDialog(CWnd* pParentWnd /*= NULL*/)
	:	CDialog( IDD_WARN_PERMANENT_SWITCH, pParentWnd ),
		m_bUserConfirmsPermanentSwitch(false)
{
}

CPermanentSwitchWarningDialog::~CPermanentSwitchWarningDialog()
{
}

void CPermanentSwitchWarningDialog::OnOK()
{
	m_bUserConfirmsPermanentSwitch = TRUE;

	// Dismiss the dialog
	CDialog::OnOK();
}

// --------------------------------------------------------------------------

bool UserConfirmsPermanentSwitch(CWnd* pParentWnd = NULL)
{
	CPermanentSwitchWarningDialog PermanentSwitchWarning(pParentWnd);
	
	PermanentSwitchWarning.DoModal();

	return PermanentSwitchWarning.UserConfirmsPermanentSwitch();
}

// --------------------------------------------------------------------------

class CRefundCodeDialog : public CDialog
{
public:
							CRefundCodeDialog(CWnd* pParentWnd = NULL);
	virtual					~CRefundCodeDialog();
	
	virtual void			DoDataExchange(CDataExchange* pDX);

protected:
	long					m_nRefundCode;
};

// --------------------------------------------------------------------------

CRefundCodeDialog::CRefundCodeDialog(CWnd* pParentWnd /*= NULL*/)
	:	CDialog( IDD_REFUND_CODE, pParentWnd ),
		m_nRefundCode(0)
{
	//	We need to make sure that srand is called before we call rand, otherwise
	//	all users will get the same refund code. Since we can't be sure that
	//	anyone else called srand yet, we do it here.
	//	We should probably only call srand one location in Eudora, rather than
	//	both here and CAuditNagTask.cpp (as of this writing). However, the
	//	refund code is almost never used (and even then it's once and done), so
	//	it doesn't really matter that we might be re-seeding the random generator.
	srand( time(NULL) );
	m_nRefundCode = (rand() << 16) | rand();
}

CRefundCodeDialog::~CRefundCodeDialog()
{
}

void CRefundCodeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	CString	szRefundCode;
	wsprintf(szRefundCode.GetBuffer(10), "%08X", m_nRefundCode);
	szRefundCode.ReleaseBuffer();

	DDX_Text(pDX, IDC_REFUND_CODE, szRefundCode);
}

// --------------------------------------------------------------------------

UINT msgSharewareNotification = ::RegisterWindowMessage("msgSharewareNotification");
/* static */ QCSharewareManager *QCSharewareManager::m_pTheSharewareManager = NULL;

CBitmap g_FullFeatureBitmap;


// --------------------------------------------------------------------------

// [PUBLIC] Initialize
/* static */ bool QCSharewareManager::Initialize()
{
	TRACE("QCSharewareManager::Initialize()\n");
	ASSERT(!m_pTheSharewareManager);

	if (m_pTheSharewareManager)
		delete m_pTheSharewareManager;

	m_pTheSharewareManager = DEBUG_NEW_NOTHROW QCSharewareManager;

	if (m_pTheSharewareManager)
		m_pTheSharewareManager->Load();

	return (m_pTheSharewareManager != NULL);
}

// --------------------------------------------------------------------------

// [PUBLIC] Destroy
/* static */ bool QCSharewareManager::Destroy()
{
	TRACE("QCSharewareManager::Destroy()\n");

	delete m_pTheSharewareManager;
	m_pTheSharewareManager = NULL;

	return (true);
}

// --------------------------------------------------------------------------

// [PROTECTED] QCSharewareManager (Default Constructor)
QCSharewareManager::QCSharewareManager()
	:	m_bWasBadProRegCodeAtStartup(false), m_bWasExpiredProRegCodeAtStartup(false),
		m_AdwareSwitchTime(0)
{
#ifdef DEATH_BUILD
	m_Mode = SWM_MODE_PRO;
#else
	// Assume Adware to start
	m_Mode = (SharewareModeType)DEFAULT_SWM_MODE;
#endif
	
	// Assume not deadbeat to start
	m_bDeadbeat = false;

	m_DemoBuildMonth = 0;
	m_DemoStartDate = 0;

	// Load up bitmap for Light mode diabled menu items
	if (g_FullFeatureBitmap.GetSafeHandle() == NULL)
	{
		int nCheckMenuHeight = ::GetSystemMetrics(SM_CYMENUCHECK);
		VERIFY( g_FullFeatureBitmap.LoadBitmap(IDB_FULL_FEATURE) );
		if (GetIniShort(IDS_INI_CENTER_UNREAD_STATUS))
			FitTransparentBitmapToHeight(g_FullFeatureBitmap, nCheckMenuHeight);
	}
}

// --------------------------------------------------------------------------

// [PROTECTED] ~QCSharewareManager (Destructor)
QCSharewareManager::~QCSharewareManager() // virtual
{
	g_FullFeatureBitmap.DeleteObject();
}

// --------------------------------------------------------------------------

// [PUBLIC] SetMode
//
// Sets the current mode.
// Returns whether or not the actual mode was changed.
//
bool QCSharewareManager::SetMode(SharewareModeType Mode, bool bAskUser /*= true*/, CWnd* pParentWnd /*= NULL*/,
									bool bUserSpecificallyChangedMode /*= false*/)
{
#ifdef DEATH_BUILD
	m_Mode = SWM_MODE_PRO;
	return false;
#endif

	if (m_Mode == Mode)
		return false;

#if DEFAULT_SWM_MODE == 2
	// Don't change modes in a Paid mode build if the user
	// didn't specifically ask to change to the new mode
	if (!bUserSpecificallyChangedMode)
		return false;
#endif

	if (Mode == SWM_MODE_LIGHT)
	{
		if (bAskUser)
		{
			if (!UserConfirmsGoingLight(pParentWnd))
			{
				// User decided not to go light after all.
				return false;
			}
		}
	}

	// if the user is a deadbeat(tm), we're gonna make 'em profile before
	// we allow them to switch modes. they can lie, but they'll get caught.
	if ( (Mode == SWM_MODE_ADWARE) && IsDeadbeat() && bAskUser && bUserSpecificallyChangedMode )
	{
		if ( DoDeadbeatNag(pParentWnd, true) )
		{
			// user claims to have profiled; change modes, and do a playlist
			// request to verify---let the server deal with fibbers.
			((CMainFrame*)AfxGetMainWnd())->UpdatePlaylists(true);
		}
		else
		{
			return false;
		}
	}

	if (Mode == SWM_MODE_DEADBEAT)
	{
		m_bDeadbeat = true;
		Mode = SWM_MODE_LIGHT;
	}
	else
		m_bDeadbeat = false;

	// How many junkers are available now, while we're still in the old mode?
	CTranslatorSortedList *pJunkList = g_pApp->m_TransManager->GetSortedTranslators(EMSF_JUNK_MAIL);
	int nJunk = pJunkList->GetCount();

	SharewareModeType OldMode = m_Mode;
	m_Mode = Mode;
	
	// If we were in the grace period for an expired paid mode reg code and we're
	// changing to one of the other modes, let's make the idle loop stop checking
	// whether the grace period has expired.
	if (Mode != SWM_MODE_PRO)
		ClearAdwareSwitchTime();

	Save();

	// Notify any registered wnds of the change in feature set
	NotifyClients(NULL, CA_SWM_CHANGE_FEATURE, (void *)&OldMode);

	// Notify any plugins of the change in feature set
	NotifyPluginsOfMode();

	// Notify any other interested parties about change in mode setting
	NotifyIniUpdates(IDS_INI_MODE);

	// And how many junkers now?
	pJunkList = g_pApp->m_TransManager->GetSortedTranslators(EMSF_JUNK_MAIL);

	if (nJunk > pJunkList->GetCount())
	{
		CJunkDownDlg dlg;
		dlg.DoModal();
		SetIniShort(IDS_INI_ASKED_ABOUT_JUNK,(short)(GetIniShort(IDS_INI_ASKED_ABOUT_JUNK)|(1<<3)));
	}
	else if (nJunk==0 && !pJunkList->IsEmpty() && !(GetIniShort(IDS_INI_ASKED_ABOUT_JUNK)&(1<<1)))
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
		SetIniShort(IDS_INI_ASKED_ABOUT_JUNK,(short)(GetIniShort(IDS_INI_ASKED_ABOUT_JUNK)|(1<<1)));
	}

	return true;
}

// --------------------------------------------------------------------------

// [PUBLIC] DoRefund
bool QCSharewareManager::DoRefund(CWnd* pParentWnd /*= NULL*/)
{
	bool bDoSwitch = UserConfirmsPermanentSwitch(pParentWnd);

	if (bDoSwitch)
	{
		CRefundCodeDialog	RefundCodeDialog(pParentWnd);

		//	Stash the paid reg code before blasting it.
		CString				szCurrentRegCode = QCSharewareManager::GetRegCodeForMode(SWM_MODE_PRO);
		
		//	Change mode to adware
		SetMode(SWM_MODE_ADWARE, true, NULL, true);

		CString				szFirstNameFromRegistry, szLastNameFromRegistry, szRegCodeFromRegistry;
		bool				bRegCodeFromRegistryGood = false;
		HKEY				hKey;

		//	Check the registry for a paid mode reg code and remove it if it matches
		//	the reg code for which we are giving a refund
		if ( ::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Qualcomm\\Eudora\\Check"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
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

			if ( bRegCodeFromRegistryGood &&
				 AreRegCodesEqual(szCurrentRegCode, szRegCodeFromRegistry) &&
				 (szFirstNameFromRegistry.CompareNoCase(GetFirstNameForMode(SWM_MODE_PRO)) == 0) &&
				 (szLastNameFromRegistry.CompareNoCase(GetLastNameForMode(SWM_MODE_PRO)) == 0) )
			{
				//	The reg code in the registry is the one for which we are giving a refund,
				//	so remove it from the registry.
				::RegDeleteValue( hKey, _T("FName") );
				::RegDeleteValue( hKey, _T("LName") );
				::RegDeleteValue( hKey, _T("RCode") );
			}

			//	Any removals will be flushed after we close at the next lazy flush time
			::RegCloseKey(hKey);
		}

		//	Set the pro reg code ini setting info to nothing in memory
		SetIniString(IDS_INI_REG_FIRST_NAME_PRO, "");
		SetIniString(IDS_INI_REG_LAST_NAME_PRO, "");
		SetIniString(IDS_INI_REG_CODE_PRO, "");

		//	Make sure pro registration empty strings are saved in ini right now -
		//	we need to flush before purging.
		FlushINIFile();

		//	Purge the pro reg code ini settings from the ini file
		PurgeIniSetting(IDS_INI_REG_FIRST_NAME_PRO);
		PurgeIniSetting(IDS_INI_REG_LAST_NAME_PRO);
		PurgeIniSetting(IDS_INI_REG_CODE_PRO);

		//	Delete RegCode.dat in one or both of its two possible locations, if either
		//	or both of them contain the regcode for which we are giving a refund.
		//	The two locations are the EudoraDir (where the INI file, etc. are
		//	located) and the ExecutableDir. The below loop will execute 1-2 times.
		bool				bGoodRegInfo, bIgnoreEudoraNeedsPaidRegistration;
		CString				szFirstName, szLastName, szFileRegCode, szDistributorID;
		SharewareModeType	regMode;
		CString				szRegInfoStartupFileName;
		GetRegInfoFilePath(szRegInfoStartupFileName);

		for (short i = 0; i < 2; i++)
		{
			if ( ::FileExistsMT(szRegInfoStartupFileName) )
			{
				bGoodRegInfo = RegInfoReader::GetInfoFromRegCodeStartupFile(
									szRegInfoStartupFileName, szFirstName, szLastName, szFileRegCode,
									szDistributorID, regMode, bIgnoreEudoraNeedsPaidRegistration );
				
				//	Does this file contain the regcode that we're blasting?
				if ( bGoodRegInfo && (regMode == SWM_MODE_PRO) &&
					 AreRegCodesEqual(szCurrentRegCode, szFileRegCode) )
				{
					DeleteFile(szRegInfoStartupFileName);

					//	If there was a distributor ID in the file, attempt to preserve it by
					//	creating a file with just the distributor ID in it.
					if ( !szDistributorID.IsEmpty() )
					{
						JJFile		regInfoDistributorIDFile;

						if ( SUCCEEDED( regInfoDistributorIDFile.Open(szRegInfoStartupFileName, O_CREAT | O_RDWR) ) )
						{
							regInfoDistributorIDFile.Put("Eudora-Distributor-ID: ");
							regInfoDistributorIDFile.PutLine(szDistributorID);
						}

						//	regInfoDistributorIDFile is closed in its destructor as it falls out of scope
					}
				}
			}

			//	If the alternate path is not different, then don't bother with a second pass.
			if ( (i == 0) && !GetRegInfoFileAlternatePath(szRegInfoStartupFileName) )
				break;
		}

		//	Display tech support random number dialog
		RefundCodeDialog.DoModal();
	}

	return bDoSwitch;
}

// --------------------------------------------------------------------------

// [PUBLIC] GetRegCodeForMode
// Returns what's in the .ini file without checking its validity.
/* static */ const char* QCSharewareManager::GetRegCodeForMode(SharewareModeType inMode)
{
	const char*	szRegCode = NULL;
	
	// Get the registration code for the indicated mode
	switch (inMode)
	{
		case SWM_MODE_DEADBEAT:
		case SWM_MODE_LIGHT:
			szRegCode = GetIniString(IDS_INI_REG_CODE_LIGHT);
			break;

		case SWM_MODE_ADWARE:
			szRegCode = GetIniString(IDS_INI_REG_CODE_AD);
			break;

		case SWM_MODE_PRO:
			szRegCode = GetIniString(IDS_INI_REG_CODE_PRO);
			break;

		default:
			// Invalid user mode!
			ASSERT(0);
			break;
	}

	return szRegCode;
}


// [PUBLIC] GetRegCodePlusMonthForMode
// Returns what's in the .ini file, and annotates it with the monthcode
/* static */ const CString QCSharewareManager::GetRegCodePlusMonthForMode(SharewareModeType inMode)
{
	const char *szRegLevel = GetRegLevelForMode(inMode);
	if (szRegLevel)
	{
		int monthCode = (atoi(szRegLevel)/19)&0xff;
		    
		if (monthCode > 0)
		{
			CString temp;

			temp.Format("%s (%d/%d)",GetRegCodeForMode(inMode),monthCode%12+1,monthCode/12+1999);

			return temp;
		}
	}
	return CString(GetRegCodeForMode(inMode));
}

// --------------------------------------------------------------------------

// [PUBLIC] GetFirstNameForMode
// Returns what's in the .ini file; doesn't validate the reg code against it, or
// check whether a reg code exists.
/* static */ const char* QCSharewareManager::GetFirstNameForMode(SharewareModeType inMode)
{
	const char*	szFirstName = NULL;
	
	// Get the registration code for the indicated mode
	switch (inMode)
	{
		case SWM_MODE_DEADBEAT:
		case SWM_MODE_LIGHT:
			szFirstName = GetIniString(IDS_INI_REG_FIRST_NAME_LIGHT);
			break;

		case SWM_MODE_ADWARE:
			szFirstName = GetIniString(IDS_INI_REG_FIRST_NAME_AD);
			break;

		case SWM_MODE_PRO:
			szFirstName = GetIniString(IDS_INI_REG_FIRST_NAME_PRO);
			break;

		default:
			// Invalid user mode!
			ASSERT(0);
			break;
	}

	return szFirstName;
}

// --------------------------------------------------------------------------

// [PUBLIC] GetLastNameForMode
// Returns what's in the .ini file; doesn't validate the reg code against it, or
// check whether a reg code exists.
/* static */ const char* QCSharewareManager::GetLastNameForMode(SharewareModeType inMode)
{
	const char*	szLastName = NULL;
	
	// Get the registration code for the indicated mode
	switch (inMode)
	{
		case SWM_MODE_DEADBEAT:
		case SWM_MODE_LIGHT:
			szLastName = GetIniString(IDS_INI_REG_LAST_NAME_LIGHT);
			break;

		case SWM_MODE_ADWARE:
			szLastName = GetIniString(IDS_INI_REG_LAST_NAME_AD);
			break;

		case SWM_MODE_PRO:
			szLastName = GetIniString(IDS_INI_REG_LAST_NAME_PRO);
			break;

		default:
			// Invalid user mode!
			ASSERT(0);
			break;
	}

	return szLastName;
}


// --------------------------------------------------------------------------

// [PUBLIC] GetFullNameForMode
// Returns what's implied by the .ini file; doesn't validate the reg code against it, or
// check whether a reg code exists.
/* static */ const char* QCSharewareManager::GetFullNameForMode(SharewareModeType inMode)
{
	static char		szFullNameArray[256];

	const char*		szFullName = NULL;
	const char*		FirstName = QCSharewareManager::GetFirstNameForCurrentMode();
	const char*		LastName = QCSharewareManager::GetLastNameForCurrentMode();

	if (strlen(FirstName) + strlen(LastName) + 1 < 256)
	{
		strcpy(szFullNameArray, FirstName);
		if (LastName && *LastName)
		{
			if (FirstName && strlen(FirstName) > 0)
			{
				strcat(szFullNameArray, " ");
			}
			strcat(szFullNameArray, LastName);
		}
		szFullName = szFullNameArray;
	}

	return szFullName;
}

// --------------------------------------------------------------------------

// [PUBLIC]
// Validates the registration info and returns NULL if it doesn't pass.
// Doesn't object to an expired paid mode reg code.
/* static */ const char* QCSharewareManager::GetRegLevelForMode(SharewareModeType inMode)
{
	static char		szRegLevelArray[32];
	
	const char*		FirstName = QCSharewareManager::GetFirstNameForMode(inMode);
	const char*		LastName = QCSharewareManager::GetLastNameForMode(inMode);
	const char*		RegCode = QCSharewareManager::GetRegCodeForMode(inMode);
	
	int				regMonth;
	SharewareModeType	regMode;
	int regPolicy;
	bool expired;

	if (IsValidRegistrationInfo(FirstName, LastName, RegCode, &regMode, &regMonth, &regPolicy, &expired)
		|| (expired && (regMode == SWM_MODE_PRO)))
	{
		int regLevel = CalcRegLevel(regPolicy, regMonth);
		sprintf(szRegLevelArray,"%d",regLevel);
		return szRegLevelArray;
	}
	else
#if DEFAULT_SWM_MODE ==  2	// i.e. if Paid mode is the default
	{
		// we want to fake a reglevel with the magic policy REG_EUD_50_TEMP_BOX_ESD
		CTime time(time(NULL));
		int minute = time.GetMinute();
		sprintf(szRegLevelArray,"%d",CalcRegLevel(REG_EUD_50_TEMP_BOX_ESD,minute));
		return szRegLevelArray;
	}
#else
		return NULL;
#endif
}


// --------------------------------------------------------------------------

// [PUBLIC] AreRegCodesEqual
/* static */ bool QCSharewareManager::AreRegCodesEqual(const char * szRegCodeLHS, const char * szRegCodeRHS)
{
	char           szPureCodeLHS[REG_CODE_LEN], szPureCodeRHS[REG_CODE_LEN];

	regcode_v2_canonicalize(szRegCodeLHS, szPureCodeLHS);
	regcode_v2_canonicalize(szRegCodeRHS, szPureCodeRHS);

	return (strncmp(szPureCodeLHS, szPureCodeRHS, REG_CODE_LEN) == 0);
}


// --------------------------------------------------------------------------

// [PUBLIC] IsRegisteredForMode
// If bReally is false, this will return true for paid mode in the box/ESD build
// whether or not it's really registered.
// If expiredPaidModeOK is true, this will return true in paid mode if there's
// a valid but expired paid mode reg code.
/* static */ bool QCSharewareManager::IsRegisteredForMode(SharewareModeType inMode, int * pMonth /*= NULL*/,
                                                          bool bReally /* = FALSE */,
                                                          bool expiredPaidModeOK /* = FALSE */,
														  int * pPolicy /* = NULL */)
{

#if DEFAULT_SWM_MODE ==  2	// i.e. if Paid mode is the default
    if ((!bReally) && (inMode == SWM_MODE_PRO))
	{
		// We're not going to fill in the month, so the caller better not
		// have wanted one!
		ASSERT(pMonth == NULL);

        return TRUE;
	}
#endif

	// Get the registration code for the mode that the user is in
	LPCTSTR szRegCode = QCSharewareManager::GetRegCodeForMode(inMode);

	bool isRegistered = (szRegCode && *szRegCode);
	
	if (isRegistered)
	{
		LPCTSTR szFirstName = QCSharewareManager::GetFirstNameForMode(inMode);
		LPCTSTR szLastName =	QCSharewareManager::GetLastNameForMode(inMode);
		SharewareModeType	regMode;
		bool expired;

		isRegistered = IsValidRegistrationInfo(szFirstName, szLastName, szRegCode, &regMode, pMonth,
		                                       pPolicy, &expired);

		// If expiredPaidModeOK is true, then the caller wants a yes if there's
		// a valid paid mode reg code, even if it's expired. IsValidRegistrationInfo()
		// returned false in that case, so we don't test isRegistered.
		if (expiredPaidModeOK && expired && (inMode == SWM_MODE_PRO) && (regMode == SWM_MODE_PRO))
			isRegistered = true;

		// Make sure that the reg mode matches the mode indicated by the reg number
		// (protects against user tampering to avoid registration nags)
		if (isRegistered)
			isRegistered = (regMode == inMode);
	}

	return isRegistered;
}


// --------------------------------------------------------------------------

// [PUBLIC] NeedsRegistrationNag
// This implements the policy that we never nag the user to register if (a) there's a valid
// reg code for any mode, even if it's expired or (b) the NeverRegister=1 switch is present
// in the .ini file.
/* static */ bool QCSharewareManager::NeedsRegistrationNag()
{
	bool needsRegistrationNag = false;
	
	// Don't nag if the user is never supposed to register
	if (!GetIniShort(IDS_INI_NEVER_REGISTER))
	{
		// Nag if it's the box build and they have a paid mode reg code with a month of 255
		int month;
		if (IsBoxBuild() && IsRegisteredForMode(SWM_MODE_PRO, &month, TRUE, FALSE) && (month == 255))
		{
			needsRegistrationNag = true;
		}
		// Otherwise, don't nag if the user already has a reg code, even it's an expired paid mode code
		else if ((!IsRegisteredForMode(SWM_MODE_LIGHT, NULL, true)) && (!IsRegisteredForMode(SWM_MODE_ADWARE, NULL, true))
			&& (!PaidModeRegCodeExists()))
		{
			// Otherwise, do nag
			needsRegistrationNag = true;
		}
	}
	return needsRegistrationNag;
}


// --------------------------------------------------------------------------

// [PUBLIC] GetRegStatusFlag
/* static */ bool QCSharewareManager::GetRegStatusFlag(long nFlag)
{
	return ( (GetIniLong(IDS_INI_REG_STATUS) & nFlag) != 0 );
}


// --------------------------------------------------------------------------

// [PUBLIC] SetRegStatusFlag
/* static */ void QCSharewareManager::SetRegStatusFlag(long nFlag, bool bValue)
{
	long	nRegStatus = GetIniLong(IDS_INI_REG_STATUS);

	if (bValue)
		nRegStatus |= nFlag;
	else
		nRegStatus &= ~nFlag;
	
	SetIniLong(IDS_INI_REG_STATUS, nRegStatus);

	// Make sure reg status is saved in ini right now.
	FlushINIFile();
}


// --------------------------------------------------------------------------

// [PUBLIC] SetNeedsPaidRegistrationNag
/* static */ void QCSharewareManager::SetNeedsPaidRegistrationNag(bool bNeedsPaidRegistrationNag)
{
	// If bNeedsPaidRegistrationNag then the "Eudora-Needs-Registration: yes" was already
	// read correctly. Set CHECKED_FOR_MISSING_CRLF to true at the same time.
	if (bNeedsPaidRegistrationNag)
		SetRegStatusFlag( (NEEDS_PAID_REG_NAG | CHECKED_FOR_MISSING_CRLF), bNeedsPaidRegistrationNag );
	else
		SetRegStatusFlag(NEEDS_PAID_REG_NAG, bNeedsPaidRegistrationNag);
}

// --------------------------------------------------------------------------

// [PUBLIC] GetNeedsPaidRegistrationNag
// Returns the same result as NeedsRegistrationNag().
/* static */ bool QCSharewareManager::GetNeedsPaidRegistrationNag()
{
	return NeedsRegistrationNag();
}

// --------------------------------------------------------------------------

// Returns false on an expired paid mode reg code, but in that case
// *pExpiredButOtherwiseValid is set true.
// [PUBLIC] IsValidRegistrationInfo
/* static */ bool QCSharewareManager::IsValidRegistrationInfo(
	const char *		firstName,
	const char *		lastName,
	const char *		regCode,
	SharewareModeType*	pMode /*= NULL*/,
	int *				pMonth /*= NULL*/,
	int *				pPolicy /* = NULL */,
	bool *				pExpiredButOtherwiseValid /* NULL */ )
{
	bool	validRegistrationCode = false;
	int		regCodeMonth = 0;
	bool	paidExpired = false;

	// Initialize for those who don't check return values
	if (pMode)
		*pMode = (SharewareModeType) DEFAULT_SWM_MODE;
	
	if ( (firstName != NULL) && (lastName != NULL) && (regCode != NULL) )
	{
		int		productCode = 0;
		char	fullName[256];
		char	mappedName[256];
		SharewareModeType mode = SWM_MODE_ADWARE;	// useless init to stifle compiler warning
		
		strncpy( fullName, firstName, sizeof(fullName) );
		
		int		nFirstNameLength = strlen(firstName);
		if ( nFirstNameLength < (sizeof(fullName) - 1) )
			strncat( fullName, lastName, (sizeof(fullName) - nFirstNameLength - 1) );
		
		fullName[sizeof(fullName)-1] = '\0';
		
		validRegistrationCode =
			( regcode_map_win_cp1252(fullName, mappedName) > 0 ) &&
			( regcode_v2_verify(regCode, mappedName, &regCodeMonth, &productCode) == 0 );

		// The expiration and product code policy lives right here.
		if (validRegistrationCode)
		{
			if (productCode == REG_EUD_AD_WARE)
			{
				mode = SWM_MODE_ADWARE;
			}
			else if (productCode == REG_EUD_LIGHT)
			{
				mode = SWM_MODE_LIGHT;
			}
			else if (productCode == REG_EUD_PAID)
			{
				mode = SWM_MODE_PRO;
				if ( (regCodeMonth >= REG_EUD_CLIENT_34_DEFUNCT_MONTH) || ((regCodeMonth + 12) < EUDORA_BUILD_MONTH) )
				{
					validRegistrationCode = false;
					paidExpired = true;
				}
			}
			else if ((productCode >= REG_EUD_50_PAID_START) && (productCode <= REG_EUD_50_PAID_STOP))
			{
				mode = SWM_MODE_PRO;
				if ((regCodeMonth + 12) < EUDORA_BUILD_MONTH)
				{
					validRegistrationCode = false;
					paidExpired = true;
				}
			}
#ifdef SUPPORT_US_ENGLISH_ONLY_REGCODES
			else if ( (productCode == REG_EUD_50_PAID_EN_ONLY) || (productCode == REG_EUD_50_PAID_EN_NOT_X1) )
			{
				mode = SWM_MODE_PRO;
				if ((regCodeMonth + 12) < EUDORA_BUILD_MONTH)
				{
					validRegistrationCode = false;
					paidExpired = true;
				}
			}
#endif
			else
			{
				// What's the user trying to pull here?!?!?
				ASSERT(0);
				validRegistrationCode = false;
				mode = SWM_MODE_ADWARE;
			}
		}

		if (pExpiredButOtherwiseValid)
			*pExpiredButOtherwiseValid = paidExpired; // regardless of validRegistrationCode

		if (validRegistrationCode || paidExpired)
		{
			if (pMode)
				*pMode = mode;
			if (pPolicy)
				*pPolicy = productCode;
		}
	}

	if (pMonth)
		*pMonth = regCodeMonth;

	return validRegistrationCode;
}


// --------------------------------------------------------------------------

// [PUBLIC] WasBadProRegCodeAtStartup
/* static */ bool QCSharewareManager::WasBadProRegCodeAtStartup()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM ? pSWM->InternalWasBadProRegCodeAtStartup() : false);
}

// --------------------------------------------------------------------------

// [PUBLIC] WasExpiredProRegCodeAtStartup
/* static */ bool QCSharewareManager::WasExpiredProRegCodeAtStartup()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM ? pSWM->InternalWasExpiredProRegCodeAtStartup() : false);
}

// --------------------------------------------------------------------------

// [PUBLIC] SetRegistrationInfo
// Stores the passed registration data in the .ini file, and only checks
// the mode for validity. Flushes the .ini file and makes sure the rest
// of the app is aware of the change.
/* static */ void QCSharewareManager::SetRegistrationInfo(
	const char *		firstName,
	const char *		lastName,
	const char *		regCode,
	SharewareModeType	regMode)
{
	//	Store the registration information
	switch (regMode)
	{
	case SWM_MODE_LIGHT:
		SetIniString(IDS_INI_REG_FIRST_NAME_LIGHT, firstName);
		SetIniString(IDS_INI_REG_LAST_NAME_LIGHT, lastName);
		SetIniString(IDS_INI_REG_CODE_LIGHT, regCode);
		break;

	case SWM_MODE_ADWARE:
		SetIniString(IDS_INI_REG_FIRST_NAME_AD, firstName);
		SetIniString(IDS_INI_REG_LAST_NAME_AD, lastName);
		SetIniString(IDS_INI_REG_CODE_AD, regCode);
		break;

	case SWM_MODE_PRO:
		SetIniString(IDS_INI_REG_FIRST_NAME_PRO, firstName);
		SetIniString(IDS_INI_REG_LAST_NAME_PRO, lastName);
		SetIniString(IDS_INI_REG_CODE_PRO, regCode);
		break;

	default:
		// Invalid registration mode!
		ASSERT(0);
		break;
	}

	// Make sure registration is saved in ini right now.
	FlushINIFile();

	GetSharewareManager()->NotifyClients(NULL, CA_SWM_CHANGE_REGCODE, (void *)&regMode);
	GetSharewareManager()->SetMode(regMode);
}

// --------------------------------------------------------------------------

// [PUBLIC] GetPaidModeRegInfo
// In the box/ESD build, if there's no paid mode reg code, or if it's expired, this constructs
// the agreed-upon fake registration info and sets *pIsFakeRegInfo to true.
/* static */ bool QCSharewareManager::GetPaidModeRegInfo(CString& regCode, CString& regFirst,
														 CString& regLast, CString& regLevel,
														 bool *pIsFakeRegInfo /* = NULL */ )
{
	bool result = false;
	if (pIsFakeRegInfo)
		*pIsFakeRegInfo = false;

#if DEFAULT_SWM_MODE == 2
	bool expired;
	bool noPaidRegCode = !PaidModeRegCodeExists(&expired);
	
	if (noPaidRegCode || expired)
	{
		// We're in the build that defaults to Paid mode, and we're either
		// registering for the first time or the existing reg code has expired.
		// The registration request that goes to the server needs to pass current
		// reg info, even though there is none, so we've elected to use
		// "IHate TheBox" as the reg name, 41 as the policy code, and the current
		// minute as the reg month.
		char rawRegCode[REG_CODE_LEN + 10];		// these sizes all include a little slop
		char canonicalizedRegCode[REG_CODE_LEN + 10];
		char firstName[10];
		char lastName[10];
		char name[20];
		firstName[0]  = 'I';
		firstName[1]  = 'h';
		firstName[2]  = 'a';
		firstName[3]  = 't';
		firstName[4]  = 'e';
		firstName[5]  =  0;
		lastName[0]   = 'T';
		lastName[1]   = 'h';
		lastName[2]   = 'e';
		lastName[3]   = 'b';
		lastName[4]   = 'o';
		lastName[5]   = 'x';
		lastName[6]   =  0;
		int month;

		// Make a single lowercase name for the regcode encoder.
		strcpy(name, firstName);
		strcat(name, " ");
		strcat(name, lastName);
		_strlwr(name);

		// Get the current time; we'll pass the current minute as the month
		// to help the server validate the registration request.
		CTime time(time(NULL));
		month = time.GetMinute();

		// Get the reg code and return the info.
		if (result = !regcode_v2_encode(name, month, REG_EUD_50_TEMP_BOX_ESD, GetRandomNumber(), rawRegCode))
		{
			result = true;
			if (pIsFakeRegInfo)
				*pIsFakeRegInfo = true;
			regcode_v2_canonicalize(rawRegCode, canonicalizedRegCode);
			regCode = canonicalizedRegCode;
			regFirst = firstName;
			regLast = lastName;
			regLevel.Format("%d", CalcRegLevel(REG_EUD_50_TEMP_BOX_ESD, month));
		}
	}
	else
#endif
	{
		const char *pCode = GetRegCodeForMode(SWM_MODE_PRO);
		const char *pFirst = GetFirstNameForMode(SWM_MODE_PRO);
		const char *pLast = GetLastNameForMode(SWM_MODE_PRO);
		const char *pLevel = GetRegLevelForMode(SWM_MODE_PRO);
		if (pCode && pFirst && pLast && pLevel)
		{
			result = true;
			regCode = pCode;
			regFirst = pFirst;
			regLast = pLast;
			regLevel = pLevel;
		}
	}

	return result;
}

ModeTypeEnum QCSharewareManager::GetCurrentPaidMode () const {
	if (QCSharewareManager::IsPaidModeOK()) return EMS_ModePaid;
	switch ( m_Mode ) {
		case SWM_MODE_ADWARE:	return EMS_ModeSponsored;
		case SWM_MODE_LIGHT:	return EMS_ModeFree;
		case SWM_MODE_PRO:		return EMS_ModePaid;
		default:				ASSERT ( false );
	}
	return EMS_ModeFree;
}


bool QCSharewareManager::UsingPaidFeatureSet() const
{
	return (GetCurrentPaidMode() == EMS_ModePaid);
}


bool QCSharewareManager::UsingPaidForX1FeatureSet() const
{
	int		nPolicy;
	bool	bIsPaidModeOK = IsRegisteredForMode(SWM_MODE_PRO, NULL, FALSE, FALSE, &nPolicy);
	
	return bIsPaidModeOK && (nPolicy != REG_EUD_50_PAID_EN_NOT_X1);
}


int QCSharewareManager::DemoDaysLeft() const
{
	const unsigned int Now = time(NULL);

	if (m_DemoBuildMonth < EUDORA_BUILD_MONTH ||
		(m_DemoBuildMonth == EUDORA_BUILD_MONTH && (Now < m_DemoStartDate + PAID_MODE_DEMO_DAYS * 60 * 60 * 24) && Now >= m_DemoStartDate))
	{
		unsigned int DaysLeft = PAID_MODE_DEMO_DAYS - ((Now - m_DemoStartDate) / (60 * 60 * 24));
		DaysLeft = min(max(DaysLeft, 1), PAID_MODE_DEMO_DAYS);
		return DaysLeft;
	}

	return 0;
}

void QCSharewareManager::NotifyPluginsOfMode() const
{
	CTranslatorManager *	transManager = g_pApp->GetTranslators();

	if (transManager)
		transManager->NotifyEveryoneOfModeChange( GetCurrentPaidMode() );
}

// This might look like a candidate for inlining in the header. I did it this
// way so I can test the box build behavior by recompiling just this module. JR.
bool QCSharewareManager::IsBoxBuild()
{
#if DEFAULT_SWM_MODE == 2
	return true;
#else
	return false;
#endif
}

// This returns true if there's a valid paid mode reg code, even if it's
// expired. If it is expired, *expired is set to true.
bool QCSharewareManager::PaidModeRegCodeExists(bool *expired /* = NULL */ )
{
	SharewareModeType mode;
	int month;
	int policy;
	bool expiredButOtherwiseValid;
	bool result;

	if (expired != NULL)
		*expired = false;
	const char *regCode = GetRegCodeForMode(SWM_MODE_PRO);
	const char *firstName = GetFirstNameForMode(SWM_MODE_PRO);
	const char *lastName = GetLastNameForMode(SWM_MODE_PRO);
	result = IsValidRegistrationInfo(firstName, lastName, regCode, &mode, &month, &policy,
	                                 &expiredButOtherwiseValid);
	if (expiredButOtherwiseValid)
	{
		result = true;
		if (expired != NULL)
			*expired = true;
	}
	return result;
}

bool QCSharewareManager::GetAdwareSwitchTime(time_t *theTime /*= NULL */ )
{
	bool result = false;

	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM != NULL)
	{
		if ((long) pSWM->m_AdwareSwitchTime != 0)
		{
			*theTime = pSWM->m_AdwareSwitchTime;
			result = true;
		}
	}
	return result;
}

bool QCSharewareManager::SetAdwareSwitchTime()
{
	bool result = false;

	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM != NULL)
	{
		long lTime = ((long) time(NULL)) + PAID_MODE_GRACE_SECS;
		if (SetIniLong(IDS_INI_ADWARE_SWITCH_TIME, lTime))
		{
			pSWM->m_AdwareSwitchTime = (time_t) lTime;
			result = true;
		}
	}
	return result;
}

bool QCSharewareManager::ClearAdwareSwitchTime()
{
	bool result = true;

	QCSharewareManager* pSWM = GetSharewareManager();
	if (pSWM == NULL)
		result = false;
	else
	{
		pSWM->m_AdwareSwitchTime = (time_t) 0;
		if (GetIniLong(IDS_INI_ADWARE_SWITCH_TIME) != 0)
		{
			if (!SetIniLong(IDS_INI_ADWARE_SWITCH_TIME, 0))
				result = false;
		}
	}
	return result;
}

// --------------------------------------------------------------------------

// [PROTECTED] Load
//
// Grab state info from INI
//
bool QCSharewareManager::Load()
{
#ifdef DEATH_BUILD
	m_Mode = SWM_MODE_PRO;
	return true;
#endif

	// See if we've got a switch from paid mode to adware pending.
	const char* rs = GetIniString(IDS_INI_ADWARE_SWITCH_TIME);
	if (rs != NULL)
	{
		long lTime = atol(rs);
		if (lTime != 0)
		{
			m_AdwareSwitchTime = (time_t) lTime;
		}
	}
	
	int val = GetIniInt(IDS_INI_MODE);
	CString RetailVersion(GetIniString(IDS_INI_RETAIL_VERSION));
	int LastDot = RetailVersion.ReverseFind('.');
	if (LastDot >= 0)
		RetailVersion.ReleaseBuffer(LastDot + 1);

#if DEFAULT_SWM_MODE == 2
	// If this is the first time running this particular paid mode build, then force to paid mode
	if (RetailVersion.IsEmpty() ||
		_tcsnicmp(RetailVersion, EUDORA_BUILD_VERSION, RetailVersion.GetLength()))
	{
		val = SWM_MODE_PRO;
		SetIniString(IDS_INI_RETAIL_VERSION, EUDORA_BUILD_VERSION);
	}
#else
	// If this is not a paid mode build, then get rid of the RetailVersion so that next
	// time we start up a paid mode build it will start in paid mode
	if (RetailVersion.IsEmpty() == FALSE)
		PurgeIniSetting(IDS_INI_RETAIL_VERSION);
#endif

	if ((val == SWM_MODE_ADWARE) || (val == SWM_MODE_LIGHT))
	{
		m_Mode = (SharewareModeType) val;
		ClearAdwareSwitchTime();
	}
	else if (val == SWM_MODE_DEADBEAT)
	{
		// same as Light at runtime
		m_Mode = SWM_MODE_LIGHT;
		m_bDeadbeat = true;
		ClearAdwareSwitchTime();
	}
	else if (val == SWM_MODE_PRO)
	{

#if DEFAULT_SWM_MODE == 2
		// We're in the Paid mode build, so take Eudora.ini at its word on this.
		m_Mode = SWM_MODE_PRO;
		ClearAdwareSwitchTime();
#else
		const char* szFirstName = QCSharewareManager::GetFirstNameForCurrentMode();
		const char* szLastName = QCSharewareManager::GetLastNameForCurrentMode();
		const char* szRegCode = QCSharewareManager::GetRegCodeForCurrentMode();
		SharewareModeType mode;
		int month;
		int policy;
		bool validCodeExpired = false;

		if (IsValidRegistrationInfo(szFirstName, szLastName, szRegCode, &mode, &month,
		                            &policy, &validCodeExpired) && (mode == SWM_MODE_PRO))
		{
			m_Mode = SWM_MODE_PRO;
			ClearAdwareSwitchTime();
		}
		else
		{
			// We need to give the user the expired demo nag,
			// but that can't be done until we've fully started up
			m_bWasBadProRegCodeAtStartup = true;

			// Switch 'em to adware here and now.
			ClearAdwareSwitchTime();

			m_Mode = SWM_MODE_ADWARE;
		}

#endif	// DEFAULT_SWM_MODE == 2

	}
	else
	{
		m_Mode = (SharewareModeType)DEFAULT_SWM_MODE;
	}

	Save();
	return (true);
}

// --------------------------------------------------------------------------

// [PROTECTED] Save
//
// Save state info to INI
//
bool QCSharewareManager::Save()
{
#ifndef DEATH_BUILD
	// deadbeat is Light mode at runtime
	SetIniInt(IDS_INI_MODE, m_bDeadbeat?SWM_MODE_DEADBEAT:m_Mode);

	// Make sure change gets written to disk
	FlushINIFile();
#endif

	return (true);
}


void OnUpdateFullFeatureSet(CMenu* pMenu, UINT nItem, BOOL bItemIsID)
{
	if (!pMenu)
		ASSERT(0);
	else
	{
		const bool bIsFull = UsingFullFeatureSet();

		UINT nFlags = bItemIsID? MF_BYCOMMAND : MF_BYPOSITION;

		pMenu->SetMenuItemBitmaps(nItem, nFlags, (bIsFull? NULL : &g_FullFeatureBitmap), NULL);
		pMenu->EnableMenuItem(nItem, nFlags | (bIsFull? MF_ENABLED : MF_DISABLED | MF_GRAYED));
	}
}

void OnUpdateFullFeatureSet(CCmdUI* pCmdUI)
{
	if (!pCmdUI)
		ASSERT(0);
	else
	{
		if (pCmdUI->m_pMenu && pCmdUI->m_pSubMenu == NULL)
		{
			ASSERT(pCmdUI->m_nIndex < pCmdUI->m_nIndexMax);
			OnUpdateFullFeatureSet(pCmdUI->m_pMenu, pCmdUI->m_nIndex, FALSE);
		}
		pCmdUI->m_bEnableChanged = TRUE;
	}
}

void FeatureNotInFree()
{
	ASSERT(UsingFullFeatureSet() == false);

	ErrorDialog(IDS_ERR_FEATURE_NOT_IN_FREE);
}

CBitmap * GetFullFeatureBitmap()
{
	return &g_FullFeatureBitmap;
}

static int CalcRegLevel(int regPolicy, int regMonth)
{
	return 19*(((((GetRandomNumber()&0xff)<<8)|regPolicy)<<8)|regMonth);
}

static int GetRandomNumber()
{
	static bool seeded = false;
	int theRandomNumber;

	if (!seeded)
	{
		srand(time(NULL));
		seeded = true;
	}
	theRandomNumber = rand();
	return theRandomNumber;
}
