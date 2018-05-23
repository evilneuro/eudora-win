# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "swmapi32.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Release"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/swmapi32.exe $(OUTDIR)/swmapi32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32  
# ADD BASE CPP /nologo /G3 /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /c
# ADD CPP /nologo /G3 /MT /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"STDAFX.H" /c
CPP_PROJ=/nologo /G3 /MT /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"swmapi32.pch" /Yu"STDAFX.H"\
 /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"swmapi.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"swmapi32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/DIALOG.SBR \
	$(INTDIR)/swmapi.sbr \
	$(INTDIR)/aboutdlg.sbr \
	$(INTDIR)/MAPIINST.SBR \
	$(INTDIR)/STDAFX.SBR

$(OUTDIR)/swmapi32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 oldnames.lib toolhelp.lib /NOLOGO /STACK:0x10240 /SUBSYSTEM:windows /MACHINE:IX86
# ADD LINK32 oldnames.lib version.lib /NOLOGO /STACK:0x10240 /SUBSYSTEM:windows /MACHINE:IX86
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=oldnames.lib version.lib /NOLOGO /STACK:0x10240 /SUBSYSTEM:windows\
 /INCREMENTAL:no /PDB:$(OUTDIR)/"swmapi32.pdb" /MACHINE:IX86\
 /OUT:$(OUTDIR)/"swmapi32.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/DIALOG.OBJ \
	$(INTDIR)/swmapi.obj \
	$(INTDIR)/swmapi.res \
	$(INTDIR)/aboutdlg.obj \
	$(INTDIR)/MAPIINST.OBJ \
	$(INTDIR)/STDAFX.OBJ

$(OUTDIR)/swmapi32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/swmapi32.exe $(OUTDIR)/swmapi32.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /Gf /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Fd"SWMAPI.PDB" /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /Gf /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"STDAFX.H" /Fd"SWMAPI32.PDB" /c
CPP_PROJ=/nologo /MT /W3 /GX /Zi /Od /Gf /I ".." /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"swmapi32.pch" /Yu"STDAFX.H"\
 /Fo$(INTDIR)/ /Fd"SWMAPI32.PDB" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"swmapi.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"swmapi32.bsc" 
BSC32_SBRS= \
	$(INTDIR)/DIALOG.SBR \
	$(INTDIR)/swmapi.sbr \
	$(INTDIR)/aboutdlg.sbr \
	$(INTDIR)/MAPIINST.SBR \
	$(INTDIR)/STDAFX.SBR

$(OUTDIR)/swmapi32.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 oldnames.lib toolhelp.lib /NOLOGO /STACK:0x10240 /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
# ADD LINK32 oldnames.lib version.lib /NOLOGO /STACK:0x10240 /SUBSYSTEM:windows /DEBUG /MACHINE:IX86
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=oldnames.lib version.lib /NOLOGO /STACK:0x10240 /SUBSYSTEM:windows\
 /INCREMENTAL:yes /PDB:$(OUTDIR)/"swmapi32.pdb" /DEBUG /MACHINE:IX86\
 /OUT:$(OUTDIR)/"swmapi32.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/DIALOG.OBJ \
	$(INTDIR)/swmapi.obj \
	$(INTDIR)/swmapi.res \
	$(INTDIR)/aboutdlg.obj \
	$(INTDIR)/MAPIINST.OBJ \
	$(INTDIR)/STDAFX.OBJ

$(OUTDIR)/swmapi32.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\DIALOG.CPP
DEP_DIALO=\
	.\STDAFX.H\
	.\swmapi.h\
	.\DIALOG.H\
	\PCEUDORA\MFC\mapiinst.h\
	.\aboutdlg.h

$(INTDIR)/DIALOG.OBJ :  $(SOURCE)  $(DEP_DIALO) $(INTDIR) $(INTDIR)/STDAFX.OBJ

# End Source File
################################################################################
# Begin Source File

SOURCE=.\swmapi.cpp
DEP_SWMAP=\
	.\STDAFX.H\
	.\swmapi.h\
	.\DIALOG.H\
	.\aboutdlg.h\
	\PCEUDORA\MFC\mapiinst.h

$(INTDIR)/swmapi.obj :  $(SOURCE)  $(DEP_SWMAP) $(INTDIR) $(INTDIR)/STDAFX.OBJ

# End Source File
################################################################################
# Begin Source File

SOURCE=.\swmapi.RC
DEP_SWMAPI=\
	.\RES\swmapi.ico\
	.\RES\swmapi.rc2

$(INTDIR)/swmapi.res :  $(SOURCE)  $(DEP_SWMAPI) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\aboutdlg.cpp
DEP_ABOUT=\
	.\STDAFX.H\
	.\aboutdlg.h

$(INTDIR)/aboutdlg.obj :  $(SOURCE)  $(DEP_ABOUT) $(INTDIR)\
 $(INTDIR)/STDAFX.OBJ

# End Source File
################################################################################
# Begin Source File

SOURCE=\PCEUDORA\MFC\MAPIINST.CPP
DEP_MAPII=\
	\PCEUDORA\MFC\stdafx.h\
	\PCEUDORA\MFC\mapiinst.h

!IF  "$(CFG)" == "Win32 Release"

# SUBTRACT CPP /Yu

$(INTDIR)/MAPIINST.OBJ :  $(SOURCE)  $(DEP_MAPII) $(INTDIR)
   $(CPP) /nologo /G3 /MT /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# SUBTRACT CPP /Yu

$(INTDIR)/MAPIINST.OBJ :  $(SOURCE)  $(DEP_MAPII) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /Zi /Od /Gf /I ".." /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fo$(INTDIR)/ /Fd"SWMAPI32.PDB" /c\
  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_STDAF=\
	.\STDAFX.H

!IF  "$(CFG)" == "Win32 Release"

# ADD CPP /Yc"stdafx.h"

$(INTDIR)/STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /G3 /MT /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"swmapi32.pch" /Yc"stdafx.h"\
 /Fo$(INTDIR)/ /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Debug"

# ADD CPP /Yc"stdafx.h"

$(INTDIR)/STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /Zi /Od /Gf /I ".." /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_MBCS" /FR$(INTDIR)/ /Fp$(OUTDIR)/"swmapi32.pch" /Yc"stdafx.h"\
 /Fo$(INTDIR)/ /Fd"SWMAPI32.PDB" /c  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
