# Microsoft Developer Studio Project File - Name="AccountWizard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=AccountWizard - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AccountWizard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AccountWizard.mak" CFG="AccountWizard - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AccountWizard - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "AccountWizard - Win32 Release" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "AccountWizard"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "AccountWizard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I ".\PrivateInc" /I ".\PublicInc" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "IMAP4" /FR /Yu"stdafx.h" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg AccountWizard
# End Special Build Tool

!ELSEIF  "$(CFG)" == "AccountWizard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /Gm /GR /GX /Zi /O2 /I ".\PrivateInc" /I ".\PublicInc" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\Eudora" /D "COMMERCIAL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "IMAP4" /Yu"stdafx.h" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel AccountWizard
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "AccountWizard - Win32 Debug"
# Name "AccountWizard - Win32 Release"
# Begin Group "Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardAcapPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardClientPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardConfirmPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardFinishPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardImapDirPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardImportPage.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\WizardInServerPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardLoginPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardOutServerPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardPersNamePage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardPropSheet.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardRetAddPage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardWelcomePage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Src\WizardYourNamePage.cpp
# ADD CPP /Yu"stdafx.h"
# End Source File
# End Group
# Begin Group "PrivateInc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\PrivateInc\WizardAcapPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardClientPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardConfirmpage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardFinishPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardImapDirPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardImportPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardInServerPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardLoginPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardOutServerPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardPersNamePage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardRetAddPage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardWelcomePage.h"
# End Source File
# Begin Source File

SOURCE=".\PrivateInc\WizardYourNamePage.h"
# End Source File
# End Group
# Begin Group "PublicInc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\PublicInc\WizardPropSheet.h"
# End Source File
# End Group
# End Target
# End Project
