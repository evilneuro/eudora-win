#ifndef _AUTH_RPA_H
#define _AUTH_RPA_H

#define SECURITY_WIN32	//Needed in sspi.h
#include "Sspi.h"
#include "QCWorkerSocket.h" //for Network
#include "Callbacks.h"

class QCAuthRPA {

	INIT_SECURITY_INTERFACE initFunctionTable;  
	OSVERSIONINFO           versionInfo;
	char                    securityDllName[_MAX_PATH];
	PSecurityFunctionTable  securityFuncs;	//static PSecurityFunctionTable  securityFuncs;
	HINSTANCE      securityDLL;		//static HINSTANCE        securityDLL= NULL;

private:
	TimeStamp        lifetime;   // do I need this??
	CredHandle      credentials;
	CtxtHandle		context;//static CtxtHandle              context;
	Network *m_NetStream;
	
	//Due to a bug in Security.dll, we can't safely call to delete an invalid credential handle.
	//So we keep track of valid handle like this
	bool m_bValidCred;

	//Error Callback
	Callback1<const char *> ReportError;

public:

	QCAuthRPA(Network *NetStream, Callback1<const char *> err);
	int LoadRPALibrary();
	int GetCredentialHandle();
	int Authenticate(CString strPOPAccount);
	void Cleanup();

	void ErrorDialog(UINT StringID, ...);

};

#endif