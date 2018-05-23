/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.01.75 */
/* at Tue Sep 30 18:00:54 1997
 */
/* Compiler settings for EudoraExe.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __BidentModel_h__
#define __BidentModel_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IBidentOmLocation_FWD_DEFINED__
#define __IBidentOmLocation_FWD_DEFINED__
typedef interface IBidentOmLocation IBidentOmLocation;
#endif 	/* __IBidentOmLocation_FWD_DEFINED__ */


#ifndef __IBidentOmLocation_Class_FWD_DEFINED__
#define __IBidentOmLocation_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmLocation_Class IBidentOmLocation_Class;
#else
typedef struct IBidentOmLocation_Class IBidentOmLocation_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmLocation_Class_FWD_DEFINED__ */


#ifndef __IBidentOmLink_FWD_DEFINED__
#define __IBidentOmLink_FWD_DEFINED__
typedef interface IBidentOmLink IBidentOmLink;
#endif 	/* __IBidentOmLink_FWD_DEFINED__ */


#ifndef __IBidentOmLinkEvents_FWD_DEFINED__
#define __IBidentOmLinkEvents_FWD_DEFINED__
typedef interface IBidentOmLinkEvents IBidentOmLinkEvents;
#endif 	/* __IBidentOmLinkEvents_FWD_DEFINED__ */


#ifndef __IBidentOmLink_Class_FWD_DEFINED__
#define __IBidentOmLink_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmLink_Class IBidentOmLink_Class;
#else
typedef struct IBidentOmLink_Class IBidentOmLink_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmLink_Class_FWD_DEFINED__ */


#ifndef __IBidentOmAnchor_FWD_DEFINED__
#define __IBidentOmAnchor_FWD_DEFINED__
typedef interface IBidentOmAnchor IBidentOmAnchor;
#endif 	/* __IBidentOmAnchor_FWD_DEFINED__ */


#ifndef __IBidentOmAnchor_Class_FWD_DEFINED__
#define __IBidentOmAnchor_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmAnchor_Class IBidentOmAnchor_Class;
#else
typedef struct IBidentOmAnchor_Class IBidentOmAnchor_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmAnchor_Class_FWD_DEFINED__ */


#ifndef __IBidentOmLinksCollection_FWD_DEFINED__
#define __IBidentOmLinksCollection_FWD_DEFINED__
typedef interface IBidentOmLinksCollection IBidentOmLinksCollection;
#endif 	/* __IBidentOmLinksCollection_FWD_DEFINED__ */


#ifndef __IBidentOmLinksCollection_Class_FWD_DEFINED__
#define __IBidentOmLinksCollection_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmLinksCollection_Class IBidentOmLinksCollection_Class;
#else
typedef struct IBidentOmLinksCollection_Class IBidentOmLinksCollection_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmLinksCollection_Class_FWD_DEFINED__ */


#ifndef __IBidentOmAnchorsCollection_FWD_DEFINED__
#define __IBidentOmAnchorsCollection_FWD_DEFINED__
typedef interface IBidentOmAnchorsCollection IBidentOmAnchorsCollection;
#endif 	/* __IBidentOmAnchorsCollection_FWD_DEFINED__ */


#ifndef __IBidentOmAnchorsCollection_Class_FWD_DEFINED__
#define __IBidentOmAnchorsCollection_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmAnchorsCollection_Class IBidentOmAnchorsCollection_Class;
#else
typedef struct IBidentOmAnchorsCollection_Class IBidentOmAnchorsCollection_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmAnchorsCollection_Class_FWD_DEFINED__ */


#ifndef __IBidentOmHistory_FWD_DEFINED__
#define __IBidentOmHistory_FWD_DEFINED__
typedef interface IBidentOmHistory IBidentOmHistory;
#endif 	/* __IBidentOmHistory_FWD_DEFINED__ */


#ifndef __IBidentOmHistory_class_FWD_DEFINED__
#define __IBidentOmHistory_class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmHistory_class IBidentOmHistory_class;
#else
typedef struct IBidentOmHistory_class IBidentOmHistory_class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmHistory_class_FWD_DEFINED__ */


#ifndef __IBidentOmNavigator_FWD_DEFINED__
#define __IBidentOmNavigator_FWD_DEFINED__
typedef interface IBidentOmNavigator IBidentOmNavigator;
#endif 	/* __IBidentOmNavigator_FWD_DEFINED__ */


#ifndef __IBidentOmNavigator_class_FWD_DEFINED__
#define __IBidentOmNavigator_class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmNavigator_class IBidentOmNavigator_class;
#else
typedef struct IBidentOmNavigator_class IBidentOmNavigator_class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmNavigator_class_FWD_DEFINED__ */


#ifndef __IBidentOmElementsCollection_FWD_DEFINED__
#define __IBidentOmElementsCollection_FWD_DEFINED__
typedef interface IBidentOmElementsCollection IBidentOmElementsCollection;
#endif 	/* __IBidentOmElementsCollection_FWD_DEFINED__ */


#ifndef __IBidentOmElementsCollection_Class_FWD_DEFINED__
#define __IBidentOmElementsCollection_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmElementsCollection_Class IBidentOmElementsCollection_Class;
#else
typedef struct IBidentOmElementsCollection_Class IBidentOmElementsCollection_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmElementsCollection_Class_FWD_DEFINED__ */


#ifndef __IBidentOmForm_FWD_DEFINED__
#define __IBidentOmForm_FWD_DEFINED__
typedef interface IBidentOmForm IBidentOmForm;
#endif 	/* __IBidentOmForm_FWD_DEFINED__ */


#ifndef __IBidentOmFormEvents_FWD_DEFINED__
#define __IBidentOmFormEvents_FWD_DEFINED__
typedef interface IBidentOmFormEvents IBidentOmFormEvents;
#endif 	/* __IBidentOmFormEvents_FWD_DEFINED__ */


#ifndef __IBidentOmForm_Class_FWD_DEFINED__
#define __IBidentOmForm_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmForm_Class IBidentOmForm_Class;
#else
typedef struct IBidentOmForm_Class IBidentOmForm_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmForm_Class_FWD_DEFINED__ */


#ifndef __IBidentOmFormsCollection_FWD_DEFINED__
#define __IBidentOmFormsCollection_FWD_DEFINED__
typedef interface IBidentOmFormsCollection IBidentOmFormsCollection;
#endif 	/* __IBidentOmFormsCollection_FWD_DEFINED__ */


#ifndef __IBidentOmFormsCollection_Class_FWD_DEFINED__
#define __IBidentOmFormsCollection_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmFormsCollection_Class IBidentOmFormsCollection_Class;
#else
typedef struct IBidentOmFormsCollection_Class IBidentOmFormsCollection_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmFormsCollection_Class_FWD_DEFINED__ */


#ifndef __IOmDocument_FWD_DEFINED__
#define __IOmDocument_FWD_DEFINED__
typedef interface IOmDocument IOmDocument;
#endif 	/* __IOmDocument_FWD_DEFINED__ */


#ifndef __IOmDocument_class_FWD_DEFINED__
#define __IOmDocument_class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IOmDocument_class IOmDocument_class;
#else
typedef struct IOmDocument_class IOmDocument_class;
#endif /* __cplusplus */

#endif 	/* __IOmDocument_class_FWD_DEFINED__ */


#ifndef __IBidentOmFramesCollection_FWD_DEFINED__
#define __IBidentOmFramesCollection_FWD_DEFINED__
typedef interface IBidentOmFramesCollection IBidentOmFramesCollection;
#endif 	/* __IBidentOmFramesCollection_FWD_DEFINED__ */


#ifndef __IBidentOmFramesCollection_class_FWD_DEFINED__
#define __IBidentOmFramesCollection_class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmFramesCollection_class IBidentOmFramesCollection_class;
#else
typedef struct IBidentOmFramesCollection_class IBidentOmFramesCollection_class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmFramesCollection_class_FWD_DEFINED__ */


#ifndef __IOmWindow_FWD_DEFINED__
#define __IOmWindow_FWD_DEFINED__
typedef interface IOmWindow IOmWindow;
#endif 	/* __IOmWindow_FWD_DEFINED__ */


#ifndef __IOmWindowEvents_FWD_DEFINED__
#define __IOmWindowEvents_FWD_DEFINED__
typedef interface IOmWindowEvents IOmWindowEvents;
#endif 	/* __IOmWindowEvents_FWD_DEFINED__ */


