' File name: EudoraResDependant.inc
' Project:  WnObjPrg
'============================================================
' Resolution Checkers Constants (HIWORDxLOWORD)
'============================================================
Const DoubleVGA = 83887360	' 1280x1024
Const HiSVGA = 75498624		' 1152x864
Const SuperVGA = 67109888	' 1024x768
Const HiVGA = 52429600		' 800x600
Const VGA = 41943680		' 640x480


Declare Function GetResolutionInLong() As Long

'============================================================
' Clicking Buttons Functions Depending on resolution
'============================================================
Declare Function EnterMessageBodyRes() As Integer
Declare Function TextSizeRes(Size$) As Integer
Declare Function BoldRes() As Integer
Declare Function ItalicRes() As Integer
Declare Function UnderlineRes() As Integer
Declare Function ChangeColorRes(Color$, SaveLastColor%) As Integer
Declare Function LastTextColorRes() As Integer
Declare Function TypeWriterRes() As Integer
Declare Function LeftTextRes() As Integer
Declare Function CenterTextRes() As Integer
Declare Function RightTextRes() As Integer
Declare Function IndentInRes() As Integer
Declare Function IndentOutRes() As Integer
Declare Function BulletListRes() As Integer
Declare Function InsertHyperlinkRes(URL$) As Integer
Declare Function ClearFormattingRes() As Integer
Declare Function ToggleQuotePrintable() As Integer
Declare Function ToggleTextAsAttachment() As Integer
Declare Function ToggleWordWrap() As Integer
Declare Function ToggleTabsinBody() As Integer
Declare Function ToggleKeepCopies() As Integer
Declare Function ToggleReturnReceipt() As Integer
Declare Function ToggleInvisibleChars() As Integer

'*************************************************************************
' Function GetResolutionInLong
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Parameters:
'
' Returns a long with resolution
' HIWORD - Horizontal
' LOWORD - Vertical. 
'
'
'*************************************************************************
Function GetResolutionInLong() As Long
  Dim SM_CXSCREEN as Integer
  Dim SM_CYSCREEN as Integer

  GetResolutionInLong = MKLNG( CINT(GetSystemMetrics (SM_CYSCREEN)),CINT(GetSystemMetrics (SM_CXSCREEN)) )
End Function

'*************************************************************************
' Function EnterMessageBodyRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Enters a Message Body with a click. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function EnterMessageBodyRes() As Integer
	EnterMessageBodyRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		EnterMessageBodyRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		EnterMessageBodyRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		EnterMessageBodyRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 602, 463, Left}"
		LogWrite("0     Entered Text body",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		EnterMessageBodyRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 744, 573, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Entered Text body",LogPath,CritErrPath)

	end select


End Function
'*************************************************************************
' Function TextSizeRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Text Size button. Checks against Resolutions
'
' Parameters:
'
' Size$ - Size to Choose, see below
'
' Returns True/False Integer
'
'*************************************************************************
Function TextSizeRes(Size$) As Integer
	TextSizeRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		TextSizeRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		TextSizeRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		TextSizeRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 320, 100, Left}"
		LogWrite("0     Text Size Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		TextSizeRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 318, 98, Left}"
		LogWrite("0     Text Size Button Clicked",LogPath,CritErrPath)
	end select

	If Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_V_SMALL) Then
		WMenuSelect("@1")	
	ElseIf Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_SMALL) Then
		WMenuSelect("@2")
	ElseIf Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_MEDIUM) Then
		WMenuSelect("@3")
	ElseIf Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_LARGE) Then
		WMenuSelect("@4")
	ElseIf Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_LARGER) Then
		WMenuSelect("@5")
	ElseIf Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_VERY_LARGE) Then
		WMenuSelect("@6")
	ElseIf Size = RES$(IDS_MENU_EDIT_TEXT_SIZE_HUMONGOUS) Then
		WMenuSelect("@7")
	End If

	LogWrite("0     Selected Text Size: "+Size+"!",LogPath,CritErrPath)
	

End Function


'*************************************************************************
' Function BoldRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Bold button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function BoldRes() As Integer
	BoldRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		BoldRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		BoldRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		BoldRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 355, 100, Left}"
		LogWrite("0     Bold Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		BoldRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 353, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Bold Button Clicked",LogPath,CritErrPath)

	end select

End Function
'*************************************************************************
' Function ItalicRes

'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Italic button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ItalicRes() As Integer
	ItalicRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ItalicRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ItalicRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ItalicRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 380, 100, Left}"
		LogWrite("0     Italic Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ItalicRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 378, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Italic Button Clicked",LogPath,CritErrPath)

	end select

End Function
'*************************************************************************
' Function UnderlineRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Underline button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function UnderlineRes() As Integer
	UnderlineRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		UnderlineRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		UnderlineRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		UnderlineRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 405, 100, Left}"
		LogWrite("0     Underline Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		UnderlineRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 403, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Underline Button Clicked",LogPath,CritErrPath)

	end select

