
#include "stdafx.h"

#ifdef WIN32

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL

#include "TreeNode.h"

#ifdef _SECDLL
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif //_SECDLL


/////////////////////////////////////////////////////////////////////////////////
// SECTreeNode
/////////////////////////////////////////////////////////////////////////////////

WORD
SECTreeNode::GetDistanceFromRoot(void) const
{
	WORD w;
	SECTreeNode *pNode=this->GetParent();

	for( w=0 ; pNode ; w++)
		pNode = pNode->GetParent();

	return w;
}

////

SECTreeNode *
SECTreeNode::GetRoot(void) const
//
// This is a question to ask SECTreeNode object!
//
{
	const SECTreeNode *pNode;

	for( pNode=this; pNode->m_pParent ; pNode=pNode->m_pParent )
		; //do nothing (like my weekends)

	return (SECTreeNode*)pNode;
}

////

SECTreeNode *
SECTreeNode::GetFirstSibling() const
{
	const SECTreeNode *pNode=this;

	while( TRUE )
	{
		if( !pNode->GetPrevSibling() )
			return (SECTreeNode*)pNode;

		pNode = pNode->GetPrevSibling();
	}

	ASSERT(FALSE);
	return NULL;

}

SECTreeNode *
SECTreeNode::GetLastSibling() const
{
	SECTreeNode *pNode = (SECTreeNode*)this;

	while (pNode->GetNextSibling())
		pNode = pNode->GetNextSibling();

	return pNode;
}

SECTreeNode *
SECTreeNode::GetLastChild(void) const
{
	SECTreeNode *pNode;
	pNode = GetFirstChild();
	if( !pNode )
		return NULL;
	
	return pNode->GetLastSibling();
}

SECTreeNode *
SECTreeNode::GetBottomLeftChild(void) const
{
	SECTreeNode *pNode = m_pFirstChild;

	if( pNode )
	{
		while( pNode->GetFirstChild() )
			pNode = pNode->GetFirstChild();
	}

	return pNode;
}

SECTreeNode *
SECTreeNode::GetBottomRightChild(void) const
{
	SECTreeNode* pNode = GetLastChild();
	if (pNode)
		return pNode->GetBottomRightChild();
	else
		return (SECTreeNode*)this;
}

////

void SECTreeNode::DetachFromTree(void)
//
// private
//
{
	//ASSERT( !HasChildren() );

	if( m_pNextSibling )
		m_pNextSibling->m_pPrevSibling = m_pPrevSibling;

	if( m_pPrevSibling )
		m_pPrevSibling->m_pNextSibling = m_pNextSibling;

	if( m_pParent->m_pFirstChild==this )
		m_pParent->m_pFirstChild = m_pNextSibling;

	m_pParent = NULL;
}

////

void SECTreeNode::DeleteChildren(void)
{
	//TRACE( "DeleteChildren\n" );
	
	SECTreeNode *pBLC;
		
	while( 	(pBLC = GetBottomLeftChild()) != NULL )
	{
		pBLC->DetachFromTree();
		delete pBLC;
	}
}

////

void SECTreeNode::DeleteAllChildren(void)
{
	SECTreeNode *pChild;
	
	while( (pChild=GetFirstChild()) != NULL )
	{
		pChild->DetachFromTree();
		delete pChild;
	}
}

////

