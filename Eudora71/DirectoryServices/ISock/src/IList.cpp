/******************************************************************************/
/*																										*/
/*	Name		:	ILIST.CPP		  																*/
/* Date     :  4/30/1997                                                      */
/* Author   :  Jim Susoy                                                      */
/* Notice   :  (C) 1997 Qualcomm, Inc. - All Rights Reserved                  */
/* Copyright (c) 2016, Computer History Museum 
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
DAMAGE. */

/*	Desc.		:	Linked list manager class													*/
/*																										*/
/******************************************************************************/
#pragma warning(disable : 4514 4505)
#include <afx.h>
#include <ole2.h>
#include <string.h>
#include "DebugNewHelpers.h"
#include "IListp.h"


extern long g_cComponents;

//LPILIST CreateIList(ILIST_TYPE wType,DWORD dwFlags,ILISTFREECB pCB,LPVOID pUser)
//{
//	LPILIST	pIList = NULL;
//	
//	if(NULL != (pIList = new IList)) {
//		if(TRUE == pIList->Initialize(wType,dwFlags,pCB,pUser))
//			return(pIList);
//		
//		delete pIList;
//	}
//	return(NULL);
//}

/* Static */
IUnknown * IList::CreateInstance()
{
	IList	*pList = DEBUG_NEW_NOTHROW IList;
	
	if(!pList)
		return NULL;
	
	return static_cast<IUnknown *>(pList);
}


static BOOL PASCAL CompareGUID(REFGUID r1,REFGUID r2)	
{
	if(r1.Data1 == r2.Data1 && \
		r1.Data2 == r2.Data2 && \
		r1.Data3 == r2.Data3 && \
		r1.Data4[0] == r2.Data4[0] && \
		r1.Data4[1] == r2.Data4[1] && \
		r1.Data4[2] == r2.Data4[2] && \
		r1.Data4[3] == r2.Data4[3] && \
		r1.Data4[4] == r2.Data4[4] && \
		r1.Data4[5] == r2.Data4[5] && \
		r1.Data4[6] == r2.Data4[6] && \
		r1.Data4[7] == r2.Data4[7]) {
		
		return(TRUE);
	}
	return(FALSE);
}


