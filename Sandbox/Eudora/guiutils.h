// GUIUTILS.HPP
//
// Various interface utilities
//

#include <stdarg.h>

/////////////////////////////////////////////////////////////////////////////
// AlertDialog() displays an message and asks the user to hit a button
//
void CheckForAutoOk( BOOL bOn );
int AlertDialog(UINT DialogID, ...);

/////////////////////////////////////////////////////////////////////////////
// ErrorDialog() displays an error message and asks the user to hit the OK button
//
void ErrorDialog(UINT StringID, ...);

/////////////////////////////////////////////////////////////////////////////
// WarnDialog() displays a warning and asks the user whether or not they want
// to continue.  There's also a checkbox in the dialog allowing the user to
// never see the warning again.
//
int WarnDialog(UINT IniID, UINT StringID, ...);
int WarnYesNoDialog(UINT IniID, UINT StringID, ...);
int WarnOneOptionCancelDialog(UINT IniID, UINT StringID, UINT BtnTxtID, ...);
int WarnTwoOptionsCancelDialog(UINT IniID, UINT StringID, UINT button1, UINT button2,  ...);


/////////////////////////////////////////////////////////////////////////////
// NoteDialog() displays a message, giving the user the chance to
// confirm the message.  There's also a checkbox in the dialog
// allowing the user to never see the message again.
//
int NoteDialog(UINT IniID, UINT StringID, ...);

/////////////////////////////////////////////////////////////////////////////
// YesNoDialog() displays a message, giving the user the chance to answer yes or no
// to the message. There's also a checkbox in the dialog
// allowing the user to never see the message again.  The response is stored if the
// latter case is used
//
int YesNoDialog(UINT WarnID, UINT responseID, UINT StringID, ...);

// A Yes/no dialog without an INI dependancies or checkbox
int YesNoDialogPlain(UINT StringID, ...);

// Try to open the given filename with its associated app or users choice
BOOL OpenFile(const char* Filename);


// Some functions for telling us about keyboard presses
//
inline BOOL ShiftDown()
  	{ return (GetKeyState(VK_SHIFT) < 0? TRUE : FALSE); }
inline BOOL AsyncShiftDown()
  	{ return (GetAsyncKeyState(VK_SHIFT) < 0? TRUE : FALSE); }
inline BOOL CtrlDown()
  	{ return (GetKeyState(VK_CONTROL) < 0? TRUE : FALSE); }
inline BOOL AsyncCtrlDown()
  	{ return (GetAsyncKeyState(VK_CONTROL) < 0? TRUE : FALSE); }
int EscapePressed(int Repost = FALSE);
BOOL AsyncEscapePressed(BOOL bReset = FALSE);

void HesiodErrorDialog(int nHesiodError);

int GetGMTOffset();
BOOL LaunchURL( LPCSTR szURL );
BOOL URLIsValid( LPCSTR	szURL );
CString EscapeURL( LPCSTR	szURL );
CString UnEscapeURL( LPCSTR	szURL );
CString EscapePercentsInURL( LPCTSTR szIn);

BOOL FitTransparentBitmapToHeight(CBitmap& theBitmap, int nTargetHeight);


// MAPI install/uninstall routines with error reporting
#include "mapiinst.h"
CMapiInstaller::Status MAPIInstall();
CMapiInstaller::Status MAPIUninstall(BOOL bShuttingDown = FALSE);

