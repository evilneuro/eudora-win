// PgStuffBucket.cpp
// big ol' bucket of glue for sticking Paige and Eudora together!

#include "stdafx.h"
#include "PgStuffBucket.h"


#include "DebugNewHelpers.h"


///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

PgStuffBucket::_PgStuffBucket()
{
    kind = kUndefinedType;
    subkind = kUndefinedSubtype;
    pMessage = NULL;
    pWndOwner = NULL;
    hlinkEvent = NULL;
    hlinkProcessing = false;
    bOleExport = false;
    pDS = NULL;
	cUniqueID = 0;
}


// FOR NOW: inline
//_PgStuffBucket::~PgStuffBucket
//{
//}


///////////////////////////////////////////////////////////////////////////////
// Utility/Accessor Methods


// UniqueID:
// returns an id that is unique to this Paige instance

long PgStuffBucket::UniqueID()
{
	return cUniqueID++;
}


// UniqueIDString:
// passes back an hex-string id that is unique to this Paige instance

void PgStuffBucket::UniqueIDString( char* pOutBuf )
{
	assert( pOutBuf );

	if ( pOutBuf ) {
		_ltoa( cUniqueID, pOutBuf, 16 );
		cUniqueID++;
	}
}

