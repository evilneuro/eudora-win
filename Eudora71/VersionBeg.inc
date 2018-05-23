#include <winver.h>
#include "Version.h"

VS_VERSION_INFO VERSIONINFO
 FILEVERSION EUDORA_BUILD_NUMBER
 PRODUCTVERSION EUDORA_BUILD_NUMBER
 FILEFLAGSMASK      VS_FFI_FILEFLAGSMASK

#ifdef _DEBUG
 FILEFLAGS          VS_FF_DEBUG|VS_FF_PRIVATEBUILD|VS_FF_PRERELEASE
#else
 FILEFLAGS 0x0L
#endif

 FILEOS             VOS__WINDOWS32
 FILETYPE           VFT_APP
 FILESUBTYPE        0   // not used
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"    // Lang=US English, CharSet=Windows Multilingual
        BEGIN
            VALUE "CompanyName", "QUALCOMM Incorporated\0"
            VALUE "LegalCopyright", "Copyright \251 1991-2006\0"

            VALUE "FileVersion", EUDORA_BUILD_VERSION
            VALUE "ProductVersion", EUDORA_BUILD_VERSION
            VALUE "ProductName", "Eudora\0"
