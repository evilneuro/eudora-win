# build50.mak : Builds the 32-Bit OT library variants for VC 5.0.
#
# This is a part of the Stingray Extension classes
# Copyright (C) 1995 Stingray Software, Inc.
# All rights reserved.
#
# This source code is only intended as a supplement to
# the Stingray Extension Classes Reference and related
# electronic documentation provided with the library.
#
# Usage: NMAKE /f build50.mak CLEAN   (removes all intermediary files)
#    or: NMAKE /f build50.mak         (builds all library variants (see below))
#    or: NMAKE /f build50.mak Target  (builds a specific library variant)
#
# 'Target' can be are one of:
#   "OT50D"     32-Bit Debug library.
#
#   "OT50R"     32-Bit Release library.
#
#   "OTA50D"    32-Bit Debug library using MFC
#               as shared Library.
#
#   "OTA50R"    32-Bit Release library using MFC
#               as shared Library.
#
#   "OT5050D"  32-Bit MFC Extension (AFXDDLL) DLL.
#               Debug library.
#
#   "OT5050R"  32-Bit MFC Extension (AFXDLL) DLL.
#               Release library.
#
#   "OT50UD"    32-Bit Unicode Debug library.
#
#   "OT50UR"    32-Bit Unicode Release library.
#
#   "OTA50UD"   32-Bit Unicode Debug library using MFC
#               as shared Library.
#
#   "OTA50UR"   32-Bit Unicode Release library using MFC
#               as shared Library.
#
#   "OT5050UD"  32-Bit Unicode MFC Extension (AFXDDLL) DLL.
#               Debug library.
#
#   "OT5050UR"  32-Bit Unicode MFC Extension (AFXDLL) DLL.
#               Release library.
#
#############################################################################
THELIBDIR=..\lib
# Default build all library variants

all:    OT50D   OT50R    \
        OTA50D  OTA50R   \
        OT5050D OT5050R

unicode:    OT50UD   OT50UR    \
            OTA50UD  OTA50UR   \
            OT5050UD OT5050UR

init:
        @-set include=..\..\SGISTL;%include%;..\include;..\..\QCUtils\public\inc;..\..\EuMemMgr\Include
#        #@-if not exist "$(THELIBDIR)" mkdir "$(THELIBDIR)"

OT50D:  init
        @echo .
        @echo .
        @echo .
                @otclean OT50D
        @echo --- Building OT50D.LIB ----------
        @echo ** 32-bit, static OT, static MFC, debug build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OT50D"
        @copy OT50d\*.lib ..\lib > nul
        @copy OT50d\*.pdb ..\lib > nul

OT50R:  init
        @echo .
        @echo .
        @echo .
                @otclean OT50R
        @echo --- Building OT50R.LIB ----------
        @echo ** 32-bit, static OT, static MFC, release build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OT50R"
        @copy OT50r\*.lib ..\lib > nul

OTA50D: init
	@-if exist OTMessagedbg.bat OTMessagedbg.bat
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OTA50D"
	@-nmake.exe /nologo /f OT_SYNC.mak cfg="Debug"

OTA50R: init
	@-if exist OTMessagerel.bat OTMessagerel.bat
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OTA50R"
	@-nmake.exe /nologo /f OT_SYNC.mak cfg="Release"

OT5050D: init
        @echo .
        @echo .
        @echo .
                @otclean OT5050D
        @echo --- Building OT5050D.LIB --------
        @echo  ** 32-bit, AFXDLL OT, AFXDLL MFC, debug build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTDLL50.mak cfg="OTDLL50 - Win32 Debug" LINK32="ld5050d.bat"
        @copy OT5050d\*.lib ..\lib > nul
        @copy OT5050d\*.dll ..\lib > nul
        @copy OT5050d\*.pdb ..\lib > nul

OT5050R: init
        @echo .
        @echo .
        @echo .
                @otclean OT5050R
        @echo --- Building OT5050R.LIB --------
        @echo  ** 32-bit, AXDLL OT, AFXDLL MFC, release build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTDLL50.mak cfg="OTDLL50 - Win32 Release" LINK32="ld5050r.bat"
        @copy OT5050r\*.lib ..\lib > nul
        @copy OT5050r\*.dll ..\lib > nul


OT50UD: init
        @echo .
        @echo .
        @echo .
                @otclean OT50UD
        @echo --- Building OT50UD.LIB ----------
        @echo ** 32-bit, static OT, static MFC, UNICODE debug build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OT50UD"
        @copy OT50UD\*.lib ..\lib > nul
        @copy OT50UD\*.pdb ..\lib > nul

