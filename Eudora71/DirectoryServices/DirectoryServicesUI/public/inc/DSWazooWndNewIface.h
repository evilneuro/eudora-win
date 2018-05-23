///////////////////////////////////////////////////////////////////////////////
// DSWazooWndNewIface.h
//
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORYSERVICES_WAZOOWNDNEW_IFACE_H_
#define __DIRECTORYSERVICES_WAZOOWNDNEW_IFACE_H_

#include "WazooWnd.h"

/////////////////////////////////////////////////////////////////////////////
// DirectoryServicesWazooWndNew window

class   DirectoryServicesViewNew;
typedef DirectoryServicesViewNew  DSVN;

class DirectoryServicesWazooWndNew : public CWazooWnd {

    DECLARE_DYNCREATE(DirectoryServicesWazooWndNew)

public:
                   DirectoryServicesWazooWndNew();
    virtual       ~DirectoryServicesWazooWndNew();
    virtual BOOL   OnCmdMsg(UINT, int, void *, AFX_CMDHANDLERINFO *);
    virtual BOOL   DestroyWindow();
            bool   GetSelectedAddresses(CStringList& addressList);
    virtual void   OnActivateWazoo();
            void   CheckClosePrintPreview();
            bool   IsPrintPreviewMode();
            void   ResetResize();
            void   DoQuery(LPCTSTR QueryString);
            void   GiveQueryInputFocus();

private:

    DSVN          *m_pDirectoryServicesViewNew;
    HCURSOR        m_hArrow;
    HCURSOR        m_hArrowHour;

protected:

            BOOL   CreateView();
    afx_msg void   OnSize(UINT, int, int);
    afx_msg BOOL   OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT msg);

    DECLARE_MESSAGE_MAP()
};

#endif // __DIRECTORYSERVICES_WAZOOWNDNEW_IFACE_H_
