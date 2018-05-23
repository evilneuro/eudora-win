'**************************************************
' GenAPI.INC
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
'$IFNDEF _GenAPI_INCLUDED	   
'$DEFINE _GenAPI_INCLUDED
' ====================================================================
'=======================================================
' Array Type definitions:
'=======================================================
' The PROCESS_INFORMATION structure is used by the CreateProcess Function to
' return information about the newly created process. The two handles that
' are returned in this structure are open handles to the new process and its
' primary thread. 
Type PROCESS_INFORMATION
    hProcess        As Long
    hThread         As Long
    dwProcessId     As Long
    dwThreadId      As Long
End Type

' The STARTUPINFO structure allows the CreateProcess Function to control
' many aspects of the creation process.
Type STARTUPINFO
    cb              As Long
    lpReserved      As Pointer To String * 1
    lpDesktop       As Pointer To String * 1
    lpTitle         As Pointer To String * 1
    dwX             As Long
    dwY             As Long
    dwXSize         As Long
    dwYSize         As Long
    dwXCountChars   As Long
    dwYCountChars   As Long
    dwFillAttribute As Long
    dwFlags         As Long
    wShowWindow     As Short
    cbReserved2     As Short
    lpReserved2     As Pointer To String * 1
    hStdInput       As Long
    hStdOutput      As Long
    hStdError       As Long
End Type

Global si           As STARTUPINFO
Global pi           As PROCESS_INFORMATION

' Structure used for GetFileVersionInfo
Type VS_FIXEDFILEINFO
   dwSignature         As Long
   dwStrucVersionl     As Integer '  e.g. = &h0000 = 0
   dwStrucVersionh     As Integer '  e.g. = &h0042 = .42
   dwFileVersionMSl    As Integer '  e.g. = &h0003 = 3
   dwFileVersionMSh    As Integer '  e.g. = &h0075 = .75
   dwFileVersionLSl    As Integer '  e.g. = &h0000 = 0
   dwFileVersionLSh    As Integer '  e.g. = &h0031 = .31
   dwProductVersionMSl As Integer '  e.g. = &h0003 = 3
   dwProductVersionMSh As Integer '  e.g. = &h0010 = .1
   dwProductVersionLSl As Integer '  e.g. = &h0000 = 0
   dwProductVersionLSh As Integer '  e.g. = &h0031 = .31
   dwFileFlagsMask     As Long    '  = &h3F for version "0.42"
   dwFileFlags         As Long    '  e.g. VFF_DEBUG Or VFF_PRERELEASE
   dwFileOS            As Long    '  e.g. VOS_DOS_WINDOWS16
   dwFileType          As Long    '  e.g. VFT_DRIVER
   dwFileSubtype       As Long    '  e.g. VFT2_DRV_KEYBOARD
   dwFileDateMS        As Long    '  e.g. 0
   dwFileDateLS        As Long    '  e.g. 0
End Type

' Structure used for strGetFile()
Type OPENFILENAME
    lStructSize       As Long
    hwndOwner         As Long
    hInstance         As Long
    lpstrFilter       As pointer to string * 1
    lpstrCustomFilter As pointer to string * 1
    nMaxCustFilter    As Long
    nFilterIndex      As Long
    lpstrFile         As pointer to string * 1
    nMaxFile          As Long
    lpstrFileTitle    As pointer to string * 1
    nMaxFileTitle     As Long
    lpstrInitialDir   As pointer to string * 1
    lpstrTitle        As pointer to string * 1
    Flags             As Long
    nFileOffset       As short
    nFileExtension    As short
    lpstrDefExt       As pointer to string * 1
    lCustData         As Long
    lpfnHook          As dlgproc
    lpTemplateName    As pointer to string * 1
End Type

const OFN_READONLY             = &H00000001&
const OFN_OVERWRITEPROMPT      = &H00000002&
const OFN_HIDEREADONLY         = &H00000004&
const OFN_NOCHANGEDIR          = &H00000008&
const OFN_SHOWHELP             = &H00000010&
const OFN_ENABLEHOOK           = &H00000020&
const OFN_ENABLETEMPLATE       = &H00000040&
const OFN_ENABLETEMPLATEHANDLE = &H00000080&
const OFN_NOVALIDATE           = &H00000100&
const OFN_ALLOWMULTISELECT     = &H00000200&
const OFN_EXTENSIONDIFFERENT   = &H00000400&
const OFN_PATHMUSTEXIST        = &H00000800&
const OFN_FILEMUSTEXIST        = &H00001000&
const OFN_CREATEPROMPT         = &H00002000&
const OFN_SHAREAWARE           = &H00004000&
const OFN_NOREADONLYRETURN     = &H00008000&
const OFN_NOTESTFILECREATE     = &H00010000&
const OFN_NONETWORKBUTTON      = &H00020000&
const OFN_NOLONGNAMES          = &H00040000&

' The following are used in GetDialogText Function
Const GW_CHILD     = 5
Const GWL_STYLE    = (-16)
Const GW_HWNDNEXT  = 2

'
' Static Control Mesages used in GetDialogBitMap
'
Const STM_SETICON = &H170&
Const STM_GETICON = &H171&
Const STM_MSGMAX  = &H172&

' Standard Icon IDs	used in GetDialogBitMap
'
Const IDI_APPLICATION = (32512)
Const IDI_HAND        = (32513)
Const IDI_QUESTION    = (32514)
Const IDI_EXCLAMATION = (32515)
Const IDI_ASTERISK    = (32516)

' Predefined by MS-Test and is shown here As a reference
'
'Type RECT
'    wleft   As Long
'    wtop    As Long
'    wright  As Long
'    wbottom As Long
'End Type

' Used by CompareFileTimes()
const FILE_SHARE_READ = &H00000001&
const HFILE_ERROR     = -1
const GENERIC_READ    = &H80000000&
const OPEN_EXISTING   = 3

' File System time stamps are represented with the following structure:
'
type FILETIME
    dwLowDateTime  As Long
    dwHighDateTime As Long
End type

'
'  System time is represented with the following structure:
'
type SYSTEMTIME
    wYear         As short
    wMonth        As short
    wDayOfWeek    As short
    wDay          As short
    wHour         As short
    wMinute       As short
    wSecond       As short
    wMilliseconds As short
End type

' Used by MemoryCheckToScreen()
type MEMORYSTATUS
    dwLength        As Long
    dwMemoryLoad    As Long
    dwTotalPhys     As Long
    dwAvailPhys     As Long
    dwTotalPageFile As Long
    dwAvailPageFile As Long
    dwTotalVirtual  As Long
    dwAvailVirtual  As Long
End type

' Used by IsWin95 IsWinNt IsWin32s
type OSVERSIONINFO
    dwOSVersionInfoSize as long
    dwMajorVersion      as long
    dwMinorVersion      as long
    dwBuildNumber       as long
    dwPlatformId        as long
    szCSDVersion as string * 128
end type

' Used by IsWin95 IsWinNt IsWin32s
Const VER_PLATFORM_WIN32s        = 0
Const VER_PLATFORM_WIN32_WINDOWS = 1
Const VER_PLATFORM_WIN32_NT      = 2

' Array used by GetPrivateList()
Global KeyItemList (1) As String

'used by Function file_size
const MAX_PATH    = 260

type WIN32_FIND_DATA
    dwFileAttributes   as long
    ftCreationTime     as FILETIME
    ftLastAccessTime   as FILETIME
    ftLastWriteTime    as FILETIME
    nFileSizeHigh      as long
    nFileSizeLow       as long
    dwReserved0        as long
    dwReserved1        as long
    cFileName          as string * MAX_PATH
    cAlternateFileName as string * 14
