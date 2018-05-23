#include "CSndBuf.h"

#include "stdlib.h"
#include "memory.h"

#include "DebugNewHelpers.h"

//************************************************************************
// CSndBuffer Class Implementation
//************************************************************************

CSndBuffer::CSndBuffer(BOOL bSync, long elmSizeBytes)
{
	m_isSync = bSync;		// syncronization?

	// the data size (in bytes) for each element in the list
	ml_elmLenByte = elmSizeBytes; // size of each elements in bytes
	
	Reset();
}

CSndBuffer::~CSndBuffer()
{
	Reset();
}

void CSndBuffer::Reset()
{
	m_isDone = FALSE;

	ml_recordedLen = 0;
	ml_curOverAllLen = 0;
	
	// these 2 are used for Put() book keeping
	// m_curElm = 0;
	ml_curLenInElm = 0;

	// these 3 are used for OverWriteNext() book keeping
	//m_curElmOWN = NULL;		 // current element in the list
	//ml_curLenInElmOWN = 0;   // current length in the element
	//ml_curOverAllLenOWN = 0; // current overall length

	// these 3 are used for GetNext() book keeping
	m_curElmGN = NULL;		// current element in the list
	ml_curLenInElmGN = 0;   // current length in the element
	ml_curOverAllLenGN = 0;	// current overall length

	// these 2 are used for GetFrame() book keeping
	m_curElmF		= NULL;	// points to latest read element
	ml_curElmIndexF	= 0;	// index to next unread element

	if (!m_list.IsEmpty())
	 {
		POSITION pos;
		slist_elm *elm;

		for( pos = m_list.GetHeadPosition(); pos != NULL; )
		 {
			elm = (slist_elm *)m_list.GetNext( pos );
			delete [] elm->data;
			delete elm;
		 }
		m_list.RemoveAll();
	 }
}

char *CSndBuffer::Get( char *dest, long indexByte, long lenByte )
{
	return _get( dest, indexByte, lenByte );
}

char *CSndBuffer::Get( long indexByte, long lenByte )
{

	char *data = new char[lenByte];
	return _get( data, indexByte, lenByte );
}

