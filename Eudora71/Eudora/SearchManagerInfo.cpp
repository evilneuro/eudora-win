//	SearchManagerInfo.cpp
//
//	Search Manager information and related methods.
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
#include "SearchManagerInfo.h"
#include "SearchManagerPersistence.h"
#include "SearchManagerUtils.h"

#include "DebugNewHelpers.h"
#include "resource.h"
#include "rs.h"
#include "tocdoc.h"
#include "XMLWriter.h"


// ---------------------------------------------------------------------------
//		* UpdateIdentifier											 [Public]
// ---------------------------------------------------------------------------
//	UpdateIdentifier constructor.

SearchManager::UpdateIdentifier::UpdateIdentifier(
	const char *				in_szMailboxPath,
	long						in_nMessageID)
	:	m_szMailboxRelativePath(in_szMailboxPath),
		m_nMessageID(in_nMessageID)
{
}


// ---------------------------------------------------------------------------
//		* UpdateIdentifier											 [Public]
// ---------------------------------------------------------------------------
//	UpdateIdentifier constructor.

bool
SearchManager::UpdateIdentifier::operator <(
	const UpdateIdentifier &	in_rhs) const
{
	int		nCompare = m_szMailboxRelativePath.CompareNoCase(in_rhs.m_szMailboxRelativePath);

	if (nCompare == 0)
		return m_nMessageID < in_rhs.m_nMessageID;
	else
		return nCompare < 0;
}


// ---------------------------------------------------------------------------
//		* IndexUpdateAction											 [Public]
// ---------------------------------------------------------------------------
//	IndexUpdateAction constructor.

SearchManager::IndexUpdateAction::IndexUpdateAction(
	UpdateActionEnum			in_eUpdateAction,
	const char *				in_szMailboxPath,
	long						in_nMessageID)
	:	m_eUpdateAction(in_eUpdateAction),
		m_szMailboxRelativePath(in_szMailboxPath),
		m_nMessageID(in_nMessageID), m_bIsUpdate(false)
{
}


// ---------------------------------------------------------------------------
//		* Info														 [Public]
// ---------------------------------------------------------------------------
//	Info constructor.

SearchManager::Info::Info()
	:	m_bInitializationFailed(false), m_bIndexedSearchOnCached(true),
		m_bNeedToInitiateScan(false), m_hX1Lib(NULL), m_pX1(),
		m_pX1ScannerManager(), m_pX1EmailDB(), m_pX1Search(),
		m_pX1EventListener(), m_pIX1EventListener(), m_pIX1DBEventListener(),
		m_pX1EmailScanner(), m_pX1DllGetClassObject(NULL), m_pX1DllCanUnloadNow(NULL),
		m_eScanType(X1EmailScanner::st_None), m_eNextScanType(X1EmailScanner::st_None),
		m_bPauseExtendedIdleProcessing(false),
		m_eDirty(d_NotDirty), m_nLastSavedTickCount(0),
		m_szIndexingMailboxRelativePath(), m_mapIndexUpdateActions(),
		m_nNumMailboxUpdateActions(0), m_nLastScanCompletionTime(0),
		m_bX1DBNeedsCleared(false), m_bX1ExceptionFlagged(false)
{
	//	Above m_nLastScanCompletionTime defaults to 0 so that we know to initiate
	//	an indexing scan immediately.
}


// ---------------------------------------------------------------------------
//		* ~Info														 [Public]
// ---------------------------------------------------------------------------
//	Info destructor.

SearchManager::Info::~Info()
{
	//	Safe to call whether or not X1 is still loaded
	ReleaseX1();

	//	Delete m_mapIndexUpdateActions items, but don't bother erasing the list
	//	because it is about to be destroyed anyway.
	for ( IndexUpdateActionMapIteratorT updateActionIterator = m_mapIndexUpdateActions.begin();
		  updateActionIterator != m_mapIndexUpdateActions.end();
		  updateActionIterator++ )
	{
		delete updateActionIterator->second;
	}
}


// ---------------------------------------------------------------------------
//		* Initialize												 [Public]
// ---------------------------------------------------------------------------
//	Loads SearchManager data and initializes X1.