end type

const INVALID_HANDLE_VALUE  = -1

'
' Standard Cursor IDs
'
const IDC_ARROW       = (32512)
const IDC_IBEAM       = (32513)
const IDC_WAIT        = (32514)
const IDC_CROSS       = (32515)
const IDC_UPARROW     = (32516)
const IDC_SIZE        = (32640)
const IDC_ICON        = (32641)
const IDC_SIZENWSE    = (32642)
const IDC_SIZENESW    = (32643)
const IDC_SIZEWE      = (32644)
const IDC_SIZENS      = (32645)
const IDC_SIZEALL     = (32646) ' not in win3.1
const IDC_NO          = (32648) ' not in win3.1
const IDC_APPSTARTING = (32650) ' not in win3.1

' some definitions for added keybd_event
const VK_SHIFT    = &H10&
const KEYEVENTF_KEYUP       = &H0002&

'============================================================
' API Declarations
'============================================================
declare function GetDiskFreeSpace lib "kernel32.dll" alias "GetDiskFreeSpaceA" (lpRootPathName$, lpSectorsPerCluster as pointer to long, lpBytesPerSector as pointer to long, _
lpNumberOfFreeClusters as pointer to long, lpTotalNumberOfClusters as pointer to long) as long
declare function FindFirstFile   lib "kernel32.dll" alias "FindFirstFileA" (lpFileName$, lpFindFileData as WIN32_FIND_DATA) as long
declare function FindClose       lib "kernel32.dll" alias "FindClose"                  (hFindFile&) as long
declare function GetFileSize     lib "kernel32.dll" alias "GetFileSize" (hFile&, lpFileSizeHigh as pointer to long) as long
declare function GetForegroundWindow lib "user32.dll" alias "GetForegroundWindow" () as long
declare function SetForegroundWindow lib "user32.dll" alias "SetForegroundWindow" (hWnd&) as long
declare function GetDlgItem      lib "user32.dll" alias "GetDlgItem"                (hDlg&, nIDDlgItem&) as long
Declare Function GetVersionEx    Lib "kernel32.dll" Alias "GetVersionExA" (osv As OSVERSIONINFO) As Long
Declare Function RemoveDirectory Lib "kernel32.dll" Alias "RemoveDirectoryA" (lpPathName$) As Long
Declare Sub      GetLocalTime    Lib "kernel32.dll" Alias "GetLocalTime" (lpSystemTime As SYSTEMTIME)
Declare Function SetLocalTime    Lib "kernel32.dll" Alias "SetLocalTime" (lp As SYSTEMTIME) As Long
Declare Sub      GetSystemTime   Lib "kernel32.dll" Alias "GetSystemTime" (lpSystemTime As SYSTEMTIME)
Declare Function SetSystemTime   Lib "kernel32.dll" Alias "SetSystemTime" (lp As SYSTEMTIME) As Long
Declare Function GetDateFormat   Lib "kernel32.dll" Alias "GetDateFormatA" (locale As Long, dwFlags As Long,lpDate As SYSTEMTIME, lpFormat As String, lpDateStr As String, cchTime As Long) As Long
Declare Function GetTimeFormat   Lib "kernel32.dll" Alias "GetTimeFormatA" (locale As Long, dwFlags As Long,lpTime As SYSTEMTIME, lpFormat As String, lpTimeStr As String, cchTime As Long) As Long
Declare Function FileTimeToSystemTime    Lib "kernel32.dll" Alias "FileTimeToSystemTime"    (lp As FILETIME, lpSystemTime As SYSTEMTIME) As Long
Declare Function FileTimeToLocalFileTime Lib "kernel32.dll" Alias "FileTimeToLocalFileTime" (lp As FILETIME, lpLocalFileTime As FILETIME) As Long
Declare Function FileTimeToDosDateTime   Lib "kernel32.dll" Alias "FileTimeToDosDateTime"   (lp As FILETIME, lpFatDate As pointer to short, lpFatTime As pointer to short) As Long
'Declare Function GetRes Lib "RSRC32.dll" Alias "_MyGetFreeSystemResources32@4" (typeSysResource%) As Long
Declare Sub      GlobalMemoryStatus  Lib "kernel32.dll" Alias "GlobalMemoryStatus" (lpBuffer As MEMORYSTATUS)
Declare Function GetWindowsDirectory Lib "kernel32.dll" Alias "GetWindowsDirectoryA" (lpBuffer$, uSize&) As Long
Declare Function CompareFileTime   Lib "kernel32.dll" Alias "CompareFileTime" (lp As FILETIME, lp As FILETIME) As Long
Declare Function GetFileTime       Lib "kernel32.dll" Alias "GetFileTime" (hFile&, lpCreationTime As FILETIME, lpLastAccessTime As FILETIME, lpLastWriteTime As FILETIME) As Long
Declare Function CreateFile        Lib "kernel32.dll" Alias "CreateFileA" (lpFileName$, dwDesiredAccess&, dwShareMode&, lpSecurityAttributes As SECURITY_ATTRIBUTES, dwCreationDisposition&, dwFlagsAndAttributes&, hTemplateFile&) As Long
Declare Function GetPrivateProfileInt Lib "kernel32.dll" Alias "GetPrivateProfileIntA" (lpAppName$, lpKeyName$, nDefault&, lpFileName$) As Long
Declare Function GetPrivateProfileString Lib "kernel32.dll" Alias "GetPrivateProfileStringA" (lpAppName$, lpKeyName As Any, lpDefault$, lpReturnedString$, nSize&, lpFileName$) As Long
Declare Function GetPrivateProfileStringKeys Lib "kernel32.dll" Alias "GetPrivateProfileStringA" (lpAppName$,lpKeyName&,lpDefault$,lpReturnedString As Any,nSize&,lpFileName$) As Long
Declare Function WritePrivateProfileString Lib "kernel32.dll" Alias "WritePrivateProfileStringA" (lpAppName$, lpKeyName$, lpString$, lpFileName$) As Long
Declare Function GetComputerName Lib "kernel32.dll" Alias "GetComputerNameA" (lpBuffer$,nSize As Pointer To Long) As Long
Declare Function WaitForSingleObject Lib "kernel32.dll" Alias "WaitForSingleObject" (hHandle&, dwMilliseconds&) As Long
Declare Function WaitForInputIdle    Lib "user32.dll" Alias "WaitForInputIdle" (hProcess&, dwMilliseconds&) As Long
Declare Function CreateProcess       Lib "kernel32.dll" Alias "CreateProcessA" (lpApplicationName$, lpCommandLine$, lpProcessAttributes As SECURITY_ATTRIBUTES, lpThreadAttributes As SECURITY_ATTRIBUTES, bInheritHandles&, _
dwCreationFlags&, lpEnvironment As Any, lpCurrentDirectory$, lpStartupInfo As STARTUPINFO, lpProcessInformation As PROCESS_INFORMATION) As Long
Declare Function GetExitCodeProcess  Lib "kernel32.dll" Alias "GetExitCodeProcess" (hProcess&, lpExitCode As Pointer To Long) As Long
Declare Function TerminateProcess    Lib "kernel32.dll" Alias "TerminateProcess" (hProcess&, uExitCode&) As Long
Declare Function CloseHandle         Lib "kernel32.dll" Alias "CloseHandle" (hObject&) As Long
Declare Function GetExitCodeThread   Lib "kernel32.dll" Alias "GetExitCodeThread" (hThread&, lpExitCode As Pointer To Long) As Long
Declare Function TerminateThread     Lib "kernel32.dll" Alias "TerminateThread" (hThread&, dwExitCode&) As Long
Declare Sub      ExitProcess         Lib "kernel32.dll" Alias "ExitProcess" (uExitCode&)
Declare Sub      ExitThread          Lib "kernel32.dll" Alias "ExitThread" (uExitCode&)
Declare Function GetLastError        Lib "kernel32.dll" Alias "GetLastError" () As Long
Declare Function GetSystemMetrics    Lib "user32.dll" Alias "GetSystemMetrics" (nIndex&) As Long
Declare Function GetWindowLong   Lib "user32.dll" Alias "GetWindowLongA"(hWnd&,nIndex&) As Long 
Declare Function GetWindowText   Lib "user32.dll" Alias "GetWindowTextA"(hWnd&,lpString$,cch&) As Long 
Declare Function GetWindow       Lib "user32.dll" (hWnd&,wCmd&) As Long 
Declare Function GetActiveWindow Lib "user32.dll" () As Long 
Declare Function FindWindow      Lib "user32.dll" Alias "FindWindowA" (lpClassName$,lpWindowName$) As Long 
Declare Sub      GetWindowRect   Lib "user32.dll" Alias "GetWindowRect" (hWnd&, lpRect As RECT)
Declare Function LoadIcon        Lib "user32.dll" Alias "LoadIconA" (hInstance&, lp As Any) As Long
Declare Function SendMessage     Lib "user32.dll" Alias "SendMessageA" (hWnd&, Msg&, lp As Any, lp As Any) As Long
Declare Function IsWindow        Lib "user32.dll"(hWnd&) As Long
Declare Function IsWindowEnabled Lib "user32.dll"(hWnd&) As Long
Declare Function IsWindowVisible Lib "user32.dll"(hWnd&) As Long
Declare Function DestroyWindow   Lib "user32.dll"(hWnd&) As Long 
Declare Function GetWindowThreadProcessId Lib "user32.dll"(hWnd&,lpdwProcessId&) As Long 
Declare Function GetFileVersionInfoSize Lib "version.dll" Alias "GetFileVersionInfoSizeA" (lptstrFilename$, lpdwHandle As pointer to Long) As Long
Declare Function GetFileVersionInfo     Lib "version.dll" Alias "GetFileVersionInfoA"    (lptstrFilename$, dwHandle&, dwLen&, lpData As any) As Long
Declare Function VerLanguageName        Lib "version.dll" Alias "VerLanguageNameA"       (wLang&, szLang$, nSize&) As Long
Declare Function VerQueryValue          Lib "version.dll" Alias "VerQueryValueA"         (pBlock As any, lpSubBlock$, lplpBuffer As any, puLen As pointer to Long) As Long
Declare Function GetOpenFileName Lib "comdlg32.dll" Alias "GetOpenFileNameA" (lp As OPENFILENAME) As Long
Declare Function GetSaveFileName Lib "comdlg32.dll" Alias "GetSaveFileNameA" (lp As OPENFILENAME) As Long
Declare Function GetFileTitle    Lib "comdlg32.dll" Alias "GetFileTitleA"    (sz$, sz$, w As short) As short
declare sub      keybd_event lib "user32.dll" alias "keybd_event" (bVk as short, bScan as short, dwFlags&, dwExtraInfo&)

