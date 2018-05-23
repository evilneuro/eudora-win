 /* ======================================================================

    Eudora Extended Message Services API SDK Windows 1.0b2 (Aug 11 1996)
    This SDK supports EMSAPI version 3
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

    Generated: Mon Aug 12 09:20:19 PDT 1996
    Filname: ems-wglu.h

    THIS IS TO BUILD THE WINDOWS EUDORA _APPLICATION_ SIDE OF THE 
    TRANSLATION API. DO NOT USE IT TO BUILD A TRANSLATOR DLL! 

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be edit manually.


/* ========== CONSTANTS AND RETURN VALUES ================================ */

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

#define EMSF_TOOLBAR_PRESENCE (0x8000L)/* Appear on the Toolbar */
#define EMSF_ALL_TEXT  (0x10000L)/* ON_REQUEST WANTS WHOLE MESSAGE */
#define EMSF_DONTSAVE		  (0x20000L)/* Mark messages as unchanged so user 
					is not prompted for save.  Add to any 
					return code */

/* ----- The version of the API defined by this include file ------------- */
#define EMS_VERSION          (4)       /* Used in plugin init */
#define EMS_COMPONENT        'EuTL'    /* Macintosh component type */
#define EMS_PB_VERSION		 (3)


/* ----- Translator and translator type specific return codes ------------ */
#define EMSC_SIGOK           (1L)      /* A signature verification succeeded */
#define EMSC_SIGBAD          (2L)      /* A signature verification failed */
#define EMSC_SIGUNKNOWN      (3L)      /* Result of verification unknown */

/* ----- V4! Peanut IDLE Events  ------------ */
/* Note that it is possible to have both EMIDLE_UI_ALLOWED and EMIDLE_QUICK set, in that
 * case the plug-in should only do a UI operation if it's absolutely essential (i.e., errors only).
 * If EMIDLE_UI_ALLOWED is NOT set, then using the progress
 */

/* ----- Values for ems_idle flags -------------------------- */
#define EMIDLE_UI_ALLOWED		(1L)      /* Interactions with user are allowed */
#define EMIDLE_QUICK			(2L)      /* Now is NOT the time to do something lengthy */
#define EMIDLE_OFFLINE			(4L)      /* Eudora is in "offline" mode */
#define EMIDLE_PRE_SEND			(8L)  /* Eudora is about to send mail */
#define	EMIDLE_TRANSFERRING		(16L)	/* Currently transferring mail */


/* ----- NEW - Values for emsGetMailBox flags -------------------------- */
#define EMSFGETMBOX_ALLOW_NEW		(1L)      /* Allow creation of new mailboxes */
#define EMSFGETMBOX_ALLOW_OTHER		(2L)      /* Allow selection of "other" mailboxes */

/*----------Needed up here -----------------*/
typedef short     (FAR* emsProgress)(short);     /* The progress function  */

/* ========== DATA STRUCTURES ============================================ */
/* All strings on Windows are NULL terminated C strings */
/* ----- MIME type data passed across the API ---------------------------- */
typedef struct emsMIMEparamS FAR*emsMIMEParamP;
typedef struct emsMIMEparamS {
    char             FAR*    name;    /* Mime parameter name (e.g., charset) */
    char             FAR*    value;   /* param value (e.g. us-ascii) */
    struct emsMIMEparamS FAR* next;   /* Linked list of parameters */
} emsMIMEparam;
typedef struct emsMIMEtypeS FAR*emsMIMEtypeP; 
typedef struct emsMIMEtypeS {
    char         FAR* mime_version;   /* The MIME-Version header */
    char         FAR* mime_type;      /* Top-level MIME type */      
    char         FAR* sub_type;       /* MIME sub-type */
    emsMIMEparam  FAR* params;        /* MIME parameter list */
} emsMIMEtype;

/* ========== FUNCTION PROTOTYPES ======================================== */

/* ----- Get the API Version number this plugin implements ----------------- */
typedef long WINAPI ems_plugin_version_t(
    short FAR* api_version      /* Place to return api version */
);