void
SearchManager::Info::Initialize()
{
	//	Get the search directory
	CString		szSearchDirectory;
	
	if ( GetDirectory(szSearchDirectory) )
	{
		//	Erase the previous sort columns
		m_sortColumns.erase( m_sortColumns.begin(), m_sortColumns.end() );
		
		//	Load previously saved data
		LoadData();

		if ( m_sortColumns.empty() )
		{
			//	If no previous setting, default to sorting by date
			m_sortColumns.push_back(BY_DATE);
		}

		//	Init X1
		InitX1();

		//	If we haven't completed an initial scan yet, then we need to
		//	save our data so that the X1 schema version gets written.
		if (m_nLastScanCompletionTime == 0)
			SaveData();

		//	Mark not dirty
		SetDirty(d_NotDirty);

		//	Note that information is fresh
		m_nLastSavedTickCount = GetTickCount();
	}
	else
	{
		//	Couldn't create our directory
		m_bInitializationFailed = true;
	}
}


// ---------------------------------------------------------------------------
//		* SetDirty													 [Public]
// ---------------------------------------------------------------------------
//	Loads SearchManager data.

void
SearchManager::Info::SetDirty(
	DirtyEnum					in_eDirty,
	bool						in_bNoSmartSet)
{
	//	Go ahead and accept the new value if any of the following is true:
	//	* We weren't dirty before (no need for smart combination)
	//	* We're being set to clean (trust caller)
	//	* The new value matches the old value (no change)
	//	* We were told not to bother with any smart combination
	if ( (m_eDirty == d_NotDirty) || (in_eDirty == d_NotDirty) ||
		 (m_eDirty == in_eDirty) || in_bNoSmartSet )
	{
		m_eDirty = in_eDirty;
	}
	else
	{
		//	None of the above is true, so we know that we're combining
		//	two different dirty values (or were already set to both dirty),
		//	so go ahead and set to both dirty.
		m_eDirty = d_BothDirty;
	}
}


// ---------------------------------------------------------------------------
//		* LoadData													 [Public]
// ---------------------------------------------------------------------------
//	Loads SearchManager data.

void
SearchManager::Info::LoadData()
{
	//	Get the search directory
	CString		szSearchDirectory;

	if ( !GetDirectory(szSearchDirectory) )
	{
		//	Nothing for us to do, our directory doesn't exist and couldn't
		//	be created. Probably need to display an error because X1
		//	won't be initialized correctly.
		return;
	}

	//	Form the path to our file in the search directory
	CString		szFilePathName;
	szFilePathName.Format( "%s\\%s", szSearchDirectory, CRString(IDS_IDX_SRCH_MGR_FILENAME) );
		
	//	Check to see if our file exists
	if ( !::FileExistsMT(szFilePathName) )
	{
		//	Log the error, but this is normal when Eudora is first run.
		CString		szLogEntry;

		szLogEntry.Format( "Search data file %s does not exist. Existing search index files will be removed.",
							szFilePathName );
		PutDebugLog(DEBUG_MASK_SEARCH, szLogEntry);
		
		//	Existing X1 DB files need to be removed, because there's no way for
		//	us to verify that the schema format matches what we expect.
		m_bX1DBNeedsCleared = true;
		
		return;
	}

	//	Open our file
	int		hFile = open(szFilePathName, _O_RDONLY | _O_TEXT);
	if (hFile == -1)
	{
		ASSERT(!"Could not open Search Manager data file");
		
		//	Log the error. This is not normal, and is extremely bad.
		CString		szLogEntry;

		szLogEntry.Format( "Search data file %s could not be opened. Existing search index files will be removed.",
							szFilePathName );
		PutDebugLog(DEBUG_MASK_SEARCH, szLogEntry);

		//	Existing X1 DB files need to be removed, because there's no way for
		//	us to verify that the schema format matches what we expect.
		//	This may prove to be overly harsh, but not being able to open the
		//	data file (read only!) is really unexpected unless things are
		//	somehow screwed up.
		m_bX1DBNeedsCleared = true;
		
		return;
	}

	bool	bLoadGood = false;

	//	Allocated a buffer big enough for the entire file contents (only a few K)
	long					nLength = lseek(hFile, 0, SEEK_END);
	std::auto_ptr<char>		szFileBuf( DEBUG_NEW_NOTHROW char [nLength+1] );

	if ( szFileBuf.get() )
	{
		//	Assume that the X1 DB files need to be removed until proved otherwise
		//	by successfully finding the correct schema version in our XML file.
		m_bX1DBNeedsCleared = true;
		
		//	Load the entire file contents into our buffer
		lseek(hFile, 0, SEEK_SET);
		read(hFile, szFileBuf.get(), nLength);
		szFileBuf.get()[nLength] = 0;

		//	Parse the entire file in one fell swoop
		XMLParser		xmlParser;
		bLoadGood = (xmlParser.Parse(szFileBuf.get(), nLength, true) == 0);
	}

	close(hFile);
}


