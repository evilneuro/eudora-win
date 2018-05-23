// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	SECMaskEdit declaration
//
//
//  CHANGELOG:
//
//     8/21/95    MSW  Started, new logic, new ui.
//     12/9/95    MSW  Added cut/copy/paste/insert mode for B5.
//
//
//
#ifndef __SECMASKEDIT_H__
#define __SECMASKEDIT_H__

//
// SEC Extension DLL
// Initialize declaration context
//

#ifdef _SECDLL
	#undef AFXAPP_DATA
	#define AFXAPP_DATA AFXAPI_DATA
	#undef AFX_DATA
	#define AFX_DATA    SEC_DATAEXT
#endif //_SECDLL

#define LITERAL_SENTINEL    128

//forward declaration
class SECMaskEdit;

//SECMaskEdit DDX/DDV routines

//medit - A SECMaskEdit control -we need this instead of an ID.
//        You can call GetDlgItem if you must
//value - the value to set or retrieve without literals
//raw_value - the value used in retrieving information only for getting the raw string.

void AFXAPI DDX_Mask(CDataExchange * pDX, SECMaskEdit &medit, CString &value,
CString &raw_value);



class SECMaskEdit : public CEdit
{
    DECLARE_DYNAMIC(SECMaskEdit);
    
// Construction
public:     
    SECMaskEdit();
    
    BOOL AttachEdit(int nCtlID, CWnd* pParentWnd);

// Attributes
public:    
    //set the input mask for the control
    BOOL SetMask(LPCTSTR lpszMask);
    
    void SetPromptChar(TCHAR chNewPrompt);
    TCHAR GetPromptChar() {return m_chPrompt;}
    
    //sets the data in the edit
    BOOL SetData(const CString& strData, int nStartPos = 0, BOOL bInsert = FALSE,
		 BOOL bIgnorePromptChars = TRUE);
    
    void GetData(CString& strData, BOOL bTrim = FALSE);
    CString& GetRawData(void) {return m_strDisplay;}

    //Is this mask in insert mode?
    BOOL InsertMode() {return m_bInsert;}
    
// Operations
public:
    //clears all data from the control
    void ClearData(void);

	// Undo operations
	// Since the CEdit::Undo functions are not virtual,
	// these are new implementations.
	virtual BOOL CanUndo();
	virtual BOOL Undo();	
	virtual void EmptyUndoBuffer();
    
// Overrideables

// Implementation
    
public:
    virtual ~SECMaskEdit();
#ifdef _DEBUG
    BOOL m_bIsAttached;
    virtual void Dump(CDumpContext& dc) const;
    virtual void AssertValid() const;
#endif
    
protected:
// Implementation - Character checking utility functions
    virtual BOOL IsCharValid(int nPos, TCHAR& ch);
    virtual BOOL IsLiteralChar(TCHAR ch);
    virtual BOOL IsMaskChar(TCHAR ch);
    
// Implementation - Mask functions    
    virtual void UpdateDisplayStr();
	virtual void ConvertMaskToDisplayStr(CString strMask,CString& strResult);
    virtual void ParseMaskString();
    virtual int  FindLastFilledDataPos(int nStartPos);
    
// Implementation - edit manipulation...    
    virtual BOOL PushChar(int nStartPos, TCHAR ch, BOOL bRefreshDisplay = TRUE);
    virtual void PullChar(int nFromPos, int nToPos);
    virtual int SetCursorPos(int nOldPos, int nDirection, BOOL bSelect = FALSE);
    virtual int GetNextDataPos(int nOldPos);
    virtual int GetPrevDataPos(int nOldPos);

// Implementation - redrawing
    virtual void UpdateDisplay();
        
protected:
// Implementation, data members    
    CString m_strMask;        // Holds the mask, set by SetMask
    CString m_strDisplay;     // Holds the string that gets displayed
    TCHAR m_chPrompt;          // The prompt character, space is default.
    BOOL m_bKeyProcessed;     // Used by OnKeyDown to signal OnChar that
                              // a key was handled so we don't handle twice.

    BOOL m_bInsert; //Is the edit in insert mode?? default - overwrite.
	CString m_strUndoText;
	CString m_strRedoText;

// Implementation - Message map and entries    
    DECLARE_MESSAGE_MAP();
    
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg LRESULT OnCopy(WPARAM, LPARAM);
    afx_msg LRESULT OnPaste(WPARAM, LPARAM);
    afx_msg LRESULT OnCut(WPARAM,LPARAM);
    afx_msg LRESULT OnClear(WPARAM,LPARAM);
	
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __SECMASKEDIT_H__
