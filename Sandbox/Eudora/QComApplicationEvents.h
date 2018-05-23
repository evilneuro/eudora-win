#ifndef _QComApplicationEvents_h_
#define _QComApplicationEvents_h_

#include "resource.h"       // main symbols
#include "Eudora.h"

class QComFolder;

//////////////////////////////////////////////////////////////////////////////
// QComApplicationEvents
template <class T>
class QComApplicationEvents : public IConnectionPointImpl<T, &DIID_IEuApplicationEvents, CComDynamicUnkArray>
{
public:
	void FireOnClose()
	{
		// Initialize parameters
		VARIANT varResult;

		// Fire an event in all clients
		FireEvent(0x01, 0, NULL, varResult);
	}
	void FireOnFolderChange()
	{
		// Initialize parameters
		VARIANT varResult;

		// Fire an event in all clients
		FireEvent(0x02, 0, NULL, varResult);
	}
	void FireCheckMailComplete()
	{
		// Initialize parameters
		VARIANT varResult;

		// Fire an event in all clients
		FireEvent(0x03, 0, NULL, varResult);
	}
	void FireSendMailComplete()
	{
		// Initialize parameters
		VARIANT varResult;

		// Fire an event in all clients
		FireEvent(0x04, 0, NULL, varResult);
	}
	void FireEmptyTrashComplete()
	{
		// Initialize parameters
		VARIANT varResult;

		// Fire an event in all clients
		FireEvent(0x05, 0, NULL, varResult);
	}
	void FireCompactFoldersComplete()
	{
		// Initialize parameters
		VARIANT varResult;

		// Fire an event in all clients
		FireEvent(0x06, 0, NULL, varResult);
	}
	void FireEvent(DISPID ID, short NumParams, VARIANTARG* pVars, VARIANT& varResult)
	{
		VariantInit(&varResult);
		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				DISPPARAMS disp = { pVars, NULL, NumParams, 0 };
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				pDispatch->Invoke(ID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
			pp++;
		}
		pT->Unlock();
	}
};

#endif //_QComApplicationEvents_h_
