/*////////////////////////////////////////////////////////////////////////////

NAME:
	CmmioFile - 

FILE:		mmioFile.cpp
AUTHOR:		Livingstone Song (lss)
COMPANY:	QUALCOMM, Inc.
TYPE:		C++-CLASS

DESCRIPTION:
	CWaveFile - derived from CmmioFile. File support for WAVE files
	CQcelpFile - derived from CmmioFile. File support for *.qcp files
	CmmioFileSupport - provide Overwrite, Insert, Delete operations for
					   CmmioFile
	CmmioFileSupportQcelp - same as above but for CQcelpFile

RESCRITICTIONS:

FILES:
	mmioFile.h

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/??/96   lss     -Initial
04/11/97   lss     -Added File Format Version (m_FileFormatVersion),
					FileVersionMajor(), and FileVersionMinor()
				   -Added backward compatibility option, old2NewFmtChunk(),
				    so we can read outdated file format during the in-house
					testing days.
04/18/97   lss	   -Modification to handle newer file format.

/*////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "stdafx.h"

// PROJECT INCLUDES
#include "lscommon.h"
#ifdef LS_DEBUG0
	#include "LsDebug.h"
#endif

// LOCAL INCLUDES
#include "mmioFile.h"

//****************************************************************************
// CmmioFile Class Implementation
//****************************************************************************

LONG CmmioFile::Seek( LONG lOff, UINT nFrom )
{
	if ( CFile::end == nFrom )
		return mmioSeek( m_hmmio, lOff, SEEK_END );
	else if ( CFile::begin == nFrom )
		return mmioSeek( m_hmmio, lOff + m_mmckinfoSubchunk.dwDataOffset,
				SEEK_SET );
	else
	 {
		return mmioSeek( m_hmmio, lOff, SEEK_CUR );
	 }
}

//****************************************************************************
// CQcelpFile Class Implementation
//****************************************************************************

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CQcelpFile::Open(
	LPCTSTR lpszFileName, 
	UINT nOpenFlags,
	CFileException* pError
)
{
	BOOL ret = FALSE;
	ms_fileName = lpszFileName;
	CmmioFile::SetFilePath( lpszFileName );
	switch( nOpenFlags )
	 {
		case READ:
			me_mode = READ;
			m_isOpen = openLoad();
			ret = m_isOpen;
		 break;
		case WRITE:
			me_mode = WRITE;
			m_isOpen = openSave();
			ret = m_isOpen;
		 break;
		case CHECK:
			me_mode = CHECK;
			if (openLoad()) ret = TRUE;
			Close();
		 break;
	 }
	m_ConvertUnits.SetInfo( m_codecInfo );
	return ret;
}

void CQcelpFile::Close()
{
	int err;

	if ( me_mode == WRITE )
	 {
		// Ascend out of the "data" subchunk.
		err = mmioAscend( m_hmmio, &m_mmckinfoSubchunk, 0 ); 
		if ( m_vratChunk.variableRate )
		 {
			// ok, create our "bookmark" offset 'offs' chunk
			m_mmckinfoSubchunkMisc.cksize = sizeof( m_offsChunk ) 
							+ (m_offsChunk.numOfOffsets * sizeof(DWORD));
			m_mmckinfoSubchunkMisc.ckid = mmioFOURCC( 'o', 'f', 'f', 's' ); 

			// Create the 'offs' subchunk. The current file position 
			// should be at the beginning of the 'offs' chunk. 
			if ( !mmioCreateChunk(m_hmmio, &m_mmckinfoSubchunkMisc, 0) )
			 { 
				mmioWrite(m_hmmio, (HPSTR) &m_offsChunk, sizeof(m_vratChunk));
				mmioWrite(m_hmmio, (HPSTR) mp_offsets, 
									m_offsChunk.numOfOffsets * sizeof(DWORD)); 
				// Ascend out of the "offs" subchunk.
				err = mmioAscend( m_hmmio, &m_mmckinfoSubchunkMisc, 0 );
			 } 
			// these two snippet is in response to a weird problem that
			// only occurs in win95 where mmioAscend out of parent
			// is not working correctly, specifically when
			// called from CmmioFileQcelpSupport (4/30/97)
			ULONG riffSize = mmioSeek( m_hmmio, 0, SEEK_END ) - 8;
			if ( mmioSeek( m_hmmio, 4, SEEK_SET ) > -1)
				mmioWrite(m_hmmio, (HPSTR) &riffSize, sizeof(ULONG));
//			err = mmioAscend( m_hmmio, &m_mmckinfoParent, 0 );
			if ( m_mmckinfoSubchunkVrat.dwDataOffset > 0 )
			 {
				mmioSeek( m_hmmio, m_mmckinfoSubchunkVrat.dwDataOffset, SEEK_SET );
			// back to beginning of first subchunk
//			mmioSeek( m_hmmio, 12, SEEK_SET );
//			m_mmckinfoSubchunkMisc.cksize = sizeof( m_vratChunk );
//			m_mmckinfoSubchunkMisc.ckid = mmioFOURCC( 'v', 'r', 'a', 't' ); 
//			if ( !(err=mmioDescend(m_hmmio, &m_mmckinfoSubchunkMisc, 
//									&m_mmckinfoParent, MMIO_FINDCHUNK)) )
//			 {
				// Write the "vrat" chunk. 
				if ( mmioWrite(m_hmmio, (HPSTR) &m_vratChunk, sizeof(m_vratChunk)) 
						!= sizeof(m_vratChunk) )
				 { 
					mi_errno = FILE_FAIL_CREATE_VRAT;
				 } 
				// Ascend out of the "vrat" subchunk.
//				err = mmioAscend( m_hmmio, &m_mmckinfoSubchunkMisc, 0 );
			 }

			// set file position at the end so ascending out of QLCM will
			// set the correct QLCM chunk size
//			mmioSeek( m_hmmio, 0, SEEK_END );
		 }
		else
			err = mmioAscend( m_hmmio, &m_mmckinfoParent, 0 );
		// debug
//		char buf[250];
//		sprintf(buf, "err = %d, size = %d", err, m_mmckinfoParent.cksize);
//		AfxGetMainWnd()->SetWindowText(buf);
	 }
	if ( mp_offsets ) 
	 {
		free( mp_offsets );
		mp_offsets = NULL;
	 }
	err = mmioClose( m_hmmio, 0 );
	if ( !err ) m_isOpen = FALSE;
}

