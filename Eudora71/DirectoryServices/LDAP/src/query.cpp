//
// File:        query.cpp
// Description: Schmookie
// Date:        6/16/98
// Version:     1.1
//
// Notes:   This is the guts of the converstion to umich libs. It started life
// as the query module for the finger driver. A lot of the cruft has been
// stripped out, but some remains in the form of dead code. This will get
// removed after schlumberger get their first release.
//
// Todo:
// - Implimentation of binary data handling. This is actually a bug, and could
//   cause bigtime ugliness.
// - It would be nice to find a solution to the long ldap_open() problem. If a
//   server cannot be found, ldap_open() blocks for quite some time, this
//   freezes the ui, until the operation times-out. I have not found a way to
//   change the timeout. We may be stuck with this one.
// - Strip all the dead code left over from finger.
// - Filtering the returned records on the server would be great, but I
//   really doubt if anyone would notice. Currently we just get back everything
//   and pick out what we want [attributes from properties dialog].
//
///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <objbase.h>

#include "DebugNewHelpers.h"
#include "trace.h"
#include "query.h"
#include "resource.h"
#include "Password.h"
#include "QCUtils.h"

extern DWORD g_dwComponentCount;
extern HINSTANCE g_hInstance;

///////////////////////////////////////////////////////////////////////////////
// local routines


// be careful with this!
#define _cstr_(s) ((char*)s.c_str())

// hide a little  umich ugliness
typedef char** ValueList;

// return values for asynch wait routines
enum WaitResult {
    kWaitMore = 0,             // must be zero. don't change.
    kGrooveOnTheRecord,        // wait no more; the record is nigh
    kGrooveOnTheConnection,    // wait no more; we have a connection

    // use this when ya just don't care 'bout nothin'
    kNonSpecificBummer = 0xFFFF
};


// umich
int start_ldap_session( const char* server, int port,  LDAP** ppLd, const char* user, const char* password, bool bLogonReqd, bool bSSLReqd, const char* searchBase );
void end_ldap_session( LDAP* pLd );
void make_filter( const char* searchTerm, const char* tmpl, string& filter );
void build_filter_string( const char* searchTerms, const char* tmpl,
                          const char* cnTmpl, string& filter );
void AND_filters( const char* p1, const char* p2, string& filter );
void OR_filters( const char* p1, const char* p2, string& filter );
WaitResult wait_for_record( LDAP* pLd, int msgID,  LDAPMessage** ppRes );
WaitResult wait_for_connect( LDAP* pLd, int msgID );


///////////////////////////////////////////////////////////////////////////////
// "Booty" list for holding/sorting results

#define BL_CHUNKSIZE 5       // number of elements to grow by when reallocating
#define nil          NULL    // why does everybody do this different?
#define BL_NAMELEN   128


struct BOLdapBooty
{
    char      bootyName[BL_NAMELEN];
    DS_FIELD  bootyType;
    ValueList bootyValue;
    char*     bootyUserName;

    friend class BOLdapBootyList;

private:
    BOLdapBooty* next;
};


// BOLdapBootyList - a quick hack that solves an architectural problem wrt
// order-dependency for attributes of the same type. this is most obvious
// with the DS_NAME type, which is the case this hack deals with.
//
// Symptom:
//
// The user picks an attribute and maps it to "Name", only to find that it does
// not display in the "Name" column as they expect, due to that fact that it
// was not the first DS_NAME typed attribute returned by the server.
//
// Note: memory not allocated by operations of this class will not be
// freed by operations of this class, nor when the collection is destroyed.

class BOLdapBootyList
{
    BOLdapBooty* m_theBooty;
    BOLdapBooty* m_pList;
    BOLdapBooty* m_pTail;

    unsigned m_nSlots;
    unsigned m_nEntries;

    bool grow_booty_list() {
        bool bRet = false;
        size_t newSize = ((sizeof(BOLdapBooty) * m_nSlots) +
                          (sizeof(BOLdapBooty) * BL_CHUNKSIZE));

        BOLdapBooty* pNew = (BOLdapBooty*) malloc( newSize );

        if ( pNew ) {
            BOLdapBooty* pTrav = m_pList;
            BOLdapBooty* pTarg = pNew;

            for ( ; pTrav->next; pTarg++, pTrav = pTrav->next ) {
                memcpy( pTarg, pTrav, sizeof(BOLdapBooty) );
                pTarg->next = pTarg + 1;
            }

            memcpy( pTarg, pTrav, sizeof(BOLdapBooty) );
            pTarg->next = nil;

            free( m_theBooty );
            m_pList = m_theBooty = pNew;
            m_pTail = pTarg;
            m_nSlots += BL_CHUNKSIZE;
            bRet = true;
        }

        return bRet;
    }

public:

