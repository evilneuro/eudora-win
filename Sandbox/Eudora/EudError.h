#ifndef __EUDERROR_H__
#define __EUDERROR_H__

//
// HRESULT error codes for the Generic Services module.  WARNING!
// Do not use the range 0x0000 to 0x01FF (511) since that is reserved
// for OLE-defined FACILITY_ITF codes.
//


#if 0  //going away from HRESULT type code

#define SMTP_E_FILE_UNSENDABLE		2001	// could not 
#define SPOOL_S_FILE_SENT			2010	// Spool file was sent successfully
#define SPOOL_S_FILE_UNSENT			2011	// Spool file was not sent because of unknown error.
#define SPOOL_S_FILE_UNSENDABLE		2012	// Spool file was unsendable because of SMTP error.
#define SPOOL_S_FILE_RECEIVED		2013	// Spool file was successfully downloaded from server.
#define SPOOL_S_FILE_SEND			2014	// Spool file was spooled successfully


#endif


#endif // __EUDERROR__
