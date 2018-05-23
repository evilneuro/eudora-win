// MyTypes.h -  General purpose classes and structures for use by IMAP.
//

#ifndef __IMAPDLLTYPES_H
#define __IMAPDLLTYPES_H

#pragma warning(disable : 4663)
#pragma warning(disable : 4244)

// #include <map>

#include <xstddef>
#pragma warning (disable: 4663 4244)
#include <algorithm>
#include <map>
#pragma warning (default: 4663 4244)


using namespace std;

// This is keyed using a message number, and the stored value is a UID.
//
class CMsgMap : public map<unsigned long, unsigned long, less<unsigned long> >
{
public:
	~CMsgMap();

	void RemoveAll ();

	// Utility:
	//
	unsigned long GetHighestUid ();

	int GetCount ()
		{ return size(); }

};


typedef CMsgMap::iterator			UidIteratorType;
typedef CMsgMap::reverse_iterator	ReverseUidIteratorType;



//=============== CPtrMsgList interface ==========================//

class CPtrMsgList  : public CPtrList
{
public:
	CPtrMsgList();
	~CPtrMsgList();

	// Insert in ascending order.
	//
	POSITION Append(unsigned long msgno, unsigned long Uid);

	// Return the removed UID.
	//
	unsigned long Remove (unsigned long msgno);

	// Return the UID.
	unsigned long FindUid (unsigned long msgno);

	// Delete data plus do a RemoveAll().
	void DeleteAll();

private:
	// Critical section guarding access to m_msgMap.
	//
	CRITICAL_SECTION	m_hMsgLockable;

};



#endif // __IMAPDLLTYPES_H
