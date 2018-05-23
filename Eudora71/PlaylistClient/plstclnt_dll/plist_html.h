#ifndef __CONVHTML_H__
#define __CONVHTML_H__

/*==================================================================*/
/* CONVHTML.H - HTML Conversion Routine                             */
/* ------------------------------------                             */
/* Version 1.0 Rev 7/14/97                                          */
/* Author V. Leamons for Qualcomm, Inc. All Rights Reserved.        */
/*                                                                  */
/* Description:                                                     */
/*    This file contains the ConvertURIs() routine to be included in */
/* Eudora 4.0.                                                      */
/*                                                                  */
/* History (all by V. Leamons unless otherwise marked):             */
/* 7/14/97  Created.                                                */
/*==================================================================*/

typedef int (* ParseURLCallback)(char* in, long userRef, char* out, int maxOut);

// *** Prototypes ***
int ParseURLs(char * pszIn, ParseURLCallback pFn, long userRef, char** ppszOut );
int MungeURLs( char* pszIn, char* base, char** ppszOut );
int ResolveURL( char* inURL, char* base, char* outURL, int maxURL );


#endif __CONVHTML_H__

