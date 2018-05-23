// 
// Stingray Software Extension Classes
// Copyright (C) 1996 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the  help files for detailed information
// regarding using these classes.
// 
//  Author:		Jan

#ifdef WIN32

#ifndef __TREENODE_H__
#define __TREENODE_H__

// The function names collide with macro names
// from windowsx.h
#undef GetNextSibling
#undef GetPrevSibling
#undef GetFirstChild
#undef GetFirstSibling
#undef GetLastSibling

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

class SECTreeNode 
{
  friend class SECTreeNodeX;

  private:	
	SECTreeNode *m_pParent;
	SECTreeNode *m_pNextSibling;
	SECTreeNode *m_pPrevSibling;
	SECTreeNode *m_pFirstChild;

  public:
	void DeleteChildren(void);
	SECTreeNode() { m_pParent=m_pNextSibling=m_pPrevSibling=m_pFirstChild=NULL; }
	virtual ~SECTreeNode() { DeleteChildren(); }
	
	//Getting nodes
	virtual SECTreeNode *GetParent(void) const { return m_pParent;      }
	virtual SECTreeNode *GetNextSibling(void) const { return m_pNextSibling; }
	virtual SECTreeNode *GetPrevSibling(void) const { return m_pPrevSibling; }
	virtual SECTreeNode *GetFirstChild(void) const { return m_pFirstChild;  }

	SECTreeNode *GetLastChild(void) const;
    SECTreeNode *GetFirstSibling(void) const;
	SECTreeNode *GetLastSibling(void) const;
	SECTreeNode *GetBottomLeftChild(void) const;
	SECTreeNode *GetBottomRightChild(void) const;
    SECTreeNode *GetRoot(void) const;

	//search for a node
	SECTreeNode *GetNextInDisplayOrder(void) const;
	SECTreeNode *GetPrevInDisplayOrder(void) const;
	SECTreeNode *Search( WORD idSearch ) const;
	virtual BOOL OnNextSearchNode( WORD idSearch, SECTreeNode *pNode ) const;

	//setting nodes
	void SetParent(      const SECTreeNode &n);
	void SetNextSibling( const SECTreeNode &n);
	void SetPrevSibling( const SECTreeNode &n);
	void SetFirstChild(  const SECTreeNode &n);
	BOOL AddChild( SECTreeNode *ptnNewChild, SECTreeNode *ptnInsertAfter);

	//major setting nodes
	void DetachFromTree(void);
	void DeleteAllChildren(void);

	//questions
	BOOL HasChildren(void) const { return m_pFirstChild!=NULL; }
	BOOL IsDescendant( SECTreeNode *pNode ) const;
    BOOL IsAncestor( SECTreeNode *pNodePossibleAncestor ) const;
    BOOL IsSibling(    SECTreeNode *pNode ) const;
	WORD GetDistanceFromRoot(void) const;
	virtual BOOL IsExpanded() const;
	virtual void Expand(BOOL bExpand = TRUE);
	void Collapse() { Expand(FALSE); };

	UINT GetNumDescendents(void) const;
};

#define TREENODE_FIRST     (SECTreeNode *)NULL
#define TREENODE_LAST      (SECTreeNode *)1
#define TREENODE_SORT      (SECTreeNode *)2

class SECTreeNodeX : public SECTreeNode
{
  private:
    WORD m_root_wFilterLevel; //only valid in root node

  public:  
    SECTreeNodeX() { m_root_wFilterLevel=0; }
    virtual ~SECTreeNodeX() { m_root_wFilterLevel=0; }
    
    BOOL SetFilterLevel(WORD wLevel);
    WORD GetFilterLevel(void) const;
    WORD GetUnfilteredDistanceFromRoot(void) const;
   
    //override of SECTreeNode virtuals
    SECTreeNode *GetParent(void) const;
    SECTreeNode *GetNextSibling(void) const;
    SECTreeNode *GetPrevSibling(void) const;
    SECTreeNode *GetFirstChild(void) const;
};

//
// SEC Extension DLL
// Reset declaration context
//

#undef AFX_DATA
#define AFX_DATA
#undef AFXAPP_DATA
#define AFXAPP_DATA NEAR

#endif // __TREENODE_H__

#endif // WIN32
