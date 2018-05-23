 /* ======================================================================

    C header for EMSAPI version 4
    Copyright 1996, 1997 QUALCOMM Inc.
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

    Generated: Mon Aug  4 09:18:37 PDT 1997
    Filename: emsapi-MacGlue.h

    THIS IS TO BUILD THE EUDORA APPLICATION SIDE OF THE API. DON'T
    USE THIS TO BUILD A TRANSLATOR OR COMPONENT

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be changed.

 */
#ifndef EMSAPI_MACGLUE_H_INCLUDED
#define EMSAPI_MACGLUE_H_INCLUDED

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
#define EMSR_NOT_INTERESTED (13L)     /* V4! Peanut Not now, but not in DISPLAY? */
#define EMSR_USER_CANCELLED	(14L)			/* V4 User cancelled */
 
 
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

#define EMSF_DEFAULT_Q_ON    (0x4000L)	/* Causes queued translation to be on 
                                          for a new message by default */
#define EMSF_TOOLBAR_PRESENCE (0x8000L)	/* Appear on the Toolbar */
#define EMSF_ALL_TEXT        (0x10000L) /* All text instead of current selection for ON_REQUEST */
#define EMSF_DONTSAVE    (0x80000000)	/* Mark messages as unchanged so user 
					is not prompted for save.  Add to any 
					return code */


/* ----- The version of the API defined by this include file ------------- */
#define EMS_VERSION          (5)       /* Used in plugin init */
#define EMS_PB_VERSION       (3)       /* Minimum version that uses parameter blocks */
#define EMS_COMPONENT        'EuTL'    /* Macintosh component type */


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
#define EMIDLE_QUICK				(2L)      /* Now is NOT the time to do something lengthy */
#define EMIDLE_OFFLINE			(4L)      /* Eudora is in "offline" mode */
#define EMIDLE_PRE_SEND			(8L)  		/* Eudora is about to send mail */
#define EMIDLE_TRANSFERRING	(16L)  		/* Eudora is transferring mail */


/* ----- NEW - Values for emsStandardGetMailBox flags -------------------------- */
#define EMSFGETMBOX_ALLOW_NEW		(1L)      /* Allow creation of new mailboxes */
#define EMSFGETMBOX_ALLOW_OTHER		(2L)      /* Allow selection of "other" mailboxes */


#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/* ========== set up for call backs ======================================*/
typedef struct emsProgressDataS *emsProgressDataP, **emsProgressDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsProgress;
enum { uppemsProgressProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsProgressDataP))) };
#else
typedef pascal short (*emsProgress)(emsProgressDataP progData); 
#endif
typedef emsProgress emsProgressUPP;

typedef struct emsStandardGetMailBoxDataS *emsStandardGetMailBoxDataP, **emsStandardGetMailBoxDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsGetMailBox;
enum { uppemsGetMailBoxProcInfo = kPascalStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsStandardGetMailBoxDataP))) };
#else
typedef pascal void (*emsGetMailBox)(emsStandardGetMailBoxDataP getMailBoxData); 
#endif
typedef emsGetMailBox emsGetMailBoxUPP;

typedef struct emsSetMailBoxTagDataS *emsSetMailBoxTagDataP, **emsSetMailBoxTagDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsSetMailBoxTag;
enum { uppemsSetMailBoxTagProcInfo = kPascalStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsSetMailBoxTagDataP))) };
#else
typedef pascal void (*emsSetMailBoxTag)(emsSetMailBoxTagDataP setMailBoxTagData); 
#endif
typedef emsSetMailBoxTag emsSetMailBoxTagUPP;
#if GENERATINGCFM
typedef UniversalProcPtr emsGetMailBoxTag;
enum { uppemsGetMailBoxTagProcInfo = kPascalStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsSetMailBoxTagDataP))) };
#else
typedef pascal void (*emsGetMailBoxTag)(emsSetMailBoxTagDataP setMailBoxTagData); 
#endif
typedef emsGetMailBoxTag emsGetMailBoxTagUPP;

