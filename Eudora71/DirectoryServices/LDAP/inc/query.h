// File:        query.h
// Description: Schmookie
// Date:        6/16/98
// Version:     1.1
///////////////////////////////////////////////////////////////////////////////

#ifndef __QUERY_H__
#define __QUERY_H__

#include <dspapi.h>
#include <isock.h>
#include "record.h"
#include "config.h"
#include "ServConfig.h"

// umich ldap -- note that while the other protocol drivers use ISOCK, the
// umich libs provide their own socket layer. when built for windows, this
// means WINSOCK. for some reason, WINSOCK needs to be defined in addition
// to WIN32, or lots of build errors ensue. kinda strange. The NEEDPROTOS
// stuff is -- you guessed it -- ANSI C Prototypes. Is that unbelievable?!?
#define WINSOCK 1
#define NEEDPROTOS 1

#include <time.h>
#include <assert.h>

#include "lber.h"
#include "ldap.h"
//#include "disptmpl.h"


// BOG: this really sucks. I can't believe that stl is really that messed-up
// on Microsoft. apparently all the string impls suck. whatever.
#include <xstddef>
#pragma warning(disable : 4663 4244 4100 4511 4512 4018)
#include <string>
#pragma warning(default : 4663 4244 4100 4511 4512 4018)

using namespace std;


// this macro converts FuncName to xFuncName (the static callback wrapper
// function)
#define CPPCALLBACK(name) (x##name)
#define ISOCKCALLBACK(name) (ISOCKCB)CPPCALLBACK(name)

#define DEFCPPCALLBACK(classname, name)                                 \
    static void CPPCALLBACK(name)(classname * pThis) {pThis->name();}   \
        void name();


class CPHRecord;


///////////////////////////////////////////////////////////////////////////////
// BOLdapQuery class

class BOLdapBootyList;
class BOLdapQuery : public IDSPQuery
{
    long m_cRef;
    BOOL m_bConnected;
    QUERY_STATUS m_nQueryStatus;
    LPSTR m_pszDatabaseID;
    time_t m_tTimeStarted;
    time_t m_tTimeOut;
    unsigned long m_nFound;
    unsigned long m_nLimit;

    LPIEXISTCFG m_pServer;          // configuration
    LPISCHEDULE m_pISchedule;       // scheduling/dispatch
    DBQUERYCB m_pfnQueryCB;         // callback on our client
    LPVOID m_pContext;              // "context" *is* our client

    BOLdapBootyList* m_pTheBooty;


    // BOG: CPHRecord is legacy -- we brought this in from another project.
    // The name will change soon. I don't know about caching this. The old
    // ldap driver did this, and filled it a bit at a time; we fill it in one
    // shot, so why keep it around? We'll see.

    CPHRecord* m_pRecord;     // current record we are building

    // search params
    LPSTR m_pszSearch;        // term we're searching for
    DWORD m_dwFlags;          // i haven't the foggiest
    int m_nField;             // ditto

    // umich stuff
    LDAP* m_pLD;              // ptr to ldap session
    int m_ldMsgID;            // points to search results
    LDAPMessage* m_pldMsg;    // storage for search results
    string m_SearchFilter;    // big ol' nasty search filter

public:
    BOLdapQuery(IUnknown *punkParent);
    ~BOLdapQuery();

private:
    HRESULT CreateSocket();
    BOOL CreateRecord();
    bool BuildRecord();
    BOOL Connect();
    void SetStatus(QUERY_STATUS nStatus);
    void ScheduleCallback();
    void SetDatabaseID(LPSTR pszID);

    // timeout handling
    void InitTimeout( time_t tIntvl = 0 );
    bool TimedOut();

    DEFCPPCALLBACK(BOLdapQuery, Callback)

    // IUnknown
    STDMETHOD(QueryInterface)(const IID& iid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IDSPQuery
    STDMETHOD(InitQuery)(LPSTR pszSearch,DWORD dwFlags, DS_FIELD nField);
    STDMETHOD(SetSearchBase)(LPSTR pszSearchBase);
    STDMETHOD_(void, Start)(DBQUERYCB pCB,LPVOID pCtx);
    STDMETHOD_(void, WaitForRecord)(DBQUERYCB pCB,LPVOID pCtx);
    STDMETHOD_(QUERY_STATUS, GetRecord)(IDSPRecord **ppRecord);
    STDMETHOD_(void, Cancel)();
    STDMETHOD_(QUERY_STATUS, GetLastError)();
    STDMETHOD(GetErrorString)(QUERY_STATUS nCode,LPSTR pszBuffer,int nlen);
};

#endif __QUERY_H__

