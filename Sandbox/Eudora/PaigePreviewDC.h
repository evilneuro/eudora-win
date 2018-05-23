#ifndef _PAIGE_PRV_DC_H_
#define _PAIGE_PRV_DC_H_

#include "paige.h"
//Helper class for CPaigeEdtView... used for print preview
class CPaigePreviewDC : public CPreviewDC
{
	DECLARE_DYNAMIC(CPaigePreviewDC)

public:
// Constructors
	CPaigePreviewDC();

// Implementation
public:
	virtual ~CPaigePreviewDC();


	int GetScaleNum() { return m_nScaleNum;}
	int GetScaleDen() { return m_nScaleDen;}
};



#endif