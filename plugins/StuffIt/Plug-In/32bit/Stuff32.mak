# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Stuff32 - Win32 Attacher Debug
!MESSAGE No configuration specified.  Defaulting to Stuff32 - Win32 Attacher\
 Debug.
!ENDIF 

!IF "$(CFG)" != "Stuff32 - Win32 Pro Debug" && "$(CFG)" !=\
 "Stuff32 - Win32 Pro Release" && "$(CFG)" != "Stuff32 - Win32 Expander Debug"\
 && "$(CFG)" != "Stuff32 - Win32 Expander Release" && "$(CFG)" !=\
 "Stuff32 - Win32 Attacher Debug" && "$(CFG)" !=\
 "Stuff32 - Win32 Attacher Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Stuff32.mak" CFG="Stuff32 - Win32 Attacher Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Stuff32 - Win32 Pro Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Stuff32 - Win32 Pro Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Stuff32 - Win32 Expander Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Stuff32 - Win32 Expander Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Stuff32 - Win32 Attacher Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Stuff32 - Win32 Attacher Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Stuff32 - Win32 Expander Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "attach__"
# PROP BASE Intermediate_Dir "attach__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ProDebug"
# PROP Intermediate_Dir "ProDebug"
# PROP Target_Dir ""
OUTDIR=.\ProDebug
INTDIR=.\ProDebug

ALL : "$(OUTDIR)\StuffitPro.DLL" "$(OUTDIR)\Stuff32.pch"\
 "$(OUTDIR)\Stuff32.bsc"

CLEAN : 
	-@erase "$(INTDIR)\AladdinProgDlg.obj"
	-@erase "$(INTDIR)\AladdinProgDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\Stuff32.obj"
	-@erase "$(INTDIR)\Stuff32.pch"
	-@erase "$(INTDIR)\Stuff32.res"
	-@erase "$(INTDIR)\Stuff32.sbr"
	-@erase "$(INTDIR)\Stuff32Tran.obj"
	-@erase "$(INTDIR)\Stuff32Tran.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Stuff32.bsc"
	-@erase "$(OUTDIR)\StuffitPro.DLL"
	-@erase "$(OUTDIR)\StuffitPro.exp"
	-@erase "$(OUTDIR)\StuffitPro.ilk"
	-@erase "$(OUTDIR)\StuffitPro.lib"
	-@erase "$(OUTDIR)\StuffitPro.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_COMMERCIAL_VER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_COMMERCIAL_VER" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ProDebug/
CPP_SBRS=.\ProDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Stuff32.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Stuff32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AladdinProgDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\Stuff32.sbr" \
	"$(INTDIR)\Stuff32Tran.sbr"

"$(OUTDIR)\Stuff32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"d:\mail\plugins\StuffitAttacher.DLL"
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"ProDebug\StuffitPro.DLL"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/StuffitPro.pdb" /debug /machine:I386 /nodefaultlib:"libcmt"\
 /def:".\Stuff32.def" /out:"$(OUTDIR)/StuffitPro.DLL"\
 /implib:"$(OUTDIR)/StuffitPro.lib" 
DEF_FILE= \
	".\Stuff32.def"
LINK32_OBJS= \
	"$(INTDIR)\AladdinProgDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Stuff32.obj" \
	"$(INTDIR)\Stuff32.res" \
	"$(INTDIR)\Stuff32Tran.obj" \
	".\StuffItEngine.lib"

"$(OUTDIR)\StuffitPro.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "attach_0"
# PROP BASE Intermediate_Dir "attach_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ProRelease"
# PROP Intermediate_Dir "ProRelease"
# PROP Target_Dir ""
OUTDIR=.\ProRelease
INTDIR=.\ProRelease

ALL : "$(OUTDIR)\StuffitPro.DLL" "$(OUTDIR)\Stuff32.pch"

