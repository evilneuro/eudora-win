/*
 *  Filename: SignTran.cpp
 *  Original Author: Scott Manjourides
 *
 *  Copyright 1995-2003 QUALCOMM, Incorporated
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


 *
 *  Send comments and questions to <eudora-emsapi@qualcomm.com>
 */

#include "stdafx.h"

#include <afxwin.h> // MFC & Windows master header
#include "resource.h" // Resource IDs for dialogs and icons
#include "PseudoSn.h" // CPseudoSnDLL

#include <stdio.h> // fopen, etc
#include <malloc.h> // malloc, free
#include <string.h> // strdup

#include <ctype.h> // isspace, isdigit
#include <limits.h> // ULONG_MAX

#include "SettingsDlg.h" // CSettingsDlg
#include "PropertiesDlg.h" // CPropertiesDlg

#include <windows.h>
#include "emsapi-win.h" // The EMS API

#include "MimeType.h"
#include "encoding.h"
#include "RFC822.h"
#include "RFC1847.h"
#include "file1847.h"

/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 19; // Unique ID of this plugin
static const unsigned long kPluginIconID = IDI_MAIN;

static const int kTransmissionTrans  = 1; // Each translator needs a
static const int kArrivalTrans       = 2; // unique ID within the plugin

static const int kNumTrans       = 2; // How many translators in this plugin

#define kMimeMainType  "multipart"
#define kMimeSubType   "signed"
#define kSigMainType   "application"
#define kSigSubType    "x.hashed-signature"
#define kSignProtocol  (kSigMainType "/" kSigSubType)

/*****************************************************************************/
/* GLOBALS */

// This externs into PseudoSn.cpp
extern CPseudoSnDLL theDLL;

static struct TransInfoStruct {
	UINT nDescID;
	long type;
	unsigned long flags;
	const char *MimeType;
	const char *MimeSubtype;
	const char *MimeProtocol;
	unsigned long nIconID;
} gTransInfo[] = {
	{
		IDS_SIGN_DESC_STR, // "Clear Pseudo-Sign"
		EMST_SIGNATURE,
		EMSF_Q4_COMPLETION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME,
		NULL, NULL, NULL, /* Any type/subtype/protocol */
		IDI_KEY
	},
	{
		IDS_VERIFY_DESC_STR, // "Clear Pseudo-Sign Verifier",
		EMST_SIGNATURE,
		EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_ALL_HEADERS | EMSF_BASIC_HEADERS,
		kMimeMainType, kMimeSubType, kSignProtocol,
		IDI_KEY /* OnArrival icon never shown */
	}
};

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); (p) = NULL; } }
#define safedelete_buf(p) { if (p) { delete_buf(p); (p) = NULL; } }

/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

// Generalized functions
int CheckNotNowContext(const long trans_id, const long context);
int CheckNowContext(const long trans_id, const long context);
int CheckValidMimeType(const long trans_id, const emsMIMEtypeP in_mime);
void DoIconInit(const long trans_id, HICON ** trans_icon);

// The actual filters
int DoTransmission(const char *pInFilename,
				   const char *pOutFilename,
				   emsProgress progress,
				   emsMIMEtypeP * out_mime,
				   TrEncType nTransferEnc);
int DoOnDisplay(const char *pInFilename,
			  const char *pOutFilename,
			  emsProgress progress,
			  emsMIMEtypeP * out_mime,
			  char ** out_desc);

// The signature functions
int CalcFileHash(const char *pFilename, unsigned long *pHashVal, emsProgress progress);
void WriteSig(const char *pFilename, unsigned long nSigVal);
unsigned long ReadSig(const char *pFilename);

/*****************************************************************************/
/* TRANSLATER API FUNCTIONS */

/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

