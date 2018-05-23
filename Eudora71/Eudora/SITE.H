/*
 * SITE.H
 * View Site for Document Objects.
 *
 * Copyright (c)1995-1997 Microsoft Corporation, All Rights Reserved
 */


#ifndef _SITE_H_
#define _SITE_H_

#include <mshtml.h>
#include <mshtmhst.h>

class CImpIDispatch;

enum SiteErrors
{
	SITE_ERROR_NONE, // no error
	SITE_ERROR_NO_MSHTML_DLL,
	SITE_ERROR_MEMORY,
	SITE_ERROR_INVALID_OBJECT,
	SITE_ERROR_INVALID_OLE_OBJECT,
	SITE_ERROR_UNABLE_TO_LOAD_FILE
};


class CImpIOleClientSite : public IOleClientSite
{
    protected:
        ULONG               m_cRef;
        class CSite        *m_pSite;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIOleClientSite(class CSite *, IUnknown *);
        ~CImpIOleClientSite(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP SaveObject(void);
        STDMETHODIMP GetMoniker(DWORD, DWORD, LPMONIKER *);
        STDMETHODIMP GetContainer(LPOLECONTAINER *);
        STDMETHODIMP ShowObject(void);
        STDMETHODIMP OnShowWindow(BOOL);
        STDMETHODIMP RequestNewObjectLayout(void);
};

typedef CImpIOleClientSite *PCImpIOleClientSite;



class CImpIAdviseSink : public IAdviseSink
{
    protected:
        ULONG               m_cRef;
        class CSite        *m_pSite;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIAdviseSink(class CSite *, IUnknown *);
        ~CImpIAdviseSink(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP_(void)  OnViewChange(DWORD, LONG);
        STDMETHODIMP_(void)  OnRename(LPMONIKER);
        STDMETHODIMP_(void)  OnSave(void);
        STDMETHODIMP_(void)  OnClose(void);
};


typedef CImpIAdviseSink *PCImpIAdviseSink;


class CImpIOleInPlaceSite : public IOleInPlaceSite
{
    protected:
        ULONG               m_cRef;
        class CSite        *m_pSite;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIOleInPlaceSite(class CSite *, IUnknown *);
        ~CImpIOleInPlaceSite(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetWindow(HWND *);
        STDMETHODIMP ContextSensitiveHelp(BOOL);
        STDMETHODIMP CanInPlaceActivate(void);
        STDMETHODIMP OnInPlaceActivate(void);
        STDMETHODIMP OnUIActivate(void);
        STDMETHODIMP GetWindowContext(LPOLEINPLACEFRAME *
                        , LPOLEINPLACEUIWINDOW *, LPRECT, LPRECT
                        , LPOLEINPLACEFRAMEINFO);
        STDMETHODIMP Scroll(SIZE);
        STDMETHODIMP OnUIDeactivate(BOOL);
        STDMETHODIMP OnInPlaceDeactivate(void);
        STDMETHODIMP DiscardUndoState(void);
        STDMETHODIMP DeactivateAndUndo(void);
        STDMETHODIMP OnPosRectChange(LPCRECT);
};

typedef CImpIOleInPlaceSite *PCImpIOleInPlaceSite;


class CImpIOleDocumentSite : public IOleDocumentSite
{
    protected:
        ULONG               m_cRef;
        class CSite        *m_pSite;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIOleDocumentSite(class CSite *, IUnknown *);
        ~CImpIOleDocumentSite(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP ActivateMe(IOleDocumentView *);
};

typedef CImpIOleDocumentSite *PCImpIOleDocumentSite;



///////////////////////////////////////////////////
// MSHTML.DLL host integration interfaces
///////////////////////////////////////////////////
class CImpIDocHostUIHandler : public IDocHostUIHandler
{
    protected:
        ULONG               m_cRef;
        class CSite        *m_pSite;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIDocHostUIHandler(class CSite *, IUnknown *);
        ~CImpIDocHostUIHandler(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		STDMETHODIMP GetHostInfo(DOCHOSTUIINFO * pInfo);
		STDMETHODIMP ShowUI(
				DWORD dwID, 
				IOleInPlaceActiveObject * pActiveObject,
				IOleCommandTarget * pCommandTarget,
				IOleInPlaceFrame * pFrame,
				IOleInPlaceUIWindow * pDoc);
		STDMETHODIMP HideUI(void);
		STDMETHODIMP UpdateUI(void);
		STDMETHODIMP EnableModeless(BOOL fEnable);
		STDMETHODIMP OnDocWindowActivate(BOOL fActivate);
		STDMETHODIMP OnFrameWindowActivate(BOOL fActivate);
		STDMETHODIMP ResizeBorder(
				LPCRECT prcBorder, 
				IOleInPlaceUIWindow * pUIWindow, 
				BOOL fRameWindow);
		STDMETHODIMP ShowContextMenu(
				DWORD dwID, 
				POINT * pptPosition,
				IUnknown* pCommandTarget,
				IDispatch * pDispatchObjectHit);
		STDMETHODIMP TranslateAccelerator(
            /* [in] */ LPMSG lpMsg,
            /* [in] */ const GUID __RPC_FAR *pguidCmdGroup,
            /* [in] */ DWORD nCmdID);
		STDMETHODIMP GetOptionKeyPath(BSTR* pbstrKey, DWORD dw);
		STDMETHODIMP GetDropTarget( 
            /* [in] */ IDropTarget __RPC_FAR *pDropTarget,
            /* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);

		STDMETHODIMP GetExternal( 
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
        
        STDMETHODIMP TranslateUrl( 
            /* [in] */ DWORD dwTranslate,
            /* [in] */ OLECHAR __RPC_FAR *pchURLIn,
            /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut);
        
        STDMETHODIMP FilterDataObject( 
            /* [in] */ IDataObject __RPC_FAR *pDO,
            /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet);


};

typedef CImpIDocHostUIHandler* PCImpIDocHostUIHandler;



class CImpIDocHostShowUI : public IDocHostShowUI
{
    protected:
        ULONG               m_cRef;
        class CSite        *m_pSite;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIDocHostShowUI(class CSite *, IUnknown *);
        ~CImpIDocHostShowUI(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		STDMETHODIMP ShowMessage(
								HWND hwnd,
								LPOLESTR lpstrText,
								LPOLESTR lpstrCaption, 
								DWORD dwType,
								LPOLESTR lpstrHelpFile,
								DWORD dwHelpContext,
								LRESULT * plResult);
		STDMETHODIMP ShowHelp(
								HWND hwnd,
								LPOLESTR pszHelpFile,
								UINT uCommand,
								DWORD dwData,
								POINT ptMouse,
								IDispatch * pDispatchObjectHit);
};

typedef CImpIDocHostShowUI* PCImpIDocHostShowUI;


/*
 * The CSite class, a COM object with the interfaces IOleClientSite,
 * IAdviseSink, IOleInPlaceSite, and IOleDocumentSite.
 */


class CSite : public IPropertyNotifySink
{
	
