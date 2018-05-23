// This class is only used for EXPIRING builds.

#include "stdafx.h"

#include <fcntl.h>
#include <io.h>
#include <time.h>

#include "regif.h"
#include "timestmp.h"
#include "doc.h"
#include "summary.h"
#include "tocdoc.h"
#include "debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CTimeStamp::CTimeStamp()
{
	m_EvalStart = m_EvalEnd0 = m_EvalEnd1 = 0L;
}

CTimeStamp::~CTimeStamp()
{
}

BOOL CTimeStamp::Read( LPCSTR lpFile )
{
	BOOL	bOK = FALSE;
	int		fid;

	fid = open( lpFile, O_RDONLY | O_BINARY );

	if ( fid != -1 )
	{
		long pos = lseek( fid, 0L - sizeof( TIMESTAMP ), SEEK_END );
		if ( pos != -1L )
		{
			int res = read( fid, &m_Stamp, sizeof( m_Stamp ) );
			if ( res != -1 )
			{
				// decode every thing
				m_Stamp.dwDays0 = DecodeDays0( m_Stamp.dwDays0 );
				m_Stamp.dwDays1 = DecodeDays1( m_Stamp.dwDays1 );
				m_Stamp.dwExpire0 = DecodeExpire0( m_Stamp.dwExpire0 );
				m_Stamp.dwExpire1 = DecodeExpire1( m_Stamp.dwExpire1 );

				// check the file checksum

				// I think this should disable the Checksum 
				// checking routine and still allow Eudora to run for it's 34 days or whatever.

				DWORD dwSum = m_Stamp.dwFileSum;
			//	DWORD dwSum = CalcFileSum( fid, pos );
				if ( dwSum != m_Stamp.dwFileSum )
				{
					m_Stamp.dwDays0 = 0L;
					m_Stamp.dwDays1 = 0L;
					m_Stamp.dwExpire0 = 0L;
					m_Stamp.dwExpire1 = 0L;
				}

				bOK = IsValid0();
			}
		}

		close( fid );
	}

	return bOK;
}

BOOL CTimeStamp::Write( LPCSTR lpFile )
{
	BOOL	bOK = FALSE;
	BOOL	bExists = FALSE;
	int		fid;

	fid = open( lpFile, O_RDWR | O_BINARY );

	if ( fid != -1 )
	{
		// look for an existing time stamp
		long pos = lseek( fid, 0L - sizeof( TIMESTAMP ), SEEK_END );
		if ( pos != -1L )
		{
			TIMESTAMP	tmpTS;

			int res = read( fid, &tmpTS, sizeof( tmpTS ) );
			if ( res != -1 )
			{
				if ( tmpTS.dwMagic0 == MAGIC0 &&
					 tmpTS.dwMagic1 == MAGIC1 &&
					 tmpTS.dwMagic2 == MAGIC2 &&
					 tmpTS.dwMagic3 == MAGIC3 )
				{
					bExists = TRUE;
				}
			}
		}

		// overwrite any existing timestamp
		if ( bExists )
			pos = lseek( fid, 0L - sizeof( TIMESTAMP ), SEEK_END );
		else
			pos = lseek( fid, 0L, SEEK_END );

		if ( pos != -1L )
		{
			// init the magic stuff
			m_Stamp.dwMagic0 = MAGIC0;
			m_Stamp.dwMagic1 = MAGIC1;
			m_Stamp.dwMagic2 = MAGIC2;
			m_Stamp.dwMagic3 = MAGIC3;

			// make sure the checksum is right
			m_Stamp.dwCheckSum = CalcSum0();

			// compute the file checksum
			m_Stamp.dwFileSum = CalcFileSum( fid, pos );

			// encode every thing
			m_Stamp.dwDays0 = EncodeDays0( m_Stamp.dwDays0 );
			m_Stamp.dwDays1 = EncodeDays1( m_Stamp.dwDays1 );
			m_Stamp.dwExpire0 = EncodeExpire0( m_Stamp.dwExpire0 );
			m_Stamp.dwExpire1 = EncodeExpire1( m_Stamp.dwExpire1 );

			int res = write( fid, &m_Stamp, sizeof( m_Stamp ) );
			if ( res == sizeof( m_Stamp ) )
				bOK = TRUE;
		}

		close( fid );
	}

	return bOK;
}