extern "C" long WINAPI ems_plugin_version(
    short * apiVersion       /* Out: Plugin Version */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*apiVersion = EMS_VERSION;
	
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_init: Initialize plugin and get its basic info
*
\*****/

extern "C" long WINAPI ems_plugin_init(
    void ** globals,            /* Out: Return for allocated instance structure */
    short eudAPIVersion,        /* In: The API version eudora is using */
    emsMailConfigP mailConfig,  /* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo   /* Out: Return Plugin Information */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (globals)
	{
		if ( (*globals) = (void *) malloc(sizeof(int)) )
			*((int *) (*globals)) = CSettingsDlg::te_ask;
		else
			return (EMSR_TRANS_FAILED);
	}
	else
		return (EMSR_UNKNOWN_FAIL);

	if (pluginInfo)
	{
		pluginInfo->numTrans = kNumTrans;
		pluginInfo->numSpecials = 0;
		pluginInfo->numAttachers = 0;
		
		CString sTitle;
		CString sVer;

		sTitle.LoadString(AFX_IDS_APP_TITLE);
		sVer.LoadString(IDS_VERSION_STR);

		pluginInfo->desc = strdup(((const char *)CString(sTitle + ", " + sVer)));
		
		DoIconInit(-1, &(pluginInfo->icon));
		
		pluginInfo->id = kPluginID;
	}
	
	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translator_info: Get details about a translator in a plugin
*
\*****/

extern "C" long WINAPI ems_translator_info(
    void * globals,             /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo    /* In/Out: Return Translator Information */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((transInfo) && (gTransInfo))
	{
		const long id = (long) transInfo->id;

		if ((id <= 0) || (id > kNumTrans))
			return (EMSR_INVALID_TRANS);

		TransInfoStruct *InfoPtr = gTransInfo + (transInfo->id - 1);

		transInfo->type = InfoPtr->type;
		transInfo->flags = InfoPtr->flags;
		DoIconInit(transInfo->id, &(transInfo->icon));

		CString sDesc;
		sDesc.LoadString(InfoPtr->nDescID);
		transInfo->desc = strdup(((const char *)sDesc));
	}

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_can_translate_file:
*     Check and see if a translation can be performed (file version)
*
\*****/

extern "C" long WINAPI ems_can_translate(
    void * globals,               /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,     /* In: Translator Info */
    emsDataFileP inTransData,     /* In: What to translate */
    emsResultStatusP transStatus  /* Out: Translations Status information */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((transInfo) && (inTransData))
	{
		const long id = (long) transInfo->id;

		if ((id <= 0) || (id > kNumTrans))
			return (EMSR_INVALID_TRANS);

		const long context = inTransData->context;
		const emsMIMEtypeP in_mime = inTransData->info;

		if (CheckValidMimeType(id, in_mime))
		{
			if (CheckNotNowContext(id, context))
				return (EMSR_NOT_NOW);

			if (CheckNowContext(id, context))
				return (EMSR_NOW);
		}
	}

	return (EMSR_CANT_TRANS);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translate_file: Actually perform a translation on a file
*
\*****/

extern "C" long WINAPI ems_translate_file(
    void * globals,              /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,    /* In: Translator Info */
    emsDataFileP inFileData,     /* In: What to translate */
    emsProgress progress,        /* Func to report progress/check for abort */
    emsDataFileP outFileData,    /* Out: Result of the translation */
    emsResultStatusP transStatus /* Out: Translations Status information */
)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	const char *in_file = inFileData->fileName;
	const char *out_file = outFileData->fileName;
	const long trans_id = transInfo->id;
	emsMIMEtypeP *out_mime = &(outFileData->info);
	char **out_desc = &(transStatus->desc);

	int nStatus = FILE1847_OK;

	if ((trans_id == kTransmissionTrans) || (trans_id == 3))
	{
		int trsenc = atoi(transInfo->properties);
		TrEncType cte = CTE_NONE;

		switch (trsenc)
		{
			case (CPropertiesDlg::te_base64):  cte = CTE_Base64; break;
			case (CPropertiesDlg::te_qp):      cte = CTE_QP;     break;
			case (CPropertiesDlg::te_none):    cte = CTE_NONE;   break;
		}

		nStatus = DoTransmission(in_file, out_file, progress, out_mime, cte);
	}
	else // if (trans_id == kArrivalTrans)
	{
		nStatus = DoOnDisplay(in_file, out_file, progress, out_mime, out_desc);
	}

	if (nStatus == FILE1847_FAIL)
	{
		if (transStatus)
		{
			CString sErr;
			sErr.LoadString(IDS_FAILED_ERR_STR);
			transStatus->error = (LPSTR) strdup((const char *)sErr);
			transStatus->code = (long) EMSC_SIGBAD;
		}

		return (EMSR_TRANS_FAILED);
	}
	else
		transStatus->code = (long) EMSC_SIGOK;


	if (nStatus == FILE1847_ABORT)
	{
		return (EMSR_ABORTED);
	}

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_finish: End use of a plugin and clean up
*
\*****/

extern "C" long WINAPI ems_plugin_finish(
    void * globals           /* Out: Return for allocated instance structure */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	safefree(globals);

	return (EMSR_OK); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_free: Free memory allocated by EMS plug-in
*
\*****/

extern "C" long WINAPI ems_free(
    void * mem               /* Memory to free */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (mem)
		safefree(mem);

	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* ----- Call the plug-ins configuration Interface ------------------------- */
extern "C" long WINAPI ems_plugin_config(
    void * globals,             /* Out: Return for allocated instance structure */
    emsMailConfigP mailConfig   /* In: Eudora mail info */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int trsenc = *((int *) (globals));
	CSettingsDlg settingsDlg(trsenc);

	int ret = settingsDlg.DoModal();
	if (ret == IDOK)
	{
		trsenc = settingsDlg.GetChoice();
		*((int *) (globals)) = trsenc;
	}

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* ----- Manage properties for queued translations ------------------------- */
extern "C" long WINAPI ems_queued_properties(
    void * globals,             /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,   /* In/Out: The translator */
    long * selected             /* In/Out: State of this translator */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((!globals) || (!transInfo) || (!selected))
		return (EMSR_UNKNOWN_FAIL);

	int trsenc = *((int *) (globals));
	LPSTR *properties = &(transInfo->properties);

	if (trsenc == (CSettingsDlg::te_ask))
	{
		if (*properties)
			trsenc = atoi(*properties);

		CPropertiesDlg propDlg(trsenc);

		int ret = propDlg.DoModal();

		if (ret != IDOK) // Dont' sign
		{
			(*selected) = FALSE;
			(*properties) = NULL;
		}
		else // Sign
		{
			(*selected) = TRUE;

			trsenc = propDlg.GetChoice();

			char buffer[100];
			sprintf(buffer, "%d", trsenc);
			(*properties) = (char *) malloc(strlen(buffer) + 1);
			strcpy((*properties), buffer);
		}
	}
	else // Never ask the user -- just toggle
	{
		if (*selected) // Toggle OFF
		{
			(*selected) = FALSE;
			(*properties) = NULL;
		}
		else // Toggle ON
		{
			(*selected) = TRUE;

			// Set the properties from 'global' setting
			char buffer[100];
			sprintf(buffer, "%d", trsenc);
			(*properties) = (char *) malloc(strlen(buffer) + 1);
			strcpy((*properties), buffer);
		}
	}

	return (EMSR_OK); // Return value somewhat meaningless
}

/*****************************************************************************/
/* THE TRANSLATOR ALGORITHMS */

int DoTransmission(const char *pInFilename,
				   const char *pOutFilename,
				   emsProgress progress,
				   emsMIMEtypeP * out_mime,
				   TrEncType nTransferEnc)
{
	int nStatus = FILE1847_OK;

	// Create the mime type and header string
	emsMIMEtypeP mainMimePtr = make_mime_type(kMimeMainType, kMimeSubType, "1.0");
	emsMIMEtypeP sigMimePtr = make_mime_type(kSigMainType, kSigSubType, "1.0");

	/* Check if everything was allocated */
	if ((!mainMimePtr) || (!sigMimePtr))
	{
		free_mime_type(mainMimePtr);
		free_mime_type(sigMimePtr);

		nStatus = FILE1847_FAIL;
	}

	unsigned long nHashVal = 0;

	if (nStatus == FILE1847_OK)
	{
		add_mime_parameter(mainMimePtr, "protocol", kSignProtocol);
		nStatus = CalcFileHash(pInFilename, &nHashVal, progress);
	}

	if (nStatus == FILE1847_OK)
	{
		char *pSigFilename = strdup(CreateTmpFile());

		WriteSig(pSigFilename, nHashVal);

		nStatus = rfc1847_file_create(pOutFilename, mainMimePtr,
						   NULL, CTE_NONE, pInFilename,
						   sigMimePtr, nTransferEnc, pSigFilename,
						   progress);

		free_mime_type(sigMimePtr);

		// If Eudora wants out mime type structure, give it up
		if ((out_mime) && (nStatus == FILE1847_OK))
			*out_mime = mainMimePtr;
		else
			free_mime_type(mainMimePtr);

		safefree(pSigFilename);
	}

	return(nStatus);
}

int DoOnDisplay(const char *pInFilename,
			  const char *pOutFilename,
			  emsProgress progress,
			  emsMIMEtypeP * out_mime,
			  char ** out_desc)
{
	emsMIMEtypeP InMimePtr = NULL, SigMimePtr = NULL;
	char *pSigFilename = strdup(CreateTmpFile());

	int nStatus = rfc1847_file_parse(pInFilename,
		&InMimePtr,

		pOutFilename,
		out_mime,
		FALSE,

		pSigFilename,
		&SigMimePtr,
		TRUE,

		progress);

	unsigned long nHashVal = 0;

	if (nStatus == FILE1847_OK)
		nStatus = CalcFileHash(pOutFilename, &nHashVal, progress);
	
	if (nStatus == FILE1847_OK)
	{
		unsigned long nSigVal = ReadSig(pSigFilename);
		CString sMsg;

		if (nSigVal == nHashVal)
		{
			sMsg.LoadString(IDS_SIGNPASS_STR);
			*out_desc = strdup((const char *)sMsg); // "Signature verified"
		}
		else
		{
			sMsg.LoadString(IDS_SIGNFAIL_STR);
			*out_desc = strdup((const char *)sMsg); // "Signature failed"
		}

		if ((*out_mime) == NULL) // No Content-Type line
		{
			// Default to text/plain
			(*out_mime) = make_mime_type("text", "plain", "1.0");
		}
	}

	safefree(pSigFilename);
	free_mime_type(InMimePtr);
	free_mime_type(SigMimePtr);

	return (nStatus);
}

/*****************************************************************************/
/* SIGNATURE FUNCTIONS */

int CalcFileHash(const char *pFilename, unsigned long *pHashVal, emsProgress progress)
{
	FILE *f = fopen(pFilename, "rb");
	unsigned long val = 0;
	int nStatus = FILE1847_OK;

	emsProgressData progData;

	progData.size = sizeof(emsProgressData);
	progData.value = 0L;
	progData.message = NULL;

	if (f)
	{
		long nTotalInLen = 0;
		int nPercentComplete = 0;

		if ((nStatus == FILE1847_OK) && progress)
		{
			// get the file length of the two input files -- use for progress
			rewind(f);
			fseek(f, 0, SEEK_END);
			nTotalInLen = ftell(f);
			rewind(f);
	
			progData.value = (long) 0;

			if (progress(&progData) ? 1 : 0)
				nStatus = FILE1847_ABORT;
		}

		const unsigned int kBufSize = 1024;
		unsigned int nRead;
		char buffer[kBufSize], *cp, *end;
		unsigned long uRemain, uTmp;

		while ((nStatus == FILE1847_OK) && (!feof(f)) && (!ferror(f)))
		{
			nRead = fread(buffer, sizeof(char), kBufSize, f);
			end = (cp = buffer) + nRead;

			while (cp < end)
			{
				uRemain = ULONG_MAX - val;
				uTmp = (unsigned long int) (*cp);

				if (uRemain >= uTmp)
					val += uTmp;
				else
					val = uTmp - uRemain;

				cp++;
			}

			if ((nStatus == FILE1847_OK) && (progress))
			{
				// Update the progress and check for abort
				nPercentComplete = (int) ((((double)ftell(f))/nTotalInLen) * 100.0);
				progData.value = (long) nPercentComplete;
				if (progress(&progData) ? 1 : 0)
					nStatus = FILE1847_ABORT;
			}
		}

	}
	else
		nStatus = FILE1847_FAIL;

	if ((nStatus == FILE1847_OK) && (progress))
	{
		// Update the progress and check for abort
		progData.value = (long) 100;
		if (progress(&progData) ? 1 : 0)
			nStatus = FILE1847_ABORT;
	}

	if (nStatus == FILE1847_OK)
		*pHashVal = val;
	
	return (nStatus);
}

void WriteSig(const char *pFilename, unsigned long nSigVal)
{
	FILE *f = fopen(pFilename, "wb");

	if (f)
	{
		fprintf(f, "%lu", nSigVal);
		fclose(f);
	}
}

#ifndef WIN32
	// 16-bit MS compiler does not have _scan functions
	// available for DLLs... so we can't use fscanf()

	// A specialized version on fscanf() for only one unsigned long
	// This function reads in from file f, skipping initial whitespace,
	// the first numeric token into an unsigned long.
	unsigned long scan_sig(FILE *f)
	{
		unsigned long uVal = 0;
		int ch;

		// Skip whitespace
		do {                                                                     
			ch = getc(f);
		} while ( (isspace(ch)) && (ch != '\r') && (ch != '\n') );

		// Read up numerics -- if any
		while (isdigit(ch))
		{
			uVal *= 10;
			uVal += (ch - '0');
			
			ch = getc(f);
		}

		return (uVal);
	}
#endif // !WIN32

unsigned long ReadSig(const char *pFilename)
{
	FILE *f = fopen(pFilename, "rb");
	unsigned long nSigVal = 0;

	if (f)
	{
#ifdef WIN32
		fscanf(f, "%lu", &nSigVal); // 32-bit
#else 
		nSigVal = scan_sig(f);      // 16-bit
#endif
		fclose(f);
	}

	return (nSigVal);
}

/*****************************************************************************/
/* HELPER FUNCTIONS */

static int CheckNotNowContext(
	const long trans_id,  /* ID of translator */
	const long context     /* Context for check; e.g. EMSF_ON_xxx */
)
{
	return ((EMSF_ON_DISPLAY & gTransInfo[trans_id-1].flags) && (EMSF_ON_ARRIVAL & context));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
static int CheckNowContext(
	const long trans_id,  /* ID of translator */
	const long context     /* Context for check; e.g. EMSF_ON_xxx */
)
{
	return ((context & gTransInfo[trans_id-1].flags) ? 1 : 0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int CheckValidMimeType(
    const long trans_id,             /* ID of translator to check */
    const emsMIMEtypeP in_mime        /* MIME type of incomming data */
)
{
	const char *pType = gTransInfo[trans_id-1].MimeType;
	const char *pSubtype = gTransInfo[trans_id-1].MimeSubtype;
	const char *pProtocol = gTransInfo[trans_id-1].MimeProtocol;

	if (match_mime_type(in_mime, pType, pSubtype))
	{
		if (!pProtocol)
			return TRUE;

		const char *pParam = get_mime_parameter(in_mime, "protocol");

		if (pParam)
			if (stricmp(pProtocol, pParam) == 0)
				return TRUE;
	}

	return FALSE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    const long trans_id,       /* ID of translator to get info for */
    HICON ** icon              /* Return for icon data */
)
{
	if (!icon)
		return;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*icon = (HICON *)malloc(sizeof(HICON));

	if (trans_id < 0) /* Main plugin icon, not specific translator */
	{
		**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else /* The actual translators */
	{
		**icon = theDLL.LoadIcon(gTransInfo[trans_id-1].nIconID); /* 16x16 */
	}
}
