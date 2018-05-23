///////////////////////////////////////////////////////////////////////////////
//
// os.h
//
// Created: 02/03/99, Scott Manjourides
//
// Removed os.rc, moved non-rc stuff here.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _OS_H_
#define _OS_H_

// --------------------------------------------------------------------------

#define	SLASH		'\\'
#define	SLASHSTR	"\\"

#define GET_WM_COMMAND_CMD(wp, lp) 	HIWORD(wp)
#define	GET_WM_COMMAND_ID(wp, lp) 	LOWORD(wp)
#define	GET_WM_COMMAND_HWND(wp, lp) (HWND)(lp)

// --------------------------------------------------------------------------

#endif // _OS_H_
