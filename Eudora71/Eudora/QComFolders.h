// QComFolders.h : Declaration of the QComFolders

#ifndef _QComFolders_h_
#define _QComFolders_h_

#include "QComObject.h"
#include "resource.h"       // main symbols

class QComFolder;

/////////////////////////////////////////////////////////////////////////////
// QComFolders
class ATL_NO_VTABLE QComFolders :
	public CPtrList, 
	public QComObject<QComFolders, IEuFolders, &IID_IEuFolders, &CLSID_EuFolders, &LIBID_EudoraLib>
{
public:
	QComFolders();
	~QComFolders();
	// Class factory functions
	static QComFolders* Create(QComFolder* pParent);
	// Internal app only functions
	bool InsertAtEnd(QComFolder* pNewFolder);
	bool InsertAtIndex(QComFolder* pNewFolder, long Index);
	void InternalUpdateFolderIndexes(void);
	void Remove(POSITION pos);
	QComFolder* FindFolderByIndex(long Index);
	QComFolder* FindFolderByID(long ID);

DECLARE_REGISTRY_RESOURCEID(IDR_COM_FOLDERS)

BEGIN_COM_MAP(QComFolders)
	COM_INTERFACE_ENTRY(IEuFolders)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// IFolders
public:
	// standard collection methods
	STDMETHOD (get_Count)(long* retval);
	STDMETHOD (get_Item)(long Index, VARIANT* retval);
	STDMETHOD (get__NewEnum)(IUnknown** ppEnum);
	STDMETHOD (Add)(BSTR Name, BOOL bCanContainMessages);
	STDMETHOD (RemoveByID)(long ID);
public:
	STDMETHOD(get_RefCount)(/*[out, retval]*/ long *pVal);
	QComFolder* m_pParent;
};

#endif // _QComFolders_h_
