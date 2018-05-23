// qcassert.h
//
// Copyright (c) 1997-2005 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */

//
// Long time QC ASSERT macros factored out of multiple stdafx.h files into
// one common file. Also reworked to support use of the Bugslayer SUPERASSERT.

#ifndef __QC_ASSERT_H__
#define __QC_ASSERT_H__

#include <afx.h> // Include ASSERT/VERIFY, so it cannot be included AFTER this

#include "BugslayerUtil.h"

#ifdef SUPERASSERT_EMAIL
#undef SUPERASSERT_EMAIL
#define SUPERASSERT_EMAIL "win-eudora-bugs@eudora.com"
#endif

#ifndef NOEXPRLOG
	#include "debug.h" // Logging (QCLogFileMT)
	#include "stdio.h" // sprintf

	#ifdef _DEBUG

		// We override the Bugslayer "NEWASSERT_REALMACRO" so that we can continue
		// our tradition of calling QCLogFileMT::WriteAssertLog.
		#ifdef NEWASSERT_REALMACRO
		#undef NEWASSERT_REALMACRO
		#endif

        // If you add local variables to this macro, make sure to update the
        // g_szAssertLocalVars array in AssertDlg.CPP so they don't show up in
        // the locals output and confuse the user.
        #ifdef _M_IX86
        #define NEWASSERT_REALMACRO( exp , type )                              \
        {                                                                      \
            /* The local instance of the ignore count and the total hits. */   \
            static int sIgnoreCount = 0 ;                                      \
            static int sFailCount   = 0 ;                                      \
            /* The local stack and frame at the assertion's location. */       \
            DWORD dwStack ;                                                    \
            DWORD dwStackFrame ;                                               \
            /* Check the expression. */                                        \
            if ( ! ( exp ) )                                                   \
            {                                                                  \
                QCLogFileMT::WriteAssertLog(#exp, __FILE__, __LINE__);         \
                /* Houston, we have a problem. */                              \
                _asm { MOV dwStack , ESP }                                     \
                _asm { MOV dwStackFrame , EBP }                                \
                if ( TRUE == SuperAssertion ( TEXT ( type )         ,          \
                                              TEXT ( #exp )         ,          \
                                              TEXT ( __FUNCTION__ ) ,          \
                                              TEXT ( __FILE__ )     ,          \
                                              __LINE__              ,          \
                                              SUPERASSERT_EMAIL     ,          \
                                              (DWORD64)dwStack      ,          \
                                              (DWORD64)dwStackFrame ,          \
                                              &sFailCount           ,          \
                                              &sIgnoreCount          ) )       \
                {                                                              \
                    __asm INT 3                                                \
                }                                                              \
            }                                                                  \
        }
        #endif  // _M_IX86

		// We override the Bugslayer definitions of ASSERT and VERIFY to instead
		// go through our wrapper macros, because the Eudora code relies on the
		// do-while(0) mechanism for wrapping. The Eudora code has cases like this:
		// if (error)
		//      ASSERT(0);
		// else
		//      DoSomething();
		// With the Bugslayer technique of just adding a level of scope code
		// like this will result in an error:
		// error C2181: illegal else without matching if
		//
		// With the do-while(0) mechanism code like above works. Long term
		// I'd like to see us rewrite cases like above, but for now we'll
		// continue to support it.

		#define QCVERIFY_WRAPPERMACRO(expr) \
			do { \
				NEWVERIFY(expr); \
			} while (0)

		#define QCASSERT_WRAPPERMACRO(expr) \
			do { \
				NEWASSERT(expr); \
			} while (0)

		#define VERIFYLOG(expr) QCVERIFY_WRAPPERMACRO(expr)
		#define EXPRLOG(expr) QCVERIFY_WRAPPERMACRO(expr)

		#ifdef VERIFY
		#undef VERIFY
		#endif

		#define VERIFY(expr) QCVERIFY_WRAPPERMACRO(expr)

		#ifdef ASSERT
		#undef ASSERT
		#endif

		#define ASSERT(expr) QCASSERT_WRAPPERMACRO(expr)
		
	#else // _DEBUG

		#define VERIFYLOG(expr) \
			do { \
				if ( !(expr) ) \
				{ \
					QCLogFileMT::WriteAssertLog(#expr, __FILE__, __LINE__); \
				} \
			} while (0)

		#define EXPRLOG(expr) \
			do { \
				if ( !(expr) && QCLogFileMT::DebugMaskSet(DEBUG_MASK_ASSERT) ) \
				{ \
					QCLogFileMT::WriteAssertLog(#expr, __FILE__, __LINE__); \
				} \
			} while (0)

	#endif // _DEBUG

#else // NOEXPRLOG

	#define VERIFYLOG(expr) VERIFY(expr)
	#define EXPRLOG(expr)   ASSERT(expr)

#endif // NOEXPRLOG

#endif // __QC_ASSERT_H__