// set position to p ms from the beginning of 'data' chunk
// grandularity is defined by the packets/sec
// return the actual position that was set to.
ULONG CQcelpFile::SeekInTime( ULONG p )
{ 
	return _seekInTime( &p );
}

// same as above except that pInMsOutPos will contain the position
// in bytes(in 'data' chunk') after the function call.
ULONG CQcelpFile::SeekInTime( ULONG* pInMsOutPos )
{ 
	return _seekInTime( pInMsOutPos );
}

BOOL CQcelpFile::OverWrite( LPCTSTR lpszFileName,
							LONG lOff, int type /*=UNIT_BYTES*/ )
{
	CQcelpFile tmpFile;
	CQcelpFile otherFile( lpszFileName );
	if ( READ != me_mode ) Close();
	CmmioFileSupportQcelp tmp( *this, tmpFile );
	if ( m_vratChunk.variableRate )
	 {
		if ( UNIT_BYTES != type )
		 {
			if ( !m_isOpen ) Open( READ ); // need to have it open of 'offs'
			lOff = ConvertUnits( UNIT_TIME_MS, lOff, type );
			_seekInTime( (ULONG*)&lOff );
		 }
	 }
	else
		lOff = ConvertUnits( UNIT_BYTES, lOff, type );
	return tmp.OverWrite( otherFile, lOff );
}

BOOL CQcelpFile::Insert( LPCTSTR lpszFileName,
							LONG lOff, int type /*=UNIT_BYTES*/ )
{
	CQcelpFile tmpFile;
	CQcelpFile otherFile( lpszFileName );
	if ( READ != me_mode ) Close();
	CmmioFileSupportQcelp tmp( *this, tmpFile );
	if ( m_vratChunk.variableRate )
	 {
		if ( UNIT_BYTES != type )
		 {
			if ( !m_isOpen ) Open( READ );	// need to have it open of 'offs'
			lOff = ConvertUnits( UNIT_TIME_MS, lOff, type );
			_seekInTime( (ULONG*)&lOff );
		 }
	 }
	else
		lOff = ConvertUnits( UNIT_BYTES, lOff, type );
	return tmp.Insert( otherFile, lOff );
}

BOOL CQcelpFile::Delete( LONG lOff, LONG lLen, int type /*=UNIT_BYTES*/ )
{
	CQcelpFile tmpFile;
	if ( READ != me_mode ) Close();
	CmmioFileSupportQcelp tmp( *this,  tmpFile );
	if ( m_vratChunk.variableRate )
	 {
		if ( UNIT_BYTES != type )
		 {
			if ( !m_isOpen ) Open( READ );	// need to have it open of 'offs'
			lOff = ConvertUnits( UNIT_TIME_MS, lOff, type );
			_seekInTime( (ULONG*)&lOff );
			lLen = ConvertUnits( UNIT_TIME_MS, lLen, type );
			lLen += lOff;
			_seekInTime( (ULONG*)&lLen );
			lLen -= lOff;
		 }
	 }
	else
	 {
		lOff = ConvertUnits( UNIT_BYTES, lOff, type );
		lLen = ConvertUnits( UNIT_BYTES, lLen, type );
	 }
	return tmp.Delete( lOff, lLen );
}

void CQcelpFile::OffsetStepSize( int step )
{
	m_offsChunk.stepSize = step;
	m_offsChunk.numOfOffsets = 0;
	if ( !mp_offsets )	// allocate if this is first time
	 {
		m_offsetMaxCnt = 7200;
		mp_offsets = (DWORD*)malloc( m_offsetMaxCnt * sizeof(DWORD) );
	 }
}

void CQcelpFile::MarkOffset( DWORD offset )
{
	if ( mp_offsets )
	 {
		// need more step sizes?
		if (m_offsChunk.numOfOffsets > m_offsetMaxCnt)
		 {
			mp_offsets = (DWORD*)realloc( mp_offsets, 7200 * sizeof(DWORD) );
			m_offsetMaxCnt += 7200;
		 }
		m_offsChunk.numOfOffsets++;
		mp_offsets[m_offsChunk.numOfOffsets-1] = offset;
	 }
}