typedef struct emsGetPersonalityDataS *emsGetPersonalityDataP, **emsGetPersonalityDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsGetPersonality;
enum { uppemsGetPersonalityProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsGetPersonalityDataP))) };
#else
typedef pascal short (*emsGetPersonality)(emsGetPersonalityDataP getPersonalityData);
#endif
typedef emsGetPersonality emsGetPersonalityUPP;

typedef struct emsGetPersonalityInfoDataS *emsGetPersonalityInfoDataP, **emsGetPersonalityInfoDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsGetPersonalityInfo;
enum { uppemsGetPersonalityInfoProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsGetPersonalityInfoDataP))) };
#else
typedef pascal short (*emsGetPersonalityInfo)(emsGetPersonalityInfoDataP getPersonalityData);
#endif
typedef emsGetPersonalityInfo emsGetPersonalityInfoUPP;

typedef struct emsRegenerateDataS *emsRegenerateDataP, **emsRegenerateDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsRegenerate;
enum { uppemsRegenerateProcInfo = kPascalStackBased
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsRegenerateDataP))) };
#else
typedef pascal short (*emsRegenerate)(emsRegenerateDataP RegenerateData);
#endif
typedef emsRegenerate emsRegenerateUPP;

typedef struct emsGetDirectoryDataS *emsGetDirectoryDataP, **emsGetDirectoryDataH;
#if GENERATINGCFM
typedef UniversalProcPtr emsGetDirectory;
enum { uppemsGetDirectoryProcInfo = kPascalStackBased
        | RESULT_SIZE(SIZE_CODE(sizeof(short)))
        | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(emsGetDirectoryDataP))) };
#else
typedef pascal short (*emsGetDirectory)(emsGetDirectoryDataP GetDirectoryData);
#endif
typedef emsGetDirectory emsGetDirectoryUPP;

#if GENERATINGCFM
typedef UniversalProcPtr emsUpdateWindows;
enum { uppemsUpdateWindowsProcInfo = kPascalStackBased };
#else
typedef pascal void (*emsUpdateWindows)(void);
#endif
typedef emsUpdateWindows emsUpdateWindowsUPP;

/* ========== DATA STRUCTURES ============================================ */
/* True Mac-style declarations aren't used yet but are included in comments. 
   All strings on the Mac are PASCAL strings and all are allocated as Handles. 
 */
/* ----- Macintosh: MIME type data passed across the API ----------------- */
/* ----- MIME Params ---------------------------- */
typedef struct emsMIMEparamS *emsMIMEParamP, **emsMIMEparamH;
typedef struct emsMIMEparamS {
    long		size;
    Str63		name;    	/* MIME parameter name */
    Handle		value;  	/* handle size determines string length */
    emsMIMEparamH       next;  	        /* Handle for next param in list */
} emsMIMEparam;
/* ----- MIME Data ----------------------------- */
typedef struct emsMIMEtypeS *emsMIMEtypeP, **emsMIMEtypeH; 
typedef struct emsMIMEtypeS {
    long	       size;
    Str63              mimeVersion; 	/* MIME-Version: header */
    Str63              mimeType;   	/* Top-level MIME type: text,message...*/
    Str63              subType;     	/* sub-type */
    emsMIMEparamH      params;      	/* Handle to first parameter in list */
    Str63              contentDisp;  	/* Content-Disposition */
    emsMIMEparamH      contentParams;  	/* Handle to first parameter in list */
} emsMIMEtype;
/* ----- User Address ------------------------- */
typedef struct emsAddressS *emsAddressP, **emsAddressH; 
typedef struct emsAddressS {
    long		size;		/* Size of this data structure */
    StringHandle	address;	/* Optional directory for config file */
    StringHandle	realname;	/* Users full name from Eudora config */
    emsAddressH		next;		/* Linked list of addresses */
} emsAddress;
/* ----- Header Data ---------------------------- */
typedef struct emsHeaderDataS *emsHeaderDataP, **emsHeaderDataH;
typedef struct emsHeaderDataS {
    long		size;		/* Size of this data structure */
    emsAddressH		to;		/* To Header */
    emsAddressH		from;		/* From Header */
    StringPtr           *subject;       /* Subject Header */
    emsAddressH		cc;		/* cc Header */
    emsAddressH		bcc;		/* bcc Header */
    Handle		rawHeaders;	/* The 822 headers */
} emsHeaderData;
/* ----- NEW - structure to hold the callback functions ------------- */
typedef struct emsCallBackS {
    emsGetMailBox     	EMSStandardGetMailBoxFunction;
    emsSetMailBoxTag    EMSSetMailBoxTagFunction;
    emsGetPersonality   EMSStandardGetPersonalityFunction;
    emsProgress					EMSProgressFunction;
    emsRegenerate				EMSRegenerateFunction;
    emsGetDirectory     EMSGetDirectoryFunction;
    emsUpdateWindows		EMSUpdateWindowsFunction;
    emsGetMailBoxTag    EMSGetMailBoxTagFunction;
    emsGetPersonalityInfo    EMSGetPersonalityInfoFunction;
} emsCallBack;

