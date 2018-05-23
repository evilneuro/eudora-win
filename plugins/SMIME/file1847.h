/*
 *  Filename: file1847.h
 *
 *  Last Edited: Wednesday, October 2, 1996
 *
 *  Author: Scott Manjourides
 *
 *  Copyright 1995, 1996 QUALCOMM Inc.
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


 *
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#ifndef EMS_FILE1847_H
#define EMS_FILE1847_H

#include <windows.h> /* Must be included for ems-win.h */
#include "ems-win.h"

#include "encoding.h"
#include "rfc1847.h"

enum { FILE1847_FAIL = (-1), FILE1847_ABORT = (0), FILE1847_OK = (1) };

/* ------------------------------------------------------------------------- */

/*  CreateTmpFile
 *
 *  Creates a empty, unique, temporary file in the users 'TEMP'
 *    directory.
 *
 *  No arguments.
 *
 *  Returns a pointer to the filename string. This should be
 *    considered temporary data, and should never be free()'d.
 */
char *CreateTmpFile();

/* ------------------------------------------------------------------------ */

/*  rfc1847_file_create
 *
 *  Creates RFC1847 MIME structure.
 *
 *  Args:
 *    pOutFilename     [IN]     Output filename ** WRITE-ONLY **
 *    OutMimePtr       [IN/OUT] MIME type of output -- boundary param will be added
 *    InPart1MimePtr   [IN]     MIME type of part 1
 *    InPart1CTE       [IN]     Content-transfer-encoding for part 1
 *    pInPart1Filename [IN]     Input filename for part 1 ** READ-ONLY **
 *    InPart2MimePtr   [IN]     MIME type of part 2
 *    InPart2CTE       [IN]     Content-transfer-encoding for part 2
 *    pInPart2Filename [IN]     Input filename for part 2 ** READ-ONLY **
 *    progress         [IN]     EMS progress callback function
 *
 *  Returns boolean. TRUE for success, FALSE for failure.
 */
int rfc1847_file_create(const char *pOutFilename,
				   emsMIMEtypeP OutMimePtr,

				   emsMIMEtypeP InPart1MimePtr,
				   TrEncType InPart1CTE,
				   const char *pInPart1Filename,

				   emsMIMEtypeP InPart2MimePtr,
				   TrEncType InPart2CTE,
				   const char *pInPart2Filename,

				   emsProgress progress);

/* ------------------------------------------------------------------------ */

/*  rfc1847_file_parse
 *
 *  Parses RFC1847 MIME structure.
 *
 *  Args:
 *    pInFilename       [IN]  Input filename of file to parse ** READ-ONLY **
 *    InMimePtr         [OUT] MIME type of input file
 *    pOutPart1Filename [IN]  Output filename for part 1 ** WRITE-ONLY **
 *    OutPart1MimePtr   [OUT] MIME type of created part 1
 *    bPart1DeMime      [IN]  Boolean: Remove header and transfer encoding, part 1?
 *    pOutPart2Filename [IN]  Output filename for part 2 ** WRITE-ONLY **
 *    OutPart2MimePtr   [OUT] MIME type of created part 2
 *    bPart2DeMime      [IN]  Boolean: Remove header and transfer encoding, part 2?
 *    progress          [IN]  EMS progress callback function
 *
 *  Returns boolean. TRUE for success, FALSE for failure.
 */
int rfc1847_file_parse(const char *pInFilename,
				  emsMIMEtypeP *InMimePtr,

				  const char *pOutPart1Filename,
				  emsMIMEtypeP *OutPart1MimePtr,
				  int bPart1DeMime,

				  const char *pOutPart2Filename,
				  emsMIMEtypeP *OutPart2MimePtr,
				  int bPart2DeMime,
				  
				  emsProgress progress);

#endif /* EMS_FILE1847_H */