BOOL CQcelpFile::Convert2New( LPCTSTR lpszFileName )
{
/*
	if ( !lpszFileName ) return FALSE;

	CQcelpFile tmpFile;

	if ( !m_isOpen )
		if ( !Open(READ) )	return FALSE;
	else
		Seek( 0, CFile::begin );

	tmpFile.setInfo( getInfo() );
	tmpFile.VariableRate( VariableRate() );
	if ( !tmpFile.Open( lpszFileName, WRITE ) )
	 {
		Close();
		return FALSE;
	 }

	tmpFile.OffsetStepSize( 10 );	// 1 second increments

	BYTE bytesPerPacket[256];
	memset( bytesPerPacket, 0, 256 );
	WORD *rate = m_codecInfo.variableRate.bytesPerPacket;
	for ( int i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
	 {
		bytesPerPacket[HIBYTE(rate[i])] = LOBYTE(rate[i]);
	 }

	LONG srcLength = getDataLen();
	LONG len;
	BYTE data[0x10000];
	BYTE *pDataIndex = data;
	int	packetsPerSec = 
				m_codecInfo.samplesPerSec / m_codecInfo.samplesPerBlock;
	ULONG current_size = 0;
	ULONG left_over = 0;
	LONG target_len = 0x10000;
	BYTE packet_size, tmp;
	ULONG packets = 0, lengthVariable = 0;

	while ( srcLength > 0 )
	 {
		if ( target_len > srcLength )
			target_len = srcLength;
		srcLength -= target_len;
		len = Read( &data, target_len );
		current_size = 0;
		while( current_size < len )
		 {
			if ( left_over > 0 )
			 {
				current_size = left_over;
				left_over = 0;
			 }
			packets++;
			if ( !(packets % packetsPerSec) )
				tmpFile.MarkOffset(lengthVariable);
			packet_size = bytesPerPacket[ data[current_size] ];
			VERIFY(packet_size);
			for ( i=1; i <= packet_size/2; i++ )
			 {
				tmp = pDataIndex[i*2-1];
				pDataIndex[i*2-1] = pDataIndex[i*2];
				pDataIndex[i*2] = tmp;
			 }
			packet_size++; // plus 1 for header byte;
			current_size += packet_size;
			lengthVariable += packet_size;
			pDataIndex += current_size;
		 }
		if ( current_size > len )
			left_over = current_size - len;
		tmpFile.Write( &data, len );
	 }

	Close();
	tmpFile.setPacketCount( packets );
	tmpFile.Close();

	return TRUE;
*/
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

DWORD CQcelpFile::getDataLen( int type /* =UNIT_BYTES */ )
{
	if ( !( (me_mode == READ) || (me_mode == CHECK) ||
		 ((me_mode == WRITE) && !m_isOpen) ) ) return 0;

	if ( type == UNIT_BYTES )
		return m_mmckinfoSubchunk.cksize;

	if ( m_vratChunk.variableRate )
	 {
		if ( UNIT_BYTES != type )
			return ConvertUnits( type, m_vratChunk.sizeInPackets, UNIT_PACKETS );
	 }
	//else
	return ConvertUnits( type, m_mmckinfoSubchunk.cksize, UNIT_BYTES );

//	return ConvertUnits( type, m_mmckinfoSubchunk.cksize, UNIT_BYTES );
//	return ConvertUnits( type, m_codecInfo.variableRate.sizeInPackets, UNIT_PACKETS );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOL CQcelpFile::openSave()
{
    // Open the given file for writing with buffered I/O 
    // using the internal buffer. Make the buffer = 64k
	MMIOINFO mminfo;
	memset( &mminfo, 0, sizeof(MMIOINFO) );
	mminfo.cchBuffer = 0xFFFF;
    m_hmmio = mmioOpen( (char *)LPCTSTR(ms_fileName), &mminfo,
					MMIO_CREATE | MMIO_ALLOCBUF | MMIO_READWRITE ); 
 
    if ( m_hmmio == NULL )
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create file."); 
        return FALSE; 
     } 
 
    // Create a "RIFF" chunk with a "QLCM" form type  
    m_mmckinfoParent.fccType = mmioFOURCC('Q', 'L', 'C', 'M'); 
	if ( mmioCreateChunk(m_hmmio, &m_mmckinfoParent, MMIO_CREATERIFF) )
	{  
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create QLCM file."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
    } 
 
    // Assign the size of the "fmt " chunk
    m_mmckinfoSubchunk.cksize = sizeof(m_codecInfo) + sizeof(WORD);

    // Create the "fmt " chunk (form type "fmt "); it must be 
    // a subchunk of the "RIFF" parent chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if ( mmioCreateChunk(m_hmmio, &m_mmckinfoSubchunk, 0) )
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create \"fmt\" chunk"); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     }  

#ifdef BIG_ENDIAN	// non-Intel
	WORD fileFormatVersion = MAKEWORD(FILE_VERSION_MINOR, FILE_VERSION_MAJOR );
#else
	WORD fileFormatVersion = MAKEWORD(FILE_VERSION_MAJOR, FILE_VERSION_MINOR );
#endif

    // Write the "fmt " chunk. 
	if ( mmioWrite(m_hmmio, (HPSTR) &fileFormatVersion, 
			sizeof(WORD)) != sizeof(WORD) )
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to write format chunk(1)."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     } 

	if ( mmioWrite(m_hmmio, (HPSTR) &m_codecInfo, sizeof(m_codecInfo))
			!= sizeof(m_codecInfo) )
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to write format chunk(2)."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     } 
	
    // Ascend out of the "fmt " subchunk.
    mmioAscend( m_hmmio, &m_mmckinfoSubchunk, 0 ); 

	if ( m_vratChunk.variableRate )
	 {
		m_mmckinfoSubchunkVrat.cksize = sizeof( m_vratChunk );
		// Create the 'vrat' subchunk. The current file position 
		// should be at the beginning of the 'vrat' chunk. 
		m_mmckinfoSubchunkVrat.ckid = mmioFOURCC( 'v', 'r', 'a', 't' ); 
		if ( mmioCreateChunk(m_hmmio, &m_mmckinfoSubchunkVrat, 0) )
		 { 
			m_mmckinfoSubchunkVrat.dwDataOffset = 0;
			mi_errno = FILE_FAIL_CREATE_VRAT;
		 } 
		// Write the "vrat" chunk. 
		if ( mmioWrite(m_hmmio, (HPSTR) &m_vratChunk, sizeof(m_vratChunk)) 
				!= sizeof(m_vratChunk) )
		 { 
			m_mmckinfoSubchunkVrat.dwDataOffset = 0;
			mi_errno = FILE_FAIL_CREATE_VRAT;
		 } 
		// Ascend out of the "vrat" subchunk.
		mmioAscend( m_hmmio, &m_mmckinfoSubchunkVrat, 0 ); 
	 }

	m_mmckinfoSubchunk.cksize = 0L;
    // Create the data subchunk. The current file position 
    // should be at the beginning of the data chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC( 'd', 'a', 't', 'a' ); 
	if ( mmioCreateChunk(m_hmmio, &m_mmckinfoSubchunk, 0) )
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create data chunk."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     } 

 	return TRUE;
} 

