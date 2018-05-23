// ImapExports.h - Exported data types from IMAP.

#ifndef __IMAPEXPORTS_H_
#define __IMAPEXPORTS_H_

// HRESULT macros.

// Make BUSY:
#define HRESULT_MAKE_BUSY \
	(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_BUSY))

// Contains BUSY:
//
#define HRESULT_CONTAINS_BUSY(hr)  (HRESULT_CODE((HRESULT)(hr)) == ERROR_BUSY)

// Make user cancelled.
#define HRESULT_MAKE_CANCEL \
	(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_CANCELLED))

// Contains user cancelled.
#define HRESULT_CONTAINS_CANCEL(hr) \
	(HRESULT_CODE((HRESULT)(hr)) == ERROR_CANCELLED)


#define HRESULT_MAKE_OFFLINE \
	(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, ERROR_CONNECTION_UNAVAIL))

#define HRESULT_CONTAINS_OFFLINE(hr) \
	(HRESULT_CODE((HRESULT)(hr)) == ERROR_CONNECTION_UNAVAIL)

#define HRESULT_CONTAINS_LOGON_FAILURE(hr) \
	(HRESULT_CODE((HRESULT)(hr)) == ERROR_LOGON_NOT_GRANTED)


// Exported declarations.
// Note: These sould be moved to a separate header file.
typedef	unsigned long  ACCOUNT_ID;
#define BAD_ACCOUNT_ID	   (ACCOUNT_ID)0L

// Different platforms would have different directory delimiters.
// This should be defined by Eudora propper and be platform dependent.
#define		DirectoryDelimiter	'\\' 

// ImapMailboxType
enum ImapMailboxType
{
	IMAP_ACCOUNT,
	IMAP_NAMESPACE,
	IMAP_MAILBOX
};


// ============== Utility ===================/
BOOL  DirectoryExists (LPCSTR pPath);
void FormatBasePath (LPCSTR pPath, CString &BasePath);


#endif // __IMAPEXPORTS_H_
