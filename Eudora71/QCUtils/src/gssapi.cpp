// gssapi.cpp: Implementation file for generic GSSAPI authenticator class.
//
// Copyright (c) 2002 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//

#include "stdafx.h"

#include <string.h>
#include <stdio.h>

#include "resource.h"

#include "gssapi.h"
#include "gssapi_generic.h"
#include "gssapi_krb5.h"
#include "krb5.h"


#define PROTOTYPE(x) x
#define KRB5_PROVIDE_PROTOTYPES


#include "DebugNewHelpers.h"

// Disable "assignment within conditional expression" warning:
#pragma warning( disable : 4706 )


#define AUTH_GSSAPI_P_NONE 1
#define AUTH_GSSAPI_P_INTEGRITY 2
#define AUTH_GSSAPI_P_PRIVACY 4

#define AUTH_GSSAPI_C_MAXSIZE 8192

#define SERVER_LOG(x,y) syslog (LOG_ALERT,x,y)

extern char *krb5_defkeyname;	/* sneaky way to get this name */

static gss_OID_desc oids[] = {
   {10, "\052\206\110\206\367\022\001\002\001\004"},
};

gss_OID_desc * gss_nt_service_name = oids+0;


void fs_give (void **block)
{
	if ( block && *block )
	{
		free (*block);
		*block = NIL;
	}

}


