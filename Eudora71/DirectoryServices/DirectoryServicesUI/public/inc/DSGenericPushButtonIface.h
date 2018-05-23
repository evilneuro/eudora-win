///////////////////////////////////////////////////////////////////////////////
// DSGenericPushButtonIface.h
//
// Created: 10/26/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_GENERIC_PUSHBUTTON_IFACE_H_
#define __DS_GENERIC_PUSHBUTTON_IFACE_H_

class DSGenericPushButton : public CButton {
    DECLARE_DYNCREATE(DSGenericPushButton)

public:
                   DSGenericPushButton(){}
    virtual       ~DSGenericPushButton(){}
    virtual BOOL   PreTranslateMessage(MSG *pMsg);

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#endif // __DS_GENERIC_PUSHBUTTON_IFACE_H_