    private:
        ULONG           m_cRef;
        HWND            m_hWnd; //Client area window of parent
		DWORD			m_dwUnadvise;
		IHTMLDocument2*	m_pDoc2;
		LONG			m_lReadyState;
		DWORD			m_dwCookie;

        class CTridentView*		m_pTridentView;
        
		//Object interfaces
        LPUNKNOWN          m_pObj;
        LPOLEOBJECT        m_pIOleObject;
        LPOLEINPLACEOBJECT m_pIOleIPObject;
        LPOLEDOCUMENTVIEW  m_pIOleDocView;
		LPOLECOMMANDTARGET m_pIOleCommandTarget;
		LPCONNECTIONPOINT  m_pCP;

        //Our interfaces
        PCImpIOleClientSite   m_pImpIOleClientSite;
        PCImpIAdviseSink      m_pImpIAdviseSink;
        PCImpIOleInPlaceSite  m_pImpIOleIPSite;
        PCImpIOleDocumentSite m_pImpIOleDocumentSite;
        PCImpIDocHostUIHandler m_pImpIDocHostUIHandler;
        PCImpIDocHostShowUI		m_pImpIDocHostShowUI;
		CImpIDispatch*			m_pImpIDispatchAmbients;


    protected:

    public:
        CSite( HWND, CTridentView* );
        ~CSite(void);

        //Gotta have an IUnknown for delegation
        STDMETHODIMP QueryInterface(REFIID, void** );
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);


		// Other functions
        INT         Create();
        void        Close(BOOL);
        void        Activate(LONG);
        void        UpdateObjectRects(void);
		HRESULT		Load(TCHAR *pchPath);
		HRESULT		Save(TCHAR *pchPath);
        INT			ObjectInitialize( TCHAR* pchPath );
		BOOL		IsEditing();


		// helper functions
		CWnd*	GetFrame( void );
		IHTMLDocument2* GetDoc2();
		IHTMLBodyElement* GetBody();
		LPUNKNOWN GetObjectUnknown( void ){ return m_pObj; } 
		PCImpIOleInPlaceSite GetIPSite( void ){return m_pImpIOleIPSite; }
		HWND GetWindow( void ){ return m_hWnd; }
		LPOLECOMMANDTARGET GetCommandTarget( void ){return m_pIOleCommandTarget;}
		void SetCommandTarget( LPOLECOMMANDTARGET pTarget ){ m_pIOleCommandTarget = pTarget; }
		void SetDocView( LPOLEDOCUMENTVIEW pDocView ){ m_pIOleDocView = pDocView; }
		
		void SetIPObject( LPOLEINPLACEOBJECT pIPObject ){ m_pIOleIPObject = pIPObject; }
		LPOLEINPLACEOBJECT GetIPObject( void ){ return m_pIOleIPObject; }

		// IPropertyNotifySink methods
		STDMETHODIMP	OnChanged(DISPID dispID);
		STDMETHODIMP	OnRequestEdit(DISPID dispID);



};


typedef CSite* PCSite;



//DeleteInterfaceImp calls 'delete' and NULLs the pointer
#define DeleteInterfaceImp(p)\
{\
            if (NULL!=p)\
            {\
                delete p;\
                p=NULL;\
            }\
}


//ReleaseInterface calls 'Release' and NULLs the pointer
#define ReleaseInterface(p)\
{\
            IUnknown *pt=(IUnknown *)p;\
            p=NULL;\
            if (NULL!=pt)\
                pt->Release();\
}

// message sent when the doc is completely loaded
extern UINT wmReadyStateComplete;

#endif //_SITE_H_
