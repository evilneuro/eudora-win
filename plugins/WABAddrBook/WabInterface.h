
#include <memory>	// for auto_ptr

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#include <wab.h>

class WabFile {
public:
	WabFile ( LPWABOBJECT wab, LPADRBOOK book );
	~WabFile ();

	typedef bool (*OneMailAddr) ( LPSRow );
	HRESULT ForEachMailAddressDo ( LPSPropTagArray props, OneMailAddr eachMail );
	LPADRBOOK GetBook () const { return fBook; }

private:
	WabFile ();
	WabFile ( const WabFile &rhs );
	WabFile & operator = ( const WabFile & rhs );

	LPWABOBJECT	fWab;
	LPADRBOOK	fBook;
};

// typedef	std::auto_ptr<WabFile> WabFilePtr;
typedef WabFile	*WabFilePtr;

class WabInterface {
public:
	WabInterface ();
	~WabInterface ();

	WabFilePtr OpenWabFile ( const char *fileName );
	HMODULE	GetModule ( ) const { return fWabLibraryInstance; }

private:
	WabInterface ( const WabInterface &rhs );
	WabInterface & operator = ( const WabInterface & rhs );

	HMODULE	fWabLibraryInstance;
};

