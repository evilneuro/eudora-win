// CLIPBOARDMGR.H
//

#ifndef __CLIPBOARDMGR_H_
#define __CLIPBOARDMGR_H_


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

class CTocDoc;
class CSummary;
class CWazooBar;
class CWazooWnd;
class QCFTRANSFER;
class QCFSTATIONERY;
class QCFSIGNATURE;
class QCFWAZOO;

////////////////////////////////////////////////////////////////////////
// QCClipboardMgr
//
////////////////////////////////////////////////////////////////////////
class QCClipboardMgr
{
public:
	QCClipboardMgr();
	virtual ~QCClipboardMgr();

	//
	// The following enum values are used as array indexes, so use
	// caution when updating this list.
	//
	enum ClipboardFormat
	{
		CF_QCWILDCARD = -1,		// wildcard for any QC clipboard format
		CF_TRANSFER = 0,
		CF_STATIONERY,
		CF_SIGNATURE,
		CF_WAZOO,
		CF_NUMFORMATS			// must be last!
	};

	BOOL RegisterAllFormats();

	BOOL CacheGlobalData(COleDataSource& oleDataSource, const QCFTRANSFER& qcfTransfer);
	BOOL CacheGlobalData(COleDataSource& oleDataSource, const QCFSTATIONERY& qcfStationery);
	BOOL CacheGlobalData(COleDataSource& oleDataSource, const QCFSIGNATURE& qcfStationery);
	BOOL CacheGlobalData(COleDataSource& oleDataSource, const QCFWAZOO& qcfWazoo);

	BOOL GetGlobalData(COleDataObject* pOleDataObject, QCFTRANSFER& qcfTransfer);
	BOOL GetGlobalData(COleDataObject* pOleDataObject, QCFSTATIONERY& qcfStationery);
	BOOL GetGlobalData(COleDataObject* pOleDataObject, QCFSIGNATURE& qcfStationery);
	BOOL GetGlobalData(COleDataObject* pOleDataObject, QCFWAZOO& qcfWazoo);

	BOOL IsInstanceDataAvailable(COleDataObject* pOleDataObject, ClipboardFormat cfId);

protected:
	//
	// Storage for registered clipboard format IDs
	//
	UINT m_uClipboardFormatTable[CF_NUMFORMATS];

private:
	QCClipboardMgr(const QCClipboardMgr&);		// not implemented
	void operator=(const QCClipboardMgr&);		// not implemented
};


//
// There should only be one global instance of this class!
//
extern QCClipboardMgr g_theClipboardMgr;


//
// QCF (Qualcomm Clipboard Format) records.  These records contain the
// data that is stuffed into OLE data buffers while performing
// intra-application drag and drops.  Note that all records are
// derived from the common QCFBASE item, which contains the
// application's HINSTANCE.  We need to store the HINSTANCE in the so
// that we can check for drag and drop between multiple instances of
// Eudora.
//
class QCFBASE
{
public:
	QCFBASE() :
		m_hInstance(::AfxGetInstanceHandle())
	{
	}

	HINSTANCE	m_hInstance;			// handle for this running instance
};


class QCFTRANSFER : public QCFBASE
{
public:
	QCFTRANSFER() :
		m_pSourceTocDoc(NULL),
		m_pSingleSummary(NULL)
	{
	}

	BOOL IsValid() const
		{ return m_pSourceTocDoc != NULL; }

	CTocDoc*	m_pSourceTocDoc;		// TocDoc object for the message drag source
	CSummary*	m_pSingleSummary;		// non-NULL: use this Summary, NULL: use TOC selection
};


class QCFSTATIONERY : public QCFBASE
{
public:
	QCFSTATIONERY()
	{
		m_szPathName[0] = '\0';
	}

	BOOL IsValid() const
		{ return (strlen(m_szPathName) > 0) && (strlen(m_szPathName) < sizeof(m_szPathName)); }

	char		m_szPathName[MAX_PATH];	// full path name of the stationery file
};


class QCFSIGNATURE : public QCFBASE
{
public:
	QCFSIGNATURE()
	{
		m_szPathName[0] = '\0';
	}

	BOOL IsValid() const
		{ return (strlen(m_szPathName) > 0) && (strlen(m_szPathName) < sizeof(m_szPathName)); }

	char		m_szPathName[MAX_PATH];	// full path name of the stationery file
};


class QCFWAZOO : public QCFBASE
{
public:
	QCFWAZOO() :
		m_pSourceWazooBar(NULL),
		m_pSourceWazooWnd(NULL)
	{
	}

	BOOL IsValid() const
		{ return (m_pSourceWazooBar != NULL) && (m_pSourceWazooWnd != NULL); }

	CWazooBar*	m_pSourceWazooBar;		// source Wazoo Container
	CWazooWnd*	m_pSourceWazooWnd;		// source Wazoo Window
};


#endif // __CLIPBOARDMGR_H_


