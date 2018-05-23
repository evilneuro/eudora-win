# Microsoft Developer Studio Project File - Name="QCUtils" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=QCUtils - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QCUtils.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QCUtils.mak" CFG="QCUtils - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QCUtils - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "QCUtils - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "QCUtils"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QCUtils - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "Regex\Include" /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\public\inc" /I ".\inc" /I "..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "inc" /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "..\OT501\Include" /i "inc" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 EuMemMgr.lib NewMgr.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\Lib\Debug"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg QCUtils
# End Special Build Tool

!ELSEIF  "$(CFG)" == "QCUtils - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "QCUtils_"
# PROP BASE Intermediate_Dir "QCUtils_"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\public\inc" /I ".\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "Regex\Include" /I "..\SGISTL" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I ".\public\inc" /I ".\inc" /I "..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "..\OT501\Include" /i "inc" /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "..\OT501\Include" /i "inc" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 EuMemMgr.lib NewMgr.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"..\Lib\ProRelease"
# ADD LINK32 EuMemMgr.lib NewMgr.lib /nologo /base:"0x60000000" /subsystem:windows /dll /map /debug /machine:I386 /libpath:"..\Lib\Release"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel QCUtils
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "QCUtils - Win32 Debug"
# Name "QCUtils - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jjfile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QCUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QCUtils.def
# End Source File
# Begin Source File

SOURCE=.\src\QCUtilsVer.rc
# End Source File
# Begin Source File

SOURCE=.\src\services.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD BASE CPP /Yc
# ADD CPP /Yc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\public\inc\AutoSerializedRegionIface.h
# End Source File
# Begin Source File

SOURCE=.\inc\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\debug.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\hsregex.h
# End Source File
# Begin Source File

SOURCE=.\inc\jjfile.h
# End Source File
# Begin Source File

SOURCE=.\inc\os.h
# End Source File
# Begin Source File

SOURCE=.\inc\QCError.h
# End Source File
# Begin Source File

SOURCE=.\public\inc\QCUtils.h
# End Source File
# Begin Source File

SOURCE=.\inc\services.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Regex"

# PROP Default_Filter ""
# Begin Group "Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Regex\Src\regcomp.cpp
# End Source File
# Begin Source File

SOURCE=.\Regex\Src\regerror.cpp
# End Source File
# Begin Source File

SOURCE=.\Regex\Src\regexec.cpp
# End Source File
# Begin Source File

SOURCE=.\Regex\Src\RegExpr.cpp
# End Source File
# Begin Source File

SOURCE=.\Regex\Src\regfree.cpp
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Regex\Include\ap_ctype.h
# End Source File
# Begin Source File

SOURCE=.\Regex\Include\cclass.h
# End Source File
# Begin Source File

SOURCE=.\Regex\Include\cname.h
# End Source File
# Begin Source File

SOURCE=.\Regex\Include\regex2.h
# End Source File
# Begin Source File

SOURCE=.\Regex\Include\regexutils.h
# End Source File
# End Group
# End Group
# End Target
# End Project
