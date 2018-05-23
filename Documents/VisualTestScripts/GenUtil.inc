'**************************************************
' GenUtil.INC
' Utility Functions and Procedures 
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
' July 1997
'**************************************************
' ====================================================================
' Metacommand Declarations
' ====================================================================
'$IFNDEF _GenUtil_INCLUDED	   
'$DEFINE _GenUtil_INCLUDED
' ====================================================================
'============================================================
'============================================================
' Construct flag variables for windows search
' Variables on right side of equal sign override default settings
'============================================================
Const FNDWINDFLGS = FW_FOCUS Or FW_PART Or FW_HIDDENOK	' Partial Match OK
Const FNDINSTFLGS = FW_FOCUS Or FW_HIDDENOK		        ' No Partial Match
Const FNDNOFOCUSFLGS = FW_NOFOCUS Or FW_HIDDENOK        ' No Focus Given
Const FNDPARTNOFOCUSFLGS = FW_PART Or FW_NOFOCUS Or FW_HIDDENOK ' Partial No Focus
'============================================================
' Window Handle Variables
'============================================================
Global FindWindowTimeout    As Integer	'Default Window Search time
Global FindErrorTimeout     As Integer  'Error Window Search time
Global MainAppWindow        As Long     'Handle for Main Application Window
Global WarningWindow        As Long		'Window Handle variable
Global ErrorWindow          As Long		'Window Handle variable
Global DialogWindow         As Long		'Window Handle variable
Global TheCurrentWindow     As Long		'Window Handle variable
'============================================================
' Global Variables
'============================================================
Global LogPath              As String    ' Error.log
Global CritErrPath          As String    ' Criterr.log
Global SMLogPath            As String    ' Status.log
Global SystemError          As Integer   ' System Error
Global RevNumber            As String    ' Revision Number
Global TheIniFile           As String    ' INI file
Global CurPrgName           As String    ' Current Program
Global ExplorerName         As String    ' Explorer.exe path and name
Global TheProductVersion    As String    ' Product Version
Global TheMachineName       As String    ' Machine name
Global MTRunExePath         As String    ' Path and Name of MTRUN.EXE
Global MTExePath            As String    ' Path and Name of MT.EXE

'============================================================
' Network Distribution Variables
'============================================================
Global MainComputerName     As String   'Name assigned to Host Computer 
Global ProjectDirPath		As String	'Dir where scripts are located
Global RunPrgCfgFile        As String	'RunPrg CFG File Path and Name
Global RunPrgIniPath 		As String	'RunPrg INI File Path and Name
Global TimerIniPath 		As String   'Timer INI Path and Name
Global NetHostPCDPath		As String	'NetHost.pcd Path and Name
Global NetHostCfgFile		As String	'NetHost Config File Path and Name
Global CopyLogDir			As String	'Dir for Remotes to copy log files 
Global TimeSyncServer		As String   'Computer to sync net time with
Global intUseStationCount   As Integer  'True/False variable
Global intHostSleepTime		As Integer  'Amount of time to sleep - used by Host
Global intNetDist           As Integer	'Are we in Network Load Test used by Client
Global intStationID         As Integer	'WorkStation ID given by Host

' Host identifier
Const HOST    = "TheHost"
' Go Word
Const SYNC_ID = "NetTest"

' Construct Data Types for TALK_DATA
Const DT_FILENAME   = 257
Const DT_DISCONNECT = 258
Const DT_STRING     = 259

' ==================================================
' RunPrg Multiple Configuration file variables
' ==================================================
' The idea of the following is that One RunPrg.pcd can utilize
' different configuration files while running at different workstations.
' Each configuration file may contain a different set of programs
' to run.
' This way we can provide a mix of programs for more realistic tests.
' The question is how do we tell the Workstation/RunPrg which configuration
' file to use?
' The answer is by the Station ID assigned when the Station connects
' to the Host.
' If you are Station number X then you use configuration file X.
' The station then sends the configuration name to RunPrg.pcd as a parameter.

' Multiple Configuration File usage
Const UseMultCfgFile = True

' Array to store Configuration File Names 
' and station numbers to use them
Type CfgFileArray
	CfgFileName	   As String
	CfgFileNumber  As Integer
End Type

' Total number of configuration files listed below (number must match):
' You may construct as many configuration files as you wish
Const TotalCfgFile = 3  ' This number must match with number of config files

' Create variable for array and set array size
Global CfgFileDriver(TotalCfgFile) As CfgFileArray

' If The following Configuration files have the same name then
' it should be obvious that there is really only one file.
' If you wish to use more than 1 Configuration file then
' you must give them all unique names.

' Default Configuration file
CfgFileDriver(1).CfgFileName   = ProjectDirPath+"RunPrg.cfg"
CfgFileDriver(1).CfgFileNumber = 1      ' Max Number of Workstations to use it
										' Set to desired number
' Second Configuration file
CfgFileDriver(2).CfgFileName   = ProjectDirPath+"RunPrg.cfg"
CfgFileDriver(2).CfgFileNumber = 2  	' Max Number of Workstations to use it
										' Set to desired number

' Third Configuration file
CfgFileDriver(3).CfgFileName   = ProjectDirPath+"RunPrg.cfg"
CfgFileDriver(3).CfgFileNumber = 250	' Max Number of Workstations to use it
										' Set to desired number


' Sets Virtual Key for KeyPress event to terminate program
Const VK_ESCAPE     = &H1B&

'============================================================
' Global Subroutine/Function Declarations
'============================================================

'---------------------------
' General Functions Chap 1-4
'---------------------------
Declare Function CheckCommandLine() As Integer
Declare Function ReadINI(TheIniFile$,TheSection$,TheVariable$) As String
Declare Function SetDefaults(SectionName$,TheIniFile$) As Integer
Declare Function CompressString(TheString$) As String
Declare Function ParseDN(DName$) As String
Declare Function ParseFN(FName As string) As string
Declare Function CompareDirFiles(FileDirName1$,FileDirName2$) As Integer 
Declare Function GetDirSize (DirName$) As Integer
Declare Function GetFileSpecs(FileName$) As Integer
Declare Sub StopScripts(vKeyInfo As Variant)
Declare Sub LogOpen(LogPath$,CritErrPath$)
Declare Sub LogClose(LogPath$,CritErrPath$)
Declare Sub LogEnter(Message$,LogPath$,CritErrPath$)
Declare Sub LogWrite(Message$,LogPath$,CritErrPath$)
Declare Sub LogOnly(Message$,LogPath$,CritErrPath$)
Declare Sub LogError(Message$,ApplicationToClose%,LogPath$,CritErrPath$)
Declare Sub TestLogVars(LogPath$)
Declare Sub MakeDirs(DirName$)
Declare Sub UAEErrorTrap(UAEErrMsg As Variant)
Declare Sub CloseOnSystemError(ApplicationToClose&,LogPath$,CritErrPath$)

'--------------------------------
' Object Manipulation Functions
'--------------------------------
Declare Function CollapseTree(ClassName$,TreeOrdinal%) As Integer
Declare Function ClickButton(ClassType$,BtnOrdValue%,ButtonName$) As Integer
Declare Function ClickButtonID(ClassType$,BtnIDValue%,ButtonName$) As Integer
Declare Function ClickGrid(GridCaption$,GridClass$,GridName$,GRight%,GDown%) As Integer
Declare Function ClickOption(ClassType$,OptOrdValue%,OptionName$) As Integer
Declare Function DblClickButton(ClassType$,BtnOrdValue%,ButtonName$) As Integer
Declare Function DblClkEditBox(ClassType$,OrdValue%,FieldName$) As Integer
Declare Function EnterEditBox(ClassType$,EditOrdValue%,FieldName$,TextToEnter$) As Integer
Declare Function ExpandTree(ClassName$,TreeOrdinal%) As Integer
Declare Function GetEditText(ClassType$,EditOrdValue%,FieldName$) As String
Declare Function GetCrlProperties(ControlName$) As Integer
Declare Function GetListItems(ClassName$,ListOrdinal%) As Integer
Declare Function GetComboItems(ClassName$,ComboOrdinal%) As Integer
Declare Function GetStatusItems(ClassName$,StatusOrdinal%) As Integer
Declare Function GetTreeItems(ClassName$,TreeOrdinal%) As Integer
Declare Function GetViewItems(ClassName$,ViewOrdinal%) As Integer
Declare Function ReadvsFlexArray(ControlName$) As Integer
Declare Function SelectCheckBox(ClassName$,CheckBoxOrdinal%,CheckBoxName$,CheckState$) As Integer
Declare Function SelectMenuItem(WindowHandle&,MenuItem$,MenuItemName$) As Integer
Declare Function SelectTabItem(ClassName$,TabOrdinal%,TabItemName$) As Integer
Declare Function SelectViewItem(ClassName$,ViewOrdinal%,ViewItemName$) As Integer

'------------------------------
' Time/Date Functions
'------------------------------
Declare Function AddDays(StartDate$,NumberToAdd%) As String
Declare Function AddMinutes(StartDate$,NumberToAdd%) As String
Declare Function AddMonths(StartDate$,NumberToAdd%) As String
Declare Function AddYears(StartDate$,NumberToAdd%) As String
Declare Function GetDaysInMonth(TheDate$) As String
Declare Function GetTheDay(TheDate$) As String
Declare Function GetTheMonth(TheDate$) As String
Declare Function GetTheYear(TheDate$) As String
Declare Sub UpdateMemStats(App2Test$,IniFile$,ScenarioName$,StatFileName$)

'------------------------------
' Explorer Specific Functions
'------------------------------
Declare Function StartExplorer() As Integer
Declare Function CloseExplorer() As Integer

'----------------------------------------
' Network Distribution Specific Functions
'----------------------------------------
Declare Function SetNDDefaults(SectionName$,TheIniFile$) As Integer

'============================================================
' Create Array Structure to input vsFlexArray cell data
'============================================================
Type CellArray
	Cell As String
