/*////////////////////////////////////////////////////////////////////////////

NAME:
	CmmioFile - 

FILE:		mmioFile.H
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
	stdafx.h
	lscommon.h
	QPL.h
	LsConvertUnit.h

REVISION:
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/??/96   lss     -Initial
04/11/97   lss     -Added File Format Version (m_FileFormatVersion),
					FileVersionMajor(), and FileVersionMinor
				   -Added backward compatibility option, old2NewFmtChunk(),
				    so we can read outdated file format during the in-house
					testing days.
04/17/97   lss	   -Added type_variableFileInfo
 					
/*////////////////////////////////////////////////////////////////////////////

#ifndef _CMMIOFILE_H_
#define _CMMIOFILE_H_

#ifndef _QPL_H_
#include "QPL.h"
#endif
#include "LsConvertUnit.h"

// DEFINES
#define FILE_VERSION_MAJOR	1
#define FILE_VERSION_MINOR	0

// Only for QcelpFile - 'VRAT' chunk
typedef struct {
	DWORD	variableRate;	// if ( == 0 ) fixed rate, 
							// else if ( < 0xFFFF0000 ) variable rate
	DWORD	sizeInPackets;	// packet data size in packets
} type_variableFileInfo;

typedef struct {
	DWORD	stepSize;		// in 100ms steps
	DWORD	numOfOffsets;	// # of offsets
} type_offsetsFileInfo;

//************************************************************************
// Class  : CmmioFile -- do not use, instead derive 
// Parent : CFile
//************************************************************************
class CmmioFile : public CFile
{
protected:
//	LIFECYCLE
	CmmioFile( LPCTSTR fn = 0 ) : m_hmmio(0), mi_errno(0), 
				  me_mode(0), m_isOpen(FALSE), ms_fileName(fn) {};

public:
//	OPERATIONS
	virtual BOOL Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL ){return FALSE;};
	virtual BOOL Open( UINT nOpenFlags, CFileException* pError = NULL )=0;

//	virtual void Close();

	virtual UINT Read( void* lpBuf, UINT nCount )
			{return mmioRead( m_hmmio, (char *)lpBuf, nCount ); };
	virtual void Write( const void* lpBuf, UINT nCount )
			{mmioWrite( m_hmmio, (char *)lpBuf, nCount );};

	virtual LONG Seek( LONG lOff, UINT nFrom );

	virtual ULONG SeekInTime( ULONG p ) = 0;

	virtual BOOL OverWrite( LPCTSTR lpszFileName, LONG lOff,
							int type=UNIT_BYTES )=0;
    virtual BOOL Insert( LPCTSTR lpszFileName, LONG lOff,
							int type=UNIT_BYTES )=0;
	virtual BOOL Delete( LONG lOff, LONG lLen,
							int type=UNIT_BYTES )=0;

//	ACCESS
	virtual void setInfo( const void *info )=0;
	virtual void getInfo( void *info )=0;
	virtual const void *getInfo()=0;

	virtual DWORD getDataLen( int type=UNIT_BYTES )=0;

	virtual LPCTSTR GetFileName(){return LPCTSTR(ms_fileName);};

	// return converted index of type 'srcType'
	// in units of 'destType'
	virtual unsigned long ConvertUnits(
		int				destType, 
		unsigned long	index,
		int				srcType )
	{
		return m_ConvertUnits.Convert( destType, index, srcType );
	};

	virtual int	getError(){return mi_errno;};

//	ENUMS
	// just to simplify things
	enum {
		READ=1,			 // open file for read
		WRITE=2,		 // open file for write, create if necessary
		CHECK=4,		 // check if it is valid file
		WAVE_SUPPORTED=8,// check if the wave file format is supported on this machine
	};

	// for mi_errno
	enum {
		SUCCESS = 0,
		FILE_FAIL_OPEN,
		FILE_FAIL_CREATE,
		FILE_FAIL_CREATE_VRAT,
		FILE_NOT_WAVE,
		FILE_NOT_QLCM,
		FILE_NOT_SUPPORTED,
		FILE_NO_DATA,
		LAST_INDEX,					// do not use. for house keeping purposes
	};

