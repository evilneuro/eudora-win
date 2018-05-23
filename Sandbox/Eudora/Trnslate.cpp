// Trnslate.cpp
//
// Eudora Translation API objects
 
#include "stdafx.h" 

#include <afxrich.h>
#include "QCUtils.h"

#include "resource.h"
#include "msgutils.h"
#include "eudora.h"
#include "utils.h"
#include "summary.h"
#include "compmsgd.h"
#include "saveas.h"
#include "debug.h"
#include "progress.h"
#include "guiutils.h"
#include "rs.h"
#include "jjnet.h"
#include "fileutil.h"
#include "sendmail.h"
#include "address.h"
#include "header.h"
#include "FileLineReader.h"
#include "MIMEReader.h"
#include "mime.h"
#include "urledit.h"
#include "tocdoc.h"
#include "tocview.h"
#include "QCCommandStack.h"
#include "QCMailboxTreeCtrl.h"
#include "QCProtocol.h"
#include "SelectMBox.h"
#include "SelectPersonalityDlg.h"
#include "persona.h"
#include "nickdoc.h"
#include "QCMailboxDirector.h"
#include "trnslate.h"  
#include "convhtml.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
 
 
CString g_OutputDesc;

extern QCMailboxDirector	g_theMailboxDirector;
extern UINT	umsgAddAttachment;
 

// =======================================================================
// CTLMime
// =======================================================================
CTLMime::CTLMime()
{
	m_MimeType3.size = sizeof(emsMIMEtype3);
	m_MimeType3.version = NULL;
	m_MimeType3.type = NULL;
	m_MimeType3.subType = NULL;
	m_MimeType3.params = NULL;
	m_MimeType3.contentDisp = NULL;
	m_MimeType3.contentParams = NULL;
}

// =======================================================================
// ~CTLMime
// =======================================================================
CTLMime::~CTLMime()
{
	FreeMimeTypeCTL();
}

// =======================================================================
// CreateMimeType
// Copy data
// =======================================================================
/*BOOL CTLMime::CreateMimeType(emsMIMEtype3 *MimeType)
{
	FreeMimeTypeV3(&m_MimeType3);
	
	if (MimeType && MimeType->version && MimeType->type && MimeType->subType)
	{
		ChangeInfo(MimeType->version, MimeType->type, MimeType->subType, MimeType->contentDisp);

		emsMIMEparam3 FAR*pParam = MimeType->params;
		while(pParam)
		{
			AddParam(pParam->name, pParam->value);
			pParam = pParam->next;
		}

		pParam = MimeType->contentParams;
		while(pParam)
		{
			AddContentParam(pParam->name, pParam->value);
			pParam = pParam->next;
		}

		return TRUE;
	}
	return FALSE;
}
*/

void CEMSDataFile::emptyAddresses(emsAddressP entry)
{
	if (entry)
	{
		emsAddress * lastEntry;
	
		while (entry)
		{
			delete [] entry->address;
			delete [] entry->realname;

			lastEntry = entry;
			entry = entry->next;

			delete lastEntry;
		}
	}
}

void CEMSDataFile::DestroyHeaderInfo()
{
	if (m_DataFile.header)
	{
		if (m_DataFile.header->to)
		{
			emptyAddresses(m_DataFile.header->to);
			m_DataFile.header->to = NULL;
		}
		if (m_DataFile.header->from)		
		{
			emptyAddresses(m_DataFile.header->from);
			m_DataFile.header->from = NULL;
		}

		delete [] m_DataFile.header->subject;
		m_DataFile.header->subject = NULL;

		if (m_DataFile.header->cc)
		{
			emptyAddresses(m_DataFile.header->cc);
			m_DataFile.header->cc = NULL;
		}
		if (m_DataFile.header->bcc)
		{
			emptyAddresses(m_DataFile.header->bcc);
			m_DataFile.header->bcc = NULL;
		}

		delete [] m_DataFile.header->rawHeaders;
		m_DataFile.header->rawHeaders = NULL;

		m_DataFile.header = NULL;
	}
}

void CEMSDataFile::DestroyMimeInfo()
{
	if (m_DataFile.info)
	{
		delete [] m_DataFile.info->version;
		delete [] m_DataFile.info->type;
		delete [] m_DataFile.info->subType;
		delete [] m_DataFile.info->contentDisp;

		emsMIMEparam3 *pParam = m_DataFile.info->params;
		emsMIMEparam3 *pNextParam = NULL;

		while (pParam)
		{
			pNextParam = pParam->next;
			delete [] pParam->value;
			delete [] pParam->name;
			delete pParam;
			pParam = pNextParam;
		}

		pParam = m_DataFile.info->contentParams;
		pNextParam = NULL;
		while (pParam)
		{
			pNextParam = pParam->next;
			delete [] pParam->value;
			delete [] pParam->name;
			delete pParam;
			pParam = pNextParam;
		}

		delete m_DataFile.info;
	}
}

void CEMSDataFile::AddMIME(emsMIMEtype3 *mimeData)
{
	emsMIMEtype3 **MimeType = &m_DataFile.info;
	if (*MimeType)
		DestroyMimeInfo();
	*MimeType = new emsMIMEtype3;

	(*MimeType)->size = mimeData->size;
	
	if (mimeData->version)
	{
		(*MimeType)->version = new char[strlen(mimeData->version)+1];
		strcpy((*MimeType)->version,mimeData->version);
	}
	else
		(*MimeType)->version = NULL;

	if (mimeData->type)
	{
		(*MimeType)->type = new char[strlen(mimeData->type)+1];
		strcpy((*MimeType)->type,mimeData->type);
	}
	else
		(*MimeType)->type = NULL;

	if (mimeData->subType)
	{
		(*MimeType)->subType = new char[strlen(mimeData->subType)+1];
		strcpy((*MimeType)->subType,mimeData->subType);
	}
	else
		(*MimeType)->subType = NULL;

	if (mimeData->contentDisp)
	{
		(*MimeType)->contentDisp = new char[strlen(mimeData->contentDisp)+1];
		strcpy((*MimeType)->contentDisp,mimeData->contentDisp);
	}
	else
		(*MimeType)->contentDisp = NULL;

	emsMIMEparam3 **nextParam = &(mimeData->params);  
	emsMIMEparam3 **copytoParam = &((*MimeType)->params);  
	*copytoParam = NULL;
	while (*nextParam)
	{
		*copytoParam = new emsMIMEparam3;
		(*copytoParam)->size = (*nextParam)->size;
		
		(*copytoParam)->next = NULL;
		(*copytoParam)->name = NULL;
		(*copytoParam)->value = NULL;

		if ((*nextParam)->name)
		{
			(*copytoParam)->name = new char[strlen((*nextParam)->name)+1];
			strcpy((*copytoParam)->name,(*nextParam)->name);
		}

		if ((*nextParam)->value)
		{
			(*copytoParam)->value = new char[strlen((*nextParam)->value)+1];
			strcpy((*copytoParam)->value,(*nextParam)->value);
		}

		copytoParam = &((*copytoParam)->next);
		nextParam = &((*nextParam)->next);
	}

	nextParam = &(mimeData->contentParams);
	copytoParam = &((*MimeType)->contentParams);
	*copytoParam = NULL;
	while (*nextParam)
	{
		*copytoParam = new emsMIMEparam3;
		(*copytoParam)->size = (*nextParam)->size;
		
		(*copytoParam)->name = new char[strlen((*nextParam)->name)+1];
		strcpy((*copytoParam)->name,(*nextParam)->name);

		(*copytoParam)->value = new char[strlen((*nextParam)->value)+1];
		strcpy((*copytoParam)->value,(*nextParam)->value);

		(*copytoParam)->next = NULL;

		copytoParam = &((*copytoParam)->next);
		nextParam = &((*nextParam)->next);
	}
}


BOOL CTLMime::CreateMimeType(emsMIMEtype3 *MimeType)
{

	FreeMimeTypeCTL();
	
	if (MimeType && MimeType->version && MimeType->type && MimeType->subType)
	{
//		m_MimeType3 = new emsMIMEtype3;
		ChangeInfo(MimeType->version, MimeType->type, MimeType->subType, MimeType->contentDisp);

		emsMIMEparam3 *pParam = MimeType->params;
		while(pParam)
		{
			AddParam(pParam->name, pParam->value);
			pParam = pParam->next;
		}

		pParam = MimeType->contentParams;
		while(pParam)
		{
			AddContentParam(pParam->name, pParam->value);
			pParam = pParam->next;
		}

		return TRUE;
	}
	return FALSE;
}

BOOL CTLMime::FreeMimeTypeCTL()
{

	delete [] m_MimeType3.version;
	delete [] m_MimeType3.type;
	delete [] m_MimeType3.subType;
	delete [] m_MimeType3.contentDisp;

	m_MimeType3.version = NULL;
	m_MimeType3.type = NULL;
	m_MimeType3.subType = NULL;
	m_MimeType3.contentDisp = NULL;

	emsMIMEparam3S *param = m_MimeType3.params;
	while (param)
	{
		emsMIMEparam3S *curParam = param;
		param = curParam->next;

		delete [] curParam->name;
		delete [] curParam->value;
		delete curParam;
	}
	m_MimeType3.params = NULL;

	param = m_MimeType3.contentParams;
	while (param)
	{
		emsMIMEparam3S *curParam = param;
		param = curParam->next;

		delete [] curParam->name;
		delete [] curParam->value;
		delete curParam;
	}
	m_MimeType3.contentParams = NULL;

	return (true);
}

// =======================================================================
// CreateMimeType
// Create an outgoing fresh MIME struct that can either be text/plain or multipart/mixed
// =======================================================================
BOOL CTLMime::CreateMimeType(BOOL isFancy /*= FALSE*/, BOOL HasAttach /*= FALSE*/)
{
	FreeMimeTypeCTL();
	
	//Create Mime info
	CString MimeType, MimeSubtype;

	CRString MimeVersion(IDS_MIME_VERSION);
		
	if (HasAttach)
	{
		MimeType.LoadString(IDS_MIME_MULTIPART);
		MimeSubtype.LoadString(IDS_MIME_MIXED);
	}
	else if (isFancy)
	{
		MimeType.LoadString(IDS_MIME_TEXT);
		MimeSubtype.LoadString(IDS_MIME_HTML);
	}
	else
	{
		MimeType.LoadString(IDS_MIME_TEXT);
		MimeSubtype.LoadString(IDS_MIME_PLAIN);
	}
	ChangeInfo(MimeVersion, MimeType, MimeSubtype, NULL);

	m_MimeType3.contentDisp = NULL;
	m_MimeType3.contentParams = NULL;
	m_MimeType3.params = NULL;

	return TRUE;
}


// =======================================================================
// CreateMimeType
// Create an  MIME struct based on the info in the file
// =======================================================================
BOOL CTLMime::CreateMimeType(const char *filename)
{
	extern JJFile *g_pPOPInboxFile;
	BOOL ret = FALSE;
	
	if (!filename) 
		return FALSE;
  
  	FileLineReader lineReader;
	JJFile in, out;
	
	// read from file
	if (FAILED(in.Open(filename, O_RDWR)))
		return FALSE;
	lineReader.SetFile(&in);
	
 	// g_pPOPInboxFile is a global (defined in pop.cpp) that is used by mime.cpp, header.cpp code
	CString tmp = ::GetTmpFileNameMT();			
	JJFile * oldPopMBox = g_pPOPInboxFile;
	if (FAILED(out.Open(tmp, O_RDWR | O_APPEND | O_CREAT)))
		return FALSE;
	g_pPOPInboxFile = &out;

	// Create A hd & Load it up, line reader must be set up, output g_pPOPInboxFile must be set up
	HeaderDesc hd;
	hd.Read(&lineReader);
   	
 	// Restore global

	g_pPOPInboxFile = oldPopMBox;
	out.Delete();
  	
  	if (&hd.m_TLMime->m_MimeType3)
  	{
  		CreateMimeType(&hd.m_TLMime->m_MimeType3);  
  		return TRUE;
  	}
  				
	return FALSE;
}

// =======================================================================
// ChangeInfo					// Used in V3 and in other parts of the code.
// Change Top Level Info		// Looks good I think.
// =======================================================================
BOOL CTLMime::ChangeInfo(const char *Version, const char *type, const char *subType, const char *contentDisp /*= NULL*/)
{
	if (Version)
	{
		delete [] m_MimeType3.version;
		m_MimeType3.version = new char[strlen(Version) + 1];
		strcpy(m_MimeType3.version, Version);
	}
	if (type)
	{
		delete [] m_MimeType3.type;
		m_MimeType3.type = new char[strlen(type) + 1];
		strcpy(m_MimeType3.type, type);
	}
	if (subType)
	{
		delete [] m_MimeType3.subType;
		m_MimeType3.subType = new char[strlen(subType) + 1];
		strcpy(m_MimeType3.subType, subType);
	}

	if (contentDisp)
	{
		delete [] m_MimeType3.contentDisp;
		m_MimeType3.contentDisp = new char[strlen(contentDisp) + 1];
		strcpy(m_MimeType3.contentDisp, contentDisp);
	}
	return TRUE;

}
/*
// =======================================================================
// AddParam
// Add A Mime Parameter
// =======================================================================
BOOL CTLMime::AddParam(const char *name, const char *value, emsMIMEparam3 *param) // param = NULL
{
	// Default to populate the simple mime parameter list
	struct emsMIMEparam3S FAR*FAR*nextParam = &param; 
//	struct emsMIMEparam3S	*nextParam = param;
	if (!param)
		nextParam = &m_MimeType3.params; 
	
	// go to the end of the list
	while (*nextParam)
		nextParam = &(*nextParam)->next;

	// Create the actual parameter
	emsMIMEparam3S *pParam = new emsMIMEparam3;
	pParam->size = sizeof(emsMIMEparam3);
	
	pParam->name = new char[strlen(name) + 1];
	strcpy(pParam->name, name);
	
	pParam->value = new char[strlen(value) + 1];
	strcpy(pParam->value, value);
	
	pParam->next = NULL;

	*nextParam = pParam;

	return TRUE;

}
*/

// =======================================================================
// AddParam
// Add A Mime Parameter		// Used a few places V3 et al. Much prettier now without FAR references.
// =======================================================================
BOOL CTLMime::AddParam(const char *name, const char *value, emsMIMEparam3 *param /* = NULL*/)
{
	// Default to populate the simple mime parameter list
	struct emsMIMEparam3S **nextParam = &param; 

	if (!param)
		nextParam = &m_MimeType3.params; 
	
	// go to the end of the list
	while (*nextParam)
		nextParam = &(*nextParam)->next;

	// Create the actual parameter
	emsMIMEparam3S *pParam = new emsMIMEparam3;
	pParam->size = sizeof(emsMIMEparam3);
	
	pParam->name = new char[strlen(name) + 1];
	strcpy(pParam->name, name);
	
	pParam->value = new char[strlen(value) + 1];
	strcpy(pParam->value, value);
	
	pParam->next = NULL;

	*nextParam = pParam;

	return TRUE;
}



// =======================================================================
// AddContentParam
// =======================================================================
BOOL CTLMime::AddContentParam(const char *name, const char *value)
{
	return (AddParam(name, value, m_MimeType3.contentParams));
}



