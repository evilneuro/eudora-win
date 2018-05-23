// gssapi.cpp - Client authenticator for GSSAPI.
//

#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "MyTypes.h"

#include <string.h>
#include <stdio.h>

#include "resource.h"
// #include "imapmail.h"
#include "exports.h"
#include "ImapDefs.h"
#include "osdep.h"
#include "Proto.h"
#include "mm.h"

#include "gssapi.h"
#include "gssapi_generic.h"
#include "gssapi_krb5.h"
#include "krb5.h"

#define PROTOTYPE(x) x
#define KRB5_PROVIDE_PROTOTYPES


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


long GssapiAuthenticator (authchallenge_t challenger,authrespond_t responder,
			 NETMBX *mb,void *stream,unsigned long *trial,
			 char *user);


#define AUTH_GSSAPI_P_NONE 1
#define AUTH_GSSAPI_P_INTEGRITY 2
#define AUTH_GSSAPI_P_PRIVACY 4

#define AUTH_GSSAPI_C_MAXSIZE 8192

#define SERVER_LOG(x,y) syslog (LOG_ALERT,x,y)

extern char *krb5_defkeyname;	/* sneaky way to get this name */

//==================================================================//

static gss_OID_desc oids[] = {
   {10, "\052\206\110\206\367\022\001\002\001\004"},
};

gss_OID_desc * gss_nt_service_name = oids+0;



//===================================================================================//


// Wrappers around the GSSAPI32 DLL imported functions
//
typedef OM_uint32 (KRB5_CALLCONV GSS_IMPORT_NAME)
		(OM_uint32 FAR *min,			/* minor_status */
            gss_buffer_t buf,			/* input_name_buffer */
            gss_OID oid,				/* input_name_type(used to be const) */
            gss_name_t FAR *name		/* output_name */
        );



typedef OM_uint32 (KRB5_CALLCONV GSS_INIT_SEC_CONTEXT)
		(OM_uint32 FAR		*min,			/* minor_status */
            gss_cred_id_t	cred_id,		/* claimant_cred_handle */
            gss_ctx_id_t FAR * ctx_id,		/* context_handle */
            gss_name_t		name,			/* target_name */
            gss_OID			oid,			/* mech_type (used to be const) */
            OM_uint32		req_flags,		/* req_flags */
            OM_uint32		time_req,		/* time_req */
            gss_channel_bindings_t	bindings, /* input_chan_bindings */
            gss_buffer_t	buf1,			/* input_token */
            gss_OID FAR *	pOid,			/* actual_mech_type */
            gss_buffer_t	buf2,			/* output_token */
            OM_uint32 FAR * ret_flags,		/* ret_flags */
            OM_uint32 FAR *	time_rec		/* time_rec */
        );


typedef	OM_uint32 (KRB5_CALLCONV GSS_RELEASE_BUFFER)
			(OM_uint32 FAR		*min,		/* minor_status */
				gss_buffer_t	buf			/* buffer */
			);


typedef	OM_uint32  (KRB5_CALLCONV GSS_DISPLAY_STATUS)
			(OM_uint32 FAR	*min,			/* minor_status */
				OM_uint32	stat_value,		/* status_value */
				int			status_type,	/* status_type */
				gss_OID		oid,			/* mech_type (used to be const) */
				OM_uint32 FAR *msg,			/* message_context */
				gss_buffer_t status_string	/* status_string */
            );


typedef	OM_uint32 (KRB5_CALLCONV GSS_DELETE_SEC_CONTEXT)
			(OM_uint32 FAR			*min,			/* minor_status */
				gss_ctx_id_t FAR	*ctx_id,		/* context_handle */
				gss_buffer_t		output_token	/* output_token */
            );


typedef	OM_uint32 (KRB5_CALLCONV GSS_WRAP)
		(OM_uint32 FAR		*min,			/* minor_status */
			gss_ctx_id_t	ctx_id,			/* context_handle */
			int				conf_req_flag,	/* conf_req_flag */
			gss_qop_t		qop_req,		/* qop_req */
			gss_buffer_t	input_msg_buf,	/* input_message_buffer */
			int FAR *		conf_state,		/* conf_state */
			gss_buffer_t	output_msg_buf	/* output_message_buffer */
	   );