End Function
'*************************************************************************
' Function ChangeColorRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Color button. Checks against Resolutions
'
' Parameters:
'
' Color$ - Color to Choose, see below
' SaveLastColor% - 0 if not, 1 if so;;; Not working yet, will figure out later
'
' Returns True/False Integer
'
'*************************************************************************
Function ChangeColorRes(Color$, SaveLastColor%) As Integer
	ChangeColorRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long


	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ChangeColorRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ChangeColorRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ChangeColorRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 428, 98, Left}"
		LogWrite("0     Color Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ChangeColorRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 428, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Color Button Clicked",LogPath,CritErrPath)

	end select


	If Color  = RES$(IDS_COLOR_BLACK) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@1")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		Sleep 1
		WMenuSelect("@1")	
		Sleep 1
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_MAROON) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@2")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@2")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_GREEN) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@3")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@3")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_OLIVE) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@4")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@4")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_NAVY) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@5")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@5")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_PURPLE) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@6")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@6")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_TEAL) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@7")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@7")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_GREY) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@8")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@8")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_GRAY) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@8")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@8")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_SILVER) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@9")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@9")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_RED) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@10")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@10")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_LIME) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@11")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@11")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_YELLOW) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@12")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@12")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_BLUE) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@13")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@13")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_FUCHSIA) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@14")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@14")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_AQUA) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@15")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@15")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_WHITE) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@16")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@16")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	ElseIf Color  = RES$(IDS_COLOR_AUTOMATIC) Then
		If SaveLastColor = 0 Then
		WMenuSelect("@17")	
		Else 
		keybd_event(VK_SHIFT,0, 0, 0)
		WMenuSelect("@17")	
		keybd_event(VK_SHIFT,0, KEYEVENTF_KEYUP, 0)
		EndIf
	End If

	LogWrite("0     Selected Text Color: "+Color+"!",LogPath,CritErrPath)

End Function
'*************************************************************************
' Function LastTextColorRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Last Text Color [Irwin's] button
' Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function LastTextColorRes() As Integer
	LastTextColorRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		LastTextColorRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		LastTextColorRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		LastTextColorRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 453, 98, Left}"
		LogWrite("0     Last Color Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		LastTextColorRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 453, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Last Color Button Clicked",LogPath,CritErrPath)

	end select
End Function


'*************************************************************************
' Function TypeWriterRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Typewriter print button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function TypeWriterRes() As Integer
	TypeWriterRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		TypeWriterRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		TypeWriterRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		TypeWriterRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 478, 98, Left}"
		LogWrite("0     TypeWriter Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		TypeWriterRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 478, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     TypeWriter Button Clicked",LogPath,CritErrPath)

	end select

End Function
'*************************************************************************
' Function LeftTextRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Left Text button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function LeftTextRes () As Integer
	LeftTextRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		LeftTextRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		LeftTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		LeftTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 513, 98, Left}"
		LogWrite("0     Left Align Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		LeftTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 513, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Left Align Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function CenterTextRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Center Text button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function CenterTextRes() As Integer
	CenterTextRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		CenterTextRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		CenterTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		CenterTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 540, 100, Left}"
		LogWrite("0     Center Align Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		CenterTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 540, 100, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Center Align Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function RightTextRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Right Text button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function RightTextRes() As Integer
	RightTextRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		RightTextRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		RightTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		RightTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 564, 98, Left}"
		LogWrite("0     Right Align Button clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		RightTextRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 564, 98, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Right Align Button clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function IndentInRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Indent Inbutton. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function IndentInRes() As Integer
	IndentInRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		IndentInRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		IndentInRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		IndentInRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 598, 99, Left}"
		LogWrite("0     Indent In Button clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		IndentInRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 598, 99, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Indent In Button clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function IndentOutRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Indent Out button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function IndentOutRes() As Integer
	IndentOutRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		IndentOutRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		IndentOutRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		IndentOutRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 621, 99, Left}"
		LogWrite("0     Indent Out Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		IndentOutRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 621, 99, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Indent Out Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function BulletListRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Bulleted list button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function BulletListRes() As Integer
	BulletListRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		BulletListRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		BulletListRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		BulletListRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 655, 99, Left}"
		LogWrite("0     Bulleted List Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		BulletListRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 655, 99, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Bulleted List Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function InsertHyperlinkRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Insert HyperLink button, Text Must be highlighted.
' Checks against Resolutions
'
' Parameters:
'
' URL$ - URL to Link, see below
'
' Returns True/False Integer
'
'*************************************************************************
Function InsertHyperlinkRes(URL$) As Integer
	InsertHyperlinkRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		InsertHyperlinkRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		InsertHyperlinkRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		InsertHyperlinkRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 680, 99, Left}"
		LogWrite("0     Insert Hyperlink Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		InsertHyperlinkRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 680, 99, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Insert Hyperlink Button Clicked",LogPath,CritErrPath)

	end select

	DialogWindow = WFndWnd(RES$(IDS_CAPTION_HYPERLINK), FW_DIALOG)
	if DialogWindow = 0 Then
		InsertHyperlinkRes = False
		LogWrite("1     Could not Find Link Dialog, Text must be highlighted",LogPath,CritErrPath)
		Exit Function
	End If
	
	DialogWindow = EnterEditBox("Edit",1,"",URL)
	If DialogWindow = 0 Then
		InsertHyperlinkRes = False
		LogWrite("1     Could not Enter URL in Edit",LogPath,CritErrPath)
		Exit Function
	End If
	
	ClickButton("Button", 1, RES$(IDS_BUTTON_OK))
	Sleep 1
	Play "{RIGHT}"
	LogWrite("0     Inserted Hyperlink: "+URL+"",LogPath,CritErrPath)