long CSndBuffer::Put( char *data, long lenByte )
{	
	// syncro stuff
	CSingleLock sLock(&m_cs);
	if (m_isSync) {
		sLock.Lock();
	}

	slist_elm *elm;
	long l = ml_elmLenByte - ml_curLenInElm;
	
	// first check if our current element in the list is full or not
	// if not full then fill in the rest of the current element
	// if this is the first call, then skip it
	if ( l && (ml_curOverAllLen > 0)  )
	{
		long len;
		elm = (slist_elm *) m_list.GetAt ( m_curElm );
		// ok, if the data we're copying and stuffing is smaller
		// than the current available space in the element then
		// adjust accordingly
		if (lenByte <= l)
			len = lenByte;
		else
			len = l;
		memcpy ( elm->data + ml_curLenInElm, data, len );
		lenByte -= len;		// update
		data += len;		// update
		ml_curLenInElm += len;
		ml_curOverAllLen += len;
		ml_recordedLen += len;
	}
	// was that enough? if not then create more elements as needed
	// and fill those
	if (lenByte)
	{
		while (1)
		{
			elm = new slist_elm;
			elm->data = new char[ml_elmLenByte];
			if ( !elm || !elm->data) return -1;
			if (lenByte > ml_elmLenByte)
			{
				memcpy ( elm->data, data, ml_elmLenByte );
				m_list.AddTail ( elm );
				ml_curOverAllLen += ml_elmLenByte;	// update overall len
				ml_recordedLen += ml_elmLenByte;
			}
			else
			 {
				memcpy ( elm->data, data, lenByte );
				m_curElm = m_list.AddTail ( elm );
				ml_curLenInElm = lenByte;
				ml_curOverAllLen += ml_curLenInElm;	// update overall len
				ml_recordedLen += ml_curLenInElm;
				break;
			 }
			lenByte -= ml_elmLenByte;
			data += ml_elmLenByte;
		}
	 }
	
// Unlock called in CSingleLock destructor
//	if (m_isSync)
//		sLock.Unlock();

	return ml_curOverAllLen;

/*
	slist_elm *elm = new slist_elm;
	elm->data = new char[lenByte];
	elm->len = lenByte;
	if (
	elm->thusfarlen = lenByte;

	memcpy( elm->data, data, lenByte );
	m_list.AddTail( elm );
*/
}
/*
long CSndBuffer::OverWriteNext( char *data, long lenByte )
{
	// syncro stuff
	CSingleLock sLock(&m_cs);
	if (m_isSync) {
		sLock.Lock();
	}

	long curOverAllLen		= ml_curOverAllLen;
	long curOverAllLenOWN	= ml_curOverAllLenOWN;

	if ( curOverAllLen == 0 ) return 0; //hey, list is empty
	
	// check lenByte and adjust it if overflow occurs
	if ( curOverAllLen < (curOverAllLenOWN + lenByte) )
		lenByte = curOverAllLen - curOverAllLenOWN;	

	if ( lenByte < 1 ) return 0; // hey, overflow dude!

	slist_elm *elm;
	char *start_of_data = data;
	long l = ml_elmLenByte - ml_curLenInElmOWN;
	long len;

	// is this a first time call? then get head element in the list
	if ( m_curElmOWN == NULL )
	 {
		m_curElmOWN = m_list.GetHeadPosition();
	 }
	
	// do we need to goto the next element?
	if ( !l )
	 {
		l = ml_elmLenByte;
		m_list.GetNext ( m_curElmOWN );
	 }

	elm = (slist_elm *) m_list.GetAt ( m_curElmOWN );

	long retLen = lenByte;

	// ok, if the data we're getting is smaller
	// than what we have in the current element in the list
	// then adjust accordingly
	if (lenByte <= l)
		len = lenByte;
	else
		len = l;
	memcpy ( elm->data + ml_curLenInElmOWN, data, len );
	lenByte -= len;		// update
	data += len;		// update
	ml_curOverAllLenOWN += len;
	ml_curLenInElmOWN += len;

	// was that enough? if not goto next elements as needed
	// and get those
	if (lenByte)
	 {
		m_list.GetNext( m_curElmOWN ); // advance m_curElmGN
		while (1)
		 {
			if (lenByte > ml_elmLenByte)
			 {
				// get next element
				elm = (slist_elm *) m_list.GetNext( m_curElmOWN );
				memcpy ( elm->data, data, ml_elmLenByte );
				ml_curOverAllLenOWN += len;
			 }
			else
			 {
				// get next element
				elm = (slist_elm *) m_list.GetAt( m_curElmOWN );
				memcpy ( elm->data, data, lenByte );
				ml_curLenInElmOWN = lenByte;
				ml_curOverAllLenOWN += len;
				break;
			 }
			lenByte -= ml_elmLenByte;
			data += ml_elmLenByte;
		}
	 }
	
// Unlock called in CSingleLock destructor
//	if (m_isSync)
//		sLock.Unlock();

	return retLen;
}
*/
char *CSndBuffer::GetNext( char *data, long lenByte )
{
	// syncro stuff
	CSingleLock sLock(&m_cs);
	if (m_isSync) {
		sLock.Lock();
	}

	long curOverAllLen		= ml_curOverAllLen;
	long curOverAllLenGN	= ml_curOverAllLenGN;

	if ( curOverAllLen == 0 ) return NULL; //hey, list is empty
	
	// check lenByte and adjust it if overflow occurs
	if ( curOverAllLen < (curOverAllLenGN + lenByte) )
		lenByte = curOverAllLen - curOverAllLenGN;	

	if ( lenByte < 1 ) return NULL; // hey, overflow dude!

	slist_elm *elm;
	char *start_of_data = data;
	long l = ml_elmLenByte - ml_curLenInElmGN;
	long len;

	// is this a first time call? then get head element in the list
	if ( m_curElmGN == NULL )
	 {
		m_curElmGN = m_list.GetHeadPosition();
	 }
	
	// do we need to goto the next element?
	if ( !l )
	 {
		l = ml_elmLenByte;
		m_list.GetNext ( m_curElmGN );
	 }

	elm = (slist_elm *) m_list.GetAt ( m_curElmGN );

	// ok, if the data we're getting is smaller
	// than what we have in the current the element in the list
	// then adjust accordingly
	if (lenByte <= l)
		len = lenByte;
	else
		len = l;
	memcpy ( data, elm->data + ml_curLenInElmGN, len );
	lenByte -= len;		// update
	data += len;		// update
	ml_curOverAllLenGN += len;
	ml_curLenInElmGN += len;

	// was that enough? if not goto next elements as needed
	// and get those
	if (lenByte)
	 {
		m_list.GetNext( m_curElmGN ); // advance m_curElmGN
		while (1)
		 {
			if (lenByte > ml_elmLenByte)
			 {
				// get next element
				elm = (slist_elm *) m_list.GetNext( m_curElmGN );
				memcpy ( data, elm->data, ml_elmLenByte );
				ml_curOverAllLenGN += len;
			 }
			else
			 {
				// get next element
				elm = (slist_elm *) m_list.GetAt( m_curElmGN );
				memcpy ( data, elm->data, lenByte );
				ml_curLenInElmGN = lenByte;
				ml_curOverAllLenGN += len;
				break;
			 }
			lenByte -= ml_elmLenByte;
			data += ml_elmLenByte;
		}
	 }
	
// Unlock called in CSingleLock destructor
//	if (m_isSync)
//		sLock.Unlock();

	return start_of_data;		
}

