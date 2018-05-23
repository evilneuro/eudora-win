/******************************************************************************/
/*																										*/
/*	Name		:	DSDEFS.H			                                                */
/* Date     :  7/18/1997                                                      */
/* Author   :  Jim Susoy                                                      */
/* Notice   :  (C) 1997 Qualcomm, Inc. - All Rights Reserved                  */
/* Copyright (c) 2016, Computer History Museum 
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
DAMAGE. */

/*	Desc.		:	Directory Services shared defintiions					            */
/*																			                     */
/******************************************************************************/
#ifndef __DSDEFS_H__
#define __DSDEFS_H__


// I can't believe this one's not allready defined by Microsoft. We got PASCAL;
// we got NTAPI, WINAPI, CALLBACK, etc. They all are defined as "__stdcall".
// But I don't wanna call things PASCAL anymore, and the others seem confusing
// in one way or another... so here is what should allready be.
#define STDCALL __stdcall



/* -------------------------------------------------------------------------- */
/* These are the agreed upon fields for which protocols module will attempt	*/
/* to map data to and from.													  				*/
/* -------------------------------------------------------------------------- */
typedef enum {
	DS_NAME = 0,	 							/* Name (ie. "Steve Sprigg")			   */
	DS_COMPANY,		 							/* Company name								*/
	DS_DIVISION,	 							/* Division of company						*/
	DS_POSITION,	 							/* Position held @ company					*/
	DS_LOCATION,								/* Location in co. (ie. Bldg I-215A)	*/
	DS_ADDRESS,									/* Work address								*/
	DS_CITY,										/* Work city							    	*/
	DS_STATE,									/* Work state									*/
	DS_ZIPCODE,									/* Work Zip Code								*/
	DS_PHONE,									/* Work phone number							*/
	DS_CELL,										/* Work cell phone number				   */
	DS_PAGER,									/* Work pager number							*/
	DS_PEMAIL,									/* Work pager email address  				*/
	DS_FAX,										/* Work Fax number							*/
	DS_EMAIL,									/* Work email address						*/
	DS_PGPKEY,									/* Work related PGP key						*/
	DS_PAGERTYPE,								/* Text representation of Pager Type	*/
	DS_PAGERPIN,								/* Pager PIN 									*/
	DS_HADDRESS,								/* Home address								*/
	DS_HCITY,									/* Home city									*/
	DS_HSTATE,									/* Home state									*/
	DS_HZIPCODE,								/* Home Zip Code								*/
	DS_HPHONE,									/* Home phone number							*/
	DS_HCELL,									/* Home cell phone number					*/
	DS_HPAGER,									/* Home pager number							*/
	DS_HPEMAIL,									/* Home pager email address 				*/
	DS_HFAX,										/* Home Fax number							*/
	DS_HEMAIL,									/* Home email address						*/
	DS_HPGPKEY,									/* Home related PGP key						*/
	DS_HPAGERTYPE,								/* Text representation of Pager Type	*/
	DS_HPAGERPIN,								/* Pager PIN 									*/
	DS_USERDEFINED,							/* User defined attribute					*/
	DS_NULL,                            // hack? why yes, it is! thanks for asking.

	DS_UNKNOWN,		 							/* Unknown data type							*/
	DS_COMMENTS,								/* Comments										*/
	DS_EMAILALIAS,								/* Email Alias									*/

	DS_MAXFIELD
} DS_FIELD;


/* -------------------------------------------------------------------------- */
/* Data type specifier for record element data.								  			*/
/* -------------------------------------------------------------------------- */
typedef enum {
	DST_ASCII,									/* ASCII string, no NULL terminator		*/
	DST_ASCIIZ,									/* NULL terminated ASCII string			*/
	DST_BINARY,									/* Binary data									*/
	DST_GIF,										/* GIF image									*/
	DST_JPG,										/* JPEG image									*/
	DST_IMAGE,									/* Unknown format image						*/
	DST_WORDDOC,								/* Microsoft word document					*/
	DST_EXCEL,									/* Microsoft Excel spreadsheet			*/
	DST_URL,										/* ASCIIZ URL									*/
	DST_NULL                            // hacky-poo
} DS_TYPE;


/* -------------------------------------------------------------------------- */
/* Return status value for a query operation.								 			*/
/* -------------------------------------------------------------------------- */
typedef enum {
	QUERY_OK,									/* GetRecord() returned a record			*/
	QUERY_FAILED,								/* Error performing query					*/
	QUERY_WOULDBLOCK,							/* No data, use WaitForRecord()			*/
	QUERY_DONE,									/* No more records to return.				*/
	QUERY_CANCELLED,							/* Cancelled by user...						*/
	QUERY_EBADFORMAT,							/* Format error (invalid args etc.)		*/
	QUERY_ECONNECT,							/* Unable to connect/Attach to database*/
	QUERY_EADDRESS,							/* Database location not valid			*/
	QUERY_EREAD,								/* Protocol Error reading 					*/
	QUERY_EWRITE,								/* Protocol Error Writing 					*/
	QUERY_EOUTOFOMEMORY,						/* I can't remember							*/
	QUERY_EPROTOCOL,							/* Database protocol error!!!				*/
   QUERY_EEXISTS,                      /* (IDSPOST) Record already in database*/
   QUERY_ENOMATCH,                      /* Matching record could not be found  */
	QUERY_TIMEDOUT
} QUERY_STATUS;