CLEAN : 
	-@erase "$(INTDIR)\AladdinProgDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Stuff32.obj"
	-@erase "$(INTDIR)\Stuff32.pch"
	-@erase "$(INTDIR)\Stuff32.res"
	-@erase "$(INTDIR)\Stuff32Tran.obj"
	-@erase "$(OUTDIR)\StuffitPro.DLL"
	-@erase "$(OUTDIR)\StuffitPro.exp"
	-@erase "$(OUTDIR)\StuffitPro.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "_COMMERCIAL_VER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "_COMMERCIAL_VER" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /Fp"$(INTDIR)/Stuff32.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ProRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Stuff32.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Stuff32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /out:"Release\StuffitAttacher.dll"
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /out:"ProRelease\StuffitPro.DLL"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/StuffitPro.pdb" /machine:I386 /nodefaultlib:"libcmt"\
 /def:".\Stuff32.def" /out:"$(OUTDIR)/StuffitPro.DLL"\
 /implib:"$(OUTDIR)/StuffitPro.lib" 
DEF_FILE= \
	".\Stuff32.def"
LINK32_OBJS= \
	"$(INTDIR)\AladdinProgDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Stuff32.obj" \
	"$(INTDIR)\Stuff32.res" \
	"$(INTDIR)\Stuff32Tran.obj" \
	".\StuffItEngine.lib"

"$(OUTDIR)\StuffitPro.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "attach_1"
# PROP BASE Intermediate_Dir "attach_1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ExpanderDebug"
# PROP Intermediate_Dir "ExpanderDebug"
# PROP Target_Dir ""
OUTDIR=.\ExpanderDebug
INTDIR=.\ExpanderDebug

ALL : "$(OUTDIR)\StuffitExpander.DLL" "$(OUTDIR)\Stuff32.pch"\
 "$(OUTDIR)\Stuff32.bsc"

CLEAN : 
	-@erase "$(INTDIR)\AladdinProgDlg.obj"
	-@erase "$(INTDIR)\AladdinProgDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\Stuff32.obj"
	-@erase "$(INTDIR)\Stuff32.pch"
	-@erase "$(INTDIR)\Stuff32.res"
	-@erase "$(INTDIR)\Stuff32.sbr"
	-@erase "$(INTDIR)\Stuff32Tran.obj"
	-@erase "$(INTDIR)\Stuff32Tran.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Stuff32.bsc"
	-@erase "$(OUTDIR)\StuffitExpander.DLL"
	-@erase "$(OUTDIR)\StuffitExpander.exp"
	-@erase "$(OUTDIR)\StuffitExpander.ilk"
	-@erase "$(OUTDIR)\StuffitExpander.lib"
	-@erase "$(OUTDIR)\StuffitExpander.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_FREE_VER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_FREE_VER" /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ExpanderDebug/
CPP_SBRS=.\ExpanderDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Stuff32.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Stuff32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AladdinProgDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\Stuff32.sbr" \
	"$(INTDIR)\Stuff32Tran.sbr"

"$(OUTDIR)\Stuff32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"d:\mail\plugins\StuffitAttacher.DLL"
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"ExpanderDebug\StuffitExpander.DLL"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/StuffitExpander.pdb" /debug /machine:I386\
 /nodefaultlib:"libcmt" /def:".\Stuff32.def"\
 /out:"$(OUTDIR)/StuffitExpander.DLL" /implib:"$(OUTDIR)/StuffitExpander.lib" 
DEF_FILE= \
	".\Stuff32.def"
LINK32_OBJS= \
	"$(INTDIR)\AladdinProgDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Stuff32.obj" \
	"$(INTDIR)\Stuff32.res" \
	"$(INTDIR)\Stuff32Tran.obj" \
	".\StuffItEngine.lib"

"$(OUTDIR)\StuffitExpander.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "attach_2"
# PROP BASE Intermediate_Dir "attach_2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ExpanderRelease"
# PROP Intermediate_Dir "ExpanderRelease"
# PROP Target_Dir ""
OUTDIR=.\ExpanderRelease
INTDIR=.\ExpanderRelease

