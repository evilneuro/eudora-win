///////////////////////////////////////////////////////////////////////////////
// DSSplitterWndIface.h
//
// Created: 09/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_SPLITTER_WND_IFACE_H_
#define __DS_SPLITTER_WND_IFACE_H_

class DSSplitterWnd : public CSplitterWnd
{
    DECLARE_DYNCREATE(DSSplitterWnd)

public:
                   DSSplitterWnd();
    virtual       ~DSSplitterWnd();
            int    SplitterAndBorderHeight();

private:

            int        nCurWidth;
            BOOL       SerializeSerializables();
            void       PrepareForDestruction();

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg void   OnSize(UINT, int, int);
    afx_msg void   OnDestroy();
    DECLARE_MESSAGE_MAP()
};

#endif // __DS_LIST_CTRL_IFACE_H_

