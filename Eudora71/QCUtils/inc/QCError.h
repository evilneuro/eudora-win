#ifndef __QCERROR_H__
#define __QCERROR_H__

//
// HRESULT error codes for the Generic Services module.  WARNING!
// Do not use the range 0x0000 to 0x01FF (511) since that is reserved
// for OLE-defined FACILITY_ITF codes.
//
#define QCUTIL_E_FILE_OPEN_WRITING		1001	// could not open file for writing
#define QCUTIL_E_FILE_OPEN_READING		1002	// could not open file for reading
#define QCUTIL_E_FILE_RENAME			1003	// could not rename file
#define QCUTIL_E_FILE_DELETE			1004	// could not delete file
#define QCUTIL_E_FILE_WRITE				1005	// could not write to file
#define QCUTIL_E_FILE_READ				1006	// could not read from file
#define QCUTIL_E_FILE_CLOSE				1007	// could not close file
#define QCUTIL_E_FILE_CHANGESIZE		1008	// could not change file size


#endif // __QCERROR_H__
