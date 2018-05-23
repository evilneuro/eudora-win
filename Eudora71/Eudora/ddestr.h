#ifndef __DDESTR_H__
#define __DDESTR_H__
////////////////////////////////////////////////////////////////////////
//
// CDDEString
// Class wrapper for DDE string objects.  As long as the object is
// alive, it represents a valid DDE string object.  When the object
// is destroyed, it cleans up the internal DDE string object.
//
////////////////////////////////////////////////////////////////////////
                  
#include <afxwin.h>                  
#include <ddeml.h>

class CDDEString
{
public:
	CDDEString(DWORD instId, const char* pStr);
	CDDEString(DWORD instId, HSZ hStr);
	~CDDEString(void);

	operator CString(void) const; 		// cast operator
	int GetLength(void) const;
	HSZ GetHandle(void) const
		{ return m_HSZ; }
	BOOL IsEqual(HSZ hStr) const;

private:
	CDDEString(void);					// not implemented
	CDDEString(const CDDEString&);		// not implemented
	void operator=(const CDDEString&);	// not implemented

	DWORD m_InstId;
	HSZ   m_HSZ;
	BOOL  m_IsHandleOwner;
};

#endif // __DDESTR_H__