OT50UR: init
        @echo .
        @echo .
        @echo .
                @otclean OT50UR
        @echo --- Building OT50UR.LIB ----------
        @echo ** 32-bit, static OT, static MFC, UNICODE release build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OT50UR"
        @copy OT50UR\*.lib ..\lib > nul

OTA50UD: init
        @echo .
        @echo .
        @echo .
                @otclean OTA50UD
        @echo --- Building OTA50UD.LIB ---------
        @echo  ** 32-bit, static OT, AFXDLL MFC, UNICODE debug build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OTA50UD"
        @copy OTA50UD\*.lib ..\lib > nul
        @copy OTA50UD\*.pdb ..\lib > nul

OTA50UR: init
        @echo .
        @echo .
        @echo .
                @otclean OTA50UR
        @echo --- Building OTA50UR.LIB ---------
        @echo  ** 32-bit, static OT, AFXDLL MFC, UNICODE release build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTlib50.mak cfg="OTlib50 - Win32 OTA50UR"
        @copy OTA50UR\*.lib ..\lib > nul

OT5050UD: init
        @echo .
        @echo .
        @echo .
                @otclean OT5050UD
        @echo --- Building OT5050UD.LIB --------
        @echo  ** 32-bit, AFXDLL OT, AFXDLL MFC, UNICODE debug build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTDLL50.mak cfg="OTDLL50 - Win32 Unicode Debug" LINK32="ld5050ud.bat"
        @copy OT5050UD\*.lib ..\lib > nul
        @copy OT5050UD\*.dll ..\lib > nul
        @copy OT5050UD\*.pdb ..\lib > nul

OT5050UR: init
        @echo .
        @echo .
        @echo .
                @otclean OT5050UR
        @echo --- Building OT5050UR.LIB --------
        @echo  ** 32-bit, AXDLL OT, AFXDLL MFC, UNICODE release build, VC++ 5.0 **
        @-nmake.exe /nologo /s /f OTDLL50.mak cfg="OTDLL50 - Win32 Unicode Release" LINK32="ld5050ur.bat"
        @copy OT5050UR\*.lib ..\lib > nul
        @copy OT5050UR\*.dll ..\lib > nul


clean:
        @echo --- Cleaning ----------
        @-call otclean ot50d
        @-call otclean ot50r
        @-call otclean ota50d
        @-call otclean ota50r
        @-call otclean OT5050d
        @-call otclean OT5050r
        @-call otclean ot50ud
        @-call otclean ot50ur
        @-call otclean ota50ud
        @-call otclean ota50ur
        @-call otclean OT5050ud
        @-call otclean OT5050ur
   
clean-ota50d:
        @echo --- Cleaning ----------
        @-call otclean ota50d
        
clean-ota50r:
        @echo --- Cleaning ----------
        @-call otclean ota50r

#
# The following targets are for Stingray internal use only
#

gen_defs:       OT5050d OT5050r OT5050ud OT5050ur \
                        OT5050d.def OT5050r.def OT5050ud.def OT5050ur.def \
                        OT5050d OT5050r OT5050ud OT5050ur

regen_defs:     clean gen_defs

OT5050D.DEF: init
        @echo .
        @echo .
        @echo .
        @echo --- Regenerating OT5050d.def --------
        @echo  ** 32-bit, AXDLL OT, AFXDLL MFC, release build, VC++ 5.0 **
                @attrib -R OT5050d.def
                @..\utils\dllxport /win32 /o OT5050d.def OT5050d\*.obj
                @..\utils\prepdef OT5050d.def

OT5050R.DEF: init
        @echo .
        @echo .
        @echo .
        @echo --- Regenerating OT5050r.def --------
        @echo  ** 32-bit, AXDLL OT, AFXDLL MFC, release build, VC++ 5.0 **
                @attrib -R OT5050r.def
                @..\utils\dllxport /win32 /o OT5050r.def OT5050r\*.obj
                @..\utils\prepdef OT5050r.def

OT5050UD.DEF: init
        @echo .
        @echo .
        @echo .
        @echo --- Regenerating OT5050ud.def --------
        @echo  ** 32-bit, AXDLL OT, AFXDLL MFC, UNICODE release build, VC++ 5.0 **
                @attrib -R OT5050ud.def
                @..\utils\dllxport /win32 /o OT5050ud.def OT5050ud\*.obj
                @..\utils\prepdef OT5050ud.def

OT5050UR.DEF: init
        @echo .
        @echo .
        @echo .
        @echo --- Regenerating OT5050ur.def --------
        @echo  ** 32-bit, AXDLL OT, AFXDLL MFC, UNICODE release build, VC++ 5.0 **
                @attrib -R OT5050ur.def
                @..\utils\dllxport /win32 /o OT5050ur.def OT5050ur\*.obj
                @..\utils\prepdef OT5050ur.def
