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

				// Sigh. Restored the sledge hammer of setting the ref count to 1
				// so that we're sure that the object will be freed.
				//
				// I don't like this at all. It *appears* this may only be necessary
				// because QComApplication::get_InBox calls pFolder->AutoAddRef even
				// though g_theAutomationDirector.FindFolderByName already incremented
				// the ref count of the in mailbox. However I'm not sure - it's possible
				// that this sledge hammer is needed for other reasons.
				//
				// With the transition to Visual Studio .NET 2003, the sledge hammer
				// approach to making sure the object is freed here was causing
				// AutomationStop to crash when it called AutomationUnregisterFactory,
				// which was attempting to decrement the ref count for the already
				// freed application object. In restoring the sledge hammer approach,
				// I reworked AutomationStop to call AutomationUnregisterFactory before
				// calling AutoFree.
				m_dwRef = 1L;

				AutoRelease();
			}
		}
		catch (CMemoryException * /* pMemoryException */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing CMemoryException in QComObject::AutoFree" );
			throw;
		}
		catch (CException * pException)
		{
			// Other MFC exception
			pException->Delete();
			ASSERT( !"Caught CException (not CMemoryException) in QComObject::AutoFree" );
		}
		catch (std::bad_alloc & /* exception */)
		{
			// Catastrophic memory exception - rethrow
			ASSERT( !"Rethrowing std::bad_alloc in QComObject::AutoFree" );
			throw;
		}
		catch (std::exception & /* exception */)
		{
			ASSERT( !"Caught std::exception (not std::bad_alloc) in QComObject::AutoFree" );
		}
	}
};

#endif // _QComObject_h_