/* New for V2 */
typedef	OM_uint32 (KRB5_CALLCONV GSS_UNWRAP)
		(OM_uint32 FAR		*min,				/* minor_status */
			gss_ctx_id_t	ctx_id,				/* context_handle */
			gss_buffer_t	input_msg_buf,		/* input_message_buffer */
			gss_buffer_t	output_msg_buf,		/* output_message_buffer */
			int FAR			*conf_state,		/* conf_state */
			gss_qop_t FAR	*qop_state			/* qop_state */
	   );

typedef	OM_uint32 (KRB5_CALLCONV GSS_RELEASE_NAME)
		(OM_uint32 FAR			*min,		/* minor_status */
            gss_name_t FAR		*input_name	/* input_name */
        );





//======================================================================================//


//=====================================================================//


class CGssapi
{
public:
	CGssapi (authchallenge_t challenger,
		  authrespond_t responder, LPCSTR pHost, CProtocol* s,
		  LPCSTR pUser);

	~CGssapi();


	long Authenticate ();


// Internal methods:
private:
	BOOL	Initialize ();

// Internal attributes.
private:
	authchallenge_t m_pChallenger;
	authrespond_t	m_pResponder;

	//
	CProtocol*		m_pProtocol;
	CString			m_szUser;

	// hostname.domain
	CString			m_szHost;

	CString			m_szService;

	// Kerberos library instance: (loaded every time??)
	//
	HINSTANCE		m_hInstGssapiLib;


public:
	OM_uint32 gss_import_name
		(OM_uint32 FAR *min,			/* minor_status */
            gss_buffer_t buf,			/* input_name_buffer */
            gss_OID oid,				/* input_name_type(used to be const) */
            gss_name_t FAR *name		/* output_name */
        );

	OM_uint32 gss_init_sec_context
		(OM_uint32 FAR		*min,			/* minor_status */
            gss_cred_id_t	cred_id,		/* claimant_cred_handle */
            gss_ctx_id_t FAR * ctx_id,		/* context_handle */
            gss_name_t		name,			/* target_name */
            gss_OID			oid,			/* mech_type (used to be const) */
            OM_uint32		req_flags,		/* req_flags */
            OM_uint32		time_req,		/* time_req */
            gss_channel_bindings_t	bindings, /* input_chan_bindings */
            gss_buffer_t	buf1,			/* input_token */
            gss_OID FAR *	pOid,			/* actual_mech_type */
            gss_buffer_t	buf2,			/* output_token */
            OM_uint32 FAR * ret_flags,		/* ret_flags */
            OM_uint32 FAR *	time_rec		/* time_rec */
           );


	OM_uint32  gss_release_buffer
			(OM_uint32 FAR		*min,		/* minor_status */
				gss_buffer_t	buf			/* buffer */
			);


	OM_uint32  gss_display_status
			(OM_uint32 FAR	*min,			/* minor_status */
				OM_uint32	stat_value,		/* status_value */
				int			status_type,	/* status_type */
				gss_OID		oid,			/* mech_type (used to be const) */
				OM_uint32 FAR *msg,			/* message_context */
				gss_buffer_t status_string	/* status_string */
            );


	OM_uint32  gss_delete_sec_context
			(OM_uint32 FAR			*min,			/* minor_status */
				gss_ctx_id_t FAR	*ctx_id,		/* context_handle */
				gss_buffer_t		output_token	/* output_token */
            );


	OM_uint32  gss_wrap
		(OM_uint32 FAR		*min,			/* minor_status */
			gss_ctx_id_t	ctx_id,			/* context_handle */
			int				conf_req_flag,	/* conf_req_flag */
			gss_qop_t		qop_req,		/* qop_req */
			gss_buffer_t	input_msg_buf,	/* input_message_buffer */
			int FAR *		conf_state,		/* conf_state */
			gss_buffer_t	output_msg_buf	/* output_message_buffer */
	   );


/* New for V2 */
	OM_uint32  gss_unwrap
		(OM_uint32 FAR		*min,				/* minor_status */
			gss_ctx_id_t	ctx_id,				/* context_handle */
			gss_buffer_t	input_msg_buf,		/* input_message_buffer */
			gss_buffer_t	output_msg_buf,		/* output_message_buffer */
			int FAR			*conf_state,		/* conf_state */
			gss_qop_t FAR	*qop_state			/* qop_state */
	   );

