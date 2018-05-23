# Microsoft Developer Studio Generated NMAKE File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=OTlib50 - Win32 OTA50UD
!MESSAGE No configuration specified.  Defaulting to OTlib50 - Win32 OTA50UD.
!ENDIF 

!IF "$(CFG)" != "OTlib50 - Win32 OT50R" && "$(CFG)" != "OTlib50 - Win32 OT50D"\
 && "$(CFG)" != "OTlib50 - Win32 OTA50R" && "$(CFG)" != "OTlib50 - Win32 OTA50D"\
 && "$(CFG)" != "OTlib50 - Win32 OT50UD" && "$(CFG)" != "OTlib50 - Win32 OT50UR"\
 && "$(CFG)" != "OTlib50 - Win32 OTA50UD" && "$(CFG)" !=\
 "OTlib50 - Win32 OTA50UR"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "otlib50.mak" CFG="OTlib50 - Win32 OTA50UD"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OTlib50 - Win32 OT50R" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OT50D" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OTA50R" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OTA50D" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OT50UD" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OT50UR" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OTA50UD" (based on "Win32 (x86) Static Library")
!MESSAGE "OTlib50 - Win32 OTA50UR" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "OTlib50 - Win32 OTA50UR"
CPP=cl.exe

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "OT50R"
# PROP Intermediate_Dir "OT50R"
OUTDIR=.\OT50R
INTDIR=.\OT50R

ALL : "$(OUTDIR)\OT50R.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OT50R.pch"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OT50R.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/OT50.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MT /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"OT50R/OT50R.pch" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MT /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)/OT50R.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\OT50R/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"OT50R\OT50R.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OT50R.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"OT50R\OT50R.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OT50R.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OT50R.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "OT50D"
# PROP Intermediate_Dir "OT50D"
OUTDIR=.\OT50D
INTDIR=.\OT50D

ALL : "$(OUTDIR)\OT50D.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OT50D.idb"
	-@erase "$(INTDIR)\OT50D.pch"
	-@erase "$(INTDIR)\OT50D.pdb"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OT50D.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"OT50D/OT50D.pch" /Yu"stdafx.h" /Fd"OT50D/OT50D.pdb" /c
CPP_PROJ=/nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/OT50D.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/OT50D.pdb"\
 /c 
CPP_OBJS=.\OT50D/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"OT50D/OT50D.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OT50D.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"OT50D\OT50D.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OT50D.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OT50D.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Win32_Af"
# PROP BASE Intermediate_Dir "Win32_Af"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "OTA50R"
# PROP Intermediate_Dir "OTA50R"
OUTDIR=.\OTA50R
INTDIR=.\OTA50R

ALL : "$(OUTDIR)\OTA50R.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OTA50R.pch"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OTA50R.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

$(OUTDIR)/OTA50.bsc : $(OUTDIR)  $(BSC32_SBRS)
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"OTlib50R/OTlib50R.pch" /YX"stdafx.h" /c
# ADD CPP /nologo /MD /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"OTA50R/OTA50R.pch" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W4 /GR /GX /Zi /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/OTA50R.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" \
 /Fd"$(INTDIR)/OTA50R.pdb" /c 
CPP_OBJS=.\OTA50R/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"OTA50R/OTA50R.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OTA50R.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"OTlib50R\OTlib50R.lib"
# ADD LIB32 /nologo /out:"OTA50R\OTA50R.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OTA50R.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OTA50R.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_A0"
# PROP BASE Intermediate_Dir "Win32_A0"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "OTA50D"
# PROP Intermediate_Dir "OTA50D"
OUTDIR=.\OTA50D
INTDIR=.\OTA50D

ALL : "$(OUTDIR)\OTA50D.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OTA50D.idb"
	-@erase "$(INTDIR)\OTA50D.pch"
	-@erase "$(INTDIR)\OTA50D.pdb"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OTA50D.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W4 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Fp"OTlib50D/OTlib50D.pch" /YX"stdafx.h" /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"OTA50D/OTA50D.pch" /Yu"stdafx.h" /Fd"OTA50D/OTA50D.pdb" /c
CPP_PROJ=/nologo /MDd /W4 /GR /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/OTA50D.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/OTA50D.pdb" /c 
CPP_OBJS=.\OTA50D/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"OTlib50D/OTlib50D.bsc"
# ADD BSC32 /nologo /o"OTA50D/OTA50D.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OTA50D.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"OTlib50D\OTlib50D.lib"
# ADD LIB32 /nologo /out:"OTA50D\OTA50D.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OTA50D.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OTA50D.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "OTlib50_"
# PROP BASE Intermediate_Dir "OTlib50_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "OT50UD"
# PROP Intermediate_Dir "OT50UD"
# PROP Target_Dir ""
OUTDIR=.\OT50UD
INTDIR=.\OT50UD

ALL : "$(OUTDIR)\OT50UD.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OT50UD.idb"
	-@erase "$(INTDIR)\OT50UD.pch"
	-@erase "$(INTDIR)\OT50UD.pdb"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OT50UD.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\INCLUDE" /I ".\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"OT50D/OT50D.pch" /Yu"stdafx.h" /Fd"OT50D/OT50.pdb" /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"OT50UD/OT50UD.pch" /Yu"stdafx.h" /Fd"OT50UD/OT50UD.pdb" /c
CPP_PROJ=/nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE"\
 /Fp"$(INTDIR)/OT50UD.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/OT50UD.pdb" /c 
CPP_OBJS=.\OT50UD/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"OT50D/OT50D.bsc"
# ADD BSC32 /nologo /o"OT50UD/OT50UD.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OT50UD.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"OT50D\OT50D.lib"
# ADD LIB32 /nologo /out:"OT50UD\OT50UD.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OT50UD.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OT50UD.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "OTlib500"
# PROP BASE Intermediate_Dir "OTlib500"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "OT50UR"
# PROP Intermediate_Dir "OT50UR"
# PROP Target_Dir ""
OUTDIR=.\OT50UR
INTDIR=.\OT50UR

ALL : "$(OUTDIR)\OT50UR.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OT50UR.pch"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OT50UR.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W4 /GX /O1 /I "..\INCLUDE" /I ".\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"OT50R/OT50R.pch" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"OT50UR/OT50UR.pch" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MT /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"$(INTDIR)/OT50UR.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\OT50UR/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"OT50R/OT50.bsc"
# ADD BSC32 /nologo /o"OT50UR/OT50UR.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OT50UR.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"OT50R\OT50R.lib"
# ADD LIB32 /nologo /out:"OT50UR\OT50UR.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OT50UR.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OT50UR.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "OTlib501"
# PROP BASE Intermediate_Dir "OTlib501"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "OTA50UD"
# PROP Intermediate_Dir "OTA50UD"
# PROP Target_Dir ""
OUTDIR=.\OTA50UD
INTDIR=.\OTA50UD

ALL : "$(OUTDIR)\OTA50UD.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OTA50UD.idb"
	-@erase "$(INTDIR)\OTA50UD.pch"
	-@erase "$(INTDIR)\OTA50UD.pdb"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OTA50UD.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "..\INCLUDE" /I ".\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"OTA50D/OTA50D.pch" /Yu"stdafx.h" /Fd"OTA50D/OTA50D.pdb" /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /Fp"OTA50UD/OTA50UD.pch" /Yu"stdafx.h" /Fd"OTA50UD/OTA50UD.pdb" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE"\
 /Fp"$(INTDIR)/OTA50UD.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/OTA50UD.pdb" /c 
CPP_OBJS=.\OTA50UD/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"OTA50D/OTA50D.bsc"
# ADD BSC32 /nologo /o"OTA50UD/OTA50UD.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OTA50UD.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"OTA50D\OTA50D.lib"
# ADD LIB32 /nologo /out:"OTA50UD\OTA50UD.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OTA50UD.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OTA50UD.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "OTlib502"
# PROP BASE Intermediate_Dir "OTlib502"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "OTA50UR"
# PROP Intermediate_Dir "OTA50UR"
# PROP Target_Dir ""
OUTDIR=.\OTA50UR
INTDIR=.\OTA50UR

ALL : "$(OUTDIR)\OTA50UR.lib"

