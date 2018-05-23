#include "stdafx.h"
#include "PaigePreviewDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//Helper class for CPaigeEdtView... used for print preview

/////////////////////////////////////////////////////////////////////////////
// CPaigePreviewDC

IMPLEMENT_DYNAMIC(CPaigePreviewDC, CPreviewDC)

/////////////////////////////////////////////////////////////////////////////
// CPaigePreviewDC construction/destruction

CPaigePreviewDC::CPaigePreviewDC()
{

}

CPaigePreviewDC::~CPaigePreviewDC()
{
}
