# Microsoft Developer Studio Project File - Name="EudoraExe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=EudoraExe - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EudoraExe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EudoraExe.mak" CFG="EudoraExe - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EudoraExe - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "EudoraExe - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName "EudoraExe"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EudoraExe - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "..\SearchEngine\PublicInc" /I "..\SpeechSDK\Include" /I "..\AccountWizard\PublicInc" /I "..\PaigeDLL\PGHEADER" /I "..\Qt3.x\CIncludes" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\QCSocket\inc" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\EuImap\public\inc" /I "..\imapdll\public\inc" /I "..\Qt\include" /I "..\DirectoryServices\DirectoryServicesUI\public\inc" /D "_DEBUG" /D "COMMERCIAL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "OEMRESOURCE" /D "_AFXDLL" /D "WIN32_COMPILE" /D "IMAP4" /D "EXPFLAG" /D "THREADED" /D "_ATL_STATIC_REGISTRY" /Fr /Yu"stdafx.h" /FD /c /Tp
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\imapdll\public\inc" /i "..\OT501\Include" /i "..\EuImap\public\inc" /d "_DEBUG" /d COMMERCIAL=1 /d "IMAP4" /d "WIN32" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ole32.lib oleaut32.lib Paige32d.lib imap.lib QCSocket.lib wininet.lib DirectoryServicesUI.lib NewMgr.lib QCUtils.lib oldnames.lib winmm.lib version.lib ctl3d32s.lib sscewd32.lib Eudora32.lib EuImap.lib AccountWizard.lib qtmlclient.lib vfw32.lib spchwrap.lib /nologo /version:4.0 /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\bin\Debug\Eudora.exe" /libpath:"..\Lib\Debug" /libpath:"..\QCSocket\Build\Debug" /libpath:"..\Qt3.x\Libraries" /libpath:"..\SpeechSDK\Lib" /libpath:"..\INetSDK\Lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessDbg Eudora
# End Special Build Tool

!ELSEIF  "$(CFG)" == "EudoraExe - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "EudoraEx"
# PROP BASE Intermediate_Dir "EudoraEx"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /O2 /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\QCSocket\inc" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\PgHeaders" /I "..\EuImap\public\inc" /I "..\imapdll\public\inc" /I "..\Qt\include" /I "..\DirectoryServices\DirectoryServicesUI\public\inc" /D "NDEBUG" /D COMMERCIAL=1 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "OEMRESOURCE" /D "_AFXDLL" /D "WIN32_COMPILE" /D "IMAP4" /D "EXPFLAG" /D "THREADED" /Yu"stdafx.h" /FD /c /Tp
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W4 /GR /GX /Zi /O2 /I "..\SearchEngine\PublicInc" /I "..\SpeechSDK\Include" /I "..\AccountWizard\PublicInc" /I "..\PaigeDLL\PGHEADER" /I "..\Qt3.x\CIncludes" /I "..\SGISTL" /I "..\QCUtils\public\inc" /I "..\EuMemMgr\public\inc\AssertionServices" /I "..\EuMemMgr\public\inc\Core" /I "..\EuMemMgr\public\inc\CoreMT" /I "..\EuMemMgr\public\inc\Generic" /I "..\NewMgr\public\inc\CoreMT" /I "..\QCSocket\inc" /I "..\INetSDK\Include" /I "..\OT501\Include" /I "..\EuImap\public\inc" /I "..\imapdll\public\inc" /I "..\Qt\include" /I "..\DirectoryServices\DirectoryServicesUI\public\inc" /D EXPIRING=1 /D "NDEBUG" /D COMMERCIAL=1 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "OEMRESOURCE" /D "_AFXDLL" /D "WIN32_COMPILE" /D "IMAP4" /D "EXPFLAG" /D "THREADED" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c /Tp
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "..\imapdll\public\inc" /i "..\OT501\Include" /i "..\EuImap\public\inc" /d "NDEBUG" /d COMMERCIAL=1 /d "WIN32" /d "_AFXDLL" /d "IMAP4"
# ADD RSC /l 0x409 /i "..\imapdll\public\inc" /i "..\OT501\Include" /i "..\EuImap\public\inc" /d "NDEBUG" /d COMMERCIAL=1 /d "WIN32" /d "_AFXDLL" /d "IMAP4"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wininet.lib Paige32.lib Imap.lib wininet.lib DirectoryServicesUI.lib NewMgr.lib QCUtils.lib oldnames.lib winmm.lib version.lib ctl3d32s.lib sscewd32.lib Eudora32.lib EuImap.lib Qtim32.lib /nologo /version:4.0 /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /out:"..\bin\ProRelease\Eudora.exe" /libpath:"..\Lib\ProRelease" /libpath:"..\INetSDK\Lib"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 Paige32.lib Imap.lib wininet.lib DirectoryServicesUI.lib NewMgr.lib QCUtils.lib oldnames.lib winmm.lib version.lib ctl3d32s.lib sscewd32.lib Eudora32.lib EuImap.lib AccountWizard.lib qtmlclient.lib vfw32.lib spchwrap.lib /nologo /version:4.0 /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\bin\Release\Eudora.exe" /libpath:"..\Lib\Release" /libpath:"..\Qt3.x\Libraries" /libpath:"..\SpeechSDK\Lib" /libpath:"..\INetSDK\Lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=..\BinTools\PostProcessRel Eudora
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "EudoraExe - Win32 Debug"
# Name "EudoraExe - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\3dformv.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutEMS.cpp
# End Source File
# Begin Source File