CLEAN : 
	-@erase "$(INTDIR)\Adler32.obj"
	-@erase "$(INTDIR)\Blackbox.obj"
	-@erase "$(INTDIR)\browedit.obj"
	-@erase "$(INTDIR)\CalcBtn.obj"
	-@erase "$(INTDIR)\Calendar.obj"
	-@erase "$(INTDIR)\CalTitle.obj"
	-@erase "$(INTDIR)\compfile.obj"
	-@erase "$(INTDIR)\Compress.obj"
	-@erase "$(INTDIR)\Crc32.obj"
	-@erase "$(INTDIR)\Cryptfil.obj"
	-@erase "$(INTDIR)\curredit.obj"
	-@erase "$(INTDIR)\DayBar.obj"
	-@erase "$(INTDIR)\DayBox.obj"
	-@erase "$(INTDIR)\Deflate.obj"
	-@erase "$(INTDIR)\DropEdit.obj"
	-@erase "$(INTDIR)\DTButton.obj"
	-@erase "$(INTDIR)\DTChange.obj"
	-@erase "$(INTDIR)\DTCtrl.obj"
	-@erase "$(INTDIR)\DTGadget.obj"
	-@erase "$(INTDIR)\DTList.obj"
	-@erase "$(INTDIR)\DTNumerc.obj"
	-@erase "$(INTDIR)\DTParser.obj"
	-@erase "$(INTDIR)\DTSpin.obj"
	-@erase "$(INTDIR)\DTStatic.obj"
	-@erase "$(INTDIR)\fdichild.obj"
	-@erase "$(INTDIR)\fdimain.obj"
	-@erase "$(INTDIR)\filesys.obj"
	-@erase "$(INTDIR)\Gzio.obj"
	-@erase "$(INTDIR)\Infblock.obj"
	-@erase "$(INTDIR)\Infcodes.obj"
	-@erase "$(INTDIR)\Inffast.obj"
	-@erase "$(INTDIR)\Inflate.obj"
	-@erase "$(INTDIR)\Inftrees.obj"
	-@erase "$(INTDIR)\Infutil.obj"
	-@erase "$(INTDIR)\Jcapimin.obj"
	-@erase "$(INTDIR)\Jcapistd.obj"
	-@erase "$(INTDIR)\Jcarith.obj"
	-@erase "$(INTDIR)\Jccoefct.obj"
	-@erase "$(INTDIR)\Jccolor.obj"
	-@erase "$(INTDIR)\Jcdctmgr.obj"
	-@erase "$(INTDIR)\Jchuff.obj"
	-@erase "$(INTDIR)\Jcinit.obj"
	-@erase "$(INTDIR)\Jcmainct.obj"
	-@erase "$(INTDIR)\Jcmarker.obj"
	-@erase "$(INTDIR)\Jcmaster.obj"
	-@erase "$(INTDIR)\Jcomapi.obj"
	-@erase "$(INTDIR)\Jcparam.obj"
	-@erase "$(INTDIR)\Jcphuff.obj"
	-@erase "$(INTDIR)\Jcprepct.obj"
	-@erase "$(INTDIR)\Jcsample.obj"
	-@erase "$(INTDIR)\Jctrans.obj"
	-@erase "$(INTDIR)\Jdapimin.obj"
	-@erase "$(INTDIR)\Jdapistd.obj"
	-@erase "$(INTDIR)\Jdarith.obj"
	-@erase "$(INTDIR)\Jdatadst.obj"
	-@erase "$(INTDIR)\Jdatasrc.obj"
	-@erase "$(INTDIR)\Jdcoefct.obj"
	-@erase "$(INTDIR)\Jdcolor.obj"
	-@erase "$(INTDIR)\Jddctmgr.obj"
	-@erase "$(INTDIR)\Jdhuff.obj"
	-@erase "$(INTDIR)\Jdinput.obj"
	-@erase "$(INTDIR)\Jdmainct.obj"
	-@erase "$(INTDIR)\Jdmarker.obj"
	-@erase "$(INTDIR)\Jdmaster.obj"
	-@erase "$(INTDIR)\Jdmerge.obj"
	-@erase "$(INTDIR)\Jdphuff.obj"
	-@erase "$(INTDIR)\Jdpostct.obj"
	-@erase "$(INTDIR)\Jdsample.obj"
	-@erase "$(INTDIR)\Jdtrans.obj"
	-@erase "$(INTDIR)\Jerror.obj"
	-@erase "$(INTDIR)\Jfdctflt.obj"
	-@erase "$(INTDIR)\Jfdctfst.obj"
	-@erase "$(INTDIR)\Jfdctint.obj"
	-@erase "$(INTDIR)\Jidctflt.obj"
	-@erase "$(INTDIR)\Jidctfst.obj"
	-@erase "$(INTDIR)\Jidctint.obj"
	-@erase "$(INTDIR)\Jidctred.obj"
	-@erase "$(INTDIR)\Jmemansi.obj"
	-@erase "$(INTDIR)\Jmemmgr.obj"
	-@erase "$(INTDIR)\Jquant1.obj"
	-@erase "$(INTDIR)\Jquant2.obj"
	-@erase "$(INTDIR)\Jutils.obj"
	-@erase "$(INTDIR)\lstbxclr.obj"
	-@erase "$(INTDIR)\lstbxedt.obj"
	-@erase "$(INTDIR)\OTA50UR.pch"
	-@erase "$(INTDIR)\Panel.obj"
	-@erase "$(INTDIR)\Panview.obj"
	-@erase "$(INTDIR)\Panwnd.obj"
	-@erase "$(INTDIR)\Rotor.obj"
	-@erase "$(INTDIR)\sbarcore.obj"
	-@erase "$(INTDIR)\sbarcust.obj"
	-@erase "$(INTDIR)\sbardlg.obj"
	-@erase "$(INTDIR)\sbardock.obj"
	-@erase "$(INTDIR)\sbarmgr.obj"
	-@erase "$(INTDIR)\sbarstat.obj"
	-@erase "$(INTDIR)\Sbartool.obj"
	-@erase "$(INTDIR)\SCutAccl.obj"
	-@erase "$(INTDIR)\SCutCmds.obj"
	-@erase "$(INTDIR)\SCutDlg.obj"
	-@erase "$(INTDIR)\SCutKDlg.obj"
	-@erase "$(INTDIR)\sdockcnt.obj"
	-@erase "$(INTDIR)\sdocksta.obj"
	-@erase "$(INTDIR)\secaux.obj"
	-@erase "$(INTDIR)\SECBMPDG.OBJ"
	-@erase "$(INTDIR)\secbtns.obj"
	-@erase "$(INTDIR)\SecCalc.obj"
	-@erase "$(INTDIR)\seccomp.obj"
	-@erase "$(INTDIR)\secdib.obj"
	-@erase "$(INTDIR)\secdsp.obj"
	-@erase "$(INTDIR)\secgif.obj"
	-@erase "$(INTDIR)\secimage.obj"
	-@erase "$(INTDIR)\secjpeg.obj"
	-@erase "$(INTDIR)\secmedit.obj"
	-@erase "$(INTDIR)\secpcx.obj"
	-@erase "$(INTDIR)\secrand.obj"
	-@erase "$(INTDIR)\secreg.obj"
	-@erase "$(INTDIR)\SECSPLSH.OBJ"
	-@erase "$(INTDIR)\sectarga.obj"
	-@erase "$(INTDIR)\sectiff.obj"
	-@erase "$(INTDIR)\sectlf.obj"
	-@erase "$(INTDIR)\sectnbmp.obj"
	-@erase "$(INTDIR)\sectndc.obj"
	-@erase "$(INTDIR)\sectndlg.obj"
	-@erase "$(INTDIR)\sectndoc.obj"
	-@erase "$(INTDIR)\sectnvw.obj"
	-@erase "$(INTDIR)\sectnwin.obj"
	-@erase "$(INTDIR)\SECTOD.OBJ"
	-@erase "$(INTDIR)\secwb.obj"
	-@erase "$(INTDIR)\secwell.obj"
	-@erase "$(INTDIR)\secwsmgr.obj"
	-@erase "$(INTDIR)\slocale.obj"
	-@erase "$(INTDIR)\slstctip.obj"
	-@erase "$(INTDIR)\slstctl.obj"
	-@erase "$(INTDIR)\Stdafx.obj"
	-@erase "$(INTDIR)\swinfrm.obj"
	-@erase "$(INTDIR)\swinmdi.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabctrl3.obj"
	-@erase "$(INTDIR)\tabctrlb.obj"
	-@erase "$(INTDIR)\tabwnd.obj"
	-@erase "$(INTDIR)\tabwnd3.obj"
	-@erase "$(INTDIR)\tabwndb.obj"
	-@erase "$(INTDIR)\tbarcust.obj"
	-@erase "$(INTDIR)\tbarmgr.obj"
	-@erase "$(INTDIR)\tbarpage.obj"
	-@erase "$(INTDIR)\tbarsdlg.obj"
	-@erase "$(INTDIR)\tbartrck.obj"
	-@erase "$(INTDIR)\tbtn2prt.obj"
	-@erase "$(INTDIR)\tbtncmbo.obj"
	-@erase "$(INTDIR)\tbtnstd.obj"
	-@erase "$(INTDIR)\tbtnwnd.obj"
	-@erase "$(INTDIR)\Tif_aux.obj"
	-@erase "$(INTDIR)\Tif_ccit.obj"
	-@erase "$(INTDIR)\Tif_clos.obj"
	-@erase "$(INTDIR)\Tif_comp.obj"
	-@erase "$(INTDIR)\Tif_dir.obj"
	-@erase "$(INTDIR)\Tif_diri.obj"
	-@erase "$(INTDIR)\Tif_dirr.obj"
	-@erase "$(INTDIR)\Tif_dirw.obj"
	-@erase "$(INTDIR)\Tif_dump.obj"
	-@erase "$(INTDIR)\Tif_erro.obj"
	-@erase "$(INTDIR)\Tif_fax3.obj"
	-@erase "$(INTDIR)\Tif_fax4.obj"
	-@erase "$(INTDIR)\Tif_flus.obj"
	-@erase "$(INTDIR)\Tif_geti.obj"
	-@erase "$(INTDIR)\Tif_jpeg.obj"
	-@erase "$(INTDIR)\Tif_lzw.obj"
	-@erase "$(INTDIR)\Tif_msdo.obj"
	-@erase "$(INTDIR)\Tif_open.obj"
	-@erase "$(INTDIR)\Tif_pack.obj"
	-@erase "$(INTDIR)\Tif_prin.obj"
	-@erase "$(INTDIR)\Tif_read.obj"
	-@erase "$(INTDIR)\Tif_stri.obj"
	-@erase "$(INTDIR)\Tif_swab.obj"
	-@erase "$(INTDIR)\Tif_thun.obj"
	-@erase "$(INTDIR)\Tif_tile.obj"
	-@erase "$(INTDIR)\Tif_vers.obj"
	-@erase "$(INTDIR)\Tif_warn.obj"
	-@erase "$(INTDIR)\Tif_writ.obj"
	-@erase "$(INTDIR)\TrCtlX.obj"
	-@erase "$(INTDIR)\TreeNode.obj"
	-@erase "$(INTDIR)\Trees.obj"
	-@erase "$(INTDIR)\Uncompr.obj"
	-@erase "$(INTDIR)\usertool.obj"
	-@erase "$(INTDIR)\usrtldlg.obj"
	-@erase "$(INTDIR)\Zoomview.obj"
	-@erase "$(INTDIR)\Zutil.obj"
	-@erase "$(OUTDIR)\OTA50UR.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W4 /GX /O1 /I "..\INCLUDE" /I ".\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"OTA50R/OTA50R.pch" /Yu"stdafx.h" /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE" /Fp"OTA50UR/OTA50UR.pch" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE"\
 /Fp"$(INTDIR)/OTA50UR.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\OTA50UR/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"OTA50R/OTA50.bsc"