/* ----- Initialize plugin and get its basic info -------------------------- */
typedef long WINAPI ems2_plugin_init_t(
    void FAR*FAR* globals,      /* Return for allocated instance structure */
    char FAR* config_dir,       /* Optional directory for config file */
    char FAR* user_name,        /* Users full name from Eudora config */
    char FAR* user_addr,        /* Users address (RFC-822 addr spec) */
    short FAR* num_trans,       /* Place to return num of translators */
    char FAR*FAR* plugin_desc,  /* Return for string description of plugin */
    short FAR* plugin_id,       /* Place to return unique plugin id */
    HICON FAR*FAR* plugin_icon /* Return for plugin icon data */
);

/* ----- Get details about a translator in a plugin ------------------------ */
typedef long WINAPI ems2_translator_info_t(
    void FAR* globals,          /* Pointer to translator instance structure */
    short trans_id,             /* ID of translator to get info for */
    long FAR* trans_type,       /* Return for translator type, e.g., EMST_xxx */
    long FAR* trans_sub_type,   /* Return for translator subtype */
    unsigned long FAR* trans_flags, /* Return for translator flags */
    char FAR*FAR* trans_desc,   /* Return for translator string description */
    HICON FAR*FAR* trans_icon /* Return for translator icon data */
);

/* ----- Check and see if a translation can be performed ------------------- */
typedef long WINAPI ems2_can_translate_t(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* Context for check; e.g. EMSF_ON_xxx */
    short trans_id,             /* ID of translator to call */
    emsMIMEtypeP in_mime,       /* MIME type of data to check */
    char FAR*FAR* addresses,    /* List of addresses (sender and recipients) */
    char FAR* properties,       /* Properties for queued translations */
    char FAR*FAR* out_error,    /* Place to return string with error message */
    long FAR* out_code          /* Return for translator-specific result code */
);

/* ----- Actually perform a translation on a file -------------------------- */
typedef long WINAPI ems2_translate_file_t(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* Context for translation; e.g. EMSF_ON_xxx */
    short trans_id,             /* ID of translator to call */
    emsMIMEtypeP in_mime,       /* MIME type of input data */
    char FAR* in_file,          /* The input file name */
    char FAR*FAR* addresses,    /* List of addresses (sender and recipients) */
    char FAR* properties,       /* Properties for queued translations */
    emsProgress progress,       /* Func to report progress / check for abort */
    emsMIMEtypeP FAR* out_mime, /* Place to return MIME type of result */
    char FAR* out_file,         /* The output file (specified by Eudora) */
    HICON FAR*FAR* out_icon,  /* Place to return icon representing result */
    char FAR*FAR* out_desc,     /* Returned string for display with the result */
    char FAR*FAR* out_error,    /* Place to return string with error message */
    long FAR* out_code          /* Return for translator-specific result code */
);

/* ----- Actually perform a translation on a buffer ------------------------ */
typedef long WINAPI ems2_translate_buf_t(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* Context for translation; e.g. EMSF_ON_xxx */
    short trans_id,             /* ID of translator to call */
    emsMIMEtypeP in_mime,       /* MIME type of input data */
    unsigned char FAR* in_buffer, /* Pointer to buffer of data to translate */
    long FAR* in_buffer_len,    /* Amount of input & returns input consumed */
    char FAR*FAR* addresses,    /* List of addresses (sender and recipients) */
    char FAR* properties,       /* Properties for queued translations */
    emsProgress progress,       /* Func to report progress/check for abort */
    emsMIMEtypeP FAR* out_mime, /* Place to return MIME type of result */
    unsigned char FAR* FAR* out_buffer, /* Eudora supplied output buffer */
    long FAR* out_buffer_len,   /* Out buffer size & returns amount filled in */
    HICON FAR*FAR* out_icon,  /* Place to return icon representing result */
    char FAR*FAR* out_desc,     /* Returned string for display with the result */
    char FAR*FAR* out_error,    /* Place to return string with error message */
    long FAR* out_code          /* Return for translator-specific result code */
);

/* ----- End use of a plugin and clean up ---------------------------------- */
typedef long WINAPI ems2_plugin_finish_t(
    void FAR* globals           /* Pointer to translator instance structure */
);

/* ----- Free memory allocated by EMS plug-in ------------------------------ */
typedef long WINAPI ems2_free_t(
    void FAR* mem               /* Memory to free */
);

