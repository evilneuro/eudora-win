// QComMessage.h : Declaration of the QComMessage

#ifndef _QComMessage_h_
#define _QComMessage_h_

#include "EudoraExe.h"	// interface declarations
#include "QComObject.h"
#include "resource.h"   // main symbols

class QCAutomationDirector;
class QCMailboxCommand;
class QComFolder;
class QComMessages;
class CMessageDoc;
class QCMessage;

/////////////////////////////////////////////////////////////////////////////
// QComMessage
class ATL_NO_VTABLE QComMessage :
	public CObject, 
	public QComObject<QComMessage, IEuMessage, &IID_IEuMessage, &CLSID_EuMessage, &LIBID_EudoraLib>
{
public:
	friend class QComMessages;

	// Class factory functions
	static QComMessage* Create(void);

	// ISupportsErrorInfo interface
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// Public class functions
	QComMessage();
	~QComMessage();
	long GetID(void) { return m_ID; }
	long GetIndex(void) { return m_Index; }
private:
	// Private class functions
	void SetID(long ID) { m_ID = ID; }
	void SetIndex(long Index) { m_Index = Index; }
	void SetParents(QComMessages* pParentList);
	bool GetDoc(void);
	void ReleaseDoc(void);

public:
	STDMETHOD(get_BodyAsSimpleText)(/*[out, retval]*/ BSTR *pVal);
	// IEuMessage interface
	STDMETHOD(get_RefCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Index)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ID)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_BodyAsHTML)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Body)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Subject)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_To)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_From)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_AttachmentListAsString)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_HeaderInfo)(enHeaderField HeaderField, BSTR *pVal);
	STDMETHOD(get_Priority)(BSTR *pVal);
	STDMETHOD(get_Date)(BSTR *pVal);
	STDMETHOD(get_RawMessage)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Status)(/*[out, retval]*/ enMessageStatus *pVal);
	STDMETHOD(put_Status)(/*[in]*/ enMessageStatus newVal);
	STDMETHOD(Move)(IEuFolder* NewParentFolder);

	// COM interface map
	DECLARE_REGISTRY_RESOURCEID(IDR_COM_MESSAGE)

BEGIN_COM_MAP(QComMessage)
	COM_INTERFACE_ENTRY(IEuMessage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
protected:
	// Internal data
	long m_ID;
	long m_Index;
	QComMessages* m_pParentList;
	QComFolder* m_pParentFolder;
	CMessageDoc * m_pDoc;
	bool m_bNewDoc;
};

#endif // _QComMessage_h_
