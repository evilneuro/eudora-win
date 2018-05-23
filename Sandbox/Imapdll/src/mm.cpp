/////////////////////////////////////////////////////////////////////////////
// 	mm.cpp - The CMailGets class imlementation. 


#include "stdafx.h"

#ifdef IMAP4 // Only for IMAP.

#include "MyTypes.h"

// #define _IMAPDLL_INTERNAL_
// #include "Network.h"
//#include "imapmail.h"

#include "imap.h"

#include "exports.h"
#include "ImapDefs.h"
#include "Stream.h"
#include "proto.h"
#include "mm.h"
#include "assert.h"


// ========== MACROS ===============
#ifdef BUFSIZE
#undef BUFSIZE
#endif
#define	BUFSIZE		2048L

// Statics:
// 
static void (*__DebugLog) (char *string,long errflg) = NULL;


// ============= static functions ==============
static char * file_gets (readfn_t readfn, void *read_data, unsigned long size, GETS_DATA* md);


// ========================================================================//


//	================================================================================
//	================================================================================
//	================================================================================
//		IMAPGETS Mail Service Object
//	================================================================================
//	================================================================================
//	================================================================================



// ======================== CMailGets =========================================/


CMailGets::CMailGets()
{
	m_pStream = NULL;
	m_pWriter	  = NULL;

	// Set our mailgets into c-client
	m_oldMailGets = NULL;
	m_newMailGets = NULL;
	m_buffer	  = NULL;

}
 

CMailGets::CMailGets(CProtocol *stream, CWriter *Writer)
{
	// User must pass a CWriter class pointer that's called to do the actual writing.
	m_pWriter = Writer;
	m_pStream = stream;

	m_newMailGets = NULL;
	m_buffer = NULL;
}


CMailGets::~CMailGets()
{
	// Free buffer.
	if (m_buffer)
		delete[] m_buffer;
}


// Set out "file_gets()" into c-client.
// NOTE: MUST always call ClearGets() after calling this!!!
void CMailGets::SetMailGets (void)
{
	if (m_pStream)
	{
		m_oldMailGets = (mailgets_t) m_pStream->GetMailGets();

		m_pStream->SetMailGets (file_gets);

		m_newMailGets = file_gets;
	}
}


// Note - we shouldn't call this without previously calling SetGets(), 
// else we run the risk of setting a NULL mailgets into c-client.
void CMailGets::ResetMailGets (void)
{
	if (m_pStream)
	{
		m_pStream->SetMailGets (m_oldMailGets);

		m_newMailGets = NULL;
	}
}



// Do the work of fetching the data, decoding it if necessary and streaming
// it to the file.
// NOTES
// The "size" parameter is the size of the total data to read.
BOOL CMailGets::DoMailgets (readfn_t readfn, void *read_data, CProtocol *stream, unsigned long size)	
{
    char			*ret = NULL;
	LONG			lSize = (long)size;
	BOOL			bResult = FALSE;

	// 
	UNREFERENCED_PARAMETER(stream);

	// Sanity checks.
    assert (readfn != NULL);
    assert (stream != NULL);

	// m_Writer must have been set. Otherwise, can't continue.
	if (!m_pWriter)
		return FALSE;

	// Allocate buffer
	if (m_buffer == NULL)
		m_buffer = new char[BUFSIZE + 1];

	if (!m_buffer)
		return FALSE;

	// Pass the reader function up to the writer object, including the total length of
	//  the data to be read.

	return m_pWriter->Write (readfn, read_data, size);
}



// This is what is called to save the data.
// NOTES
// The "size" parameter if the size of the total data.
// END NOTES
static char * file_gets (readfn_t readfn, void *read_data, unsigned long size, GETS_DATA* md)
{
	if (!md)
		return NULL;

	CProtocol *l_stream = (CProtocol *) md->stream;
	
	if (!l_stream)
	{
		ASSERT(0);
		return NULL;
	}

	CMailGets *Mailgets = (CMailGets *) (l_stream->mgets_data);
	if (Mailgets)
	{
		return (char *) Mailgets->DoMailgets (readfn, read_data, l_stream, size);
	}
	else
		return NULL;
}




//====================== Implementation of the CMboxGets class ============/

CMboxGets::CMboxGets (CProtocol *stream, CLister *Lister)
{
	m_pStream = stream;
	m_pLister  = Lister;
}


CMboxGets::~CMboxGets ()
{
}



// Sanitize the mailbox name and send it up the pipeline.
BOOL CMboxGets::DoMboxgets (char *mailbox, int delimiter, long attributes)
{
	char		*name = NULL, *p = NULL;
	BOOLEAN		IN_ACTUAL_NAME = FALSE;	

	// m_Lister must have been set. Otherwise, can't continue.
	if (!m_pLister)
		return FALSE;

	if (!mailbox)
		return FALSE;
	else if (!*mailbox)		// Must have something in it.
		return FALSE;

	// Simply call CLister::Add() routine.
	return m_pLister->AddMailbox (mailbox, delimiter, attributes);
}




//================== CLoginGets class ==================//

CLoginGets::CLoginGets (CProtocol *stream, CIMLogin *Login)
{
	m_pStream = stream;
	m_pLogin  = Login;
}


CLoginGets::~CLoginGets ()
{
}



BOOL CLoginGets::DoLogingets (NETMBX *mb, char *user, char *pwd, long trial)
{
	CIMLogin *pLogin = NULL;

	// mb is unused:
	if (mb);

	if (m_pLogin)
	{
		pLogin = (CIMLogin *)m_pLogin;
		return m_pLogin->Login (user, pwd, trial);
	}
	else
		return FALSE;
}



//====================== This is global ==================/

// Exported functions
// Set the debug logging callback.
//
// NOTE: These should eventually be object soecific - i.e., should go insto a CProtocol.
//
void mm_set_debug_log(void (*fn) (char *, long))
{
	// Can set this only once.
	if (__DebugLog)
		return;

	if (fn)
		__DebugLog = fn;
}


void mm_log (char *string,long errflg)
{
	//
	// Only in debug mode.
	//
#ifdef _DEBUG
	if (!string)
		return;

	if (__DebugLog)
	{
		(*__DebugLog) (string, errflg);
	}

#else

	UNREFERENCED_PARAMETER(string);
	UNREFERENCED_PARAMETER(errflg);

#endif // _DEBUG
}


#endif // IMAP4

