/* ======================================================================
    Regcode generation and verification
    Authors: Laurence Lundblade and Steve Dorner
    Original date: March 7, 1999
    Copyright 1999, 2000 QUALCOMM Inc.
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


    
    PLEASE Readme.txt for a full explanation. It contains fairly critical 
    details needed for using this code.

    The reg codes generated are 16 digits. This is basically an ASCII
    encoding of 52 bits. We use digits only so they can be entered
    on a touch tone phone for a tech support system The following
    components go into the reg code:
      Name - The registeree's name. This should be folded to lower case,
             and be only US-ASCII in the range of space to z. Folding
             accented characters in other character sets should be done
             with regcode_charsets.[ch].
      Month - Basically an 8 bit quantity that is recovered when the
             code is being verified. The convention is to number months
             with Jan 1, 1999 being 0. This fails in March of 2021.
      Product - Basically an 8 bit quantity that is recovered when the
             code is being verified. Products number wil be assigned to
             different products and versions of different products.
      Random - A random number for the sake of obfuscation. This means
             that two reg codes for the same person, month and product
             will not necessarily be the same.

    When verifying the registration code, the name must be supplied, and
    the product number and month number are returned, along with the 
    result of the verification. Any characters in the reg code other than
    those in 0-9, are ignored. I, O are also mapped to 1, 0. Because
    of the random component of the reg code, it is possible for a slightly
    mis-typed reg code to verify.

    Two bits in the reg code are allocated for scheme version numbering 
    in case we have to change something about the way the codes work.

    Note that this is version 2 of the reg code scheme. v1 used a shorter
    regcode of 14 characters and included A and B. In switching to numeric
    we switched to 16 digits. That gave us 2 additional bits (actually 3,
    but we didn't use the 3rd because it rearragne the implementation
    substantially). The two extra bits are used for more internal CRC.
    In addition v2 has a better bit swapping obfuscation.

    Character canonicalization
    --------------------------
    Please use regcode_charset_v3 to convert the characters to
    a cannonical form before passing in here. Details are in 
    regcode_charsets and the readme.

    Compilation controls
    --------------------
    By default the internal functions are all inlined to make the object
    code more messy and thus harder to reverse engineer. If your compiler
    doesn't support inlining or you care about size you can turn it off
    by with 
             #define NO_REGCODE_INLINING

    Changes
    -------
             March 8; fixed document return code from regcode_verify to
             be 1 on failure.
     
             March 15 - lgl - updated comment about I and O

             March 16 - lgl - updated comment about A,B,a,b

	     March 24 - lgl - added regcode_canonicalize entry point

             March 29 - lgl - regcode_encode return value
         
             April 5  - lgl - added define for max name chars, and comment
                              about character canonicalization
			     
             Nov 3 1999 - lgl - Switch to v2 scheme
	     
             Apr 24 2000 - lgl - Switch to v3. Not a change to the scheme
                               itself, just in the way char sets are 
                               handled. Use the v3 char sets and
                               regcode stuff together. Don't mix'n'match.

   ====================================================================== */

#ifndef ___REGCODE_V3_INCLUDED
#define ___REGCODE_V3_INCLUDED

/* Length of fully formated ASCII regcode + terminating \0 */
#define REG_CODE_LEN (20)

/* This is reg code scheme version 0 */
#define REG_CODE_SCHEME_VERSION (0)

/* NOTE that this length has NOT been adhered to in most implementations
   and thus should probably not be used in the future. */
/* First and last name should be truncated to this length 
   before character canonicalization and passing to either
   the verifier or encoder. This is basically the max number
   of chars you have to accept in user input                */
#define REG_CODE_MAX_NAME (20)

/* Different types of reg code returned by canonicalizer */
typedef enum {REG_CODE_V1, REG_CODE_V2, REG_CODE_BAD} RegCodeVEnum;

/* ----------------------------------------------------------------------
     Reformat regcode so it is in canonical form:
       - insert '-' in proper place
       - strip all non-significant chars
       - convert to upper case
       - convert I and O to 1 and 0
       - limit chars copied to REG_CODE_LEN
       
     Args: szRegCode - input code to canonicalize
           szCanonCode - place to output code, must be REG_CODE_LEN big
     
     Returns: REG_CODE_V1, REG_CODE_V2, REG_CODE_BAD
   --- */
RegCodeVEnum regcode_v3_canonicalize(const char *szRegcode, char *szCanonCode);


/* ----------------------------------------------------------------------
    Generate a reg code

    Args: szName   - Name of person registering. '\0' terminated string.
                     Should be processed by regcode_charsets_v3 first.
          nMonth   - Month number with Jan 1999 being 0
          nProduct - Product number
          nRandom  - Uses lower 6-bits of random number to permute gen'd code
          szResult - Place to put generated ASCII reg code; must be 
                     REG_CODE_LEN long

    Returns: 0 on success, 1 if user name had no valid chars (no alpha or
             numeric)
   ---- */
int regcode_v3_encode(
    const char *szName,
    int         nMonth,
    int         nProduct,
    int         nRandom,
    char       *szResult);


/* ----------------------------------------------------------------------
    Validate a reg code
    Args: szCode    - The reg code - \0 terminated
          szName    - Name of person. '\0' terminated strings. Should be
                      processed by regcode_charsets_v3 first.
          pnMonth   - Place to return month number               (not NULL)
          pnProduct - Place to return product number code is for (not NULL)
    Returns: 0 on successful verification, 1 on unsuccesful
   --- */
int regcode_v3_verify(
  const char *szCode,
  const char *szName,
  int        *pnMonth,
  int        *pnProduct);


#endif /* ___REGCODE_V3_INCLUDED */
