/*
 *  Sample EMS plugin: Upper-Lower
 *
 *  Filename: CaseTran.cpp
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
#include "UpprLowr.h" // CUpprLowrDLL

//#include <ctype.h>

#include <malloc.h> // malloc, free
#include <string.h> // strdup
#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

#include <windows.h>
#include "emsapi-win.h" // The EMS API

#include "MimeType.h"    

#ifndef WIN32    
#include <ctype.h>
#endif

/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 11;
static const char *kPluginDescription = "UPPERlower Sample Plugin, v1.3";
static const unsigned long kPluginIconID = IDI_MAIN;

static const int kConvertUpperID = 1;
static const int kConvertLowerID = 2;
static const int kToggleCaseID   = 3;
static const int kWordCaseID     = 4;
static const int kSentenceCaseID = 5;

static const int kNumTrans       = 5;


static const int kBufferSize = 8192;

static const char *kFileErrorStr = "File I/O Error";
static const char *kTransFailedStr = "Translator Failed";

/*****************************************************************************/
/* GLOBALS */

// This externs into UpprLowr.cpp
extern CUpprLowrDLL theDLL;

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
		"UPPER CASE",
		EMST_LANGUAGE,
		1L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		IDI_UPPER
	},
	{
		"lower case",
		EMST_LANGUAGE,
		2L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		IDI_LOWER
	},
	{
		"tOGGLE cASE",
		EMST_LANGUAGE,
		3L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		IDI_CHANGE
	},
	{
		"Word Case",
		EMST_LANGUAGE,
		4L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		IDI_WORD
	},
	{
		"Sentence case",
		EMST_LANGUAGE,
		5L, // unused
		EMSF_ON_REQUEST,
		"text", "plain",
		IDI_SENTENCE
	}
};


/*****************************************************************************/
/* TYEPDEFS */

// Yes, this is a pointer to a function
typedef void (_cdecl *FilterFuncType)(char *, UINT, int) ;

/*****************************************************************************/
/* MACROS */

#define safefree(p) { if (p) { free(p); p=NULL; } }

/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

// Generalized functions
int CheckValidContext(long trans_id, long context);
int CheckValidMimeType(long trans_id, emsMIMEtypeP in_mime);
void DoIconInit(long trans_id, HICON ** trans_icon);
int DoFilter(ifstream& inFile, ofstream& outFile, emsProgress progress, FilterFuncType filter);

// The actual filter algorithms
int ismyspace(int c);
void UpperFilter(register char *cp, UINT size, int Ignored);
void LowerFilter(register char *cp, UINT size, int Ignored);
void ToggleFilter(register char *cp, UINT size, int Ignored);
void WordFilter(register char *cp, UINT size, int Ignored);
void SentenceFilter(register char *cp, UINT size, int nAfterBufChar);

int IsUpper( int c );
int IsLower( int c );
int ToUpper( int c );
int ToLower( int c );


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
	pluginInfo->numTrans = kNumTrans;
	
	pluginInfo->desc = strdup(kPluginDescription);
	
	DoIconInit(-1, &(pluginInfo->icon));
	
	pluginInfo->id = kPluginID;
	
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
	TransInfoStruct *InfoPtr = NULL;

	if ( (transInfo->id <= 0) || (transInfo->id > kNumTrans) )
		return (EMSR_INVALID_TRANS);

	InfoPtr = gTransInfo + (transInfo->id - 1);

	transInfo->type = InfoPtr->type;
	
	transInfo->flags = InfoPtr->flags;
	
	transInfo->desc = strdup(InfoPtr->description);

	DoIconInit(transInfo->id, &(transInfo->icon));

	return (EMSR_OK);
}

/*****\
*
* ems_can_translate_file:
*     Check and see if a translation can be performed (file version)
*
\*****/

extern "C" long WINAPI ems_can_translate(
    void * globals,                 /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,       /* In: Translator Info */
    emsDataFileP inTransData,       /* In: What to translate */
    emsResultStatusP transStatus    /* Out: Translations Status information */
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

/*****\
*
* ems_translate_file: Actually perform a translation on a file
*
\*****/

extern "C" long WINAPI ems_translate_file(
    void * globals,                /* Out: Return for allocated instance structure */
    emsTranslatorP transInfo,      /* In: Translator Info */
    emsDataFileP inFilePB,         /* In: What to translate */
    emsProgress progress,          /* Func to report progress/check for abort */
    emsDataFileP outFilePB,        /* Out: Result of the translation */
    emsResultStatusP transStatus   /* Out: Translations Status information */
)
{ 
	const char *in_file = inFilePB->fileName;
	const char *out_file = outFilePB->fileName;

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
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kFileErrorStr);
			transStatus->code = (long) __LINE__;
		}

		return (EMSR_CANT_CREATE);
	}

	int err = 0;

	// An array of functions -- each function does a different filter (translator)
	FilterFuncType filterArr[] = {
		NULL, // No translator has ID value of zero
		UpperFilter,   // ID = 1
		LowerFilter,   // ID = 2
		ToggleFilter,  // ID = 3
		WordFilter,    // ID = 4
		SentenceFilter // ID = 5
	};

	err = DoFilter(inFile, outFile, progress, filterArr[transInfo->id]);

	inFile.close();
	outFile.close();

	// Check if anything went wrong 
	if (err)
	{
		if (transStatus)
		{
			transStatus->error = (LPSTR) strdup(kTransFailedStr);
			transStatus->code = (long) __LINE__;
		}
		return (EMSR_TRANS_FAILED);
	}

	// Required to set the returned MIME type
	if (outFilePB)
		outFilePB->info = make_mime_type("text", "plain", "1.0");

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*****\
*
* ems_plugin_finish: End use of a plugin and clean up
*
\*****/