/* ----- Call the plug-ins configuration Interface ------------------------- */
typedef long WINAPI ems2_plugin_config_t(
    void FAR* globals,          /* Pointer to translator instance structure */
    char FAR* config_dir,       /* Optional directory for config file */
    char FAR* user_name,        /* Users full name from Eudora config */
    char FAR* user_addr        /* Users address (RFC-822 addr spec) */
);

/* ----- Manage properties for queued translations ------------------------- */
typedef long WINAPI ems2_queued_properties_t(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* In: EMSF_Q4_COMPLETION or _TRANSMISSION */
    short trans_id,             /* ID of translator to call */
    short FAR* selected,        /* Returned flag - is trans selected? */
    char FAR*FAR* properties    /* ASCII string encoded properties */
);

//+++++ VERSION 3+++++++
/* ----- Progress Data ---------------------------- */
typedef struct emsProgressDataS FAR* emsProgressDataP;
typedef struct emsProgressDataS {
    long			size;			/* Size of this data structure */
    long			value;			/* Range of Progress, percent complete */
    LPSTR			message;		/* Progress Message */
} emsProgressData;

/* ========== FUNCTIONS PASSED ACROSS THE API ============================ */
typedef short     (FAR* emsAllocate)(void **mem, size_t size);      


/* ----- NEW - structure to hold the callback functions ------------- */
/* Each callback has the same signature, one pointer to a parameter block */ 
typedef struct emsMBoxS FAR*emsMBoxP;
typedef struct emsMBoxS {
    long			size;			/* Size of this data structure */
	LPSTR			name;
	long			hash;
} emsMBox;



/* ----- NEW - GetMailBoxFunction Data ---------------------------- */
typedef struct emsGetMailBoxDataS *emsGetMailBoxDataP;
typedef struct emsGetMailBoxDataS{
    long                size;           /* IN: Size of this data structure */
	long                flags;          /* IN: see flags above */
    LPSTR               prompt;         /* IN: Prompt for user */
    emsMBoxP			mailbox;        /* OUT: the chosen mailbox */
} emsGetMailBoxData;

/* ----- User Address ---------------------------- */
typedef struct emsAddressS FAR*emsAddressP; 
typedef struct emsAddressS {
    long			size;		/* Size of this data structure */
    LPSTR			address;	/* Optional directory for config file */
    LPSTR			realname;	/* Users full name from Eudora config */
    emsAddressP		next;	/* Linked list of addresses */
} emsAddress;

/* ----- NEW - GetPersonalityData ---------------------------- */
typedef struct emsGetPersonalityDataS *emsGetPersonalityDataP;
typedef struct emsGetPersonalityDataS{
    long                size;           /* IN: Size of this data structure */
    LPSTR               prompt;         /* IN: Prompt for user, Set to NULL for standard */
	BOOL                defaultPers;    /* IN: set to 1 for defause, otherwise selection dialog comes up */
    long				persCount;		/* OUT: Number of personalities */
	emsAddressP         personality;	/* OUT: the chosen personality */
} emsGetPersonalityData;

/* ----- NEW - GetPersonalityInfo ---------------------------- */
typedef struct emsGetPersonalityInfoDataS *emsGetPersonalityInfoDataP;
typedef struct emsGetPersonalityInfoDataS{
    long                size;           /* IN: Size of this data structure */
    LPSTR               personalityName;/* IN: name of Personality */
	emsAddressP         personality;	/* OUT: the chosen personality */
} emsGetPersonalityInfoData;

/* ----- NEW - SetMailBoxTag Data ---------------------------- */

typedef struct emsSetMailBoxTagDataS *emsSetMailBoxTagDataP, **emsSetMailBoxTagDataH;
typedef struct emsSetMailBoxTagDataS {
    long                size;           /* IN: Size of this data structure */
    emsMBoxP			mailbox;        /* IN: the selected mailbox */
    long                key;            /* IN: the attribute key (usually the plug-in's ID */
    long                value;          /* IN: the attribute value, zero to clear the attribute */
    long                oldvalue;       /* OUT: the attribute's old value, zero by default */
} emsSetMailBoxTagData;


/* ----- NEW - Type of Updated Files -------------------------- */
typedef enum { emsRegenerateFilters, emsRegenerateNicknames} 
RegenerateType;

