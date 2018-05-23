// ehlo.cpp
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

#include "stdafx.h"

#include <qcutils.h>
#include <afxmt.h>


#include "ehlo.h"


#include "DebugNewHelpers.h"


//=============== Implementation of CEhlo ====================//

CEhlo::CEhlo () :
	m_MaxSize(0), m_bSawGreeting(FALSE), m_bMime8Bit(FALSE),
	m_bAuth (FALSE),m_bTLS(FALSE)
{
	

}

CEhlo::~CEhlo()
{

}


void CEhlo::ParseMaxSize (LPCSTR pszBuf)
{
	if (!pszBuf)
		return;

	m_MaxSize = 0x7fffffff;

	long longVal = atol(pszBuf);
	if (longVal > 1000)
		m_MaxSize = longVal;
}





//static
EsmtpKey CEhlo::FindESMTPType (LPCSTR pDirective)
{
	if (!pDirective)
		return ESMTP_ERROR;

	// Must be one of these:
	//
	EsmtpKey key = ESMTP_ERROR;

	if ( strcmp (pDirective, "8BITMIME") == 0)
		key = ESMTP_MIME8BIT;
	else if ( strcmp(pDirective, "AUTH") == 0)
		key = ESMTP_AUTH;
	else if( strcmp(pDirective, "STARTTLS") ==0)
		key = ESMTP_STARTTLS;

	return key;
}
