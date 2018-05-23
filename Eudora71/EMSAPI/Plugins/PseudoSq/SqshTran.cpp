/*
 *  Filename: SqshTran.cpp
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

#ifdef _DEBUG
#include <ctype.h>
#endif // _DEBUG

#include <afxwin.h> // MFC & Windows master header
#include "resource.h" // Resource IDs for dialogs and icons
#include "PseudoSq.h" // CPseudoSnDLL

#include <stdio.h>
#include <malloc.h> // malloc, free
#include <string.h> // strdup
#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

#include "mimetype.h"
#include "encoding.h"
#include "rfc822.h"
#include "ROT13.h"

#include <windows.h>
#include "emsapi-win.h" // The EMS API

/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 13; // Unique ID of this plugin
static const char *kPluginDescription = "Pseudo-Squish Sample Plugin, v2.0";
static const unsigned long kPluginIconID = IDI_MAIN;

static const int kTransmissionPseudoSquish  = 1; // Each translator needs a
static const int kArrivalPseudoSquish       = 2; // unique ID within the plugin

static const int kNumTrans       = 2; // How many translators in this plugin

static const int kBufferSize = 8192; // Used for file buffering

static const char *kMIME_EOL = "\r\n";

static const char *kFileErrorStr = "File I/O Error";
static const char *kTransFailedStr = "Translator Failed";

/*****************************************************************************/
/* GLOBALS */

// This externs into PseudoSq.cpp
extern CPseudoSqDLL theDLL;

static struct TransInfoStruct {
	char *description;
	long type;
	unsigned long flags;
	char *MimeType;
	char *MimeSubtype;
	unsigned long nIconID;
} gTransInfo[] = {
	{
		"Transmission Pseudo-Squish",
		EMST_LANGUAGE,
		EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME  | EMSF_GENERATES_MIME,
		NULL, NULL, /* Any type/subtype */
		IDI_PSEUDOSQUISH
	},
	{
		"Arrival Pseudo-Squish",
		EMST_LANGUAGE,
		EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY,
		"application", "x-pseudo-squish",
		IDI_PSEUDOSQUISH /* OnArrival icon never shown */
	}
};

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); p=NULL; } }

/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

// Generalized functions
int CheckValidContext(const long trans_id, const long context);
int CheckValidMimeType(const long trans_id, const emsMIMEtypeP in_mime);
void DoIconInit(const long trans_id, HICON ** trans_icon);

// The actual filters
int DoTransmissionPseudoSquish(ifstream& inFile, ofstream& outFile, emsProgress progress, emsMIMEtypeP * out_mime);
int DoArrivalPseudoSquish(ifstream& inFile, ofstream& outFile, emsProgress progress, emsMIMEtypeP * out_mime);

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
	// We don't use 'globals'

	if (pluginInfo)
	{
		pluginInfo->numTrans = (long) kNumTrans;
		pluginInfo->desc     = (LPSTR) strdup(kPluginDescription);
		pluginInfo->id       = (long) kPluginID;

		DoIconInit(-1, &(pluginInfo->icon));
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

		const TransInfoStruct *InfoPtr = gTransInfo + (id - 1);

		transInfo->type = (long) InfoPtr->type;
		transInfo->flags = (ULONG) InfoPtr->flags;
		transInfo->desc = (LPSTR) strdup(InfoPtr->description);

		DoIconInit(id, &(transInfo->icon));
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

		if ( (CheckValidContext(id, context)) && (CheckValidMimeType(id, in_mime)) )
		{
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
	// This screws up the progesss dialog
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	const char *in_file = inFileData->fileName;
	const char *out_file = outFileData->fileName;
	const long trans_id = transInfo->id;

	// Open the files
	ifstream inFile(in_file, ios::binary);

	if (!inFile.good())
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kFileErrorStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_NO_INPUT_FILE);
	}

	ofstream outFile(out_file, ios::trunc | ios::binary);

	if (!outFile.good())
	{
		inFile.close();

		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kFileErrorStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_CANT_CREATE);
	}

	int err = 0;

	// Which translator should we use?
	switch(trans_id)
	{
		case kTransmissionPseudoSquish:
			err = DoTransmissionPseudoSquish(inFile, outFile, progress, &(outFileData->info));
		break;

		case kArrivalPseudoSquish:
			err = DoArrivalPseudoSquish(inFile, outFile, progress, &(outFileData->info));
		break;
	}


	// Close all the files
	inFile.close();
	outFile.close();

	if (err)
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kTransFailedStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_TRANS_FAILED);
	}

	if (outFileData)
	{
		inFile.open(out_file, ios::binary);

		if (inFile.good())
		{
			char *pHeader = NULL, *pConType = NULL;

			if (pHeader = rfc822_read_header(inFile))
				if (pConType = rfc822_extract_header(pHeader, "Content-Type:"))
					outFileData->info = parse_make_mime_type(pConType);

			safefree(pHeader);
			safefree(pConType);
		}

		inFile.close();
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

	// We don't use 'globals'
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
	if (mem)
		safefree(mem);

	return (EMSR_OK);
}