SOURCE=.\acapdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\address.cpp
# End Source File
# Begin Source File

SOURCE=.\AdView.cpp
# End Source File
# Begin Source File

SOURCE=.\AdWazooBar.cpp
# End Source File
# Begin Source File

SOURCE=.\AdWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\AMBIENTS.CPP
# End Source File
# Begin Source File

SOURCE=.\AuthRPA.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoCompleteSearcher.cpp
# End Source File
# Begin Source File

SOURCE=.\Automation.cpp
# End Source File
# Begin Source File

SOURCE=.\Base64.cpp
# End Source File
# Begin Source File

SOURCE=.\BidentPreviewView.cpp
# End Source File
# Begin Source File

SOURCE=.\BidentReadMessageView.cpp
# End Source File
# Begin Source File

SOURCE=.\BidentView.cpp
# End Source File
# Begin Source File

SOURCE=.\binhex.cpp
# End Source File
# Begin Source File

SOURCE=.\BlahBlahBlahView.cpp
# End Source File
# Begin Source File

SOURCE=.\bmpcombo.cpp
# End Source File
# Begin Source File

SOURCE=.\Bstr.cpp
# End Source File
# Begin Source File

SOURCE=.\changeq.cpp
# End Source File
# Begin Source File

SOURCE=.\ClipboardMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\COLORLIS.CPP
# End Source File
# Begin Source File

SOURCE=.\CompMessageFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\compmsgd.cpp
# End Source File
# Begin Source File

SOURCE=.\ContainerView.cpp
# End Source File
# Begin Source File

SOURCE=.\controls.cpp
# End Source File
# Begin Source File

SOURCE=.\Convhtml.cpp
# End Source File
# Begin Source File

SOURCE=.\CramMd5.cpp
# End Source File
# Begin Source File

SOURCE=.\cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\ddeclien.cpp
# End Source File
# Begin Source File

SOURCE=.\ddeserve.cpp
# End Source File
# Begin Source File

SOURCE=.\ddestr.cpp
# End Source File
# Begin Source File

SOURCE=.\DelSigDg.cpp
# End Source File
# Begin Source File

SOURCE=.\DMMDumpLogDlgImpl.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\doc.cpp
# End Source File
# Begin Source File

SOURCE=.\ehlo.cpp
# End Source File
# Begin Source File

SOURCE=.\etf2html.cpp
# End Source File
# Begin Source File

SOURCE=.\eudora.cpp
# End Source File
# Begin Source File

SOURCE=.\EudoraExe.cpp
# End Source File
# Begin Source File

SOURCE=.\EudoraExe.idl
USERDEP__EUDOR="EudoraExe.idl"	

!IF  "$(CFG)" == "EudoraExe - Win32 Debug"

# Begin Custom Build
InputPath=.\EudoraExe.idl

BuildCmds= \
	midl /Oicf /h "EudoraExe.h" /iid "EudoraExe_i.c" /tlb "EudoraExe.tlb"\
                                                   "EudoraExe.idl"

"EudoraExe.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EudoraExe_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EudoraExe.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "EudoraExe - Win32 Release"

# Begin Custom Build
InputPath=.\EudoraExe.idl

BuildCmds= \
	midl /Oicf /h "EudoraExe.h" /iid "EudoraExe_i.c" /tlb "EudoraExe.tlb"\
                                                   "EudoraExe.idl"

"EudoraExe.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EudoraExe_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"EudoraExe.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\eudordoc.cpp
# End Source File
# Begin Source File

SOURCE=.\eudorvw.cpp
# End Source File
# Begin Source File

SOURCE=.\ExceptionHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\FaceNameDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBrowseView.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBrowseWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\FileLineReader.cpp
# End Source File
# Begin Source File

SOURCE=.\fileutil.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterReportView.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterReportWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\filtersd.cpp
# End Source File
# Begin Source File

SOURCE=.\filtersv.cpp
# End Source File
# Begin Source File

SOURCE=.\FiltersWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\GenericReader.cpp
# End Source File
# Begin Source File

SOURCE=.\GetMail.cpp
# End Source File
# Begin Source File

SOURCE=.\guiutils.cpp
# End Source File
# Begin Source File

SOURCE=.\header.cpp
# End Source File
# Begin Source File

SOURCE=.\headervw.cpp
# End Source File
# Begin Source File

SOURCE=.\helpcntx.cpp
# End Source File
# Begin Source File

SOURCE=.\helpxdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\hexbin.cpp
# End Source File
# Begin Source File

SOURCE=.\hostlist.cpp
# End Source File
# Begin Source File