#ifndef __IOmWindow_class_FWD_DEFINED__
#define __IOmWindow_class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IOmWindow_class IOmWindow_class;
#else
typedef struct IOmWindow_class IOmWindow_class;
#endif /* __cplusplus */

#endif 	/* __IOmWindow_class_FWD_DEFINED__ */


#ifndef __IBidentOmRadioButtonGroup_FWD_DEFINED__
#define __IBidentOmRadioButtonGroup_FWD_DEFINED__
typedef interface IBidentOmRadioButtonGroup IBidentOmRadioButtonGroup;
#endif 	/* __IBidentOmRadioButtonGroup_FWD_DEFINED__ */


#ifndef __IBidentOmRadioButtonGroup_Class_FWD_DEFINED__
#define __IBidentOmRadioButtonGroup_Class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentOmRadioButtonGroup_Class IBidentOmRadioButtonGroup_Class;
#else
typedef struct IBidentOmRadioButtonGroup_Class IBidentOmRadioButtonGroup_Class;
#endif /* __cplusplus */

#endif 	/* __IBidentOmRadioButtonGroup_Class_FWD_DEFINED__ */


#ifndef __IBidentHTMLDocument_FWD_DEFINED__
#define __IBidentHTMLDocument_FWD_DEFINED__
typedef interface IBidentHTMLDocument IBidentHTMLDocument;
#endif 	/* __IBidentHTMLDocument_FWD_DEFINED__ */


#ifndef __IBidentHTMLDocument_class_FWD_DEFINED__
#define __IBidentHTMLDocument_class_FWD_DEFINED__

#ifdef __cplusplus
typedef class IBidentHTMLDocument_class IBidentHTMLDocument_class;
#else
typedef struct IBidentHTMLDocument_class IBidentHTMLDocument_class;
#endif /* __cplusplus */

#endif 	/* __IBidentHTMLDocument_class_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/****************************************
 * Generated header for interface: __MIDL_itf_EudoraExe_0000
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [local] */ 






















extern RPC_IF_HANDLE __MIDL_itf_EudoraExe_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EudoraExe_0000_v0_0_s_ifspec;


#ifndef __IEScriptObjectModel_LIBRARY_DEFINED__
#define __IEScriptObjectModel_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: IEScriptObjectModel
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_IEScriptObjectModel;

