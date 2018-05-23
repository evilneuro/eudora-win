/*
 * AMBIENTS.H
 * Events IDispatch
 *
 * Copyright (c)1995-1997 Microsoft Corporation, All Rights Reserved
 */


#ifndef _AMBIENTS_H_
#define _AMBIENTS_H_

class CImpIDispatch : public IDispatch
{
    protected:
        ULONG               m_cRef;
        LPUNKNOWN           m_pUnkOuter;
		class CSite*		m_pSite;

    public:
        CImpIDispatch(CSite* pSite, IUnknown * );
        ~CImpIDispatch(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		//IDispatch
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo(/* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo** ppTInfo);
		STDMETHODIMP GetIDsOfNames(
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
		STDMETHODIMP Invoke(
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS  *pDispParams,
            /* [out] */ VARIANT  *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);

};

typedef CImpIDispatch* PCImpIDispatch;

#endif //_AMBIENTS_H_
