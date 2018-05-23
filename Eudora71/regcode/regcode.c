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



    Changes: March 15 - lgl - on verification treat I, O as 1, 0
             
             March 16 - lgl - circulate bits based on random component
             for more obfuscation, and so there is more interdependency
             internally so a smaller change will cause the reg code to 
             be invalid

             March 16 - lgl - Runs on PalmOS/68K/16bit. Fixed a number of
             bugs with casts and "L" on constants.
 
             March 16 - lgl - Allow lower case in reg codes
       
             March 21 - lgl - In user name, only alpha and numeric chars
             are considered. All others are completely ignored.

	     March 24 - lgl - Added regcode_canonicalization entry point
             and modified verification to use the entry point. 

             March 24 - lgl - Fixed a couple of casts and type to pass
	     gcc's most pedantic compiler options
 
             March 29 - lgl - regcode_encode returns 1 now when the
             user name given has no alpha numeric chars

   ====================================================================== */

#include "regcode.h"

/* Some randomly chosen quantities we xor with for obfuscation */
#define XOR_NAME (0x35)
#define XOR_1 (0x01C5E73A)
#define XOR_2 (0x01F6433E)


/* ----------------------------------------------------------------------
    public function - see regcode.h
   --- */
int regcode_canonicalize(const char *szRegcode, char *szCanonCode)
{
  const char *s;
  char       *d;
  int         nCharCount;
  int         nReturn;
 
  nReturn = 0;
 	
  for(s = szRegcode, d = szCanonCode, nCharCount = 0; *s; s++) {
  	/* Copy the valid chars with some translation */
    if((*s >= '0' && *s <= '9') || 
       (*s >= 'a' && *s <= 'b') ||
       (*s >= 'A' && *s <= 'B')) {
      *d = *s;
      if(*d >= 'a')
        *d &= ~0x20;
      d++;
      nCharCount++;
    } else if(*s == 'O') {
      *d++ = '0';
      nCharCount++;
    } else if(*s == 'I') {
      *d++ = '1';
      nCharCount++;
    }
    
    /* Insert '-' in the correct place */
    if(nCharCount == 5 || nCharCount == 10) {
	    nCharCount++;
      *d++ = '-';
    }
    
    /* Abort if given input has too many valid char */
    if(nCharCount > REG_CODE_LEN - 1) {
      nReturn = 1;
      break;
    }
  }  
  *d = '\0';
  
  if(nCharCount != REG_CODE_LEN - 1)
    nReturn = 1;

  return(nReturn);
}


/* ----------------------------------------------------------------------
   MD4 hash
   Args: szString - \0 terminated string to hash
         nXor     - byte to XOR each character with before hashing
         pbValid  - var to set to 0 if OK, 1 if not
   Returns: hash

   The following hashing algorithm, KRHash, is derived from Karp & Rabin,
   Harvard Center for Research in Computing Technology Tech. Report TR-31-81
   The constant prime number in use is kKRHashPrime.  It happens to be the
   largest prime number that will fit in 31 bits, except for 2^31-1 itself...

   We add the xor to cheaply make this something other than a straight
   md4 hash
   
   This is also were we ignore non-alphanumeric chars. If there are none 
   (no input to hash) we return 1 in pbValid.
 
   --- */
static unsigned long XORMD4Hash(const char* szString, int nXor, int *pbValid)
{
  const unsigned long kKRHashPrime = 2147483629;
	
  unsigned long ulSum = 0;        /* returned */

  *pbValid = 1;

  for(; *szString; szString++) {
    int      nBit;
    unsigned nChar;
   
    /* Only pay attention to alpha and numeric chars */
    nChar = (unsigned char )*szString;
    if(nChar < '0')
      continue;
    nChar |= 0x20;
    if(nChar > '9' && nChar < 'a')
      continue;
    if(nChar > 'z')
      continue;

    *pbValid = 0;
 
    for(nBit = 0x80; nBit != 0; nBit >>= 1) {
      ulSum += ulSum;
      if (ulSum >= kKRHashPrime)
          ulSum -= kKRHashPrime;
      if ((nChar ^ nXor) & nBit)
          ++ulSum;
      if (ulSum>= kKRHashPrime)
          ulSum -= kKRHashPrime;
    }
  }

  return (ulSum + 1);
}


/* ----------------------------------------------------------------------
   Base 12 encode 25 bits of a long, placing result in dest. Dest must
   point to at least 7 bytes of storage. Note the ASCII representation is
   'B':0, 'A':1, '9':2, '8':3,... '0':11, 
   --- */
static void b12encode(char *dest, unsigned long uValue)
{
  unsigned long uMult;
  unsigned int c;
  unsigned int i;
  unsigned int n;

  uMult = 12L * 12L * 12L * 12L * 12L * 12L;
  for(i = 0; i < 7; i++) {
    n = uValue / uMult;
    c = (11 - n) + '0';
    if(c > '9')
      c = c + 'A' - '0' - 10;
    dest[i] = (char )c;
    uValue -= n * uMult;
    uMult = uMult / 12;
  }
}


/* ----------------------------------------------------------------------
   Base 12 decode string. Expected to be 8 characters, no more no less.
   Would be seven, except we expect a single - from canonical code.
   Characters must be upper case.
   Note that ASCII is reversed: 'B':0 ... '0':11.
   --- */
static unsigned long b12decode(const char *szString)
{
  unsigned long uResult = 0;
  int           c;
  const char   *szS;

  for(szS = szString; szS < szString+8; szS++) {
    if(*szS == '-')
    	continue;
    c = *szS - '0';
    if(c > 9)
      c = c + '0' - 'A' + 10;
    uResult = (uResult * 12) + (11 - c);
  }
  return(uResult);
}