//////////////////////////////////////////////////////////////////////////////
protected:
//	ATTRIBUTES
	HMMIO       m_hmmio;
    MMCKINFO    m_mmckinfoParent;	// parent chunk information structure
    MMCKINFO    m_mmckinfoSubchunk;	// subchunk information structure    

	CString		ms_fileName;

	int			mi_errno;
	int			me_mode;
	BOOL		m_isOpen;

	LsConvertUnit	m_ConvertUnits;	// for unit conversion

	friend class CmmioFileSupport;
};


//************************************************************************
// Class  : CQcelpFile
// Parent : CmmioFile
//************************************************************************
class CQcelpFile : public CmmioFile
{
public:
//	LIFECYCLE
	CQcelpFile( LPCTSTR fn = 0 ) : CmmioFile( fn ), m_FileFormatVersion(0),
		mp_offsets(0)
		{m_vratChunk.variableRate=0; m_vratChunk.sizeInPackets=0;
		 m_offsChunk.stepSize = 10; m_offsChunk.numOfOffsets = 0;}

//	OPERATIONS
	virtual BOOL Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );
	virtual BOOL Open( UINT nOpenFlags, CFileException* pError = NULL )
				 { return Open( ms_fileName, nOpenFlags ); };
	virtual void Close();

	virtual ULONG SeekInTime( ULONG p );
	virtual ULONG SeekInTime( ULONG* pInMsOutPos );

	virtual BOOL OverWrite( LPCTSTR lpszFileName, LONG lOff,
							int type=UNIT_BYTES );
	virtual BOOL Insert( LPCTSTR lpszFileName, LONG lOff,
							int type=UNIT_BYTES );
	virtual BOOL Delete( LONG lOff, LONG lLen,
							int type=UNIT_BYTES );


	BOOL	Convert2New( LPCTSTR lpszFileName );

	// Only in Write Mode for the next two
	void	OffsetStepSize( int step );
	void	MarkOffset( DWORD offset );

//	ACCESS
	virtual DWORD getDataLen( int type=UNIT_BYTES );	
	virtual void setInfo( const void *info )
		{memcpy( &m_codecInfo, info, sizeof(m_codecInfo) );};
	virtual void getInfo( void *info )
		{memcpy( info, &m_codecInfo, sizeof(m_codecInfo) );};
	virtual const void *getInfo()
		{return &m_codecInfo;};

	// Only available to QcelpFile 
	int  VariableRate(){return m_vratChunk.variableRate;}
	void VariableRate( int rate )
			{m_vratChunk.variableRate = rate;}
	void setPacketCount( ULONG nPackets )
			{m_vratChunk.sizeInPackets = nPackets;}
	BYTE FileVersionMajor(){return HIBYTE(m_FileFormatVersion);}
	BYTE FileVersionMinor(){return LOBYTE(m_FileFormatVersion);}


//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	BOOL	openLoad();
	BOOL	openSave();
	ULONG	_seekInTime( ULONG* inTimeOutPos );

	// for compatibility mode
	typedef struct
	{
		WORD	Version;
		char    Name[80];		
		WORD	avgBitsPerSec;
		WORD	bytesPerPacket;
		WORD	samplesPerBlock;
		WORD	samplesPerSec;	
		WORD	bitsPerSample;
		WORD	pad0;
		struct
		{
			WORD	isVariableRate;	
			WORD	pad1;
			WORD	numOfRates;
			WORD	bytesPerPacket[8];
		} variableRate;
		WORD	pad2;
		WORD	packetSizeLo;
		WORD	packetSizeHi;
	} type_old_info;

	BOOL	old2NewFmtChunk( t_qpl_codec_info& newInfo, type_old_info& oldInfo );
	void	createTempOffs();

//	ATTRIBUTES
	t_qpl_codec_info			m_codecInfo;
	type_variableFileInfo	m_vratChunk;
	type_offsetsFileInfo	m_offsChunk;

	DWORD*					mp_offsets;
	ULONG					m_offsetMaxCnt;
    MMCKINFO				m_mmckinfoSubchunkMisc; 
    MMCKINFO				m_mmckinfoSubchunkVrat; 
	WORD					m_FileFormatVersion;

	friend class CmmioFileSupportQcelp;
};