/*
// =======================================================================
// FreeMimeTypeV3 :: API Version
// If no free function, use the default delete
// =======================================================================
BOOL FreeMimeTypeV3(emsMIMEtype3 *mimeType, ems2_free_t *freeFunc)
{
	if (mimeType)
	{
 		if (mimeType->version)
			freeFunc ? freeFunc(mimeType->version) : delete [] mimeType->version;;
		if (mimeType->type)
			freeFunc ? freeFunc(mimeType->type) : delete [] mimeType->type;
		if (mimeType->subType)
			freeFunc ? freeFunc(mimeType->subType) : delete [] mimeType->subType;
		if (mimeType->contentDisp)
			freeFunc ? freeFunc(mimeType->contentDisp) : delete [] mimeType->contentDisp;

		mimeType->version = NULL;
		mimeType->type = NULL;
		mimeType->subType = NULL;
		mimeType->contentDisp = NULL;

		emsMIMEparam3S *param = mimeType->params;
		while (param)
		{
			emsMIMEparam3S *curParam = param;
			param = curParam->next;

			if(curParam->name)
				freeFunc ? freeFunc(curParam->name) : delete [] curParam->name;
			if(curParam->value)
				freeFunc ? freeFunc(curParam->value) : delete [] curParam->value;
			if(curParam)
				freeFunc ? freeFunc(curParam) : delete curParam;
		}
		mimeType->params = NULL;

		param = mimeType->contentParams;
		while (param)
		{
			emsMIMEparam3S *curParam = param;
			param = curParam->next;

			if(curParam->name)
				freeFunc ? freeFunc(curParam->name) : delete [] curParam->name;
			if(curParam->value)
				freeFunc ? freeFunc(curParam->value) : delete [] curParam->value;
			if(curParam)
				freeFunc ? freeFunc(curParam) : delete curParam ;
		}
		mimeType->contentParams = NULL;
	}
	
	return TRUE;
}
*/
// =======================================================================
// FreeMimeTypeV3 :: API Version	// This is the function we'll use if it's the plugin's memory
// If no free function, use the default delete
// =======================================================================
BOOL FreeMimeTypeV3(emsDataFile *MimeFile, ems2_free_t *freeFunc)
{
	emsMIMEtype3 *mimeType = MimeFile->info;
	
	if (mimeType && freeFunc) // Good Plugin! We should only get here if the plugin allocated the data
	{
 		if (mimeType->version)
		{
			freeFunc(mimeType->version);
			mimeType->version = NULL;
		}
		if (mimeType->type)
		{
			freeFunc(mimeType->type);
			mimeType->type = NULL;
		}
		if (mimeType->subType)
		{
			freeFunc(mimeType->subType);
			mimeType->subType = NULL;
		}
		if (mimeType->contentDisp)
		{
			freeFunc(mimeType->contentDisp);
			mimeType->contentDisp = NULL;
		}	

		emsMIMEparam3S *param = mimeType->params;
		while (param)
		{
			emsMIMEparam3S *curParam = param;
			param = curParam->next;

			if(curParam->name)
			{
				freeFunc(curParam->name);
				curParam->name = NULL;
			}
			if(curParam->value)
			{
				freeFunc(curParam->value);
				curParam->value = NULL;
			}
			if(curParam)
			{
				freeFunc(curParam);
				curParam = NULL;
			}
		}
		mimeType->params = NULL;

		param = mimeType->contentParams;
		while (param)
		{
			emsMIMEparam3S *curParam = param;
			param = curParam->next;

			if(curParam->name)
			{
				freeFunc(curParam->name);
				curParam->name = NULL;
			}
			if(curParam->value)
			{
				freeFunc(curParam->value);
				curParam->value = NULL;
			}
			if(curParam)
			{
				freeFunc(curParam);
				curParam = NULL;
			}
		}
		mimeType->contentParams = NULL;
		freeFunc(MimeFile->info);
		MimeFile->info = NULL;
		return (true);
	}
	return (false);	// No free function?! Awww. Maybe set everything to NULL and lose memory?
}

// =======================================================================
// FreeMimeTypeV3 :: API Version		// We use this one if we allocated the memory
// If no free function, use the default delete
// =======================================================================
BOOL FreeMimeTypeV3(emsDataFile *MimeFile)
{
	emsMIMEtype3 *mimeType = MimeFile->info;

	if (mimeType)
	{
		delete [] mimeType->version;
		delete [] mimeType->type;
		delete [] mimeType->subType;
		delete [] mimeType->contentDisp;

		mimeType->version = NULL;
		mimeType->type = NULL;
		mimeType->subType = NULL;
		mimeType->contentDisp = NULL;

		emsMIMEparam3S *param = mimeType->params;
		while (param)
		{
			emsMIMEparam3S *curParam = param;
			param = curParam->next;

			delete [] curParam->name;
			delete [] curParam->value;
			delete curParam;
		}
		mimeType->params = NULL;

		param = mimeType->contentParams;
		while (param)
		{
			emsMIMEparam3S *curParam = param;
			param = curParam->next;

			delete [] curParam->name;
			delete [] curParam->value;
			delete curParam;
		}
		mimeType->contentParams = NULL;
		delete MimeFile->info;
		MimeFile->info = NULL;
	}
		return (true);
}


// =======================================================================
// DegradeMimeType
// =======================================================================
emsMIMEtype *CTLMime::DegradeMimeType()
{
	emsMIMEtype *oldMimeType = new emsMIMEtype;
	
	oldMimeType->mime_version = NULL;
	oldMimeType->mime_type = NULL;
	oldMimeType->sub_type = NULL;
	
	if (m_MimeType3.version)
	{
		oldMimeType->mime_version = new char[strlen(m_MimeType3.version) + 1];
		strcpy(oldMimeType->mime_version, m_MimeType3.version);
	}

	if (m_MimeType3.type)
	{
		oldMimeType->mime_type = new char[strlen(m_MimeType3.type) + 1];
		strcpy(oldMimeType->mime_type, m_MimeType3.type);
	}

	if (m_MimeType3.subType)
	{
		oldMimeType->sub_type = new char[strlen(m_MimeType3.subType) + 1];
		strcpy(oldMimeType->sub_type, m_MimeType3.subType);
	}

	emsMIMEparam3 *param3 = m_MimeType3.params;

	oldMimeType->params = NULL;
	emsMIMEParamP * nxtParam = &oldMimeType->params;
	
	while(param3)
	{
		emsMIMEparam * v2Params = new emsMIMEparam; 
		v2Params->name = NULL;
		v2Params->value = NULL;
		
		if (param3->name)
		{
			v2Params->name = new char[strlen(param3->name) + 1];
			strcpy(v2Params->name, param3->name);
		}
		if (param3->value)
		{
			v2Params->value = new char[strlen(param3->value) + 1];
			strcpy(v2Params->value, param3->value);
		}
		*nxtParam = v2Params;
		v2Params->next = NULL;
		nxtParam = &v2Params->next;
		param3 = param3->next;

	}
	return oldMimeType;
}
// =======================================================================
// UpgradeMimeType
// =======================================================================
void CTLMime::UpgradeMimeType(emsMIMEtypeP oldMimeType)
{
	ASSERT(oldMimeType);

	FreeMimeTypeCTL();
	
	ChangeInfo(oldMimeType->mime_version, oldMimeType->mime_type, oldMimeType->sub_type, NULL);

	emsMIMEparam *pParams = oldMimeType->params;
	while(pParams)
	{
		AddParam(pParams->name, pParams->value);
		pParams = pParams->next;
	}
}


// =======================================================================
// CTLAddress
// =======================================================================
CTLAddress::CTLAddress()
{
	m_AddressEnd = 0;

	m_HeaderData.size = sizeof(emsHeaderData);
    m_HeaderData.to = NULL;
    m_HeaderData.from = NULL;
    m_HeaderData.subject = NULL;
    m_HeaderData.cc = NULL;
    m_HeaderData.bcc = NULL;
    m_HeaderData.rawHeaders = NULL;
}

// =======================================================================
// ~CTLAddress
// =======================================================================
CTLAddress::~CTLAddress()
{
	CleanAddressList();
}


// =======================================================================
// CreateAddressList
// =======================================================================
BOOL CTLAddress::CreateAddressList(
CDocument *pDocument )
{
	CCompMessageDoc* msg;
	char name[255];

	ASSERT_KINDOF( CCompMessageDoc, pDocument );

	msg = ( CCompMessageDoc* ) pDocument;
	
	CleanAddressList();

	if (msg)
	{
		// The way the address list is:
		//	From Recipients NULL
		//  From: address RealName Nickname
		//  Recipient: all the entries in To, CC, Bcc in From Format
		
		// Get the From info
		const char *ra = GetReturnAddress();
		if (ra)
		{
			char *address;
			strcpy(name,ra);
			char * sp = strchr(name,'<');
			if (sp)
			{
				*sp++ = 0;
				address = sp;
				sp = strchr(address,'>');
				if (sp)
					*sp = 0;
			}
			else
				address = name;

			AddEntry(address, name, "");
			m_HeaderData.from = CreateEntryV3(address, name);
		}

		// Add in all the recipients
		AddRecipients(msg->GetHeaderLine(HEADER_TO), &m_HeaderData.to);
		AddRecipients(msg->GetHeaderLine(HEADER_CC), &m_HeaderData.cc);
		AddRecipients(msg->GetHeaderLine(HEADER_BCC), &m_HeaderData.bcc);

		m_HeaderData.subject = NULL;
		const char *subject = msg->GetHeaderLine(HEADER_SUBJECT);
		if (subject && *subject)
		{
			m_HeaderData.subject = new char[strlen(subject) + 1];
			strcpy(m_HeaderData.subject, subject);
		}

		// Do we have them !!!!
		m_HeaderData.rawHeaders = NULL;

	}

	return TRUE;
}
// =======================================================================
// CreateAddressList
// =======================================================================
BOOL CTLAddress::CreateAddressList(const char *to, 
									const char *from, 
									const char *subject, 
									const char *cc, 
									const char *bcc,
									const char *rawHeaders)
{
	CleanAddressList();
	// Add in all the recipients
	AddRecipients(from, &m_HeaderData.from);
	AddRecipients(to, &m_HeaderData.to);
	AddRecipients(cc, &m_HeaderData.cc);
	AddRecipients(bcc, &m_HeaderData.bcc);

	if (subject && *subject)
	{
		m_HeaderData.subject = new char[strlen(subject) + 1];
		strcpy(m_HeaderData.subject, subject);
	}

	if (rawHeaders && *rawHeaders)
	{
		// Just grab off the header of the message
		char *endOfHeader = strstr(rawHeaders, "\r\n\r\n");
		int len = 0;
		if (endOfHeader)
			len = endOfHeader - rawHeaders;
		else
			len = strlen(rawHeaders);
		m_HeaderData.rawHeaders = new char[len + 1];
		strncpy(m_HeaderData.rawHeaders, rawHeaders, len);
		m_HeaderData.rawHeaders[len] = 0;
	}

	return TRUE;
}


// =======================================================================
// AddRecipients
// =======================================================================
BOOL CTLAddress::AddRecipients(const char *recLine, emsAddressP * field)
{
	emsAddressP pAddEntry;
	emsAddressP * nextEntry = field;
 	
	if (!recLine || !*recLine || !strlen(recLine))
		return TRUE;
	char* recBuf = ExpandAliases(recLine, TRUE, TRUE);
	if (!recBuf || !*recBuf)
		return TRUE;


	int len = strlen(recBuf);
	char *start = recBuf;
	char *end = FindAddressEnd(start);

	while (end)
	{
		*end = 0;

		char *refStart = new char[strlen(start) + 1];
		strcpy(refStart,start);
		
		char *add = StripAddress(start);
		char *rn = GetRealName(refStart);
	
		AddEntry(add, rn, "");
		
		pAddEntry = CreateEntryV3(add, rn);
		*nextEntry = pAddEntry;
		nextEntry = &(*nextEntry)->next;

		delete [] refStart;
		
		if (end < (recBuf + len))
		{
			start = end + 1;
			end = FindAddressEnd(start);
		}
		else
			break;
	}
	delete [] recBuf;
	return TRUE;

}

// =======================================================================
// AddEntry
// =======================================================================
BOOL CTLAddress::AddEntry(const char*address, const char*name, const char*nick)
{
	ASSERT(!(ADD_BUF_SIZE <= m_AddressEnd));
	if (ADD_BUF_SIZE <= m_AddressEnd)
		return FALSE;
	
	// go to end of list
	char *padd = new char[strlen(address)+1];
	strcpy(padd, address);
	m_AddressList[m_AddressEnd++] = padd;

	char *pname = new char[strlen(name)+1];
	strcpy(pname, name);
	m_AddressList[m_AddressEnd++] = pname;

	if (nick && *nick)
	{
		char *pnick = new char[strlen(nick)+1];
		strcpy(pnick, nick);
		m_AddressList[m_AddressEnd++] = pnick;
	}
	else
		m_AddressList[m_AddressEnd++] = NULL;


	m_AddressList[m_AddressEnd] = NULL;


	return TRUE;

}

// =======================================================================
// CreateEntryV3
// =======================================================================
emsAddressP  CTLAddress::CreateEntryV3(const char*address, const char*name)
{
	emsAddress * pAdd = new emsAddress;
	pAdd->size = sizeof(emsAddressP);
	pAdd->address = NULL;
	pAdd->realname = NULL;
	pAdd->next = NULL;
	
	if (address && *address)
	{
		pAdd->address = new char[strlen(address) + 1];
		strcpy( pAdd->address, address);
	}
	
	if (name && *name)
	{
		pAdd->realname = new char[strlen(name) + 1];
		strcpy( pAdd->realname, name);
	}

	return pAdd;
}

// =======================================================================
// RemoveV3Entry
// =======================================================================
void CTLAddress::RemoveEntryV3(emsAddressP entry)
{
	emsAddress * lastEntry;
	
	while (entry)
	{
		delete [] entry->address;
		delete [] entry->realname;

		lastEntry = entry;
		entry = entry->next;

		delete lastEntry;
	}
}


// =======================================================================
// CleanAddressList
// =======================================================================
BOOL CTLAddress::CleanAddressList()
{
 	while(m_AddressEnd--)
	{
		delete [] m_AddressList[m_AddressEnd];
		m_AddressList[m_AddressEnd] = NULL;
	}
	
	m_AddressEnd = 0;
	m_AddressList[m_AddressEnd] = NULL;


	// Now clean out the v3 stuff
	RemoveEntryV3(m_HeaderData.to);		
	RemoveEntryV3(m_HeaderData.from);		
	RemoveEntryV3(m_HeaderData.cc);		
	RemoveEntryV3(m_HeaderData.bcc);		

	delete [] m_HeaderData.subject;  
	delete [] m_HeaderData.rawHeaders;  

	return TRUE;
}

// =======================================================================
// CEMSMailConfig
// =======================================================================
CEMSMailConfig::CEMSMailConfig()
{
	CString pluginDir = EudoraDir + CRString(IDS_TRANS_DIRECTORY);

	const char *retAdd = GetIniString(IDS_INI_RETURN_ADDRESS);
	if (!retAdd || !*retAdd)
		retAdd = GetIniString(IDS_INI_POP_ACCOUNT);
	
	const char *realName = GetIniString(IDS_INI_REAL_NAME);
	
	m_Config.size = sizeof(emsMailConfig);
	m_Config.eudoraWnd = AfxGetMainWnd() ? &(AfxGetMainWnd()->m_hWnd) : NULL;
	m_Config.configDir = new char[pluginDir.GetLength() + 1];
	strcpy(m_Config.configDir, pluginDir);

	
	m_Config.userAddr.size = sizeof(emsAddress);
	m_Config.userAddr.address = new char[strlen(retAdd) + 1];
	strcpy(m_Config.userAddr.address, retAdd);
	m_Config.userAddr.realname = new char[strlen(realName) + 1];
	strcpy(m_Config.userAddr.realname, realName);
	m_Config.userAddr.next = NULL;

	m_Config.callBacks = new emsCallBack;
	m_Config.callBacks->size = sizeof(emsCallBack);
    m_Config.callBacks->EMSProgressCB = &EMSV3Progress;
    m_Config.callBacks->EMSGetMailBoxCB = &EMSGetMailBox;		
    m_Config.callBacks->EMSSetMailBoxTagCB = &EMSSetMailBoxTag;			
	m_Config.callBacks->EMSGetPersonalityCB = &EMSGetPersonality;
	m_Config.callBacks->EMSRegenerateCB = &EMSRegenerate;
	m_Config.callBacks->EMSGetDirectoryCB = &EMSGetDirectory;
	m_Config.callBacks->EMSGetMailBoxTagCB = &EMSGetMailBoxTag;
	m_Config.callBacks->EMSGetPersonalityInfoCB = &EMSGetPersonalityInfo;
	m_Config.callBacks->EMSEnumAddressBooksCB = &EMSEnumAddressBooks;
	m_Config.callBacks->EMSAddABEntryCB = &EMSAddABEntry;

}

CEMSMailConfig::~CEMSMailConfig()
{
	delete [] m_Config.configDir;
	delete [] m_Config.userAddr.realname;
	delete [] m_Config.userAddr.address;
	delete m_Config.callBacks;
}

// =======================================================================
// CEMSPluginInfo
// =======================================================================
CEMSPluginInfo::CEMSPluginInfo()
{
	m_PluginInfo.size = sizeof(emsPluginInfo);
	m_PluginInfo.numTrans = 0;
	m_PluginInfo.numAttachers = 0;
	m_PluginInfo.numSpecials = 0;
	m_PluginInfo.desc = "";
	m_PluginInfo.id = 0;
	m_PluginInfo.mem_rqmnt = 0;
	m_PluginInfo.icon = NULL;
	m_PluginInfo.numMBoxContext = 0;
	m_PluginInfo.idleTimeFreq = 0;
}

// =======================================================================
// CEMSTransInfo
// ===================================, )
CEMSTransInfo::CEMSTransInfo(long id)
{
	m_TransInfo.size = sizeof(emsTranslator);
    m_TransInfo.id = id;
    m_TransInfo.type = 0;		
    m_TransInfo.flags = 0;	
    m_TransInfo.desc = "";	
    m_TransInfo.icon = NULL;	
    m_TransInfo.properties = "";	
}


