// LDAP Directory Services
//
// File: 			export.cpp
// Author:          Mark K. Joseph, Ph.D.
// Copyright		08/26/1997 Qualcomm Inc.  All Rights Reserved.
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

// Version:  		1.0
// Description:		
// Convert our IDSPRecord format into standard data formats such as: HTML, plain text, and vCard.
//
#pragma warning(disable : 4514 4706)
#include <afx.h>
#include <ole2.h>
#include <stdlib.h>
#include "DebugNewHelpers.h"
#include "factory.h"
#include "ds.h"


// Constants used to generate HTML
//
#define  HTML_HEAD_BEGIN    "<HTML>\r\n<HEAD>\r\n"
#define  HTML_TITLE         "<TITLE>Directory Search Match</TITLE>\r\n"
#define  HTML_GENERATOR     "<META NAME=\"Eudora Directory 1.0\" CONTENT=\"DServ.exe\">\r\n"
#define  HTML_HEAD_END      "</HEAD>\r\n<BODY bgcolor=\"#C0C0C0\">\r\n"
#define  HTML_DOC_END       "</BODY>\r\n</HTML>\r\n"
#define  HTML_MAILTO_START  "<A HREF=mailto:"
#define  HTML_LINK_END      "</A>"
#define  HTML_TAG_END       ">"
#define  HTML_LINE_BREAK    "<BR>\r\n"
#define  HTML_BOLD_START    "<B>"
#define  HTML_BOLD_END      "</B>"
#define  HTML_TABLE_START   "<TABLE BORDER=5 CELLPADDING=2>\r\n"
#define  HTML_TABLE_END     "</TABLE>\r\n"
#define  HTML_TABLE_ROW     "<TR>"
#define  HTML_TCELL_START   "<TD>"
#define  HTML_TCELL_END     "</TD>\r\n"


