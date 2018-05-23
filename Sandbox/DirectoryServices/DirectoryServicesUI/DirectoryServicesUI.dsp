# Microsoft Developer Studio Project File - Name="DirectoryServicesUI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DirectoryServicesUI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DirectoryServicesUI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DirectoryServicesUI.mak"\
 CFG="DirectoryServicesUI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DirectoryServicesUI - Win32 Debug" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "DirectoryServicesUI - Win32 Release" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "DirectoryServicesUI"
# PROP Scc_LocalPath ".."
CPP=cl.exe

!IF  "$(CFG)" == "DirectoryServicesUI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\..\SGISTL" /I "..\..\QCUtils\public\inc" /I "..\..\EuMemMgr\public\inc\AssertionServices" /I "..\..\EuMemMgr\public\inc\Core" /I "..\..\EuMemMgr\public\inc\CoreMT" /I "..\..\EuMemMgr\public\inc\Generic" /I "..\..\NewMgr\public\inc\CoreMT" /I "..\..\INetSDK\Include" /I "..\..\OT501\Include" /I ".\inc" /I ".\public\inc" /I "..\System\CritSectionImpl\inc" /I "..\System\CritSectionImpl\public\inc" /I "..\Kernel\public\inc" /I "..\..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /WX
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=.\PostProcessDbg DirectoryServicesUI
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DirectoryServicesUI - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Director"
# PROP BASE Intermediate_Dir "Director"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /I "..\..\SGISTL" /I "..\..\QCUtils\public\inc" /I "..\..\EuMemMgr\public\inc\AssertionServices" /I "..\..\EuMemMgr\public\inc\Core" /I "..\..\EuMemMgr\public\inc\CoreMT" /I "..\..\EuMemMgr\public\inc\Generic" /I "..\..\NewMgr\public\inc\CoreMT" /I "..\..\INetSDK\Include" /I "..\..\OT501\Include" /I ".\inc" /I ".\public\inc" /I "..\System\CritSectionImpl\inc" /I "..\System\CritSectionImpl\public\inc" /I "..\Kernel\public\inc" /I "..\..\Eudora" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# SUBTRACT BASE CPP /WX
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\..\SGISTL" /I "..\..\QCUtils\public\inc" /I "..\..\EuMemMgr\public\inc\AssertionServices" /I "..\..\EuMemMgr\public\inc\Core" /I "..\..\EuMemMgr\public\inc\CoreMT" /I "..\..\EuMemMgr\public\inc\Generic" /I "..\..\NewMgr\public\inc\CoreMT" /I "..\..\INetSDK\Include" /I "..\..\OT501\Include" /I ".\inc" /I ".\public\inc" /I "..\System\CritSectionImpl\inc" /I "..\System\CritSectionImpl\public\inc" /I "..\Kernel\public\inc" /I "..\..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /WX
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=.\PostProcessRel DirectoryServicesUI
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DirectoryServicesUI - Win32 Debug"
# Name "DirectoryServicesUI - Win32 Release"
# Begin Group "Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\inc\DirectoryServicesViewNewImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DirectoryServicesWazooWndNewImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSBounceMessageImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSEditImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSGenericListCtrlImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSGenericPushButtonImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSGenericRichEditCtrlImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSLeftViewImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSListCtrlImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSMailImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSMainSplitterWndImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSPrintEdit.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSRECtrlImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSResultsTocImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSRightViewImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSSplitterWndImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSUtilsImpl.h
# End Source File
# Begin Source File

SOURCE=.\inc\RegisterComponentImpl.h
# End Source File
# End Group
# Begin Group "Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\System\CritSectionImpl\src\DirectoryServicesCritSectionImpl.cpp
# ADD CPP /WX /Yu
# End Source File
# Begin Source File

SOURCE=.\src\DirectoryServicesViewNewImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DirectoryServicesWazooWndNewImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSEditImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSGenericListCtrlImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSGenericPushButtonImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSGenericRichEditCtrlImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSLeftViewImpl.cpp

!IF  "$(CFG)" == "DirectoryServicesUI - Win32 Debug"

# ADD CPP /Yu
# SUBTRACT CPP /WX

!ELSEIF  "$(CFG)" == "DirectoryServicesUI - Win32 Release"

# ADD BASE CPP /WX
# ADD CPP /WX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\DSListCtrlImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSMailImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSMainSplitterWndImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSPrintEdit.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSRECtrlImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSResultsTocImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DSRightViewImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSSplitterWndImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\DSUtilsImpl.cpp
# ADD CPP /WX
# End Source File
# Begin Source File

SOURCE=.\src\imgcache.cpp
# ADD CPP /WX /Yu
# End Source File
# Begin Source File

SOURCE=.\src\RegisterComponentImpl.cpp
# ADD CPP /WX /Yu
# End Source File
# Begin Source File

SOURCE=.\src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Public_Inc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\public\inc\DirectoryServicesBaseTypes.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DirectoryServicesViewNewIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DirectoryServicesWazooWndNewIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSEditIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSGenericListCtrlIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSGenericPushButtonIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSGenericRichEditCtrlIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSLeftViewIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSListCtrlIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSMailIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSMainSplitterWndIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSRECtrlIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSResultsTocIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSRightViewIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSSplitterWndIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\DSUtilsIface.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imgcache.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\RegisterComponentIface.h
# End Source File
# End Group
# End Target
# End Project
