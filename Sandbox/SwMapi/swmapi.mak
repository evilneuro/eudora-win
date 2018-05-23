# Microsoft Visual C++ generated build script - Do not modify

PROJ = SWMAPI
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG
R_RCDEFINES = /dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\PCEUDORA\MFC\SWMAPI\
USEMFC = 1
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = /YcSTDAFX.H
CUSEPCHFLAG = 
CPPUSEPCHFLAG = /YuSTDAFX.H
FIRSTC =             
FIRSTCPP = STDAFX.CPP  
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Gf /Zi /AL /Od /D "_DEBUG" /I ".." /GA /Fd"SWMAPI.PDB" 
CFLAGS_R_WEXE = /nologo /Gs /G3 /W3 /Gf /AL /O2 /D "NDEBUG" /I ".." /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = lafxcwd oldnames libw llibcew commdlg.lib shell.lib toolhelp.lib ver.lib 
LIBS_R_WEXE = lafxcw oldnames libw llibcew commdlg.lib shell.lib toolhelp.lib ver.lib 
RCFLAGS = /nologo /z
RESFLAGS = /nologo /t
RUNFLAGS = 
DEFFILE = SWMAPI.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = STDAFX.SBR \
		ABOUTDLG.SBR \
		MAPIINST.SBR \
		DIALOG.SBR \
		SWMAPI.SBR \
		STATUS.SBR


STDAFX_DEP = c:\pceudora\mfc\swmapi\stdafx.h


SWMAPI_RCDEP = c:\pceudora\mfc\swmapi\res\swmapi.ico \
	c:\pceudora\mfc\swmapi\res\swmapi.rc2


ABOUTDLG_DEP = c:\pceudora\mfc\swmapi\stdafx.h \
	c:\pceudora\mfc\swmapi\aboutdlg.h


MAPIINST_DEP = c:\pceudora\mfc\stdafx.h \
	c:\pceudora\mfc\mapiinst.h


DIALOG_DEP = c:\pceudora\mfc\swmapi\stdafx.h \
	c:\pceudora\mfc\swmapi\swmapi.h \
	c:\pceudora\mfc\swmapi\dialog.h \
	c:\pceudora\mfc\swmapi\status.h \
	c:\pceudora\mfc\swmapi\aboutdlg.h


SWMAPI_DEP = c:\pceudora\mfc\swmapi\stdafx.h \
	c:\pceudora\mfc\swmapi\swmapi.h \
	c:\pceudora\mfc\swmapi\dialog.h \
	c:\pceudora\mfc\swmapi\aboutdlg.h


STATUS_DEP = c:\pceudora\mfc\swmapi\stdafx.h \
	c:\pceudora\mfc\swmapi\swmapi.h \
	c:\pceudora\mfc\swmapi\status.h


all:	$(PROJ).EXE

STDAFX.OBJ:	STDAFX.CPP $(STDAFX_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c STDAFX.CPP

SWMAPI.RES:	SWMAPI.RC $(SWMAPI_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r SWMAPI.RC

ABOUTDLG.OBJ:	ABOUTDLG.CPP $(ABOUTDLG_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ABOUTDLG.CPP

MAPIINST.OBJ:	..\MAPIINST.CPP $(MAPIINST_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\MAPIINST.CPP

DIALOG.OBJ:	DIALOG.CPP $(DIALOG_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c DIALOG.CPP

SWMAPI.OBJ:	SWMAPI.CPP $(SWMAPI_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c SWMAPI.CPP

STATUS.OBJ:	STATUS.CPP $(STATUS_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c STATUS.CPP


$(PROJ).EXE::	SWMAPI.RES

$(PROJ).EXE::	STDAFX.OBJ ABOUTDLG.OBJ MAPIINST.OBJ DIALOG.OBJ SWMAPI.OBJ STATUS.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
STDAFX.OBJ +
ABOUTDLG.OBJ +
MAPIINST.OBJ +
DIALOG.OBJ +
SWMAPI.OBJ +
STATUS.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) SWMAPI.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	SWMAPI.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) SWMAPI.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
