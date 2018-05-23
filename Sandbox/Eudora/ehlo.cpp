// ehlo.cpp

#include "stdafx.h"

#include <qcutils.h>
#include <afxmt.h>


#include "ehlo.h"




//=============== Implementation of CEhlo ====================//

CEhlo::CEhlo () :
	m_MaxSize(0), m_bSawGreeting(FALSE), m_bMime8Bit(FALSE),
	m_bAuth (FALSE)
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

	return key;
}