// =======================================================================
// CEMSDataFile
// =======================================================================
CEMSDataFile::CEMSDataFile(long context)
{
	m_DataFile.size = sizeof(emsDataFile);
    m_DataFile.context = context;
    m_DataFile.info = NULL;		
    m_DataFile.header = NULL; 		
    m_DataFile.fileName = NULL;
}

// =======================================================================
// CEMSDataFile
// =======================================================================
CEMSDataFile::CEMSDataFile(long context, const char* filename)
{
	m_DataFile.size = sizeof(emsDataFile);
    m_DataFile.context = context;
    m_DataFile.info = NULL;		
    m_DataFile.header = NULL; 		
    m_DataFile.fileName = new char[strlen(filename) + 1];
    strcpy(m_DataFile.fileName, filename);
}
// =======================================================================
// CEMSDataFile
// =======================================================================
CEMSDataFile::~CEMSDataFile()
{
	delete [] m_DataFile.fileName;

	DestroyMimeInfo();
	DestroyHeaderInfo();
}


// =======================================================================
// CEMSResultStatus
// =======================================================================
CEMSResultStatus::CEMSResultStatus()
{
	m_ResultStatus.size = sizeof(emsResultStatus);
    m_ResultStatus.desc = NULL;
    m_ResultStatus.error = NULL;		
    m_ResultStatus.code = 0;			
}
// =======================================================================
// CEMSDataFile
// =======================================================================
CEMSResultStatus::~CEMSResultStatus()
{
}


// =======================================================================
// CEMSCallbacks
// =======================================================================
CEMSCallbacks::CEMSCallbacks()
{
	m_Callbacks.size = sizeof(emsCallBack);
    m_Callbacks.EMSProgressCB = &EMSV3Progress;
    m_Callbacks.EMSGetMailBoxCB = &EMSGetMailBox;		
    m_Callbacks.EMSSetMailBoxTagCB = &EMSSetMailBoxTag;			
	m_Callbacks.EMSGetPersonalityCB = &EMSGetPersonality;
}

// =======================================================================
// CEMSIdleData
// =======================================================================
CEMSIdleData::CEMSIdleData()
{
	m_IdleData.size = sizeof(emsIdleData);
    m_IdleData.flags = 0;
    m_IdleData.idleTimeFreq = 0;		
    m_IdleData.Progress = &EMSV3Progress;			
}




// =======================================================================
// CAttacher
// =======================================================================
CAttacher::CAttacher(CTransAPI *ptlAPI, long id): 
		m_TransAPI(ptlAPI)
{
	long error = 0;

	// Initialize data
	m_MenuItem.size = sizeof(emsMenu);
	m_MenuItem.id = id;
	m_MenuItem.desc = NULL;
	m_MenuItem.icon = NULL;
	m_MenuItem.flags = 0;

	FARPROC fnAttachInfo = GetProcAddress(m_TransAPI->GetInstance(), EMS_ATTACH_INFO);
	if (fnAttachInfo)
	{
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_ATTACH_INFO);
		if (ptlAPI->GetVersion() >= EMS_PB_VERSION)
		{
			error = (*(ems3_attacher_info_t *)fnAttachInfo)(m_TransAPI->GetGlobals(),&m_MenuItem);
			if (error != EMSR_OK)
				DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_ATTACH_INFO);
		}
	}
}

// =======================================================================
CAttacher::~CAttacher()
{
	if (m_TransAPI)
	{
		if (m_MenuItem.desc && *m_MenuItem.desc && m_TransAPI->m_fnFreeFunc)	
			m_TransAPI->m_fnFreeFunc(m_MenuItem.desc);

		if (m_MenuItem.icon && m_TransAPI->m_fnFreeFunc)	
			m_TransAPI->m_fnFreeFunc((void FAR*)m_MenuItem.icon);
	}
}

// =======================================================================
CAttacher::GetIcon(HICON &icn)
{
	if (m_MenuItem.icon && *m_MenuItem.icon)
	{
		icn = *m_MenuItem.icon; 
		return TRUE;
	}
	else
	{	// Get the bogus icon
		icn = QCLoadIcon(IDI_DEFAULT_PLUGIN_ATTACH);
		return TRUE;
	}

	return FALSE;
}


// =======================================================================
long CAttacher::MenuHook(CCompMessageDoc* msg)
{
	long error = 0;
	ems3_attacher_hook_t *fnAttachHook;

	if (m_TransAPI)
		fnAttachHook = (ems3_attacher_hook_t *)m_TransAPI->GetFunction(EMS_ATTACH_HOOK);
	if (fnAttachHook)
	{
		char attachDir[_MAX_PATH + 1];
		GetIniString(IDS_INI_AUTO_RECEIVE_DIR, attachDir, _MAX_PATH);
		if ( (attachDir[0] == 0) || !::FileExistsMT(attachDir, TRUE))
			sprintf(attachDir, "%s%s", (const char *)EudoraDir, (const char *)CRString(IDS_ATTACH_FOLDER)); 

		emsDataFileP *dataFile = NULL;
		long numAttachments = 0;
		
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_ATTACH_HOOK);
		error = (*fnAttachHook)(m_TransAPI->GetGlobals(), &m_MenuItem, attachDir, &numAttachments, &dataFile); 
		if (error == EMSR_OK)
		{
			for (int i = 0; i < numAttachments; i++)
			{
				char full[_MAX_PATH];
				// Grab off the first datafile
				emsDataFileP df = dataFile[i];

				

				if (::FileExistsMT(df->fileName) && _fullpath( full, df->fileName, _MAX_PATH ))
				{
					msg->GetView()->GetParentFrame()->SendMessage( umsgAddAttachment, 0, LPARAM( full) );
					
					// If the suggested directory is used, Eudora will manage
					CString atDir = attachDir;
					CString atFile = df->fileName;
					atDir.MakeLower();
					atFile.MakeLower();

					if (atFile.Find(atDir) >= 0)
						msg->m_Sum->SetFlagEx(MSFEX_AUTO_ATTACHED);
				}
				else
					ErrorDialog(IDS_EMS_FILE_NOT_FOUND,(const char *)df->fileName);
				
				m_TransAPI->m_fnFreeFunc(df->fileName);
				m_TransAPI->m_fnFreeFunc(df);
			}
			m_TransAPI->m_fnFreeFunc(dataFile);
		}
	}
	else
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_ATTACH_HOOK);

	return error;
}

// =======================================================================
// CSpecial
// =======================================================================
CSpecial::CSpecial(CTransAPI *ptlAPI, long id) 
{
	long error = 0;

	m_TransAPI = ptlAPI;
	// Initialize data
	m_MenuItem.size = sizeof(emsMenu);
	m_MenuItem.id = id;
	m_MenuItem.desc = NULL;
	m_MenuItem.icon = NULL;
	m_MenuItem.flags = 0;


	FARPROC fnSpecialInfoFun = GetProcAddress(m_TransAPI->GetInstance(), EMS_SPECIAL_INFO);
	if (fnSpecialInfoFun)
	{
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_SPECIAL_INFO);

		if (ptlAPI->GetVersion() >= EMS_PB_VERSION)
		{
			error = (*(ems3_special_info_t *)fnSpecialInfoFun)(m_TransAPI->GetGlobals(),&m_MenuItem);
			if (error != EMSR_OK)
				DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_SPECIAL_INFO);
		}
	}
}

// =======================================================================
CSpecial::GetIcon(HICON &icn)
{
	if (m_MenuItem.icon && *m_MenuItem.icon)
	{
		icn = *m_MenuItem.icon; 
		return TRUE;
	}
	else
	{	// Get the bogus icon
		icn = QCLoadIcon(IDI_DEFAULT_PLUGIN_TOOL);
		return TRUE;
	}

	return FALSE;
}

// =======================================================================
long CSpecial::MenuHook(CCompMessageDoc* msg)
{
	long error = 0;
	ems3_special_hook_t *fnHook;

	if (m_TransAPI)
		fnHook = (ems3_special_hook_t *)m_TransAPI->GetFunction(EMS_SPECIAL_HOOK);
	if (fnHook)
	{
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_SPECIAL_HOOK);
		error = (*fnHook)(m_TransAPI->GetGlobals(), &m_MenuItem); 
	}
	else
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_SPECIAL_HOOK);

	return error;
}

// =======================================================================
// CMBoxContext
// =======================================================================
CMBoxContext::CMBoxContext(CTransAPI *ptlAPI, long id) 
{
	long error = 0;

	m_TransAPI = ptlAPI;
	// Initialize data
	m_MenuItem.size = sizeof(emsMenu);
	m_MenuItem.id = id;
	m_MenuItem.desc = NULL;
	m_MenuItem.icon = NULL;
	m_MenuItem.flags = 0;


	FARPROC fnMboxCMInfoFun = GetProcAddress(m_TransAPI->GetInstance(), EMS_MBOX_CONEXT_INFO);
	if (fnMboxCMInfoFun)
	{
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_MBOX_CONEXT_INFO);

		if (ptlAPI->GetVersion() >= EMS_PB_VERSION)
		{
			error = (*(ems_mbox_context_info_t *)fnMboxCMInfoFun)(m_TransAPI->GetGlobals(),&m_MenuItem);
			if (error != EMSR_OK)
				DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_MBOX_CONEXT_INFO);
		}
	}
}

// =======================================================================
long CMBoxContext::ContextHook(CTocDoc* toc)
{
	long error = 0;
	ems_mbox_context_hook_t *fnHook;

	if (m_TransAPI)
		fnHook = (ems_mbox_context_hook_t *)m_TransAPI->GetFunction(EMS_MBOX_CONEXT_HOOK);
	if (fnHook)
	{
		emsMBox eMbox;
		eMbox.size = sizeof(emsMBox);


		// JES: I changed the behavior of this to return the mailboxes relative
		// path instead of it's absolute path to be more like EMSGetMailBox()
		// The Peanut folks are the only people using this as far as I know.

		eMbox.name = new char[(strlen(toc->MBFilename()) - EudoraDirLen) + 1];
		CString WholePath = toc->MBFilename();
		CString MboxName = WholePath.Right(WholePath.GetLength() - EudoraDirLen);

		strcpy(eMbox.name, MboxName);
		eMbox.hash = 0;


		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_MBOX_CONEXT_HOOK);
		error = (*fnHook)(m_TransAPI->GetGlobals(), &eMbox, &m_MenuItem); 

		delete [] eMbox.name;
	}
	else
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_MBOX_CONEXT_HOOK);

	return error;
}


// =======================================================================
// CTranslator 
// The Constructor
// Record basic info we need to get functions out of the API and 
// load up basic info about the translator
// =======================================================================
CTranslator::CTranslator(CTransAPI *tlAPI, short transID) : 
	m_TransAPI(tlAPI), m_TransID(transID) 
{
	InitTranslator();
}

// =======================================================================
// CTranslator 
// The Constructor
// =======================================================================
BOOL CTranslator::InitTranslator() 
{
	long error = 0;
	m_Desc = NULL;  
	m_HIcon = NULL; 
	m_Type = 0; 
	m_Subtype = 0;  
	m_TLflags = 0;
	m_Properties.Empty();
	
	DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_TRANS_INFO);

	FARPROC fnTransInfo = GetProcAddress(m_TransAPI->GetInstance(), EMS_TRANS_INFO);

	if (fnTransInfo)
	{

		if (m_TransAPI->GetVersion() == PARAM_VERSION)
			error = (*(ems2_translator_info_t *)fnTransInfo)(m_TransAPI->GetGlobals(), m_TransID, &m_Type, &m_Subtype, &m_TLflags, &m_Desc, &m_HIcon);
		else if (m_TransAPI->GetVersion() >= EMS_PB_VERSION)
		{
			CEMSTransInfo transInfo(m_TransID);	
			error = (*(ems3_translator_info_t *)fnTransInfo)(m_TransAPI->GetGlobals(),&(transInfo.m_TransInfo));
			if (error == EMSR_OK)
			{
				m_Type = transInfo.m_TransInfo.type;
				m_TLflags = transInfo.m_TransInfo.flags;
				m_Desc = transInfo.m_TransInfo.desc;
				m_HIcon = transInfo.m_TransInfo.icon;
			}
		}
		if (error != EMSR_OK)
		{
			DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_TRANS_INFO);
			return FALSE;
		}
		
		// Set our globals to see if we need to get the addresses
		BOOL bBasic = m_TLflags & EMSF_BASIC_HEADERS;
		BOOL bRaw = m_TLflags & EMSF_ALL_HEADERS;
		if (bBasic || bRaw)
		{
			if (m_TLflags & EMSF_ON_ARRIVAL)
			{
				if (bBasic) GetTransMan()->SetHeaderFlag(EMSAPI_ARRIVAL_BASIC);
				if (bRaw) GetTransMan()->SetHeaderFlag(EMSAPI_ARRIVAL_RAW);
			}
			if (m_TLflags & EMSF_ON_DISPLAY)
			{
				if (bBasic) GetTransMan()->SetHeaderFlag(EMSAPI_DISPLAY_BASIC);
				if (bRaw) GetTransMan()->SetHeaderFlag(EMSAPI_DISPLAY_RAW);
			}
			if (m_TLflags & EMSF_ON_REQUEST)
			{
				if (bBasic) GetTransMan()->SetHeaderFlag(EMSAPI_REQUEST_BASIC);
				if (bRaw) GetTransMan()->SetHeaderFlag(EMSAPI_REQUEST_RAW);
			}
			if (m_TLflags & EMSF_Q4_TRANSMISSION)
			{
				if (bBasic) GetTransMan()->SetHeaderFlag(EMSAPI_XMISSION_BASIC);
				if (bRaw) GetTransMan()->SetHeaderFlag(EMSAPI_XMISSION_RAW);
			}
		}

		return TRUE;
	}
	return FALSE;
	

}

	
// =======================================================================
// UpdateTranlator
// =======================================================================
BOOL CTranslator::UpdateTranlator()
{
	// Clean out the old stuff
	if (m_TransAPI)
	{
		if (m_Desc && m_TransAPI->m_fnFreeFunc)	
			m_TransAPI->m_fnFreeFunc(m_Desc);

		if (m_HIcon && m_TransAPI->m_fnFreeFunc)	
			m_TransAPI->m_fnFreeFunc((void FAR*)m_HIcon);
	}
	// Update the tranlator
	return InitTranslator();
}

// =======================================================================
// CTranslator
// =======================================================================
CTranslator::~CTranslator()
{  
	if (m_TransAPI)
	{
		if (m_Desc && m_TransAPI->m_fnFreeFunc)	
			m_TransAPI->m_fnFreeFunc(m_Desc);

		if (m_HIcon && m_TransAPI->m_fnFreeFunc)	
			m_TransAPI->m_fnFreeFunc((void FAR*)m_HIcon);
	}
}

// =======================================================================
// getDescription
// =======================================================================
BOOL CTranslator::getDescription(CString &desc)
{
	if (m_Desc)
	{
		desc = m_Desc; 
		return TRUE;
	}
	else
	{
		desc = GetIniString(IDS_EMS_TRANSLATOR_MISSING);
	}
	return FALSE;
}


// =======================================================================
// GetIcon
// =======================================================================
BOOL CTranslator::GetIcon(HICON &icn)
{
	if (m_HIcon && *m_HIcon)
	{
		icn = *m_HIcon; 
		return TRUE;
	}
	else
	{	// Get the bogus icon
		if (matchContext(EMSF_ON_REQUEST))
			icn = QCLoadIcon(IDI_DEFAULT_PLUGIN_REQUEST);
		else
			icn = QCLoadIcon(IDI_NO_TRANSLATOR);
	}

	return FALSE;
}

// =======================================================================
// This will Do the actual FileBased Translation
// =======================================================================
long CTranslator::Translate(CTLMime *mimeInfo, const long context, CTLAddress *addInfo, const char *raw, const char *cooked, long *retCode)
{
	long ret = 0;
	char *desc = NULL;
	HICON *outIcon = NULL;
	// Translate
	if (EMSR_NOW == (ret = this->CanTranslate(context,
								mimeInfo,
								addInfo)))
	{
		CTLMime outMIME;	

		ret = this->TranslateFile(context,				// context,
								mimeInfo,
								raw,							// inputFile,
								addInfo,
								cooked,							//*outputFile,
								&outMIME,						//**returnedMIME,
								&outIcon,						//**out_icon,
								&desc,							//**out_desc,
								retCode);
		
		// If we go it back, copy it over, free it up
		if (outMIME.GetMimeType())
		{
			mimeInfo->CreateMimeType(outMIME.GetMimeType());
		}
		if (desc && *desc)
		{ 
			if (context & EMSF_ON_DISPLAY)
			{  
				g_OutputDesc += "\r\n";
				g_OutputDesc += desc;
			}
			else
				g_OutputDesc.Empty(); 
			m_TransAPI->m_fnFreeFunc(desc);
		}
		if (outIcon)
			m_TransAPI->m_fnFreeFunc((void FAR*)outIcon);


	}
	return ret;
}