# ADD BSC32 /nologo /o"OTA50UR/OTA50UR.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/OTA50UR.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"OTA50R\OTA50R.lib"
# ADD LIB32 /nologo /out:"OTA50UR\OTA50UR.lib"
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/OTA50UR.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Adler32.obj" \
	"$(INTDIR)\Blackbox.obj" \
	"$(INTDIR)\browedit.obj" \
	"$(INTDIR)\CalcBtn.obj" \
	"$(INTDIR)\Calendar.obj" \
	"$(INTDIR)\CalTitle.obj" \
	"$(INTDIR)\compfile.obj" \
	"$(INTDIR)\Compress.obj" \
	"$(INTDIR)\Crc32.obj" \
	"$(INTDIR)\Cryptfil.obj" \
	"$(INTDIR)\curredit.obj" \
	"$(INTDIR)\DayBar.obj" \
	"$(INTDIR)\DayBox.obj" \
	"$(INTDIR)\Deflate.obj" \
	"$(INTDIR)\DropEdit.obj" \
	"$(INTDIR)\DTButton.obj" \
	"$(INTDIR)\DTChange.obj" \
	"$(INTDIR)\DTCtrl.obj" \
	"$(INTDIR)\DTGadget.obj" \
	"$(INTDIR)\DTList.obj" \
	"$(INTDIR)\DTNumerc.obj" \
	"$(INTDIR)\DTParser.obj" \
	"$(INTDIR)\DTSpin.obj" \
	"$(INTDIR)\DTStatic.obj" \
	"$(INTDIR)\fdichild.obj" \
	"$(INTDIR)\fdimain.obj" \
	"$(INTDIR)\filesys.obj" \
	"$(INTDIR)\Gzio.obj" \
	"$(INTDIR)\Infblock.obj" \
	"$(INTDIR)\Infcodes.obj" \
	"$(INTDIR)\Inffast.obj" \
	"$(INTDIR)\Inflate.obj" \
	"$(INTDIR)\Inftrees.obj" \
	"$(INTDIR)\Infutil.obj" \
	"$(INTDIR)\Jcapimin.obj" \
	"$(INTDIR)\Jcapistd.obj" \
	"$(INTDIR)\Jcarith.obj" \
	"$(INTDIR)\Jccoefct.obj" \
	"$(INTDIR)\Jccolor.obj" \
	"$(INTDIR)\Jcdctmgr.obj" \
	"$(INTDIR)\Jchuff.obj" \
	"$(INTDIR)\Jcinit.obj" \
	"$(INTDIR)\Jcmainct.obj" \
	"$(INTDIR)\Jcmarker.obj" \
	"$(INTDIR)\Jcmaster.obj" \
	"$(INTDIR)\Jcomapi.obj" \
	"$(INTDIR)\Jcparam.obj" \
	"$(INTDIR)\Jcphuff.obj" \
	"$(INTDIR)\Jcprepct.obj" \
	"$(INTDIR)\Jcsample.obj" \
	"$(INTDIR)\Jctrans.obj" \
	"$(INTDIR)\Jdapimin.obj" \
	"$(INTDIR)\Jdapistd.obj" \
	"$(INTDIR)\Jdarith.obj" \
	"$(INTDIR)\Jdatadst.obj" \
	"$(INTDIR)\Jdatasrc.obj" \
	"$(INTDIR)\Jdcoefct.obj" \
	"$(INTDIR)\Jdcolor.obj" \
	"$(INTDIR)\Jddctmgr.obj" \
	"$(INTDIR)\Jdhuff.obj" \
	"$(INTDIR)\Jdinput.obj" \
	"$(INTDIR)\Jdmainct.obj" \
	"$(INTDIR)\Jdmarker.obj" \
	"$(INTDIR)\Jdmaster.obj" \
	"$(INTDIR)\Jdmerge.obj" \
	"$(INTDIR)\Jdphuff.obj" \
	"$(INTDIR)\Jdpostct.obj" \
	"$(INTDIR)\Jdsample.obj" \
	"$(INTDIR)\Jdtrans.obj" \
	"$(INTDIR)\Jerror.obj" \
	"$(INTDIR)\Jfdctflt.obj" \
	"$(INTDIR)\Jfdctfst.obj" \
	"$(INTDIR)\Jfdctint.obj" \
	"$(INTDIR)\Jidctflt.obj" \
	"$(INTDIR)\Jidctfst.obj" \
	"$(INTDIR)\Jidctint.obj" \
	"$(INTDIR)\Jidctred.obj" \
	"$(INTDIR)\Jmemansi.obj" \
	"$(INTDIR)\Jmemmgr.obj" \
	"$(INTDIR)\Jquant1.obj" \
	"$(INTDIR)\Jquant2.obj" \
	"$(INTDIR)\Jutils.obj" \
	"$(INTDIR)\lstbxclr.obj" \
	"$(INTDIR)\lstbxedt.obj" \
	"$(INTDIR)\Panel.obj" \
	"$(INTDIR)\Panview.obj" \
	"$(INTDIR)\Panwnd.obj" \
	"$(INTDIR)\Rotor.obj" \
	"$(INTDIR)\sbarcore.obj" \
	"$(INTDIR)\sbarcust.obj" \
	"$(INTDIR)\sbardlg.obj" \
	"$(INTDIR)\sbardock.obj" \
	"$(INTDIR)\sbarmgr.obj" \
	"$(INTDIR)\sbarstat.obj" \
	"$(INTDIR)\Sbartool.obj" \
	"$(INTDIR)\SCutAccl.obj" \
	"$(INTDIR)\SCutCmds.obj" \
	"$(INTDIR)\SCutDlg.obj" \
	"$(INTDIR)\SCutKDlg.obj" \
	"$(INTDIR)\sdockcnt.obj" \
	"$(INTDIR)\sdocksta.obj" \
	"$(INTDIR)\secaux.obj" \
	"$(INTDIR)\SECBMPDG.OBJ" \
	"$(INTDIR)\secbtns.obj" \
	"$(INTDIR)\SecCalc.obj" \
	"$(INTDIR)\seccomp.obj" \
	"$(INTDIR)\secdib.obj" \
	"$(INTDIR)\secdsp.obj" \
	"$(INTDIR)\secgif.obj" \
	"$(INTDIR)\secimage.obj" \
	"$(INTDIR)\secjpeg.obj" \
	"$(INTDIR)\secmedit.obj" \
	"$(INTDIR)\secpcx.obj" \
	"$(INTDIR)\secrand.obj" \
	"$(INTDIR)\secreg.obj" \
	"$(INTDIR)\SECSPLSH.OBJ" \
	"$(INTDIR)\sectarga.obj" \
	"$(INTDIR)\sectiff.obj" \
	"$(INTDIR)\sectlf.obj" \
	"$(INTDIR)\sectnbmp.obj" \
	"$(INTDIR)\sectndc.obj" \
	"$(INTDIR)\sectndlg.obj" \
	"$(INTDIR)\sectndoc.obj" \
	"$(INTDIR)\sectnvw.obj" \
	"$(INTDIR)\sectnwin.obj" \
	"$(INTDIR)\SECTOD.OBJ" \
	"$(INTDIR)\secwb.obj" \
	"$(INTDIR)\secwell.obj" \
	"$(INTDIR)\secwsmgr.obj" \
	"$(INTDIR)\slocale.obj" \
	"$(INTDIR)\slstctip.obj" \
	"$(INTDIR)\slstctl.obj" \
	"$(INTDIR)\Stdafx.obj" \
	"$(INTDIR)\swinfrm.obj" \
	"$(INTDIR)\swinmdi.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\tabctrl3.obj" \
	"$(INTDIR)\tabctrlb.obj" \
	"$(INTDIR)\tabwnd.obj" \
	"$(INTDIR)\tabwnd3.obj" \
	"$(INTDIR)\tabwndb.obj" \
	"$(INTDIR)\tbarcust.obj" \
	"$(INTDIR)\tbarmgr.obj" \
	"$(INTDIR)\tbarpage.obj" \
	"$(INTDIR)\tbarsdlg.obj" \
	"$(INTDIR)\tbartrck.obj" \
	"$(INTDIR)\tbtn2prt.obj" \
	"$(INTDIR)\tbtncmbo.obj" \
	"$(INTDIR)\tbtnstd.obj" \
	"$(INTDIR)\tbtnwnd.obj" \
	"$(INTDIR)\Tif_aux.obj" \
	"$(INTDIR)\Tif_ccit.obj" \
	"$(INTDIR)\Tif_clos.obj" \
	"$(INTDIR)\Tif_comp.obj" \
	"$(INTDIR)\Tif_dir.obj" \
	"$(INTDIR)\Tif_diri.obj" \
	"$(INTDIR)\Tif_dirr.obj" \
	"$(INTDIR)\Tif_dirw.obj" \
	"$(INTDIR)\Tif_dump.obj" \
	"$(INTDIR)\Tif_erro.obj" \
	"$(INTDIR)\Tif_fax3.obj" \
	"$(INTDIR)\Tif_fax4.obj" \
	"$(INTDIR)\Tif_flus.obj" \
	"$(INTDIR)\Tif_geti.obj" \
	"$(INTDIR)\Tif_jpeg.obj" \
	"$(INTDIR)\Tif_lzw.obj" \
	"$(INTDIR)\Tif_msdo.obj" \
	"$(INTDIR)\Tif_open.obj" \
	"$(INTDIR)\Tif_pack.obj" \
	"$(INTDIR)\Tif_prin.obj" \
	"$(INTDIR)\Tif_read.obj" \
	"$(INTDIR)\Tif_stri.obj" \
	"$(INTDIR)\Tif_swab.obj" \
	"$(INTDIR)\Tif_thun.obj" \
	"$(INTDIR)\Tif_tile.obj" \
	"$(INTDIR)\Tif_vers.obj" \
	"$(INTDIR)\Tif_warn.obj" \
	"$(INTDIR)\Tif_writ.obj" \
	"$(INTDIR)\TrCtlX.obj" \
	"$(INTDIR)\TreeNode.obj" \
	"$(INTDIR)\Trees.obj" \
	"$(INTDIR)\Uncompr.obj" \
	"$(INTDIR)\usertool.obj" \
	"$(INTDIR)\usrtldlg.obj" \
	"$(INTDIR)\Zoomview.obj" \
	"$(INTDIR)\Zutil.obj"