#ifndef __IBidentOmLocation_INTERFACE_DEFINED__
#define __IBidentOmLocation_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmLocation
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmLocation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("163BB1E0-6E00-11CF-837A-48DC04C10000")
    IBidentOmLocation : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT __stdcall get_href( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_href( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_protocol( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_protocol( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_host( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_host( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_hostname( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_hostname( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_port( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_port( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_pathname( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_pathname( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_search( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_search( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_hash( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_hash( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [id] */ HRESULT __stdcall reload( void) = 0;
        
        virtual /* [id] */ HRESULT __stdcall replace( 
            /* [in] */ BSTR bstr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmLocationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmLocation __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmLocation __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_href )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_href )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_protocol )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_protocol )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_host )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_host )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_hostname )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_hostname )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_port )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_port )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_pathname )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_pathname )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_search )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_search )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_hash )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_hash )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *reload )( 
            IBidentOmLocation __RPC_FAR * This);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *replace )( 
            IBidentOmLocation __RPC_FAR * This,
            /* [in] */ BSTR bstr);
        
        END_INTERFACE
    } IBidentOmLocationVtbl;

    interface IBidentOmLocation
    {
        CONST_VTBL struct IBidentOmLocationVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmLocation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmLocation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmLocation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmLocation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmLocation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmLocation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmLocation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmLocation_get_href(This,retval)	\
    (This)->lpVtbl -> get_href(This,retval)

#define IBidentOmLocation_put_href(This,retval)	\
    (This)->lpVtbl -> put_href(This,retval)

#define IBidentOmLocation_get_protocol(This,retval)	\
    (This)->lpVtbl -> get_protocol(This,retval)

#define IBidentOmLocation_put_protocol(This,retval)	\
    (This)->lpVtbl -> put_protocol(This,retval)

#define IBidentOmLocation_get_host(This,retval)	\
    (This)->lpVtbl -> get_host(This,retval)

#define IBidentOmLocation_put_host(This,retval)	\
    (This)->lpVtbl -> put_host(This,retval)

#define IBidentOmLocation_get_hostname(This,retval)	\
    (This)->lpVtbl -> get_hostname(This,retval)

#define IBidentOmLocation_put_hostname(This,retval)	\
    (This)->lpVtbl -> put_hostname(This,retval)

#define IBidentOmLocation_get_port(This,retval)	\
    (This)->lpVtbl -> get_port(This,retval)

#define IBidentOmLocation_put_port(This,retval)	\
    (This)->lpVtbl -> put_port(This,retval)

#define IBidentOmLocation_get_pathname(This,retval)	\
    (This)->lpVtbl -> get_pathname(This,retval)

#define IBidentOmLocation_put_pathname(This,retval)	\
    (This)->lpVtbl -> put_pathname(This,retval)

#define IBidentOmLocation_get_search(This,retval)	\
    (This)->lpVtbl -> get_search(This,retval)

#define IBidentOmLocation_put_search(This,retval)	\
    (This)->lpVtbl -> put_search(This,retval)

#define IBidentOmLocation_get_hash(This,retval)	\
    (This)->lpVtbl -> get_hash(This,retval)

#define IBidentOmLocation_put_hash(This,retval)	\
    (This)->lpVtbl -> put_hash(This,retval)

#define IBidentOmLocation_reload(This)	\
    (This)->lpVtbl -> reload(This)

#define IBidentOmLocation_replace(This,bstr)	\
    (This)->lpVtbl -> replace(This,bstr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_href_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_href_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_href_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_href_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_protocol_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_protocol_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_host_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_host_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_host_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_host_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_hostname_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_hostname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_hostname_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_hostname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_port_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_port_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_pathname_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_pathname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_pathname_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_pathname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_search_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_search_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_search_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_search_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLocation_get_hash_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLocation_get_hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmLocation_put_hash_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmLocation_put_hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmLocation_reload_Proxy( 
    IBidentOmLocation __RPC_FAR * This);


void __RPC_STUB IBidentOmLocation_reload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmLocation_replace_Proxy( 
    IBidentOmLocation __RPC_FAR * This,
    /* [in] */ BSTR bstr);


void __RPC_STUB IBidentOmLocation_replace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmLocation_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmLocation_Class;

class DECLSPEC_UUID("163BB1E1-6E00-11CF-837A-48DC04C10000")
IBidentOmLocation_Class;
#endif

#ifndef __IBidentOmLink_INTERFACE_DEFINED__
#define __IBidentOmLink_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmLink
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmLink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("163BB1E2-6E00-11CF-837A-48DC04C10000")
    IBidentOmLink : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT __stdcall get_href( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_protocol( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_host( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_hostname( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_port( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_pathname( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_search( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_hash( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_target( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmLinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmLink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmLink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmLink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmLink __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmLink __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmLink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmLink __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_href )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_protocol )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_host )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_hostname )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_port )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_pathname )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_search )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_hash )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_target )( 
            IBidentOmLink __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        END_INTERFACE
    } IBidentOmLinkVtbl;

    interface IBidentOmLink
    {
        CONST_VTBL struct IBidentOmLinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmLink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmLink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmLink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmLink_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmLink_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmLink_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmLink_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmLink_get_href(This,retval)	\
    (This)->lpVtbl -> get_href(This,retval)

#define IBidentOmLink_get_protocol(This,retval)	\
    (This)->lpVtbl -> get_protocol(This,retval)

#define IBidentOmLink_get_host(This,retval)	\
    (This)->lpVtbl -> get_host(This,retval)

#define IBidentOmLink_get_hostname(This,retval)	\
    (This)->lpVtbl -> get_hostname(This,retval)

#define IBidentOmLink_get_port(This,retval)	\
    (This)->lpVtbl -> get_port(This,retval)

#define IBidentOmLink_get_pathname(This,retval)	\
    (This)->lpVtbl -> get_pathname(This,retval)

#define IBidentOmLink_get_search(This,retval)	\
    (This)->lpVtbl -> get_search(This,retval)

#define IBidentOmLink_get_hash(This,retval)	\
    (This)->lpVtbl -> get_hash(This,retval)

#define IBidentOmLink_get_target(This,retval)	\
    (This)->lpVtbl -> get_target(This,retval)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_href_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_href_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_protocol_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_protocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_host_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_host_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_hostname_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_hostname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_port_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_pathname_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_pathname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_search_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_search_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_hash_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLink_get_target_Proxy( 
    IBidentOmLink __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmLink_get_target_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmLink_INTERFACE_DEFINED__ */


#ifndef __IBidentOmLinkEvents_DISPINTERFACE_DEFINED__
#define __IBidentOmLinkEvents_DISPINTERFACE_DEFINED__

/****************************************
 * Generated header for dispinterface: IBidentOmLinkEvents
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][uuid] */ 



EXTERN_C const IID DIID_IBidentOmLinkEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface DECLSPEC_UUID("163BB1E3-6E00-11CF-837A-48DC04C10000")
    IBidentOmLinkEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmLinkEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmLinkEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmLinkEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmLinkEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmLinkEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmLinkEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmLinkEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmLinkEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IBidentOmLinkEventsVtbl;

    interface IBidentOmLinkEvents
    {
        CONST_VTBL struct IBidentOmLinkEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmLinkEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmLinkEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmLinkEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmLinkEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmLinkEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmLinkEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmLinkEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IBidentOmLinkEvents_DISPINTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmLink_Class;

class DECLSPEC_UUID("B3E43805-6DFF-11CF-837A-48DC04C10000")
IBidentOmLink_Class;
#endif

#ifndef __IBidentOmAnchor_INTERFACE_DEFINED__
#define __IBidentOmAnchor_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmAnchor
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmAnchor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("163BB1E4-6E00-11CF-837A-48DC04C10000")
    IBidentOmAnchor : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT __stdcall get_name( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_name( 
            /* [in] */ BSTR retval) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmAnchorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmAnchor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmAnchor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_name )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_name )( 
            IBidentOmAnchor __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        END_INTERFACE
    } IBidentOmAnchorVtbl;

    interface IBidentOmAnchor
    {
        CONST_VTBL struct IBidentOmAnchorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmAnchor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmAnchor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmAnchor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmAnchor_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmAnchor_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmAnchor_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmAnchor_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmAnchor_get_name(This,retval)	\
    (This)->lpVtbl -> get_name(This,retval)

#define IBidentOmAnchor_put_name(This,retval)	\
    (This)->lpVtbl -> put_name(This,retval)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT __stdcall IBidentOmAnchor_get_name_Proxy( 
    IBidentOmAnchor __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmAnchor_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmAnchor_put_name_Proxy( 
    IBidentOmAnchor __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmAnchor_put_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmAnchor_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmAnchor_Class;

class DECLSPEC_UUID("163BB1E5-6E00-11CF-837A-48DC04C10000")
IBidentOmAnchor_Class;
#endif

#ifndef __IBidentOmLinksCollection_INTERFACE_DEFINED__
#define __IBidentOmLinksCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmLinksCollection
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmLinksCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("163BB1E8-6E00-11CF-837A-48DC04C10000")
    IBidentOmLinksCollection : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall Item( 
            /* [in] */ long i,
            /* [retval][out] */ IBidentOmLink __RPC_FAR *__RPC_FAR *pplinkResult) = 0;
        
        virtual /* [id] */ HRESULT __stdcall Count( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall Add( 
            /* [in] */ IBidentOmLink __RPC_FAR *plink) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmLinksCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmLinksCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmLinksCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Item )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [in] */ long i,
            /* [retval][out] */ IBidentOmLink __RPC_FAR *__RPC_FAR *pplinkResult);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Count )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *Add )( 
            IBidentOmLinksCollection __RPC_FAR * This,
            /* [in] */ IBidentOmLink __RPC_FAR *plink);
        
        END_INTERFACE
    } IBidentOmLinksCollectionVtbl;

    interface IBidentOmLinksCollection
    {
        CONST_VTBL struct IBidentOmLinksCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmLinksCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmLinksCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmLinksCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmLinksCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmLinksCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmLinksCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmLinksCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmLinksCollection_Item(This,i,pplinkResult)	\
    (This)->lpVtbl -> Item(This,i,pplinkResult)

#define IBidentOmLinksCollection_Count(This,pl)	\
    (This)->lpVtbl -> Count(This,pl)

#define IBidentOmLinksCollection_get_length(This,pl)	\
    (This)->lpVtbl -> get_length(This,pl)

#define IBidentOmLinksCollection_Add(This,plink)	\
    (This)->lpVtbl -> Add(This,plink)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmLinksCollection_Item_Proxy( 
    IBidentOmLinksCollection __RPC_FAR * This,
    /* [in] */ long i,
    /* [retval][out] */ IBidentOmLink __RPC_FAR *__RPC_FAR *pplinkResult);


void __RPC_STUB IBidentOmLinksCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmLinksCollection_Count_Proxy( 
    IBidentOmLinksCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmLinksCollection_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmLinksCollection_get_length_Proxy( 
    IBidentOmLinksCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmLinksCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IBidentOmLinksCollection_Add_Proxy( 
    IBidentOmLinksCollection __RPC_FAR * This,
    /* [in] */ IBidentOmLink __RPC_FAR *plink);


void __RPC_STUB IBidentOmLinksCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmLinksCollection_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmLinksCollection_Class;

class DECLSPEC_UUID("163BB1E9-6E00-11CF-837A-48DC04C10000")
IBidentOmLinksCollection_Class;
#endif

#ifndef __IBidentOmAnchorsCollection_INTERFACE_DEFINED__
#define __IBidentOmAnchorsCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmAnchorsCollection
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmAnchorsCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("163BB1EA-6E00-11CF-837A-48DC04C10000")
    IBidentOmAnchorsCollection : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall Item( 
            /* [in] */ long i,
            /* [retval][out] */ IBidentOmAnchor __RPC_FAR *__RPC_FAR *ppanchorResult) = 0;
        
        virtual /* [id] */ HRESULT __stdcall Count( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall Add( 
            /* [in] */ IBidentOmAnchor __RPC_FAR *panchor) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmAnchorsCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmAnchorsCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmAnchorsCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Item )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [in] */ long i,
            /* [retval][out] */ IBidentOmAnchor __RPC_FAR *__RPC_FAR *ppanchorResult);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Count )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *Add )( 
            IBidentOmAnchorsCollection __RPC_FAR * This,
            /* [in] */ IBidentOmAnchor __RPC_FAR *panchor);
        
        END_INTERFACE
    } IBidentOmAnchorsCollectionVtbl;

    interface IBidentOmAnchorsCollection
    {
        CONST_VTBL struct IBidentOmAnchorsCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmAnchorsCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmAnchorsCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmAnchorsCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmAnchorsCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmAnchorsCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmAnchorsCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmAnchorsCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmAnchorsCollection_Item(This,i,ppanchorResult)	\
    (This)->lpVtbl -> Item(This,i,ppanchorResult)

#define IBidentOmAnchorsCollection_Count(This,pl)	\
    (This)->lpVtbl -> Count(This,pl)

#define IBidentOmAnchorsCollection_get_length(This,pl)	\
    (This)->lpVtbl -> get_length(This,pl)

#define IBidentOmAnchorsCollection_Add(This,panchor)	\
    (This)->lpVtbl -> Add(This,panchor)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmAnchorsCollection_Item_Proxy( 
    IBidentOmAnchorsCollection __RPC_FAR * This,
    /* [in] */ long i,
    /* [retval][out] */ IBidentOmAnchor __RPC_FAR *__RPC_FAR *ppanchorResult);


void __RPC_STUB IBidentOmAnchorsCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmAnchorsCollection_Count_Proxy( 
    IBidentOmAnchorsCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmAnchorsCollection_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmAnchorsCollection_get_length_Proxy( 
    IBidentOmAnchorsCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmAnchorsCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IBidentOmAnchorsCollection_Add_Proxy( 
    IBidentOmAnchorsCollection __RPC_FAR * This,
    /* [in] */ IBidentOmAnchor __RPC_FAR *panchor);


void __RPC_STUB IBidentOmAnchorsCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmAnchorsCollection_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmAnchorsCollection_Class;

class DECLSPEC_UUID("163BB1EC-6E00-11CF-837A-48DC04C10000")
IBidentOmAnchorsCollection_Class;
#endif

#ifndef __IBidentOmHistory_INTERFACE_DEFINED__
#define __IBidentOmHistory_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmHistory
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmHistory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("FECEAAA2-8405-11CF-8BA1-00AA00476DA6")
    IBidentOmHistory : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ short __RPC_FAR *retval) = 0;
        
        virtual /* [id] */ HRESULT __stdcall back( 
            /* [in] */ VARIANT __RPC_FAR *pvargdistance) = 0;
        
        virtual /* [id] */ HRESULT __stdcall forward( 
            /* [in] */ VARIANT __RPC_FAR *pvargdistance) = 0;
        
        virtual /* [id] */ HRESULT __stdcall go( 
            /* [in] */ VARIANT __RPC_FAR *pvargdistance) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmHistoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmHistory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmHistory __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [retval][out] */ short __RPC_FAR *retval);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *back )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvargdistance);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *forward )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvargdistance);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *go )( 
            IBidentOmHistory __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvargdistance);
        
        END_INTERFACE
    } IBidentOmHistoryVtbl;

    interface IBidentOmHistory
    {
        CONST_VTBL struct IBidentOmHistoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmHistory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmHistory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmHistory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmHistory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmHistory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmHistory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmHistory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmHistory_get_length(This,retval)	\
    (This)->lpVtbl -> get_length(This,retval)

#define IBidentOmHistory_back(This,pvargdistance)	\
    (This)->lpVtbl -> back(This,pvargdistance)

#define IBidentOmHistory_forward(This,pvargdistance)	\
    (This)->lpVtbl -> forward(This,pvargdistance)

#define IBidentOmHistory_go(This,pvargdistance)	\
    (This)->lpVtbl -> go(This,pvargdistance)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT __stdcall IBidentOmHistory_get_length_Proxy( 
    IBidentOmHistory __RPC_FAR * This,
    /* [retval][out] */ short __RPC_FAR *retval);


void __RPC_STUB IBidentOmHistory_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmHistory_back_Proxy( 
    IBidentOmHistory __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvargdistance);


void __RPC_STUB IBidentOmHistory_back_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmHistory_forward_Proxy( 
    IBidentOmHistory __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvargdistance);


void __RPC_STUB IBidentOmHistory_forward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmHistory_go_Proxy( 
    IBidentOmHistory __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvargdistance);


void __RPC_STUB IBidentOmHistory_go_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmHistory_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmHistory_class;

class DECLSPEC_UUID("FECEAAA3-8405-11CF-8BA1-00AA00476DA6")
IBidentOmHistory_class;
#endif

#ifndef __IBidentOmNavigator_INTERFACE_DEFINED__
#define __IBidentOmNavigator_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmNavigator
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmNavigator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("FECEAAA5-8405-11CF-8BA1-00AA00476DA6")
    IBidentOmNavigator : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT __stdcall get_appCodeName( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_appName( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_appVersion( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_userAgent( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmNavigatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmNavigator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmNavigator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_appCodeName )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_appName )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_appVersion )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_userAgent )( 
            IBidentOmNavigator __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        END_INTERFACE
    } IBidentOmNavigatorVtbl;

    interface IBidentOmNavigator
    {
        CONST_VTBL struct IBidentOmNavigatorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmNavigator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmNavigator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmNavigator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmNavigator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmNavigator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmNavigator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmNavigator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmNavigator_get_appCodeName(This,retval)	\
    (This)->lpVtbl -> get_appCodeName(This,retval)

#define IBidentOmNavigator_get_appName(This,retval)	\
    (This)->lpVtbl -> get_appName(This,retval)

#define IBidentOmNavigator_get_appVersion(This,retval)	\
    (This)->lpVtbl -> get_appVersion(This,retval)

#define IBidentOmNavigator_get_userAgent(This,retval)	\
    (This)->lpVtbl -> get_userAgent(This,retval)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT __stdcall IBidentOmNavigator_get_appCodeName_Proxy( 
    IBidentOmNavigator __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmNavigator_get_appCodeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmNavigator_get_appName_Proxy( 
    IBidentOmNavigator __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmNavigator_get_appName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmNavigator_get_appVersion_Proxy( 
    IBidentOmNavigator __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmNavigator_get_appVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmNavigator_get_userAgent_Proxy( 
    IBidentOmNavigator __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmNavigator_get_userAgent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmNavigator_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmNavigator_class;

class DECLSPEC_UUID("FECEAAA6-8405-11CF-8BA1-00AA00476DA6")
IBidentOmNavigator_class;
#endif

#ifndef __IBidentOmElementsCollection_INTERFACE_DEFINED__
#define __IBidentOmElementsCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmElementsCollection
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmElementsCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0D04D289-6BEC-11CF-8B97-00AA00476DA6")
    IBidentOmElementsCollection : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall Item( 
            /* [in] */ VARIANT __RPC_FAR *pvarIndex,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppeleresult) = 0;
        
        virtual /* [id] */ HRESULT __stdcall Count( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmElementsCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmElementsCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmElementsCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Item )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *pvarIndex,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppeleresult);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Count )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmElementsCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        END_INTERFACE
    } IBidentOmElementsCollectionVtbl;

    interface IBidentOmElementsCollection
    {
        CONST_VTBL struct IBidentOmElementsCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmElementsCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmElementsCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmElementsCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmElementsCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmElementsCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmElementsCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmElementsCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmElementsCollection_Item(This,pvarIndex,ppeleresult)	\
    (This)->lpVtbl -> Item(This,pvarIndex,ppeleresult)

#define IBidentOmElementsCollection_Count(This,pl)	\
    (This)->lpVtbl -> Count(This,pl)

#define IBidentOmElementsCollection_get_length(This,pl)	\
    (This)->lpVtbl -> get_length(This,pl)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmElementsCollection_Item_Proxy( 
    IBidentOmElementsCollection __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *pvarIndex,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppeleresult);


void __RPC_STUB IBidentOmElementsCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmElementsCollection_Count_Proxy( 
    IBidentOmElementsCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmElementsCollection_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmElementsCollection_get_length_Proxy( 
    IBidentOmElementsCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmElementsCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmElementsCollection_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmElementsCollection_Class;

class DECLSPEC_UUID("0D04D28A-6BEC-11CF-8B97-00AA00476DA6")
IBidentOmElementsCollection_Class;
#endif

#ifndef __IBidentOmForm_INTERFACE_DEFINED__
#define __IBidentOmForm_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmForm
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmForm;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0D04D283-6BEC-11CF-8B97-00AA00476DA6")
    IBidentOmForm : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall elementItem( 
            /* [in] */ long i,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppelement) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_action( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_action( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_encoding( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_encoding( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_method( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_method( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_target( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_target( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_elements( 
            /* [retval][out] */ IBidentOmElementsCollection __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [id] */ HRESULT __stdcall submit( void) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall IsItemInForm( 
            /* [in] */ long hWndElement) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall SubmitForm( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmFormVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmForm __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmForm __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmForm __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *elementItem )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ long i,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppelement);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_action )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_action )( 
            IBidentOmForm __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_encoding )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_encoding )( 
            IBidentOmForm __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_method )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_method )( 
            IBidentOmForm __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_target )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_target )( 
            IBidentOmForm __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_elements )( 
            IBidentOmForm __RPC_FAR * This,
            /* [retval][out] */ IBidentOmElementsCollection __RPC_FAR *__RPC_FAR *retval);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *submit )( 
            IBidentOmForm __RPC_FAR * This);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *IsItemInForm )( 
            IBidentOmForm __RPC_FAR * This,
            /* [in] */ long hWndElement);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *SubmitForm )( 
            IBidentOmForm __RPC_FAR * This);
        
        END_INTERFACE
    } IBidentOmFormVtbl;

    interface IBidentOmForm
    {
        CONST_VTBL struct IBidentOmFormVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmForm_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmForm_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmForm_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmForm_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmForm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmForm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmForm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmForm_elementItem(This,i,ppelement)	\
    (This)->lpVtbl -> elementItem(This,i,ppelement)

#define IBidentOmForm_put_action(This,retval)	\
    (This)->lpVtbl -> put_action(This,retval)

#define IBidentOmForm_get_action(This,retval)	\
    (This)->lpVtbl -> get_action(This,retval)

#define IBidentOmForm_put_encoding(This,retval)	\
    (This)->lpVtbl -> put_encoding(This,retval)

#define IBidentOmForm_get_encoding(This,retval)	\
    (This)->lpVtbl -> get_encoding(This,retval)

#define IBidentOmForm_put_method(This,retval)	\
    (This)->lpVtbl -> put_method(This,retval)

#define IBidentOmForm_get_method(This,retval)	\
    (This)->lpVtbl -> get_method(This,retval)

#define IBidentOmForm_put_target(This,retval)	\
    (This)->lpVtbl -> put_target(This,retval)

#define IBidentOmForm_get_target(This,retval)	\
    (This)->lpVtbl -> get_target(This,retval)

#define IBidentOmForm_get_elements(This,retval)	\
    (This)->lpVtbl -> get_elements(This,retval)

#define IBidentOmForm_submit(This)	\
    (This)->lpVtbl -> submit(This)

#define IBidentOmForm_IsItemInForm(This,hWndElement)	\
    (This)->lpVtbl -> IsItemInForm(This,hWndElement)

#define IBidentOmForm_SubmitForm(This)	\
    (This)->lpVtbl -> SubmitForm(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmForm_elementItem_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [in] */ long i,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppelement);


void __RPC_STUB IBidentOmForm_elementItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmForm_put_action_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmForm_put_action_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmForm_get_action_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmForm_get_action_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmForm_put_encoding_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmForm_put_encoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmForm_get_encoding_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmForm_get_encoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmForm_put_method_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmForm_put_method_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmForm_get_method_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmForm_get_method_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IBidentOmForm_put_target_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IBidentOmForm_put_target_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmForm_get_target_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IBidentOmForm_get_target_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmForm_get_elements_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [retval][out] */ IBidentOmElementsCollection __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IBidentOmForm_get_elements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmForm_submit_Proxy( 
    IBidentOmForm __RPC_FAR * This);


void __RPC_STUB IBidentOmForm_submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IBidentOmForm_IsItemInForm_Proxy( 
    IBidentOmForm __RPC_FAR * This,
    /* [in] */ long hWndElement);


void __RPC_STUB IBidentOmForm_IsItemInForm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IBidentOmForm_SubmitForm_Proxy( 
    IBidentOmForm __RPC_FAR * This);


void __RPC_STUB IBidentOmForm_SubmitForm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmForm_INTERFACE_DEFINED__ */


#ifndef __IBidentOmFormEvents_DISPINTERFACE_DEFINED__
#define __IBidentOmFormEvents_DISPINTERFACE_DEFINED__

/****************************************
 * Generated header for dispinterface: IBidentOmFormEvents
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][uuid] */ 



EXTERN_C const IID DIID_IBidentOmFormEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface DECLSPEC_UUID("0D04D284-6BEC-11CF-8B97-00AA00476DA6")
    IBidentOmFormEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmFormEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmFormEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmFormEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmFormEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmFormEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmFormEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmFormEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmFormEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IBidentOmFormEventsVtbl;

    interface IBidentOmFormEvents
    {
        CONST_VTBL struct IBidentOmFormEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmFormEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmFormEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmFormEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmFormEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmFormEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmFormEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmFormEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IBidentOmFormEvents_DISPINTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmForm_Class;

class DECLSPEC_UUID("0D04D285-6BEC-11CF-8B97-00AA00476DA6")
IBidentOmForm_Class;
#endif

#ifndef __IBidentOmFormsCollection_INTERFACE_DEFINED__
#define __IBidentOmFormsCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmFormsCollection
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmFormsCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0D04D286-6BEC-11CF-8B97-00AA00476DA6")
    IBidentOmFormsCollection : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall Item( 
            /* [in] */ long i,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppformresult) = 0;
        
        virtual /* [id] */ HRESULT __stdcall Count( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmFormsCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmFormsCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmFormsCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Item )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [in] */ long i,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppformresult);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Count )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmFormsCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        END_INTERFACE
    } IBidentOmFormsCollectionVtbl;

    interface IBidentOmFormsCollection
    {
        CONST_VTBL struct IBidentOmFormsCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmFormsCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmFormsCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmFormsCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmFormsCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmFormsCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmFormsCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmFormsCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmFormsCollection_Item(This,i,ppformresult)	\
    (This)->lpVtbl -> Item(This,i,ppformresult)

#define IBidentOmFormsCollection_Count(This,pl)	\
    (This)->lpVtbl -> Count(This,pl)

#define IBidentOmFormsCollection_get_length(This,pl)	\
    (This)->lpVtbl -> get_length(This,pl)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmFormsCollection_Item_Proxy( 
    IBidentOmFormsCollection __RPC_FAR * This,
    /* [in] */ long i,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppformresult);


void __RPC_STUB IBidentOmFormsCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmFormsCollection_Count_Proxy( 
    IBidentOmFormsCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmFormsCollection_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmFormsCollection_get_length_Proxy( 
    IBidentOmFormsCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmFormsCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmFormsCollection_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmFormsCollection_Class;

class DECLSPEC_UUID("0D04D287-6BEC-11CF-8B97-00AA00476DA6")
IBidentOmFormsCollection_Class;
#endif

#ifndef __IOmDocument_INTERFACE_DEFINED__
#define __IOmDocument_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOmDocument
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IOmDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("D48A6EC8-6A4A-11CF-94A7-444553540000")
    IOmDocument : public IDispatch
    {
    public:
        virtual /* [propput][id] */ HRESULT __stdcall put_linkColor( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_alinkColor( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_vlinkColor( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_bgColor( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_fgColor( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_linkColor( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_alinkColor( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_vlinkColor( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_bgColor( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_fgColor( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_anchors( 
            /* [retval][out] */ IBidentOmAnchorsCollection __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_links( 
            /* [retval][out] */ IBidentOmLinksCollection __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_forms( 
            /* [retval][out] */ IBidentOmFormsCollection __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_location( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_frames( 
            /* [retval][out] */ IBidentOmFramesCollection __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_lastModified( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_title( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_cookie( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_cookie( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_referrer( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [id] */ HRESULT __stdcall open( 
            /* [in] */ VARIANT __RPC_FAR *mimetype,
            /* [retval][out] */ VARIANT __RPC_FAR *retval) = 0;
        
        virtual /* [id] */ HRESULT __stdcall close( void) = 0;
        
        virtual /* [id] */ HRESULT __stdcall clear( void) = 0;
        
        virtual /* [vararg][id] */ HRESULT __stdcall write( 
            /* [in] */ SAFEARRAY __RPC_FAR * psarray) = 0;
        
        virtual /* [vararg][id] */ HRESULT __stdcall writeln( 
            /* [in] */ SAFEARRAY __RPC_FAR * psarray) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOmDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOmDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOmDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IOmDocument __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_linkColor )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_alinkColor )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_vlinkColor )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_bgColor )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_fgColor )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_linkColor )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_alinkColor )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_vlinkColor )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_bgColor )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_fgColor )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_anchors )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ IBidentOmAnchorsCollection __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_links )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ IBidentOmLinksCollection __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_forms )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ IBidentOmFormsCollection __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_location )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_frames )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ IBidentOmFramesCollection __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_lastModified )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_title )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_cookie )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_cookie )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_referrer )( 
            IOmDocument __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *open )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *mimetype,
            /* [retval][out] */ VARIANT __RPC_FAR *retval);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *close )( 
            IOmDocument __RPC_FAR * This);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *clear )( 
            IOmDocument __RPC_FAR * This);
        
        /* [vararg][id] */ HRESULT ( __stdcall __RPC_FAR *write )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * psarray);
        
        /* [vararg][id] */ HRESULT ( __stdcall __RPC_FAR *writeln )( 
            IOmDocument __RPC_FAR * This,
            /* [in] */ SAFEARRAY __RPC_FAR * psarray);
        
        END_INTERFACE
    } IOmDocumentVtbl;

    interface IOmDocument
    {
        CONST_VTBL struct IOmDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOmDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOmDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOmDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOmDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IOmDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IOmDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IOmDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IOmDocument_put_linkColor(This,retval)	\
    (This)->lpVtbl -> put_linkColor(This,retval)

#define IOmDocument_put_alinkColor(This,retval)	\
    (This)->lpVtbl -> put_alinkColor(This,retval)

#define IOmDocument_put_vlinkColor(This,retval)	\
    (This)->lpVtbl -> put_vlinkColor(This,retval)

#define IOmDocument_put_bgColor(This,retval)	\
    (This)->lpVtbl -> put_bgColor(This,retval)

#define IOmDocument_put_fgColor(This,retval)	\
    (This)->lpVtbl -> put_fgColor(This,retval)

#define IOmDocument_get_linkColor(This,retval)	\
    (This)->lpVtbl -> get_linkColor(This,retval)

#define IOmDocument_get_alinkColor(This,retval)	\
    (This)->lpVtbl -> get_alinkColor(This,retval)

#define IOmDocument_get_vlinkColor(This,retval)	\
    (This)->lpVtbl -> get_vlinkColor(This,retval)

#define IOmDocument_get_bgColor(This,retval)	\
    (This)->lpVtbl -> get_bgColor(This,retval)

#define IOmDocument_get_fgColor(This,retval)	\
    (This)->lpVtbl -> get_fgColor(This,retval)

#define IOmDocument_get_anchors(This,retval)	\
    (This)->lpVtbl -> get_anchors(This,retval)

#define IOmDocument_get_links(This,retval)	\
    (This)->lpVtbl -> get_links(This,retval)

#define IOmDocument_get_forms(This,retval)	\
    (This)->lpVtbl -> get_forms(This,retval)

#define IOmDocument_get_location(This,retval)	\
    (This)->lpVtbl -> get_location(This,retval)

#define IOmDocument_get_frames(This,retval)	\
    (This)->lpVtbl -> get_frames(This,retval)

#define IOmDocument_get_lastModified(This,retval)	\
    (This)->lpVtbl -> get_lastModified(This,retval)

#define IOmDocument_get_title(This,retval)	\
    (This)->lpVtbl -> get_title(This,retval)

#define IOmDocument_get_cookie(This,retval)	\
    (This)->lpVtbl -> get_cookie(This,retval)

#define IOmDocument_put_cookie(This,retval)	\
    (This)->lpVtbl -> put_cookie(This,retval)

#define IOmDocument_get_referrer(This,retval)	\
    (This)->lpVtbl -> get_referrer(This,retval)

#define IOmDocument_open(This,mimetype,retval)	\
    (This)->lpVtbl -> open(This,mimetype,retval)

#define IOmDocument_close(This)	\
    (This)->lpVtbl -> close(This)

#define IOmDocument_clear(This)	\
    (This)->lpVtbl -> clear(This)

#define IOmDocument_write(This,psarray)	\
    (This)->lpVtbl -> write(This,psarray)

#define IOmDocument_writeln(This,psarray)	\
    (This)->lpVtbl -> writeln(This,psarray)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propput][id] */ HRESULT __stdcall IOmDocument_put_linkColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmDocument_put_linkColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmDocument_put_alinkColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmDocument_put_alinkColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmDocument_put_vlinkColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmDocument_put_vlinkColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmDocument_put_bgColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmDocument_put_bgColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmDocument_put_fgColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmDocument_put_fgColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_linkColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_linkColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_alinkColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_alinkColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_vlinkColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_vlinkColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_bgColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_bgColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_fgColor_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_fgColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_anchors_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ IBidentOmAnchorsCollection __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_anchors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_links_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ IBidentOmLinksCollection __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_links_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_forms_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ IBidentOmFormsCollection __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_forms_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_location_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_location_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_frames_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ IBidentOmFramesCollection __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_frames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_lastModified_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_lastModified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_title_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_cookie_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_cookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmDocument_put_cookie_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmDocument_put_cookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmDocument_get_referrer_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmDocument_get_referrer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmDocument_open_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *mimetype,
    /* [retval][out] */ VARIANT __RPC_FAR *retval);


void __RPC_STUB IOmDocument_open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmDocument_close_Proxy( 
    IOmDocument __RPC_FAR * This);


void __RPC_STUB IOmDocument_close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmDocument_clear_Proxy( 
    IOmDocument __RPC_FAR * This);


void __RPC_STUB IOmDocument_clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][id] */ HRESULT __stdcall IOmDocument_write_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ SAFEARRAY __RPC_FAR * psarray);


