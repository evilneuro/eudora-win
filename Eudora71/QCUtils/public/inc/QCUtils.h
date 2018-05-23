///////////////////////////////////////////////////////////////////////////////
// QCUtils.h
//
// Public interface to QCUtils DLL.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __QCUTILS_H__
#define __QCUTILS_H__

//
// The idea is to have a unified header file QCUTILS.H which auto-includes
// all the stuff supported by the QCUTILS module.
//
#include "../../inc/QCError.h"

#include "../../inc/jjfile.h"
#include "../../inc/services.h"
//#include "../../inc/QCHesiodLib.h"   //remove it and add it to QCSocket
#include "../../inc/bitmap.h"
#include "../../inc/os.h" // Some old #define macros
#include "../../inc/qcsecfail.h"

#endif // __QCUTILS_H__
