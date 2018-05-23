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

	     Nov 3 - lgl - Branch to v2; see regcode_v2.h

             Apr 24 2000 - lgl - Fix bug causing infinite loop when
             verify some invalid reg codes. V2 regcodes with the problem
             are in the range xxxx-xxxx-00xx-xxxx to xxxx-xxxx-32xx-xxxx
 
             May 26 2000 - lgl - Fixed verification of certain regcodes
             mainly v2 codes from desktop Eudora

             June 6, 2000 - lgl - Fixed verification of v1 regcodes

   ====================================================================== */

#include "regcode_v2.h"

//	Ignore warning for "conditional expression is constant"
#pragma warning(disable : 4127)  

/* Some randomly chosen quantities we xor with for obfuscation */
#define XOR_NAME (0x35)
#define XOR_1 (0x01C5E73A)
#define XOR_2 (0x01F6433E)


/* ----------------------------------------------------------------------
    See if character exists in string (our own func for portability)
   --- */
static int CharInStr(char c, char *szString)
{
  int nReturn = 1;
  while(*szString) {
    if(*szString == c) {
      nReturn = 0;
      break;
    }
    szString++;
  }
  return(nReturn);
}


/* ----------------------------------------------------------------------
    public function - see regcode.h
   --- */
RegCodeVEnum regcode_v2_canonicalize(const char *szRegcode, char *szCanonCode)
{
  const char  *s;
  char        *d;
  int          nCharCount, nMod, nIsValid;
  RegCodeVEnum nReturn;

  int nCharPlusDigitCnt = 0;
  for(s = szRegcode; *s; s++) {
    if(!CharInStr(*s, "0123456789IOabAB"))
      nCharPlusDigitCnt++;
  }

if((nCharPlusDigitCnt != 14) && (nCharPlusDigitCnt != 16)) {
   nReturn = REG_CODE_BAD;
    goto done;
  }

  nReturn = REG_CODE_V2;
  nMod = 4;

  if(nCharPlusDigitCnt == 14) {
    nMod = 5;
    nReturn = REG_CODE_V1;
  }

	s = szRegcode;
	d = szCanonCode;
	nCharCount = 0;

  while(1) {
  	/* Copy the valid chars with some translation */
    nIsValid = 0;
    if((*s >= '0' && *s <= '9') || 
       (*s >= 'a' && *s <= 'b') ||
       (*s >= 'A' && *s <= 'B')) {
      *d = *s;
      if(*d >= 'a')
        *d &= ~0x20;
      d++;
      nIsValid = 1;
    } else if(*s == 'O') {
      *d++ = '0';
      nIsValid = 1;
    } else if(*s == 'I') {
      *d++ = '1';
      nIsValid = 1;
    }

		s++;
		if(!*s || (d - szCanonCode) >= (REG_CODE_LEN-1))
			break;
			
    if(nIsValid) {
      nCharCount++;
     
      /* Insert '-' in the correct place */
      if(!(nCharCount % nMod)) {
        *d++ = '-';
      }

      if ( (d - szCanonCode) >= (REG_CODE_LEN-1) )
		  break;
    }
  }  
  *d = '\0';
 done:  
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
   Base 10 encoder. Will insert "-" after 4th char generated
   Note that ASCII is reversed: '9':0 ... '0':9.
   --- */
static void b10encode(char *dest, unsigned long uValue)
{
  unsigned long uMult;
  unsigned int c;
  unsigned int i;
  unsigned int n;
  
  uMult = 10000000L;
  for(i = 0; i < 9; i++) {
    n = uValue / uMult;
    c = (9 - n) + '0';
    dest[i] = (char) c;
    if(i == 3) {
      i++;
      dest[i] = '-';
    }
    uValue -= n * uMult;
    uMult = uMult / 10;
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
    Base 10 decode a string
   --- */
static unsigned long b10decode(const char *szString)
{
  unsigned long uResult = 0;
  int           c;
  const char   *szS;

  for(szS = szString; szS < szString+9; szS++) {
    if(*szS == '-')
    	continue;
    c = *szS - '0';
    uResult = (uResult * 10) + (9 - c);
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
    A one bit parity check over five bits
   --- */
static unsigned int onebitcrc(unsigned int uValue)
{
  unsigned int nBit;
  unsigned int nCrc;

  nBit = 0x10;
  nCrc = 0;
  while(nBit) {
    if(uValue & nBit)
      nCrc++;
    nBit = nBit >> 1;
  }
  return(nCrc & 0x01);
}


/* ----------------------------------------------------------------------
    Four parity bits give a 20 bit input
   --- */
static unsigned int fourbitcrc(unsigned long uValue)
{
  unsigned int nCrc = 0;

  
  while(uValue) {
    nCrc = (nCrc << 1) + onebitcrc(uValue & 0x1f);
    uValue = uValue >> 5;
  }
  return(nCrc);
}


/* ----------------------------------------------------------------------
     Swap bits in chunks of 0-6 bits

     uBits are the ones to swap
     uAmount is the chunk size to swap
     nMax is the maxinum bit number to mess with

     This obfuscation takes the chunk size from the random part of the
     reg code and swaps adjacent chunks. 
   --- */
static unsigned long swap_bits(unsigned long uBits, unsigned int uAmount, int nMax)
{
  unsigned long uMask1, uMask2;

  if(uAmount) {
    /* make the initial masks for the first swap */
    uMask1 = 0x3f >> (6 - uAmount);
    uMask2 = uMask1 << uAmount;
  
    while(uMask2 < (unsigned long)(1L << nMax)) {
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
int regcode_v2_encode(
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
           ((uNameHash & 0xfffc00L) >> 2)) |
           ((unsigned long)fourbitcrc(
           			(unsigned long)nProduct +
           			((unsigned long)nMonth << 8) + 
           			(((unsigned long)nRandom & 0xFL) << 16)) << 22);
  uBits2 = ((uNameHash & 0x3ffL) |
           ((unsigned long)nMonth << 10) |
           (unsigned long)(REG_CODE_SCHEME_VERSION << 18)) |
           (((unsigned long)nRandom & 0x3f) << 20);

  /* Conditional swapping for added security */
  uBits1 = swap_bits(uBits1, (uBits2 >> 24)*2, 26);
  uBits2 = swap_bits(uBits2, (uBits2 >> 24)*2, 22);

  /* Assemble two halves */
  b10encode(szCode, uBits1 ^ XOR_1);
  szCode[9] = '-';
  b10encode(szCode + 10, uBits2 ^ XOR_2);
  szCode[19] = '\0';
  return(0);
}


/* ----------------------------------------------------------------------
    Note we only verify v1 regcodes. They are obsolete and not generated.
   --- */
static int regcode_v1_verify(
  const char   *szPureCode,
  unsigned long uNameHash,
  int          *pnMonth,
  int          *pnProduct)
{
  int            nReturn = 0;
  unsigned long  uBits1, uBits2;
  unsigned long  uRegNameHash;
  unsigned int   uSwapChunkSize;

  /* Undo base 12 decoding */
  uBits1 = b12decode(szPureCode);
  uBits2 = b12decode(szPureCode+8);

  uBits1 ^= XOR_1;
  uBits2 ^= XOR_2;

  uSwapChunkSize = uBits2 >> 23;
  if(uSwapChunkSize > 6) {
    nReturn = 1;
  } else {
    uBits1 = swap_bits(uBits1, uSwapChunkSize, 23);
    uBits2 = swap_bits(uBits2, uSwapChunkSize, 23);
  }
  
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


/* ----------------------------------------------------------------------
    public function - see regcode.h
   --- */
int regcode_v2_verify(
  const char *szCode,
  const char *szName,
  int        *pnMonth,
  int        *pnProduct)
{
  char           szPureCode[REG_CODE_LEN];
  int            nReturn = 0;
  unsigned long  uBits1, uBits2;
  unsigned long  uNameHash;
  unsigned long  uRegNameHash;
  int            bValidName;
  RegCodeVEnum   nCodeType;
  unsigned int   uSwapChunkSize;

  /* Figure out version of regcode input */
  nCodeType = regcode_v2_canonicalize(szCode, szPureCode);
  if(nCodeType == REG_CODE_BAD)
    return(1);

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

  /* Bounce over to v1 verifier */
  if(nCodeType == REG_CODE_V1)
    return(regcode_v1_verify(szPureCode, uNameHash, pnMonth, pnProduct));

  /* Undo ascii string encoding */
  uBits1 = b10decode(szPureCode);
  uBits2 = b10decode(szPureCode+10);

  /* Undo XORing obfuscation*/
  uBits1 ^= XOR_1;
  uBits2 ^= XOR_2;

  /* Undo swapping obfuscation */
  uSwapChunkSize = (uBits2 >> 24) * 2;
  if(uSwapChunkSize > 6) {
    nReturn = 1;
  } else {
    uBits1 = swap_bits(uBits1, uSwapChunkSize, 26);
    uBits2 = swap_bits(uBits2, uSwapChunkSize, 22);
  }
  
  /* pull components out */
  uRegNameHash = ((uBits1 & 0x03fff00L) << 2) | (uBits2 & 0x3ffL);
  *pnProduct = (int)(uBits1 & 0xffL);
  *pnMonth   = (int)((uBits2 & 0x3fc00L) >> 10);

  /* Make sure reg scheme matches */
  if(((uBits2 & 0xc0000L) >> 18) != 0)
    nReturn = 1;

  /* Check the CRC over the month and product */
  if(fourbitcrc((unsigned long)*pnProduct + ((unsigned long)*pnMonth << 8) + ((uBits2 & 0xf00000L) >> 4)) != (uBits1 >> 22))
    nReturn = 1;

  /* See if name hash matched */
  if(uRegNameHash != uNameHash)
    nReturn = 1;
  return(nReturn);
}
