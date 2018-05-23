/*
 *  Filename: SignTran.cpp
 *
 *  Author: Jeff Beckley
 *
 *  Copyright 1995-2005 QUALCOMM Incorporated
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
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#include "stdafx.h"

#include <afxwin.h> // MFC & Windows master header
#include "resource.h" // Resource IDs for dialogs and icons
#include "SMIME.h" // CSMIMEDLL

#include <stdio.h> // fopen, etc
#include <malloc.h> // malloc, free
#include <string.h> // strdup

#include <ctype.h> // isspace, isdigit
#include <limits.h> // ULONG_MAX

#include "SetngDlg.h" // CSettingsDlg

#include <windows.h>
#include "ems-win.h" // The EMS API

#include "mimetype.h"
#include "encoding.h"
#include "rfc822.h"
#include "rfc1847.h"
#include "file1847.h"

#include <atlbase.h> // For various COM utilities

#include "WinCrypt.h"
#include "CAPICOM.tlh"

//! Cryptographic library
/*!
	CAPICOM stands for Cryptographic API with a COM interface.  It's an easy-to-use
	set of functions using the COM interface that enables developers to easily add
	signing and encrypting functionality to their applications.  Details for the
	API can be found in the header files CAPICOM.tlh and CAPICOM.tli.
*/
using namespace CAPICOM;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*****************************************************************************/
/* CONSTANTS */

static const int kPluginID = 5335;                     //!< The unique ID for this plug-in
static const unsigned long kPluginIconID = IDI_MAIN;   //!< The main icon resource ID for this plug-in

static const int kSignTransmissionTrans     = 1;       //!< Translator ID (1-based) for outgoing signed messages
static const int kSignArrivalTrans          = 2;       //!< Translator ID (1-based) for incoming signed messages
static const int kEncryptTransmissionTrans  = 3;       //!< Translator ID (1-based) for outgoing encrypted messages
static const int kEncryptArrivalTrans       = 4;       //!< Translator ID (1-based) for incoming encrypted messages
static const int kCoalescedTransmissonTrans = 5;       //!< Translator ID (1-based) for outgoing signed and encrypted messages

static const int kNumTrans       = 5;                  //!< How many translators are contained in this plug-in

//! The top-level MIME type for transparently signed messages
#define kMimeMainType		"multipart"
//! The MIME sub-type for transparently signed messages
#define kMimeSubType		"signed"

//! The top-level MIME type for opaque signed messages and encrypted messages
#define kMainType			"application"
//! The MIME sub-type for opaque signed messages and encrypted messages
#define kMainSubType		"pkcs7-mime"
//! An alternate name for MIME sub-type for opaque signed messages and encrypted messages, used by older S/MIME clients
#define kAltMainSubType		"x-pkcs7-mime"
//! The MIME sub-type for the signing data portion 
#define kSignSubType		"pkcs7-signature"

//! The CMS content type for signed messages
#define kSignSMIMEType		"signed-data"
//! The CMS content type for encrypted messages
#define kEncryptSMIMEType	"enveloped-data"

//! The full MIME type for the signing data portion
#define kSignProtocol		(kMainType "/" kSignSubType)
#define kAltSignSubType		"x-pkcs7-signature"
//! The full MIME type for the signing data portion, used by older S/MIME clients
#define kAltSignProtocol	(kMainType "/" kAltSignSubType)

/*****************************************************************************/
/* GLOBALS */

//! Reference to the one instsance of the plug-in DLL
extern CSMIMEDLL theDLL;

//! Translator info
/*!
  TransInfoStruct contains a number of members that help decide which parts of messages
  will get acted upon by a particular translator.  There is a global gTransInfo array that
  keeps track of this info for all translators in the plug-in.
*/
static struct TransInfoStruct {
	UINT nDescID;                 //!< The ID of the string resource for the name of the translator
	long type;                    //!< The EMSAPI type of the translator.  EMST_SIGNATURE, EMST_PREPROCESS, or EMST_COALESCED for this S/MIME plug-in.
	unsigned long flags;          //!< The EMSAPI flags of the translator
	const char *MimeType;         //!< The string to match for the MIME top-level type
	const char *MimeSubtype;      //!< The string to match for the MIME sub-type
	const char *MimeProtocol;     //!< The string to match for the protocol type for multipart/signed messages
	const char *SMIMEType;        //!< The string to match for the CMS content type (see RFC 3851)
	unsigned long nIconID;        //!< The ID of the icon resource used to display for this translator
} gTransInfo[] = {
	{
		IDS_SIGN_DESC_STR, // "S/MIME Signer"
		EMST_SIGNATURE,
		EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME,
		NULL, NULL, NULL, NULL,/* Any type/subtype/protocol/SMIME type */
		IDI_SIGN
	},
	{
		IDS_VERIFY_DESC_STR, // "S/MIME Signature Verifier",
		EMST_SIGNATURE,
		EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_ALL_HEADERS | EMSF_BASIC_HEADERS,
		kMimeMainType, kMimeSubType, kSignProtocol, kSignSMIMEType,
		IDI_SIGN /* OnArrival icon never shown */
	},
	{
		IDS_ENCRYPT_DESC_STR, // "S/MIME Encrypter"
		EMST_PREPROCESS,
		EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME,
		NULL, NULL, NULL, NULL, /* Any type/subtype/protocol/SMIME type */
		IDI_ENCRYPT
	},
	{
		IDS_DECRYPT_DESC_STR, // "S/MIME Decrypter",
		EMST_PREPROCESS,
		EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_ALL_HEADERS | EMSF_BASIC_HEADERS,
		kMainType, kMainSubType, NULL/*kEncryptProtocol*/, kEncryptSMIMEType, // No protocol for encryption since it's not multipart 
		IDI_ENCRYPT /* OnArrival icon never shown */
	},
	{
		NULL,
		EMST_COALESCED,
		EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE | EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME,
		NULL, NULL, NULL, NULL, /* Any type/subtype/protocol/SMIME type */
		NULL // No icon as we don't want it to show up on the comp toolbar
	}
};

/*****************************************************************************/
/* MACROS */

//! Helper macro to ensure that NULL pointers aren't dereferenced when freeing generic memory
#define safefree(p) { if (p) { free(p); (p) = NULL; } }
//! Helper macro to ensure that NULL pointers aren't dereferenced when deleting buffers 
#define safedelete_buf(p) { if (p) { delete_buf(p); (p) = NULL; } }


//! Resource string utility class
/*!
	CRString is a little utility class to make it easier to load up a string resource.
	It allows you to define a variable and load up the string all in one statement.
*/
class CRString : public CString
{
public:
//! Constructor taking the ID of the string resource to load
/*!
	\param StringID The ID of the string resource to load
*/
	CRString(UINT StringID) { LoadString(StringID); }
};


/*****************************************************************************/
/* LOCAL FUNCTION PROTOTYPES */

// Generalized functions

