# Microsoft Developer Studio Project File - Name="SearchEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SearchEngine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SearchEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SearchEngine.mak" CFG="SearchEngine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SearchEngine - Win32 Debug" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "SearchEngine - Win32 Release" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "SearchEngine"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "SearchEngine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SearchEn"
# PROP BASE Intermediate_Dir "SearchEn"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I ".\PrivateInc" /I ".\PublicInc" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "IMAP4" /FR /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /WX /Gf /Gy
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg SearchEngine
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SearchEngine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SearchE0"
# PROP BASE Intermediate_Dir "SearchE0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /Gm /GR /GX /Zi /O2 /I ".\PrivateInc" /I ".\PublicInc" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "IMAP4" /Yu"stdafx.h" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel SearchEngine
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SearchEngine - Win32 Debug"
# Name "SearchEngine - Win32 Release"
# Begin Group "Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\SearchCriteria.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SearchEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\SearchResult.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "PublicInc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PublicInc\SearchCriteria.h
# End Source File
# Begin Source File

SOURCE=.\PublicInc\SearchEngine.h
# End Source File
# Begin Source File

SOURCE=.\PublicInc\SearchResult.h
# End Source File
# End Group
# End Target
# End Project
