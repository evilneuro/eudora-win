// plist_md5.h -- secret decoder ring for playlists & requests
//
// Copyright (c) 1999 by QUALCOMM, Incorporated
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

//

//
/*
 ***********************************************************************
 ** md5.h -- header file for implementation of MD5										**
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm							**
 ** Created: 2/17/90 RLR																							**
 ** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version 							**
 ** Revised (for MD5): RLR 4/27/91																		**
 ** 	-- G modified to have y&~z instead of y&z 											**
 ** 	-- FF, GG, HH modified to add in last register done 						**
 ** 	-- Access pattern: round 2 works mod 5, round 3 works mod 3 		**
 ** 	-- distinct additive constant for each step 										**
 ** 	-- round 4 added, working mod 7 																**
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.	**
 ** 																																	**
 ** License to copy and use this software is granted provided that		**
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.																				**
 ** 																																	**
 ** License is also granted to make and use derivative works					**
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.							**
 ** 																																	**
 ** RSA Data Security, Inc. makes no representations concerning 			**
 ** either the merchantability of this software or the suitability		**
 ** of this software for any particular purpose.	It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 ** 																																	**
 ** These notices must be retained in any copies of any part of this	**
 ** documentation and/or software.																		**
 ***********************************************************************
 */

/* typedef a 32-bit type */
typedef unsigned long int UINT4;

/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
	UINT4 i[2]; 									/* number of _bits_ handled mod 2^64 */
	UINT4 buf[4]; 																	 /* scratch buffer */
	unsigned char in[64]; 														 /* input buffer */
	unsigned char digest[16]; 		/* actual digest after MD5Final call */
} MD5_CTX;

void MD5Init (MD5_CTX *mdContext);
void MD5Final (MD5_CTX *mdContext);
void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);

/*
 ***********************************************************************
 ** End of md5.h																											**
 ******************************** (cut) ********************************
 */

// generate a checksum for a given buffer of text
bool GenerateMD5Digest(const char* pszSecret, const char* pszBuf, char pszDigest[33]);
bool GenerateMD5DigestFromFile(const char* pszSecret, const char* pszFile, char pszDigest[33]);