	OM_uint32  gss_release_name
		(OM_uint32 FAR			*min,		/* minor_status */
            gss_name_t FAR		*input_name	/* input_name */
        );




// Function pointers to the kerberos DLL routines.
//
private:
		GSS_IMPORT_NAME			*m_pfnGssImportName;

		GSS_INIT_SEC_CONTEXT	*m_pfnGssInitSecContext;

		GSS_RELEASE_BUFFER		*m_pfnGssReleaseBuffer;

		GSS_DISPLAY_STATUS		*m_pfnGssDisplayStatus;

		GSS_DELETE_SEC_CONTEXT	*m_pfnGssDeleteSecContext;

		GSS_WRAP				*m_pfnGssWrap;	

		GSS_UNWRAP				*m_pfnGssUnwrap;

		GSS_RELEASE_NAME		*m_pfnGssReleaseName;
};





//================================================================================//




/* Client authenticator
 * Accepts: challenger function
 *	    responder function
 *	    parsed network mailbox structure
 *	    stream argument for functions
 *	    pointer to current trial count
 *	    returned user name
 * Returns: T if success, NIL otherwise, number of trials incremented if retry
 */



long GssapiAuthenticator (authchallenge_t challenger,
			      authrespond_t responder, void *s,
			      unsigned long *trial, char *user)
{
	long lResult = 0;

	// Sanity:
	//
	if (! (challenger && responder && s && user && trial) )
	{
		return 0;
	}

	CProtocol *pProto = (CProtocol *) s;
	TCHAR tmp[MAILTMPLEN];

	if (pProto)
	{
		//
		// JOK - Call back to upper layers. to get user.
		//
		pProto->mm_login (user, tmp, *trial);

		CString szHost; pProto->GetCanonicalHostname (szHost);

		//
		*trial = 0;			/* never retry */

		// Instantiate a CGssapi to do all the work.
		//
		CGssapi gssAuth (challenger, responder, szHost, pProto, user);

		lResult = gssAuth.Authenticate();
	}

	return lResult;

}











// =========== class CKrb4 implementation ==============================//

CGssapi::CGssapi (authchallenge_t challenger,
			  authrespond_t responder, LPCSTR pHost, CProtocol* s,
			  LPCSTR pUser)

			  : m_pChallenger (challenger),
			    m_pResponder (responder),
				m_szHost (pHost),
				m_pProtocol (s),
				m_szUser (pUser)
{

	m_hInstGssapiLib = NULL;

	m_szService = CRString (IDS_IMAP_SERVICE);

}


CGssapi::~CGssapi()
{
	// Unload library every time:
	//
	if (m_hInstGssapiLib)
	{
		FreeLibrary (m_hInstGssapiLib);
		m_hInstGssapiLib = NULL;
	}

}





// Wrappers around the GSSAPI32 DLL imported functions
//
OM_uint32  CGssapi::gss_import_name
		(OM_uint32 FAR *min,			/* minor_status */
            gss_buffer_t buf,			/* input_name_buffer */
            gss_OID oid,				/* input_name_type(used to be const) */
            gss_name_t FAR *name		/* output_name */
        )
{
	return m_pfnGssImportName ? (*m_pfnGssImportName) (min, buf, oid, name) : 0;
}





OM_uint32 CGssapi::gss_init_sec_context
		(OM_uint32 FAR		*min,			/* minor_status */
            gss_cred_id_t	cred_id,		/* claimant_cred_handle */
            gss_ctx_id_t FAR * ctx_id,		/* context_handle */
            gss_name_t		name,			/* target_name */
            gss_OID			oid,			/* mech_type (used to be const) */
            OM_uint32		req_flags,		/* req_flags */
            OM_uint32		time_req,		/* time_req */
            gss_channel_bindings_t	bindings, /* input_chan_bindings */
            gss_buffer_t	buf1,			/* input_token */
            gss_OID FAR *	pOid,			/* actual_mech_type */
            gss_buffer_t	buf2,			/* output_token */
            OM_uint32 FAR * ret_flags,		/* ret_flags */
            OM_uint32 FAR *	time_rec		/* time_rec */
           )
{

	return m_pfnGssInitSecContext ? (*m_pfnGssInitSecContext) ( min,
																cred_id,
																ctx_id,
																name,
																oid,
																req_flags,
																time_req,
																bindings,
																buf1,
																pOid,
																buf2,
																ret_flags,
																time_rec) : 0;
}




