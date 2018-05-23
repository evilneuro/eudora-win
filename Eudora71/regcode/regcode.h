/* ======================================================================
    Regcode generation and verification
    Authors: Laurence Lundblade and Steve Dorner
    Original date: March 7, 1999
    Copyright 1999 QUALCOMM Inc.
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



    The reg codes generated are 14 characters in the range of 0-9AB. This
    is basically an ASCII encoding of 50 bits. The following components
    go into the reg code:
      Name - The registeree's name. This should be folded to lower case,
             and be only US-ASCII in the range of space to z. Folding
             accented characters in other character sets is left to the
             caller because it is character set and platform variable.
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
    those in 0-9,A,B,a,b are ignored. I, O are also mapped to 1, 0. Because
    of the random component of the reg code, it is possible for a slightly
    mis-typed reg code to verify.

    Two bits in the reg code are allocated for scheme version numbering 
    in case we have to change something about the way the codes work. 

    Character canonicalization
    --------------------------
    Ultimately the only characters of a name that are used are
    a-z and 0-9. Getting a name reduced to that happens in two stages.
    1) The caller of this code folds the local character into US-ASCII.
    2) The encode and verify functions here do the rest of the work.
    
    For example, with ISO-8859-1 and very similarly Windows CP 1252 the
    caller of this code would fold the accented alphabetic characters 
    into US-ASCII characters (e.g., accented a goes to a). The non-alpha
    characters can be left as is for regcode_xx functions to ignore or
    they can be removed.

    The actual character mappings to US-ASCII are of course dependent
    on the character set. They should also be agreed upon and documented
    here or nearby so registration works properly for all implementations
    with a particular character set. As of April '99 we don't have this
    locked down.

   
    Changes: March 8; fixed document return code from regcode_verify to
             be 1 on failure.
     
             March 15 - lgl - updated comment about I and O

             March 16 - lgl - updated comment about A,B,a,b

	     March 24 - lgl - added regcode_canonicalize entry point

             March 29 - lgl - regcode_encode return value
         
             April 5  - lgl - added define for max name chars, and comment
                              about character canonicalization

   ====================================================================== */

/* Length of fully formated ASCII regcode + terminating \0 */
#define REG_CODE_LEN (17)

/* This is reg code scheme version 0 */
#define REG_CODE_SCHEME_VERSION (0)

/* First and last name should be truncated to this length 
   before character canonicalization and passing to either
   the verifier or encoder. This is basically the max number
   of chars you have to accept in user input                */
#define REG_CODE_MAX_NAME (20)

/* ----------------------------------------------------------------------
     Reformat regcode so it is in canonical form:
       - insert '-' in proper place
       - strip all non-significant chars
       - convert to upper case
       - convert I and O to 1 and 0
       - limit chars copied to REG_CODE_LEN
       
     Args: szRegCode - input code to canonicalize
           szCanonCode - place to output code, must be REG_CODE_LEN big
     
     Returns: 0 if regcode is correct length, 1 if it is not    
   --- */
int regcode_canonicalize(const char *szRegcode, char *szCanonCode);


/* ----------------------------------------------------------------------
    Generate a reg code

    Args: szName   - Name of person registering. Chars should US-ASCII, lower
                     case only
          nMonth   - Month number with Jan 1999 being 0
          nProduct - Product number
          nRandom  - Uses lower 6-bits of random number to permute gen'd code
          szResult - Place to put generated ASCII reg code; must be 
                     REG_CODE_LEN long

    Returns: 0 on success, 1 if user name had no valid chars (no alpha or
             numeric)
   ---- */
int regcode_encode(
    const char *szName,
    int         nMonth,
    int         nProduct,
    int         nRandom,
    char       *szResult);


/* ----------------------------------------------------------------------
    Validate a reg code
    Args: szCode    - The reg code - \0 terminated
          szName    - Name of person. US-ASCII, lower case, end in \0
          pnMonth   - Place to return month number               (not NULL)
          pnProduct - Place to return product number code is for (not NULL)
    Returns: 0 on successful verification, 1 on unsuccesful
   --- */
int regcode_verify(
  const char *szCode,
  const char *szName,
  int        *pnMonth,
  int        *pnProduct);


