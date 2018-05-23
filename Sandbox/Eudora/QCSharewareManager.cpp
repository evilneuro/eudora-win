// QCSharewareManager.cpp: implementation of the QCSharewareManager class.
//
// 3/3/98, S. Manjourides
//
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "rs.h" // For INI stuff

#include "QCSharewareManager.h"

#ifdef _DEBUG
	#undef new
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif // _DEBUG

// --------------------------------------------------------------------------

UINT msgSharewareNotification = ::RegisterWindowMessage("msgSharewareNotification");
/* static */ QCSharewareManager *QCSharewareManager::m_pTheSharewareManager = NULL;

// --------------------------------------------------------------------------

// [PUBLIC] Initialize
/* static */ bool QCSharewareManager::Initialize()
{
	TRACE("QCSharewareManager::Initialize()\n");
	ASSERT(!m_pTheSharewareManager);

	if (m_pTheSharewareManager)
		delete m_pTheSharewareManager;

	m_pTheSharewareManager = new QCSharewareManager;

	if (m_pTheSharewareManager)
		m_pTheSharewareManager->Load();

	return (m_pTheSharewareManager != NULL);
}

// --------------------------------------------------------------------------

// [PUBLIC] Destroy
/* static */ bool QCSharewareManager::Destroy()
{
	TRACE("QCSharewareManager::Destroy()\n");

	if (m_pTheSharewareManager)
	{
		m_pTheSharewareManager->Save();

		delete m_pTheSharewareManager;
		m_pTheSharewareManager = NULL;
	}

	return (true);
}

// --------------------------------------------------------------------------

// [PROTECTED] QCSharewareManager (Default Constructor)
QCSharewareManager::QCSharewareManager()
{
	// Assume Adware to start
	m_Mode = SWM_MODE_ADWARE;
}

// --------------------------------------------------------------------------

// [PROTECTED] ~QCSharewareManager (Destructor)
QCSharewareManager::~QCSharewareManager() // virtual
{
}

// --------------------------------------------------------------------------

// [PUBLIC] SetMode
//
// Sets the current feature set
//
void QCSharewareManager::SetMode(SharewareModeType Mode)
{
	if (m_Mode == Mode)
		return;

#ifdef _DEBUG
	if (Mode == SWM_MODE_LIGHT)
	{
		if (AfxMessageBox("Goin' Light.  Doing so will get rid of some customized info.  Are you sure you want to do this?",
			MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
		{
			return;
		}
	}
#endif

	SharewareModeType OldMode = m_Mode;
	m_Mode = Mode;

	// Notify any registered wnds of the change in feature set
	NotifyClients(NULL, CA_SWM_CHANGE_FEATURE, (void *)&OldMode);
}

// --------------------------------------------------------------------------

// [PROTECTED] Load
//
// Grab state info from INI
//
bool QCSharewareManager::Load()
{
	int val = 0;
	if (GetEudoraProfileInt("DEBUG", "FeatureMode", &val))
		m_Mode = (SharewareModeType)val;

	return (true);
}

// --------------------------------------------------------------------------

// [PROTECTED] Save
//
// Save state info to INI
//
bool QCSharewareManager::Save()
{
	WriteEudoraProfileInt("DEBUG", "FeatureMode", m_Mode);

	return (true);
}