ALL : "$(OUTDIR)\StuffitExpander.DLL" "$(OUTDIR)\Stuff32.pch"

CLEAN : 
	-@erase "$(INTDIR)\AladdinProgDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Stuff32.obj"
	-@erase "$(INTDIR)\Stuff32.pch"
	-@erase "$(INTDIR)\Stuff32.res"
	-@erase "$(INTDIR)\Stuff32Tran.obj"
	-@erase "$(OUTDIR)\StuffitExpander.DLL"
	-@erase "$(OUTDIR)\StuffitExpander.exp"
	-@erase "$(OUTDIR)\StuffitExpander.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "_FREE_VER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "_FREE_VER" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /Fp"$(INTDIR)/Stuff32.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ExpanderRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Stuff32.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Stuff32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /out:"Release\StuffitAttacher.dll"
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /out:"ExpanderRelease\StuffitExpander.DLL"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/StuffitExpander.pdb" /machine:I386 /nodefaultlib:"libcmt"\
 /def:".\Stuff32.def" /out:"$(OUTDIR)/StuffitExpander.DLL"\
 /implib:"$(OUTDIR)/StuffitExpander.lib" 
DEF_FILE= \
	".\Stuff32.def"
LINK32_OBJS= \
	"$(INTDIR)\AladdinProgDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Stuff32.obj" \
	"$(INTDIR)\Stuff32.res" \
	"$(INTDIR)\Stuff32Tran.obj" \
	".\StuffItEngine.lib"

"$(OUTDIR)\StuffitExpander.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "attach__"
# PROP BASE Intermediate_Dir "attach__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AttacherDebug"
# PROP Intermediate_Dir "AttacherDebug"
# PROP Target_Dir ""
OUTDIR=.\AttacherDebug
INTDIR=.\AttacherDebug

ALL : "$(OUTDIR)\StuffitAttacher.DLL" "$(OUTDIR)\Stuff32.bsc"\
 "$(OUTDIR)\Stuff32.pch"

CLEAN : 
	-@erase "$(INTDIR)\AladdinProgDlg.obj"
	-@erase "$(INTDIR)\AladdinProgDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\Stuff32.obj"
	-@erase "$(INTDIR)\Stuff32.pch"
	-@erase "$(INTDIR)\Stuff32.res"
	-@erase "$(INTDIR)\Stuff32.sbr"
	-@erase "$(INTDIR)\Stuff32Tran.obj"
	-@erase "$(INTDIR)\Stuff32Tran.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Stuff32.bsc"
	-@erase "$(OUTDIR)\StuffitAttacher.DLL"
	-@erase "$(OUTDIR)\StuffitAttacher.exp"
	-@erase "$(OUTDIR)\StuffitAttacher.ilk"
	-@erase "$(OUTDIR)\StuffitAttacher.lib"
	-@erase "$(OUTDIR)\StuffitAttacher.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_FREE_VER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_OEM_VER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_OEM_VER" /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\AttacherDebug/
CPP_SBRS=.\AttacherDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Stuff32.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Stuff32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AladdinProgDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\Stuff32.sbr" \
	"$(INTDIR)\Stuff32Tran.sbr"

"$(OUTDIR)\Stuff32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"d:\mail\plugins\StuffitExpander.DLL"
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"AttacherDebug\StuffitAttacher.DLL"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/StuffitAttacher.pdb" /debug /machine:I386\
 /nodefaultlib:"libcmt" /def:".\Stuff32.def"\
 /out:"$(OUTDIR)/StuffitAttacher.DLL" /implib:"$(OUTDIR)/StuffitAttacher.lib" 
DEF_FILE= \
	".\Stuff32.def"
LINK32_OBJS= \
	"$(INTDIR)\AladdinProgDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Stuff32.obj" \
	"$(INTDIR)\Stuff32.res" \
	"$(INTDIR)\Stuff32Tran.obj" \
	".\StuffItEngine.lib"

