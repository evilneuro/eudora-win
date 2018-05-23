
#include "WabInterface.h"

#include "wab.h"
#include <exception>

#define	TRACE(foo)	

//	Open the WAB library
WabInterface::WabInterface () {
	TCHAR  szWABDllPath[MAX_PATH];
	DWORD  dwType = 0;
	ULONG  cbData = sizeof(szWABDllPath);
	HKEY hKey = NULL;

	fWabLibraryInstance = NULL;
	*szWABDllPath = '\0';

//	First we look under the default WAB DLL path location in the Registry. 
//	WAB_DLL_PATH_KEY is defined in wabapi.h
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
		RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szWABDllPath, &cbData);

	if(hKey) RegCloseKey(hKey);

//	if the Registry came up blank, we do a loadlibrary on the wab32.dll
//	WAB_DLL_NAME is defined in wabapi.h
	fWabLibraryInstance = LoadLibrary((lstrlen(szWABDllPath)) ? szWABDllPath : WAB_DLL_NAME );

//	If we still haven't succeeded, then bail
	if ( !fWabLibraryInstance ) {
		TRACE ( "Can't load Wab Library" );
	}
}


//	Close the library
WabInterface::~WabInterface () {
	if ( fWabLibraryInstance )
		FreeLibrary ( fWabLibraryInstance );
}


WabFilePtr WabInterface::OpenWabFile ( const char *fileName ) {
	
	if ( !fWabLibraryInstance ) {
		TRACE ( "Library not open" );
		return NULL;
	}

	LPWABOPEN wabOpen = (LPWABOPEN) GetProcAddress ( fWabLibraryInstance, "WABOpen" );
	if ( !wabOpen ) {
		TRACE ( "Can't load entry point" );
		return NULL;
		}

	HRESULT hr;
	LPWABOBJECT		wab;
	LPADRBOOK		book;

	WAB_PARAM wp	= {0};
	wp.cbSize		= sizeof(WAB_PARAM);
	wp.szFileName	= (LPTSTR) fileName;

//	if we choose not to pass in a WAB_PARAM object, 
//	the default WAB file will be opened up
	hr = wabOpen ( &book, &wab, &wp, 0 );
	if ( hr ) {
		TRACE ( "Can't open address book" );
		return NULL;
		}

	return WabFilePtr ( new WabFile ( wab, book ));
}



WabFile::WabFile ( LPWABOBJECT wab, LPADRBOOK book ) : fWab ( wab ), fBook ( book ) {}
WabFile::~WabFile () {
	fBook->Release ();
	fWab->Release ();
}


//	Enumerate the entries in a WAB file, returning only the fields
//	that the caller is interested in.
HRESULT WabFile::ForEachMailAddressDo ( LPSPropTagArray props, OneMailAddr eachMail ) {
    HRESULT			hr =			E_FAIL;
    ULONG			ulObjType =		0;
	LPMAPITABLE		lpAB =			NULL;
    ULONG			cRows =			0;
    LPABCONT		lpContainer =	NULL;

    ULONG lpcbEID;
	LPENTRYID lpEID = NULL;

//	Get the entryid of the root PAB container
	hr = fBook->GetPAB ( &lpcbEID, &lpEID );

//	Open the root PAB container
//	This is where all the WAB contents reside
	ulObjType = 0;
	hr = fBook->OpenEntry ( lpcbEID, (LPENTRYID)lpEID, NULL, 0,
							&ulObjType, (LPUNKNOWN *) &lpContainer );
	fWab->FreeBuffer ( lpEID );
	lpEID = NULL;

    if ( HR_SUCCEEDED ( hr )) {

	// Get a contents table of all the contents in the
	// WABs root container
		hr = lpContainer->GetContentsTable( 0, &lpAB );

		if ( HR_SUCCEEDED ( hr )) {

		//	Order the columns in the ContentsTable to conform to the
		//	ones we want
			hr = lpAB->SetColumns ( props, 0 );

			if ( HR_SUCCEEDED ( hr )) {

			//	Reset to the beginning of the table
				hr = lpAB->SeekRow( BOOKMARK_BEGINNING, 0, NULL );

				if( HR_SUCCEEDED ( hr )) {

				//	Read all the rows of the table one by one
					int			cNumRows;
					do {
						LPSRowSet	lpRowAB;

						lpRowAB = NULL;
						hr = lpAB->QueryRows(1,	0, &lpRowAB );

						if ( HR_FAILED ( hr ) || !lpRowAB )
							break;

						cNumRows = lpRowAB->cRows;

						if ( cNumRows ) {
							if ( !eachMail ( &lpRowAB->aRow[0] ))
								break;
						//	Free the memory that was allocated in queryRows
							for ( int i = 0; i < cNumRows; ++i )
								fWab->FreeBuffer ( lpRowAB->aRow[i].lpProps );
							fWab->FreeBuffer ( lpRowAB );
						}

					} while ( cNumRows );
				}
			}
		}
		if ( lpContainer )
			lpContainer->Release();
	}

	if ( lpAB )
		lpAB->Release();

	return hr;
}