    BOLdapBootyList() {
        m_nSlots = BL_CHUNKSIZE;
        m_nEntries = 0;

        m_theBooty =
            (BOLdapBooty*) malloc( sizeof(BOLdapBooty) * m_nSlots );

        m_pList = NULL;
        m_pTail = NULL;
    }
                
    ~BOLdapBootyList() {
        if ( m_theBooty )
            free( m_theBooty );
    }
    
    // performs an insert operation. the list is built so that user defined
    // names come first, and the "cn" name-type comes before anything else.
    bool InsertBooty( char* bootyName, DS_FIELD bootyType,
                      ValueList bootyValue, char* bootyUserName ) {

        if ( m_nEntries == m_nSlots )
            if ( !grow_booty_list() )
                return false;

        BOLdapBooty& theEntry = m_theBooty[m_nEntries];
        strncpy( theEntry.bootyName, bootyName, BL_NAMELEN );
        theEntry.bootyName[BL_NAMELEN - 1] = '\0';
        theEntry.bootyType = bootyType;
        theEntry.bootyValue = bootyValue;
        theEntry.bootyUserName = bootyUserName;
        theEntry.next = nil;

        if ( !m_pList )
            m_pList = m_pTail = &theEntry;
        else if ( stricmp( theEntry.bootyName, "cn" ) == 0 ) {
            BOLdapBooty* pTmp = m_pList;
            m_pList = &theEntry;
            m_pList->next = pTmp;
        }
        else if ( theEntry.bootyUserName ) {
            if ( (stricmp( theEntry.bootyName, "cn" ) == 0) ) {
                BOLdapBooty* pTmp = m_pList;
                m_pList = &theEntry;
                m_pList->next = pTmp;
            }
            else {
                BOLdapBooty* pTmp = m_pList->next;
                m_pList->next = &theEntry;
                theEntry.next = pTmp;
            }
        }
        else {
            m_pTail->next = &theEntry;
            m_pTail = &theEntry;
        }

        m_nEntries++;
        return true;
    }

    // yer basic, minimum requirement of iterators
    BOLdapBooty* FirstEntry()
        { return m_pList; }
    BOLdapBooty* NextEntry( BOLdapBooty* pPrev )
        { return pPrev->next; }

    // doesn't really "purge" the list, just sets everything nil so that
    // the list looks empty when the next insert happens.
    void Purge( void ) {
        m_nEntries = 0;
        m_pList = m_pTail = nil;
    }
};


///////////////////////////////////////////////////////////////////////////////
// BOLdapQuery class


BOLdapQuery::BOLdapQuery(IUnknown *punkParent)
    : m_cRef(1)
{
    m_pLD              = NULL;
    m_ldMsgID          = 0;
    m_pldMsg           = NULL;
    m_bConnected       = 0;
    m_pServer          = NULL;
    m_pISchedule       = NULL;
    m_dwFlags          = 0;
    m_nField           = 0;
    m_pfnQueryCB       = NULL;
    m_pRecord          = NULL;
    m_nQueryStatus     = QUERY_OK;
    m_pszDatabaseID    = NULL;
    m_tTimeStarted     = 0;
    m_tTimeOut         = 0;
    m_nFound           = 0;
    m_nLimit           = 0;

    // get our private configuration interface
    HRESULT hr =
        punkParent->QueryInterface( IID_ExistConfig, (LPVOID*)&m_pServer );

    assert( SUCCEEDED(hr) );
    InterlockedIncrement( (long*)&g_dwComponentCount );
    CreateSocket();
    m_pTheBooty = DEBUG_NEW_NOTHROW BOLdapBootyList;
}


BOLdapQuery::~BOLdapQuery()
{
    if (m_pServer)
        m_pServer->Release();

    // never did care much for schedules!
    if (m_pISchedule) {
        m_pISchedule->Cancel();
        m_pISchedule->Release();
    }

	delete [] m_pszDatabaseID;

    // if there is a leftover record, release it
    if (m_pRecord)
        m_pRecord->Release();

    // cut umich loose
    end_ldap_session( m_pLD );
    InterlockedDecrement( (long*)&g_dwComponentCount );
}