void __RPC_STUB IOmDocument_write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][id] */ HRESULT __stdcall IOmDocument_writeln_Proxy( 
    IOmDocument __RPC_FAR * This,
    /* [in] */ SAFEARRAY __RPC_FAR * psarray);


void __RPC_STUB IOmDocument_writeln_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOmDocument_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IOmDocument_class;

class DECLSPEC_UUID("D48A6EC9-6A4A-11CF-94A7-444553540000")
IOmDocument_class;
#endif

#ifndef __IBidentOmFramesCollection_INTERFACE_DEFINED__
#define __IBidentOmFramesCollection_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmFramesCollection
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmFramesCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("CC95F7C0-83EA-11CF-9FA7-00AA006C42C4")
    IBidentOmFramesCollection : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall Item( 
            /* [in] */ VARIANT __RPC_FAR *index,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *ppframeresult) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_Count( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [id] */ HRESULT __stdcall _Item( 
            /* [in] */ long i,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *ppframeresult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmFramesCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmFramesCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmFramesCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Item )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *index,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *ppframeresult);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Count )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *_Item )( 
            IBidentOmFramesCollection __RPC_FAR * This,
            /* [in] */ long i,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *ppframeresult);
        
        END_INTERFACE
    } IBidentOmFramesCollectionVtbl;

    interface IBidentOmFramesCollection
    {
        CONST_VTBL struct IBidentOmFramesCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmFramesCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmFramesCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmFramesCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmFramesCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmFramesCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmFramesCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmFramesCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmFramesCollection_Item(This,index,ppframeresult)	\
    (This)->lpVtbl -> Item(This,index,ppframeresult)

#define IBidentOmFramesCollection_get_Count(This,pl)	\
    (This)->lpVtbl -> get_Count(This,pl)

#define IBidentOmFramesCollection_get_length(This,pl)	\
    (This)->lpVtbl -> get_length(This,pl)

#define IBidentOmFramesCollection__Item(This,i,ppframeresult)	\
    (This)->lpVtbl -> _Item(This,i,ppframeresult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmFramesCollection_Item_Proxy( 
    IBidentOmFramesCollection __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *index,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *ppframeresult);


void __RPC_STUB IBidentOmFramesCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmFramesCollection_get_Count_Proxy( 
    IBidentOmFramesCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmFramesCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmFramesCollection_get_length_Proxy( 
    IBidentOmFramesCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmFramesCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmFramesCollection__Item_Proxy( 
    IBidentOmFramesCollection __RPC_FAR * This,
    /* [in] */ long i,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *ppframeresult);


void __RPC_STUB IBidentOmFramesCollection__Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmFramesCollection_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmFramesCollection_class;

class DECLSPEC_UUID("EE2C4BA0-83EA-11CF-9FA7-00AA006C42C4")
IBidentOmFramesCollection_class;
#endif

#ifndef __IOmWindow_INTERFACE_DEFINED__
#define __IOmWindow_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOmWindow
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IOmWindow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("D48A6EC4-6A4A-11CF-94A7-444553540000")
    IOmWindow : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT __stdcall get_name( 
            /* [retval][out] */ BSTR __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_name( 
            /* [in] */ BSTR retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_parent( 
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_self( 
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_top( 
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_window( 
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_location( 
            /* [retval][out] */ IBidentOmLocation __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_frames( 
            /* [retval][out] */ IBidentOmFramesCollection __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [hidden][restricted][propget][id] */ HRESULT __stdcall get_explorer( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_navigator( 
            /* [retval][out] */ IBidentOmNavigator __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_history( 
            /* [retval][out] */ IBidentOmHistory __RPC_FAR *__RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_defaultStatus( 
            /* [in] */ BSTR rhs) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_status( 
            /* [in] */ BSTR rhs) = 0;
        
        virtual /* [id] */ HRESULT __stdcall setTimeout( 
            /* [in] */ BSTR expression,
            /* [in] */ long msec,
            /* [in] */ VARIANT __RPC_FAR *language,
            /* [retval][out] */ long __RPC_FAR *timerID) = 0;
        
        virtual /* [id] */ HRESULT __stdcall clearTimeout( 
            /* [in] */ long timerID) = 0;
        
        virtual /* [id] */ HRESULT __stdcall alert( 
            /* [in] */ VARIANT __RPC_FAR *msg) = 0;
        
        virtual /* [id] */ HRESULT __stdcall close( 
            /* [in] */ VARIANT __RPC_FAR *dummy1,
            /* [in] */ VARIANT __RPC_FAR *dummy2) = 0;
        
        virtual /* [id] */ HRESULT __stdcall confirm( 
            /* [in] */ VARIANT __RPC_FAR *msg,
            /* [retval][out] */ VARIANT __RPC_FAR *confirmed) = 0;
        
        virtual /* [id] */ HRESULT __stdcall prompt( 
            /* [in] */ VARIANT __RPC_FAR *msg,
            /* [in] */ VARIANT __RPC_FAR *initialTxt,
            /* [retval][out] */ BSTR __RPC_FAR *textdata) = 0;
        
        virtual /* [id] */ HRESULT __stdcall open( 
            /* [in] */ VARIANT __RPC_FAR *url,
            /* [in] */ VARIANT __RPC_FAR *windowName,
            /* [in] */ VARIANT __RPC_FAR *windowFeatures,
            /* [in] */ VARIANT __RPC_FAR *width,
            /* [in] */ VARIANT __RPC_FAR *height,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *pomWindowResult) = 0;
        
        virtual /* [id] */ HRESULT __stdcall get_document( 
            /* [retval][out] */ IOmDocument __RPC_FAR *__RPC_FAR *pomDocumentResult) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall fireOnParseComplete( void) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall fireOnLoad( void) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall fireOnUnload( void) = 0;
        
        virtual /* [id] */ HRESULT __stdcall navigate( 
            /* [in] */ BSTR url) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall GetLoadComplete( void) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall GetPageHandle( 
            /* [retval][out] */ long __RPC_FAR *pdwPageHandle) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall postNavigateMessage( void) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_opener( 
            /* [retval][out] */ VARIANT __RPC_FAR *retval) = 0;
        
        virtual /* [propput][id] */ HRESULT __stdcall put_opener( 
            /* [in] */ VARIANT __RPC_FAR *retval) = 0;
        
        virtual /* [hidden][id] */ HRESULT __stdcall GetStream( 
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppstream) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOmWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOmWindow __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOmWindow __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IOmWindow __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_name )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_name )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ BSTR retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_parent )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_self )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_top )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_window )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_location )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IBidentOmLocation __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_frames )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IBidentOmFramesCollection __RPC_FAR *__RPC_FAR *retval);
        
        /* [hidden][restricted][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_explorer )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_navigator )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IBidentOmNavigator __RPC_FAR *__RPC_FAR *retval);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_history )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IBidentOmHistory __RPC_FAR *__RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_defaultStatus )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ BSTR rhs);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_status )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ BSTR rhs);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *setTimeout )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ BSTR expression,
            /* [in] */ long msec,
            /* [in] */ VARIANT __RPC_FAR *language,
            /* [retval][out] */ long __RPC_FAR *timerID);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *clearTimeout )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ long timerID);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *alert )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *msg);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *close )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *dummy1,
            /* [in] */ VARIANT __RPC_FAR *dummy2);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *confirm )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *msg,
            /* [retval][out] */ VARIANT __RPC_FAR *confirmed);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *prompt )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *msg,
            /* [in] */ VARIANT __RPC_FAR *initialTxt,
            /* [retval][out] */ BSTR __RPC_FAR *textdata);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *open )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *url,
            /* [in] */ VARIANT __RPC_FAR *windowName,
            /* [in] */ VARIANT __RPC_FAR *windowFeatures,
            /* [in] */ VARIANT __RPC_FAR *width,
            /* [in] */ VARIANT __RPC_FAR *height,
            /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *pomWindowResult);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *get_document )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ IOmDocument __RPC_FAR *__RPC_FAR *pomDocumentResult);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *fireOnParseComplete )( 
            IOmWindow __RPC_FAR * This);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *fireOnLoad )( 
            IOmWindow __RPC_FAR * This);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *fireOnUnload )( 
            IOmWindow __RPC_FAR * This);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *navigate )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ BSTR url);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *GetLoadComplete )( 
            IOmWindow __RPC_FAR * This);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *GetPageHandle )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pdwPageHandle);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *postNavigateMessage )( 
            IOmWindow __RPC_FAR * This);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_opener )( 
            IOmWindow __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *retval);
        
        /* [propput][id] */ HRESULT ( __stdcall __RPC_FAR *put_opener )( 
            IOmWindow __RPC_FAR * This,
            /* [in] */ VARIANT __RPC_FAR *retval);
        
        /* [hidden][id] */ HRESULT ( __stdcall __RPC_FAR *GetStream )( 
            IOmWindow __RPC_FAR * This,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppstream);
        
        END_INTERFACE
    } IOmWindowVtbl;

    interface IOmWindow
    {
        CONST_VTBL struct IOmWindowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOmWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOmWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOmWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOmWindow_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IOmWindow_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IOmWindow_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IOmWindow_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IOmWindow_get_name(This,retval)	\
    (This)->lpVtbl -> get_name(This,retval)

#define IOmWindow_put_name(This,retval)	\
    (This)->lpVtbl -> put_name(This,retval)

#define IOmWindow_get_parent(This,retval)	\
    (This)->lpVtbl -> get_parent(This,retval)

#define IOmWindow_get_self(This,retval)	\
    (This)->lpVtbl -> get_self(This,retval)

#define IOmWindow_get_top(This,retval)	\
    (This)->lpVtbl -> get_top(This,retval)

#define IOmWindow_get_window(This,retval)	\
    (This)->lpVtbl -> get_window(This,retval)

#define IOmWindow_get_location(This,retval)	\
    (This)->lpVtbl -> get_location(This,retval)

#define IOmWindow_get_frames(This,retval)	\
    (This)->lpVtbl -> get_frames(This,retval)

#define IOmWindow_get_explorer(This,retval)	\
    (This)->lpVtbl -> get_explorer(This,retval)

#define IOmWindow_get_navigator(This,retval)	\
    (This)->lpVtbl -> get_navigator(This,retval)

#define IOmWindow_get_history(This,retval)	\
    (This)->lpVtbl -> get_history(This,retval)

#define IOmWindow_put_defaultStatus(This,rhs)	\
    (This)->lpVtbl -> put_defaultStatus(This,rhs)

#define IOmWindow_put_status(This,rhs)	\
    (This)->lpVtbl -> put_status(This,rhs)

#define IOmWindow_setTimeout(This,expression,msec,language,timerID)	\
    (This)->lpVtbl -> setTimeout(This,expression,msec,language,timerID)

#define IOmWindow_clearTimeout(This,timerID)	\
    (This)->lpVtbl -> clearTimeout(This,timerID)

#define IOmWindow_alert(This,msg)	\
    (This)->lpVtbl -> alert(This,msg)

#define IOmWindow_close(This,dummy1,dummy2)	\
    (This)->lpVtbl -> close(This,dummy1,dummy2)

#define IOmWindow_confirm(This,msg,confirmed)	\
    (This)->lpVtbl -> confirm(This,msg,confirmed)

#define IOmWindow_prompt(This,msg,initialTxt,textdata)	\
    (This)->lpVtbl -> prompt(This,msg,initialTxt,textdata)

#define IOmWindow_open(This,url,windowName,windowFeatures,width,height,pomWindowResult)	\
    (This)->lpVtbl -> open(This,url,windowName,windowFeatures,width,height,pomWindowResult)

#define IOmWindow_get_document(This,pomDocumentResult)	\
    (This)->lpVtbl -> get_document(This,pomDocumentResult)

#define IOmWindow_fireOnParseComplete(This)	\
    (This)->lpVtbl -> fireOnParseComplete(This)

#define IOmWindow_fireOnLoad(This)	\
    (This)->lpVtbl -> fireOnLoad(This)

#define IOmWindow_fireOnUnload(This)	\
    (This)->lpVtbl -> fireOnUnload(This)

#define IOmWindow_navigate(This,url)	\
    (This)->lpVtbl -> navigate(This,url)

#define IOmWindow_GetLoadComplete(This)	\
    (This)->lpVtbl -> GetLoadComplete(This)

#define IOmWindow_GetPageHandle(This,pdwPageHandle)	\
    (This)->lpVtbl -> GetPageHandle(This,pdwPageHandle)

#define IOmWindow_postNavigateMessage(This)	\
    (This)->lpVtbl -> postNavigateMessage(This)

#define IOmWindow_get_opener(This,retval)	\
    (This)->lpVtbl -> get_opener(This,retval)

#define IOmWindow_put_opener(This,retval)	\
    (This)->lpVtbl -> put_opener(This,retval)

#define IOmWindow_GetStream(This,ppstream)	\
    (This)->lpVtbl -> GetStream(This,ppstream)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT __stdcall IOmWindow_get_name_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmWindow_put_name_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ BSTR retval);


