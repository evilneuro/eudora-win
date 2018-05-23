// gssapi.cpp: Header file for generic GSSAPI authenticator class.
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

/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 * 
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GSSAPI_H_
#define _GSSAPI_H_

/*
 * Determine platform-dependent configuration.
 */

#if defined(_MSDOS) || defined(_WIN32) || defined(_MACINTOSH)
#include "win-mac.h"
#endif

#ifndef KRB5_CALLCONV
#define KRB5_CALLCONV
#define KRB5_CALLCONV_C
#define KRB5_DLLIMP
#define KRB5_EXPORTVAR
#endif
#ifndef FAR
#define FAR
#define NEAR
#endif

#define	GSS_SIZEOF_INT		SIZEOF_INT
#define	GSS_SIZEOF_LONG		SIZEOF_LONG
#define	GSS_SIZEOF_SHORT	SIZEOF_SHORT

/*
 * Make sure we have a definition for PROTOTYPE.
 */
#if !defined(PROTOTYPE)
#if defined(__STDC__) || defined(__cplusplus) || defined(_MSDOS) || defined(_WIN32) || defined(__ultrix)
#define PROTOTYPE(x) x
#else
#define PROTOTYPE(x) ()
#endif
#endif

/*
 * First, include stddef.h to get size_t defined.
 */
#if	HAVE_STDDEF_H
#include <stddef.h>
#endif	/* HAVE_STDDEF_H */

/*
 * POSIX says that sys/types.h is where size_t is defined.
 */
#ifndef _MACINTOSH
#include <sys/types.h>
#endif

/*
 * If the platform supports the xom.h header file, it should be included here.
 */
#if	HAVE_XOM_H
#include <xom.h>
#endif	/* HAVE_XOM_H */

/*
 * $Id: gssapi.hin,v 1.7 1997/03/28 04:43:21 tytso Exp $
 */

/*
 * First, define the three platform-dependent pointer types.
 */

typedef void FAR * gss_name_t;
typedef void FAR * gss_cred_id_t;
typedef void FAR * gss_ctx_id_t;

/*
 * The following type must be defined as the smallest natural unsigned integer
 * supported by the platform that has at least 32 bits of precision.
 */
#if (GSS_SIZEOF_SHORT == 4)
typedef unsigned short gss_uint32;
typedef short gss_int32;
#elif (GSS_SIZEOF_INT == 4)
typedef unsigned int gss_uint32;
typedef int gss_int32;
#elif (GSS_SIZEOF_LONG == 4)
typedef unsigned long gss_uint32;
typedef long gss_int32;
#endif

#ifdef	OM_STRING
/*
 * We have included the xom.h header file.  Use the definition for
 * OM_object identifier.
 */
typedef OM_object_identifier	gss_OID_desc, *gss_OID;
#else	/* OM_STRING */
/*
 * We can't use X/Open definitions, so roll our own.
 */
typedef gss_uint32	OM_uint32;

typedef struct gss_OID_desc_struct {
      OM_uint32 length;
      void      FAR *elements;
} gss_OID_desc, FAR *gss_OID;
#endif	/* OM_STRING */

typedef struct gss_OID_set_desc_struct  {
      size_t  count;
      gss_OID elements;
} gss_OID_set_desc, FAR *gss_OID_set;

typedef struct gss_buffer_desc_struct {
      size_t length;
      void FAR *value;
} gss_buffer_desc, FAR *gss_buffer_t;

typedef struct gss_channel_bindings_struct {
      OM_uint32 initiator_addrtype;
      gss_buffer_desc initiator_address;
      OM_uint32 acceptor_addrtype;
      gss_buffer_desc acceptor_address;
      gss_buffer_desc application_data;
} FAR *gss_channel_bindings_t;

/*
 * For now, define a QOP-type as an OM_uint32 (pending resolution of ongoing
 * discussions).
 */
typedef	OM_uint32	gss_qop_t;
typedef	int		gss_cred_usage_t;

/*
 * Flag bits for context-level services.
 */
#define GSS_C_DELEG_FLAG 1
#define GSS_C_MUTUAL_FLAG 2
#define GSS_C_REPLAY_FLAG 4
#define GSS_C_SEQUENCE_FLAG 8
#define GSS_C_CONF_FLAG 16
#define GSS_C_INTEG_FLAG 32
#define	GSS_C_ANON_FLAG 64
#define GSS_C_PROT_READY_FLAG 128
#define GSS_C_TRANS_FLAG 256

