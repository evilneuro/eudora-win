// persona.h
//
// Copyright (c) 1997-2000 by QUALCOMM, Incorporated
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

//

#ifndef PERSONA_H
#define PERSONA_H

#include "PersParams.h"

const int MAX_PERSONAS = 100;
const int MAX_PERSONA_LEN = 64;


class CPersonality
{
public:
	CPersonality();

	void	Init( void );

	BOOL	IsA( const char* Name ) const;
	BOOL	IsDominant( const char* Name ) const;

	DWORD	GetHash( const char* Name );
	CString GetByHash( DWORD Hash );

	CString ImapGetByHash( DWORD HashVal );

	CString GetCurrent( void ) const;
	BOOL	SetCurrent( const char* Name );

	BOOL    GetParams(const char* Name, CPersParams& params);

	BOOL	Modify(const CPersParams& Params);
	BOOL	Add( CPersParams & Params );
	BOOL	Clone(const char* newName, const char* oldName);
	BOOL	Remove( const char* Name );

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

	long	GetPersonaCount( void );

	BOOL	IsImapPersona (LPCSTR pszPersonaName);
	void	FillComboBox(CComboBox* pCB);

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

	BOOL	SavePersonaInfo( const CPersParams & Params, bool bAddPersona );
	BOOL	WriteBoolHelper( LPCTSTR lpSection, int idEntry, BOOL bOn );
	BOOL	WriteStrHelper( LPCTSTR lpSection, int idEntry, LPCSTR str );
	BOOL    WriteIntHelper(LPCTSTR lpSection, int idEntry, int iVal);

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