/* ----- Regenerate Data ---------------------------- */
typedef struct emsRegenerateDataS FAR*emsRegenerateDataP; 
typedef struct emsRegenerateDataS {
    long			size;					/* Size of this data structure */
    RegenerateType	which;					/* What type of file this is */
	char			path[_MAX_PATH + 1];	/* The File Path */
} emsRegenerateData;

/* ----- GetDirectory ---------------------------- */

typedef enum { 
	EMS_EudoraDir,				/* Eudora folder, contains all otherfolders */
	EMS_AttachmentsDir,			/* Attachments folder */
	EMS_PluginFiltersDir,		/* Filters folder for plug-ins */
	EMS_PluginNicknamesDir,		/* Nicknames folder for plug-ins */
	EMS_ConfigDir,				/* Folder for plug-ins' preferences */
	EMS_MailDir,				/* Folder containing mailboxes */
	EMS_NicknamesDir,			/* Eudora nicknames folder */
	EMS_SignaturesDir,			/* Signatures folder */
	EMS_SpoolDir,				/* Spool folder */
	EMS_StationeryDir			/* Stationery folder */
} DirectoryEnum;

typedef struct emsGetDirectoryDataS FAR*emsGetDirectoryDataP;
typedef struct emsGetDirectoryDataS{
    long                size;					/* IN: Size of this datastructure */
    DirectoryEnum		which;					/* IN: Which directory? */
    char				directory[_MAX_PATH + 1];	/* OUT: Path for directory */
} emsGetDirectoryData;

typedef struct emsNicknameS FAR*emsNicknameP;
typedef struct emsNicknameS{

	LPSTR		name;				// Name
	LPSTR		addresses;			// Addresses
	LPSTR		notes;				// Notes
	
} emsNickname;

typedef struct ABDataS FAR*ABDataP;
typedef struct ABDataS{
	
	LPSTR name;
	ABDataP	next;

} ABData;

/* ========== FUNCTIONS PASSED ACROSS THE API ============================ */
typedef short			(FAR* emsProgress3)(emsProgressDataP);	/* The progress function  */
typedef short			(FAR* emsAllocate)(void **, size_t);     /* The progress function  */
typedef short			(FAR* emsGetMailBox)(emsAllocate, emsGetMailBoxDataP);     
typedef short			(FAR* emsSetMailBoxTag)(emsSetMailBoxTagDataP);     
typedef short			(FAR* emsGetMailBoxTag)(emsSetMailBoxTagDataP);
typedef short			(FAR* emsGetPersonality)(emsAllocate, emsGetPersonalityDataP);      
typedef short			(FAR* emsRegenerate)(emsRegenerateDataP);      
typedef short			(FAR* emsGetDirectory)(emsGetDirectoryDataP);  
typedef short			(FAR* emsGetPersonalityInfo)(emsAllocate, emsGetPersonalityInfoDataP); 
typedef ABDataP			(FAR* emsEnumAddressBooks)(void);
typedef short			(FAR* emsAddABEntry)(emsNicknameP, LPSTR);

/* ========== DATA STRUCTURES ============================================ */
/* All strings on Windows are NULL terminated C strings */


/* ----- MIME Params ---------------------------- */
typedef struct emsMIMEparam3S FAR*emsMIMEparam3P;
typedef struct emsMIMEparam3S {
    long			size;
    LPSTR			name;		/* Mime parameter name (e.g., charset) */
    LPSTR			value;		/* param value (e.g. us-ascii) */
    emsMIMEparam3P  next;		/* Linked list of parameters */
} emsMIMEparam3;

/* ----- MIME Info ---------------------------- */
typedef struct emsMIMEtype3S FAR*emsMIMEtype3P; 
typedef struct emsMIMEtype3S {
    long			size;
    LPSTR			version;	/* The MIME-Version header */
    LPSTR			type;		/* Top-level MIME type */      
    LPSTR			subType;	/* MIME sub-type */
    emsMIMEparam3P	params;		/* MIME parameter list */
    LPSTR       	contentDisp;  	/* Content-Disposition */
    emsMIMEparam3P  contentParams; 	/* Handle to first parameter in list */
} emsMIMEtype3;


