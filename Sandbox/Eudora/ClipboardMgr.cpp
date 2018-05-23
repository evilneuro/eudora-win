// EUDORA.CPP
//

#include "stdafx.h"
#include "resource.h"

#include "rs.h"
#include "ClipboardMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//
// The one global instance.
//
QCClipboardMgr g_theClipboardMgr;


////////////////////////////////////////////////////////////////////////
// QCClipboardMgr [public, constructor]
//
////////////////////////////////////////////////////////////////////////
QCClipboardMgr::QCClipboardMgr()
{
	for (int i = 0; i < CF_NUMFORMATS; i++)
		m_uClipboardFormatTable[i] = 0;			// Zero is an invalid ID
}


////////////////////////////////////////////////////////////////////////
// ~QCClipboardMgr [public, virtual, destructor]
//
////////////////////////////////////////////////////////////////////////
QCClipboardMgr::~QCClipboardMgr()
{
}


////////////////////////////////////////////////////////////////////////
// RegisterAllFormats [public]
//
// Register custom clipboard formats for drag and drops.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::RegisterAllFormats()
{
	ASSERT(0 == m_uClipboardFormatTable[CF_TRANSFER]);
	m_uClipboardFormatTable[CF_TRANSFER] = ::RegisterClipboardFormat(CRString(IDS_TRANSFER_CLIPBOARD_FORMAT_NAME));
	if (! m_uClipboardFormatTable[CF_TRANSFER])
		return FALSE;

	ASSERT(0 == m_uClipboardFormatTable[CF_STATIONERY]);
	m_uClipboardFormatTable[CF_STATIONERY] = ::RegisterClipboardFormat(CRString(IDS_STATIONERY_CLIPBOARD_FORMAT_NAME));
	if (! m_uClipboardFormatTable[CF_STATIONERY])
		return FALSE;

	ASSERT(0 == m_uClipboardFormatTable[CF_SIGNATURE]);
	m_uClipboardFormatTable[CF_SIGNATURE] = ::RegisterClipboardFormat(CRString(IDS_SIGNATURE_CLIPBOARD_FORMAT_NAME));
	if (! m_uClipboardFormatTable[CF_SIGNATURE])
		return FALSE;

	ASSERT(0 == m_uClipboardFormatTable[CF_WAZOO]);
	m_uClipboardFormatTable[CF_WAZOO] = ::RegisterClipboardFormat(CRString(IDS_WAZOO_CLIPBOARD_FORMAT_NAME));
	if (! m_uClipboardFormatTable[CF_WAZOO])
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CacheGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::CacheGlobalData(COleDataSource& oleDataSource, const QCFTRANSFER& qcfTransfer)
{
	ASSERT(m_uClipboardFormatTable[CF_TRANSFER]);			// forget to call RegisterAllFormats()?
	ASSERT(qcfTransfer.IsValid());

	HGLOBAL hGlobal = ::GlobalAlloc(GPTR, sizeof(QCFTRANSFER));   
	if (NULL == hGlobal)
		return FALSE;

	QCFTRANSFER* pData = (QCFTRANSFER *) ::GlobalLock(hGlobal);
	if (pData)
	{
		*pData = qcfTransfer;

		oleDataSource.CacheGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_TRANSFER]), hGlobal);
		return TRUE;
	}
	
	::GlobalFree(hGlobal);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// CacheGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::CacheGlobalData(COleDataSource& oleDataSource, const QCFSTATIONERY& qcfStationery)
{
	ASSERT(m_uClipboardFormatTable[CF_STATIONERY]);			// forget to call RegisterAllFormats()?
	ASSERT(qcfStationery.IsValid());

	HGLOBAL hGlobal = ::GlobalAlloc(GPTR, sizeof(QCFSTATIONERY));   
	if (NULL == hGlobal)
		return FALSE;

	QCFSTATIONERY* pData = (QCFSTATIONERY *) ::GlobalLock(hGlobal);
	if (pData)
	{
		*pData = qcfStationery;

		oleDataSource.CacheGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_STATIONERY]), hGlobal);
		return TRUE;
	}
	
	::GlobalFree(hGlobal);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// CacheGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::CacheGlobalData(COleDataSource& oleDataSource, const QCFSIGNATURE& qcfStationery)
{
	ASSERT(m_uClipboardFormatTable[CF_SIGNATURE]);			// forget to call RegisterAllFormats()?
	ASSERT(qcfStationery.IsValid());

	HGLOBAL hGlobal = ::GlobalAlloc(GPTR, sizeof(QCFSIGNATURE));   
	if (NULL == hGlobal)
		return FALSE;

	QCFSIGNATURE* pData = (QCFSIGNATURE *) ::GlobalLock(hGlobal);
	if (pData)
	{
		*pData = qcfStationery;

		oleDataSource.CacheGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_SIGNATURE]), hGlobal);
		return TRUE;
	}
	
	::GlobalFree(hGlobal);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// CacheGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::CacheGlobalData(COleDataSource& oleDataSource, const QCFWAZOO& qcfWazoo)
{
	ASSERT(m_uClipboardFormatTable[CF_WAZOO]);				// forget to call RegisterAllFormats()?
	ASSERT(qcfWazoo.IsValid());

	HGLOBAL hGlobal = ::GlobalAlloc(GPTR, sizeof(QCFWAZOO));   
	if (NULL == hGlobal)
		return FALSE;

	QCFWAZOO* pData = (QCFWAZOO *) ::GlobalLock(hGlobal);
	if (pData)
	{
		*pData = qcfWazoo;

		oleDataSource.CacheGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_WAZOO]), hGlobal);
		return TRUE;
	}
	
	::GlobalFree(hGlobal);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::GetGlobalData(COleDataObject* pOleDataObject, QCFTRANSFER& qcfTransfer)
{
	ASSERT(pOleDataObject);
	ASSERT(m_uClipboardFormatTable[CF_TRANSFER]);			// forget to call RegisterAllFormats()?

	::ZeroMemory(&qcfTransfer, sizeof(QCFTRANSFER));		// good hygiene

	if (! pOleDataObject->IsDataAvailable(CLIPFORMAT(m_uClipboardFormatTable[CF_TRANSFER])))
		return FALSE;		// that was easy

	// Get global clipboard data from COleDataObject
	HGLOBAL hGlobal = pOleDataObject->GetGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_TRANSFER]));
	if (NULL == hGlobal)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Copy the global data to the caller-provided record.
	//
	QCFTRANSFER* pData = (QCFTRANSFER *) ::GlobalLock(hGlobal);
	if (pData)
	{
		qcfTransfer = *pData;
		ASSERT(qcfTransfer.IsValid());
		::GlobalUnlock(hGlobal);
		return TRUE;
	}
	else
	{
		ASSERT(0);
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::GetGlobalData(COleDataObject* pOleDataObject, QCFSTATIONERY& qcfStationery)
{
	ASSERT(pOleDataObject);
	ASSERT(m_uClipboardFormatTable[CF_STATIONERY]);			// forget to call RegisterAllFormats()?

	::ZeroMemory(&qcfStationery, sizeof(QCFSTATIONERY));		// good hygiene

	if (! pOleDataObject->IsDataAvailable(CLIPFORMAT(m_uClipboardFormatTable[CF_STATIONERY])))
		return FALSE;		// that was easy

	// Get global clipboard data from COleDataObject
	HGLOBAL hGlobal = pOleDataObject->GetGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_STATIONERY]));
	if (NULL == hGlobal)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Copy the global data to the caller-provided record.
	//
	QCFSTATIONERY* pData = (QCFSTATIONERY *) ::GlobalLock(hGlobal);
	if (pData)
	{
		qcfStationery = *pData;
		ASSERT(qcfStationery.IsValid());
		::GlobalUnlock(hGlobal);
		return TRUE;
	}
	else
	{
		ASSERT(0);
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::GetGlobalData(COleDataObject* pOleDataObject, QCFSIGNATURE& qcfStationery)
{
	ASSERT(pOleDataObject);
	ASSERT(m_uClipboardFormatTable[CF_SIGNATURE]);			// forget to call RegisterAllFormats()?

	::ZeroMemory(&qcfStationery, sizeof(QCFSIGNATURE));		// good hygiene

	if (! pOleDataObject->IsDataAvailable(CLIPFORMAT(m_uClipboardFormatTable[CF_SIGNATURE])))
		return FALSE;		// that was easy

	// Get global clipboard data from COleDataObject
	HGLOBAL hGlobal = pOleDataObject->GetGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_SIGNATURE]));
	if (NULL == hGlobal)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Copy the global data to the caller-provided record.
	//
	QCFSIGNATURE* pData = (QCFSIGNATURE *) ::GlobalLock(hGlobal);
	if (pData)
	{
		qcfStationery = *pData;
		ASSERT(qcfStationery.IsValid());
		::GlobalUnlock(hGlobal);
		return TRUE;
	}
	else
	{
		ASSERT(0);
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// GetGlobalData [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::GetGlobalData(COleDataObject* pOleDataObject, QCFWAZOO& qcfWazoo)
{
	ASSERT(pOleDataObject);
	ASSERT(m_uClipboardFormatTable[CF_WAZOO]);				// forget to call RegisterAllFormats()?

	::ZeroMemory(&qcfWazoo, sizeof(QCFWAZOO));		// good hygiene

	if (! pOleDataObject->IsDataAvailable(CLIPFORMAT(m_uClipboardFormatTable[CF_WAZOO])))
		return FALSE;		// that was easy

	// Get global clipboard data from COleDataObject
	HGLOBAL hGlobal = pOleDataObject->GetGlobalData(CLIPFORMAT(m_uClipboardFormatTable[CF_WAZOO]));
	if (NULL == hGlobal)
	{
		ASSERT(0);
		return FALSE;
	}

	//
	// Copy the global data to the caller-provided record.
	//
	QCFWAZOO* pData = (QCFWAZOO *) ::GlobalLock(hGlobal);
	if (pData)
	{
		qcfWazoo = *pData;
		ASSERT(qcfWazoo.IsValid());
		::GlobalUnlock(hGlobal);
		return TRUE;
	}
	else
	{
		ASSERT(0);
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////
// IsInstanceDataAvailable [public]
//
// Handy shortcut for a commonly used code sequence.
////////////////////////////////////////////////////////////////////////
BOOL QCClipboardMgr::IsInstanceDataAvailable(COleDataObject* pOleDataObject, ClipboardFormat cfId)
{
	ASSERT(pOleDataObject);

	if (cfId != CF_QCWILDCARD && (cfId < 0 || cfId >= CF_NUMFORMATS))
	{
		ASSERT(0);
		return FALSE;
	}

	if (CF_QCWILDCARD == cfId)
	{
		for (int i = 0; i < CF_NUMFORMATS; i++)
		{
			//
			// Tricky recursion...
			//
			if (IsInstanceDataAvailable(pOleDataObject, ClipboardFormat(i)))
				return TRUE;
		}
		
		return FALSE;
	}

	ASSERT(m_uClipboardFormatTable[cfId]);				// forget to call RegisterAllFormats()?

	// Get global clipboard data from COleDataObject
	HGLOBAL hGlobal = pOleDataObject->GetGlobalData(CLIPFORMAT(m_uClipboardFormatTable[cfId]));
	if (NULL == hGlobal)
	{
		//
		// Nope.  No data of that type available.
		//
		return FALSE;
	}

	//
	// Get pointer to global data (which stores the instance handle
	// for the drop source) and check whether or not we're dragging and
	// dropping in the same instance of Eudora.
	//
	QCFBASE* pData = (QCFBASE *) ::GlobalLock(hGlobal);
	if (pData)
	{
		if (::AfxGetInstanceHandle() == pData->m_hInstance)
		{
			::GlobalUnlock(hGlobal);
			return TRUE;
		}

		::GlobalUnlock(hGlobal);
	}
	else
	{
		ASSERT(0);
	}

	return FALSE;
}


