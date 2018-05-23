// EUDORA.H
//

#ifndef __EUDORA_H_
#define __EUDORA_H_


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <QCUtils.h>

// Eudora Debug Memory Manager Debugger Output class.
// smohanty: 05/27/97
#ifdef __DEBUG__
#include "DMMDumpHandlerIface.h"
#endif

#include "AutoSerializedRegionIface.h"

#include "resource.h"       // main symbols
#include "dos.h" 

#define	LOCAL_INSTANCE		0
#define REMOTE_INSTANCE		1

#ifndef EudoraMainWindowClass
	#define EudoraMainWindowClass "EudoraMainWindow"
#endif

class	CCompMessageDoc;
class   QComApplication;
class	CTranslatorManager;
class	QCMailboxDirector;


/////////////////////////////////////////////////////////////////////////////
// CEudoraApp:
// See eudora.cpp for the implementation of this class
//

class CEudoraApp : public CWinApp
{
	friend QComApplication; // attempt to keep automation code mostly seperate

	CObList	m_FilterList; // a list of all the filtered messages

// Eudora Debug Memory Manager Debugger Output class.
// smohanty: 05/27/97
#ifdef __DEBUG__
private:
    DMMDumpHandler dmmDumpHandler;
#endif

protected:
	unsigned long	m_LastIdle;
	unsigned long	m_NextCheckMail;
	unsigned long	m_NextSendQueued;
	unsigned long	m_LastOverwritePressed;	// Uggghh Seems there are some spazs who don't know how to type.
	BOOL			m_ForceCheckMail;
	 
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
	virtual BOOL OnIdle( LONG lCount );
	void LoadEPIs( void );
	void FreeResources( void );
	void AddEPIs( CString & clsDirectory );
	void FindFirstEPI( CString & clsDirectory, CString & clsEPIPath  );
	void FindNextEPI( CString & clsEPIPath );
	void FindFirstEPIFile( CString & clsDirectory, CString & clsEPIPath  );
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
	BOOL PreTranslateMessage(MSG* pMsg);
	//Manually reset the m_LastIdle so that the idle loop wont kick-in right away
	void ResetLastIdle();
	unsigned long GetLastIdle();
	void OnDoPostProcessing(WPARAM, LPARAM);

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
	CFrameWnd* NewChildFrame(CDocTemplate* templ, CDocument* pDoc);
	CDocument* NewChildDocFrame(CDocTemplate* templ, BOOL OnlyOne = TRUE);

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
	BOOL	RegisterMailto();
	BOOL	RegisterStationery();
	BOOL	RegisterCommandLine(char *cmdLine);
	BOOL	ProcessAttachmentWEI(int ct);
	BOOL	StartEudoraWithReg( void );
	BOOL	IsEudoraRunning( void );
	BOOL	IsLocalInstance( HWND hWnd );
	BOOL	IsLocalPath( CString csPath );
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
	void DoCheckProFilters();
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


// Function prototypes

inline CDocument* NewChildDocument(CDocTemplate* templ)
	{ return (((CEudoraApp*)AfxGetApp())->NewChildDocument(templ)); }
inline CFrameWnd* NewChildFrame(CDocTemplate* templ, CDocument* pDoc)
	{ return (((CEudoraApp*)AfxGetApp())->NewChildFrame(templ, pDoc)); }
inline CDocument* NewChildDocFrame(CDocTemplate* templ)
	{ return (((CEudoraApp*)AfxGetApp())->NewChildDocFrame(templ)); }
	
inline CTranslatorManager *GetTransMan()
	{ return (((CEudoraApp*)AfxGetApp())->GetTranslators()); }

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

#endif // __EUDORA_H_
