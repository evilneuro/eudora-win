///////////////////////////////////////////////////////////////////////////////
// DSRightViewIface.h
//
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_RIGHT_VIEW_IFACE_H_
#define __DS_RIGHT_VIEW_IFACE_H_

#include "3dformv.h"
#include "DirectoryServicesBaseTypes.h"
#include "imgcache.h"
#include "DSGenericListCtrlIface.h"
#include "DSGenericPushButtonIface.h"
#include "QICommandClient.h"

typedef _directory_services _DS;

class DSRightView;

struct _DS_RV_Composite {
    DSRightView *pDSRV;
    int          subItem;
};

class DSRightView : public C3DFormView, public QICommandClient {

    DECLARE_DYNCREATE(DSRightView)

public:

                   DSRightView();
    virtual       ~DSRightView();

    virtual BOOL   Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
			  DWORD dwRequestedStyle, const RECT& rect,
			  CWnd *pParentWnd, UINT nID,
			  CCreateContext *pContext = NULL)
    {
	return(C3DFormView::Create(lpszClassName, lpszWindowName,
				   dwRequestedStyle, rect, pParentWnd, nID,
				   pContext));
    }

    virtual BOOL   OnCmdMsg(UINT nID, int nCode, void *pExtra,
			    AFX_CMDHANDLERINFO *pHandlerInfo)
    {

	return(C3DFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo));
    }

	
	virtual void Notify(QCCommandObject*	pCommand,
						COMMAND_ACTION_TYPE	theAction,
						void*				pData); 

    void           PrepareForDestruction();
    void           Error(char *s);
    BOOL           IsAnyDatabaseChecked();
    _DS           *GetDSInterface() { return(pDS); }
    HRESULT        AddDatabases();
    bool           OnlyPhSelected();
    CImgCache     *GetImageCache() { return imgCache; }

private:

    CButton             m_GB_Protocols;
    DSGenericListCtrl   m_LC_Protocols;
    DSGenericPushButton m_PB_NewDatabase;
    CButton             m_GB_Databases;
    DSGenericListCtrl   m_LC_Databases;
    DSGenericPushButton m_PB_ModifyDatabase;
    DSGenericPushButton m_PB_DeleteDatabase;

            _DS    *pDS;
    const   int    XBaseUnit;
    const   int    YBaseUnit;
            int    xSpace;
            int    ySpace;
            int    tcSpace;
            int    pushButtonWidth;
            int    pushButtonWidthEx;
            int    pushButtonHeight;
            int    minProtocolsWidth;
            int    minDatabasesWidth;
            int    minLCProtocolsHeight;
            int    maxProtocolsHeight;
            int    minProtocolsHeight;
            int    minLCDatabasesHeight;
            int    minDatabasesHeight;
            int    minDialogWidth;
            int    minDialogHeight;
            BOOL   shiftDown;
       CImgCache  *imgCache;
            int    nCurWidth;

            void   SmartSize(int cx, bool xIsMin, int cy, bool yIsMin);
            void   InitDatabases();
            void   PopulateDatabases();
            void   ResetDatabases();
            void   InitProtocols();
            void   ResetProtocols();
            void   PopulateProtocols();
            void   SerializeDatabasesState();
            void   ModifyDatabase();
            void   UpdateListFocused(void *pv);
            BOOL   ConfigDatabase(IDSDatabase *pDatabase);
            int    DisplayPropertySheet(HPROPSHEETPAGE *pPages, int iCount);
            void   ProcessTab();
            bool   ProcessShortcut(WPARAM wParam);
            BOOL   SerializeSerializables();

protected:

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    static  int    CALLBACK CompareFuncDatabases(LPARAM, LPARAM, LPARAM);
    static  int    CALLBACK CompareFuncProtocols(LPARAM, LPARAM, LPARAM);
            void   DoDataExchange(CDataExchange *pDX);
            void   OnInitialUpdate();

    afx_msg void   OnSize(UINT, int, int);
    afx_msg void   OnDestroy();
    afx_msg void   OnDatabasesClk(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void   OnDatabasesDblClk(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void   OnProtocolsDblClk(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void   OnDatabasesItemChange(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void   OnModifyDatabase();
    afx_msg void   OnDeleteDatabase();
    afx_msg void   OnNewDatabase();
    afx_msg void   OnColumnClickDatabases(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void   OnColumnClickProtocols(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg long   OnActivateWazoo(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};

#endif // __DS_RIGHT_VIEW_IFACE_H_