SOURCE=.\html2text.cpp
# End Source File
# Begin Source File

SOURCE=.\IADVSINK.CPP
# End Source File
# Begin Source File

SOURCE=.\ICLISITE.CPP
# End Source File
# Begin Source File

SOURCE=.\IDocHostShowUI.cpp
# End Source File
# Begin Source File

SOURCE=.\IDocHostUIHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\IDOCSITE.CPP
# End Source File
# Begin Source File

SOURCE=.\IIPSITE.CPP
# End Source File
# Begin Source File

SOURCE=.\ImportMailDlog.cpp
# End Source File
# Begin Source File

SOURCE=.\InsertLinkDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Kerberos.cpp
# End Source File
# Begin Source File

SOURCE=.\LabelComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\lex822.cpp
# End Source File
# Begin Source File

SOURCE=.\ListCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\mainfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MakeFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\mapicmc.cpp
# End Source File
# Begin Source File

SOURCE=.\MAPIImport.cpp
# End Source File
# Begin Source File

SOURCE=.\mapiinst.cpp
# End Source File
# Begin Source File

SOURCE=.\MboxConverters.cpp
# End Source File
# Begin Source File

SOURCE=.\mboxtree.cpp
# End Source File
# Begin Source File

SOURCE=.\MBoxTreeCtrlCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\MboxWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\mdichild.cpp
# End Source File
# Begin Source File

SOURCE=.\mdidrag.cpp
# End Source File
# Begin Source File

SOURCE=.\mime.cpp
# End Source File
# Begin Source File

SOURCE=.\MIMEMap.cpp
# End Source File
# Begin Source File

SOURCE=.\MimeStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\ModifyAcctDlgs.cpp
# End Source File
# Begin Source File

SOURCE=.\ModifyAcctSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\msgdoc.cpp
# End Source File
# Begin Source File

SOURCE=.\msgframe.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgLabel.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgOpts.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgRecord.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgSplitter.cpp
# End Source File
# Begin Source File

SOURCE=.\msgutils.cpp
# End Source File
# Begin Source File

SOURCE=.\MTRANSOP.CPP
# End Source File
# Begin Source File

SOURCE=.\MultiReader.cpp
# End Source File
# Begin Source File

SOURCE=.\namenick.cpp
# End Source File
# Begin Source File

SOURCE=.\NameToolBarDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\newmbox.cpp
# End Source File
# Begin Source File

SOURCE=.\NewSignatureDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\nickdoc.cpp
# End Source File
# Begin Source File

SOURCE=.\NicknamesWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\nickpage.cpp
# End Source File
# Begin Source File

SOURCE=.\nicksht.cpp
# End Source File
# Begin Source File

SOURCE=.\nicktree.cpp
# End Source File
# Begin Source File

SOURCE=.\nickview.cpp
# End Source File
# Begin Source File

SOURCE=.\Paige_io.cpp
# End Source File
# Begin Source File

SOURCE=.\PaigeEdtView.cpp
# End Source File
# Begin Source File

SOURCE=.\PaigePreviewDC.cpp
# End Source File
# Begin Source File

SOURCE=.\PaigeStyle.cpp
# End Source File
# Begin Source File

SOURCE=.\password.cpp
# End Source File
# Begin Source File

SOURCE=.\persona.cpp
# End Source File
# Begin Source File

SOURCE=.\PersonalityView.cpp
# End Source File
# Begin Source File

SOURCE=.\PersonalityWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\PersParams.cpp
# End Source File
# Begin Source File

SOURCE=.\PgCompMsgView.cpp
# End Source File
# Begin Source File

SOURCE=.\PGDEFTBL.C
# End Source File
# Begin Source File

SOURCE=.\PgDocumentFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\PgEmbeddedImage.cpp
# End Source File
# Begin Source File

SOURCE=.\PGEXPORT.CPP
# End Source File
# Begin Source File

SOURCE=.\PgFixedCompMsgView.cpp
# End Source File
# Begin Source File

SOURCE=.\PgGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\PgHLinks.cpp
# End Source File
# Begin Source File

SOURCE=.\PGHTMDEF.C
# End Source File
# Begin Source File

SOURCE=.\PGHTMEXP.CPP
# End Source File
# Begin Source File

SOURCE=.\PGHTMIMP.CPP
# End Source File
# Begin Source File

SOURCE=.\PGIMPORT.CPP
# End Source File
# Begin Source File

SOURCE=.\PgMsgView.cpp
# End Source File
# Begin Source File

SOURCE=.\PGNATIVE.CPP
# End Source File
# Begin Source File

SOURCE=.\PgReadMsgPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\PgReadMsgView.cpp
# End Source File
# Begin Source File

SOURCE=.\PGRTFDEF.C
# End Source File
# Begin Source File

SOURCE=.\PGRTFEXP.CPP
# End Source File
# Begin Source File

SOURCE=.\PGRTFIMP.CPP
# End Source File
# Begin Source File

SOURCE=.\PgStuffBucket.cpp
# End Source File
# Begin Source File