//! Tell whether a given translator can operate while messages are being downloaded
/*!
	\return 0 if the translator cannot be applied, non-zero if it can
*/
static int CheckNotNowContext(
	const long trans_id,            //!< The ID of the translator to be checked
	const long context              //!< The context to be checked, e.g. EMSF_ON_xxx
);

//! Tell whether a given translator can operate after messages are finished being downloaded
/*!
	\return 0 if the translator can not be applied, non-zero if it can
*/
static int CheckNowContext(
	const long trans_id,            //!< The ID of the translator to be checked
	const long context              //!< The context to be checked, e.g. EMSF_ON_xxx
);

//! Tell whether a message part matches a particular translator
/*!
	\return 0 if the message part does not match the translator, non-zero if it does match
*/
static int CheckValidMimeType(
	const long trans_id,            //!< The ID of the translator to be matched against
	const emsMIMEtypeP in_mime      //!< The type details of the message part
);

//! Loads up the icon for a given transaltor
static void DoIconInit(
	const long trans_id,            //!< The ID of the translator
	HICON FAR*FAR* trans_icon       //!< The place in which to put the newly loaded icon
);

//! Tell if the CAPICOM components are present on the user's system
/*!
	\return true if CAPICOM is present, false if it's not
*/
static boolean IsCAPICOMPresent();

//! Perform translation of outgoing signed messages
/*!
	\return S_OK if translation was successful, otherwise error codes embedded in result
*/
static HRESULT DoSignTransmission(
	const char *pInFilename,        //!< The name of the file that holds the message part to be translated
	const char *pOutFilename,       //!< The name of the file to put the results of the translation
	emsProgress progress,           //!< A structure that allows feedback to the user of how long the translation will take
	emsMIMEtypeP FAR* out_mime,     //!< The MIME stucture of the resulting translation
	emsDataFileP inFileData,        //!< Information about the original message part
	const boolean bDetachedSig      //!< Whether or not the signature should be separate from the message, or grouped together with the message
);

//! Perform translation of incoming signed messages
/*!
	\return S_OK if translation was successful, otherwise error codes embedded in result
*/
static HRESULT DoVerifyOnDisplay(
	const char *pInFilename,        //!< The name of the file that holds the message part to be translated
	const char *pOutFilename,       //!< The name of the file to put the results of the translation
	emsProgress progress,           //!< A structure that allows feedback to the user of how long the translation will take
	emsMIMEtypeP FAR* out_mime,     //!< The MIME stucture of the resulting translation
	char FAR*FAR* out_desc,         //!< A buffer to hold a message to be displayed to the user about the translation
	emsDataFileP inFileData         //!< Information about the original message part
);

//! Perform translation of outgoing encrypted messages
/*!
	\return S_OK if translation was successful, otherwise error codes embedded in result
*/
static HRESULT DoEncryptTransmission(
	const char *pInFilename,        //!< The name of the file that holds the message part to be translated
	const char *pOutFilename,       //!< The name of the file to put the results of the translation
	emsProgress progress,           //!< A structure that allows feedback to the user of how long the translation will take
	emsMIMEtypeP FAR* out_mime,     //!< The MIME stucture of the resulting translation
	emsHeaderDataP pHeaders         //!< A list of pertinent headers (and their contents) from the message part to be translated
);

//! Perform translation of incoming encrypted messages
/*!
	\return S_OK if translation was successful, otherwise error codes embedded in result
*/
static HRESULT DoDecryptOnDisplay(
	const char *pInFilename,        //!< The name of the file that holds the message part to be translated
	const char *pOutFilename,       //!< The name of the file to put the results of the translation
	emsProgress progress,           //!< A structure that allows feedback to the user of how long the translation will take
	emsMIMEtypeP FAR* out_mime,     //!< The MIME stucture of the resulting translation
	char FAR*FAR* out_desc          //!< A buffer to hold a message to be displayed to the user about the translation
);

//! Perform translation of outgoing signed and encrypted messages
/*!
	\return S_OK if translation was successful, otherwise error codes embedded in result
*/
HRESULT DoCoalescedTransmission(
	const char *pInFilename,        //!< The name of the file that holds the message part to be translated
	const char *pOutFilename,       //!< The name of the file to put the results of the translation
	emsProgress progress,           //!< A structure that allows feedback to the user of how long the translation will take
	emsMIMEtypeP FAR* out_mime,     //!< The MIME stucture of the resulting translation
	emsDataFileP inFileData         //!< Information about the original message part
);

//! Write the signature of a message part out to a file
/*!
	\return S_OK if writing the signature was successful, otherwise error codes embedded in result
*/
static HRESULT WriteSig(
	const char *pSigFilename,       //!< The name of the file to put the signature data in to
	const char *pInFilename,        //!< The name of the file that holds the message part to have the signature generated
	const char *pSenderEmail,       //!< The email address of the sender of the message
	const boolean bDetachedSig      //!< Whether or not the signature should be separate from the message, or grouped together with the message
);

//! Tell whether the signer of the message matches the given email address (assumed to be the sender of the message)
/*!
	\return TRUE if the addresses match, FALSE if they don't match
*/
static BOOL DoesSignerAddressMatch(
	ISignedDataPtr piSignedData,    //!< The signature of the message
	LPCTSTR pSenderEmail,           //!< The email address of the sender of the message
	_bstr_t& SignerAddress          //!< Gets filled in with the email address of the signer of the message
);

//! Add certificates from the OS certificate store based on passed in address
/*!
	\return S_OK if the certificate was successfully added, otherwise error codes embedded in result
*/
static HRESULT AddCertsAddr(
	ICertificates2Ptr piCertificates,     //!< Pointer to the OS certificate store
	IRecipientsPtr piRecipients,          //!< List of recipients to return
	LPCTSTR pAddress                      //!< Address used to lookup certificates
);

//! Add certificates from the OS certificate store based on passed in recipients
/*!
	\return Comma-separated list of addresses not found in the OS certificate store, empty if all addresses found
*/
static CString AddCertsHeader(
	ICertificates2Ptr piCertificatesCU,   //!< Pointer to the Current User certificate store of the OS
	ICertificates2Ptr piCertificatesLM,   //!< Pointer to the Local Machine certificate store of the OS
	IRecipientsPtr piRecipients,          //!< List of recipients to return
	emsAddressP pAddrHeader               //!< Addresses used to build the list of recipients
);
 
//! Creates a list of certificates from the OS certificate store based on the addresses of the passed in headers
/*!
	An error dialog is displayed to the user if one of more certificates belonging to the addresses in the
	headers could not be found in the OS certificate store.
	\return S_OK if extracting the recipients was successful, otherwise error codes embedded in result
*/
static HRESULT PopulateRecipients(
	IRecipientsPtr piRecipients,    //!< Where the list of certificates are stored
	emsHeaderDataP pHeaders         //!< A list of headers (To:, Cc:, Bcc:) that contain addresses of recipients
);

//! Returns the email address for the signer of a certificate
/*!
	\return The email address if successful, otherwise an empty string
*/
static _bstr_t GetEmailAddress(
	ICertificate2Ptr& piCert        //!< The certificate to extract the email address from
);

