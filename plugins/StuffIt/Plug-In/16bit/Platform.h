#if !defined PlatformHIncluded
#define PlatformHIncluded

#define enginePlatformWindowsIntel32 1
#define enginePlatformWindowsIntel16 2
#undef enginePlatform
#if !defined enginePlatform && defined WIN32
#define enginePlatform enginePlatformWindowsIntel32
#endif
#if !defined enginePlatform && (defined _WINDOWS || defined _DOS) && !defined WIN32
#define enginePlatform enginePlatformWindowsIntel16
#endif

#endif // PlatformHIncluded