// ---------------------------------------------------------------------------
//		* SaveData													 [Public]
// ---------------------------------------------------------------------------
//	Saves SearchManager data.

void
SearchManager::Info::SaveData()
{
	//	Form the path to our file in our data directory
	CString		szSearchDirectory;

	if ( !GetDirectory(szSearchDirectory) )
	{
		//	Nothing for us to do, our directory doesn't exist and couldn't
		//	be created.
		return;
	}

	CString		szFilePathName = szSearchDirectory;
	szFilePathName.Format( "%s\\%s", szSearchDirectory, CRString(IDS_IDX_SRCH_MGR_FILENAME) );

	CString		szTempFilePathName;

	HRESULT		hr = GetTempFileName( szSearchDirectory, "ISM", 0, szTempFilePathName.GetBuffer(MAX_PATH + 1) );
	if ( SUCCEEDED(hr) )
		szTempFilePathName.ReleaseBuffer();
	else
		szTempFilePathName = "ISM-temp.tmp";

	JJFile		file;

	hr = file.Open(szTempFilePathName, O_CREAT | O_TRUNC | O_WRONLY);

	if ( SUCCEEDED(hr) )
	{
		//	Open succeeded - write out the XML
		XMLWriter		xmlWriter(file);

		//	Start <IndexedSearchData>
		xmlWriter.WriteTagStart(XMLParser::kXMLBaseContainer, true);

		//	Write DataFormatVersion tag
		xmlWriter.WriteTaggedData(XMLParser::kKeyDataFormatVersion, true, "%d", SearchManager::XMLParser::kDataFormatVersion);

		//	Write SchemaVersion tag
		xmlWriter.WriteTaggedData(XMLParser::kKeyX1SchemaVersion, true, "%d", Info::kDBSchemaVersion);

		//	Write IndexingScanType tag
		xmlWriter.WriteTaggedData(XMLParser::kKeyIndexingScanType, true, "%d", m_eScanType);

		//	Write IndexingNextScanType tag
		xmlWriter.WriteTaggedData(XMLParser::kKeyIndexingNextScanType, true, "%d", m_eNextScanType);

		//	Write IndexingMailbox tag
		xmlWriter.WriteTaggedData(XMLParser::kKeyIndexingMailbox, true, "%s", m_szIndexingMailboxRelativePath);

		//	Write IndexingCompletionTime tag
		xmlWriter.WriteTaggedData(XMLParser::kKeyIndexingCompletionTime, true, "%d", m_nLastScanCompletionTime);

		//	Start <SortColumns>
		xmlWriter.WriteTagStart(XMLParser::kKeySortColumns, true);

		//	Count how many legitimate columns we have
		for ( SortColumnsIteratorT sortColumnsIterator = m_sortColumns.begin();
			  sortColumnsIterator != m_sortColumns.end();
			  sortColumnsIterator++ )
		{
			//	Write SortBy tag
			xmlWriter.WriteTaggedData(XMLParser::kKeySortBy, true, "%d", *sortColumnsIterator);
		}

		//	Close </SortColumns>
		xmlWriter.WriteTagEnd(XMLParser::kKeySortColumns, true);

		//	Start <IndexUpdateActions>
		xmlWriter.WriteTagStart(XMLParser::kKeyIndexUpdateActions, true);

		//	Write index update actions
		for ( IndexUpdateActionMapIteratorT updateActionIterator = m_mapIndexUpdateActions.begin();
			  updateActionIterator != m_mapIndexUpdateActions.end();
			  updateActionIterator++ )
		{
			IndexUpdateAction *		pUpdateAction = updateActionIterator->second;
			
			if (pUpdateAction == NULL)
			{
				ASSERT(!"Memory error in SearchManager::Info::SaveData");
				continue;
			}

			//	Start <UpdateAction>
			xmlWriter.WriteTagStart(XMLParser::kKeyUpdateAction, true);
			
			//	Write Action tag
			xmlWriter.WriteTaggedData( XMLParser::kKeyActionType, true, "%d", pUpdateAction->GetUpdateAction() );

			//	Write MailboxPath tag
			xmlWriter.WriteTaggedData( XMLParser::kKeyMailboxRelativePath, true, "%s", pUpdateAction->GetMailboxRelativePath() );

			//	Write MessageID tag
			xmlWriter.WriteTaggedData( XMLParser::kKeyMessageID, true, "%d", pUpdateAction->GetMessageID() );

			//	Close </UpdateAction>
			xmlWriter.WriteTagEnd(XMLParser::kKeyUpdateAction, true);
		}

		//	Close </IndexUpdateActions>
		xmlWriter.WriteTagEnd(XMLParser::kKeyIndexUpdateActions, true);

		//	Close </IndexedSearchData>
		xmlWriter.WriteTagEnd(XMLParser::kXMLBaseContainer, true);

		//	Close the file
		hr = file.Close();
		ASSERT( SUCCEEDED(hr) );

		//	Rename the temp file to our final name
		CString		szFilePathName;
		szFilePathName.Format( "%s\\%s", szSearchDirectory, CRString(IDS_IDX_SRCH_MGR_FILENAME) );

		hr = file.Rename(szFilePathName);
		ASSERT( SUCCEEDED(hr) );

		//	If renaming failed, clean up the temp file
		if ( FAILED(hr) )
			FileRemoveMT(szTempFilePathName);
	}
	else
	{
		ASSERT(!"Failed to open indexed search temp file");
		
		CString		szLogEntry;

		szLogEntry.Format("Failed to open indexed search temp file: %s", szTempFilePathName);

		PutDebugLog(DEBUG_MASK_SEARCH, szLogEntry);
		FileRemoveMT(szTempFilePathName);
	}
}