SOURCE=.\PgStyleUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\PgTextTranslator.cpp
# End Source File
# Begin Source File

SOURCE=.\PGWMF.C
# End Source File
# Begin Source File

SOURCE=.\PooterButton.cpp
# End Source File
# Begin Source File

SOURCE=.\pop.cpp
# End Source File
# Begin Source File

SOURCE=.\pophost.cpp
# End Source File
# Begin Source File

SOURCE=.\POPSession.cpp
# End Source File
# Begin Source File

SOURCE=.\progress.cpp
# End Source File
# Begin Source File

SOURCE=.\QC3DTabWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\QCAutomationCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCAutomationDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCChildToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCommandDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCommandObject.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCommandStack.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeGeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizePersonalitiesPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizePluginsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeRecipientsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeSignaturesPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeStationeryPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\QCCustomToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\QCError.cpp
# End Source File
# Begin Source File

SOURCE=.\QCFindMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\QCGraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\QCImageList.cpp
# End Source File
# Begin Source File

SOURCE=.\QCLabelDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCMailboxCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCMailboxCustomizeToolbarPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCMailboxDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCMailboxTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\QCMailSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\QCOleDropSource.cpp
# End Source File
# Begin Source File

SOURCE=.\QComApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\QComFolder.cpp
# End Source File
# Begin Source File

SOURCE=.\QComFolders.cpp
# End Source File
# Begin Source File

SOURCE=.\QComMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\QComMessages.cpp
# End Source File
# Begin Source File

SOURCE=.\QCPersonalityCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCPersonalityDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCPluginCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCPluginDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCProtocol.cpp
# End Source File
# Begin Source File

SOURCE=.\QCRecipientCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCRecipientDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCSharewareManager.cpp
# End Source File
# Begin Source File

SOURCE=.\QCSignatureCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCSignatureDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCSocketStub.cpp
# End Source File
# Begin Source File

SOURCE=.\QCStationeryCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\QCStationeryDirector.cpp
# End Source File
# Begin Source File

SOURCE=.\QCTaskManager.cpp
# End Source File
# Begin Source File

SOURCE=.\QCToolbarCmdPage.cpp
# End Source File
# Begin Source File

SOURCE=.\QCToolBarManager.cpp
# End Source File
# Begin Source File

SOURCE=.\qctree.cpp
# End Source File
# Begin Source File

SOURCE=.\QCWorkerThreadMT.cpp
# End Source File
# Begin Source File

SOURCE=.\QICommandClient.cpp
# End Source File
# Begin Source File

SOURCE=.\QP.cpp
# End Source File
# Begin Source File

SOURCE=.\qtwrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMessageDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMessageFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMessageToolBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadReceiptDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\regif.cpp
# End Source File
# Begin Source File

SOURCE=.\reltoabs.cpp
# End Source File
# Begin Source File

SOURCE=.\RInstDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\rs.cpp
# End Source File
# Begin Source File

SOURCE=.\sasl.cpp
# End Source File
# Begin Source File

SOURCE=.\saveas.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveAsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchView.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectMBox.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectPersonalityDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sendmail.cpp
# End Source File
# Begin Source File

SOURCE=.\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\signatur.cpp
# End Source File
# Begin Source File

SOURCE=.\SignatureTree.cpp
# End Source File
# Begin Source File

SOURCE=.\SignatureWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\SingleReader.cpp
# End Source File
# Begin Source File

SOURCE=.\SITE.CPP
# End Source File
# Begin Source File

SOURCE=.\SMTPSession.cpp
# End Source File
# Begin Source File

SOURCE=.\spell.cpp
# End Source File
# Begin Source File

SOURCE=.\SpellDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SpellEditLexDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SpellOptionsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\spellpaige.cpp
# End Source File
# Begin Source File

SOURCE=.\SplitHlp.cpp
# End Source File
# Begin Source File

SOURCE=.\SpoolMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\statbar.cpp
# End Source File
# Begin Source File

SOURCE=.\StationeryTree.cpp
# End Source File
# Begin Source File

SOURCE=.\StationeryWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StripRich.cpp
# End Source File
# Begin Source File

SOURCE=.\summary.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskErrorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskErrorMT.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskErrorView.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskErrorWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskObject.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskStatus.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskStatusView.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskStatusWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskSummary.cpp
# End Source File
# Begin Source File

SOURCE=.\TBarBmpCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\TBarCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\TBarEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\TBarMenuButton.cpp
# End Source File
# Begin Source File

SOURCE=.\TBarSendButton.cpp
# End Source File
# Begin Source File

SOURCE=.\TBarStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\Text2Html.cpp
# End Source File
# Begin Source File

SOURCE=.\textfile.cpp
# End Source File
# Begin Source File

SOURCE=.\TextReader.cpp
# End Source File
# Begin Source File

SOURCE=.\TextToSpeech.cpp
# End Source File
# Begin Source File

SOURCE=.\timestmp.cpp
# End Source File
# Begin Source File

