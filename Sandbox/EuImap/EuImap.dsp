# Microsoft Developer Studio Project File - Name="EuImap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=EuImap - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EuImap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EuImap.mak" CFG="EuImap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EuImap - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "EuImap - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "EuImap"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "EuImap - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /GX /Z7 /Od /I "..\eudora" /I "..\public\inc" /I "..\imapdll\public\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "IMAP4" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\SearchEngine\PublicInc" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\Eudora" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\public\inc" /I "..\OT501\Include" /I "..\Imapdll\public\inc" /I "..\QCSocket\inc" /D "_DEBUG" /D COMMERCIAL=1 /D "_MBCS" /D "OEMRESOURCE" /D "WIN32_COMPILE" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "IMAP4" /D "THREADED" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg EuImap
# End Special Build Tool

!ELSEIF  "$(CFG)" == "EuImap - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "EuImap__"
# PROP BASE Intermediate_Dir "EuImap__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\public\inc" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\Eudora" /I "..\PgHeaders" /I "..\Imapdll\public\inc" /D "NDEBUG" /D COMMERCIAL=1 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "OEMRESOURCE" /D "_AFXDLL" /D "WIN32_COMPILE" /D "IMAP4" /Yu"stdafx.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\INetSDK\Include" /I "..\Paige DLL\PGHEADER" /I "..\SearchEngine\PublicInc" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\Eudora" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\public\inc" /I "..\OT501\Include" /I "..\Imapdll\public\inc" /I "..\QCSocket\inc" /D EXPIRING=1 /D "NDEBUG" /D COMMERCIAL=1 /D "_MBCS" /D "OEMRESOURCE" /D "WIN32_COMPILE" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "IMAP4" /D "THREADED" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel EuImap
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "EuImap - Win32 Debug"
# Name "EuImap - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\append.cpp
# End Source File
# Begin Source File

SOURCE=.\src\append.h
# End Source File
# Begin Source File

SOURCE=.\src\imapacct.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapactl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapChecker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapConnectionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapConnectionMgr.h
# End Source File
# Begin Source File

SOURCE=.\src\Imapdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Imapdlg.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapDownload.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapFiltersd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapfol.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapgets.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapgets.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapGuiUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapHeader.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapJournal.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapLex822.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapLex822.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapMailMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapmime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapmlst.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapopt.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imapopt.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapPrefilterActs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapPreFiltersd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapPreFiltersd.h
# End Source File
# Begin Source File

SOURCE=.\src\imapresync.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapSettings.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapSum.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImapSUm.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapThread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Imapthread.h
# End Source File
# Begin Source File

SOURCE=.\src\ImapTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imaputil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImHexbin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImHexbin.h
# End Source File
# Begin Source File

SOURCE=.\src\ImSingle.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImSingle.h
# End Source File
# Begin Source File

SOURCE=.\src\ImUudecode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImUudecode.h
# End Source File
# Begin Source File

SOURCE=.\src\QCImapMailboxCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\searchutil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\searchutil.h
# End Source File
# Begin Source File

SOURCE=.\src\stdafx.cpp

!IF  "$(CFG)" == "EuImap - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "EuImap - Win32 Release"

# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Public Include Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\public\inc\imapacct.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imapactl.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapChecker.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapExports.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapFiltersd.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imapfol.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapGuiUtils.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapJournal.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\IMapMailMgr.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imapmime.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imapmlst.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapPrefilterActs.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imapresync.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\ImapTypes.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\imaputil.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\QCImapMailboxCommand.h
# End Source File
# End Group
# Begin Group "Private Include Files"

# PROP Default_Filter ""
# End Group
# End Target
# End Project
