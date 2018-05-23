# Microsoft Developer Studio Project File - Name="Imap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Imap - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "imap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "imap.mak" CFG="Imap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Imap - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Imap - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "Imap"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Imap - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\SGISTL" /I "..\QCSocket\inc" /I "..\QCUtils\Public\Inc" /I "..\QCSocket\Inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\inc" /I ".\public\inc" /I "..\Eudora" /D "COMMERCIAL" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "THREADED" /D "IMAP4" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Build\Debug\Imap.res" /i ".\inc" /i ".\public\inc" /d "_DEBUG" /d "_AFXDLL" /d "_AFXEXT"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 EuMemMgr.lib NewMgr.lib QCSocket.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\Lib\Debug"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg Imap
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Imap - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "imap___W"
# PROP BASE Intermediate_Dir "imap___W"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I ".\inc" /I ".\public\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "IMAP4" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\QCUtils\public\inc" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\SGISTL" /I "..\QCUtils\Public\Inc" /I "..\QCSocket\Inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\inc" /I ".\public\inc" /I "..\Eudora" /D "COMMERCIAL" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "THREADED" /D "IMAP4" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "..\OT501\Include" /i ".\inc" /i ".\public\inc" /d "NDEBUG" /d "_AFXDLL" /d "_AFXEXT" /d "WIN32"
# ADD RSC /l 0x409 /fo"Build\Release/Imap.res" /i "..\OT501\Include" /i ".\inc" /i ".\public\inc" /d "NDEBUG" /d "_AFXDLL" /d "_AFXEXT" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 QCUtils.lib EuMemMgr.lib NewMgr.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /libpath:"..\Lib\ProRelease"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 QCUtils.lib EuMemMgr.lib NewMgr.lib QCSocket.lib /nologo /base:"0x61000000" /subsystem:windows /dll /map /debug /machine:I386 /libpath:"..\Lib\Release"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel Imap
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Imap - Win32 Debug"
# Name "Imap - Win32 Release"
# Begin Group "Exported Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\public\inc\Imap.h
# End Source File
# End Group
# Begin Group "Private Header Files"

# PROP Default_Filter ""
# End Group
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\com_err.h
# End Source File
# Begin Source File

SOURCE=.\src\CramMd5.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CramMd5.h
# End Source File
# Begin Source File

SOURCE=.\src\gssapi.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gssapi.h
# End Source File
# Begin Source File

SOURCE=.\src\gssapi_generic.h
# End Source File
# Begin Source File

SOURCE=.\src\gssapi_krb5.h
# End Source File
# Begin Source File

SOURCE=.\src\Imap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Imap.rc
# End Source File
# Begin Source File

SOURCE=.\src\ImapDefs.h
# End Source File
# Begin Source File

SOURCE=.\src\imapfr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapfr.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapIface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapUtil.h
# End Source File
# Begin Source File

SOURCE=.\src\ImGui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImGui.h
# End Source File
# Begin Source File

SOURCE=.\src\krb5.h
# End Source File
# Begin Source File

SOURCE=.\src\krbv4.cpp
# End Source File
# Begin Source File

SOURCE=.\src\krbv4.h
# End Source File
# Begin Source File

SOURCE=.\src\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\src\md5.h
# End Source File
# Begin Source File

SOURCE=.\src\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc.h
# End Source File
# Begin Source File

SOURCE=.\src\mm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mm.h
# End Source File
# Begin Source File

SOURCE=.\src\MyTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MyTypes.h
# End Source File
# Begin Source File

SOURCE=.\src\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network.h
# End Source File
# Begin Source File

SOURCE=.\src\osdep.cpp
# End Source File
# Begin Source File

SOURCE=.\src\osdep.h
# End Source File
# Begin Source File

SOURCE=.\src\profile.h
# End Source File
# Begin Source File

SOURCE=.\src\Proto.cpp
# End Source File
# Begin Source File

SOURCE=.\src\proto.h
# End Source File
# Begin Source File

SOURCE=.\src\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\rfc822.cpp
# End Source File
# Begin Source File

SOURCE=.\src\rfc822.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp

!IF  "$(CFG)" == "Imap - Win32 Debug"

# ADD CPP /Yc"Stdafx.h"

!ELSEIF  "$(CFG)" == "Imap - Win32 Release"

# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\src\Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Stream.h
# End Source File
# Begin Source File

SOURCE=".\src\win-mac.h"
# End Source File
# End Group
# End Target
# End Project