/* ----- How Eudora is configured ------------- */
typedef struct emsCallBackS *emsCallBacksP, **emsCallBacksH;
typedef struct emsMailConfigS *emsMailConfigP, **emsMailConfigH; 
typedef struct emsMailConfigS {
    long		size;		/* Size of this data structure */
    FSSpec		configDir;	/* Optional directory for config file */
    emsAddress		userAddr;	/* Current users address */
    emsCallBack	callBacks;	/* V4! Peanut - Pointer to callback structure */
} emsMailConfig;
/* ----- Plugin Info -------------------------- */
typedef struct emsPluginInfoS *emsPluginInfoP, **emsPluginInfoH; 
typedef struct emsPluginInfoS {
    long		size;		/* Size of this data structure */
    long		id;		/* Place to return unique plugin id */
    long		numTrans;	/* Place to return num of translators */
    long		numAttachers;	/* Place to return num of attach hooks */
    long		numSpecials;	/* Place to return num of special hooks */
    StringHandle	desc;		/* Return for string description of plugin */
    Handle		icon;		/* Return for plugin icon data */
    long		mem_rqmnt;	/* V4! Return Memory Required to run this */
    long                numMBoxContext; /* V4! Peanut Place to return num of mailbox context hooks */
                                        /* These are shown when the mailbox has a non-zer attribute with the plugin-s ID as key */
    long                idleTimeFreq;   /* V4! Peanut Return 0 for no idle time, otherwise initial idle frequentcy in milliseconds */
} emsPluginInfo;
/* ----- Translator Info --------------------- */
typedef struct emsTranslatorS *emsTranslatorP, **emsTranslatorH; 
typedef struct emsTranslatorS {
    long		size;		/* Size of this data structure */
    long		id;		/* ID of translator to get info for */
    long		type;		/* translator type, e.g., EMST_xxx */
    unsigned long	flags;		/* translator flags */
    StringHandle	desc;		/* translator string description */
    Handle		icon;		/* translator icon data */
    StringHandle	properties;	/* Properties for queued translations */
} emsTranslator;
/* ----- Menu Item Info --------------------- */
typedef struct emsMenuS *emsMenuP, **emsMenuH; 
typedef struct emsMenuS {
    long		size;		/* Size of this data structure */
    long		id;		/* ID of menu item to get info for */
    StringHandle	desc;		/* translator string description */
    Handle		icon;		/* Icon suite */
    unsigned long	flags;		/* flags (EMSF_TOOLBAR_PRESENCE) */
} emsMenu;
/* ----- Translation Data -------------------- */
typedef struct emsDataFileS *emsDataFileP, **emsDataFileH; 
typedef struct emsDataFileS {
    long		size;		/* Size of this data structure */
    long		context;
    emsMIMEtypeH	mimeInfo;	/* MIME type of data to check */
    emsHeaderDataP      header;  	/* EMSF_BASIC_HEADERS & EMSF_ALL_HEADERS determine contents */
    FSSpec		file;		/* The input file name */
} emsDataFile;
/* ----- Resulting Status Data -------------- */
typedef struct emsResultStatusS *emsResultStatusP, **emsResultStatusH; 
typedef struct emsResultStatusS {
    long		size;		/* Size of this data structure */
    StringHandle	desc;		/* Returned string for display with the result */
    StringHandle	error;		/* Place to return string with error message */
    long		code;		/* Return for translator-specific result code */
} emsResultStatus;
/* ----- Progress Data ---------------------------- */
typedef struct emsProgressDataS {
    long		size;		/* Size of this data structure */
    long		value;		/* Range of Progress, percent complete */
    StringPtr		message;	/* Progress Message */
} emsProgressData;
/* ----- NEW - StandardGetMailBoxFunction Data ---------------------------- */
typedef AliasHandle emsMBoxP;
typedef struct emsStandardGetMailBoxDataS {
    long                size;           /* IN: Size of this data structure */
    long                flags;          /* IN: see flags above */
    StringPtr           prompt;         /* IN: Prompt for user */
    emsMBoxP            mailbox;        /* OUT: the chosen mailbox */
} emsStandardGetMailBoxData;
/* ----- NEW - SetMailBoxTag Data ---------------------------- */
typedef struct emsSetMailBoxTagDataS{
    long                size;           /* IN: Size of this data structure */
    emsMBoxP            mailbox;        /* IN: the selected mailbox */
    long                key;            /* IN: the attribute key (usually the plug-in's ID */
    long                value;          /* IN: the attribute value, zero to clear the attribute */
    long                oldvalue;       /* OUT: the attribute's old value, zero by default */
    long                oldkey;       	/* OUT: the attribute's old key, zero by default */
} emsSetMailBoxTagData;
/* ----- NEW - GetPersonality Data ---------------------------- */
typedef struct emsGetPersonalityDataS{
    long                size;           /* IN: Size of this data structure */
    StringPtr           prompt;         /* IN: Prompt for user. Set to nil for standard prompt */
    Boolean             defaultPers;    /* IN: Return default personality */
    short               persCount;      /* OUT: number of personalities available */
    emsAddress          personality;    /* OUT: Address info for selected personality */
    StringHandle        personalityName;/* OUT: name of personality */
} emsGetPersonalityData;
/* ----- NEW - GetPersonalityInfo Data ---------------------------- */
typedef struct emsGetPersonalityInfoDataS{
    long                size;           /* IN: Size of this data structure */
    StringHandle        personalityName;/* IN: name of personality */
    emsAddress          personality;    /* OUT: Address info for selected personality */
} emsGetPersonalityInfoData;
/* ----- NEW - Regenerate Data ---------------------------- */
typedef enum { emsRegenerateFilters,emsRegenerateNicknames } RegenerateType;
typedef struct emsRegenerateDataS{
    long                size;           /* IN: Size of this data structure */
    RegenerateType			which;					/* IN: What should Eudora regenerate? */
} emsRegenerateData;
/* ----- NEW - Get Directory Data ---------------------------- */
typedef enum { EMS_EudoraDir,					/* Eudora folder, contains all other folders */
						EMS_AttachmentsDir,				/* Attachments folder */
						EMS_PluginFiltersDir,			/* Filters folder for plug-ins */
						EMS_PluginNicknamesDir,		/* Nicknames folder for plug-ins */
						EMS_ConfigDir,						/* Folder for plug-ins' preferences */
						EMS_MailDir,							/* Folder containing mailboxes */
						EMS_NicknamesDir,					/* Eudora nicknames folder */
						EMS_SignaturesDir,				/* Signatures folder */
						EMS_SpoolDir,							/* Spool folder */
						EMS_StationeryDir					/* Stationery folder */
					} DirectoryEnum;