End Type
'============================================================
' Initialize Global variables
'============================================================
FindWindowTimeout = 10        ' Set Find Window Timout Interval
FindErrorTimeout  =  5        ' Set Find Error Window Timout Interval
SetDeadLockTimeout(500000)    ' Set Deadlock Timeout Interval
'============================================================
' Procedure and Function Section Follows:
'============================================================
'*************************************************************************
'*************************************************************************
'*************************************************************************
'
'  Object Manipulation Functions
'
'*************************************************************************
'*************************************************************************
'*************************************************************************
'*************************************************************************
' Function SelectMenuItem()
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' WindowHandle& -   The Handle of the window containing the menu
' MenuItem$		-   The Menu item to select i.e. &File\E&xit 
' MenuItemName$	-   Descriptive name of menu item i.e. File\Exit      
'
' 	' Select File\Close from menu to exit
'	ReturnInteger = SelectMenuItem(MainAppWindow,"&File\&Close","File\Close")
'
'
'*************************************************************************
Function SelectMenuItem(WindowHandle&,MenuItem$,MenuItemName$) As Integer

	SelectMenuItem = True

	' Search for Sytem Menu in nominated window
	If WSysMenuExists(WindowHandle) Then
		Sleep .25
		LogWrite("0     Found System Menu.",LogPath,CritErrPath)
		WSysMenu(WindowHandle)
		Sleep .25
		LogWrite("0     Selected System Menu.",LogPath,CritErrPath)
		' Select Insert Contact/Account
		If WMenuExists(MenuItem,(FindWindowTimeout)) Then
			Sleep .25
			If WMenuEnabled(MenuItem,(FindWindowTimeout)) Then
				Sleep .25
				WMenuSelect(MenuItem)
				Sleep .25
				LogWrite("0     Selected Menu item: "+MenuItemName+".",LogPath,CritErrPath)
			Else
				SelectMenuItem = False
				' Write error message
				LogWrite("1     "+MenuItemName+" Bar isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			SelectMenuItem = False
			' Write error message
			LogWrite("1     Can't find "+MenuItemName+" Bar on System Menu!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		SelectMenuItem = False
		' Write error message
		LogWrite("1     Can't find System Menu to "+MenuItemName+"!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function Function ExpandTree(ClassName$,TreeOrdinal%) As Integer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of Tree from WInfo
' TreeOrdinal% - Ordinal Value of Tree from WInfo
'
' Returns True/False
'
' Expands Tree and writes path names to Error.log
'
'*************************************************************************
Function ExpandTree(ClassName$,TreeOrdinal%) As Integer

	Dim TreeHandle     As Long
	Dim TreeCount      As Integer
	Dim TreeCounter    As Integer
	Dim TreeString     As String
	Dim TreeItemsExist As Integer

	ExpandTree = True
    
	' Set the Tree Class
	Sleep .5
	WTreeSetClass(ClassName)

	' Get handle for Tree Control
	TreeHandle = WTreeFind(_ord((TreeOrdinal)),(FindWindowTimeout))
	If TreeHandle <> 0 Then
		' Find Tree Control
		If WTreeExists(_hWnd(TreeHandle),(FindWindowTimeout)) Then
			' Make sure Tree is enabled
			If WTreeEnabled(_hWnd(TreeHandle),(FindWindowTimeout)) Then
				' Count Tree Items
				TreeCount = WTreeCount(_hWnd(TreeHandle),(FindWindowTimeout))
				' Loop through Tree	Items
				If TreeCount > 0 Then
					' Initialize variable
					TreeItemsExist = True
					LogWrite("0     ==============",LogPath,CritErrPath)
					LogWrite("0     Expanding Tree",LogPath,CritErrPath)
					LogWrite("0     ==============",LogPath,CritErrPath)
					' Expand Tree View and write items to Log File
					Do
						' Increment Counter
						TreeCounter = TreeCounter + 1
						' Test for Tree Item
						If WTreeItemExists(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))	Then
							' Ignore Floppy CDROM Drives and Network directory
							If Instr(1,TreeItemText (_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)),"A:" ) = 0 _
							And Instr(1,TreeItemText (_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)),"D:" ) = 0 _
							And Instr(1,TreeItemText (_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)),"Network" ) = 0 Then
								Sleep .15
								' Expand Tree Item
								If Not WTreeItemExpanded(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)) Then
									WTreeItemExpand(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))
									' Click Tree Item
									WTreeItemClk(_hwnd((TreeHandle)),_idx((TreeCounter)),VK_LBUTTON,(FindWindowTimeout))
								End If
							End If
							' Get Text of Tree Item Path
							TreeString = TreeItemPath(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))
							' Write Tree Items to Log File
							LogWrite("0     "+TreeString,LogPath,CritErrPath)
						Else
							' All done exit loop
							LogWrite("0     Tree Item Count: "+ Trim(Str$(TreeCounter-1))+".",LogPath,CritErrPath)
							TreeItemsExist = False
						End If
					Loop While TreeItemsExist
				Else
					ExpandTree = False
					LogWrite("1     No Items found in Tree!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				ExpandTree = False
				LogWrite("1     Tree Control isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			ExpandTree = False
			LogWrite("1     Tree Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		ExpandTree = False
		LogWrite("1     Can't Find Tree Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function Function CollapseTree(ClassName$,TreeOrdinal%) As Integer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of Tree from WInfo
' TreeOrdinal% - Ordinal Value of Tree from WInfo
'
' Returns True/False
'
' Expands Tree and writes path names to Error.log
'
'*************************************************************************
Function CollapseTree(ClassName$,TreeOrdinal%) As Integer

	Dim TreeHandle     As Long
	Dim TreeCount      As Integer
	Dim TreeCounter    As Integer
	Dim TreeString     As String

	CollapseTree = True
    
	' Set the Tree Class
	Sleep .5
	WTreeSetClass(ClassName)

	' Get handle for Tree Control
	TreeHandle = WTreeFind(_ord((TreeOrdinal)),(FindWindowTimeout))
	If TreeHandle <> 0 Then
		' Find Tree Control
		If WTreeExists(_hWnd(TreeHandle),(FindWindowTimeout)) Then
			' Make sure Tree is enabled
			If WTreeEnabled(_hWnd(TreeHandle),(FindWindowTimeout)) Then
				' Count Tree Items
				TreeCount = WTreeCount(_hWnd(TreeHandle),(FindWindowTimeout))
				' Loop through Tree	Items
				If TreeCount > 0 Then
					LogWrite("0     ===============",LogPath,CritErrPath)
					LogWrite("0     Collapsing Tree",LogPath,CritErrPath)
					LogWrite("0     ===============",LogPath,CritErrPath)
					For TreeCounter = TreeCount To 1 Step -1
						' Test for Tree Item
						If WTreeItemExists(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))	Then
							' Ignore Floppy and CDROM Drive
							If Instr(1,TreeItemText (_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)),"A:" ) = 0 _
							And Instr(1,TreeItemText (_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)),"D:" ) = 0 Then
								' Collapse Tree Item
								If WTreeItemExpanded(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout)) Then
									Sleep .15
									WTreeItemCollapse(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))
									' Click Tree Item
									WTreeItemClk(_hwnd((TreeHandle)),_idx((TreeCounter)),VK_LBUTTON,(FindWindowTimeout))
								End If
							End If
							' Get Text of Tree Item Path
							TreeString = TreeItemPath(_hwnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))
							' Write Tree Items to Log File
							LogWrite("0     "+TreeString,LogPath,CritErrPath)
						Else
							CollapseTree = False
							LogWrite("0     Tree Item: "+ Trim(Str$(TreeCounter-1))+" doesn't exist!",LogPath,CritErrPath)
							Exit Function
						End If
					Next
				Else
					CollapseTree = False
					LogWrite("1     No Items found in Tree!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				CollapseTree = False
				LogWrite("1     Tree Control isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			CollapseTree = False
			LogWrite("1     Tree Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		CollapseTree = False
		LogWrite("1     Can't Find Tree Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function Function GetTreeItems(ClassName$,ViewOrdinal%) As Integer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of Tree from WInfo
' TreeOrdinal% - Ordinal Value of Tree from WInfo
'
' Returns True/False
'
' Prints results to Error.log
'
'*************************************************************************
Function GetTreeItems(ClassName$,TreeOrdinal%) As Integer

	Dim TreeHandle   As Long
	Dim TreeCount    As Integer
	Dim TreeCounter  As Integer
	Dim ItemText     As String

	GetTreeItems = True
    
	' Set the Tree Class
	Sleep .5
	WTreeSetClass(ClassName)

	' Get handle for Tree Control
	TreeHandle = WTreeFind(_ord((TreeOrdinal)),(FindWindowTimeout))
	If TreeHandle <> 0 Then
		' Find Tree Control
		If WTreeExists(_hWnd(TreeHandle),(FindWindowTimeout)) Then
			' Make sure Tree is enabled
			If WTreeEnabled(_hWnd(TreeHandle),(FindWindowTimeout)) Then
				' Count Tree Items
				TreeCount = WTreeCount(_hWnd(TreeHandle),(FindWindowTimeout))
				' Loop through Tree	Items
				If TreeCount > 0 Then
					For TreeCounter = 1 To TreeCount
						' Write Tree Items to Log File
						ItemText = TreeItemText(_hWnd((TreeHandle)),_idx((TreeCounter)),(FindWindowTimeout))
						LogWrite("0     Item"+Trim(Str(TreeCounter))+":"+ItemText+".",LogPath,CritErrPath)
					Next
				Else
					GetTreeItems = False
					LogWrite("1     No Items found in Tree!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				GetTreeItems = False
				LogWrite("1     Tree Control isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			GetTreeItems = False
			LogWrite("1     Tree Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		GetTreeItems = False
		LogWrite("1     Can't Find Tree Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function SelectTabItem(ClassName$,TabOrdinal%,TabItemName$)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' ClassName$   - Class Name of Tab from WInfo
' TabOrdinal%  - Ordinal Value of Tab Control from WInfo
' TabItemName$ - Name of Tab To Select
'
' Returns True/False
'
' ReturnInteger = SelectTabItem("SysTabControl32",1,"View")
'
'*************************************************************************
Function SelectTabItem(ClassName$,TabOrdinal%,TabItemName$) As Integer

	Dim TabHandle As Long
	SelectTabItem = True
    
	' Set the Tab Class
	Sleep .5
	WTabSetClass (ClassName)

	' Get handle for Tab Control
	TabHandle = WTabFind(_ord((TabOrdinal)),(FindWindowTimeout))
	If TabHandle <> 0 Then
		' Find Tab Control
		If WTabExists(_hWnd(TabHandle),(FindWindowTimeout)) Then
			' Find Tab Item
			If WTabItemExists(_hWnd(TabHandle),TabItemName,(FindWindowTimeout)) Then
				' Click Tab Item
				WTabItemClk(_hWnd(TabHandle),TabItemName,(FindWindowTimeout))
				LogWrite("0     Selected Tab Item "+TabItemName+".",LogPath,CritErrPath)
			Else
				SelectTabItem = False
				LogWrite("1     Tab Item "+TabItemName+" doesn't exist!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			SelectTabItem = False
			LogWrite("1     Tab Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		SelectTabItem = False
		LogWrite("1     Can't Find Tab Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function ClickButton
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassType$  	From WInfo i.e.	  Button
' OrdValue%		From WInfo i.e.	  1
' ButtonName$	Button Caption i.e. Yes
'
' Syntax Example:
' Click OK
' ReturnInteger=ClickButton("Button",1,"OK")
'
' Returns True/False Integer
'
'*************************************************************************
Function ClickButton(ClassType$,BtnOrdValue%,ButtonName$) As Integer

	Dim ObjectHandle As Long

	ClickButton = True

	' Set the Button Class
	Sleep 1
	WButtonSetClass(ClassType)

	' Click Button
	ObjectHandle = WButtonFind(_ord((BtnOrdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WButtonExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WButtonEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				WButtonClick(_hWnd(ObjectHandle),(FindWindowTimeout))
				LogWrite("0     Clicked "+ButtonName+".",LogPath,CritErrPath)
			Else
				ClickButton = False
				' Write error message
				LogWrite("1     "+ButtonName+" Button isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			ClickButton = False
			' Write error message
			LogWrite("1     "+ButtonName+" Button doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		ClickButton = False
		' Write error message
		LogWrite("1     Can't Find "+ButtonName+" Button!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function ClickButtonID
'		Based on ClickButton by:
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassType$  	From WInfo i.e.	  Button
' IDValue%		From WInfo i.e.	  1
' ButtonName$	Button Caption i.e. Yes
'
' Syntax Example:
' Click OK
' ReturnInteger=ClickButton("Button",1,"OK")
'
' Returns True/False Integer
'
'*************************************************************************
Function ClickButtonID(ClassType$,BtnIDValue%,ButtonName$) As Integer

	Dim ObjectHandle As Long

	ClickButtonID = True

	' Set the Button Class
	Sleep 1
	WButtonSetClass(ClassType)

	' Click Button
	ObjectHandle = WButtonFind(_id((BtnIDValue)),(40))
	If ObjectHandle <> 0 Then
		If WButtonExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WButtonEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				WButtonClick(_hWnd(ObjectHandle),(FindWindowTimeout))
				LogWrite("0     Clicked "+ButtonName+".",LogPath,CritErrPath)
			Else
				ClickButtonID = False
				' Write error message
				LogWrite("1     "+ButtonName+" Button isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			ClickButtonID = False
			' Write error message
			LogWrite("1     "+ButtonName+" Button doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		ClickButtonID = False
		' Write error message
		LogWrite("1     Can't Find "+ButtonName+" Button!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function ClickGrid
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Does a mouse click on nominated Grid
'
' Parameters:
'
' GridCaption$  From WInfo   
' GridClass$	From WInfo 
' GridName$	    Screen Name of Grid or however you identify it
' GRight%       Position from Left Edge of Grid Window
' GDown%        Position from Top Edge of Grid Window
'
' Returns True/False Integer
'
' Syntax Example:
' ReturnInteger=ClickGrid("","TslGrid","A Funky Grid",70,22)
'
'*************************************************************************
Function ClickGrid(GridCaption$,GridClass$,GridName$,GRight%,GDown%) As Integer

	Dim GridHandle As Long

	ClickGrid = True

	' Find Grid window
    GridHandle = WFndWndC(GridCaption,GridClass,FNDWINDFLGS,(FindWindowTimeout))
	If GridHandle Then

		LogWrite("0     Found Grid "+ GridName+".",LogPath,CritErrPath)

		' Click Grid
		Play GridHandle,"{CLICK "+Str$(GRight)+","+Str$(GDown)+" }"
		LogWrite("0     Clicked on: "+GridName+".",LogPath,CritErrPath)

	Else
		ClickGrid = False
		' Write error message
		LogWrite("1     Can't find Grid "+ GridName+"!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function DblClickButton
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassType$  	From WInfo i.e.	  "Button"
' BtnOrdValue%	From WInfo i.e.	  1
' ButtonName$	Button Caption i.e. Yes
'
' Syntax Example:
' Double Click OK
' ReturnInteger=DblClickButton("Button",1,"OK")
'
' Returns True/False Integer
'
'*************************************************************************
Function DblClickButton(ClassType$,BtnOrdValue%,ButtonName$) As Integer

	Dim ObjectHandle As Long

	DblClickButton = True

	' Set the Button Class
	Sleep 1
	WButtonSetClass(ClassType)

	' Click Button
	ObjectHandle = WButtonFind(_ord((BtnOrdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WButtonExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WButtonEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				' Double click on nominated field
				Play ObjectHandle,"{DBLCLICK}"
				LogWrite("0     Double Clicked on: "+ButtonName+".",LogPath,CritErrPath)
			Else
				DblClickButton = False
				' Write error message
				LogWrite("1     "+ButtonName+" Button isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			DblClickButton = False
			' Write error message
			LogWrite("1     "+ButtonName+" Button doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		DblClickButton = False
		' Write error message
		LogWrite("1     Can't Find "+ButtonName+" Button!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function DblClkEditBox
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Does a double mouse click on nominated Edit box.
'
' Parameters:
'
' ClassType$    From WInfo i.e.	  Edit
' OrdValue%		From WInfo i.e.	  1
' FieldName$	Screen Field Name  "Address1"
'
' Returns True/False Integer
'
' Syntax Example:
' Double Click Address1
' ReturnInteger=DblClkEditBox("Edit",1,"Address1")
'
'*************************************************************************
Function DblClkEditBox(ClassType$,OrdValue%,FieldName$) As Integer

	Dim ObjectHandle As Long

	DblClkEditBox = True

	' Set the Edit Class
	Sleep 1
	WEditSetClass(ClassType)

	' Find Handle for nominated field
	ObjectHandle = WEditFind(_ord((OrdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WEditExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WEditEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				' Double click on nominated field
				Play ObjectHandle,"{DBLCLICK}"
				LogWrite("0     Double Clicked on: "+FieldName+".",LogPath,CritErrPath)
			Else
				DblClkEditBox = False
				' Write error message
				LogWrite("1     "+FieldName+" Edit isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			DblClkEditBox = False
			' Write error message
			LogWrite("1     "+FieldName+" Edit doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		DblClkEditBox = False
		' Write error message
		LogWrite("1     Can't Find "+FieldName+" Edit!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function ClickOption
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassType$  	From WInfo i.e.	  Button
' OrdValue%		From WInfo i.e.	  1
' OptionName$	Option Caption i.e. Yes
'
' Syntax Example:
' Click Show All Files
' ReturnInteger=ClickOption("Button",1,"Show All Files")
'
' Returns True/False Integer
'
'*************************************************************************
Function ClickOption(ClassType$,OptOrdValue%,OptionName$) As Integer

	Dim ObjectHandle As Long

	ClickOption = True

	' Set the Option Class
	Sleep 1
	WOptionSetClass(ClassType)

	' Click Option
	ObjectHandle = WOptionFind(_ord((OptOrdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WOptionExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WOptionEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				WOptionClick(_hWnd(ObjectHandle),(FindWindowTimeout))
				LogWrite("0     Clicked "+OptionName+".",LogPath,CritErrPath)
			Else
				ClickOption = False
				' Write error message
				LogWrite("1     "+OptionName+" Option isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			ClickOption = False
			' Write error message
			LogWrite("1     "+OptionName+" Option doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		ClickOption = False
		' Write error message
		LogWrite("1     Can't Find "+OptionName+" Option!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function GetEditText
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassType$    From WInfo i.e.	  Edit
' OrdValue%		From WInfo i.e.	  1
' FieldName$	Screen Field Name "Address1"
'
' Returns Edit String or "ERROR"
'
' Syntax Example:
' Get Address1
' EditString=GetEditText("Edit",1,"Address1")
'
'*************************************************************************
Function GetEditText(ClassType$,EditOrdValue%,FieldName$) As String

	Dim ObjectHandle   As Long
	Dim	EditTextString As String

	' Set the Edit Class
	Sleep .5
	WEditSetClass(ClassType)

	' Enter New Type
	ObjectHandle = WEditFind(_ord((EditOrdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WEditExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WEditEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				EditTextString = Trim$(EditText(_hWnd(ObjectHandle),(FindWindowTimeout)))
				If Len(EditTextString) > 0 Then
					LogWrite("0     Found "+FieldName+" Text: "+ EditTextString+".",LogPath,CritErrPath)
					GetEditText = EditTextString
				Else
					GetEditText = "ERROR"
					' Write error message
					LogWrite("1     No text Found in "+FieldName+"!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				GetEditText = "ERROR"
				' Write error message
				LogWrite("1     "+FieldName+" Edit isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			GetEditText = "ERROR"
			' Write error message
			LogWrite("1     "+FieldName+" Edit doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		GetEditText = "ERROR"
		' Write error message
		LogWrite("1     Can't Find "+FieldName+" Edit!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function GetListItems(ClassName$,ListOrdinal%)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of List from WInfo
' ListOrdinal% - Ordinal Value of List from WInfo
'
' Returns True/False
'
' Prints results to Error.log
'
'*************************************************************************
Function GetListItems(ClassName$,ListOrdinal%) As Integer

	Dim ListHandle   As Long
	Dim ListCount    As Integer
	Dim ListCounter  As Integer
	Dim ItemText     As String

	GetListItems = True
    
	' Set the List Class
	Sleep .5
	WListSetClass(ClassName)

	' Get handle for List Control
	ListHandle = WListFind(_ord((ListOrdinal)),(FindWindowTimeout))
	If ListHandle <> 0 Then
		' Find List Control
		If WListExists(_hWnd(ListHandle),(FindWindowTimeout)) Then
			' Make sure List is enabled
			If WListEnabled(_hWnd(ListHandle),(FindWindowTimeout)) Then
				' Count List Items
				ListCount = WListCount(_hWnd(ListHandle),(FindWindowTimeout))
				' Loop through List	Items
				If ListCount > 0 Then
					LogWrite("0     --------------------- ",LogPath,CritErrPath)
					LogWrite("0     Contents of List Box: ",LogPath,CritErrPath)
					LogWrite("0     --------------------- ",LogPath,CritErrPath)
					For ListCounter = 1 To ListCount
						' Write List Items to Log File
						ItemText = ListItemText(_hWnd(ListHandle),(ListCounter),(FindWindowTimeout))
						LogWrite("0     Item"+Trim(Str(ListCounter))+": "+ItemText+".",LogPath,CritErrPath)
					Next
				Else
					GetListItems = False
					LogWrite("1     No Items found in List!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				GetListItems = False
				LogWrite("1     List Control isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			GetListItems = False
			LogWrite("1     List Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		GetListItems = False
		LogWrite("1     Can't Find List Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function GetComboItems(ClassName$,ComboOrdinal%)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of Combo from WInfo
' ComboOrdinal% - Ordinal Value of Combo from WInfo
'
' Returns True/False
'
' Prints results to Error.log
'
'*************************************************************************
Function GetComboItems(ClassName$,ComboOrdinal%) As Integer

	Dim ComboHandle   As Long
	Dim ComboCount    As Integer
	Dim ComboCounter  As Integer
	Dim ItemText      As String

	GetComboItems = True
    
	' Set the Combo Class
	Sleep .5
	WComboSetClass(ClassName)

	' Get handle for Combo Control
	ComboHandle = WComboFind(_ord((ComboOrdinal)),(FindWindowTimeout))
	If ComboHandle <> 0 Then
		' Find Combo Control
		If WComboExists(_hWnd(ComboHandle),(FindWindowTimeout)) Then
			' Make sure Combo is enabled
			If WComboEnabled(_hWnd(ComboHandle),(FindWindowTimeout)) Then
				' Count Combo Items
				ComboCount = WComboCount(_hWnd(ComboHandle),(FindWindowTimeout))
				' Loop through Combo Items
				If ComboCount > 0 Then
					LogWrite("0     --------------------- ",LogPath,CritErrPath)
					LogWrite("0     Contents of Combo Box: ",LogPath,CritErrPath)
					LogWrite("0     --------------------- ",LogPath,CritErrPath)
					For ComboCounter = 1 To ComboCount
						' Write Combo Items to Log File
						ItemText = ComboItemText(_hWnd(ComboHandle),(ComboCounter),(FindWindowTimeout))
						LogWrite("0     Item"+Trim(Str(ComboCounter))+": "+ItemText+".",LogPath,CritErrPath)
					Next
				Else
					GetComboItems = False
					LogWrite("1     No Items found in Combo!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				GetComboItems = False
				LogWrite("1     Combo Control isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			GetComboItems = False
			LogWrite("1     Combo Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		GetComboItems = False
		LogWrite("1     Can't Find Combo Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function SelectCheckBox(ClassName$,CheckBoxOrdinal%,CheckBoxName$,CheckState$) As Integer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassName$        - Class Name of check box
' CheckBoxOrdinal%  - Ordinal value of check box
' CheckBoxName$     - Name of check box
' CheckState$       - Desired state to set "Check" or "UnCheck"
'
' ReturnInteger = SelectCheckBox("CheckBox",1,"Select This Item","Check")
'
'*************************************************************************
Function SelectCheckBox(ClassName$,CheckBoxOrdinal%,CheckBoxName$,CheckState$) As Integer
	
	Dim CheckBoxHandle As Long
	SelectCheckBox = True

	Sleep .5
	WCheckSetClass(ClassName)

	CheckBoxHandle = WCheckFind(_ord((CheckBoxOrdinal)),(FindWindowTimeout))

	If CheckBoxHandle <> 0 Then
		If WCheckExists(_hWnd(CheckBoxHandle),(FindWindowTimeout)) Then
			If WCheckEnabled(_hWnd(CheckBoxHandle),(FindWindowTimeout)) Then
				If CheckState = "Check" Then
					WCheckCheck(_hWnd(CheckBoxHandle),(FindWindowTimeout))
					LogWrite("0     Set CheckBox "+CheckBoxName+" Checked.",LogPath,CritErrPath)
				Else
					WCheckUnCheck(_hWnd(CheckBoxHandle),(FindWindowTimeout))
					LogWrite("0     Set CheckBox "+CheckBoxName+" UnChecked.",LogPath,CritErrPath)
				End If
			Else
				LogWrite("1     CheckBox "+CheckBoxName+" isn't enabled!",LogPath,CritErrPath)
				SelectCheckBox = False
				Exit Function
			End If
		Else
			LogWrite("1     CheckBox "+CheckBoxName+" doesn't exist!",LogPath,CritErrPath)
			SelectCheckBox = False
			Exit Function
		End If
	Else
		LogWrite("1     Can't Find CheckBox "+CheckBoxName+"!",LogPath,CritErrPath)
		SelectCheckBox = False
		Exit Function
	End If

End Function
'*************************************************************************
' Function SelectViewItem(ClassName$,ViewOrdinal%,ViewItemName$)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' ClassName$    - Class Name of View from WInfo
' ViewOrdinal%  - Ordinal Value of View from WInfo
' ViewItemName$ - Name of View To Select
'
' Returns True/False
'
' ReturnInteger = SelectViewItem("SysListView32",1,"Help File")
'
'*************************************************************************
Function SelectViewItem(ClassName$,ViewOrdinal%,ViewItemName$) As Integer

	Dim ViewHandle As Long
	SelectViewItem = True
    
	' Set the View Class
	Sleep .5
	WViewSetClass(ClassName)

	' Get handle for View Control
	ViewHandle = WViewFind(_ord((ViewOrdinal)),(FindWindowTimeout))
	If ViewHandle <> 0 Then
		' Find View Control
		If WViewExists(_hWnd(ViewHandle),(FindWindowTimeout)) Then
			' Find View Item
			If WViewItemExists(_hWnd(ViewHandle),ViewItemName,(FindWindowTimeout)) Then
				' Dbl Click View Item
				WViewItemDblClk(_hWnd(ViewHandle),ViewItemName,VK_LBUTTON,(FindWindowTimeout))
				LogWrite("0     Selected View Item: "+ViewItemName+".",LogPath,CritErrPath)
			Else
				SelectViewItem = False
				LogWrite("1     View Item: "+ViewItemName+" doesn't exist!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			SelectViewItem = False
			LogWrite("1     View Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		SelectViewItem = False
		LogWrite("1     Can't Find View Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'****************************************************************************************
' Function GetViewItems(ClassName$,ListOrdinal%)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of View from WInfo
' ListOrdinal% - Ordinal Value of View from WInfo
'
' Returns True/False
'
' Puts the View Header and View Items into an array 
' and prints the array to log file as follows:
'
' 06/03/97 12:44:06 PM  0     Name                  Driver                              
' 06/03/97 12:44:06 PM  0     ==========================================================
' 06/03/97 12:44:06 PM  0     dBASE Files           Microsoft dBase Driver (*.dbf)      
' 06/03/97 12:44:06 PM  0     Excel Files           Microsoft Excel Driver (*.xls)      
' 06/03/97 12:44:06 PM  0     FoxPro Files          Microsoft FoxPro Driver (*.dbf)     
' 06/03/97 12:44:06 PM  0     MS Access 97 Database Microsoft Access Driver (*.mdb)     
' 06/03/97 12:44:06 PM  0     Text Files            Microsoft Text Driver (*.txt; *.csv)
'
'*****************************************************************************************
Function GetViewItems(ClassName$,ViewOrdinal%) As Integer

	Dim ViewHandle    As Long
	Dim ViewCount     As Integer
	Dim ViewCounter   As Integer
	Dim HeaderCount   As Integer
	Dim HeaderCounter As Integer
	Dim ItemString    As String
	Dim HeaderString  As String
	Dim ItemLength    As Integer
	Dim ArrayCounter  As Integer
	Dim HeaderArray   As Array Of String
	Dim ItemArray     As Array Of String
	Dim WidthArray    As Array Of Integer

	GetViewItems = True
    
	' Set the View Class
	Sleep .5
	WViewSetClass(ClassName)

	' Get handle for View Control
	ViewHandle = WViewFind(_ord((ViewOrdinal)),(FindWindowTimeout))
	If ViewHandle <> 0 Then
		' Find View Control
		If WViewExists(_hWnd(ViewHandle),(FindWindowTimeout)) Then
			' Make sure View is enabled
			If WViewEnabled(_hWnd(ViewHandle),(FindWindowTimeout)) Then
				' Get count of Header Items of List View
				HeaderCount = WViewHeaderCount(_hwnd(ViewHandle),(FindWindowTimeout))
				' Make sure there are Items in Header
				If HeaderCount > 0 Then
					' Redimension arrays to proper size
					ReDim HeaderArray(1 To HeaderCount) As String 
					ReDim WidthArray(1 To HeaderCount) As Integer 
					' Loop through Header Details
					For HeaderCounter = 1 To HeaderCount
						' Get Header text and put into HeaderArray 
						HeaderArray(HeaderCounter) = ViewHeaderText(_hwnd((ViewHandle)),_idx((HeaderCounter)),(FindWindowTimeout))
						' Determine Column Width and put into WidthArray
						ItemLength = Len(HeaderArray(HeaderCounter))
						If WidthArray(HeaderCounter) <= ItemLength Then
							WidthArray(HeaderCounter) = ItemLength 
						End If
					Next
				Else
					GetViewItems = False
					LogWrite("1     No Items found in Header!",LogPath,CritErrPath)
					Exit Function
				End If
				' Count View Items
				ViewCount = WViewCount(_hWnd(ViewHandle),(FindWindowTimeout))
				' Make sure there are Items in View
				If ViewCount > 0 Then
					' Redimension ItemArray to hold all Items
					ReDim ItemArray(1 To ViewCount*HeaderCount) As String 
					' Loop through View	Items (down)
					For ViewCounter = 1 To ViewCount
						' Loop Through Columns (across)
						For	HeaderCounter = 1 To HeaderCount
							' Increment counter
							ArrayCounter = ArrayCounter + 1
							' Get Items into ItemArray
							ItemArray(ArrayCounter) = ViewItemText(_hwnd((ViewHandle)),_idx((ViewCounter)),_idx((HeaderCounter)),(FindWindowTimeout))
							' Determine Column Width and put into WidthArray
							ItemLength = Len(ItemArray(ArrayCounter))
							If WidthArray(HeaderCounter) <= ItemLength Then
								WidthArray(HeaderCounter) = ItemLength 
							End If
						Next
					Next
				Else
					GetViewItems = False
					LogWrite("1     No Items found in View Control!",LogPath,CritErrPath)
					Exit Function
				End If
			Else
				GetViewItems = False
				LogWrite("1     View Control isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			GetViewItems = False
			LogWrite("1     View Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		GetViewItems = False
		LogWrite("1     Can't Find View Control!",LogPath,CritErrPath)
		Exit Function
	End If

	' Write Header to LogFile
	If HeaderCount > 0 Then
		' Reinitialize string
		HeaderString = ""
		' Loop through header
		For	HeaderCounter = 1 To HeaderCount
			' Create Header String
			HeaderString=HeaderString+Space(1)+"|"+HeaderArray(HeaderCounter)+Space(WidthArray(HeaderCounter)-Len(HeaderArray(HeaderCounter)))
		Next
		' Write Header to Log File
		LogWrite("0     ",LogPath,CritErrPath)
		LogWrite("0     "+String$(Len(HeaderString),"="),LogPath,CritErrPath)
		LogWrite("0     "+LTrim$(HeaderString),LogPath,CritErrPath)
		LogWrite("0     "+String$(Len(HeaderString),"="),LogPath,CritErrPath)
	Else
		GetViewItems = False
		LogWrite("1     No Items found in Header!",LogPath,CritErrPath)
		Exit Function
	End If

	' Write View Items to LogFile
	If ViewCount > 0 Then
		' Reset counter
		ArrayCounter = 0
		' Loop through View	Items (down)
		For ViewCounter = 1 To ViewCount
			' Initialize string
			ItemString = ""
			' Loop Through Columns (across)
			For	HeaderCounter = 1 To HeaderCount
				' Increment counter
				ArrayCounter = ArrayCounter + 1
				' Create String with all Items in columns
				ItemString = ItemString+Space(1)+"|"+ItemArray(ArrayCounter)+Space(WidthArray(HeaderCounter)-Len(ItemArray(ArrayCounter)))
			Next
			' Write View Items to Log File
			LogWrite("0     "+LTrim$(ItemString),LogPath,CritErrPath)
			LogWrite("0     "+String$(Len(ItemString),"-"),LogPath,CritErrPath)
		Next
		' Write Blank Line to Log File
		LogWrite("0     ",LogPath,CritErrPath)
	Else
		GetViewItems = False
		LogWrite("1     No Items found in View Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function EnterEditBox
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' ClassType$    From WInfo i.e.	  Edit
' OrdValue%		From WInfo i.e.	  1
' FieldName$	Screen Field Name
' TextToEnter$  Text to enter in Edit Box
'
' Returns True/False Integer
'
' Syntax Example:
' Enter Address1
' ReturnInteger=EnterEditBox("Edit",1,"Address1","123 Main Street")
'
'*************************************************************************
Function EnterEditBox(ClassType$,EditOrdValue%,FieldName$,TextToEnter$) As Integer

	Dim ObjectHandle As Long

	EnterEditBox = True

	' Set the Edit Class
	Sleep .5
	WEditSetClass(ClassType)

	' Enter New Type
	ObjectHandle = WEditFind(_ord((EditOrdValue)),(FindWindowTimeout))
	If ObjectHandle <> 0 Then
		If WEditExists(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
			If WEditEnabled(_hWnd(ObjectHandle),(FindWindowTimeout)) Then
				WEditSetText(_hWnd(ObjectHandle),TextToEnter,(FindWindowTimeout))
				LogWrite("0     Entered New "+FieldName+": "+ TextToEnter+".",LogPath,CritErrPath)
			Else
				EnterEditBox = False
				' Write error message
				LogWrite("1     "+FieldName+" Edit isn't enabled!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			EnterEditBox = False
			' Write error message
			LogWrite("1     "+FieldName+" Edit doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		EnterEditBox = False
		' Write error message
		LogWrite("1     Can't Find "+FieldName+" Edit!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function GetStatusItems(ClassName$,StatusOrdinal%)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters: 
'
' ClassName$   - Class Name of Status from WInfo
' StatusOrdinal% - Ordinal Value of Status from WInfo
'
' Returns True/False
'
' Prints results to Error.log
'
'*************************************************************************
Function GetStatusItems(ClassName$,StatusOrdinal%) As Integer

	Dim StatusHandle   As Long
	Dim StatusCount    As Integer
	Dim StatusCounter  As Integer
	Dim ItemText     As String

	GetStatusItems = True
    
	' Set the Status Class
	Sleep .5
	WStatusSetClass(ClassName)

	' Get handle for Status Control
	StatusHandle = WStatusFind(_ord((StatusOrdinal)),(FindWindowTimeout))
	If StatusHandle <> 0 Then
		' Find Status Control
		If WStatusExists(_hWnd(StatusHandle),(FindWindowTimeout)) Then
			' Count Status Items
			StatusCount = WStatusCount(_hWnd(StatusHandle),(FindWindowTimeout))
			' Loop through Status	Items
			If StatusCount > 0 Then
				LogWrite("0     ----------------------- ",LogPath,CritErrPath)
				LogWrite("0     Contents of Status Box: ",LogPath,CritErrPath)
				LogWrite("0     ----------------------- ",LogPath,CritErrPath)
				For StatusCounter = 1 To StatusCount
					' Write Status Items to Log File
					ItemText = StatusItemText(_hWnd(StatusHandle),_idx((StatusCounter)),(FindWindowTimeout))
					LogWrite("0     Item"+Trim(Str(StatusCounter))+": "+ItemText+".",LogPath,CritErrPath)
				Next
			Else
				GetStatusItems = False
				LogWrite("1     No Items found in Status!",LogPath,CritErrPath)
				Exit Function
			End If
		Else
			GetStatusItems = False
			LogWrite("1     Status Control doesn't exist!",LogPath,CritErrPath)
			Exit Function
		End If
	Else
		GetStatusItems = False
		LogWrite("1     Can't Find Status Control!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
'*************************************************************************
'*************************************************************************
'
'  Explorer Specific Functions
'
'*************************************************************************
'*************************************************************************
'*************************************************************************
'*************************************************************************
' Function StartExplorer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Attempts to Start Explorer.Exe
'
' Returns True/False
'
'*************************************************************************
Function StartExplorer() As Integer

 	Dim ReturnInteger As Integer

	StartExplorer = True

	'Check Application path
    If Exists(ExplorerName) Then
		' Use API to start the program
		ReturnInteger = StartTheProgram(ExplorerName, NULL)
		If ReturnInteger = False Then
			StartExplorer = False
			LogWrite("1     Can't Start "+ExplorerName+"!",LogPath,CritErrPath)
			Exit Function
		End If
    Else
		StartExplorer = False
        ' Write error message
        LogWrite("1     Can't Find "+ExplorerName+"!",LogPath,CritErrPath)
		Exit Function
	End If

	' Close	StatusBox if open
	StatusBox Close

    ' Check for existence of Application and remember main window handle
    MainAppWindow = WFndWndC("Exploring","ExploreWClass",FNDWINDFLGS Or FW_RESTORE,(FindWindowTimeout))
	If MainAppWindow = 0 Then
		StartExplorer = False
       ' Write error message
		LogWrite("1     Can't find "+ExplorerName+" Main Window!",LogPath,CritErrPath)
		Exit Function
	Else
		LogWrite("0     "+ExplorerName+" Started successfully.",LogPath,CritErrPath)
		' Use API to check validity of Window Handle - that it exists
		LogWrite("0     Checking for existence of Main Explorer Window.",LogPath,CritErrPath)
		If IsWindow(MainAppWindow) <> 0 Then
			' Make sure window is enabled
			If IsWindowEnabled(MainAppWindow) <> 0 Then
				LogWrite("0     "+ExplorerName+" Main Window exists.",LogPath,CritErrPath)
				' Set Main Explorer window as active
				WSetActWnd(MainAppWindow)
				' Make sure window is visible and restore size
				If WIsVisible(MainAppWindow) = 0 Then
				   WMaxWnd(MainAppWindow)
				   WResWnd(MainAppWindow)
				End If
			Else
				LogWrite("1     "+ExplorerName+" Window handle: "+Trim(Str$(MainAppWindow))+" exists but is not enabled.",LogPath,CritErrPath)
				StartExplorer = False
				Exit Function
			End If
		Else
			LogWrite("1     "+ExplorerName+" Window handle: "+Trim(Str$(MainAppWindow))+" is closed.",LogPath,CritErrPath)
			StartExplorer = False
			Exit Function
		End If
		' Test for system menu
		If Not WSysMenuExists(MainAppWindow) Then
			StartExplorer = False
			LogWrite("1     Can't find "+ExplorerName+" System Menu!",LogPath,CritErrPath)
			Exit Function
		Else
			LogWrite("0     Found "+ExplorerName+" System Menu.",LogPath,CritErrPath)
		End If
    End If

End Function
'*************************************************************************
' Function CloseExplorer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Attempts to Close Explorer.Exe
'
' Returns True/False
'
'*************************************************************************
Function CloseExplorer() As Integer

	Dim ReturnInteger As Integer
	Dim DialogInteger As Integer

	CloseExplorer = True

	' Test for Main Application Window Handle
	If MainAppWindow = 0 Then
        MainAppWindow = WFndWndC("Exploring","ExploreWClass",FNDWINDFLGS Or FW_RESTORE,(FindWindowTimeout))
	End If

	' Test again for Main Application Window Handle
	If MainAppWindow = 0 Then
		' Write error message
		LogWrite("1     Can't find "+ExplorerName+" Main Window to Exit!",LogPath,CritErrPath)
		' Try to Terminate Application
		ReturnInteger = StopTheProgram
		If ReturnInteger <> False Then
			LogWrite("0     Terminated Application successfully!",LogPath,CritErrPath)
			Exit Function
		Else
			CloseExplorer = False
			LogWrite("1     Can't Terminate Application!",LogPath,CritErrPath)
			Exit Function
		End if
	Else
		' Use API to check validity of Window Handle - that it exists
		LogWrite("0     Checking for existence of Main "+ExplorerName+" Window.",LogPath,CritErrPath)
		If IsWindow(MainAppWindow) <> 0 Then
			' Make sure window is enabled
			If IsWindowEnabled(MainAppWindow) <> 0 Then
				LogWrite("0     Main "+ExplorerName+" Window exists.",LogPath,CritErrPath)
				' Set Main Explorer window as active
				WSetActWnd(MainAppWindow)
				' Make sure window is visible and restore size
				If WIsVisible(MainAppWindow) = 0 Then
				   WMaxWnd(MainAppWindow)
				   WResWnd(MainAppWindow)
				End If
			Else
				LogWrite("1     "+ExplorerName+" Window handle: "+Trim(Str$(MainAppWindow))+" exists but is not enabled.",LogPath,CritErrPath)
				CloseExplorer = False
				Exit Function
			End If
		Else
			LogWrite("1     "+ExplorerName+" Window handle: "+Trim(Str$(MainAppWindow))+" is closed.",LogPath,CritErrPath)
			CloseExplorer = False
			Exit Function
		End If
	End If

	' Select File\Close from menu to exit
	ReturnInteger = SelectMenuItem(MainAppWindow,"&File\&Close","File\Close")

	' Test Return Value
	If ReturnInteger = False Then
		' Try to Terminate Application
		ReturnInteger = StopTheProgram
		If ReturnInteger <> False Then
			LogWrite("0     Terminated Application successfully!",LogPath,CritErrPath)
			Exit Function
		Else
			LogWrite("1     Can't Terminate "+ExplorerName+" Application!",LogPath,CritErrPath)
			CloseExplorer = False
			Exit Function
		End If
	End If

	' Reset	MainAppWindow to zero
	MainAppWindow = 0

End Function
'**********************************************************
'  GetCrlProperties() - Get Properties from custom control
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Parameters:
'
'  ControlName$  - Name assigned by developer to control
'
'**********************************************************
Function GetCrlProperties(ControlName$) As Integer

	GetCrlProperties = True

	' Make sure ControlName variable is correct
	If Left$(ControlName,1) <> "#" Then
		ControlName = "#"+ControlName
	End If

	' Make sure control exists
	If OleCtlExists(ControlName,(FindWindowTimeout)) Then

		Dim NumberOfProperties As Integer
		Dim ArrayCount As Integer

		LogWrite("*****************************",LogPath,CritErrPath)

		Dim PropertyList(1 To 100) As OLECTL_PROPERTY

		' Determine number of properties in control
		NumberOfProperties = OleCtlPropertyList(ControlName,PropertyList ,(FindWindowTimeout))

		' Print each property
		For ArrayCount = 1 To NumberOfProperties
			LogEnter (PropertyList(ArrayCount).PropName +" "+PropertyList(ArrayCount).PropValue,LogPath,CritErrPath)
		Next

		LogWrite("*****************************",LogPath,CritErrPath)

	Else
		LogWrite("1     Can't Find OLE Control "+ControlName+"!",LogPath,CritErrPath)
		GetCrlProperties = False
		Exit Function
	End If

End Function
'*****************************************************
'  ReadvsFlexArray() - Read cell data from vsFlexArray
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Parameters:
'
'  ControlName$  - Name assigned by developer to control
'
'*****************************************************
Function ReadvsFlexArray(ControlName$) As Integer

	ReadvsFlexArray = True

	' Make sure ControlName variable is correct
	If Left$(ControlName,1) <> "#" Then
		ControlName = "#"+ControlName
	End If

	' Make sure control exists
	If OleCtlExists(ControlName,(FindWindowTimeout)) Then

		Dim i             As Integer
		Dim j             As Integer
		Dim CellCounter   As Integer
		Dim TotalFileSpec As Integer
		Dim TotalCols     As Integer
		Dim TotalRows     As Integer
		Dim RetString     As String

		' Determine number of columns and rows in custom control
		TotalCols = OleCtlGetProperty(ControlName, "Cols")
		TotalRows = OleCtlGetProperty(ControlName, "Rows")

		' Create Array Variable and set array size
		Dim CellDriver(TotalCols*TotalRows) As CellArray

		LogEnter("TotalCols: "+Str$(TotalCols),LogPath,CritErrPath)
		LogEnter("TotalRows: "+Str$(TotalRows),LogPath,CritErrPath)

		' Loop through columns
		For j = 0 To TotalCols -1

			' Inner loop through rows
			For i = 0 To TotalRows -1

				LogEnter("Iteration: "+Str$(j)+" "+Str$(i),LogPath,CritErrPath)

				' Specify active column and row position
				OleCtlSetProperty(ControlName,"Col",j)
				OleCtlSetProperty(ControlName,"Row",i)

				' Select starting and ending column and row position
				OleCtlSetProperty(ControlName,"ColSel",j)
				OleCtlSetProperty(ControlName,"RowSel",i)

				' Retrieve text from active cell
				RetString = OleCtlGetProperty(ControlName, "Text")
				If Len(Trim$(RetString)) > 0 Then
					LogEnter("Text "+RetString,LogPath,CritErrPath)
					' Enter active cell text into array
					CellDriver(CellCounter).Cell = Trim$(RetString)
					' Increment TotalFileSpec
					TotalFileSpec = TotalFileSpec + 1
				End If

				' Increment CellCounter
				CellCounter = CellCounter + 1

			Next i

		Next j

		LogWrite("===>Total Cells: "+Str$(CellCounter),LogPath,CritErrPath)
		LogWrite("***********************",LogPath,CritErrPath)
		LogWrite("Contents of Cell Array:",LogPath,CritErrPath)

		For i = 0 To TotalFileSpec
			LogEnter("Cell"+Str$(i)+" "+CellDriver(i).Cell,LogPath,CritErrPath)
		Next
	
	Else
		LogWrite("1     Can't Find OLE Control "+ControlName+"!",LogPath,CritErrPath)
		ReadvsFlexArray = False
		Exit Function
	End If

End Function
'***************************************************************
' Subroutine:  UpdateMemStats()
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Purpose:	   Check memory for any possible memory leaks over time.
'			   write results to statistics file
'
' Parameters:
'
' App2Test$     - Application tested
' IniFile$      - INI file
' ScenarioName$ - Name of current scenario
' StatFileName$ - Name of Statistics file to write to
'
' Note:	RSRC32.dll is Windows 95 only
'
'***************************************************************
Sub UpdateMemStats(App2Test$,IniFile$,ScenarioName$,StatFileName$)

	Dim lpMemStat           As MEMORYSTATUS
	Dim Msg                 As String
	Dim StatMessage         As String
	Dim MemoryLoad          As String
	Dim TotalPhysicalMemory	As String
	Dim FreePhysicalMemory  As String
	Dim TotalPagingFile     As String
	Dim FreePagingFile      As String
	Dim TotalVirtualMemory  As String
	Dim FreeVirtualMemory   As String
	Dim LogFH               As Integer
    Dim ProgramName         As String
    Dim WorkStation         As String
    Dim VersionNumber       As String
    Dim TestDate            As String
    Dim NoOfStations        As String
    Dim StationCount        As String

    ' Retrieve TheMachineName from Operating System
    TheMachineName = GetMyComputerName()

    ' Retrieve Product version via API
    TheProductVersion = sFileVersion(App2Test) ' Product Version
    'TheProductVersion = GetFileDate(App2Test)+" "+GetMyFileTime(App2Test)' Product Date/time
    'TheProductVersion = GetFileDate(App2Test)' Product Date

    ' Initialize variables
    ProgramName   = CurPrgName
    WorkStation   = TheMachineName+Space(12-Len(TheMachineName))
    VersionNumber = TheProductVersion
    TestDate      = FixTheDate(Date$)

    ' Get number of workstations from RunPrg.INI file
    StationCount = ReadINI(IniFile,"WorkStationCount","NumberOfStations")

    If StationCount <> "NotFnd" Then
	    If Len(StationCount) < 3 Then
		    StationCount = Space(3-Len(StationCount))+StationCount
	    End If
	    NoOfStations = StationCount
    Else 
	    ' Use default value of 1
	    NoOfStations = "  1"
    End If

    If Len(ScenarioName) < 11 Then
    	ScenarioName = ScenarioName+Space(11-Len(ScenarioName))
    End If

	lpMemStat.dwLength = LenB(lpMemStat)
	GlobalMemoryStatus (lpMemStat)

	' Retrieve values
	MemoryLoad = Trim$(Str$(lpMemStat.dwMemoryLoad))
	If Len(MemoryLoad) < 2 Then
	'	MemoryLoad = Space(2-Len(FreePhysicalMemory))+FreePhysicalMemory
		MemoryLoad = " 0"
	End If
	TotalPhysicalMemory = Trim$(Str$(Int(lpMemStat.dwTotalPhys/1000000)))
	If Len(TotalPhysicalMemory) < 2 Then
		TotalPhysicalMemory = Space(2-Len(FreePhysicalMemory))+FreePhysicalMemory
	End If
	FreePhysicalMemory  = Trim$(Str$(Int(lpMemStat.dwAvailPhys/1000)))
	If Len(FreePhysicalMemory) < 4 Then
		FreePhysicalMemory = Space(4-Len(FreePhysicalMemory))+FreePhysicalMemory
	End If
	TotalPagingFile = Trim$(Str$(Int(lpMemStat.dwTotalPageFile/1000000)))
	If Len(TotalPagingFile) < 4 Then
		TotalPagingFile	= Space(4-Len(TotalPagingFile))+TotalPagingFile
	End If
	FreePagingFile = Trim$(Str$(Int(lpMemStat.dwAvailPageFile/1000000)))
	If Len(FreePagingFile) < 4 Then
		FreePagingFile = Space(4-Len(FreePagingFile))+FreePagingFile
	End If
	TotalVirtualMemory = Trim$(Str$(Int(lpMemStat.dwTotalVirtual/1000000)))
	If Len(TotalVirtualMemory) < 4 Then
		TotalVirtualMemory = Space(4-Len(TotalVirtualMemory))+TotalVirtualMemory
	End If
	FreeVirtualMemory = Trim$(Str$(Int(lpMemStat.dwAvailVirtual/1000000)))
	If Len(FreeVirtualMemory) < 4 Then
		FreeVirtualMemory = Space(4-Len(FreeVirtualMemory))+FreeVirtualMemory
	End If

'   Following will only work on Windows 95
'	SystemRes		    = Trim$(Str$(GetRes(0))) '0 = System Resources
'	GDIUsage		    = Trim$(Str$(GetRes(1))) '1 = GDI Resources
'	UserResourceUsage   = Trim$(Str$(GetRes(2))) '2 = User Resources

	' Create message string
	Msg = _
	DateTime+Space(2)+"0"+Space(8)+"============================="+ CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Current Memory Statistics:"+ CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"============================="+ CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Memory Load:           " + Space(2)+MemoryLoad + "%"  + CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Total Physical Memory: " + Space(2)+TotalPhysicalMemory + "MB" + CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Free Physical Memory:  " + FreePhysicalMemory  + "KB" + CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Total Paging File:     " + TotalPagingFile     + "MB" + CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Free Paging File:      " + FreePagingFile      + "MB" + CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Total Virtual Memory:  " + TotalVirtualMemory  + "MB" + CRLF + _
	DateTime+Space(2)+"0"+Space(8)+"Free Virtual Memory:   " + FreeVirtualMemory   + "MB"

'   Following will only work on Windows 95
'	"System Res:            " + SystemRes           + "% used." + CRLF + _ 
'	"GDI Usage:             " + GDIUsage            + "% used." + CRLF + _  
'	"User Resource Usage:   " + UserResourceUsage   + "% used."	 
	                                            
	' Put up message box
'	MsgBox Msg, MB_ICONINFORMATION, "Memory Info"

	' Write info to Error.log
	LogOnly(Msg,LogPath,CritErrPath)

	' Create Message String from variables
	If Exists(StatFileName) Then
		' File exists no need to add header
		StatMessage = ProgramName _
		+ "," + ScenarioName _
		+ "," + WorkStation _	
		+ "," + VersionNumber _	
		+ "," + TestDate _	
		+ "," + NoOfStations _
		+ "," + MemoryLoad _
		+ "," + TotalPhysicalMemory _
		+ "," + FreePhysicalMemory _
		+ "," + TotalPagingFile _
		+ "," + FreePagingFile _
		+ "," + TotalVirtualMemory _
		+ "," + FreeVirtualMemory
	Else
		' Add Header Record if its a new file
		StatMessage = "TestCase" _
		+ "," + "Scenario" _
		+ "," + "WorkStation" _
		+ "," + "Version" _
		+ "," + "TestDate" _
		+ "," + "Stations" _
		+ "," + "MLoad%" _
		+ "," + "TPhsMemMB" _
		+ "," + "FPhsMemKB" _
		+ "," + "TPgFlMB" _
		+ "," + "FPgFlMB" _
		+ "," + "TMemMB" _
		+ "," + "FeVMemMB" _
		+ CHR(13) + CHR(10) _
		+ ProgramName _
		+ "," + ScenarioName _
		+ "," + WorkStation _	
		+ "," + VersionNumber _	
		+ "," + TestDate _	
		+ "," + NoOfStations _
		+ "," + MemoryLoad _
		+ "," + TotalPhysicalMemory _
		+ "," + FreePhysicalMemory _
		+ "," + TotalPagingFile _
		+ "," + FreePagingFile _
		+ "," + TotalVirtualMemory _
		+ "," + FreeVirtualMemory
	End If

	' Get next available file handle
	LogFH=FREEFILE  
	' Open Statistics Log
	Open StatFileName for Append As #LogFH
	' Write info to file
	Print #LogFH, + StatMessage
	' Close Statistics Log
	Close #LogFH
	' Write message to LogFile
	LogWrite("0        Updated: "+StatFileName+".",LogPath,CritErrPath)

End Sub
'*************************************************************************
'*************************************************************************
'*************************************************************************
'
'  Time/Date Functions
'
'*************************************************************************
'*************************************************************************
'*************************************************************************
'**********************************************************
'  AddYears() - Add Years to a date
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Returns String incremented by NumberToAdd
'  i.e. NumberToAdd = 1 Then 10/01/96 returns 97 
'**********************************************************
Function AddYears(StartDate$,NumberToAdd%) As String

	Dim Slash1   As Integer
	Dim Slash2   As Integer
	Dim NewYear  As Integer
	Dim TheYear  As String

	' Find slashes
	Slash1 = Instr(1,StartDate,"/")
	Slash2 = Instr(Slash1+1,StartDate,"/")

	' Add necessary years
	NewYear = Val(Mid$(StartDate,Slash2+1,Len(StartDate)-Slash2))+(NumberToAdd)
	TheYear = Trim$(Str$(NewYear))

	' Add zero to beginning of string if necessary
	If Len(TheYear) = 1 Then
		TheYear = "0"+TheYear
	End If

	' Return value
	AddYears = TheYear

End Function
'**********************************************************
'  AddMonths() - Add Months to a date
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Returns String incremented by NumberToAdd
'  i.e. NumberToAdd = 1 Then 10/01/96 returns 11 
'**********************************************************
Function AddMonths(StartDate$,NumberToAdd%) As String

	Dim Slash1   As Integer
	Dim Slash2   As Integer
	Dim NewMonth As Integer
	Dim TheMonth As String

	' Find slashes
	Slash1 = Instr(1,StartDate,"/")
	Slash2 = Instr(Slash1+1,StartDate,"/")

	' Add necessary months
	NewMonth = Val(Mid$(StartDate,1,Slash1-1))+(NumberToAdd)
	TheMonth = Trim$(Str$(NewMonth))

	' Add zero to beginning of string if necessary
	If Len(TheMonth) = 1 Then
		TheMonth = "0"+TheMonth
	End If

	' Return value
	AddMonths  = TheMonth

End Function
'****************************************************************
'  AddDays() - Add Days to a date
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Returns String incremented by NumberToAdd
'  i.e. NumberToAdd = 1 Then 10/01/96 returns 10/02/96 
'****************************************************************
Function AddDays(StartDate$,NumberToAdd%) As String

	Dim Slash1   As Integer
	Dim Slash2   As Integer
	Dim TheDays  As String
	Dim TheMonth As String
	Dim TheYear  As String

	If Len(Trim(StartDate)) = 0 Then
		LogWrite("1     StartDate parameter is blank!",LogPath,CritErrPath)
		AddDays = "Error"
		Exit Function
	End If

	If Len(Trim(Str$(NumberToAdd))) = 0 Then
		LogWrite("1     NumberToAdd parameter is blank!",LogPath,CritErrPath)
		AddDays = "Error"
		Exit Function
	End If

	Slash1 = Instr(1,StartDate,"/")
	Slash2 = Instr(Slash1+1,StartDate,"/")

	TheDays = Mid$(StartDate,Slash1+1,(Slash2-1)-Slash1)
	TheDays = Trim$(Str(Val(TheDays)+NumberToAdd))
	If Len(TheDays) = 1 Then
		TheDays = "0"+TheDays
	End If

	TheMonth = Mid$(StartDate,1,Slash1-1)
	If Len(TheMonth) = 1 Then
		TheMonth = "0"+TheMonth
	End If

	TheYear  = Mid$(StartDate,Slash2+1,Len(StartDate)-Slash2)
	AddDays  = TheMonth+"/"+TheDays+"/"+TheYear

End Function
'****************************************************************
'  AddMinutes() - Add Minutes to a Time string
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Parameters:
'	StartTime - The Time to change
'	NumberToAdd - The number of minutes to add
'
'  Returns String incremented by NumberToAdd
'  e.g. NumberToAdd = 1 Then 10:01:10 AM returns 10:02:10 AM
'****************************************************************
Function AddMinutes(StartTime As String, NumberToAdd As Integer) As String

	Dim Colon1      As Integer
	Dim Colon2      As Integer
	Dim TheMinutes  As String
	Dim TheHour     As String
	Dim TheSeconds  As String

	Colon1 = Instr(1,StartTime,":")
	Colon2 = Instr(Colon1+1,StartTime,":")

	TheMinutes = Mid$(StartTime,Colon1+1,(Colon2-1)-Colon1)
	TheMinutes = Trim$(Str(Val(TheMinutes)+NumberToAdd))
	If Len(TheMinutes) = 1 Then
		TheMinutes = "0"+TheMinutes
	End If

	TheHour = Mid$(StartTime,1,Colon1-1)
	If Len(TheHour) = 1 Then
		TheHour = "0"+TheHour
	End If

	TheSeconds = Mid$(StartTime,Colon2+1,Len(StartTime)-Colon2)
	AddMinutes = TheHour+":"+TheMinutes+":"+TheSeconds

End Function
'*****************************************************
'  GetTheYear() - Return Year from Date String
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*****************************************************
Function GetTheYear(TheDate As String) As String

	Dim Slash1  As Integer
	Dim Slash2  As Integer
	Dim TheYear As String

	Slash1 = Instr(1,TheDate,"/")
	Slash2 = Instr(Slash1+1,TheDate,"/")

	TheYear = Mid$(TheDate,Slash2+1,Len(TheDate)-Slash2)
	If Len(TheYear) = 1 Then
		TheYear = "0"+TheYear
	End If

	GetTheYear	= TheYear

End Function
'*****************************************************
'  GetTheMonth() - Return Month from Date String
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*****************************************************
Function GetTheMonth(TheDate As String) As String

	Dim Slash1 As Integer
	Dim Slash2 As Integer
	Dim TheMonth As String

	Slash1 = Instr(1,TheDate,"/")
	Slash2 = Instr(Slash1+1,TheDate,"/")

	TheMonth = Mid$(TheDate,1,Slash1-1)
	If Len(TheMonth) = 1 Then
		TheMonth = "0"+TheMonth
	End If

	GetTheMonth	= TheMonth

End Function
'*****************************************************
'  GetTheDay() - Return Day from Date String
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*****************************************************
Function GetTheDay(TheDate As String) As String

	Dim Slash1 As Integer
	Dim Slash2 As Integer
	Dim TheDay As String

	Slash1 = Instr(1,TheDate,"/")
	Slash2 = Instr(Slash1+1,TheDate,"/")

	TheDay = Mid$(TheDate,Slash1+1,(Slash2-1)-Slash1)
	If Len(TheDay) = 1 Then
		TheDay = "0"+TheDay
	End If

	GetTheDay = TheDay

End Function
'*************************************************************************
' Function FixTheDate(TheDate) As String
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Makes sure date string is in Format MM/DD/YY
'
'*************************************************************************
Function FixTheDate(TheDate As String) As String

	FixTheDate = GetTheMonth(TheDate)+"/"+GetTheDay(TheDate)+"/"+GetTheYear(TheDate)

End Function
'*************************************************************************
' Function GetDaysInMonth(TheDate As String) As String
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'      Year 2000 compliance thanks to Steve Sherwood
'
' Parameters 
'
'	 TheDate - A date string in format MM/DD/YY
'
' Returns
'
'	The number of days in the month of the date string parameter
'	as a string.
'
' Note:
'
'	Calls  GetTheMonth()
'		   GetTheYear()
'
'*************************************************************************
Function GetDaysInMonth(TheDate As String) As String

	Dim TheCurrentMonth As Integer

	' Parse month from date
	TheCurrentMonth = Val(GetTheMonth(TheDate))

	Select Case TheCurrentMonth

		Case 1,3,5,7,8,10,12

		  GetDaysInMonth = "31"

		Case 4,6,9,11

		  GetDaysInMonth = "30"

		Case 2
			' Parse the year
			' Divide the year by 4
			' and check remainder
			' Year 2000 compliance thanks to Steve Sherwood
			If Val(GetTheYear(TheDate)) Mod 4 = 0 _
				And Val(GetTheYear(TheDate)) Mod 1000 <> 0 Then
				GetDaysInMonth = "29"
			Else
				GetDaysInMonth = "28"
			End If

	End Select

End Function
'*************************************************************************
'*************************************************************************
'*************************************************************************
'
'  General Functions
'
'*************************************************************************
'*************************************************************************
'*************************************************************************
'*************************************************************************
' Sub StopScripts(vKeyInfo As Variant)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'   Traps ESCAPE KEY to stop current script
'
'*************************************************************************
Sub StopScripts(vKeyInfo As Variant)
		
	Dim EndProgram As Integer
		
	EndProgram = MsgBox("Escape pressed."+ Chr(10)+"Click OK to end program, Cancel to continue.",1 OR 64 OR 0 OR 4096,"End "+CurPrgName)

	If EndProgram = 1 Then

		LogWrite("0     'Escape' Pressed, Ending "+CurPrgName+".",LogPath,CritErrPath)

		' End program
		StatusBox "Ending "+CurPrgName
		Sleep 2
		LogClose(LogPath,CritErrPath)
		LogWrite("",LogPath,CritErrPath)
		End

	End If

End Sub
'*************************************************************************
' Function SetDefaults(SectionName$,TheIniFile$)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' SectionName$	- The Section [] within the INI file
'
' TheIniFile$	- Path and name of INI file
'
' Retrieves default values from INI file.
'
'*************************************************************************
Function SetDefaults(SectionName$,TheIniFile$) As Integer

	Dim tmpLogDir      As String
	Dim tmpLogPath     As String
	Dim tmpCritErrPath As String

	SetDefaults = True

	' Test parameters
	If SectionName = "" Then
		tmpLogDir		= CurDir+"\"
		tmpLogPath		= tmpLogDir + "ERROR.LOG"
		tmpCritErrPath  = tmpLogDir + "CRITERR.LOG"
		SetDefaults = False
		LogWrite("1     Bad Section Name parameter passed to SetDefaults()!",tmpLogPath,tmpCritErrPath)
		Exit Function
	End If

	' Create temporary defaults
	tmpLogDir		= CurDir+"\"
	tmpLogPath		= TheIniFile
	tmpCritErrPath  = tmpLogDir + "CRITERR.LOG"

	' Retrieve INI file variables
	' ---------------------------
	LogPath = ReadIni(TheIniFile,SectionName,"LogPath")
	' Test value returned
	If LogPath = "NotFnd" Then
		SetDefaults = False
		LogWrite("1     Can't find LogPath in: " + TheIniFile+"!",tmpLogPath,tmpCritErrPath)
		Exit Function
	End If

	CritErrPath = ReadIni(TheIniFile,SectionName,"CritErrPath")
	' Test value returned
	If CritErrPath = "NotFnd" Then
		SetDefaults = False
		LogWrite("1     Can't find CritErrPath in: " + TheIniFile+"!",LogPath,tmpCritErrPath)
		Exit Function
	End If

	SMLogPath = ReadIni(TheIniFile,SectionName,"SMLogPath")
	' Test value returned
	If SMLogPath = "NotFnd" Then
		SetDefaults = False
		LogWrite("1     Can't find SMLogPath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	ExplorerName = ReadIni(TheIniFile,SectionName,"ExplorerName")
	' Test value returned
	If ExplorerName = "NotFnd" Then
		SetDefaults = False
		LogWrite("1     Can't find ExplorerName in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	MTRunExePath = ReadIni(TheIniFile,SectionName,"MTRunExePath")
	' Test value returned
	If MTRunExePath = "NotFnd" Then
		SetDefaults = False
		LogWrite("1     Can't find MTRunExePath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	MTExePath = ReadIni(TheIniFile,SectionName,"MTExePath")
	' Test value returned
	If MTExePath = "NotFnd" Then
		SetDefaults = False
		LogWrite("1     Can't find MTExePath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
' Function SetNDDefaults(SectionName$,TheIniFile$)
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
' SectionName$	- The Section [] within the INI file
'
' TheIniFile$	- Path and name of INI file
'
' Retrieves Network Distribution default values from INI file.
'
'*************************************************************************
Function SetNDDefaults(SectionName$,TheIniFile$) As Integer

	SetNDDefaults = True

	MTRunExePath = ReadIni(TheIniFile,SectionName,"MTRunExePath")
	' Test value returned
	If MTRunExePath = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find MTRunExePath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	MTExePath = ReadIni(TheIniFile,SectionName,"MTExePath")
	' Test value returned
	If MTExePath = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find MTExePath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	TimerIniPath = ReadIni(TheIniFile,SectionName,"TimerIniPath")
	' Test value returned
	If TimerIniPath = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find TimerIniPath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	CopyLogDir = ReadIni(TheIniFile,SectionName,"CopyLogDir")
	' Test value returned
	If CopyLogDir = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find CopyLogDir in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	NetHostPCDPath = ReadIni(TheIniFile,SectionName,"NetHostPCDPath")
	' Test value returned
	If NetHostPCDPath = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find NetHostPCDPath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	ProjectDirPath = ReadIni(TheIniFile,SectionName,"ProjectDirPath")
	' Test value returned
	If ProjectDirPath = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find ProjectDirPath in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	MainComputerName = ReadIni(TheIniFile,SectionName,"MainComputerName")
	' Test value returned
	If MainComputerName = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find MainComputerName in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	' Retrieve Integer value
	intNetDist = ReadIniInt(TheIniFile,SectionName,"intNetDist")

	' Retrieve Integer value
	intUseStationCount = ReadIniInt(TheIniFile,SectionName,"intUseStationCount")

	' Retrieve Integer value
	intHostSleepTime = ReadIniInt(TheIniFile,SectionName,"intHostSleepTime")


	NetHostCfgFile = ReadIni(TheIniFile,SectionName,"NetHostCfgFile")
	' Test value returned
	If NetHostCfgFile = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find NetHostCfgFile in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

	TimeSyncServer = ReadIni(TheIniFile,SectionName,"TimeSyncServer")
	' Test value returned
	If TimeSyncServer = "NotFnd" Then
		SetNDDefaults = False
		LogWrite("1     Can't find TimeSyncServer in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If

End Function
'*************************************************************************
'  CheckCommandLine() Parse command line for INI file
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'
' Returns True/False
'
'*************************************************************************
Function CheckCommandLine() As Integer

    Dim CommandCounter  As Integer
    Dim Dummy           As String

	CheckCommandLine = True

	' Create dummy string from command line
    Dummy = UCase$(Command$)

	' Test Dummy
	If Len(Trim(Dummy))	= 0 Then
        LogWrite("1     Ini file not passed to program: "+CurPrgName+"!",LogPath,CritErrPath)
		CheckCommandLine = False
        Exit Function
	End If

	' Check for parameter separator - comma
    CommandCounter = InStr(1, Dummy, ",")

    If CommandCounter = 0 Then
        TheIniFile = Dummy
        If Not Exists(TheIniFile) Then
            LogWrite("1     Ini file: "+TheIniFile+" does not exist!",LogPath,CritErrPath)
			CheckCommandLine = False
            Exit Function
        End If
    Else
        TheIniFile = Mid$(Dummy, 1, CommandCounter - 1)
        If Not Exists(TheIniFile) Then
            LogWrite("1     Ini file: "+TheIniFile+" does not exist!",LogPath,CritErrPath)
			CheckCommandLine = False
            Exit Function
        End If
	End If

End Function
'*****************************************************
'  TestLogVars() - Test to make sure logfile
'                  Variables are initialized
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*****************************************************
Sub TestLogVars(LogPath$)

	Dim LogDir As String

	' Detail Error Log File path and name
	If Len(LogPath) = 0 Then
		LogPath = Curdir+"\ERROR.LOG"
	End If

	' Critical Error Log File
	If Len(CritErrPath) = 0 Then
		CritErrPath = Curdir+"\CRITERR.LOG"
	End If

	' Status Log File e.g. the suite manager log file
	If Len(SMLogPath) = 0 Then
		SMLogPath = Curdir+"\STATUS.LOG"
	End If

	' Detail Error Log File path only
	If Len(LogDir) = 0 Then
		LogDir = Curdir+"\"
	End If

	' Make sure Logging Directory exists
	If Not Exists(Mid(LogDir,1,Len(LogDir)-1),"+d") Then
		MakeDirs(LogDir)
	End If

End Sub
'*****************************************************
'  MakeDirs() - Create directories from a string
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'  Parameters:
'
'  DirName$  The directory path that you wish to create.
'
'    \MSDEV\PROJECTS\TESTDIR\PROJECT          
'
'	 creates the following directories:
'	 \msdev
'	 \msdev\projects
'	 \msdev\projects\testdir
'	 \msdev\projects\testdir\project
'
'*****************************************************
Sub MakeDirs(DirName As String)

	Dim SlashInString As Integer
	Dim NewDirString  As String
	Dim DirCounter    As Integer
	Dim LoopCounter   As Integer

	' Create Array to store directory names
	Dim DirArray(50) As String 

	NewDirString  = DirName
	SlashInString = Instr(2,NewDirString,"\")

	' Parse elements from DirName
	' and put each element into an array
	Do Until SlashInString = 0
		NewDirString  = ParseDN(NewDirString)
		SlashInString = Instr(2,NewDirString,"\")
		If Len(NewDirString) > 2 Then
			DirCounter = DirCounter + 1
			DirArray(DirCounter) = NewDirString 
		End If
	Loop

	' Create directories from array starting at the end
	If DirCounter > 0 Then
		For LoopCounter = DirCounter To 1 Step -1
			If Not Exists(DirArray(LoopCounter),"+d") Then
				MkDir(DirArray(LoopCounter))
			End If
		Next
	End If

	' Write directories created to Log File
	If DirCounter > 0 Then
		For LoopCounter = DirCounter To 1 Step -1
			If Exists(DirArray(LoopCounter),"+d") Then
				LogWrite("0     Created Directory: "+DirArray(LoopCounter)+".",LogPath,CritErrPath)
			End If
		Next
	End If

End Sub
'********************************************************************
' Function CompareDirFiles(FileDirName1$,FileDirName2$) As Integer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Compares FileDirName1$  - The Source or directory
' To       FileDirName2   - The Destination  directory
'
' Returns True/False
'
'********************************************************************
Function CompareDirFiles(FileDirName1$,FileDirName2$) As Integer 

    Dim buf           As String
    Dim SrcLen        As Integer
    Dim Src           As String
    Dim Dest          As String
    Dim Part          As String
    Dim SrcAttr       As String
	Dim DestAttr      As String
	Dim ExtraPath     As String
    Dim Drv           As String
	Dim Dir           As String
	Dim Fname         As String
	Dim Ext           As String
	Dim SourceCount   As Integer
	Dim DestCount     As Integer
	Dim NoComparErr   As Integer
	Dim NoAttribErr   As Integer
	Dim ReturnInteger As Integer

	CompareDirFiles = True
	NoComparErr     = True
	NoAttribErr     = True

	LogWrite("0     -----------------------------------",LogPath,CritErrPath)
	LogWrite("0     Comparing Directories",LogPath,CritErrPath)
	LogWrite("0     -----------------------------------",LogPath,CritErrPath)
	LogWrite("0     Source Directory:      " + FileDirName1,LogPath,CritErrPath)
	LogWrite("0     Destination Directory: " + FileDirName2,LogPath,CritErrPath)
	my_status("Comparing: "+FileDirName1+" to "+ FileDirName2)

	' Determine length of file path
    SrcLen% = Len(FileDirName1)

    Filelist Clear

	' Create wildcard string
    buf$ = FileDirName1 + "\*.*"

	' Loop through source dir files and compare with destination files
    For Src$ In Filelist

		' Count source files
		SourceCount = SourceCount + 1
		' Parse out file name
        Part$ = Mid$(Src$,SrcLen% + 2)
		' Get attribute of file
        SrcAttr$ = Attrib$(Src$)
        DestAttr$ = ""
		' Create destination file name
        Dest$ = FileDirName2 + ExtraPath$ + Part$

        ' If not a directory
        If InStr(SrcAttr$, "+D") = 0 Then
			' Make sure destination file exists
			If Exists(Dest$) Then
				DestCount = DestCount + 1
				' Compare files
				If Compare(Src$, Dest$) <> 0 Then
					NoComparErr     = False
					CompareDirFiles = False
					DestAttr$       = Attrib$(Dest$)
					LogWrite("1     ERROR Comparing Files!",LogPath,CritErrPath)
					LogWrite("1     File: "+Src$,LogPath,CritErrPath)
					ReturnInteger = GetFileSpecs(Src$)
					LogWrite("1     File: "+Dest$,LogPath,CritErrPath)
					ReturnInteger = GetFileSpecs(Dest$)
					LogWrite("1     Attributes Source: "+SrcAttr$+" Destination: "+DestAttr$+" Compare Okay",LogPath,CritErrPath)
				Else
					DestAttr$ = Attrib$(Dest$)
					If DestAttr = SrcAttr Then 
						'LogWrite("0     Compare of File: " + Src$ + " and " + Dest$ + " Succeeded",LogPath,CritErrPath)
					Else
						LogWrite("1     ERROR Attributes Source: "+SrcAttr$+" Destination: "+DestAttr$+"!",LogPath,CritErrPath)
						NoAttribErr = False
					End If
				End If
			Else
				LogWrite("1     Destination File: " + Dest$ + " doesn't exist!",LogPath,CritErrPath)
				CompareDirFiles = False
				DestAttr$ = ""
			End If
        Else
			' Compare subdirectory
            If  Exists(Dest$, "+d") = 0 Then
				NoComparErr     = False
				CompareDirFiles = False
                DestAttr$       = Attrib$(Dest$)
				LogWrite("1     FAIL Compare of Dir: " + Src$ + " and " + Dest$ + "!",LogPath,CritErrPath)
				LogWrite("0     Attributes Source: "+SrcAttr$+" Destination: "+DestAttr$+" Compare Okay",LogPath,CritErrPath)
	        Else
				DestCount = DestCount + 1
				DestAttr$ = Attrib$(Dest$)
				If DestAttr = SrcAttr Then
					'LogWrite("0     Compare of File: " + Src$ + " and " + Dest$ + " Succeeded",LogPath,CritErrPath)
				Else
					LogWrite("1     FAIL Attributes Source: "+SrcAttr$+" Destination: "+DestAttr$+"!",LogPath,CritErrPath)
					NoAttribErr = False
				End If
            End If
        End If

    Next

	LogWrite("4     ==========================================",LogPath,CritErrPath)
	If SourceCount = DestCount Then
		LogWrite("4     File count OK.",LogPath,CritErrPath)
	Else
		LogWrite("1     File count Failed!",LogPath,CritErrPath)
	End If
	If NoComparErr = True Then
		LogWrite("4     All files Compare OK.",LogPath,CritErrPath)
	Else
		LogWrite("1     Comparison Check Failed!",LogPath,CritErrPath)
	End If
	If NoAttribErr = True Then
		LogWrite("4     All file Attributes OK.",LogPath,CritErrPath)
	Else
		LogWrite("1     Attributes Check Failed!",LogPath,CritErrPath)
	End If

	LogWrite("4     Source File Count:      "+Str(SourceCount),LogPath,CritErrPath)
	LogWrite("4     Destination File Count: "+Str(DestCount),LogPath,CritErrPath)
	ReturnInteger = GetDirSize(FileDirName1+"\*.*")
	ReturnInteger = GetDirSize(FileDirName2+"\*.*")
	LogWrite("4     ==========================================",LogPath,CritErrPath)
	my_status("Source File Count:           "+Str(SourceCount))
	my_status("Destination File Count:      "+Str(SourceCount))

End Function
'*************************************************************************
' Function GetFileSpecs(FileName$) As Integer
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Writes File Creation Size, Date and Time
' to logfile
'
' Returns True/False
'
'*************************************************************************
Function GetFileSpecs(FileName$) As Integer

	Dim ReturnLong   As Long
	Dim ReturnString As String
	Dim WriteString  As String

	GetFileSpecs = True

	' Get Size of file
	ReturnLong = GetMyFileSize(FileName$)
	If ReturnLong <= 0	Then
		GetFileSpecs = False
	End If
	WriteString  = "     Size: "+Trim$(Str$(ReturnLong))
	' Get Date of file
	ReturnString = GetFileDate(FileName$)
	If Len(Trim$(ReturnString)) <= 0 Then
		GetFileSpecs = False
	End If
	WriteString  = WriteString + " Date: "+ReturnString
	' Get Time of file
	ReturnString = GetMyFileTime(FileName$)
	If Len(Trim$(ReturnString)) <= 0 Then
		GetFileSpecs = False
	End If
	WriteString  = WriteString + " Time: "+ReturnString
	' Write results to logfile
	LogWrite("0     "+WriteString,LogPath,CritErrPath)

End Function
'*************************************************************************
' Function GetDirSize (DirName$) As Long
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Writes the file size of directory
' by adding up all the filesizes
'
' Syntax:
' ReturnInteger = GetDirSize (C:\Temp\*.*)
'
'*************************************************************************
Function GetDirSize (DirName$) As Integer

	Dim FileName      As String
	Dim FileCounter   As Integer
	Dim ReturnDouble  As Double
	Dim ReturnString  As String

	GetDirSize = True

	' Get all files including hidden, system and read only
	FileList Clear
	FileCounter = 0
	FileList Add DirName,"+h"
	FileList Add DirName,"+r"
	FileList Add DirName,"+s"
	FileList Add DirName

	' Loop through files and get total filesize
	For FileName In FileList
		FileCounter = FileCounter+1
		ReturnDouble = ReturnDouble+ GetMyFileSize (FileName)
	Next
	FileList Clear

 	' Convert FileSize to string
	Select Case ReturnDouble
		Case Is > 1024000
		 	ReturnString = Trim$(Str$(ReturnDouble/1024000)) +" MB"
		Case Is > 1024
		 	ReturnString = Trim$(Str$(ReturnDouble/1024)) +" KB"
		Case Is > 0
		 	ReturnString = Trim$(Str$(ReturnDouble)) +" Bytes"
		Case Else
		 	ReturnString = Trim$(Str$(ReturnDouble))
	End Select

	' Write results to logfile
	LogWrite("4     Directory: "+DirName+" Size: "+ReturnString,LogPath,CritErrPath)

End Function
'*****************************************************
'  LogOpen() - Create and initialize Error log
'              Open file and write one record
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*****************************************************
Sub LogOpen(LogPath$,CritErrPath$)

	Dim LogFH      As Integer

	' Notify user
	StatusBox CurPrgName+" Started. "+RevNumber

	' Clear Viewport
 	Viewport Clear

	' Make sure log file variables are initialized
	TestLogVars(LogPath)

	' Write opening message to Error Log
	LogFH=FREEFILE            ' get next available file handle
    Open LogPath For Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  0   "+CurPrgName+" STARTED. "+RevNumber
    Close #LogFH

	' Write opening message to Critical Error Log
	LogFH=FREEFILE            ' get next available file handle
    Open CritErrPath For Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  0   "+CurPrgName+" STARTED. "+RevNumber
    Close #LogFH

End Sub
'**********************************************************
'  LogClose() - Write last record and close Error Log file
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'**********************************************************
Sub LogClose(LogPath$,CritErrPath$)
    
	Dim LogFH      As Integer

	' Make sure log file variables are initialized
	TestLogVars(LogPath)

	' Write closing message to Error Log
    LogFH=FREEFILE            ' get next available file handle
    Open LogPath For Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  0   "+CurPrgName+" ENDED."
    Close #LogFH

	' Write closing message to Critical Error Log
	LogFH=FREEFILE            ' get next available file handle
    Open CritErrPath For Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  0   "+CurPrgName+" ENDED."
    Close #LogFH

	' Notify user
	StatusBox CurPrgName+" Ended."
	Sleep 2
	StatusBox Close

End Sub
'***********************************************************
'  LogEnter() - Write Entry to the Error Log - no date/time
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'***********************************************************
Sub LogEnter(Message$,LogPath$,CritErrPath$)

	Dim LogFH      As Integer
	Dim LogMessage As String

	' Make sure log file variables are initialized
	TestLogVars(LogPath)

	' get next available file handle
    LogFH=FREEFILE 

	' open file and append message
    Open LogPath for Append As #LogFH
    Print #LogFH, + "   "+ Message
    Close #LogFH

	' Write message to Suite Manager Log (Status Log)
	LogMessage = Mid(Message,2,Len(Message))

	Select Case	Left$(Message,1)

		Case ""

		Case " "
			Log #0,+LogMessage
		Case "0"
			Log #0,+LogMessage
		Case "1"
			Log #1,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH
		Case "2"
			Log #2,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH
		Case "3"
			Log #3,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH
		Case Else
			Log #3,+LogMessage
	End Select

End Sub
'*************************************************************
'  LogWrite() - Write Entry to the Error Log - with date/time
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'
'  Message$     - What is the message you want to write.
'                 The first character of the message is the
'                 Severity level of the message.
'  LogPath$     - Error.log path and name.
'  CritErrPath$ - CritErr.log path and name.
'
'*************************************************************
Sub LogWrite(Message$,LogPath$,CritErrPath$)
 
	Dim LogFH      As Integer
 	Dim LogMessage As String

	' Make sure log file variables are initialized
	TestLogVars(LogPath)

	' get next available file handle
	LogFH=FREEFILE

	' open file and append message
    Open LogPath for Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
    Close #LogFH

	' Write message to Suite Manager Log (Status Log)
	LogMessage = Mid(Message,2,Len(Message))

	Select Case	Left$(Message,1)

		Case ""

	'	Case " "
	'		Log #0,+LogMessage
		Case "0"
			Log #0,+LogMessage
		Case "1"
			Log #1,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH
		Case "2"
			Log #2,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH
		Case "3"
			Log #3,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH
		Case Else
			Log #4,+LogMessage
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Date$ + " " + Time$ + "  "+ Message
			Close #LogFH

	End Select

End Sub
'*************************************************************
'  LogOnly() - Write Entry to the Error Log - no date/time
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*************************************************************
Sub LogOnly(Message$,LogPath$,CritErrPath$)
 
	Dim LogFH      As Integer
 	Dim LogMessage As String
	
	' Make sure log file variables are initialized
	TestLogVars(LogPath)

	' get next available file handle
	LogFH=FREEFILE
    
	' open file and append message
	Open LogPath for Append As #LogFH
    Print #LogFH, Message
    Close #LogFH

	Select Case	Left$(Message,1)
		Case "1"
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Message
			Close #LogFH
		Case "2"
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Message
			Close #LogFH
		Case "3"
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Message
			Close #LogFH
		Case Else
			' Write Error message to Critical Error Log
			LogFH=FREEFILE 
			Open CritErrPath for Append As #LogFH
		    Print #LogFH, Message
			Close #LogFH
	End Select


End Sub
'*****************************************************
' LogError() - Write Fatal Error to the Error Log
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
'	1) Error message to write
'
'	2) ApplicationToClose
'
'		0 None
'		1 close routine for application 1
'		2 close routine for application 2
'
'*****************************************************
Sub LogError(Message$,ApplicationToClose%,LogPath$,CritErrPath$)

	Dim LogFH     As Integer
    Dim lcVisible As String
    Dim lcEnabled As String
	
	Dim lpInfo As Info
    WGetInfo (WGetActWnd(0), lpInfo)

	If lpInfo.Visible = 1 Then
		lcVisible = "TRUE"
	Else
		lcVisible = "FALSE"
	End If

	If lpInfo.Enabled = 1 Then
		lcEnabled = "TRUE"
	Else
		lcEnabled = "FALSE"
	End If

	' Set SystemError variable to True
	SystemError = True

	' Make sure log file variables are initialized
	TestLogVars(LogPath)

    LogFH=FREEFILE      ' get next available file handle
    Open LogPath For Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  1 ********************************************************"
    Print #LogFH, Date$ + " " + Time$ + "  1 ***               FATAL ERROR!"
    Print #LogFH, Date$ + " " + Time$ + "  1 *** " + Curdir$+"\"+CurPrgName+" Could Not Continue!"
    Print #LogFH, Date$ + " " + Time$ + "  1 *** " + Message
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Current Window Information Follows: "
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window Handle:         "lpInfo.hWnd
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window ID:             "lpInfo.Id
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Visible:               "+lcVisible
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Enabled:               "+lcEnabled
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window Caption:        "+CompressString(lpInfo.Text)
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window Classname:      "+lpInfo.Class
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Name of Owning Module: "+lpInfo.Module
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Associated Label:      "+lpInfo.Label
    Print #LogFH, Date$ + " " + Time$ + "  1 ********************************************************"
    Close #LogFH

    LogFH=FREEFILE      ' get next available file handle
    Open CritErrPath For Append As #LogFH
    Print #LogFH, Date$ + " " + Time$ + "  1 ********************************************************"
    Print #LogFH, Date$ + " " + Time$ + "  1 ***               FATAL ERROR!"
    Print #LogFH, Date$ + " " + Time$ + "  1 *** " + Curdir$+"\"+CurPrgName+" Could Not Continue!"
    Print #LogFH, Date$ + " " + Time$ + "  1 *** " + Message
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Current Window Information Follows: "
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window Handle:         "lpInfo.hWnd
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window ID:             "lpInfo.Id
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Visible:               "+lcVisible
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Enabled:               "+lcEnabled
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window Caption:        "+CompressString(lpInfo.Text)
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Window Classname:      "+lpInfo.Class
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Name of Owning Module: "+lpInfo.Module
    Print #LogFH, Date$ + " " + Time$ + "  1 *** Associated Label:      "+lpInfo.Label
    Print #LogFH, Date$ + " " + Time$ + "  1 ********************************************************"
    Close #LogFH

	' Display Error notice
	StatusBox "Fatal Error! "+Message,0,0,0,0,True,True,"MS Sans Serif", 12, 900
	Sleep 3
	StatusBox Close

	' Write message to Suite Manager Log (Status Log)
	Log #1,+Space(5)+Message
	Sleep 2
	
	' Close desired application
	Select Case	ApplicationToClose

		Case 0
			' Do nothing
		Case 1
			'Run Close application 1 procedure
		Case 2
			'Run Close application 2 procedure
			' Do nothing

	End Select

	' End Program
	LogClose(LogPath,CritErrPath)
	LogWrite("",LogPath,CritErrPath)
	End

End Sub
'************************************************************
' Sub CloseOnSystemError - Reports and closes on System Error
'                          Called by Error Handler Routine.
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
' Parameters:
'
'	1) ApplicationToClose
'
'		0 None
'		1 close routine for application 1
'		2 close routine for application 2
'
'************************************************************
Sub CloseOnSystemError(ApplicationToClose&,LogPath$,CritErrPath$)

    Dim ErrFileNum    As Integer
	Dim ReturnInteger As Integer
	Dim DialogInteger As Integer

    ' Get next available handle for Error Log
	ErrFileNum = FREEFILE

	' Append to Error Log
    Open LogPath For Append As #ErrFileNum

	' If error exists write to Error Log
    If Err > 0 Then

        BEEP
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  ********************************************************"
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** MS-TEST Reports the following ERROR:"
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** "+Error$(Err)+" On Line: "Trim$(Str$(Erl(0)))
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** In Program: "+Erf(0)
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** This may cause unexpected results."
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** Attempting to end "+CurPrgName+"!"
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  ********************************************************"

    End If

	' Close Error Log
    Close #ErrFileNum

	' Append to Critical Error Log
    Open CritErrPath For Append As #ErrFileNum

	' If error exists write to Critical Error Log
    If Err > 0 Then

        BEEP
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  ********************************************************"
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** MS-TEST Reports the following ERROR:"
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** "+Error$(Err)+" On Line: "Trim$(Str$(Erl(0)))
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** In Program: "+Erf(0)
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** This may cause unexpected results."
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  *** Attempting to end "+CurPrgName+"!"
        Print #ErrFileNum, Date$ + " " + Time$ + " "+ Str$(Err)+"  ********************************************************"

    End If

	' Close Critical Error Log
    Close #ErrFileNum

	' Write error notice to Status Log
	Log #1, "     SYSTEM ERROR "+ Trim$(Str$(Err))+": Check "+LogPath+"!"
	StatusBox "SYSTEM ERROR "+Error$(Err)+"!"
	Sleep 3
	StatusBox Close

	' Close desired application
	Select Case	ApplicationToClose
		Case 0
			' Do nothing
		Case 1
			' Run Close application 1 procedure
			' Do nothing
		Case 2
			' Run Close application 2 procedure
			' Do nothing
	End Select

	' End program
    LogClose(LogPath,CritErrPath)
	LogWrite("",LogPath,CritErrPath)
    End

	' Instead of ending the program, resume
	' Resume Next

End Sub
'***************************************************************
' CompressString - Removes Tabs and extra spaces from a string
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'***************************************************************
Function CompressString(TheString As String) As String

    Dim Spacer As Integer
	Dim NewString As String

	' Remove Tabs
	Spacer = 1
	Do
		Spacer = Instr(Spacer,TheString,Chr(9))
		If Spacer <> 0 Then
			TheString = Mid$(TheString,1,Spacer-1)+" "+Mid$(TheString,Spacer+1)
			Spacer = Spacer+1
		End If
	Loop While Spacer <> 0

	' Remove Carriage Returns
	Spacer = 1
	Do
		Spacer = Instr(Spacer,TheString,Chr(13))
		If Spacer <> 0 Then
			TheString = Mid$(TheString,1,Spacer-1)+" "+Mid$(TheString,Spacer+1)
			Spacer = Spacer+1
		End If
	Loop While Spacer <> 0

	' Remove Line Feeds
	Spacer = 1
	Do
		Spacer = Instr(Spacer,TheString,Chr(10))
		If Spacer <> 0 Then
			TheString = Mid$(TheString,1,Spacer-1)+" "+Mid$(TheString,Spacer+1)
			Spacer = Spacer+1
		End If
	Loop While Spacer <> 0

	' Remove Spaces
	Spacer = 1
	Do
		Spacer = Instr(1,TheString,Space(2))
		If Spacer <> 0 Then
			TheString = Mid$(TheString,1,Spacer-1)+Mid$(TheString,Spacer+1)
			Spacer = Spacer+1
		End If
	Loop While Spacer <> 0

	CompressString = TheString

End Function
'***************************************************************

'  ParseDN() - Parse Directory name from explicit filename
'              c:\dirname\dirname\data.dat = c:\dirname\dirname
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'***************************************************************
Function ParseDN(DName As String) As String

    Dim backslash As Integer
    Dim LdName    As String

    backslash=1                                ' init backslash found var

    Do While backslash <> 0                    ' search until last '\'
        backslash=Instr(backslash,DName,"\")   ' find backslash
        If backslash <> 0 Then                 ' found Then
            LdName=Mid$(DName,1,backslash-1)   ' substr remaining String
            backslash=backslash+1              ' increment past '\'
        End If
    Loop                                       ' do it again
    ParseDN=LdName                             ' return just the directory name

End Function
'*****************************************************
'  ParseFN() - Parse Filename.ext from explicit filename
'              c:\data\data.dat = data.dat
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'*****************************************************
Function ParseFN(FName As string) As string

    Dim slash As Integer
    Dim LoadName As String

    slash=1                                ' init slash found var
    Do While slash <> 0                    ' search until last '\'
        slash=Instr(slash,FName,"\")       ' find slash
        If slash <> 0 Then                 ' found then
            LoadName=Mid$(FName,slash+1)   ' substr remaining string
            slash=slash+1                  ' increment past '\'
        End If
    Loop                                   ' do it again
    ParseFN=LoadName                       ' return just the filename

End Function
'************************************************************
'  UAEErrorTrap() - Trap and Report Unhandled Exception Errors
'
'      Richard J. Wartner
'      7610 North Via De Manana
'      Scottsdale, Arizona 85258
'      RWartner@aol.com
'
'************************************************************
Sub UAEErrorTrap(UAEErrMsg As Variant)

	' Write to error logs
	LogWrite("1 **************************************",LogPath,CritErrPath)
	LogWrite("1   Unhandled Exception Error Message Follows:",LogPath,CritErrPath)
	LogWrite("1   "+UAEErrMsg,LogPath,CritErrPath)
	LogWrite("1   Ending Program!",LogPath,CritErrPath)
	LogWrite("1 **************************************",LogPath,CritErrPath)

	StatusBox "Unhandled Exception Error: "+ UAEErrMsg,0,0,0,0,True,True,"MS Sans Serif", 12, 900
	Sleep 3
	StatusBox Close

	' Write to status log
	Log #1, "*****************************************"
	Log #1, "Unhandled Exception Error Message Follows:"
	Log #1,  UAEErrMsg
	Log #1, "*****************************************"

	LogClose(LogPath,CritErrPath)
	LogWrite("",LogPath,CritErrPath)
	End

End Sub
'*****************************************************
' EOF End Of GenUtil.INC
'*****************************************************
'$ENDIF _GenUtil_INCLUDED
