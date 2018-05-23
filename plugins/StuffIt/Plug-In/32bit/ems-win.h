#ifndef __EMS_WIN__
#define __EMS_WIN__
/* ======================================================================

    C header for EMSAPI version 3
    Copyright 1995, 1996 QUALCOMM Inc.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


    Send comments and questions to <emsapi-info@qualcomm.com>

    Generated: Wed Dec  4 12:37:42 PST 1996
    Filname: ems-win.h
    
    USE THIS FILE TO BUILD YOUR WINDOWS TRANSLATOR DLL

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be changed.


/* ========== CONSTANTS AND RETURN VALUES ================================ */
#ifndef WIN32
#define ULONG unsigned long
#endif

/* ----- Translator return codes --- store as a long --------------------- */ 
#define EMSR_OK              (0L)     /* The translation operation succeeded */
#define EMSR_UNKNOWN_FAIL    (1L)     /* Failed for unspecified reason */
#define EMSR_CANT_TRANS      (2L)     /* Don't know how to translate this */
#define EMSR_INVALID_TRANS   (3L)     /* The translator ID given was invalid */
#define EMSR_NO_ENTRY        (4L)     /* The value requested doesn't exist */
#define EMSR_NO_INPUT_FILE   (5L)     /* Couldn't find input file */
#define EMSR_CANT_CREATE     (6L)     /* Couldn't create the output file */
#define EMSR_TRANS_FAILED    (7L)     /* The translation failed. */
#define EMSR_INVALID         (8L)     /* Invalid argument(s) given */
#define EMSR_NOT_NOW         (9L)     /* Translation can be done not in current
                                         context */
#define EMSR_NOW            (10L)     /* Indicates translation can be performed
                                         right away */
#define EMSR_ABORTED        (11L)     /* Translation was aborted by user */
#define EMSR_DATA_UNCHANGED (12L)     /* Trans OK, data was not changed */
 
 
/* ----- Translator types --- store as a long ---------------------------- */ 
#define EMST_NO_TYPE          (-1L)
#define EMST_LANGUAGE         (0x10L)
#define EMST_TEXT_FORMAT      (0x20L)
#define EMST_GRAPHIC_FORMAT   (0x30L)
#define EMST_COMPRESSION      (0x40L)
#define EMST_COALESCED        (0x50L)
#define EMST_SIGNATURE        (0x60L)
#define EMST_PREPROCESS       (0x70L)
#define EMST_CERT_MANAGEMENT  (0x80L)


/* ----- Translator info flags and contexts --- store as a long ---------- */
/* Used both as bit flags and as constants */
#define EMSF_ON_ARRIVAL      (0x0001L) /* Call on message arrivial */
#define EMSF_ON_DISPLAY      (0x0002L) /* Call when user views message */
#define EMSF_ON_REQUEST      (0x0004L) /* Call when selected from menu */
#define EMSF_Q4_COMPLETION   (0x0008L) /* Queue and call on complete 
                                          composition of a message */
#define EMSF_Q4_TRANSMISSION (0x0010L) /* Queue and call on transmission
                                          of a message */
#define EMSF_WHOLE_MESSAGE   (0x0200L) /* Works on the whole message even if
                                         it has sub-parts. (e.g. signature) */
#define EMSF_REQUIRES_MIME   (0x0400L) /* Items presented for translation
                                          should be MIME entities with
                                          canonical end of line representation,
                                          proper transfer encoding
                                          and headers */
#define EMSF_GENERATES_MIME  (0x0800L) /* Data produced will be MIME format */
#define EMSF_ALL_HEADERS     (0x1000L) /* All headers in & out of trans when
                                          MIME format is used */
#define EMSF_BASIC_HEADERS   (0x2000L) /* Just the basic to, from, subject, cc,b
					  cc headers */

#define EMSF_DEFAULT_Q_ON    (0x4000L)/* Causes queued translation to be on 
                                          for a new message by default */
/* all other flag bits in the long are RESERVED and may not be used */


/* ----- The version of the API defined by this include file ------------- */
#define EMS_VERSION          (3)       /* Used in plugin init */
#define EMS_PB_VERSION       (3)       /* Minimum version that uses parameter blocks */
#define EMS_COMPONENT        'EuTL'    /* Macintosh component type */


