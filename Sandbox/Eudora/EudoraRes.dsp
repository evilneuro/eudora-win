# Microsoft Developer Studio Project File - Name="EudoraRes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=EudoraRes - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EudoraRes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EudoraRes.mak" CFG="EudoraRes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EudoraRes - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "EudoraRes - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "EudoraRes"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EudoraRes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build\Debug"
# PROP BASE Intermediate_Dir "Build\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\EuImap\public\inc" /D "COMMERCIAL" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "IMAP4" /FD /c
# SUBTRACT CPP /WX /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\OT501\Include" /d "_DEBUG" /d COMMERCIAL=1 /d "_WIN32" /d "WIN32" /d "_AFXDLL" /d "IMAP4"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /version:4.0 /entry:"DllMain" /subsystem:windows /dll /debug /machine:I386 /out:"Build\Debug\Eudora32.dll" /libpath:"..\Lib\Debug"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg Eudora32
# End Special Build Tool

!ELSEIF  "$(CFG)" == "EudoraRes - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "EudoraRe"
# PROP BASE Intermediate_Dir "EudoraRe"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\EuImap\public\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "IMAP4" /FD /c
# SUBTRACT BASE CPP /WX /YX /Yc /Yu
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\EuImap\public\inc" /D "COMMERCIAL" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /D "_AFXDLL" /D "IMAP4" /FD /c
# SUBTRACT CPP /WX /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "..\OT501\Include" /d "NDEBUG" /d COMMERCIAL=1 /d "_WIN32" /d "WIN32" /d "_AFXDLL" /d "IMAP4"
# ADD RSC /l 0x409 /i "..\OT501\Include" /d "NDEBUG" /d COMMERCIAL=1 /d "_WIN32" /d "WIN32" /d "_AFXDLL" /d "IMAP4"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /version:4.0 /entry:"DllMain" /subsystem:windows /dll /machine:I386 /out:"Build\Release\Eudora32.dll" /libpath:"..\Lib\Release"
# ADD LINK32 /nologo /base:"0x62000000" /version:4.0 /entry:"DllMain" /subsystem:windows /dll /map /debug /machine:I386 /out:"Build\Release\Eudora32.dll" /libpath:"..\Lib\Release"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel Eudora32
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "EudoraRes - Win32 Debug"
# Name "EudoraRes - Win32 Release"
# Begin Source File

SOURCE=.\res\acap.ico
# End Source File
# Begin Source File

SOURCE=.\res\Ad.ico
# End Source File
# Begin Source File

SOURCE=.\res\anetwork.ico
# End Source File
# Begin Source File

SOURCE=.\res\ARROWCOP.CUR
# End Source File
# Begin Source File

SOURCE=.\res\attach.bmp
# End Source File
# Begin Source File

SOURCE=.\res\attach.ico
# End Source File
# Begin Source File

SOURCE=.\res\attach_d.ico
# End Source File
# Begin Source File

SOURCE=.\res\attachnfound.ico
# End Source File
# Begin Source File

SOURCE=.\res\autocomp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\binhex.bmp
# End Source File
# Begin Source File

SOURCE=.\res\blah.bmp
# End Source File
# Begin Source File

SOURCE=.\res\checking.ico
# End Source File
# Begin Source File

SOURCE=.\res\checkmar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\columns.ico
# End Source File
# Begin Source File

SOURCE=.\res\compmess.ico
# End Source File
# Begin Source File

SOURCE=.\res\comptbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Credits.txt
# End Source File
# Begin Source File

SOURCE=.\res\credits2.txt
# End Source File
# Begin Source File

SOURCE=.\res\DateDisp.ico
# End Source File
# Begin Source File

SOURCE=.\res\defattach.ico
# End Source File
# Begin Source File

SOURCE=.\res\defreq.ico
# End Source File
# Begin Source File

SOURCE=.\res\deftool.ico
# End Source File
# Begin Source File

