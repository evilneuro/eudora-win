#ifndef __IMAPOPT_H__
#define __IMAPOPT_H__

// imapopt.h : header file
//


// CImapOption class declaration
// This will maintain a cache of options. For now, just read from
// INI file.
class CImapOption : public CObject
{
public:
	CImapOption();
	CImapOption(const char *pPath);	// Full pathname of options file.
   ~CImapOption();

// Interface
public:
	// Fetch a single value. Note: ALL values are strings!!
	BOOL	ReadString (LPCTSTR section, LPCTSTR OptionName, CString &value);
	BOOL	WriteString (LPCTSTR section, LPCTSTR OptionName, LPCTSTR value);
	BOOL	ClearDatabase ();

// Attributes
private:
	CString			m_Path;		// Full pathname of options file.

};



/////////////////////////////////////////////////////////////////////////////
#endif // __IMAPOPT_H__