/* ----- Translator and translator type specific return codes ------------ */
#define EMSC_SIGOK           (1L)      /* A signature verification succeeded */
#define EMSC_SIGBAD          (2L)      /* A signature verification failed */
#define EMSC_SIGUNKNOWN      (3L)      /* Result of verification unknown */



/* ========== DATA STRUCTURES ============================================ */
/* All strings on Windows are NULL terminated C strings */
/* ----- MIME type data passed across the API ---------------------------- */
/* ----- MIME Params ---------------------------- */
typedef struct emsMIMEparamS FAR*emsMIMEParamP;
typedef struct emsMIMEparamS {
    long		size;
    LPSTR		name;		/* Mime parameter name (e.g., charset) */
    LPSTR		value;		/* param value (e.g. us-ascii) */
    emsMIMEParamP  	next;		/* Linked list of parameters */
} emsMIMEparam;
/* ----- MIME Info ---------------------------- */
typedef struct emsMIMEtypeS FAR*emsMIMEtypeP; 
typedef struct emsMIMEtypeS {
    long		size;
    LPSTR		version;	/* The MIME-Version header */
    LPSTR		type;		/* Top-level MIME type */      
    LPSTR		subType;	/* Top-level MIME subtype */      
    emsMIMEParamP       contentDisp;  	/* Content-Disposition */
    emsMIMEParamP	params;		/* MIME parameter list */
} emsMIMEtype;
/* ----- User Address ---------------------------- */
typedef struct emsAddressS FAR*emsAddressP; 
typedef struct emsAddressS {
    long		size;		/* Size of this data structure */
    LPSTR		address;	/* Optional directory for config file */
    LPSTR		realname;	/* Users full name from Eudora config */
    emsAddressP		next;		/* Linked list of addresses */
} emsAddress;
/* ----- Header Data ---------------------------- */
typedef struct emsHeaderDataS FAR*emsHeaderDataP; 
typedef struct emsHeaderDataS {
    long		size;		/* Size of this data structure */
    emsAddressP		to;		/* To Header */
    emsAddressP		from;		/* From Header */
    LPSTR           	subject;       	/* Subject Header */
    emsAddressP		cc;		/* cc Header */
    emsAddressP		bcc;		/* bcc Header */
    LPSTR		rawHeaders;	/* The 822 headers */
} emsHeaderData;
/* ----- How Eudora is configured ---------------------------- */
typedef struct emsMailConfigS FAR*emsMailConfigP; 
typedef struct emsMailConfigS {
    long		size;		/* Size of this data structure */
    HWND		FAR*eudoraWnd;	/* Eudora's main window */
    LPSTR		configDir;	/* Optional directory for config file */
    emsAddress		userAddr;	/* Users full name from Eudora config */
} emsMailConfig;
/* ----- Plugin Info ---------------------------- */
typedef struct emsPluginInfoS FAR*emsPluginInfoP; 
typedef struct emsPluginInfoS {
    long		size;		/* Size of this data structure */
    long		numTrans;	/* Place to return num of translators */
    long		numAttachers;	/* Place to return num of attach hooks */
    long		numSpecials;	/* Place to return num of special hooks */
    LPSTR		desc;		/* Return for string description of plugin */
    long		id;		/* Place to return unique plugin id */
    HICON		FAR*icon;		/* Return for plugin icon data */
} emsPluginInfo;
/* ----- Translator Info ---------------------------- */
typedef struct emsTranslatorS FAR*emsTranslatorP; 
typedef struct emsTranslatorS {
    long		size;		/* Size of this data structure */
    long		id;		/* ID of translator to get info for */
    long		type;		/* translator type, e.g., EMST_xxx */
    ULONG		flags;		/* translator flags */
    LPSTR		desc;		/* translator string description */
    HICON		FAR*icon;		/* translator icon data */
    LPSTR		properties;	/* Properties for queued translations */
} emsTranslator;
/* ----- Menu Item Info ---------------------------- */
typedef struct emsMenuS FAR*emsMenuP; 
typedef struct emsMenuS {
    long		size;		/* Size of this data structure */
    long		id;		/* ID of translator to get info for */
    LPSTR		desc;		/* translator string description */
} emsMenu;
/* ----- Translation Data ---------------------------- */
typedef struct emsDataFileS FAR*emsDataFileP; 
typedef struct emsDataFileS {
    long		size;		/* Size of this data structure */
    long		context;
    emsMIMEtypeP	info;		/* MIME type of data to check */
    emsHeaderDataP      header; 	/* EMSF_BASIC_HEADERS & EMSF_ALL_HEADERS determine contents */
    LPSTR		fileName;	/* The input file name */
} emsDataFile;
/* ----- Resulting Status Data ---------------------------- */
typedef struct emsResultStatusS FAR*emsResultStatusP; 
typedef struct emsResultStatusS {
    long		size;		/* Size of this data structure */
    LPSTR		desc;		/* Returned string for display with the result */
    LPSTR		error;		/* Place to return string with error message */
    long		code;		/* Return for translator-specific result code */
} emsResultStatus;
/* ----- Progress Data ---------------------------- */
typedef struct emsProgressDataS FAR* emsProgressDataP;
typedef struct emsProgressDataS {
    long		size;		/* Size of this data structure */
    long		value;		/* Range of Progress, percent complete */
    LPSTR		message;	/* Progress Message */
} emsProgressData;

