' File name: EudoraUtil.inc
' Project:  WnObjPrg
' QUALCOMM Inc. Proprietary - Shred bin disposal only.

Global EudoraName         As String


'$INCLUDE 'GenUtil.INC'
'$INCLUDE 'C:\vt\EudoraResDependant.INC'
'$INCLUDE 'C:\vt\EudoraIniFunctions.INC'

'$RESOURCE 'c:\vt\ResStrings.vtr'

Declare Function InitStartEudora() As Integer
Declare Function StartEudora() As Integer
Declare Function EudoraIsOpen() As Integer
Declare Function StartMessage() As Integer
Declare Function SendaMessage() As Integer
Declare Function CheckDefaultClient() As Integer
Declare Function DismissIntro() As Integer
Declare Function CreateDominantPersonality() As Integer
Declare Function MaximizeEudora() As Integer
Declare Function RemoveAndDisableTips() As Integer
Declare Function ChangeDefaultOptions() As Integer
Declare Function LaunchToolsOptions() As Integer
Declare Function SetDefaultOptions() As Integer
Declare Function CheckSendingStyles() As Integer
Declare Function GetAddressBook() As Integer
Declare Function MakeNicknames() As Integer
Declare Function MakeNicknames2() As Integer ' for 5.0 addr book
Declare Function EnterAddressRichEdit(ClassType$,EditIdValue%,FieldName$,TextToEnter$) As Integer
Declare Function EnterAddressEdit(ClassType$,IDValue%,FieldName$,TextToEnter$) As Integer
Declare Function CloseAddressBook(AndSave%) As Integer
Declare Function OpenSignatures() As Integer
Declare Function CreateSignatures() As Integer
Declare Function OpenStationery() As Integer
Declare Function CreateStationery() As Integer
Declare Function SaveAsStationery(SaveName$, AndSave%) As Integer
Declare Function UseStationeryandSignature() As Integer
Declare Function OpenMailboxes() As Integer
Declare Function CreateInitialMailboxes() As Integer
Declare Function DeleteMailbox(MbxName$, DeleteIfStuff% ) As Integer
Declare Function CreateMailbox(MbxName$) As Integer
Declare Function CreateMailboxFolder(FolderName$, MbxName$) As Integer
Declare Function OpenFilters() As Integer
Declare Function InitMakeFilters() As Integer
Declare Function SendFilterMessageOne() As Integer
Declare Function SendFilterMessageTwo() As Integer
Declare Function SendFilterMessageThree() As Integer
Declare Function SendFilterMails() As Integer
Declare Function OpenTopLevelMailboxByName(MbxName$) as Integer
Declare Function GetEudoraParentFolderName() as String
Declare Function SendETFMail(Recipient$, Subject$, PicturePath$) as Integer
Declare Function InsertPicture(PicturePath$) as Integer
Declare Function InsertLine() as Integer
Declare Function ParseFileName(FileAndPath$) as String
Declare Function SelectFont(FontName$) As Integer
Declare Function SetPriority(Priority$) As Integer
Declare Function SetSignature(SigName$, SigIndex%) As Integer
Declare Function SetEncoding(EncodeType$) As Integer
Declare Function HighlightLastWords(NumberOfWords%) as Integer


'*************************************************************************
' Function InitStartEudora
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' This function is for starting Eudora without an INI file.
'
' Returns True/False
'
'*************************************************************************
Function InitStartEudora() As Integer
 	Dim UtilReturnInteger As Integer

	InitStartEudora = True

	'Check Application path
    If Exists(EudoraName) Then
		' Use API to start the program
		UtilReturnInteger = StartTheProgram(EudoraName, NULL)
		If UtilReturnInteger = False Then
			InitStartEudora = False
			LogWrite("1     Can't Start "+EudoraName+"!",LogPath,CritErrPath)
			Exit Function
		End If
    Else
		InitStartEudora = False
        ' Write error message
        LogWrite("1     Can't Find "+EudoraName+"!",LogPath,CritErrPath)
		Exit Function
	End If

	' Close	StatusBox if open
	StatusBox Close
	Sleep 22
    ' Check for existence of Application and remember main window handle
    MainAppWindow = WFndWndC("Eudora","EudoraMainWindow",FW_PART Or FW_MAXIMIZE ,(FindWindowTimeout))

	If MainAppWindow = 0 Then
		InitStartEudora = False
       ' Write error message
		LogWrite("1     Can't find "+EudoraName+" Main Window!",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     "+EudoraName+" Started successfully.",LogPath,CritErrPath)
		' Use API to check validity of Window Handle - that it exists
		LogWrite("0     Checking for existence of Main Eudora Window.",LogPath,CritErrPath)
		If IsWindow(MainAppWindow) <> 0 Then
		Else
			LogWrite("1     "+EudoraName+" Window handle: "+Trim(Str$(MainAppWindow))+" is closed.",LogPath,CritErrPath)
			InitStartEudora = False
			Exit Function
		End If
		' Test for system menu
		If Not WSysMenuExists(MainAppWindow) Then
			InitStartEudora = False
			LogWrite("1     Can't find "+EudoraName+" System Menu!",LogPath,CritErrPath)
			Exit Function
		Else
			LogWrite("0     Found "+EudoraName+" System Menu.",LogPath,CritErrPath)
		End If
    End If
	
	UtilReturnInteger = CheckDefaultClient()
	' Test Return Value
	If UtilReturnInteger = False Then
		LogWrite("0     Eudora was Default",LogPath,CritErrPath)
	Else 
		LogWrite("0     Made Eudora Default",LogPath,CritErrPath)
	End If 

End Function
'*************************************************************************
' Function StartEudora
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Start Eudora is used when you are not expecting another window to pop up 
' with Eudora. such as nags, default email client, or account wizard
' We do handle the tip of the day(be careful it does disable this)
'
' Returns True/False
'
'*************************************************************************
Function StartEudora() As Integer
 	Dim UtilReturnInteger As Integer

	StartEudora = True

	'Check Application path
    If Exists(EudoraName) Then
		' Use API to start the program
		UtilReturnInteger = StartTheProgram(EudoraName, NULL)
		If UtilReturnInteger = False Then
			StartEudora = False
			LogWrite("1     Can't Start "+EudoraName+"!",LogPath,CritErrPath)
			Exit Function
		End If
    Else
		StartEudora = False
        ' Write error message
        LogWrite("1     Can't Find "+EudoraName+"!",LogPath,CritErrPath)
		Exit Function
	End If

	' Close	StatusBox if open
	StatusBox Close
	Sleep 22
	DismissIntro()
    ' Check for existence of Application and remember main window handle
    MainAppWindow = WFndWndC("Eudora","EudoraMainWindow",FW_PART Or FW_MAXIMIZE ,(FindWindowTimeout))

	If MainAppWindow = 0 Then
		StartEudora = False
       ' Write error message
		LogWrite("1     Can't find "+EudoraName+" Main Window!",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     "+EudoraName+" Started successfully.",LogPath,CritErrPath)
		' Use API to check validity of Window Handle - that it exists
		LogWrite("0     Checking for existence of Main Eudora Window.",LogPath,CritErrPath)
				
		UtilReturnInteger = CheckDefaultClient()
		' Test Return Value
		If UtilReturnInteger = False Then
			LogWrite("0     Eudora was Default",LogPath,CritErrPath)
		Else 
			LogWrite("0     Made Eudora Default",LogPath,CritErrPath)
		End If 

		DialogWindow = WFndWnd(RES$(IDS_INTRODUCTION), FW_DIALOG)
		if DialogWindow = 0 Then
			LogWrite("0     No Introduction Screen Found",LogPath,CritErrPath)
			Exit Function
		End If
		ClickButton("Button", 1, RES$(IDS_OK))
		LogWrite("0     Clicked Ok to dismiss introductiont",LogPath,CritErrPath)

		DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_ACCOUNT_WIZARD), FW_DIALOG Or FW_FOCUS)
			If DialogWindow <> 0 Then
				UtilReturnInteger = CreateDominantPersonality()
				If UtilReturnInteger = True Then
					LogWrite("0     Successfully Created Dominant Personality",LogPath,CritErrPath)
				End If
			EndIf

		DialogWindow = WFndWnd(RES$(IDS_TIP_OF_THE_DAY), FW_DIALOG Or FW_FOCUS)
			If DialogWindow <> 0 Then
				UtilReturnInteger = RemoveAndDisableTips()
				If UtilReturnInteger = True Then
					LogWrite("0     Successfully Removed Tips of the Day",LogPath,CritErrPath)
				End If
			EndIf

		DialogWindow = WFndWnd(RES$(IDS_REGISTRATION), FW_DIALOG Or FW_FOCUS)
			If DialogWindow <> 0 Then
				LogWrite("0     Registration Nag Popup",LogPath,CritErrPath)
				ClickButton("Button", 2, RES$(IDS_BUTTON_MAYBE_LATER))
				If UtilReturnInteger = True Then
					LogWrite("0     Registration Nag Released with Maybe Later",LogPath,CritErrPath)
				End If
			EndIf
		
		If IsWindow(MainAppWindow) <> 0 Then
			' Make sure window is enabled
			If IsWindowEnabled(MainAppWindow) <> 0 Then
				LogWrite("0     "+EudoraName+" Main Window exists.",LogPath,CritErrPath)
				' Set Main Eudora window as active
				WSetActWnd(MainAppWindow)
				' Make sure window is visible and restore size
				If WIsVisible(MainAppWindow) = 0 Then
				   WMaxWnd(MainAppWindow)
				 End If
			Else
				LogWrite("1     "+EudoraName+" Window handle: "+Trim(Str$(MainAppWindow))+" exists but is not enabled.",LogPath,CritErrPath)
				StartEudora = False
				Exit Function
			End If
		Else
			LogWrite("1     "+EudoraName+" Window handle: "+Trim(Str$(MainAppWindow))+" is closed.",LogPath,CritErrPath)
			StartEudora = False
			Exit Function
		End If
		' Test for system menu
		If Not WSysMenuExists(MainAppWindow) Then
			StartEudora = False
			LogWrite("1     Can't find "+EudoraName+" System Menu!",LogPath,CritErrPath)
			Exit Function
		Else
			LogWrite("0     Found "+EudoraName+" System Menu.",LogPath,CritErrPath)
		End If
    End If
    MainAppWindow = WFndWndC("Eudora","EudoraMainWindow",FW_PART Or FW_MAXIMIZE ,(FindWindowTimeout))

End Function
'*************************************************************************
' Function CheckDefaultClient
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Checks for a popup window Asking for default client status. We are 
' going to make it default if it isn't. This is called from *StartEudora
'
' Returns True/False
'
'*************************************************************************
Function CheckDefaultClient() As Integer
	
	CheckDefaultClient = True

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_NOTE), FW_DIALOG)
	if DialogWindow = 0 Then
		CheckDefaultClient = False
		LogWrite("0     Eudora Already Is Default or Window Not Found",LogPath,CritErrPath)
		Exit Function
	End If


	ClickButton("Button", 1, RES$(IDS_BUTTON_YES))
	LogWrite("0     Clicked Yes for Default Client",LogPath,CritErrPath)