CGssapi::CGssapi(void *challenger,
				 void *responder,
				 void *s,
				 LPCSTR pHost,
				 LPCSTR pUser)
			  : m_pChallenger (challenger),
			    m_pResponder (responder),
				m_pProtocol (s),
				m_szHost (pHost),
				m_szUser (pUser)
{
	m_hInstGssapiLib = NULL;
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


//
//	CGssapi::Initialize()
//
//	Initializes the GSSAPI object.
//
BOOL CGssapi::Initialize()
{
	// Validate attributes.
	//
	if ( m_szUser.IsEmpty() || m_szHost.IsEmpty()  )
	{
		return FALSE;
	}

	if (!m_pProtocol)
		return FALSE;

	// If the GSSAPI DLL has already been loaded, return.
	if (m_hInstGssapiLib)
		return TRUE;

	// Get the DLL name and load the library.
	m_hInstGssapiLib = LoadLibrary(GetDLLName());

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

//
//	CGssapi::ShowStatusMessage()
//
//	Obtains the text message for the specified minor status and calls the
//	routine to display the message.
//
//	Parameters:
//		min[in] - Minor status code whose text we are seeking.
//
void CGssapi::ShowStatusMessage(OM_uint32 min)
{
	OM_uint32		 mmaj = 0;
	OM_uint32		 mmin = 0;
	OM_uint32		 mctx = 0;
	gss_buffer_desc	 mresp;
	char			 tmp[MAILTMPLEN];

	do
	{
		mmaj = gss_display_status (&mmin,min,GSS_C_MECH_CODE,
								 GSS_C_NULL_OID,&mctx,&mresp);
		switch (mmaj)
		{
			case GSS_S_COMPLETE:
			case GSS_S_CONTINUE_NEEDED:
				sprintf (tmp,"GSSAPI failure: %s",mresp.value);
				AddLastErrorString (tmp);

				gss_release_buffer (&mmin,&mresp);
		}
	}
	while (mmaj == GSS_S_CONTINUE_NEEDED);
}

//
//	CGssapi::Abort()
//
//	Aborts the authentication by sending an empty response then
//	reads the server's error message.
//
void CGssapi::Abort()
{
	// Send the abort.
	SendResponse(NIL, 0);

	// Obtain and display the server's response.
	char			*szChal = NULL;
	unsigned long	 lLen = 0;
	if (szChal = (char*)GetChallenge(&lLen))
	{
		AddLastErrorString((const char *)szChal);
		free(szChal);
	}
}

//
//	CGssapi::Authenticate()
//
//	Does the actual GSSAPI authentication negotiation.
//
//	Return:
//		T if the authentication succeeded, NIL on failure.
//
long CGssapi::Authenticate()
{
	long		ret = NIL;
	char		tmp[MAILTMPLEN];
	OM_uint32	maj,min;
	gss_ctx_id_t ctx = GSS_C_NO_CONTEXT;
	gss_buffer_desc chal,resp,buf;
	gss_name_t	crname = NIL;
	BOOL		bRespSent = FALSE;
	int			conf;
	gss_qop_t	qop;

	// Make sure we are initialized.
	if ( !Initialize() )
		return 0;

    // Get the initial (empty) challenge.
	chal.value = GetChallenge((unsigned long *) &chal.length);

	// Set up the printable version of service name for gss_import_name().
	sprintf (tmp,"%s@%s", m_szService, m_szHost);

	buf.length = strlen ( (char *) (buf.value = tmp)) + 1;

	// Convert the service name to internal form.
	if (gss_import_name(
						&min,						// [Output] Minor status
						&buf,						// Buffer containing printable name to convert
						gss_nt_service_name,		// Input name type
						&crname)					// Name in internal form
						!=GSS_S_COMPLETE)
	{
		// Failed to convert name: abort.
		Abort();
	}
	else
	{
		// Initiate a security context with the initial challenge.
		maj = gss_init_sec_context(
						&min,							// [Output] Minor status
						GSS_C_NO_CREDENTIAL,			// Use default credential
						&ctx,							// [Output] Handle for this context
						crname,							// Internal name of context acceptor generated above
						GSS_C_NO_OID,					// Use default security mechanism
						GSS_C_MUTUAL_FLAG |				// Have the context acceptor authenticate itself and
							GSS_C_REPLAY_FLAG,			//  detect replayed signed/sealed messages
						0,								// Use default context validity period
						GSS_C_NO_CHANNEL_BINDINGS,		// Do not bind channel ID info to context
						GSS_C_NO_BUFFER,				// Input token: pass this value the first time
						NIL,							// [Output] Acutal security mechanism types
						&resp,							// [Output] Token to send to context acceptor
						NIL,							// [Output] Flags to indicate supported service options
						NIL);							// [Output] Seconds for which context will be valid
		switch (maj)
		{
		case GSS_S_CONTINUE_NEEDED:
			// There is more to do: loop until every challenge has been given a proper response.
			do
			{
				// Give the value memory back.
				fs_give ((void **) &chal.value);

				// Send response generated by gss_init_sec_context().
				bRespSent = SendResponse((char *) resp.value, resp.length);

				// Free the response buffer's value field.
				gss_release_buffer (&min,&resp);

				if (bRespSent)
				{
					// Get the next challenge.
					if (chal.value = GetChallenge((unsigned long *)&chal.length))
					{
						 // Initiate another security context with the latest challenge.
						 maj = gss_init_sec_context (
									&min,							// [Output] Minor status
									GSS_C_NO_CREDENTIAL,			// Use default credential
									&ctx,							// Handle for this context (obtained above)
									crname,							// Internal name of context acceptor generated above
									GSS_C_NO_OID,					// Use default security mechanism
									GSS_C_MUTUAL_FLAG |				// Have the context acceptor authenticate itself and
										GSS_C_REPLAY_FLAG,			//  detect replayed signed/sealed messages
									0,								// Use default context validity period
									GSS_C_NO_CHANNEL_BINDINGS,		// Do not bind channel ID info to context
									&chal,							// Input token returned above
									NIL,							// [Output] Acutal security mechanism types
									&resp,							// [Output] Output token
									NIL,							// [Output] Flags to indicate supported service options
									NIL);							// [Output] Seconds for which context will be valid
					}
				}
			} while (bRespSent && chal.value && (maj == GSS_S_CONTINUE_NEEDED));

			if (maj != GSS_S_COMPLETE)
			{
				// Authentication did not succeed.

				// Give the value memory back.
				fs_give ((void **) &chal.value);

				// Abort.
				Abort();

				// Obtain and display the status message.
				ShowStatusMessage(min);

				// Don't need context any more.
				gss_delete_sec_context (&min,&ctx,NIL);

				break;
			}

			// If we reach this point the status is GSS_S_COMPLETE: fall through.

		case GSS_S_COMPLETE:
			// We have successful mutual authentication and completion of context establishment.

			// Give the value memory back.
			fs_give ((void **) &chal.value);

			// Get the protection mechanisms and max size.
			if (// Send the response generated by the last gss_init_sec_context() call.
				SendResponse(resp.value ? (char *) resp.value : "", resp.length) &&
				// Get the next challenge.
  				(chal.value = GetChallenge((unsigned long *)&chal.length)) &&
				// Decrypt the challenge.
				(gss_unwrap (
								&min,					// [Output] Minor status
								ctx,					// Handle for this context (obtained above)
								&chal,					// Input token returned above
								&resp,					// [Output] Output token
								&conf,					// [Output] Confidentiality state of the token
								&qop)					// [Output] Quality of protection gained
								== GSS_S_COMPLETE) &&
				// Min valid response length is 4 (flag byte, NULL byte?, length byte, NULL byte?).
				(resp.length >= 4) &&
				// Make sure AUTH_GSSAPI_P_NONE bit is set.
				(*((char *) resp.value) & AUTH_GSSAPI_P_NONE))
			{
				// Copy the value field (containing the flags and length).
				memcpy (tmp, resp.value, 4);

				// Free the response buffer's value field.
				gss_release_buffer (&min, &resp);

				// Reset the session protection value.
				tmp[0] = AUTH_GSSAPI_P_NONE;

				// Append user name to response.
				strcpy (tmp+4, m_szUser);

				// Set the buffer's value and length fields to what we just generated.
				buf.value = tmp;
				buf.length = strlen (m_szUser) + 4;

				// Encrypt the response.
				if (gss_wrap (
								&min,					// [Output] Minor status
								ctx,					// Handle for this context (obtained above)
								FALSE,					// Don't request confidentiality
								qop,					// Quality of protection (from gss_unwrap())
								&buf,					// Message to protect
								&conf,					// [Output] Confidentiality state of the token
								&resp)					// [Output] Buffer to receive protected message
								== GSS_S_COMPLETE)
				{
					// Send the encrypted response.
					if (SendResponse((char *) resp.value, resp.length))
						// The authentication succeeded.
						ret = T;

					// Free the response buffer's value field.
					gss_release_buffer (&min,&resp);
				}
				else
				{
					// Encryption failed: abort.
					Abort();
				}
			}

			// Give the value memory back.
			fs_give ((void **) &chal.value);

			// Don't need context any more.
			gss_delete_sec_context (&min,&ctx,NIL);

			break;

		case GSS_S_CREDENTIALS_EXPIRED:
			// Credentials expired.

			// Give the value memory back.
			fs_give ((void **) &chal.value);

			// Show and log the error message.
			sprintf (tmp,"Kerberos credentials expired (try running kinit) for %s", m_szHost);
			AddLastErrorString (tmp);

			// Abort.
			Abort();

			break;

		case GSS_S_FAILURE:
			// Authentication failed.

			// Give the value memory back.
			fs_give ((void **) &chal.value);

			// Try to obtain a more specific error message which will be shown
			// to the user and logged.
			if (min == (OM_uint32) KRB5_FCC_NOFILE)
			{
				sprintf (tmp,"No credentials cache found (try running kinit) for %s",
						m_szHost);

				AddLastErrorString (tmp);
			}
			else
			{
				ShowStatusMessage(min);
			}

			// Abort.
			Abort();

			break;

		default:
			// Handle miscellaneous errors.

			// Give the value memory back.
			fs_give ((void **) &chal.value);

			// Try to obtain a more specific error message which will be shown
			// to the user and logged.
			ShowStatusMessage(min);

			// Abort.
			SendResponse(NIL, 0);

			break;
		}
	}

	// Finished with credentials name.
	if (crname) gss_release_name (&min, &crname);

	// Return status.
	return ret;
}


//
// The following are pure virtual methods that must be overridden by a subclass.
// They have been given implementations here to prevent "unresolved external"
// compiler errors due to the subclass being part of a different DLL. -dwiggins
//

//
//	CGssapi::GetServiceName()
//
//	Returns the service name for this protocol.
//
//	Return:
//		A CString containing the service name.
//
CString CGssapi::GetServiceName()
{
	return CString("");
}

//
//	CGssapi::GetDLLName()
//
//	Returns the name of the GSSAPI DLL file.
//
//	Return:
//		A CString containing the DLL file name.
//
CString CGssapi::GetDLLName()
{
	return CString("");
}

//
//	CGssapi::GetChallenge()
//
//	Obtains the challenge from the m_pChallenger routine.
//
//	Parameters:
//		lSize[out] - Length of challenge data returned.
//
//	Return:
//		The challenge data or NULL if the read failed.
//
void *CGssapi::GetChallenge(unsigned long *)
{
	return NULL;
}

//
//	CGssapi::SendResponse()
//
//	Sends the specified response to the m_pResponder routine.
//
//	Parameters:
//		szResp[in] - Buffer to send.  If this is NULL the NULL will be passed
//			along causing the authentication to be aborted.  If this is an empty
//			string the appropriate "end of data" string will be passed along.
//			In all other cases this data is simply passed to the server.
//		lSize[in] - Length of data in szResp.
//
//	Return:
//		The return value of the responder routine (TRUE on success, FALSE on failure).
//
BOOL CGssapi::SendResponse(char *, unsigned long)
{
	return FALSE;
}

//
//	CGssapi::AddLastErrorString()
//
//	Displays to the user and logs the specified error.
//
//	Parameters:
//		szError[in] - Error string to display and log.
//
void CGssapi::AddLastErrorString(const char *)
{
}
