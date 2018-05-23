///////////////////////////////////////////////////////////////////////////////
// DSMainSplitterWndIface.h
//
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_MAIN_SPLITTER_WND_IFACE_H_
#define __DS_MAIN_SPLITTER_WND_IFACE_H_

class CImgCache;

class DSMainSplitterWnd : public CSplitterWnd {

    DECLARE_DYNCREATE(DSMainSplitterWnd)

public:

                       DSMainSplitterWnd();
    virtual           ~DSMainSplitterWnd();

            int        SplitterAndBorderWidth();
           void        ResetResize();

private:

            int        nCurHeight;
            BOOL       SerializeSerializables();
            void       PrepareForDestruction();
protected:

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg void   OnDestroy();
    afx_msg void  OnSize(UINT, int, int);

    DECLARE_MESSAGE_MAP()
};

#endif // __DS_MAIN_SPLITTER_WND_IFACE_H_

