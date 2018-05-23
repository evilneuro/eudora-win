//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Name: 					llist.cpp
// Description:		Implements doubly linked-list node class
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include "DebugNewHelpers.h"
#include "llist.h"


CNode::CNode()
{
	pNext = NULL;
	pPrev = NULL;
}

BOOL CNode::Attach(LPVOID * ppList, eAttachType eType/*=APPEND*/, LPCNODE pRefNode/*=NULL*/)
{
	if (!ppList)
		return FALSE;

	// special case if empty list
	if (!*ppList){
		*ppList = this;
		pNext = this;
		pPrev = this;
		return TRUE;
	}

	CNode *pHead=NULL, *pTail=NULL;

	// inserting before or after a reference node?
	if (pRefNode) {
		CNode *pRef = pRefNode;

		if (eType == AFTER)														
			pRef = pRef->pNext;				// skip to next node to do insert after
		pHead = pRef;
		pTail = pRef->pPrev;
	}
	// otherwise we are appending or prepending
	else{
		pHead = GetHead(ppList);
		pTail = GetTail(ppList);
	}
		
	if (!pHead || !pTail)
		return FALSE;
	
	// fix up the links
	pHead->pPrev = this;
	pTail->pNext = this;
	pPrev = pTail;
	pNext = pHead;

	// adjust the list pointer if prepending or 
	// if doing an explicit insert before the head
	if ((eType == PREPEND && !pRefNode ) ||
			(eType == BEFORE  && pRefNode && pRefNode->IsHead(ppList)))
		*ppList = this;

	return TRUE;
}


BOOL CNode::Detach(LPVOID * ppList)
{
	if (!ppList || !IsAttached())
		return FALSE;

	// adjust the list pointer if we are the first node
	if (IsHead(ppList))
		*ppList = (pNext != this) ? pNext : NULL;
	
	pNext->pPrev = pPrev;
	pPrev->pNext = pNext;
	pNext = pPrev = NULL;

	return TRUE;
}


// static
CNode *	CNode::GetAt(LPVOID * ppList, int nIndex)
{
	if (!ppList)
		return NULL;
	
	CNode 	*p=NULL, *pTail=GetTail(ppList);
	int 		i;
	
	for (i=0, p=GetHead(ppList); p; i++, p=p->pNext) {
		if (i == nIndex)
			return p;
		if (p == pTail)
			break;
	}

	return NULL;
}


// static
BOOL CNode::_enumerate(LPVOID * ppList, PFNENUMCALLBACK pfnEnumCallback, LPVOID pData, BOOL bDetach)
{
	if (!ppList)
		return FALSE;

	CNode *pn, *p, *pTail=GetTail(ppList);

	for (p=GetHead(ppList); p; p=pn) {
		pn = p->pNext;
		if (bDetach)
			p->Detach(ppList);

		BOOL bAtTail = (p == pTail);
		// break out of loop if callback returns FALSE or we processed the last node
		if (!(*pfnEnumCallback)(ppList, p, pData) || bAtTail)
			break;
	}

	return TRUE;
}


#ifdef _DEBUG

// static
BOOL CNode::_dump(LPVOID * ppList, LPCNODE pNode, LPVOID pData)
{ 
	char buffer[80];

  PFNENUMCALLBACK pfnCallback = (PFNENUMCALLBACK) pData;

	wsprintf(buffer, "this=%lx  pNext=%lx  pPrev=%lx", pNode, pNode->pNext, pNode->pPrev);
	OutputDebugString(buffer);

	if (pData){
		BOOL bReturn = (*pfnCallback)(ppList, pNode, NULL);
 		OutputDebugString("\n");
		return bReturn;
	}
	else
		return TRUE;
}

BOOL CNode::Dump(LPVOID * ppList, PFNENUMCALLBACK pfnEnumCallback/*=NULL*/)
{ 
	return Enumerate(ppList, _dump, pfnEnumCallback); 
}

#endif