/* IUnknown methods																				*/
STDMETHODIMP IList::QueryInterface(REFIID riid, void **ppv)
{
	if (TRUE == IsEqualIID(riid,IID_IUnknown) || TRUE == IsEqualIID(riid,IID_IListMan)) {
		*ppv = static_cast<IListMan *>(this);
	}
	else {
      *ppv = 0;
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) IList::AddRef(void)
{
	return InterlockedIncrement(&m_nRefs);
}

STDMETHODIMP_(ULONG) IList::Release(void)
{
	if (0 == InterlockedDecrement(&m_nRefs)) {
		delete this;
		return 0;
	}
	return m_nRefs;
}

IList::IList(void) : m_nRefs(1)
{
	m_dwCount			= 0;
	m_pHead				= NULL;
	m_wType				= ILIST_DOUBLE;	/* Default to double circular list		*/
	m_dwFlags			= 0;
	m_pFreeCB			= NULL;
	m_pFreeUserData	= NULL;
	InterlockedIncrement(&g_cComponents);
}

IList::~IList(void)
{
	Erase();
	InterlockedDecrement(&g_cComponents);
}

//BOOL PASCAL IList::Initialize(ILIST_TYPE wType,DWORD dwFlags,ILISTFREECB pCB,LPVOID pUser)
//{
//	if(!(ILIST_STANDARDALLOC & dwFlags) && !pCB)
//		return(FALSE);
//	
//	if(ILIST_NOFLAGS == dwFlags)
//		m_dwFlags = ILIST_APPEND;
//	else
//		m_dwFlags			= dwFlags;
//	
//	m_wType 				= wType;
//	m_pFreeCB			= pCB;
//	m_pFreeUserData	= pUser;
//	return(TRUE);
//}

BOOL PASCAL IList::Initialize(ILISTFREECB pCB,LPVOID pUser)
{
	if(!pCB)
		return(FALSE);
	
	m_dwFlags 			= ILIST_APPEND;
	m_wType 				= ILIST_DOUBLE;
	m_pFreeCB			= pCB;
	m_pFreeUserData	= pUser;
	return(TRUE);
}

BOOL PASCAL IList::VerifyForwardLink(LPITEM pItem)
{
	if(!pItem->pNext || pItem->pNext->pPrev != pItem)
		return(FALSE);
	
	return(TRUE);
}

BOOL PASCAL IList::VerifyBackwardLink(LPITEM pItem)
{
	if(!pItem->pPrev || pItem->pPrev->pNext != pItem)
		return(FALSE);
	
	return(TRUE);
}

void PASCAL IList::Insert(LPVOID pRefItem,LPVOID pItem,WORD wWhere)
{
	LPITEM	pRef 	= (LPITEM) pRefItem;
	LPITEM   pI 	= (LPITEM) pItem;
	
	if(!m_pHead) {
		m_pHead = pI;
		m_pHead->pNext = m_pHead->pPrev = m_pHead;
	}
	else {
		switch(wWhere) {
			case ILIST_INS_BEFORE: {
				pI->pNext 			= pRef;
				pI->pPrev 			= pRef->pPrev;
				pRef->pPrev->pNext= pI;
				pRef->pPrev 		= pI;
				if(m_pHead == pRef)
					m_pHead = pI;
				break;
			}
			case ILIST_INS_AFTER:
			default: {
				pI->pPrev 			= pRef;
				pI->pNext 			= pRef->pNext;
				pRef->pNext->pPrev= pI;
				pRef->pNext			= pI;
				break;
			}
		}
	}
	m_dwCount++;
}

void PASCAL IList::Attach(LPVOID pItem)
{
	if(ILIST_APPEND & m_dwFlags)
		Insert(m_pHead,pItem,ILIST_INS_AFTER);
	else if(ILIST_PREPEND & m_dwFlags)
		Insert(m_pHead,pItem,ILIST_INS_BEFORE);
}

LPVOID PASCAL IList::Unlink(LPVOID pItem)
{
	LPITEM	pTmp = NULL;
	
	/* Must be valid item and currently in the list										*/
	if(!pItem || !((LPITEM)pItem)->pNext || !((LPITEM)pItem)->pPrev)
		return(NULL);
	
	/* Are we the only item and the head???												*/
	if(m_dwCount == 1 && pItem == m_pHead) {
		pTmp 		= (LPITEM) pItem;
		
		/* Set the NODE info to NULL															*/
		pTmp->pPrev = NULL;
		pTmp->pNext = NULL;
		
		/* Update our internal data															*/
		m_pHead 	= NULL;
		m_dwCount= 0;
	}
	else {
		/* Unlink the item from the list and dec the reference count				*/
		pTmp 						= (LPITEM) pItem;
		pTmp->pNext->pPrev 	= pTmp->pPrev;
		pTmp->pPrev->pNext 	= pTmp->pNext;
		m_dwCount--;
		
		/* If the list is empty, set the head to NULL									*/
		if(0 == m_dwCount) {
			m_pHead = NULL;
		}
		/* Otherwise if we removed the head, re-set it to the following item		*/
		else if(m_pHead == pItem) {
			m_pHead = pTmp->pNext;
		}
	}
	
	/* Set the NODE info to NULL															*/
	if(pTmp) {
		pTmp->pPrev = NULL;
		pTmp->pNext = NULL;
	}
	return(pTmp);
}

void PASCAL IList::DeleteItem(LPVOID pItem)
{
	//LPITEM pTmp = (LPITEM) Unlink(pItem);
	Unlink(pItem);
	
	if(m_pFreeCB) m_pFreeCB((LPITEM) pItem,m_pFreeUserData);
}

void PASCAL IList::Erase(void)
{
	LPITEM	pTmp = NULL;
	
	if(m_dwCount > 0) {
		while(NULL != (pTmp = (LPITEM) Unlink(m_pHead))) {
			if(ILIST_STANDARDALLOC & m_dwFlags) {
				free(pTmp);
			}
			else {
				m_pFreeCB(pTmp,m_pFreeUserData);
			}
		}
	}
}

void PASCAL IList::Push(LPVOID pItem)
{
	Insert(m_pHead,pItem,ILIST_INS_BEFORE);
}

LPVOID PASCAL IList::Pop(void)
{
	return(Unlink(m_pHead));
}


LPVOID PASCAL IList::Head(void)
{
	return(m_pHead);
}

DWORD PASCAL IList::Count(void)
{
	return(m_dwCount);
}

WORD PASCAL IList::VerifyList(void)
{
	LPITEM	pTmp = (LPITEM) Head();
	
	if(NULL != pTmp) {
		do {
			if(FALSE == VerifyForwardLink(pTmp))
				return(ILIST_BADFORLINK);
			
			if(FALSE == VerifyBackwardLink(pTmp))
				return(ILIST_BADBACKLINK);
		
		} while (NULL != (pTmp = (LPITEM) ILIST_ITEMNEXT(pTmp)));
	}
	return(ILIST_OK);
	
}