// Given a search record hit and a file name, translate that record hit into simple HTML for pretty viewing.
//
HRESULT __stdcall CDirServ::ExportToHTML(IDSPRecord* pSearchHit,LPSTR pszFileName,BOOL OverWrite,DWORD)
{
    char       szBuffer[500];
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;
    HANDLE     hFile;
    HRESULT    hErr;

    // -> append to the end or over write what is there ?
    if (pSearchHit == NULL || pszFileName == NULL) return( E_INVALIDARG );
	if ( OverWrite ) {
	     hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
         if (hFile == INVALID_HANDLE_VALUE) hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
	}
	else {
	     hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
         if (hFile != INVALID_HANDLE_VALUE) SetFilePointer( hFile, 0, NULL, FILE_END );
	}
    if (hFile == INVALID_HANDLE_VALUE) return( E_FAIL );


    // -> start the HTML file
	WriteFile( hFile, HTML_HEAD_BEGIN,  (DWORD)lstrlen(HTML_HEAD_BEGIN),  &BytesWritten, NULL );
	WriteFile( hFile, HTML_TITLE,       (DWORD)lstrlen(HTML_TITLE),       &BytesWritten, NULL );
	WriteFile( hFile, HTML_GENERATOR,   (DWORD)lstrlen(HTML_GENERATOR),   &BytesWritten, NULL );
	WriteFile( hFile, HTML_HEAD_END,    (DWORD)lstrlen(HTML_HEAD_END),    &BytesWritten, NULL );
	WriteFile( hFile, HTML_TABLE_START, (DWORD)lstrlen(HTML_TABLE_START), &BytesWritten, NULL );


    pField = pSearchHit->GetRecordList();
    while( pField )
    {
         WriteFile( hFile, HTML_TABLE_ROW,  (DWORD)lstrlen(HTML_TABLE_ROW),  &BytesWritten, NULL );

         // -> print out the attribute's name
         WriteFile( hFile, HTML_TCELL_START, (DWORD)lstrlen(HTML_TCELL_START), &BytesWritten, NULL );
         WriteFile( hFile, HTML_BOLD_START,  (DWORD)lstrlen(HTML_BOLD_START),  &BytesWritten, NULL );
         if ( pField->lpszUserName != NULL ) {
              WriteFile( hFile, pField->lpszUserName, (DWORD)lstrlen(pField->lpszUserName), &BytesWritten, NULL );
              WriteFile( hFile, ": ", (DWORD)2, &BytesWritten, NULL );
         }
         else if (pField->nName != DS_UNKNOWN && pField->nName < DS_MAXFIELD) {
              szBuffer[0] = '\0';
              hErr = MapFieldName( CFactory::s_hModule, pField->nName, (LPSTR)szBuffer, 500 );
              WriteFile( hFile, szBuffer, (DWORD)lstrlen(szBuffer), &BytesWritten, NULL );
              WriteFile( hFile, ": ", (DWORD)2, &BytesWritten, NULL );
         }
         else {
              WriteFile( hFile, "Unknown: ", (DWORD)lstrlen("Unknown: "), &BytesWritten, NULL );
         }
         WriteFile( hFile, HTML_BOLD_END,  (DWORD)lstrlen(HTML_BOLD_END),  &BytesWritten, NULL );
         WriteFile( hFile, HTML_TCELL_END, (DWORD)lstrlen(HTML_TCELL_END), &BytesWritten, NULL );

         // -> print out the attribute's value
         WriteFile( hFile, HTML_TCELL_START, (DWORD)lstrlen(HTML_TCELL_START), &BytesWritten, NULL );
         switch( pField->nName ) {
	     case DS_EMAIL:					
     	 case DS_HEMAIL:
              // -> MAKE THE email LINK LIVE: <A HREF=mailto:mjoseph@qualcomm.com>mjoseph@qualcomm.com</A>
              WriteFile( hFile, HTML_MAILTO_START, (DWORD)lstrlen(HTML_MAILTO_START), &BytesWritten, NULL );
              WriteFile( hFile, pField->data,      (DWORD)pField->dwSize,             &BytesWritten, NULL );
              WriteFile( hFile, HTML_TAG_END,      (DWORD)lstrlen(HTML_TAG_END),      &BytesWritten, NULL );
              WriteFile( hFile, pField->data,      (DWORD)pField->dwSize,             &BytesWritten, NULL );
              WriteFile( hFile, HTML_LINK_END,     (DWORD)lstrlen(HTML_LINK_END),     &BytesWritten, NULL );
              break;

         default:
              if ( pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ )
                   WriteFile( hFile, pField->data, (DWORD)pField->dwSize, &BytesWritten, NULL );
              else WriteFile( hFile, "<Binary data removed>", (DWORD)lstrlen("<Binary data removed>"), &BytesWritten, NULL );
              break;
         }
         WriteFile( hFile, HTML_TCELL_END, (DWORD)lstrlen(HTML_TCELL_END), &BytesWritten, NULL );
         pField = pField->pNext;
    }


    // -> end the HTML file
	WriteFile( hFile, HTML_TABLE_END, (DWORD)lstrlen(HTML_TABLE_END), &BytesWritten, NULL );
	WriteFile( hFile, "<P>", (DWORD)lstrlen("<P>"), &BytesWritten, NULL );
	WriteFile( hFile, HTML_DOC_END,   (DWORD)lstrlen(HTML_DOC_END),   &BytesWritten, NULL );
    CloseHandle( hFile );
    return( S_OK );
}


// Each hit is identified by two identifiers: the database ID, and the record ID.
//
void WriteHitIDS( IDSPRecord*  pSearchHit,  // in:
                  HANDLE       hFile        // in:
                )
{
    LPSTR  szID = NULL;
    DWORD  BytesWritten;

    if (szID = pSearchHit->GetDatabaseID()) {
        WriteFile( hFile, "X-QUALCOMM-DBID:", (DWORD)lstrlen("X-QUALCOMM-DBID:"), &BytesWritten, NULL );
        WriteFile( hFile, szID,               (DWORD)lstrlen(szID),               &BytesWritten, NULL );
        WriteFile( hFile, "\r\n",             (DWORD)2,                           &BytesWritten, NULL );
    }
    if (szID = pSearchHit->GetRecordID()) {
        WriteFile( hFile, "X-QUALCOMM-RECID:", (DWORD)lstrlen("X-QUALCOMM-RECID:"), &BytesWritten, NULL );
        WriteFile( hFile, szID,                (DWORD)lstrlen(szID),                &BytesWritten, NULL );
        WriteFile( hFile, "\r\n",              (DWORD)2,                            &BytesWritten, NULL );
    }
}