"$(OUTDIR)\OTA50UR.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

# Name "OTlib50 - Win32 OT50R"
# Name "OTlib50 - Win32 OT50D"
# Name "OTlib50 - Win32 OTA50R"
# Name "OTlib50 - Win32 OTA50D"
# Name "OTlib50 - Win32 OT50UD"
# Name "OTlib50 - Win32 OT50UR"
# Name "OTlib50 - Win32 OTA50UD"
# Name "OTlib50 - Win32 OTA50UR"

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Stdafx.cpp
DEP_CPP_STDAF=\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"

# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MT /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)/OT50R.pch" /Yc\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OT50R.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"

# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/OT50D.pch" /Yc /Fo"$(INTDIR)/" /Fd"$(INTDIR)/OT50D.pdb" /c\
 $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OT50D.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"

# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MD /W4 /GR /GX /Zi /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/OTA50R.pch" /Yc /Fo"$(INTDIR)/" /Fd"$(INTDIR)/OTA50R.pdb" /c $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OTA50R.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"

# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MDd /W4 /GR /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/OTA50D.pch" /Yc /Fo"$(INTDIR)/" /Fd"$(INTDIR)/OTA50D.pdb" /c\
 $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OTA50D.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc

BuildCmds= \
	$(CPP) /nologo /MTd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE"\
 /Fp"$(INTDIR)/OT50UD.pch" /Yc /Fo"$(INTDIR)/" /Fd"$(INTDIR)/OT50UD.pdb" /c\
 $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OT50UD.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MT /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"$(INTDIR)/OT50UR.pch" /Yc\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OT50UR.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MDd /W4 /Gm /GX /Zi /Od /I "..\src" /I "..\..\src" /I\
 "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE"\
 /Fp"$(INTDIR)/OTA50UD.pch" /Yc /Fo"$(INTDIR)/" /Fd"$(INTDIR)/OTA50UD.pdb" /c\
 $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OTA50UD.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc""

BuildCmds= \
	$(CPP) /nologo /MD /W4 /GX /O1 /I "..\src" /I "..\..\src" /I "..\include" /D\
 "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_UNICODE"\
 /Fp"$(INTDIR)/OTA50UR.pch" /Yc /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\Stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\OTA50UR.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\secaux.cpp