End Function
'*************************************************************************
' Function ClearFormattingRes
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Clear Formatting button, Text Must be highlighted 
' and formatted.
' Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ClearFormattingRes() As Integer
	ClearFormattingRes = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ClearFormattingRes = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ClearFormattingRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ClearFormattingRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 705, 99, Left}"
		LogWrite("0     Clear Formatting Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ClearFormattingRes = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 705, 99, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Clear Formatting Button Clicked",LogPath,CritErrPath)

	end select
	
	Sleep 1
	Play "{RIGHT}"

End Function
'*************************************************************************
' Function ToggleQuotePrintable
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Quoted Printable button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleQuotePrintable() As Integer
	ToggleQuotePrintable = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleQuotePrintable = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleQuotePrintable = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleQuotePrintable = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 392, 75, Left}"
		LogWrite("0     Quoted Printable Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleQuotePrintable = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 392, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Quoted Printable Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function ToggleTextAsAttachment
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Text as Attachment button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleTextAsAttachment() As Integer
	ToggleTextAsAttachment = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleTextAsAttachment = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleTextAsAttachment = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleTextAsAttachment = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 417, 75, Left}"
		LogWrite("0     Text as attachment Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleTextAsAttachment = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 417, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Text as attachment Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function ToggleWordWrap
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Word Wrap button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleWordWrap() As Integer
	ToggleWordWrap = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleWordWrap = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleWordWrap = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleWordWrap = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 442, 75, Left}"
		LogWrite("0     Word Wrap Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleWordWrap = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 442, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Word Wrap Button Clicked",LogPath,CritErrPath)

	end select

End Function


'*************************************************************************
' Function ToggleTabsinBody
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Tabs in Body button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleTabsinBody() As Integer
	ToggleTabsinBody = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleTabsinBody = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleTabsinBody = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleTabsinBody = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 467, 75, Left}"
		LogWrite("0     Tabs in Body Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleTabsinBody = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 467, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Tabs in Body Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function ToggleKeepCopies
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Keep Copies button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleKeepCopies() As Integer
	ToggleKeepCopies = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleKeepCopies = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleKeepCopies = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleKeepCopies = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 492, 75, Left}"
		LogWrite("0     Keep Copies Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleKeepCopies = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 492, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Keep Copies Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function ToggleReturnReceipt
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Keep Copies button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleReturnReceipt() As Integer
	ToggleReturnReceipt = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleReturnReceipt = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleReturnReceipt = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleReturnReceipt = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 517, 75, Left}"
		LogWrite("0     Return Receipt Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleReturnReceipt = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 517, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Return Receipt Button Clicked",LogPath,CritErrPath)

	end select

End Function

'*************************************************************************
' Function ToggleInvisibleChars
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Clicks the Invisible Chars button. Checks against Resolutions
'
' Parameters:
'
' Returns True/False Integer
'
'*************************************************************************
Function ToggleInvisibleChars() As Integer
	ToggleInvisibleChars = True
	Dim UtilReturnInteger as Integer
	Dim screenres as Long
	Dim EudoraHandle as Long



	screenres = GetResolutionInLong() 
    MainAppWindow = WFndWndC("Eudora", "EudoraMainWindow", FW_PART Or FW_FOCUS, (FindWindowTimeout))

	If screenres = NULL Then
		ToggleInvisibleChars = False
		LogWrite("1     Could not get the resolution!",LogPath,CritErrPath)
		Exit Function		
	End If

	select case screenres

	case VGA ' 640x480
		ToggleInvisibleChars = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case HiVGA ' 800x600
		ToggleInvisibleChars = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case SuperVGA ' 1024x768
		Play "{Click 548, 75, Left}"
		LogWrite("0     Invisible Chars Button Clicked",LogPath,CritErrPath)
	case HiSVGA ' 1152x864
		ToggleInvisibleChars = False
		LogWrite("1     This Resolution is not supported yet",LogPath,CritErrPath)
		Exit Function
	case DoubleVGA ' 1280x1024
		Play "{Click 552, 75, Left}"
		'wClkWnd(MainAppWindow,744, 573, VK_LBUTTON)
		LogWrite("0     Invisible Chars Button Clicked",LogPath,CritErrPath)

	end select

End Function