SOURCE=.\res\DeleteFromServer.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Digits_4x5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\DirectoryServices.ico
# End Source File
# Begin Source File

SOURCE=.\res\display.ico
# End Source File
# Begin Source File

SOURCE=.\res\DownloadFromServer.bmp
# End Source File
# Begin Source File

SOURCE=.\res\edit.bmp
# End Source File
# Begin Source File

SOURCE=.\res\empty.cur
# End Source File
# Begin Source File

SOURCE=.\res\eudbyqc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\eudora.ico
# End Source File
# Begin Source File

SOURCE=.\EudoraRes.cpp
# ADD CPP /Yc"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=.\EudoraRes.def
# End Source File
# Begin Source File

SOURCE=.\EudoraRes.rc

!IF  "$(CFG)" == "EudoraRes - Win32 Debug"

!ELSEIF  "$(CFG)" == "EudoraRes - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\EXTRWARN.ICO
# End Source File
# Begin Source File

SOURCE=.\res\fieldcom.ico
# End Source File
# Begin Source File

SOURCE=.\res\FileBrowse.ico
# End Source File
# Begin Source File

SOURCE=.\res\filters.bmp
# End Source File
# Begin Source File

SOURCE=.\res\filters.ico
# End Source File
# Begin Source File

SOURCE=.\res\filtrep.ico
# End Source File
# Begin Source File

SOURCE=.\RES\FIRST.ICO
# End Source File
# Begin Source File

SOURCE=.\res\fontdisp.ico
# End Source File
# Begin Source File

SOURCE=.\res\getatten.ico
# End Source File
# Begin Source File

SOURCE=.\res\getstart.ico
# End Source File
# Begin Source File

SOURCE=.\res\havemail.ico
# End Source File
# Begin Source File

SOURCE=.\res\havesend.ico
# End Source File
# Begin Source File

SOURCE=.\res\hosts.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_sear.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_task.ico
# End Source File
# Begin Source File

SOURCE=.\res\il_common16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\il_common32.bmp
# End Source File
# Begin Source File

SOURCE=.\res\il_mailtree.bmp
# End Source File
# Begin Source File

SOURCE=.\res\il_status.bmp
# End Source File
# Begin Source File

SOURCE=.\res\il_statusm.bmp
# End Source File
# Begin Source File

SOURCE=.\res\il_TaskStatus.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Imfilters.ico
# End Source File
# Begin Source File

SOURCE=.\res\ImportBanner.bmp
# End Source File
# Begin Source File

SOURCE=.\res\kerberos.ico
# End Source File
# Begin Source File

SOURCE=.\res\labels.ico
# End Source File
# Begin Source File

SOURCE=.\res\launch.cur
# End Source File
# Begin Source File

SOURCE=.\res\locked.bmp
# End Source File
# Begin Source File

SOURCE=.\res\lookup.ico
# End Source File
# Begin Source File

SOURCE=.\res\mailbox.ico
# End Source File
# Begin Source File

SOURCE=.\res\mailboxs.ico
# End Source File
# Begin Source File

SOURCE=.\res\mapi.ico
# End Source File
# Begin Source File

SOURCE=.\res\mboxes.ico
# End Source File
# Begin Source File

SOURCE=.\res\menu.cur
# End Source File
# Begin Source File

SOURCE=.\res\mime.bmp
# End Source File
# Begin Source File

SOURCE=.\res\miscell.ico
# End Source File
# Begin Source File

SOURCE=.\res\newfile.ico
# End Source File
# Begin Source File

SOURCE=.\res\newmail.wav
# End Source File
# Begin Source File

SOURCE=.\res\nickname.ico
# End Source File
# Begin Source File

SOURCE=.\res\nicktree.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nomail.ico
# End Source File
# Begin Source File

SOURCE=.\res\NOTRANS.ICO
# End Source File
# Begin Source File

SOURCE=.\res\options.ico
# End Source File
# Begin Source File

SOURCE=.\res\peanut.bmp
# End Source File
# Begin Source File

