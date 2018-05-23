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



    This code is intended to be 100% portable. It has been ported to UNIX
    and PalmOS thus far. It has no references to external libraries.

    Note that one of the reasons for the structure of this code is so 
    compilers that strip unreferenced functions can remove code for 
    characters sets that are not used in a particular implementation.
    ====================================================================== */

#include <stdio.h>
#include "regcode_charsets_v3.h"



/* ----------------------------------------------------------------------
    Does most of the work for iso-8859-*. This assumes US-ASCII in the
    lower 128 and two ranges of upper and lower. 
    The first range assumes 0xa0-0xaf is the upper of 0xb0 to 0xbf
    The second range assumes 0xc0-0xdf is the upper of 0xe0 to 0xff

    This takes a chracter and the two maps and returns the mapped
    character.
   --- */
static int prv_map_iso_8859(int nChar, const char *pcMap1, const char *pcMap2)
{
  if(nChar >= 0xC0) {
    /* Char is in accented range for which we use a map */
    nChar -= 0xC0;
    if(nChar >= 0x20)
      nChar -= 0x20;
    nChar = pcMap1[nChar];
        
  } else if(pcMap2 && nChar >= 0xA0) {
    nChar -= 0xA0;
    if(nChar >= 0x10)
      nChar -= 0x10;
    nChar = pcMap2[nChar];
  } else  if(!((nChar >= '0' && nChar <= '9') ||
               (nChar >= 'A' && nChar <= 'Z') ||
               (nChar >= 'a' && nChar <= 'z'))) {
    /* Char is NOT alpha-numeric; ignore it */
    nChar = '\0';
  }
  return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_1(int nChar)
{
  const char *pcISO_8859_1_MAP =
    "aaaaaaac"     /* C0  */
    "eeeeiiii"     /* D0  */
    "enooooo\x00"  /* D8 - note the multiply symbol is mapped to nothing */
    "ouuuuyts";    /* Note also that the final character might be an */

  if(nChar == 0xff)
    nChar = 'y';
  else
    nChar = prv_map_iso_8859(nChar, pcISO_8859_1_MAP, NULL);

  return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_2(int nChar)
{
  const char *pcISO_8859_2_MAP =
    "raaaalcc"
    "ceeeeiid"
    "dnnoooo\x00"
    "ruuuuyts";
  
  const char pcISO_8859_2_MAP_2[16] =
        {0x00,'a',0x00,'l',0x00,'l','s',0x00,0x00,'s','s','t','z',0x00,'z','z'};

   if(nChar == 0xff)
     nChar = 0;
   else
     nChar =  prv_map_iso_8859(nChar, pcISO_8859_2_MAP, pcISO_8859_2_MAP_2);

   return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_3(int nChar)
{
   const char *pcISO_8859_3_MAP =
     "aaa\x00"        /* C0 - C3 */
     "accc"           /* C4 - C7 */
     "eeeeiiii"       /* C8 - CF */
     "\x00nooogo\x00" /* D0 - D7 */
     "guuuuuss";      /* D8 - DF */

   const char pcISO_8859_3_MAP_2[16] =
      {0x00,'h',0x00,0x00,0x00,0x00,'h',0x00,0x00,'i','s','g','j',0x00,0x00,'z'};

   if(nChar == 0xff)
     nChar = 0;
   else
     nChar =  prv_map_iso_8859(nChar, pcISO_8859_3_MAP, pcISO_8859_3_MAP_2);

   return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_4(int nChar)
{
   const char *pcISO_8859_4_MAP =
     "aaaaaaai"
     "ceeeeiii"
     "dnokooo\x00"
     "ouuuuuus";
  
   const char pcISO_8859_4_MAP_2[16] =
      {0x00,'a','k','r',0x00,'i','l',0x00,0x00,'s','e','g','t',0x00,'z',0x00};
   
   if(nChar == 0xb2 || nChar == 0xff)
     nChar = 0;
   else if(nChar == 0xbd || nChar == 0xbf)
     nChar = 'e';
   else
     nChar = prv_map_iso_8859(nChar, pcISO_8859_4_MAP, pcISO_8859_4_MAP_2);

   return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_9(int nChar)
{
  const char *pcISO_8859_9_MAP =
    "aaaaaaac"
    "eeeeiiii"
    "gnooooo\x00"
    "ouuuuiss";

  if(nChar == 0xff)
    nChar = 'y';
  else
    nChar = prv_map_iso_8859(nChar, pcISO_8859_9_MAP, NULL);

  return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_10(int nChar)
{
   const char *pcISO_8859_10_MAP =
     "aaaaaaai"
     "ceeeeiii"
     "enooooou"
     "ouuuuyts";

   const char pcISO_8859_10_MAP_2[16] =
      {0x00,'a','e','g','i','i','k',0x00,'l','d','s','t','z',0x00,'u','e'};
     
   if(nChar == 0xff)
     nChar = 'k';
   else
     nChar = prv_map_iso_8859(nChar, pcISO_8859_10_MAP, pcISO_8859_10_MAP_2);

   return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_13(int nChar)
{
   const char *pcISO_8859_13_MAP =
     "aiacaaee"
     "cezegkil"
     "snnoooo\x00"
     "ulsuuzzs";

   if(nChar == 0xA8 || nChar == 0xB8) {
     nChar = 'o';
   } else if(nChar == 0xAA || nChar == 0xBA) {
     nChar = 'r';
   } else if(nChar == 0xAF || nChar == 0xBF) {
     nChar = 'a';
   } else if(nChar == 0xFF) {
     nChar = 0;
   } else {
     nChar = prv_map_iso_8859(nChar, pcISO_8859_13_MAP, NULL);
   }
   return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_iso_8859_14(int nChar)
{
   const char *pcISO_8859_14_MAP =
     "aaaaaaac"
     "eeeeiiii"
     "wnooooot"
     "ouuuuyys";

   // gotta be careful when mixing string literal concatenation and escape
   // sequences---they can behave unexpectedly. for instance:
   //
   //     "\0123" --->  | \12 | 3 | \0 |
   //
   // while:
   //
   //     "\01" "23" --->  | \01 | 2 | 3 | \0 |

   const char *pcISO_8859_14_MAP_2 = 
     "\x00" "bb\x00" "ccd\x00"
         "w\x00wdy\x00\x00y"
         "ffggmm\x00p"
         "wpwsywws";
 
   if(nChar >= 0xA0 && nChar < 0xC0)
     nChar = pcISO_8859_14_MAP_2[nChar - 0xA0];
   else if(nChar == 0xff)
     nChar = 'y';
   else
     nChar = prv_map_iso_8859(nChar, pcISO_8859_14_MAP, NULL);

   return(nChar);
}


/* ----------------------------------------------------------------------

   --- */
static int prv_map_win_cp1252(int nChar)
{
  if(nChar == 0x8A)
    nChar = 's';
  else if(nChar == 0x8C)
    nChar = 'o';
  else if(nChar == 0x8E)
    nChar = 'z';
  else if(nChar == 0x9A)
    nChar = 's';
  else if(nChar == 0x9C)
    nChar = 'o';
  else if(nChar == 0x9E)
    nChar = 'z';
  else if(nChar == 0x9F)
    nChar = 'y';
  else
    nChar = prv_map_iso_8859_1(nChar);
  return(nChar);
}
  

/* ----------------------------------------------------------------------

   --- */
static int prv_map_palmos(int nChar)
{
  if(nChar == 0x8e)
    nChar = 0;
  else
    nChar = prv_map_win_cp1252(nChar);
  return(nChar);
}   


/* ----------------------------------------------------------------------

   --- */
static int prv_map_macroman(int nChar)
{
        const char *mac_Roman_MAP =
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "01234567"
                "89\000\000\000\000\000\000"
                "\000abcdefg"
                "hijklmno"
                "pqrstuvw"
                "xyz\000\000\000\000\000"
                "\000abcdefg"
                "hijklmno"
                "pqrstuvw"
                "xyz\000\000\000\000\000"
                "aacenoua"
                "aaaaacee"
                "eeiiiino"
                "oooouuuu"
                "y\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000ao"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000ao"
                "\000\000aaooo\000"
                "\000\000\000\000\000\000\000\000"
                "yy\000\000eett"
                "y\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000"
                "\000\000\000\000\000\000\000\000";
                
        return(mac_Roman_MAP[nChar]);
}   


/* ----------------------------------------------------------------------
    Here's the main starting point for all mappers. It takes the
    input and output and a function to do map individual characters.
    It returns the length of the output.
   --- */
static int prv_map_latin(const char *szIn, char *szOut, int (*fCharMapper)(int))
{
  char         *pc;
  unsigned char u;

  pc = szOut;
  if(szIn != NULL) {
    for(; *szIn; szIn++) {
      u = (*fCharMapper)(*(const unsigned char *)szIn);
      if(u < '0')
        continue;
      u |= 0x20;
      if(u > '9' && u < 'a')
        continue;
      if(u > 'z')
        continue; 
      *pc++ = u;
    }
  }
  *pc = '\0';
  return(pc - szOut);
}





/* ----------------------------------------------------------------------
    All the public entry points for the different character sets. 
   --- */
int regcode_v3_map_iso_8859_1(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_1));
}

int regcode_v3_map_iso_8859_2(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_2));
}

int regcode_v3_map_iso_8859_3(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_3));
}

int regcode_v3_map_iso_8859_4(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_4));
}

int regcode_v3_map_iso_8859_9(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_9));
}

int regcode_v3_map_iso_8859_10(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_10));
}

int regcode_v3_map_iso_8859_13(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_13));
}

int regcode_v3_map_iso_8859_14(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_iso_8859_14));
}

int regcode_v3_map_win_cp1252(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_win_cp1252));
}

int regcode_v3_map_palmos(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_palmos));
}

int regcode_v3_map_macroman(const char *szIn, char *szOut)
{
 return(prv_map_latin(szIn, szOut, prv_map_macroman));
}


int regcode_v3_map_jis(const char *szIn, char *szOut)
{
  /* Do Japanese */
  /* Warning this code has not been tested or debugged! */
  int nReturn = 0;

  if(szIn) {
    /* Just a string copy, but we don't want to 
       references any libraries to stay portable */
    while(*szIn) {
      *szOut++ = *szIn;
       nReturn++;
    }
  }
  return(nReturn);
}