BOOL CQcelpFile::openLoad()
{
    DWORD			dwFmtSize;		// size of "fmt" chunk               
    char			*buf;			// tmp buffer for reading stuff
	t_qpl_codec_info	*pinfo;			// pointer to memory for "fmt" chunk 

    // Open the given file for reading with buffered I/O 
    // using the default internal buffer. Make the buffer = 64k
	MMIOINFO mminfo;
	memset( &mminfo, 0, sizeof(MMIOINFO) );
	mminfo.cchBuffer = 0xFFFF;
    m_hmmio = mmioOpen( (char *)LPCTSTR(ms_fileName), &mminfo,
		MMIO_READ | MMIO_ALLOCBUF ); 
 
    if ( m_hmmio == NULL )
	 { 
        mi_errno = FILE_FAIL_OPEN;//AfxMessageBox("Failed to open file."); 
        return FALSE; 
     } 
  
    // Locate a "RIFF" chunk with a "QLCM" form type 
    // to make sure the file is a QLCM file. 
    m_mmckinfoParent.fccType = mmioFOURCC( 'Q', 'L', 'C', 'M' ); 
    if (mmioDescend( m_hmmio, (LPMMCKINFO) &m_mmckinfoParent, NULL, 
                    MMIO_FINDRIFF) )
	{ 
        mi_errno = FILE_NOT_QLCM;//AfxMessageBox("This is not a QLCM file."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
    } 
 
    // Find the "fmt " chunk (form type "fmt "); it must be 
    // a subchunk of the "RIFF" parent chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC( 'f', 'm', 't', ' ' ); 
    if ( mmioDescend(m_hmmio, &m_mmckinfoSubchunk, &m_mmckinfoParent, 
                    MMIO_FINDCHUNK) ) 
	 { 
        mi_errno = FILE_NOT_QLCM;//AfxMessageBox("QLCM file has no \"fmt\" chunk"); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     } 
 
    // Get the file format version in the "fmt " chunk.
    if ( mmioRead(m_hmmio, (HPSTR)&m_FileFormatVersion, 
			sizeof(m_FileFormatVersion)) !=
			(LRESULT)sizeof(m_FileFormatVersion) )
	 { 
        mi_errno = FILE_NOT_QLCM;//AfxMessageBox("Failed to read format chunk."); 
        mmioClose( m_hmmio, 0 );
        return FALSE; 
     } 

#ifndef BIG_ENDIAN	
	// if intel then swap bytes
	m_FileFormatVersion = 
		MAKEWORD(HIBYTE(m_FileFormatVersion), LOBYTE(m_FileFormatVersion));
#endif

	// Get the size of the "fmt " chunk--allocate and lock memory for it.  
	dwFmtSize = m_mmckinfoSubchunk.cksize - sizeof(m_FileFormatVersion); 
	buf = new char[dwFmtSize];

	// Read the "fmt " chunk.  
	if ( mmioRead(m_hmmio, (HPSTR) buf, dwFmtSize) != (LRESULT)dwFmtSize )
	 { 
		mi_errno = FILE_NOT_QLCM;//AfxMessageBox("Failed to read format chunk."); 
		mmioClose( m_hmmio, 0 );
		return FALSE; 
	 } 

	// check if this was an outdated file, if so, convert
	if ( FileVersionMinor() == 0xF0 )
	 {
		if ( !old2NewFmtChunk( m_codecInfo, *((type_old_info *)buf) ) )
		 {
			delete buf;
			mi_errno = FILE_NOT_QLCM;
			mmioClose( m_hmmio, 0 );
			return FALSE; 
		 }
	 }	
	else
	 {		
		// Get fmt info
		pinfo = (t_qpl_codec_info *)buf;
		memcpy( &m_codecInfo, pinfo, sizeof (m_codecInfo) );
	 }

	delete buf;

    // Ascend out of the "fmt " subchunk.
    mmioAscend( m_hmmio, &m_mmckinfoSubchunk, 0 ); 

	DWORD oldOffset = m_mmckinfoSubchunk.dwDataOffset - 8;

    // Find the "vrat" chunk (form type "vrat") if exists
    m_mmckinfoSubchunk.ckid = mmioFOURCC( 'v', 'r', 'a', 't' ); 
    if ( !mmioDescend(m_hmmio, &m_mmckinfoSubchunk, &m_mmckinfoParent, 
                    MMIO_FINDCHUNK) ) 
	 {
		// Read the "vrat" chunk.  
		mmioRead(m_hmmio, (HPSTR)&m_vratChunk, sizeof(m_vratChunk) );
		// Ascend out of the "vrat" subchunk.
		mmioAscend( m_hmmio, &m_mmckinfoSubchunk, 0 ); 
		// back to beginning of first subchunk
		mmioSeek( m_hmmio, 12, SEEK_SET );
	 }
	else
	 {
		// make sure we set the file position to the place before 'vrat' find
		// failed since failed mmioDescend operation has file position
		// undefined
		mmioSeek( m_hmmio, oldOffset, SEEK_SET );
	 }

	if ( READ == me_mode )
	 {
		oldOffset = m_mmckinfoSubchunk.dwDataOffset - 8;
		m_offsetMaxCnt = 0;
		// Find the "offs" chunk (form type "offs") if exists
		m_mmckinfoSubchunk.ckid = mmioFOURCC( 'o', 'f', 'f', 's' ); 
		if ( !mmioDescend(m_hmmio, &m_mmckinfoSubchunk, &m_mmckinfoParent, 
						MMIO_FINDCHUNK) ) 
		 {
			ULONG size = m_mmckinfoSubchunk.cksize - sizeof(m_offsChunk);
			if ( size > 0 )
			 {
				// Read the "offs" chunk.  
				mmioRead(m_hmmio, (HPSTR)&m_offsChunk, sizeof(m_offsChunk) );
				m_offsetMaxCnt = size / sizeof(DWORD);
				mp_offsets = (DWORD*)malloc( size );
				m_offsChunk.numOfOffsets = m_offsetMaxCnt;
				mmioRead(m_hmmio, (HPSTR)mp_offsets, size );
			 }
			// Ascend out of the "offs" subchunk
			mmioAscend( m_hmmio, &m_mmckinfoSubchunk, 0 ); 
			// back to beginning of first subchunk
			mmioSeek( m_hmmio, 12, SEEK_SET );
		 }
		else
		 {
			// make sure we set the file position to the place before 'offs' find
			// failed since failed mmioDescend operation has file position
			// undefined
			mmioSeek( m_hmmio, oldOffset, SEEK_SET );
		 }
	 }

    // Find the data subchunk. The current file position 
    // should be at the beginning of the data chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC( 'd', 'a', 't', 'a' ); 
    if ( mmioDescend(m_hmmio, &m_mmckinfoSubchunk, &m_mmckinfoParent, 
                    MMIO_FINDCHUNK) )
	 { 
        mi_errno = FILE_NO_DATA;//AfxMessageBox("file has no data chunk."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     } 

    if ( 0L == m_mmckinfoSubchunk.cksize )
	 { 
        mi_errno = FILE_NO_DATA;//AfxMessageBox("The data chunk contains no data."); 
        mmioClose( m_hmmio, 0 ); 
        return FALSE; 
     }

	if ( READ == me_mode )
	 {
		// create a temp offs if necessary
		if ( (m_vratChunk.variableRate > 0) &&
			 (m_offsetMaxCnt == 0) )
		 {
			 createTempOffs();
		 }
		// back to beginning of 'data' chunk
		mmioSeek( m_hmmio, m_mmckinfoSubchunk.dwDataOffset, SEEK_SET );		
	 }

	return TRUE;
}

// this is used for converting an outdated type_lcp_info to type_lqp_info
// for backwards compatibility
BOOL CQcelpFile::old2NewFmtChunk(
	t_qpl_codec_info& newInfo,
	type_old_info& oldInfo 
)
{
	memset( &newInfo, 0, sizeof(newInfo) );
	if ( FileVersionMajor() == 2 )
	 {
		SetGUID( newInfo.Id, CID_Qcelp13k );
	 }
	else if ( FileVersionMajor() == 3 )
	{
		SetGUID( newInfo.Id, CID_Qcelp13kMinor );
	}
	else return FALSE;

	m_FileFormatVersion		= 0x0009;
	newInfo.Version			= oldInfo.Version;
	strcpy((char*)newInfo.Name, (char*)oldInfo.Name );
	newInfo.avgBitsPerSec	= oldInfo.avgBitsPerSec;
	newInfo.bytesPerPacket	= oldInfo.bytesPerPacket;
	newInfo.samplesPerBlock	= oldInfo.samplesPerBlock;
	newInfo.samplesPerSec	= oldInfo.samplesPerSec;
	newInfo.bitsPerSample	= oldInfo.bitsPerSample;
//	newInfo.variableRate.isVariableRate	= oldInfo.variableRate.isVariableRate;
//	newInfo.variableRate.sizeInPackets	= 
//		MAKELONG(oldInfo.packetSizeLo, oldInfo.packetSizeHi);
	m_vratChunk.variableRate	= oldInfo.variableRate.isVariableRate ? 1 : 0;
	m_vratChunk.sizeInPackets	= 
		MAKELONG(oldInfo.packetSizeLo, oldInfo.packetSizeHi) ;
	newInfo.variableRate.numOfRates		   = oldInfo.variableRate.numOfRates;
	for ( int i=0; i<oldInfo.variableRate.numOfRates; i++ )
		newInfo.variableRate.bytesPerPacket[i] =
			oldInfo.variableRate.bytesPerPacket[i];

	return TRUE;
}

void CQcelpFile::createTempOffs()
{
	int		i, n;
	long	srcLength = 0, real_src_len;
	ULONG	nPackets = 0;
	BYTE	header = 0;
	int		packetsPerSec = 
				m_codecInfo.samplesPerSec / m_codecInfo.samplesPerBlock;
	WORD	*rate = m_codecInfo.variableRate.bytesPerPacket;
	BYTE	bytes_per_packet[256];

	memset( bytes_per_packet, 0, 256 );
	for ( i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
	 {
		bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i]);
	 }

	real_src_len = getDataLen();
	OffsetStepSize( 10 );	// 1 second increments
	while( srcLength < real_src_len )
	 {
		n = Read( &header, 1 );

		if ( n != 1 ) break;
		nPackets++;
		if ( !(nPackets % packetsPerSec) )
			MarkOffset(srcLength);
		srcLength += n;
		if (m_vratChunk.variableRate < 0xFFFF0000)
			n = bytes_per_packet[header];
		else
			n = header;
		if ( (n == 0) || (n >= m_codecInfo.bytesPerPacket)  )
		 {
#ifdef _DEBUG
			TRACE("Error at packet #%d - snagged on %d\n",
				nPackets, header);
			Seek( -32, CFile::current );
			BYTE debug[64];
			n = Read( debug, 64 );
#endif
			break;
		 }
		srcLength += n;
		mmioSeek( m_hmmio, n, SEEK_CUR );
	 }
}

ULONG CQcelpFile::_seekInTime( ULONG* pInTimeOutPos )
{ 
	ULONG p = *pInTimeOutPos;
	ULONG lOff = 0;
	unsigned int min = m_codecInfo.samplesPerBlock * 1000 
						/ m_codecInfo.samplesPerSec;
	*pInTimeOutPos = 0;
	// do we have offset chunk we can use (meaning we have variable length
	// packets)
	if ( (m_offsChunk.numOfOffsets > 0) &&
		 (m_vratChunk.variableRate > 0) )
	 {
		ULONG inSecs = p/1000;	// find the index, truncate
		if ( inSecs > m_offsChunk.numOfOffsets )
			inSecs = m_offsChunk.numOfOffsets;
		if ( inSecs > 0 )
		 {
			// now seek to this position
			lOff = mp_offsets[inSecs-1] + m_mmckinfoSubchunk.dwDataOffset;
			*pInTimeOutPos = mp_offsets[inSecs-1];
			mmioSeek( m_hmmio, lOff, SEEK_SET );
		 }
		// ok, if the position we're seeking is not on the boundary
		// then do finer search
		if ( p %= 1000 )
		 {
			p /= min;
			int n = 0;
			BYTE header = 0;
			WORD *rate = m_codecInfo.variableRate.bytesPerPacket;
			BYTE bytes_per_packet[256];
			memset( bytes_per_packet, 0, 256 );
			for ( int i=0; i < m_codecInfo.variableRate.numOfRates; ++i )
			 {
				bytes_per_packet[HIBYTE(rate[i])] = LOBYTE(rate[i]);
			 }
			for ( i=0; i < p; i++ )
			 {
				n = Read( &header, 1 );
				if ( n != 1 ) break;
				*pInTimeOutPos += n;
				if (m_vratChunk.variableRate < 0xFFFF0000)
					n = bytes_per_packet[header];
				else
					n = header;
				if ( (n == 0) || (n >= m_codecInfo.bytesPerPacket)  )
				 {
#ifdef _DEBUG
					Seek( -32, CFile::current );
					BYTE debug[64];
					n = Read( debug, 64 );
#endif
					break;
				 }
				*pInTimeOutPos += n;
				mmioSeek( m_hmmio, n, SEEK_CUR );
			 }
			p += inSecs*1000;
		 }
	 }
	// must be fixed size packets. if not then we're in trouble	
	else	
	 {
		p /= min;
		lOff = p * (m_codecInfo.bytesPerPacket);
		*pInTimeOutPos = lOff;
		mmioSeek( m_hmmio, lOff+m_mmckinfoSubchunk.dwDataOffset, SEEK_SET );
	 }

	return (p*min);
}

//****************************************************************************
// CWaveFile Class Implementation
//****************************************************************************

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CWaveFile::Open(
	LPCTSTR lpszFileName,
	UINT nOpenFlags,
	CFileException* pError
)
{
	BOOL ret = FALSE;
	ms_fileName = lpszFileName;
	CmmioFile::SetFilePath( lpszFileName );
	switch( nOpenFlags )
	 {
		case READ:
			me_mode = READ;
			m_isOpen = openLoad();
			ret = m_isOpen;
		 break;
		case WRITE:
			me_mode = WRITE;
			m_isOpen = openSave();
			ret = m_isOpen;
		 break;
		case CHECK:
		case WAVE_SUPPORTED:
			me_mode = CHECK;
			if (openLoad()) ret = TRUE;
			Close();
			if ( ret && (nOpenFlags == WAVE_SUPPORTED) )
				ret = isSupported();
		 break;
	 }
	m_ConvertUnits.SetInfo( m_waveFormat );
	return ret;
}

void CWaveFile::Close()
{
	int err;

	if ( me_mode == WRITE )
	 {
		// Ascend out of the "data" subchunk.
		mmioAscend(m_hmmio, &m_mmckinfoSubchunk, 0); 

		// Ascend out of the "RIFF" chunk.
		mmioAscend(m_hmmio, &m_mmckinfoParent, 0); 
		// these two snippet is in response to a weird problem that
		// only occurs in some win95 where mmioAscend out of parent
		// is not working correctly
		ULONG riffSize = mmioSeek( m_hmmio, 0, SEEK_END ) - 8;
		if ( mmioSeek( m_hmmio, 4, SEEK_SET ) > -1)
			mmioWrite(m_hmmio, (HPSTR) &riffSize, sizeof(ULONG));

	 }
	err = mmioClose(m_hmmio, 0);
	if ( !err ) m_isOpen = FALSE;
}

// set position to p ms from the beginning of 'data' chunk
// return the actual position that was set to.
ULONG CWaveFile::SeekInTime( ULONG p )
{ 
	float min = 1000 / m_waveFormat.nSamplesPerSec;
	p = p * (m_waveFormat.wBitsPerSample >> 3) / min;
	mmioSeek( m_hmmio, p+m_mmckinfoSubchunk.dwDataOffset, SEEK_SET );

	return p;
}

BOOL CWaveFile::OverWrite( LPCTSTR lpszFileName,
						  LONG lOff, int type /*=UNIT_BYTES*/ )
{
	CWaveFile tmpFile;
	CWaveFile otherFile( lpszFileName );
	if ( m_isOpen )	Close();
	CmmioFileSupport tmp( *this, tmpFile );
	lOff = ConvertUnits( UNIT_BYTES, lOff, type );
	return tmp.OverWrite( otherFile, lOff );
}

BOOL CWaveFile::Insert( LPCTSTR lpszFileName, 
						  LONG lOff, int type /*=UNIT_BYTES*/ )
{
	CWaveFile tmpFile;
	CWaveFile otherFile( lpszFileName );
	if ( m_isOpen )	Close();
	CmmioFileSupport tmp( *this, tmpFile );
	lOff = ConvertUnits( UNIT_BYTES, lOff, type );
	return tmp.Insert( otherFile, lOff );
}

BOOL CWaveFile::Delete( LONG lOff, LONG lLen, int type /*=UNIT_BYTES*/ )
{
	CWaveFile tmpFile;
	if ( m_isOpen )	Close();
	CmmioFileSupport tmp( *this, tmpFile );
	lOff = ConvertUnits( UNIT_BYTES, lOff, type );
	lLen = ConvertUnits( UNIT_BYTES, lLen, type );
	return tmp.Delete( lOff, lLen );
}

//////////////////////////////////////////////////////////////////////////////
// ACCESS
/////////

DWORD CWaveFile::getDataLen( int type /* =UNIT_BYTES */ )
{
	if ( !((me_mode != WRITE) ||
		 ((me_mode == WRITE) && !m_isOpen)) ) return 0;

	return ConvertUnits( type, m_mmckinfoSubchunk.cksize, UNIT_BYTES );
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOL CWaveFile::openSave()
{
	// MMRESULT	err;

    // Open the given file for writing with buffered I/O 
    // using the default internal buffer. 
    m_hmmio = mmioOpen((char *)LPCTSTR(ms_fileName), NULL, 
						MMIO_CREATE | MMIO_ALLOCBUF | MMIO_WRITE ); 
 
    if ( m_hmmio == NULL )
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create file."); 
        return FALSE; 
     } 
 
    // Create a "RIFF" chunk with a "WAVE" form type  
    m_mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
	if (mmioCreateChunk(m_hmmio, &m_mmckinfoParent, MMIO_CREATERIFF))
	{  
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create wave file."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
    } 
 
    // Assign the size of the "fmt " chunk
    m_mmckinfoSubchunk.cksize = sizeof(m_waveFormat);

    // Create the "fmt " chunk (form type "fmt "); it must be 
    // a subchunk of the "RIFF" parent chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioCreateChunk(m_hmmio, &m_mmckinfoSubchunk, 0))
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create \"fmt\" chunk"); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     }  

    // Write the "fmt " chunk. 
	if (mmioWrite(m_hmmio, (HPSTR) &m_waveFormat, sizeof(m_waveFormat))
					!= sizeof(m_waveFormat))
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to write format chunk."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 
	
    // Ascend out of the "fmt " subchunk.
    mmioAscend(m_hmmio, &m_mmckinfoSubchunk, 0); 
     
	m_mmckinfoSubchunk.cksize = 0L;
    // Create the data subchunk. The current file position 
    // should be at the beginning of the data chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a'); 
	if (mmioCreateChunk(m_hmmio, &m_mmckinfoSubchunk, 0))
	 { 
        mi_errno = FILE_FAIL_CREATE;//AfxMessageBox("Failed to create data chunk."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 

 	return TRUE;
} 