void __RPC_STUB IOmWindow_put_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_parent_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_self_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_self_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_top_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_window_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_window_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_location_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IBidentOmLocation __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_location_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_frames_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IBidentOmFramesCollection __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_frames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][restricted][propget][id] */ HRESULT __stdcall IOmWindow_get_explorer_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_explorer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_navigator_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IBidentOmNavigator __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_navigator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_history_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IBidentOmHistory __RPC_FAR *__RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_history_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmWindow_put_defaultStatus_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ BSTR rhs);


void __RPC_STUB IOmWindow_put_defaultStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmWindow_put_status_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ BSTR rhs);


void __RPC_STUB IOmWindow_put_status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_setTimeout_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ BSTR expression,
    /* [in] */ long msec,
    /* [in] */ VARIANT __RPC_FAR *language,
    /* [retval][out] */ long __RPC_FAR *timerID);


void __RPC_STUB IOmWindow_setTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_clearTimeout_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ long timerID);


void __RPC_STUB IOmWindow_clearTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_alert_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *msg);


void __RPC_STUB IOmWindow_alert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_close_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *dummy1,
    /* [in] */ VARIANT __RPC_FAR *dummy2);


void __RPC_STUB IOmWindow_close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_confirm_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *msg,
    /* [retval][out] */ VARIANT __RPC_FAR *confirmed);