SOURCE=.\TipOfDayDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\tocdoc.cpp
# End Source File
# Begin Source File

SOURCE=.\TocFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\tocview.cpp
# End Source File
# Begin Source File

SOURCE=.\TransReader.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeCtrlCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\TridentPreviewView.cpp
# End Source File
# Begin Source File

SOURCE=.\TridentReadMessageView.cpp
# End Source File
# Begin Source File

SOURCE=.\TridentView.cpp
# End Source File
# Begin Source File

SOURCE=.\Trnslate.cpp
# End Source File
# Begin Source File

SOURCE=.\urledit.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\uudecode.cpp
# End Source File
# Begin Source File

SOURCE=.\ValidEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\WarnEditHTMLDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\WazooBar.cpp
# End Source File
# Begin Source File

SOURCE=.\WazooBarMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\WazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\WebBrowserView.cpp
# End Source File
# Begin Source File

SOURCE=.\WebBrowserWazooWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\workbook.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\3dformv.h
# End Source File
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AboutEMS.h
# End Source File
# Begin Source File

SOURCE=.\acapdlg.h
# End Source File
# Begin Source File

SOURCE=.\address.h
# End Source File
# Begin Source File

SOURCE=.\AdView.h
# End Source File
# Begin Source File

SOURCE=.\AdWazooBar.h
# End Source File
# Begin Source File

SOURCE=.\AdWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\all.h
# End Source File
# Begin Source File

SOURCE=.\Anchor.h
# End Source File
# Begin Source File

SOURCE=.\AuthRPA.h
# End Source File
# Begin Source File

SOURCE=.\AutoCompleteSearcher.h
# End Source File
# Begin Source File

SOURCE=.\Automation.h
# End Source File
# Begin Source File

SOURCE=.\Base64.h
# End Source File
# Begin Source File

SOURCE=.\binhex.h
# End Source File
# Begin Source File

SOURCE=.\BlahBlahBlahView.h
# End Source File
# Begin Source File

SOURCE=.\bmpcombo.h
# End Source File
# Begin Source File

SOURCE=.\Bstr.h
# End Source File
# Begin Source File

SOURCE=.\Callbacks.h
# End Source File
# Begin Source File

SOURCE=.\CEudoraModule.h
# End Source File
# Begin Source File

SOURCE=.\changeq.h
# End Source File
# Begin Source File

SOURCE=.\ClipboardMgr.h
# End Source File
# Begin Source File

SOURCE=.\COLORLIS.H
# End Source File
# Begin Source File

SOURCE=.\CompMessageFrame.h
# End Source File
# Begin Source File

SOURCE=.\compmsgd.h
# End Source File
# Begin Source File

SOURCE=.\ContainerView.h
# End Source File
# Begin Source File

SOURCE=.\controls.h
# End Source File
# Begin Source File

SOURCE=.\cursor.h
# End Source File
# Begin Source File

SOURCE=.\ddeclien.h
# End Source File
# Begin Source File

SOURCE=.\ddeserve.h
# End Source File
# Begin Source File

SOURCE=.\ddestr.h
# End Source File
# Begin Source File

SOURCE=..\QCUtils\public\inc\debug.h
# End Source File
# Begin Source File

SOURCE=.\Decode.h
# End Source File
# Begin Source File

SOURCE=.\DelSigDg.h
# End Source File
# Begin Source File

SOURCE=.\DMMDumpLogDlgIface.h
# End Source File
# Begin Source File

SOURCE=.\DMMDumpLogDlgImpl.h
# End Source File
# Begin Source File

SOURCE=.\doc.h
# End Source File
# Begin Source File

SOURCE=.\ehlo.h
# End Source File
# Begin Source File

SOURCE=".\ems-wglu.h"
# End Source File
# Begin Source File

SOURCE=.\Encode.h
# End Source File
# Begin Source File

SOURCE=.\etf2html.h
# End Source File
# Begin Source File

SOURCE=.\eudora.h
# End Source File
# Begin Source File

SOURCE=.\EudoraMsgs.h
# End Source File
# Begin Source File

SOURCE=.\eudordoc.h
# End Source File
# Begin Source File

SOURCE=.\eudorvw.h
# End Source File
# Begin Source File

SOURCE=.\eumapi.h
# End Source File
# Begin Source File

SOURCE=.\ExceptionHandler.h
# End Source File
# Begin Source File

SOURCE=.\FaceNameDialog.h
# End Source File
# Begin Source File

SOURCE=.\FileBrowseView.h
# End Source File
# Begin Source File

SOURCE=.\FileBrowseWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\FileLineReader.h
# End Source File
# Begin Source File

SOURCE=.\fileutil.h
# End Source File
# Begin Source File

SOURCE=.\FilterReportView.h
# End Source File
# Begin Source File

SOURCE=.\FilterReportWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\filtersd.h
# End Source File
# Begin Source File

SOURCE=.\filtersv.h
# End Source File
# Begin Source File

SOURCE=.\FiltersWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\GenericReader.h
# End Source File
# Begin Source File

SOURCE=.\guiutils.h
# End Source File
# Begin Source File

