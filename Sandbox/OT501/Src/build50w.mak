# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

!IF "$(CFG)" == ""
CFG=build50 - Win32 OT
!MESSAGE No configuration specified.  Defaulting to build50 - Win32 OT.
!ENDIF

!IF "$(CFG)" != "build50 - Win32 OT" && "$(CFG)" !=\
 "build50 - Win32 OT Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE
!MESSAGE NMAKE /f "build50w.mak" CFG="build50 - Win32 OT"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "build50 - Win32 OT" (based on "Win32 (x86) External Target")
!MESSAGE "build50 - Win32 OT Unicode" (based on "Win32 (x86) External Target")
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
# PROP Target_Last_Scanned "build50 - Win32 OT"

!IF  "$(CFG)" == "build50 - Win32 OT"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "build501"
# PROP BASE Intermediate_Dir "build501"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f build50.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "build50w.exe"
# PROP BASE Bsc_Name "build50.bsc"
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build501"
# PROP Intermediate_Dir "build501"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f build50.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "build50w.exe"
# PROP Bsc_Name "build50.bsc"
OUTDIR=.\build501
INTDIR=.\build501

ALL :

CLEAN :
        -@erase

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ELSEIF  "$(CFG)" == "build50 - Win32 OT Unicode"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "build502"
# PROP BASE Intermediate_Dir "build502"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f build50.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "build50w.exe"
# PROP BASE Bsc_Name "build50.bsc"
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build502"
# PROP Intermediate_Dir "build502"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f build50.mak unicode"
# PROP Rebuild_Opt "/a"
# PROP Target_File "build50w.exe"
# PROP Bsc_Name "build50.bsc"
OUTDIR=.\build502
INTDIR=.\build502

ALL :

CLEAN :
        -@erase

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ENDIF

################################################################################
# Begin Target

# Name "build50 - Win32 OT"
# Name "build50 - Win32 OT Unicode"

!IF  "$(CFG)" == "build50 - Win32 OT"

".\build50w.exe" :
   CD D:\OT12\Src
   NMAKE /f build50.mak

!ELSEIF  "$(CFG)" == "build50 - Win32 OT Unicode"

".\build50w.exe" :
   CD D:\OT12\Src
   NMAKE /f build50.mak unicode

!ENDIF

################################################################################
# Begin Source File

SOURCE=.\build50.mak

!IF  "$(CFG)" == "build50 - Win32 OT"

!ELSEIF  "$(CFG)" == "build50 - Win32 OT Unicode"

!ENDIF

# End Source File
# End Target
# End Project
################################################################################
