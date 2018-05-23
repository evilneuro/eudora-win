#ifndef _SRANDOM_H_
#define _SRANDOM_H_

#include "QCSSL.h"

class QCRandomObject
{
public:
	cic_Err Create();
	cic_Err SeedRandom(const char * poolFile = NULL, bool bHighSecurity = FALSE);
	static cic_Err RandomCallback(cic_Buffer *data, void * const randomRef);
	cic_Err Destroy();
};





#endif 
