#ifndef REGIF_H
#define REGIF_H

#ifndef WIN32
	#define MAX_PATH			256
	#define HKEY_CURRENT_USER	HKEY_CLASSES_ROOT
	#define HKEY_LOCAL_MACHINE	HKEY_CLASSES_ROOT
	#define HKEY_USERS			HKEY_CLASSES_ROOT
#endif

class REGIF
{
	private:
		HKEY	m_hKeyRoot;
		char 	m_cStrRoot[ MAX_PATH + 1 ];

	public:
		REGIF();
		~REGIF();
		BOOL Attach( LPCSTR pKey = "\\SOFTWARE\\Qualcomm" );

		// works with values (name=data) to the current key
		BOOL Read( LPCSTR pName, LPSTR pVal, DWORD dwValLen );
		BOOL Write( LPCSTR pName, LPCSTR pData );
};

#endif	//REGIF_H
