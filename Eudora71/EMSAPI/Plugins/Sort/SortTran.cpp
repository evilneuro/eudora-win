/*
 *  Sample EMS plugin: Sort
 *
 *  Filename: SortTran.cpp
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
#include "Sort.h" // CSortDLL

#include <malloc.h> // malloc, free
#include <string.h> // strlen, strdup, strrev
#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

#include <windows.h>
#include "emsapi-win.h" // The EMS API

#include "mimetype.h"

/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 18;
static const char *kPluginDescription = "Sort Sample Plugin, v4.0";
static const unsigned long kPluginIconID = IDI_MAIN;

static const int kSortTransID = 1;
static const int kNumTrans    = 1;

static const char *kFileErrorStr = "File I/O Error";
static const char *kOutOfMem = "Out of Memory";

/*****************************************************************************/
/* GLOBALS */

// This externs into Sort.cpp
extern CSortDLL theDLL;

static struct TransInfoStruct {
	char *description;
	long type;
	long subtype;
	unsigned long flags;
	char *MimeType;
	char *MimeSubtype;
	unsigned long nIconID;
} gTransInfo[] = {
	{
		"Sort",
		EMST_LANGUAGE,
		1L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		IDI_SORT
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

/*****************************************************************************/
/* TRANSLATER API FUNCTIONS */

/*****\
*
* ems_plugin_version: Get the version of the API used for this plugin
*
\*****/

extern "C" long WINAPI ems_plugin_version(
    short * api_version      /* Place to return api version */
)
{
	*api_version = EMS_VERSION;
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

int compare( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
   return _stricmp( * ( char** ) arg1, * ( char** ) arg2 );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_translate_file: Actually perform a translation on a file
*
\*****/

extern "C" long WINAPI ems_translate_file(
    void * globals,                 /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,       /* In: Translator Info */
    emsDataFileP inFileData,        /* In: What to translate */
    emsProgress progress,           /* Func to report progress/check for abort */
    emsDataFileP outFileData,       /* Out: Result of the translation */
    emsResultStatusP transStatus    /* Out: Translations Status information */
)
{
	const char *in_file = inFileData->fileName;
	const char *out_file = outFileData->fileName;

	// Buffer the whole file in memory
	ifstream inFile(in_file, filebuf::binary);
	inFile.setmode(filebuf::binary);

	if (!inFile.good())
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kFileErrorStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_NO_INPUT_FILE);
	}

	inFile.seekg(0, ios::end);
	long inFileLength = inFile.tellg();
	inFile.seekg(0);

	char *buffer = (char *) malloc(inFileLength+1 * sizeof(char));
	memset(buffer, 0, inFileLength+1);

	if (!buffer)
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kOutOfMem);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_UNKNOWN_FAIL);
	}

	inFile.read(buffer, inFileLength);
	inFileLength = inFile.gcount();

	buffer[inFileLength] = '\0';

	inFile.close();

	// Pass #1: Count the lines

	char *cp;
	unsigned long int nLineCount = 0;

	cp = buffer;
	do {
		// Inc counter
		nLineCount++;

		// Move to end of line
		while ((*cp) && (*cp != '\r') && (*cp != '\n'))
			cp++;

		// Move to beg of next line -- handles single \n and \r\n pair
		if (*cp == '\r')
		{
			if (*(cp+1) == '\n')
				cp+=2;
			else
				cp += 1;
		}
		else if (*cp == '\n')
			cp++;
	} while (*cp);

	// Pass #2: Tag the lines with pointers

	char **pBufLines = (char **) malloc(nLineCount * sizeof(char *));

	if (!pBufLines)
	{
		safefree(buffer);
		return (EMSR_UNKNOWN_FAIL);
	}

	char **pPos = pBufLines;
	
	cp = buffer;
	do {
		*pPos++ = cp;

		// Move to end of line
		while ((*cp) && (*cp != '\r') && (*cp != '\n'))
			cp++;

		// NULL terminate -- skip EOL
		if (*cp == '\r')
		{
			*cp = '\0';
			if (*(cp+1) == '\n')
				cp += 2; // Assumes \r\n pair
			else
				cp += 1;
		} 
		else if (*cp == '\n')
			*cp++ = '\0';
	} while (*cp);

	// Now sort the pointers
	qsort(pBufLines, nLineCount, sizeof(char *), compare );

	// And finally output the sorted version
	ofstream outFile(out_file, filebuf::text);

	if (!outFile)
		return (EMSR_UNKNOWN_FAIL);

	pPos = pBufLines;
	char **endPos = pPos + nLineCount;

	while (pPos < endPos)
	{
		outFile << (*pPos) << endl;
		pPos++;
	}

	outFile.close();

	safefree(buffer);
	safefree(pBufLines);

	// Required to set the returned MIME type
	if (outFileData)
		outFileData->info = make_mime_type("text", "plain", "1.0");

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
    const long trans_id,         /* ID of translator to get info for */
    HICON ** icon                /* Return for icon data */
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
