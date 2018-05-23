// imap.h 
//

// Declarations for the CImap* classes.
//
// Notes:
// This file is the main C++ interface to the imap.dll and exports the following:
// 1. CIMAP class - Does low-level IMAP fetch, store, etc for a single mailbox or
//    control stream.


#ifndef _IMAP_H_
#define _IMAP_H_

#define DllExport	__declspec( dllexport )


class CReader;
class CLister;
class CIMLogin;
class CEltWriter;


// Typedefs used internally and also exported:
//
#include "exports.h"
#include "sslsettings.h"

// ================ New for Eudora  =====================//

// For passing IMAP flag info.
// Note: NEED to extend this to handle Eudora's flags via annotations. In that
// case, we'd probably need a way to indicate which flags IMAP can handle (via annotations).
typedef struct
{
	BOOL	DELETED;
	BOOL	SEEN;
	BOOL	FLAGGED;
	BOOL	ANSWERED;
	BOOL	DRAFT;
	BOOL	RECENT;
} IMAPFLAGS;


// IMAPFULL structure for returning info from FetchFast, FetchFlags and FetchFull.

typedef struct
{
	IMAPFLAGS		*Flags;
	char			*InternalDate;
	ENVELOPE		*Env;
	BODY			*Body;
} IMAPFULL;


// CWriter: abstract base class for saving data returned from the IMAP server.

class CWriter
{
public:
	CWriter (){}

// These functions must be overridden:
	virtual  BOOL   Write   (readfn_t readfn, void * read_data, unsigned long size) = 0;
};


// CReader: abstract base class for fetching data and sending it to the IMAP server.

const unsigned long  BufferSize = 4096;

class CReader
{
public:
	CReader (){}

// These functions must be overridden:
	virtual  unsigned long	GetTotalSize () = 0;
	virtual	 long 			Read	(char *buffer, ULONG BufferSize) = 0;
	virtual	 BOOL			Done	() = 0;
};


class CLister
{
public:
	CLister(){}

// These methods MUST be overridden:
	virtual BOOL	AddMailbox (char *name, int delimiter, long attributes) = 0;
};



class  CIMLogin
{
public:
	CIMLogin(){}

// These methods MUST be overridden:
	virtual BOOL	Login (char *user, char *pwd, long trial) = 0;
};


// This class is used to fetch flags, timestamps, etc., about messages. 
class CEltWriter
{
public:
	CEltWriter (){}

	// This must be overridden.
	virtual BOOL  SetFlag (	unsigned long uid,
							unsigned int seen,
							unsigned int deleted,
							unsigned int flagged,
							unsigned int answered,
							unsigned int draft,
							unsigned int recent) = 0;
};


// ==========================================================================
// Exported functions ==============
void DllExport SetDebugLog (void (*fn) (char *, long));
void DllExport SetTimeoutQuery (BOOL (*fn) () );
void DllExport SetPreviewMessage (UINT umsgPreviewMessage);


#endif // _IMAP_H_

