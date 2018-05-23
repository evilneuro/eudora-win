// QCSharewareManager.h: interface for the QCSharewareManager class.
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

#if !defined(AFX_QCSHAREWAREMANAGER_H__D0BB7530_D25B_11D2_A3E8_00805F9BF4D7__INCLUDED_)
#define AFX_QCSHAREWAREMANAGER_H__D0BB7530_D25B_11D2_A3E8_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

#include "QCCommandDirector.h"
#include "QCCommandActions.h"
#include "rs.h" // For INI stuff
#include "resource.h"
#include "..\Version.h"

#ifndef EMSR_OK
#include "ems-wglu.h"		// For ModeTypeEnum
#endif

#include <list>

// --------------------------------------------------------------------------

typedef enum {
	SWM_MODE_ADWARE,	// aka sponsored mode
	SWM_MODE_LIGHT,
	SWM_MODE_PRO,		// aka paid mode
	SWM_MODE_DEADBEAT
} SharewareModeType;

#ifndef DEFAULT_SWM_MODE
	#ifdef BUILD_BOX_OR_SITE_R_VERSION
		#define DEFAULT_SWM_MODE (2)
	#else
		#define DEFAULT_SWM_MODE (0)
	#endif
#endif

// Get to stay in paid mode with an expired reg code this long before getting
// switched to sponsored mode.
#define PAID_MODE_GRACE_SECS	(60 * 60)	// one hour

#define PAID_MODE_DEMO_DAYS 30

// --------------------------------------------------------------------------

class QCSharewareManager : public QCCommandDirector
{
public:
	static bool Initialize(); // Init the one-and-only SharewareManager
	static bool Destroy();    // Destroy the SharewareManager

	// Get access to the SharewareManager
	static QCSharewareManager* GetSharewareManager() {	ASSERT(m_pTheSharewareManager); return (m_pTheSharewareManager); }

	bool UsingFullFeatureSet() const { return (m_Mode != SWM_MODE_LIGHT); }
	ModeTypeEnum GetCurrentPaidMode () const;
	bool UsingPaidFeatureSet() const;
	bool UsingPaidForX1FeatureSet() const;
	bool IsDeadbeat() const { return m_bDeadbeat; }
	int  DemoDaysLeft() const;
	bool IsNewDemoBuild() const { return m_DemoBuildMonth < EUDORA_BUILD_MONTH; }

	// Sets the current mode.
	// Returns whether or not the actual mode was changed.
	bool SetMode(SharewareModeType Mode, bool bAskUser = true, CWnd* pParentWnd = NULL,
		bool bUserSpecificallyChangedMode = false);

	SharewareModeType GetMode() { return (m_Mode); }
	bool					DoRefund(CWnd* pParentWnd = NULL);

	static SharewareModeType GetModeFromIni() 
	{
#ifdef DEATH_BUILD
		return SWM_MODE_PRO;
#else
		int nMode = GetIniInt(IDS_INI_MODE);
		if  (nMode == -1) 
			nMode = DEFAULT_SWM_MODE; 
		return static_cast<SharewareModeType>(nMode);
#endif
	}

	static bool				AreRegCodesEqual(const char * szRegCodeLHS, const char * szRegCodeRHS);

	// If bReally is false, these will return true for paid mode in the box/ESD build
	// whether or not it's really registered.
	// If expiredPaidModeOK is true, these will return true in paid mode if there's
	// a valid but expired paid mode reg code.
	static bool				IsRegisteredForCurrentMode(bool bReally = FALSE, bool expiredPaidOK = FALSE)
								{ return IsRegisteredForMode( GetModeFromIni(), NULL, bReally, expiredPaidOK); }
	static bool				IsRegisteredForMode(SharewareModeType inMode, int * pMonth = NULL, bool bReally = FALSE,
							                    bool expiredPaidOK = FALSE, int * pPolicy = NULL);