long CSndBuffer::GetFrame( char *data, long numFrames )
{
	// syncro stuff
	CSingleLock sLock(&m_cs);
	if (m_isSync) {
		sLock.Lock();
	}

	if ( ml_curOverAllLen == 0 ) return 0; // hey, list is empty
	
	long curCount = m_list.GetCount();
	
	// check for overflow
	if ( curCount < (ml_curElmIndexF + numFrames) )
		numFrames = curCount - ml_curElmIndexF;	

	if ( numFrames < 1 ) return 0; // hey, overflow dude!

	slist_elm *elm;

	// is this a first time call? then get head element in the list
	if ( m_curElmF == NULL )
	 {
		m_curElmF = m_list.GetHeadPosition();
	 }
	else
		m_list.GetNext ( m_curElmF ); // onto next element
	
	for ( long l = 0; l < numFrames; l++ )
	 {
		if (l) m_list.GetNext ( m_curElmF );
		elm = (slist_elm *) m_list.GetAt ( m_curElmF );
		memcpy ( data, elm->data, ml_elmLenByte );
		ml_curElmIndexF++;
		data += ml_elmLenByte;
	 }

// Unlock called in CSingleLock destructor
//	if (m_isSync)
//		sLock.Unlock();

	return numFrames;	
}

long CSndBuffer::GetFrameDel( char *data, long numFrames )
{
	// syncro stuff
	CSingleLock sLock(&m_cs);
	if (m_isSync) {
		sLock.Lock();
	}

	if ( ml_curOverAllLen == 0 ) return 0; // hey, list is empty
	
	long curCount = m_list.GetCount();
	
	// adjust for overflow
	if ( curCount < numFrames )
		numFrames = curCount;

	slist_elm *elm;
	POSITION HeadElm;

	HeadElm = m_list.GetHeadPosition();

	for ( long l = 0; l < numFrames; l++ )
	 {
		elm = (slist_elm *) m_list.GetNext ( HeadElm );
		memcpy ( data, elm->data, ml_elmLenByte );
		ml_curOverAllLen -= ml_elmLenByte;
		data += ml_elmLenByte;
		ml_curElmIndexF--;
		delete [] elm->data;
		delete elm;
		m_list.RemoveHead(); 
	 }

	if (ml_curElmIndexF < 0) ml_curElmIndexF = 0;
	// adjust our list's house keeping variables
	if (ml_curOverAllLen < 1)
	 {
		m_curElm = NULL;
		ml_curOverAllLen = 0;
		ml_curLenInElm = 0;

		m_curElmF = 0;
		ml_curElmIndexF = 0;
	 }
	
// Unlock called in CSingleLock destructor
//	if (m_isSync)
//		sLock.Unlock();

	return numFrames;	
}

char *CSndBuffer::_get( char *data, long indexByte, long lenByte )
{
	slist_elm *elm;
	long startPosInElm;
	long elm_index;
	long len;
	long elm_index_last;
	long curOverAllLen;
	long curLenInElm;

	// syncro stuff
	CSingleLock sLock(&m_cs);
	if (m_isSync) sLock.Lock();
	
	curOverAllLen = ml_curOverAllLen;
	curLenInElm = ml_curLenInElm;
	elm_index_last = m_list.GetCount() - 1;
	
	// check lenByte and adjust it if overflow occurs
	if (curOverAllLen < (indexByte + lenByte))
		lenByte = curOverAllLen - indexByte;

	if ( lenByte < 1 ) return NULL;	// hey, overflow dude!

	// ok, first find the starting element
	elm_index = int(indexByte / ml_elmLenByte);

	// find the starting position in the element
	startPosInElm = indexByte - (elm_index * ml_elmLenByte);
	
	// is it available all with one element?
	// if so then copy and return it
	// doesn't matter if this is the non-full last element in the list
	// the above overflow check saves our butt
	len = ml_elmLenByte - startPosInElm; 
	if ( lenByte <= len )
	 {
		elm = (slist_elm *) m_list.GetAt ( m_list.FindIndex(elm_index) );
		memcpy( data, (elm->data + startPosInElm), lenByte );
	 }
	else
	 {
	// guess not... it spans more than one element
	// so let's traverse and copy them
	// the above will guarantee that we won't be starting with the last element
		char *curData = data;
		while (lenByte > len)
		{
			lenByte -= len;
			elm = (slist_elm *) m_list.GetAt ( m_list.FindIndex(elm_index) );
			memcpy( curData, (elm->data + ml_elmLenByte - len), len );
			curData += len;
			++elm_index;
			if (elm_index < elm_index_last)
				len = ml_elmLenByte;		
			else
				len = curLenInElm;			// last element
		}
		// ok, finish off the last element
		elm = (slist_elm *) m_list.GetAt ( m_list.FindIndex(elm_index) );
		memcpy( curData, elm->data, lenByte );
	 }
	
// Unlock called in CSingleLock destructor
//	if (m_isSync)
//		sLock.Unlock();

	return data;
}

void CSndBuffer::setWaveFormat(LPWAVEFORMATEX format, int size)
{
	memcpy( mp_wvFormat, format, size );
}

void CSndBuffer::getWaveFormat(LPWAVEFORMATEX format, int size)
{
	memcpy( format, mp_wvFormat, size );
}

const LPWAVEFORMATEX CSndBuffer::getWaveFormat()
{
	return mp_wvFormat;
}