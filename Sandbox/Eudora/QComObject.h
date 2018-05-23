// QComObject.h : Declaration of the QComObject

#ifndef _QComObject_h_
#define _QComObject_h_

#include <typeinfo.h>

/////////////////////////////////////////////////////////////////////////////
// QComObject

template <
	class ClassName,
	class InterfaceName,
	const IID* InterfaceID,
	const CLSID* ClassID,
	const GUID* LibraryID>
class QComObject : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CComCoClass<ClassName, ClassID>,
	public IDispatchImpl<InterfaceName, InterfaceID, LibraryID>
{
public:
	HRESULT STDMETHODCALLTYPE AutoQueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return ((InterfaceName*)this)->QueryInterface(riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AutoAddRef()
	{
#if (defined(_DEBUG) && defined(_ATL_DEBUG_REFCOUNT))
		const type_info &ti = typeid(this);
		LPCSTR lpszClassName = ti.name();
		ATLTRACE(_T("AutoAddRef %s\n"), lpszClassName);
#endif // (defined(_DEBUG) && defined(_ATL_DEBUG_REFCOUNT))

		return ((InterfaceName*)this)->AddRef();
	}

	ULONG STDMETHODCALLTYPE AutoRelease()
	{
#if (defined(_DEBUG) && defined(_ATL_DEBUG_REFCOUNT))
		const type_info &ti = typeid(this);
		LPCSTR lpszClassName = ti.name();
		ATLTRACE(_T("AutoRelease %s\n"), lpszClassName);
#endif // (defined(_DEBUG) && defined(_ATL_DEBUG_REFCOUNT))

		return ((InterfaceName*)this)->Release();
	}
	void AutoFree(void)
	{
		try
		{
			if (this)
			{
				CoDisconnectObject((InterfaceName*)this, 0L);
				m_dwRef = 1L;
				((InterfaceName*)this)->Release();
			}
		}
		catch (...)
		{
			ASSERT(0);
		}
	}
};

#endif // _QComObject_h_