void CTimeStamp::ReadRegistry( void )
{
	REGIF	theReg;
	BOOL	bOK_0, bOK_1;
	char	szBuf[ 80 ];

	if ( m_Stamp.dwDays0 < 1000 && theReg.Attach( REG_ROOT ) )
	{
		bOK_0 = theReg.Read( REG_KEY_0, szBuf, sizeof( szBuf ) );
//		if ( ! bOK_0 )
//			PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Key0 not found");
		m_EvalEnd0 = atol( szBuf );

		bOK_1 = theReg.Read( REG_KEY_1,  szBuf, sizeof( szBuf ) );
//		if ( ! bOK_1 )
//			PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Key1 not found");
		m_EvalEnd1 = atol( szBuf );

		if ( !bOK_0 && !bOK_1 )
		{
			// looks be the first time - init the registry
			m_EvalEnd0 = m_EvalEnd1 = 
				time( NULL ) + m_Stamp.dwDays0 * 60L * 60L * 24L;

			// check the OutToc to make sure the user didn't simply delete the
			// registry keys
			CTocDoc* toc = GetOutToc();
			if ( toc )
			{
				BOOL used = toc->GetProductUsedFlag( PRODUCT_FLAG );
				if ( used )	
				{
					// muff up one of the values
					// this will cause the app to expire immediately
					m_EvalEnd0 += 10;
//					PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Prior application use detected");
				}
			}

			m_EvalEnd0 = EncodeExpire0( m_EvalEnd0 );
			m_EvalEnd1 = EncodeExpire1( m_EvalEnd1 );

			sprintf( szBuf, "%ld", m_EvalEnd0 );
			bOK_0 = theReg.Write( REG_KEY_0,szBuf );
//			if ( bOK_0 )
//				PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Key0 written ok");
//			else
//				PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Error writing Key0");

			sprintf( szBuf, "%ld", m_EvalEnd1 );
			bOK_1 = theReg.Write( REG_KEY_1,  szBuf );
//			if ( bOK_1 )
//				PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Key1 written ok");
//			else
//				PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Error writing Key1");

			if ( bOK_0 && bOK_1 && toc )
			{
				toc->SetProductUsedFlag( PRODUCT_FLAG );
				toc->Write( TRUE );	// flush the flag
			}
		}

		m_EvalEnd0 = DecodeExpire0( m_EvalEnd0 );
		m_EvalEnd1 = DecodeExpire1( m_EvalEnd1 );
		m_EvalStart = m_EvalEnd0 - ( m_Stamp.dwDays0 * 60L * 60L * 24L );
	}
}

BOOL CTimeStamp::Exists0( void )
{
	if ( m_Stamp.dwMagic0 != MAGIC0 ||
		 m_Stamp.dwMagic1 != MAGIC1 ||
		 m_Stamp.dwMagic2 != MAGIC2 ||
		 m_Stamp.dwMagic3 != MAGIC3 )
		return FALSE;

	return TRUE;
}

BOOL CTimeStamp::Exists1( void )
{
	if ( m_Stamp.dwMagic0 != MAGIC0 ||
		 m_Stamp.dwMagic1 != MAGIC1 ||
		 m_Stamp.dwMagic2 != MAGIC2 ||
		 m_Stamp.dwMagic3 != MAGIC3 )
		return FALSE;

	return TRUE;
}

BOOL CTimeStamp::Exists2( void )
{
	if ( m_Stamp.dwMagic0 != MAGIC0 ||
		 m_Stamp.dwMagic1 != MAGIC1 ||
		 m_Stamp.dwMagic2 != MAGIC2 ||
		 m_Stamp.dwMagic3 != MAGIC3 )
		return FALSE;

	return TRUE;
}

BOOL CTimeStamp::IsValid0( void )
{
	if ( ! Exists0() )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Stamp doesn't exist");
		return FALSE;
	}

	if ( m_Stamp.dwDays0 != m_Stamp.dwDays1 ||
		 m_Stamp.dwExpire0 != m_Stamp.dwExpire1 )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Decoded expiration info in Stamp is bad");
		return FALSE;
	}

	if ( m_Stamp.dwCheckSum != CalcSum0() )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Checksum in Stamp is bad");
		return FALSE;
	}

	return TRUE;
}

