# Microsoft Visual C++ generated build script - Do not modify

PROJ = ATTACH16
DEBUG = 0
PROGTYPE = 1
CALLER = ..\eudora.exe d:\mail
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\EUDORA3\PLUGINS\
USEMFC = 1
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = ATTACH.CPP  
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /W3 /Zi /AL /Gx- /Od /D "_DEBUG" /D "_USRDLL" /D "_WINDLL" /GD /GEf /Fd"ATTACH16.PDB"
CFLAGS_R_WDLL = /nologo /G2 /W3 /ALw /O2 /D "NDEBUG" /D "_USRDLL" /D "_WINDLL" /GD /GEf 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP /LINE /MAP:FULL
LIBS_D_WDLL = lafxdwd oldnames libw ldllcew commdlg.lib shell.lib 
LIBS_R_WDLL = lafxdw oldnames libw ldllcew commdlg.lib shell.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo 
RUNFLAGS = 
DEFFILE = ATTACH16.DEF
OBJS_EXT = 
LIBS_EXT = SEDLL.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = ATTACH.SBR \
		ATTACHTR.SBR


SEDLL_DEP = 

ATTACH_RCDEP = c:\eudora3\plugins\res\plugin.ico


ATTACH_DEP = c:\eudora3\plugins\stdafx.h \
	c:\eudora3\plugins\attach.h


ATTACHTR_DEP = c:\eudora3\plugins\ems-win.h \
	c:\eudora3\plugins\attach.h \
	c:\eudora3\plugins\engine.h \
	c:\eudora3\plugins\platform.h \
	c:\eudora3\plugins\sepream.h \
	c:\eudora3\plugins\setypes.h


all:	$(PROJ).DLL

ATTACH.RES:	ATTACH.RC $(ATTACH_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r ATTACH.RC

ATTACH.OBJ:	ATTACH.CPP $(ATTACH_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c ATTACH.CPP

ATTACHTR.OBJ:	ATTACHTR.CPP $(ATTACHTR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ATTACHTR.CPP


$(PROJ).DLL::	ATTACH.RES

$(PROJ).DLL::	ATTACH.OBJ ATTACHTR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
ATTACH.OBJ +
ATTACHTR.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
D:\MSVC\LIB\+
D:\MSVC\MFC\LIB\+
SEDLL.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) ATTACH.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	ATTACH.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) ATTACH.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