'============================================================
' Subroutine/Function Declarations
'============================================================
Declare Function GetMyFileSize (FileName$) As Long
Declare Function GetDriveSpace(DriveName$,LogInfo%) As Long
Declare Function file_size (szFid As String) As Long
Declare Function CompareDialogString(DialogHandle&, CaptionID&, CaptionText$) As Integer
Declare Function My_Exists(FileName$) As Integer
Declare Function ReadINI(TheIniFile$,TheSection$,TheVariable$) As String
Declare Function ReadINIInt(TheIniFile$,TheSection$,TheVariable$) As Integer
Declare Function WriteINI(TheIniFile$,TheSection$,TheVariable$,TheString$) As Integer
Declare Function GetMyComputerName() As String
Declare Function sFileVersion(fn$) As String
Declare Function CheckTheResolution(x As Long, y As Long) As Integer
Declare Function strGetFile(strDir$,strFilter$) As String
Declare Function GetPrivateList(SectionName$,IniFileName$) As Integer
Declare Function CompareFileTimes (FileName1 As string, FileName2 As string) As Long
Declare Function GetWinDir() As string
Declare Function GetMyFileTime (FileName$) As String
Declare Function GetFileDate (FileName$) As String
Declare Function ChgSysDate(TheDay%,TheMonth%,TheYear%) As Integer
Declare Function IsWin95 () As Short
Declare Function IsWinNT () As Short
Declare Function IsWin32s () As Short
Declare Function GetDialogText(DialogHandle As Long) As String
Declare Function GetDialogBitMap(DialogHandle As Long) As String
Declare Function GetCaptionText(WindowHandle As Long) As String

'=========================================================================
' Procedures and Functions Follow
'=========================================================================
'*************************************************************************
' Function GetDriveSpace(DriveName$,LogInfo%) As Long
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Writes Drive size info to log file
' and returns free space
'
' DriveName$ - Name of drive       ( C:\ )
' LogInfo%	 - Write info to log?  ( TRUE/FALSE )
'
' Returns number of Free Bytes
'
'*************************************************************************
Function GetDriveSpace(DriveName$,LogInfo%) As Long

	GetDriveSpace = 0

	Dim SectorsPerCluster     As Long
	Dim BytesPerSector        As Long
	Dim NumberOfFreeClusters  As Long
	Dim TotalNumberOfClusters As Long
	Dim FreeBytes             As Long
	Dim TotalBytes			  As Long
	Dim RetLong               As Long

	' Make sure drive name ends with a slash
	If Right$(DriveName$,1) <> "\" Then DriveName$ = DriveName$ +"\"

	' Call API function
	RetLong = GetDiskFreeSpace(DriveName,VarPtr(SectorsPerCluster),VarPtr(BytesPerSector),VarPtr(NumberOfFreeClusters),VarPtr(TotalNumberOfClusters))
	If RetLong = FALSE Then
		LogWrite("1     ERROR determining Disk Free Space!",LogPath,CritErrPath)
		RetLong = GetLastError
		GetDriveSpace = False
		Exit Function
	End If

	' Log results
	If LogInfo = True Then
		LogWrite("4     Disk Usage for drive:      "+DriveName,LogPath,CritErrPath)
		LogWrite("4     Sectors per cluster:      "+Str(SectorsPerCluster),LogPath,CritErrPath)
		LogWrite("4     Bytes per sector:         "+Str(BytesPerSector),LogPath,CritErrPath)
		LogWrite("4     Number of free clusters:  "+Str(NumberOfFreeClusters),LogPath,CritErrPath)
		LogWrite("4     Total number of clusters: "+Str(TotalNumberOfClusters),LogPath,CritErrPath)
		FreeBytes  = NumberOfFreeClusters  * SectorsPerCluster * BytesPerSector
		TotalBytes = TotalNumberOfClusters * SectorsPerCluster * BytesPerSector
		LogWrite("4     Total bytes:              "+Str(TotalBytes),LogPath,CritErrPath)
		LogWrite("4     Total free bytes:         "+Str(FreeBytes),LogPath,CritErrPath)
		LogWrite("4     Percent Free:             "+Left(Str((FreeBytes/TotalBytes)*100),8),LogPath,CritErrPath)
	Else
		FreeBytes  = NumberOfFreeClusters  * SectorsPerCluster * BytesPerSector
	End If

	' Return number of Free Bytes
	GetDriveSpace = FreeBytes

