# Microsoft Developer Studio Project File - Name="NewMgr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=NewMgr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NewMgr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NewMgr.mak" CFG="NewMgr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NewMgr - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "NewMgr - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "NewMgr"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "NewMgr - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "NewMgr_3"
# PROP BASE Intermediate_Dir "NewMgr_3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /WX /GX /Z7 /Od /I "inc" /I "public\inc" /I "..\EuMemMgr\public\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W4 /WX /GR /GX /Zi /Od /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\QCUtils\public\inc" /I ".\public\inc\CoreMT" /I ".\inc\CoreMT\private" /D "COMMERCIAL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT CPP /YX
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg NewMgr
# End Special Build Tool

!ELSEIF  "$(CFG)" == "NewMgr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NewMgr__"
# PROP BASE Intermediate_Dir "NewMgr__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /WX /GX /O2 /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\QCUtils\public\inc" /I ".\public\inc\CoreMT" /I ".\inc\CoreMT\private" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W4 /WX /GR /GX /Zi /O2 /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\QCUtils\public\inc" /I ".\public\inc\CoreMT" /I ".\inc\CoreMT\private" /D "COMMERCIAL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel NewMgr
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "NewMgr - Win32 Debug"
# Name "NewMgr - Win32 Release"
# Begin Source File

SOURCE=.\public\inc\CoreMT\NewIfaceMT.h
# End Source File
# Begin Source File

SOURCE=.\src\CoreMT\NewImplMT.cpp
# End Source File
# Begin Source File

SOURCE=.\inc\CoreMT\private\NewImplMT.h
# End Source File
# End Target
# End Project
