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
//  Description:	Declarations for Crypto-BlackBox (SECCryptoBlackBox)
//  Created:		Apr 4, 1995
//
//  CHANGELOG:
//     PCB          12/1/95    SEC++ Accomodations
//
//


#ifndef __BLACKBOX_H__
#define __BLACKBOX_H__


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

#include "rotor.h"



#define KEYSIZE 72
#define NROTORS 3

/////////////////////////////////////////////////////////////////////////////
// SECCryptoBlackBox

class SECCryptoBlackBox
{
public:
enum 	eDir { ENC, DEC };		// Direction: ENCrypt, DECrypt...
enum 	eMode { ECB, OFB }; 	// ECB == Electronic CodeBook 
								// OFB == Output FeedBack

// Construction
		SECCryptoBlackBox( TCHAR FAR *pswd = NULL, eDir dir=ENC, eMode mode = ECB );
		~SECCryptoBlackBox( );
		
// Operations
BOOL	SetPassword( const TCHAR FAR *buf );
void 	SetPtr( long toWhere );
void	SetDir( eDir dir );
void 	SetMode( eMode mode );
SECCryptoBlackBox::eMode	
		GetMode( void );
void 	Reset( void );

void 	TransformBuf( char FAR *buf, long nChars );

protected:
void	Encrypt( char FAR &c );
void	Decrypt( char FAR &c );

private:
const 	char &NextKey( void );
void 	HashString( char FAR *from, int fromSize, char FAR *to, int toSize );

// Attributes		
int 	m_nRotors;
SECCryptoRotor 
		*m_rotor;
eDir	m_opDir;
eMode 	m_opMode;					
 };


//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // _BLACKBOX_H_
		
