#ifndef _CLIENTCERTINFO_H_
#define _CLIENTCERTINFO_H_
#include <afxcoll.h>

class ClientCertInfo
{
private:
	int m_Version;
	CStringList m_Heirarchy;
public:
	CStringList* GetHeirarchy();
	void SetHeirarchy(CStringList& );
	bool Read(const CString& filename);
	bool Write(const CString& filename);
};

#endif
