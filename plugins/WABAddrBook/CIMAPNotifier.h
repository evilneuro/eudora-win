/*
	A base class for implementing notifications on the system address book
*/


#include <WabDefs.h>

interface CIMAPNotifier : public IMAPIAdviseSink {
protected:
	long m_lRef;
public:
	CIMAPNotifier () : IMAPIAdviseSink (), m_lRef(1) {}
	virtual ~CIMAPNotifier () {}

//	IUnknown bits
	virtual HRESULT STDMETHODCALLTYPE QueryInterface ( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject );

	virtual ULONG STDMETHODCALLTYPE AddRef ( void ) { return ++m_lRef; }    

	virtual ULONG STDMETHODCALLTYPE Release( void ) { 
		if ( --m_lRef == 0 ) {
			delete this;
			return 0;
			}
		return m_lRef;
		}

//	IMAPIAdviseSink bits
	virtual unsigned long STDMETHODCALLTYPE OnNotify ( 
			ULONG cNotification, LPNOTIFICATION lpNotifications ) = 0;
};

//	We really should provide an "IMAPIAdviseSink" interface too....
inline HRESULT CIMAPNotifier::QueryInterface ( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject ) {
	if ( IID_IUnknown == riid /*|| IID_IAdviseSink == riid */ ) {
		*ppvObject = (IUnknown *) this;
		return NOERROR;
	}
	return E_NOINTERFACE;
}