// Given a search record hit and a file name, translate that record hit into simple plain text.
//
HRESULT __stdcall CDirServ::ExportToText( IDSPRecord*  pSearchHit,   // in: search results from a database query
														LPSTR        pszFileName,  // in: file to create and copy the generated text into
														BOOL         OverWrite     // in: append to the end or over write what is there ?
													 )
{
    char       szBuffer[500];
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;
    HANDLE     hFile;
    HRESULT    hErr;

    // -> append to the end or over write what is there ?
    if (pSearchHit == NULL || pszFileName == NULL) return( E_INVALIDARG );
	if ( OverWrite ) {
	     hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
         if (hFile == INVALID_HANDLE_VALUE) hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
	}
	else {
	     hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
         if (hFile != INVALID_HANDLE_VALUE) SetFilePointer( hFile, 0, NULL, FILE_END );
	}
    if (hFile == INVALID_HANDLE_VALUE) return( E_FAIL );


    // -> one line per attribute value pair
    pField = pSearchHit->GetRecordList();
    while( pField )
    {
         // -> write out the attribute name
         if ( pField->lpszUserName != NULL ) {
              WriteFile( hFile, pField->lpszUserName, (DWORD)lstrlen(pField->lpszUserName), &BytesWritten, NULL );
              WriteFile( hFile, ": ", (DWORD)2, &BytesWritten, NULL );
         }
         else if (pField->nName != DS_UNKNOWN && pField->nName < DS_MAXFIELD) {
              szBuffer[0] = '\0';
              hErr = MapFieldName( CFactory::s_hModule, pField->nName, (LPSTR)szBuffer, 500 );
              WriteFile( hFile, szBuffer, (DWORD)lstrlen(szBuffer), &BytesWritten, NULL );
              WriteFile( hFile, ": ", (DWORD)2, &BytesWritten, NULL );
         }
         else WriteFile( hFile, "Unknown: ", (DWORD)lstrlen("Unknown: "), &BytesWritten, NULL );

         // -> write out the attribute value
         if ( pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ )
              WriteFile( hFile, pField->data, (DWORD)pField->dwSize, &BytesWritten, NULL );
         else WriteFile( hFile, "<Binary data removed>", (DWORD)lstrlen("<Binary data removed>"), &BytesWritten, NULL );
         WriteFile( hFile, "\r\n", (DWORD)2, &BytesWritten, NULL );
         pField = pField->pNext;
    }
    // -> end the text file
    WriteHitIDS( pSearchHit, hFile );
    WriteFile( hFile, "\r\n", (DWORD)2, &BytesWritten, NULL );
    CloseHandle( hFile );
    return( S_OK );
}



// *******************************************************************************************
// vCard Version 2.1 Generation Functions
//
// *******************************************************************************************
//
//
// Problem: our built in DS_FIELD type does not distinguish between parts of a name and an entire name.
// So we must always write one vCard form for all encountered names.
//
void vCard_WriteNAME( IDSPRecord*  pSearchHit,  // in:
                      HANDLE       hFile        // in:
                    )
{
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;

    pField = pSearchHit->GetRecordList();
    while( pField )
    {
           if (pField->nName == DS_NAME && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ))
           {
                WriteFile( hFile, "FN:",        (DWORD)lstrlen("FN:"),  &BytesWritten, NULL );
                WriteFile( hFile, pField->data, (DWORD)pField->dwSize,  &BytesWritten, NULL );
                WriteFile( hFile, "\r\n",       (DWORD)2,               &BytesWritten, NULL );
           }
           pField = pField->pNext;
    }
}


// vCard properity format: "ORG:"Organization Name";"Organization Unit Name
//
void vCard_WriteORG( IDSPRecord*  pSearchHit,  // in:
                     HANDLE       hFile        // in:
                   )
{
    DBRECENT*  pField        = NULL;
    BOOL       company_found = FALSE;
    DWORD      BytesWritten;

    // -> this is a compound vCard property whose components may appear anywhere in our list of DBRECENTs
    pField = pSearchHit->GetRecordList();
    while( pField )
    {
           if (pField->nName == DS_COMPANY && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ))
           {
                WriteFile( hFile, "ORG:",       (DWORD)lstrlen("ORG:"), &BytesWritten, NULL );
                WriteFile( hFile, pField->data, (DWORD)pField->dwSize,  &BytesWritten, NULL );
                company_found = TRUE;
                break;
           }
           pField = pField->pNext;
    }

    pField = pSearchHit->GetRecordList();
    while( pField )
    {
           if (pField->nName == DS_DIVISION && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ))
           {
                if (!company_found) WriteFile( hFile, "ORG:", (DWORD)lstrlen("ORG:"), &BytesWritten, NULL );
                WriteFile( hFile, ";",          (DWORD)1,              &BytesWritten, NULL );
                WriteFile( hFile, pField->data, (DWORD)pField->dwSize, &BytesWritten, NULL );
                company_found = TRUE;
                break;
           }
           pField = pField->pNext;
    }
    if (company_found) WriteFile( hFile, "\r\n", 2, &BytesWritten, NULL );
}