extern "C" long WINAPI ems_plugin_finish(
    void * globals           /* Pointer to translator instance structure */
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
/* THE TRANSLATOR ALGORITHMS */

void UpperFilter(register char *cp, UINT size, int Ignored)
{
	if (cp)
	{
		register char *end_buf;

		for (end_buf = (cp + size); cp < end_buf; cp++)
			if (IsLower(*cp))
				*cp = ToUpper(*cp);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void LowerFilter(register char *cp, UINT size, int Ignored)
{
	if (cp)
	{
		register char *end_buf;

		for (end_buf = (cp + size); cp < end_buf; cp++)
			if (IsUpper(*cp))
				*cp = ToLower(*cp);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ToggleFilter(register char *cp, UINT size, int Ignored)
{
	if (cp)
	{
		register char *end_buf;

		for (end_buf = (cp + size); cp < end_buf; cp++)
			if (IsLower(*cp))
				*cp = ToUpper(*cp);
			else if (IsUpper(*cp))
				*cp = ToLower(*cp);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int ismyspace(int c)
{
	return (!isalnum(c)
				&& (c != '.')
				&& (c != '_')
				&& (c != '-')
				&& (c != '\'')
			);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void WordFilter(register char *cp, UINT size, int Ignored)
{
	/* Words are strings of non-whitespace characters */

	static int bWasLastChrWhitespace;

	if (!cp) // Initialize
	{
		// This will force first character to be cap'd
		bWasLastChrWhitespace = TRUE;
		return;
	}

	register char *end_buf;

	for (end_buf = (cp + size); cp < end_buf; cp++)
	{
		if ((IsUpper(*cp)) && (!bWasLastChrWhitespace))
			*cp = ToLower(*cp);
		else if ((IsLower(*cp)) && (bWasLastChrWhitespace))
			*cp = ToUpper(*cp);

		bWasLastChrWhitespace = ismyspace(*cp);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void SentenceFilter(register char *cp, UINT size, int nAfterBufChar)
{
	static int bInSentence, bWasLastChrWhitespace;

	if (!cp) // Initialize
	{
		bInSentence = FALSE;
		bWasLastChrWhitespace = TRUE;

		return;
	}

	const char *kEndOfSentenceChars = ".!?";

	register char *end_buf;
	register int nNextChar;

	for (end_buf = (cp + size); cp < end_buf; cp++)
	{
		if ((cp+1) < end_buf)
			nNextChar = *(cp+1);
		else
			nNextChar = nAfterBufChar;

		if (bInSentence) // Within a sentence, lowercase everything except isolated "I"
		{
			// Is this an "I" that is isolated (whitespace on both sides)?
			if ((ToUpper(*cp) == 'I') && (bWasLastChrWhitespace) && (ismyspace(nNextChar)))
				*cp = 'I'; // Yes, force uppercase
			else if (IsUpper(*cp)) // No, so lowercase it
				*cp = ToLower(*cp);

			// Check for end of sentence character followed by space
			if ((strchr(kEndOfSentenceChars, *cp)) && (ismyspace(nNextChar)))
				bInSentence = FALSE; // Change state
		}
		else if (!ismyspace(*cp)) // Between sentences, look for non-whitespace
		{
			// Found beginning of next sentence -- cap it
			if (IsLower(*cp))
				*cp = ToUpper(*cp);

			bInSentence = TRUE; // Change state
		}

		bWasLastChrWhitespace = ismyspace(*cp);
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int DoFilter(ifstream& inFile, ofstream& outFile, emsProgress progress, FilterFuncType filter)
{
	char *buffer = (char *) malloc(kBufferSize);

	if (!buffer)
		return (-1);

	filter(NULL, 0, 0); // Initialize the filter
	int nCharsRead;

	while ((inFile.good()) && (outFile.good()) && (!inFile.eof()))
	{
		inFile.read(buffer, kBufferSize);
		nCharsRead = inFile.gcount();

		filter(buffer, nCharsRead, inFile.peek());
		outFile.write(buffer, nCharsRead);
	}

	safefree(buffer);

	return (0);
}

/*****************************************************************************/
/* HELPER FUNCTIONS */

static int CheckValidContext(
	long trans_id,  /* ID of translator */
	long context     /* Context for check; e.g. EMSF_ON_xxx */
)
{
	return ((context & gTransInfo[trans_id-1].flags) ? 1 : 0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int CheckValidMimeType(
    long trans_id,             /* ID of translator to check */
    emsMIMEtypeP in_mime        /* MIME type of incomming data */
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
    long trans_id,         /* ID of translator to get info for */
    HICON ** icon          /* Return for icon data */
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

/* Support for non-ASCII Windows ANSI character set (which is a superset of ISO-8859-1),*/

int IsUpper(int c) 
{
    unsigned int uc = c;

    if ((uc >= 'A' && uc <= 'Z') || (uc >= 0xC0 && uc <= 0xDF))
        return (1);
    else
        return (0);
}

int IsLower(int c)
{
    unsigned int uc = c;

    if ((uc >= 'a' && uc <= 'z') || (uc >= 0xE0 && uc <= 0xFF))
        return (1);
    else
        return (0);
}

int ToUpper(int c)
{
    if (IsLower(c))
        return (c - 32);
    else
        return (c);
}

int ToLower(int c)
{
    if (IsUpper(c))
        return (c + 32);
    else
        return (c);
}
