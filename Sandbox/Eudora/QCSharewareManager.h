// QCSharewareManager.h: interface for the QCSharewareManager class.
//
// 3/3/98, S. Manjourides
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QCSHAREWAREMANAGER_H__D0BB7530_D25B_11D2_A3E8_00805F9BF4D7__INCLUDED_)
#define AFX_QCSHAREWAREMANAGER_H__D0BB7530_D25B_11D2_A3E8_00805F9BF4D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// --------------------------------------------------------------------------

//#include "NewIfaceMT.h"
#include "QCCommandDirector.h"
#include "QCCommandActions.h"

#include <list>

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

// --------------------------------------------------------------------------

typedef enum {
	SWM_MODE_ADWARE,
	SWM_MODE_LIGHT,
	SWM_MODE_PRO
} SharewareModeType;

// --------------------------------------------------------------------------

class QCSharewareManager : public QCCommandDirector
{
public:
	static bool Initialize(); // Init the one-and-only SharewareManager
	static bool Destroy();    // Destroy the SharewareManager

	// Get access to the SharewareManager
	static QCSharewareManager* GetSharewareManager() {	ASSERT(m_pTheSharewareManager); return (m_pTheSharewareManager); }

	bool UsingFullFeatureSet() { return (m_Mode != SWM_MODE_LIGHT); }
	void SetMode(SharewareModeType Mode);
	SharewareModeType GetMode() { return (m_Mode); }

protected:
	// Protected constructor
	QCSharewareManager();
	virtual ~QCSharewareManager();

	static QCSharewareManager* m_pTheSharewareManager;

	// ----------

	void Notify(unsigned int nNotifyFlag, LPARAM lParam = 0);
	bool RemoveReg(CWnd* pWnd);

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

	list<CNotifyClientWrapper> m_RegClientList; // The list of clients to notify of changes
	SharewareModeType m_Mode; // Free, Paid, or Ads
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

inline SharewareModeType GetSharewareMode()
{
	QCSharewareManager* pSWM = GetSharewareManager();
	return (pSWM? pSWM->GetMode() : SWM_MODE_ADWARE);
}

#endif // !defined(AFX_QCSHAREWAREMANAGER_H__D0BB7530_D25B_11D2_A3E8_00805F9BF4D7__INCLUDED_)
