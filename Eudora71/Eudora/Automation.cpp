
#include "stdafx.h"
#include "Automation.h"
#include "resource.h"
#include "rs.h"
#include "QCAutomationDirector.h"
#include "QComApplication.h"
#include "CEudoraModule.h"
#include "TraceFile.h"

#include "DebugNewHelpers.h"

bool gbAutomationEnabled = false;
bool gbAutomationFactoryRegistered = false;
bool gbAutomationRunning = false;
bool gbAutomationExternalStart = false;
bool gbAutomationCall = false;
bool gbAutomationCommandQueued = false;
bool gbAutomationCheckMail = false;
bool gbAutomatedShutdown = false;

long g_NextId;
HINSTANCE ghResourceInst = NULL;

/////////////////////////////////////////////////////////////////////////////
// Global application functions

void AutomationEnable(void)
{
	if (gbAutomationEnabled)
		return;

	// Register Eudora objects with registry
	VERIFY(SUCCEEDED(_Module.UpdateRegistryFromResource(IDR_COM_EUDORA, TRUE)));
	VERIFY(SUCCEEDED(_Module.RegisterServer(TRUE)));
	
	// Set Eudora ini file to automation enabled
	SetIniShort(IDS_INI_AUTOMATION_ENABLED, 1);
	gbAutomationEnabled = true;
}

void AutomationDisable(void)
{
	if (!gbAutomationEnabled)
		return;

	if (gbAutomationRunning)
	{
		AutomationStop();
	}

	// Unregister Eudora objects with registry
	VERIFY(SUCCEEDED(_Module.UpdateRegistryFromResource(IDR_COM_EUDORA, FALSE)));
	VERIFY(SUCCEEDED(_Module.UnregisterServer(NULL)));

	// Set Eudora ini file to automation disabled
	SetIniShort(IDS_INI_AUTOMATION_ENABLED, 0);
	gbAutomationEnabled = false;
}

void AutomationRegisterFactory(void)
{
	if (gbAutomationFactoryRegistered)
		return;

	// Registers the class factory in the Running Object Table
	VERIFY(SUCCEEDED(_Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE)));

	gbAutomationFactoryRegistered = true;
}

void AutomationUnregisterFactory(void)
{
	if (!gbAutomationFactoryRegistered)
		return;

	// Unregisters the class factory in the Running Object Table
	_Module.RevokeClassObjects(); // revoke class factories for ATL
	
	gbAutomationFactoryRegistered = false;
}

void AutomationStart(void)
{
	// This must only be called in response to an 
	// external object creating the one and only 
	// instance of the QComApplication
	if ((gbAutomationRunning) || (!g_pApplication))
		return;
	
	// Register this object as the active object for this class
	g_pApplication->RegisterActiveObject();

	// Save resource handle for later use when Eudora
	// is being called externally
	ghResourceInst = AfxGetResourceHandle();
	
	// Add two reference count so that we can control
	// the shutdown of this object
	g_pApplication->AutoAddRef();
//	g_pApplication->AutoAddRef();

	// Register the classs factory in the Running Object Table
	AutomationRegisterFactory();

	gbAutomationRunning = true;
}

void AutomationStop(void)
{
	if ((!gbAutomationRunning) || (!g_pApplication))
		return;

	// Fire event CLOSE to clients
	g_pApplication->OnClose();
	
	// Unregister the various directors (Mailbox, Plugin, Recipient, Stationary, Signature)
	g_theAutomationDirector.Stop();
	
	// Revoke this object as the current running object
	g_pApplication->UnregisterActiveObject();
	
	// Unregister the class factory in the Running Object Table
	//
	// Do this *before* AutoFree, because AutoFree forces the ref count to 0 and frees
	// the application object (which would cause AutomationUnregisterFactory to crash
	// when it tried to decrement the ref count for the application object).
	AutomationUnregisterFactory();

	g_pApplication->AutoFree();
	
	gbAutomationRunning = false;
}

long GetUniqueID(void)
{
	return (++g_NextId);
}

/////////////////////////////////////////////////////////////////////////////
// Automation call class

CAutomationCall::CAutomationCall()
{
	gbAutomationCall = true;
}

CAutomationCall::~CAutomationCall()
{
	gbAutomationCall = false;
}