/* -------------------------------------------------------------------------- */
/* Return status value for a write operation.								 			*/
/* -------------------------------------------------------------------------- */
typedef enum {
	WRITE_OK,									/* GetRecord() returned a record			*/
	WRITE_NOUPDATE,							/* Failed - Update not specified...		*/
	WRITE_NOCREATE,							/* Failed - Create not specified...		*/
} WRITE_STATUS;


// ?????
	#define DSWF_CREATE			0x0001
	#define DSWF_UPDATE			0x0000

/* -------------------------------------------------------------------------- */
/* Database record element structure defintion								  			*/
/* -------------------------------------------------------------------------- */
typedef struct tagDBRECENT {
	struct tagDBRECENT   *pNext;			/* Ptr to next element in record			*/
	DS_FIELD             nName;			/* Agreed meaning of element				*/
	LPSTR                lpszUserName;	/* User defined name of this attribute */
	DS_TYPE              nType;			/* Type of data								*/
	DWORD                dwSize;			/* Size of data								*/
	BYTE                 data[1];			/* Data... 										*/
} DBRECENT, *LPDBRECENT;

/* -------------------------------------------------------------------------- */
/* Flags for the IDSPostRecord->PostRecord()	method									*/
/* -------------------------------------------------------------------------- */
#define POST_NOFLAGS	0x000000
#define POST_ADD		0x000001
#define POST_UPDATE  0x000002
#define POST_DELETE  0x000004

/* -------------------------------------------------------------------------- */
/* Flags for the IDSPostRecord->Initialize() method									*/
/* -------------------------------------------------------------------------- */
#define POST_INIT_NOFLAGS	0x000000

/* -------------------------------------------------------------------------- */
/* Interface for records returned by Query engine										*/
/* -------------------------------------------------------------------------- */
interface IDSPRecord : IUnknown
{
	virtual DBRECENT *	__stdcall GetRecordList() PURE;
	virtual LPSTR			__stdcall GetRecordID() PURE;
	virtual LPSTR        __stdcall GetDatabaseID() PURE;
	virtual QUERY_STATUS __stdcall GetError(LPSTR pszBuffer,int nLen) PURE;
};
// {221E6BD1-1027-11d1-8880-00805F4A192C}
DEFINE_GUID(IID_IDSPRecord,   0x221e6bd1,0x1027,0x11d1,0x88,0x80,0x0,0x80,0x5f,0x4a,0x19,0x2c);

/* -------------------------------------------------------------------------- */
/* Query Flags...																					*/
/* -------------------------------------------------------------------------- */
#define DSQUERY_INCREMENTAL	0x0000001/* Live query.  PH uses '*' 				*/

/* -------------------------------------------------------------------------- */
/* Protocol entry flags definitions															*/
/* -------------------------------------------------------------------------- */
#define PROTO_LOCAL				0x000001	/* Protocol to local resource				*/
#define PROTO_WRITEABLE			0x000002	/* Protocol supports updates				*/
#define PROTO_MODSRCHBASE  	0x000004	/* Protocol allow base of search TBD	*/
#define PROTO_INCREMENTAL		0x000008	/* Protocol supports incremental query */
#define PROTO_RAW     			0x000010 /* Protocol supports RAW queries			*/
#define PROTO_RAWONLY         0x000020 /* Protocol supports RAW ONLY queries  */
#define PROTO_DBRDONLY			0x000040 /* Databases cannot be created			*/

/* -------------------------------------------------------------------------- */
/* Image size definitions for GetXXXXXXXImage() functions							*/
/* -------------------------------------------------------------------------- */
#define IMG_SMALL					0
#define IMG_LARGE             1

/* -------------------------------------------------------------------------- */
/* Query flags																						*/
/* -------------------------------------------------------------------------- */
#define QUERY_FLG_NONE			0x000000
#define QUERY_FLG_RAW			0x000001
#define QUERY_FLG_RAWQUERY		0x000002	/* Accecpts raw query, returns 			*/
													/* structured data							*/

/* -------------------------------------------------------------------------- */
/* Prototype for the Query Callback function passed to IAsyncQuery::StartQuery*/
/* -------------------------------------------------------------------------- */
typedef void (*DBQUERYCB)(LPVOID pCxt);

/* -------------------------------------------------------------------------- */
/* Interface Constants and Types for IDirConfig											*/
/* -------------------------------------------------------------------------- */
enum DataType {
     DIR_UNKNOWN = 0,       // -> encoundered data written by other program ?
     DIR_INT     = 1,       // -> a 32 bit signed value
     DIR_BOOL    = 2,       // -> a 16 bit UINT
     DIR_DWORD   = 3,       // -> a 32 bit unsigned value
     DIR_STR     = 4,       // -> an ASCII NULL terminated string
     DIR_BINARY  = 5,       // -> a block of binary data of unlimited size
};


#endif



