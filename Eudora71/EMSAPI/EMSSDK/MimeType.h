/*
 *  Functions to manage emsMIMEtype structures
 *  for use with Eudora EMS API under MS Windows.
 *
 *  Filename: MimeType.h
 *  Original Author: Scott Manjourides
 *
 *  Copyright 1995-2003 QUALCOMM, Incorporated
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
 *  Send comments and questions to <eudora-emsapi@qualcomm.com>
 */

#ifndef EMS_MIMETYPE_H
#define EMS_MIMETYPE_H

#include <windows.h>
#include "emsapi-win.h"

/*
 *  Create an emsMIMEtype structure to hold MIME information. Only the
 *  type and subtype are set here, use add_mime_parameter() to add
 *  any name/value parameter pairs.
 *
 *  NOTE: All input strings are COPIED before permanent use. The user
 *        of this function is responsible for calling free_mime_type()
 *        on returned structure.
 *
 *  Args:
 *   mime_type    [IN] the main MIME type: e.g., "text", "application", "image"
 *   sub_type     [IN] the sub type: e.g., "plain", "octet-stream", "jpeg"
 *   mime_version [IN] the MIME verion number, if NULL will default to "1.0"
 *
 *  Returns: Pointer to the created emsMIMEtype structure, NULL if error.
 */
emsMIMEtypeP make_mime_type(const char *mime_type, const char *sub_type, const char *mime_version);

/*
 *  Create an emsMIMEtype structure to hold MIME information. Structure is
 *  initialized to values provided in RFC822 content-type header line. This
 *  includes all parameter name-value pairs.
 *   
 *  NOTE: The user of this function is responsible for calling
 *        free_mime_type() on returned structure.
 *
 *  Args:
 *   content_type [IN] The RFC822 content-type string to parse
 *
 *  Returns: Pointer to the created emsMIMEtype structure, NULL if error.
 */
emsMIMEtypeP parse_make_mime_type(const char *src);

/*
 *  Add a parameter to an existing emsMIMEtype structure. This structure
 *  should be created using make_mime_type().
 *
 *  NOTE: All input strings are COPIED before permanent use.
 *
 * Args:
 *   mimePtr [IN] Pointer to the emsMIMEtype structure to be added too
 *   name    [IN] Name of the parameter
 *   value   [IN] Value of the parameter
 *   
 *  Returns: Boolean (TRUE = success, FALSE = failure)
 */
int add_mime_parameter(emsMIMEtypeP mimePtr, const char *name, const char *value);

/*
 *  Remove a parameter from an existing emsMIMEtype structure. This structure
 *  should be created using make_mime_type().
 *
 *  NOTE: All input strings are COPIED before permanent use.
 *
 * Args:
 *   mimePtr [IN] Pointer to the emsMIMEtype structure to altered
 *   name    [IN] Name of the parameter to be removed
 *   
 *  Returns: Boolean (TRUE = success, FALSE = failure)
 */
int remove_mime_parameter(emsMIMEtypeP mimePtr, const char *name);

/*
 *  Free an emsMIMEtype structure, including all strings and parameters.
 *
 *  Args:
 *   mimePtr [IN] Pointer to the emsMIMEtype structure to be freed
 *
 *  No return value.
 */
void free_mime_type(emsMIMEtypeP mimePtr);

/*
 *  Convert an emsMIMEtype structure to a Content-Type header field line
 *  in the format specified by RFC 882.
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   mimePtr [IN] Pointer to the emsMIMEtype structure
 *
 *  Returns: String containing header field; dynamically allocated.
 */
char *string_mime_type(emsMIMEtypeP mimePtr);

/*
 *  Pick out a specific parameter name-value pair from an emsMIMEtype
 *  structure. The match is case sensitive.
 *
 *  NOTE: The user of this function should NOT alter the returned string
 *        in any way -- it should be considered READ-ONLY.
 *
 *  Args:
 *   mimePtr   [IN] Pointer to the emsMIMEtype structure
 *   paramName [IN] Name of parameter to look for
 *
 *  Returns: String of the associated value or NULL if no match is found
 */
const char *get_mime_parameter(emsMIMEtypeP mimePtr, const char *paramName);

/*
 *  Check for a matching MIME type/subtype. If either type or subtype is
 *  NULL, then it won't be checked. If both are NULL, then TRUE is always
 *  returned.
 *
 *  Args:
 *   mimePtr   [IN] Pointer to the emsMIMEtype structure
 *   mime_type [IN] The major MIME type to check
 *   sub_type  [IN] The MIME subtype to check
 *  
 *  Returns: Boolean (TRUE if the MIME type matches, FALSE if not)
 */
int match_mime_type(emsMIMEtypeP mimePtr, const char *mime_type, const char *sub_type);

#endif /* EMS_MIMETYPE_H */
