///////////////////////////////////////////////////////////////////////////////
// DSGenericRichEditCtrlIface.h
//
// Created: 10/26/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_GENERIC_RICHEDITCTRL_IFACE_H_
#define __DS_GENERIC_RICHEDITCTRL_IFACE_H_

// Used to derive from CRichEditCtrl, but now derive from CURLEdit to
// get URL highlighting benefits.
class DSGenericRichEditCtrl : public CURLEdit {
    DECLARE_DYNCREATE(DSGenericRichEditCtrl)

public:
                   DSGenericRichEditCtrl(){}
    virtual       ~DSGenericRichEditCtrl(){}
    virtual BOOL   PreTranslateMessage(MSG *pMsg);

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};

#endif // __DS_GENERIC_RICHEDITCTRL_IFACE_H_