// =======================================================================
// This is the protocol based tranalator
// =======================================================================
long CTranslator::TranslateMessage(
QCProtocol*	pProtocol,
CTLAddress* paddInfo)
{
	long ret = 0;
	CTLAddress tAddress;
	CTLMime tMime; 
	UINT howSel = ST_CARET; 
	long retCode = 0;
	BOOL fancyTranslator = FALSE;

	// get unique file names
	CString raw = ::GetTmpFileNameMT();			
	CString cooked = ::GetTmpFileNameMT();		

	// Set up a fancy MIME tyep
	tMime.CreateMimeType(TRUE);								// Get MIME Stuff

	// First see if it can translate text/html?
	retCode = CanTranslate(EMSF_ON_REQUEST, &tMime, &tAddress);

	// EMSR_NOW is checked too becuase systran did things wrong and it's too late
	// for them to fix their plugin!!!
	if (retCode == EMSR_OK || retCode == EMSR_NOW)
		fancyTranslator = TRUE;
	else
		tMime.CreateMimeType();

	// Write out to a file
	{
		CString sSel;
		CSaveAs sa;
		if (sa.CreateFile(raw))
		{
			if (! (m_TLflags & EMSF_ALL_TEXT) )
			{
				if (fancyTranslator )
					pProtocol->GetSelectedHTML(sSel);
				else
					pProtocol->GetSelectedText(sSel);
			}
		}
		
		if (sSel.IsEmpty())
		{
			howSel = ST_ALL;
			if (fancyTranslator)
				pProtocol->GetAllHTML(sSel);
			else
				pProtocol->GetAllText(sSel);
		}
		sa.PutText(sSel);
		// sa has to go out of scope to close file
	}


	ret = Translate(&tMime, EMSF_ON_REQUEST, paddInfo, raw, cooked, &retCode);
	

	// See if we what is returned is fancy?
	CRString txt(IDS_MIME_TEXT); 
	CRString html(IDS_MIME_HTML); 
	CRString plain(IDS_MIME_PLAIN); 
	if ( tMime.GetType() && tMime.GetSubtype())
	{
		if (!txt.CompareNoCase(tMime.GetType()) && !html.CompareNoCase(tMime.GetSubtype()) )
			fancyTranslator = TRUE;
		else if(!txt.CompareNoCase(tMime.GetType()) && !plain.CompareNoCase(tMime.GetSubtype()) )
			fancyTranslator = FALSE;
	}

	if (ret == EMSR_OK)
	{
		// Read up the result
		CFile cookedFile;
		char* cookedText;
		if( cookedFile.Open( cooked, CFile::modeRead ) )
		{
			UINT uLen = cookedFile.GetLength();

			cookedText = new char[ uLen + 1];

			if (!cookedText)
			{
				return FALSE;
			}

			memset( cookedText, 0, uLen + 1 );

			DWORD dwRead = cookedFile.ReadHuge(cookedText, uLen );

			if( uLen == dwRead )
			{
				// replace the text
				
				if( howSel == ST_CARET )
				{
					if (fancyTranslator)
						pProtocol->SetSelectedHTML( cookedText,FALSE );
					else
						pProtocol->SetSelectedText( cookedText,FALSE );
				}
				else
				{	
					if (fancyTranslator)
						pProtocol->SetAllHTML( cookedText,FALSE );
					else
						pProtocol->SetAllText( cookedText,FALSE );
				}
			}

			cookedFile.Close();
			delete [] cookedText;
		}
		else
		{	//
			// Oops.  Whoever wrote this code didn't account for file 
			// I/O errors.  WKS 97.08.08.
			//
			ASSERT(0);		// headed for trouble below...
		}
	}
	else if (ret == EMSR_DATA_UNCHANGED)
	{
		// Just leave the selection on!
	}
	
	VERIFY(DeleteFile(raw));
	VERIFY(DeleteFile(cooked));
	
	return ret;	
}




// =======================================================================
// This is the edit control based tranalator
// =======================================================================
long CTranslator::TranslateMessage(CWnd* edtCtrl, CTLAddress* paddInfo)
{
	CEdit *editControl = NULL;
	CRichEditCtrl *richEditCtrl = NULL;
	long retCode = 0;

	if (edtCtrl->IsKindOf(RUNTIME_CLASS(CEdit)))
		editControl = (CEdit *)edtCtrl;
	else if (edtCtrl->IsKindOf(RUNTIME_CLASS(CEditView)))
		editControl = &(((CEditView *)edtCtrl)->GetEditCtrl());
	else if (edtCtrl->IsKindOf(RUNTIME_CLASS(CRichEditCtrl)))
		richEditCtrl = (CRichEditCtrl *)edtCtrl;
	else if (edtCtrl->IsKindOf(RUNTIME_CLASS(CRichEditView)))
		richEditCtrl = &(((CRichEditView *)edtCtrl)->GetRichEditCtrl());
	else
		return -1;

	long ret = 0;
	CTLAddress tAddress;
	CTLMime tMime; 
	UINT howSel = ST_CARET; 

	// get unique file names
	CString raw = ::GetTmpFileNameMT();			
	CString cooked = ::GetTmpFileNameMT();		

	// Write out to a file
	{
		CString sSel;
		CSaveAs sa;
		if (sa.CreateFile(raw))
			if (richEditCtrl)
				sSel = CURLEdit::GetSelEditText(richEditCtrl, howSel);
			else
				sSel = CURLEdit::GetSelEditText(editControl, howSel);
		if (sSel.IsEmpty())
		{
			howSel = ST_ALL;
			if (richEditCtrl)
				sSel = CURLEdit::GetSelEditText(richEditCtrl, ST_ALL);
			else
				sSel = CURLEdit::GetSelEditText(editControl, ST_ALL);
		}
		sa.PutText(sSel);
		// sa has to go out of scope to close file
	}

	tMime.CreateMimeType();								// Get MIME Stuff

	ret = Translate(&tMime, EMSF_ON_REQUEST, paddInfo, raw, cooked, &retCode);

	if (ret == EMSR_OK)
	{
		// Read up the result
		JJFile cookedFile;
		char *cookedText = NULL;
		if (FAILED(cookedFile.Open(cooked, O_RDONLY)))
		{
			//
			// Oops.  Whoever wrote this code didn't account for file 
			// I/O errors.  WKS 97.08.08.
			//
			ASSERT(0);		// headed for trouble below...
		}

		const unsigned int MaxLength = 32 * 1024L;	// biggest message size for 16 bit, read size 32 bit
	
		while (1)
		{
			delete [] cookedText;

			cookedText = new char[MaxLength + 1];
			if (!cookedText)
				return FALSE;
			memset(cookedText,0,MaxLength);
			HRESULT hrRead = cookedFile.Read(cookedText, MaxLength);

				// Get the view
			if (richEditCtrl)
				CURLEdit::SetSelEditText(richEditCtrl, cookedText, ST_CARET);
			else
				CURLEdit::SetSelEditText(editControl, cookedText, ST_CARET);
			if (FAILED(hrRead) || (S_FALSE == hrRead))
				break;
		}
		
		cookedFile.Close();
		edtCtrl->UpdateWindow();

		delete [] cookedText;
	}
	else if (ret == EMSR_DATA_UNCHANGED)
	{
		// Just leave the selection on!
	}
	
	DeleteFile(raw);
	DeleteFile(cooked); 
	
	return ret;	
}




// =======================================================================
// CanTranslate: this call the dll function
// =======================================================================
long CTranslator::CanTranslate(
        long        context,
		CTLMime		*givenMIME,
		CTLAddress	*addresses)
{
	CString err;
	FARPROC fnCanTranslate;
	long    result;
	char	*errorMessage = NULL;
	long	errorCode = 0;

	fnCanTranslate = GetProcAddress( m_TransAPI->GetInstance(), EMS_CAN_TRANS);

	if (fnCanTranslate)
	{
		char *props = NULL;
		if (!m_Properties.IsEmpty())
		{
			props = new char[m_Properties.GetLength() + 1];
			strcpy(props, m_Properties);
		}
		
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_CAN_TRANS);
		

		if (m_TransAPI->GetVersion() == PARAM_VERSION)
		{
			emsMIMEtype * v2Mime = givenMIME->DegradeMimeType();
			result =(*(ems2_can_translate_t *)fnCanTranslate)(m_TransAPI->GetGlobals(),
																		context,
																		m_TransID,
																		v2Mime,
																		addresses ? addresses->GetAddressList() : NULL,
																		props,
																		&errorMessage,
 																		&errorCode);
			FreeMimeTypeV2(v2Mime);

			if (errorMessage && *errorMessage)
			{
				err = errorMessage;
				if (m_TransAPI->m_fnFreeFunc)
					m_TransAPI->m_fnFreeFunc(errorMessage);
			}

		}
		else if(m_TransAPI->GetVersion() >= EMS_PB_VERSION)
		{
			CEMSTransInfo	ETI(m_TransID);
			if (props)
				ETI.m_TransInfo.properties = props;


			// Do all this MIME massaging because we changed format ... fix later!!!!
			
			// Create all the PB's
			CEMSDataFile EDF(context);
			if (addresses)
				EDF.AddHeaders(&addresses->m_HeaderData);
			if (givenMIME)
			EDF.AddMIME(givenMIME->GetMimeType());
			
			CEMSResultStatus ETS;
			emsResultStatus ResultSt= ETS.m_ResultStatus;
			
			result =(*(ems3_can_translate_t *)fnCanTranslate)(m_TransAPI->GetGlobals(),
																		&ETI.m_TransInfo,  
																		&EDF.m_DataFile, 
																		&ResultSt);

			EDF.m_DataFile.header = NULL;	// We set it to NULL so it doesn't get wacked when EDF goes out of scope 
											// because we're sharing it with another EmsDatafile because whoever wrote it was a mean person.


			if (ResultSt.error && *(ResultSt.error))
			{
				err = ResultSt.error;
				if (m_TransAPI->m_fnFreeFunc)
					m_TransAPI->m_fnFreeFunc(ResultSt.error);
			}
		}
		delete [] props;
	}
	else
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_CAN_TRANS);

	TranslatorError(result, (const char *)err, errorCode);
	return result; 
}


// EnforceMIMEFormatting
//
// Check to make sure it is indeed MIME. If not then just slap a blank line before the body and say it's t/p
void CTranslator::EnforceMIMEFormatting(char* filename)
{
	HANDLE theFile = CreateFile( filename, GENERIC_READ,  FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (theFile == INVALID_HANDLE_VALUE)
	{
		ASSERT(0);
		return;
	}

	char ReadBuffer[1024];
	unsigned long numBytes;

	VERIFY(ReadFile(theFile, ReadBuffer, sizeof(ReadBuffer) - 1, &numBytes, NULL));

	if (strnicmp(ReadBuffer, "Mime-Version", 12) == 0)
	{
		VERIFY(CloseHandle(theFile));
		return;
	}

	CString NewFilename(::GetTmpFileNameMT());

	HANDLE newFile = CreateFile( NewFilename, GENERIC_WRITE,  0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (newFile == INVALID_HANDLE_VALUE)
	{
		ASSERT(0);
		VERIFY(CloseHandle(theFile));
		return;
	}
	
	VERIFY(SetFilePointer(theFile, 0, 0, FILE_BEGIN) != 0xFFFFFFFF);
	VERIFY(WriteFile(newFile, _T("Mime-Version: 1.0\r\nContent-Type: text/plain\r\n\r\n"), 47, &numBytes, NULL));

	while (ReadFile(theFile, ReadBuffer, sizeof(ReadBuffer) - 1, &numBytes, NULL) && numBytes > 0)
		VERIFY(WriteFile(newFile, ReadBuffer, numBytes, &numBytes, NULL));

	VERIFY(CloseHandle(theFile));
	VERIFY(CloseHandle(newFile));
	VERIFY(DeleteFile(filename));
	VERIFY(MoveFile(NewFilename, filename));
}


// =======================================================================
// TranslateFile: this call the dll function
// =======================================================================
long CTranslator::TranslateFile(
        long            context,
		CTLMime 		*givenMIME,
	    const char		*inputFile,
		CTLAddress		*addressList,
		const char		*outputFile,
		CTLMime 		*returnedMIME,
		HICON			**out_icon,
		char			**out_desc,
		long			*out_code)
{
	CString err;
	FARPROC fnTranslateFile;
	long    result;
	char	*errorMessage = NULL;

	g_ProgressMsg.Empty();
	g_ProgressMsg.Format(CRString(IDS_EMS_PROCESSING), m_Desc);
	g_ProgStarted = FALSE;

	fnTranslateFile = GetProcAddress(m_TransAPI->GetInstance(), EMS_TRANS_FILE);

	if (fnTranslateFile)
	{
		char *props = NULL;
		if (!m_Properties.IsEmpty())
		{
			props = new char[m_Properties.GetLength() + 1];
			strcpy(props, m_Properties);
		}

		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_TRANS_FILE);

		if (m_TransAPI->GetVersion() == PARAM_VERSION)
		{
			// Set up the Progress stuff
			emsProgress progress = &EMSProgress;
			
			emsMIMEtype * v2Mime = givenMIME->DegradeMimeType();
			emsMIMEtype * v2OutMime = NULL;

			result =(*(ems2_translate_file_t *)fnTranslateFile)(m_TransAPI->GetGlobals(), 
										context, 
										m_TransID, 
										v2Mime, 
										(char *)inputFile, 
										addressList ? addressList->GetAddressList() : NULL,
										props, 
										progress, 
										&v2OutMime, 
										(char *)outputFile, 
										out_icon, 
										out_desc, 
										&errorMessage, 
										out_code);
			FreeMimeTypeV2(v2Mime);
			if (v2OutMime)
			{
				returnedMIME->UpgradeMimeType(v2OutMime);
				FreeMimeTypeV2(v2OutMime, m_TransAPI->m_fnFreeFunc);
			}
			
			if (errorMessage && *errorMessage)
			{
				err = errorMessage;
				if (m_TransAPI->m_fnFreeFunc)
					m_TransAPI->m_fnFreeFunc(errorMessage);
			}	
		}
		else if(m_TransAPI->GetVersion() >= EMS_PB_VERSION)
		{
			// Set up the Progress stuff
			emsProgress3 progress = &EMSV3Progress;

			CEMSTransInfo	ETI(m_TransID);
			if (props)
				ETI.m_TransInfo.properties = props;


			// Create all the PB's
			CEMSDataFile inDataFile(context, inputFile);
			if (addressList)
				inDataFile.AddHeaders(&addressList->m_HeaderData);
			if (givenMIME)
				inDataFile.AddMIME(givenMIME->GetMimeType());

			CEMSDataFile outDataFile(0, outputFile);
			outDataFile.AddMIME(returnedMIME->GetMimeType());
			
			
			CEMSResultStatus ETS;
			emsResultStatus ResultSt= ETS.m_ResultStatus;

			result =(*(ems3_translate_file_t *)fnTranslateFile)(m_TransAPI->GetGlobals(),
																		&ETI.m_TransInfo,  
																		&inDataFile.m_DataFile, 
																		progress,      
																		&outDataFile.m_DataFile,		
																		&ResultSt);
			

			if (ResultSt.error && *(ResultSt.error))
			{
				err = ResultSt.error;
				if (m_TransAPI->m_fnFreeFunc)
					m_TransAPI->m_fnFreeFunc(ResultSt.error);
			}
			if (result == EMSR_OK)
			{
				// Get the returned MIME type (bug fix, 4/2/97)
				returnedMIME->CreateMimeType(outDataFile.m_DataFile.info);

				// since we got the data out of it that we need ... free it up
				FreeMimeTypeV3(&outDataFile.m_DataFile, m_TransAPI->m_fnFreeFunc);

				// PGP passes back sometimes a non-MIME message even though it claims it'll pass back
				// MIME. In these cases we need to treat it like text/plain, we can put in all the headers
				// or just throw in a blank line and let it be placed in the body. Guess which I'm doing.
				if (context != EMSF_ON_REQUEST)
				{
					CTranslator *trltor = GetTransMan()->GetTranslator(m_TransAPI->GetID(), m_TransID);
					if (trltor->GetFlags() & EMSF_GENERATES_MIME)
						EnforceMIMEFormatting(outDataFile.m_DataFile.fileName);
				}

				// no more icon
				*out_icon = NULL;

				// here's the return
				*out_desc = ResultSt.desc;
				*out_code = ResultSt.code;
			}
		}
		
		EMSProgress(EMS_PROGRESS_END);
		delete [] props;
	}
	else
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_TRANS_FILE);

	TranslatorError(result, (const char *)err, *out_code);
	return result;
}

