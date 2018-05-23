#ifndef _HOST_LIST_H_
#define _HOST_LIST_H_

//Forward declarations
class CPopHost;
class JJFile;

#include "afxmt.h"
#include "pophost.h"

class CHostList : public CList<CPopHost *, CPopHost *> //CSortedList
{
public:
	static CHostList* CreateNewHostList();
	static void WriteLMOSFile();
	static CPopHost* GetHostForCurrentPersona();
	static CPopHost* GetHost(CString strPersona, CString strPOPAccount="");
	
	static CPopHost *GetHostWithMsgHash(long Hash, CMsgRecord *);
    
	void ReplaceHost(CPopHost* pOldPopHost, CPopHost* pNewPopHost);
	~CHostList();
	
protected:
	//
	// FORNOW, very bad style.  This is a non-virtual override
	// of the virtual CSortedList::Compare(CMsgRecord*, CMsgRecord*) 
	// method from the base class.  It is non-virtual since it doesn't
	// match the prototype of the base class method.  Is this really 
	// what we intended?  WKS 97.07.02.
	//
	short Compare(CPopHost *, CPopHost *);

private:
	
	static CHostList* MakeNewHostList();
	
	void InsertPopHost(CPopHost *);
	static CPopHost* MakeNewHost_(JJFile* pLMOSFile, bool *); 
	static CCriticalSection m_HostListGuard;
};



#endif