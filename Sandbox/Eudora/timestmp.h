// This class is only used for EXPIRING builds.

#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H

#include <windows.h>

// Product Used bit designator
#define FLAG_BETA_302		0
#define FLAG_DEMO_302		1
#define FLAG_BETA_400		2
#define FLAG_DEMO_400		3
#define FLAG_BETA_401		4
#define FLAG_DEMO_401		5
#define FLAG_BETA_410		6
#define FLAG_DEMO_410		7
#define FLAG_BETA_420		8
#define FLAG_DEMO_420		9

// the one we're currently building 
// the project (makefile) should define a build type

#define PRODUCT_FLAG	FLAG_BETA_420
#define REG_ROOT		"HKEY_CURRENT_USER\\Software\\Microsoft\\Notepad"
#define REG_KEY_0		"rtWindowPos"
#define REG_KEY_1		"rtIconPos"


const DWORD SECRET_SEED	= 0x29D7AB54;

const DWORD MAGIC0		= 0x59253847;
const DWORD MAGIC1		= 0xADCFEACB;
const DWORD MAGIC2		= 0x8D5F0E5C;
const DWORD MAGIC3		= 0xB6F8E9A4;


typedef struct
{
	DWORD	dwMagic0;
	DWORD	dwMagic1;
	DWORD	dwMagic2;
	DWORD	dwMagic3;
	DWORD	dwDays0;		// number of days to allow after first execution (in days)
	DWORD	dwDays1;		// number of days to allow after first execution (in days)
	DWORD	dwExpire0;		// time() of drop dead date
	DWORD	dwExpire1;		// time() of drop dead date
	DWORD	dwCheckSum;
	DWORD	dwFileSum;
} TIMESTAMP;

class CTimeStamp
{
	// duplicate methods exist to confuse hackers

private:
	TIMESTAMP	m_Stamp;
	DWORD		m_EvalStart;	// time() representation
	DWORD		m_EvalEnd0;		// time() representation
	DWORD		m_EvalEnd1;		// time() representation

	DWORD DecodeDays0( DWORD dwCooked );
	DWORD EncodeDays0( DWORD dwRaw );
	DWORD DecodeDays1( DWORD dwCooked );
	DWORD EncodeDays1( DWORD dwRaw );
	DWORD DecodeExpire0( DWORD dwCooked );
	DWORD EncodeExpire0( DWORD dwRaw );
	DWORD DecodeExpire1( DWORD dwCooked );
	DWORD EncodeExpire1( DWORD dwRaw );

	DWORD CalcSum0( void );
	DWORD CalcSum1( void );
	DWORD CalcSum2( void );

	DWORD CalcFileSum( int fid, DWORD dwBytes );

public:
	CTimeStamp();
	~CTimeStamp();

	BOOL Read( LPCSTR lpFile );
	BOOL Write( LPCSTR lpFile );

	// the rest assume a Read has been performed
	void ReadRegistry( void );
	BOOL Exists0( void );
	BOOL Exists1( void );
	BOOL Exists2( void );

	BOOL IsValid0( void );
	BOOL IsValid1( void );
	BOOL IsValid2( void );

	BOOL IsExpired0( void );
	BOOL IsExpired1( void );
	BOOL IsExpired2( void );

	DWORD GetDays0( void );
	DWORD GetDays1( void );
	DWORD GetDays2( void );

	// used for seven day warning code
	DWORD GetDaysRemaining( void );

	BOOL SetDays( DWORD dwRunDays, DWORD dwMaxDays );
};

#endif _TIMESTAMP_H
