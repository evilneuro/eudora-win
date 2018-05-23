// LDAP Directory Services 
//
// File: 			records.cpp
// Copyright		08/06/1997 Qualcomm Inc.  All Rights Reserved.
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

// Version:  		1.0 
// Description:		
// Implementation of an IDSPRecord COM object.
//
#pragma warning(disable : 4514)
#include <afx.h>
#include <stdlib.h>
#include <objbase.h>
#include "DebugNewHelpers.h"
#include <dspapi.h>
#include "standard.h"
#include "records.h"
#include "util.h"

extern DWORD g_dwComponentCount;


//****************************************************************
// Standard IUnknown interface methods
//  
//****************************************************************
//
//
CRecord::CRecord() : m_Ref( 1 ),
					 m_ItemHead( NULL ),
                     m_ItemTail( NULL ),
                     m_szRecordID( NULL ),
                     m_szDBID( NULL ),
				     m_QueryStatus( QUERY_OK ),
					 m_LDAPCode( 0 )
{
    g_dwComponentCount++;

} 


// The "DBRECENT" structure was malloced but any data pointed to was allocated with the C++ new operator.
//
CRecord::~CRecord()
{
    LPDBRECENT next_item = m_ItemHead;

    // -> free each item on the ItemList
    while( next_item != NULL )
    {
           m_ItemHead = m_ItemHead->pNext;
           if (next_item->lpszUserName) delete [] next_item->lpszUserName;

		   delete [] next_item;

		   next_item = m_ItemHead;           
    }

    if (m_szRecordID ) delete [] m_szRecordID;
    if (m_szDBID     ) delete [] m_szDBID;

    g_dwComponentCount--;
}


//
//
HRESULT	__stdcall CRecord::QueryInterface( REFIID iid, LPVOID* ppv)
{
	     if (iid == IID_IUnknown) {
		     *ppv = static_cast<IDSPRecord*>(this);
    }
	else if (iid == IID_ISetRecord) {
		     *ppv = static_cast<LPISETREC>(this);
    }
	else {
		     *ppv = NULL;
		     return( E_NOINTERFACE );
	}	
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return( S_OK );
}


//
ULONG __stdcall CRecord::AddRef()
{
	return( InterlockedIncrement( &m_Ref ));
}


//
ULONG __stdcall CRecord::Release()
{
	if (InterlockedDecrement( &m_Ref ) == 0) {
		delete this;
		return( 0 );
	}
	return( m_Ref );
}



//****************************************************************
// IDSPRecord Interface
//
//****************************************************************
//
DBRECENT* __stdcall CRecord::GetRecordList()
{     return( m_ItemHead );     }

//
LPSTR __stdcall CRecord::GetRecordID()
{     return( m_szRecordID );   }

//
LPSTR __stdcall CRecord::GetDatabaseID()
{     return( m_szDBID );       }
 
//
QUERY_STATUS __stdcall CRecord::GetError( LPSTR pszBuffer, int nLen )
{

    TranslateResultCode( m_LDAPCode, pszBuffer, nLen );
    return( m_QueryStatus );
}



//****************************************************************
// ISetRecord Interface
//
//****************************************************************
//
// ********* NOTE:
// For all of the functions in this interface the caller provides the memory that 
// we take and include in this object (i.e., we do not copy the data passed into use we take its ownership)
// ********* NOTE:
//
HRESULT __stdcall CRecord::AddItem( LPDBRECENT pItem )
{
    if (pItem == NULL) return( E_INVALIDARG );

         if (m_ItemHead == NULL) {
             m_ItemHead = m_ItemTail = pItem;    // -> no list exists
    }
    else {   m_ItemTail->pNext = pItem;          // -> place all new items onto the end of the list
             m_ItemTail        = pItem;
    }
    pItem->pNext = NULL;
    return( S_OK );     
}


//
HRESULT __stdcall CRecord::SetRecordId( LPSTR szRecordID )
{
    char* szTemp = NULL;

    // ->
    if (szRecordID == NULL) return( E_INVALIDARG );
    if ((szTemp = DEBUG_NEW_NOTHROW char[lstrlen( szRecordID )+1]) == 0) return( E_OUTOFMEMORY );
    // ->
    if (m_szRecordID) delete [] m_szRecordID;
    m_szRecordID    = szTemp;
    m_szRecordID[0] = '\0';
    lstrcpy( m_szRecordID, szRecordID );
    return( S_OK );
}


//
HRESULT __stdcall CRecord::SetDBId( LPSTR szDbID )
{
    char* szTemp = NULL;

    // ->
    if (szDbID == NULL) return( E_INVALIDARG );
    if ((szTemp = DEBUG_NEW_NOTHROW char[lstrlen( szDbID )+1]) == 0) return( E_OUTOFMEMORY );

    // ->
    if (m_szDBID) delete [] m_szDBID;
    m_szDBID    = szTemp;
    m_szDBID[0] = '\0';
    lstrcpy( m_szDBID, szDbID );
    return( S_OK );
}


//
HRESULT __stdcall CRecord::SetErrorStr( QUERY_STATUS ResultCode, int LDAP_ResultCode )
{
    m_QueryStatus = ResultCode;
    m_LDAPCode    = LDAP_ResultCode;
    return( S_OK );
}