"$(OUTDIR)\StuffitAttacher.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "attach_0"
# PROP BASE Intermediate_Dir "attach_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "AttacherRelease"
# PROP Intermediate_Dir "AttacherRelease"
# PROP Target_Dir ""
OUTDIR=.\AttacherRelease
INTDIR=.\AttacherRelease

ALL : "$(OUTDIR)\StuffitAttacher.DLL" "$(OUTDIR)\Stuff32.pch"

CLEAN : 
	-@erase "$(INTDIR)\AladdinProgDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\Stuff32.obj"
	-@erase "$(INTDIR)\Stuff32.pch"
	-@erase "$(INTDIR)\Stuff32.res"
	-@erase "$(INTDIR)\Stuff32Tran.obj"
	-@erase "$(OUTDIR)\StuffitAttacher.DLL"
	-@erase "$(OUTDIR)\StuffitAttacher.exp"
	-@erase "$(OUTDIR)\StuffitAttacher.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "_FREE_VER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "_OEM_VER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "_OEM_VER" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /Fp"$(INTDIR)/Stuff32.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\AttacherRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Stuff32.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Stuff32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /out:"ExpanderRelease\StuffitExpander.DLL"
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmt" /out:"AttacherRelease\StuffitAttacher.DLL"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/StuffitAttacher.pdb" /machine:I386 /nodefaultlib:"libcmt"\
 /def:".\Stuff32.def" /out:"$(OUTDIR)/StuffitAttacher.DLL"\
 /implib:"$(OUTDIR)/StuffitAttacher.lib" 
DEF_FILE= \
	".\Stuff32.def"
LINK32_OBJS= \
	"$(INTDIR)\AladdinProgDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Stuff32.obj" \
	"$(INTDIR)\Stuff32.res" \
	"$(INTDIR)\Stuff32Tran.obj" \
	".\StuffItEngine.lib"

"$(OUTDIR)\StuffitAttacher.DLL" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "Stuff32 - Win32 Pro Debug"
# Name "Stuff32 - Win32 Pro Release"
# Name "Stuff32 - Win32 Expander Debug"
# Name "Stuff32 - Win32 Expander Release"
# Name "Stuff32 - Win32 Attacher Debug"
# Name "Stuff32 - Win32 Attacher Release"

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Stuff32.cpp
DEP_CPP_STUFF=\
	".\StdAfx.h"\
	".\Stuff32.h"\
	

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"


"$(INTDIR)\Stuff32.obj" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"

"$(INTDIR)\Stuff32.sbr" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"


"$(INTDIR)\Stuff32.obj" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"


"$(INTDIR)\Stuff32.obj" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"

"$(INTDIR)\Stuff32.sbr" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"


"$(INTDIR)\Stuff32.obj" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"


"$(INTDIR)\Stuff32.obj" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"

"$(INTDIR)\Stuff32.sbr" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"


