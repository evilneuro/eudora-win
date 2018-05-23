# Microsoft Developer Studio Project File - Name="Paige32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Paige32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Paige32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Paige32.mak" CFG="Paige32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Paige32 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Paige32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Eudora/Paige DLL/PAIGE32", KTYBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Paige32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "..\PGHEADER" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "CREATE_MS_DLL" /D "WIN32_COMPILE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "\Paige Editor\Paige Library\pgheader"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x66500000" /subsystem:windows /dll /map /debug /machine:I386 /out:"..\Bin\Paige32.dll"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=.\PostProcessRel Paige32
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Paige32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\PGHEADER" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "CREATE_MS_DLL" /D "WIN32_COMPILE" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "..\Paige DLL\pgheader"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\bin\Paige32d.dll"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=.\PostProcessDbg Paige32d
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Paige32 - Win32 Release"
# Name "Paige32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\Pgsource\Pgbasics.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgboxes.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgclipbd.c
# End Source File
# Begin Source File

SOURCE=..\Pgdebug\Pgdebug.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgdefpar.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgdefstl.c
# End Source File
# Begin Source File

SOURCE=..\Pgplatfo\Pgdll32.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgedit.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgembed.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgexceps.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgfiles.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgfonts.c
# End Source File
# Begin Source File

SOURCE=..\PGSOURCE\PGFRAME.C
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pggrafx.c
# End Source File
# Begin Source File

SOURCE=..\PGHLEVEL\PGHLEVEL.C
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pghtext.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pginit.c
# End Source File
# Begin Source File

SOURCE=..\Pgplatfo\Pgio.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pglines.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pglists.c
# End Source File
# Begin Source File

SOURCE=..\Pgplatfo\Pgmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgmerge.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgnboxes.c
# End Source File
# Begin Source File

SOURCE=..\Pgplatfo\Pgosutl.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgpar.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgprint.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgread.c
# End Source File
# Begin Source File

SOURCE=..\Pgplatfo\Pgscrap.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgscript.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgscroll.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgselect.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgshapes.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgstlsht.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgstyles.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgsubref.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtables.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtabs.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtext.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtxr.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtxtblk.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtxtutl.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgtxtwid.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgutils.c
# End Source File
# Begin Source File

SOURCE=..\Pgplatfo\Pgwin.c
# End Source File
# Begin Source File

SOURCE=..\Pgsource\Pgwrite.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
