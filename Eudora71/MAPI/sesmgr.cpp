////////////////////////////////////////////////////////////////////////
//
// CMapiSessionMgr
//
// Handles MAPI sessions and memory management for multiple MAPI 
// client applications.
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <afxwin.h>			// FORNOW, probably should use precompiled header

#include "..\Eudora\eumapi.h"
#include "recip.h"
#include "message.h"
#include "sesmgr.h"

#ifdef _DEBUG
#define OUTPUTDEBUGSTRING(_dbg_Msg_) OutputDebugString(_dbg_Msg_)
#else
#define OUTPUTDEBUGSTRING(_dbg_Msg_)
#endif // _DEBUG


//
// CBufferListNode
//
// Private class for storing and identifying the class type of data 
// in the the memory buffer.
//
class CBufferListNode : public CObject
{
	friend class CMapiSessionMgr;

private:
	enum ObjectType
	{
		OBJ_MAPI_MESSAGE,
		OBJ_MAPI_RECIP_DESC_ARRAY
	};


	ObjectType	m_ObjType;
	void*		m_pBuffer;
	
	// Constructor
	CBufferListNode(ObjectType objType, void* pBuffer) :
		m_ObjType(objType),
		m_pBuffer(pBuffer)
	{
		ASSERT(pBuffer != NULL);
	}

	// Destructor
	~CBufferListNode(void)
	{
#ifdef _DEBUG
		char buf[100];
#endif // _DEBUG
		switch (m_ObjType)
		{
		case OBJ_MAPI_MESSAGE:
#ifdef _DEBUG
			sprintf(buf, "FreeBuffer: MapiMessage at <%p>\n", m_pBuffer);
			OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
			delete ((CMapiMessage *) m_pBuffer);
			break;
		case OBJ_MAPI_RECIP_DESC_ARRAY:
#ifdef _DEBUG
			sprintf(buf, "FreeBuffer: MapiRecipDescArray at <%p>\n", m_pBuffer);
			OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG
			delete [] ((CMapiRecipDesc *) m_pBuffer);
			break;
		default:
			ASSERT(0);		// memory leak!
			break;
		}
	}

	CBufferListNode(void);						// not implemented
	CBufferListNode(const CBufferListNode&);	// not implemented
	void operator=(const CBufferListNode&);		// not implemented
};



////////////////////////////////////////////////////////////////////////
// CMapiSessionMgr [public, constructor]
//
////////////////////////////////////////////////////////////////////////
CMapiSessionMgr::CMapiSessionMgr(void) :
	m_NextAvailableSessionID(1)
{
	// nothing to do
	OUTPUTDEBUGSTRING("CMapiSessionMgr: constructing\n");
}


////////////////////////////////////////////////////////////////////////
// ~CMapiSessionMgr [public, destructor]
//
////////////////////////////////////////////////////////////////////////
CMapiSessionMgr::~CMapiSessionMgr(void)
{
	//
	// Step 1.  The 'm_SessionList' is a CPtrList with void* casts
	// of scalars rather than pointers to memory blocks,
	// so there's nothing to do there.
	//

	//
	// Step 2.  The 'm_BufferList' is a list of nodes containing
	// pointers to memory blocks.  But, to be safe, it is probably
	// better to just let me go without attempting to delete them.
	// It's somewhat likely that caller's of this module still have
	// live pointers to these memory blocks.
	//

	//
	// Step 3.  The 'm_MessageList' is CString based, so there it is
	// cleaned up automatically.
	//

	//
	// Time to eat cake.  Hmm.  I guess that leaves us with nothing
	// to do here...
	//
	OUTPUTDEBUGSTRING("CMapiSessionMgr: destructing\n");
}


////////////////////////////////////////////////////////////////////////
// CreateSession [public]
//
// Returns non-zero session handle on success, else zero on failure.
//
////////////////////////////////////////////////////////////////////////
LHANDLE CMapiSessionMgr::CreateSession(void)
{
	//
	// Sanity check to make sure that the session doesn't already exist.
	//
	OUTPUTDEBUGSTRING("CMapiSessionMgr::CreateSession\n");
	ASSERT(! IsValidSessionId(m_NextAvailableSessionID));

	//
	// Register a new session ID.
	//
	m_SessionList.AddTail((void *) m_NextAvailableSessionID);

	return m_NextAvailableSessionID++;
}