/*****************************************************************************/
/* THE TRANSLATOR ALGORITHMS */

int DoArrivalPseudoSquish(ifstream& inFile, ofstream& outFile, emsProgress progress, emsMIMEtypeP * out_mime)
{
	const unsigned int kSizeOutBuf = kBufferSize; /* OUTPUT */
	const unsigned int kSizeInBuf  = kBufferSize; /* INPUT: B64, QP, 7Bit, 8Bit, Binary Format */

	Dec64Ptr d64state = NULL;
	DecQPPtr dQPstate = NULL;
	char *pBufOut = NULL, *pBufIn = NULL, *pHeader = NULL, *pCTE = NULL;
	TrEncType cte_mechanism;
	long nOutLength = 0, nCharsCompleted = 0, nInFileLength = 0, nErrorCnt = 0;
	int bAbort, nPercentComplete;

	emsProgressData progData;

	progData.size = sizeof(emsProgressData);
	progData.value = 0L;
	progData.message = NULL;

	if (progress)
		bAbort = (progress(&progData) ? 1 : 0);
	else
		bAbort = FALSE;

	if (!bAbort)
	{
		// Get the file length -- jump to end, check position, jump back to beginning
		inFile.seekg(0, ios::end);
		nInFileLength = inFile.tellg();
		inFile.seekg(0);

		if (pHeader = rfc822_read_header(inFile))
		{
			if (pCTE = rfc822_extract_cte(pHeader))
				cte_mechanism = rfc822_parse_cte(pCTE);
			else
				cte_mechanism = CTE_Binary; /* No CTE header, so no encoding */
		}
		else
			return (-1);

		safefree(pCTE);
		safefree(pHeader);

		nInFileLength -= inFile.tellg();

		if (cte_mechanism == CTE_Error)
			return (-1);

		switch (cte_mechanism)
		{
			case CTE_Base64:
				d64state = (Dec64Ptr) malloc(sizeof(Dec64)); // Used by Decode64()
				pBufOut = (char *) malloc(kSizeOutBuf); // Buffer for binary output
			break;

			case CTE_QP:
				dQPstate = (DecQPPtr) malloc(sizeof(DecQP)); // Used by DecodeQP()
				pBufOut = (char *) malloc(kSizeOutBuf); // Buffer for binary output
			break;

			case CTE_7bit:
			case CTE_8bit:
			case CTE_Binary:
				/* Do nothing */
			break;

			default:
				return (-1); /* Unknown CTE */
		}

		pBufIn = (char *) malloc(kSizeInBuf); // Buffer for B64 input
	}

	// Did any of our allocations fail? (or user aborted)
	if ((bAbort)
		|| (!pBufIn)
		|| ((cte_mechanism == CTE_Base64) && ((!d64state) || (!pBufOut)))
		|| ((cte_mechanism == CTE_QP) && ((!dQPstate) || (!pBufOut))))
	{
		// Something couldn't be allocated --- clean up and exit
		safefree(d64state);
		safefree(dQPstate);
		safefree(pBufOut);
		safefree(pBufIn);
		safefree(pHeader);
		safefree(pCTE);

		return (-1);
	}

	// Initialize state structure -- if needed
	switch (cte_mechanism)
	{
		case CTE_Base64:
			d64state->decoderState = 0;
			d64state->invalCount = 0;
			d64state->padCount = 0;
			d64state->partial = 0;
			d64state->wasCR = FALSE;
		break;

		case CTE_QP:
			dQPstate->CurState = qpNormal;
			dQPstate->cLastChar = 0;
		break;

		case CTE_7bit:
		case CTE_8bit:
		case CTE_Binary:
			/* Do nothing */
		break;
	}

	// Update the progress and check for abort
	nPercentComplete = (int) ((((double)inFile.tellg())/nInFileLength) * 100.0);
	progData.value = (long) nPercentComplete;

	if (progress)
		bAbort = (progress(&progData) ? 1 : 0);

	// Loop until EOF or something terrible happens
	while ((inFile.good()) && (outFile.good()) && (!inFile.eof()) && (!bAbort))
	{
		// Fill our input buffer from file
		inFile.read(pBufIn, kSizeInBuf);

		// Did we get anything out?
		if (inFile.gcount() > 0)
		{
			if ((cte_mechanism == CTE_Base64) || (cte_mechanism == CTE_QP))
			{
				switch (cte_mechanism)
				{
					case CTE_Base64:
						nOutLength = Decode64(pBufIn, inFile.gcount(), pBufOut, d64state, &nErrorCnt);
					break;

					case CTE_QP:
						nOutLength = DecodeQP(pBufIn, inFile.gcount(), pBufOut, dQPstate, &nErrorCnt);
					break;
				}

				if (nOutLength > 0)
				{
					EncodeROT13(pBufOut, nOutLength);
					outFile.write(pBufOut, nOutLength);
				}
			}
			else /* No encoding, no need to copy to out buffer */
			{
				if ((nOutLength = inFile.gcount()) > 0)
				{
					EncodeROT13(pBufIn, nOutLength);
					outFile.write(pBufIn, nOutLength);
				}
			}
		}

		// Update the progress and check for abort
		nPercentComplete = (int) ((((double)inFile.tellg())/nInFileLength) * 100.0);
		
		if (progress)
		{
			progData.value = (long) nPercentComplete;
			bAbort = (progress(&progData) ? 1 : 0);
		}
	}
	
	if ((nPercentComplete < 100) && (progress))
	{
		progData.value = (long) 100;
		bAbort = (progress(&progData) ? 1 : 0);
	}

	if (!bAbort)
	{
		// Flush the decoder
		switch (cte_mechanism)
		{
			case CTE_Base64:
				nOutLength = Decode64(NULL, 0, pBufOut, d64state, &nErrorCnt);
			break;

			case CTE_QP:
				nOutLength = DecodeQP(NULL, 0, pBufOut, dQPstate, &nErrorCnt);
			break;

			case CTE_7bit:
			case CTE_8bit:
			case CTE_Binary:
				nOutLength = 0; /* No cleanup needed */
			break;
		}

		// Send flushed output to file, if needed
		if (nOutLength > 0)
		{
			EncodeROT13(pBufOut, nOutLength);
			outFile.write(pBufOut, nOutLength);
		}
	}

	// Safely free everything
	safefree(d64state);
	safefree(dQPstate);
	safefree(pBufOut);
	safefree(pBufIn);
	safefree(pHeader);
	safefree(pCTE);

	return (bAbort);
}

