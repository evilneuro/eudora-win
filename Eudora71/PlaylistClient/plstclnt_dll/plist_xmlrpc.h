// plist_xmlrpc.h

#ifndef _PLIST_XMLRPC_H_
#define _PLIST_XMLRPC_H_

#include "plist_inet.h"
#include "plist_utils.h"
#include "plist_parser.h"
#include "process.h"
#include "plist_cinfo.h"

struct ClientUpdateParams {
	char userAgent[64];
    char playlistID[256];
	char langID[6];
    PlistCInfoPtr pcip;
	int  faceTime;
	int  faceTimeLeft;
	int  dbHdl;
	char *profile;
	char *distributor;
	void *dbRef;
#ifdef _DEBUG
	bool ignoreCurrent;
#endif
};

////////////////////////////////////////////////////////////////////////////////
// ClientUpdateContext -- threaded "DoClientUpdate" operation

int _DoClientUpdate( /*InetContext* pctx,*/ char* pURL, ClientUpdateParams* p, char** xml );

class ClientUpdateContext : public InetContext
{
	char* m_xml;
	char* m_pURL;
	ClientUpdateParams* m_pParams;

public:
	int m_hplst;   // temp hack doesn't work for multi-plist

private:
	ClientUpdateContext(){}

public:
	ClientUpdateContext( char* url, ClientUpdateParams* p )
		{ kind = 'updt'; m_xml = 0; m_pURL = DupeString(url); m_pParams = p; }
	virtual ~ClientUpdateContext();

	void exec()
		{ _beginthread( update_thread_proc, 0, this ); }

private:
	static void update_thread_proc( void* p );
};


#ifdef __cplusplus
extern "C" {
#endif


ClientUpdateContext* DoClientUpdate( char* pURL, ClientUpdateParams* p );

#ifdef _DEBUG
char *DBBuildClientUpdate( ClientUpdateParams *p );
#endif

#ifdef __cplusplus
}
#endif    // __cplusplus

#endif    // _PLIST_XMLRPC_H_