////////////////////////////////////////////////////////////////////////
// DestroySession [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiSessionMgr::DestroySession(LHANDLE sessionId)
{
	//
	// Walk through the list of existing session IDs in search of a
	// matching session ID.  If found, destroy the session entry.
	//
	OUTPUTDEBUGSTRING("CMapiSessionMgr::DestroySession\n");
	POSITION pos = m_SessionList.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION save_pos = pos;
		if (sessionId == LHANDLE(m_SessionList.GetNext(pos)))
		{
			//
			// Found matchine session ID, so blow it away and we're
			// outta here...
			//
			m_SessionList.RemoveAt(save_pos);
			return TRUE;
		}
	}

	return FALSE;		// matching session not found
}


////////////////////////////////////////////////////////////////////////
// IsValidSessionId [public]
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiSessionMgr::IsValidSessionId(LHANDLE sessionId)
{
	//
	// Walk through the list of existing session IDs in search of a
	// matching session ID.  If found, return TRUE.
	//
	POSITION pos = m_SessionList.GetHeadPosition();
	while (pos != NULL)
	{
		if (sessionId == LHANDLE(m_SessionList.GetNext(pos)))
		{
			//
			// Found a match, so we're outta here...
			//
			return TRUE;
		}
	}

	return FALSE;		// matching session not found
}


////////////////////////////////////////////////////////////////////////
// NewMapiMessage [public]
//
////////////////////////////////////////////////////////////////////////
CMapiMessage* CMapiSessionMgr::NewMapiMessage(void)
{
	//
	// Allocate a new MapiMessage record.
	//
	CMapiMessage* p_mapi_message = new CMapiMessage;
	if (NULL == p_mapi_message)
		return NULL;		// out of memory

	//
	// Save a copy of the pointer to the record in the master memory 
	// buffer list for this session ID.
	//
	CBufferListNode* p_new_node = new CBufferListNode(CBufferListNode::OBJ_MAPI_MESSAGE, p_mapi_message);
	if (NULL == p_new_node)
	{
		delete p_mapi_message;
		return NULL;
	}
	m_BufferList.AddTail(p_new_node);

#ifdef _DEBUG
	char buf[100];
	sprintf(buf, "NewMapiMessage: alloc at <%p>\n", p_mapi_message);
	ASSERT(sizeof(buf) > strlen(buf));		// better late than never
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	return p_mapi_message;
}


////////////////////////////////////////////////////////////////////////
// NewMapiRecipDescArray [public]
//
////////////////////////////////////////////////////////////////////////
CMapiRecipDesc* CMapiSessionMgr::NewMapiRecipDescArray(ULONG numElements)
{
	//
	// Allocate a new array of MapiRecipDesc records.
	//
	CMapiRecipDesc* p_recip_array = new CMapiRecipDesc[numElements];
	if (NULL == p_recip_array)
		return NULL;		// out of memory

	//
	// Save a copy of the pointer to the array in the master memory 
	// buffer list.
	//
	CBufferListNode* p_new_node = new CBufferListNode(CBufferListNode::OBJ_MAPI_RECIP_DESC_ARRAY, p_recip_array);
	if (NULL == p_new_node)
	{
		delete [] p_recip_array;
		return NULL;
	}
	m_BufferList.AddTail(p_new_node);

#ifdef _DEBUG
	char buf[100];
	sprintf(buf, "NewMapiRecipDescArray: alloc at <%p>\n", p_recip_array);
	ASSERT(sizeof(buf) > strlen(buf));		// better late than never
	OUTPUTDEBUGSTRING(buf);
#endif // _DEBUG

	return p_recip_array;
}


////////////////////////////////////////////////////////////////////////
// FreeBuffer [public]
//
// Okay, this is wicked.  We're just given a raw pointer to some 
// memory.  We don't know its type or size or what session was used
// to create the block.  Therefore, we must use the brute force
// approach of walking all memory buffer lists for all sessions until
// we find a match or we don't.
//
////////////////////////////////////////////////////////////////////////
BOOL CMapiSessionMgr::FreeBuffer(void* pBuffer)
{
	//
	// Walk through the list of existing memory buffers in search of a
	// matching memory buffer.  If found, destroy the memory buffer.
	//
	POSITION pos = m_BufferList.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION save_pos = pos;
		CBufferListNode* p_node = (CBufferListNode *) m_BufferList.GetNext(pos);
		ASSERT(p_node != NULL);
		if (p_node->m_pBuffer == pBuffer)
		{
			//
			// Found matching memory buffer, so blow it away and we're
			// outta here...  Note that the destructor for the memory
			// buffer takes care of doing the proper cleanup.
			//
			m_BufferList.RemoveAt(save_pos);
			delete p_node;
			return TRUE;
		}
	}

	return FALSE;		// memory buffer not found
}
