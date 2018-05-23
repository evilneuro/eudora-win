///////////////////////////////////////////////////////////////////////////////
// DSBaseTypes.h
//
// Base Types.
//
// Created: 08/13/97 smohanty
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __DIRECTORY_SERVICES_BASETYPES_H_
#define __DIRECTORY_SERVICES_BASETYPES_H_

#include "dsapi.h"

typedef struct _directory_services {
    CImageList  ilDatabase_State;
    HIMAGELIST  hilDatabase_Entry;
    IDirServ   *pDirServ;
    IDSQuery   *pQuery;
    _directory_services()
	: pDirServ(NULL), pQuery(NULL)
    {}
    ~_directory_services()
    {}
} Directory_Services, *p_Directory_Services;

#endif // __DIRECTORY_SERVICES_BASETYPES_H_