BOOL SECTreeNode::AddChild(SECTreeNode *pNewTreeNode,
                         SECTreeNode *pInsAfter  )
{
	if( pInsAfter==TREENODE_SORT ) //24/05/96
	{
		TRACE( _T("Warning: haven't implemented TREENODE_SORT\n") );
		pInsAfter=TREENODE_LAST;
	}
	
	pNewTreeNode->m_pParent = this;

	if( !m_pFirstChild )
	{
		//node does not have any children
		ASSERT(    pInsAfter==TREENODE_FIRST 
		        || pInsAfter==TREENODE_LAST );

		m_pFirstChild = pNewTreeNode;
		pNewTreeNode->m_pNextSibling=NULL;
		pNewTreeNode->m_pPrevSibling=NULL;
	}
	else
	{
		//node has children
		SECTreeNode *pChild;

		if( pInsAfter==TREENODE_LAST )
		{
			//insert as last child
			pChild = m_pFirstChild; 
			while(  pChild->m_pNextSibling )
				pChild= pChild->m_pNextSibling;
			
			//now pChild is last child
			pChild->m_pNextSibling = pNewTreeNode;
			pNewTreeNode->m_pPrevSibling = pChild;
			pNewTreeNode->m_pNextSibling = NULL;
		}
		else if( pInsAfter==TREENODE_FIRST )
		{
			//insert as first child
			pNewTreeNode->m_pPrevSibling  = NULL;
			pNewTreeNode->m_pNextSibling  = m_pFirstChild;
			m_pFirstChild->m_pPrevSibling = pNewTreeNode;
			m_pFirstChild                 = pNewTreeNode;
		}
		else
		{
			//insert after a specified sibling
			BOOL bFound=FALSE;
			for( pChild = m_pFirstChild; 
			     pChild; 
				 pChild = pChild->m_pNextSibling )
			{
				if( pChild==pInsAfter )
				{
					bFound=TRUE;
					break;
				}
			}

			if( !bFound )
			{
				ASSERT(FALSE);
				return FALSE;
			}

			//put in pNewTreeNode
			pNewTreeNode->m_pNextSibling=pInsAfter->m_pNextSibling;
			pNewTreeNode->m_pPrevSibling=pInsAfter;
			//make a whole to file pNewTreeNode into
			if( pInsAfter->m_pNextSibling )
				pInsAfter->m_pNextSibling->m_pPrevSibling = pNewTreeNode;
			pInsAfter->m_pNextSibling = pNewTreeNode;
		}
	}

	return TRUE;
}

////

BOOL SECTreeNode::OnNextSearchNode( WORD idSearch, SECTreeNode *pNode ) const
//
// A serach func for subclass to over-ride
//
{ 
	idSearch; // UNUSED
	pNode; // UNUSED
	ASSERT(FALSE); 
	return FALSE;
}

SECTreeNode *SECTreeNode::Search( WORD idSearch ) const
//
// This search is ordered in a specific manner :-
//   in order of bottom left most branch.
//
{
	SECTreeNode *pNode;
	BOOL       bFound;
	
	pNode=GetBottomLeftChild();

	while( pNode )
	{
		bFound=OnNextSearchNode( idSearch, pNode );
		if( bFound )
			return pNode;
	
		if( pNode->GetNextSibling() )
			pNode=pNode->GetNextSibling();
		else if( pNode->GetParent() && pNode->GetParent()!=this )
			pNode=pNode->GetParent();
		else
			break;
	}

	return NULL;
}

////

SECTreeNode *
SECTreeNode::GetNextInDisplayOrder(void) const
{
	if( GetFirstChild() )
		return GetFirstChild();

	if( GetNextSibling() )
		return GetNextSibling();
	
	const SECTreeNode *pNodeAncestor=this;
	while( pNodeAncestor->GetParent() )
	{
		pNodeAncestor=pNodeAncestor->GetParent();
		if( pNodeAncestor->GetNextSibling() )
			return pNodeAncestor->GetNextSibling();
	}

	return NULL;
}

////

SECTreeNode *
SECTreeNode::GetPrevInDisplayOrder(void) const
{
	if( GetPrevSibling() )
	{
		SECTreeNode *pBRC=GetPrevSibling()->GetBottomRightChild();
		if( pBRC )
			return pBRC;

		return GetPrevSibling();
	}

	return GetParent();
}

////

BOOL
SECTreeNode::IsDescendant( SECTreeNode *pNode ) const
{
	if( !pNode	/*|| pNode==this*/ )
	{
		ASSERT(FALSE);
		return FALSE;
	}

	while( pNode->GetParent() )
	{
		pNode=pNode->GetParent();
		if( pNode == this )
			return TRUE;

		if( !AfxIsValidAddress(pNode,sizeof(*pNode) ) )
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}

	return FALSE;
}

////

BOOL
SECTreeNode::IsAncestor( SECTreeNode *pNodePossibleAncestor ) const
//
// kkep looking at parents of this and compare to arg
//
{
    const SECTreeNode *pNode=this;

	for( pNode=GetParent() ; pNode ; pNode=pNode->GetParent() )
	{
		if( pNode == pNodePossibleAncestor )
			return TRUE;
	}

	return FALSE;
}

