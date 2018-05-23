# Microsoft Developer Studio Project File - Name="QCSocket" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=QCSocket - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QCSocket.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QCSocket.mak" CFG="QCSocket - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QCSocket - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "QCSocket - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "QCSocket"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QCSocket - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\QCUtils\public\inc" /I "..\NewMgr\public\inc\CoreMT" /I ".\inc" /I "..\Eudora" /D COMMERCIAL=1 /D "_DEBUG" /D "THREADED" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "inc" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 QCUtils.lib EuMemMgr.lib NewMgr.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\Lib\Debug"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg QCSocket
# End Special Build Tool

!ELSEIF  "$(CFG)" == "QCSocket - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "QCSocket"
# PROP BASE Intermediate_Dir "QCSocket"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\QCUtils\public\inc" /I "..\NewMgr\public\inc\CoreMT" /I ".\inc" /I "..\Eudora" /D "NDEBUG" /D "THREADED" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\QCUtils\public\inc" /I "..\NewMgr\public\inc\CoreMT" /I ".\inc" /I "..\Eudora" /D "COMMERCIAL" /D "NDEBUG" /D "THREADED" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "inc" /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "inc" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 QCUtils.lib EuMemMgr.lib NewMgr.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"..\Lib\Release"
# ADD LINK32 QCUtils.lib EuMemMgr.lib NewMgr.lib /nologo /base:"0x60500000" /subsystem:windows /dll /map /debug /machine:I386 /libpath:"..\Lib\Release"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel QCSocket
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "QCSocket - Win32 Debug"
# Name "QCSocket - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\QCHesiodLib.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QCRasConnectionMT.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QCSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QCSocketVer.rc
# End Source File
# Begin Source File

SOURCE=.\src\QcWinSockLib.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QCWorkerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\inc\CallbackDefines.h
# End Source File
# Begin Source File

SOURCE=.\inc\QCHesiodLib.h
# End Source File
# Begin Source File

SOURCE=.\inc\QCRASConnection.h
# End Source File
# Begin Source File

SOURCE=.\inc\QCSockCallbacks.h
# End Source File
# Begin Source File

SOURCE=.\inc\QCWinSockLib.h
# End Source File
# Begin Source File

SOURCE=.\inc\QCWorkerSocket.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
