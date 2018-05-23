// TransReader.cpp
//
// MIMEReader classes for handling EMSAPI translations

#include "stdafx.h"

#include "resource.h"
#include "rs.h"
#include "eudora.h"
#include "fileutil.h"
#include "lex822.h"
#include "header.h"
#include "LineReader.h"
#include "mime.h"
#include "pop.h"
#include "trnslate.h"
#include "TransReader.h"


#include "DebugNewHelpers.h"

extern CString g_OutputDesc;

bool g_bMessageToBeDeleted = FALSE;
//This variable will be set when a plugin wants that message to be nuked.
//Its OK to keep this as a global for now since the code that uses it is always 
//called in the main thread.

// TransReader
//
// Base class for doing EMSAPI translations.
// Reads in a stream from a LineReader object, and spits it out to a file.
//
BoundaryType TransReader::ReadTL(CObArray& MimeStates, char* buf, LONG bSize, JJFile* file)
{
	BoundaryType boundaryType = btEndOfMessage;
	MIMEState* ms;
	MIMEState* parentMS = NULL;
	HeaderDesc* hd;
	long err = 0;

	// grab descriptors for our message
	if (!MimeStates.GetSize())
		return (btError);
	ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];
	hd = ms->m_hd;
	
	// find our birth mother
	if (MimeStates.GetSize() > 1)
		parentMS = (MIMEState*)MimeStates[MimeStates.GetUpperBound() - 1];

	// Write out the headers to file
	WriteHeaders(hd, file);

	// Read & Write bytes to file
	int size = 0;
	for (size = ms->m_LineReader->ReadLine(buf, bSize); size > 0; size = ms->m_LineReader->ReadLine(buf, bSize))
	{
		if (parentMS && (boundaryType = parentMS->IsBoundaryLine(buf)))
			break;

		if (size)
			file->Put(buf, size);
	}
	file->PutLine();

	if (file)
	{
		file->Close();
		if (err) file->Delete();
	}
	return(err ? btError : boundaryType);
}

// WriteHeaders
//
// Writes out the various MIME headers to an EMSAPI translation file
//
BOOL TransReader::WriteHeaders(HeaderDesc* hd, JJFile* file)
{
	file->Put((const char *)CRString(IDS_MIME_HEADER_VERSION));
	file->Put(' ');
	file->PutLine(hd->m_TLMime->GetVersion());
	
	file->Put((const char *)CRString(IDS_MIME_HEADER_CONTENT_TYPE));
	file->Put(' ');
	file->Put(hd->m_TLMime->GetType());
	file->Put("/");
	file->Put(hd->m_TLMime->GetSubtype());
	
	char qtValue[256];
	const emsMIMEparam *pParam = hd->m_TLMime->GetParams();
	while(pParam)
	{
		// This is a hack because we store content disposition and params with the main
		// params, so don't dump them here
		if (CompareRStringI(IDS_MIME_HEADER_CONTENT_DISP, pParam->name) &&
			CompareRStringI(IDS_MIME_CONTENT_DISP_FILENAME, pParam->name) )
		{
			file->Put("; ");
			file->Put(pParam->name);
			file->Put("=");
			Quote822(qtValue, pParam->value, TRUE, TRUE);
			file->Put(qtValue);
		}
		pParam = pParam->next;
	} 
	
	file->PutLine();

	if (hd->contentEnco[0])
	{
		file->Put((const char *)CRString(IDS_MIME_HEADER_CONTENT_ENCODING));
		file->Put(' ');
		file->PutLine(hd->contentEnco);
	}
	
	AttributeElement *ae = hd->GetAttribute(CRString(IDS_MIME_HEADER_CONTENT_DISP));
	if (ae && ae->m_Value)
	{
		file->Put((const char *)CRString(IDS_MIME_HEADER_CONTENT_DISP));
		file->Put(' ');
		file->Put(ae->m_Value);

		AttributeElement *ae = hd->GetAttribute(CRString(IDS_MIME_CONTENT_DISP_FILENAME));
		if (ae && ae->m_Value)
		{
			file->Put("; ");
			file->Put(ae->m_Name);
			file->Put("=");
			Quote822(qtValue, ae->m_Value, TRUE, TRUE);
			file->Put(qtValue);
		}
		file->PutLine();
	}
	
	file->PutLine();
	
	return TRUE;
}