//
//
void vCard_WriteTITLE( IDSPRecord*  pSearchHit,   // in:
                       HANDLE       hFile         // in:
                     )
{
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;

    pField = pSearchHit->GetRecordList();
    while( pField )
    {
           if (pField->nName == DS_POSITION && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ))
           {
                WriteFile( hFile, "TITLE:",     (DWORD)lstrlen("TITLE:"), &BytesWritten, NULL );
                WriteFile( hFile, pField->data, (DWORD)pField->dwSize,    &BytesWritten, NULL );
                WriteFile( hFile, "\r\n",       (DWORD)2,                 &BytesWritten, NULL );
           }
           pField = pField->pNext;
    }
}


//
//
void vCard_WriteEMAIL( IDSPRecord*  pSearchHit,  // in:
                       HANDLE       hFile        // in:
                     )
{
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;

    pField = pSearchHit->GetRecordList();
    while( pField )
    {
           if (   (pField->nName == DS_EMAIL  || pField->nName == DS_HEMAIL )
               && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ)
              )
           {
                WriteFile( hFile, "EMAIL:",     (DWORD)lstrlen("EMAIL:"), &BytesWritten, NULL );
                WriteFile( hFile, pField->data, (DWORD)pField->dwSize,    &BytesWritten, NULL );
                WriteFile( hFile, "\r\n",       (DWORD)2,                 &BytesWritten, NULL );
           }
           pField = pField->pNext;
    }
}


//
//
void vCard_WriteTEL( IDSPRecord*  pSearchHit,  // in:
                     HANDLE       hFile        // in:
                   )
{
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;

    pField = pSearchHit->GetRecordList();
    while( pField )
    {
        if ( pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ )
        {
             switch( pField->nName ) {
             case DS_PHONE:  WriteFile( hFile, "TEL;WORK:",       (DWORD)lstrlen("TEL;WORK:"),       &BytesWritten, NULL ); break;
             case DS_CELL:   WriteFile( hFile, "TEL;WORK;CELL:",  (DWORD)lstrlen("TEL;WORK;CELL:"),  &BytesWritten, NULL ); break;
             case DS_PAGER:  WriteFile( hFile, "TEL;WORK;PAGER:", (DWORD)lstrlen("TEL;WORK;PAGER:"), &BytesWritten, NULL ); break;
             case DS_FAX:    WriteFile( hFile, "TEL;WORK;FAX:",   (DWORD)lstrlen("TEL;WORK;FAX:"),   &BytesWritten, NULL ); break;
             case DS_HPHONE: WriteFile( hFile, "TEL;HOME:",       (DWORD)lstrlen("TEL;HOME:"),       &BytesWritten, NULL ); break;
             case DS_HCELL:  WriteFile( hFile, "TEL;HOME;CELL:",  (DWORD)lstrlen("TEL;HOME;CELL:"),  &BytesWritten, NULL ); break;
             case DS_HPAGER: WriteFile( hFile, "TEL;HOME;PAGER:", (DWORD)lstrlen("TEL;HOME;PAGER:"), &BytesWritten, NULL ); break;
             case DS_HFAX:   WriteFile( hFile, "TEL;HOME;FAX:",   (DWORD)lstrlen("TEL;HOME;FAX:"),   &BytesWritten, NULL ); break;
             default: { pField = pField->pNext; continue; }
             }
             WriteFile( hFile, pField->data, (DWORD)pField->dwSize,  &BytesWritten, NULL );
             WriteFile( hFile, "\r\n",       (DWORD)2,               &BytesWritten, NULL );
        }
        pField = pField->pNext;
    }
}