// =======================================================================
// QueueTrans: 
// =======================================================================
BOOL CTranslator::QueueTrans(const long context, CString &paramProps, BOOL state /*=FALSE*/)
{
	if (EMSF_Q4_COMPLETION == context)
	{
		FARPROC fnQueueFun;
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_QUEUE_TRANS);

		fnQueueFun = GetProcAddress(m_TransAPI->GetInstance(), EMS_QUEUE_TRANS);

		if (fnQueueFun)
		{
			long ret = 0;
			short selected = (short)state;
			char *props = NULL;
			if (!paramProps.IsEmpty())
			{
				props = new char[paramProps.GetLength() + 1];
				strcpy(props, paramProps);
			}
			char *oldProps = props;

			if (m_TransAPI->GetVersion() == PARAM_VERSION)
			{
				ret = (*(ems2_queued_properties_t *)fnQueueFun)(m_TransAPI->GetGlobals(), 
																EMSF_Q4_COMPLETION | EMSF_Q4_TRANSMISSION, 
																m_TransID, 
																&selected, 
																&props);
				paramProps = props;
			}
			else if(m_TransAPI->GetVersion() >= EMS_PB_VERSION)
			{
				CEMSTransInfo eTransInfo(m_TransID);
				long sel = selected;
				eTransInfo.m_TransInfo.properties = props; // Bug fix, 4/2/97

				ret = (*(ems3_queued_properties_t *)fnQueueFun)(m_TransAPI->GetGlobals(),
																	&(eTransInfo.m_TransInfo), 
																	&sel);
				selected = (short)sel;
				paramProps = eTransInfo.m_TransInfo.properties;
			}
			
			if (m_TransAPI->m_fnFreeFunc && (oldProps != props))
				m_TransAPI->m_fnFreeFunc(props);
			
			delete [] oldProps;
			
			return (selected);	
		}
		else 
			DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_QUEUE_TRANS);
	}
	// Just toggle it!
	return !state;
}


// =======================================================================
// TranslatorError: Error message will be handled here
// =======================================================================
BOOL CTranslator::TranslatorError(const long funReturn, const char *error, const long errorCode)
{
	DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_OCCURED,  error, errorCode);

	if  (!error && 
			funReturn >= EMSR_UNKNOWN_FAIL && 
			funReturn <= EMSR_INVALID && 
			funReturn != EMSR_CANT_TRANS && 
			funReturn != EMSR_ABORTED &&
			funReturn != EMSR_DATA_UNCHANGED)
	{
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_OCCURED, error, errorCode);
		ErrorDialog(IDS_EMSAPI_ERROR, (const char*)m_Desc, (const char*)CRString(IDS_EMSAPI_ERROR + funReturn + 1));
	}

	if (error && *error)
	{
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_OCCURED, error, errorCode);
		ErrorDialog(IDS_TRANS_ERROR, (const char*)m_Desc, (const char*)error,  errorCode);
	}

	return TRUE;
}


// =======================================================================
// matchType::See if Translator is of the given type 
// =======================================================================
BOOL CTranslator::matchType(const long transType)
{
	return(m_Type == transType);
}

// =======================================================================
// matchContext::See if Translator is of the given type 
// =======================================================================
BOOL CTranslator::matchContext(const unsigned long  flags)
{
	if (flags & m_TLflags)
		return (TRUE);
	return (FALSE);
}


// =======================================================================
// matchType::See if translator matches selection criteria
// =======================================================================
BOOL CTranslator::matchType(const long transType,const unsigned long  flags)
{
    unsigned long  flagBit, flagsChecked;

	// Check the type
	if(m_Type != transType && transType != EMST_NO_TYPE)
	  return(FALSE);

	// Check the bits
	for(flagBit = 0x01, flagsChecked = flags; flagsChecked; flagBit << 1) 
	{
	    if(flagsChecked & flagBit && m_TLflags & flagBit) 
		{
		    flagsChecked &= ~flagBit; // Clear bits checked 
		} 
		else 
		{
		    return(FALSE);
	    }
	}
 	return(TRUE);
}




//================================================================
// CTransAPI 
//================================================================

//  Translator entry point look up function
//  This takes an OP Code ad defined in tlapi.h and returns
//  A pointer to a C function in the DLL that can be called.
//  If the DLL is not loaded for the particular translator
//  it is loaded (currently all are always loaded)
// =======================================================================
short (*CTransAPI::GetFunction(LPSTR tlOperation))(...)
{
	// The lookup function in Windows translators returns string
	// to lookup real function in the DLL
	return((short (*)(...))GetProcAddress(m_HInstance, tlOperation));
}

//================================================================
// ~CTransAPI 
//================================================================

CTransAPI::~CTransAPI()
{
	ems2_plugin_finish_t *transFinish = (ems2_plugin_finish_t *)GetFunction(EMS_PLUGIN_FINISH);

	if (m_Desc && *m_Desc && m_fnFreeFunc)	
		m_fnFreeFunc(m_Desc);

	if (m_HIcon && *m_HIcon && m_fnFreeFunc)	
		m_fnFreeFunc((void FAR*)m_HIcon);

	if (transFinish)
		transFinish(m_Globals);   

}
// =======================================================================
// GetIcon
// =======================================================================
BOOL CTransAPI::GetIcon(HICON &icn)
{
	if (m_HIcon && *m_HIcon)
	{
		icn = *m_HIcon; 
		return TRUE;
	}
	return FALSE;
}

// =======================================================================
// Settings
// =======================================================================
FARPROC CTransAPI::GetSettingsFunc()
{
	return (GetProcAddress(m_HInstance, EMS_PLUGIN_SETTINGS));
}

// =======================================================================
// Settings
// =======================================================================
BOOL CTransAPI::Settings()
{
	long ret = EMSR_OK;
	FARPROC pSettingsFun;

	DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_PLUGIN_SETTINGS);

	pSettingsFun = GetSettingsFunc();
	if (pSettingsFun)
	{
		CEMSMailConfig eMailC;

		HWND topWnd = GetActiveWindow();
		eMailC.SetHWnd(&topWnd);

		emsMailConfig EMCStr = (emsMailConfig)eMailC;
 		
		if (GetVersion() == PARAM_VERSION)
			ret = (*(ems2_plugin_config_t *)pSettingsFun)(m_Globals,
															(char *)EMCStr.configDir,
															(char *)EMCStr.userAddr.realname, 
															(char *)EMCStr.userAddr.address);
		else if(GetVersion() >= EMS_PB_VERSION)
		{
			ret = (*(ems3_plugin_config_t *)pSettingsFun)(m_Globals,
															&EMCStr); 
			if (ret == EMSR_OK)
			{
				CTranslator *tltr = NULL;
				int i = 1;
				while (1)
				{
					tltr = GetTransMan()->GetTranslator(m_ModuleID, i++);
					if (tltr)
						tltr->UpdateTranlator();
					else
						break;
				}
			}
		}

		if (ret == EMSR_OK)
			return TRUE;		
	}
	else
	{
		ErrorDialog(IDS_EMS_NO_SETTINGS, (const char*)m_Desc);
	}
	return FALSE;
}

// =======================================================================
// Idle from Plugin
// =======================================================================
FARPROC CTransAPI::GetIdleFunc()
{
	return (GetProcAddress(m_HInstance, EMS_IDLE));
}

// =======================================================================
// Idle
// =======================================================================
BOOL CTransAPI::IdleFunc(const long idleTime, long idleFlags)
{
	long ret = EMSR_OK;

	if (!m_LastTimeCalled)
		m_LastTimeCalled = GetTickCount();

	DWORD elapsedTime = GetTickCount() - m_LastTimeCalled;

	DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_IDLE);

	FARPROC pIdleFun = GetIdleFunc();
	if (pIdleFun)
	{
		if (m_IdleFreq <= elapsedTime || (idleFlags & EMIDLE_PRE_SEND))
		{
			if (! (idleFlags & EMIDLE_PRE_SEND) )
			{
				if (idleTime > GetIniShort(IDS_INI_ESMAPI_IDLE))
					idleFlags |= EMIDLE_UI_ALLOWED;
				else
					idleFlags |= EMIDLE_QUICK;
			}

			if (GetIniShort(IDS_INI_CONNECT_OFFLINE))
				idleFlags |= EMIDLE_OFFLINE;

		
			CEMSIdleData idleData;
			emsIdleData idleDataStrct = (emsIdleData)idleData;
			idleDataStrct.flags = idleFlags;
			idleDataStrct.idleTimeFreq = m_IdleFreq;
 			
			if( GetVersion() >= EMS_PB_VERSION)
			{
				ret = (*(ems_idle_t *)pIdleFun)(m_Globals, &idleDataStrct); 
				if (ret == EMSR_OK)
				{
					m_IdleFreq = idleDataStrct.idleTimeFreq;
				}
				m_LastTimeCalled = GetTickCount();
			}
		}

		if (ret == EMSR_OK)
			return TRUE;		
	}
	return FALSE;
}



// =======================================================================
// CTranslatorSortedList 
// =======================================================================
// Add make the list sorted
// =======================================================================
void CTranslatorSortedList::Add(CTranslator *trans)
{
	POSITION pos = GetHeadPosition();
	POSITION savePOS = pos;

	if (!pos)
	{
		AddHead(trans);
		return;
	}


	while (pos)
	{ 
		savePOS = pos;
		CTranslator *tr = GetNext(pos);
		if (trans->GetType() <= tr->GetType())
		{
			InsertBefore(savePOS, trans);
			return;
		}
	}

	if (!pos)
	{
		pos = GetTailPosition(); 
		InsertAfter(pos, trans);
		return;
	}

}

// =======================================================================
// Add make the list sorted
// =======================================================================
CTranslator *CTranslatorSortedList::FindMatch(const long type, const long moduleID /*= 0*/)
{
	POSITION pos = GetHeadPosition();

	if (!pos)
	{
		return NULL;
	}

	while (pos)
	{
		CTranslator *tr = GetNext(pos);
		if (tr->GetType() == type)
		{
			if (moduleID)
			{
				long ModID = 0, TrID = 0;
				tr->GetID(ModID, TrID); 
				if (ModID == moduleID)
					return tr;
			}
			else
				return tr;
		}
	}
	return NULL;

}

//================================================================
// CTranslatorManager 
//================================================================

BOOL CTranslatorManager::m_PluginIdle = FALSE;
WORD CTranslatorManager::m_HeaderFlags = 0;


//================================================================
// CTranslatorManager: this manages the dll's and all the translators
// =======================================================================
CTranslatorManager::CTranslatorManager(short numArgs, ...)
{
	va_list           vArgs;
	HANDLE            dirHandle;
	HINSTANCE         libHandle;
	WIN32_FIND_DATA   dllFileInfo;
    char             *path;
	char              dllPattern[_MAX_PATH +1];
	CString           dllPathStr;


	va_start(vArgs, numArgs);
	
	// Go through all the paths given to us
    while(numArgs--) 
	{
    	path = (char *)va_arg(vArgs, char *);

		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_CHECKING, path);
		TRACE1("Checking Path %s\n", path);
	    
		// Set up to search for DLL's in the path
		strcpy(dllPattern, path);
		if (dllPattern[strlen(dllPattern)-1] != SLASH)
			strcat(dllPattern, SLASHSTR);
		strcat(dllPattern, "*.dll");
	    
		dirHandle = FindFirstFile(dllPattern, &dllFileInfo);
		if(dirHandle == INVALID_HANDLE_VALUE)
			continue;
		
		// Go through all the DLL's in the particular path
		do 
		{
			// Make the path fully qualified
			dllPathStr = path;
			dllPathStr	+= SLASHSTR; 
			dllPathStr += dllFileInfo.cFileName;
			
			// Actually load the library
			libHandle = LoadLibrary(dllPathStr);

			if( !libHandle )
			{
				DWORD err = GetLastError();
				TRACE("LoadLibrary failed (%s): err = %lu (0x%04lX)\n", ((const char *)dllPathStr), err, err);

				continue;
			}

			TRACE("Library loaded (%s)\n", ((const char *)dllPathStr));

			DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOADING, dllPathStr);
			LoadModule(dllPathStr, libHandle);

		} while(FindNextFile(dirHandle, &dllFileInfo));
		
		FindClose(dirHandle);
	}

	va_end(vArgs);

	// Init cache varibles
	m_SortedList = NULL;
	m_SortedListContext = -1;

}

//================================================================
// CTranslatorManager: this manages the dll's and all the translators
// =======================================================================
CTranslatorManager::LoadModule(const char *path, HINSTANCE libHandle)
{
	void FAR*globals = NULL;
	
	HICON *icn = NULL;
	short numTrans = 0, numAttachers = 0, numSpecials = 0, numMBoxContextMenus = 0;
	short id = 0;
	char *desc = NULL;
	long idleFreq = 0;

	FARPROC	fnPluginInfoFun = NULL;
	FARPROC fnTransInfoFun = NULL;
	FARPROC fnAttachInfoFun = NULL;
	FARPROC fnSpecialInfoFun = NULL;
	FARPROC fnMboxCMInfoFun = NULL;
	

	// Get The Version Function
	short APIVersion = 0;
	ems_plugin_version_t  *fnVersion = (ems_plugin_version_t *)GetProcAddress(libHandle, EMS_PLUGIN_VERSION);
	
	if (!fnVersion)
	{
		// Missing ems_plugin_version
		TRACE("Plugin (%s): Missing function \"%s\" -- load failed.\n", (const char *)path, (const char *)EMS_PLUGIN_VERSION);
		goto fail;
	}
	else 
	{
		// Get The Version
		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_PLUGIN_VERSION);

		if ( ((*fnVersion)(&APIVersion) != EMSR_OK) )
		{  
			// This supports version 2 and version 3+ of the API
			if ( (!(APIVersion == 2)) || (APIVersion > EMS_VERSION) ) 
			{
				TRACE("Plugin (%s): \"%s\" unsuccessful (failed or unsupported version) -- load failed.\n", (const char *)path, (const char *)EMS_PLUGIN_VERSION);
				
				CString err;
				err.Format(CRString(IDS_EMS_BAD_VERSION), (const char *)CRString(IDS_VERSION),
					EMS_VERSION, path, APIVersion);
				PutDebugLog(DEBUG_MASK_TRANS_BASIC, err);
				ErrorDialog(IDS_TRANS_ERROR, (const char *)err, "", APIVersion);
				goto fail;
			}
		}
	}

	// Get the plugin init Function
	fnPluginInfoFun = GetProcAddress(libHandle, EMS_PLUGIN_INIT);
	if(!fnPluginInfoFun)
	{
		// Missing ems_plugin_init
		TRACE("Plugin (%s): Missing function \"%s\" -- load failed.\n", (const char *)path, (const char *)EMS_PLUGIN_INIT);
		goto fail;
	}

	DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_LOG_API, EMS_PLUGIN_INIT);
	if (APIVersion == PARAM_VERSION)
	{
		emsMailConfig emc = (emsMailConfig)m_MailConfig;
		if ( (*(ems2_plugin_init_t *)fnPluginInfoFun)(&globals, 
							emc.configDir, 
							emc.userAddr.realname, 
							emc.userAddr.address, 
							&numTrans, 
							&desc, 
							&id, 
							&icn) != EMSR_OK)
		{
			TRACE("Plugin (%s): \"%s\" failed -- load failed.\n", (const char *)path, (const char *)EMS_PLUGIN_INIT);
			goto fail;
		}
		
	}
	else
	{  
		emsMailConfig emc =  (emsMailConfig)m_MailConfig;
 		CEMSPluginInfo PluginInfo;
		if ( (*(ems3_plugin_init_t *)fnPluginInfoFun)(&globals, 
												EMS_VERSION,      
												&emc, 
												&PluginInfo.m_PluginInfo) != EMSR_OK)
		{
			TRACE("Plugin (%s): \"%s\" failed -- load failed.\n", (const char *)path, (const char *)EMS_PLUGIN_INIT);
			goto fail;
		}
		numTrans = (short) PluginInfo.m_PluginInfo.numTrans;
		desc = PluginInfo.m_PluginInfo.desc;
		id = (short) PluginInfo.m_PluginInfo.id;
		icn = PluginInfo.m_PluginInfo.icon;
		numAttachers = (short) PluginInfo.m_PluginInfo.numAttachers;
		numSpecials = (short) PluginInfo.m_PluginInfo.numSpecials;
		numMBoxContextMenus = (short) PluginInfo.m_PluginInfo.numMBoxContext;
		idleFreq = PluginInfo.m_PluginInfo.idleTimeFreq;
		
	}
	
	// It has to have a unique Module ID, if this was already loaded .. get outa here!
	if ( (numTrans && GetTranslator(id, 1)) ||
		 (numAttachers && GetAttacher(id, 1)) ||
		 (numSpecials && GetSpecial(id, 1)) ||
		 (numMBoxContextMenus && GetMboxCM(id, 1)) ) 
	{
		TRACE("Plugin (%s): Non-unique ID (%i) -- load failed.\n", (const char *)path, (int) id);

		DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_DUPLICAT_DLL, id);
		goto fail;
	}

	// We got a plugin, create the container for it
	{
		// The API contains the DLL details, its stored in a list
		CTransAPI *ptrapi = new CTransAPI(path, libHandle, id, APIVersion, desc, icn, globals, idleFreq);
		m_TranAPIs.InsertAt(m_TranAPIs.GetSize(), ptrapi);
		
		if (idleFreq)
			m_PluginIdle = TRUE;
		
		// Set up the free function 
		ptrapi->m_fnFreeFunc = (ems2_free_t *)GetProcAddress(libHandle,EMS_FREE);
		if (!ptrapi->m_fnFreeFunc)
			DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_ERR_NO_FUNCTION, EMS_FREE);
				
		
		fnTransInfoFun = GetProcAddress(libHandle, EMS_TRANS_INFO);
		// Create the translators
		for( short trIndex = 1; fnTransInfoFun && (numTrans > 0 && trIndex <= numTrans); trIndex++) 
		{
			// Get info on each translator and create a translator object for it
			int TransNumInAPI = m_Translators.GetSize();
			// Construct the new Translator and put it in our list 
			m_Translators.InsertAt(TransNumInAPI,
				  new CTranslator(ptrapi, trIndex));
		}
		
		// Create the attachment menus
		fnAttachInfoFun = GetProcAddress(libHandle, EMS_ATTACH_INFO);
		for( trIndex = 1; fnAttachInfoFun &&  (numAttachers > 0 && trIndex <= numAttachers); trIndex++) 
		{
			// Get info on each translator and create a translator object for it
			int AttachNumInAPI = m_AttachItems.GetSize();
			// Construct the new Attacher and put it in our list 
			m_AttachItems.InsertAt(AttachNumInAPI,
				  new CAttacher(ptrapi, trIndex));
		}
		
		// Create the special menus
		fnSpecialInfoFun = GetProcAddress(libHandle, EMS_SPECIAL_INFO);
		for( trIndex = 1; fnSpecialInfoFun && (numSpecials > 0 && trIndex <= numSpecials); trIndex++) 
		{
			// Get info on each translator and create a translator object for it
			int SpecialInAPI = m_SpecialItems.GetSize();
			// Construct the new Attacher and put it in our list 
			m_SpecialItems.InsertAt(SpecialInAPI,
				  new CSpecial(ptrapi, trIndex));
		}

		// Create the special menus
		fnMboxCMInfoFun = GetProcAddress(libHandle, EMS_MBOX_CONEXT_INFO);
		for( trIndex = 1; fnMboxCMInfoFun && (numMBoxContextMenus > 0 && trIndex <= numMBoxContextMenus); trIndex++) 
		{
			// Get info on each translator and create a translator object for it
			int CMInAPI = m_MboxCMItems.GetSize();
			// Construct the new Attacher and put it in our list 
			m_MboxCMItems.InsertAt(CMInAPI,
				  new CMBoxContext(ptrapi, trIndex));
		}
		
	}

	TRACE("Plugin (%s): Successfully loaded [ID=%i, VER=%i, DESC=\"%s\"].\n", (const char *)path, (int)id, (int)APIVersion, desc);



	return TRUE;

