// Version.h

#ifndef _VERSION_H_
#define _VERSION_H_

#include "regcode\regcode_eudora.h"

#define EUDORA_VERSION1 7
#define EUDORA_VERSION2 1
#define EUDORA_VERSION3 0
#define EUDORA_VERSION4 9
#define EUDORA_BUILD_NUMBER    7,1,0,9
#define EUDORA_BUILD_DESC      "Version 7.1.0.9\0"
#define EUDORA_BUILD_VERSION   "7.1.0.9"

// This is the month in which the product was built.
// It is used for determining whether a registered Pro user is qualified
// to use this version.  It is the number of months since Jan, 1999.
#define EUDORA_BUILD_MONTH REG_EUD_CLIENT_7_1_MONTH

#endif // _VERSION_H_
