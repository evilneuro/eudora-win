@echo off

if "%1" == "" goto usage
if not exist %1 goto usage

copy /v Version.h      %1\
copy /v VersionBeg.inc %1\
copy /v VersionEnd.inc %1\

md %1\Eudora
copy /v Eudora\EudoraRes.dsp    %1\Eudora\
copy /v Eudora\EudoraRes.def    %1\Eudora\
copy /v Eudora\EudoraRes.cpp    %1\Eudora\
copy /v Eudora\EudoraRes.rc     %1\Eudora\
copy /v Eudora\EudoraResVer.rc  %1\Eudora\
copy /v Eudora\resource.h       %1\Eudora\

md %1\Eudora\res
del %1\Eudora\res\*
copy /v Eudora\res\* %1\Eudora\res\

md %1\BinTools
copy /v BinTools\PostProcessRel.* %1\BinTools\
copy /v BinTools\tcsh.exe         %1\BinTools\

md %1\OT501
md %1\OT501\Include
xcopy /v/s OT501\Include\* %1\OT501\Include\

md %1\Lib
md %1\Lib\Release
copy /v Lib\Release\OTA50R.lib %1\Lib\Release\

goto done

:usage

echo.
echo This utilty copies all the necessary files for building the resource DLL
echo for Eudora (Eudora32.dll).  All you need to do is specify the directory you
echo want the files to be copied to on the commandline.
echo.

:done