fail:
	DebugLog(DEBUG_MASK_TRANS_BASIC, IDS_EMS_FREE, path);
	FreeLibrary(libHandle);
	return FALSE;
}

//================================================================
// ~CTranslatorManager
//================================================================
CTranslatorManager::~CTranslatorManager()
{
	int i = m_Translators.GetSize() - 1;
	for (; i >= 0; i--)
	{
		CTranslator *t  = m_Translators.GetAt(i);
		m_Translators.RemoveAt(i);
		delete t;
	}
	i = m_AttachItems.GetSize() - 1;
	for (; i >= 0; i--)
	{
		CAttacher *am  = m_AttachItems.GetAt(i);
		m_AttachItems.RemoveAt(i);
		delete am;
	}
	
	i = m_SpecialItems.GetSize() - 1;
	for (; i >= 0; i--)
	{
		CAttacher *am  = m_SpecialItems.GetAt(i);
		m_SpecialItems.RemoveAt(i);
		delete am;
	}

	i = m_MboxCMItems.GetSize() - 1;
	for (; i >= 0; i--)
	{
		CMBoxContext *pConMenu  = m_MboxCMItems.GetAt(i);
		m_MboxCMItems.RemoveAt(i);
		delete pConMenu;
	}


	i = m_TranAPIs.GetSize() - 1;
	for (; i >= 0; i--)
	{
		CTransAPI *t = m_TranAPIs.GetAt(i);
		m_TranAPIs.RemoveAt(i);
		HINSTANCE hi = t->GetInstance();
		delete t;
		FreeLibrary(hi);
	}

	delete m_SortedList;
}

//================================================================
// GetTranslator
//================================================================
CTransAPI* 
CTranslatorManager::GetModule(const long moduleID)
{
	int i = m_TranAPIs.GetSize() - 1;
	for (; i >= 0; i--)
	{
		CTransAPI *t = m_TranAPIs.GetAt(i);
		if (t && (t->GetID() == moduleID))
			return t;
	}
	return NULL;
}

//================================================================
// GetTranslator
//================================================================
CTranslator* 
CTranslatorManager::GetTranslator(long moduleID, long TranslatorID)
{
	CTranslator	*tlater = NULL;
	long ModID = 0, TransID = 0;
	
	for (int i = 0; i < m_Translators.GetSize(); i++)
	{
		tlater = m_Translators.GetAt(i);
		tlater->GetID(ModID, TransID);
		if (ModID == moduleID && TransID == TranslatorID)
			return tlater;
	}

	return NULL;
}
//================================================================
// GetAttacher
//================================================================
CAttacher* 
CTranslatorManager::GetAttacher(long moduleID, long AttacherID)
{
	CAttacher	*pAMI = NULL;
	long ModID = 0, AttachID = 0;
	
	for (int i = 0; i < m_AttachItems.GetSize(); i++)
	{
		pAMI = m_AttachItems.GetAt(i);
		if (pAMI)
		{
			pAMI->GetID(ModID, AttachID);
			if (ModID == moduleID && AttachID == AttacherID)
				return pAMI;
		}
	}

	return NULL;
}

//================================================================
// GetSpecial
//================================================================
CSpecial* 
CTranslatorManager::GetSpecial(long moduleID, long AttacherID)
{
	CSpecial	*pAMI = NULL;
	long ModID = 0, AttachID = 0;
	
	for (int i = 0; i < m_SpecialItems.GetSize(); i++)
	{
		pAMI = m_SpecialItems.GetAt(i);
		if (pAMI)
		{
			pAMI->GetID(ModID, AttachID);
			if (ModID == moduleID && AttachID == AttacherID)
				return pAMI;
		}
	}

	return NULL;
}

//================================================================
// GetMboxCM
//================================================================
CMBoxContext* 
CTranslatorManager::GetMboxCM(long moduleID, long mboxCMID)
{
	CMBoxContext	*pAMI = NULL;
	long ModID = 0, cmID = 0;
	
	for (int i = 0; i < m_MboxCMItems.GetSize(); i++)
	{
		pAMI = m_MboxCMItems.GetAt(i);
		if (pAMI)
		{
			pAMI->GetID(ModID, cmID);
			if (ModID == moduleID && cmID == mboxCMID)
				return pAMI;
		}
	}

	return NULL;
}

//================================================================
// FindFirstTranslator ... return index to first translator matching criteria
//================================================================
short 
CTranslatorManager::FindFirstTranslator(const long type /*= 0*/, const long context /*= 0*/, const char *desc, short startAt /* =0*/)
{
	// Empty List!
	if (!m_Translators.GetSize())
		return -1;
	
	// No checks required, return first index (remember arrays start at 0)
	if (!type && !context &&!desc)
		return (startAt);
	else
	{
		BOOL MATCH = TRUE;
		CString curDesc;
		// Walk through list meeting condition
		for (short i = startAt; i < m_Translators.GetSize(); i++)
		{
			CTranslator *t = m_Translators.GetAt(i);
			if (type > 0 && !t->matchType(type))
				MATCH = FALSE;
			if (context && !t->matchContext(context))
				MATCH = FALSE;
			if (desc && t->getDescription(curDesc) && curDesc != desc)
				MATCH = FALSE;

			if (MATCH)
				return i;

			// Reset for next round
			MATCH = TRUE;
		}
	}

	return -1;
}

//================================================================
//GetNextTranslator
//================================================================
CTranslator* 
CTranslatorManager::GetNextTranslator(short* index, const long type /*= 0*/, const long context /*= 0*/, const char *desc)
{
	CTranslator	*tlater = NULL;
	
	// Out of range!
	if (*index < 0 || *index >= m_Translators.GetSize())
		return NULL;
	
	// No checks required, return next index
	if (!type && !context)
		tlater = m_Translators.GetAt(*index);
	else
	{
		// Walk through list meeting condition
		short i = FindFirstTranslator(type, context, desc, *index);
		if (i >= 0) 
			tlater = m_Translators.GetAt(i);
		if (tlater) 
			*index = i;
		else
			*index = -2;
	}

	(*index)++;
	return tlater;
}

//================================================================
// This will sort list according to type
//================================================================
CTranslatorSortedList *
CTranslatorManager::GetSortedTranslators(const long context /*= 0*/)
{
	// If list is already loaded, return it
	if (m_SortedListContext == context) 
		return m_SortedList;

	else
	{
		delete m_SortedList;
		m_SortedList = new CTranslatorSortedList();
	
		short index = FindFirstTranslator(0, context);

		while (index >= 0)
		{
			CTranslator *tr = GetNextTranslator(&index,  0, context);
			if (tr)
				m_SortedList->Add(tr);
			else 
				break;
		}
		m_SortedListContext = context;
	}
	return m_SortedList;
}

//================================================================
// This will sort list according to type ... given a selection
//================================================================
CTranslatorSortedList *
CTranslatorManager::GetSortedTranslators(const char* sel, const long context /*= 0*/)
{       
    // Just reset, so next time wont use this cache
    m_SortedListContext = -1; 
        
    // Clean out old list
    delete m_SortedList;
    m_SortedList = new CTranslatorSortedList();

    long ModuleID = 0, TransID = 0;
    char * dot = NULL;
    char * comma = NULL;
    char * space = NULL;

    bool bTranslatorNotFound = false;

        // The format of the translor line is modID.transID modID2.transID ...
    while (sel)
    {
        // get rid of the first char <xxxxx.yyyyy>
        if (sel[0] == '<')
            sel += 1;

        ModuleID = atoi(sel);
        dot = strchr(sel,'.');

        if (dot)
        {
            sel = dot+1;
            TransID = atoi(sel);
        }
        else
            break;


        comma = strchr(sel,',');
        space = strchr(sel,' ');
        CTranslator *tltr = GetTranslator(ModuleID, TransID);

        if ( tltr ) {
            tltr->m_Properties.Empty();

            if (comma && space)
            {
                space++;
                while (space < comma)
                    tltr->m_Properties += *space++;
            }
            else if (space)
                tltr->m_Properties = ++space;

            if ( tltr->GetFlags() & context )
                m_SortedList->Add(tltr);
        }
	else {
	    bTranslatorNotFound = true;
	}

        if (comma)
            sel = comma+1;
        else 
            break;
    }

    if ( bTranslatorNotFound )
	ErrorDialog( IDS_EMSAPI_ERROR, CRString(IDS_TLR_ERROR_CANT_TRANS), CRString(IDS_TLR_ERROR_INVALID_TRANS) );


    return m_SortedList;
}

// BOG

// ValidateTransIDs:
// Validates a list of trans ids, as would be found in either the headers or the
// body of a message.
//
// Issues:
//    Right now, this routine does not generate a string w/enclosing brackets.
//    This is not currently a problem, because it is only used called by
//    CSummary::SetTranslators, which inits a summary field that doesn't
//    expect them. Something to be aware of!

int CTranslatorManager::ValidateTransIDs
(
    const char* sel,                // string of trans ids to validated
    const long  context /*= 0*/,    // EMS_Q4_TRANSMISSION, etc.
    CString&    strValidIDs         // resulting string of validated trans ids
)
{
    strValidIDs = "";

    long ModuleID = 0, TransID = 0;
    char * dot = NULL;
    char * comma = NULL;
    char * space = NULL;

    bool bTranslatorNotFound = false;

    if ( sel[0] == '<' )
        sel += 1;
    
    while ( sel ) {
        ModuleID = atoi( sel );
        dot = strchr( sel,'.' );

        if ( dot ) {
            sel = dot + 1;
            TransID = atoi( sel );
        }
        else
            break;

        comma = strchr( sel, ',' );
        space = strchr( sel, ' ' );
        CTranslator *tltr = GetTranslator( ModuleID, TransID );

        if ( tltr && (tltr->GetFlags() & context) ) {
            if ( !strValidIDs.IsEmpty() )
                strValidIDs += ",";

            char buf[16] = "";
            sprintf( buf, "%04ld.%04ld", ModuleID, TransID );
            strValidIDs += buf;

            if ( comma && space ) {
                space++;

                while ( space < comma )
                    strValidIDs += *space++;
            }
            else if ( space )
                strValidIDs += ++space;
        }
        else
	    bTranslatorNotFound = true;

        if ( comma )
            sel = comma + 1;
        else 
            break;
    }

    if ( bTranslatorNotFound )
	ErrorDialog( IDS_EMSAPI_ERROR, CRString(IDS_TLR_ERROR_CANT_TRANS), CRString(IDS_TLR_ERROR_INVALID_TRANS) );


    return strValidIDs.GetLength();
}

// end BOG


// =======================================================================
// IdleEveryone
// =======================================================================
BOOL CTranslatorManager::IdleEveryone(const long idleTime, const long idleFlags)
{
	// Don't bother going through the list if there's no idling plugin
	if (!m_PluginIdle)
		return TRUE;
	
	// elapsedTime is in seconds ...
	for (int i = 0; i < m_TranAPIs.GetSize(); i++)
	{
		CTransAPI *t = m_TranAPIs.GetAt(i);
		if (t->GetIdleFunc())
			t->IdleFunc(idleTime, idleFlags);
	}
	return TRUE;
}


// =======================================================================
// MboxContextMenu
// =======================================================================
BOOL CTranslatorManager::MboxContextMenu(CTocDoc *tocdoc, CRect &rect)
{
	CMenu	mboxMenu;
	mboxMenu.CreatePopupMenu( );

	for (int i = 0; i < m_MboxCMItems.GetSize(); i++)
	{
		CMBoxContext *mbc = m_MboxCMItems.GetAt(i);
		if (mbc)
		{
			long modID = 0, cmID = 0;
			mbc->GetID(modID, cmID);
			if (modID == (long)tocdoc->GetPeanutID())
			{
				CString desc =  ((emsMenu)*mbc).desc;
				mboxMenu.AppendMenu( MF_STRING | MF_ENABLED, i + QC_FIRST_COMMAND_ID, desc );
			}
		}
	}

	UINT cmdID = mboxMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, rect.left, rect.bottom, AfxGetMainWnd());
	if (cmdID)
	{
		CMBoxContext *mbc = m_MboxCMItems.GetAt(cmdID - QC_FIRST_COMMAND_ID);
		mbc->ContextHook(tocdoc);
	}

	return TRUE;
	
}


//================================================================
// CTranslatorManager:This is a function that see's if there's sign & encrypt seleceted == coalese
//================================================================
void CTranslatorManager::RemoveDupFuncs(CTranslatorSortedList *pSortedList)
{

	POSITION pos = pSortedList->GetHeadPosition();
	while(pos)
	{
		CTranslator *sigTr = pSortedList->GetNext(pos);
		if (sigTr->GetType() == EMST_SIGNATURE)
		{
			long ModID = 0, TrID = 0;
			sigTr->GetID(ModID, TrID); 
			
			// Find matching preprocessor
			CTranslator *prepTr = pSortedList->FindMatch(EMST_PREPROCESS,ModID);
			if (prepTr)
			{
				// now see if there's a coalessor
				short index = FindFirstTranslator(EMST_COALESCED);
				while (index >= 0)
				{
					CTranslator *coalTr = GetNextTranslator(&index, EMST_COALESCED);
					if (coalTr)
					{
						long coalModID = 0, coalTrID = 0;
						coalTr->GetID(coalModID, coalTrID); 
						if (coalModID == ModID)
						{
							coalTr->m_Properties = sigTr->m_Properties + ',' + prepTr->m_Properties;
							pSortedList->Add(coalTr);
							pSortedList->RemoveAt(pSortedList->Find( sigTr ));
							pSortedList->RemoveAt(pSortedList->Find( prepTr ));
							pos = pSortedList->GetHeadPosition();
						break; // get out of while index
						}
					}// coalTr
				}// while index
			}//prepTr
		
		}// EMST_SIGNATURE
	
	}// while pos


}