SOURCE=.\header.h
# End Source File
# Begin Source File

SOURCE=.\headervw.h
# End Source File
# Begin Source File

SOURCE=.\helpcntx.h
# End Source File
# Begin Source File

SOURCE=.\helpxdlg.h
# End Source File
# Begin Source File

SOURCE=.\hexbin.h
# End Source File
# Begin Source File

SOURCE=.\hostlist.h
# End Source File
# Begin Source File

SOURCE=.\ImportMailDlog.h
# End Source File
# Begin Source File

SOURCE=.\InsertLinkDialog.h
# End Source File
# Begin Source File

SOURCE=.\jjnet.h
# End Source File
# Begin Source File

SOURCE=.\KCLIENT.H
# End Source File
# Begin Source File

SOURCE=.\LabelComboBox.h
# End Source File
# Begin Source File

SOURCE=.\lex822.h
# End Source File
# Begin Source File

SOURCE=.\LineReader.h
# End Source File
# Begin Source File

SOURCE=.\ListCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\mainfrm.h
# End Source File
# Begin Source File

SOURCE=.\mapi.h
# End Source File
# Begin Source File

SOURCE=.\MAPIImport.h
# End Source File
# Begin Source File

SOURCE=.\mapiinst.h
# End Source File
# Begin Source File

SOURCE=.\MboxConverters.h
# End Source File
# Begin Source File

SOURCE=.\mboxtree.h
# End Source File
# Begin Source File

SOURCE=.\MBoxTreeCtrlCheck.h
# End Source File
# Begin Source File

SOURCE=.\MboxWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\mdichild.h
# End Source File
# Begin Source File

SOURCE=.\mdidrag.h
# End Source File
# Begin Source File

SOURCE=.\mime.h
# End Source File
# Begin Source File

SOURCE=.\MIMEMap.h
# End Source File
# Begin Source File

SOURCE=.\MIMEReader.h
# End Source File
# Begin Source File

SOURCE=.\MimeStorage.h
# End Source File
# Begin Source File

SOURCE=.\msgdoc.h
# End Source File
# Begin Source File

SOURCE=.\msgframe.h
# End Source File
# Begin Source File

SOURCE=.\MsgLabel.h
# End Source File
# Begin Source File

SOURCE=.\MsgOpts.h
# End Source File
# Begin Source File

SOURCE=.\MsgRecord.h
# End Source File
# Begin Source File

SOURCE=.\MsgSplitter.h
# End Source File
# Begin Source File

SOURCE=.\msgutils.h
# End Source File
# Begin Source File

SOURCE=.\MTRANSOP.H
# End Source File
# Begin Source File

SOURCE=.\MultiReader.h
# End Source File
# Begin Source File

SOURCE=.\namenick.h
# End Source File
# Begin Source File

SOURCE=.\newmbox.h
# End Source File
# Begin Source File

SOURCE=.\NewSignatureDialog.h
# End Source File
# Begin Source File

SOURCE=.\nickdoc.h
# End Source File
# Begin Source File

SOURCE=.\NicknamesWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\nickpage.h
# End Source File
# Begin Source File

SOURCE=.\nicksht.h
# End Source File
# Begin Source File

SOURCE=.\nicktree.h
# End Source File
# Begin Source File

SOURCE=.\nickview.h
# End Source File
# Begin Source File

SOURCE=.\Paige_io.h
# End Source File
# Begin Source File

SOURCE=.\PaigeEdtView.h
# End Source File
# Begin Source File

SOURCE=.\PaigePreviewDC.h
# End Source File
# Begin Source File

SOURCE=.\PaigeStyle.h
# End Source File
# Begin Source File

SOURCE=.\password.h
# End Source File
# Begin Source File

SOURCE=.\persona.h
# End Source File
# Begin Source File

SOURCE=.\PersonalityView.h
# End Source File
# Begin Source File

SOURCE=.\PersonalityWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\PersParams.h
# End Source File
# Begin Source File

SOURCE=.\PgCompMsgView.h
# End Source File
# Begin Source File

SOURCE=.\PgDocumentFrame.h
# End Source File
# Begin Source File

SOURCE=.\PgEmbeddedImage.h
# End Source File
# Begin Source File

SOURCE=.\PgFixedCompMsgView.h
# End Source File
# Begin Source File

SOURCE=.\PgGlobals.h
# End Source File
# Begin Source File

SOURCE=.\PgHLinks.h
# End Source File
# Begin Source File

SOURCE=.\PgMsgView.h
# End Source File
# Begin Source File

SOURCE=.\PgReadMsgPreview.h
# End Source File
# Begin Source File

SOURCE=.\PgReadMsgView.h
# End Source File
# Begin Source File

SOURCE=.\PgStuffBucket.h
# End Source File
# Begin Source File

SOURCE=.\PgStyleUtils.h
# End Source File
# Begin Source File

SOURCE=.\PgTextTranslator.h
# End Source File
# Begin Source File

SOURCE=.\PooterButton.h
# End Source File
# Begin Source File

