// imgui.HPP
//
// Various interface utilities
//

#include <stdarg.h>

/////////////////////////////////////////////////////////////////////////////
// AlertDialog() displays an message and asks the user to hit a button
//
void CheckForAutoOk( BOOL bOn );
int AlertDialog(UINT DialogID, ...);

// win32 specific
#define GET_WM_COMMAND_CMD(wp, lp) 	HIWORD(wp)
#define	GET_WM_COMMAND_ID(wp, lp) 	LOWORD(wp)
#define	GET_WM_COMMAND_HWND(wp, lp) (HWND)(lp)