End Function
'*************************************************************************
' Function DismissIntro
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' This Dismisses the introduction screen
' This is called from InitStartEudora
'
' Returns True/False
'
'*************************************************************************
Function DismissIntro() As Integer

	DismissIntro = True
	Sleep 1

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_INTRODUCTION), FW_DIALOG)
	If DialogWindow = 0 Then
		DismissIntro = False
		LogWrite("0     There was no Introduction Window",LogPath,CritErrPath)
		Exit Function
	End If

	LogWrite("0     Dismissed Introduction with OK", LogPath, CritErrPath)

	ClickButton("Button", 1, RES$(IDS_BUTTON_OK))

End Function
'*************************************************************************
' Function	CreateDominantPersonality
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Here we will walk through the introduction and create a personality
' This personality uses Pop3 server. Uses \\Swamp and the jmiller account
' L/P: jmiller/jmiller
'
' Returns True/False
'
'*************************************************************************
Function CreateDominantPersonality() As Integer
	CreateDominantPersonality = True

	Dim H as long 
	LogWrite("0     Creating Dominant Personality",LogPath,CritErrPath)
	DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_ACCOUNT_WIZARD), FW_DIALOG)
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in Finding New Account Wizard",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 1",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 2",LogPath,CritErrPath)
		Exit Function
	End If


	DialogWindow = EnterEditBox("Edit",1,"",GetPersonalityName("Eudora", TheIniFile))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 3:Entering Name",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 3",LogPath,CritErrPath)
		Exit Function
	End If
	
	DialogWindow = EnterEditBox("Edit",1,"",GetPersonalityEmail("Eudora", TheIniFile))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 4:Entering email",LogPath,CritErrPath)
		Exit Function
	End If
	
	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 4",LogPath,CritErrPath)
		Exit Function
	End If
	
	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 5: next on Login Name",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = EnterEditBox("Edit",1,"",GetPersonalityServer("Eudora", TheIniFile))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 6:Entering Incoming Server",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 6",LogPath,CritErrPath)
		Exit Function
	End If
	
	DialogWindow = ClickButton("Button", 2, RES$(IDS_NEXT))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 7: next on Outgoing",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 2, RES$(IDS_FINISH))
	If DialogWindow = 0 Then
		CreateDominantPersonality = False
		LogWrite("1     Critical Error in New Account Wizard Step 8: Finish!",LogPath,CritErrPath)
		Exit Function
	End If

End Function

'*************************************************************************
' Function EudoraIsOpen
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Checks if Eudora is already running
' Handy to check for it, so you don't need to start it
'
' Returns True/False
'
'*************************************************************************
Function EudoraIsOpen() As Integer


	EudoraIsOpen = True

    MainAppWindow = WFndWndC("Eudora","EudoraMainWindow",FW_PART Or FW_MAXIMIZE ,(FindWindowTimeout))

	If MainAppWindow = 0 Then
		EudoraIsOpen = False
       ' Write error message
		LogWrite("1     "+EudoraName+" is not open... Trying to open Eudora!",LogPath,CritErrPath)
	End If

End Function

'*************************************************************************
' Function StartMessage
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Creates a new blank Message
'
' Returns True/False
'
'*************************************************************************

Function StartMessage() As Integer

	Dim UtilReturnInteger As Integer
	Dim menustr as String

	menustr = RES$(IDS_MENU_MESSAGE) + "\" + RES$(IDS_MENU_MESSAGE_NEW)
	StartMessage = True

	' Select View\Toolbar
	UtilReturnInteger = SelectMenuItem(MainAppWindow,menustr ,menustr )
	' Check Return Value
	If UtilReturnInteger = False Then
		StartMessage = False
		LogWrite("0     "+EudoraName+" Could not Start New Message.",LogPath,CritErrPath)
	End If
		

End Function
'*************************************************************************
' Function MaximizeEudora
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Finds Eudora and Maximizes it
'
' Returns True/False
'
'*************************************************************************

Function MaximizeEudora() As Integer
	MaximizeEudora = True

	If WFndWndC(NULL, "EudoraMainWindow", FW_MAXIMIZE, (FindWindowTimeout)) Then
	MaximizeEudora = True
	LogWrite("0     Maximized Eudora",LogPath,CritErrPath)

	Else
	MaximizeEudora = False
	EndIf

End Function
'*************************************************************************
' Function RemoveAndDisableTips
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Called from StartEudora. This will remove the Tip of the Day Dialog
' and disable it from appearing again
'
' Returns True/False
'
'*************************************************************************
Function RemoveAndDisableTips() As Integer

	RemoveAndDisableTips = True

	Play "%(s)"
	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_CLOSE))
	If DialogWindow = 0 Then
		RemoveAndDisableTips = False
		LogWrite("1     Critical Error Removing Tip of the Day",LogPath,CritErrPath)
		Exit Function
	End If
End Function



'*************************************************************************
' Function SendaMessage
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Sends a message with bold/underline/italicized text
'
' Returns True
'
'*************************************************************************

Function SendaMessage() As Integer
	Dim UtilReturnInteger As Integer

	SendaMessage = True
	LogWrite("0     Composing a Message With formatting",LogPath,CritErrPath)

	Sleep 3

	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	Play "This is an Automated Message"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	Play "Here is some body text"
	Play "~"
	Play "^(b)"
	Play "Here is some bold text"
	Play "^(b)"
	Play "~"
	Play "^(i)"
	Play "Here is some ital text"
	Play "^(i)"
	Play "~"
	Play "^(u)"
	Play "Here is some underlined text"
	Play "^(u)"

	Sleep 2
	LogWrite("0     Sending a Message With formatting",LogPath,CritErrPath)

	Play "^(e)" 

End Function

'*************************************************************************
' Function LaunchToolsOptions
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Will Launch Eudora Options Dialog
'
' Returns True/False
'
'*************************************************************************
Function LaunchToolsOptions() As Integer
	Dim UtilReturnInteger As Integer
	Dim menustr as String

	menustr = RES$(IDS_MENU_TOOLS) + "\" + RES$(IDS_MENU_TOOLS_OPTIONS)

	LaunchToolsOptions = True
	
	' Select Tools\Options
	UtilReturnInteger = SelectMenuItem(MainAppWindow,menustr,menustr)
	' Check Return Value
	If UtilReturnInteger = False Then
		LaunchToolsOptions = False
		LogWrite("0     "+EudoraName+" Could not Start Options Dialog.",LogPath,CritErrPath)
	End If

End Function

'*************************************************************************
' Function ChangeDefaultOptions
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Will change Default Options for BBFT 
'
' Returns True/False
'
'*************************************************************************
Function ChangeDefaultOptions() As Integer
	ChangeDefaultOptions = True
	Dim UtilReturnInteger as Integer

	UtilReturnInteger = SetDefaultOptions()
		' Test Return Value
	If UtilReturnInteger = False Then
		ChangeDefaultOptions = False
		LogError("Error Setting Save Password",0,LogPath,CritErrPath)
	End If


End Function
'*************************************************************************
' Function SetDefaultOptions
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Will UnSet Allow Authentication in
' Tools -> Options -> Getting Started
' Will Set the Save Password option in
' Tools -> Options -> Checking Mail
' Will UnSet Show Large Buttons in
' Tools -> Options -> Display
' Will unset Use Alert Dialog box in
' Tools -> Options -> Getting Attenion
' Will Set Turbo Redirect in
' Tools -> Options -> Miscellaneous
' Will set Automatically Expand Nicknames in
' Tools -> Options -> Miscellaneous
' Note this function is doomed to fail if extra option entries are 
' in the listbox
'
' Returns True/False
'
'*************************************************************************

Function SetDefaultOptions() As Integer

	SetDefaultOptions = True
	Dim tempstr as String


	Dim Where as long
	Sleep 3
	Dim I as Integer
	DialogWindow = WFndWnd(RES$(IDS_CAPTION_OPTIONS) , FW_DIALOG)

	Play "%(c)"      

	PLAY "{HOME}"
	Sleep 1
	Play "%(w)" 
	LogWrite("0     Toggled CheckBox Allow Authentication.",LogPath,CritErrPath)
	Play "%(c)" 
	PLAY "{DOWN}"

	Sleep 3
	

	SelectCheckBox("Button", 4,RES$(IDS_CHECKBOX_SAVE_PASS) ,"Check")

	Play "%(c)"      ' Alt+c

	FOR I = 1 TO 8
	PLAY "{DOWN}"
	NEXT I

	Sleep 3
	SelectCheckBox("Button", 4,RES$(IDS_CHECKBOX_SHOW_LARGE_BUTTONS),"UnCheck")
	Play "%(c)" 

	FOR I = 1 TO 8
	PLAY "{DOWN}"
	NEXT I

	Sleep 3
	SelectCheckBox("Button", 1,RES$(IDS_CHECKBOX_USE_ALERT),"UnCheck")

	Play "%(c)"      ' Alt+c

	Play "{END}"
	Play "%(c)"      ' Alt+c
	Sleep 3
	SelectCheckBox("Button", 6,RES$(IDS_CHECKBOX_TURBO_REDIRECT),"Check")
	SelectCheckBox("Button", 8,RES$(IDS_CHECKBOX_EXPAND_NICK),"Check")
	
	Sleep 3

	ClickButton("Button", 1, RES$(IDS_BUTTON_OK))

End Function

'*************************************************************************
' Function CheckSendingStyles
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Check For Sending Styles Dialog and set it default to send
' Plain and styled
'
' Returns True/False
'
'*************************************************************************
Function CheckSendingStyles() As Integer
	CheckSendingStyles = True

	LogWrite("0     Checking For Styled Text Warning",LogPath,CritErrPath)
	Dim UtilReturnInteger as Integer

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_SENDING_STYLES), FW_DIALOG)
	If DialogWindow <> 0 Then
		LogWrite("0     Found Sending Styled Text Warning",LogPath,CritErrPath)
	
		UtilReturnInteger = SetPlainAndStyled()
		If UtilReturnInteger = True Then
			LogWrite("0     Successfully Set Plain and Styled",LogPath,CritErrPath)
		Else
			CheckSendingStyles = False
			LogWrite("0     Could not set Sending Styles",LogPath,CritErrPath)		
		End If
	Else
		CheckSendingStyles = True
		LogWrite("0     Sending Styles Already Set",LogPath,CritErrPath)


	EndIf
End Function