SOURCE=.\pop.h
# End Source File
# Begin Source File

SOURCE=.\pophost.h
# End Source File
# Begin Source File

SOURCE=.\POPSession.h
# End Source File
# Begin Source File

SOURCE=.\progress.h
# End Source File
# Begin Source File

SOURCE=.\QC3DTabWnd.h
# End Source File
# Begin Source File

SOURCE=.\QCAutomationCommand.h
# End Source File
# Begin Source File

SOURCE=.\QCAutomationDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCChildToolBar.h
# End Source File
# Begin Source File

SOURCE=.\QCCommandActions.h
# End Source File
# Begin Source File

SOURCE=.\QCCommandDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCCommandObject.h
# End Source File
# Begin Source File

SOURCE=.\QCCommandStack.h
# End Source File
# Begin Source File

SOURCE=.\QCCustomizePluginsPage.h
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeRecipientsPage.h
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeSignaturesPage.h
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeStationeryPage.h
# End Source File
# Begin Source File

SOURCE=.\QCCustomizeToolBar.h
# End Source File
# Begin Source File

SOURCE=.\QCCustomToolBar.h
# End Source File
# Begin Source File

SOURCE=.\QCFileIO.h
# End Source File
# Begin Source File

SOURCE=.\QCFindMgr.h
# End Source File
# Begin Source File

SOURCE=.\QCGraphics.h
# End Source File
# Begin Source File

SOURCE=.\QCImageList.h
# End Source File
# Begin Source File

SOURCE=.\QCLabelDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCMailboxCommand.h
# End Source File
# Begin Source File

SOURCE=.\QCMailboxCustomizeToolbarPage.h
# End Source File
# Begin Source File

SOURCE=.\QCMailboxDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCMailboxTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\QCNetSettings.h
# End Source File
# Begin Source File

SOURCE=.\QCOleDropSource.h
# End Source File
# Begin Source File

SOURCE=.\QComApplication.h
# End Source File
# Begin Source File

SOURCE=.\QComApplicationEvents.h
# End Source File
# Begin Source File

SOURCE=.\QComFolder.h
# End Source File
# Begin Source File

SOURCE=.\QComFolders.h
# End Source File
# Begin Source File

SOURCE=.\QComMessage.h
# End Source File
# Begin Source File

SOURCE=.\QComMessages.h
# End Source File
# Begin Source File

SOURCE=.\QCPluginCommand.h
# End Source File
# Begin Source File

SOURCE=.\QCPluginDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCPOPSettings.h
# End Source File
# Begin Source File

SOURCE=.\QCProtocol.h
# End Source File
# Begin Source File

SOURCE=.\QCRecipientCommand.h
# End Source File
# Begin Source File

SOURCE=.\QCRecipientDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCSharewareManager.h
# End Source File
# Begin Source File

SOURCE=.\QCSignatureCommand.h
# End Source File
# Begin Source File

SOURCE=.\QCSignatureDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCSMTPSettings.h
# End Source File
# Begin Source File

SOURCE=.\QCStationeryCommand.h
# End Source File
# Begin Source File

SOURCE=.\QCStationeryDirector.h
# End Source File
# Begin Source File

SOURCE=.\QCTaskManager.h
# End Source File
# Begin Source File

SOURCE=.\QCToolbarCmdPage.h
# End Source File
# Begin Source File

SOURCE=.\QCToolBarManager.h
# End Source File
# Begin Source File

SOURCE=.\qctree.h
# End Source File
# Begin Source File

SOURCE=.\QCWorkerThreadMT.h
# End Source File
# Begin Source File

SOURCE=.\QICommandClient.h
# End Source File
# Begin Source File

SOURCE=.\QP.h
# End Source File
# Begin Source File

SOURCE=.\qtwrapper.h
# End Source File
# Begin Source File

SOURCE=.\ReadMessageDoc.h
# End Source File
# Begin Source File

SOURCE=.\ReadMessageFrame.h
# End Source File
# Begin Source File

SOURCE=.\ReadReceiptDialog.h
# End Source File
# Begin Source File

SOURCE=.\regif.h
# End Source File
# Begin Source File

SOURCE=.\relapp.h
# End Source File
# Begin Source File

SOURCE=.\reltoabs.h
# End Source File
# Begin Source File

SOURCE=.\rescomm.h
# End Source File
# Begin Source File

SOURCE=.\resdel.h
# End Source File
# Begin Source File

SOURCE=.\reseud.h
# End Source File
# Begin Source File

SOURCE=.\resfree.h
# End Source File
# Begin Source File

SOURCE=.\resizbar.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource16.h
# End Source File
# Begin Source File

SOURCE=.\resource32.h
# End Source File
# Begin Source File

SOURCE=.\RInstDlg.h
# End Source File
# Begin Source File

SOURCE=.\rs.h
# End Source File
# Begin Source File

SOURCE=.\sasl.h
# End Source File
# Begin Source File

SOURCE=.\saveas.h
# End Source File
# Begin Source File

SOURCE=.\SaveAsDialog.h
# End Source File
# Begin Source File

