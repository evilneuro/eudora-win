#if !defined EnginePreambleHIncluded
#define EnginePreambleHIncluded

#include "Platform.h"
#if enginePlatform == enginePlatformWindowsIntel32
#define haveNamespaces 1
#if _MSC_VER < 1100
#define haveIntrinsicBool 0
#pragma warning(disable: 4237)
#else
#define haveIntrinsicBool 1
#endif
#define haveDllExport 1
#else
#define haveNamespaces 0
#define haveIntrinsicBool 0
#define haveDllExport 0
#endif

#if haveNamespaces && defined __cplusplus
#define beginEngineNamespace namespace StuffItEngine {
#define endEngineNamespace }
#define usingEngineNamespace using namespace StuffItEngine;
#else
#define beginEngineNamespace
#define endEngineNamespace
#define usingEngineNamespace
#endif

#if !haveIntrinsicBool || !defined __cplusplus
typedef int bool;
#if defined false
#undef false
#endif
#define false 0
#if defined true
#undef true
#endif
#define true 1
#endif

#if haveDllExport
#if defined __cplusplus
#define engineAPI extern "C" __declspec(dllexport)
#else
#define engineAPI __declspec(dllexport)
#endif
#else // haveDllExport == 0
#if defined __cplusplus
#define engineAPI extern "C"
#else
#define engineAPI
#endif
#endif

#endif // EnginePreambleHIncluded

