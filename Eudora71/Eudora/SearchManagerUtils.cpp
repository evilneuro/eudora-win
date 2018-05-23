//	SearchManagerUtils.cpp
//
//	Search Manager utility methods.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#include "stdafx.h"
#include "SearchManagerUtils.h"


// ---------------------------------------------------------------------------
//		* LogError													 [Public]
// ---------------------------------------------------------------------------
//	Logs the specified error.

void
SearchManager::Utils::LogError(
	const char *				in_szErrorType,
	const char *				in_szFunctionName,
	const char *				in_szErrorMessage)
{
	//	Log error
	CString		szExceptionLog;

	szExceptionLog.Format("X1 %s Error in %s: %s", in_szErrorType, in_szFunctionName, in_szErrorMessage);
	PutDebugLog( DEBUG_MASK_SEARCH, szExceptionLog );
}


// ---------------------------------------------------------------------------
//		* LogCException												 [Public]
// ---------------------------------------------------------------------------
//	Logs the specified error.

void
SearchManager::Utils::LogCException(
	CException *				in_pException,
	const char *				in_szFunctionName)
{
	//	Log error
	const char *	szExceptionType;
		
	//	Get the exception type to log
	if ( in_pException->IsKindOf(RUNTIME_CLASS(CFileException)) )
		szExceptionType = "File";
	else if ( in_pException->IsKindOf(RUNTIME_CLASS(CMemoryException)) )
		szExceptionType = "Memory";
	else
		szExceptionType = "Generic";
	
	//	Log error
	TCHAR		szExceptionMessage[256];

	in_pException->GetErrorMessage(szExceptionMessage, 255);
	LogError( szExceptionType, in_szFunctionName, szExceptionMessage );
}



// ---------------------------------------------------------------------------
//		* EncodeRomanNumeralDigit									 [Public]
// ---------------------------------------------------------------------------
//	Encodes individual roman numeral digit padding with a space if necessary
//	to separate from previous roman numeral digits.

void
SearchManager::Utils::EncodeRomanNumeralDigit(
	long						in_nDigits,
	char						in_cRomanNumeral,
	char *						in_pStartBuffer,
	char *&						io_pBuffer)
{
	if (in_nDigits)
	{
		if (io_pBuffer > in_pStartBuffer)
			*io_pBuffer++ = ' ';
		
		//	Fill buffer with specified number of a given roman numeral
		memset(io_pBuffer, in_cRomanNumeral, in_nDigits);

		//	Move past number of digits we encoded
		io_pBuffer += in_nDigits;
	}
}


// ---------------------------------------------------------------------------
//		* EncodeRomanNumeralDigits									 [Public]
// ---------------------------------------------------------------------------
//	Encodes number as roman numeral'ish entity that allows us to find ranges
//	of numbers based on combinations of string prefix searches.

void
SearchManager::Utils::EncodeRomanNumeralDigits(
	long						in_nThousands,
	long						in_nHundreds,
	long						in_nTens,
	long						in_nOnes,
	CString &					out_szRomanNumeral)
{
	if (!in_nThousands && !in_nHundreds && !in_nTens && !in_nOnes)
	{
		out_szRomanNumeral.Empty();
		return;
	}
	
	//	Get buffer bigger than what we could ever need
	char *		szBuffer = out_szRomanNumeral.GetBuffer(128);
	char *		pBuffer = szBuffer;

	EncodeRomanNumeralDigit(in_nThousands, 'M', szBuffer, pBuffer);
	EncodeRomanNumeralDigit(in_nHundreds, 'C', szBuffer, pBuffer);
	EncodeRomanNumeralDigit(in_nTens, 'X', szBuffer, pBuffer);
	EncodeRomanNumeralDigit(in_nOnes, 'I', szBuffer, pBuffer);

	//	Release buffer with correct length
	out_szRomanNumeral.ReleaseBuffer(pBuffer - szBuffer);
}


// ---------------------------------------------------------------------------
//		* CalculateNumRomanNumeralDigits							 [Public]
// ---------------------------------------------------------------------------
//	Calculates number of roman numeral digist needed to encode number
//	as roman numeral'ish entity.

void
SearchManager::Utils::CalculateNumRomanNumeralDigits(
	long						in_nValue,
	long						in_nMaxValue,
	RomanNumeralsEnum			in_eLastDigit,
	long &						out_nThousands,
	long &						out_nHundreds,
	long &						out_nTens,
	long &						out_nOnes)
{
	if (in_nValue > in_nMaxValue)
	{
		ASSERT(!"Number exceed maximum allowable value");
		in_nValue = in_nMaxValue;
	}
	
	//	Calculate how many of each roman numeral we need
	long		nRemainder = in_nValue;

	if (in_eLastDigit == rn_LastDigitIsThousand)
	{
		out_nThousands = nRemainder / 1000;
		nRemainder %= 1000;
	}

	if (in_eLastDigit >= rn_LastDigitIsHundred)
	{
		out_nHundreds = nRemainder / 100;
		nRemainder %= 100;
	}

	if (in_eLastDigit >= rn_LastDigitIsTen)
		out_nTens = nRemainder / 10;

	out_nOnes = nRemainder % 10;
}


// ---------------------------------------------------------------------------
//		* EncodeNumberAsRomanNumeral								 [Public]
// ---------------------------------------------------------------------------
//	Encodes number as roman numeral'ish entity that allows us to find ranges
//	of numbers based on combinations of string prefix searches.
//
//	Numbers are encoded in simplified roman numeral rules. Numerals are
//	additive, never subtractive.
//	M = 1000
//	C = 100
//	X = 10
//	I = 1
//
//	For example 5746 would be encoded as:
//	MMMMM CCCCCCC XXXX IIIIII
//
//	The maximum number that can be represented is limited by the thousand
//	place and the size word that X1 allows (39 characters at the moment).
//	That currently means a limit of a little more than 39,000.

void
SearchManager::Utils::EncodeNumberAsRomanNumeral(
	long						in_nValue,
	long						in_nMaxValue,
	RomanNumeralsEnum			in_eLastDigit,
	CString &					out_szRomanNumeral)
{
	//	Calculate how many of each roman numeral we need
	long		nThousands = 0;
	long		nHundreds = 0;
	long		nTens = 0;
	long		nOnes = 0;

	CalculateNumRomanNumeralDigits( in_nValue, in_nMaxValue, in_eLastDigit,
									nThousands, nHundreds, nTens, nOnes );

	//	Encode number as roman numeral
	EncodeRomanNumeralDigits(nThousands, nHundreds, nTens, nOnes, out_szRomanNumeral);
}
