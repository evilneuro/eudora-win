//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Name: 					llist.h
// Description:		Interface file for doubly linked-list class
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifndef __LLIST_H__
#define __LLIST_H__

// forward declarations
class CNode;
typedef CNode FAR * LPCNODE;


// (Derive your class from CNode to store it in a list)

///////////////////////////////////////////////////////////////////////
// CNode class - doubly linked list node


class CNode
{
// class data members
public:
	CNode *				pNext;
	CNode *				pPrev;

public:
  // enumerator callback - return 0 to stop enum, return 1 to continue
	typedef BOOL (*PFNENUMCALLBACK)(LPVOID * ppList, LPCNODE pNode, LPVOID pData);
	// different ways to attach a node
	enum eAttachType {APPEND=0, PREPEND, AFTER=0, BEFORE};	 // note APPEND==AFTER, PREPEND==BEFORE

public:
	// constructor
	CNode();

public:
	// public interfaces
	BOOL					Attach(LPVOID *ppList, eAttachType=APPEND, LPCNODE pRefNode=NULL);
	BOOL					Detach(LPVOID *ppList);
	BOOL 					IsAttached() 							{ return (pNext && pPrev); }
	BOOL 					IsHead(LPVOID * ppList) 	{ return (GetHead(ppList) == this); }
	BOOL 					IsTail(LPVOID * ppList) 	{ return (GetTail(ppList) == this); }
static
	CNode * 			GetHead(LPVOID * ppList) 	{ return (ppList && *ppList) ? *((CNode**)ppList) : NULL; }
static
	CNode *				GetTail(LPVOID * ppList) 	{ return (ppList && *ppList) ? (*((CNode**)ppList))->pPrev : NULL; }
static
	BOOL 					Enumerate(LPVOID * ppList, PFNENUMCALLBACK pfnEnumCallback, LPVOID pData)
																					{ return _enumerate(ppList, pfnEnumCallback, pData, FALSE); }
static
	BOOL 					EnumDetach(LPVOID * ppList, PFNENUMCALLBACK pfnEnumCallback, LPVOID pData)
																					{ return _enumerate(ppList, pfnEnumCallback, pData, TRUE); }
static
	CNode *				GetAt(LPVOID * ppList, int nIndex);

#ifdef _DEBUG
	// debugging functions
static
	BOOL 					Dump(LPVOID * ppList, PFNENUMCALLBACK pfnEnumCallback=NULL);
private:
static
	BOOL 					_dump(LPVOID * ppList, LPCNODE pNode, LPVOID pData);
#endif

	// internal methods
protected:
static
  BOOL 					_enumerate(LPVOID * ppList, PFNENUMCALLBACK pfnEnumCallback, LPVOID pData, BOOL bDetach);
};




#endif // __LLIST_H__

