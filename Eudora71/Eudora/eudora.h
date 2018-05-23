// EUDORA.H
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

#ifndef __EUDORA_H_
#define __EUDORA_H_


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <QCUtils.h>

#include "resource.h"       // main symbols
#include "dos.h"

#define	LOCAL_INSTANCE		0
#define REMOTE_INSTANCE		1

#define JUNK_MBOX_NONE		0
#define JUNK_MBOX_SYSTEM	1
#define JUNK_MBOX_REGULAR	2

#ifndef EudoraMainWindowClass
	#define EudoraMainWindowClass "EudoraMainWindow"
#endif

class	CCompMessageDoc;
class   QComApplication;
class	CTranslatorManager;
class	QCMailboxDirector;
class	QCFaceTime;


/////////////////////////////////////////////////////////////////////////////
// CEudoraApp:
// See eudora.cpp for the implementation of this class
//

class CEudoraApp : public CWinApp
{
	friend QComApplication; // attempt to keep automation code mostly seperate

	CObList	m_FilterList; // a list of all the filtered messages

protected:
	unsigned long	m_nStartIdle;
	unsigned long	m_nStartIdleFacetime;
	unsigned long	m_nStartCurrentIdleProcessing;
	unsigned long	m_nCurrentIdlePeriod;
	unsigned long	m_nCurrentIdleFaceTime;
	BOOL			m_bCallCWinAppIdle;
	BOOL			m_bRestartIdleProcessing;
	BOOL			m_bDoingIdleTimerProcessing;

	CPoint			m_LastMouseMovePoint;
	QCFaceTime*		m_FaceTime;
	unsigned long	m_NextCheckMail;
	unsigned long	m_NextSendQueued;
	unsigned long	m_LastOverwritePressed;	// Uggghh Seems there are some spazs who don't know how to type.
	BOOL			m_ForceCheckMail;
	int				m_iJunkMbxStatus;


	CPtrList *		m_pResList;
	HANDLE			m_hFindHandle;
	WIN32_FIND_DATA m_FindData;
	CString			m_clsFindDir;
	POSITION		m_Pos;
	CSortedStringListMT m_EPIList;

	int				m_fdOwner;
	HWND			m_hOwner;
	HWND			m_hOurWnd;

	enum ResourceStatus
	{
		RESOURCE_UNKNOWN,
		RESOURCE_NOT_LOADED,
		RESOURCE_VERSION_MISMATCH,
		RESOURCE_OKAY
	};

	ResourceStatus	LoadResourceDLL( void );

	virtual BOOL IsIdleMessage( MSG* pMsg );

	BOOL IdleSortMailboxes();
	BOOL IdleUpdateFaceTime();
	BOOL IdleProcessIMAPQueues(bool bHighPriorityOnly = false);
	BOOL IdleAutoSave();
	BOOL IdleCheckOwnership();
	BOOL IdleCheckMail();
	BOOL IdleSendMail();
	BOOL IdleTaskManagerPostProcessing();
	BOOL IdleWriteModifiedTocs();
	BOOL IdleCalculateMood();
	BOOL IdleSwitchToSponsoredMode();
	BOOL IdleCheckNetscapeInteraction();
	BOOL IdleLogTocDiscrepancies();

	virtual BOOL OnIdle( LONG lCount );
	void LoadEPIs( void );
	void FreeResources( void );
	void AddEPIs( const char* clsDirectory );
	void FindFirstEPI( const char* clsDirectory, CString & clsEPIPath  );
	void FindNextEPI( CString & clsEPIPath );
	void FindFirstEPIFile( const char* clsDirectory, CString & clsEPIPath  );
	void FindNextEPIFile( CString & clsEPIPath );

	void CleanupIcons();
	void MakeEmbeddedDir();

public:
	CEudoraApp();
// Added to delete reserve memory and dump memory log
// to debugger.
// smohanty: 05/27/97
	virtual ~CEudoraApp();

// Overrides
	BOOL InitApplication();
	virtual BOOL InitInstance();
	void InitJunkFeature();
	BOOL PreTranslateMessage(MSG* pMsg);