BOOL CTimeStamp::IsValid1( void )
{
	if ( ! Exists1() )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Stamp doesn't exist");
		return FALSE;
	}

	if ( m_Stamp.dwDays0 != m_Stamp.dwDays1 ||
		 m_Stamp.dwExpire0 != m_Stamp.dwExpire1 )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Decoded expiration info in Stamp is bad");
		return FALSE;
	}

	if ( m_Stamp.dwCheckSum != CalcSum1() )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Checksum in Stamp is bad");
		return FALSE;
	}

	return TRUE;
}

BOOL CTimeStamp::IsValid2( void )
{
	if ( ! Exists2() )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Stamp doesn't exist");
		return FALSE;
	}

	if ( m_Stamp.dwDays0 != m_Stamp.dwDays1 ||
		 m_Stamp.dwExpire0 != m_Stamp.dwExpire1 )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Decoded expiration info in Stamp is bad");
		return FALSE;
	}

	if ( m_Stamp.dwCheckSum != CalcSum2() )
	{
//		PutDebugLog( DEBUG_MASK_DEMO_EVAL, "Checksum in Stamp is bad");
		return FALSE;
	}

	return TRUE;
}

BOOL CTimeStamp::IsExpired0( void )
{
	BOOL	bRunDaysOK = FALSE;
	BOOL	bMaxDaysOK = FALSE;

	if ( IsValid0() ) 
	{
		DWORD now = (DWORD)time( NULL );
		
		if ( m_Stamp.dwDays0 >= 1000 )
			bRunDaysOK = TRUE;
		else if ( m_EvalEnd0 == m_EvalEnd1	&&
		 		  now < m_EvalEnd0			&&
		 		  now >= m_EvalStart )
			bRunDaysOK = TRUE;
			
		if ( now < m_Stamp.dwExpire0 )
			bMaxDaysOK = TRUE;

		if ( bRunDaysOK && bMaxDaysOK )
			return FALSE;	// not expired
		
	}

//	CString msg;
//	msg.Format( "Stamp days0 %ld days1 %ld expire0 %ld expire1 %ld",
//		m_Stamp.dwDays0, m_Stamp.dwDays1, m_Stamp.dwExpire0, m_Stamp.dwExpire1 );
//	PutDebugLog( DEBUG_MASK_DEMO_EVAL, msg );

//	msg.Format( "Eval start %ld end0 %ld end1 %ld", 
//		m_EvalStart, m_EvalEnd0, m_EvalEnd1 );
//	PutDebugLog( DEBUG_MASK_DEMO_EVAL, msg );

	return TRUE;
}

BOOL CTimeStamp::IsExpired1( void )
{
	BOOL	bRunDaysOK = FALSE;
	BOOL	bMaxDaysOK = FALSE;

	if ( IsValid1() ) 
	{
		DWORD now = (DWORD)time( NULL );
		
		if ( m_Stamp.dwDays0 >= 1000 )
			bRunDaysOK = TRUE;
		else if ( m_EvalEnd0 == m_EvalEnd1	&&
		 		  now < m_EvalEnd0			&&
		 		  now >= m_EvalStart )
			bRunDaysOK = TRUE;
			
		if ( now < m_Stamp.dwExpire0 )
			bMaxDaysOK = TRUE;

		if ( bRunDaysOK && bMaxDaysOK )
			return FALSE;	// not expired
		
	}

//	CString msg;
//	msg.Format( "Stamp days0 %ld days1 %ld expire0 %ld expire1 %ld",
//		m_Stamp.dwDays0, m_Stamp.dwDays1, m_Stamp.dwExpire0, m_Stamp.dwExpire1 );
//	PutDebugLog( DEBUG_MASK_DEMO_EVAL, msg );

//	msg.Format( "Eval start %ld end0 %ld end1 %ld", 
//		m_EvalStart, m_EvalEnd0, m_EvalEnd1 );
//	PutDebugLog( DEBUG_MASK_DEMO_EVAL, msg );

	return TRUE;
}