///////////////////////////////////////////////////////////////////////////////
// IUnknown

STDMETHODIMP BOLdapQuery::QueryInterface(const IID& iid, void** ppv)
{
    if (iid == IID_IUnknown)
        *ppv = (IUnknown*)this;
    else
        if (iid == IID_IDSPQuery)
            *ppv = (IDSPQuery*)this;
        else{
            *ppv = NULL;
            return E_NOINTERFACE;
        }

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) BOLdapQuery::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) BOLdapQuery::Release()
{
    if (InterlockedDecrement(&m_cRef) == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}


///////////////////////////////////////////////////////////////////////////////
// Non-interface methods

// BOG: CreateSocket is a legacy name (from finger query) and should probably
// be something else. we don't worry 'bout sockets here. InitScheduling?
HRESULT BOLdapQuery::CreateSocket()
{
    HRESULT hr;

    if (SUCCEEDED(hr = CoCreateInstance(CLSID_ISchedule,
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_ISchedule,
                                        (LPVOID *)&m_pISchedule))) {

        return hr;
    }

    m_nQueryStatus = QUERY_EPROTOCOL;
    return hr;
}


// BOG: this routine can block badly (long time) and it doesn't appear there's
// a whole lot we can do about it. the scenario that i've seen is when a server
// with a valid dns doesn't respond.

BOOL BOLdapQuery::Connect()
{

    BOOL bRet = FALSE;
    m_bConnected = FALSE;

    assert(m_pServer);
    Server_t* pSP = NULL;
    m_pServer->GetServerProperties( &pSP );

	CString password;
	if( pSP->LogonRequired && ( !pSP->AccountPassword || !strcmp(pSP->AccountPassword, "") ) ) {
		
		GetPassword(password);

		delete [] pSP->AccountPassword;
		pSP->AccountPassword = NULL;
		pSP->AccountPassword = new char[password.GetLength()+1];
		lstrcpy( pSP->AccountPassword, (LPCTSTR)password );

	}

    m_ldMsgID = start_ldap_session( pSP->DomainName, pSP->Port, &m_pLD, pSP->AccountName,
									pSP->AccountPassword, (pSP->LogonRequired != FALSE),
									(pSP->SSLRequired != FALSE), pSP->SearchBase );

    if ( m_pLD && (m_ldMsgID == LDAP_SUCCESS) ) {
        m_nQueryStatus = QUERY_WOULDBLOCK;
        bRet = TRUE;
    }
    else {

		if (m_ldMsgID == LDAP_INVALID_CREDENTIALS) {
			m_nQueryStatus = QUERY_INVALID_CREDENTIALS;	
			delete [] pSP->AccountPassword;
			pSP->AccountPassword = NULL;
		} else if (m_ldMsgID == LDAP_UNWILLING_TO_PERFORM) {
			m_nQueryStatus = QUERY_UNWILLING_TO_PERFORM;
		} else if (m_ldMsgID == LDAP_INVALID_DN_SYNTAX) {
			m_nQueryStatus = QUERY_INVALID_DN_SYNTAX;
		} else if (m_ldMsgID == LDAP_SERVER_DOWN) {
			m_nQueryStatus = QUERY_SERVER_DOWN;
		} else {
			m_nQueryStatus = QUERY_ECONNECT;
		}
        
	}
        
    return bRet;
}


BOOL BOLdapQuery::CreateRecord()
{
    BOOL bRet = FALSE;
    assert( m_pRecord == NULL );

    if ( m_pRecord ) {
        m_pRecord->Release();
        m_pRecord = NULL;
    }

    m_pRecord = DEBUG_NEW_NOTHROW CPHRecord();

    if ( m_pRecord ) {
        Server_t* pSP = NULL;
        m_pServer->GetServerProperties( &pSP );
        m_pRecord->SetDatabaseID( pSP->szUID );
        m_nQueryStatus = QUERY_WOULDBLOCK;
        bRet = TRUE;
    }
    else {
        m_nQueryStatus = QUERY_EOUTOFOMEMORY;
    }

    return bRet;
}



bool BOLdapQuery::BuildRecord()
{
    bool bRet = false;
    assert( m_pldMsg );
    assert( m_pRecord );
    assert( m_pServer );

    if ( m_pldMsg && m_pRecord ) {
        ValueList vlst;
        DS_TYPE fieldType;
        DS_FIELD fieldID;

        BerElement* pBer = NULL;
        char* pAttr = ldap_first_attribute( m_pLD, m_pldMsg, &pBer );

        while ( pAttr ) {
            char* userdefName = NULL;
            if ( m_pServer->MapAttribName( pAttr, &fieldID, &fieldType,
                                           &userdefName ) == S_OK ) {

                vlst = ldap_get_values( m_pLD, m_pldMsg, pAttr );

                if ( vlst ) {
                    m_pTheBooty->InsertBooty(
                        pAttr,                   // ldap attribute name
                        fieldID,                 // type of attribute
                        vlst,                    // attribute value(s)
                        userdefName              // user specified name
                        );
                }
            }

            // pAttr points to temp mem, and pBer will be freed automatically
            // when ldap_next_attribute returns nil (last call).
            pAttr = ldap_next_attribute( m_pLD, m_pldMsg, pBer );
        }
    }

    // the attributes were sorted in the order we wanted as they were inserted
    // into the list; now we just first/next 'em into the record object. Note
    // that there can be multiple values for each attribute.

    BOLdapBooty* pBooty = m_pTheBooty->FirstEntry();

    for ( ; pBooty; pBooty = m_pTheBooty->NextEntry( pBooty ) ) {
        char* pDisplayName =
            pBooty->bootyUserName ? pBooty->bootyUserName : pBooty->bootyName;

	// the first value is added to the record with its display name; any
	// additional values are added, instead, with a colon, creating a
	// vertical list effect in the ui.
	//
	// e.g.   email: bogdon@qualcomm.com
	//             : bjammin@cts.com

        m_pRecord->AddStringRecEnt(
            pBooty->bootyValue[0], pBooty->bootyType, pDisplayName );

        for ( int i = 1; pBooty->bootyValue[i]; i++ ) {
            m_pRecord->AddStringRecEnt( pBooty->bootyValue[i],
                                        pBooty->bootyType,
                                        ":" );
        }

        if ( pBooty->bootyValue )
            ldap_value_free( pBooty->bootyValue );

        if ( pBooty->bootyUserName )
            delete [] pBooty->bootyUserName;

        bRet = true;
    }

    m_pTheBooty->Purge();
    return bRet;
}


void BOLdapQuery::ScheduleCallback()
{
    m_pISchedule->Schedule(ISOCKCALLBACK(Callback), this);
}


void BOLdapQuery::Callback()
{
    assert(m_pfnQueryCB);

    // if we're in the ol' "wouldblock" state, it means were in the middle
    // of waiting for a connection or record and need to keep waiting.

    if ( m_nQueryStatus == QUERY_WOULDBLOCK ) {
        if ( m_bConnected )
            WaitForRecord( m_pfnQueryCB, m_pContext );
        else
            Start( m_pfnQueryCB, m_pContext );
    }
    else
        m_pfnQueryCB(m_pContext);
}


void BOLdapQuery::SetDatabaseID(LPSTR pszID)
{
	delete [] m_pszDatabaseID;
    m_pszDatabaseID = NULL;

    if (pszID)
		m_pszDatabaseID = SafeStrdupMT(pszID);
}


void BOLdapQuery::SetStatus(QUERY_STATUS nStatus)
{
    m_nQueryStatus = nStatus;
    ScheduleCallback();
}


inline void BOLdapQuery::InitTimeout( time_t tIntvl /* = 0 */ )
{
    // only really hafta set the interval once
    if ( tIntvl )
        m_tTimeOut = tIntvl;

    // reset the timer
    m_tTimeStarted = time( NULL );
}


inline bool BOLdapQuery::TimedOut()
{
    if ( m_tTimeOut )
        return ((m_tTimeStarted + m_tTimeOut) <= time( NULL ));
    else
        return false;    // wait forever
}


///////////////////////////////////////////////////////////////////////////////
// IDSPQuery


STDMETHODIMP BOLdapQuery::InitQuery(LPSTR pszSearch, DWORD dwFlags,
                                     DS_FIELD nField)
{
    if (!pszSearch)
        return E_FAIL;

    assert(m_pServer);
    Server_t* pSP = NULL;
    m_pServer->GetServerProperties( &pSP );

    // the query entered by the user is used to form a ldap search filter,
    // which is what we actually search with on the server.

    build_filter_string( pszSearch, pSP->SearchFilter,
                         pSP->CNSearchFilter, m_SearchFilter );

    if ( m_pRecord ) {
        m_pRecord->Release();
        m_pRecord = NULL;
    }

    m_dwFlags         = dwFlags;
    m_nField          = nField;
    m_nQueryStatus    = QUERY_OK;

    m_nFound = 0;
    m_nLimit = pSP->ServerHitLimit;

    return S_OK;
}


STDMETHODIMP_(void) BOLdapQuery::Start(DBQUERYCB pCB, LPVOID pCtx)
{
    m_pfnQueryCB = pCB;
    m_pContext = pCtx;

    assert(m_pServer);
    Server_t* pSP = NULL;
    m_pServer->GetServerProperties( &pSP );

    // if we don't have a connection yet, we do that first. we schedule a
    // timeout for the user defined time (convert sec to msec).

    if ( !m_pLD ) {
        if ( Connect() )
            InitTimeout( pSP->ServerTimeOut );
    }
    else if ( TimedOut() ) {
        ldap_abandon( m_pLD, m_ldMsgID );
        m_nQueryStatus = QUERY_TIMEDOUT;
    }
    else {
        WaitResult wr = wait_for_connect( m_pLD, m_ldMsgID );

        if ( wr == kWaitMore ) {
            m_nQueryStatus = QUERY_WOULDBLOCK;
        }
        else if ( wr == kGrooveOnTheConnection ) {
            m_bConnected = TRUE;
            m_nQueryStatus = QUERY_OK;

            m_ldMsgID = ldap_search( m_pLD,                  // ldap session
                                     pSP->SearchBase,        // base dn
                                     LDAP_SCOPE_SUBTREE,     // scope specifier
                                     _cstr_(m_SearchFilter), // search filter
                                     0,                      // all attributes
                                     0 );                    // !attrs-only

            InitTimeout();
        }
        else {
            m_nQueryStatus = QUERY_ECONNECT;
        }
    }

    ScheduleCallback();
}


STDMETHODIMP_(void) BOLdapQuery::Cancel()
{
    if (m_pISchedule)
        m_pISchedule->Cancel();

//    assert( m_pLD );
//    assert( m_ldMsgID );

    if ( m_pLD && m_ldMsgID )
        ldap_abandon( m_pLD, m_ldMsgID );

    m_nQueryStatus = QUERY_CANCELLED;
}


STDMETHODIMP_(void) BOLdapQuery::WaitForRecord(DBQUERYCB pCB, LPVOID pCtx)
{
    m_pfnQueryCB = pCB;
    m_pContext = pCtx;

    // we should never be called w/out an intervening GetRecord()
    assert( m_pRecord == NULL );

    // none of the following happen in the normal course of events, including
    // timeout conditions. so if that ol' assert fires, ya might be messing up.
    // "word" to your mother's brother's daughter's best-friend's dog!

    if ( m_pRecord  || m_nQueryStatus == QUERY_CANCELLED ||
         m_nQueryStatus == QUERY_TIMEDOUT || m_nQueryStatus == QUERY_DONE ) {

        assert( 0 );    // gotcha!
    }
    else {
        WaitResult ldr = wait_for_record( m_pLD, m_ldMsgID, &m_pldMsg );

        if ( ldr == kWaitMore ) {
            if ( TimedOut() ) {
                ldap_abandon( m_pLD, m_ldMsgID );
                m_nQueryStatus = QUERY_TIMEDOUT;
            }
            else
                m_nQueryStatus = QUERY_WOULDBLOCK;
        }
        else
            m_nQueryStatus = QUERY_OK;
    }

    ScheduleCallback();
}


STDMETHODIMP_(QUERY_STATUS) BOLdapQuery::GetRecord(IDSPRecord **ppRecord)
{
    QUERY_STATUS nStatus = QUERY_FAILED;

    if ( (m_nQueryStatus == QUERY_CANCELLED) || 
         (m_nQueryStatus == QUERY_WOULDBLOCK) || 
         (m_nQueryStatus == QUERY_DONE) ||
         (m_nQueryStatus == QUERY_TIMEDOUT) ) {

        nStatus =  m_nQueryStatus;
    }
    else {
        CreateRecord();    

        if ( ppRecord && m_pRecord ) {
            bool bMaxxedOut = m_nLimit ? !(m_nFound < m_nLimit) : false;

            if ( !bMaxxedOut && BuildRecord() ) {

                // return the IDSPRecord interface
                m_pRecord->QueryInterface(IID_IDSPRecord, (void**)ppRecord);
                m_pRecord->Release();    // release our pointer to object
                m_pRecord = NULL;

                if ( *ppRecord ) {
                    m_nFound++;
                    nStatus = QUERY_OK;
                }
            }
            else {
                if ( bMaxxedOut )
                    ldap_abandon( m_pLD, m_ldMsgID );

                m_nQueryStatus = QUERY_DONE;
            }
        }
    }

    return nStatus;
}


STDMETHODIMP_(QUERY_STATUS) BOLdapQuery::GetLastError()
{
    return m_nQueryStatus;
}


STDMETHODIMP BOLdapQuery::GetErrorString(QUERY_STATUS /*nCode*/,
                                          LPSTR /*pszBuffer*/, int /*nlen*/)
{
    return E_FAIL;
}


STDMETHODIMP BOLdapQuery::SetSearchBase(LPSTR /*pszSearchBase*/)
{ 
    return S_OK;    // this is really dorky. it's part of our own config!!
                    // why do i need to provide a protocol for it?
                    // why not have methods for every config item?
                    // I don't get it. smells like pasta to me.
}


///////////////////////////////////////////////////////////////////////////////
// umich ldap stuff

// this is verbatim from "lfind" [Linux], the test stub used to prototype all
// of the umich-based functionality. this is only temporary. i'm gonna rework
// this stuff as a STL string kind of a deal, and these hacks will go the
// way of the dodo. "lfind" will always be the reference impl.


int start_ldap_session( const char* server, int port,  LDAP** ppLd, const char* user, const char* password, bool bLogonReqd, bool bSSLReqd, const char* searchBase )
{
 
    LDAP* pLd = NULL;
	int rc = -1;
	int ldap_version = LDAP_VERSION3;
	int x = LDAP_OPT_X_TLS_HARD;
	int y = LDAP_OPT_X_TLS_NEVER;


	//logging code
	/*
	int    iDebugLevel = 127;
	FILE   *LdapLogFile;

	LdapLogFile = fopen("ldap.log", "a+");

	ber_set_option(NULL, 
		            LBER_OPT_LOG_PRINT_FILE, 
			        LdapLogFile);

	ldap_set_option(NULL, 
		           LDAP_OPT_DEBUG_LEVEL, 
			       &iDebugLevel);*/


    // init the basic ldap object
    pLd = ldap_init( server, port );

	if (pLd) {

		rc = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);

		if (rc != LDAP_SUCCESS) 
			return rc;

		if(bSSLReqd) {
	
			rc = ldap_set_option(pLd, LDAP_OPT_X_TLS, &x);
	
			if (rc != LDAP_SUCCESS)
				return rc;

			//apparently the server needs to be pointed to any dummy certificate and then to the directory for the API to work
			char szPath[ 256 ];
			GetModuleFileName(NULL, szPath, sizeof(szPath) );
			GetShortPathName(szPath,szPath, sizeof(szPath));
			const char* pSlash = strrchr( szPath, '\\' );
			if( pSlash) {
				
				char szCertPath[1024];
				char szCertDirPath[1024];
				char szRootCertFile[80];
				char szRootCertDir[80];

				int index = (int)(pSlash - szPath + 1);

				strncpy( szCertPath, szPath, index) ;
				szCertPath[index] = '\0';		
				LoadString( g_hInstance, IDS_ROOT_CERT_FILE, szRootCertFile, sizeof(szRootCertFile)-1 );
				strcat( szCertPath, szRootCertFile);
				
				strncpy( szCertDirPath, szPath, index) ;
				szCertDirPath[index] = '\0';
				LoadString( g_hInstance, IDS_ROOT_CERT_DIR, szRootCertDir, sizeof(szRootCertDir)-1 );
				strcat( szCertDirPath, szRootCertDir);

				rc = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE, szCertPath);
				rc = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTDIR, szCertDirPath);

			}
	
			if(rc != LDAP_SUCCESS)
				return rc;

			rc = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &y);
	
			if(rc != LDAP_SUCCESS) 
				return rc;

		}

		if (bLogonReqd) {
		
			char* dn = DEBUG_NEW_NOTHROW char[1024];
			strcpy(dn, "uid=");
			strcat(dn, user);
			strcat(dn, ",");
			strcat(dn, searchBase);
		
			// bind to the server
			rc = ldap_simple_bind_s(pLd, dn, password);

		} else {

			// bind to the server
			rc = ldap_simple_bind_s(pLd, "", "");

		}

	}

	if(rc == LDAP_SUCCESS) 
		*ppLd = pLd;

	return rc;
}


