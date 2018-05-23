/*////////////////////////////////////////////////////////////////////////////

File  : CSNDBUF.H

Class : CSndBuffer
Parent: None
  
DESCRIPTION
===========

Sound Buffer maintains a list of frames. Each frame is fixed length
which can be defined by the user. This is specially useful for defining
frame length to match our encoded packet length. This makes
handling data much easier.

REVISION
========
when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/10/96   lss     Initial
  
/*////////////////////////////////////////////////////////////////////////////

#ifndef __CSNDBUF_H
#define __CSNDBUF_H

#include "stdafx.h"
#include <afxmt.h>		// for synchro stuff

//************************************************************************
// Class  : CSndBuffer
// Parent : none
//
//************************************************************************
class CSndBuffer
{
public:
	CSndBuffer(BOOL bSync = FALSE, long elmSizeBytes = 640);
	~CSndBuffer();

	void	Reset();

// retrieve functions
	char	*Get( long index, long lenByte);
	char	*Get( char *dest, long index, long lenByte);

	char	*GetNext( char *data, long lenByte );
	
	// copy 'numFrames' frames into memory pointed to by 'data'
	long	GetFrame( char *data, long numFrames );
	// same as above but after copying, delete 'em
	long	GetFrameDel( char *data, long numFrames );
	long	GetFrameSize(){return ml_elmLenByte;};

// storage functions
	long	Put( char *data, long lenByte ); // append at the end of list
	
	//long	Insert( char *data, long startFrame, long numFrames = 1 );
	//long    OverWrite( char *data, long startFrame, long numFrames = 1 );
	//long    OverWrite( char *data, long lenByte );

// wave format stuff
	void	setWaveFormat( LPWAVEFORMATEX format, int size = sizeof(WAVEFORMATEX) );
	void	getWaveFormat( LPWAVEFORMATEX format, int size = sizeof(WAVEFORMATEX) );
	const	LPWAVEFORMATEX	getWaveFormat();

	long	getCurLen() {return ml_curOverAllLen;};

	BOOL	isSync() {return m_isSync;};
	BOOL	isDone() {return m_isDone;};
//	BOOL	isDoneMinor {return m_isDoneMinor};

	void	Done() {m_isDone = TRUE;};
//	void	DoneMinor() {m_isDoneMinor = TRUE;};
	long	getRecordedLen() {return ml_recordedLen;};

	// syncronization stuff
	CCriticalSection m_cs;
	//CMutex m_cs;

private:
	long		ml_recordedLen;
	CPtrList	m_list;
	long		ml_curOverAllLen;

	char		*_get( char *data, long indexByte, long lenByte );

	// for Put()
	POSITION	m_curElm;
	long		ml_curLenInElm;

	// for OverWriteNext()
	//POSITION	m_curElmOWN;
	//long		ml_curLenInElmOWN;
	//long		ml_curOverAllLenOWN;

	// for GetNext()
	POSITION	m_curElmGN;
	long		ml_curLenInElmGN;
	long		ml_curOverAllLenGN;

	// for GetFrame()
	POSITION	m_curElmF;
	long		ml_curElmIndexF;

	// size of each element's data
	long		ml_elmLenByte;

	// wave format stuff
	LPWAVEFORMATEX	mp_wvFormat;

	// syncro?
	BOOL		m_isSync;

	BOOL		m_isDone;

	struct slist_elm
	{
		char *data;
		//long  len;
		//long  thusfarlen;
	};
};


#endif