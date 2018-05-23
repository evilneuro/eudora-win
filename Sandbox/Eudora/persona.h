#ifndef PERSONA_H
#define PERSONA_H

#include "PersParams.h"

#ifdef COMMERCIAL
const	int MAX_PERSONAS = 100; // Pro, max 100 personas
#else
const	int MAX_PERSONAS = 1; // Light, only one persona
#endif // COMMERCIAL

const	int	MAX_PERSONA_LEN = 64;


class CPersonality
{
public:
	CPersonality();

	void	Init( void );

	BOOL	IsA( const CString & Name ) const;
	BOOL	IsDominant( const CString & Name ) const;

	DWORD	GetHash( const CString & Name );
	CString GetByHash( DWORD Hash );

	CString ImapGetByHash( DWORD HashVal );

	CString GetCurrent( void ) const;
	BOOL	SetCurrent( const CString & Name );

	BOOL    GetParams(const CString& Name, CPersParams& params);

	BOOL	Modify(const CPersParams& Params);
	BOOL	Add( CPersParams & Params );
	BOOL	Clone(const CString& newName, const CString& oldName);
	BOOL	Remove( CString & Name );

	static CString GetIniKeyName(UINT uIniId);
	static CString GetIniDefaultValue(UINT uIniId);

	UINT	GetProfileInt( LPCTSTR	lpName,
						   LPCTSTR	lpKeyName,
						   int		nDefault );
	
	DWORD	GetProfileString( LPCTSTR	lpName, 
		                      LPCTSTR	lpKeyName, 
							  LPCTSTR	lpDefault, 
							  LPTSTR	lpReturnString, 
							  DWORD		nSize );

	BOOL	WriteProfileString( LPCTSTR	lpName,
								LPCTSTR	lpKeyName,
								LPCTSTR	lpString );

	// returns ptr to null delimited list of null delimited strings
	// treat as read-only limited-life (like a getenv() call)
	LPSTR	List( void );
	LPSTR	CheckMailList( void );

	void	SaveAllPasswords( void );
	void	EraseAllPasswords( void );
	void	ForgetPasswords(LPCSTR lpPersonalities);
	bool	AccountsHavePasswords();

	void	Test( void );

	long	GetPersonaCount( void );

	BOOL	IsImapPersona (LPCSTR pszPersonaName);

private:
	// cached string literals used in INI lookups
	char	m_szPersonality[ 64 ];	// "Personality" by default

	BOOL	m_bListStale;
	char	m_ListBuf[ MAX_PERSONAS * MAX_PERSONA_LEN ];
	char	m_CheckMailBuf[ MAX_PERSONAS * MAX_PERSONA_LEN ];

	CString	m_Current;
	CString m_Dominant;				//  = "<Dominant>"

	//CStringList m_Passwords;		// holds alternate personality passwords
	//CString	m_DefaultPassword;		// holds the Dominant personality password
	CMapStringToString m_Passwords;
	
	void	SavePassInfo( void );
	void	RestorePassInfo( void );

	BOOL	AddPersonaInfo( const CPersParams & Params );
	BOOL	WriteBoolHelper( LPCTSTR lpSection, int idEntry, BOOL bOn );
	BOOL	WriteStrHelper( LPCTSTR lpSection, int idEntry, LPCSTR str );
};

// The one-and-only personality object
extern CPersonality g_Personalities;

class CPersonaStateMgr {
public:
	CPersonaStateMgr() { m_strPersona = g_Personalities.GetCurrent(); }
	~CPersonaStateMgr(){ g_Personalities.SetCurrent(m_strPersona); }
private:
	CString m_strPersona;
};



#endif // PERSONA_H
