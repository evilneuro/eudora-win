//////////////////////////////////////////////////////////////////////////////
// DSResultsTocImpl.cpp
// 
// Created: 10/25/97 smohanty
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define __DS_RESULTS_TOC_INTERFACE_IMPLEMENTATION_
#include "DSResultsTocImpl.h"

#include "DebugNewHelpers.h"


IMPLEMENT_DYNCREATE(DSResultsToc, DSGenericListCtrl)

BEGIN_MESSAGE_MAP(DSResultsToc, DSGenericListCtrl)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


DSResultsToc::DSResultsToc()
    : _capt(false), _clickedOnMe(false),
      _anItemSelected(false), _canDragNDrop(false),
      _gData(NULL), _clipFormat(0)
{
    _clipFormat	= static_cast<CLIPFORMAT>
	(
	    ::RegisterClipboardFormat("EudoraDS-ResultsTocData-4.2")
	);
}

void
DSResultsToc::OnLButtonDown(UINT nFlags, CPoint pt)
{
    DSGenericListCtrl::OnLButtonDown(nFlags, pt);
    OnResultsTocClk();

    if (_anItemSelected == true) {

	_canDragNDrop = false;
	// Drain the string.
	dataStr.Empty();

	// Get the long results.
	(*(dsbm.grl))(dataStr, dsbm.pv);
	COleDataSource dataSource;
	int len = dataStr.GetLength();
	if (len > 0) {
	    ExportData(dataSource, _clipFormat);
	    ExportData(dataSource, CF_TEXT);
	}

	if (_canDragNDrop == true) {
	    dataSource.DoDragDrop(DROPEFFECT_COPY);
	    dataSource.Empty();
	}
    }	
}

void
DSResultsToc::ExportData(COleDataSource& dataSource, CLIPFORMAT cf)
{
    int len = dataStr.GetLength();
    ASSERT(len > 0);

    if (len > 0) {
	len++;
	HGLOBAL gData = ::GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, len);
	void *pVoid   = static_cast<void *>(::GlobalLock(gData));

	if (pVoid != 0) {
	    memcpy(pVoid, dataStr, len);
	    dataSource.CacheGlobalData(cf, gData);
	    _canDragNDrop = true;
	    ::GlobalUnlock(gData);
	}
	else {
	    ::GlobalFree(gData);
	}
    }
}
	

void
DSResultsToc::OnLButtonUp(UINT nFlags, CPoint pt)
{
    DSGenericListCtrl::OnLButtonUp(nFlags, pt);
}

void
DSResultsToc::OnMouseMove(UINT nFlags, CPoint pt)
{
    DSGenericListCtrl::OnMouseMove(nFlags, pt);
}

void
DSResultsToc::InitResultsTOC(DSBM *pDSBM)
{
    dsbm.pv    =  pDSBM->pv;
    dsbm.rlr   =  pDSBM->rlr;
    dsbm.aslr  =  pDSBM->aslr;
    dsbm.aslrf =  pDSBM->aslrf;
    dsbm.asi   =  pDSBM->asi;
    dsbm.htfrt =  pDSBM->htfrt;
    dsbm.qpqip =  pDSBM->qpqip;
    dsbm.asnnl =  pDSBM->asnnl;
    dsbm.grl   =  pDSBM->grl;
}

void
DSResultsToc::ResetRecordList()
{
    pRecCur = pRecHead;
    while (pRecCur) {
	PRecNode pRecTmp = pRecCur;
	pRecCur          = pRecCur->next;
	delete pRecTmp;
    }
    pRecHead = 0;
    pRecCur  = 0;
}

void
//DSResultsToc::OnResultsTocClk(NMHDR *pNMHDR, LRESULT *pResult)
DSResultsToc::OnResultsTocClk()
{
    // clear the contents...
    (*(dsbm.rlr))(dsbm.pv);
    if (GetSelectedCount() != 0) {
	int idx  = -1;
	ResetRecordList();
	while((idx = GetNextItem(idx, LVNI_SELECTED))
	      != -1) {
	    IDSPRecord *pRec = (IDSPRecord *) GetListItemLPARAM(idx);
	    // Create the linked-list of pRecords.
	    if (pRecHead == 0) {
		pRecHead       = DEBUG_NEW RecNode;
		pRecHead->data = pRec;
		pRecHead->next = 0;
		pRecCur        = pRecHead;
	    }
	    else {
		pRecCur->next = DEBUG_NEW RecNode;
		pRecCur       = pRecCur->next;
		pRecCur->data = pRec;
		pRecCur->next = 0;
	    }
	}
	// Splat the details.
	(*(dsbm.aslrf))((void *)pRecHead, dsbm.pv);
	ResetRecordList();
    }
    // Apprise the parent that items have changed.
    AppriseChangeView();

//    *pResult = 0;
}

void
DSResultsToc::OnKeyUp(UINT nChar, UINT, UINT)
{
    if (nChar == VK_UP || nChar == VK_DOWN || nChar == VK_HOME ||
	nChar == VK_END || nChar == VK_PRIOR || nChar == VK_NEXT) {
	// clear the contents...
	(*(dsbm.rlr))(dsbm.pv);
	if (GetSelectedCount() != 0) {
	    int idx  = -1;
	    ResetRecordList();
	    while((idx = GetNextItem(idx, LVNI_SELECTED))
		  != -1) {
		IDSPRecord *pRec = (IDSPRecord *) GetListItemLPARAM(idx);

		// Create the linked-list of pRecords.
		if (pRecHead == 0) {
		    pRecHead       = DEBUG_NEW RecNode;
		    pRecHead->data = pRec;
		    pRecHead->next = 0;
		    pRecCur        = pRecHead;
		}
		else {
		    pRecCur->next = DEBUG_NEW RecNode;
		    pRecCur       = pRecCur->next;
		    pRecCur->data = pRec;
		    pRecCur->next = 0;
		}
	    }
	    // Splat the details.
	    (*(dsbm.aslrf))((void *)pRecHead, dsbm.pv);
	    ResetRecordList();
	}
	AppriseChangeView();
    }
}

void
DSResultsToc::AppriseChangeView(void)
{
    // Apprise the DS View that an item has changed.
    (*(dsbm.asi))(0, dsbm.pv);
    if (GetNextItem(-1, LVNI_SELECTED) != -1) {
	_anItemSelected = true;
    }
    else {
	_anItemSelected = false;
    }
}

void
DSResultsToc::SplatContents(IDSPRecord *pRecord, void *data)
{
    pRecHead = DEBUG_NEW RecNode;
    pRecHead->data = pRecord;
    pRecHead->next = 0;
    (*(dsbm.aslrf))((void *)pRecHead, data);
    ResetRecordList();
    AppriseChangeView();
}    

LPARAM
DSResultsToc::GetListItemLPARAM(int nItem)
{
    LV_ITEM item;

    memset(&item, 0, sizeof(LV_ITEM));

    item.mask = LVIF_PARAM;
    item.iItem = nItem;
    if (GetItem(&item) == TRUE) {
	return(item.lParam);
    }
    return(0);
}