"$(INTDIR)\Stuff32.obj" : $(SOURCE) $(DEP_CPP_STUFF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Stuff32.def

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_COMMERCIAL_VER" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/Stuff32.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Stuff32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "_COMMERCIAL_VER" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /Fp"$(INTDIR)/Stuff32.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Stuff32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_FREE_VER" /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/Stuff32.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Stuff32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "_FREE_VER" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /Fp"$(INTDIR)/Stuff32.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Stuff32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_OEM_VER" /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/Stuff32.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Stuff32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "_OEM_VER" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL"\
 /Fp"$(INTDIR)/Stuff32.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Stuff32.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Stuff32.rc
DEP_RSC_STUFF3=\
	".\Res\Plugin.ico"\
	

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"


"$(INTDIR)\Stuff32.res" : $(SOURCE) $(DEP_RSC_STUFF3) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"


"$(INTDIR)\Stuff32.res" : $(SOURCE) $(DEP_RSC_STUFF3) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"


"$(INTDIR)\Stuff32.res" : $(SOURCE) $(DEP_RSC_STUFF3) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"


"$(INTDIR)\Stuff32.res" : $(SOURCE) $(DEP_RSC_STUFF3) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"


"$(INTDIR)\Stuff32.res" : $(SOURCE) $(DEP_RSC_STUFF3) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"


"$(INTDIR)\Stuff32.res" : $(SOURCE) $(DEP_RSC_STUFF3) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Stuff32Tran.CPP

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

DEP_CPP_STUFF32=\
	".\AladdinProgDlg.h"\
	".\ems-win.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\Stuff32.h"\
	
NODEP_CPP_STUFF32=\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\Stuff32Tran.obj" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"

"$(INTDIR)\Stuff32Tran.sbr" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

DEP_CPP_STUFF32=\
	".\AladdinProgDlg.h"\
	".\ems-win.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\Stuff32.h"\
	
NODEP_CPP_STUFF32=\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\Stuff32Tran.obj" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

DEP_CPP_STUFF32=\
	".\AladdinProgDlg.h"\
	".\ems-win.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\Stuff32.h"\
	
NODEP_CPP_STUFF32=\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\Stuff32Tran.obj" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"

"$(INTDIR)\Stuff32Tran.sbr" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

DEP_CPP_STUFF32=\
	".\AladdinProgDlg.h"\
	".\ems-win.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\Stuff32.h"\
	
NODEP_CPP_STUFF32=\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\Stuff32Tran.obj" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

DEP_CPP_STUFF32=\
	".\AladdinProgDlg.h"\
	".\ems-win.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\Stuff32.h"\
	
NODEP_CPP_STUFF32=\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\Stuff32Tran.obj" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"

"$(INTDIR)\Stuff32Tran.sbr" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

DEP_CPP_STUFF32=\
	".\AladdinProgDlg.h"\
	".\ems-win.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\Stuff32.h"\
	
NODEP_CPP_STUFF32=\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\Stuff32Tran.obj" : $(SOURCE) $(DEP_CPP_STUFF32) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\AladdinProgDlg.cpp

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

DEP_CPP_ALADD=\
	".\AladdinProgDlg.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\StdAfx.h"\
	
NODEP_CPP_ALADD=\
	".\attach.h"\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\AladdinProgDlg.obj" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"

"$(INTDIR)\AladdinProgDlg.sbr" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

DEP_CPP_ALADD=\
	".\AladdinProgDlg.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\StdAfx.h"\
	
NODEP_CPP_ALADD=\
	".\attach.h"\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\AladdinProgDlg.obj" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

DEP_CPP_ALADD=\
	".\AladdinProgDlg.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\StdAfx.h"\
	
NODEP_CPP_ALADD=\
	".\attach.h"\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\AladdinProgDlg.obj" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"

"$(INTDIR)\AladdinProgDlg.sbr" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

DEP_CPP_ALADD=\
	".\AladdinProgDlg.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\StdAfx.h"\
	
NODEP_CPP_ALADD=\
	".\attach.h"\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\AladdinProgDlg.obj" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

DEP_CPP_ALADD=\
	".\AladdinProgDlg.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\StdAfx.h"\
	
NODEP_CPP_ALADD=\
	".\attach.h"\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\AladdinProgDlg.obj" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"

"$(INTDIR)\AladdinProgDlg.sbr" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

DEP_CPP_ALADD=\
	".\AladdinProgDlg.h"\
	".\Engine.h"\
	".\EnginePreamble.h"\
	".\EngineTypes.h"\
	".\Platform.h"\
	".\StdAfx.h"\
	
NODEP_CPP_ALADD=\
	".\attach.h"\
	".\sepream.h"\
	".\setypes.h"\
	

"$(INTDIR)\AladdinProgDlg.obj" : $(SOURCE) $(DEP_CPP_ALADD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StuffItEngine.lib

!IF  "$(CFG)" == "Stuff32 - Win32 Pro Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Pro Release"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Expander Release"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Debug"

!ELSEIF  "$(CFG)" == "Stuff32 - Win32 Attacher Release"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
