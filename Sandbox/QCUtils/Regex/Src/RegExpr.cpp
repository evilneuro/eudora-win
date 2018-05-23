#include "stdafx.h"
#include "hsregex.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CRegex::CRegex(CString strRegex, bool bCaseInsensitive /*= true*/) : m_strRegex(strRegex), m_bCompiled(false), m_nResult(0)
{
	m_strRegex = strRegex;
	m_bCompiled = Compile(strRegex, bCaseInsensitive);
}


bool CRegex::Compile(CString strRegex, bool bCaseInsensitive /*= true*/)
{
	int m_nResult = regcomp(&m_Regex, strRegex, REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (bCaseInsensitive?REG_ICASE:0));
	if(m_nResult != 0)
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