int DoTransmissionPseudoSquish(ifstream& inFile, ofstream& outFile, emsProgress progress, emsMIMEtypeP * out_mime)
{
	const unsigned int kSizeBinBuf = kBufferSize;      /* INPUT: Raw binary */
	const unsigned int kSize64Buf = (kBufferSize * 2); /* OUTPUT: B64 format, about 33% larger */

	char *bufferBin = (char *) malloc(kSizeBinBuf); // Buffer for binary input
	char *buffer64 = (char *) malloc(kSize64Buf); // Buffer for B64 output

	int bAbort = 0, nPercentComplete = 0;
	long e64length = 0, charsCompleted = 0, inFileLength = 0, nReadLen = 0;

	Enc64Ptr e64state = (Enc64Ptr) malloc(sizeof(Enc64)); // Used by Encode64()

	// Create the mime type and header string
	emsMIMEtypeP mimePtr = make_mime_type("application", "x-pseudo-squish", "1.0");
	char *ConTypeStr = string_mime_type(mimePtr);
	char *pConTransEncStr = rfc822_make_cte(CTE_Base64);

	emsProgressData progData;

	progData.size = sizeof(emsProgressData);
	progData.value = 0L;
	progData.message = NULL;

	// Could we allocate everything?
	if ((!bufferBin)
		|| (!buffer64)
		|| (!e64state)
		|| (!mimePtr)
		|| (!ConTypeStr)
		|| (!pConTransEncStr))
	{
		// Something didn't get allocated, so clean up everything and exit
		safefree(bufferBin);
		safefree(buffer64);
		safefree(e64state);
		free_mime_type(mimePtr);
		safefree(ConTypeStr);
		safefree(pConTransEncStr);

		return (-1); // Error
	}

	// Initialize the BASE64 encoder state structure
	e64state->partialCount = e64state->bytesOnLine = 0;

	// Get the file length -- jump to end, check position, jump back to beginning
	inFile.seekg(0, ios::end);
	inFileLength = inFile.tellg();
	inFile.seekg(0);

	// Output the MIME headers
	outFile << "MIME-Version: 1.0" << kMIME_EOL;
	outFile << ConTypeStr << kMIME_EOL;
	outFile << pConTransEncStr << kMIME_EOL;
	outFile << kMIME_EOL; // Blank line separates header from body

	// Start up the progress dialog
	nPercentComplete = 0;
	
	if (progress)
	{
		progData.value = (long) nPercentComplete;
		bAbort = (progress(&progData) ? 1 : 0);
	}

	// Loop until EOF or something terrible happens
	while ((inFile.good()) && (outFile.good()) && (!inFile.eof()) && (!bAbort))
	{
		// Fill our input buffer from file
		inFile.read(bufferBin, kSizeBinBuf);

		// Did we REALLY read anything?
		if ((nReadLen = inFile.gcount()) > 0)
		{
			// Scramble text using ROT13
			EncodeROT13(bufferBin, nReadLen);

			// Encode the binary input input using BASE64
			e64length = Encode64(bufferBin, nReadLen, buffer64, e64state);

			if (e64length > 0)
				outFile.write(buffer64, e64length);
		}

		// Update the progress and check for abort
		nPercentComplete = (int) ((((double)inFile.tellg())/inFileLength) * 100.0);

		if (progress)
		{
			progData.value = (long) nPercentComplete;
			bAbort = (progress(&progData) ? 1 : 0);
		}
	}

	if ((nPercentComplete < 100) && (progress))
	{
		progData.value = (long) 100;
		bAbort = (progress(&progData) ? 1 : 0);
	}

	if (!bAbort)
	{
		// Flush the encoder
		e64length = Encode64(NULL, 0, buffer64, e64state);

		// Send flushed output to file, if needed
		if (e64length > 0)
			outFile.write(buffer64, e64length);
	}

	// Safely free everything
	safefree(bufferBin);
	safefree(buffer64);
	safefree(e64state);
	safefree(ConTypeStr);
	safefree(pConTransEncStr);

	// If Eudora wants out mime type structure, give it up
	if ((out_mime) && (!bAbort))
		*out_mime = mimePtr;
	else
		free_mime_type(mimePtr);

	return (bAbort);
}

/*****************************************************************************/
/* HELPER FUNCTIONS */

static int CheckValidContext(
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
	char *pType = gTransInfo[trans_id-1].MimeType;
	char *pSubtype = gTransInfo[trans_id-1].MimeSubtype;

	if ( ((!pType) || (strcmp(in_mime->type, pType) == 0))
		&& ((!pSubtype) || (strcmp(in_mime->subType, pSubtype) == 0)) )
		return TRUE;

	return FALSE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    const long trans_id,      /* ID of translator to get info for */
    HICON ** icon             /* Return for icon data */
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!icon)
		return;

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