End Function
'*************************************************************************
' Function File_Size (FileName As String)
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Returns Long Size of file passed as parameter
'
'*************************************************************************
Function File_Size (FileName As String) As Long

  Dim FileHandle&
  Dim lpFindFileData As WIN32_FIND_DATA, fDirectory%

  ' get handle of file and load structure
  FileHandle = FindFirstFile(FileName, lpFindFileData)
  
  ' close file
  FindClose(FileHandle)
  
  ' return file size
  File_Size = lpFindFileData.nFileSizeLow

End Function
'*************************************************************************
' Sub My_Status (MessageString As String)
'
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' This is used instead of regular status box
' because sometimes status box takes away focus
'
'*************************************************************************
Sub My_Status (MessageString As String)

	Dim hwndForeground&

	' Get handle of foreground window
    hwndForeground = GetForegroundWindow()

	' State your business
	' Status box may take focus away
    StatusBox "Please wait ..." + MessageString, 1, 1, , , TRUE

	' Put focus back to original window
	hwndForeground = SetForegroundWindow(hwndForeground)
	Sleep 1

End Sub
'*************************************************************************
' Function My_Exists(FileName$)
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
'  This is used instead of EXISTS function
'  because EXISTS can
'  return a system error.
'
' Tests for file existence
' Returns True/False
'
'*************************************************************************
Function My_Exists(FileName$) As Integer

	Dim lpFindFileData As WIN32_FIND_DATA
	Dim ReturnLong     As Long

	My_Exists = True

	ReturnLong = FindFirstFile (FileName, lpFindFileData)
	If ReturnLong = INVALID_HANDLE_VALUE Then
		My_Exists = False
	End If
	FindClose(ReturnLong)

End Function
'*************************************************************************
' Function ReadINI()
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
'	Retrieve variables from INI file
' 	Parameters:
'		TheIniFile  - The name of the INI file
'		TheSection  - The section within the INI file
'		TheVariable	- The variable name within the Section
'
'   Returns retrieved variable or NotFnd
'
'*************************************************************************
Function ReadINI(TheIniFile As String,TheSection As String,TheVariable As String) As String

Dim ReturnBuffer    As String 
Dim ReturnInteger   As Integer
Dim ReturnString    As String 

ReturnBuffer  = String$(256,0)
ReturnInteger = GetPrivateProfileString(TheSection,TheVariable,"NotFnd",ReturnBuffer,Len(ReturnBuffer),TheIniFile)
ReturnString  = Left$(ReturnBuffer,ReturnInteger)

ReadINI = ReturnString

End Function
'*************************************************************************
' Function WriteINI()
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
'	Writes to an INI file
' 	Parameters:
'		TheIniFile  - The name of the INI file
'		TheSection  - The section within the INI file
'		TheVariable	- The variable name within the Section
'		TheString	- The string to write to the INI file
'
'   Returns True (-1) for success or False (0) for Failure
'
'*************************************************************************
Function WriteINI(TheIniFile As String,TheSection As String,TheVariable As String,TheString As String) As Integer

Dim ReturnInteger   As Integer

' Read INI file
ReturnInteger = WritePrivateProfileString(TheSection,TheVariable,TheString,TheIniFile)

' Return Value
If ReturnInteger <> False Then
	WriteINI = True
Else
	WriteINI = False
End If

End Function
'*************************************************************************
' Function ReadINIInt()
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
'	Retrieve Integer variables from INI file
' 	Parameters:
'		TheIniFile  - The name of the INI file
'		TheSection  - The section within the INI file
'		TheVariable	- The variable name within the Section
'
'   Returns retrieved integer variable or 0
'
'*************************************************************************
Function ReadINIInt(TheIniFile$,TheSection$,TheVariable$) As Integer

	Dim ReturnInteger As Integer

	ReturnInteger = GetPrivateProfileInt(TheSection,TheVariable,0,TheIniFile)

	ReadINIInt = ReturnInteger

End Function
'****************************************************************************
' GetPrivateList - Return all key items for a section within 
'                  an INI file in a user provided array.
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' An Array must be declared	As follows for this Function to work:
'
'    Global KeyItemList (1) As String
'
' Parameters:
'
'	 SectionName$  - The INI file section containing the desired key items
'
'	 IniFileName$	- The path and name to the INI file
'
'
' Example INI file:
'
'    [Defaults]
'
'    Variable1 = C:\Temp\Test1.txt
'    Variable2 = C:\Temp\Test1.txt
'    Variable3 = C:\Temp\Test1.txt
'    Variable4 = C:\Temp\Test1.txt
'
' In the above example the key items are:
'
'    Variable1
'    Variable2
'    Variable3
'    Variable4
'
' And would end up in the KeyItemList array As follows;
'
'    KeyItemList(1) = "Variable1"
'    KeyItemList(2) = "Variable2"
'    KeyItemList(3) = "Variable3"
'    KeyItemList(4) = "Variable4"
'
' Returns:  True/False
'
' Thanks to Jake Vizner for helping me with this code.
'
'****************************************************************************
Function GetPrivateList(SectionName$,IniFileName$) As Integer

    Dim ReturnBuffer  As String * 2048
    Dim ReturnInteger As Integer
    Dim ArrayCount    As Integer
    Dim Pos	          As Integer
	Dim Prev          As Integer

	GetPrivateList = True

	' Initialize Buffer to contain key items
    ReturnBuffer = String$(2048,Chr(0))

	' Retrieve key items from INI file
    ReturnInteger = GetPrivateProfileStringKeys(SectionName,0,"NotFND",ReturnBuffer,Len(ReturnBuffer)-1,IniFileName)

	' Extract key items from ReturnBuffer
	ReturnBuffer = Left$(ReturnBuffer,ReturnInteger)

	' Test Buffer for error
	If InStr(1,ReturnBuffer,"NotFND") Then
		GetPrivateList = False
		LogWrite("1     Error retrieving Key Items from: "+IniFileName+" section: "+SectionName+"!",LogPath,CritErrPath)
		Exit Function
	End If

	' Intialize position variables
    Pos  = InStr(1,ReturnBuffer,Chr$(0))
    Prev = 1

	' Count key items in ReturnBuffer
    While (Pos) 
        Prev = Pos + 1
        Pos = InStr(Prev, ReturnBuffer, Chr$(0))
		ArrayCount = ArrayCount + 1
    Wend

	' Redim array to match number of key items
	ReDim KeyItemList (1 To ArrayCount) As String
	ArrayCount = 0

	' Intialize position variables
    Pos  = InStr(1,ReturnBuffer,Chr$(0))
    Prev = 1

	' Insert key items into array
    While (Pos) 
        ArrayCount = ArrayCount + 1
        KeyItemList(ArrayCount) = Mid$(ReturnBuffer, Prev, Pos - Prev)
        Prev = Pos + 1
        Pos = InStr(Prev, ReturnBuffer, Chr$(0))
    Wend