	// Returns true if there's a valid paid mode reg code, even if it's
	// expired. If it is expired, *expired is set to true.
	static bool				PaidModeRegCodeExists(bool *expired = NULL);

	// Returns true if it's OK to be in paid mode, whether or not there's a reg code.
	static bool				IsPaidModeOK()
								{ return IsRegisteredForMode(SWM_MODE_PRO, NULL, FALSE, FALSE); }

	// These implement the policy that we never nag the user to register if (a) there's a valid
	// reg code for any mode, even if it's expired or (b) the NeverRegister=1 switch is present
	// in the .ini file. NeedsRegistrationNag() and GetNeedsPaidRegistrationNag() both return
	// the same result.
	static bool				NeedsRegistrationNag();
	static bool				GetNeedsPaidRegistrationNag();

	// This only updates the RegStatus setting in the .ini file, which nothing looks at any more.
	// This situation needs to be cleaned up.
	static void				SetNeedsPaidRegistrationNag(bool bNeedsRegistrationNag);

	static bool				GetCheckedForMissingCRLF()
								{ return GetRegStatusFlag(CHECKED_FOR_MISSING_CRLF); }
	static void				SetCheckedForMissingCRLF(bool bChecked)
								{ SetRegStatusFlag(CHECKED_FOR_MISSING_CRLF, bChecked); }

		// GetRegCodeForCurrentMode, et. al. - Get the registration info for the mode that
		// the user is currently in. Use the ini value so that this can be called before
		// QCSharewareManager is loaded (i.e. from the splash screen about box at startup).
		// Besides we access ini settings in GetRegCodeForMode anyway... - GCW
		// With the exception of GetRegLevelForMode() and GetRegLevelForCurrentMode(),
		// none of these actually validate the the registration info in the .ini file. - JR
	static const char*		GetRegCodeForMode(SharewareModeType inMode);
	static const CString		GetRegCodePlusMonthForMode(SharewareModeType inMode);
	static const char*		GetRegCodeForCurrentMode()
								{ return GetRegCodeForMode( GetModeFromIni() ); }
	static const CString		GetRegCodePlusMonthForCurrentMode()
								{ return GetRegCodePlusMonthForMode( GetModeFromIni() ); }
	static const char*		GetFirstNameForMode(SharewareModeType inMode);
	static const char*		GetFirstNameForCurrentMode()
								{ return GetFirstNameForMode( GetModeFromIni() ); }
	static const char*		GetLastNameForMode(SharewareModeType inMode);
	static const char*		GetLastNameForCurrentMode()
								{ return GetLastNameForMode( GetModeFromIni() ); }
	static const char*		GetFullNameForMode(SharewareModeType inMode);
	static const char*		GetFullNameForCurrentMode()
								{ return GetFullNameForMode( GetModeFromIni() ); }
	static const char*		GetRegLevelForMode(SharewareModeType inMode);
	static const char*		GetRegLevelForCurrentMode()
								{ return GetRegLevelForMode( GetModeFromIni() ); }
	// End scope of above comment.

	bool					InternalWasBadProRegCodeAtStartup() const { return m_bWasBadProRegCodeAtStartup; }
	static bool				WasBadProRegCodeAtStartup();
	bool					InternalWasExpiredProRegCodeAtStartup() const { return m_bWasExpiredProRegCodeAtStartup; }
	static bool				WasExpiredProRegCodeAtStartup();

	// Returns false on an expired paid mode reg code, but in that case
	// *pExpiredButOtherwiseValid is set true. *pMonth returns the month
	// encoded in the reg code, not the month of expiration.
	static bool				IsValidRegistrationInfo(
								const char *		firstName,
								const char *		lastName,
								const char *		regCode,
								SharewareModeType*	pMode = NULL,
								int *				pMonth = NULL,
								int *				pPolicy = NULL,
								bool *				pExpiredButOtherwiseValid = NULL);