SOURCE=.\SearchDoc.h
# End Source File
# Begin Source File

SOURCE=.\SearchView.h
# End Source File
# Begin Source File

SOURCE=.\SelectMBox.h
# End Source File
# Begin Source File

SOURCE=.\sendmail.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\Sigmgr.h
# End Source File
# Begin Source File

SOURCE=.\signatur.h
# End Source File
# Begin Source File

SOURCE=.\SignatureTree.h
# End Source File
# Begin Source File

SOURCE=.\SignatureWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\SingleReader.h
# End Source File
# Begin Source File

SOURCE=.\SITE.H
# End Source File
# Begin Source File

SOURCE=.\SMTPSession.h
# End Source File
# Begin Source File

SOURCE=.\spell.h
# End Source File
# Begin Source File

SOURCE=.\SpellDialog.h
# End Source File
# Begin Source File

SOURCE=.\spellpaige.h
# End Source File
# Begin Source File

SOURCE=.\SplitHlp.h
# End Source File
# Begin Source File

SOURCE=.\SpoolMgr.h
# End Source File
# Begin Source File

SOURCE=.\ssce.h
# End Source File
# Begin Source File

SOURCE=.\statbar.h
# End Source File
# Begin Source File

SOURCE=.\station.h
# End Source File
# Begin Source File

SOURCE=.\StationeryTree.h
# End Source File
# Begin Source File

SOURCE=.\StationeryWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\StripRich.h
# End Source File
# Begin Source File

SOURCE=.\summary.h
# End Source File
# Begin Source File

SOURCE=.\TaskErrorDlg.h
# End Source File
# Begin Source File

SOURCE=.\TaskErrorMT.h
# End Source File
# Begin Source File

SOURCE=.\TaskErrorTypes.h
# End Source File
# Begin Source File

SOURCE=.\TaskErrorView.h
# End Source File
# Begin Source File

SOURCE=.\TaskErrorWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\TaskInfo.h
# End Source File
# Begin Source File

SOURCE=.\TaskObject.h
# End Source File
# Begin Source File

SOURCE=.\TaskStatus.h
# End Source File
# Begin Source File

SOURCE=.\TaskStatus.h
# End Source File
# Begin Source File

SOURCE=.\TaskStatusView.h
# End Source File
# Begin Source File

SOURCE=.\TaskStatusWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\TaskSummary.h
# End Source File
# Begin Source File

SOURCE=.\TBarBmpCombo.h
# End Source File
# Begin Source File

SOURCE=.\TBarEdit.h
# End Source File
# Begin Source File

SOURCE=.\TBarStatic.h
# End Source File
# Begin Source File

SOURCE=.\textfile.h
# End Source File
# Begin Source File

SOURCE=.\TextReader.h
# End Source File
# Begin Source File

SOURCE=.\TextToSpeech.h
# End Source File
# Begin Source File

SOURCE=.\timestmp.h
# End Source File
# Begin Source File

SOURCE=.\tocdoc.h
# End Source File
# Begin Source File

SOURCE=.\TocFrame.h
# End Source File
# Begin Source File

SOURCE=.\tocview.h
# End Source File
# Begin Source File

SOURCE=.\TransReader.h
# End Source File
# Begin Source File

SOURCE=.\TreeCtrlCheck.h
# End Source File
# Begin Source File

SOURCE=.\TridentReadMessageView.h
# End Source File
# Begin Source File

SOURCE=.\trnslate.h
# End Source File
# Begin Source File

SOURCE=.\urledit.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\uudecode.h
# End Source File
# Begin Source File

SOURCE=.\ValidEdit.h
# End Source File
# Begin Source File

SOURCE=.\WarnEditHTMLDialog.h
# End Source File
# Begin Source File

SOURCE=.\WazooBar.h
# End Source File
# Begin Source File

SOURCE=.\WazooBarMgr.h
# End Source File
# Begin Source File

SOURCE=.\WazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\WebBrowserView.h
# End Source File
# Begin Source File

SOURCE=.\WebBrowserWazooWnd.h
# End Source File
# Begin Source File

SOURCE=.\winsock.h
# End Source File
# Begin Source File

SOURCE=.\workbook.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\EUDORA.ICO
# End Source File
# Begin Source File

SOURCE=.\EudoraExe.rc

!IF  "$(CFG)" == "EudoraExe - Win32 Debug"

!ELSEIF  "$(CFG)" == "EudoraExe - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RES\NOMAIL.ICO
# End Source File
# End Group
# Begin Source File

SOURCE=.\QComApplication.rgs
# End Source File
# Begin Source File

SOURCE=.\QComAppTypeLib.rgs
# End Source File
# Begin Source File

SOURCE=.\QComFolder.rgs
# End Source File
# Begin Source File

SOURCE=.\QComFolders.rgs
# End Source File
# Begin Source File

SOURCE=.\QComMessage.rgs
# End Source File
# Begin Source File

SOURCE=.\QComMessages.rgs
# End Source File
# End Target
# End Project
