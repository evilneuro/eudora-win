//	X1EventListener.cpp
//
//	Responds to events from X1.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
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



#include "stdafx.h"
#include "X1EventListener.h"
#include "SearchManager.h"


class X1EventListener::X1GeneralEventListener : public X1::IX1EventListener
{
  public:
										X1GeneralEventListener(
											X1EventListener *			in_pX1EventListener);
	virtual								~X1GeneralEventListener();

	virtual HRESULT __stdcall			QueryInterface(
											const IID &					in_iid,
											void **						out_ppvObject);
		//	Standard IUnknown::QueryInterface() implementation

	virtual ULONG __stdcall				AddRef();
		//	Standard IUnknown::AddRef() implementation

	virtual ULONG __stdcall				Release();
		//	Standard IUnknown::Release() implementation

	virtual HRESULT __stdcall			raw_OnLog(
											BSTR						in_bstrLine);
		//	Receives logging events

	virtual HRESULT __stdcall			raw_OnLogException(
											BSTR						in_bstrLogMessage,
											BSTR						in_bstrExceptionMessage,
											unsigned char				in_bOutOfResources,
											unsigned char				in_bWin32Error);
		//	Receives exception logging events

	virtual HRESULT __stdcall			raw_OnScanYield();
		//	Called when scanning is yielding to another process

	virtual HRESULT __stdcall			raw_OnScanResume();
		//	Called when scanning is resume after yielding to another process

  protected:
	X1EventListener *					m_pX1EventListener;
};


class X1EventListener::X1DBEventListener : public X1::IX1DbEventListener
{
  public:
										X1DBEventListener(
											X1EventListener *			in_pX1EventListener);
	virtual								~X1DBEventListener();

	virtual HRESULT __stdcall			QueryInterface(
											const IID &					in_iid,
											void **						out_ppvObject);
		//	Standard IUnknown::QueryInterface() implementation

	virtual ULONG __stdcall				AddRef();
		//	Standard IUnknown::AddRef() implementation

	virtual ULONG __stdcall				Release();
		//	Standard IUnknown::Release() implementation

	virtual HRESULT __stdcall			raw_OnLoadStart(
											long						in_nDBID,
											unsigned char				in_bInitialLoad);
		//	Called when the database is about to start loading

	virtual HRESULT __stdcall			raw_OnLoadComplete(
											long						in_nDBID,
											unsigned char				in_bInitialLoad);
		//	Called when the database has finished loading

	virtual HRESULT __stdcall			raw_OnUnload(
											long						in_nDBID,
											enum X1::X1UnloadReason		in_eReason);
		//	Called when the database has been unloaded

	virtual HRESULT __stdcall			raw_OnBuildStart(
											long						in_nDBID);
		//	Called when a database build is about to start

	virtual HRESULT __stdcall			raw_OnBuildComplete(
											long						in_nDBID);
		//	Called when a database build has completed

	virtual HRESULT __stdcall			raw_OnMergeStart(
											long						in_nDBID);
		//	Called when a database merge is about to start

	virtual HRESULT __stdcall			raw_OnMergeComplete(
											long						in_nDBID);
		//	Called when a database merge has completed

	virtual HRESULT __stdcall			raw_OnFlushStart(
											long						in_nDBID);
		//	Called when a database flush is about to start

	virtual HRESULT __stdcall			raw_OnFlushComplete(
											long						in_nDBID);
		//	Called when a database flush has completed

	virtual HRESULT __stdcall			raw_OnCrashCleanup(
											long						in_nDBID);
		//	Called when the database has been unloaded

	virtual HRESULT __stdcall			raw_OnLoadError(
											long						in_nDBID);
		//	Called if an error occurred while loading the database

	virtual HRESULT __stdcall			raw_OnCommitError(
											long						in_nDBID);
		//	Called if an error occurred while trying to commit database changes to disk

	virtual HRESULT __stdcall			raw_OnNoIncDict(
											long						in_nDBID);
		//	Called if the database doesn't have an incremental dictionary

	virtual HRESULT __stdcall			raw_OnLargeMerge(
											long						in_nDBID);
		//	Called when the database is getting large

	virtual HRESULT __stdcall			raw_OnScanError(
											long						in_nDBID);
		//	Called when an error occurs while scanning

  protected:
	X1EventListener *					m_pX1EventListener;
};


// ---------------------------------------------------------------------------
//		* X1EventListener											 [Public]
// ---------------------------------------------------------------------------
//	X1EventListener constructor.

X1EventListener::X1EventListener()
	:	m_nRefCount(0),
		m_pIX1EventListener( DEBUG_NEW X1GeneralEventListener(this) ),
		m_pIX1DBEventListener( DEBUG_NEW X1DBEventListener(this) )
{
}


// ---------------------------------------------------------------------------
//		* ~X1EventListener											 [Public]
// ---------------------------------------------------------------------------
//	X1EventListener destructor.

X1EventListener::~X1EventListener() 
{
}