/* ----- Header Data ---------------------------- */
typedef struct emsHeaderDataS FAR*emsHeaderDataP; 
typedef struct emsHeaderDataS {
    long			size;		/* Size of this data structure */
    emsAddressP		to;			/* To Header */
    emsAddressP		from;		/* From Header */
    LPSTR           subject;    /* Subject Header */
    emsAddressP		cc;			/* cc Header */
    emsAddressP		bcc;		/* bcc Header */
    LPSTR			rawHeaders;	/* The 822 headers */
} emsHeaderData;



/* ----- NEW - structure to hold the callback functions ------------- */
/* Each callback has the same signature, one pointer to a parameter block */ 
typedef struct emsCallBackS FAR*emsCallBacksP;
typedef struct emsCallBackS {
    long							size;				/* Size of this data structure */
	emsProgress3					EMSProgressCB;
    emsGetMailBox					EMSGetMailBoxCB;
    emsSetMailBoxTag				EMSSetMailBoxTagCB;
    emsGetPersonality				EMSGetPersonalityCB;
	emsRegenerate					EMSRegenerateCB;
	emsGetDirectory					EMSGetDirectoryCB;
	emsGetMailBoxTag				EMSGetMailBoxTagCB;
	emsGetPersonalityInfo			EMSGetPersonalityInfoCB;
	emsEnumAddressBooks				EMSEnumAddressBooksCB;
	emsAddABEntry					EMSAddABEntryCB;
} emsCallBack;

/* ----- How Eudora is configured ---------------------------- */
typedef struct emsMailConfigS FAR*emsMailConfigP; 
typedef struct emsMailConfigS {
    long			size;		/* Size of this data structure */
    HWND			*eudoraWnd;	/* Eudora's main window */
    LPSTR			configDir;	/* Optional directory for config file */
    emsAddress		userAddr;	/* Users full name from Eudora config */
    emsCallBacksP	callBacks;	/* V4! Peanut - Pointer to callback structure */
} emsMailConfig;

/* ----- Plugin Info ---------------------------- */
typedef struct emsPluginInfoS FAR*emsPluginInfoP; 
typedef struct emsPluginInfoS {
    long			size;			/* Size of this data structure */
    long			numTrans;		/* Place to return num of translators */
    long			numAttachers;	/* Place to return num of attach hooks */
    long			numSpecials;	/* Place to return num of special hooks */
    LPSTR			desc;			/* Return for string description of plugin */
    long			id;				/* Place to return unique plugin id */
    HICON			FAR*icon;		/* Return for plugin icon data */
    long			mem_rqmnt;		/* V4! Return Memory Required to run this */
    long            numMBoxContext; /* V4! Peanut Place to return num of mailbox context hooks */
                                    /* These are shown when the mailbox has a non-zer attribute with the plugin-s ID as key */
    long            idleTimeFreq;   /* V4! Peanut Return 0 for no idle time, otherwise initial idle frequentcy in milliseconds */
} emsPluginInfo;

/* ----- Translator Info ---------------------------- */
typedef struct emsTranslatorS FAR*emsTranslatorP; 
typedef struct emsTranslatorS {
    long			size;		/* Size of this data structure */
    long			id;			/* ID of translator to get info for */
    long			type;		/* translator type, e.g., EMST_xxx */
    ULONG			flags;		/* translator flags */
    LPSTR			desc;		/* translator string description */
    HICON			FAR*icon;	/* Return for plugin icon data */
    LPSTR			properties;	/* Properties for queued translations */
} emsTranslator;

/* ----- Menu Item Info ---------------------------- */
typedef struct emsMenuS FAR*emsMenuP; 
typedef struct emsMenuS {
    long			size;		/* Size of this data structure */
    long			id;			/* ID of translator to get info for */
    LPSTR			desc;		/* translator string description */
    HICON			FAR*icon;	/* Return for plugin icon data */
    long			flags;		/* any special flags*/
} emsMenu;

/* ----- Translation Data ---------------------------- */
typedef struct emsDataFileS FAR*emsDataFileP; 
typedef struct emsDataFileS {
    long				size;		/* Size of this data structure */
    long				context;
    emsMIMEtype3P		info;		/* MIME type of data to check */
    emsHeaderDataP		header; 	/* EMSF_BASIC_HEADERS & EMSF_ALL_HEADERS determine contents */
    LPSTR				fileName;	/* The input file name */
} emsDataFile;