//************************************************************************
// Class  : CWaveFile
// Parent : CmmioFile
//************************************************************************
class CWaveFile : public CmmioFile
{
public:
//	LIFECYCLE
	CWaveFile( LPCTSTR fn = 0 ) : CmmioFile( fn ) {};

//	OPERATIONS
	virtual BOOL Open( LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );
	virtual BOOL Open( UINT nOpenFlags, CFileException* pError = NULL )
				 { return Open( ms_fileName, nOpenFlags ); };
	virtual void Close();

	virtual BOOL OverWrite( LPCTSTR lpszFileName, LONG lOff,
							int type=UNIT_BYTES );
	virtual BOOL Insert( LPCTSTR lpszFileName, LONG lOff,
							int type=UNIT_BYTES );
	virtual BOOL Delete( LONG lOff, LONG lLen,
							int type=UNIT_BYTES );

	virtual ULONG SeekInTime( ULONG p );

//	ACCESS
	DWORD getDataLen( int type=UNIT_BYTES );	// get size of data chunk.

	virtual void setInfo( const void *info )
		{memcpy( &m_waveFormat, info, sizeof(m_waveFormat) );};
	virtual void getInfo( void *info )
		{memcpy( info, &m_waveFormat, sizeof(m_waveFormat) );};
	virtual const void* getInfo()
		{return &m_waveFormat;};

//	ENUMS
	enum{
		DEVICE_ERROR = LAST_INDEX
	};

//////////////////////////////////////////////////////////////////////////////
private:
//	METHODS
	BOOL		openLoad();
	BOOL		openSave();
	BOOL		isSupported();

//	ATTRIBUTES
	WAVEFORMATEX m_waveFormat;
};

//************************************************************************
// Class  : CmmioFileSupport
// Parent : None
//
// Essentially support for CmmioFile class. Only used by CmmioFile and its
// children
//************************************************************************
class CmmioFileSupport
{
public:
//	LIFECYCLE
	CmmioFileSupport( CmmioFile& srcFile, CmmioFile& tmpFile );

//	OPERATIONS
	BOOL OverWrite( CmmioFile& otherFile, LONG lOff );
	BOOL Insert( CmmioFile& otherFile, LONG lOff );
	BOOL Delete( LONG lOff, LONG lLen );

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	virtual void readWrite( CmmioFile& srcFile, LONG toOff );
	virtual BOOL _open();
	virtual void _close();

//	ATTRIBUTES
	CmmioFile&	mr_srcFile;
	CmmioFile&	mr_tmpFile;
	CString		ms_tmpFileName;

private:
	// do not allow default, copy ctor or assignment
	CmmioFileSupport();
	CmmioFileSupport( const CmmioFileSupport& rhs );
	CmmioFileSupport& operator=( const CmmioFileSupport& rhs );
};

//************************************************************************
// Class  : CmmioFileSupportQcelp
// Parent : CmmioFileSupport
//
// Special case for Qcelp file. When Insert, Delete, or OverWrite operation
// is performed on CQcelpFile, we need to know what the "compressed"
// size would be for the final file. So this information can be used for
// display.
//************************************************************************
class CmmioFileSupportQcelp : public CmmioFileSupport
{
public:
//	LIFECYCLE
	CmmioFileSupportQcelp( CmmioFile& srcFile, CmmioFile& tmpFile )
		: CmmioFileSupport( srcFile, tmpFile ), ml_lengthVariable(0),
		ml_packets(0)
		{ mp_codecInfo = (t_qpl_codec_info*)srcFile.getInfo(); }

//////////////////////////////////////////////////////////////////////////////
protected:
//	METHODS
	virtual void readWrite( CmmioFile& srcFile, LONG toOff );
	virtual BOOL _open();
	virtual void _close();

//	ATTRIBUTES
	unsigned long	ml_lengthVariable;
	unsigned long	ml_packets;
	t_qpl_codec_info*	mp_codecInfo;
	BYTE			m_bytesPerPacket[256];

private:
	// do not allow default, copy ctor or assignment
	CmmioFileSupportQcelp();
	CmmioFileSupportQcelp( const CmmioFileSupportQcelp& );
	CmmioFileSupportQcelp& operator=( const CmmioFileSupportQcelp& );
};

#endif