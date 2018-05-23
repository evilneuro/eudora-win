// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		    PCB
//  Description:	Declarations for CryptoRotor (SECCryptoRotor)
//  Created:		Apr 4, 1995
//
//  CHANGELOG:
//     PCB          12/1/95    SEC++ Accomodations
//
//

#ifndef __ROTOR_H__
#define __ROTOR_H__


//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL


#ifndef FALSE
#define FALSE 0
#endif /* FALSE */
#ifndef TRUE
#define TRUE (!FALSE)
#endif /* TRUE */

/////////////////////////////////////////////////////////////////////////////
// SECCryptoRotor

class SECCryptoRotor
{
// Construction
public:
		SECCryptoRotor( char *array, long size );
		SECCryptoRotor( );
		~SECCryptoRotor( );
		
// Operations
BOOL 	Load( char *array, long size );
void 	Set( long what );
void	Reset( void );
int	GetSize( void );
char	GetChar( void );
char	operator++( int );// int forces it to be postfix.

// Attributes
private:
		char *m_pBuffer;
		long  m_bufSz;
		long  m_ptr;
};
#endif // __ROTOR_H__