BOOL CTimeStamp::IsExpired2( void )
{
	BOOL	bRunDaysOK = FALSE;
	BOOL	bMaxDaysOK = FALSE;

	if ( IsValid2() ) 
	{
		DWORD now = (DWORD)time( NULL );
		
		if ( m_Stamp.dwDays0 >= 1000 )
			bRunDaysOK = TRUE;
		else if ( m_EvalEnd0 == m_EvalEnd1	&&
		 		  now < m_EvalEnd0			&&
		 		  now >= m_EvalStart )
			bRunDaysOK = TRUE;
			
		if ( now < m_Stamp.dwExpire0 )
			bMaxDaysOK = TRUE;

		if ( bRunDaysOK && bMaxDaysOK )
			return FALSE;	// not expired
		
	}

//	CString msg;
//	msg.Format( "Stamp days0 %ld days1 %ld expire0 %ld expire1 %ld",
//		m_Stamp.dwDays0, m_Stamp.dwDays1, m_Stamp.dwExpire0, m_Stamp.dwExpire1 );
//	PutDebugLog( DEBUG_MASK_DEMO_EVAL, msg );

//	msg.Format( "Eval start %ld end0 %ld end1 %ld", 
//		m_EvalStart, m_EvalEnd0, m_EvalEnd1 );
//	PutDebugLog( DEBUG_MASK_DEMO_EVAL, msg );

	return TRUE;
}

DWORD CTimeStamp::GetDays0( void )
{
	if ( IsValid0() && m_Stamp.dwDays0 == m_Stamp.dwDays1 )
		return m_Stamp.dwDays0;

	return 0;
}

DWORD CTimeStamp::GetDays1( void )
{
	if ( IsValid1() && m_Stamp.dwDays0 == m_Stamp.dwDays1 )
		return m_Stamp.dwDays0;

	return 0;
}

DWORD CTimeStamp::GetDays2( void )
{
	if ( IsValid2() && m_Stamp.dwDays0 == m_Stamp.dwDays1 )
		return m_Stamp.dwDays0;

	return 0;
}

// used for seven day warning code
// returns the best-case number of calendar days
// I.E. the app will expire in LESS THAN GetDaysRemaining()
//      the app will stop running sometime during the last day
DWORD CTimeStamp::GetDaysRemaining( void )
{
	DWORD	dwRunDaysLeft = 0L;
	DWORD	dwMaxDaysLeft = 0L;

	if ( IsValid0() ) 
	{
		DWORD now = (DWORD)time( NULL );
		
		if ( m_Stamp.dwDays0 >= 1000 )
			dwRunDaysLeft = 1000;
		else if ( m_EvalEnd0 == m_EvalEnd1	&&
		 		  now < m_EvalEnd0			&&
		 		  now >= m_EvalStart )
			dwRunDaysLeft = (( m_EvalEnd0 - now ) + 86399 ) / 86400;
			
		if ( now < m_Stamp.dwExpire0 )
			dwMaxDaysLeft = (( m_Stamp.dwExpire0 - now ) + 86399 ) / 86400;

		if ( dwRunDaysLeft < dwMaxDaysLeft )
			return dwRunDaysLeft;
		else
			return dwMaxDaysLeft;
		
	}

	return 0;
}

BOOL CTimeStamp::SetDays( DWORD dwRunDays, DWORD dwMaxDays )
{
	m_Stamp.dwDays0 = m_Stamp.dwDays1 = dwRunDays;

	if ( dwMaxDays >= 1000 )
	{
		m_Stamp.dwExpire0 = m_Stamp.dwExpire1 = 0xFEFDEFDC;	// a long time from now...
	}
	else
	{
		DWORD dwSeconds = dwMaxDays * 60L * 60L * 24L;
		m_Stamp.dwExpire0 = m_Stamp.dwExpire1 = time( NULL ) + dwSeconds;
	}

	return TRUE;
}

// helper methods
DWORD CTimeStamp::DecodeDays0( DWORD dwCooked )
{
	return dwCooked >> 5;
}

DWORD CTimeStamp::EncodeDays0( DWORD dwRaw )
{
	DWORD dwTemp = time( NULL );

	dwTemp &= 0x1F;		// we only want the low 5 bits
	dwTemp |= dwRaw << 5;

	return dwTemp;
}

DWORD CTimeStamp::DecodeDays1( DWORD dwCooked )
{
	DWORD dwTemp = ~dwCooked;
	return dwTemp >> 7;
}

