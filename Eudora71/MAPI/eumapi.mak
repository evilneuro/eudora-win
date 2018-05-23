# Microsoft Visual C++ generated build script - Do not modify

PROJ = EUMAPI
DEBUG = 1
PROGTYPE = 1
CALLER = \apps\winword\winword.exe
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\EUDORA\MAPI\
USEMFC = 1
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = EUMAPI.CPP  
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /W3 /Zi /ALw /Od /D "_DEBUG" /D "_USRDLL" /D "_WINDLL" /GD /GEf /Fd"MAPI.PDB"
CFLAGS_R_WDLL = /nologo /W3 /ALw /O1 /D "NDEBUG" /D "_USRDLL" /D "_WINDLL" /GD /GEf 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LIBS_D_WDLL = lafxdwd oldnames libw ldllcew commdlg.lib ddeml.lib mmsystem.lib shell.lib ver.lib 
LIBS_R_WDLL = lafxdw oldnames libw ldllcew commdlg.lib ddeml.lib mmsystem.lib shell.lib ver.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo /t 
RUNFLAGS = 
DEFFILE = EUMAPI.DEF
OBJS_EXT = 
LIBS_EXT = 
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
SBRS = EUMAPI.SBR \
		CMCFUNC.SBR \
		MAPIFUNC.SBR \
		ADDRDLG.SBR \
		RECIPLST.SBR \
		RECIP.SBR \
		MESSAGE.SBR \
		SESMGR.SBR \
		FILEDESC.SBR \
		DDECLIEN.SBR \
		DDESTR.SBR


EUMAPI_DEP = d:\eudora\mapi\stdafx.h


CMCFUNC_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\xcmc.h


MAPIFUNC_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\eudora\eumapi.h \
	d:\eudora\eudora\mapi.h \
	d:\eudora\mapi\addrdlg.h \
	d:\eudora\mapi\reciplst.h \
	d:\eudora\mapi\sesmgr.h \
	d:\eudora\mapi\recip.h \
	d:\eudora\mapi\message.h \
	d:\eudora\eudora\ddeclien.h


ADDRDLG_DEP = d:\eudora\mapi\stdafx.h


ADDRDLG_DEP = d:\eudora\mapi\addrdlg.h \
	d:\eudora\mapi\reciplst.h \
	d:\eudora\eudora\ddeclien.h


RECIPLST_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\reciplst.h


RECIP_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\recip.h \
	d:\eudora\eudora\eumapi.h


MESSAGE_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\message.h \
	d:\eudora\eudora\eumapi.h \
	d:\eudora\mapi\recip.h \
	d:\eudora\mapi\filedesc.h \
	d:\eudora\eudora\mapi.h


SESMGR_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\eudora\eumapi.h \
	d:\eudora\eudora\mapi.h \
	d:\eudora\mapi\recip.h \
	d:\eudora\mapi\message.h \
	d:\eudora\mapi\sesmgr.h


FILEDESC_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\filedesc.h \
	d:\eudora\eudora\eumapi.h \
	d:\eudora\eudora\mapi.h


DDECLIEN_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\ddestr.h \
	d:\eudora\mapi\ddeclien.h


DDESTR_DEP = d:\eudora\mapi\stdafx.h \
	d:\eudora\mapi\ddestr.h


all:	$(PROJ).DLL

EUMAPI.OBJ:	EUMAPI.CPP $(EUMAPI_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c EUMAPI.CPP

CMCFUNC.OBJ:	CMCFUNC.CPP $(CMCFUNC_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c CMCFUNC.CPP

MAPIFUNC.OBJ:	MAPIFUNC.CPP $(MAPIFUNC_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MAPIFUNC.CPP

EUMAPI.RES:	EUMAPI.RC $(EUMAPI_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r EUMAPI.RC

ADDRDLG.OBJ:	ADDRDLG.CPP $(ADDRDLG_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ADDRDLG.CPP

RECIPLST.OBJ:	RECIPLST.CPP $(RECIPLST_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c RECIPLST.CPP

RECIP.OBJ:	RECIP.CPP $(RECIP_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c RECIP.CPP

MESSAGE.OBJ:	MESSAGE.CPP $(MESSAGE_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c MESSAGE.CPP

SESMGR.OBJ:	SESMGR.CPP $(SESMGR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c SESMGR.CPP

FILEDESC.OBJ:	FILEDESC.CPP $(FILEDESC_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c FILEDESC.CPP

DDECLIEN.OBJ:	DDECLIEN.CPP $(DDECLIEN_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c DDECLIEN.CPP

DDESTR.OBJ:	DDESTR.CPP $(DDESTR_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c DDESTR.CPP


$(PROJ).DLL::	EUMAPI.RES

$(PROJ).DLL::	EUMAPI.OBJ CMCFUNC.OBJ MAPIFUNC.OBJ ADDRDLG.OBJ RECIPLST.OBJ RECIP.OBJ \
	MESSAGE.OBJ SESMGR.OBJ FILEDESC.OBJ DDECLIEN.OBJ DDESTR.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
EUMAPI.OBJ +
CMCFUNC.OBJ +
MAPIFUNC.OBJ +
ADDRDLG.OBJ +
RECIPLST.OBJ +
RECIP.OBJ +
MESSAGE.OBJ +
SESMGR.OBJ +
FILEDESC.OBJ +
DDECLIEN.OBJ +
DDESTR.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
D:\MSVC\LIB\+
D:\MSVC\MFC\LIB\+
D:\MSDEV\LIB\+
D:\MSDEV\MFC\LIB\+
D:\OT1232\Lib\+
C:\Ot50\Lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) EUMAPI.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	EUMAPI.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) EUMAPI.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