// ---------------------------------------------------------------------------
//		* QueryInterface											 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::QueryInterface() implementation

HRESULT __stdcall
X1EventListener::QueryInterface(
	const IID &					in_iid,
	void **						out_ppvObject)
{
	if ( (in_iid == IID_IUnknown) || (in_iid == X1::IID_IX1EventListener) )
	{
		*out_ppvObject = m_pIX1EventListener.get();
	}
	else if (in_iid == X1::IID_IX1DbEventListener)
	{
		*out_ppvObject = m_pIX1DBEventListener.get();
	}
	else
	{
		*out_ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* AddRef													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::AddRef() implementation

ULONG __stdcall
X1EventListener::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}


// ---------------------------------------------------------------------------
//		* Release													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::Release() implementation

ULONG __stdcall
X1EventListener::Release()
{
	if (InterlockedDecrement(&m_nRefCount) == 0)
	{
		delete this;
		return 0;
	}

	return m_nRefCount;
}



// ---------------------------------------------------------------------------
//		* X1GeneralEventListener									 [Public]
// ---------------------------------------------------------------------------
//	X1GeneralEventListener constructor.

X1EventListener::X1GeneralEventListener::X1GeneralEventListener(
	X1EventListener *			in_pX1EventListener)
	:	m_pX1EventListener(in_pX1EventListener)
{
}


// ---------------------------------------------------------------------------
//		* ~X1GeneralEventListener									 [Public]
// ---------------------------------------------------------------------------
//	X1GeneralEventListener destructor.

X1EventListener::X1GeneralEventListener::~X1GeneralEventListener() 
{
}


// ---------------------------------------------------------------------------
//		* QueryInterface											 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::QueryInterface() implementation

HRESULT __stdcall
X1EventListener::X1GeneralEventListener::QueryInterface(
	const IID &					in_iid,
	void **						out_ppvObject)
{
	if ( (in_iid == IID_IUnknown) || (in_iid == X1::IID_IX1EventListener) )
	{
		*out_ppvObject = this;
	}
	else
	{
		*out_ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* AddRef													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::AddRef() implementation

ULONG __stdcall
X1EventListener::X1GeneralEventListener::AddRef()
{
	return m_pX1EventListener->AddRef();;
}


// ---------------------------------------------------------------------------
//		* Release													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::Release() implementation

ULONG __stdcall
X1EventListener::X1GeneralEventListener::Release()
{
	return m_pX1EventListener->Release();
}


// ---------------------------------------------------------------------------
//		* raw_OnLog													 [Public]
// ---------------------------------------------------------------------------
//	Receives logging events

HRESULT __stdcall
X1EventListener::X1GeneralEventListener::raw_OnLog(
	BSTR						in_bstrLine)
{
	CString		szX1Log;

	szX1Log.Format( "X1 Log: %s",
					static_cast<char *>(_bstr_t(in_bstrLine)) );
	
	TRACE(szX1Log);
	PutDebugLog( DEBUG_MASK_SEARCH, szX1Log );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnLogException										 [Public]
// ---------------------------------------------------------------------------
//	Receives exception logging events

HRESULT __stdcall
X1EventListener::X1GeneralEventListener::raw_OnLogException(
	BSTR						in_bstrLogMessage,
	BSTR						in_bstrExceptionMessage,
	unsigned char				in_bOutOfResources,
	unsigned char				in_bWin32Error)
{	
	//	Log error
	CString		szX1Log;

	szX1Log.Format( "X1 Error: %s, %s, Out of resources: %d, Win32 Error: %d",
					static_cast<char *>(_bstr_t(in_bstrLogMessage)),
					static_cast<char *>(_bstr_t(in_bstrExceptionMessage)),
					in_bOutOfResources, in_bWin32Error );
	PutDebugLog( DEBUG_MASK_SEARCH, szX1Log );

	//	Get developer's attention
	ASSERT(!"X1 Error Notification - raw_OnLogException");

	SearchManager::Instance()->NotifyX1Exception();

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnScanYield											 [Public]
// ---------------------------------------------------------------------------
//	Called when scanning is yielding to another process

HRESULT __stdcall
X1EventListener::X1GeneralEventListener::raw_OnScanYield()
{
	TRACE("X1 scanner is yielding to another process\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 scanner is yielding to another process" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnScanResume											 [Public]
// ---------------------------------------------------------------------------
//	Called when scanning is resume after yielding to another process

HRESULT __stdcall
X1EventListener::X1GeneralEventListener::raw_OnScanResume()
{
	TRACE("X1 scanner is resuming\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 scanner is resuming" );

	return S_OK;
}

// ---------------------------------------------------------------------------
//		* X1DBEventListener											 [Public]
// ---------------------------------------------------------------------------
//	X1DBEventListener constructor.

X1EventListener::X1DBEventListener::X1DBEventListener(
	X1EventListener *			in_pX1EventListener)
	:	m_pX1EventListener(in_pX1EventListener)
{
}


// ---------------------------------------------------------------------------
//		* ~X1DBEventListener										 [Public]
// ---------------------------------------------------------------------------
//	X1DBEventListener destructor.

X1EventListener::X1DBEventListener::~X1DBEventListener() 
{
}


// ---------------------------------------------------------------------------
//		* QueryInterface											 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::QueryInterface() implementation

HRESULT __stdcall
X1EventListener::X1DBEventListener::QueryInterface(
	const IID &					in_iid,
	void **						out_ppvObject)
{
	if ( (in_iid == IID_IUnknown) || (in_iid == X1::IID_IX1DbEventListener) )
	{
		*out_ppvObject = this;
	}
	else
	{
		*out_ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* AddRef													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::AddRef() implementation

ULONG __stdcall
X1EventListener::X1DBEventListener::AddRef()
{
	return m_pX1EventListener->AddRef();
}


// ---------------------------------------------------------------------------
//		* Release													 [Public]
// ---------------------------------------------------------------------------
//	Standard IUnknown::Release() implementation

ULONG __stdcall
X1EventListener::X1DBEventListener::Release()
{
	return m_pX1EventListener->Release();
}


// ---------------------------------------------------------------------------
//		* raw_OnLoadStart											 [Public]
// ---------------------------------------------------------------------------
//	Called when the database is about to start loading

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnLoadStart(
	long						in_nDBID,
	unsigned char				in_bInitialLoad)
{
	TRACE("X1 loading database\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 loading database" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnLoadComplete										 [Public]
// ---------------------------------------------------------------------------
//	Called when the database has finished loading

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnLoadComplete(
	long						in_nDBID,
	unsigned char				in_bInitialLoad)
{
	TRACE("X1 database load complete\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database load complete" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnUnload												 [Public]
// ---------------------------------------------------------------------------
//	Called when the database has been unloaded

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnUnload(
	long						in_nDBID,
	enum X1::X1UnloadReason		in_eReason)
{
	TRACE("X1 database unloaded\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database unloaded" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnBuildStart											 [Public]
// ---------------------------------------------------------------------------
//	Called when a database build is about to start

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnBuildStart(
	long						in_nDBID)
{
	TRACE("X1 building database\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 building database" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnBuildComplete										 [Public]
// ---------------------------------------------------------------------------
//	Called when a database build has completed

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnBuildComplete(
	long						in_nDBID)
{
	TRACE("X1 database build complete\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database build complete" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnMergeStart											 [Public]
// ---------------------------------------------------------------------------
//	Called when a database merge is about to start

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnMergeStart(
	long						in_nDBID)
{
	TRACE("X1 merging database\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 merging database" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnMergeComplete										 [Public]
// ---------------------------------------------------------------------------
//	Called when a database merge has completed

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnMergeComplete(
	long						in_nDBID)
{
	TRACE("X1 database merge complete\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database merge complete" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnFlushStart											 [Public]
// ---------------------------------------------------------------------------
//	Called when a database flush is about to start

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnFlushStart(
	long						in_nDBID)
{
	TRACE("X1 flushing database\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 flushing database" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnFlushComplete										 [Public]
// ---------------------------------------------------------------------------
//	Called when a database flush has completed

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnFlushComplete(
	long						in_nDBID)
{
	TRACE("X1 database flush complete\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database flush complete" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnCrashCleanup										 [Public]
// ---------------------------------------------------------------------------
//	Called when the database has been unloaded

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnCrashCleanup(
	long						in_nDBID)
{
	TRACE("X1 on crash cleanup\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 on crash cleanup" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnLoadError											 [Public]
// ---------------------------------------------------------------------------
//	Called if an error occurred while loading the database

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnLoadError(
	long						in_nDBID)
{
	TRACE("X1 unable to load database\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 unable to load database" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnCommitError											 [Public]
// ---------------------------------------------------------------------------
//	Called if an error occurred while trying to commit database changes to disk

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnCommitError(
	long						in_nDBID)
{
	TRACE("X1 unable to commit database\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 unable to commit database" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnNoIncDict											 [Public]
// ---------------------------------------------------------------------------
//	Called if the database doesn't have an incremental dictionary

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnNoIncDict(
	long						in_nDBID)
{
	TRACE("X1 database missing incremental dictionary\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database missing incremental dictionary" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnLargeMerge											 [Public]
// ---------------------------------------------------------------------------
//	Called when the database is getting large

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnLargeMerge(
	long						in_nDBID)
{
	TRACE("X1 database is getting large\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 database is getting large" );

	return S_OK;
}


// ---------------------------------------------------------------------------
//		* raw_OnScanError											 [Public]
// ---------------------------------------------------------------------------
//	Called when an error occurs while scanning

HRESULT __stdcall
X1EventListener::X1DBEventListener::raw_OnScanError(
	long						in_nDBID)
{
	TRACE("X1 a scan error occurred, scanning has been paused\n");
	PutDebugLog( DEBUG_MASK_SEARCH, "X1 a scan error occurred, scanning has been paused" );

	return S_OK;
}