void __RPC_STUB IOmWindow_confirm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_prompt_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *msg,
    /* [in] */ VARIANT __RPC_FAR *initialTxt,
    /* [retval][out] */ BSTR __RPC_FAR *textdata);


void __RPC_STUB IOmWindow_prompt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_open_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *url,
    /* [in] */ VARIANT __RPC_FAR *windowName,
    /* [in] */ VARIANT __RPC_FAR *windowFeatures,
    /* [in] */ VARIANT __RPC_FAR *width,
    /* [in] */ VARIANT __RPC_FAR *height,
    /* [retval][out] */ IOmWindow __RPC_FAR *__RPC_FAR *pomWindowResult);


void __RPC_STUB IOmWindow_open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_get_document_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ IOmDocument __RPC_FAR *__RPC_FAR *pomDocumentResult);


void __RPC_STUB IOmWindow_get_document_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_fireOnParseComplete_Proxy( 
    IOmWindow __RPC_FAR * This);


void __RPC_STUB IOmWindow_fireOnParseComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_fireOnLoad_Proxy( 
    IOmWindow __RPC_FAR * This);


void __RPC_STUB IOmWindow_fireOnLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_fireOnUnload_Proxy( 
    IOmWindow __RPC_FAR * This);


void __RPC_STUB IOmWindow_fireOnUnload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IOmWindow_navigate_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ BSTR url);