/* ----------------------------------------------------------------------
    Compute a CRC of sorts (we might do better than this simple minded
    approach by looking up some real stuff about CRCs).
   
    Returns: 0,1,2 or 3 - the last two bits total number of bits on
   --- */
static unsigned int twobitcrc(int uValue)
{
  unsigned int nBit = 0x8000;
  unsigned int nCrc = 0;
  
  while(nBit) {
    if(uValue & nBit)
      nCrc++;
    nBit = nBit >> 1;
  }
  return(nCrc & 0x3);
}


/* ----------------------------------------------------------------------
     Swap bits in chunks of 1, 2 or 3

     This obfuscation takes the chunk size from the random part of the
     reg code and swaps adjacent chunks.  We never muck with the last
     two of 25 bits because that's where the chunk size is stored.
   --- */
static unsigned long swap_bits(unsigned long uBits, unsigned int uAmount)
{
  unsigned long uMask1, uMask2;

  if(uAmount) {
    /* make the initial masks for the first swap */
    uMask1 = 0x7 >> (3 - uAmount);
    uMask2 = uMask1 << uAmount;
  
    while(uMask2 < (unsigned long)(1L << 23)) {
      unsigned long uTmp;
  
      /* do the swap */
      uTmp = (uBits & uMask1) << uAmount;
      uBits = (uBits & ~uMask1) | ((uBits & uMask2) >> uAmount);
      uBits = (uBits & ~uMask2) | uTmp;
  
      /* Get ready for next swap */
      uMask1 <<= uAmount*2;
      uMask2 <<= uAmount*2;
    }
  }
  return(uBits);
}


/* ----------------------------------------------------------------------
   public function - see regcode.h
   --- */
int regcode_encode(
    const char *szName,
    int         nMonth,
    int         nProduct,
    int         nRandom,
    char       *szCode)
{
  unsigned long uBits1, uBits2;
  unsigned long uNameHash;
  int           bValidName;

  /* Process name. It is XOR'd character by character first, 
     then run through the MD4 hash to get a 32 bit quantity. 
     The XOR is kinda like DESX "whitening". Having the XOR
     before the hash makes the guesser have to do the hash
     multiple times.
   */
  uNameHash = XORMD4Hash(szName, XOR_NAME, &bValidName);
  if(bValidName)
    return(1);

  
  /* Assemble components */
  uBits1 = (nProduct |
           ((uNameHash & 0xfffe00L) >> 1)) |
           ((unsigned long)twobitcrc(nProduct + (nMonth << 8)) << 23);
  uBits2 = ((uNameHash & 0x1ffL) |
           ((unsigned long)nMonth << 9) |
           (unsigned long)(REG_CODE_SCHEME_VERSION << 17)) |
           (((unsigned long)nRandom & 0x3f) << 19);


  uBits1 = swap_bits(uBits1, uBits2 >> 23);
  uBits2 = swap_bits(uBits2, uBits2 >> 23);


  /* Finally we base-12 encode the results into a 14 
     character string using the alphabet of [0-9AB]. 
     The dashes inserted are just for visual appearance
   */
  b12encode(szCode, uBits1 ^ XOR_1);
  szCode[7] = szCode[6];
  szCode[6] = szCode[5];
  szCode[5] = '-';
  b12encode(szCode+9, uBits2 ^ XOR_2);
  szCode[8] = szCode[9];
  szCode[9] = szCode[10];
  szCode[10] = '-';
  szCode[16] = '\0';
  return(0);
}



/* ----------------------------------------------------------------------
    public function - see regcode.h
   --- */
int regcode_verify(
  const char *szCode,
  const char *szName,
  int        *pnMonth,
  int        *pnProduct)
{
  char           szPureCode[REG_CODE_LEN + 20];
  int            nReturn = 0;
  unsigned long  uBits1, uBits2;
  unsigned long  uNameHash;
  unsigned long  uRegNameHash;
  int            bValidName;

  if(regcode_canonicalize(szCode, szPureCode))
    /* Didn't canonicalize bad length - must fail */
    return(1);

  /* Undo base 12 decoding */
  uBits1 = b12decode(szPureCode);
  uBits2 = b12decode(szPureCode+8);

  uBits1 ^= XOR_1;
  uBits2 ^= XOR_2;


  uBits1 = swap_bits(uBits1, uBits2 >> 23);
  uBits2 = swap_bits(uBits2, uBits2 >> 23); 
  

  /* Process name. It is XOR'd character by character first, 
     then run through the MD4 hash to get a 32 bit quantity. 
     The XOR is kinda like DESX "whitening". We also strip
     the first 8 bytes because we don't include them in code
     to keep the length of the code down.
  
     Fail if no valid chars (alpha num chars) in name given
   */
  uNameHash = XORMD4Hash(szName, XOR_NAME, &bValidName) & 0xffffff;
  if(bValidName)
    return(1);

  /* pull components out */
  uRegNameHash = ((uBits1 & 0x07fff00L) << 1) | (uBits2 & 0x1ffL);
  *pnProduct = (int)(uBits1 & 0xffL);
  *pnMonth   = (int)((uBits2 & 0x1fe00L) >> 9);

  /* Make sure reg scheme matches */
  if(((uBits2 & 0x60000L) >> 17) != 0)
    nReturn = 1;

  /* Check the CRC over the month and product */
  if(twobitcrc(*pnProduct + (*pnMonth << 8)) != (uBits1 >> 23))
    nReturn = 1;

  /* See if name hash matched */
  if(uRegNameHash != uNameHash)
    nReturn = 1;
  return(nReturn);
}
