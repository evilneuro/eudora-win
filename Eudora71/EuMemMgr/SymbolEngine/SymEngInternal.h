/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#ifndef _SYMENGINTERNAL_H
#define _SYMENGINTERNAL_H

#pragma once

#ifndef ASSERT
#pragma warning ( disable : 4127 )
#define ASSERT(x)   if (!(x)) __asm INT 3
#define VERIFY(x)   ASSERT(x)
#endif


struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
{
    ULONG   MoreChildIds[1024];
    FINDCHILDREN ( )
    {
        Count = sizeof(MoreChildIds) / sizeof(MoreChildIds[0]);
    }
} ;

#endif  // _SYMENGINTERNAL_H

