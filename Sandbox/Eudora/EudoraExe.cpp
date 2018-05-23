#include "StdAfx.h"
#include "initguid.h"
#include "EudoraExe.h"
#include "Automation.h"
#include "QComApplication.h"
#include "TraceFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// This cpp file is part of ATL and must be included in this way
//#include "atlimpl.cpp"

#define IID_DEFINED
// This file is dynamically generated from the IDL file and must be included in this way
#include "EudoraExe_i.c"

/////////////////////////////////////////////////////////////////////////////
// Construct Eudora application

CEudoraModule _Module;

/////////////////////////////////////////////////////////////////////////////
// Global application variables

CEudoraApp* g_pApp = NULL;
QComApplication* g_pApplication = NULL;

/////////////////////////////////////////////////////////////////////////////
// Externally constructable object map

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_EuApplication, QComApplication)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEudoraModule

CEudoraModule::CEudoraModule()
{
#ifdef _DEBUG
	TraceStart();
#endif

	// create the Eudora app
	g_pApp = new CEudoraApp;
}

CEudoraModule::~CEudoraModule()
{
	delete g_pApp;
	g_pApp = NULL;
}

LONG CEudoraModule::Lock()
{
	if (GetLockCount() == 0)
	{
		TRACE0("Automation: Locking application in memory\n");
		AfxOleLockApp();
	}

	return CComModule::Lock();
}

LONG CEudoraModule::Unlock()
{
	ASSERT(GetLockCount() > 0);

	LONG l = CComModule::Unlock();

	if (GetLockCount() == 0)
	{
		TRACE0("Automation: Unlocking application in memory\n");
		AfxOleUnlockApp();
	}

	return l;
}

void CEudoraModule::InitObjectMap(HINSTANCE hInst)
{
	_Module.Init(ObjectMap, hInst);
	_Module.dwThreadID = GetCurrentThreadId();
}