End Function
'*************************************************************************
'  StartAndStopProcess() - Starts and Terminates Run Process
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
'  This is used instead of Run NoWait
'
'  Parameters:
'
'	TheFileToRun:      The program name to run including Drive and Path.	
'
'	TheCommandLine:	The Command Line to pass If any.  Else  Null.
'
'  Return Value:  True  (-1) False (0)
'
'  Syntax Example:
'
' ReturnInteger = StartAndStopProcess("c:\winnt\40\Notepad.exe", "Null")
' ReturnInteger = StartAndStopProcess("C:\msdev\bin\mtrun.exe","c:\TstPrg.pcd")
'
'  This Function gets around the RUN NOWAIT problem because If we
'  use NOWAIT Then the second program will start before the first has ended.
'  This Function launches the program and Then goes into a loop until the
'  program becomes inactive.
'
'*************************************************************************
Function StartAndStopProcess(TheFileToRun As String, TheCommandLine As String) As Integer

	Dim ReturnInteger           As Integer
	Dim ReturnCode              As Long
	Dim StopProcess             As Integer
	Dim lExitCode               As Long
	Dim STILL_ACTIVE            As Long
	Dim NORMAL_PRIORITY_CLASS   As Long
	Dim DEBUG_PROCESS           As Long
	Dim DEBUG_ONLY_THIS_PROCESS	As Long
	Dim WAIT_TIMEOUT            As Long
	Dim INFINITE                As Long

	STILL_ACTIVE                = &H0103&
	NORMAL_PRIORITY_CLASS       = &H00000020&
	DEBUG_PROCESS               = &H1
	DEBUG_ONLY_THIS_PROCESS		= &H2
	WAIT_TIMEOUT				= &H102&
	INFINITE                    = &HFFFFFFFF
	StartAndStopProcess         = False

	' Make sure TheCommandLine has a beginning space
	If TheCommandLine <> Null Then
		If Left$(TheCommandLine,1) <> " " Then
			TheCommandLine = " "+TheCommandLine
		End If
	End If

	' Start process
	ReturnInteger = CreateProcess(TheFileToRun,TheCommandLine,Null,Null,FALSE,NORMAL_PRIORITY_CLASS,0,Null,si,pi)
'	ReturnInteger = CreateProcess(TheFileToRun,TheCommandLine,Null,Null,True,NORMAL_PRIORITY_CLASS,0,Null,si,pi)
'	ReturnInteger = CreateProcess(TheFileToRun,TheCommandLine,Null,Null,True,DEBUG_PROCESS Or DEBUG_ONLY_THIS_PROCESS Or NORMAL_PRIORITY_CLASS,0,Null,si,pi)

	If ReturnInteger <> False Then

		'WaitForInputIdle(pi.hProcess,INFINITE)
 		CloseHandle(pi.hThread)

		' Loop until process no longer active
		Do

			' Determine If process is still running
			ReturnCode = WaitForSingleObject(pi.hProcess, 5*1000)

			GetExitCodeProcess(pi.hProcess,VarPtr(lExitCode))

			If lExitCode <> STILL_ACTIVE Then
				StopProcess = TerminateProcess(pi.hProcess,0)
				Exit Do
			End If

		Loop While True

		' Close Handles
		CloseHandle(pi.hProcess)

		' Return True
		StartAndStopProcess = True
	
	Else
		' Return False
		StartAndStopProcess = False
	End If

End Function
'*************************************************************************
' Function StartTheProgram
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' This is used instead of Run command because it returns handle of
' application being started.
'
' Note: If no Command line use Null
'
' Parameters:
'
'	TheFileToRun:   The program to start
'	TheCommandLine: The Command Line passed to program to start
'
' Syntax Examples:
'
'	ReturnInteger = StartTheProgram("C:\WINDOWS\EXPLORER.EXE",Null)
'	ReturnInteger = StartTheProgram("c:\winnt\40\Notepad.exe",Null)
'	ReturnInteger = StartTheProgram("C:\msdev\bin\mtrun.exe","c:\TstPrg.pcd")
'
' Returns:  True (-1) or False (0)
'
'*************************************************************************
Function StartTheProgram(TheFileToRun As String, TheCommandLine As String) As Integer

	Dim ReturnInteger           As Integer
	Dim ErrorValue              As Long
	Dim NORMAL_PRIORITY_CLASS   As Long

	NORMAL_PRIORITY_CLASS = &H00000020&

	' Make sure TheCommandLine syntax is correct
	If TheCommandLine <> Null Then
		If Left$(TheCommandLine,1) <> " " Then
			TheCommandLine = " "+TheCommandLine
		End If
		If TheCommandLine = " " Then
			TheCommandLine = Null
		End If
	End If

	' Start the program specified
	ReturnInteger = CreateProcess(TheFileToRun,TheCommandLine,Null,Null,FALSE,NORMAL_PRIORITY_CLASS,0,Null,si,pi)
'	ReturnInteger = CreateProcess(TheFileToRun,TheCommandLine,Null,Null,TRUE,NORMAL_PRIORITY_CLASS,0,Null,si,pi)
'	ReturnInteger = CreateProcess(TheFileToRun,TheCommandLine,Null,Null,True,DEBUG_PROCESS Or DEBUG_ONLY_THIS_PROCESS Or NORMAL_PRIORITY_CLASS,0,Null,si,pi)

 	' Return Value
	If ReturnInteger <> False Then
		StartTheProgram = True
	Else
		StartTheProgram = False
	End If

End Function
'*************************************************************************
' Function StopTheProgram() As Integer
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Returns:  True (-1) or False (0)
'
'*************************************************************************
Function StopTheProgram() As Integer

	Dim ReturnInteger           As Integer
	Dim StopProcess             As Integer
	Dim StopThread              As Integer
	Dim RetHndInt               As Integer

	Dim lExitCode               As Long
	Dim STILL_ACTIVE            As Long

	STILL_ACTIVE = &h0103&

	ReturnInteger = GetExitCodeProcess(pi.hProcess,VarPtr(lExitCode))
	If ReturnInteger <> False Then
		If lExitCode = STILL_ACTIVE Then
			StopProcess = TerminateProcess(pi.hProcess,lExitCode)
			If StopProcess <> False Then
				' Close Process Handles
				RetHndInt = CloseHandle(pi.hProcess)
				' Close Thread Handles
				RetHndInt = CloseHandle(pi.hThread)
				StopTheProgram = True
			Else
				StopTheProgram = False
			End If
		Else
			StopTheProgram = False
		End If
	Else
		StopTheProgram = False
	End If

End Function
'***************************************************************
' GetMyComputerName - Returns local computer name As string
'                     used for Status Log.
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Returns the computer name	As a string
'
'***************************************************************
Function GetMyComputerName() As String

Dim BufferString   As String
Dim BufferPointer  As Pointer To Long
Dim MaxLength      As Long
Dim ReturnLong     As Long
Dim ReturnString   As String

MaxLength     = 256
BufferString  = String$(256,0)
BufferPointer = VarPtr(MaxLength)

' Call API Function
ReturnLong = GetComputerName(BufferString, BufferPointer)

' ReturnLong should be length of Computer Name
' which is contained in BufferString
If ReturnLong <> 0 Then
	ReturnString  = Trim$(BufferString)
Else
	ReturnString  = "NotFound"
End If

' Return the computer name
GetMyComputerName = ReturnString

End Function
'=========================================================================
'	Function sFileVersion$(fn$)
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' 	Returns version number for a given filename
'
'=========================================================================
Function sFileVersion$(fn$)

	Dim junk                 As Long
	Dim rc                   As Integer
	Dim lValueSize           As Long
	Dim pudtVerBuffer        As Pointer To VS_FIXEDFILEINFO
	Dim lVersionBufferLength As Long
	Dim sVersionBuffer       As String
	Dim bVsffiOk             As Long

	bVsffiOk = 0
	
	' Determine size of buffer to contain version info
	lVersionBufferLength = GetFileVersionInfoSize(fn, varptr(junk))

	' Test length
	If lVersionBufferLength > 0 Then
		sVersionBuffer = space$(lVersionBufferLength)
	Else
		sFileVersion = "2.00"
