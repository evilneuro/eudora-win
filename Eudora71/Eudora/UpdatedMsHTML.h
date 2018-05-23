//	UpdatedMsHMTL.h
//
//	Excerpted portions of a newer version of MsHTML.h to allow TridentView.cpp
//	to make use of IHTMLElement2. Contains only those interfaces necessary
//	to allow the IHTMLElement2 interface to compile.
//
//	Excerpted because bringing in the entire newer version of MsHTML.h did
//	not compile without the rest of the latest MS SDK. Will no longer
//	be needed once we update to the latest MS SDK, or the latest compiler
//	which includes the latest MS SDK.
//
//	When this is no longer necessary, the copies of Uuid.Lib in the lib
//	directories will no longer be necessary either.


#ifndef __IHTMLRect_INTERFACE_DEFINED__
#define __IHTMLRect_INTERFACE_DEFINED__

/* interface IHTMLRect */
/* [object][uuid][dual][oleautomation] */ 


EXTERN_C const IID IID_IHTMLRect;

    
    MIDL_INTERFACE("3050f4a3-98b5-11cf-bb82-00aa00bdce0b")
    IHTMLRect : public IDispatch
    {
    public:
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_left( 
            /* [in] */ long v) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_left( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_top( 
            /* [in] */ long v) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_top( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_right( 
            /* [in] */ long v) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_right( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_bottom( 
            /* [in] */ long v) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_bottom( 
            /* [out][retval] */ long *p) = 0;
        
    };
    



/* [id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLRect_put_left_Proxy( 
    IHTMLRect * This,
    /* [in] */ long v);


void __RPC_STUB IHTMLRect_put_left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLRect_get_left_Proxy( 
    IHTMLRect * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLRect_get_left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLRect_put_top_Proxy( 
    IHTMLRect * This,
    /* [in] */ long v);


void __RPC_STUB IHTMLRect_put_top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLRect_get_top_Proxy( 
    IHTMLRect * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLRect_get_top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLRect_put_right_Proxy( 
    IHTMLRect * This,
    /* [in] */ long v);


void __RPC_STUB IHTMLRect_put_right_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLRect_get_right_Proxy( 
    IHTMLRect * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLRect_get_right_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLRect_put_bottom_Proxy( 
    IHTMLRect * This,
    /* [in] */ long v);


void __RPC_STUB IHTMLRect_put_bottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLRect_get_bottom_Proxy( 
    IHTMLRect * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLRect_get_bottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHTMLRect_INTERFACE_DEFINED__ */


#ifndef __IHTMLRectCollection_INTERFACE_DEFINED__
#define __IHTMLRectCollection_INTERFACE_DEFINED__

/* interface IHTMLRectCollection */
/* [object][uuid][dual][oleautomation] */ 


EXTERN_C const IID IID_IHTMLRectCollection;

    
    MIDL_INTERFACE("3050f4a4-98b5-11cf-bb82-00aa00bdce0b")
    IHTMLRectCollection : public IDispatch
    {
    public:
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_length( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get__newEnum( 
            /* [out][retval] */ IUnknown **p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE item( 
            /* [in] */ VARIANT *pvarIndex,
            /* [out][retval] */ VARIANT *pvarResult) = 0;
        
    };
    



/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLRectCollection_get_length_Proxy( 
    IHTMLRectCollection * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLRectCollection_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLRectCollection_get__newEnum_Proxy( 
    IHTMLRectCollection * This,
    /* [out][retval] */ IUnknown **p);


void __RPC_STUB IHTMLRectCollection_get__newEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLRectCollection_item_Proxy( 
    IHTMLRectCollection * This,
    /* [in] */ VARIANT *pvarIndex,
    /* [out][retval] */ VARIANT *pvarResult);


void __RPC_STUB IHTMLRectCollection_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHTMLRectCollection_INTERFACE_DEFINED__ */



#ifndef __IHTMLCurrentStyle_INTERFACE_DEFINED__
#define __IHTMLCurrentStyle_INTERFACE_DEFINED__

/* interface IHTMLCurrentStyle */
/* [object][uuid][dual][oleautomation] */ 


EXTERN_C const IID IID_IHTMLCurrentStyle;

    
    MIDL_INTERFACE("3050f3db-98b5-11cf-bb82-00aa00bdce0b")
    IHTMLCurrentStyle : public IDispatch
    {
    public:
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_position( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_styleFloat( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_color( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_backgroundColor( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_fontFamily( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_fontStyle( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [hidden][bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_fontVariant( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_fontWeight( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_fontSize( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_backgroundImage( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_backgroundPositionX( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_backgroundPositionY( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_backgroundRepeat( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderLeftColor( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderTopColor( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderRightColor( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderBottomColor( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderTopStyle( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderRightStyle( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderBottomStyle( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderLeftStyle( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderTopWidth( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderRightWidth( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderBottomWidth( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderLeftWidth( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_left( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_top( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_width( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_height( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_paddingLeft( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_paddingTop( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_paddingRight( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_paddingBottom( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textAlign( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textDecoration( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_display( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_visibility( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_zIndex( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_letterSpacing( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_lineHeight( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textIndent( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_verticalAlign( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_backgroundAttachment( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_marginTop( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_marginRight( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_marginBottom( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_marginLeft( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clear( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_listStyleType( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_listStylePosition( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_listStyleImage( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clipTop( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clipRight( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clipBottom( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clipLeft( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_overflow( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_pageBreakBefore( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_pageBreakAfter( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_cursor( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_tableLayout( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderCollapse( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_direction( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_behavior( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getAttribute( 
            /* [in] */ BSTR strAttributeName,
            /* [in][defaultvalue] */ LONG lFlags,
            /* [out][retval] */ VARIANT *AttributeValue) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_unicodeBidi( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_right( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_bottom( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_imeMode( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_rubyAlign( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_rubyPosition( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_rubyOverhang( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textAutospace( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_lineBreak( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_wordBreak( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textJustify( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textJustifyTrim( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textKashida( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_blockDirection( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_layoutGridChar( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_layoutGridLine( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_layoutGridMode( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_layoutGridType( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderStyle( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderColor( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_borderWidth( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_padding( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_margin( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_accelerator( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_overflowX( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_overflowY( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_textTransform( 
            /* [out][retval] */ BSTR *p) = 0;
        
    };
    



/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_position_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_position_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_styleFloat_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_styleFloat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_color_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_color_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_backgroundColor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_backgroundColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_fontFamily_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_fontFamily_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_fontStyle_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_fontStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_fontVariant_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_fontVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_fontWeight_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_fontWeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_fontSize_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_fontSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_backgroundImage_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_backgroundImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_backgroundPositionX_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_backgroundPositionX_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_backgroundPositionY_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_backgroundPositionY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_backgroundRepeat_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_backgroundRepeat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderLeftColor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderLeftColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderTopColor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderTopColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderRightColor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderRightColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderBottomColor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderBottomColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderTopStyle_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderTopStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderRightStyle_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderRightStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderBottomStyle_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderBottomStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderLeftStyle_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderLeftStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderTopWidth_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderTopWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderRightWidth_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderRightWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderBottomWidth_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderBottomWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderLeftWidth_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderLeftWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_left_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_top_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_width_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_height_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_paddingLeft_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_paddingLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_paddingTop_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_paddingTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_paddingRight_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_paddingRight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_paddingBottom_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_paddingBottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textAlign_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_textAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textDecoration_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_textDecoration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_display_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_display_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_visibility_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_visibility_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_zIndex_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_zIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_letterSpacing_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_letterSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_lineHeight_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_lineHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textIndent_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_textIndent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_verticalAlign_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_verticalAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_backgroundAttachment_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_backgroundAttachment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_marginTop_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_marginTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_marginRight_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_marginRight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_marginBottom_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_marginBottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_marginLeft_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_marginLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_clear_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_listStyleType_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_listStyleType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_listStylePosition_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_listStylePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_listStyleImage_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_listStyleImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_clipTop_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_clipTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_clipRight_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_clipRight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_clipBottom_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_clipBottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_clipLeft_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_clipLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_overflow_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_overflow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_pageBreakBefore_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_pageBreakBefore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_pageBreakAfter_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_pageBreakAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_cursor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_cursor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_tableLayout_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_tableLayout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderCollapse_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderCollapse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_direction_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_behavior_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_behavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_getAttribute_Proxy( 
    IHTMLCurrentStyle * This,
    /* [in] */ BSTR strAttributeName,
    /* [in][defaultvalue] */ LONG lFlags,
    /* [out][retval] */ VARIANT *AttributeValue);


void __RPC_STUB IHTMLCurrentStyle_getAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_unicodeBidi_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_unicodeBidi_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_right_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_right_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_bottom_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_bottom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_imeMode_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_imeMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_rubyAlign_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_rubyAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_rubyPosition_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_rubyPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_rubyOverhang_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_rubyOverhang_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textAutospace_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_textAutospace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_lineBreak_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_lineBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_wordBreak_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_wordBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textJustify_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_textJustify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textJustifyTrim_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_textJustifyTrim_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textKashida_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_textKashida_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_blockDirection_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_blockDirection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_layoutGridChar_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_layoutGridChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_layoutGridLine_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLCurrentStyle_get_layoutGridLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_layoutGridMode_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_layoutGridMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_layoutGridType_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_layoutGridType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderStyle_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderColor_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_borderWidth_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_borderWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_padding_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_padding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_margin_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_margin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_accelerator_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_accelerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_overflowX_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_overflowX_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_overflowY_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_overflowY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLCurrentStyle_get_textTransform_Proxy( 
    IHTMLCurrentStyle * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLCurrentStyle_get_textTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHTMLCurrentStyle_INTERFACE_DEFINED__ */



#ifndef __IHTMLElement2_INTERFACE_DEFINED__
#define __IHTMLElement2_INTERFACE_DEFINED__

/* interface IHTMLElement2 */
/* [object][uuid][dual][oleautomation] */ 


EXTERN_C const IID IID_IHTMLElement2;

    
    MIDL_INTERFACE("3050f434-98b5-11cf-bb82-00aa00bdce0b")
    IHTMLElement2 : public IDispatch
    {
    public:
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_scopeName( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE setCapture( 
            /* [in][defaultvalue] */ VARIANT_BOOL containerCapture = -1) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE releaseCapture( void) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onlosecapture( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onlosecapture( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE componentFromPoint( 
            /* [in] */ long x,
            /* [in] */ long y,
            /* [out][retval] */ BSTR *component) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE doScroll( 
            /* [in][optional] */ VARIANT component) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onscroll( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onscroll( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondrag( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_ondrag( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondragend( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_ondragend( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondragenter( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_ondragenter( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondragover( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_ondragover( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondragleave( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_ondragleave( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_ondrop( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_ondrop( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onbeforecut( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onbeforecut( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_oncut( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_oncut( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onbeforecopy( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onbeforecopy( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_oncopy( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_oncopy( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onbeforepaste( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onbeforepaste( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onpaste( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onpaste( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE get_currentStyle( 
            /* [out][retval] */ IHTMLCurrentStyle **p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onpropertychange( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onpropertychange( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getClientRects( 
            /* [out][retval] */ IHTMLRectCollection **pRectCol) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getBoundingClientRect( 
            /* [out][retval] */ IHTMLRect **pRect) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE setExpression( 
            /* [in] */ BSTR propname,
            /* [in] */ BSTR expression,
            /* [in][defaultvalue] */ BSTR language = L"") = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getExpression( 
            /* [in] */ BSTR propname,
            /* [out][retval] */ VARIANT *expression) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE removeExpression( 
            /* [in] */ BSTR propname,
            /* [out][retval] */ VARIANT_BOOL *pfSuccess) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_tabIndex( 
            /* [in] */ short v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_tabIndex( 
            /* [out][retval] */ short *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE focus( void) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_accessKey( 
            /* [in] */ BSTR v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_accessKey( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onblur( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onblur( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onfocus( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onfocus( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onresize( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onresize( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE blur( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE addFilter( 
            /* [in] */ IUnknown *pUnk) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE removeFilter( 
            /* [in] */ IUnknown *pUnk) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clientHeight( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clientWidth( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clientTop( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_clientLeft( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE attachEvent( 
            /* [in] */ BSTR event,
            /* [in] */ IDispatch *pDisp,
            /* [out][retval] */ VARIANT_BOOL *pfResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE detachEvent( 
            /* [in] */ BSTR event,
            /* [in] */ IDispatch *pDisp) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_readyState( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onreadystatechange( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onreadystatechange( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onrowsdelete( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onrowsdelete( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onrowsinserted( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onrowsinserted( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_oncellchange( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_oncellchange( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_dir( 
            /* [in] */ BSTR v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_dir( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE createControlRange( 
            /* [out][retval] */ IDispatch **range) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_scrollHeight( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_scrollWidth( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_scrollTop( 
            /* [in] */ long v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_scrollTop( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_scrollLeft( 
            /* [in] */ long v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_scrollLeft( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE clearAttributes( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE mergeAttributes( 
            /* [in] */ IHTMLElement *mergeThis) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_oncontextmenu( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_oncontextmenu( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE insertAdjacentElement( 
            /* [in] */ BSTR where,
            /* [in] */ IHTMLElement *insertedElement,
            /* [out][retval] */ IHTMLElement **inserted) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE applyElement( 
            /* [in] */ IHTMLElement *apply,
            /* [in] */ BSTR where,
            /* [out][retval] */ IHTMLElement **applied) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getAdjacentText( 
            /* [in] */ BSTR where,
            /* [out][retval] */ BSTR *text) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE replaceAdjacentText( 
            /* [in] */ BSTR where,
            /* [in] */ BSTR newText,
            /* [out][retval] */ BSTR *oldText) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_canHaveChildren( 
            /* [out][retval] */ VARIANT_BOOL *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE addBehavior( 
            /* [in] */ BSTR bstrUrl,
            /* [in][optional] */ VARIANT *pvarFactory,
            /* [out][retval] */ long *pCookie) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE removeBehavior( 
            /* [in] */ long cookie,
            /* [out][retval] */ VARIANT_BOOL *pfResult) = 0;
        
        virtual /* [nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE get_runtimeStyle( 
            /* [out][retval] */ IHTMLStyle **p) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_behaviorUrns( 
            /* [out][retval] */ IDispatch **p) = 0;
        
        virtual /* [id][propput] */ HRESULT STDMETHODCALLTYPE put_tagUrn( 
            /* [in] */ BSTR v) = 0;
        
        virtual /* [id][propget] */ HRESULT STDMETHODCALLTYPE get_tagUrn( 
            /* [out][retval] */ BSTR *p) = 0;
        
        virtual /* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE put_onbeforeeditfocus( 
            /* [in] */ VARIANT v) = 0;
        
        virtual /* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE get_onbeforeeditfocus( 
            /* [out][retval] */ VARIANT *p) = 0;
        
        virtual /* [restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get_readyStateValue( 
            /* [out][retval] */ long *p) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE getElementsByTagName( 
            /* [in] */ BSTR v,
            /* [out][retval] */ IHTMLElementCollection **pelColl) = 0;
        
    };
    



/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_scopeName_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLElement2_get_scopeName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_setCapture_Proxy( 
    IHTMLElement2 * This,
    /* [in][defaultvalue] */ VARIANT_BOOL containerCapture);


void __RPC_STUB IHTMLElement2_setCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_releaseCapture_Proxy( 
    IHTMLElement2 * This);


void __RPC_STUB IHTMLElement2_releaseCapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onlosecapture_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onlosecapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onlosecapture_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onlosecapture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_componentFromPoint_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ long x,
    /* [in] */ long y,
    /* [out][retval] */ BSTR *component);


void __RPC_STUB IHTMLElement2_componentFromPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_doScroll_Proxy( 
    IHTMLElement2 * This,
    /* [in][optional] */ VARIANT component);


void __RPC_STUB IHTMLElement2_doScroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onscroll_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onscroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onscroll_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onscroll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_ondrag_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_ondrag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_ondrag_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_ondrag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_ondragend_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_ondragend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_ondragend_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_ondragend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_ondragenter_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_ondragenter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_ondragenter_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_ondragenter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_ondragover_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_ondragover_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_ondragover_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_ondragover_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_ondragleave_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_ondragleave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_ondragleave_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_ondragleave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_ondrop_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_ondrop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_ondrop_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_ondrop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onbeforecut_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onbeforecut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onbeforecut_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onbeforecut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_oncut_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_oncut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_oncut_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_oncut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onbeforecopy_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onbeforecopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onbeforecopy_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onbeforecopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_oncopy_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_oncopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_oncopy_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_oncopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onbeforepaste_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onbeforepaste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onbeforepaste_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onbeforepaste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onpaste_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onpaste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onpaste_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onpaste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_currentStyle_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ IHTMLCurrentStyle **p);


void __RPC_STUB IHTMLElement2_get_currentStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onpropertychange_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onpropertychange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onpropertychange_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onpropertychange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_getClientRects_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ IHTMLRectCollection **pRectCol);


void __RPC_STUB IHTMLElement2_getClientRects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_getBoundingClientRect_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ IHTMLRect **pRect);


void __RPC_STUB IHTMLElement2_getBoundingClientRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_setExpression_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR propname,
    /* [in] */ BSTR expression,
    /* [in][defaultvalue] */ BSTR language);


void __RPC_STUB IHTMLElement2_setExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_getExpression_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR propname,
    /* [out][retval] */ VARIANT *expression);


void __RPC_STUB IHTMLElement2_getExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_removeExpression_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR propname,
    /* [out][retval] */ VARIANT_BOOL *pfSuccess);


void __RPC_STUB IHTMLElement2_removeExpression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_tabIndex_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ short v);


void __RPC_STUB IHTMLElement2_put_tabIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_tabIndex_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ short *p);


void __RPC_STUB IHTMLElement2_get_tabIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_focus_Proxy( 
    IHTMLElement2 * This);


void __RPC_STUB IHTMLElement2_focus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_accessKey_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR v);


void __RPC_STUB IHTMLElement2_put_accessKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_accessKey_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLElement2_get_accessKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onblur_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onblur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onblur_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onblur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onfocus_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onfocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onfocus_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onfocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onresize_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onresize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onresize_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onresize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_blur_Proxy( 
    IHTMLElement2 * This);


void __RPC_STUB IHTMLElement2_blur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_addFilter_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ IUnknown *pUnk);


void __RPC_STUB IHTMLElement2_addFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_removeFilter_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ IUnknown *pUnk);


void __RPC_STUB IHTMLElement2_removeFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_clientHeight_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_clientHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_clientWidth_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_clientWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_clientTop_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_clientTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_clientLeft_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_clientLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_attachEvent_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR event,
    /* [in] */ IDispatch *pDisp,
    /* [out][retval] */ VARIANT_BOOL *pfResult);


void __RPC_STUB IHTMLElement2_attachEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_detachEvent_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR event,
    /* [in] */ IDispatch *pDisp);


void __RPC_STUB IHTMLElement2_detachEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_readyState_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_readyState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onreadystatechange_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onreadystatechange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onreadystatechange_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onreadystatechange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onrowsdelete_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onrowsdelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onrowsdelete_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onrowsdelete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onrowsinserted_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onrowsinserted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onrowsinserted_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onrowsinserted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_oncellchange_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_oncellchange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_oncellchange_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_oncellchange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_dir_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR v);


void __RPC_STUB IHTMLElement2_put_dir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_dir_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLElement2_get_dir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_createControlRange_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ IDispatch **range);


void __RPC_STUB IHTMLElement2_createControlRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_scrollHeight_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_scrollHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_scrollWidth_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_scrollWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_scrollTop_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ long v);


void __RPC_STUB IHTMLElement2_put_scrollTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_scrollTop_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_scrollTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_scrollLeft_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ long v);


void __RPC_STUB IHTMLElement2_put_scrollLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_scrollLeft_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_scrollLeft_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_clearAttributes_Proxy( 
    IHTMLElement2 * This);


void __RPC_STUB IHTMLElement2_clearAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_mergeAttributes_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ IHTMLElement *mergeThis);


void __RPC_STUB IHTMLElement2_mergeAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_oncontextmenu_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_oncontextmenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_oncontextmenu_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_oncontextmenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_insertAdjacentElement_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR where,
    /* [in] */ IHTMLElement *insertedElement,
    /* [out][retval] */ IHTMLElement **inserted);


void __RPC_STUB IHTMLElement2_insertAdjacentElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_applyElement_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ IHTMLElement *apply,
    /* [in] */ BSTR where,
    /* [out][retval] */ IHTMLElement **applied);


void __RPC_STUB IHTMLElement2_applyElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_getAdjacentText_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR where,
    /* [out][retval] */ BSTR *text);


void __RPC_STUB IHTMLElement2_getAdjacentText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_replaceAdjacentText_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR where,
    /* [in] */ BSTR newText,
    /* [out][retval] */ BSTR *oldText);


void __RPC_STUB IHTMLElement2_replaceAdjacentText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_canHaveChildren_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT_BOOL *p);


void __RPC_STUB IHTMLElement2_get_canHaveChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_addBehavior_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR bstrUrl,
    /* [in][optional] */ VARIANT *pvarFactory,
    /* [out][retval] */ long *pCookie);


void __RPC_STUB IHTMLElement2_addBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_removeBehavior_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ long cookie,
    /* [out][retval] */ VARIANT_BOOL *pfResult);


void __RPC_STUB IHTMLElement2_removeBehavior_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [nonbrowsable][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_runtimeStyle_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ IHTMLStyle **p);


void __RPC_STUB IHTMLElement2_get_runtimeStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_behaviorUrns_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ IDispatch **p);


void __RPC_STUB IHTMLElement2_get_behaviorUrns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_tagUrn_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR v);


void __RPC_STUB IHTMLElement2_put_tagUrn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_tagUrn_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ BSTR *p);


void __RPC_STUB IHTMLElement2_get_tagUrn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propput] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_put_onbeforeeditfocus_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ VARIANT v);


void __RPC_STUB IHTMLElement2_put_onbeforeeditfocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [bindable][displaybind][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_onbeforeeditfocus_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ VARIANT *p);


void __RPC_STUB IHTMLElement2_get_onbeforeeditfocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_get_readyStateValue_Proxy( 
    IHTMLElement2 * This,
    /* [out][retval] */ long *p);


void __RPC_STUB IHTMLElement2_get_readyStateValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IHTMLElement2_getElementsByTagName_Proxy( 
    IHTMLElement2 * This,
    /* [in] */ BSTR v,
    /* [out][retval] */ IHTMLElementCollection **pelColl);


void __RPC_STUB IHTMLElement2_getElementsByTagName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHTMLElement2_INTERFACE_DEFINED__ */