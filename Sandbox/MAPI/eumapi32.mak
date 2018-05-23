# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Eumapi32 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Eumapi32 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Eumapi32 - Win32 Release" && "$(CFG)" !=\
 "Eumapi32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Eumapi32.mak" CFG="Eumapi32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Eumapi32 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Eumapi32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "Eumapi32 - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Eumapi32 - Win32 Release"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build/Release"
# PROP BASE Intermediate_Dir "Build/Release"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build/Release"
# PROP Intermediate_Dir "Build/Release"
OUTDIR=.\Build/Release
INTDIR=.\Build/Release

ALL : "$(OUTDIR)\Eumapi32.dll"

CLEAN : 
	-@erase "$(INTDIR)\addrdlg.obj"
	-@erase "$(INTDIR)\cmcfunc.obj"
	-@erase "$(INTDIR)\ddeclien.obj"
	-@erase "$(INTDIR)\ddestr.obj"
	-@erase "$(INTDIR)\eumapi.obj"
	-@erase "$(INTDIR)\eumapi.res"
	-@erase "$(INTDIR)\filedesc.obj"
	-@erase "$(INTDIR)\mapifunc.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\recip.obj"
	-@erase "$(INTDIR)\reciplst.obj"
	-@erase "$(INTDIR)\sesmgr.obj"
	-@erase "$(OUTDIR)\Eumapi32.dll"
	-@erase "$(OUTDIR)\Eumapi32.exp"
	-@erase "$(OUTDIR)\Eumapi32.lib"
	-@erase ".\FULL"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_MBCS" /FR /YX /c
# ADD CPP /nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_MBCS" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_USRDLL" /D "_WINDLL" /D "_MBCS" /Fp"$(INTDIR)/Eumapi32.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Build/Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/eumapi.res" /d "NDEBUG" /d "WIN32" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Eumapi32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 oldnames.lib ldllcew.lib /nologo /subsystem:windows /dll /map:"FULL" /machine:IX86
# ADD LINK32 /nologo /subsystem:windows /dll /map:"FULL" /machine:IX86 /stub:"mapistub.exe"
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Eumapi32.pdb" /map:"FULL" /machine:IX86 /def:".\eumapi32.def"\
 /stub:"mapistub.exe" /out:"$(OUTDIR)/Eumapi32.dll"\
 /implib:"$(OUTDIR)/Eumapi32.lib" 
DEF_FILE= \
	".\eumapi32.def"
LINK32_OBJS= \
	"$(INTDIR)\addrdlg.obj" \
	"$(INTDIR)\cmcfunc.obj" \
	"$(INTDIR)\ddeclien.obj" \
	"$(INTDIR)\ddestr.obj" \
	"$(INTDIR)\eumapi.obj" \
	"$(INTDIR)\eumapi.res" \
	"$(INTDIR)\filedesc.obj" \
	"$(INTDIR)\mapifunc.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\recip.obj" \
	"$(INTDIR)\reciplst.obj" \
	"$(INTDIR)\sesmgr.obj"

"$(OUTDIR)\Eumapi32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Eumapi32 - Win32 Debug"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build/Debug"
# PROP BASE Intermediate_Dir "Build/Debug"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build/Debug"
# PROP Intermediate_Dir "Build/Debug"
OUTDIR=.\Build/Debug
INTDIR=.\Build/Debug

ALL : "$(OUTDIR)\Eumapi32.dll"

CLEAN : 
	-@erase "$(INTDIR)\addrdlg.obj"
	-@erase "$(INTDIR)\cmcfunc.obj"
	-@erase "$(INTDIR)\ddeclien.obj"
	-@erase "$(INTDIR)\ddestr.obj"
	-@erase "$(INTDIR)\eumapi.obj"
	-@erase "$(INTDIR)\eumapi.res"
	-@erase "$(INTDIR)\filedesc.obj"
	-@erase "$(INTDIR)\mapifunc.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\recip.obj"
	-@erase "$(INTDIR)\reciplst.obj"
	-@erase "$(INTDIR)\sesmgr.obj"
	-@erase "$(OUTDIR)\Eumapi32.dll"
	-@erase "$(OUTDIR)\Eumapi32.exp"
	-@erase "$(OUTDIR)\Eumapi32.ilk"
	-@erase "$(OUTDIR)\Eumapi32.lib"
	-@erase "$(OUTDIR)\Eumapi32.pdb"
	-@erase ".\eumapi32.idb"
	-@erase ".\eumapi32.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_MBCS" /FR /YX /Fd"eumapi32.pdb" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_MBCS" /YX /Fd"eumapi32.pdb" /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_USRDLL" /D "_WINDLL" /D "_MBCS" /Fp"$(INTDIR)/Eumapi32.pch" /YX\
 /Fo"$(INTDIR)/" /Fd"eumapi32.pdb" /c 