/*
 * Credential usage options
 */
#define GSS_C_BOTH 0
#define GSS_C_INITIATE 1
#define GSS_C_ACCEPT 2

/*
 * Status code types for gss_display_status
 */
#define GSS_C_GSS_CODE 1
#define GSS_C_MECH_CODE 2

/*
 * The constant definitions for channel-bindings address families
 */
#define GSS_C_AF_UNSPEC     0
#define GSS_C_AF_LOCAL      1
#define GSS_C_AF_INET       2
#define GSS_C_AF_IMPLINK    3
#define GSS_C_AF_PUP        4
#define GSS_C_AF_CHAOS      5
#define GSS_C_AF_NS         6
#define GSS_C_AF_NBS        7
#define GSS_C_AF_ECMA       8
#define GSS_C_AF_DATAKIT    9
#define GSS_C_AF_CCITT      10
#define GSS_C_AF_SNA        11
#define GSS_C_AF_DECnet     12
#define GSS_C_AF_DLI        13
#define GSS_C_AF_LAT        14
#define GSS_C_AF_HYLINK     15
#define GSS_C_AF_APPLETALK  16
#define GSS_C_AF_BSC        17
#define GSS_C_AF_DSS        18
#define GSS_C_AF_OSI        19
#define GSS_C_AF_X25        21

#define GSS_C_AF_NULLADDR   255

/*
 * Various Null values.
 */
#define GSS_C_NO_BUFFER ((gss_buffer_t) 0)
#define GSS_C_NO_OID ((gss_OID) 0)
#define GSS_C_NO_OID_SET ((gss_OID_set) 0)
#define GSS_C_NO_CONTEXT ((gss_ctx_id_t) 0)
#define GSS_C_NO_CREDENTIAL ((gss_cred_id_t) 0)
#define GSS_C_NO_CHANNEL_BINDINGS ((gss_channel_bindings_t) 0)
#define GSS_C_EMPTY_BUFFER {0, NULL}

/*
 * Some alternate names for a couple of the above values.  These are defined
 * for V1 compatibility.
 */
#define	GSS_C_NULL_OID		GSS_C_NO_OID
#define	GSS_C_NULL_OID_SET	GSS_C_NO_OID_SET

/*
 * Define the default Quality of Protection for per-message services.  Note
 * that an implementation that offers multiple levels of QOP may either reserve
 * a value (for example zero, as assumed here) to mean "default protection", or
 * alternatively may simply equate GSS_C_QOP_DEFAULT to a specific explicit
 * QOP value.  However a value of 0 should always be interpreted by a GSSAPI
 * implementation as a request for the default protection level.
 */
#define GSS_C_QOP_DEFAULT 0

/*
 * Expiration time of 2^32-1 seconds means infinite lifetime for a
 * credential or security context
 */
#define GSS_C_INDEFINITE ((OM_uint32) 0xfffffffful)


/* Major status codes */

#define GSS_S_COMPLETE 0

/*
 * Some "helper" definitions to make the status code macros obvious.
 */
#define GSS_C_CALLING_ERROR_OFFSET 24
#define GSS_C_ROUTINE_ERROR_OFFSET 16
#define GSS_C_SUPPLEMENTARY_OFFSET 0
#define GSS_C_CALLING_ERROR_MASK ((OM_uint32) 0377ul)
#define GSS_C_ROUTINE_ERROR_MASK ((OM_uint32) 0377ul)
#define GSS_C_SUPPLEMENTARY_MASK ((OM_uint32) 0177777ul)

/*
 * The macros that test status codes for error conditions.  Note that the
 * GSS_ERROR() macro has changed slightly from the V1 GSSAPI so that it now
 * evaluates its argument only once.
 */
#define GSS_CALLING_ERROR(x) \
  ((x) & (GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET))
#define GSS_ROUTINE_ERROR(x) \
  ((x) & (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET))
#define GSS_SUPPLEMENTARY_INFO(x) \
  ((x) & (GSS_C_SUPPLEMENTARY_MASK << GSS_C_SUPPLEMENTARY_OFFSET))
#define GSS_ERROR(x) \
  ((x) & ((GSS_C_CALLING_ERROR_MASK << GSS_C_CALLING_ERROR_OFFSET) | \
	  (GSS_C_ROUTINE_ERROR_MASK << GSS_C_ROUTINE_ERROR_OFFSET)))

