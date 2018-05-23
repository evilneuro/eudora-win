// Bstr.h: interface for the CBstr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BSTR_H__27BCC8E3_5923_11D1_A7EA_0060972F7459__INCLUDED_)
#define AFX_BSTR_H__27BCC8E3_5923_11D1_A7EA_0060972F7459__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

typedef BSTR*	BSTRARG;

class CBstr  
{
	BSTR	m_bstr;
public:
	CBstr();
	CBstr( UINT uStringID );
	CBstr( LPCSTR szText );

	void Release();

	virtual ~CBstr();
	operator BSTR() { return m_bstr; }
	operator BSTRARG() { Release(); return &m_bstr; }

	BSTR operator = ( BSTR b );
	BSTR operator = ( UINT uID );
	BSTR operator = ( LPCSTR szText );
	BSTR*	BSTRArg();

	BOOL IsNull() { return ( m_bstr == NULL ); }
};

#endif // !defined(AFX_BSTR_H__27BCC8E3_5923_11D1_A7EA_0060972F7459__INCLUDED_)
