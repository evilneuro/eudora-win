///////////////////////////////////////////////////////////////////////////////
// DMMDumpLogDlg.h
//
// Created: 11/18/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DMM_DUMP_LOG_IFACE_H_
#define __DMM_DUMP_LOG_IFACE_H_

#ifdef _DEBUG


//#include "DMMDumpHandlerIface.h"

//typedef flag (*DBLCB)(void *pv, c_char * const buf, pDBF_OutType, void *);

class DMMDumpLogDialog : public CDialog {
    DECLARE_DYNCREATE(DMMDumpLogDialog)
public:
    DMMDumpLogDialog(CWnd *pParentWnd = NULL)
	: CDialog(IDD_DMM_DUMPLOG, pParentWnd), m_Size(0)
    {}
    
    virtual BOOL OnInitDialog();

	CString GetLabel(){ return m_strLabel;}
	CString GetFile(){ return m_strFile;}
	int GetMinSize(){ return m_Size;}

private:

    CEdit   m_EB_Label;
    CEdit   m_EB_File;
    CEdit	m_EB_Size;
	CString m_strLabel;
    CString m_strFile;
	int m_Size;
	//DBLCB   pDBLCB;
    //DMM_DH bo;

protected:
            void DoDataExchange(CDataExchange *pDX);

    virtual void OnOK();
    virtual void OnCancel();
    DECLARE_MESSAGE_MAP()
};
#endif // _DEBUG

#endif // __DMM_DUMP_LOG_IFACE_H_



