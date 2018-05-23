// ImapTypes.h -  General purpose classes and structures for use by IMAP.
//

#ifndef __IMAPTYPES_H
#define __IMAPTYPES_H

#pragma warning(disable : 4663)
#pragma warning(disable : 4244)

// #include <map>

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include <algorithm>
#include <map>
#pragma warning (default: 4663 4244)



using namespace std;

class CUidMap : public map<unsigned long, CImapFlags *, less<unsigned long> >
{
public:
	virtual ~CUidMap();

	void DeleteAll ();
	void RemoveAll ();

	BOOL OrderedInsert(unsigned long uid, unsigned long Imflags, BOOL IsNew);
	BOOL OrderedInsert(unsigned long uid,
									BOOL seen,
									BOOL deleted,
									BOOL flagged,
									BOOL answered,
									BOOL draft,
									BOOL recent,
									BOOL IsNew);

	// Utility:
	//
	unsigned long GetHighestUid ();

	int GetCount ()
		{ return size(); }
		
};


typedef CUidMap::iterator			UidIteratorType;
typedef CUidMap::reverse_iterator	ReverseUidIteratorType;


//================================================================//

// Use a MAP to store case sensitive generic strings. We key on the string.
// Don't store any data.
//

// #include "cstring.h"

class CStringMap : public map<CString, unsigned long, less<CString> >
{
public:
	virtual ~CStringMap();

	void DeleteAll ();
	void RemoveAll ();

	int GetCount ()
		{ return size(); }

private:
		
};


typedef CStringMap::iterator			StringMapIteratorType;
typedef CStringMap::reverse_iterator	ReverseStringMapUidIteratorType;



// Exported functions.
//

void MergeUidMapsMT (CUidMap& CurrentUidMap, CUidMap& NewUidMap, CUidMap& NewOldUidMap);
void RemoveSubUidMapMT (CUidMap& MainUidMap,CUidMap& SubUidMap);
void CommaListToUidMap (LPCSTR pStr, CUidMap& DstUidMap);

#endif // __IMAPTYPES_H