void __RPC_STUB IOmWindow_navigate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_GetLoadComplete_Proxy( 
    IOmWindow __RPC_FAR * This);


void __RPC_STUB IOmWindow_GetLoadComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_GetPageHandle_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pdwPageHandle);


void __RPC_STUB IOmWindow_GetPageHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_postNavigateMessage_Proxy( 
    IOmWindow __RPC_FAR * This);


void __RPC_STUB IOmWindow_postNavigateMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IOmWindow_get_opener_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *retval);


void __RPC_STUB IOmWindow_get_opener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT __stdcall IOmWindow_put_opener_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [in] */ VARIANT __RPC_FAR *retval);


void __RPC_STUB IOmWindow_put_opener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][id] */ HRESULT __stdcall IOmWindow_GetStream_Proxy( 
    IOmWindow __RPC_FAR * This,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppstream);


void __RPC_STUB IOmWindow_GetStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOmWindow_INTERFACE_DEFINED__ */


#ifndef __IOmWindowEvents_DISPINTERFACE_DEFINED__
#define __IOmWindowEvents_DISPINTERFACE_DEFINED__

/****************************************
 * Generated header for dispinterface: IOmWindowEvents
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [helpstring][uuid] */ 



EXTERN_C const IID DIID_IOmWindowEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface DECLSPEC_UUID("96A0A4E0-D062-11CF-94B6-00AA0060275C")
    IOmWindowEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IOmWindowEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOmWindowEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOmWindowEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOmWindowEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IOmWindowEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IOmWindowEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IOmWindowEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IOmWindowEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IOmWindowEventsVtbl;

    interface IOmWindowEvents
    {
        CONST_VTBL struct IOmWindowEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOmWindowEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOmWindowEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOmWindowEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOmWindowEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IOmWindowEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IOmWindowEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IOmWindowEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IOmWindowEvents_DISPINTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IOmWindow_class;

class DECLSPEC_UUID("D48A6EC6-6A4A-11CF-94A7-444553540000")
IOmWindow_class;
#endif

#ifndef __IBidentOmRadioButtonGroup_INTERFACE_DEFINED__
#define __IBidentOmRadioButtonGroup_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentOmRadioButtonGroup
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentOmRadioButtonGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("BCC5AF22-7300-11CF-8F20-00805F2CD064")
    IBidentOmRadioButtonGroup : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT __stdcall Item( 
            /* [in] */ long i,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppeleresult) = 0;
        
        virtual /* [id] */ HRESULT __stdcall Count( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
        virtual /* [propget][id] */ HRESULT __stdcall get_length( 
            /* [retval][out] */ long __RPC_FAR *pl) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentOmRadioButtonGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Item )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [in] */ long i,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppeleresult);
        
        /* [id] */ HRESULT ( __stdcall __RPC_FAR *Count )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        /* [propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_length )( 
            IBidentOmRadioButtonGroup __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pl);
        
        END_INTERFACE
    } IBidentOmRadioButtonGroupVtbl;

    interface IBidentOmRadioButtonGroup
    {
        CONST_VTBL struct IBidentOmRadioButtonGroupVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentOmRadioButtonGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentOmRadioButtonGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentOmRadioButtonGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentOmRadioButtonGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentOmRadioButtonGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentOmRadioButtonGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentOmRadioButtonGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentOmRadioButtonGroup_Item(This,i,ppeleresult)	\
    (This)->lpVtbl -> Item(This,i,ppeleresult)

#define IBidentOmRadioButtonGroup_Count(This,pl)	\
    (This)->lpVtbl -> Count(This,pl)

#define IBidentOmRadioButtonGroup_get_length(This,pl)	\
    (This)->lpVtbl -> get_length(This,pl)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT __stdcall IBidentOmRadioButtonGroup_Item_Proxy( 
    IBidentOmRadioButtonGroup __RPC_FAR * This,
    /* [in] */ long i,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppeleresult);