void end_ldap_session( LDAP* pLd )
{
    if ( pLd )
        ldap_unbind_s( pLd );
}


// BOG: need to improve this code so that it can report the difference
// between a failure and "no record found".
WaitResult wait_for_record( LDAP* pLd, int msgID,  LDAPMessage** ppRes )
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    WaitResult retVal;
    int result = ldap_result( pLd, msgID, 0, &tv, ppRes );

    if ( !result )
        retVal = kWaitMore;
    else
        retVal = kGrooveOnTheRecord;

    return retVal;
}


WaitResult wait_for_connect( LDAP* pLd, int msgID )
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    WaitResult retVal;
    LDAPMessage* pRes = NULL;
    ldap_result( pLd, msgID, 0, &tv, &pRes );

	switch (msgID) {
		case LDAP_SUCCESS: 
			retVal = kGrooveOnTheConnection;
			break;
		case  LDAP_TIMELIMIT_EXCEEDED:
			retVal = kWaitMore;
			break;
		default:
			retVal = kNonSpecificBummer;
			break;
	}

    return retVal;
}


// Here's some old, retired code that shows what a search filter template
// looks like. So if the user typed "bob", we would substitute "bob" for all
// the "^0"s. The "|" symbol means whatcha think.
//
//    const char* pDefTmpl =
//        "(|(givenname=*^0*)(sn=*^0*)(gn=*^0*)(cn=*^0*))";