'*************************************************************************
' Function SetPlainAndStyled
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Set Default to send Plain and Styled messages
'
' Returns True/False
'
'*************************************************************************
Function SetPlainAndStyled() As Integer
	SetPlainAndStyled = True

	DialogWindow = WFndWndC ( RES$(IDS_CHECKBOX_DONT_ASK_MAKE_DEFAULT),"Button", FW_FOCUS , (FindWindowTimeout))
	If DialogWindow = 0 Then
		SetPlainAndStyled = False
		LogWrite("1     Critical Error Setting Plain and Styled Option",LogPath,CritErrPath)
		Exit Function
	End If


	PLAY " "

	ClickButton( "Button", 3, RES$(IDS_BUTTON_PLAIN_N_STYLED))
	LogWrite("0     Set Send Plain and Styled as default",LogPath,CritErrPath)

End Function

'*************************************************************************
' Function GetAddressBook
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Launch Address book
'
' Returns True/False
'
'*************************************************************************

Function GetAddressBook() As Integer

	Dim UtilReturnInteger As Integer
	Dim tmpstr as String

	tmpstr = RES$(IDS_MENU_TOOLS) + "\" + RES$(IDS_MENU_TOOLS_ADDRESS_BOOK)
	GetAddressBook = True
	
	' Select Tools\Options
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr,tmpstr)
	' Check Return Value
	If UtilReturnInteger = False Then
		GetAddressBook = False
		LogWrite("1    "+EudoraName+" Could not Start AddressBook.",LogPath,CritErrPath)
	Else
		LogWrite("0    "+EudoraName+" Successfully Launched AddressBook.",LogPath,CritErrPath)
	End If
	sleep 5
	WFndWnd(RES$(IDS_CHECKBOX_ADR_BOOK), FW_FOCUS or FW_RESTORE, (FindWindowTimeout))
End Function
'*************************************************************************
' Function MakeNicknames
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Make Nicknames to: jason, jmiller@swamp.qualcomm.com, 
' david, dhom@swamp.qualcomm.com
' and a combo nickname Called Both using jason and david
'
' Returns True/False
'
'*************************************************************************

Function MakeNicknames() As Integer
	MakeNicknames = True
	Dim UtilReturnInteger As Integer
	Dim tmpNameString as String
	Dim tmpAdrString as String
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Next Nickname			Next Nickname			Next Nickname			Next Nickname				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	tmpNameString = GetNickName("Eudora", TheIniFile, 1)
	tmpAdrString = GetNickAdr("Eudora", TheIniFile, 1)

	DialogWindow = ClickButton("Button", 9, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Clicking New Button to make nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating First Nickname: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEWNICK), FW_DIALOG)
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Finding New Nickname Dialog",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Preparing to write: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	DialogWindow = EnterEditBox("Edit",1,"",tmpNameString)
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Critical Error Writing NickName",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)
	End If

	' For problems Clicking by ordinal
	PLAY "{TAB}"
	PLAY "{TAB}"
	PLAY " "
		LogWrite("0     Set Nickname "+tmpNameString+" as recipient",LogPath,CritErrPath)
	
	DialogWindow = ClickButton("Button", 1, "OK")
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Clicking OK to set nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     First Nickname "+tmpNameString+" created preparing to add address",LogPath,CritErrPath)
	End If

	UtilReturnInteger = EnterAddressRichEdit("RICHEDIT",432,"Address RichEdit",tmpAdrString)
	If UtilReturnInteger = False Then
		MakeNicknames = False
		LogWrite("1     Can't Write "+tmpAdrString+"!",LogPath,CritErrPath)
		Exit Function
	End If
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Next Nickname			Next Nickname			Next Nickname			Next Nickname				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	tmpNameString = GetNickName("Eudora", TheIniFile, 2)
	tmpAdrString = GetNickAdr("Eudora", TheIniFile, 2)

	DialogWindow = ClickButton("Button", 9, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Clicking New Button to make nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Second Nickname: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEWNICK), FW_DIALOG)
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Finding New Nickname Dialog",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Preparing to write: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	DialogWindow = EnterEditBox("Edit",1,"",tmpNameString)
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Critical Error Writing NickName",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)
	End If


	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Clicking OK to set nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Second Nickname "+tmpNameString+" created preparing to add address",LogPath,CritErrPath)
	End If

	UtilReturnInteger = EnterAddressRichEdit("RICHEDIT",432,"Address RichEdit",tmpAdrString)
	If UtilReturnInteger = False Then
		MakeNicknames = False
		LogWrite("1     Can't Write "+tmpAdrString+"!",LogPath,CritErrPath)
		Exit Function
	End If
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Next Nickname			Next Nickname			Next Nickname			Next Nickname				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	tmpNameString = GetNickName("Eudora", TheIniFile, 3)
	tmpAdrString = GetNickAdr("Eudora", TheIniFile, 3)

	DialogWindow = ClickButton("Button", 9, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Clicking New Button to make nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Third Nickname: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEWNICK), FW_DIALOG)
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Finding New Nickname Dialog",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Preparing to write: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	DialogWindow = EnterEditBox("Edit",1,"",tmpNameString)
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Critical Error Writing NickName",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)
	End If

	' For problems Clicking by ordinal
	PLAY "{TAB}"
	PLAY "{TAB}"
	PLAY " "
		LogWrite("0     Set Nickname "+tmpNameString+" as recipient",LogPath,CritErrPath)
	
	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
	If DialogWindow = 0 Then
		MakeNicknames = False
		LogWrite("1     Error Clicking OK to set nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Third Nickname "+tmpNameString+" created preparing to add address",LogPath,CritErrPath)
	End If

	UtilReturnInteger = EnterAddressRichEdit("RICHEDIT",432,"Address RichEdit",tmpAdrString)
	If UtilReturnInteger = False Then
		MakeNicknames = False
		LogWrite("1     Can't Write "+tmpAdrString+"!",LogPath,CritErrPath)
		Exit Function
	End If
	


End Function
'*************************************************************************
' Function MakeNicknames
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Make Nicknames to: jason, jmiller@swamp.qualcomm.com, 
' david, dhom@swamp.qualcomm.com
' and a combo nickname Called Both using jason and david
'
' Returns True/False
'
'*************************************************************************

Function MakeNicknames2() As Integer
	MakeNicknames2 = True
	Dim UtilReturnInteger As Integer
	Dim tmpNameString as String
	Dim tmpAdrString as String
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Next Nickname			Next Nickname			Next Nickname			Next Nickname				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	tmpNameString = GetNickName("Eudora", TheIniFile, 1)
	tmpAdrString = GetNickAdr("Eudora", TheIniFile, 1)

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		MakeNicknames2 = False
		LogWrite("1     Error Clicking New Button to make nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating First Nickname: "+tmpNameString+"",LogPath,CritErrPath)
	End If

	PLAY "%(K)"

	PLAY ""+tmpNameString+""
	LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)
	
	' For problems recipient by ordinal
	PLAY "+({TAB})"
	PLAY " "
		LogWrite("0     Set Nickname "+tmpNameString+" as recipient",LogPath,CritErrPath)
	
	UtilReturnInteger = EnterAddressRichEdit("RICHEDIT",409,"Address RichEdit",tmpAdrString)
	If UtilReturnInteger = False Then
		MakeNicknames2 = False
		LogWrite("1     Can't Write "+tmpAdrString+"!",LogPath,CritErrPath)
		Exit Function
	End If
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Next Nickname			Next Nickname			Next Nickname			Next Nickname				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	tmpNameString = GetNickName("Eudora", TheIniFile, 2)
	tmpAdrString = GetNickAdr("Eudora", TheIniFile, 2)

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		MakeNicknames2 = False
		LogWrite("1     Error Clicking New Button to make nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Second Nickname: "+tmpNameString+"",LogPath,CritErrPath)
	End If


	PLAY "%(K)"

	PLAY ""+tmpNameString+""
	LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)
	
		LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)

	' For problems recipient by ordinal
	PLAY "+({TAB})"
	PLAY " "
		LogWrite("0     Set Nickname "+tmpNameString+" as recipient",LogPath,CritErrPath)


	UtilReturnInteger = EnterAddressRichEdit("RICHEDIT",409,"Address RichEdit",tmpAdrString)
	If UtilReturnInteger = False Then
		MakeNicknames2 = False
		LogWrite("1     Can't Write "+tmpAdrString+"!",LogPath,CritErrPath)
		Exit Function
	End If
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Next Nickname			Next Nickname			Next Nickname			Next Nickname				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	tmpNameString = GetNickName("Eudora", TheIniFile, 3)
	tmpAdrString = GetNickAdr("Eudora", TheIniFile, 3)

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		MakeNicknames2 = False
		LogWrite("1     Error Clicking New Button to make nickname",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Third Nickname: "+tmpNameString+"",LogPath,CritErrPath)
	End If


	PLAY "%(K)"

	PLAY ""+tmpNameString+""
	LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)
	
	LogWrite("0     Wrote Nickname "+tmpNameString+"",LogPath,CritErrPath)

	' For problems recipient by ordinal
	PLAY "+({TAB})"
	PLAY " "
		LogWrite("0     Set Nickname "+tmpNameString+" as recipient",LogPath,CritErrPath)
	

	UtilReturnInteger = EnterAddressRichEdit("RICHEDIT",409,"Address RichEdit",tmpAdrString)
	If UtilReturnInteger = False Then
		MakeNicknames2 = False
		LogWrite("1     Can't Write "+tmpAdrString+"!",LogPath,CritErrPath)
		Exit Function
	End If
	


