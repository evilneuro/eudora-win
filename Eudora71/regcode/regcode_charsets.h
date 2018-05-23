/* ======================================================================

    Regcode character set folding
    Author: Laurence Lundblade <lgl@qualcomm.com>
    Original Date: Feb 29, 2000
    Copyright 2000 QUALCOMM Inc
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


 
    Please see readme.txt for complete details on using this code before
    using it.   

    This replaces MapString as the way to fold non-us characters for
    Eudora reg codes.

    The theory is to map all non-USASCII characters to something similar
    in US-ASCII. Thus an accented e becomes a plain e. This is important
    because regcodes are platform and character set independent. A user
    may get a regcode using MS Windows Polish and then want to use it
    in a Macintosh in the US. Or they might register on a web page and
    in windows 1252 and then try and enter it in Czech Windows. Unless
    the canonicalization is done correctly these cases will fail.
   
    The regcode_map_* funtions:
    Args: szIn - is a nil terminated string to be mapped. This may be
                 NULL or an empty string
          
          szOut - is a buffer at least as long as szIn into which the
                  mapped string is placed. This may NOT be NULL unless
                  szIn is NULL.

    Returns: length of the output string which may be 0.

    ====================================================================== */

/* Convenience definition of the map functions below */
typedef int (regcode_map_type)(const char *, char *);



int regcode_map_iso_8859_1(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_2(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_3(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_4(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_9(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_10(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_13(
    const char *szIn,
    char       *szOut);

int regcode_map_iso_8859_14(
    const char *szIn,
    char       *szOut);

int regcode_map_win_cp1252(
    const char *szIn,
    char       *szOut);

int regcode_map_palmos(
    const char *szIn,
    char       *szOut);

int regcode_map_macroman(
    const char *szIn,
    char       *szOut);
