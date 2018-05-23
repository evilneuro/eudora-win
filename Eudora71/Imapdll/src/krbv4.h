// KRBV4.H - 
// 

#ifndef __KRBV4_H
#define __KRBV4_H

//============ DEFS from kerberos V4 stuff ===================//

// From des.h (JOK)

// There's a good chance this header will need to be updated if
// we implement more than just K_V4 authentication for IMAP.  In
// particular, the structures seem incompatible with KClient and
// newer PC-LeLands.
//
// -jdboyd 8/16/99

#ifndef KRB_INT
#define KRB_INT short
#endif

#ifndef KRB_INT32
#define KRB_INT32 long
#endif
#ifndef KRB_UINT32
#define KRB_UINT32 unsigned KRB_INT32
#endif

typedef unsigned char des_cblock[8];	/* crypto-block size */

#define C_Block des_cblock

typedef struct des_ks_struct { union { KRB_INT32 pad; des_cblock _;} __; } des_key_schedule[16];

#define DES_KEY_SZ 	(sizeof(des_cblock))
#define DES_ENCRYPT	1
#define DES_DECRYPT	0

#define Key_schedule des_key_schedule

// Declarations:
//


// end JOK




/* General definitions */
#define		KSUCCESS	0
#define		KFAILURE	255

/* The maximum sizes for aname, realm, sname, and instance +1 */
#define 	ANAME_SZ	40
#define		REALM_SZ	40
#define		SNAME_SZ	40
#define		INST_SZ		40

/* include space for '.' and '@' */
#define		MAX_K_NAME_SZ	(ANAME_SZ + INST_SZ + REALM_SZ + 2)
#define		KKEY_SZ		100
#define		VERSION_SZ	1
#define		MSG_TYPE_SZ	1
#define		DATE_SZ		26	/* RTI date output */

#ifndef DEFAULT_TKT_LIFE		/* allow compile-time override */
#define		DEFAULT_TKT_LIFE	120 /* default lifetime 10 hrs */
#endif

/* Definition of text structure used to pass text around */
#define		MAX_KTXT_LEN	1250

// Older versions of PC-LeLand expect this ticket structure: -jdboyd 8/16/99
struct ktext {
    KRB_INT  length;		/* Length of the text */ 
    unsigned char dat[MAX_KTXT_LEN];	/* The data itself */
    unsigned long mbz;		/* zero to catch runaway strings */
};

typedef struct ktext *KTEXT;
typedef struct ktext KTEXT_ST;

// KClient, and newer PC-LeLands, use this newer structure: 
struct newktext {
    KRB_INT32  length;		/* Length of the text */ 
    unsigned char dat[MAX_KTXT_LEN];	/* The data itself */
    unsigned long mbz;		/* zero to catch runaway strings */
};

typedef struct newktext *NEWKTEXT;
typedef struct newktext NEWKTEXT_ST;

/* Definitions for send_to_kdc */
#define	CLIENT_KRB_TIMEOUT	4	/* time between retries */
#define CLIENT_KRB_RETRY	5	/* retry this many times */
#define	CLIENT_KRB_BUFLEN	512	/* max unfragmented packet */

/* Definitions for ticket file utilities */
#define	R_TKT_FIL	0
#define	W_TKT_FIL	1

/* Structure definition for rd_ap_req */

struct auth_dat {
    unsigned char k_flags;	/* Flags from ticket */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];	/* His Instance */
    char    prealm[REALM_SZ];	/* His Realm */
    unsigned KRB_INT32 checksum; /* Data checksum (opt) */
    C_Block session;		/* Session Key */
    KRB_INT  life;		/* Life of ticket */
    unsigned KRB_INT32 time_sec; /* Time ticket issued */
    unsigned KRB_INT32 address;	/* Address in ticket */
    KTEXT_ST reply;		/* Auth reply (opt) */
};

typedef struct auth_dat AUTH_DAT;

/* Structure definition for credentials returned by get_cred */

struct credentials {
    char    service[ANAME_SZ];	/* Service name */
    char    instance[INST_SZ];	/* Instance */
    char    realm[REALM_SZ];	/* Auth domain */
    C_Block session;		/* Session key */
    int lifetime;		/* Lifetime */
    int kvno;		/* Key version number */
    NEWKTEXT_ST ticket_st;		/* The ticket itself */
    long    issue_date;		/* The issue time */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];	/* Principal's instance */
};

typedef struct credentials CREDENTIALS;

/* Structure definition for rd_private_msg and rd_safe_msg */

struct msg_dat {
    unsigned char *app_data;	/* pointer to appl data */
    unsigned KRB_INT32 app_length; /* length of appl data */
    unsigned long hash;	/* hash to lookup replay */
    KRB_INT  swap;		/* swap bytes? */
    KRB_INT32    time_sec;	/* msg timestamp seconds */
    unsigned char time_5ms;	/* msg timestamp 5ms units */
};