	void SetJunkMboxStatus(int iStatus) {m_iJunkMbxStatus = iStatus;};
	
	//Manually reset the m_nStartIdle so that the idle loop wont kick-in right away
	void ResetStartIdle();
	unsigned long GetStartIdle();
	unsigned long GetIdlePeriod();
	// Facetime versions of the above functions.
	// Facetime is slightly different from normal consideration of whether the user is there or not.
	// Mouse movements indicate that the user is present for facetime, but not normally.
	void ResetStartIdleFacetime();
	unsigned long GetStartIdleFacetime();
	unsigned long GetIdlePeriodFacetime();

	virtual int Run();

	void SaveStdProfileSettings();
	int ExitInstance();

// Implementation

	//{{AFX_MSG(CEudoraApp)
	afx_msg void OnUpdateCheckMail(CCmdUI* pCmdUI);
	afx_msg void OnCheckMail();
	afx_msg void OnAppExit();
	afx_msg void OnAppAbout();
	afx_msg void OnTipOfTheDay();
	afx_msg void OnUpdateTipOfTheDay(CCmdUI* pCmdUI);
	afx_msg void OnCheckRegCodeFile();
	afx_msg void OnGetHelp();
	afx_msg void OnReportABug();
	afx_msg void OnAboutEMSPlugins();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	//}}AFX_MSG

// Debug Memory Manager command and update handlers.
// smohanty: 05/27/97
protected:

#ifdef __DEBUG__
        afx_msg void DMMDumpLog();
        afx_msg void DMMHighWatermark();
        afx_msg void DMMCurrentAlloc();
        afx_msg void DMMHeapToggle();
        afx_msg void UpdateDMMDumpLog(CCmdUI *pCmdUI);
        afx_msg void UpdateDMMHighWatermark(CCmdUI *pCmdUI);
        afx_msg void UpdateDMMCurrentAlloc(CCmdUI *pCmdUI);
        afx_msg void UpdateDMMHeapToggle(CCmdUI *pCmdUI);
#endif // __DEBUG__

	DECLARE_MESSAGE_MAP()
	
	void HandleRegError(HKEY hkey, LPCTSTR pRegPath, UINT nErrID);
  	void GetMailCheckTime(CString &CheckMailText);

public:
	
	CDocument* NewChildDocument(CDocTemplate* templ);
	CFrameWnd* NewChildFrame(CDocTemplate* templ, CDocument* pDoc, BOOL bDoIntialUpdate = TRUE);
	CDocument* NewChildDocFrame(CDocTemplate* templ, BOOL OnlyOne = TRUE, BOOL bDoIntialUpdate = TRUE);

	void			AddFilterListEntry( CObject* pEntry );
	const CObList&	GetFilterList();
		
 	void UpdateMailCheckLabel();
 	void SetNextMailCheck(unsigned long NewTime) {this->m_NextCheckMail = NewTime; } 
	void SetNextSendMail(unsigned long NewTime) {this->m_NextSendQueued = NewTime; } 

	void RegisterFormats();

	CPtrList *	GetResListPtr( void ) { return m_pResList; }

	OSVERSIONINFO	m_OSVersion;

	BOOL	AddToRegistry(HKEY hkey, const char *regPath, const char *value, const char *valueData, bool bShowError = false);
	BOOL	RegisterIcon();
	BOOL	RegisterURLSchemes();
	BOOL	RegisterMailbox();
	BOOL	RegisterStationery();
	BOOL	RegisterCommandLine(char *cmdLine);
	BOOL	ProcessAttachmentWEI(int ct);
	BOOL	DoCommandLineActions(LPCTSTR CommandLine);
	BOOL	StartEudoraWithDefaultLocation( void );
	BOOL	IsEudoraRunning( void );
	BOOL	IsLocalInstance( HWND hWnd );
	BOOL	IsLocalPath( const char* csPath );
	BOOL	LocalLateral( void );
	BOOL	RemoteInstance( void );
	void	RegisterOwnership( void );
	BOOL	CheckOwnership( void );
	void	OwnershipCleanup( void );
	
