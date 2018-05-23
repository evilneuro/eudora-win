/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File: 					cnode.cpp
Description:		
Date:						7/30/97
Version:  			1.0 
Module:					
Notice:					Copyright 1997 Qualcomm Inc.  All Rights Reserved.
 Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. 


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Revisions:			
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma warning(disable : 4201 4514)
#include <windows.h>
#pragma warning(default : 4201)
#include "DebugNewHelpers.h"
#include "cnode.h"


BOOL CListNode::Attach(LPVOID * ppList, CListNode* pRefNode/*=INSERT_APPEND*/)
{
	if (!ppList)
		return FALSE;

	pNext = NULL;

	// special case if empty list
	if (!*ppList) {
		*ppList = this;
	}
	// inserting at head of list
	else
	if (pRefNode == CLIST_PREPEND) {
		pNext = (CListNode*)*ppList;
		*ppList = this;
	}
	// inserting at tail of list
	else
	if (pRefNode == CLIST_APPEND) {
		CListNode * p = FindPrev(ppList, NULL);
		p->pNext = this;
	}
	// inserting after a reference node
	else {
    pNext = pRefNode->pNext;
		pRefNode->pNext = this;
	}
		
	return TRUE;
}


BOOL CListNode::Detach(LPVOID * ppList)
{
	if (!ppList)
		return FALSE;

	// adjust the list pointer if we are the first node
	if (IsHead(ppList))
		*ppList = pNext;
	
	CListNode* p = FindPrev(ppList, this);
	if (!p)
		return FALSE;

	p->pNext = pNext;
	return TRUE;
}


CListNode * CListNode::FindPrev(LPVOID *ppList, CListNode* pNode)
{
	CListNode *p;
	
	for (p=(CListNode*)*ppList; p; p=p->pNext)
		if (pNode == p->pNext)
			return p;

	return NULL;
}


//// (static)
//void CListNode::FreeList(LPVOID * ppList)
//{
//	while (*ppList) {
//		CListNode *p = GetHead(ppList);
//		p->Detach(ppList);
//		delete p;
//	}
//}