// GetTransIDString
//
// Formats and returns a string that contains the plug-in and translator IDs
// that looks like this: "<plug-in ID,translator ID>"
//
CString TransReader::GetTransIDString(MIMEState* ms)
{
	long ModuleID = 0, TransID = 0;
	CString Result;

	ms->m_Translator->GetID(ModuleID, TransID);
	Result.Format("<%04ld.%04ld>", ModuleID, TransID);

	return Result;
}




// NowTransReader
//
// Class for doing EMSAPI translations that an ON_ARRIVAL translator
// has said to do now
//
BoundaryType NowTransReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	BoundaryType boundaryType = btEndOfMessage;
	JJFile *file = NULL;
	// grab descriptors for our message
	if (!MimeStates.GetSize())
		return (btError);
	MIMEState *ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];

	// Get the Message ..
	// Write to a file
	CString outFileName = ::GetTmpFileNameMT();
	char fileName[_MAX_PATH + 1];
	strcpy(fileName,ms->m_hd->subj);
	if (ms->m_hd->subj && *(ms->m_hd->subj))
		strcpy(fileName, ms->m_hd->subj);
	else
		strcpy(fileName, CRString(IDS_UNTITLED));
	strcat(fileName,".ems");

	file = OpenAttachFile(fileName);
	boundaryType = ReadTL(MimeStates, buf, bSize, file);

	// Get all the addresses
	CTLAddress *addresses = GetEMSHeaders(ms->m_hd->lDiskStart);
