#include "stdafx.h"
#include "hsregex.h"


#include "DebugNewHelpers.h"

CRegex::CRegex(CString strRegex, bool bCaseInsensitive /*= true*/)
	:	m_strRegex(strRegex), m_bCaseInsensitive(bCaseInsensitive), m_bCompiled(false), m_nResult(0)
{
	m_bCompiled = Compile(strRegex, bCaseInsensitive);
}


CRegex::CRegex(const CRegex & in_copy)
	:	m_strRegex(in_copy.m_strRegex), m_bCaseInsensitive(in_copy.m_bCaseInsensitive), m_bCompiled(false), m_nResult(0)
{
	m_bCompiled = Compile(m_strRegex, m_bCaseInsensitive);
}


CRegex::~CRegex()
{
	if (m_bCompiled)
	{
		// Free memory that regcomp allocated inside of m_Regex
		regfree(&m_Regex);
	}
}


bool CRegex::Compile(CString strRegex, bool bCaseInsensitive /*= true*/)
{
	int		nResult = regcomp(&m_Regex, strRegex, REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (bCaseInsensitive?REG_ICASE:0));
	
	if (nResult != 0)
	{
		ASSERT(0);
		return false;
	}
	return true;
}

bool CRegex::Match(const char *szText)
{
	if( szText == NULL)
		return false;

	//ASSERT(m_bCompiled);
	if(!m_bCompiled)
		return false;

	m_nResult = regexec(&m_Regex, szText, 0, NULL, 0);
	if( m_nResult == 0) 
		return true;
	else
		return false;
}


CString CRegex::GetErrorText()
{
	char error[256]={0};
	if(m_nResult != 0)
		regerror(m_nResult, &m_Regex, error, sizeof(error));
	return CString(error);
}