void build_filter_string( const char* searchTerms, const char* pSearchFilter,
                          const char* pCNSearchFilter, string& filter )
{
    string tmpFilter;
    char* p1, * p2;
    p1 = p2 = (char*) searchTerms;

    // our "LDAP_SEARCH_FILTER" is used to form a term from a word. a filter is
    // made for each word typed by the user, and they are all ANDed together.

    for( ;; ) {
        if ( *p1 == ',' || *p1 == ' ' || *p1 == '\0' ) {
            string boodgie, kloodgie, term;
            term.assign( p2, p1 - p2 );
            make_filter( term.c_str(), pSearchFilter, boodgie );

            if ( tmpFilter.empty() ) {
                tmpFilter = boodgie;
            }
            else {
                AND_filters( tmpFilter.c_str(), boodgie.c_str(), kloodgie );
                tmpFilter = kloodgie;
            }

            while ( (*p1 == ',' || *p1 == ' ') && *p1 != '\0' )
                p1++;

            if ( *p1 == '\0' )
                break;

            p2 = p1;
            continue;
        }

        p1++;
    }

    // the "LDAP_CN_SEARCH_FILTER" is used to form a term from the whole query
    // (as typed by the user). it is ORed with the "LDAP_SEARCH_FILTER".

    if ( pCNSearchFilter && strlen( pCNSearchFilter ) ) {
        string strTmp, strTmp2;
        make_filter( searchTerms, pCNSearchFilter, strTmp );
        OR_filters(  tmpFilter.c_str(), strTmp.c_str(), strTmp2 );
        tmpFilter = strTmp2;
    }

    // user-entered wildcards may double-up with ones in the template; we
    // colapse these down to one. note that one could probably write this to be
    // slightly faster by using two iters, and avoiding .erase(). whatever.

    for ( string::iterator it = tmpFilter.begin(); it != tmpFilter.end();) {
        if ( *it == '*' && *(it + 1) == '*' ) {
            it = tmpFilter.erase( it );
            continue;
        }

        it++;
    }

    filter = tmpFilter;
}


void make_filter( const char* searchTerm, const char* tmpl, string& filter )
{
    string booger = tmpl;
    string::iterator it, woody;
    woody = it = booger.begin();

    while ( it != booger.end() ) {
        if ( *it == '^' ) {
            filter.append( woody, it );
            filter.append( searchTerm );
            it += 2;
            woody = it;
            continue;
        }

        it++;
    }

    filter.append( woody, it );
}


void AND_filters( const char* p1, const char* p2, string& filter )
{
    filter = "(&";
    filter += p1;
    filter += p2;
    filter += ")";
}


void OR_filters( const char* p1, const char* p2, string& filter )
{
    filter = "(|";
    filter += p1;
    filter += p2;
    filter += ")";
}