//================================================================
// GetDefOutTransString: Grab the id's of the outgoing default translators
//================================================================
BOOL CTranslatorManager::GetDefOutTransString(CString& transString)
{
	CTranslatorSortedList *trLst = GetSortedTranslators(EMSF_Q4_COMPLETION | EMSF_Q4_TRANSMISSION );

	if(!trLst)
		return (FALSE);

	POSITION		pos = trLst->GetHeadPosition();
	CTranslator*	tltr = NULL;
	long			ModID = 0, TrID = 0;
	CString			idStr;


	while(pos)
	{
		tltr = trLst->GetNext(pos);
		if (tltr)
		{
			if (tltr->GetFlags() & EMSF_DEFAULT_Q_ON)
			{
				tltr->GetID(ModID, TrID);
				idStr.Format("%04ld.%04ld",ModID, TrID);
				if (!transString.IsEmpty())
					transString += ',';
				transString += idStr;
			}
		}
	}

	return TRUE;
}


//================================================================
// XLateMessageOut: Given a message, grab list of translators, then translate
//================================================================
int CTranslatorManager::XLateMessageOut(const long context, CCompMessageDoc* msg, const char *raw, char *cooked)
{
	CTranslatorSortedList *trLst = GetSortedTranslators(msg->m_Sum->GetTranslators(), 
												EMSF_Q4_TRANSMISSION | EMSF_Q4_COMPLETION);
	CTranslator *tltr = NULL;
	CString srcFile = raw;
	long err = EMSR_OK;
	long retCode = 0;

	CTLMime tMime ;
	CTLAddress tAddress;

	tMime.CreateMimeType(raw);				// Get MIME Stuff
	tAddress.CreateAddressList(msg);		// Build Address Info

	// This will consolidate functions
	ASSERT(trLst);
	if (!trLst)
		return EMSR_INVALID;
	
	RemoveDupFuncs(trLst);

	POSITION pos = trLst->GetHeadPosition();
	ASSERT(pos);
	if (!pos)
		return EMSR_INVALID;

	while(pos && (err == EMSR_OK || err == EMSR_NOW))
	{
		// we've been through here, delete old src, move cooked into it
		if (cooked && *cooked)
		{
			DeleteFile(srcFile);
			srcFile = cooked;
		}
		strcpy(cooked,::GetTmpFileNameMT("mim"));	

		tltr = trLst->GetNext(pos);
		ASSERT(tltr);
		if (tltr)
			err = tltr->Translate(&tMime, context, &tAddress, srcFile, cooked, &retCode);
	}
	DeleteFile(srcFile);
	
	return err;
}

//================================================================
// CanXLateMessageOut: Check to see if all outgoing translators are OK
//================================================================
int CTranslatorManager::CanXLateMessageOut(CCompMessageDoc* msg, UINT uType)
{
	CTLMime tMime ;
	CTLAddress tAddress;	 

	long ret = EMSR_NOW;
	CTranslatorSortedList *trLst = GetSortedTranslators(msg->m_Sum->GetTranslators(), uType);
	CTranslator *tltr = NULL;

	// Get the message info
	tMime.CreateMimeType(msg->m_Sum->IsHTML(), msg->m_Sum->HasAttachment());	// Get MIME Stuff
	tAddress.CreateAddressList(msg);					// Build Address Info

	POSITION pos = trLst->GetHeadPosition();
	if (!pos)
		return FALSE;

	while(pos && ret == EMSR_NOW)
	{
		tltr = trLst->GetNext(pos);

		// Translate File
		ret = tltr->CanTranslate(uType, &tMime, &tAddress);

		if (ret != EMSR_NOW)
			tltr->TranslatorError(EMSR_CANT_TRANS, (const char *)CRString(IDS_EMS_ERR_CANT_QUEUE),0);
	}

	return ret == EMSR_NOW ? TRUE : FALSE;
}

//================================================================
// CanXLateMessage: Given a message, grab list of translators, then check
//  Output:
//		retTran will return a pointer to the translator
//
//  returns:
//			-1 : No Trnslators
//			EMSR_CANT_TRANS 
//			EMSR_NOW
//			EMSR__NOT_NOW
//			
//================================================================
long CTranslatorManager::CanXLateMessage(CTLMime *tlmime, long context, CTranslator **retTrans, CTLAddress *address /* = NULL */)
{
	POSITION pos;
	BOOL FORWARD = TRUE;
	long ret = -1;
	CTranslatorSortedList *trLst = GetSortedTranslators(context);

	if(!trLst)
		return (-1);

	CTranslator *tltr = NULL;


	switch (context)
	{
		//  decoder type translation get reverse order
		case EMSF_ON_ARRIVAL:
		case EMSF_ON_DISPLAY:
			pos = trLst->GetTailPosition();
			FORWARD = FALSE;
			break;

		//  encoder type translation get forward order
		case EMSF_ON_REQUEST:
		case EMSF_Q4_COMPLETION:
		case EMSF_Q4_TRANSMISSION:
		default:
			pos = trLst->GetHeadPosition();
			break;
	}

	while(pos)
	{
		if (FORWARD)
			tltr = trLst->GetNext(pos);
		else
			tltr = trLst->GetPrev(pos);


		// Now check to see if we 'can'
		ret = tltr->CanTranslate(context,tlmime, address);
		if (ret == EMSR_NOW || ret == EMSR_NOT_NOW)
		{
			if (retTrans);
				*retTrans = tltr;
			break;
		}

	}

	return ret;
}

//================================================================
// Translate display message:
//
//================================================================
long CTranslatorManager::XLateDisplay( QCProtocol* pXLateProtocol, const char *inFileCmd)
{
	CTLAddress addresses;
	CString rawHeaders = "";

	// BOG: this gets us the message headers, and a whole lot more; right now
	// the protocol doesn't support just getting the headers. the problem here
	// is that we get a *copy* of the message and it could be quite large.
	pXLateProtocol->GetMessageAsText( rawHeaders, TRUE );

	BOOL bBasic = GetTransMan()->GetHeaderFlag() & EMSAPI_DISPLAY_BASIC;
	BOOL bRaw = GetTransMan()->GetHeaderFlag() & EMSAPI_DISPLAY_RAW;
	
	char *to = NULL, *from = NULL, *cc = NULL, *subject = NULL, *bcc = NULL;
	// Do we need any headers?
	if (bBasic)
	{
		to = HeaderContents(IDS_HEADER_TO, rawHeaders);	
		from = HeaderContents(IDS_HEADER_FROM, rawHeaders);
		cc = HeaderContents(IDS_HEADER_CC, rawHeaders);	
		subject = HeaderContents(IDS_HEADER_SUBJECT, rawHeaders);
	}
	
	
	addresses.CreateAddressList(to, 
								from, 
								subject, 
								cc, 
								bcc,
								bRaw ? (const char*)rawHeaders : NULL);
	delete [] to;
	delete [] from;
	delete [] cc;
	delete [] subject;


	// Parse out filename the translator  ( drv:\path\filename.ems <xxx.xxx> )
	char *tltrID = NULL;
	char *space = strstr(inFileCmd,".ems ");
	if (space && (space += 4))
	{
		*space = 0;
		tltrID = space+1;

		if (*(space - 1) == '\"')
			*(space -1) = 0;
		if (*inFileCmd == '\"')
			inFileCmd++;
	}
	
	CString out = ::GetTmpFileNameMT();	
	
	// This will contain the outputs of all the translators
	g_OutputDesc.Empty();
	
	// Run it through tranlation, read it up, then display
	long retCode = 0;
	long transReturn = InterpretFile(EMSF_ON_DISPLAY, inFileCmd, out, &addresses, &retCode, tltrID);

	if (InProgress)
		CloseProgress();

	// BOG: I've hammered this around a little from what was in 3.x. It appeared
	// we didn't need to allocate memory every time through the loop, so I moved
	// that outside. I removed the win32/16bit specific stuff, although I have
	// left the MaxLength value alone.
	//
	// For the QCProtocol changes, I've removed the SetSel/GetSel stuff; I
	// figure that caret position should be set by the caller before they
	// call this routine. I wonder if Christy won't have to special case the
	// "\r\n" stuff tho; insert a "br" tag or something? Paige will just run
	// it through the importer like everything else, which is a pretty
	// expensive way to get a line break; better to just "insert" the thing
	// directly. Maybe we need another way of applying a line/par break?

	const unsigned int MaxLength = 32 * 1024L;
	HRESULT hrRead;
	BOOL firstTime = TRUE;
	
	JJFile cookedFile;
	CString finalText;
	bool bFileReady = !(FAILED(cookedFile.Open(out,O_RDONLY)));
	char* cookedText = new char[MaxLength + 1];

	if ( transReturn == EMSR_OK && bFileReady && cookedText ) 
	{
		while (1) 
		{
			memset(cookedText,0,MaxLength);
			hrRead = cookedFile.Read(cookedText, MaxLength);

			if ( !FAILED(hrRead) )
			{
				if (!firstTime)
					finalText += cookedText;
				else
				{
					CRString MimeVersion(IDS_MIME_HEADER_VERSION);
					char *nicerText = cookedText;

					if (!strnicmp(nicerText, MimeVersion, MimeVersion.GetLength()))
					{
						// Get past MIME headers.
						// A blank line signifies the end of the headers.
						char *eol;
						while (eol = strstr(nicerText, "\r\n"))
						{
							nicerText = eol + 2;
							if (!strncmp(nicerText, "\r\n", 2))
							{
								nicerText += 2;
								break;
							}
						}
					}

					if (!g_OutputDesc.IsEmpty())
					{
						if (g_OutputDesc[0] != '\r')
							finalText += "\r\n";
						finalText += g_OutputDesc;
						finalText += "\r\n";
						g_OutputDesc.Empty();
					}
					else if (nicerText[0] != '\r')
						finalText += "\r\n";

					finalText += nicerText;
					firstTime = FALSE;
				}
				
				if (S_FALSE == hrRead)
					break;
			}

		}

		char *in = new char[finalText.GetLength()+1];
		strcpy(in, finalText);
		MorphMHTML(&in);
		GetBodyAsHTML(finalText, in);
		delete [] in;
		
		pXLateProtocol->PasteOnDisplayHTML( finalText, EMSC_SIGOK == retCode );

		delete [] cookedText;

		cookedFile.Close();
		DeleteFile(out); 
		return 0;
	}

	delete [] cookedText;

	cookedFile.Close();
	DeleteFile(out); 
	return -1;
}



//================================================================
// Translate incoming message:
//
//================================================================
long CTranslatorManager::InterpretFile(const long context, 
									   const char *inFile, 
									   const char *outFileName, 
									   CTLAddress *address,
									   long *retCode,
									   const char*firstTrans /*= NULL*/)
{
	CTranslator *trltor = NULL;
	CTLMime mimeType;
	long err = EMSR_CANT_TRANS;
	
	if (firstTrans)
	{
		long moduleID, transID;
	    sscanf( firstTrans, "<%04ld.%04ld>", &moduleID, &transID );
		trltor = GetTranslator(moduleID,transID);
		if (!trltor)
		{
			// Throw up an error message
			ErrorDialog(IDS_TRANS_ERROR, firstTrans, (const char *)CRString(IDS_EMS_NO_PLUGIN),  -1);
			return -1;
		}
	}

	if (mimeType.CreateMimeType(inFile))
	{
		// This is the first translator ... since it requested it now must translate it!
		if (trltor)
		{
			err = trltor->CanTranslate(context, &mimeType, address);
			if (err != EMSR_NOW)
			{
				// Throw up an error message
				CString desc;
				trltor->getDescription(desc);
				ErrorDialog(IDS_TRANS_ERROR,  (const char *)desc, (const char *)CRString(IDS_TLR_ERROR_CANT_TRANS),  err);
				return -1;
			}
		}
		else
			err = CanXLateMessage(&mimeType, context, &trltor, address);
	}

	if (err == EMSR_CANT_TRANS || err == EMSR_NOT_NOW)
	{
		// Parse Message from src to dst
		DeMimeMsg(inFile, outFileName);
		err = EMSR_OK;
	}
	else if (trltor && err == EMSR_NOW)
	{
		CString trOut = ::GetTmpFileNameMT();	
		err = trltor->Translate(&mimeType, context, address, inFile, trOut, retCode);
        
		if (err == EMSR_DATA_UNCHANGED)
		{
		// Copy the infile to the trout. changing the mime lines so the data is different.
			// We don't want it processed again by this plug in, I think this is wacky.
			// We either have to change this or I'm going to start hurting people.
			// We could have said "Demime this messsage and be done" but that's not fair
			// some other plug in might want the data, we have to offer it. WHY we offer it
			// to the same plug ins again is BEYOND me. If you wanted it you should have
			// used it!!! I'm getting very testy.
			// Then I guess we set EMSR OK.

			JJFile *INFILE;
			JJFile *OUTFILE;
			char *LineBuffer;
			long sizeofbuf;
			CRString MimeVersion(IDS_MIME_HEADER_VERSION);
			CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
			CRString ContentDisp(IDS_MIME_HEADER_CONTENT_DISP);
			int i;

			LineBuffer = new char[2048];
			INFILE = new JJFile;
			OUTFILE = new JJFile;

			INFILE->Open(inFile, O_RDONLY);
			OUTFILE->Open(trOut, O_WRONLY);


			for (sizeofbuf=1, i=0; sizeofbuf; i++)
			{

				INFILE->GetLine(LineBuffer, 2048, &sizeofbuf);

				char *tempPtr;
				if (sizeofbuf < 3)
				{
					OUTFILE->PutLine();
				}
				else
				{
					tempPtr = (LineBuffer + sizeofbuf-2);
					*tempPtr = 0;

					if (i < 3)
					{
					
						if (!strncmp(LineBuffer,(const char *)MimeVersion, MimeVersion.GetLength()))
						{
							// Do nothing to it, just output it.
							OUTFILE->PutLine(LineBuffer, strlen(LineBuffer));
						}	

						else if (!strncmp(LineBuffer,(const char *)ContentType, ContentType.GetLength()))
						{
							char *follower, *leader;

							follower = LineBuffer;

							while ((*follower != '/') && (*follower != '\r'))
							{
								follower++;
							}
					
							if (*follower == '/')	// Cool, we're at the seperator. Replace  / with !/! to munge header
							{
								char *tempstorage;
								tempstorage = new char[strlen(LineBuffer)];
							
								follower++;

								strcpy (tempstorage, (follower));

								leader = tempstorage;
								strcpy(follower, _T("Processed-"));

								follower += 10;
								while (*leader != 0)
								{
									*follower++ = *leader++;
								}
								*follower = 0;
	
								OUTFILE->PutLine(LineBuffer, strlen(LineBuffer));

								delete [] tempstorage;
							}
							else	// no / character? hrm...
							{
							OUTFILE->PutLine(LineBuffer, strlen(LineBuffer));
							}

						}

						else if (!strncmp(LineBuffer,(const char *)ContentDisp, ContentType.GetLength()))
						{
							// Do nothing to it, just output it.
							OUTFILE->PutLine(LineBuffer, strlen(LineBuffer));
						}
				
						else
						{	// Not one of the evil headers.
							OUTFILE->PutLine(LineBuffer, strlen(LineBuffer));
						}
					}
					else
					{ // Just write it
					OUTFILE->PutLine(LineBuffer, strlen(LineBuffer));
					}
				}
			}

			OUTFILE->Flush();
			OUTFILE->Close();
			INFILE->Close();

		delete INFILE;
		delete OUTFILE;
		delete [] LineBuffer;
		err = EMSR_OK;

		}


		
        if (!err)
        {
			// Call Interpret Recursively in case we need to translate result
			err = InterpretFile(context, trOut, outFileName, address, retCode);
			DeleteFile(trOut);   
		}
		else 
		{
			DeleteFile(trOut);   
			return -1;
		}
	}
	// There are no translators in this context
	else 
		return -1;

	return err;
}
//================================================================
// Translate incoming message:
//
//================================================================
long CTranslatorManager::DeMimeMsg(const char *inFileName, const char *outFileName)
{
	extern JJFile *g_pPOPInboxFile;
	extern long g_lEstimatedMsgSize;

	long ret = 0;
	BoundaryType endType;
	CObArray MimeStates;
	char buf[2048];

	FileLineReader lineReader;
	JJFile in, out;
	
	// read from file
	if (FAILED(in.Open(inFileName, O_RDWR)))
		return -1;
	long lSize = 0;
	HRESULT hrSeek = in.Seek(0, SEEK_END, &lSize);
	if (FAILED(hrSeek))
		return -1;
	ASSERT(lSize >= 0);
	g_lEstimatedMsgSize = lSize;
	in.Reset();
	lineReader.SetFile(&in);
	
	// g_pPOPInboxFile is a global (defined in pop.cpp) that is used by mime.cpp, header.cpp code
	JJFile * oldPopMBox = g_pPOPInboxFile;
	if (FAILED(out.Open(outFileName, O_RDWR | O_APPEND | O_CREAT)))
		return -1;
	g_pPOPInboxFile = &out;

	// Create A hd & Load it up, line reader must be set up, output g_pPOPInboxFile must be set up
	HeaderDesc hd(lSize);
	hd.Read(&lineReader);

	// We really know this is mime, so set it to true
	hd.isMIME = TRUE;

	// Create a file base line reader
	MIMEState ms(&lineReader);

	if (!ms.Init(&hd) || MimeStates.Add(&ms) < 0) return -1;
	
	endType = ms.m_Reader->ReadIt(MimeStates, buf, sizeof(buf));
	MimeStates.RemoveAt(MimeStates.GetUpperBound());
	if (endType == btError)
		ret = -1;

	in.Close();
	out.Flush();
	out.Close();
	
	// Restore global
	g_pPOPInboxFile = oldPopMBox;
	return ret;
}