OM_uint32  CGssapi::gss_release_buffer
		(OM_uint32 FAR		*min,		/* minor_status */
			gss_buffer_t	buf			/* buffer */
		)
{
	return m_pfnGssReleaseBuffer ? (*m_pfnGssReleaseBuffer) (min, buf) : 0;
}


OM_uint32 CGssapi::gss_display_status
		(OM_uint32 FAR	*min,			/* minor_status */
				OM_uint32	status_value,		/* status_value */
				int			status_type,	/* status_type */
				gss_OID		oid,			/* mech_type (used to be const) */
				OM_uint32 FAR *msg,			/* message_context */
				gss_buffer_t status_string	/* status_string */
        )
{

	return m_pfnGssDisplayStatus ? (*m_pfnGssDisplayStatus) (min,
															 status_value,
															 status_type,
															 oid,
															 msg,
															 status_string) : 0;
}


OM_uint32 CGssapi::gss_delete_sec_context
		(OM_uint32 FAR			*min,			/* minor_status */
			gss_ctx_id_t FAR	*ctx_id,		/* context_handle */
			gss_buffer_t		output_token	/* output_token */
        )
{
	
	return m_pfnGssDeleteSecContext ? (*m_pfnGssDeleteSecContext) ( min,
																	ctx_id,
																	output_token) : 0;
}


OM_uint32 CGssapi::gss_wrap
		(OM_uint32 FAR		*min,			/* minor_status */
			gss_ctx_id_t	ctx_id,			/* context_handle */
			int				conf_req_flag,	/* conf_req_flag */
			gss_qop_t		qop_req,		/* qop_req */
			gss_buffer_t	input_msg_buf,	/* input_message_buffer */
			int FAR *		conf_state,		/* conf_state */
			gss_buffer_t	output_msg_buf	/* output_message_buffer */
	   )
{

	return m_pfnGssWrap ? (*m_pfnGssWrap) ( min,
											ctx_id,
											conf_req_flag,
											qop_req,
											input_msg_buf,
											conf_state,
											output_msg_buf) : 0;
}






/* New for V2 */
OM_uint32  CGssapi::gss_unwrap
		(OM_uint32 FAR		*min,				/* minor_status */
			gss_ctx_id_t	ctx_id,				/* context_handle */
			gss_buffer_t	input_msg_buf,		/* input_message_buffer */
			gss_buffer_t	output_msg_buf,		/* output_message_buffer */
			int FAR			*conf_state,		/* conf_state */
			gss_qop_t FAR	*qop_state			/* qop_state */
	   )
{

	return m_pfnGssUnwrap ? (*m_pfnGssUnwrap) (	min, 
												ctx_id,
												input_msg_buf,
												output_msg_buf,
												conf_state,
												qop_state) : 0;

}


OM_uint32  CGssapi::gss_release_name
		(OM_uint32 FAR			*min,		/* minor_status */
            gss_name_t FAR		*input_name	/* input_name */
        )
{

	return m_pfnGssReleaseName ? (*m_pfnGssReleaseName)	 (	min,
															input_name) : 0;
}