CPP_OBJS=.\Build/Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "WIN32"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/eumapi.res" /d "_DEBUG" /d "WIN32" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Eumapi32.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 oldnames.lib ldllcew.lib winmm.lib /nologo /subsystem:windows /dll /map:"FULL" /debug /machine:IX86
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:IX86 /stub:"mapistub.exe"
# SUBTRACT LINK32 /map
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Eumapi32.pdb" /debug /machine:IX86 /def:".\eumapi32.def"\
 /stub:"mapistub.exe" /out:"$(OUTDIR)/Eumapi32.dll"\
 /implib:"$(OUTDIR)/Eumapi32.lib" 
DEF_FILE= \
	".\eumapi32.def"
LINK32_OBJS= \
	"$(INTDIR)\addrdlg.obj" \
	"$(INTDIR)\cmcfunc.obj" \
	"$(INTDIR)\ddeclien.obj" \
	"$(INTDIR)\ddestr.obj" \
	"$(INTDIR)\eumapi.obj" \
	"$(INTDIR)\eumapi.res" \
	"$(INTDIR)\filedesc.obj" \
	"$(INTDIR)\mapifunc.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\recip.obj" \
	"$(INTDIR)\reciplst.obj" \
	"$(INTDIR)\sesmgr.obj"

"$(OUTDIR)\Eumapi32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Eumapi32 - Win32 Release"
# Name "Eumapi32 - Win32 Debug"

!IF  "$(CFG)" == "Eumapi32 - Win32 Release"

!ELSEIF  "$(CFG)" == "Eumapi32 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\eumapi.cpp

"$(INTDIR)\eumapi.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\cmcfunc.cpp
DEP_CPP_CMCFU=\
	".\xcmc.h"\
	

"$(INTDIR)\cmcfunc.obj" : $(SOURCE) $(DEP_CPP_CMCFU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mapifunc.cpp
DEP_CPP_MAPIF=\
	"..\Eudora\eumapi.h"\
	

"$(INTDIR)\mapifunc.obj" : $(SOURCE) $(DEP_CPP_MAPIF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\eumapi32.def

!IF  "$(CFG)" == "Eumapi32 - Win32 Release"

!ELSEIF  "$(CFG)" == "Eumapi32 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\eumapi.rc

"$(INTDIR)\eumapi.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\reciplst.cpp
DEP_CPP_RECIP=\
	".\reciplst.h"\
	

"$(INTDIR)\reciplst.obj" : $(SOURCE) $(DEP_CPP_RECIP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\addrdlg.cpp
DEP_CPP_ADDRD=\
	".\addrdlg.h"\
	

"$(INTDIR)\addrdlg.obj" : $(SOURCE) $(DEP_CPP_ADDRD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\recip.cpp
DEP_CPP_RECIP_=\
	"..\Eudora\eumapi.h"\
	".\recip.h"\
	

"$(INTDIR)\recip.obj" : $(SOURCE) $(DEP_CPP_RECIP_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\sesmgr.cpp
DEP_CPP_SESMG=\
	"..\Eudora\eumapi.h"\
	".\message.h"\
	".\recip.h"\
	".\sesmgr.h"\
	

"$(INTDIR)\sesmgr.obj" : $(SOURCE) $(DEP_CPP_SESMG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\message.cpp
DEP_CPP_MESSA=\
	"..\Eudora\eumapi.h"\
	".\message.h"\
	".\recip.h"\
	

"$(INTDIR)\message.obj" : $(SOURCE) $(DEP_CPP_MESSA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=..\Eudora\ddestr.cpp
DEP_CPP_DDEST=\
	"..\Eudora\ddestr.h"\
	

"$(INTDIR)\ddestr.obj" : $(SOURCE) $(DEP_CPP_DDEST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\Eudora\ddeclien.cpp
DEP_CPP_DDECL=\
	"..\Eudora\ddeclien.h"\
	"..\Eudora\ddestr.h"\
	

"$(INTDIR)\ddeclien.obj" : $(SOURCE) $(DEP_CPP_DDECL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\filedesc.cpp
DEP_CPP_FILED=\
	"..\Eudora\eumapi.h"\
	".\filedesc.h"\
	

"$(INTDIR)\filedesc.obj" : $(SOURCE) $(DEP_CPP_FILED) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
