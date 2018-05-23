// JunkMail.h
//
// Copyright (c) 2003 by QUALCOMM, Incorporated
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

#ifndef _EUD_JUNKMAIL_H_
#define _EUD_JUNKMAIL_H_

class CSummary;
class CTocDoc;
class CFilterActions;
class CImapSum;

enum {specialJunkIDManual, specialJunkIDWhitelist, specialJunkIDScoredZero};

class CJunkMail : public CObject
{
public:
	
	CJunkMail();
	~CJunkMail();

	// Whitelisting
	static bool		 IsWhitelisted(CString &strFrom, CObArray *poaABHashes);
	static bool		 IsWhitelisted(CSummary *pSum, CObArray *poaABHashes);
	static bool		 IsWhitelisted(CImapSum *pSum, CObArray *poaABHashes);

	// Adding sender to address book
	static void		 AddMsgSenderToAddressBook(CSummary *pSum);
	static void		 AddMsgSenderToAddressBook(CImapSum *pSum);
	static void		 AddAddressToAddressBook(CString &strAddress);

	// Junk processing
	static bool		 ProcessOne(CSummary *pSum, CObArray *poaABHashes = NULL);
	static bool		 ReprocessOne(CSummary *pSum, CFilterActions *pFilt, CObArray *poaABHashes);
	static CSummary	*DeclareJunk(CSummary *pSum,
								 bool bJunk,
								 bool bAddNotJunkedSendersToAB,
								 CObArray *poaABHashes,
								 CFilterActions *pFilt = NULL);
};

#define CTRL_J_UNKNOWN			0
#define CTRL_J_JUNK				1
#define CTRL_J_FILTER			2

#endif