long CGssapi::Authenticate ()
{
	long		ret = NIL;
	char		tmp[MAILTMPLEN];
	OM_uint32	maj,min,mmaj,mmin;
	OM_uint32	mctx = 0;
	gss_ctx_id_t ctx = GSS_C_NO_CONTEXT;
	gss_buffer_desc chal,resp,buf;
	gss_name_t	crname = NIL;
	long		i;
	int			conf;
	gss_qop_t	qop;

	// Make sure:
	if ( !Initialize() )
		return 0;

    // Get initial (empty) challenge */
	//
	chal.value = (*m_pChallenger)(m_pProtocol, (unsigned long *) &chal.length);

	if (chal.value)
	{
		// get initial (empty) challenge:
		//
		sprintf (tmp,"%s@%s", m_szService, m_szHost);

		buf.length = strlen ( (char *) (buf.value = tmp)) + 1;

				/* get service name */
		if (gss_import_name(&min,&buf,gss_nt_service_name,&crname)!=GSS_S_COMPLETE)
		{
			(*m_pResponder) (m_pProtocol, NIL, 0);
		}
		else
		{
			switch (maj =		/* get context */
				gss_init_sec_context (&min,GSS_C_NO_CREDENTIAL,&ctx,
				       crname,GSS_C_NO_OID,
				       GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG,
				       0,GSS_C_NO_CHANNEL_BINDINGS,
				       GSS_C_NO_BUFFER,NIL,&resp,NIL,NIL))
			{
			case GSS_S_CONTINUE_NEEDED:
				do
				{
					/* negotiate authentication */
					if (chal.value)
						fs_give ((void **) &chal.value);

					/* send response */
					i = (*m_pResponder) (m_pProtocol, (char *) resp.value, resp.length);

					gss_release_buffer (&min,&resp);
				}
				while (i &&		/* get next challenge */
					(chal.value=(*m_pChallenger)(m_pProtocol, (unsigned long *)&chal.length))&&
					(maj = gss_init_sec_context (&min,GSS_C_NO_CREDENTIAL,&ctx,
						crname,GSS_C_NO_OID,
						GSS_C_MUTUAL_FLAG|GSS_C_REPLAY_FLAG,
						0,GSS_C_NO_CHANNEL_BINDINGS,&chal,
						NIL,&resp,NIL,NIL) ==
						GSS_S_CONTINUE_NEEDED));


			case GSS_S_COMPLETE:
				if (chal.value)
				{
					fs_give ((void **) &chal.value);
					if (maj != GSS_S_COMPLETE)
						(*m_pResponder) (m_pProtocol, NIL, 0);
				}

				/* get prot mechanisms and max size */
				if ((maj == GSS_S_COMPLETE) &&
						(*m_pResponder) (m_pProtocol, resp.value ? (char *) resp.value : "", resp.length) &&
						(chal.value = (*m_pChallenger) (m_pProtocol, (unsigned long *)&chal.length))&&
						(gss_unwrap (&min,ctx,&chal,&resp,&conf,&qop) == GSS_S_COMPLETE) &&
						(resp.length >= 4) && (*((char *) resp.value) & AUTH_GSSAPI_P_NONE))
				{
					/* make copy of flags and length */
					memcpy (tmp,resp.value,4);
					gss_release_buffer (&min,&resp);

					/* no session protection */
					tmp[0] = AUTH_GSSAPI_P_NONE;

					/* install user name */
					strcpy (tmp+4, m_szUser);

					buf.value = tmp; buf.length = strlen (m_szUser) + 4;

					/* successful negotiation */
					if (gss_wrap (&min,ctx,FALSE,qop,&buf,&conf,&resp) == GSS_S_COMPLETE)
					{
						if ((*m_pResponder) (m_pProtocol, (char *) resp.value, resp.length)) ret = T;
						gss_release_buffer (&min,&resp);
					}
					else (*m_pResponder) (m_pProtocol, NIL, 0);
				}

				/* flush final challenge */
				if (chal.value) fs_give ((void **) &chal.value);

				/* don't need context any more */
				gss_delete_sec_context (&min,&ctx,NIL);

				break;

			case GSS_S_CREDENTIALS_EXPIRED:

				if (chal.value) fs_give ((void **) &chal.value);

				sprintf (tmp,"Kerberos credentials expired (try running kinit) for %s", m_szHost);

				m_pProtocol->AddLastErrorString (IMAPERR_LOCAL_ERROR, tmp);

				mm_log (tmp,ERROR);
				(*m_pResponder) (m_pProtocol, NIL, 0);

				break;

			case GSS_S_FAILURE:
				if (chal.value) fs_give ((void **) &chal.value);

				if (min == (OM_uint32) KRB5_FCC_NOFILE)
				{
					sprintf (tmp,"No credentials cache found (try running kinit) for %s",
							m_szHost);

					m_pProtocol->AddLastErrorString (IMAPERR_LOCAL_ERROR, tmp);

					mm_log (tmp, ERROR);
				}
				else
					do switch (mmaj = gss_display_status (&mmin,min,GSS_C_MECH_CODE,
						 GSS_C_NULL_OID,&mctx,&resp))
					{
						case GSS_S_COMPLETE:
						case GSS_S_CONTINUE_NEEDED:
							sprintf (tmp,"GSSAPI failure: %s",resp.value);
							mm_log (tmp,ERROR);

							m_pProtocol->AddLastErrorString (IMAPERR_LOCAL_ERROR, tmp);

							gss_release_buffer (&mmin,&resp);
					}
					while (mmaj == GSS_S_CONTINUE_NEEDED);

					(*m_pResponder) (m_pProtocol, NIL, 0);

					break;

				default:			/* miscellaneous errors */

					if (chal.value) fs_give ((void **) &chal.value);

					do switch (mmaj = gss_display_status (&mmin,maj,GSS_C_GSS_CODE,
									GSS_C_NULL_OID,&mctx,&resp))
					{
						case GSS_S_COMPLETE:
						mctx = 0;

						case GSS_S_CONTINUE_NEEDED:
							sprintf (tmp,"Unknown GSSAPI failure: %s",resp.value);

							m_pProtocol->AddLastErrorString (IMAPERR_LOCAL_ERROR, tmp);

							mm_log (tmp,ERROR);
							gss_release_buffer (&mmin,&resp);
					}
					while (mmaj == GSS_S_CONTINUE_NEEDED);

					do switch (mmaj = gss_display_status (&mmin,min,GSS_C_MECH_CODE,
							GSS_C_NULL_OID,&mctx,&resp))
					{
						case GSS_S_COMPLETE:
						case GSS_S_CONTINUE_NEEDED:
							sprintf (tmp,"GSSAPI mechanism status: %s",resp.value);

							m_pProtocol->AddLastErrorString (IMAPERR_LOCAL_ERROR, tmp);

							mm_log (tmp,ERROR);
							gss_release_buffer (&mmin,&resp);
					}
					while (mmaj == GSS_S_CONTINUE_NEEDED);

					(*m_pResponder) (m_pProtocol, NIL, 0);

					break;
			}
		}

		/* finished with credentials name */
		if (crname) gss_release_name (&min, &crname);
	}

	return ret;			/* return status */
}








