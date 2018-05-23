' File name: EudoraIniFunctions.inc
' Project:  WnObjPrg

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Function Prototypes                                                   '
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Declare Function GetEudora(SectionName$,TheIniFile$) As Integer
Declare Function GetPersonalityName(SectionName$,TheIniFile$) As String
Declare Function GetPersonalityEmail(SectionName$,TheIniFile$) As String
Declare Function GetPersonalityServer(SectionName$,TheIniFile$) As String
Declare Function GetNickName(SectionName$,TheIniFile$, NickNumber%) As String
Declare Function GetNickAdr(SectionName$,TheIniFile$, AdrNumber%) As String

'*************************************************************************
' Function GetEudora
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets Eudora Path loaded into Global Variable EudoraName
'
' Returns True/False
'
'*************************************************************************
Function GetEudora(SectionName$,TheIniFile$) As Integer

	Dim tmpLogDir      As String
	Dim tmpLogPath     As String
	Dim tmpCritErrPath As String

	GetEudora = True


	' Test parameters
	If SectionName = "" Then
		tmpLogDir		= CurDir+"\"
		tmpLogPath		= tmpLogDir + "ERROR.LOG"
		tmpCritErrPath  = tmpLogDir + "CRITERR.LOG"
		GetEudora = False
		LogWrite("1     Bad Section Name parameter passed to GetEudora()!",tmpLogPath,tmpCritErrPath)
		Exit Function
	End If

	' Create temporary defaults
	tmpLogDir		= CurDir+"\"
	tmpLogPath		= TheIniFile
	tmpCritErrPath  = tmpLogDir + "CRITERR.LOG"


	EudoraName = ReadIni(TheIniFile,SectionName,"EudoraName")
	' Test value returned
	If EudoraName = "NotFnd" Then
		GetEudora = False
		LogWrite("1     Can't find EudoraName in: " + TheIniFile+"!",LogPath,CritErrPath)
		Exit Function
	End If
End Function
'*************************************************************************
' Function GetPersonalityName
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets Personality name
'
' Returns String with name
'
'*************************************************************************
Function GetPersonalityName(SectionName$,TheIniFile$) As String
	GetPersonalityName = NULL
	
	GetPersonalityName = ReadIni(TheIniFile,SectionName,"DomName")

End Function

'*************************************************************************
' Function GetPersonalityEmail
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets Personality Email Address
'
' Returns String with Email Address
'
'*************************************************************************
Function GetPersonalityEmail(SectionName$,TheIniFile$) As String
	GetPersonalityEmail = NULL
	
	GetPersonalityEmail = ReadIni(TheIniFile,SectionName,"DomEmail")

End Function
'*************************************************************************
' Function GetPersonalityServer
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets Personality SMTP/POP server
'
' Returns String with SMTP/POP server
'
'*************************************************************************
Function GetPersonalityServer(SectionName$,TheIniFile$) As String
	GetPersonalityServer = NULL
	
	GetPersonalityServer = ReadIni(TheIniFile,SectionName,"DomServer")

End Function

'*************************************************************************
' Function GetNickName
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets Nickname 
'
' Returns String with Nickname
'
'*************************************************************************
Function GetNickName(SectionName$,TheIniFile$, NickNumber%) As String
	GetNickName = NULL
	Dim tmp as String
	tmp = "Nickname" + LTRIM$(STR$ ( NickNumber ), " ")

	GetNickName = ReadIni(TheIniFile,SectionName,tmp)

End Function
'*************************************************************************
' Function GetNickAdr
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets email address(s) for nickname
'
' Returns String with email address, called after GetNickname usually
'
'*************************************************************************
Function GetNickAdr(SectionName$,TheIniFile$, AdrNumber%) As String
	GetNickAdr = NULL
	Dim tmp as String
	Dim cSpace as Integer

	tmp = "NickAdr" + LTRIM$(STR$ ( AdrNumber ), " ")
'	cSpace = InStr(tmp , " ")
'	If( cSpace ) <> 0
'		tmp = left$(tmp, cSpace)
	GetNickAdr = ReadIni(TheIniFile,SectionName,tmp)

End Function
'*************************************************************************
' Function GetGifFile
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Gets email address(s) for nickname
'
' Returns String with email address, called after GetNickname usually
'
'*************************************************************************
Function GetGifFile(SectionName$,TheIniFile$) As String
	GetGifFile = NULL
	
	GetGifFile = ReadIni(TheIniFile,SectionName,"GIFFile")

End Function