//	if (!g_tempaddresses)
//	{
//		g_tempaddresses = GetEMSHeaders(ms->m_hd->lDiskStart);
//	}
	
	// Run translation
	CString strFilename("???");
	{
		BSTR bstrFilename = NULL;
		if (SUCCEEDED(file->GetFName(&bstrFilename)))
		{
			strFilename = bstrFilename;
			::SysFreeString(bstrFilename);
		}
	}
	long retCode = 0;
	g_OutputDesc.Empty();
	long err = GetTransMan()->InterpretFile(EMSF_ON_ARRIVAL, (const char *) strFilename, outFileName, addresses, &retCode);

	if (g_OutputDesc.IsEmpty() == FALSE)
	{
		g_pPOPInboxFile->EnsureNewline();
		g_pPOPInboxFile->Put(g_OutputDesc);
	}

	if (addresses) delete addresses;
	// JES: This following Line is designed to remove the content type and header 
	// thrown into a message when a NOTNOW translator is run.
	if (ms->IsGrokked())
	{
		char		szLogBuf[256];
		sprintf(szLogBuf, "LOGNULL NowTransReader::ReadIt() JJFileMT::Truncate(%d)", ms->m_hd->lDiskStart);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		g_pPOPInboxFile->Truncate(ms->m_hd->lDiskStart);
	}
	
	if (boundaryType != btError)
	{
		if (err == EMSR_DATA_UNCHANGED)
		{
			//	The translator didn't change the data. Write the "Run Plugin:" line so
			//	that the translator can be run again later. This makes Eudora friendlier to
			//	Peanut & MLM by fixing the bug where it incorrectly added the text of
			//	their command attachments to the email message - GCW
			WriteAttachNote( file, IDS_TRANS_FILE_TAG, GetTransIDString(ms) );
			CFile::Remove(outFileName);
		}
		else if (err == EMSR_DELETE_MESSAGE)
		{
			if (GetIniShort(IDS_INI_EMS_IGNOREDELETE) == 1) 
				g_bMessageToBeDeleted = FALSE;
			else
				g_bMessageToBeDeleted = TRUE;
			//the plugin wants this message to be nuked.
			CFile::Remove(outFileName);
		}
		else
		{
			int i = 0;
			char locBuf[2048];
			long lNumBytesRead = 0;
			// Move the result into inbox
			JJFile outFile;
			if (FAILED(outFile.Open(outFileName, O_RDWR)))
			{
				//
				// Oops.  Whoever wrote this didn't check for file I/O errors.
				// WKS 97.08.08
				//
				ASSERT(0);		// headed for trouble below...
			}

			do
			{
				i++;
				outFile.GetLine(locBuf, bSize, &lNumBytesRead);
				// First 2 lines are mime headers that we no longer use, that's what i's counting
				if ( i < 4  && (locBuf))
				{
					CRString MimeVersion(IDS_MIME_HEADER_VERSION);
					CRString ContentType(IDS_MIME_HEADER_CONTENT_TYPE);
					CRString ContentDisp(IDS_MIME_HEADER_CONTENT_DISP);
					
					if (!strncmp(locBuf,(const char *)MimeVersion, MimeVersion.GetLength())
						|| !strncmp(locBuf,(const char *)ContentType, ContentType.GetLength())
						|| !strncmp(locBuf,(const char *)ContentDisp, ContentType.GetLength()) )
					{
						while (locBuf[strlen(locBuf)-1] == ';')
						{
							outFile.GetLine(locBuf, bSize, &lNumBytesRead);
						}
					continue;
					}
				}
				if (lNumBytesRead > 0 )
				{
					// line length of 2 is a newline (Get doesn't acutally return that)
					if (lNumBytesRead == 2)
						g_pPOPInboxFile->PutLine();
					else
						g_pPOPInboxFile->PutLine(locBuf);
				}
			} while (lNumBytesRead > 0);

			outFile.Close();
			outFile.Delete();

			// Now clean up 
			file->Delete();
		}
	}
	
	delete file;


	return (boundaryType);
}




// NotNowTransReader
//
// Class for doing EMSAPI translations that an ON_ARRIVAL translator
// has said not to do now.  Bascially just saves to a file, then writes
// the "Run Plugin:" line so that the translator can be run later.
//
BoundaryType NotNowTransReader::ReadIt(CObArray& MimeStates, char* buf, LONG bSize)
{
	BoundaryType boundaryType = btEndOfMessage;
	JJFile* file = NULL;
	JJFile outFile;

	// grab descriptors for our message
	if (!MimeStates.GetSize())
		return (btError);
	MIMEState *ms = (MIMEState*)MimeStates[MimeStates.GetUpperBound()];

	// Get the Message ..
	// Write to a file
	char fileName[_MAX_PATH + 1];
	if (ms->m_hd->subj && *(ms->m_hd->subj))
		strcpy(fileName, ms->m_hd->subj);
	else
		strcpy(fileName, CRString(IDS_UNTITLED));
	strcat(fileName,".ems");

	
	file = OpenAttachFile(fileName);
	boundaryType = ReadTL(MimeStates, buf, bSize, file);

	// JES: This following Line is designed to remove the content type and header 
	// thrown into a message when a NOTNOW translator is run.
	if (ms->IsGrokked())
	{
		char		szLogBuf[256];
		sprintf(szLogBuf, "LOGNULL NotNowTransReader::ReadIt() JJFileMT::Truncate(%d)", ms->m_hd->lDiskStart);
		PutDebugLog(DEBUG_MASK_TOC_CORRUPT, szLogBuf);

		g_pPOPInboxFile->Truncate(ms->m_hd->lDiskStart);	
	}

	// Put file from text into msg
	if (boundaryType!=btError)
	{
		WriteAttachNote(file, IDS_TRANS_FILE_TAG, GetTransIDString(ms));
	}

	delete file;

	return (boundaryType);
}
