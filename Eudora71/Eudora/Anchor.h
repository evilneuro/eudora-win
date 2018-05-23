#ifndef ANCHOR_H
#define ANCHOR_H

#include <afxtempl.h>

class CAnchor : public CObject
{
public:

//	DECLARE_DYNCREATE( CAnchor )

	typedef enum AnchorTypes
	{
		AT_NONE,
		AT_URL,
		AT_RELURL,
		AT_NAME,
		AT_NAMEREF
	} AnchorType;

private:
	LONG		m_lStartOffset;
	LONG		m_lEndOffset;
	AnchorType	m_uType;
	CString		m_szValue;
	LONG		m_lReferencePos;

public:

//	DECLARE_DYNCREATE( CAnchor )

	CAnchor::CAnchor() 
	{
		m_lStartOffset = -1;
		m_lEndOffset = -1;
		m_uType = AT_NONE;
		m_szValue = "";
		m_lReferencePos = -1;
	}

	CAnchor::CAnchor( const CAnchor& src ) 
	{
		*this = src;
	}

	CAnchor&	operator = ( const CAnchor& src )
	{
		m_lStartOffset = src.m_lStartOffset;
		m_lEndOffset = src.m_lEndOffset;
		m_uType = src.m_uType;
		m_szValue = src.m_szValue;
		m_lReferencePos = src.m_lReferencePos;
		return *this;
	}

	AnchorType	GetAnchorType() const { return m_uType; }
	LONG		GetEndPosition() const  { return m_lEndOffset; }
	LONG		GetReferencePos() const  { return m_lReferencePos; }
	LONG		GetStartPosition() const  { return m_lStartOffset; }
	LPCSTR		GetValue() const { return m_szValue; }

	void SetEndPosition( LONG l ) { m_lEndOffset = l; }
	void SetReferencePos( LONG l ) { m_lReferencePos = l; }
	void SetStartPosition( LONG l ) { m_lStartOffset = l; }
	void SetType( AnchorType at ) { m_uType = at; }
	void SetValue( LPCSTR sz ) { m_szValue = sz; }
};

typedef CList<CAnchor, CAnchor&> CAnchorList;
#endif
