///////////////////////////////////////////////////////////////////////////////
// DSGenericListCtrlIface.h
//
// Created: 10/26/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_GENERIC_LIST_CTRL_IFACE_H_
#define __DS_GENERIC_LIST_CTRL_IFACE_H_

class DSGenericListCtrl : public CListCtrl {
    DECLARE_DYNCREATE(DSGenericListCtrl)

public:
                   DSGenericListCtrl(){}
    virtual       ~DSGenericListCtrl(){}
    virtual BOOL   PreTranslateMessage(MSG *pMsg);

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#endif // __DS_GENERIC_LIST_CTRL_IFACE_H_