////

BOOL
SECTreeNode::IsSibling( SECTreeNode *pNodeTest ) const
{
    SECTreeNode *pNode;

    for( pNode=GetFirstSibling(); pNode ; pNode=pNode->GetNextSibling() )
    {
        if( pNodeTest==pNode )
        {
            return(TRUE);
        }
    } 

    return FALSE;
}

/////

UINT
SECTreeNode::GetNumDescendents(void) const
//
// iterate and count all descendent nodes below this one
//
{
	UINT iCount=0;

	for ( SECTreeNode *pNode=GetFirstChild()	; 
	      pNode ; 
		  pNode = pNode->GetNextInDisplayOrder() )  // BUG?? GetNextInDisplayOrder??
	{
		iCount++;
	}

	return iCount;


}

BOOL SECTreeNode::IsExpanded() const
{
	return FALSE;
}

void SECTreeNode::Expand(BOOL bExpand)
{
	bExpand; // UNUSED
}

/////////////////////////////////////////////////////////////////////////////////
// SECTreeNodeX
/////////////////////////////////////////////////////////////////////////////////

WORD
SECTreeNodeX::GetUnfilteredDistanceFromRoot(void) const
//
// private
//
{
	WORD w;
	SECTreeNode *pNode=this->m_pParent;

	for( w=0 ; pNode ; w++)
		pNode = pNode->m_pParent;

	return w;

}

////

BOOL
SECTreeNodeX::SetFilterLevel(WORD wFilterLevel)
{
    ((SECTreeNodeX *)GetRoot())->m_root_wFilterLevel=wFilterLevel;
    return TRUE;
}

////

WORD
SECTreeNodeX::GetFilterLevel(void) const
{
    return ((SECTreeNodeX *)GetRoot())->m_root_wFilterLevel;
}

////

SECTreeNode *
SECTreeNodeX::GetParent(void) const
{
    if( GetFilterLevel()>0 && m_pParent )
    {
        if( GetUnfilteredDistanceFromRoot()==(GetFilterLevel()+1) )
        {
            return m_pParent->m_pParent;
        }
    }

    return m_pParent;
}

////

SECTreeNode *
SECTreeNodeX::GetFirstChild(void) const
{
    if( m_pFirstChild && GetFilterLevel()>0 )
    {
        if( GetUnfilteredDistanceFromRoot()==(GetFilterLevel()-1) )
        {
            return m_pFirstChild->m_pFirstChild;
        }
    }

    return m_pFirstChild;
}

////

SECTreeNode *
SECTreeNodeX::GetNextSibling(void) const
{
    if( GetFilterLevel()>0 )
    {
        if( GetUnfilteredDistanceFromRoot()==(GetFilterLevel()+1) )
        {
            if( !m_pNextSibling )
            {
                SECTreeNode *pNode = m_pParent;
                if( !pNode )
                    return NULL;
                
                pNode = pNode->m_pNextSibling;
                if( !pNode )
                    return NULL;

                pNode = pNode->m_pFirstChild;
                return pNode;
            }
        }
    }

    return m_pNextSibling;
}

////

SECTreeNode *
SECTreeNodeX::GetPrevSibling(void) const
{
    if( GetFilterLevel()>0 )
    {
        if( GetUnfilteredDistanceFromRoot()==(GetFilterLevel()+1) )
        {
            if( !m_pPrevSibling )
            {
                SECTreeNode *pNode = m_pParent;
                if( !pNode )
                    return NULL;
                
                pNode = pNode->m_pPrevSibling;
                if( !pNode )
                    return NULL;

                pNode = pNode->m_pFirstChild;
                if( !pNode )
                    return NULL;

                while( pNode->m_pNextSibling )
                    pNode=pNode->m_pNextSibling;

                return pNode;
            }
        }
    }

    return m_pPrevSibling;
}

#endif // WIN32

/////////////////////////////////////////////////////////////////////////////////
// End of SECTreeNodeX
/////////////////////////////////////////////////////////////////////////////////
