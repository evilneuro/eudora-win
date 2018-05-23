///////////////////////////////////////////////////////////////////////////////
// DSLeftViewIface.h
//
// Created: 10/09/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_LEFT_VIEW_IFACE_H_
#define __DS_LEFT_VIEW_IFACE_H_

#include "3dformv.h"
#include "DirectoryServicesBaseTypes.h"
#include "DSBounceMessageImpl.h"
#include "imgcache.h"
#include "DSGenericPushButtonIface.h"

typedef _directory_services _DS;

class AutoCompList;
class AutoCompleterListBox;

enum _DS_QUERY_STATE {
    _STARTED = 1,
    _STOPPED  = 2
};

enum _DS_DATA_STATE {
    _DATA_BEGIN = 1,
    _DATA_END   = 2
};

class DSLeftView : public C3DFormView {

    DECLARE_DYNCREATE(DSLeftView)

public:

                   DSLeftView();
    virtual       ~DSLeftView();

    virtual BOOL   Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
			  DWORD dwRequestedStyle, const RECT& rect,
			  CWnd *pParentWnd, UINT nID,
			  CCreateContext *pContext = NULL);

    virtual BOOL   OnCmdMsg(UINT nID, int nCode, void *pExtra,
			    AFX_CMDHANDLERINFO *pHandlerInfo)
    {

	return(C3DFormView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo));
    }

            void   PrepareForDestruction();
            BOOL   OnCreateClient();
            bool   IsQueryInProgress() { return(queryState == _STARTED); }
            bool   IsQueryInputEmpty();
            void   GetQueryInput(CString& wndStr);
	    void   EnableStartStop(BOOL);
            void   QuitQuery();
            void   StartQuery();
            void   QueryStart();
            void   ClearContents();
            void   InitializeExportToBuffer(void *pRecord);
            void   InitializeExportToBufferFormatted(void *data);
            void   ResultsTocItemChanged(int nItem);
    QUERY_STATUS   GetQueryLastError();
            void   SetDataState(_DS_DATA_STATE _dsDataState);
            void   WaitForRecord();
            void   GetTheData();
            void   Error(char *s);
            void   Error( QUERY_STATUS nQueryStatus );
    QUERY_STATUS   GetQueryRecord(IDSPRecord **ppRecord);
            void   AddStringNoNewLine(char *s);
            void   AddHit(IDSPRecord *pRecord);
            void   ProcessTabFromQueryInput();
            BOOL   IsAnyDatabaseChecked();

            UINT   GetResultsTocSelectedCount();
            int    GetResultsTocItemCount();
            UINT   GetResultsTocItemState(int i, UINT state);
         CString   GetResultsTocItemText(int item, int subitem);
            bool   GetSelectedAddresses(CStringList& addressList);
            void   AddHitCallback();
            void   SetQueryInputFocus() { m_EB_QueryInput.SetFocus();}
            void   ProcessTabFromResultsToc(BOOL shiftDown);
            void   ProcessTabFromResultsLong(BOOL shiftDown);
            void   ProcessTabIntoResultsToc();
            void   ActivateQueryInput();
            bool   GetResultsTocEmailList(int item, CStringList& curEmailList);
            void   GetResultsLong(CString& details);
            void   ReflectQueryInTitle(const char *query);
         CString&  GetLastQuery() { return(lastQuery); }
            void   GiveQueryInputFocus() { m_EB_QueryInput.SetFocus(); }
            bool   IsPrintable();
			void   FinishQueryAutoComplete();
private:

    CStatic        m_ST_QueryLabel;
    DSEdit         m_EB_QueryInput;
//	BigOleComboBox m_EB_QueryCombo;
    DSGenericPushButton        m_PB_QueryStartStop;
    CStatic        m_ST_QueryResults;
    DSGenericPushButton        m_PB_MailTo;
    DSGenericPushButton        m_PB_MailCc;
    DSGenericPushButton        m_PB_MailBcc;
    DSGenericPushButton        m_CB_KeepOnTop;
    DSSplitterWnd  m_SW_Results;
    CImgCache     *imgCache;
	DSQueryBtn					m_QueryBtn;

            _DS   *pDS;
    const   int    XBaseUnit;
    const   int    YBaseUnit;
            int    xSpace;
            int    ySpace;
            int    pushButtonWidth;
            int    pushButtonHeight;
            int    textHeight;
            int    queryLabelWidth;
            int    keepOnTopWidth;
            int    keepOnTopHeight;
            int    minQueryRowWidth;
            int    minWidth;
            int    minHeight;
            long   theHits;
           _DS_QUERY_STATE     queryState;
           _DS_DATA_STATE      dataState;
           DSBM                dsbm;
           DSMail              dsMail;
           CString             lastQuery;
           bool                resultsSWPanesCreated;
           int                 nCurWidth;

            void   SmartSize(int cx, bool xIsMin, int cy, bool yIsMin);
            void   ResetQueryState(_DS_QUERY_STATE _dsqst);
            void   ResetResultsToc();
            void   OnAddress(UINT HeaderNum);
            void   DisplayHits();
         LRESULT   ProcessTab(UINT, WPARAM, LPARAM);
            bool   ProcessShortcut(WPARAM);
            BOOL   SerializeSerializables();

protected:

//	virtual BOOL   PreTranslateMessage(MSG* pMsg);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual BOOL   PreCreateWindow(CREATESTRUCT& cs);        
            void   DoDataExchange(CDataExchange *pDX);
            void   OnInitialUpdate();
    afx_msg void   OnQueryInputUpdate();
    afx_msg void   OnSize(UINT, int, int);
    afx_msg void   OnDestroy();
    afx_msg void   OnQueryInputChange();
    afx_msg void   OnQueryStartStop();
    afx_msg void   OnMailTo();
    afx_msg void   OnMailCc();
    afx_msg void   OnMailBcc();
	afx_msg void   OnQueryDropDown();

    DECLARE_MESSAGE_MAP()
};

#endif // __DS_LEFT_VIEW_IFACE_H_