// ADR is a structured vCard property.
//
void vCard_WriteADR( IDSPRecord*  pSearchHit,    // in:
                     HANDLE       hFile,         // in:
                     UINT         num_fields,    // in: number of items in fields
                     DS_FIELD*    fields         // in: array of DS_FIELD values that make up an address
                   )
{
    DBRECENT*  pField      = NULL;
    BOOL       found_field = FALSE;
    DWORD      BytesWritten;

    // [A] Don't write out a properity unless we have at least one fields match
    for( UINT i=0; i < num_fields && found_field == FALSE; i++ )
    {
         pField = pSearchHit->GetRecordList();
         while( pField &&  fields[i] != DS_UNKNOWN)
         {
             if (pField->nName == fields[i] && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ )) {
                 found_field = TRUE;
                 break;
             }
             pField = pField->pNext;
         }
    }
    if (!found_field) return;


    // [B] If we don't match on any one field we must place a ";" in the property for that field.
    WriteFile( hFile, "ADR:;", (DWORD)lstrlen("ADR:;"), &BytesWritten, NULL );
    for( i=0; i < num_fields; i++ )
    {
         pField = pSearchHit->GetRecordList();
         while( pField &&  fields[i] != DS_UNKNOWN)
         {
             if (pField->nName == fields[i] && (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ )) {
                 WriteFile( hFile, pField->data, (DWORD)pField->dwSize,  &BytesWritten, NULL );
                 break;
             }
             pField = pField->pNext;
         }
         WriteFile( hFile, ";", 1, &BytesWritten, NULL );
    }
    WriteFile( hFile, "\r\n", 2, &BytesWritten, NULL );
}


// User defined field names can have spaces in them, yet vCard property names cannot.
//
BOOL vCard_XProperty( char*   szString,   // in:
                      HANDLE  hFile       // in:
                    )
{
     DWORD BytesWritten;
     char* szTemp = strchr( szString, ' ' );
     char* szCopy = 0;

     if ( szTemp != NULL )
     {
          // -> convert spaces into '-'
          if ((szCopy = DEBUG_NEW_NOTHROW char[lstrlen(szString)+1]) == 0) return( FALSE );
          lstrcpy( szCopy, szString );

          szTemp = strchr( szCopy, ' ' );
          *szTemp = '-';
          while( (szTemp = strchr( szTemp, ' ' )) ) *szTemp = '-';
          WriteFile( hFile, "X-",   (DWORD)lstrlen("X-"),   &BytesWritten, NULL );
          WriteFile( hFile, szCopy, (DWORD)lstrlen(szCopy), &BytesWritten, NULL );
     }
     else {
          WriteFile( hFile, "X-",     (DWORD)lstrlen("X-"),     &BytesWritten, NULL );
          WriteFile( hFile, szString, (DWORD)lstrlen(szString), &BytesWritten, NULL );
     }
     WriteFile( hFile, ":", (DWORD)lstrlen(":"), &BytesWritten, NULL );
     return( TRUE );
}


// vCard format allows "X-" (extension) properties.  We include this so no loss of data occurs.
//
void vCard_WriteUNKNOWN( IDSPRecord*  pSearchHit,  // in:
                         HANDLE       hFile        // in:
                       )
{
    DBRECENT*  pField = NULL;
    DWORD      BytesWritten;
    BOOL       bResult;

    pField = pSearchHit->GetRecordList();
    while( pField )
    {
           if (   (pField->nName == DS_UNKNOWN || pField->nName == DS_USERDEFINED )
               && (pField->nType == DST_ASCII  || pField->nType == DST_ASCIIZ     )
              )
           {
                if ( pField->lpszUserName ) {
                     bResult = vCard_XProperty( pField->lpszUserName, hFile );
                     if (!bResult) WriteFile( hFile, "X-Unknown:", (DWORD)lstrlen("X-Unknown:"), &BytesWritten, NULL );
                }
                else WriteFile( hFile, "X-Unknown:", (DWORD)lstrlen("X-Unknown:"), &BytesWritten, NULL );

                WriteFile( hFile, pField->data, (DWORD)pField->dwSize,    &BytesWritten, NULL );
                WriteFile( hFile, "\r\n",       (DWORD)2,                 &BytesWritten, NULL );
           }
           pField = pField->pNext;
    }
}


