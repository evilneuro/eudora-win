///////////////////////////////////////////////////////////////////////////////
// DSMailIface.h
//
// Created: 09/24/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_MAIL_IFACE_H_
#define __DS_MAIL_IFACE_H_


class DSMail {

public:

     DSMail() {}
    ~DSMail() {}

    void OnAddress(UINT HeaderNum, CString& strMailees, CWnd *pParent,
		   CFrameWnd *pParentFrame, BOOL bKeepOnTop);
    void CreateAddress(CString&, CString&, CString&);

private:

};

#endif // __DS_MAIL_IFACE_H_