/* ========== FUNCTIONS PASSED ACROSS THE API ============================ */
typedef short (FAR* emsProgress)(emsProgressDataP);     /* The progress function  */

/* ========== FUNCTION PROTOTYPES ======================================== */

/* ----- Get the API Version number this plugin implements ----------------- */
extern "C" long WINAPI ems_plugin_version(
    short FAR* apiVersion       /* Out: Plugin Version */
);

/* ----- Initialize plugin and get its basic info -------------------------- */
extern "C" long WINAPI ems_plugin_init(
    void FAR*FAR* globals,          /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
);

/* ----- Get details about a translator in a plugin ------------------------ */
extern "C" long WINAPI ems_translator_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
);

/* ----- Check and see if a translation can be performed ------------------- */
extern "C" long WINAPI ems_can_translate(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inTransData,   /* In: What to translate */
    emsResultStatusP transStatus /* Out: Translations Status information */
);

/* ----- Actually perform a translation on a file -------------------------- */
extern "C" long WINAPI ems_translate_file(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inFile,        /* In: What to translate */
    emsProgress progress,       /* Func to report progress/check for abort */
    emsDataFileP outFile,       /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
);

/* ----- End use of a plugin and clean up ---------------------------------- */
extern "C" long WINAPI ems_plugin_finish(
    void FAR* globals           /* Out: Return for allocated instance structure */
);

/* ----- Free memory allocated by EMS plug-in ------------------------------ */
extern "C" long WINAPI ems_free(
    void FAR* mem               /* Memory to free */
);

/* ----- Call the plug-ins configuration Interface ------------------------- */
extern "C" long WINAPI ems_plugin_config(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMailConfigP mailConfig   /* In: Eudora mail info */
);

/* ----- Manage properties for queued translations ------------------------- */
extern "C" long WINAPI ems_queued_properties(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    long context,               /* In: EMSF_Q4_COMPLETION or _TRANSMISSION */
    emsTranslatorP trans,        /* In/Out: The translator */
    long selected               /* In/Out: State of this translator */
);

/* ----- Info about menu hook to attach/insert composed object ------------- */
extern "C" long WINAPI ems_attacher_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP attachMenu         /* Out: The menu */
);

/* ----- Call an attacher hook to compose some special object -------------- */
extern "C" long WINAPI ems_attacher_hook(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP attachMenu,        /* In: The menu */
    LPSTR attachDir,            /* In: Location to put attachments */
    long * numAttach,           /* Out: Number of files attached */
    emsDataFileP ** attachFiles /* Out: Name of files written */
);

/* ----- Info about special menu items hooks ------------------------------- */
extern "C" long WINAPI ems_special_info(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* Out: The menu */
);

/* ----- Call a special menu item hook ------------------------------------- */
extern "C" long WINAPI ems_special_hook(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* In: The menu */
);


#endif /* __EMS_WIN__ */