/*
 * Now the actual status code definitions
 */

/*
 * Calling errors:
 */
#define GSS_S_CALL_INACCESSIBLE_READ \
                             (((OM_uint32) 1ul) << GSS_C_CALLING_ERROR_OFFSET)
#define GSS_S_CALL_INACCESSIBLE_WRITE \
                             (((OM_uint32) 2ul) << GSS_C_CALLING_ERROR_OFFSET)
#define GSS_S_CALL_BAD_STRUCTURE \
                             (((OM_uint32) 3ul) << GSS_C_CALLING_ERROR_OFFSET)

/*
 * Routine errors:
 */
#define GSS_S_BAD_MECH (((OM_uint32) 1ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_NAME (((OM_uint32) 2ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_NAMETYPE (((OM_uint32) 3ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_BINDINGS (((OM_uint32) 4ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_STATUS (((OM_uint32) 5ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_SIG (((OM_uint32) 6ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NO_CRED (((OM_uint32) 7ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_NO_CONTEXT (((OM_uint32) 8ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DEFECTIVE_TOKEN (((OM_uint32) 9ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_DEFECTIVE_CREDENTIAL \
     (((OM_uint32) 10ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_CREDENTIALS_EXPIRED \
     (((OM_uint32) 11ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_CONTEXT_EXPIRED \
     (((OM_uint32) 12ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_FAILURE (((OM_uint32) 13ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_BAD_QOP (((OM_uint32) 14ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_UNAUTHORIZED (((OM_uint32) 15ul) << GSS_C_ROUTINE_ERROR_OFFSET)
#define GSS_S_UNAVAILABLE (((OM_uint32) 16ul) << GSS_C_ROUTINE_ERROR_OFFSET)
/*
 * XXX new functions.  Check to get official error number assigments?
 */
#define GSS_S_DUPLICATE_ELEMENT \
     (((OM_uint32) 17ul) << GSS_C_ROUTINE_ERROR_OFFSET)

/*
 * Supplementary info bits:
 */
#define GSS_S_CONTINUE_NEEDED (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 0))
#define GSS_S_DUPLICATE_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 1))
#define GSS_S_OLD_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 2))
#define GSS_S_UNSEQ_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 3))
/*
 * XXX not in the cbindings yet.  remove this comment when it is
 */
#define GSS_S_GAP_TOKEN (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 4))


/* Build parameters */
#define MAILTMPLEN 1024		/* size of a temporary buffer */

/* Constants */
#define NIL 0			/* convenient name */
#define T 1			/* opposite of NIL */
#define LONGT (long) 1		/* long T */


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



class AFX_EXT_CLASS CGssapi
{
public:
	CGssapi (void *challenger,
			  void *responder,
			  void *s,
			  LPCSTR pHost,
			  LPCSTR pUser);

	virtual ~CGssapi();

	virtual long		 Authenticate();


// Internal methods:
protected:
	virtual BOOL		 Initialize();
	virtual void		 ShowStatusMessage(OM_uint32 min);
	virtual void		 Abort();

	// Encapsulation of protocol specific functions.
	virtual CString		 GetServiceName() = 0;
	virtual CString		 GetDLLName() = 0;
	virtual void		*GetChallenge(unsigned long *lLength) = 0;
	virtual BOOL		 SendResponse(char *szResp, unsigned long lSize) = 0;
	virtual void		 AddLastErrorString(const char *szError) = 0;

// Internal attributes.
protected:
	// Protocol specific
	void				*m_pChallenger;
	void				*m_pResponder;
	void				*m_pProtocol;

	CString				 m_szUser;
	CString				 m_szHost;
	CString				 m_szService;

	// Kerberos library instance: (loaded every time??)
	HINSTANCE			 m_hInstGssapiLib;


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
protected:
		GSS_IMPORT_NAME			*m_pfnGssImportName;

		GSS_INIT_SEC_CONTEXT	*m_pfnGssInitSecContext;

		GSS_RELEASE_BUFFER		*m_pfnGssReleaseBuffer;

		GSS_DISPLAY_STATUS		*m_pfnGssDisplayStatus;

		GSS_DELETE_SEC_CONTEXT	*m_pfnGssDeleteSecContext;

		GSS_WRAP				*m_pfnGssWrap;	

		GSS_UNWRAP				*m_pfnGssUnwrap;

		GSS_RELEASE_NAME		*m_pfnGssReleaseName;
};


#endif /* _GSSAPI_H_ */