End Function
'*************************************************************************
' Function EnterAddressRichEdit
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' ClassType$    From WInfo i.e.	  Edit
' IDValue%		From WInfo i.e.	  1
' FieldName$	Screen Field Name
' TextToEnter$  Text to enter in Edit Box
'
' Returns True/False Integer
'
' Syntax Example:
' Enter Address1
' ReturnInteger=EnterAddressRichEdit("Edit",1,"Address1","123 Main Street")
'
'*************************************************************************
Function EnterAddressRichEdit(ClassType$,EditIdValue%,FieldName$,TextToEnter$) As Integer

	Dim ObjectHandle As Long

	EnterAddressRichEdit = True

	' Set the Edit Class
	Sleep .5
	WEditSetClass(ClassType)

	' Enter New Type
	ObjectHandle = WEditFind(_id((EditIdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WEditExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WEditEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				WEditSetText(_hWnd(ObjectHandle),TextToEnter,(FindWindowTimeout))
				LogWrite("0     Entered New "+FieldName+": "+ TextToEnter+".",LogPath,CritErrPath)
			Else
				EnterAddressRichEdit = False
				' Write error message
				LogWrite("1     "+FieldName+" Edit isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			EnterAddressRichEdit = False
			' Write error message
			LogWrite("1     "+FieldName+" Edit doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		EnterAddressRichEdit = False
		' Write error message
		LogWrite("1     Can't Find "+FieldName+" Edit!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function EnterAddressEdit
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' ClassType$    From WInfo i.e.	  Edit
' IDValue%		From WInfo i.e.	  1
' FieldName$	Screen Field Name
' TextToEnter$  Text to enter in Edit Box
'
' Returns True/False Integer
'
' Syntax Example:
' Enter Address1
' ReturnInteger=EnterAddressEdit("Edit",1,"Address1","123 Main Street")
'
'*************************************************************************
Function EnterAddressEdit(ClassType$,IDValue%,FieldName$,TextToEnter$) As Integer

	Dim ObjectHandle As Long

	EnterAddressEdit = True

	' Set the Edit Class
	Sleep .5
	WEditSetClass(ClassType)

	' Enter New Type
	ObjectHandle = WEditFind(_id((IDValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WEditExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WEditEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				WEditSetText(_hWnd(ObjectHandle),TextToEnter,(FindWindowTimeout))
				LogWrite("0     Entered New "+FieldName+": "+ TextToEnter+".",LogPath,CritErrPath)
			Else
				EnterAddressEdit = False
				' Write error message
				LogWrite("1     "+FieldName+" Edit isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			EnterAddressEdit = False
			' Write error message
			LogWrite("1     "+FieldName+" Edit doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		EnterAddressEdit = False
		' Write error message
		LogWrite("1     Can't Find "+FieldName+" Edit!",LogPath,CritErrPath)
		Exit Function
	End If

End Function

'*************************************************************************
' Function CloseAddressBook
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' AndSave%    Save this address Book?True = yes, False = no
'
' Returns True/False Integer
'
' Syntax Example:
' Enter Address1
' ReturnInteger=EnterAddressRichEdit("Edit",1,"Address1","123 Main Street")
'
'*************************************************************************

Function CloseAddressBook(AndSave%) As Integer

	CloseAddressBook = True

	WFndWnd(RES$(IDS_CHECKBOX_ADR_BOOK), FW_FOCUS, (FindWindowTimeout))

	Play "^({F4})" 

	LogWrite("0     Closing Address Book",LogPath,CritErrPath)

	DialogWindow = WFndWnd("Eudora", FW_DIALOG)
	if DialogWindow = 0 Then
		CloseAddressBook = True
		LogWrite("0     Not Saving Address Book, No Changes",LogPath,CritErrPath)
		Exit Function
	End If

	if AndSave = True Then
		LogWrite("0     Saving Address Book",LogPath,CritErrPath)
		ClickButton("Button", 1, RES$(IDS_BUTTON_YES))
	else
		LogWrite("0     Not Saving Address Book, discarding changes",LogPath,CritErrPath)
		ClickButton("Button", 2, RES$(IDS_BUTTON_NO))
	End If


End Function

'*************************************************************************
' Function OpenSignatures
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Launch Signatures
'
' Returns True/False
'
'*************************************************************************


Function OpenSignatures() As Integer
	Dim UtilReturnInteger As Integer
	Dim tmpstr as String

	tmpstr = RES$(IDS_MENU_TOOLS) + "\" + RES$(IDS_MENU_TOOLS_SIGNATURES)
	OpenSignatures = True
	
	' Select Tools\Options
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		OpenSignatures = False
		LogWrite("1     Could not Open Signatures.",LogPath,CritErrPath)
	Else
		LogWrite("0     Successfully Launched Signatures.",LogPath,CritErrPath)
	End If
	Sleep 3

End Function
'*************************************************************************
' Function CreateSignatures
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Create Signatures when window is open somewhere
'
' Returns True/False
'
'*************************************************************************

Function CreateSignatures() As Integer

	Dim SignatureHandle as long

	CreateSignatures = True

	SignatureHandle = WTreeFind( _ord(1),  (FindWindowTimeout) )

	if SignatureHandle = NULL Then
		CreateSignatures = False
		LogWrite("1    Could not Find Signature Tree.",LogPath,CritErrPath)		
		Exit Function
	End If

	WTreeItemClk ( _hwnd(SignatureHandle), RES$(IDS_STR_STANDARD), VK_RBUTTON, (FindWindowTimeout))
	WMenuSelect("&New...")

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_CREATE_NEW_SIG), FW_DIALOG)
	If DialogWindow <> 0 Then
		LogWrite("0     Entering New Signature Name",LogPath,CritErrPath)
		DialogWindow = EnterEditBox("Edit",1,"","TestSig")
		
		If DialogWindow = 0 Then
			CreateSignatures = False
			LogWrite("1     Could not enter New Signature name",LogPath,CritErrPath)
			Exit Function
		End If
		
		ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
		LogWrite("0     Created a Signature",LogPath,CritErrPath)
	Else
		LogWrite("1    Could not Find New Signature Dialog",LogPath,CritErrPath)		
		Exit Function
	EndIf

	Play "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-"
	Play "~"
	Play "This is Plain Text in a Signature"
	Play "~"
	Play "^(b)"
	Play "This is Bold Text in a Signature"
	Play "^(b)"
	Play "~"
	Play "^(i)"
	Play "This is Ital Text in a Signature"
	Play "^(i)"
	Play "~"
	Play "^(u)"
	Play "This is Underlined Text in a Signature"
	Play "^(u)"
	Play "~"
	Play "^(s)"
	Sleep 2
	Play "^(w)"

	LogWrite("0     Created a Signature Text",LogPath,CritErrPath)
End Function

'*************************************************************************
' Function OpenStationery
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Launch Stationery
'
' Returns True/False
'
'*************************************************************************

Function OpenStationery() As Integer
	Dim UtilReturnInteger As Integer
	Dim tmpstr as String

	OpenStationery = True
	
	tmpstr = RES$(IDS_MENU_TOOLS) + "\" + RES$(IDS_MENU_TOOLS_STATIONERY)
	' Select Tools\Options
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		OpenStationery = False
		LogWrite("1    "+EudoraName+" Could not Open Stationery Window.",LogPath,CritErrPath)
	Else
		LogWrite("0    "+EudoraName+" Successfully Launched Stationery Window.",LogPath,CritErrPath)
	End If
	Sleep 3

End Function

'*************************************************************************
' Function CreateStationery
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Create Stationery when window is open somewhere
'
' Returns True/False
'
'*************************************************************************

Function CreateStationery() As Integer
	Dim StationeryHandle as long
	Dim UtilReturnInteger As Integer

	CreateStationery = True

	StationeryHandle = WTreeFind( _ord(1),  (FindWindowTimeout) )

	if StationeryHandle = NULL Then
		CreateStationery = False
		LogWrite("1    Could not Find Stationery Tree.",LogPath,CritErrPath)		
		Exit Function
	End If
	WTreeClk ( _hwnd(StationeryHandle), W_CENTER , W_CENTER, VK_RBUTTON, (FindWindowTimeout) )
	WMenuSelect(RES$(IDS_MENU_SIG_NEW))

	LogWrite("0     Creating New Stationery",LogPath,CritErrPath)

	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	
	WComboItemClk ( _ord(1), "Verdana",(FindWindowTimeout) )
	Play "This is Plain Text in a Stationery"
	Play "~"
	Play "^(b)"
	Play "This is Bold Text in a Stationery"
	Play "^(b)"
	Play "~"
	Play "^(i)"
	Play "This is Ital Text in a Stationery"
	Play "^(i)"
	Play "~"
	Play "^(u)"
	Play "This is Underlined Text in a Stationery"
	Play "^(u)"
	Play "~"
	' TestSig in a system with a new install
	WComboItemClk ( _ord(3), "@3",(FindWindowTimeout) )
	Sleep 2
	UtilReturnInteger  = SaveAsStationery("TestStat", True)
	If UtilReturnInteger = False Then
		CreateStationery = False
		LogWrite("1    "+EudoraName+" Could not Save as Stationery.",LogPath,CritErrPath)
		Exit Function
	End If
		
	LogWrite("0     Completed Created a Stationery Text",LogPath,CritErrPath)

End Function

'*************************************************************************
' Function SaveAsStationery
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' SaveName$    Filename To Save Stationery as
' AndSave%     True/False Whether to save Stationery
'
' Returns True/False Integer
'
'*************************************************************************

Function SaveAsStationery(SaveName$, AndSave%) As Integer
	SaveAsStationery = True

	Dim UtilReturnInteger As Integer

	Play "^(w)"
	LogWrite("0     Saving As Stationery",LogPath,CritErrPath)

	DialogWindow = WFndWnd("Eudora", FW_DIALOG)
	if DialogWindow = 0 Then
		SaveAsStationery = True
		LogWrite("0     Not Saving Stationery, No Changes",LogPath,CritErrPath)
		Exit Function
	End If

	if AndSave = True Then
		ClickButton("Button", 1, RES$(IDS_BUTTON_YES))
		LogWrite("0     Saving Stationery File",LogPath,CritErrPath)
	else
		ClickButton("Button", 2, RES$(IDS_BUTTON_NO))
		LogWrite("0     Not Saving stationery File, discarding changes",LogPath,CritErrPath)
		Exit Function
	End If
	
	Sleep 4

	DialogWindow = WFndWnd(RES$(IDS_MENU_FILE_SAVE_STAT), FW_DIALOG)
	if DialogWindow = 0 Then
		Sleep 4
		DialogWindow = WFndWnd(RES$(IDS_MENU_FILE_SAVE_STAT), FW_DIALOG)
		if DialogWindow = 0 Then
			SaveAsStationery = False
			LogWrite("1     Can't Find Save As Stationery Dialog",LogPath,CritErrPath)
			Exit Function
		End If
	End If

	LogWrite("0     Found Save As Stationery Dialog",LogPath,CritErrPath)
			
	DialogWindow = EnterEditBox("Edit",1,"",SaveName)
	If DialogWindow = 0 Then
		SaveAsStationery = False
		LogWrite("1     Critical Error Typing Save as Stationery Name",LogPath,CritErrPath)
		Exit Function
	End If	

	
	LogWrite("0    "+EudoraName+" Successfully Wrote Stationery File name: "+SaveName+".",LogPath,CritErrPath)
	ClickButton("Button", 1, RES$(IDS_MENU_FILE_SAVE))	

	LogWrite("0    "+EudoraName+" Successfully Saved Stationery File.",LogPath,CritErrPath)

End Function

'*************************************************************************
' Function UseStationeryandSignature
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Returns True/False Integer
'
'*************************************************************************

Function UseStationeryandSignature() As Integer
	Dim UtilReturnInteger As Integer
	UseStationeryandSignature = True
	Dim tmpstr As String

	tmpstr = RES$(IDS_MENU_MESSAGE) + "\" + RES$(IDS_MENU_MESSAGE_NEW_WITH) + "\TestStat"
	' Select Tools\Options
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		UseStationeryandSignature = False
		LogWrite("1     "+EudoraName+" Could not Find Menu New Message With TestStat.",LogPath,CritErrPath)
		Exit Function
	End If
	
	Play "B"
	Sleep 1
	Play "oth"
	Play "{TAB}"
	Sleep 1
	Play "Automated Test to both accounts with TestStat and TestSig"
	Play "^(e)"


End Function


'*************************************************************************
' Function OpenMailboxes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Returns True/False Integer
'
'*************************************************************************
Function OpenMailboxes() As Integer
	Dim UtilReturnInteger As Integer
	Dim tmpstr As String


	tmpstr = RES$(IDS_MENU_TOOLS) + "\" + RES$(IDS_MENU_TOOLS_MAILBOXES)
	OpenMailboxes = True
	
	' Select Tools\Options
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		OpenMailboxes = False
		LogWrite("1    Could not Open Mailboxes.",LogPath,CritErrPath)
	Else
		LogWrite("0     Successfully Launched Mailboxes.",LogPath,CritErrPath)
	End If
	Sleep 3

End Function


'*************************************************************************
' Function CreateInitialMailboxes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Returns True/False Integer
'
'*************************************************************************
Function CreateInitialMailboxes() As Integer
	Dim UtilReturnInteger As Integer
		
	CreateInitialMailboxes = True


	UtilReturnInteger = CreateMailbox("Test1")
	' Test Return Value
	If UtilReturnInteger = False Then
		LogWrite("1     Error Creating Mailbox Test1.",LogPath,CritErrPath)
	End If
	UtilReturnInteger = CreateMailbox("Test2")
	' Test Return Value
	If UtilReturnInteger = False Then
		LogWrite("1    Error Creating Mailbox Test2.",LogPath,CritErrPath)
	End If
	UtilReturnInteger = DeleteMailbox("Test2", True )
	' Test Return Value
	If UtilReturnInteger = False Then
		LogWrite("1    Error Deleting Mailbox Test2.",LogPath,CritErrPath)
	End If
	UtilReturnInteger = CreateMailboxFolder("Test2","Test2")
	' Test Return Value
	If UtilReturnInteger = False Then
		LogWrite("1    Error Creating Mailbox Test2.",LogPath,CritErrPath)
	End If
	UtilReturnInteger = CreateMailbox("Test3")
	' Test Return Value
	If UtilReturnInteger = False Then
		LogWrite("1     Error Creating Mailbox Test3.",LogPath,CritErrPath)
	End If

	
	LogWrite("0     Creating New Stationery",LogPath,CritErrPath)

End Function


'*************************************************************************
' Function DeleteMailbox
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' MbxName$    Mailbox Name to be deleted
' DeleteIfStuff% Boolean Delete mailbox if there is stuff in it. True = Delete
'
' Returns True/False Integer
'
'*************************************************************************

Function DeleteMailbox(MbxName$, DeleteIfStuff%) As Integer
	DeleteMailbox = True
	Dim UtilReturnInteger As Integer
	Dim MailboxWazooHandle as long
	Dim EudoraParentName as String

	UtilReturnInteger = OpenMailboxes()
	If UtilReturnInteger = False Then
		DeleteMailbox = False
		LogWrite("1    Could not Find Mailboxes Tree to Create.",LogPath,CritErrPath)		
		Exit Function
	Else
		
		EudoraParentName = GetEudoraParentFolderName()
		if EudoraParentName = NULL Then
			DeleteMailbox = False
			LogWrite("1    Could not Get Eudora Parent Name.",LogPath,CritErrPath)		
			Exit Function
		End If

		MailboxWazooHandle = WTreeFind( NULL,  (FindWindowTimeout) )

		If MailboxWazooHandle = NULL Then
			DeleteMailbox = False
			LogWrite("1    Could not Find Mailboxes Tree.",LogPath,CritErrPath)		
			Exit Function
		End If
		WTreeItemClk ( _hwnd(MailboxWazooHandle), MbxName, VK_RBUTTON, (FindWindowTimeout))
		WMenuSelect(RES$(IDS_BUTTON_DELETE))

		DialogWindow = WFndWnd(RES$(IDS_CAPTION_DELETING_MAILBOX), FW_DIALOG or FW_FOCUS)
		If DialogWindow = 0 Then
			LogWrite("0     Deleted Mailbox was Empty",LogPath,CritErrPath)
		Else
			LogWrite("0     Deleted Mailbox Has Stuff in it",LogPath,CritErrPath)
		
		
			If DeleteIfStuff = True Then
				LogWrite("0     Deleting Stuff and Mailbox",LogPath,CritErrPath)
				ClickButton("Button", 1, RES$(IDS_BUTTON_REMOVE_IT))
			Else
				LogWrite("0     Not Deleting Stuff and Mailbox",LogPath,CritErrPath)
				ClickButton("Button", 2, RES$(IDS_BUTTON_CANCEL))
			End If
		End If

	EndIf

End Function

'*************************************************************************
' Function CreateMailbox
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' MbxName$    Mailbox Name to be deleted
'
' Returns True/False Integer
'
'*************************************************************************

Function CreateMailbox(MbxName$ ) As Integer
	CreateMailbox = True
	Dim UtilReturnInteger As Integer
	Dim MailboxWazooHandle as long
	Dim EudoraParentName as String

	' Open the mailboxes pane
	UtilReturnInteger = OpenMailboxes()
	if UtilReturnInteger = False Then
		CreateMailbox = False
		LogWrite("1    Could not Find Mailboxes Tree to Create.",LogPath,CritErrPath)		
		Exit Function
	
	Else
		' retrieve Parent Folder Name/Tree Header
		EudoraParentName = GetEudoraParentFolderName()
		if EudoraParentName = NULL Then
			CreateMailbox = False
			LogWrite("1    Could not Get Eudora Parent Name.",LogPath,CritErrPath)		
			Exit Function
		End If

		' Find mailbox tree, should have focus
		MailboxWazooHandle = WTreeFind( NULL,  (FindWindowTimeout) )
		if MailboxWazooHandle = NULL Then
			CreateMailbox = False
			LogWrite("1    Could not Find Mailboxes Tree.",LogPath,CritErrPath)		
			Exit Function
		End If
		
		' select New by rightclicking parent
		WTreeItemClk ( _hwnd(MailboxWazooHandle), EudoraParentName, VK_RBUTTON, (FindWindowTimeout))
		WMenuSelect(RES$(IDS_MENU_SIG_NEW))
		
		' Find New Mailbox Dialog
		DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_MAILBOX), FW_DIALOG or FW_FOCUS)
		if DialogWindow = 0 Then
			CreateMailbox = False
			LogWrite("1     No New Mailbox Dialog",LogPath,CritErrPath)
			Exit Function
		End If
		
		' Enter In the new mailbox name
		DialogWindow = EnterEditBox("Edit",1,"",MbxName)
		If DialogWindow = 0 Then
			CreateMailbox = False
			LogWrite("1     Could Not Enter Mailbox Name",LogPath,CritErrPath)
			Exit Function
		End If
		' Click OK
		LogWrite("0     Created Mailbox "+MbxName+"",LogPath,CritErrPath)
		ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
		
		' See if I get Already Exists error
		DialogWindow = WFndWnd("Eudora", FW_DIALOG or FW_FOCUS)
		if DialogWindow <> 0 Then
			LogWrite("1     A Folder/Mailbox exists with that name",LogPath,CritErrPath)
			ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
			
			Sleep 2

			' Find New Mailbox Dialog Again
			DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_MAILBOX), FW_DIALOG or FW_FOCUS)
			if DialogWindow = 0 Then
				CreateMailbox = False
				LogWrite("1     No New Mailbox Dialog after already exists",LogPath,CritErrPath)
				Exit Function
			End If
			
			' Cancel Creation since we already have it
			ClickButton("Button", 2, RES$(IDS_BUTTON_CANCEL))

			Exit Function
		End If
		
		
	EndIf

			
End Function

'*************************************************************************
' Function CreateMailboxFolder
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' FolderName$  Folder Name to create
' MbxName$    Mailbox Name to create under folder
'
' Returns True/False Integer
'
'*************************************************************************

Function CreateMailboxFolder(FolderName$, MbxName$) As Integer
	CreateMailboxFolder = True
	Dim UtilReturnInteger As Integer
	Dim MailboxWazooHandle as long
	Dim EudoraParentName as String

	UtilReturnInteger = OpenMailboxes()
	if UtilReturnInteger = False Then
		CreateMailboxFolder = False
		LogWrite("1    Could not Find Mailboxes Tree to Create.",LogPath,CritErrPath)		
		Exit Function
	Else

		' retrieve Parent Folder Name/Tree Header
		EudoraParentName = GetEudoraParentFolderName()
		if EudoraParentName = NULL Then
			CreateMailboxFolder = False
			LogWrite("1    Could not Get Eudora Parent Name.",LogPath,CritErrPath)		
			Exit Function
		End If
		
		MailboxWazooHandle = WTreeFind( NULL,  (FindWindowTimeout) )

		if MailboxWazooHandle = NULL Then
			CreateMailboxFolder = False
			LogWrite("1    Could not Find Mailboxes Tree.",LogPath,CritErrPath)		
			Exit Function
		End If
		WTreeItemClk ( _hwnd(MailboxWazooHandle), EudoraParentName, VK_RBUTTON, (FindWindowTimeout))
		WMenuSelect(RES$(IDS_MENU_SIG_NEW))
	
		DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_MAILBOX), FW_DIALOG or FW_FOCUS)
		if DialogWindow = 0 Then
			CreateMailboxFolder = False
			LogWrite("1     No New Mailbox Dialog",LogPath,CritErrPath)
			Exit Function
		End If
		
		DialogWindow = EnterEditBox("Edit",1,"",FolderName)
		If DialogWindow = 0 Then
			CreateMailboxFolder = False
			LogWrite("1     Could Not Enter Folder Name",LogPath,CritErrPath)
			Exit Function
		End If
		
		Play "{TAB}"
		Play " "
		ClickButton("Button", 1, RES$(IDS_BUTTON_OK))

		' See if I get Already Exists error
		DialogWindow = WFndWnd("Eudora", FW_DIALOG or FW_FOCUS)
		if DialogWindow <> 0 Then
			LogWrite("1     A Folder/Mailbox exists with that name",LogPath,CritErrPath)
			ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
			
			Sleep 2

			' Find New Mailbox Dialog Again
			DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_MAILBOX), FW_DIALOG or FW_FOCUS)
			if DialogWindow = 0 Then
				CreateMailboxFolder = False
				LogWrite("1     No New Mailbox Dialog after already exists",LogPath,CritErrPath)
				Exit Function
			End If
			
			' Cancel Creation since we already have it
			ClickButton("Button", 2, RES$(IDS_BUTTON_CANCEL))

			Exit Function
		End If
		
		DialogWindow = WFndWnd(RES$(IDS_CAPTION_NEW_MAILBOX), FW_DIALOG or FW_FOCUS)
		if DialogWindow = 0 Then
			CreateMailboxFolder = False
			LogWrite("1     No New Mailbox Dialog",LogPath,CritErrPath)
			Exit Function
		End If
		DialogWindow = EnterEditBox("Edit",1,"",MbxName)
		If DialogWindow = 0 Then
			CreateMailboxFolder = False
			LogWrite("1     Could Not Enter Mailbox Name",LogPath,CritErrPath)
			Exit Function
		End If

		ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
	EndIf

End Function

'*************************************************************************
' Function OpenFilters
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Returns True/False Integer
'
'*************************************************************************
Function OpenFilters() As Integer
	Dim UtilReturnInteger As Integer
	Dim tmpstr As String

	tmpstr = RES$(IDS_MENU_TOOLS) + "\" + RES$(IDS_MENU_TOOLS_FILTERS)
	OpenFilters = True
	
	' Select Tools\Filters
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		OpenFilters = False
		LogWrite("1    Could not Open Filters.",LogPath,CritErrPath)
	Else
		LogWrite("0     Successfully Launched Filters.",LogPath,CritErrPath)
	End If
	Sleep 3

End Function
'*************************************************************************
' Function InitMakeFilters
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Creating Initial Filters As defined by Brief Basic Functionality
' Test Rev 2.2 by David Hom <dhom@qualcomm.com>
'
' Returns True/False
'
'*************************************************************************

Function InitMakeFilters() As Integer
	InitMakeFilters = True
	Dim UtilReturnInteger As Integer
	Dim I as Integer
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			First Filter			First Filter			First Filter			First Filter				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	UtilReturnInteger = OpenFilters()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Opening/Finding Filters Window",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Error Clicking New Button to make New Filter: 1",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating First Filter: alpha ignore testing",LogPath,CritErrPath)
	End If

	Play "{F6}"
	Play "%(m)"

	WComboItemClk ( _ord(1), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",1,"","alpha")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: alpha",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter1 Subject: param: alpha",LogPath,CritErrPath)
	End If
	
	WComboItemClk ( _ord(4), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",2,"","testing")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: testing",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter1 Subject: param: testing",LogPath,CritErrPath)
	End If

	WComboSetFocus( _ord(6), (FindWindowTimeout) )
	For I = 0 To 16
	Play "{DOWN}"    ' Transfer To:
	Next I
	LogWrite("0     Set First Filter1 Action: Transfer To:",LogPath,CritErrPath)

	ClickButton("Button", 3, RES$(IDS_MENU_MAILBOX_IN))
	WMenuSelect("Test1")
	LogWrite("0     Set First Filter1 Action param: Test1",LogPath,CritErrPath)

	WComboSetFocus( _ord(7), (FindWindowTimeout) )
	Play "{DOWN}"   ' Make Status
	LogWrite("0     Set Second Filter1 Action: Make Status:",LogPath,CritErrPath)
	
	WComboItemClk ( _ord(11), RES$(IDS_MENU_MESSAGE_CHANGE_STATUS_READ),(FindWindowTimeout) )
	LogWrite("0     Set Second Filter1 Action param: Read",LogPath,CritErrPath)
	
	WComboSetFocus( _ord(8), (FindWindowTimeout) )
	Play "{DOWN}"   ' Make Priority
	LogWrite("0     Set Third Filter1 Action: Make Priority:",LogPath,CritErrPath)

	WComboSetFocus( _ord(12), (FindWindowTimeout) )
	Play "{UP}"   ' Make Priority Highest
	Play "{UP}"   ' Make Priority High
	LogWrite("0     Set Third Filter1 Action param: High",LogPath,CritErrPath)

	WComboSetFocus( _ord(9), (FindWindowTimeout) )
	Play "{DOWN}"   ' Make Label
	LogWrite("0     Set Fourth Filter1 Action: Make Label:",LogPath,CritErrPath)

	ClickButton("Button", 4, RES$(IDS_MENU_MESSAGE_CHANGE_LABEL_NONE))
	WMenuSelect("@3")
	LogWrite("0     Set Fourth Filter1 Action param: Label2 Red",LogPath,CritErrPath)

	Play "^(s)"
	UtilReturnInteger = SendFilterMessageOne()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Sending First Filter Message to test first filter",LogPath,CritErrPath)
		Exit Function
	End If

	UtilReturnInteger = SendFilterMessageTwo()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Sending Second Filter Message to test first filter",LogPath,CritErrPath)
		Exit Function
	End If

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Second Filter			Second Filter			Second Filter			Second Filter				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	UtilReturnInteger = OpenFilters()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Opening/Finding Filters Window",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Error Clicking New Button to make New Filter: 2",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Second Filter: alpha or gamma",LogPath,CritErrPath)
	End If

	Play "{F6}"
	Play "%(m)"

	WComboItemClk ( _ord(1), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",1,"","alpha")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: alpha",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter2 Subject: param: alpha",LogPath,CritErrPath)
	End If

	WComboItemClk ( _ord(3), RES$(IDS_COMBO_OR),(FindWindowTimeout) )
	
	WComboItemClk ( _ord(4), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",2,"","gamma")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: gamma",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter2 Subject: param: gamma",LogPath,CritErrPath)
	End If

	
	WComboSetFocus( _ord(6), (FindWindowTimeout) )
	For I = 0 To 2
	Play "{DOWN}"
	Next I     ' Make Label
	LogWrite("0     Set First Filter2 Action: Make Label:",LogPath,CritErrPath)

	ClickButton("Button", 3, RES$(IDS_MENU_FILTER_ACTION_NONE))
	WMenuSelect("@5")
	LogWrite("0     Set First Filter2 Action param: Label4 Lt. Blue",LogPath,CritErrPath)

	WComboSetFocus( _ord(7), (FindWindowTimeout) )
	For I = 0 To 15
	Play "{DOWN}"
	Next I      ' Transfer To:
	LogWrite("0     Set Second Filter2 Action: Transfer To:",LogPath,CritErrPath)

	ClickButton("Button", 4, RES$(IDS_MENU_MAILBOX_IN))
	WMenuSelect("Test3")
	LogWrite("0     Set Second Filter2 Action param: Test3",LogPath,CritErrPath)

	' Save Filter
	Play "^(s)"

	' Send Third Message to test this filter
	UtilReturnInteger = SendFilterMessageThree()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Sending Third Filter Message to test second filter",LogPath,CritErrPath)
		Exit Function
	End If

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Modify Filter 1			Modify Filter 1			Modify Filter 1			Modify Filter 1				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	WListItemClk ( _ord(1), "@1", (FindWindowTimeout) )
	
	WComboSetFocus( _ord(10), (FindWindowTimeout) )
	For I = 0 To 17
	Play "{Down}"
	Next I      ' Skip Rest

	LogWrite("0     Set Fifth Filter1 Action: Skip Rest:",LogPath,CritErrPath)

	' Set Focus Back to Second Filter
	WListItemClk ( _ord(1), "@2", (FindWindowTimeout) )
	
	Play "^(s)"


'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Third Filter			Third Filter			Third Filter			Third Filter				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	UtilReturnInteger = OpenFilters()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Opening/Finding Filters Window",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Error Clicking New Button to make New Filter: 3",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Third Filter: From jmiller Subject testing",LogPath,CritErrPath)
	End If

	Play "{F6}"
	Play "%(m)"

	WComboItemClk ( _ord(1), RES$(IDS_COMBO_FROM),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",1,"","jmiller")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing From: param: jmiller",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter3 From: param: jmiller",LogPath,CritErrPath)
	End If

	WComboItemClk ( _ord(3), RES$(IDS_COMBO_AND),(FindWindowTimeout) )
	
	WComboItemClk ( _ord(4), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",2,"","testing")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: testing",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter3 Subject: param: testing",LogPath,CritErrPath)
	End If

	
	WComboSetFocus( _ord(6), (FindWindowTimeout) )
	For I = 0 To 15
	Play "{DOWN}"
	Next I     ' Copy To
	LogWrite("0     Set First Filter3 Action: Copy To:",LogPath,CritErrPath)

	ClickButton("Button", 3, RES$(IDS_MENU_MAILBOX_IN))
	WMenuSelect("Test2\Test2")
	LogWrite("0     Set First Filter3 Action param: Test2 -> Test2",LogPath,CritErrPath)

	' Save Filter
	Play "^(s)"

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Fourth Filter			Fourth Filter			Fourth Filter			Fourth Filter				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
	UtilReturnInteger = OpenFilters()
	If UtilReturnInteger = False Then
		InitMakeFilters = False
		LogWrite("1     Error Opening/Finding Filters Window",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = ClickButton("Button", 1, RES$(IDS_BUTTON_NEW))
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Error Clicking New Button to make New Filter: 4",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Creating Fourth Filter: beta unless delta",LogPath,CritErrPath)
	End If

	Play "{F6}"
	Play "%(m)"

	WComboItemClk ( _ord(1), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",1,"","beta")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: beta",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter4 Subject: param: beta",LogPath,CritErrPath)
	End If

	WComboItemClk ( _ord(3), RES$(IDS_COMBO_UNLESS),(FindWindowTimeout) )
	
	WComboItemClk ( _ord(4), RES$(IDS_COMBO_SUBJECT),(FindWindowTimeout) )

	DialogWindow = EnterEditBox("Edit",2,"","delta")
	If DialogWindow = 0 Then
		InitMakeFilters = False
		LogWrite("1     Critical Error Choosing Subject: param: delta",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     Entered Filter4 Subject: param: delta",LogPath,CritErrPath)
	End If

	
	WComboSetFocus( _ord(6), (FindWindowTimeout) )
	For I = 0 To 13
	Play "{DOWN}"
	NExt I     ' Reply With
	LogWrite("0     Set First Filter4 Action: Reply With:",LogPath,CritErrPath)

	WComboItemClk ( _ord(11), "teststat",(FindWindowTimeout) )
	LogWrite("0     Set First Filter4 Action param: teststat",LogPath,CritErrPath)
	


	WComboSetFocus( _ord(7), (FindWindowTimeout) )
	For I = 0 To 2
	Play "{DOWN}"
	Next I     ' Make Label
	LogWrite("0     Set Second Filter4 Action: Make Label:",LogPath,CritErrPath)

	ClickButton("Button", 3, RES$(IDS_MENU_MESSAGE_CHANGE_LABEL_NONE))
	WMenuSelect("@4")
	LogWrite("0     Set Second Filter4 Action param: Label3 Purple",LogPath,CritErrPath)
	' Save Filter
	Play "^(s)" 
	' Close Filters
	Play "^({F4})"

End Function


'*************************************************************************
' Function SendFilterMessageOne
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Called From InitMakeFilters () To create the first email based on
' Brief Basic Functionality Test Rev 2.2 by David Hom <dhom@qualcomm.com>
'
' Returns True/False
'
'*************************************************************************

Function SendFilterMessageOne() As Integer
	SendFilterMessageOne = True
	Dim UtilReturnInteger As Integer


	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMessageOne = False
		LogWrite("1     Can't Start New Message for filter Test One!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: alpha 1
	Play "alpha 1"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the First Filter Test...
	Play "This is the First Filter Test"
	Play "~"
	Play "Should be in Folder: Test1, Status: Read, Priority: High, Label: 2(Red)"
	Play "~"
	
	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent Filter Message One!",LogPath,CritErrPath)
		
End Function

'*************************************************************************
' Function SendFilterMessageTwo
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Called From InitMakeFilters () To create the second email based on
' Brief Basic Functionality Test Rev 2.2 by David Hom <dhom@qualcomm.com>
'
' Returns True/False
'
'*************************************************************************

Function SendFilterMessageTwo() As Integer 
	SendFilterMessageTwo = True
	Dim UtilReturnInteger As Integer


	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMessageTwo = False
		LogWrite("1     Can't Start New Message for filter Test Two!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: alpha 2 testing
	Play "alpha 2 testing"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the second message for the First Filter Test...
	Play "This is the second message for the First Filter Test"
	Play "~"
	Play "Should be in Folder: Test1, Status: Read, Priority: High, Label: 2(Red)"
	Play "~"
	
	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent Filter Message Two!",LogPath,CritErrPath)
		
End Function

'*************************************************************************
' Function SendFilterMessageThree
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Called From InitMakeFilters () To create the third email based on
' Brief Basic Functionality Test Rev 2.2 by David Hom <dhom@qualcomm.com>
'
' Returns True/False
'
'*************************************************************************

Function SendFilterMessageThree() As Integer
	SendFilterMessageThree = True
	Dim UtilReturnInteger As Integer


	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMessageThree = False
		LogWrite("1     Can't Start New Message for filter Test Three!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: alpha 3
	Play "alpha 3"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the third message for the Second Filter Test...
	Play "This is the third message for the Second Filter Test"
	Play "~"
	Play "Should be in Folder: Test1, Status: Read, Priority: High, Label: 2(Red)"
	Play "~"
	Play "Test1 Mailbox should be open"
	Play "~"

	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent Filter Message Three!",LogPath,CritErrPath)
		
End Function


'*************************************************************************
' Function SendFilterMails
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Creates Mailings to match the Filters created in InitMakeFilters
' They come from:
' Brief Basic Functionality Test Rev 2.2 by David Hom <dhom@qualcomm.com>
'
' Returns True/False
'
'*************************************************************************

Function SendFilterMails() As Integer

	SendFilterMails = True
	Dim UtilReturnInteger As Integer



'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			First Message			First Message			First Message			First Message				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMails = False
		LogWrite("1     Can't Start New Message for SendFilterMails Mail 1!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: alpha 4
	Play "alpha 4"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the First message for the Filter matching Test...
	Play "This is the First message for the Filter matching Test"
	Play "~"
	Play "Should be in Folder: Test1, Status: Read, Priority: High, Label: 2 - Red"
	Play "~"
	Play "Test1 Mailbox should be open"
	Play "~"

	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent SendFilterMails Mail 1!",LogPath,CritErrPath)

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Second Message			Second Message			Second Message			Second Message				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMails = False
		LogWrite("1     Can't Start New Message for SendFilterMails Mail 2!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: gamma
	Play "gamma"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the Second message for the Filter matching Test...
	Play "This is the Second message for the Filter matching Test"
	Play "~"
	Play "Should be in Folder: Test3, Label: 4 - Lt. Blue"
	Play "~"
	Play "Test3 Mailbox should be open"
	Play "~"

	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent SendFilterMails Mail 2!",LogPath,CritErrPath)

	
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Third Message			Third Message			Third Message			Third Message				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMails = False
		LogWrite("1     Can't Start New Message for SendFilterMails Mail 3!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: testing
	Play "testing"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the Third message for the Filter matching Test...
	Play "This is the Third message for the Filter matching Test"
	Play "~"
	Play "Should be in Folder: Test2 + Inbox"
	Play "~"
	Play "Test2 + Inbox Mailboxes should be open"
	Play "~"

	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent SendFilterMails Mail 3!",LogPath,CritErrPath)


'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Fourth Message			Fourth Message			Fourth Message			Fourth Message				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMails = False
		LogWrite("1     Can't Start New Message for SendFilterMails Mail 4!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: beta
	Play "beta"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the Fourth message for the Filter matching Test...
	Play "This is the Fourth message for the Filter matching Test"
	Play "~"
	Play "Should be in Folder: Inbox , Label: 3 - Purple"
	Play "~"
	Play "Inbox Mailbox should be open, Message should be queued with teststat in outbox"
	Play "~"

	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent SendFilterMails Mail 4!",LogPath,CritErrPath)


'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'			Fifth Message			Fifth Message			Fifth Message			Fifth Message				'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

	UtilReturnInteger = StartMessage() 
	If UtilReturnInteger = False Then
		SendFilterMails = False
		LogWrite("1     Can't Start New Message for SendFilterMails Mail 5!",LogPath,CritErrPath)
		Exit Function
	End If

	Sleep 3

	' To: jmiller@swamp.qualcomm.com
	Play "jmiller@swamp.qualcomm.com"     
	Play "{TAB}"
	' Subject: beta delta
	Play "beta delta"
	Play "{TAB}"
	Play "{TAB}"
	Play "{TAB}"
	' Body: This is the Fifth message for the Filter matching Test...
	Play "This is the Fifth message for the Filter matching Test"
	Play "~"
	Play "Should be in Folder: Inbox"
	Play "~"
	Play "Inbox Mailbox should be open"
	Play "~"

	Sleep 2

	Play "^(e)" 
	LogWrite("0     Sent SendFilterMails Mail 5!",LogPath,CritErrPath)

End Function


'*************************************************************************
' Function OpenTopLevelMailboxByName
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' MbxName$    Mailbox Name to Open
'
' Returns True/False Integer
'
'*************************************************************************

Function OpenTopLevelMailboxByName(MbxName$) as Integer
	OpenTopLevelMailboxByName = True
	Dim UtilReturnInteger As Integer
	Dim MailboxWazooHandle as long

	UtilReturnInteger = OpenMailboxes()
	If UtilReturnInteger = False Then
		OpenTopLevelMailboxByName = False
		LogWrite("1    Could not Find Mailboxes Tree to Open.",LogPath,CritErrPath)		
		Exit Function
	Else
		
		MailboxWazooHandle = WTreeFind( NULL,  (FindWindowTimeout) )

		If MailboxWazooHandle = NULL Then
			OpenTopLevelMailboxByName = False
			LogWrite("1    Could not Find Mailboxes Tree.",LogPath,CritErrPath)		
			Exit Function
		End If
		
		WTreeItemDblClk ( _hwnd(MailboxWazooHandle), MbxName, VK_LBUTTON, (FindWindowTimeout))
		Sleep 4

	EndIf

End Function


'*************************************************************************
' Function GetEudoraParentFolderName
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' Returns String of Eudora's Parent Folder. 
' Tells what mailbox tree head is called
'
'*************************************************************************
Function GetEudoraParentFolderName() as String
	GetEudoraParentFolderName = NULL
	
	Dim UtilReturnInteger As Integer
	Dim EudoraParentName as String
	Dim I As Integer
	Dim RightSlash As Integer
	Dim J As Integer
	
	' Grabs VT's command line specifying location of VT ini file
	UtilReturnInteger = CheckCommandLine() 
	' Test Return Value
	If UtilReturnInteger = False Then
		LogError("Error Retrieving Command Line in "+CurPrgName+"!",0,LogPath,CritErrPath)
	End If
	
	' Retrieves Eudora path from INI file
	UtilReturnInteger = GetEudora("Eudora",TheIniFile)
	' Test Return Value
	If UtilReturnInteger = False Then
		LogError("Error Finding Eudora Name "+CurPrgName+"!",0,LogPath,CritErrPath)
	End If

	
	I = 1
	J = 1
	
	' Looks for the last right slash c:\test\eudora\eudora.exe
	While I <> 0
		RightSlash = I
		I = INSTR( J, EudoraName, "\")
		J = I + 1
	WEnd	
	RightSlash = RightSlash  - 1

	' Pull out the eudora.exe portion c:\test\eudora
	EudoraParentName = Left$(EudoraName, RightSlash)

	I = 1
	J = 1

	' Looks for the last right slash  again c:\test\eudora
	While I <> 0
		RightSlash = I
		I = INSTR( J, EudoraParentName, "\")
		J = I + 1
	WEnd	
	J = Len(EudoraParentName)
	
	' Pull out parent folder name eudora
	GetEudoraParentFolderName = Right$(EudoraParentName, (J - RightSlash))

End Function

'*************************************************************************
' Function SendETFMail
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' Recipient$ - Recipient of this ETF message
' Subject$ - Subject of this ETF Message
' PicturePath$ - Path of the picture to embedd, NULL if none
'
' Returns True/False Integer
'
'*************************************************************************
Function SendETFMail(Recipient$, Subject$, PicturePath$) as Integer
	SendETFMail = True
	Dim UtilReturnInteger As Integer
	Dim screenres as Long
	screenres = GetResolutionInLong() 

	' start My ETF message
	UtilReturnInteger = StartMessage()
	If UtilReturnInteger = False Then
		SendETFMail = False
		LogWrite("1     Could not Start an ETF Message",LogPath,CritErrPath)
		Exit Function
	End If

	' Maximize new Window. Should have text 'No Recipient, No Subject'
	' Using FW_PART flag, because I don't want to fail to find window
	' if Eudora is maximized. It would fail because the Word Eudora
	' precedes the Recipient and Subject.
	' We are maximizing in order to have a 'known' location of buttons 
	' in the composition frame
    MainAppWindow = WFndWnd(RES$(IDS_CAPTION_BLANK_NEW_MESSAGE),FW_PART Or FW_MAXIMIZE ,(FindWindowTimeout))

	' The following will fill in the To: and Subject
	If MainAppWindow = 0 Then
		SendETFMail = False
       ' Write error message
		LogWrite("1     Can't find New Message Window in ETF Message!",LogPath,CritErrPath)
		Exit Function
	End If

	LogWrite("0     Preparing to write recipient's name in an ETF message: "+Recipient+"!",LogPath,CritErrPath)

	DialogWindow = EnterEditBox("Edit",1,RES$(IDS_BUTTON_TO),Recipient)
	If DialogWindow = 0 Then
		SendETFMail = False
		LogWrite("1     Could Not Enter Recipient Name: "+Recipient+" for an ETF message",LogPath,CritErrPath)
		Exit Function
	End If


	LogWrite("0     Preparing to write Subject in an ETF message: "+Subject+"!",LogPath,CritErrPath)

	DialogWindow = EnterEditBox("Edit",3,RES$(IDS_COMBO_SUBJECT),Subject)
	If DialogWindow = 0 Then
		SendETFMail = False
		LogWrite("1     Could Not Enter Subject Name: "+Subject+" for an ETF message",LogPath,CritErrPath)
		Exit Function
	End If

	
	EnterMessageBodyRes()
	Sleep 1
	InsertLine()
	Sleep 1
	InsertPicture(PicturePath) 
	Sleep 1
	CenterTextRes()
	Sleep 1
	Play "~"
	Play "Centered aligned Text"
	Play "~"
	RightTextRes()
	Sleep 1
	Play "Right aligned Text"
	Play "~"
	LeftTextRes() 
	Sleep 1
	Play "Left aligned Text"
	Play "~"
	IndentInRes()
	Sleep 1
	Play "Indented Text"
	Play "~"
	IndentInRes()
	Sleep 1
	Play "More Indented Text"
	Play "~"
	IndentOutRes()
	Sleep 1
	Play "Indented Text"
	Play "~"
	IndentOutRes()
	Sleep 1
	Play "Unindented Text"
	Play "~"
	BulletListRes()
	Sleep 1
	Play "Bullet 1"
	Play "~"
	Play "Bullet 2"
	Play "~"
	BulletListRes()
	Sleep 1
	TypeWriterRes()
	Sleep 1
	Play "TypeWriter Type Text"
	Play "~"
	Sleep 1
	TypeWriterRes()
	Sleep 1
	UnderLineRes()
	Sleep 1
	Play "UnderLine"
	Play "~"
	Sleep 1
	UnderLineRes()
	Sleep 1
	ItalicRes()
	Sleep 1
	Play "ItalicText"
	Play "~"
	Sleep 1
	ItalicRes()
	Sleep 1
	BoldRes()
	Sleep 1
	Play "Bold Text"
	Play "~"
	Sleep 1
	BoldRes()
	Sleep 1
	ChangeColorRes("teal", 0)
	Sleep 1
	Play "Teal Text"
	Play "~"
	Sleep 1
	ChangeColorRes("black", 1)
	Sleep 1
	TextSizeRes(RES$(IDS_MENU_EDIT_TEXT_SIZE_HUMONGOUS))
	Sleep 1
	Play "humongous"
	Play "~"
	Sleep 1
	TextSizeRes(RES$(IDS_MENU_EDIT_TEXT_SIZE_MEDIUM))
	Sleep 1
	SelectFont("Verdana")
	Sleep 1
	Play "Verdana"
	Play "~"
	Sleep 1
	SelectFont("Arial")
	Sleep 1
	Play "Visit Eudora"
	HighlightLastWords(2)
	Sleep 1
	InsertHyperlinkRes("http://www.eudora.com")
	Sleep 1
	Play "~"
	ChangeColorRes("red", 0)
	Sleep 1
	Play "Text was red then clear Formatted"
	Sleep 1
	ChangeColorRes("black", 1)
	Sleep 1
	HighlightLastWords(6)
	Sleep 1
	ClearFormattingRes()
	Sleep 1
	Play "~"
	Sleep 1
	
	SetPriority("High")




	Play "^(e)"
	LogWrite("0     Sent ETF Message with above styles!",LogPath,CritErrPath)
	
End Function



'*************************************************************************
' Function SelectFont
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Selects composition font
'
' Parameters:
'
' FontName$ - Font Name to use
'
' Returns True/False Integer
'
'*************************************************************************
Function SelectFont(FontName$) As Integer
	SelectFont = True


	WComboItemClk ( _ord(1), FontName,(FindWindowTimeout) )

	LogWrite("0     Selected Font: "+FontName+"!",LogPath,CritErrPath)
	

End Function
'*************************************************************************
' Function SetPriority
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Selects Message priority
'
' Parameters:
'
' Priority$ - Priority to use
'
' Returns True/False Integer
'
'*************************************************************************
Function SetPriority(Priority$) As Integer
	SetPriority = True

	If Priority = RES$(IDS_MENU_MESSAGE_CHANGE_PRIORITY_HIGHEST) Then
		WComboItemClk ( _ord(2), "@1",(FindWindowTimeout) )
	ElseIf Priority = RES$(IDS_MENU_MESSAGE_CHANGE_PRIORITY_HIGH) Then
		WComboItemClk ( _ord(2), "@2",(FindWindowTimeout) )
	ElseIf Priority = RES$(IDS_MENU_MESSAGE_CHANGE_PRIORITY_NORMAL) Then
		WComboItemClk ( _ord(2), "@3",(FindWindowTimeout) )
	ElseIf Priority = RES$(IDS_MENU_MESSAGE_CHANGE_PRIORITY_LOW) Then
		WComboItemClk ( _ord(2), "@4",(FindWindowTimeout) )
	ElseIf Priority = RES$(IDS_MENU_MESSAGE_CHANGE_PRIORITY_LOWEST) Then
		WComboItemClk ( _ord(2), "@5",(FindWindowTimeout) )
	End If

	LogWrite("0     Selected Message Priority: "+Priority+"!",LogPath,CritErrPath)
	

End Function
'*************************************************************************
' Function SetSignature
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Sets Signature for message
'
' Parameters:
'
' SigName$ - Signature Name to use
'
' Returns True/False Integer
'
'*************************************************************************
Function SetSignature(SigName$, SigIndex%) As Integer
	SetSignature = True
	
	select case SigIndex
	
	case 1
		WComboItemClk ( _ord(3), "@1",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 1!",LogPath,CritErrPath)
	case 2
		WComboItemClk ( _ord(3), "@2",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 2!",LogPath,CritErrPath)
	case 3
		WComboItemClk ( _ord(3), "@3",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 3!",LogPath,CritErrPath)
	case 4
		WComboItemClk ( _ord(3), "@4",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 4!",LogPath,CritErrPath)
	case 5
		WComboItemClk ( _ord(3), "@5",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 5!",LogPath,CritErrPath)
	case 6
		WComboItemClk ( _ord(3), "@6",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 6!",LogPath,CritErrPath)
	case 7
		WComboItemClk ( _ord(3), "@7",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 7!",LogPath,CritErrPath)
	case 8
		WComboItemClk ( _ord(3), "@8",(FindWindowTimeout) )
		LogWrite("0     Selected Signature: "+SigName+" at Index: 8!",LogPath,CritErrPath)
	end select

End Function
'*************************************************************************
' Function SetEncoding
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Sets the encoding type of attachments
'
' Parameters:
'
' EncodeType$ - Type of encoding to use, see below
'
' Returns True/False Integer
'
'*************************************************************************
Function SetEncoding(EncodeType$) As Integer
	SetEncoding = True

	If EncodeType = RES$(IDS_COMBO_MIME) Then
		WComboItemClk ( _ord(4), "@1",(FindWindowTimeout) )
	ElseIf EncodeType = RES$(IDS_COMBO_BINHEX) Then
		WComboItemClk ( _ord(4), "@2",(FindWindowTimeout) )
	ElseIf EncodeType = RES$(IDS_COMBO_UUE) Then
		WComboItemClk ( _ord(4), "@3",(FindWindowTimeout) )
	End If

	LogWrite("0     Selected Attachment Encoding: "+EncodeType+"!",LogPath,CritErrPath)
	

End Function

'*************************************************************************
' Function InsertPicture
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Chooses Edit\Insert\Horizontal Line
'
' Parameters:
'
' PicturePath$ - Path of the picture and filename
'
' Returns True/False Integer
'
'*************************************************************************
Function InsertPicture(PicturePath$) as Integer
	InsertPicture = True
	Dim UtilReturnInteger As Integer
	Dim filename As String
	Dim tmpstr As String


	tmpstr = RES$(IDS_MENU_EDIT) + "\" + RES$(IDS_MENU_EDIT_INSERT) + "\" + RES$(IDS_MENU_EDIT_INSERT_PICTURE)
	' Select Edit\Insert\Picture
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		InsertPicture = False
		LogWrite("1     Could not Insert Picture!",LogPath,CritErrPath)
	End If

	DialogWindow = WFndWnd(RES$(IDS_MENU_FILTER_ACTION_OPEN), FW_DIALOG)
	if DialogWindow = 0 Then
		InsertPicture = False
		LogWrite("1     Could not find Dialog to open Picture",LogPath,CritErrPath)
		Exit Function
	End If

	DialogWindow = EnterEditBox("Edit",1,"",PicturePath)
	If DialogWindow = 0 Then
		InsertPicture = False
		LogWrite("1     Could not Type File name in Edit",LogPath,CritErrPath)
		Exit Function
	End If
	
	filename = ParseFileName(PicturePath)

	LogWrite("0     Opening Picture file:"+PicturePath+"!",LogPath,CritErrPath)
		
	ClickButton("Button", 1, RES$(IDS_MENU_FILTER_ACTION_OPEN))
				
End Function
'*************************************************************************
' Function InsertLine
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Chooses Edit\Insert\Horizontal Line
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************

Function InsertLine() as Integer
	InsertLine = True
	Dim UtilReturnInteger as Integer
	Dim tmpstr As String


	tmpstr = RES$(IDS_MENU_EDIT) + "\" + RES$(IDS_MENU_EDIT_INSERT) + "\" + RES$(IDS_MENU_EDIT_INSERT_HLINE)

	' Select Edit\Insert\Horizontal Line
	UtilReturnInteger = SelectMenuItem(MainAppWindow,tmpstr ,tmpstr )
	' Check Return Value
	If UtilReturnInteger = False Then
		InsertLine = False
		LogWrite("1     Could not Insert Line!",LogPath,CritErrPath)
	End If

End Function

'*************************************************************************
' Function ParseFileName
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Chooses Edit\Insert\Horizontal Line
'
' Parameters:
'
' FileAndPath$ - Path of the File and filename
'
' Returns filename.ext where filename is the filename and ext is the 
' extension
'
'*************************************************************************
Function ParseFileName(FileAndPath$) as String
	ParseFileName = NULL
	Dim I as Integer
	Dim J as Integer
	Dim RightSlash as Integer

	I = 1
	J = 1

	' Looks for the last right slash  again c:\test\eudora
	While I <> 0
		RightSlash = I
		I = INSTR( J, FileAndPath, "\")
		J = I + 1
	WEnd	
	J = Len(FileAndPath)
	
	' Pull out parent folder name eudora
	ParseFileName = Right$(FileAndPath, (J - RightSlash))

End Function

'*************************************************************************
' Function HighlightLastWords
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Highlights Last Word
'
' Parameters:
'
' NumberOfWords - Number of prior words to highlight
'
' Returns True/False
'
'*************************************************************************
Function HighlightLastWords(NumberOfWords%) as Integer
	HighlightLastWords = True
	Dim I as Integer

	For I = 1 To NumberOfWords
	Play "^(+({LEFT}))"      	
	Next I

End Function
