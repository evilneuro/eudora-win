///////////////////////////////////////////////////////////////////////////////
// DSViewNewIface.h
//
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORYSERVICES_VIEWNEW_IFACE_H_
#define __DIRECTORYSERVICES_VIEWNEW_IFACE_H_

#include "3dformv.h"

class DSPrintEditView;

// Minimal document for printing. Added because latest MFC made CDocument abstract.
class CDSPrintDoc : public CDocument
{
  public:
				CDSPrintDoc() : CDocument() {}
				~CDSPrintDoc() {}
};

class DirectoryServicesViewNew : public C3DFormView {

    DECLARE_DYNCREATE(DirectoryServicesViewNew)

public:
                  DirectoryServicesViewNew();
    virtual      ~DirectoryServicesViewNew();
    virtual BOOL  Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
			  DWORD dwRequestedStyle, const RECT& rect,
			  CWnd *pParentWnd, UINT nID,
			  CCreateContext *pContext = NULL)
    {
	return(C3DFormView::Create(lpszClassName, lpszWindowName,
				   dwRequestedStyle, rect, pParentWnd, nID,
				   pContext));
    }
    virtual BOOL  OnCmdMsg(UINT nID, int nCode, void *pExtra,
			    AFX_CMDHANDLERINFO *pHandlerInfo)
    {
 	return(C3DFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo));
    }

    virtual BOOL  PreTranslateMessage(MSG *pMsg);

            BOOL  InitializeSplitterWindow();

            void  PrepareForDestruction();
            bool  GetSelectedAddresses(CStringList& addressList);
            bool  IsQueryInProgress();
            void  QueryStart();
            void  GiveQueryInputFocus();
            void  CheckClosePrintPreview();
            void  SetPrintState(bool);
            bool  IsPrintPreviewMode();
            void  ResetResize();
            void  DoQuery(LPCTSTR QueryString);
       CDSPrintDoc  doc;

private:

    DSMainSplitterWnd          m_SW_Main;
    const  int                 XBaseUnit;
    const  int                 YBaseUnit;
           bool                panesCreated;
           int                 xSpace;
           int                 ySpace;
           bool                printing_or_previewing;

    DSPrintEditView              *m_PrintPreView;
    DSPrintEditView              *PreparePrint();

protected:

            void  DoDataExchange(CDataExchange *pDX);
            void  OnInitialUpdate();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg int   OnCreate(LPCREATESTRUCT);
    afx_msg void  OnSize(UINT, int, int);
    afx_msg void  OnUpdateMakeNickname(CCmdUI* pCmdUI);
    afx_msg void  OnMakeNickname();
    afx_msg void  OnNextPane();
    afx_msg long  OnActivateWazoo(WPARAM wParam, LPARAM lParam);
    afx_msg void  OnUpdateNeedActiveFocus(CCmdUI* pCmdUI);
    afx_msg void  OnFilePrint();
    afx_msg void  OnFilePrintOne();
    afx_msg void  OnFilePrintPreview();

    DECLARE_MESSAGE_MAP()
};

#endif // __DIRECTORYSERVICES_VIEWNEW_IFACE_H_