// =============== PRIVATE CKrb4 METHODS ================//

BOOL CGssapi::Initialize ()
{
	// Validate attributes.
	//
	if ( m_szUser.IsEmpty() || m_szHost.IsEmpty()  )
	{
		return FALSE;
	}

	if (!m_pProtocol)
		return FALSE;

	// If already loaded, return:
	//
	if (m_hInstGssapiLib)
		return TRUE;

	// The library name may have been overridden by caller.
	//
	CString libName; m_pProtocol->GetGssLibraryName(libName);

	m_hInstGssapiLib = LoadLibrary(libName);

	if (!m_hInstGssapiLib)
	{
		return FALSE;
	}

	m_pfnGssImportName		= (GSS_IMPORT_NAME *) GetProcAddress (m_hInstGssapiLib, "gss_import_name");

	m_pfnGssInitSecContext	= (GSS_INIT_SEC_CONTEXT *) GetProcAddress (m_hInstGssapiLib, "gss_init_sec_context");

	m_pfnGssReleaseBuffer	= (GSS_RELEASE_BUFFER *) GetProcAddress (m_hInstGssapiLib, "gss_release_buffer");

	m_pfnGssDisplayStatus	= (GSS_DISPLAY_STATUS *) GetProcAddress (m_hInstGssapiLib, "gss_display_status");

	m_pfnGssDeleteSecContext = (GSS_DELETE_SEC_CONTEXT *) GetProcAddress (m_hInstGssapiLib, "gss_delete_sec_context");

	m_pfnGssWrap			= (GSS_WRAP *) GetProcAddress (m_hInstGssapiLib, "gss_wrap");

	m_pfnGssUnwrap			= (GSS_UNWRAP *) GetProcAddress (m_hInstGssapiLib, "gss_unwrap");

	m_pfnGssReleaseName = (GSS_RELEASE_NAME *) GetProcAddress (m_hInstGssapiLib, "gss_release_name");


	// Verify that these are OK
	//
	if ( ! ( m_pfnGssImportName			&&
	         m_pfnGssInitSecContext		&&
			 m_pfnGssReleaseBuffer		&&
			 m_pfnGssDisplayStatus		&&
			 m_pfnGssDeleteSecContext	&&
			 m_pfnGssWrap				&&
			 m_pfnGssUnwrap				&&
			 m_pfnGssReleaseName ) )
	{
		return FALSE;
	}

	return TRUE;
}




#endif // IMAP4
