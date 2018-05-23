///////////////////////////////////////////////////////////////////////////////
// DSResultsTocIface.h
//
// Created: 12/15/98 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_RESULTS_TOC_IFACE_H_
#define __DS_RESULTS_TOC_IFACE_H_

#include "dsapi.h"
#include "DSBounceMessageImpl.h"
#include "DSGenericListCtrlIface.h"

class DSResultsToc : public DSGenericListCtrl {
    DECLARE_DYNCREATE(DSResultsToc)

public:
                     DSResultsToc();
    virtual         ~DSResultsToc(){}

    void             InitResultsTOC(_ds_bounce_message *pDSBounceMessage);
    void             SplatContents(IDSPRecord *pRecord, void *data);
    LPARAM           GetListItemLPARAM(int nItem);
    void             AppriseChangeView(void);

    void             OnResultsTocClk();
    void             OnKeyUp(UINT, UINT, UINT);

private:

    DSBM             dsbm;
    bool             _capt;
    bool             _clickedOnMe;
    bool             _anItemSelected;
    bool             _canDragNDrop;
    HGLOBAL          _gData;
    CLIPFORMAT       _clipFormat;
    CString          dataStr;

    void             ResetRecordList();
    void             ExportData(COleDataSource &, CLIPFORMAT);

protected:
    afx_msg  void    OnMouseMove(UINT, CPoint);
    afx_msg  void    OnLButtonDown(UINT, CPoint);
    afx_msg  void    OnLButtonUp(UINT, CPoint);

    DECLARE_MESSAGE_MAP()
};

#endif // __DS_RESULTS_TOC_IFACE_H_


