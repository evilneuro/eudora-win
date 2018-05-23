// trace.h

#ifndef __TRACE_H__
#define __TRACE_H__

#if defined( __cplusplus )
extern "C" {
#endif

#ifdef DEBUG
void __cdecl Trace(LPSTR pszFmt, ...);
#else
#define Trace ((void)0)
#endif

#if defined( __cplusplus )
}
#endif

#endif  // __TRACE_H__