/*****************************************************************************/
/* TRANSLATER API FUNCTIONS */

//! Called by Eudora to get the version of the API used for this plug-in
/*!
	\return Always returns EMSR_OK (version is in apiVersion parameter)
*/
extern "C" long WINAPI ems_plugin_version(
    short FAR* apiVersion           //!< Resulting pointer to version of the plug-in
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*apiVersion = EMS_VERSION;
	
	return (EMSR_OK);
}
//	\param apiVersion Pointer to version of the plug-in

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//! Called by Eudora to initialize the plug-in and get its basic info
/*!
	\return Always returns EMSR_OK
*/
extern "C" long WINAPI ems_plugin_init(
    void FAR* FAR* globals,         //!< Return for allocated instance structure
    short eudAPIVersion,            //!< The API version Eudora is using
    emsMailConfigP mailConfig,      //!< Eudora's mail configuration
    emsPluginInfoP pluginInfo       //!< Return plug-in information
)
{
	// Need to load the settings before we call AFX_MANAGE_STATE because we want to
	// get Eudora's app, not the plug-in DLL's app
	g_Settings.Load();

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (pluginInfo)
	{
		pluginInfo->numTrans = kNumTrans;
		pluginInfo->numSpecials = 0;
		pluginInfo->numAttachers = 0;
		
		pluginInfo->desc = strdup(CRString(AFX_IDS_APP_TITLE) + ", " + CRString(IDS_VERSION_STR));
		
		DoIconInit(-1, &(pluginInfo->icon));
		
		pluginInfo->id = kPluginID;
	}

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


//! Called by Eudora to get details about a translator in the plug-in
/*!
	\return EMSR_OK when the translator info is successfully filled out, EMSR_INVALID_TRANS for a bad translator ID
*/

extern "C" long WINAPI ems_translator_info(
    void FAR* globals,              //!< Return for allocated instance structure
    emsTranslatorP transInfo        //!< Return translator information
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


//! Called by Eudora to check and see if a translation can be performed
/*!
	\return EMSR_NOW if the translation can be performed, EMSR_NOT_NOW when the translation can't be performed
	at this time, EMSR_ABORTED if the process was canceled, and EMSR_CANT_TRANS if the translator can't handle
	the given message part
*/

extern "C" long WINAPI ems_can_translate(
    void FAR* globals,              //!< Return for allocated instance structure
    emsTranslatorP transInfo,       //!< Translator info
    emsDataFileP inTransData,       //!< What to translate
    emsResultStatusP transStatus    //!< Translations status information
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

		if (id == kEncryptTransmissionTrans || id == kCoalescedTransmissonTrans)
		{
			// Check to see if we have public keys for all of the recipients
			IRecipientsPtr piRecipients = NULL;
			HRESULT hr = PopulateRecipients(piRecipients, inTransData->header);
			return (SUCCEEDED(hr)? EMSR_NOW : EMSR_ABORTED);
		}

		if (CheckValidMimeType(id, in_mime))
		{
			if (!(g_Settings.m_VerifyOnDownload && id == kSignArrivalTrans && (context & EMSF_ON_ARRIVAL)))
			{
				if (CheckNotNowContext(id, context))
					return (EMSR_NOT_NOW);
			}

			if (CheckNowContext(id, context))
				return (EMSR_NOW);
		}
	}

	return (EMSR_CANT_TRANS);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


//! Called by Eudora to actually perform translation on a file
/*!
	\return EMSR_OK if the translation was successful, EMSR_ABORTED if the process was canceled,
	and EMSR_TRANS_FAILED if the translation failed
*/

extern "C" long WINAPI ems_translate_file(
    void FAR* globals,              //!< Return for allocated instance structure
    emsTranslatorP transInfo,       //!< Translator Info
    emsDataFileP inFileData,        //!< What to translate
    emsProgress progress,           //!< Func to report progress/check for abort
    emsDataFileP outFileData,       //!< Result of the translation
    emsResultStatusP transStatus    //!< Translations Status information
)
{
	const char *in_file = inFileData->fileName;
	const char *out_file = outFileData->fileName;
	const long trans_id = transInfo->id;
	const long context = transInfo->type;
	emsMIMEtypeP FAR *out_mime = &(outFileData->info);
	char FAR* FAR*out_desc = &(transStatus->desc);

	HRESULT hr = S_OK;

	if (!IsCAPICOMPresent())
		return EMSR_ABORTED;

	switch(trans_id)
	{
	case kSignTransmissionTrans:
		hr = DoSignTransmission(in_file, out_file, progress, out_mime, inFileData, true);
		break;

	case kSignArrivalTrans:
		hr = DoVerifyOnDisplay(in_file, out_file, progress, out_mime, out_desc, inFileData);
		break;

	case kEncryptTransmissionTrans:
		hr = DoEncryptTransmission(in_file, out_file, progress, out_mime, inFileData->header);
		break;

	case kEncryptArrivalTrans:
		hr = DoDecryptOnDisplay(in_file, out_file, progress, out_mime, out_desc);
		break;

	case kCoalescedTransmissonTrans:
		hr = DoCoalescedTransmission(in_file, out_file, progress, out_mime, inFileData);
		break;
	}

	if (FAILED(hr))
	{
		if (hr == CAPICOM_E_CANCELLED)
			return EMSR_ABORTED;

		if (transStatus)
		{
			CString ErrorMessage;

			if (hr != E_FAIL)
			{
				LPTSTR lpBuffer;
				if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
									NULL, hr, 0, (LPTSTR)&lpBuffer, 0, NULL))
				{
					ErrorMessage = lpBuffer;
					LocalFree(lpBuffer);
				}
			}

			if (ErrorMessage.IsEmpty())
			{
				AFX_MANAGE_STATE(AfxGetStaticModuleState());
				ErrorMessage.LoadString(IDS_FAILED_ERR_STR);
			}
			transStatus->error = strdup(ErrorMessage);
			if (trans_id == kSignTransmissionTrans || trans_id == kSignArrivalTrans || trans_id == kCoalescedTransmissonTrans)
				transStatus->code = (long) EMSC_SIGBAD;
		}

		return (EMSR_TRANS_FAILED);
	}

	if (transStatus)
	{
		if (trans_id == kSignTransmissionTrans || trans_id == kSignArrivalTrans || trans_id == kCoalescedTransmissonTrans)
			transStatus->code = (long) EMSC_SIGOK;
	}

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


//! Called by Eudora to end use of the plug-in and clean up
/*!
	\return Always returns EMSR_OK
*/
extern "C" long WINAPI ems_plugin_finish(
    void FAR* globals               //!< Return for allocated instance structure
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	safefree(globals);

	return (EMSR_OK); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


//! Called by Eudora to free memory allocated by the plug-in
/*!
	\return Always returns EMSR_OK
*/
extern "C" long WINAPI ems_free(
    void FAR* mem                   //!< Memory to free
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (mem)
		safefree(mem);

	return (EMSR_OK);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//! Called by Eudora to allow the plug-in to put up a configuration interface
/*!
	\return Always returns EMSR_OK
*/
extern "C" long WINAPI ems_plugin_config(
    void FAR* globals,              //!< Return for allocated instance structure
    emsMailConfigP mailConfig       //!< Eudora mail info
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CSettingsDlg settingsDlg;

	settingsDlg.DoModal();

	return (EMSR_OK);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


//! Called by Eudora to manage properties for queued translations
/*!
	\return EMSR_OK on success, EMSR_UNKNOWN_FAIL on failure
*/
extern "C" long WINAPI ems_queued_properties(
    void FAR* globals,              //!< Return for allocated instance structure
    emsTranslatorP transInfo,       //!< The translator
    long FAR* selected              //!< State of this translator
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((!transInfo) || (!selected))
		return (EMSR_UNKNOWN_FAIL);

	if (*selected) // Toggle OFF
	{
		(*selected) = FALSE;
	}
	else // Toggle ON
	{
		if (!IsCAPICOMPresent())
			return EMSR_UNKNOWN_FAIL;

		if (transInfo->id == kSignTransmissionTrans || transInfo->id == kCoalescedTransmissonTrans)
		{
			// First check to see if the user has a signing certificate
			IStorePtr piStore(__uuidof(Store));
			ICertificates2Ptr piCertificates;
			boolean bFoundSome = false;
			HRESULT hr = E_FAIL;

			try
			{
				piStore->Open(CAPICOM_CURRENT_USER_STORE, CAPICOM_MY_STORE,
								(CAPICOM_STORE_OPEN_MODE)(CAPICOM_STORE_OPEN_READ_ONLY | CAPICOM_STORE_OPEN_EXISTING_ONLY));
				piCertificates = piStore->GetCertificates();
				long NumCerts = piCertificates->GetCount();

				for (long i = 1; i <= NumCerts; i++)
				{
					ICertificate2Ptr piCert(piCertificates->GetItem(i));

					if (piCert->HasPrivateKey() == VARIANT_TRUE)
					{
						bFoundSome = true;
						break;
					}
				}
			}
			catch(_com_error e)
			{
				hr = e.Error();
			}

			if (!bFoundSome)
			{
				AfxGetMainWnd()->MessageBox(CRString(IDS_NO_SIGNING_CERTS), CRString(AFX_IDS_APP_TITLE), MB_OK | MB_ICONERROR);
				return EMSR_UNKNOWN_FAIL;
			}
		}
		(*selected) = TRUE;
	}

	return (EMSR_OK); // Return value somewhat meaningless
}

/*****************************************************************************/
/* THE TRANSLATOR ALGORITHMS */

static HRESULT DoSignTransmission(
	const char *pInFilename,
	const char *pOutFilename,
	emsProgress progress,
	emsMIMEtypeP FAR* out_mime,
	emsDataFileP inFileData,
	const boolean bDetachedSig
)
{
	LPCTSTR pSenderEmail = NULL;
	HRESULT hr = E_FAIL;

	if (inFileData->header && inFileData->header->from)
		pSenderEmail = inFileData->header->from->address;

	if (bDetachedSig)
	{
		// Create the mime type and header string
		emsMIMEtypeP mainMimePtr = make_mime_type(kMimeMainType, kMimeSubType, "1.0");
		emsMIMEtypeP sigMimePtr = make_mime_type(kMainType, g_Settings.m_UseOldSigType? kAltSignSubType : kSignSubType, "1.0");

		/* Check if everything was allocated */
		if ((!mainMimePtr) || (!sigMimePtr))
		{
			free_mime_type(mainMimePtr);
			free_mime_type(sigMimePtr);
		}
		else
		{
			char *pSigFilename = strdup(CreateTmpFile());

			add_mime_parameter(mainMimePtr, "protocol", g_Settings.m_UseOldSigType? kAltSignProtocol : kSignProtocol);
			add_mime_parameter(mainMimePtr, "micalg", "sha1");
			hr = WriteSig(pSigFilename, pInFilename, pSenderEmail, bDetachedSig);
			if (SUCCEEDED(hr))
			{
				if (rfc1847_file_create(pOutFilename, mainMimePtr,
										NULL, CTE_NONE, pInFilename,
										sigMimePtr, CTE_Base64, pSigFilename,
										progress) != FILE1847_OK)
				{
					hr = E_FAIL;
				}

				free_mime_type(sigMimePtr);

				// If Eudora wants out mime type structure, give it up
				if (out_mime && SUCCEEDED(hr))
					*out_mime = mainMimePtr;
				else
					free_mime_type(mainMimePtr);
			}
			remove(pSigFilename);
			safefree(pSigFilename);
		}
	}
	else
	{
		hr = WriteSig(pOutFilename, pInFilename, pSenderEmail, bDetachedSig);
		*out_mime = make_mime_type(kMainType, kMainSubType, "1.0");
	}

	return (hr);
}

static HRESULT DoVerifyOnDisplay(
	const char *pInFilename,
	const char *pOutFilename,
	emsProgress progress,
	emsMIMEtypeP FAR* out_mime,
	char FAR*FAR* out_desc,
	emsDataFileP inFileData
)
{
	emsMIMEtypeP InMimePtr = NULL, SigMimePtr = NULL;
	const char *pSigFilename;
	LPCTSTR pSenderEmail = NULL;
	const VARIANT_BOOL vbIsDetached = match_mime_type(inFileData->info, kMimeMainType, kMimeSubType)? VARIANT_TRUE : VARIANT_FALSE;
	const bool bIsArrival = (inFileData->context & EMSF_ON_ARRIVAL);

	if (inFileData->header && inFileData->header->from)
		pSenderEmail = inFileData->header->from->address;

	if (vbIsDetached == VARIANT_FALSE)
	{
		pSigFilename = pInFilename;
	}
	else
	{
		pSigFilename = strdup(CreateTmpFile());
		if (rfc1847_file_parse(pInFilename,
								&InMimePtr,
								pOutFilename,
								out_mime,
								FALSE,
								pSigFilename,
								&SigMimePtr,
								FALSE,
								progress) != FILE1847_OK)
		{
			return E_FAIL;
		}
	}

	ISignedDataPtr piSignedData(__uuidof(SignedData));
	HRESULT hr = E_FAIL;	
	CFileStatus status;
	CString ErrorInfo;
	_bstr_t SignerEmailAddress;

	try {
		CString RawContents;
		LPTSTR pRawContents;
		CFile tempFile;

		// Some strange code here because CAPICOM expects that the Contents be an ANSI basic string rather than the
		// more normal UNICODE basic string (aka BSTR).  An ANSI basic string has the general form of a basic
		// string -- the number of characters in the string before the data and NULL terminated -- but the characters
		// are ANSI (single byte) instead of Unicode (double byte).
		if (vbIsDetached == VARIANT_FALSE)
			hr = S_OK;
		else if (CFile::GetStatus(pOutFilename, status))
		{
			tempFile.Open(pOutFilename, CFile::modeRead);
			pRawContents = RawContents.GetBuffer((UINT)status.m_size + 5);
			tempFile.Read(pRawContents + 4, (UINT)status.m_size);
			pRawContents[status.m_size + 4] = 0;
			*(UINT*)pRawContents = (UINT)status.m_size;

			// Can't use PutContent or Content = because it will convert the ANSI basic string to a BSTR,
			// which will corrupt the string if it is an odd number of bytes in length,
			// so we need to use the raw put_Content() method
			hr = piSignedData->put_Content((BSTR)(void*)(pRawContents + 4));
			tempFile.Close();
		}

		if (SUCCEEDED(hr) && CFile::GetStatus(pSigFilename, status))
		{
			pRawContents = RawContents.GetBuffer((UINT)status.m_size);
			tempFile.Open(pSigFilename, CFile::modeRead);
			tempFile.Read(pRawContents, (UINT)status.m_size);
			pRawContents[status.m_size] = 0;

			LPCSTR pBegData = strstr(pRawContents, "\r\n\r\n");
			if (!pBegData)
				hr = E_INVALIDARG;
			else
			{
				CComBSTR bstrContent(pBegData + 4);
				HRESULT hr2 = E_FAIL;

				hr = piSignedData->raw_Verify((BSTR)bstrContent, vbIsDetached, CAPICOM_VERIFY_SIGNATURE_AND_CERTIFICATE);
				if (FAILED(hr))
					hr2 = piSignedData->raw_Verify((BSTR)bstrContent, vbIsDetached, CAPICOM_VERIFY_SIGNATURE_ONLY);

				if (SUCCEEDED(hr) || SUCCEEDED(hr2))
				{
					if (vbIsDetached == VARIANT_FALSE)
					{
						// Can't use GetContent because it will convert the ANSI basic string to a BSTR,
						// which will corrupt the string if it is an odd number of bytes in length,
						// so we need to use the raw get_Content() method
						BSTR SignedMIMEPart;

						hr = piSignedData->get_Content(&SignedMIMEPart);
						FILE *f = fopen(pOutFilename, "wb");
						if (f)
						{
							fwrite((void*)SignedMIMEPart, 1, SysStringByteLen(SignedMIMEPart), f);
							fclose(f);

							char* pCT = rfc822_extract_header((const char*)SignedMIMEPart, "Content-Type:");
							if (pCT)
							{
								*out_mime = parse_make_mime_type(pCT);
								free(pCT);
							}
						}
					}
					if (pSenderEmail)
					{
						if (!DoesSignerAddressMatch(piSignedData, pSenderEmail, SignerEmailAddress))
						{
							AFX_MANAGE_STATE(AfxGetStaticModuleState());

							ErrorInfo.Format(IDS_ERR_WRONG_SIGNER, (LPCTSTR)SignerEmailAddress, pSenderEmail);
							hr = E_FAIL;
						}
					}
				}
			}
		}
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	

	ICertificate2Ptr piSignerCert;
	bool bDisplayCertificate = false;
	try
	{
		ISignersPtr piSigners(piSignedData->GetSigners());
		ISignerPtr piSigner(piSigners->GetItem(1));
		piSignerCert = piSigner->GetCertificate();
	}
	catch(_com_error e)
	{
		HRESULT hr2 = e.Error();
	}

	time_t Now(time(NULL));
	CString sMsg(ctime(&Now), 24);
	sMsg += "\r\n";

	{
		// Do this in a scope so that we go back to the previous module state.
		// This helps stop a bunch of asserts in debug builds.
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CString SystemErrorMessage;

		if (FAILED(hr) && hr != E_FAIL)
		{
			LPTSTR lpBuffer;
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
								NULL, hr, 0, (LPTSTR)&lpBuffer, 0, NULL))
			{
				SystemErrorMessage = lpBuffer;
				LocalFree(lpBuffer);
			}
		}
		if (FAILED(hr) && !bIsArrival && g_Settings.m_ErrorDisplay != CSettingsDlg::ED_MESSAGE)
		{
			CString DialogText;

			DialogText.Format(IDS_BAD_VERIFY_TEXT, (LPCTSTR)SystemErrorMessage, (LPCTSTR)ErrorInfo);

			UINT MBParams = MB_ICONERROR;
			if (piSignerCert != NULL)
			{
				MBParams |= MB_YESNO | MB_DEFBUTTON2;
				DialogText += CRString(IDS_VIEW_CERTIFICATE);
			}

			if (AfxGetMainWnd()->MessageBox((LPCTSTR)DialogText, (LPCTSTR)CRString(IDS_SIGNFAIL_STR), MBParams) == IDYES)
			{
				if (piSignerCert)
					bDisplayCertificate = true;
			}
		}
		if (bIsArrival || g_Settings.m_ErrorDisplay != CSettingsDlg::ED_DIALOG)
		{
			sMsg += CRString(SUCCEEDED(hr)? IDS_SIGNPASS_STR : IDS_SIGNFAIL_STR);
			sMsg += "\r\n";

			if (SystemErrorMessage.IsEmpty() == FALSE)
				sMsg += SystemErrorMessage;

			if (ErrorInfo.IsEmpty() == FALSE)
			{
				sMsg += ErrorInfo;
				sMsg += "\r\n";
			}
			sMsg += "------------------------------------------------------------\r\n";
			*out_desc = strdup((const char *)sMsg);
		}
	}

	if ((*out_mime) == NULL) // No Content-Type line
	{
		// Default to text/plain
		(*out_mime) = make_mime_type("text", "plain", "1.0");
	}

	if (pSigFilename != pInFilename)
	{
		remove(pSigFilename);
		free((void*)pSigFilename);
	}
	free_mime_type(InMimePtr);
	free_mime_type(SigMimePtr);

	// Do this down here to be out of the AFX_MANAGE_STATE scope because otherwise it causes
	// a bunch of ASSERTs in debug builds
	if (bDisplayCertificate)
		piSignerCert->Display();
	else if (SUCCEEDED(hr) && SignerEmailAddress.length() > 0 && g_Settings.m_AddNewCerts)
	{
		// Ask the user if he wants to add the certificate to his store, if it's not already there
		try
		{
			IStorePtr piCurrentUserStore(__uuidof(Store));
			ICertificates2Ptr piCurrentUserCertificates;
			ICertificates2Ptr piFoundCertificates;

			piCurrentUserStore->Open(CAPICOM_CURRENT_USER_STORE, CAPICOM_OTHER_STORE, CAPICOM_STORE_OPEN_READ_WRITE);
			piCurrentUserCertificates = piCurrentUserStore->GetCertificates();
			const long NumCerts = piCurrentUserCertificates->GetCount();
			boolean bFoundMatch = false;

			for (long i = 1; i <= NumCerts; i++)
			{
				ICertificate2Ptr piCert(piCurrentUserCertificates->GetItem(i));
				_bstr_t CertAddress(GetEmailAddress(piCert));

				if (CertAddress == SignerEmailAddress)
				{
					bFoundMatch = true;
					break;
				}
			}
			if (!bFoundMatch)
			{
				AFX_MANAGE_STATE(AfxGetStaticModuleState());
				CString DialogText;

				DialogText.Format(IDS_ADD_CERT_PROMPT, (LPCTSTR)SignerEmailAddress);
				
				if (bIsArrival ||
					AfxGetMainWnd()->MessageBox(DialogText, CRString(AFX_IDS_APP_TITLE), MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					piCurrentUserStore->Add(piSignerCert);
				}
			}
		}
		catch(_com_error e)
		{
			HRESULT hr2 = e.Error();
		}
	}

	// Always return S_OK because we did the error reporting on our own and
	// we want to have the new text get displayed in the message
	return (S_OK);
}


static HRESULT DoEncryptTransmission(
	const char *pInFilename,
	const char *pOutFilename,
	emsProgress progress,
	emsMIMEtypeP FAR* out_mime,
	emsHeaderDataP pHeaders
)
{
	IEnvelopedDataPtr piEnvelopedData(__uuidof(EnvelopedData));
	HRESULT hr = E_FAIL;
	CFileStatus status;

	try {
		// Some strange code here because CAPICOM expects that the Contents be an ANSI basic string rather than the
		// more normal UNICODE basic string (aka BSTR).  An ANSI basic string has the general form of a basic
		// string -- the number of characters in the string before the data and NULL terminated -- but the characters
		// are ANSI (single byte) instead of Unicode (double byte).
		if (CFile::GetStatus(pInFilename, status))
		{
			CString RawContents;
			CFile tempFile(pInFilename, CFile::modeRead);
			LPTSTR pRawContents = RawContents.GetBuffer((UINT)status.m_size + 5);
			tempFile.Read(pRawContents + 4, (UINT)status.m_size);
			pRawContents[status.m_size + 4] = 0;
			*(UINT*)pRawContents = (UINT)status.m_size;

			// Can't use PutContent or Content = because it will convert the ANSI basic string to a BSTR,
			// which will corrupt the string if it is an odd number of bytes in length,
			// so we need to use the raw put_Content() method
			hr = piEnvelopedData->put_Content((BSTR)(void*)(pRawContents + 4));
			tempFile.Close();

			if (SUCCEEDED(hr))
			{
				IRecipientsPtr piRecipients(piEnvelopedData->GetRecipients());

				hr = PopulateRecipients(piRecipients, pHeaders);
				if (SUCCEEDED(hr))
				{
					IAlgorithmPtr piAlgorithm(piEnvelopedData->Algorithm);

					piAlgorithm->Name = (CAPICOM_ENCRYPTION_ALGORITHM)g_Settings.m_EncryptAlgorithm;
					piAlgorithm->KeyLength = (CAPICOM_ENCRYPTION_KEY_LENGTH)g_Settings.m_EncryptKeyLength;

					_bstr_t EncryptedData(piEnvelopedData->Encrypt(CAPICOM_ENCODE_BASE64));

					if (SUCCEEDED(hr))
					{
						FILE *f = fopen(pOutFilename, "wb");
						if (f)
						{
							fputs(	"Mime-Version: 1.0\r\n"
									"Content-Type: application/pkcs7-mime; smime-type=enveloped-data; name=smime.p7m\r\n"
									"Content-Transfer-Encoding: base64\r\n"
									"Content-Disposition: attachment; filename=smime.p7m\r\n"
									"\r\n",
									f);
							CW2A pszED(EncryptedData);
							fputs(pszED, f);
							fclose(f);

							// If Eudora wants our mime type structure, give it up
							if (out_mime)
							{
								emsMIMEtypeP mainMimePtr = make_mime_type(kMainType, kMainSubType, "1.0");

								if (mainMimePtr)
								{
									add_mime_parameter(mainMimePtr, "smime-type", "enveloped-data");
									add_mime_parameter(mainMimePtr, "name", "smime.p7m");
								}

								*out_mime = mainMimePtr;
							}
						}
					}
				}
			}
		}
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	

	return (hr);
}

static HRESULT DoDecryptOnDisplay(
	const char *pInFilename,
	const char *pOutFilename,
	emsProgress progress,
	emsMIMEtypeP FAR* out_mime,
	char FAR*FAR* out_desc
)
{
	IEnvelopedDataPtr piEnvelopedData(__uuidof(EnvelopedData));
	HRESULT hr = E_FAIL;
	CFileStatus status;

	try {
		// Some strange code here because CAPICOM expects that the Contents be an ANSI basic string rather than the
		// more normal UNICODE basic string (aka BSTR).  An ANSI basic string has the general form of a basic
		// string -- the number of characters in the string before the data and NULL terminated -- but the characters
		// are ANSI (single byte) instead of Unicode (double byte).
		if (CFile::GetStatus(pInFilename, status))
		{
			CString RawContents;
			CFile tempFile(pInFilename, CFile::modeRead);
			LPTSTR pRawContents = RawContents.GetBuffer((UINT)status.m_size + 5);
			tempFile.Read(pRawContents + 4, (UINT)status.m_size);
			pRawContents[status.m_size + 4] = 0;
			tempFile.Close();

			LPCSTR pBegData = strstr(pRawContents + 4, "\r\n\r\n");
			if (!pBegData)
				hr = E_INVALIDARG;
			else
			{
				CComBSTR bstrContent(pBegData + 4);
				hr = piEnvelopedData->Decrypt((BSTR)bstrContent);
				if (SUCCEEDED(hr))
				{
					// Can't use GetContent because it will convert the ANSI basic string to a BSTR,
					// which will corrupt the string if it is an odd number of bytes in length,
					// so we need to use the raw get_Content() method
					BSTR DecryptedData;

					hr = piEnvelopedData->get_Content(&DecryptedData);
					FILE *f = fopen(pOutFilename, "wb");
					if (f)
					{
						fwrite((void*)DecryptedData, 1, SysStringByteLen(DecryptedData), f);
						fclose(f);
					}
				}
			}
		}
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	

	time_t Now(time(NULL));
	CString sMsg(ctime(&Now), 24);
	sMsg += "\r\n";

	{
		// Do this in a scope so that we go back to the previous module state.
		// This helps stop a bunch of asserts in debug builds.
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CString SystemErrorMessage;

		if (FAILED(hr) && hr != E_FAIL)
		{
			LPTSTR lpBuffer;
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
								NULL, hr, 0, (LPTSTR)&lpBuffer, 0, NULL))
			{
				SystemErrorMessage = lpBuffer;
				LocalFree(lpBuffer);
			}
		}
		if (FAILED(hr) && g_Settings.m_ErrorDisplay != CSettingsDlg::ED_MESSAGE)
		{
			CString DialogText;

			DialogText.Format(IDS_BAD_DECRYPT_TEXT, (LPCTSTR)SystemErrorMessage);

			AfxGetMainWnd()->MessageBox((LPCTSTR)DialogText, (LPCTSTR)CRString(IDS_DECRYPTFAIL_STR), MB_ICONERROR);
		}
		if (g_Settings.m_ErrorDisplay != CSettingsDlg::ED_DIALOG)
		{
			sMsg += CRString(SUCCEEDED(hr)? IDS_DECRYPTPASS_STR : IDS_DECRYPTFAIL_STR);
			sMsg += "\r\n";

			if (SystemErrorMessage.IsEmpty() == FALSE)
				sMsg += SystemErrorMessage;

			sMsg += "------------------------------------------------------------\r\n";
			*out_desc = strdup((const char *)sMsg);
		}
	}

	if ((*out_mime) == NULL) // No Content-Type line
	{
		// Default to text/plain
		(*out_mime) = make_mime_type("text", "plain", "1.0");
	}

	// Always return S_OK because we did the error reporting on our own and
	// we want to have the new text get displayed in the message
	return (S_OK);
}

static HRESULT DoCoalescedTransmission(
	const char *pInFilename,
	const char *pOutFilename,
	emsProgress progress,
	emsMIMEtypeP FAR* out_mime,
	emsDataFileP inFileData)
{
	char *pSigFilename = strdup(CreateTmpFile());
	HRESULT hr = DoSignTransmission(pInFilename, pSigFilename, progress, out_mime, inFileData, false);
	if (SUCCEEDED(hr))
		hr = DoEncryptTransmission(pSigFilename, pOutFilename, progress, out_mime, inFileData->header);

	remove(pSigFilename);
	free(pSigFilename);

	return (hr);
}

/*****************************************************************************/
/* SIGNATURE FUNCTIONS */

static HRESULT WriteSig(const char *pSigFilename, const char *pInFilename, const char *pSenderEmail, const boolean bDetachedSig)
{
	HRESULT hr = E_FAIL;
	CFileStatus status;

	try {
		// Some strange code here because CAPICOM expects that the Contents be an ANSI basic string rather than the
		// more normal UNICODE basic string (aka BSTR).  An ANSI basic string has the general form of a basic
		// string -- the number of characters in the string before the data and NULL terminated -- but the characters
		// are ANSI (single byte) instead of Unicode (double byte).
		if (CFile::GetStatus(pInFilename, status))
		{
			CString RawContents;
			CFile tempFile(pInFilename, CFile::modeRead);
			LPTSTR pRawContents = RawContents.GetBuffer((UINT)status.m_size + 5);
			tempFile.Read(pRawContents + 4, (UINT)status.m_size);
			pRawContents[status.m_size + 4] = 0;
			*(UINT*)pRawContents = (UINT)status.m_size;

			ISignedDataPtr piSignedData(__uuidof(SignedData));

			// Can't use PutContent or Content = because it will convert the ANSI basic string to a BSTR,
			// which will corrupt the string if it is an odd number of bytes in length,
			// so we need to use the raw put_Content() method
			hr = piSignedData->put_Content((BSTR)(void*)(pRawContents + 4));
			tempFile.Close();

			if (SUCCEEDED(hr))
			{
				ISigner2Ptr piSigner;

				try
				{
					IStorePtr piStore(__uuidof(Store));
					ICertificates2Ptr piCertificates;
					piStore->Open(CAPICOM_CURRENT_USER_STORE, CAPICOM_MY_STORE,
									(CAPICOM_STORE_OPEN_MODE)(CAPICOM_STORE_OPEN_READ_ONLY | CAPICOM_STORE_OPEN_EXISTING_ONLY));
					piCertificates = piStore->GetCertificates();
					long NumCerts = piCertificates->GetCount();

					for (long i = 1; i <= NumCerts; i++)
					{
						ICertificate2Ptr piCert(piCertificates->GetItem(i));

						if (piCert->HasPrivateKey() == VARIANT_TRUE)
						{
							_bstr_t CertAddress(GetEmailAddress(piCert));
							if (stricmp(CertAddress, pSenderEmail) == 0)
							{
								if (piSigner != NULL)
								{
									// Found more than one cert with a matching address, so set the signer to NULL 
									// so that a dialog comes up asking the user with cert to use for signing
									piSigner = NULL;
									break;
								}
								piSigner.CreateInstance(__uuidof(Signer));
								piSigner->PutCertificate(piCert);
							}
						}
					}
				}
				catch(_com_error e)
				{
					hr = e.Error();
					piSigner = NULL;
				}

				// Can't use Sign() because it will convert the ANSI basic string to a BSTR,
				// which will corrupt the string if it is an odd number of bytes in length,
				// so we need to use the raw_Sign() method
				BSTR SignatureData;
				hr = piSignedData->raw_Sign(piSigner, bDetachedSig? VARIANT_TRUE : VARIANT_FALSE,
					bDetachedSig? CAPICOM_ENCODE_BINARY : CAPICOM_ENCODE_BASE64, &SignatureData);

				if (SUCCEEDED(hr))
				{
					// Wrap the BSTR in a CComBSTR so it gets deleted
					CComBSTR dummy(SignatureData);

					_bstr_t EmailAddress;
					if (!DoesSignerAddressMatch(piSignedData, pSenderEmail, EmailAddress))
					{
						AFX_MANAGE_STATE(AfxGetStaticModuleState());

						CString DialogText;

						DialogText.Format(IDS_SIGN_NO_MATCH_ADDRESS, (LPCTSTR)EmailAddress, (LPCTSTR)pSenderEmail);
						
						if (AfxGetMainWnd()->MessageBox((LPCTSTR)DialogText, (LPCTSTR)CRString(IDS_SIGN_DESC_STR), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDNO)
							hr = CAPICOM_E_CANCELLED;
					}

					if (SUCCEEDED(hr))
					{
						FILE *f = fopen(pSigFilename, "wb");
						if (f)
						{
							if (bDetachedSig)
								fwrite((void*)SignatureData, 1, SysStringByteLen(SignatureData), f);
							else
							{
								fputs(	"Mime-Version: 1.0\r\n"
										"Content-Type: application/pkcs7-mime; smime-type=signed-data; name=smime.p7m\r\n"
										"Content-Transfer-Encoding: base64\r\n"
										"Content-Disposition: attachment; filename=smime.p7m\r\n"
										"\r\n",
										f);
								CW2A pszED(SignatureData);
								fputs(pszED, f);
							}
							fclose(f);
						}
					}
				}
			}
		}
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	

	return (hr);
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
	const char *pSMIMEType = gTransInfo[trans_id-1].SMIMEType;

	// Is this applicaton/pkcs7-MIME (or application/x-pkcs7-MIME)?
	if (match_mime_type(in_mime, kMainType, kMainSubType) || match_mime_type(in_mime, kMainType, kAltMainSubType))
	{
		const char *pParam = get_mime_parameter(in_mime, "smime-type");

		if (pParam)
		{
			if (stricmp(pParam, pSMIMEType) == 0)
				return TRUE;
		}
		else if (trans_id == kEncryptArrivalTrans)
		{
			// Assume that messages without the smime-type parameter are encrypted.
			// Helps for older S/MIME clients, like Thunderbird.
			return TRUE;
		}
		return FALSE;
	}

	if (match_mime_type(in_mime, pType, pSubtype))
	{
		if (!pProtocol)
			return TRUE;

		const char *pParam = get_mime_parameter(in_mime, "protocol");

		if (pParam)
		{
			// This is a hack way to check for "x-pkcs-signature" in addition to "pkcs-signature",
			// but it was the best way without rewriting a bunch of code
			if (stricmp(pProtocol, pParam) == 0 ||
				(trans_id == kSignArrivalTrans && stricmp(kAltSignProtocol, pParam) == 0))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void DoIconInit(
    const long trans_id,         /* ID of translator to get info for */
    HICON FAR*FAR* icon   /* Return for icon data */
)
{
	long nID = gTransInfo[trans_id-1].nIconID;

	*icon = NULL;

	if (!icon || !nID)
		return;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*icon = (HICON *)malloc(sizeof(HICON));

	if (trans_id < 0) /* Main plugin icon, not specific translator */
	{
		**icon = theDLL.LoadIcon(kPluginIconID); /* 32x32 */
	}
	else /* The actual translators */
	{
		**icon = theDLL.LoadIcon(nID); /* 16x16 */
	}
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static boolean IsCAPICOMPresent()
{
	boolean bResult = false;

	try
	{
		// Try to create a CAPICOM IStore object.
		// If we can't even do that then nothing is going to work.
		IStorePtr piStore(__uuidof(Store));
		bResult = true;
	}
	catch(_com_error e)
	{
		HRESULT hr = e.Error();
	}

	if (!bResult)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		AfxGetMainWnd()->MessageBox(CRString(IDS_CAPICOM_LOAD_FAILED), CRString(AFX_IDS_APP_TITLE), MB_OK | MB_ICONERROR);
	}

	return bResult;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static BOOL DoesSignerAddressMatch(ISignedDataPtr piSignedData, LPCTSTR pSenderEmail, _bstr_t& SignerAddress)
{
	ISignersPtr piSigners(piSignedData->GetSigners());
	long NumSigners = piSigners->GetCount();

	for (long i = 1; i <= NumSigners; i++)
	{
		ISignerPtr piSigner(piSigners->GetItem(i));
		ICertificate2Ptr piCertificate2(piSigner->GetCertificate());

		SignerAddress = GetEmailAddress(piCertificate2);

		if (pSenderEmail && stricmp(SignerAddress, pSenderEmail) == 0)
			return TRUE;
	}

	return FALSE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static HRESULT AddCertsAddr(ICertificates2Ptr piCertificates, IRecipientsPtr piRecipients, LPCTSTR pAddress)
{
	HRESULT hr = E_FAIL;

	try
	{
		long NumCerts = piCertificates->GetCount();

		for (long i = 1; i <= NumCerts; i++)
		{
			ICertificate2Ptr piCert(piCertificates->GetItem(i));
			_bstr_t CertAddress(GetEmailAddress(piCert));

			if (stricmp(CertAddress, pAddress) == 0)
			{
				// piRecipients is NULL when we are just checking that the recipient certs are available
				if (piRecipients != NULL)
					piRecipients->Add(piCert);
				hr = S_OK;
			}
		}
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	

	return (hr);
}

static CString AddCertsHeader(ICertificates2Ptr piCertificatesCU, ICertificates2Ptr piCertificatesLM, IRecipientsPtr piRecipients, emsAddressP pAddrHeader)
{
	CString MissingAddr;

	while (pAddrHeader)
	{
		LPCTSTR pAddr = pAddrHeader->address;
		if (FAILED(AddCertsAddr(piCertificatesCU, piRecipients, pAddr)) && FAILED(AddCertsAddr(piCertificatesLM, piRecipients, pAddr)))
		{
			MissingAddr += "    ";
			MissingAddr += pAddr;
			MissingAddr += "\r\n";
		}
		pAddrHeader = pAddrHeader->next;
	}

	return MissingAddr;
}

static HRESULT PopulateRecipients(IRecipientsPtr piRecipients, emsHeaderDataP pHeaders)
{
	HRESULT hr = E_FAIL;

	IStorePtr piCurrentUserStore(__uuidof(Store));
	IStorePtr piLocalMachineStore(__uuidof(Store));
	ICertificates2Ptr piCurrentUserCertificates;
	ICertificates2Ptr piLocalMachineCertificates;

	try
	{
		piCurrentUserStore->Open(CAPICOM_CURRENT_USER_STORE, CAPICOM_OTHER_STORE,
									(CAPICOM_STORE_OPEN_MODE)(CAPICOM_STORE_OPEN_READ_ONLY | CAPICOM_STORE_OPEN_EXISTING_ONLY));
		piCurrentUserCertificates = piCurrentUserStore->GetCertificates();
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	
	try
	{
		piLocalMachineStore->Open(CAPICOM_CURRENT_USER_STORE, CAPICOM_OTHER_STORE,
									(CAPICOM_STORE_OPEN_MODE)(CAPICOM_STORE_OPEN_READ_ONLY | CAPICOM_STORE_OPEN_EXISTING_ONLY));
		piLocalMachineCertificates = piLocalMachineStore->GetCertificates();
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}

	CString MissingCerts;

	MissingCerts += AddCertsHeader(piCurrentUserCertificates, piLocalMachineCertificates, piRecipients, pHeaders->to);
	MissingCerts += AddCertsHeader(piCurrentUserCertificates, piLocalMachineCertificates, piRecipients, pHeaders->cc);
	MissingCerts += AddCertsHeader(piCurrentUserCertificates, piLocalMachineCertificates, piRecipients, pHeaders->bcc);

	if (MissingCerts.IsEmpty())
		hr = S_OK;
	else
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		CRString ErrorMsg(IDS_NO_CERTS);

		ErrorMsg += MissingCerts;

		AfxGetMainWnd()->MessageBox(ErrorMsg, CRString(AFX_IDS_APP_TITLE), MB_ICONERROR);
	}

	return (hr);
}

static _bstr_t GetEmailAddress(ICertificate2Ptr& piCert)
{
	_bstr_t Address;
	HRESULT hr;

	try
	{
		// Use the raw CryptoAPI to get the email address because it will look through
		// the Subject Alternative Name extension, whereas CAPICOM won't
		ICertContextPtr piCertContext(piCert);
		LONG pCAPICertContext = piCertContext->GetCertContext();
		TCHAR addr[128];

		if (CertGetNameString((PCCERT_CONTEXT)pCAPICertContext, CERT_NAME_EMAIL_TYPE,
								0, NULL, addr, sizeof(addr)/sizeof(addr[0])) > 1)
		{
			Address = addr;
		}
		piCertContext->FreeContext(pCAPICertContext);
	}
	catch(_com_error e)
	{
		hr = e.Error();
	}	

	if (Address.length() == 0)
	{
		// This should probably fail too if the above failed,
		// but give it a shot as there's nothing to lose
		try
		{
			Address = piCert->GetInfo(CAPICOM_CERT_INFO_SUBJECT_EMAIL_NAME);
		}
		catch(_com_error e)
		{
			hr = e.Error();
		}
	}

	return Address;
}