// ---------------------------------------------------------------------------
//		* InitX1													 [Public]
// ---------------------------------------------------------------------------
//	

HRESULT
SearchManager::Info::InitX1()
{	
	HRESULT		hr = ERROR_SUCCESS;
	
	try
	{
		//	Make sure that we can create/get the directory where X1 will
		//	store its indices
		CString		szSearchDir;

		//	Attempt to get the directory. Passing true makes it throw
		//	upon failure.
		GetDirectory(szSearchDir, true);
		
		hr = LoadX1();

		if ( FAILED(hr) )
		{
			//	LoadX1 has already done all the asserting and logging for us.
			//	Just release any X1 memory and return the error.
			ReleaseX1();

			m_bInitializationFailed = true;

			return hr;
		}

		//	Turn on DiskIO-based performance monitoring. This allows X1 to automatically
		//	yield to other applications that need disk access when scanning in the
		//	background. 
		//	Note: We must still indicate what CPU usage level we want by calling
		//	SetCPUUsage when switching between foreground and background
		//	(see OnActivateApp).
		m_pX1->EnableDiskIOBasedPerformanceMonitoring();

		//	Create the event listener
		m_pX1EventListener = DEBUG_NEW X1EventListener();

		//	Get IX1EventListener and IX1DbEventListener interfaces for use in SetupEvents
		//	calls below. These look like simple assignments, but QueryInterface is called
		//	through the magic of _com_ptr_t (template class that generated the smart pointers).
		m_pIX1EventListener = static_cast<X1EventListener *>( m_pX1EventListener );
		m_pIX1DBEventListener = static_cast<X1EventListener *>( m_pX1EventListener );

		//	Setup X1 events
		m_pX1->SetupEvents(m_pIX1EventListener);

		//	Init X1 database
		InitX1DB();

		if (m_bX1DBNeedsCleared)
		{
			//	Log that DB needs to be cleared
			ASSERT(!"Erasing X1 indices because DB schema was missing or doesn't match");
			PutDebugLog( DEBUG_MASK_SEARCH, "Erasing X1 indices because DB schema was missing or doesn't match" );
			
			//	Tell X1 to erase all of its index files
			m_pX1EmailDB->Erase();

			//	Reset our last scan time to zero
			m_nLastScanCompletionTime = 0;

			//	We'll want to start over with any scanning
			m_szIndexingMailboxRelativePath.Empty();

			//	Set this back to false just to be safe
			m_bX1DBNeedsCleared = false;
		}

		//	Create the email scanner
		m_pX1EmailScanner = DEBUG_NEW X1EmailScanner(m_pX1EmailDB);

		//	Get the scanner manager
		m_pX1->GetScannerManager(&m_pX1ScannerManager);

		//	Connect the email scanner to the scanner manager
		UINT	hConnection = m_pX1ScannerManager->AddScanner(m_pX1EmailScanner, m_pX1EmailDB);
		m_pX1EmailScanner->SetConnectionHandle(hConnection);
	}
	catch (_com_error & e)
	{
		//	Get developer's attention
		ASSERT(!"X1 COM Error");

		//	Get the error code to return
		hr = e.Error();

		SearchManager::Utils::LogError( "COM", "InitX1", e.ErrorMessage() );
	}
	catch (CException * pException)
	{
		//	Get developer's attention
		ASSERT(!"X1 CException");
				
		//	Get the error code to return
		if ( pException->IsKindOf(RUNTIME_CLASS(CFileException)) )
			hr = reinterpret_cast<CFileException *>(pException)->m_lOsError;
		else if ( pException->IsKindOf(RUNTIME_CLASS(CMemoryException)) )
			hr = E_OUTOFMEMORY;
		else
			hr = E_UNEXPECTED;

		//	Log error
		SearchManager::Utils::LogCException(pException, "InitX1");

		//	Delete exception object
		pException->Delete();
	}

	//	If we failed, then free up any memory that we started to allocate
	if ( FAILED(hr) )
	{
		m_bInitializationFailed = true;
		
		ReleaseX1();
	}

	return hr;
}