//================================================================
// TranslateLiveOut: this is for outgoing translations
//
// Hey, we're on the wire in sendmail, thus no reference to the text,
// it's all in the globals (ouch)
//================================================================
int TranslateLiveOut(CCompMessageDoc* msg)
{
	long err = 0;
	char cooked[_MAX_PATH];
	*cooked = 0;
	CString raw = ::GetTmpFileNameMT();	
	err = WriteLiveMessage(raw);
	if (!err)
		err = GetTransMan()->XLateMessageOut(EMSF_Q4_TRANSMISSION | 
												EMSF_WHOLE_MESSAGE | 
												EMSF_REQUIRES_MIME | 
												EMSF_GENERATES_MIME, 
												msg, 
												raw, 
												cooked);
	
	
	if (!err && cooked)
	{
		XmitMessage(FALSE);
		SendRawMIME(cooked);
		FinishSMTP();
	}
	else
	{ 
		EndSMTP(TRUE);
		msg->m_Sum->SetState(MS_UNSENDABLE);
	}
	
	DeleteFile(raw); 
	DeleteFile(cooked); 

	// Sendmail needs negative number to abort
	return err ? -1 : 0;
}


//================================================================
// This relies on sendmail Netconnection global.  The connection started up,
// It swithes to the output to a file, then swithes back (YIKES)
//================================================================
int WriteLiveMessage(const char* rawName)
{
	// Save the old global
	Network *oldNetConnection = NetConnection;
	
	// This will set NetConnetion to a file I/O in the constructor
	::NetConnection = NULL;
	::NetConnection = ::CreateNetConnection(FALSE, TRUE);
//	ASSERT_KINDOF(CFileIO, ::NetConnection);
//FORNOW	CFileIO *fileIO = new CFileIO();
	int err = 0;

	((CFileIO *) ::NetConnection)->OpenFile(rawName,_O_CREAT | _O_RDWR | _S_IREAD | _S_IWRITE );

	XmitMessage(TRUE, FALSE);

	::NetConnection->Close();
	delete ::NetConnection;

	// Reset connection back
	::NetConnection = oldNetConnection;

	return err;
}

//================================================================
// IsTranslatorAttachment
//================================================================
BOOL IsTranslatorAttachment(const char *attachLine)
{
	return (strstr(attachLine, ".ems") ? TRUE : FALSE);
}


//================================================================
// The Progress Callback
//================================================================
CString g_ProgressMsg;
CCursor *g_EMSCursor = NULL;
BOOL	g_WasProg = FALSE;
BOOL	g_ProgStarted = FALSE;

extern "C" short EMSProgress(short howFar)    
{
	switch(howFar)
	{
		case EMS_PROGRESS_WAIT:
			if (!g_EMSCursor)
				g_EMSCursor = new CCursor();
			break;
		
		
		case EMS_PROGRESS_END:
			if (g_ProgStarted)
			{
				delete g_EMSCursor;
				g_EMSCursor = NULL;

				if (g_WasProg)
				{
					PopProgress();
					g_WasProg = FALSE;
				}
				else
					CloseProgress();
				g_ProgStarted = FALSE;
				g_ProgressMsg.Empty();
			}
			break;
	
		case EMS_PROGRESS_START:
		default:
			if (g_ProgStarted || (howFar < EMS_PROGRESS_START) || (howFar > EMS_PROGRESS_END))
				break;

			if (!g_ProgStarted)
			{
				if (InProgress)
				{
					PushProgress();
					g_WasProg = TRUE;
				}
				Progress(0, (const char *)g_ProgressMsg, EMS_PROGRESS_END);
				g_ProgStarted = TRUE;
			}
			break;
	}
	
	if ((howFar > EMS_PROGRESS_START) && (howFar < EMS_PROGRESS_END))
	{
		Progress(howFar);
	}

	return ((short)EscapePressed());
}

//================================================================
// The V3 Progress Callback
//================================================================

extern "C" short EMSV3Progress(emsProgressDataP pProgData)    
{
	if (pProgData)
	{
		if (pProgData->message && *pProgData->message) 
			g_ProgressMsg = pProgData->message;
		return (EMSProgress((short)(pProgData->value)) );
	}
	return (EMSProgress(EMS_PROGRESS_WAIT));
}

//================================================================
// The EMSGetMailBox Callback
//================================================================

extern "C" short EMSGetMailBox(emsAllocate allocFunc, emsGetMailBoxDataP pMboxInfo)    
{
	CSelectMBox	mboxDlg(NULL);

	// Figure out the size of the button
	if (pMboxInfo && pMboxInfo->prompt)
	{
		mboxDlg.m_PromptText = pMboxInfo->prompt;
	}

	
	if (pMboxInfo->flags == EMSFGETMBOX_ALLOW_NEW)
		mboxDlg.m_bAllowNew = TRUE;
	
	// We don't have Other... on Windows
	//if (pMboxInfo->flags == EMSFGETMBOX_ALLOW_OTHER)
	
	if (mboxDlg.DoModal() == IDOK && !mboxDlg.m_MailboxPath.IsEmpty())
	{
		// PI must deallocate
		if (pMboxInfo)
		{
			if (pMboxInfo->mailbox)
			{
				pMboxInfo->mailbox->size = sizeof(emsMBoxP);
				if (allocFunc)
				{
					CString relPath = mboxDlg.m_MailboxPath.Right( 
												mboxDlg.m_MailboxPath.GetLength() - EudoraDirLen);
					allocFunc((void **)&(pMboxInfo->mailbox->name), relPath.GetLength() + 1);
					strcpy( pMboxInfo->mailbox->name, relPath);
				}
			}
		}
	}
	else
		return EMSR_ABORTED;
	
	return EMSR_OK;
}

//================================================================
// The EMSSetMailBoxTag Callback
//================================================================

extern "C" short EMSSetMailBoxTag(emsSetMailBoxTagDataP pMboxTagInfo)    
{
	if (pMboxTagInfo)
	{
		if (pMboxTagInfo->mailbox && pMboxTagInfo->mailbox->name)
		{
			// Find this mailbox, marks it as a Peanut mbox
			CTocDoc* toc = GetToc(EudoraDir + pMboxTagInfo->mailbox->name);
			if (toc)
			{
				toc->SetPeanutID(pMboxTagInfo->key);
				pMboxTagInfo->oldvalue = toc->GetPeanutTag();
				toc->SetPeanutTag(pMboxTagInfo->value);
				toc->WriteHeader();
				if (toc->GetView())
					toc->GetView()->RefreshView();

			}
			else 
				return EMSR_INVALID;
		}

	}
	else 
		return EMSR_INVALID;
	return EMSR_OK;
}

//================================================================
// The EMSGetPersonality Callback
//================================================================

extern "C" short EMSGetPersonality(emsAllocate allocFunc, emsGetPersonalityDataP pPersonaData)    
{
	if (pPersonaData)
	{
		CString Address;
		CString ReaName;

		if (pPersonaData->defaultPers)
		{
			Address = GetIniString(IDS_INI_RETURN_ADDRESS);
			if (Address.IsEmpty())
					Address = GetIniString(IDS_INI_POP_ACCOUNT);
			ReaName = GetIniString(IDS_INI_REAL_NAME);
		}
		else
		{
			CSelectPersonalityDlg perDlg;
			if (perDlg.DoModal() == IDOK)
			{
				CString strCurrentPersona = g_Personalities.GetCurrent();
				g_Personalities.SetCurrent(perDlg.m_SelectedPersona);

				Address = GetIniString(IDS_INI_RETURN_ADDRESS);
				if (Address.IsEmpty())
					Address = GetIniString(IDS_INI_POP_ACCOUNT);
				ReaName = GetIniString(IDS_INI_REAL_NAME);
				
				// Restore the selected personality
				g_Personalities.SetCurrent(strCurrentPersona);
			}
			else goto fail;
		}
		if (allocFunc && pPersonaData->personality)
		{
			pPersonaData->personality->size = sizeof(emsAddressS);
			allocFunc((void **)&(pPersonaData->personality->address), Address.GetLength() + 1);
			strcpy(pPersonaData->personality->address, Address);

			allocFunc((void **)&(pPersonaData->personality->realname), ReaName.GetLength() + 1);
			strcpy(pPersonaData->personality->realname, ReaName);
			pPersonaData->personality->next = NULL;

			pPersonaData->persCount = g_Personalities.GetPersonaCount();
		}
		else goto fail;
	}
	else goto fail;
	
	return EMSR_OK;

fail:
	return EMSR_INVALID;
}

//================================================================
// The EMSRegenerate Callback
//================================================================

extern "C" short EMSRegenerate(emsRegenerateDataP pRegenData)    
{
	if(pRegenData)
	{
		if (pRegenData->which == emsRegenerateFilters)
		{
			// Reload the Filters
			// Since Plugin Filters are reloaded everytime they are used, it's not necessary
		}
		else if (pRegenData->which == emsRegenerateNicknames)
		{
			// Reload the Nicknames
			g_Nicknames->UpdateNicknameFile(pRegenData->path);
		}
	}
	else 
		return EMSR_INVALID;

	return EMSR_OK;
}

//================================================================
// The EMSGetDirectory Callback
//================================================================

extern "C" short EMSGetDirectory(emsGetDirectoryDataP pGetDirData)    
{
	if (!pGetDirData) goto fail;	

	switch(pGetDirData->which)
	{
		case EMS_EudoraDir:
			strcpy(pGetDirData->directory, ExecutableDir);
			break;
		case EMS_AttachmentsDir:
			{
				char attachDir[_MAX_PATH + 1];
				GetIniString(IDS_INI_AUTO_RECEIVE_DIR, attachDir, _MAX_PATH);
				if ( (attachDir[0] == 0) || !::FileExistsMT(attachDir, TRUE))
					sprintf(attachDir, "%s%s", (const char *)EudoraDir, (const char *)CRString(IDS_ATTACH_FOLDER)); 
				strcpy(pGetDirData->directory, attachDir);
			}
			break;
		case EMS_PluginFiltersDir:
			strcpy(pGetDirData->directory, EudoraDir);
			strcat(pGetDirData->directory, (const char *)CRString(IDS_FILTER_DIRECTORY));
			break;
		case EMS_PluginNicknamesDir:
			strcpy(pGetDirData->directory, EudoraDir);
			strcat(pGetDirData->directory, (const char *)CRString(IDS_NICK_DIR_NAME));
			break;
		case EMS_ConfigDir:
			strcpy(pGetDirData->directory, EudoraDir);
			strcat(pGetDirData->directory, (const char *)CRString(IDS_TRANS_DIRECTORY));
			break;
		case EMS_MailDir:
			strcpy(pGetDirData->directory, EudoraDir);
			break;
		case EMS_NicknamesDir:
			strcpy(pGetDirData->directory, EudoraDir);
			strcat(pGetDirData->directory, (const char *)CRString(IDS_NICK_DIR_NAME));
			break;
		case EMS_SignaturesDir:
			strcpy(pGetDirData->directory, EudoraDir);
			strcat(pGetDirData->directory, (const char *)CRString(IDS_SIGNATURE_FOLDER));
			break;
		case EMS_SpoolDir:
			pGetDirData->directory[0] = 0;
			break;
		case EMS_StationeryDir:
			strcpy(pGetDirData->directory, EudoraDir);
			strcat(pGetDirData->directory, (const char *)CRString(IDS_STATIONERY_FOLDER));
			break;
		default:
			goto fail;
	}

	return EMSR_OK;

fail:
	if (pGetDirData)
		pGetDirData->directory[0] = 0;
	return EMSR_INVALID;
}


//These two functions seem to be unimplemented and the people who complained
// about them (peanut people) haven't noticed in over a month. If they do I guess we'll write them.
extern "C" short			EMSGetMailBoxTag(emsSetMailBoxTagDataP pMboxTagInfo)
{
	return 0;
}

extern "C" short			EMSGetPersonalityInfo(emsAllocate, emsGetPersonalityInfoDataP)
{
	return 0;
}


// =======================================================================
// FreeMimeTypeV2 :: API Version
// If no free function, use the default delete
// =======================================================================
BOOL FreeMimeTypeV2(emsMIMEtype *mimeType, ems2_free_t *freeFunc)
{
	if (mimeType)
	{
 		if (mimeType->mime_version)
			freeFunc ? freeFunc(mimeType->mime_version) : delete [] mimeType->mime_version;
		if (mimeType->mime_type)
			freeFunc ? freeFunc(mimeType->mime_type) : delete [] mimeType->mime_type;
		if (mimeType->sub_type)
			freeFunc ? freeFunc(mimeType->sub_type) : delete [] mimeType->sub_type;

		emsMIMEparam *param = mimeType->params;
		while (param)
		{
			emsMIMEparam *curParam = param;
			param = curParam->next;

			if(curParam->name)
				freeFunc ? freeFunc(curParam->name) : delete [] curParam->name;
			if(curParam->value)
				freeFunc ? freeFunc(curParam->value) : delete [] curParam->value;
			if(curParam)
				freeFunc ? freeFunc(curParam) : delete curParam;
		}

		freeFunc ?	freeFunc(mimeType) : delete mimeType;
	}
	return TRUE;
}

//Untested. Do not use without testing!!!!
extern "C" ABDataP EMSEnumAddressBooks()
{
	ABDataP head = NULL;
	ABDataP current = NULL;

	CNicknameFile* NNFile = g_Nicknames->GetFirstNicknameFile();
	while (NNFile != NULL)
	{
		if (!head)
		{
			head = new ABDataS;
			current = head;
			head->next = NULL;
		}
		else
		{
			current->next = new ABDataS;
			current = current->next;
			current->next = NULL;
		}

		CString NNFileName = NNFile->GetName();

		if (NNFileName.GetLength() < 1)
		{
			current->name = new char[1];
			(current->name)[0] = 0;
		}
		else
		{
			current->name = new char[NNFileName.GetLength() +1];
			strcpy(current->name, NNFileName);
		}

		NNFile = g_Nicknames->GetNextNicknameFile();
	}
	return head;
}

extern "C" short EMSAddABEntry(emsNicknameP srcNickname, LPSTR NicknameFileName)
{
	return 0;
}



//================================================================
// This is a convienience function
//================================================================
void DebugLog(DWORD Mask, UINT StringID, ...)
{
	char buf[1024];
	
	va_list args;
	va_start(args, StringID);
	_vsnprintf(buf, sizeof(buf) - 1, CRString(StringID), args);
	va_end(args);
	PutDebugLog(Mask, buf);
}


// converts all the URIs to local file references based on the "Embedded Content
// on input *ppHTML should have the raw html
// on return *ppHTML will point to a buffer that contains morphed html
// *ppHTML may be changed as a side effect of this call
void MorphMHTML( char ** ppHTML )
{
	if ( ! *ppHTML ) return;	// did we get raw html?

	char * pCookedHTML = NULL;

	long lCookedHTMLSize = ConvertURIs( *ppHTML, &pCookedHTML );
	if ( lCookedHTMLSize > 0 && pCookedHTML )
	{
		delete [] *ppHTML;									// delete the old buffer
		*ppHTML = new char[ lCookedHTMLSize ];				// alloc a new buffer
		if ( *ppHTML )
			memcpy( *ppHTML, pCookedHTML, lCookedHTMLSize );// fill in the new buffer
		free( pCookedHTML );								// all done 
	}
}
