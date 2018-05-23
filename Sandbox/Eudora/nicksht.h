// NICKSHT.H
//
// Property sheet for right-hand view of Address Book window.
//

class CNicknamesPropSheet : public CPropertySheet
{
public:
	DECLARE_DYNAMIC(CNicknamesPropSheet)
	CNicknamesPropSheet(CWnd* pWndParent = NULL);
	~CNicknamesPropSheet();

	BOOL LoadPages();

	CPropertyPage* GetNicknamePage(int nPage) const;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	//
	// External interface to private custom property pages.
	//
	BOOL InitCustomPages();
	void RegisterCustomFields();

	void PopulateCustomEditControls(CNickname* pNickname);
	BOOL SaveModifiedCustomEditControls(CNickname* pNickname);

	//
	// Since the CPropertySheet caches the initial sizes of the
	// property pages when they are first created, we run into
	// problems with resizing the pages later.  Therefore, the view
	// containing this property sheet must cache the current property
	// page size everytime the view is resized.  This size is actually
	// used whenever the user *switches* to a different property page
	// held by this sheet.  See the OnResizePage() handler.
	//
	CRect m_PageRect;	// cached client rect for resized pages

	//
	// FORNOW, these should be made private.
	//
	CNicknamesAddressPage	m_addressPage;
	CNicknamesNotesPage		m_notesPage;


// Overridables
protected:
	virtual BOOL OnInitDialog();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// Operations
protected:
	//{{AFX_MSG(CNicknamesPropSheet)
	afx_msg LONG OnResizePage(UINT, LONG);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Attributes
private:
	CObList		m_customPageList;

	//
	// UGLY, UGLY, UGLY.  Override of a non-virtual function in our
	// base class.  We stick this in the private section in order to
	// hide the (buggy) public CPropertySheet::GetPage()
	// implementation in the base class.  This is a workaround for an
	// ugly release mode compiler bug (at least we think it is a
	// compiler bug) discovered in MSVC 5.0 on 8/26/97 by Jeff
	// Beckley.
	//
	// Users who need this functionality should call GetNicknamePage() 
	// above.
	//
	CPropertyPage* GetPage(int nPage) const
		{ VERIFY(0); return NULL; }
};
