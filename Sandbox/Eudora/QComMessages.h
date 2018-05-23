// QComMessages.h : Declaration of the QComMessages

#ifndef _QComMessages_h_
#define _QComMessages_h_

#include "QComObject.h"
#include "resource.h"       // main symbols

class QComMessage;
class QComFolder;
class CTocDoc;
class CSummary;

/////////////////////////////////////////////////////////////////////////////
// QComMessages
class ATL_NO_VTABLE QComMessages :
	public CPtrList, 
	public QComObject<QComMessages, IEuMessages, &IID_IEuMessages, &CLSID_EuMessages, &LIBID_EudoraLib>
{
public:
	// Class factory functions
	static QComMessages* Create(QComFolder* pParent);

	// ISupportsErrorInfo interface
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// IEuMessages interface
	STDMETHOD (get_Count)(long* retval);
	STDMETHOD (get_Item)(long Index, VARIANT* retval);
	STDMETHOD (get_ItemByID)(long ID, VARIANT* retval);
	STDMETHOD (get__NewEnum)(IUnknown** ppEnum);
	STDMETHOD (get_RefCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(RemoveByID)(long ID);
	STDMETHOD(UpdateList)();

	// Public class functions
	QComMessages();
	~QComMessages();
	long GetCount(void);
	QComFolder* GetParent(void) { return m_pParent; }
	CSummary* FindSummaryByID(long ID);

private:
	// Private class functions
	bool UpdateTocPointers(void);
	bool CreateMessageList(void);
	void FreeMessageList(void);
	bool InsertAtEnd(QComMessage* pNewMessage);
	void Remove(POSITION pos);
	QComMessage* FindMessageByIndex(long Index);
	QComMessage* FindMessageByID(long ID);

	// COM interface map
DECLARE_REGISTRY_RESOURCEID(IDR_COM_MESSAGES)

BEGIN_COM_MAP(QComMessages)
	COM_INTERFACE_ENTRY(IEuMessages)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

protected:
	// Class data
	QComFolder* m_pParent;
	CTocDoc* m_pTocDoc;
	bool m_bListReady;
};

#endif // _QComMessages_h_
