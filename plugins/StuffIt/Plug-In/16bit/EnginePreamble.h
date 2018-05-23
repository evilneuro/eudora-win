#if !defined EnginePreambleHIncluded
#define EnginePreambleHIncluded

#include "Platform.h"
#if enginePlatform == enginePlatformWindowsIntel32
#define haveNamespaces 1
#define haveIntrinsicBool 1
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
typedef char bool;
#define false 0
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