typedef struct msg_dat MSG_DAT;


/* Location of default ticket file for save_cred and get_cred */
#ifndef	TKT_FILE
#define TKT_FILE        ((char *)0)
#endif /* TKT_FILE */

/* Error codes returned from the KDC */
#define		KDC_OK		0	/* Request OK */
#define		KDC_NAME_EXP	1	/* Principal expired */
#define		KDC_SERVICE_EXP	2	/* Service expired */
#define		KDC_AUTH_EXP	3	/* Auth expired */
#define		KDC_PKT_VER	4	/* Protocol version unknown */
#define		KDC_P_MKEY_VER	5	/* Wrong master key version */
#define		KDC_S_MKEY_VER 	6	/* Wrong master key version */
#define		KDC_BYTE_ORDER	7	/* Byte order unknown */
#define		KDC_PR_UNKNOWN	8	/* Principal unknown */
#define		KDC_PR_N_UNIQUE 9	/* Principal not unique */
#define		KDC_NULL_KEY   10	/* Principal has null key */
#define		KDC_GEN_ERR    20	/* Generic error from KDC */


/* Values returned by get_credentials */
#define		GC_OK		0	/* Retrieve OK */
#define		RET_OK		0	/* Retrieve OK */
#define		GC_TKFIL       21	/* Can't read ticket file */
#define		RET_TKFIL      21	/* Can't read ticket file */
#define		GC_NOTKT       22	/* Can't find ticket or TGT */
#define		RET_NOTKT      22	/* Can't find ticket or TGT */


/* Values returned by mk_ap_req	 */
#define		MK_AP_OK	0	/* Success */
#define		MK_AP_TGTEXP   26	/* TGT Expired */

/* Values returned by rd_ap_req */
#define		RD_AP_OK	0	/* Request authentic */
#define		RD_AP_UNDEC    31	/* Can't decode authenticator */
#define		RD_AP_EXP      32	/* Ticket expired */
#define		RD_AP_NYV      33	/* Ticket not yet valid */
#define		RD_AP_REPEAT   34	/* Repeated request */
#define		RD_AP_NOT_US   35	/* The ticket isn't for us */
#define		RD_AP_INCON    36	/* Request is inconsistent */
#define		RD_AP_TIME     37	/* delta_t too big */
#define		RD_AP_BADD     38	/* Incorrect net address */
#define		RD_AP_VERSION  39	/* protocol version mismatch */
#define		RD_AP_MSG_TYPE 40	/* invalid msg type */
#define		RD_AP_MODIFIED 41	/* message stream modified */
#define		RD_AP_ORDER    42	/* message out of order */
#define		RD_AP_UNAUTHOR 43	/* unauthorized request */

/* Values returned by get_pw_tkt */
#define		GT_PW_OK	0	/* Got password changing tkt */
#define		GT_PW_NULL     51	/* Current PW is null */
#define		GT_PW_BADPW    52	/* Incorrect current password */
#define		GT_PW_PROT     53	/* Protocol Error */
#define		GT_PW_KDCERR   54	/* Error returned by KDC */
#define		GT_PW_NULLTKT  55	/* Null tkt returned by KDC */


/* Values returned by send_to_kdc */
#define		SKDC_OK		0	/* Response received */
#define		SKDC_RETRY     56	/* Retry count exceeded */
#define		SKDC_CANT      57	/* Can't send request */

/*
 * Values returned by get_intkt
 * (can also return SKDC_* and KDC errors)
 */

#define		INTK_OK		0	/* Ticket obtained */
#define		INTK_PW_NULL   51	/* Current PW is null */
#define		INTK_W_NOTALL  61	/* Not ALL tickets returned */
#define		INTK_BADPW     62	/* Incorrect password */
#define		INTK_PROT      63	/* Protocol Error */
#define		INTK_ERR       70	/* Other error */

/* Values returned by get_adtkt */
#define         AD_OK           0	/* Ticket Obtained */
#define         AD_NOTGT       71	/* Don't have tgt */

/* Error codes returned by ticket file utilities */
#define		NO_TKT_FIL	76	/* No ticket file found */
#define		TKT_FIL_ACC	77	/* Couldn't access tkt file */
#define		TKT_FIL_LCK	78	/* Couldn't lock ticket file */
#define		TKT_FIL_FMT	79	/* Bad ticket file format */
#define		TKT_FIL_INI	80	/* tf_init not called first */

/* Error code returned by kparse_name */
#define		KNAME_FMT	81	/* Bad Kerberos name format */

/* Error code returned by krb_mk_safe */
#define		SAFE_PRIV_ERROR	-1	/* syscall error */


//========================================================================//

//======= Entry point from SASL mechanism =================//

long KrbV4Authenticator (authchallenge_t challenger,
			      authrespond_t responder, NETMBX *mb, void *s,
			      unsigned long *trial, char *user);


