// QCRASConnection.h
// 
// Manages dynamic loading of RAS Library DLL and establishment and
// shutdown of a RAS connection.
//

#ifndef __QCRASCONNECTION_H__
#define __QCRASCONNECTION_H__

#include <ras.h>
#include <raserror.h>
#include <rasdlg.h>
#include "Callbacks.h"
#include "TaskErrorTypes.h"

//
// Typedefs for RAS API functions.
//
typedef DWORD (APIENTRY RAS_RASDIAL)(LPRASDIALEXTENSIONS, LPSTR, LPRASDIALPARAMS, DWORD, LPVOID, LPHRASCONN);
typedef DWORD (APIENTRY RAS_RASEDITPHONEBOOKENTRY)(HWND, LPCTSTR, LPCTSTR);
typedef DWORD (APIENTRY RAS_RASCREATEPHONEBOOKENTRY)(HWND, LPCTSTR);

typedef WORD (APIENTRY RAS_RASENUMCONNECTIONS)(LPRASCONN, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY RAS_RASENUMENTRIES)(LPSTR, LPSTR, LPRASENTRYNAME, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY RAS_RASGETCONNECTSTATUS)(HRASCONN, LPRASCONNSTATUS);
typedef DWORD (APIENTRY RAS_RASGETERRORSTRING)(UINT, LPSTR, DWORD);
typedef DWORD (APIENTRY RAS_RASHANGUP)(HRASCONN);
typedef DWORD (APIENTRY RAS_RASGETENTRYDIALPARAMS)(LPTSTR, LPRASDIALPARAMS, LPBOOL);
typedef DWORD (APIENTRY RAS_RASSETENTRYDIALPARAMS)(LPTSTR, LPRASDIALPARAMS, BOOL);
typedef BOOL (APIENTRY RAS_RASDIALDLG)(LPTSTR, LPTSTR, LPTSTR, LPRASDIALDLG);


/*
class AFX_EXT_CLASS QCRasConnection //: public CWnd//: private CWnd
{
public:
	virtual ~QCRasConnection(){}

	bool OpenConnection(const CString strConnectionName, Callback1<const char *>);
	bool CloseConnection();
	void DialConnection(CString strConnectionName, CString strUserName, 
		CString strPassword, CString strDomain);

private:

	// Handle to our one and only RAS connection.
	HRASCONN m_hRasConn;

};*/


class AFX_EXT_CLASS QCRasLibrary
{
	
public:
	typedef Callback2<const char *, TaskErrorType> ErrorCB;
	typedef Callback1<const char *> ProgressCB;

	static bool IsReady();
	static HRESULT LoadRasLibrary_(BOOL bShowError);
	static HRESULT FreeRasLibrary_();

	static bool IsLibraryLoaded(){ return m_bLibraryLoaded; }
	
	static bool GetRasPassword(CString strEntry, RASDIALPARAMS* pRasDialParams, bool *pSavePassFlag);
	static HRESULT GetRasConnectionList(CStringList& strConnectionList);
	static HRESULT FillRasConnectionNamesCombo(CComboBox* pComboBox, const char *entry);
	static void ResetRasDialParams(RASDIALPARAMS *pRasDialParams, const char *szEntry);
	
	static bool OpenConnection(RASDIALPARAMS *pRasDialParams, bool bSavePass, int nTimeOut,
					ProgressCB Progress, ErrorCB Error);
	static bool CloseConnection(bool bHardClose=false);
	static void RequestStop();
	static void ResynchRasConnection();
	static bool DialConnection(RASDIALPARAMS *pRasDialParams, bool bSavePass, int nTimeOut);

	
	//FORNOW	HRESULT IsRasConnectionOpen() const;
	//static HRESULT SetRasPassword(const CString& strRasPassword, BOOL bSavePassword);
	//static HRESULT GetRasPassword(CString& strRasPassword);
	//static HRESULT ForgetRasPassword();


	

	//
	// Function pointer declarations.
	//
	static RAS_RASDIALDLG* m_pfnRasDialDlg;
	static RAS_RASCREATEPHONEBOOKENTRY* m_pfnRasCreatePhonebookEntry;
	static RAS_RASEDITPHONEBOOKENTRY* m_pfnRasEditPhonebookEntry;

	static RAS_RASDIAL* m_pfnRasDial;
	static RAS_RASENUMCONNECTIONS* m_pfnRasEnumConnections;
	static RAS_RASENUMENTRIES* m_pfnRasEnumEntries;
	static RAS_RASGETCONNECTSTATUS* m_pfnRasGetConnectStatus;
	static RAS_RASGETERRORSTRING* m_pfnRasGetErrorString;
	static RAS_RASHANGUP* m_pfnRasHangUp;
	static RAS_RASGETENTRYDIALPARAMS* m_pfnRasGetEntryDialParams;
	static RAS_RASSETENTRYDIALPARAMS* m_pfnRasSetEntryDialParams;
	
	static void Init(bool);
	static bool HasValidActiveConn(CString strEntry);
	static void Hangup();
	static bool IsActive();
	static bool GetActiveConn(CString& strEntry);
	//static void SetActiveConn(CString strEntry);
	//static void UnsetActiveConn(CString strEntry);
	static bool UsingRasDialDlg(){ return m_bCanUseRasDialDlg; }

	static Callback1<const char *> g_Progress;
	static Callback2<const char *, TaskErrorType> g_Error;

private:

	QCRasLibrary(const QCRasLibrary&);
	void operator=(const QCRasLibrary&);

	static CString m_strEntry;
	static bool m_bActiveConn;

	// Handle to our one and only RAS connection.
	static HRASCONN m_hRasConn;

	static bool m_bLibraryLoaded;
	static bool m_bCanUseRasDialDlg;
	static bool m_bStopRequested;

	static bool m_bConnectedByEudora;
	static CString m_strEntryNotConnectedByEudora;

	static CCriticalSection m_RasGuard;
	static bool m_bUseExistingConn;

	//
	// HINSTANCE for dynamically-loaded DLL.
	//
	static HINSTANCE m_hRasLibrary;
	static HINSTANCE m_hRasDlgLibrary;
	
};





#endif // __QCRASCONNECTION_H__
