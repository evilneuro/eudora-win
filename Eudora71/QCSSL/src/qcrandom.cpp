#include <stdio.h>
#include "qcrandom.h"
#include "DebugNewHelpers.h"


cic_Err QCRandomObject::Create()
{
	return CIC_ERR_NONE;
}

cic_Err QCRandomObject::SeedRandom(const char * /*poolFile*/, bool /*bHighSecurity*/)
{
	return CIC_ERR_NONE;
}

cic_Err QCRandomObject::RandomCallback(cic_Buffer *data, void * const /*randomRef*/)
{
	ssl_GenerateRandomSeed(NULL, data);

    return CIC_ERR_NONE;
}

cic_Err QCRandomObject::Destroy()
{
   return CIC_ERR_NONE;
}