typedef	long (CALLBACK KRB_MK_PRIV)  (unsigned char* in, unsigned char* out,
				unsigned long in_length , Key_schedule schedule,
				C_Block *key, struct sockaddr_in *sender,
				struct sockaddr_in *receiver);

typedef long (CALLBACK KRB_RD_PRIV) (unsigned char *in, unsigned long in_length,
				Key_schedule schedule, C_Block *key, struct sockaddr_in *sender,
				struct sockaddr_in *receiver, MSG_DAT *msg_data);

typedef	KRB_INT  (CALLBACK KRB_MK_REQ) (KTEXT authent, char *service, char *instance,
				char *realm, unsigned long checksum);

typedef	KRB_INT	 (CALLBACK KRB_RD_REQ) (KTEXT authent, char *service, char *instance, unsigned long from_addr,
					AUTH_DAT *ad, char *fn);

typedef	char * (CALLBACK KRB_REALMOFHOST) (char *host);

typedef KRB_INT (CALLBACK KRB_GET_CRED) (char *service, char *instance, char *realm,
					CREDENTIALS *c);

typedef const char* (CALLBACK KRB_GET_ERR_TEXT) (KRB_INT result);


// DES
typedef	KRB_INT (CALLBACK DES_KEY_SCHED) (des_cblock block, des_key_schedule keyshed);

typedef	KRB_INT (CALLBACK DES_ECB_ENCRYPT) (des_cblock *block1, des_cblock *block2, des_key_schedule keyshed, int len);

typedef	KRB_INT (CALLBACK DES_PCBC_ENCRYPT) (des_cblock *block1, des_cblock *block2, long uLen,
			     des_key_schedule keyshed, des_cblock *block3, int len);


// Forward class decs.
//
class CProtocol;

class CKrb4
{
public:
	CKrb4 (authchallenge_t challenger,
		  authrespond_t responder, LPCSTR pFQDN, CProtocol *s,
		  LPCSTR pUser);

	~CKrb4();


	long Authenticate ();


	long krb_mk_priv  (unsigned char* in, unsigned char* out,
				unsigned long in_length , Key_schedule schedule,
				C_Block *key, struct sockaddr_in *sender,
				struct sockaddr_in *receiver);


	long krb_rd_priv (unsigned char *in, unsigned long in_length,
			Key_schedule schedule, C_Block *key, struct sockaddr_in *sender,
			struct sockaddr_in *receiver, MSG_DAT *msg_data);


	int krb_mk_req (KTEXT authent, char *service, char *instance,
					char *realm, unsigned long checksum);

	int krb_rd_req (KTEXT authent, char *service, char *instance, unsigned long from_addr,
					AUTH_DAT *ad, char *fn);

	char* krb_realmofhost (char *host);

	int krb_get_cred (char *service, char *instance, char *realm,
			CREDENTIALS *c);

	const char * krb_get_err_text (KRB_INT result);


	// DES:
	KRB_INT des_key_sched (des_cblock block, des_key_schedule keyshed);

	KRB_INT des_ecb_encrypt (des_cblock *block1, des_cblock *block2, des_key_schedule keyshed, KRB_INT encrypt);

	KRB_INT des_pcbc_encrypt (des_cblock *block1, des_cblock *block2, long uLen,
			     des_key_schedule keyshed, des_cblock *block3, KRB_INT encrypt);


// Internal methods:
private:
	BOOL	Initialize ();

	BOOL	Stage0Challenge (unsigned long *ulRndNum);

	BOOL	Stage1Response (unsigned long ulRndNum);

	BOOL	Stage2Challenge (unsigned long ulRndNum);

// Internal attributes.
private:
	authchallenge_t m_pChallenger;
	authrespond_t	m_pResponder;

	//
	CString			m_szService;
	CString			m_szInstance;
	CString			m_szRealm;

	CProtocol*		m_pProtocol;
	CString			m_szUser;

	// hostname.domain
	CString			m_szServerFQDN;

	// Kerberos library instance: (loaded every time??)
	//
	HINSTANCE		m_hInstKerberosLib;

	// DES library:
	//
	HINSTANCE		m_hDesLib;

// Function pointers to the kerberos DLL routines.
//
private:
	KRB_MK_PRIV*	m_pfnKrbMkPriv;
	KRB_RD_PRIV*	m_pfnKrbRdPriv;

	KRB_MK_REQ*		m_pfnKrbMkReq;
	KRB_RD_REQ*		m_pfnKrbRdReq;	

	KRB_REALMOFHOST* m_pfnKrbRealmofhost;

	KRB_GET_CRED*	m_pfnKrbGetCred;

	KRB_GET_ERR_TEXT* m_pfnKrbGetErrText;


	// DES
	DES_KEY_SCHED*		m_pfnDesKeySched;
	DES_ECB_ENCRYPT*	m_pfnDesEcbEncrypt;
	DES_PCBC_ENCRYPT*	m_pfnDesPcbcEncrypt;

};


#endif // __KRBV4_H