DWORD CTimeStamp::EncodeDays1( DWORD dwRaw )
{
	DWORD dwTemp = time( NULL );

	dwTemp &= 0x7F;		// we only want the low 7 bits
	dwTemp |= dwRaw << 7;
	dwTemp = ~dwTemp;

	return dwTemp;
}

DWORD CTimeStamp::DecodeExpire0( DWORD dwCooked )
{
	DWORD dwTemp = dwCooked << 16;		// lsw into msw
	dwTemp |= dwCooked >> 16;			// msw into lsw
	return dwTemp;
}

DWORD CTimeStamp::EncodeExpire0( DWORD dwRaw )
{
	DWORD dwTemp = dwRaw >> 16;
	dwTemp |= dwRaw << 16;
	return dwTemp;
}

DWORD CTimeStamp::DecodeExpire1( DWORD dwCooked )
{
	DWORD dwTemp = dwCooked >> 8;
	dwTemp |= dwCooked << 24;
	dwTemp = ~dwTemp;
	return dwTemp;
}

DWORD CTimeStamp::EncodeExpire1( DWORD dwRaw )
{
	DWORD dwTemp = dwRaw << 8;
	dwTemp |= dwRaw >> 24;			// msb into lsb
	dwTemp = ~dwTemp;
	return dwTemp;
}

DWORD CTimeStamp::CalcSum0( void )
{
	LPBYTE	lpBuf = (LPBYTE)&m_Stamp;
	DWORD	dwSum = SECRET_SEED;
	DWORD	dwTemp;
	int		i, j;

	// this code sums 32 bytes ( sizeof( TIMESTAMP ) - 4 )
	BYTE	arBytes[] = { 1,3,4,3,1,2,3,4,3,1,2,3,2 };

	for ( i = 0; i < sizeof( arBytes ); i++ )
	{
		// get some bytes
		dwTemp = 0L;
		for ( j = 0; j < arBytes[ i ]; j++ )
		{
			dwTemp <<= 8;
			dwTemp += *lpBuf++;
		}

		dwSum += dwTemp;	
	}

	return dwSum;
}

DWORD CTimeStamp::CalcSum1( void )
{
	LPBYTE	lpBuf = (LPBYTE)&m_Stamp;
	DWORD	dwSum = SECRET_SEED;
	DWORD	dwTemp;
	int		i, j;

	// this code sums 32 bytes ( sizeof( TIMESTAMP ) - 4 )
	BYTE	arBytes[] = { 1,3,4,3,1,2,3,4,3,1,2,3,2 };

	for ( i = 0; i < sizeof( arBytes ); i++ )
	{
		// get some bytes
		dwTemp = 0L;
		for ( j = 0; j < arBytes[ i ]; j++ )
		{
			dwTemp <<= 8;
			dwTemp += *lpBuf++;
		}

		dwSum += dwTemp;	
	}

	return dwSum;
}

DWORD CTimeStamp::CalcSum2( void )
{
	LPBYTE	lpBuf = (LPBYTE)&m_Stamp;
	DWORD	dwSum = SECRET_SEED;
	DWORD	dwTemp;
	int		i, j;

	// this code sums 32 bytes ( sizeof( TIMESTAMP ) - 4 )
	BYTE	arBytes[] = { 1,3,4,3,1,2,3,4,3,1,2,3,2 };

	for ( i = 0; i < sizeof( arBytes ); i++ )
	{
		// get some bytes
		dwTemp = 0L;
		for ( j = 0; j < arBytes[ i ]; j++ )
		{
			dwTemp <<= 8;
			dwTemp += *lpBuf++;
		}

		dwSum += dwTemp;	
	}

	return dwSum;
}

DWORD CTimeStamp::CalcFileSum( int fid, DWORD dwBytes )
{
	DWORD dwSum = SECRET_SEED;
	DWORD i;
	char buf[ 0x1000 ];

	long org = tell( fid );
	lseek( fid, 0L, SEEK_SET );

	for ( i = 0; i < dwBytes; i++ )
	{
		// check for buffer fault
		DWORD j = i & 0xFFF;
		if ( j == 0 )
			read( fid, buf, sizeof( buf ) );

		dwSum += buf[ j ];
	}

	lseek( fid, org, SEEK_SET );

	return dwSum;
}