	// Stores the passed registration data in the .ini file, and only checks
	// the mode for validity. Flushes the .ini file and makes sure the rest
	// of the app is aware of the change.	
	static void				SetRegistrationInfo(
								const char *		firstName,
								const char *		lastName,
								const char *		regCode,
								SharewareModeType	regMode);

	// In the box/ESD build, if there's no paid mode reg code, this returns the
	// agreed-upon fake registration info (policy 41 and all that) and sets
	// *pIsFakeRegInfo to true.
	static bool				GetPaidModeRegInfo(
								CString&			regCode,
								CString&			regFirst,
								CString&			regLast,
								CString&			regLevel,
								bool *				pIsFakeRegInfo = NULL);

	static bool				IsBoxBuild();

	// These functions pertain to the grace period someone with an expired
	// paid mode reg code is allowed before being switched to sponsored mode.
	// The time stored is the time the switch is supposed to occur.
	static bool				GetAdwareSwitchTime(time_t *theTime = NULL);
	static bool				SetAdwareSwitchTime();
	static bool				ClearAdwareSwitchTime();

protected:
	enum { NEEDS_PAID_REG_NAG		= 0x00000001,
		   CHECKED_FOR_MISSING_CRLF	= 0x00000002,
		   NO_REG_NAG				= 0x00000004 };

	static bool				GetRegStatusFlag(long nFlag);
	static void				SetRegStatusFlag(long nFlag, bool bValue);
	
	// Protected constructor
	QCSharewareManager();
	virtual ~QCSharewareManager();

	static QCSharewareManager* m_pTheSharewareManager;

	// ----------

	void Notify(unsigned int nNotifyFlag, LPARAM lParam = 0);
	bool RemoveReg(CWnd* pWnd);

	void					NotifyPluginsOfMode() const;

	bool Load();
	bool Save();

	// ----------

	class CNotifyClientWrapper
	{
	public:
		CNotifyClientWrapper(unsigned int fgs = 0, CWnd* wnd = NULL) : nFlags(fgs), pWnd(wnd) { };

		unsigned int nFlags;
		CWnd* pWnd;
	};

	std::list<CNotifyClientWrapper> m_RegClientList; // The list of clients to notify of changes
	SharewareModeType m_Mode; // Free, Paid, or Ads
	bool	m_bWasBadProRegCodeAtStartup;
	bool	m_bWasExpiredProRegCodeAtStartup;
	bool    m_bDeadbeat;
	time_t	m_AdwareSwitchTime;
	int		m_DemoBuildMonth;
	unsigned int m_DemoStartDate;
};


inline QCSharewareManager* GetSharewareManager()
{
	return QCSharewareManager::GetSharewareManager();
}


inline bool UsingFullFeatureSet()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM? pSWM->UsingFullFeatureSet() : false);
}


inline ModeTypeEnum GetCurrentPaidMode()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM? pSWM->GetCurrentPaidMode() : EMS_ModeFree );
}


inline bool UsingPaidFeatureSet()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM? pSWM->UsingPaidFeatureSet() : false );
}


inline bool UsingPaidForX1FeatureSet()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM? pSWM->UsingPaidForX1FeatureSet() : false );
}


inline SharewareModeType GetSharewareMode()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (SharewareModeType)(pSWM? pSWM->GetMode() : DEFAULT_SWM_MODE);
}


inline bool IsDeadbeat()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return pSWM ? pSWM->IsDeadbeat() : false;
}


void OnUpdateFullFeatureSet(CMenu* pMenu, UINT nItem, BOOL bItemIsID);
void OnUpdateFullFeatureSet(CCmdUI* pCmdUI);
void FeatureNotInFree();

CBitmap * GetFullFeatureBitmap();

#endif // !defined(AFX_QCSHAREWAREMANAGER_H__D0BB7530_D25B_11D2_A3E8_00805F9BF4D7__INCLUDED_)