	void	WinHelp(DWORD dwData, UINT nCmd=NULL) ;
	void	SetExternalCheckMail(BOOL bWaitForIdle=TRUE);

	CTranslatorManager		*m_TransManager;
	CTranslatorManager		*GetTranslators() {return m_TransManager;}
	BOOL InitTranslators();
};

extern CEudoraApp * g_pApp;

// Data Items

// Templates for all the modeless child windows
extern CMultiDocTemplate* TextFileTemplate;
extern CMultiDocTemplate* TocTemplate;
extern CMultiDocTemplate* ReadMessageTemplate;
extern CMultiDocTemplate* CompMessageTemplate;
extern CMultiDocTemplate* FindTemplate;
extern CMultiDocTemplate* SearchTemplate;
extern CMultiDocTemplate* FiltersTemplate;
extern CMultiDocTemplate* NicknamesTemplate;
extern CMultiDocTemplate* SignatureTemplate;
extern CMultiDocTemplate* MoreHelpTemplate;
extern CMultiDocTemplate* UpdateNagTemplate;
extern CMultiDocTemplate* UsageStatsTemplate;


// Function prototypes

inline CDocument* NewChildDocument(CDocTemplate* templ)
	{ return (((CEudoraApp*)AfxGetApp())->NewChildDocument(templ)); }
inline CFrameWnd* NewChildFrame(CDocTemplate* templ, CDocument* pDoc)
	{ return (((CEudoraApp*)AfxGetApp())->NewChildFrame(templ, pDoc)); }
inline CDocument* NewChildDocFrame(CDocTemplate* templ)
	{ return (((CEudoraApp*)AfxGetApp())->NewChildDocFrame(templ)); }
	
inline CTranslatorManager *GetTransMan()
	{ return (((CEudoraApp*)AfxGetApp())->GetTranslators()); }

inline BOOL IsWin98SE()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
			  ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMinorVersion == 10) &&
			  ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwBuildNumber >= 2222; }
inline BOOL IsWinME()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
	((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMinorVersion >= 90); }
inline BOOL IsWinNT2000()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
	((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMajorVersion >= 5; }
inline BOOL IsWinXP()
	{ return IsWinNT2000() && ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMinorVersion >= 1; }
inline BOOL IsWinServer2003()
	{ return IsWinNT2000() && ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMinorVersion >= 2; }
inline BOOL IsWinVista()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
	((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMajorVersion >= 6; }

//- Added functions which are exclusive, Didn't want to add a bunch of needless compares
//- When using new functions use latest versions comparisons first
//- if you need something like 98 specifically, check is win98 && not win98SE ** not winME
//- Same goes for win NT and not win2k
// IsWinNT Gets all NT
// IsWin98 Gets all 98 and up
inline BOOL IsWinNT()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT); }
inline BOOL IsWin98()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
			  ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMinorVersion >= 10); }
inline BOOL IsWin95()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS); }
inline BOOL IsWin32s()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32s); }
inline BOOL IsVersion4()
	{ return (((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMajorVersion >= 4); }
inline	DWORD GetMajorVersion()
	{ return ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMajorVersion; }
inline	DWORD GetMinorVersion()
	{ return ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwMinorVersion; }
inline	DWORD GetBuildNumber()
	{ return ((CEudoraApp*)AfxGetApp())->m_OSVersion.dwBuildNumber; }
inline LPCTSTR GetServicePack()
	{ return ((CEudoraApp*)AfxGetApp())->m_OSVersion.szCSDVersion; }

#endif // __EUDORA_H_