/* ----- Resulting Status Data ---------------------------- */
typedef struct emsResultStatusS FAR*emsResultStatusP; 
typedef struct emsResultStatusS {
    long			size;			/* Size of this data structure */
    LPSTR			desc;			/* Returned string for display with the result */
    LPSTR			error;			/* Place to return string with error message */
    long			code;			/* Return for translator-specific result code */
} emsResultStatus;




/* ----- Resulting Status Data ---------------------------- */
typedef struct emsIdleDataS FAR*emsIdleDataP; 
typedef struct emsIdleDataS {
    long			size;			/* Size of this data structure */
    long			flags;			/* Idle Flags */
    long			idleTimeFreq;	/* In/Out: current->new request idle */
    emsProgress3	Progress;		/* Progress Func */
} emsIdleData;


/* ========== FUNCTION PROTOTYPES ======================================== */


/* ----- Initialize plugin and get its basic info -------------------------- */
typedef long WINAPI ems3_plugin_init_t(
    void FAR*FAR* globals,          /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
);

/* ----- Get details about a translator in a plugin ------------------------ */
typedef long WINAPI ems3_translator_info_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
);

/* ----- Check and see if a translation can be performed ------------------- */
typedef long WINAPI ems3_can_translate_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inTransData,   /* In: What to translate */
    emsResultStatusP transStatus /* Out: Translations Status information */
);

/* ----- Actually perform a translation on a file -------------------------- */
typedef long WINAPI ems3_translate_file_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inFile,        /* In: What to translate */
    emsProgress3 progress,       /* Func to report progress/check for abort */
    emsDataFileP outFile,       /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
);

/* ----- End use of a plugin and clean up ---------------------------------- */
typedef long WINAPI ems3_plugin_finish_t(
    void FAR* globals           /* Out: Return for allocated instance structure */
);

/* ----- Free memory allocated by EMS plug-in ------------------------------ */
typedef long WINAPI ems3_free_t(
    void FAR* mem               /* Memory to free */
);

/* ----- Allocate memory allocated by EMS plug-in ------------------------------ */
typedef long WINAPI ems_alloc_t(
    void FAR* mem,
	size_t size
);

/* ----- Call the plug-ins configuration Interface ------------------------- */
typedef long WINAPI ems3_plugin_config_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMailConfigP mailConfig   /* In: Eudora mail info */
);

/* ----- Manage properties for queued translations ------------------------- */
typedef long WINAPI ems3_queued_properties_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In/Out: The translator */
    long FAR*selected            /* In/Out: State of this translator */
);

/* ----- Info about menu hook to attach/insert composed object ------------- */
typedef long WINAPI ems3_attacher_info_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP attachMenu         /* Out: The menu */
);

/* ----- Call an attacher hook to compose some special object -------------- */
typedef long WINAPI ems3_attacher_hook_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP attachMenu,        /* In: The menu */
    LPSTR attachDir,            /* In: Location to put attachments */
    long * numAttach,           /* Out: Number of files attached */
    emsDataFileP ** attachFiles /* Out: Name of files written */
);

/* ----- Info about special menu items hooks ------------------------------- */
typedef long WINAPI ems3_special_info_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* Out: The menu */
);

/* ----- Call a special menu item hook ------------------------------------- */
typedef long WINAPI ems3_special_hook_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* In: The menu */
);

/* ----- idle function ----------------------------------------------------- */
typedef long WINAPI ems_idle_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsIdleDataP idleData       /* In/Out: idle omfp */
);


/* ----- info about the context menu hook ---------------------------------- */
typedef long WINAPI ems_mbox_context_info_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMenuP mboxMenu			/* Out: The menu */
);

/* ----- a mailbox context menu hook --------------------------------------- */
typedef long WINAPI ems_mbox_context_hook_t(
    void FAR* globals,          /* Out: Return for allocated instance structure */
    emsMBoxP mailbox,        /* Out: The Chosen Mailbox */
    emsMenuP mboxMenu			/* In: The menu */
);