void __RPC_STUB IBidentOmRadioButtonGroup_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT __stdcall IBidentOmRadioButtonGroup_Count_Proxy( 
    IBidentOmRadioButtonGroup __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmRadioButtonGroup_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT __stdcall IBidentOmRadioButtonGroup_get_length_Proxy( 
    IBidentOmRadioButtonGroup __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pl);


void __RPC_STUB IBidentOmRadioButtonGroup_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentOmRadioButtonGroup_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentOmRadioButtonGroup_Class;

class DECLSPEC_UUID("BCC5AF23-7300-11CF-8F20-00805F2CD064")
IBidentOmRadioButtonGroup_Class;
#endif

#ifndef __IBidentHTMLDocument_INTERFACE_DEFINED__
#define __IBidentHTMLDocument_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IBidentHTMLDocument
 * at Tue Sep 30 18:00:54 1997
 * using MIDL 3.01.75
 ****************************************/
/* [object][oleautomation][dual][helpstring][uuid] */ 



EXTERN_C const IID IID_IBidentHTMLDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("626FC520-A41E-11CF-A731-00A0C9082637")
    IBidentHTMLDocument : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT __stdcall get_Script( 
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBidentHTMLDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBidentHTMLDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBidentHTMLDocument __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBidentHTMLDocument __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBidentHTMLDocument __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBidentHTMLDocument __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBidentHTMLDocument __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBidentHTMLDocument __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][propget][id] */ HRESULT ( __stdcall __RPC_FAR *get_Script )( 
            IBidentHTMLDocument __RPC_FAR * This,
            /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
        
        END_INTERFACE
    } IBidentHTMLDocumentVtbl;

    interface IBidentHTMLDocument
    {
        CONST_VTBL struct IBidentHTMLDocumentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBidentHTMLDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBidentHTMLDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBidentHTMLDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBidentHTMLDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBidentHTMLDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBidentHTMLDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBidentHTMLDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBidentHTMLDocument_get_Script(This,ppDisp)	\
    (This)->lpVtbl -> get_Script(This,ppDisp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][propget][id] */ HRESULT __stdcall IBidentHTMLDocument_get_Script_Proxy( 
    IBidentHTMLDocument __RPC_FAR * This,
    /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp);


void __RPC_STUB IBidentHTMLDocument_get_Script_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBidentHTMLDocument_INTERFACE_DEFINED__ */


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_IBidentHTMLDocument_class;

class DECLSPEC_UUID("25336920-03F9-11CF-8FD0-00AA00686F13")
IBidentHTMLDocument_class;
#endif
#endif /* __IEScriptObjectModel_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