// Given a search record hit and a file name, translate that record hit into vCard V2.1 format.
//
HRESULT __stdcall CDirServ::ExportToVCard(IDSPRecord*  pSearchHit,   // in: search results from a database query
														LPSTR        pszFileName,  // in: file to create and copy the generated text into
														BOOL         OverWrite     // in: append to the end or over write what is there ?
													  )
{
    DS_FIELD   work_adr[5] = { DS_LOCATION, DS_ADDRESS,  DS_CITY,  DS_STATE,  DS_ZIPCODE  };
    DS_FIELD   home_adr[5] = { DS_UNKNOWN,  DS_HADDRESS, DS_HCITY, DS_HSTATE, DS_HZIPCODE };
    DWORD      BytesWritten;
    HANDLE     hFile;

    // -> append to the end or over write what is there ?
    if (pSearchHit == NULL || pszFileName == NULL) return( E_INVALIDARG );
	if ( OverWrite ) {
	     hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
         if (hFile == INVALID_HANDLE_VALUE) hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
	}
	else {
	     hFile = CreateFile( pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
         if (hFile != INVALID_HANDLE_VALUE) SetFilePointer( hFile, 0, NULL, FILE_END );
	}
    if (hFile == INVALID_HANDLE_VALUE) return( E_FAIL );


    // -> one vCard property can take several attributes to fill in
    WriteFile( hFile, "BEGIN:VCARD\r\n",  (DWORD)lstrlen("BEGIN:VCARD\r\n"),  &BytesWritten, NULL );
    WriteFile( hFile, "Version: 2.1\r\n", (DWORD)lstrlen("Version: 2.1\r\n"), &BytesWritten, NULL );
    vCard_WriteNAME(    pSearchHit, hFile );
    vCard_WriteORG(     pSearchHit, hFile );
    vCard_WriteTITLE(   pSearchHit, hFile );
    vCard_WriteEMAIL(   pSearchHit, hFile );
    vCard_WriteTEL(     pSearchHit, hFile );
    vCard_WriteADR(     pSearchHit, hFile, 5, work_adr );
    vCard_WriteADR(     pSearchHit, hFile, 5, home_adr );
    vCard_WriteUNKNOWN( pSearchHit, hFile );
    WriteHitIDS( pSearchHit, hFile );
    WriteFile( hFile, "END:VCARD\r\n",  (DWORD)lstrlen("END:VCARD\r\n"),  &BytesWritten, NULL );

    // -> end the text file
    WriteFile( hFile, "\r\n", (DWORD)2, &BytesWritten, NULL );
    CloseHandle( hFile );
    return( S_OK );
}

HRESULT __stdcall
CDirServ::ExportToBuffer(IDSPRecord* pSearchHit, PETBCB petbcb, void *pv)
{
    DBRECENT *pField = NULL;
    char      szBuffer[500];
    HRESULT hErr;

    pField = pSearchHit->GetRecordList();

    while (pField) {
	if (pField->lpszUserName != NULL) {
	    // Dump the line.
	    (*petbcb)(pField->lpszUserName, pv);
	    // Put in colon if necessary.
	    int nLen = strlen(pField->lpszUserName);
	    if ((pField->lpszUserName)[nLen - 1] != ':') {
		(*petbcb)(": ", pv);
	    }
	    else {
		(*petbcb)(" ", pv);
	    }
	}
	else if (pField->nName != DS_UNKNOWN && pField->nName < DS_MAXFIELD) {
	    szBuffer[0] = '\0';
	    hErr = MapFieldName(CFactory::s_hModule, pField->nName,
				(LPSTR)szBuffer, 500);
	    (*petbcb)(szBuffer, pv);
	    
	    int i = strlen(szBuffer);
	    if (i >= 2) {
		i = strcmp(szBuffer + i - 2, ": ");
		if (i != 0) {
		    (*petbcb)(": ", pv);
		}
	    }
	}
	else {
	    (*petbcb)("Unknown: ", pv);
	}

	if (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ) {
	    (*petbcb)((char *)(pField->data), pv);
	}
	else {
	    (*petbcb)("<Binary data removed>", pv);
	}
	(*petbcb)("\r\n", pv);
	pField = pField->pNext;
    }
    return(S_OK);
}

void
ComputeColonPosition(char *s, int& pos)
{
    ASSERT(s != NULL);
    if (s) {
	int nLen = strlen(s);
	if (nLen > 0) {
	    if (s[nLen - 1] == ':') { // colon at end of string
		nLen--;
	    }
	    pos  = (nLen > pos) ? nLen : pos;
	}
    }
}

int
ComputeFieldSize(char *s, int pos)
{
    int retVal = 0;
    ASSERT(s != NULL);
    if (s) {
	int nLen = strlen(s);
	if (nLen > 0) {
	    int lastCharPos = nLen - 1;
	    int padding;
	    if (s[lastCharPos] == ':') { // colon at end of string.
		nLen--;
	    }
	    padding = (nLen < pos) ? (pos - nLen) : 0;
	    retVal = nLen + padding + 2;
	}
    }
		    
    return(retVal);
}

int
FillBuffer(char *source, char *destination, int pos)
{
    int retVal = 0;
    ASSERT(source != NULL);
    ASSERT(destination != NULL);
    if (source) {
	int nLen = strlen(source);
	if (nLen > 0) {
	    bool putColon = true;
	    int lastCharPos = nLen - 1;
	    int padding     = 0;
	    if (source[lastCharPos] == ':') { // colon at end of string.
		nLen--;
		putColon = false;
	    }
	    padding = (nLen < pos) ? (pos - nLen) : 0;
	    if (padding > 0) {
		memset((void *)destination, ' ', padding);    // padding
	    }
	    memcpy(destination + padding, source,
		   (putColon) ? nLen : nLen + 1);
	    if (putColon) { // nLen is pristine for this case.
		destination[padding + nLen] = ':';
		destination[padding + nLen + 1] = ' ';
	    }
	    else { // nLen has been decremented by 1.
		destination[padding + nLen + 1] = ' ';
	    }
	    retVal = nLen + padding + 2;
	}
    }
    return(retVal);
}

char *_divider_ds =
"------------------------------------------------------------\r\n";

HRESULT __stdcall
CDirServ::ExportToBufferFormatted(void *data, PETBCB petbcb, void *pv)
{
    typedef struct _hit_list {
	IDSPRecord *pRecord;
	_hit_list  *next;
    } HitList, *P_HitList;

    P_HitList pHitList = static_cast<P_HitList>(data);
    P_HitList pCopy;

    DBRECENT *pField = NULL;
    char      szBuffer[500];
    long      nBytesToWrite  = 0, bytesWritten = 0, nLen;
    int       position = 0, pass = 0, dividerLen = strlen(_divider_ds);
    char     *binData = "<Binary data removed>";
    char     *unknown = "Unknown:";
    char     *finUnknown = ":";
    char     *allData = NULL;
    HRESULT hErr;

    // Pass 1: loop through all records and find colon position.
    pCopy = pHitList;
    while(pCopy) {
	pField = pCopy->pRecord->GetRecordList();
	while (pField) {
	    if (pField->lpszUserName != NULL) { // no string mapping.
		ComputeColonPosition(pField->lpszUserName, position);
	    }
	    else if (pField->nName != DS_UNKNOWN &&
		     pField->nName < DS_MAXFIELD) { // string mapping.
		szBuffer[0] = '\0';
		hErr = MapFieldName(CFactory::s_hModule, pField->nName,
				    (LPSTR)szBuffer, 500);
		ComputeColonPosition(szBuffer, position);
	    }
	    else { // unknown.
		char *szInput = unknown;
		char *szDatabaseID = pCopy->pRecord->GetDatabaseID();
		if (szDatabaseID) {
		    IDSDatabase *pDatabase = FindDatabase(szDatabaseID);
		    if (pDatabase) {
		        char szBuf[128] = { '\0' };
		        HRESULT hErr = pDatabase->GetProtocolName(szBuf, 128);
			if (SUCCEEDED(hErr)) {
			    int cmp = strcmp(szBuf, "Finger");
			    if (cmp == 0) {
				szInput = finUnknown;
			    }
			}
		    }
		}
		ComputeColonPosition(szInput, position);
	    }
	    pField = pField->pNext;
	}
	pCopy = pCopy->next;
    }

    // Pass 2: allocate memory.
    pCopy = pHitList;
    while (pCopy) {
	if (pass != 0) {
	    nBytesToWrite += dividerLen;
	}
	pField = pCopy->pRecord->GetRecordList();
	while (pField) {

	    if (pField->lpszUserName != NULL) { // no string mapping.
		nBytesToWrite += ComputeFieldSize(pField->lpszUserName,
						  position);
	    }
	    else if (pField->nName != DS_UNKNOWN &&
		     pField->nName < DS_MAXFIELD) { // string mapping.
		szBuffer[0] = '\0';
		hErr = MapFieldName(CFactory::s_hModule, pField->nName,
				    (LPSTR)szBuffer, 500);
		nBytesToWrite += ComputeFieldSize(szBuffer, position);
	    }
	    else { // unknown.
		char *szInput = unknown;
		char *szDatabaseID = pCopy->pRecord->GetDatabaseID();
		if (szDatabaseID) {
		    IDSDatabase *pDatabase = FindDatabase(szDatabaseID);
		    if (pDatabase) {
		        char szBuf[128] = { '\0' };
		        HRESULT hErr = pDatabase->GetProtocolName(szBuf, 128);
			if (SUCCEEDED(hErr)) {
			    int cmp = strcmp(szBuf, "Finger");
			    if (cmp == 0) {
				szInput = finUnknown;
			    }
			}
		    }
		}
		nBytesToWrite += ComputeFieldSize(szInput, position);
	    }

	    if (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ) {
		nBytesToWrite += strlen((char *)(pField->data));
	    }
	    else {
		nBytesToWrite += strlen(binData);
	    }
	    nBytesToWrite += 2; // \r\n
	    pField = pField->pNext;
	}
	pCopy = pCopy->next;
	pass++;
    }
	
    nBytesToWrite++; // the null terminator.

    allData = DEBUG_NEW_NOTHROW char[nBytesToWrite];

    if (!allData) {
		return(E_FAIL);
    }

    // Pass 3: Fill buffer and call callback.
    pass = 0;
    pCopy = pHitList;
    while (pCopy) {
	if (pass != 0) {
	    memcpy(allData + bytesWritten, _divider_ds, dividerLen);
	    bytesWritten += dividerLen;
	}
	pField = pCopy->pRecord->GetRecordList();
	while (pField) {
	    if (pField->lpszUserName != NULL) { // no string mapping.
		bytesWritten += FillBuffer(pField->lpszUserName,
					   allData + bytesWritten, position);
	    }
	    else if (pField->nName != DS_UNKNOWN &&
		     pField->nName < DS_MAXFIELD) { // string mapping.
		szBuffer[0] = '\0';
		hErr = MapFieldName(CFactory::s_hModule, pField->nName,
				    (LPSTR)szBuffer, 500);
		bytesWritten += FillBuffer(szBuffer, allData + bytesWritten,
					   position);
	    }
	    else { // unknown.
		char *szInput = unknown;
		char *szDatabaseID = pCopy->pRecord->GetDatabaseID();
		if (szDatabaseID) {
		    IDSDatabase *pDatabase = FindDatabase(szDatabaseID);
		    if (pDatabase) {
		        char szBuf[128] = { '\0' };
		        HRESULT hErr = pDatabase->GetProtocolName(szBuf, 128);
			if (SUCCEEDED(hErr)) {
			    int cmp = strcmp(szBuf, "Finger");
			    if (cmp == 0) {
				szInput = finUnknown;
			    }
			}
		    }
		}
		bytesWritten += FillBuffer(szInput, allData + bytesWritten,
					   position);
	    }

	    if (pField->nType == DST_ASCII || pField->nType == DST_ASCIIZ) {
		nLen = strlen((char *)(pField->data));
		memcpy(allData + bytesWritten, pField->data, nLen);
		bytesWritten += nLen;
	    }
	    else {
		nLen = strlen(binData);
		memcpy(allData + bytesWritten, binData, nLen);
		bytesWritten += nLen;
	    }
	    memcpy(allData + bytesWritten, "\r\n", 2);
	    bytesWritten += 2;
	    pField = pField->pNext;
	}
	pCopy = pCopy->next;
	pass++;
    }    

    allData[bytesWritten] = '\0';
    bytesWritten++;
    ASSERT(bytesWritten == nBytesToWrite);
    (*petbcb)(allData, pv);
    delete [] allData;
    return(S_OK);
}