DEP_CPP_SECAU=\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secaux.obj" : $(SOURCE) $(DEP_CPP_SECAU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\secwell.cpp
DEP_CPP_SECWE=\
	"..\include\secImpl.h"\
	"..\include\secwell.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secwell.obj" : $(SOURCE) $(DEP_CPP_SECWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\secbtns.cpp
DEP_CPP_SECBT=\
	"..\include\SecBtns.h"\
	"..\include\secImpl.h"\
	"..\include\secwell.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secbtns.obj" : $(SOURCE) $(DEP_CPP_SECBT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\secmedit.cpp
DEP_CPP_SECME=\
	"..\include\secmedit.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secmedit.obj" : $(SOURCE) $(DEP_CPP_SECME) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\sbarcust.cpp
DEP_CPP_SBARC=\
	"..\include\sbarcore.h"\
	"..\include\sbarcust.h"\
	"..\include\sbarstat.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sbarcust.obj" : $(SOURCE) $(DEP_CPP_SBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calendar\Panel.cpp
DEP_CPP_PANEL=\
	"..\include\calendar.h"\
	"..\include\CalTitle.h"\
	"..\include\DayBar.h"\
	"..\include\DayBox.h"\
	"..\include\Panel.h"\
	"..\include\secImpl.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Panel.obj" : $(SOURCE) $(DEP_CPP_PANEL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calendar\CalTitle.cpp
DEP_CPP_CALTI=\
	"..\include\calendar.h"\
	"..\include\CalTitle.h"\
	"..\include\DayBar.h"\
	"..\include\DayBox.h"\
	"..\include\Panel.h"\
	"..\include\slocale.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\CalTitle.obj" : $(SOURCE) $(DEP_CPP_CALTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calendar\DayBar.cpp
DEP_CPP_DAYBA=\
	"..\include\calendar.h"\
	"..\include\CalTitle.h"\
	"..\include\DayBar.h"\
	"..\include\DayBox.h"\
	"..\include\Panel.h"\
	"..\include\slocale.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DayBar.obj" : $(SOURCE) $(DEP_CPP_DAYBA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calendar\DayBox.cpp
DEP_CPP_DAYBO=\
	"..\include\calendar.h"\
	"..\include\CalTitle.h"\
	"..\include\DayBar.h"\
	"..\include\DayBox.h"\
	"..\include\Panel.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DayBox.obj" : $(SOURCE) $(DEP_CPP_DAYBO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calendar\Calendar.cpp
DEP_CPP_CALEN=\
	"..\include\calendar.h"\
	"..\include\CalTitle.h"\
	"..\include\DayBar.h"\
	"..\include\DayBox.h"\
	"..\include\Panel.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Calendar.obj" : $(SOURCE) $(DEP_CPP_CALEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\tabctrl\tabwnd3.cpp
DEP_CPP_TABWN=\
	"..\include\tabctrl3.h"\
	"..\include\tabctrlb.h"\
	"..\include\tabwnd3.h"\
	"..\include\tabwndb.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tabwnd3.obj" : $(SOURCE) $(DEP_CPP_TABWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\tabctrl\tabctrl3.cpp
DEP_CPP_TABCT=\
	"..\include\tabctrl3.h"\
	"..\include\tabctrlb.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tabctrl3.obj" : $(SOURCE) $(DEP_CPP_TABCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sdocksta.cpp
DEP_CPP_SDOCK=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdockcnt.h"\
	"..\include\sdocksta.h"\
	"..\include\swinfrm.h"\
	"..\include\swinmdi.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sdocksta.obj" : $(SOURCE) $(DEP_CPP_SDOCK) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sbardlg.cpp
DEP_CPP_SBARD=\
	"..\include\sbarcore.h"\
	"..\include\sbardlg.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sbardlg.obj" : $(SOURCE) $(DEP_CPP_SBARD) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sbardock.cpp
DEP_CPP_SBARDO=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\sdockcnt.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sbardock.obj" : $(SOURCE) $(DEP_CPP_SBARDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sbarstat.cpp
DEP_CPP_SBARS=\
	"..\include\sbarcore.h"\
	"..\include\sbarstat.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sbarstat.obj" : $(SOURCE) $(DEP_CPP_SBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\Sbartool.cpp
DEP_CPP_SBART=\
	"..\include\sbarcore.h"\
	"..\include\sbartool.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Sbartool.obj" : $(SOURCE) $(DEP_CPP_SBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sdockcnt.cpp
DEP_CPP_SDOCKC=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\sdockcnt.h"\
	"..\include\SecAux.h"\
	"..\include\swinfrm.h"\
	"..\include\swinmdi.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sdockcnt.obj" : $(SOURCE) $(DEP_CPP_SDOCKC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sbarcore.cpp
DEP_CPP_SBARCO=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\sdockcnt.h"\
	"..\include\SecAux.h"\
	"..\include\swinfrm.h"\
	"..\include\swinmdi.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sbarcore.obj" : $(SOURCE) $(DEP_CPP_SBARCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\sectiff.cpp
DEP_CPP_SECTI=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectiff.obj" : $(SOURCE) $(DEP_CPP_SECTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\secdsp.cpp
DEP_CPP_SECDS=\
	"..\include\secimage.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secdsp.obj" : $(SOURCE) $(DEP_CPP_SECDS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\secgif.cpp
DEP_CPP_SECGI=\
	"..\include\secgif.h"\
	"..\include\secimage.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secgif.obj" : $(SOURCE) $(DEP_CPP_SECGI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\secimage.cpp
DEP_CPP_SECIM=\
	"..\include\secimage.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secimage.obj" : $(SOURCE) $(DEP_CPP_SECIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\secjpeg.cpp
DEP_CPP_SECJP=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secjpeg.obj" : $(SOURCE) $(DEP_CPP_SECJP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\secpcx.cpp
DEP_CPP_SECPC=\
	"..\include\secpcx.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secpcx.obj" : $(SOURCE) $(DEP_CPP_SECPC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\sectarga.cpp
DEP_CPP_SECTA=\
	"..\include\sectarga.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectarga.obj" : $(SOURCE) $(DEP_CPP_SECTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\secdib.cpp
DEP_CPP_SECDI=\
	"..\include\secdib.h"\
	"..\include\secimage.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secdib.obj" : $(SOURCE) $(DEP_CPP_SECDI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jutils.cpp
DEP_CPP_JUTIL=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jutils.obj" : $(SOURCE) $(DEP_CPP_JUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcapistd.cpp
DEP_CPP_JCAPI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcapistd.obj" : $(SOURCE) $(DEP_CPP_JCAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcarith.cpp
DEP_CPP_JCARI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcarith.obj" : $(SOURCE) $(DEP_CPP_JCARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jccoefct.cpp
DEP_CPP_JCCOE=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jccoefct.obj" : $(SOURCE) $(DEP_CPP_JCCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jccolor.cpp
DEP_CPP_JCCOL=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jccolor.obj" : $(SOURCE) $(DEP_CPP_JCCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcdctmgr.cpp
DEP_CPP_JCDCT=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcdctmgr.obj" : $(SOURCE) $(DEP_CPP_JCDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jchuff.cpp
DEP_CPP_JCHUF=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jchuff.obj" : $(SOURCE) $(DEP_CPP_JCHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcinit.cpp
DEP_CPP_JCINI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcinit.obj" : $(SOURCE) $(DEP_CPP_JCINI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcmainct.cpp
DEP_CPP_JCMAI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcmainct.obj" : $(SOURCE) $(DEP_CPP_JCMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcmarker.cpp
DEP_CPP_JCMAR=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcmarker.obj" : $(SOURCE) $(DEP_CPP_JCMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcmaster.cpp
DEP_CPP_JCMAS=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcmaster.obj" : $(SOURCE) $(DEP_CPP_JCMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcomapi.cpp
DEP_CPP_JCOMA=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcomapi.obj" : $(SOURCE) $(DEP_CPP_JCOMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcparam.cpp
DEP_CPP_JCPAR=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcparam.obj" : $(SOURCE) $(DEP_CPP_JCPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcphuff.cpp
DEP_CPP_JCPHU=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcphuff.obj" : $(SOURCE) $(DEP_CPP_JCPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcprepct.cpp
DEP_CPP_JCPRE=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcprepct.obj" : $(SOURCE) $(DEP_CPP_JCPRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcsample.cpp
DEP_CPP_JCSAM=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcsample.obj" : $(SOURCE) $(DEP_CPP_JCSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jctrans.cpp
DEP_CPP_JCTRA=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jctrans.obj" : $(SOURCE) $(DEP_CPP_JCTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdapimin.cpp
DEP_CPP_JDAPI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdapimin.obj" : $(SOURCE) $(DEP_CPP_JDAPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdapistd.cpp
DEP_CPP_JDAPIS=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdapistd.obj" : $(SOURCE) $(DEP_CPP_JDAPIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdarith.cpp
DEP_CPP_JDARI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdarith.obj" : $(SOURCE) $(DEP_CPP_JDARI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdatadst.cpp
DEP_CPP_JDATA=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdatadst.obj" : $(SOURCE) $(DEP_CPP_JDATA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdatasrc.cpp
DEP_CPP_JDATAS=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdatasrc.obj" : $(SOURCE) $(DEP_CPP_JDATAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdcoefct.cpp
DEP_CPP_JDCOE=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdcoefct.obj" : $(SOURCE) $(DEP_CPP_JDCOE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdcolor.cpp
DEP_CPP_JDCOL=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdcolor.obj" : $(SOURCE) $(DEP_CPP_JDCOL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jddctmgr.cpp
DEP_CPP_JDDCT=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jddctmgr.obj" : $(SOURCE) $(DEP_CPP_JDDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdhuff.cpp
DEP_CPP_JDHUF=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdhuff.obj" : $(SOURCE) $(DEP_CPP_JDHUF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdinput.cpp
DEP_CPP_JDINP=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdinput.obj" : $(SOURCE) $(DEP_CPP_JDINP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdmainct.cpp
DEP_CPP_JDMAI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdmainct.obj" : $(SOURCE) $(DEP_CPP_JDMAI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdmarker.cpp
DEP_CPP_JDMAR=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdmarker.obj" : $(SOURCE) $(DEP_CPP_JDMAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdmaster.cpp
DEP_CPP_JDMAS=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdmaster.obj" : $(SOURCE) $(DEP_CPP_JDMAS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdmerge.cpp
DEP_CPP_JDMER=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdmerge.obj" : $(SOURCE) $(DEP_CPP_JDMER) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdphuff.cpp
DEP_CPP_JDPHU=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdphuff.obj" : $(SOURCE) $(DEP_CPP_JDPHU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdpostct.cpp
DEP_CPP_JDPOS=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdpostct.obj" : $(SOURCE) $(DEP_CPP_JDPOS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdsample.cpp
DEP_CPP_JDSAM=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdsample.obj" : $(SOURCE) $(DEP_CPP_JDSAM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jdtrans.cpp
DEP_CPP_JDTRA=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jdtrans.obj" : $(SOURCE) $(DEP_CPP_JDTRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jerror.cpp
DEP_CPP_JERRO=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\jversion.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jerror.obj" : $(SOURCE) $(DEP_CPP_JERRO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jfdctflt.cpp
DEP_CPP_JFDCT=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jfdctflt.obj" : $(SOURCE) $(DEP_CPP_JFDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jfdctfst.cpp
DEP_CPP_JFDCTF=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jfdctfst.obj" : $(SOURCE) $(DEP_CPP_JFDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jfdctint.cpp
DEP_CPP_JFDCTI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jfdctint.obj" : $(SOURCE) $(DEP_CPP_JFDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jidctflt.cpp
DEP_CPP_JIDCT=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jidctflt.obj" : $(SOURCE) $(DEP_CPP_JIDCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jidctfst.cpp
DEP_CPP_JIDCTF=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jidctfst.obj" : $(SOURCE) $(DEP_CPP_JIDCTF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jidctint.cpp
DEP_CPP_JIDCTI=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jidctint.obj" : $(SOURCE) $(DEP_CPP_JIDCTI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jidctred.cpp
DEP_CPP_JIDCTR=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jidctred.obj" : $(SOURCE) $(DEP_CPP_JIDCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jmemansi.cpp
DEP_CPP_JMEMA=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jmemansi.obj" : $(SOURCE) $(DEP_CPP_JMEMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jmemmgr.cpp
DEP_CPP_JMEMM=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jmemmgr.obj" : $(SOURCE) $(DEP_CPP_JMEMM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jquant1.cpp
DEP_CPP_JQUAN=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jquant1.obj" : $(SOURCE) $(DEP_CPP_JQUAN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jquant2.cpp
DEP_CPP_JQUANT=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jquant2.obj" : $(SOURCE) $(DEP_CPP_JQUANT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Jpeg\Jcapimin.cpp
DEP_CPP_JCAPIM=\
	"..\include\jchuff.h"\
	"..\include\jdct.h"\
	"..\include\jdhuff.h"\
	"..\include\jerror.h"\
	"..\include\jmemsys.h"\
	"..\include\secimage.h"\
	"..\include\secjpeg.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\jconfig.h"\
	{$(INCLUDE)}"\jinclude.h"\
	{$(INCLUDE)}"\jmorecfg.h"\
	{$(INCLUDE)}"\jpegint.h"\
	{$(INCLUDE)}"\jpeglib.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Jcapimin.obj" : $(SOURCE) $(DEP_CPP_JCAPIM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_writ.cpp
DEP_CPP_TIF_W=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_writ.obj" : $(SOURCE) $(DEP_CPP_TIF_W) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_ccit.cpp
DEP_CPP_TIF_C=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tif_fax3.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_ccit.obj" : $(SOURCE) $(DEP_CPP_TIF_C) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_clos.cpp
DEP_CPP_TIF_CL=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_clos.obj" : $(SOURCE) $(DEP_CPP_TIF_CL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_comp.cpp
DEP_CPP_TIF_CO=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_comp.obj" : $(SOURCE) $(DEP_CPP_TIF_CO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_dir.cpp
DEP_CPP_TIF_D=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_dir.obj" : $(SOURCE) $(DEP_CPP_TIF_D) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_diri.cpp
DEP_CPP_TIF_DI=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_diri.obj" : $(SOURCE) $(DEP_CPP_TIF_DI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_dirr.cpp
DEP_CPP_TIF_DIR=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_dirr.obj" : $(SOURCE) $(DEP_CPP_TIF_DIR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_dirw.cpp
DEP_CPP_TIF_DIRW=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_dirw.obj" : $(SOURCE) $(DEP_CPP_TIF_DIRW) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_dump.cpp
DEP_CPP_TIF_DU=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_dump.obj" : $(SOURCE) $(DEP_CPP_TIF_DU) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_erro.cpp
DEP_CPP_TIF_E=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_erro.obj" : $(SOURCE) $(DEP_CPP_TIF_E) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_fax3.cpp
DEP_CPP_TIF_F=\
	"..\include\g3states.h"\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tif_fax3.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_fax3.obj" : $(SOURCE) $(DEP_CPP_TIF_F) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_fax4.cpp
DEP_CPP_TIF_FA=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tif_fax3.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_fax4.obj" : $(SOURCE) $(DEP_CPP_TIF_FA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_flus.cpp
DEP_CPP_TIF_FL=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_flus.obj" : $(SOURCE) $(DEP_CPP_TIF_FL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_geti.cpp
DEP_CPP_TIF_G=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_geti.obj" : $(SOURCE) $(DEP_CPP_TIF_G) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_jpeg.cpp
DEP_CPP_TIF_J=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_jpeg.obj" : $(SOURCE) $(DEP_CPP_TIF_J) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_lzw.cpp
DEP_CPP_TIF_L=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_lzw.obj" : $(SOURCE) $(DEP_CPP_TIF_L) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_msdo.cpp
DEP_CPP_TIF_M=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\STAT.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_msdo.obj" : $(SOURCE) $(DEP_CPP_TIF_M) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_open.cpp
DEP_CPP_TIF_O=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_open.obj" : $(SOURCE) $(DEP_CPP_TIF_O) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_pack.cpp
DEP_CPP_TIF_P=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_pack.obj" : $(SOURCE) $(DEP_CPP_TIF_P) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_prin.cpp
DEP_CPP_TIF_PR=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_prin.obj" : $(SOURCE) $(DEP_CPP_TIF_PR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_read.cpp
DEP_CPP_TIF_R=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_read.obj" : $(SOURCE) $(DEP_CPP_TIF_R) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_stri.cpp
DEP_CPP_TIF_S=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_stri.obj" : $(SOURCE) $(DEP_CPP_TIF_S) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_swab.cpp
DEP_CPP_TIF_SW=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_swab.obj" : $(SOURCE) $(DEP_CPP_TIF_SW) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_thun.cpp
DEP_CPP_TIF_T=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_thun.obj" : $(SOURCE) $(DEP_CPP_TIF_T) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_tile.cpp
DEP_CPP_TIF_TI=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_tile.obj" : $(SOURCE) $(DEP_CPP_TIF_TI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_vers.cpp
DEP_CPP_TIF_V=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_vers.obj" : $(SOURCE) $(DEP_CPP_TIF_V) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_warn.cpp
DEP_CPP_TIF_WA=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_warn.obj" : $(SOURCE) $(DEP_CPP_TIF_WA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\image\Tiff\Tif_aux.cpp
DEP_CPP_TIF_A=\
	"..\include\sectiff.h"\
	"..\include\t4.h"\
	"..\include\tiffiop.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\tiff.h"\
	{$(INCLUDE)}"\tiffcomp.h"\
	{$(INCLUDE)}"\tiffconf.h"\
	{$(INCLUDE)}"\tiffio.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Tif_aux.obj" : $(SOURCE) $(DEP_CPP_TIF_A) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mdi\swinmdi.cpp
DEP_CPP_SWINM=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\sdockcnt.h"\
	"..\include\SecAux.h"\
	"..\include\swinfrm.h"\
	"..\include\swinmdi.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\swinmdi.obj" : $(SOURCE) $(DEP_CPP_SWINM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mdi\fdichild.cpp
DEP_CPP_FDICH=\
	"..\include\fdichild.h"\
	"..\include\fdimain.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\fdichild.obj" : $(SOURCE) $(DEP_CPP_FDICH) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mdi\fdimain.cpp
DEP_CPP_FDIMA=\
	"..\include\fdimain.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\fdimain.obj" : $(SOURCE) $(DEP_CPP_FDIMA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mdi\sectlf.cpp
DEP_CPP_SECTL=\
	"..\include\sectlf.h"\
	"..\include\swinfrm.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectlf.obj" : $(SOURCE) $(DEP_CPP_SECTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mdi\secwb.cpp
DEP_CPP_SECWB=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\SecAux.h"\
	"..\include\secwb.h"\
	"..\include\swinfrm.h"\
	"..\include\swinmdi.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secwb.obj" : $(SOURCE) $(DEP_CPP_SECWB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mdi\swinfrm.cpp
DEP_CPP_SWINF=\
	"..\include\sbarcore.h"\
	"..\include\sbardock.h"\
	"..\include\sdockcnt.h"\
	"..\include\SecAux.h"\
	"..\include\swinfrm.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\swinfrm.obj" : $(SOURCE) $(DEP_CPP_SWINF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\secwsmgr.cpp
DEP_CPP_SECWS=\
	"..\include\lstbxedt.h"\
	"..\include\secreg.h"\
	"..\include\secwsmgr.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secwsmgr.obj" : $(SOURCE) $(DEP_CPP_SECWS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\shortcut\SCutKDlg.cpp
DEP_CPP_SCUTK=\
	"..\include\secall.h"\
	"..\include\slocale.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SCutKDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTK) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\shortcut\SCutCmds.cpp
DEP_CPP_SCUTC=\
	"..\include\SCutAccl.h"\
	"..\include\SCutCmds.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SCutCmds.obj" : $(SOURCE) $(DEP_CPP_SCUTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\shortcut\SCutDlg.cpp
DEP_CPP_SCUTD=\
	"..\include\SCutAccl.h"\
	"..\include\SCutCmds.h"\
	"..\include\SCutDlg.h"\
	"..\include\SCutKDlg.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SCutDlg.obj" : $(SOURCE) $(DEP_CPP_SCUTD) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\shortcut\SCutAccl.cpp
DEP_CPP_SCUTA=\
	"..\include\SCutAccl.h"\
	"..\include\SCutCmds.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SCutAccl.obj" : $(SOURCE) $(DEP_CPP_SCUTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\thumnail\sectnwin.cpp
DEP_CPP_SECTN=\
	"..\include\sectnwin.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectnwin.obj" : $(SOURCE) $(DEP_CPP_SECTN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\thumnail\sectndc.cpp
DEP_CPP_SECTND=\
	"..\include\sectnbmp.h"\
	"..\include\sectndc.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectndc.obj" : $(SOURCE) $(DEP_CPP_SECTND) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\thumnail\sectndlg.cpp
DEP_CPP_SECTNDL=\
	"..\include\sectnbmp.h"\
	"..\include\sectndlg.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectndlg.obj" : $(SOURCE) $(DEP_CPP_SECTNDL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\thumnail\sectndoc.cpp
DEP_CPP_SECTNDO=\
	"..\include\sectnbmp.h"\
	"..\include\sectndoc.h"\
	"..\include\sectnvw.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectndoc.obj" : $(SOURCE) $(DEP_CPP_SECTNDO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\thumnail\sectnvw.cpp
DEP_CPP_SECTNV=\
	"..\include\sectnvw.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectnvw.obj" : $(SOURCE) $(DEP_CPP_SECTNV) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\thumnail\sectnbmp.cpp
DEP_CPP_SECTNB=\
	"..\include\sectnbmp.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sectnbmp.obj" : $(SOURCE) $(DEP_CPP_SECTNB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\secreg.cpp
DEP_CPP_SECRE=\
	"..\include\secreg.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secreg.obj" : $(SOURCE) $(DEP_CPP_SECRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\seccomp.cpp
DEP_CPP_SECCO=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\seccomp.obj" : $(SOURCE) $(DEP_CPP_SECCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\secrand.cpp
DEP_CPP_SECRA=\
	"..\include\secrand.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\secrand.obj" : $(SOURCE) $(DEP_CPP_SECRA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\filesys.cpp
DEP_CPP_FILES=\
	"..\include\filesys.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\filesys.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\crypt\Rotor.cpp
DEP_CPP_ROTOR=\
	"..\include\rotor.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Rotor.obj" : $(SOURCE) $(DEP_CPP_ROTOR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\crypt\Cryptfil.cpp
DEP_CPP_CRYPT=\
	"..\include\blackbox.h"\
	"..\include\cryptfil.h"\
	"..\include\rotor.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Cryptfil.obj" : $(SOURCE) $(DEP_CPP_CRYPT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\crypt\Blackbox.cpp
DEP_CPP_BLACK=\
	"..\include\blackbox.h"\
	"..\include\rotor.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Blackbox.obj" : $(SOURCE) $(DEP_CPP_BLACK) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Zutil.cpp
DEP_CPP_ZUTIL=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Zutil.obj" : $(SOURCE) $(DEP_CPP_ZUTIL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\compfile.cpp
DEP_CPP_COMPF=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\seccomp.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\compfile.obj" : $(SOURCE) $(DEP_CPP_COMPF) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Compress.cpp
DEP_CPP_COMPR=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Compress.obj" : $(SOURCE) $(DEP_CPP_COMPR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Crc32.cpp
DEP_CPP_CRC32=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Crc32.obj" : $(SOURCE) $(DEP_CPP_CRC32) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Deflate.cpp
DEP_CPP_DEFLA=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Deflate.obj" : $(SOURCE) $(DEP_CPP_DEFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Gzio.cpp
DEP_CPP_GZIO_=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Gzio.obj" : $(SOURCE) $(DEP_CPP_GZIO_) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Infblock.cpp
DEP_CPP_INFBL=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\infcodes.h"\
	"..\include\infutil.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Infblock.obj" : $(SOURCE) $(DEP_CPP_INFBL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Infcodes.cpp
DEP_CPP_INFCO=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\infcodes.h"\
	"..\include\inffast.h"\
	"..\include\infutil.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Infcodes.obj" : $(SOURCE) $(DEP_CPP_INFCO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Inffast.cpp
DEP_CPP_INFFA=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\infutil.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Inffast.obj" : $(SOURCE) $(DEP_CPP_INFFA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Inflate.cpp
DEP_CPP_INFLA=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Inflate.obj" : $(SOURCE) $(DEP_CPP_INFLA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Inftrees.cpp
DEP_CPP_INFTR=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Inftrees.obj" : $(SOURCE) $(DEP_CPP_INFTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Infutil.cpp
DEP_CPP_INFUT=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\infutil.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Infutil.obj" : $(SOURCE) $(DEP_CPP_INFUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Trees.cpp
DEP_CPP_TREES=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Trees.obj" : $(SOURCE) $(DEP_CPP_TREES) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Uncompr.cpp
DEP_CPP_UNCOM=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Uncompr.obj" : $(SOURCE) $(DEP_CPP_UNCOM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\zlib\Adler32.cpp
DEP_CPP_ADLER=\
	"..\include\compeng.h"\
	"..\include\deflate.h"\
	"..\include\zutil.h"\
	"..\src\stdafx.h"\
	{$(INCLUDE)}"\zconf.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Adler32.obj" : $(SOURCE) $(DEP_CPP_ADLER) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\views\Zoomview.cpp
DEP_CPP_ZOOMV=\
	"..\include\seczmvw.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Zoomview.obj" : $(SOURCE) $(DEP_CPP_ZOOMV) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\views\Panwnd.cpp
DEP_CPP_PANWN=\
	"..\include\secpanvw.h"\
	"..\include\secpanwn.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Panwnd.obj" : $(SOURCE) $(DEP_CPP_PANWN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\views\Panview.cpp
DEP_CPP_PANVI=\
	"..\include\secpanvw.h"\
	"..\include\secpanwn.h"\
	"..\include\seczmvw.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\Panview.obj" : $(SOURCE) $(DEP_CPP_PANVI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\lstbxedt.cpp
DEP_CPP_LSTBX=\
	"..\include\browedit.h"\
	"..\include\lstbxedt.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\lstbxedt.obj" : $(SOURCE) $(DEP_CPP_LSTBX) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\lstbxclr.cpp
DEP_CPP_LSTBXC=\
	"..\include\lstbxclr.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\lstbxclr.obj" : $(SOURCE) $(DEP_CPP_LSTBXC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\toolsmnu\usrtldlg.cpp
DEP_CPP_USRTL=\
	"..\include\browedit.h"\
	"..\include\usertool.h"\
	"..\include\usrtldlg.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\usrtldlg.obj" : $(SOURCE) $(DEP_CPP_USRTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\toolsmnu\usertool.cpp
DEP_CPP_USERT=\
	"..\include\usertool.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\usertool.obj" : $(SOURCE) $(DEP_CPP_USERT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\tabctrl\tabwnd.cpp
DEP_CPP_TABWND=\
	"..\include\tabctrl.h"\
	"..\include\tabctrlb.h"\
	"..\include\tabwnd.h"\
	"..\include\tabwndb.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tabwnd.obj" : $(SOURCE) $(DEP_CPP_TABWND) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\tabctrl\tabctrl.cpp
DEP_CPP_TABCTR=\
	"..\include\tabctrl.h"\
	"..\include\tabctrlb.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tabctrl.obj" : $(SOURCE) $(DEP_CPP_TABCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\tabctrl\tabctrlb.cpp
DEP_CPP_TABCTRL=\
	"..\include\tabctrlb.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tabctrlb.obj" : $(SOURCE) $(DEP_CPP_TABCTRL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\tabctrl\tabwndb.cpp
DEP_CPP_TABWNDB=\
	"..\include\tabctrlb.h"\
	"..\include\tabwndb.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tabwndb.obj" : $(SOURCE) $(DEP_CPP_TABWNDB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\browedit.cpp
DEP_CPP_BROWE=\
	"..\include\browedit.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\browedit.obj" : $(SOURCE) $(DEP_CPP_BROWE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\SECBMPDG.CPP
DEP_CPP_SECBM=\
	"..\include\secbmpdg.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SECBMPDG.OBJ" : $(SOURCE) $(DEP_CPP_SECBM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\SECSPLSH.CPP
DEP_CPP_SECSP=\
	"..\include\secsplsh.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SECSPLSH.OBJ" : $(SOURCE) $(DEP_CPP_SECSP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\SECTOD.CPP
DEP_CPP_SECTO=\
	"..\include\sectod.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SECTOD.OBJ" : $(SOURCE) $(DEP_CPP_SECTO) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\docking\sbarmgr.cpp
DEP_CPP_SBARM=\
	"..\include\sbarmgr.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\sbarmgr.obj" : $(SOURCE) $(DEP_CPP_SBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbtnwnd.cpp
DEP_CPP_TBTNW=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbartrck.h"\
	"..\include\tbtnstd.h"\
	"..\include\tbtnwnd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbtnwnd.obj" : $(SOURCE) $(DEP_CPP_TBTNW) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbarmgr.cpp
DEP_CPP_TBARM=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\SecAux.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbarsdlg.h"\
	"..\include\tbtnstd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbarmgr.obj" : $(SOURCE) $(DEP_CPP_TBARM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbarpage.cpp
DEP_CPP_TBARP=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbarpage.h"\
	"..\include\tbtnstd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbarpage.obj" : $(SOURCE) $(DEP_CPP_TBARP) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbarsdlg.cpp
DEP_CPP_TBARS=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbarsdlg.h"\
	"..\include\tbtnstd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbarsdlg.obj" : $(SOURCE) $(DEP_CPP_TBARS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbartrck.cpp
DEP_CPP_TBART=\
	"..\include\tbartrck.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbartrck.obj" : $(SOURCE) $(DEP_CPP_TBART) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbtn2prt.cpp
DEP_CPP_TBTN2=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbtn2prt.h"\
	"..\include\tbtnstd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbtn2prt.obj" : $(SOURCE) $(DEP_CPP_TBTN2) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbtncmbo.cpp
DEP_CPP_TBTNC=\
	"..\include\tbtncmbo.h"\
	"..\include\tbtnstd.h"\
	"..\include\tbtnwnd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbtncmbo.obj" : $(SOURCE) $(DEP_CPP_TBTNC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbtnstd.cpp
DEP_CPP_TBTNS=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\SecAux.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbtnstd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbtnstd.obj" : $(SOURCE) $(DEP_CPP_TBTNS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ui\ctoolbar\tbarcust.cpp
DEP_CPP_TBARC=\
	"..\include\sbarcore.h"\
	"..\include\sbarmgr.h"\
	"..\include\sdocksta.h"\
	"..\include\SecAux.h"\
	"..\include\tbarcust.h"\
	"..\include\tbarmgr.h"\
	"..\include\tbartrck.h"\
	"..\include\tbtnstd.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\tbarcust.obj" : $(SOURCE) $(DEP_CPP_TBARC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTStatic.cpp
DEP_CPP_DTSTA=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTStatic.obj" : $(SOURCE) $(DEP_CPP_DTSTA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTChange.cpp
DEP_CPP_DTCHA=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTChange.obj" : $(SOURCE) $(DEP_CPP_DTCHA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTCtrl.cpp
DEP_CPP_DTCTR=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\SecAux.h"\
	"..\include\SecBtns.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTCtrl.obj" : $(SOURCE) $(DEP_CPP_DTCTR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTGadget.cpp
DEP_CPP_DTGAD=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTGadget.obj" : $(SOURCE) $(DEP_CPP_DTGAD) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTList.cpp
DEP_CPP_DTLIS=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTList.obj" : $(SOURCE) $(DEP_CPP_DTLIS) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTNumerc.cpp
DEP_CPP_DTNUM=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTNumerc.obj" : $(SOURCE) $(DEP_CPP_DTNUM) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTParser.cpp
DEP_CPP_DTPAR=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\slocale.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTParser.obj" : $(SOURCE) $(DEP_CPP_DTPAR) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTSpin.cpp
DEP_CPP_DTSPI=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTSpin.obj" : $(SOURCE) $(DEP_CPP_DTSPI) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\datetime\DTButton.cpp
DEP_CPP_DTBUT=\
	"..\include\DTCtrl.h"\
	"..\include\DTGadget.h"\
	"..\include\SecAux.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DTButton.obj" : $(SOURCE) $(DEP_CPP_DTBUT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\DropEdit.cpp
DEP_CPP_DROPE=\
	"..\include\dropedit.h"\
	"..\include\SecBtns.h"\
	"..\include\secImpl.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\DropEdit.obj" : $(SOURCE) $(DEP_CPP_DROPE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calc\SecCalc.cpp
DEP_CPP_SECCA=\
	"..\include\CalcBtn.h"\
	"..\include\SecAux.h"\
	"..\include\seccalc.h"\
	"..\include\secImpl.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\SecCalc.obj" : $(SOURCE) $(DEP_CPP_SECCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\calc\CalcBtn.cpp
DEP_CPP_CALCB=\
	"..\include\CalcBtn.h"\
	"..\include\SecAux.h"\
	"..\include\seccalc.h"\
	"..\include\secImpl.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\CalcBtn.obj" : $(SOURCE) $(DEP_CPP_CALCB) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utility\slocale.cpp
DEP_CPP_SLOCA=\
	"..\include\slocale.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\slocale.obj" : $(SOURCE) $(DEP_CPP_SLOCA) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\curredit.cpp
DEP_CPP_CURRE=\
	"..\include\curredit.h"\
	"..\include\dropedit.h"\
	"..\include\seccalc.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\curredit.obj" : $(SOURCE) $(DEP_CPP_CURRE) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\treectrl\TreeNode.cpp
DEP_CPP_TREEN=\
	"..\include\TreeNode.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\TreeNode.obj" : $(SOURCE) $(DEP_CPP_TREEN) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\treectrl\TrCtlX.cpp
DEP_CPP_TRCTL=\
	"..\include\slstctl.h"\
	"..\include\TrCtlX.h"\
	"..\include\TreeNode.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\TrCtlX.obj" : $(SOURCE) $(DEP_CPP_TRCTL) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\listctrl\slstctl.cpp
DEP_CPP_SLSTC=\
	"..\include\slstctip.h"\
	"..\include\slstctl.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\slstctl.obj" : $(SOURCE) $(DEP_CPP_SLSTC) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\controls\listctrl\slstctip.cpp
DEP_CPP_SLSTCT=\
	"..\include\slstctip.h"\
	"..\include\slstctl.h"\
	"..\src\stdafx.h"\
	

!IF  "$(CFG)" == "OTlib50 - Win32 OT50R"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OT50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50D"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OT50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50R"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50R.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50D"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50D.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UD"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OT50UR"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OT50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UD"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UD.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OTlib50 - Win32 OTA50UR"


"$(INTDIR)\slstctip.obj" : $(SOURCE) $(DEP_CPP_SLSTCT) "$(INTDIR)"\
 "$(INTDIR)\OTA50UR.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