typedef struct emsGetDirectoryDataS{
    long                size;           /* IN: Size of this data structure */
    DirectoryEnum		    which;					/* IN: Which directory? */
    FSSpec							directory;			/* OUT: FileSpec for directory */
} emsGetDirectoryData;
/* ----- NEW - Idle Data ---------------------------- */
typedef struct emsIdleDataS{
    long								flags;					/* IN: flags */
    long								*idleTimeFreq;	/* IN,OUT: requested idle time */
    emsProgress					progress;				/* IN: Callback function to report progress/check for abort */
} emsIdleData, *emsIdleDataP;


/* ========== ENTRY POINT ENUM CONSTANTS ==================================*/

enum {
    kems_plugin_versionRtn = 256,
    kems_plugin_initRtn = 257,
    kems_translator_infoRtn = 258,
    kems_can_translateRtn = 259,
    kems_translate_fileRtn = 260,
    kems_plugin_finishRtn = 261,
    kems_plugin_configRtn = 262,
    kems_queued_propertiesRtn = 263,
    kems_attacher_infoRtn = 264,
    kems_attacher_hookRtn = 265,
    kems_special_infoRtn = 266,
    kems_special_hookRtn = 267,
    kems_idleRtn = 268,
    kems_mbox_context_infoRtn = 269,
    kems_mbox_context_hookRtn = 270
};