// ---------------------------------------------------------------------------
//		* LoadX1													 [Public]
// ---------------------------------------------------------------------------
//	Loads the X1 COM library.
//
//	The X1 folks recommend manually loading the X1 COM library instead of
//	using CoCreateInstance. Manually loading entails loading the DLL
//	(via LoadLibrary) and then using the exported class factory function to
//	create an instance of X1. By doing a manual load of the DLL, we ensure
//	that we always load the correct version of the DLL from the correct
//	location.

HRESULT
SearchManager::Info::LoadX1()
{
	//	Load x1lib.dll - should be in the same directory as Eudora
	CString		szX1Path = ExecutableDir + "\\x1lib.dll"; 

	m_hX1Lib = LoadLibrary(szX1Path);

	ASSERT(m_hX1Lib);
	if (!m_hX1Lib)
	{
		ASSERT( !"Failed to load x1lib.dll" );
		PutDebugLog( DEBUG_MASK_SEARCH, "Failed to load x1lib.dll" );
		return MAKE_HRESULT(1, FACILITY_ITF, ERROR_DLL_NOT_FOUND);
	}

	m_pX1DllGetClassObject = reinterpret_cast<LPFNGETCLASSOBJECT>( GetProcAddress(m_hX1Lib, "DllGetClassObject") );
	if (!m_pX1DllGetClassObject)
	{
		ASSERT( !"Failed to get address for X1DllGetClassObject" );
		PutDebugLog( DEBUG_MASK_SEARCH, "Failed to get address for X1DllGetClassObject" );
		return MAKE_HRESULT(1, FACILITY_ITF, ERROR_DELAY_LOAD_FAILED);
	}

	//	Use the DllCanUnloadNow function to determine when it's safe to unload the DLL.
	//	Used in UninitX1 as a sanity check.
	m_pX1DllCanUnloadNow = reinterpret_cast<LPFNCANUNLOADNOW>( GetProcAddress(m_hX1Lib, "DllCanUnloadNow") );
	if (!m_pX1DllCanUnloadNow)
	{
		ASSERT( !"Failed to get address for X1DllCanUnloadNow" );
		PutDebugLog( DEBUG_MASK_SEARCH, "Failed to get address for X1DllCanUnloadNow" );
		return MAKE_HRESULT(1, FACILITY_ITF, ERROR_DELAY_LOAD_FAILED);
	}

	//	Get class factory
	IClassFactory *		pClassFactory;
	HRESULT				hr = m_pX1DllGetClassObject( X1::CLSID_X1, IID_IClassFactory, reinterpret_cast<void **>(&pClassFactory) );
	if ( FAILED(hr) )
	{
		ASSERT( !"X1DllGetClassObject failed to get class factory for X1" );
		PutDebugLog( DEBUG_MASK_SEARCH, "X1DllGetClassObject failed to get class factory for X1" );
		return hr;
	}

	//	Get X1
	hr = pClassFactory->CreateInstance( NULL, X1::IID_IX1, reinterpret_cast<void **>(&m_pX1) );
	pClassFactory->Release();
	pClassFactory = NULL;
	if ( FAILED(hr) )
	{
		ASSERT( !"X1 class factory failed to create X1" );
		PutDebugLog( DEBUG_MASK_SEARCH, "X1 class factory failed to create X1" );
		return hr;
	}

	//	Create the search object.
	//	We do a manual load here as above for the X1 interface.
	hr = m_pX1DllGetClassObject( X1::CLSID_X1Search, IID_IClassFactory, reinterpret_cast<void **>(&pClassFactory) );
	if ( FAILED(hr) )
	{
		ASSERT( !"X1DllGetClassObject failed to get class factory for X1 Search" );
		PutDebugLog( DEBUG_MASK_SEARCH, "X1DllGetClassObject failed to get class factory for X1 Search" );
		return hr;
	}

	hr = pClassFactory->CreateInstance( NULL, X1::IID_IX1Search, reinterpret_cast<void **>(&m_pX1Search) );
	pClassFactory->Release();
	pClassFactory = NULL;
	if ( FAILED(hr) )
	{
		ASSERT( !"X1 Search class factory failed to create X1 Search" );
		PutDebugLog( DEBUG_MASK_SEARCH, "X1 Search class factory failed to create X1 Search" );
		return hr;
	}

	return ERROR_SUCCESS;
}


