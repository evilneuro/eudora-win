// swmapi.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "swmapi.h"

#include "dialog.h"
#include "aboutdlg.h"
#include "mapiinst.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSwitchApp

BEGIN_MESSAGE_MAP(CSwitchApp, CWinApp)
    //{{AFX_MSG_MAP(CSwitchApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSwitchApp construction

CSwitchApp::CSwitchApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSwitchApp object

CSwitchApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////
// CSwitchApp initialization

BOOL CSwitchApp::InitInstance()
{
    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

    if (m_lpCmdLine[0] != '\0')
    {
        //
		// The recognized command line switches are:
		//
		//		/install
		//		/uninstall
		//
		// Any other argument is treated as a program to be executed
		// via WinExec.  To support program pathnames with embedded
		// spaces or programs with command line arguments of their own,
		// you can wrap an argument with double quotes.
		//
		// IMPORTANT!  Command line arguments are processed and executed
		// in order from left-to-right.  If any command fails, the
		// remainder of the commands are not executed.
		//
		CString cmdline(m_lpCmdLine);		// make working copy
		cmdline.TrimLeft();

		while (! cmdline.IsEmpty())
		{
			//
			// Grab next token from the command line string.
			//
			CString arg;	// command line token
			if ('"' == cmdline[0])
			{
				//
				// Drop the opening quote.
				//
				cmdline = cmdline.Right(cmdline.GetLength() - 1);

				//
				// Look for the closing quote.
				//
				int idx = cmdline.Find('"');
				if (-1 == idx)
				{
					CString msg;
					msg.Format("ERROR: Unterminated quoted argument: %s", (const char *) cmdline);
					::AfxMessageBox(msg, MB_OK | MB_ICONSTOP);
					return FALSE;
				}
				else
				{
					arg = cmdline.Left(idx);
					cmdline = cmdline.Right(cmdline.GetLength() - (idx + 1));
				}
			}
			else
			{
				int idx = cmdline.Find(' ');
				if (-1 == idx)
				{
					arg = cmdline;
					cmdline.Empty();
				}
				else
				{
					arg = cmdline.Left(idx);
					cmdline = cmdline.Right(cmdline.GetLength() - (idx + 1));
				}
			}
			cmdline.TrimLeft();

			if (arg.CompareNoCase("/install") == 0)
			{
				//
				// Do install.
				//
				switch (CMapiInstaller::Install())
				{
				case CMapiInstaller::STATUS_SUCCESSFUL:
				case CMapiInstaller::STATUS_ALREADY_INSTALLED:
					break;
				case CMapiInstaller::STATUS_DLL_IN_USE:
					::AfxMessageBox("ERROR: Could not install Eudora MAPI components since a MAPI component is in use.  "
								    "Close all MAPI applications and try again.",
									MB_ICONSTOP | MB_OK);
					return FALSE;
				case CMapiInstaller::STATUS_FAILED:
				case CMapiInstaller::STATUS_INTERNAL_ERROR:		// Installer had internal error
					::AfxMessageBox("ERROR: Could not install Eudora MAPI components.",
									MB_ICONSTOP | MB_OK);
					return FALSE;
				case CMapiInstaller::STATUS_SOURCE_DLL_NOT_FOUND:
					::AfxMessageBox("ERROR: Could not find Eudora MAPI components to install.",
									MB_ICONSTOP | MB_OK);
					return FALSE;
				default:
					ASSERT(0);		// recognized status code
					return FALSE;
				}
			}
			else if (arg.CompareNoCase("/uninstall") == 0)
			{
				//
				// Do uninstall.
				//
				switch (CMapiInstaller::Uninstall())
				{
				case CMapiInstaller::STATUS_SUCCESSFUL:
				case CMapiInstaller::STATUS_NOT_INSTALLED:
					break;
				case CMapiInstaller::STATUS_DLL_IN_USE:
					::AfxMessageBox("ERROR: Could not uninstall Eudora MAPI components since a MAPI component is in use.  "
								    "Close all MAPI applications and try again.",
									MB_ICONSTOP | MB_OK);
					return FALSE;
				case CMapiInstaller::STATUS_FAILED:
				case CMapiInstaller::STATUS_INTERNAL_ERROR:
					::AfxMessageBox("ERROR: Could not uninstall Eudora MAPI components.",
									MB_ICONSTOP | MB_OK);
					return FALSE;
				default:
					ASSERT(0);
					return FALSE;
				}
			}
			else
			{
				//
				// Do WinExec() with the argument.
				//
				switch (WinExec(arg, SW_SHOWNORMAL))
				{
#ifdef WIN32
				case 0:						// The system is out of memory or resources.
				case ERROR_BAD_FORMAT:		// The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).
				case ERROR_FILE_NOT_FOUND:	// The specified file was not found.
				case ERROR_PATH_NOT_FOUND:	// The specified path was not found.
#else
				case 0:		// System was out of memory, executable file was corrupt, or relocations were invalid. 
				case 2:		// File was not found. 
				case 3:		// Path was not found. 
				case 5:		// Attempt was made to dynamically link to a task, or there was a sharing or network-protection error. 
				case 6:		// Library required separate data segments for each task. 
				case 8:		// There was insufficient memory to start the application. 
				case 10:	// Windows version was incorrect. 
				case 11:	// Executable file was invalid. Either it was not a Windows application or there was an error in the .EXE image. 
				case 12:	// Application was designed for a different operating system. 
				case 13:	// Application was designed for MS-DOS 4.0. 
				case 14:	// Type of executable file was unknown. 
				case 15:	// Attempt was made to load a real-mode application (developed for an earlier version of Windows). 
				case 16:	// Attempt was made to load a second instance of an executable file containing multiple data segments that were not marked read-only. 
				case 19:	// Attempt was made to load a compressed executable file. The file must be decompressed before it can be loaded. 
				case 20:	// Dynamic-link library (DLL) file was invalid. One of the DLLs required to run this application was corrupt. 
				case 21:	// Application requires Microsoft Windows 32-bit extensions. 
#endif // WIN32
					{
						CString msg;
						msg.Format("ERROR: Could not run program: %s", (const char *) arg);
						::AfxMessageBox(msg, MB_ICONSTOP | MB_OK);
					}
					return FALSE;
				default:
					break;
				}
			}
		}

		return FALSE;		// fool AFX into quitting immediately
    }

    SetDialogBkColor();        // set dialog background color to gray

    CSwitchDialog dlg;
    dlg.DoModal();

    //
    // Return FALSE to fool AFX into quitting immediately.
    //
    return FALSE;
}


// App command to run the dialog
void CSwitchApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CSwitchApp commands