'		sFileVersion = "Version Buffer zero length"
		Exit Function
	End If

	' Get version info into buffer
	rc = GetFileVersionInfo(fn, junk, lVersionBufferLength,sVersionBuffer)
	
	' Test return value
	If rc <> 0 Then 
		' Retrieve information from version resource
		' Examine resource information and copy requested
		' data into a buffer
		bVsffiOk = VerQueryValue(sVersionBuffer, "\", varptr(pudtVerBuffer),varptr(lValueSize))
	End If

	' Test and retrieve version info from structure
	If bVsffiOk <> 0 Then 
		' Parse version info from structure
		sFileVersion = Trim$(Str$(pudtVerBuffer[0].dwProductVersionMSh))+"."+ _
		Trim$(Str$(pudtVerBuffer[0].dwProductVersionMSl))+"."+ _
		Trim$(Str$(pudtVerBuffer[0].dwProductVersionLSh))+"."+ _
		Trim$(Str$(pudtVerBuffer[0].dwProductVersionLSl))
	Else 
		sFileVersion = "could not be retrieved"
	Endif

End Function
'*************************************************************************
' Function strGetFile(strDir$,strFilter$) As String
'
' Thanks to Tom Arnold
' "Software Testing with Visual Test 4.0"
'
' pages 311-316
'
' ISBN 0-7645-8000-0
'
' If you don't own a copy of his book Then quit asking stupid questions.
'
' Displays the "Common Open dialog box" and returns file selected
'
'
'*************************************************************************
Function strGetFile(strDir$,strFilter$) As String

	Dim iRet           As Integer
	Dim ofnFile	       As OPENFILENAME
	Dim strFileChoice  As String * 128
	Dim strStartDir    As String * 128
	Dim strFileFilter  As String * 128

	strStartDir = strDir + CHR$(0) ' Ensure this is null terminated

	' Replace all comma characters with CHR$(0). 
	iRet = Instr(strFilter, ",")
	While (iRet) 
		Mid$(strFilter,iRet,1) = CHR$(0)
		iRet = Instr(strFilter,",")
	WEnd

    ' Make sure string is terminated with two nulls As required by
	' GetOpenFileName() API.
	strFileFilter = Trim$(strFilter) + CHR$(0) + CHR$(0)

	' Initialize the structure before passing it
	ofnFile.lStructSize     = Len(OPENFILENAME)
	ofnFile.hwndOwner       = wGetActWnd(0)
	ofnFile.lpstrFilter     = cptr(varptr(strFileFilter))
	ofnFile.nFilterIndex    = 1
	ofnFile.lpstrInitialDir = cptr(varptr(strStartDir))
	ofnFile.lpstrFile       = cptr(varptr(strFileChoice))
	ofnFile.nMaxFile        = Len(strFileChoice)
	ofnFile.Flags           = OFN_FILEMUSTEXIST OR _
							  OFN_HIDEREADONLY OR _
							  OFN_PATHMUSTEXIST

	If GetOpenFileName(ofnFile) Then
		strGetFile = Trim$(Trim$(strFileChoice,0))
	Else
		strGetFile = ""
	Endif

End Function
'***************************************************************************
'*  Function:   IsWin95
'*  Purpose:	Check if running OS is Windows 95
'*  Parameters:	None
'*  Return:		TRUE if running OS is Windows 95 otherwise FALSE
'*  Author:     Jake Visner
'***************************************************************************
Function IsWin95 () As Short

	Dim lpVerInfo As OSVERSIONINFO

	' Specify the size of this data structure.
	lpVerInfo.dwOSVersionInfoSize = Lenb (OSVERSIONINFO)

	' Obtain extended information about the version 
	' of the operating system that is currently running.
 	GetVersionEx (lpVerInfo)

	' Is this Win32 on Windows 95 ?
	If lpVerInfo.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS Then
		IsWin95 = TRUE
	Else
		IsWin95 = FALSE
	End If 

End Function
'***************************************************************************
'*  Function:   IsWinNT
'*  Purpose:	Check if running OS is Windows NT
'*  Parameters:	None
'*  Return:		TRUE if running OS is Windows NT otherwise FALSE
'*  Author:     Jake Visner
'***************************************************************************
Function IsWinNT () As Short

	Dim lpVerInfo As OSVERSIONINFO

	' Specify the size of this data structure.
	lpVerInfo.dwOSVersionInfoSize = Lenb (OSVERSIONINFO)

	' Obtain extended information about the version 
	' of the operating system that is currently running.
 	GetVersionEx (lpVerInfo)

	' Is this Windows NT  ?
	If lpVerInfo.dwPlatformId = VER_PLATFORM_WIN32_NT Then
		IsWinNT = TRUE
	Else
		IsWinNT = FALSE
	End If 

End Function
'***************************************************************************
'*  Function:   IsWin32s
'*  Purpose:	Check if running OS is Win32s running under Win 3.1
'*  Parameters:	None
'*  Return:		TRUE if running OS is Win32s otherwise FALSE
'*  Author:     Jake Visner
'***************************************************************************
Function IsWin32s () As Short

	Dim lpVerInfo As OSVERSIONINFO

	' Specify the size of this data structure.
	lpVerInfo.dwOSVersionInfoSize = Lenb (OSVERSIONINFO)

	' Obtain extended information about the version 
	' of the operating system that is currently running.
 	GetVersionEx (lpVerInfo)

	' Is this Win32s on Windows 3.1 ?
	If lpVerInfo.dwPlatformId = VER_PLATFORM_WIN32s Then
		IsWin32s = TRUE
	Else
		IsWin32s = FALSE
	End if 
	
End Function
'***************************************************************************
' Function:    GetMyFileSize(FileName$)
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Returns Long filesize of file passed as parameter
'
'***************************************************************************
Function GetMyFileSize (FileName$) As Long

	Dim hFile         As Long
	Dim ReturnLong    As Long

	' Get file handle
    hFile = CreateFile (FileName, GENERIC_READ, FILE_SHARE_READ, Null,OPEN_EXISTING, Null, Null)

	' Test return value
	If hFile = HFILE_ERROR Then 
		GetMyFileSize = 0
	    Exit Function
	End If

	' Get file Size
    ReturnLong = GetFileSize(hFile, Null)
	
	' Close open file handle
	CloseHandle (hFile)

	' Return value
	GetMyFileSize = ReturnLong

End Function	
'***************************************************************************
'Function ChgSysDate(TheDay%,TheMonth%,TheYear%) As Integer
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
'  Changes the system day/month/and or year
'
'  Returns True/False
'
'***************************************************************************
Function ChgSysDate(TheDay%,TheMonth%,TheYear%) As Integer
 
	Dim SysTime       As SYSTEMTIME
	Dim RetLong       As Long
	Dim RetError      As Long

	Dim NewYear       As Integer
	Dim NewMonth      As Integer
	Dim NewDay        As Integer
	Dim NewHour	      As Integer
	Dim NewMinute     As Integer
	Dim NewSecond     As Integer

'	GetSystemTime(SysTime)
	GetLocalTime(SysTime)

' 	NewYear	  =	SysTime.wYear
'	NewMonth  =	SysTime.wMonth
'	NewDay	  =	32
'	NewHour	  =	SysTime.wHour
'	NewMinute = SysTime.wMinute
'	NewSecond = SysTime.wSecond

	SysTime.wDay    = SysTime.wDay+TheDay
	SysTime.wMonth  = SysTime.wMonth+TheMonth
	SysTime.wYear   = SysTime.wYear+TheYear


'	SysTime.wHour   = NewHour
'	SysTime.wMinute = NewMinute
'	SysTime.wSecond = NewSecond

	RetLong = SetLocalTime(SysTime)

	If RetLong = 0 Then
		RetError = GetLastError()
		ChgSysDate = False
		Exit Function
	End If

'	GetSystemTime(SysTime)


End Function
'***************************************************************************
' Function:    GetMyFileTime()
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Purpose:     Gets the creation time of a file supplied by its full path 
'
' Parameters:  FileName$ -  input string representing full path to file name 
'
' Returns:     File Time i.e. 02:05:22 PM  (12 hour clock)
'
'***************************************************************************
Function GetMyFileTime (FileName$) As String
	
	Dim hFile         As Long
	Dim LastWrite     As FILETIME
	Dim NewTime       As FILETIME
	Dim SysTime       As SYSTEMTIME
	Dim ReturnLong    As Long
    Dim TimeBuffer    As String
	Dim ReturnTime    As String

	' Initialize Buffer to contain time
    TimeBuffer = String$(144,Chr(0))

	' Get file handle
    hFile = CreateFile (FileName, GENERIC_READ, FILE_SHARE_READ, Null,OPEN_EXISTING, Null, Null)

	' Test return value
	If hFile = HFILE_ERROR Then 
		GetMyFileTime = "CreateFile Error"
	    Exit Function
	End If

	' Get file time
    ReturnLong = GetFileTime ( hFile, Null, Null, LastWrite )
	
	' Close open file handle
	CloseHandle ( hFile )

	' Test return value
	If ReturnLong = 0 Then
		GetMyFileTime = "GetFileTime Error"
	    Exit Function
	End If

	' Convert file time to local time
	ReturnLong = FileTimeToLocalFileTime(LastWrite,NewTime)

	' Test return value
	If ReturnLong = 0 Then
		GetMyFileTime = "LocalTime Error"
	    Exit Function
	End If

	' Convert local time to system time
	ReturnLong = FileTimeToSystemTime(NewTime,SysTime)

	' Test return value
	If ReturnLong = 0 Then
		GetMyFileTime = "SystemTime Error"
	    Exit Function
	End If

	' Retrieve file time from buffer 
'   Examples of different return formats
'             								  default format
'	ReturnLong = GetTimeFormat(Null,0,SysTime,Null,TimeBuffer,Len(TimeBuffer))
'    										  24 hour clock format
'	ReturnLong = GetTimeFormat(Null,0,SysTime,"HH:mm:ss tt",TimeBuffer,Len(TimeBuffer))
'											  12 hour clock format
	ReturnLong = GetTimeFormat(Null,0,SysTime,"hh:mm:ss tt",TimeBuffer,Len(TimeBuffer))

	' Test return value
	If ReturnLong = 0 Then
		GetMyFileTime = "TimeFormat Error"
	    Exit Function
	End If

	' Return File Time
	GetMyFileTime = Left(TimeBuffer,ReturnLong)

End Function
'***************************************************************************
' Function:    GetFileDate
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Purpose:     Gets the creation date of a file supplied by its full path 
'
' Parameters:  FileName$ -  input string representing full path to file name 
'
' Return:      File Date -  format:  MM/DD/YY
'
'***************************************************************************
Function GetFileDate (FileName$) As String

	Dim hFile         As Long
	Dim LastWrite     As FILETIME
	Dim NewTime       As FILETIME
	Dim SysTime       As SYSTEMTIME
	Dim ReturnLong    As Long
    Dim DateBuffer    As String
	Dim ReturnDate    As String

	' Initialize Buffer to contain Date
    DateBuffer = String$(144,Chr(0))

	' Get file handle
    hFile = CreateFile (FileName, GENERIC_READ, FILE_SHARE_READ, Null,OPEN_EXISTING, Null, Null)

	' Test return value
	If hFile = HFILE_ERROR Then 
		GetFileDate = "CreateFile Error"
	    Exit Function
	End If

	' Get file time
    ReturnLong = GetFileTime ( hFile, Null, Null, LastWrite )
	
	' Close open file handle
	CloseHandle ( hFile )

	' Test return value
	If ReturnLong = 0 Then
		GetFileDate = "GetFileTime Error"
	    Exit Function
	End If

	' Convert file time to local time
	ReturnLong = FileTimeToLocalFileTime(LastWrite,NewTime)

	' Test return value
	If ReturnLong = 0 Then
		GetFileDate = "LocalTime Error"
	    Exit Function
	End If

	' Convert local time to system time
	ReturnLong = FileTimeToSystemTime(NewTime,SysTime)

	' Test return value
	If ReturnLong = 0 Then
		GetFileDate = "SystemTime Error"
	    Exit Function
	End If

	' Retrieve date from buffer 

'   Examples of different date formats
'											  default date format
'	ReturnLong = GetDateFormat(Null,0,SysTime,Null,DateBuffer,Len(DateBuffer))
'
'	ReturnLong = GetDateFormat(Null,0,SysTime,"MM/dd/yyyy",DateBuffer,Len(DateBuffer))

	ReturnLong = GetDateFormat(Null,0,SysTime,"MM/dd/yy",DateBuffer,Len(DateBuffer))

	' Test return value
	If ReturnLong = 0 Then
		GetFileDate = "TimeFormat Error"
	    Exit Function
	End If

	' Return File Date
	GetFileDate = Left(DateBuffer,ReturnLong)

End Function
'***************************************************************************
' Function:    CompareFileTimes
'
' Thanks to Jake Vizner for this code.
'
' Purpose:     Compare file times of files supplied by their full path 
'
' Parameters:  FileName1 -  input string representing full path to first file name 
'
'              FileName2 -  input string representing full path to second file name
'
' Return:      -1	First file time is less than second file time.
'				0	First file time is equal to second file time.
'				+1	First file time is greater than second file time.
'
'				Anything else is error number returned by GetLastError(), for example
'				3   FileName1 or FileName2 does not exist
'
'
'***************************************************************************
Function CompareFileTimes (FileName1 As string, FileName2 As string) As Long

	Dim hFile1 As Long
	Dim hFile2 As Long
	Dim LastWrite1 As FILETIME
	Dim LastWrite2 As FILETIME
	Dim Result As Long

	' Get first file handle
    hFile1 = CreateFile (FileName1, GENERIC_READ, FILE_SHARE_READ, Null,OPEN_EXISTING, Null, Null)

	' If there is an error, return error number returned by GetLastError
	If hFile1 = HFILE_ERROR Then 
		Result = GetLastError ()
		CompareFileTimes = GetLastError ()
	    Exit Function
	End If

	' Get second file handle
    hFile2 = CreateFile (FileName2, GENERIC_READ, FILE_SHARE_READ, Null,OPEN_EXISTING, Null, Null)

	If hFile2 = HFILE_ERROR Then 
		CloseHandle ( hFile1 )
		CompareFileTimes = GetLastError ()
	    Exit Function
	End If

	' Get first file time
    GetFileTime ( hFile1, Null, Null, LastWrite1 )

	' Get second file time
	GetFileTime ( hFile2, Null, Null, LastWrite2 )

	' Compare file times
	Result = CompareFileTime ( LastWrite1, LastWrite2 )

	CompareFileTimes = Result

	' Close open files handles
	CloseHandle ( hFile1 )
	CloseHandle ( hFile2 )

End Function
'***************************************************************************
'  Function:      GetWinDir
'  Purpose:       Retrieve Windows directory
'  Parameters:    None
'  Return Value:  Windows directory
'
'  Thanks to Jake Vizner for this code.
'
'***************************************************************************
Function GetWinDir() As string

    Dim Buffer As string
    Dim TempDir As string
    Dim Ret As short

    Buffer = string$(144,0)
    Ret = GetWindowsDirectory (Buffer, len(Buffer))
    TempDir = left$(Buffer, Ret)   ' get rid of garbage

    ' If last character is "\" Then remove it
    If right$(TempDir, 1) = "\" Then
        TempDir = left$(TempDir, len(TempDir) - 1)
    End If

    GetWinDir = TempDir

End Function
'*******************************************************************
' Function: CheckTheResolution

' Desc: This Sub will compare the width and height specified by 'x' and 'y'
'       to the real width and height of the screen.  If they
'       do not match, False is returned
'
' Returns True for Success False for FAIL
'
'*******************************************************************
Function CheckTheResolution(x As Long, y As Long) As Integer

	Dim SM_CXSCREEN As Integer
	Dim SM_CYSCREEN As Integer

	SM_CXSCREEN = 0
	SM_CYSCREEN = 1

    If (GetSystemMetrics (SM_CXSCREEN) <> x) Or (GetSystemMetrics (SM_CYSCREEN) <> y) Then
		' Write error message to Log File
		LogWrite("1     This script was designed for "+Trim$(Str$(x))+"x"+Trim$(Str$(y))+" resolution!",LogPath,CritErrPath)
		CheckTheResolution = False
	Else
		CheckTheResolution = True
    End If

End Sub
'*************************************************************************
' Function GetCaptionText
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Retrieves a window caption
'
' Parameters:
'     DialogHandle - The Handle of the Window to retrieve the Caption.
'
' Returns:
'	  The Window Caption or NotFound.
'
'*************************************************************************
Function GetCaptionText(WindowHandle As Long) As String

	Dim MsgLong       As Long
	Dim MsgBuffer     As String

	MsgBuffer = String$(256,0)

	' Retrieve Window Caption for the window handle
	MsgLong = GetWindowText(WindowHandle,MsgBuffer,Len(MsgBuffer))

	' Return Window Caption
	If MsgLong > 0 Then
		GetCaptionText = Left$(MsgBuffer,MsgLong)
	Else
		GetCaptionText = "NotFound"
	End If

End Function
'*************************************************************************
' Function CompareDialogString
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Compares the Text within a dialog box to an expected string
'
' Parameters:
'	DialogHandle& - The Handle of the dialog box from which to 
'                 	 retrieve the Text.
'	CaptionID&    - ID of Caption to retrieve
'	CaptionText$  - Text to compare to	
'
' Returns True/False
'
'*************************************************************************
Function CompareDialogString(DialogHandle&, CaptionID&, CaptionText$ ) As Integer

	Dim hMsgText As Long
	Dim Caption  As String

	CompareDialogString = False

	' Get handle of Message
    hMsgText& = GetDlgItem(DialogHandle,CaptionID)

    If hMsgText& <> NULL Then
		' Get caption from dialog
        Caption$ = GetText(hMsgText&)
		LogWrite("0     Dialog Message: "+CompressString(Caption$),LogPath,CritErrPath)
        If InStr(1,Caption,CaptionText) Then
			CompareDialogString = True
        End If
    End If

End Function
'*************************************************************************
' Function GetDialogText
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Retrieves the Text within a dialog box
'
' Parameters:
'     DialogHandle - The Handle of the dialog box from which to 
'                 	 retrieve the Text.
' Returns:
'	  The Text within the dialog box
'	  or NotFound.
'
'
'	NOTE!!  The style number must match
'	the style of a dialog you are looking for.
'   You can retrieve the style with WInfo
'
'*************************************************************************
Function GetDialogText(DialogHandle As Long) As String

	Dim FoundText      As Integer
	Dim ChildHandle    As Long
	Dim MsgLong        As Long
	Dim MsgBuffer      As String
	Dim BufferString   As String
	Dim CRPos          As Integer

	MsgBuffer = String$(256,0)

	' Retrieve handle of 1st child within window
	ChildHandle = GetWindow(DialogHandle,GW_CHILD)

	' Loop through dialog objects until we find child handle
	' that equals the style of a dialog text message.
	Do 
		If GetWindowLong(ChildHandle,GWL_STYLE) = 1342316672 Or _
			GetWindowLong(ChildHandle,GWL_STYLE) = 1342308480 Then
			FoundText = True
			Exit Do
		End If
		ChildHandle = GetWindow(ChildHandle,GW_HWNDNEXT)
	Loop While ChildHandle <> Null

	If FoundText = True Then
		' Retrieve Window Text for the Child handle
		MsgLong = GetWindowText(ChildHandle,MsgBuffer,Len(MsgBuffer))
	End If

	' Return Text message
	If MsgLong > 0 Then

		BufferString = Left$(MsgBuffer,MsgLong)

		' If Carriage Return(s) add Spaces and Line Feed(s)
		' This formats it for the log file
		CRPos = Instr(1,BufferString,CHR(13))
		
		While CRPos >0
			BufferString = Mid(BufferString,1,CRPos)+CHR(10)+SPACE(45)+Mid(BufferString,CRPos+1,Len(BufferString))
			CRPos = Instr(CRPos+1,BufferString,CHR(13))
		WEnd

		GetDialogText = BufferString

	Else
		GetDialogText = "NotFound"
	End If

End Function
'*************************************************************************
' Function GetDialogBitMap
'
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
'
' Retrieves the type of Bitmap within a dialog box
'
' Parameters:
'     DialogHandle - The Handle of the dialog box from which to 
'                 	 retrieve the Text.
' Returns:
'	  The type of Bitmap within the dialog box
'	  or NotFound or Undefined.
'
'	NOTE!!  The style number must match
'	the style of a dialog Bitmap you are looking for.
'   You can retrieve the style with WInfo
'
'*************************************************************************
Function GetDialogBitMap(DialogHandle As Long) As String

	Dim FoundBitmap    As Integer
	Dim ChildHandle    As Long
	Dim NewIconHandle  As Long
	Dim OldIconHandle  As Long

	FoundBitmap = False

	' Retrieve handle of 1st child within window
	ChildHandle = GetWindow(DialogHandle,GW_CHILD)

	' Loop through dialog objects until we find child handle
	' that equals the style of a dialog Bitmap.
	Do
		If GetWindowLong(ChildHandle,GWL_STYLE) = 1342308355 Or _
           GetWindowLong(ChildHandle,GWL_STYLE) = 1342177283 Then
		   FoundBitmap = True						
		   Exit Do									
		End If
		ChildHandle = GetWindow(ChildHandle,GW_HWNDNEXT)
	Loop While ChildHandle <> Null

	If FoundBitmap = True Then

		FoundBitmap = False

		' Retrieve Handle of Bitmap
		NewIconHandle = SendMessage(ChildHandle,STM_GETICON,0,0)

		' Find match to determine type of Bitmap
		OldIconHandle = LoadIcon(0,IDI_APPLICATION)
		If OldIconHandle = NewIconHandle Then
			GetDialogBitmap = "Application"
			FoundBitmap = True
		End If

		OldIconHandle = LoadIcon(0,IDI_HAND)
		If OldIconHandle = NewIconHandle Then
			GetDialogBitmap = "Critical Stop"
			FoundBitmap = True
		End If

		OldIconHandle = LoadIcon(0,IDI_QUESTION)
		If OldIconHandle = NewIconHandle Then
			GetDialogBitmap = "Question Mark"
			FoundBitmap = True
		End If
	
		OldIconHandle = LoadIcon(0,IDI_EXCLAMATION)
		If OldIconHandle = NewIconHandle Then
			GetDialogBitmap = "Exclamation Point"
			FoundBitmap = True
		End If
					
		OldIconHandle = LoadIcon(0,IDI_ASTERISK)
		If OldIconHandle = NewIconHandle Then
			GetDialogBitmap = "Information"
			FoundBitmap = True
		End If

		If FoundBitmap = False Then
			GetDialogBitmap = "Undefined"
		End If

	Else
		GetDialogBitmap = "NotFound"
	End If

End Function
'*****************************************************
' EOF End Of GenAPI.INC
'*****************************************************
'$ENDIF _GenAPI_INCLUDED
