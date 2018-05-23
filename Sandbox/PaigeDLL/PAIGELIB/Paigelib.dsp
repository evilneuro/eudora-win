# Microsoft Developer Studio Project File - Name="Paigelib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Paigelib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Paigelib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Paigelib.mak" CFG="Paigelib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Paigelib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Paigelib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "Paigelib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "C:\Paige3.0"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O1 /I "C:\Paige3.0\pgheader" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_COMPILE" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Paigelib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "C:\Paige3.0"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "C:\Paige3.0\pgheader" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_COMPILE" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Paigelib - Win32 Release"
# Name "Paigelib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgbasics.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgboxes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgclipbd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgdefpar.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgdefstl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgedit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgembed.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgexceps.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgfiles.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgfonts.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\pgFrame.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pggrafx.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pghtext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pginit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgplatfo\Pgio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pglines.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pglists.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgplatfo\Pgmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgnboxes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgplatfo\Pgosutl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgpar.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgprint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgplatfo\Pgscrap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgscript.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgscroll.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgselect.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgshapes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgstlsht.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgstyles.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgsubref.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtables.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtabs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtxr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtxtblk.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtxtutl.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgtxtwid.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgutils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgplatfo\Pgwin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Paige3.0\Pgsource\Pgwrite.c
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
