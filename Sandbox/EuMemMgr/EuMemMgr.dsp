# Microsoft Developer Studio Project File - Name="EuMemMgr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=EuMemMgr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EuMemMgr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EuMemMgr.mak" CFG="EuMemMgr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EuMemMgr - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EuMemMgr - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "EuMemMgr"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EuMemMgr - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "EuMemMg3"
# PROP BASE Intermediate_Dir "EuMemMg3"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W4 /WX /Gm /GX /Zi /Od /I "inc" /I "public\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MDd /W4 /WX /Gm /GR /GX /Zi /Od /I "..\SGISTL" /I ".\inc\Win32" /I ".\inc\AssertionServices\private" /I ".\inc\AssertionServices\public" /I ".\inc\AssertionUtils\private" /I ".\inc\AssertionUtils\public" /I ".\inc\Core\private" /I ".\inc\Core\public" /I ".\inc\CoreMT\private" /I ".\inc\CoreMT\public" /I ".\public\inc\AssertionServices" /I ".\public\inc\Core" /I ".\public\inc\CoreMT" /I ".\public\inc\Generic" /I "..\QCUtils\public\inc" /D "COMMERCIAL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "inc" /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.0 /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.0 /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg EuMemMgr
# End Special Build Tool

!ELSEIF  "$(CFG)" == "EuMemMgr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "EuMemMgr"
# PROP BASE Intermediate_Dir "EuMemMgr"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /WX /GX /O1 /I ".\inc\Win32" /I ".\inc\AssertionServices\private" /I ".\inc\AssertionServices\public" /I ".\inc\AssertionUtils\private" /I ".\inc\AssertionUtils\public" /I ".\inc\Core\private" /I ".\inc\Core\public" /I ".\inc\CoreMT\private" /I ".\inc\CoreMT\public" /I ".\public\inc\AssertionServices" /I ".\public\inc\Core" /I ".\public\inc\CoreMT" /I ".\public\inc\Generic" /I "..\QCUtils\public\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W4 /WX /GR /GX /Zi /O2 /I "..\SGISTL" /I ".\inc\Win32" /I ".\inc\AssertionServices\private" /I ".\inc\AssertionServices\public" /I ".\inc\AssertionUtils\private" /I ".\inc\AssertionUtils\public" /I ".\inc\Core\private" /I ".\inc\Core\public" /I ".\inc\CoreMT\private" /I ".\inc\CoreMT\public" /I ".\public\inc\AssertionServices" /I ".\public\inc\Core" /I ".\public\inc\CoreMT" /I ".\public\inc\Generic" /I "..\QCUtils\public\inc" /D "COMMERCIAL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x66000000" /subsystem:windows /dll /map /debug /machine:I386
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel EuMemMgr
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "EuMemMgr - Win32 Debug"
# Name "EuMemMgr - Win32 Release"
# Begin Source File

SOURCE=.\public\inc\Generic\DMMDumpHandlerIface.h
# End Source File
# Begin Source File

SOURCE=.\src\Win32\EuMemMgrImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\Win32\EuMemMgrInternalIface.h
# End Source File
# Begin Source File

SOURCE=.\src\Win32\EuMemMgrVer.rc
# End Source File
# Begin Source File

SOURCE=.\public\inc\AssertionServices\MemAssertExternalIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\CoreMT\MemAssertExternalIfaceMT.h
# End Source File
# Begin Source File

SOURCE=.\src\AssertionServices\MemAssertImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\AssertionServices\private\MemAssertImpl.h
# End Source File
# Begin Source File

SOURCE=.\src\CoreMT\MemAssertImplMT.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\CoreMT\private\MemAssertImplMT.h
# End Source File
# Begin Source File

SOURCE=.\inc\AssertionServices\public\MemAssertInternalIface.h
# End Source File
# Begin Source File

SOURCE=.\src\AssertionUtils\MemAssertUtilsImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\AssertionUtils\private\MemAssertUtilsImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\AssertionUtils\public\MemAssertUtilsInternalIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\Generic\MemDbgDefIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\Generic\MemExportIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\Core\MemExternalIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\CoreMT\MemExternalIfaceMT.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\CoreMT\MemIfaceMT.h
# End Source File
# Begin Source File

SOURCE=.\src\Core\MemImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\Core\private\MemImpl.h
# End Source File
# Begin Source File

SOURCE=.\src\CoreMT\MemImplMT.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\CoreMT\private\MemImplMT.h
# End Source File
# Begin Source File

SOURCE=.\inc\Core\public\MemInternalIface.h
# End Source File
# Begin Source File

SOURCE=.\src\CoreMT\MemMgrSerializedRegionImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\CoreMT\private\MemMgrSerializedRegionImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\Win32\resource.h
# End Source File
# End Target
# End Project