BOOL CWaveFile::openLoad()
{
	//MMRESULT	err;
    DWORD       dwFmtSize;			// size of "fmt" chunk               
    char		*buf;				// tmp buffer for reading stuff
	WAVEFORMATEX*	pformat;		// pointer to memory for "fmt" chunk 

    // Open the given file for reading with buffered I/O 
    // using the default internal buffer. 
    m_hmmio = mmioOpen((char *)LPCTSTR(ms_fileName), NULL,
				MMIO_READ | MMIO_ALLOCBUF); 
 
    if ( m_hmmio == NULL )
	 { 
        mi_errno = FILE_FAIL_OPEN;//AfxMessageBox("Failed to open file."); 
        return FALSE; 
     } 
 
    // Locate a "RIFF" chunk with a "WAVE" form type 
    // to make sure the file is a WAVE file. 
    m_mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
    if (mmioDescend(m_hmmio, (LPMMCKINFO) &m_mmckinfoParent, NULL, 
                    MMIO_FINDRIFF)) 
	 { 
        mi_errno = FILE_NOT_WAVE;//AfxMessageBox("This is not a WAVE file."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 
 
    // Find the "fmt " chunk (form type "fmt "); it must be 
    // a subchunk of the "RIFF" parent chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); 
    if ( mmioDescend(m_hmmio, &m_mmckinfoSubchunk, &m_mmckinfoParent, 
                    MMIO_FINDCHUNK) ) 
	 { 
        mi_errno = FILE_NOT_WAVE;//AfxMessageBox("WAVE file has no \"fmt\" chunk"); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 
 
    // Get the size of the "fmt " chunk--allocate and lock memory for it.  
    dwFmtSize = m_mmckinfoSubchunk.cksize; 
	buf = new char[dwFmtSize];

    // Read the "fmt " chunk.  
    if ( mmioRead(m_hmmio, (HPSTR) buf, dwFmtSize) != (LRESULT)dwFmtSize )
	 { 
        mi_errno = FILE_NOT_WAVE;//AfxMessageBox("Failed to read format chunk."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 
	
	// Get fmt info
	pformat = (WAVEFORMATEX *)buf;

	memcpy( &m_waveFormat, pformat, sizeof (WAVEFORMATEX) );
	delete [] buf;

    // Ascend out of the "fmt " subchunk.
    mmioAscend(m_hmmio, &m_mmckinfoSubchunk, 0); 
     
    // Find the data subchunk. The current file position 
    // should be at the beginning of the data chunk. 
    m_mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a'); 
    if ( mmioDescend(m_hmmio, &m_mmckinfoSubchunk, &m_mmckinfoParent, 
                    MMIO_FINDCHUNK) )
	 { 
        mi_errno = FILE_NO_DATA;//AfxMessageBox("WAVE file has no data chunk."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 
 
    if ( 0L == m_mmckinfoSubchunk.cksize )
	 { 
        mi_errno = FILE_NO_DATA;//AfxMessageBox("The data chunk contains no data."); 
        mmioClose(m_hmmio, 0); 
        return FALSE; 
     } 

	return TRUE;
}

BOOL CWaveFile::isSupported()
{
	MMRESULT err;

	// make sure this format is supported
	if ((err=waveOutOpen(NULL,              // NULL for query 
            WAVE_MAPPER,                    // device identifier 
            &m_waveFormat,					// requested format 
            NULL,                           // no callback 
            NULL,                           // no instance data 
            WAVE_FORMAT_QUERY))				// query only
			!= MMSYSERR_NOERROR )
	 {
		if ( err == WAVERR_BADFORMAT )
			mi_errno = FILE_NOT_SUPPORTED;
		else
			mi_errno = DEVICE_ERROR;
		return FALSE;             
	 }

	return TRUE;
}

//****************************************************************************
// CmmioFileSupport Class Implementation
//****************************************************************************
#define BLOCK_SIZE 65536

//////////////////////////////////////////////////////////////////////////////
// LIFECYCLE
////////////

CmmioFileSupport::CmmioFileSupport( CmmioFile& srcFile, CmmioFile& tmpFile )
: mr_srcFile(srcFile), mr_tmpFile(tmpFile)
{
	char tempName[MAX_PATH];
	char tempPath[MAX_PATH];

	::GetTempPath( sizeof(tempPath), tempPath );
	::GetTempFileName( tempPath, "~qc", 0, tempName );
	::DeleteFile( tempName );	// delete file, we just want the name

	ms_tmpFileName = tempName;
}

//////////////////////////////////////////////////////////////////////////////
// OPERATIONS
/////////////

BOOL CmmioFileSupport::Delete( LONG lOff, LONG lLen )
{
	if ( _open() == FALSE ) return FALSE;

	// ok, now copy our source file from beginning to lOff
	readWrite( mr_srcFile, lOff );

	// if delete was not to end of file, then copy rest of it
	mr_srcFile.Seek( lLen, SEEK_CUR );
	readWrite( mr_srcFile, mr_srcFile.getDataLen() - lOff - lLen );
	mr_srcFile.Close();

	// ok we're done copying our current file minus the delete region
	// to our temp file. now delete our current file and rename
	// our temp file to our current file name. and presto!
	_close();

    return TRUE;
}

BOOL CmmioFileSupport::OverWrite( CmmioFile& otherFile, LONG lOff )
{
	if ( _open() == FALSE ) return FALSE;

	if ( !otherFile.Open( CmmioFile::READ ) ) return FALSE;

	// ok, now copy our current file from beginning to lOff
	readWrite( mr_srcFile, lOff );
	mr_srcFile.Close();

	readWrite( otherFile, otherFile.getDataLen() );	
	otherFile.Close();

/*
	if ( (lOff + otherFile.getDataLen()) < mr_srcFile.getDataLen() )
	 {
		mr_srcFile.Seek( otherFile.getDataLen() );
		readWrite( mr_srcFile, mr_srcFile.getDataLen() - lOff -
			otherFile.getDataLen() );
	 }

	mr_srcFile.Close();
*/
	_close();

    return TRUE;
}

BOOL CmmioFileSupport::Insert( CmmioFile& otherFile, LONG lOff )
{
	if ( _open() == FALSE ) return FALSE;

	if ( !otherFile.Open( CmmioFile::READ ) ) return FALSE;

	// ok, now copy our current file from beginning to lOff
	readWrite( mr_srcFile, lOff );

	readWrite( otherFile, otherFile.getDataLen() );	
	otherFile.Close();

	readWrite( mr_srcFile, mr_srcFile.getDataLen()-lOff );	
	mr_srcFile.Close();

	_close();

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void CmmioFileSupport::readWrite( CmmioFile& srcFile, LONG toOff )
{
	LONG len;
	BYTE data[BLOCK_SIZE];
	LONG target_len = BLOCK_SIZE;

	while ( toOff > 0 )
	 {
		if ( target_len > toOff )
			target_len = toOff;
		toOff -= target_len;
		len = srcFile.Read( &data, target_len );
		mr_tmpFile.Write( &data, len );
	 }
}

BOOL CmmioFileSupport::_open()
{
	mr_tmpFile.setInfo( mr_srcFile.getInfo() );

	// open a temp and our source file if not opened already
	if ( !mr_tmpFile.Open( LPCTSTR(ms_tmpFileName), CmmioFile::WRITE ) )
	 {
		return FALSE;
	 }
	if ( !mr_srcFile.m_isOpen )
	 {
		if ( !mr_srcFile.Open( CmmioFile::READ ) )
		 {
			mr_tmpFile.Close();
			return FALSE;
		 }
	 }
	else
		mr_srcFile.Seek( 0, CFile::begin );

	return TRUE;
}

void CmmioFileSupport::_close()
{
	mr_tmpFile.Close();
	BOOL err = ::DeleteFile( mr_srcFile.GetFileName() );
#ifdef LS_DEBUG0
	if ( !err )
	 {
		LsDebug err;
		err.SystemErrorMsgBox("CmmioFileSupport::_close : DeleteFile Failed");
	 }
#endif
	err = ::MoveFileEx( LPCTSTR(ms_tmpFileName), mr_srcFile.GetFileName(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED );
	// ok, most likely we're using Win95 which don't support
	// MoveFileEx
	if ( !err )
	 {
		err = ::MoveFile( LPCTSTR(ms_tmpFileName), mr_srcFile.GetFileName() );
		if ( !err )
		 {
			// ok, most likely we're doing cross volume move,
			// so do copy and delete
			err = ::CopyFile( LPCTSTR(ms_tmpFileName), mr_srcFile.GetFileName(), FALSE );
			err = ::DeleteFile( LPCTSTR(ms_tmpFileName) );
#ifdef LS_DEBUG0
	if ( !err )
	 {
		LsDebug err;
		err.SystemErrorMsgBox("CmmioFileSupport::_close : CopyFile/MoveFile/DeleteFile Failed");
	 }
#endif
		 }
	 }

}

//****************************************************************************
// CmmioFileSupportQcelp Class Implementation
//****************************************************************************
/*
BOOL CmmioFileSupportQcelp::OverWrite( CmmioFile& otherFile, LONG lOff )
{
	if ( _open() == FALSE ) return FALSE;

	if ( !otherFile.Open( CmmioFile::READ ) ) return FALSE;

	// ok, now copy our current file from beginning to lOff
	readWrite( mr_srcFile, lOff );
//	mr_srcFile.Close();

	readWrite( otherFile, otherFile.getDataLen() );	
	otherFile.Close();


	if ( (lOff + otherFile.getDataLen()) < mr_srcFile.getDataLen() )
	 {
		mr_srcFile.SeekInTime( otherFile.getDataLen() );
		readWrite( mr_srcFile, mr_srcFile.getDataLen() - lOff -
			otherFile.getDataLen() );
	 }

	mr_srcFile.Close();

	_close();

    return TRUE;
}
*/
void CmmioFileSupportQcelp::readWrite( CmmioFile& srcFile, LONG tmpOff )
{
	// check if it is a variable rate, if not then do the regular
	if ( ((CQcelpFile*)&srcFile)->VariableRate() == 0 )
	 {
		CmmioFileSupport::readWrite( srcFile, tmpOff );
		return;
	 }

	LONG len;
	BYTE data[BLOCK_SIZE];
	int i;
	int	packetsPerSec = 
				mp_codecInfo->samplesPerSec / mp_codecInfo->samplesPerBlock;

	ULONG current_size = 0;
	ULONG left_over = 0;
	LONG target_len = BLOCK_SIZE;
	BYTE header = 0;
	while ( tmpOff > 0 )
	 {
		if ( target_len > tmpOff )
			target_len = tmpOff;
		tmpOff -= target_len;
		len = srcFile.Read( &data, target_len );
		current_size = 0;
		while( current_size < len )
		 {
			if ( left_over > 0 )
			 {
				current_size = left_over;
				left_over = 0;
			 }
			header = data[current_size];
			ml_packets++;
			if ( !(ml_packets % packetsPerSec) )
				((CQcelpFile*)&mr_tmpFile)->MarkOffset(ml_lengthVariable);
			i = m_bytesPerPacket[header];

			VERIFY(i);
			current_size += i;
			ml_lengthVariable += i;
		 }
		if ( current_size > len )
			left_over = current_size - len;
		mr_tmpFile.Write( &data, len );
	 }
}

BOOL CmmioFileSupportQcelp::_open()
{
	mr_tmpFile.setInfo( mr_srcFile.getInfo() );

	// open a temp and our source file if not opened already
	if ( !((CQcelpFile*)&mr_srcFile)->m_isOpen )
	 {
		if ( !mr_srcFile.Open( CmmioFile::READ ) )
		 {
			return FALSE;
		 }
	 }
	else
		mr_srcFile.Seek( 0, CFile::begin );
	
	((CQcelpFile*)&mr_tmpFile)->VariableRate(
				((CQcelpFile*)&mr_srcFile)->VariableRate() );
	
	if ( !mr_tmpFile.Open( LPCTSTR(ms_tmpFileName), CmmioFile::WRITE ) )
	 {
		mr_srcFile.Close();
		return FALSE;
	 }

	((CQcelpFile*)&mr_tmpFile)->OffsetStepSize( 10 );	// 1 second increments
	
	ml_packets = 0;
	ml_lengthVariable = 0;

	memset( m_bytesPerPacket, 0, 256 );
	WORD *rate = mp_codecInfo->variableRate.bytesPerPacket;
	for ( int i=0; i < mp_codecInfo->variableRate.numOfRates; ++i )
	 {
		m_bytesPerPacket[HIBYTE(rate[i])] = LOBYTE(rate[i])+1;// plus 1 for header byte
	 }
	
	return TRUE;
}

void CmmioFileSupportQcelp::_close()
{
	((CQcelpFile*)&mr_tmpFile)->setPacketCount( ml_packets );
	CmmioFileSupport::_close();
}