// ---------------------------------------------------------------------------
//		* InitX1DB													 [Public]
// ---------------------------------------------------------------------------
//	Creates directory for X1 DB files, creates X1 DB, and sets up X1 DB

void
SearchManager::Info::InitX1DB()
{
	//	Get the directory where all the X1 files will be stored. Also makes
	//	sure DB exists, which is important before calling CreateDatabase.
	CString		szSearchDir;
	
	GetDirectory(szSearchDir, true);

	//	Create X1 database
	m_pX1->CreateDatabase( kDBID, _bstr_t(L"SearchIndex"), _bstr_t(L"SearchIndex"),
						   0, _bstr_t(szSearchDir), _bstr_t(L"QUALCOMM"), &m_pX1EmailDB );

	//	Setup listener for database events
	m_pX1EmailDB->SetupEvents(m_pIX1DBEventListener);
	
	//	*** Setup fields for the X1 email DB ***
	//
	//	Note: The order in which we setup the fields is important. We must first
	//	setup all indexed fields, followed by all "extra" fields (fields which are
	//	not themselves indexed, but instead makeup a larger field), followed by the
	//	one and only "cols" field. The "cols" field is a special field where field
	//	data is actually stored.
	m_pX1EmailDB->SetupFieldCount(efid_TotalNumFields);

	//	Subject: header
	m_pX1EmailDB->SetupField( efid_Subject, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexed, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	All Headers
	m_pX1EmailDB->SetupField( efid_AllHeaders, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexed, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Body
	m_pX1EmailDB->SetupField( efid_Body, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexed, kIsBig, kIsNotColumn,
							  kIsMainContent, kUseDefaultMaxWordLength );
	//	Mailbox relative path
	m_pX1EmailDB->SetupField( efid_MailboxRelativePath, _bstr_t(L"mailboxpath"), kIgnore, kIgnore,
							  X1::fdtString, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Mailbox name
	m_pX1EmailDB->SetupField( efid_MailboxName, _bstr_t(L"mailboxname"), kIgnore, kIgnore,
							  X1::fdtString, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Message ID
	m_pX1EmailDB->SetupField( efid_MessageID, _bstr_t(L"id"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Who
	m_pX1EmailDB->SetupField( efid_Who, _bstr_t(L"who"), kIgnore, kIgnore,
							  X1::fdtString, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	From: header
	m_pX1EmailDB->SetupField( efid_FromHeader, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	To: header
	m_pX1EmailDB->SetupField( efid_ToHeader, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Cc: header
	m_pX1EmailDB->SetupField( efid_CcHeader, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Bcc: header
	m_pX1EmailDB->SetupField( efid_BccHeader, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Attachments
	m_pX1EmailDB->SetupField( efid_Attachments, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Date (in seconds)
	m_pX1EmailDB->SetupField( efid_DateSeconds, _bstr_t(L"date"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Date days as roman numeral
	m_pX1EmailDB->SetupField( efid_DateDaysRomanNumeral, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	State/Status
	m_pX1EmailDB->SetupField( efid_State, _bstr_t(L"state"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Label
	m_pX1EmailDB->SetupField( efid_Label, _bstr_t(L"label"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Personality
	m_pX1EmailDB->SetupField( efid_PersonaHash, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Priority
	m_pX1EmailDB->SetupField( efid_PriorityNumber, _bstr_t(L"priority"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Priority encoded as a roman numeral - for ranged junk searches
	m_pX1EmailDB->SetupField( efid_PriorityRomanNumeral, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Junk score number
	m_pX1EmailDB->SetupField( efid_JunkScoreNumber, _bstr_t(L"junk"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Junk score encoded as a roman numeral - for ranged junk searches
	m_pX1EmailDB->SetupField( efid_JunkScoreRomanNumeral, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Attachment count number
	m_pX1EmailDB->SetupField( efid_AttachmentCountNumber, _bstr_t(L"numattach"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Attachment count encoded as a roman numeral - for ranged attachment count searches
	m_pX1EmailDB->SetupField( efid_AttachmentCountRomanNumeral, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Size
	m_pX1EmailDB->SetupField( efid_SizeNumber, _bstr_t(L"size"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Size encoded as a roman numeral - for ranged size searches
	m_pX1EmailDB->SetupField( efid_SizeRomanNumeral, _bstr_t(L""), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftIndexedExtra, kIsSmall, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	IMAP flags (currently only storing deleted status)
	m_pX1EmailDB->SetupField( efid_ImapFlags, _bstr_t(L"imapflags"), kIgnore, kIgnore,
							  X1::fdtString, X1::ftIndexedExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Mood
	m_pX1EmailDB->SetupField( efid_Mood, _bstr_t(L"mood"), kIgnore, kIgnore,
							  X1::fdtUint32, X1::ftExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Time Zone
	m_pX1EmailDB->SetupField( efid_TimeZoneMinutes, _bstr_t(L"timezone"), kIgnore, kIgnore,
							  X1::fdtInt64, X1::ftExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Subject: header prefix (i.e. Re:, Fwd:, etc.)
	m_pX1EmailDB->SetupField( efid_SubjectPrefix, _bstr_t(L"subject-pre"), kIgnore, kIgnore,
							  X1::fdtString, X1::ftExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Subject: header main topic (i.e. without prefix)
	m_pX1EmailDB->SetupField( efid_SubjectMain, _bstr_t(L"subject"), kIgnore, kIgnore,
							  X1::fdtString, X1::ftExtra, kIsSmall, kIsColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );
	//	Special field for X1 Column Info
	m_pX1EmailDB->SetupField( efid_X1ColumnInfo, _bstr_t(L"cols"), kIgnore, kIgnore,
							  X1::fdtOther, X1::ftCols, kIsBig, kIsNotColumn,
							  kIsNotMainContent, kUseDefaultMaxWordLength );

	//	*** Setup combined fields with SetupFieldComponents ***
	COleSafeArray	componentFields;
	int *			pFields;

	//	Prep a variant object for setting up subject field
	V_VT(&componentFields) = VT_ARRAY | VT_INT;
	V_ARRAY(&componentFields) = SafeArrayCreateVector(VT_INT, 0, 2);

	componentFields.AccessData( reinterpret_cast<void **>(&pFields) );
	pFields[0] = efid_SubjectPrefix;
	pFields[1] = efid_SubjectMain;
	componentFields.UnaccessData();

	//	Setup the subject field as a composite of the subject prefix and the main
	//	subject text. This is done to reduce the database size.
	//	We'll sort by the main portion of the subject.
	m_pX1EmailDB->SetupFieldComponents(efid_Subject, efid_SubjectMain, componentFields);

	//	Release subject field memory
	componentFields.Clear();

	//	Setup the mapping between user visible column names and fields. This allows
	//	users to use the "column-name:" prefix syntax when searching (e.g. "to:foo"
	//	to search in the efid_To field). The "column-name:" syntax is used when
	//	the user searches "Anywhere" using the "matches X1 query".
	m_pX1EmailDB->SetupColumn(_bstr_t(L"mailboxpath"), efid_MailboxRelativePath);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"mailboxname"), efid_MailboxName);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"id"), efid_MessageID);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"who"), efid_Who);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"subject"), efid_Subject);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"from"), efid_FromHeader);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"to"), efid_ToHeader);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"cc"), efid_CcHeader);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"bcc"), efid_BccHeader);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"headers"), efid_AllHeaders);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"body"), efid_Body);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"attachment"), efid_Attachments);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"date"), efid_DateSeconds);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"status"), efid_State);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"label"), efid_Label);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"priority"), efid_PriorityNumber);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"junk"), efid_JunkScoreNumber);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"attachmentcount"), efid_AttachmentCountNumber);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"size"), efid_SizeNumber);
	m_pX1EmailDB->SetupColumn(_bstr_t(L"imapflags"), efid_ImapFlags);
}