SOURCE=.\res\peanutf.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PERSONA.ICO
# End Source File
# Begin Source File

SOURCE=.\res\personal.ico
# End Source File
# Begin Source File

SOURCE=.\res\phigh.bmp
# End Source File
# Begin Source File

SOURCE=.\res\phighest.bmp
# End Source File
# Begin Source File

SOURCE=.\res\plow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\plowest.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pmenuhigh.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pmenuhighest.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pmenulow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pmenulowest.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pmenunormal.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pnormal.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pointer_.cur
# End Source File
# Begin Source File

SOURCE=.\res\prop_mult_win.bmp
# End Source File
# Begin Source File

SOURCE=.\res\prop_mult_wins.bmp
# End Source File
# Begin Source File

SOURCE=.\res\prop_tac.bmp
# End Source File
# Begin Source File

SOURCE=.\res\proptacU.bmp
# End Source File
# Begin Source File

SOURCE=.\res\qclogo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\qclogomask.bmp
# End Source File
# Begin Source File

SOURCE=.\res\readmess.ico
# End Source File
# Begin Source File

SOURCE=.\res\replying.ico
# End Source File
# Begin Source File

SOURCE=.\res\RTB1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\safety.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sdelete.bmp
# End Source File
# Begin Source File

SOURCE=.\res\search.ico
# End Source File
# Begin Source File

SOURCE=.\res\sending.ico
# End Source File
# Begin Source File

SOURCE=.\res\sendmail.ico
# End Source File
# Begin Source File

SOURCE=.\res\setcat.ico
# End Source File
# Begin Source File

SOURCE=.\res\SETTAUTO.ICO
# End Source File
# Begin Source File

SOURCE=.\res\sfetch.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sfetchde.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sig.bmp
# End Source File
# Begin Source File

SOURCE=.\res\signatur.ico
# End Source File
# Begin Source File

SOURCE=.\res\signone.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sleave.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortByAttachment.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortByDate.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortByLabel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortByPriority.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortBySender.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortByServerStatus.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortBySize.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortByStatus.bmp
# End Source File
# Begin Source File

SOURCE=.\res\SortBySubject.bmp
# End Source File
# Begin Source File

SOURCE=.\res\spelling.ico
# End Source File
# Begin Source File

SOURCE=.\res\StatBarTaskStatusIL.bmp
# End Source File
# Begin Source File

SOURCE=.\res\stationery.ico
# End Source File
# Begin Source File

SOURCE=.\res\StatusForwarded.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusQueued.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusRead.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusRecovered.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusRedirect.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusReplied.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusSendable.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusSent.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusTimeQueued.bmp
# End Source File
# Begin Source File

SOURCE=.\res\statusun.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusUnread.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusUnsendable.bmp
# End Source File
# Begin Source File

SOURCE=.\res\StatusUnsent.bmp
# End Source File
# Begin Source File

SOURCE=.\res\styltext.ico
# End Source File
# Begin Source File

SOURCE=.\res\TaskError.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TaskStatus.ico
# End Source File
# Begin Source File

SOURCE=.\res\TaskStatusSetting.ico
# End Source File
# Begin Source File

SOURCE=.\res\tbar32.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tbar32a.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tbar32b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\towtruck.ico
# End Source File
# Begin Source File

SOURCE=.\res\TRACE.ICO
# End Source File
# Begin Source File

SOURCE=.\res\transfer.bmp
# End Source File
# Begin Source File

SOURCE=.\res\urstatus.bmp
# End Source File
# Begin Source File

SOURCE=.\res\uuencode.bmp
# End Source File
# Begin Source File

SOURCE=.\res\viewmail.ico
# End Source File
# Begin Source File

SOURCE=.\res\WebBrowser.ico
# End Source File
# Begin Source File

SOURCE=.\res\WizWelcome.bmp
# End Source File
# Begin Source File

SOURCE=.\res\XRICHBAR.BMP
# End Source File
# End Target
# End Project
