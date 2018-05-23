// SearchDoc.h : header file
//

#ifndef	__SEARCHDOC_H__
#define	__SEARCHDOC_H__

#include "Doc.h"

class	CSearchView;
class	CMDIChild;
class	CSummary;
class	CTocDoc;

/////////////////////////////////////////////////////////////////////////////
// CSearchDoc document

class CSearchDoc : public CDoc
{
	DECLARE_DYNCREATE(CSearchDoc)
public:
	virtual 	~CSearchDoc();

protected:
	//	Member functions
	CSearchDoc();			// protected constructor used by dynamic creation

	CSearchView *						GetSearchView();

	virtual	BOOL 	OnNewDocument();
	virtual	void	OnCloseDocument();
	virtual BOOL	OnSaveDocument(const char * pszPathName);

	afx_msg void						OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void						OnFileSave();
	afx_msg void						OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void						OnFileSaveAs();

	//{{AFX_MSG(CSearchDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()  
	
private:           
};

extern	CSearchDoc	*gSearchDoc;

#endif	// __SEARCHDOC_H__