// ---------------------------------------------------------------------------
//		* ReleaseX1													 [Public]
// ---------------------------------------------------------------------------
//	Release all X1 related memory.

void
SearchManager::Info::ReleaseX1()
{
	//	Save our information one last time
	SaveData();
	
	//	We use smart pointers. When we release the pointer is automatically
	//	also set to NULL. The CComPtr-based smart pointers can handle us
	//	calling Release without checking for NULL first. For the _com_ptr_t
	//	based smart pointers (X1::SomeTypePtr) we need to check to see if
	//	they exist before calling release, because they will report an error
	//	if we call Release when they contain NULL.
	
	//	Release scanner and scanner manager early on so that m_pX1IndexedItems
	//	inside of m_pX1EmailScanner can be released safely.
	m_pX1EmailScanner.Release();

	if (m_pX1ScannerManager)
		m_pX1ScannerManager.Release();

	if (m_pX1Search)
		m_pX1Search.Release();

	if (m_pX1EmailDB) 
		m_pX1EmailDB.Release();

	if (m_pIX1EventListener)
		m_pIX1EventListener.Release();

	if (m_pIX1DBEventListener)
		m_pIX1DBEventListener.Release();

	m_pX1EventListener.Release();

	if (m_pX1) 
		m_pX1.Release();

	if (m_hX1Lib)
	{
		//	Sanity check to DllCanUnloadNow to make sure we can unload the dll.
		if ( m_pX1DllCanUnloadNow && FAILED(m_pX1DllCanUnloadNow()) )
		{
			ASSERT( !"X1DllCanUnloadNow didn't want us to unload X1" );
			PutDebugLog( DEBUG_MASK_SEARCH, "X1DllCanUnloadNow didn't want us to unload X1" );
		}

		FreeLibrary(m_hX1Lib);
		m_hX1Lib = NULL;
	}

	//	Function pointers are no longer valid
	m_pX1DllGetClassObject = NULL;
	m_pX1DllCanUnloadNow = NULL;
}


