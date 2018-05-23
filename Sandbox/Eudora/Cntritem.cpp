#include "stdafx.h"
#include <afxrich.h>
#include "cntritem.h"
#include "doc.h"
#include "msgdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CWordPadCntrItem implementation

IMPLEMENT_SERIAL(CRichCntrItem, CRichEditCntrItem, 0)

CRichCntrItem::CRichCntrItem(REOBJECT *preo, CMessageDoc* pContainer)
	: CRichEditCntrItem(preo, pContainer)
{
}

/////////////////////////////////////////////////////////////////////////////
// CWordPadCntrItem diagnostics

#ifdef _DEBUG
void CRichCntrItem::AssertValid() const
{
	CRichEditCntrItem::AssertValid();
}

void CRichCntrItem::Dump(CDumpContext& dc) const
{
	CRichEditCntrItem::Dump(dc);
}
#endif

/////////////////////////////////////////////////////////////////////////////