/* ========== FUNCTION PROTOTYPES ==========================================*/
/* The following is so the same headers can be used for component routines. */
/* FIVEWORDINLINE is magic inline stuff for 68K application and nada for    */
/* PPC */
#define CallemsComponent(callType) \
    FIVEWORDINLINE(0x2F3C,\
                   sizeof(ems_##callType##Params)-sizeof(ComponentInstance),\
                   kems_##callType##Rtn,0x7000,0xA82A)



#define EMSGCPCALL(x) \
    (ems_GCP.what = kems_##x##Rtn, \
     ems_GCP.paramSize = (sizeof(ems_##x##Params)-sizeof(ComponentInstance)), \
     ems_GCP.flags = 0, \
     Callems_ComponentFunction(ems_GCP))




/* ----- Get the API Version number this plugin implements ----------------- */
typedef struct ems_plugin_versionParams {
    short *apiVersion;
    Handle globals;
} ems_plugin_versionParams;

pascal ComponentResult ems_plugin_version(
    Handle globals,             /* Out: Return for allocated instance structure */
    short *apiVersion           /* Out: Plugin Version */
)
CallemsComponent(plugin_version);

#if GENERATINGCFM
#define ems_plugin_version(p1,p2)\
    (\
        ems_GCP.params.ems_plugin_version.globals = p1,\
        ems_GCP.params.ems_plugin_version.apiVersion = p2,\
    EMSGCPCALL(plugin_version))
#endif



/* ----- Initialize plugin and get its basic info -------------------------- */
typedef struct ems_plugin_initParams {
    emsPluginInfoP pluginInfo;
    emsMailConfigP mailConfig;
    short eudAPIVersion;
    Handle globals;
} ems_plugin_initParams;

pascal ComponentResult ems_plugin_init(
    Handle globals,             /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
CallemsComponent(plugin_init);

#if GENERATINGCFM
#define ems_plugin_init(p1,p2,p3,p4)\
    (\
        ems_GCP.params.ems_plugin_init.globals = p1,\
        ems_GCP.params.ems_plugin_init.eudAPIVersion = p2,\
        ems_GCP.params.ems_plugin_init.mailConfig = p3,\
        ems_GCP.params.ems_plugin_init.pluginInfo = p4,\
    EMSGCPCALL(plugin_init))
#endif



/* ----- Get details about a translator in a plugin ------------------------ */
typedef struct ems_translator_infoParams {
    emsTranslatorP transInfo;
    Handle globals;
} ems_translator_infoParams;

pascal ComponentResult ems_translator_info(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
)
CallemsComponent(translator_info);

#if GENERATINGCFM
#define ems_translator_info(p1,p2)\
    (\
        ems_GCP.params.ems_translator_info.globals = p1,\
        ems_GCP.params.ems_translator_info.transInfo = p2,\
    EMSGCPCALL(translator_info))
#endif



/* ----- Check and see if a translation can be performed ------------------- */
typedef struct ems_can_translateParams {
    emsResultStatusP transStatus;
    emsDataFileP inTransData;
    emsTranslatorP trans;
    Handle globals;
} ems_can_translateParams;

pascal ComponentResult ems_can_translate(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inTransData,   /* In: What to translate */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
CallemsComponent(can_translate);

#if GENERATINGCFM
#define ems_can_translate(p1,p2,p3,p4)\
    (\
        ems_GCP.params.ems_can_translate.globals = p1,\
        ems_GCP.params.ems_can_translate.trans = p2,\
        ems_GCP.params.ems_can_translate.inTransData = p3,\
        ems_GCP.params.ems_can_translate.transStatus = p4,\
    EMSGCPCALL(can_translate))
#endif



/* ----- Actually perform a translation on a file -------------------------- */
typedef struct ems_translate_fileParams {
    emsResultStatusP transStatus;
    emsDataFileP outFile;
    emsProgress progress;
    emsDataFileP inFile;
    emsTranslatorP trans;
    Handle globals;
} ems_translate_fileParams;

pascal ComponentResult ems_translate_file(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In: Translator Info */
    emsDataFileP inFile,        /* In: What to translate */
    emsProgress progress,       /* Func to report progress/check for abort */
    emsDataFileP outFile,       /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
CallemsComponent(translate_file);

#if GENERATINGCFM
#define ems_translate_file(p1,p2,p3,p4,p5,p6)\
    (\
        ems_GCP.params.ems_translate_file.globals = p1,\
        ems_GCP.params.ems_translate_file.trans = p2,\
        ems_GCP.params.ems_translate_file.inFile = p3,\
        ems_GCP.params.ems_translate_file.progress = p4,\
        ems_GCP.params.ems_translate_file.outFile = p5,\
        ems_GCP.params.ems_translate_file.transStatus = p6,\
    EMSGCPCALL(translate_file))
#endif



/* ----- End use of a plugin and clean up ---------------------------------- */
typedef struct ems_plugin_finishParams {
    Handle globals;
} ems_plugin_finishParams;

pascal ComponentResult ems_plugin_finish(
    Handle globals              /* Out: Return for allocated instance structure */
)
CallemsComponent(plugin_finish);

#if GENERATINGCFM
#define ems_plugin_finish(p1)\
    (\
        ems_GCP.params.ems_plugin_finish.globals = p1,\
    EMSGCPCALL(plugin_finish))
#endif



/* ----- Call the plug-ins configuration Interface ------------------------- */
typedef struct ems_plugin_configParams {
    emsMailConfigP mailConfig;
    Handle globals;
} ems_plugin_configParams;

pascal ComponentResult ems_plugin_config(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsMailConfigP mailConfig   /* In: Eudora mail info */
)
CallemsComponent(plugin_config);

#if GENERATINGCFM
#define ems_plugin_config(p1,p2)\
    (\
        ems_GCP.params.ems_plugin_config.globals = p1,\
        ems_GCP.params.ems_plugin_config.mailConfig = p2,\
    EMSGCPCALL(plugin_config))
#endif



/* ----- Manage properties for queued translations ------------------------- */
typedef struct ems_queued_propertiesParams {
    long *selected;
    emsTranslatorP trans;
    Handle globals;
} ems_queued_propertiesParams;

pascal ComponentResult ems_queued_properties(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsTranslatorP trans,       /* In/Out: The translator */
    long *selected              /* In/Out: State of this translator */
)
CallemsComponent(queued_properties);

#if GENERATINGCFM
#define ems_queued_properties(p1,p2,p3)\
    (\
        ems_GCP.params.ems_queued_properties.globals = p1,\
        ems_GCP.params.ems_queued_properties.trans = p2,\
        ems_GCP.params.ems_queued_properties.selected = p3,\
    EMSGCPCALL(queued_properties))
#endif



/* ----- Info about menu hook to attach/insert composed object ------------- */
typedef struct ems_attacher_infoParams {
    emsMenuP attachMenu;
    Handle globals;
} ems_attacher_infoParams;

pascal ComponentResult ems_attacher_info(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsMenuP attachMenu         /* Out: The menu */
)
CallemsComponent(attacher_info);

#if GENERATINGCFM
#define ems_attacher_info(p1,p2)\
    (\
        ems_GCP.params.ems_attacher_info.globals = p1,\
        ems_GCP.params.ems_attacher_info.attachMenu = p2,\
    EMSGCPCALL(attacher_info))
#endif



/* ----- Call an attacher hook to compose some special object -------------- */
typedef struct ems_attacher_hookParams {
    emsDataFileH *attachFiles;
    long *numAttach;
    FSSpec *attachDir;
    emsMenuP attachMenu;
    Handle globals;
} ems_attacher_hookParams;

pascal ComponentResult ems_attacher_hook(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsMenuP attachMenu,        /* In: The menu */
    FSSpec *attachDir,          /* In: Location to put attachments */
    long *numAttach,            /* Out: Number of files attached */
    emsDataFileH *attachFiles   /* Out: Name of files written */
)
CallemsComponent(attacher_hook);

#if GENERATINGCFM
#define ems_attacher_hook(p1,p2,p3,p4,p5)\
    (\
        ems_GCP.params.ems_attacher_hook.globals = p1,\
        ems_GCP.params.ems_attacher_hook.attachMenu = p2,\
        ems_GCP.params.ems_attacher_hook.attachDir = p3,\
        ems_GCP.params.ems_attacher_hook.numAttach = p4,\
        ems_GCP.params.ems_attacher_hook.attachFiles = p5,\
    EMSGCPCALL(attacher_hook))
#endif



/* ----- Info about special menu items hooks ------------------------------- */
typedef struct ems_special_infoParams {
    emsMenuP specialMenu;
    Handle globals;
} ems_special_infoParams;

pascal ComponentResult ems_special_info(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* Out: The menu */
)
CallemsComponent(special_info);

#if GENERATINGCFM
#define ems_special_info(p1,p2)\
    (\
        ems_GCP.params.ems_special_info.globals = p1,\
        ems_GCP.params.ems_special_info.specialMenu = p2,\
    EMSGCPCALL(special_info))
#endif



/* ----- Call a special menu item hook ------------------------------------- */
typedef struct ems_special_hookParams {
    emsMenuP specialMenu;
    Handle globals;
} ems_special_hookParams;

pascal ComponentResult ems_special_hook(
    Handle globals,             /* Out: Return for allocated instance structure */
    emsMenuP specialMenu        /* In: The menu */
)
CallemsComponent(special_hook);

#if GENERATINGCFM
#define ems_special_hook(p1,p2)\
    (\
        ems_GCP.params.ems_special_hook.globals = p1,\
        ems_GCP.params.ems_special_hook.specialMenu = p2,\
    EMSGCPCALL(special_hook))
#endif



/* -----   Get some idle time --------------------------------------------- */
typedef struct ems_idleParams {
		emsIdleDataP	idleData;
    Handle globals;
} ems_idleParams;

pascal ComponentResult ems_idle(
    Handle globals,             /* Out: Return for allocated instance structure */
		emsIdleDataP	idleData			/* In:  data */
)
CallemsComponent(idle);

#if GENERATINGCFM
#define ems_idle(p1,p2)\
    (\
        ems_GCP.params.ems_idle.globals = p1,\
        ems_GCP.params.ems_idle.idleData = p2,\
    EMSGCPCALL(idle))
#endif



/* -----   Handle                 globals         Out: Return for allocated instance structure  */
typedef struct ems_mbox_context_infoParams {
    emsMenuP mbox_context_Menu;
    Handle globals;
} ems_mbox_context_infoParams;

pascal ComponentResult ems_mbox_context_info(
    Handle globals,
    emsMenuP mbox_context_Menu    /* nu Out: The menu */
)
CallemsComponent(mbox_context_info );

#if GENERATINGCFM
#define ems_mbox_context_info(p1,p2)\
    (\
        ems_GCP.params.ems_mbox_context_info.globals = p1,\
        ems_GCP.params.ems_mbox_context_info.mbox_context_Menu = p2,\
    EMSGCPCALL(mbox_context_info))
#endif



/* -----   Handle                 globals         Out: Return for allocated instance structure  */
typedef struct ems_mbox_context_hookParams {
    emsMenuP mbox_context_Menu;
    emsMBoxP mailbox;
    Handle globals;
} ems_mbox_context_hookParams;

pascal ComponentResult ems_mbox_context_hook(
    Handle globals,
    emsMBoxP mailbox,           /* Out: The Chosen Mailbox */
    emsMenuP mbox_context_Menu    /* nu In: The menu */
)
CallemsComponent(mbox_context_hook);

#if GENERATINGCFM
#define ems_mbox_context_hook(p1,p2,p3)\
    (\
        ems_GCP.params.ems_mbox_context_hook.globals = p1,\
        ems_GCP.params.ems_mbox_context_hook.mailbox = p2,\
        ems_GCP.params.ems_mbox_context_hook.mbox_context_Menu = p3,\
    EMSGCPCALL(mbox_context_hook))
#endif



/* ========== UNION DEFINITION PPC only ====================================*/
#if GENERATINGCFM
typedef union ems_ComponentParamUnion
{
    ems_plugin_versionParams    ems_plugin_version;
    ems_plugin_initParams       ems_plugin_init;
    ems_translator_infoParams   ems_translator_info;
    ems_can_translateParams     ems_can_translate;
    ems_translate_fileParams    ems_translate_file;
    ems_plugin_finishParams     ems_plugin_finish;
    ems_plugin_configParams     ems_plugin_config;
    ems_queued_propertiesParams ems_queued_properties;
    ems_attacher_infoParams     ems_attacher_info;
    ems_attacher_hookParams     ems_attacher_hook;
    ems_special_infoParams      ems_special_info;
    ems_special_hookParams      ems_special_hook;
    ems_idleParams          		ems_idle;
    ems_mbox_context_infoParams ems_mbox_context_info;
    ems_mbox_context_hookParams ems_mbox_context_hook;
} ems_ComponentParamUnion;

typedef struct ems_ComponentParameters
{
	unsigned char flags;		/* Flags - set to zero */
	unsigned char paramSize;	/* Size of the params struct */
	short what;                     /* The component request selector */
	ems_ComponentParamUnion params;
} ems_ComponentParameters;


/* This global takes the routine parameters for each component function call */
extern ems_ComponentParameters ems_GCP;


/* Routine to put a copy of the component params on the stack and call
   CallComponentUPP.  This function is implemented in emsapi-gum.c */
ComponentResult Callems_ComponentFunction(ems_ComponentParameters params);

#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


#ifdef __cplusplus
}
#endif
#endif /* EMSAPI_MACGLUE_H_INCLUDED */