// ---------------------------------------------------------------------------
//		* GetDirectory												 [Public]
// ---------------------------------------------------------------------------
//	Logs the specified error.

BOOL
SearchManager::Info::GetDirectory(
	CString &					out_szSearchDir,
	bool						in_bThrowExceptionOnFailure)
{
	//	Format the X1 directory location
	out_szSearchDir = EudoraDir + CRString(IDS_IDX_SRCH_MGR_DIR);

	//	Check to see if it exists
	BOOL		bDirectoryExists = FileExistsMT(out_szSearchDir);

	//	Create it if it doesn't exist
	if (!bDirectoryExists)
		bDirectoryExists = CreateDirectory(out_szSearchDir, NULL);

	ASSERT(bDirectoryExists);

	if (!bDirectoryExists)
	{
		//	Get the error code
		HRESULT		hr = HRESULT_FROM_WIN32( GetLastError() );
		
		//	Log failure
		CString		szX1DirectoryError;

		szX1DirectoryError.Format("Unable to create X1 search DB directory: %s", out_szSearchDir);
		PutDebugLog( DEBUG_MASK_SEARCH, szX1DirectoryError );
		
		//	Throw exception if caller wanted that
		if (in_bThrowExceptionOnFailure)
			AfxThrowFileException(CFileException::badPath, hr, out_szSearchDir);
	}

	return bDirectoryExists;
}

