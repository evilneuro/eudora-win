// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	secall.h - Mega header file.
//  Created:		Tue Aug 29 14:08:25 1995
//
// CHANGELOG: 
//
//    MSW     9/30/95  Added SECRegistry and SECRandom
//
//

#ifndef __SECALL_H__
#define __SECALL_H__

// OT needs 8 byte packing for structures
#if _MFC_VER >= 0x0300
// pshpack8.h saves the current #pragma pack value
#include /* */ "pshpack8.h"
#else
#pragma pack(8)
#endif

//Needed by SECTNFileDialog
#ifndef __AFXDLGS_H__
#include /* */ <afxdlgs.h>
#endif


//Needed by SECRegistry
//
#ifdef WIN32
#ifndef _WINREG_
#include /* */ <winreg.h>
#endif
#endif //WIN32


// version info

#include /* */ "secver.h"

// resources
#include /* */ "secres.h"

// dll support
#ifndef __SECDLL_H__
#include /* */ "secdll.h"
#endif //_SECDLL_H_

//  *************** View Classes ******************
// SECZoomView
#ifndef __SECZMVW_H__
#include /* */ "seczmvw.h"
#endif //__SECZMVW_H__

// SECPanView
#ifndef __SECPANVW_H__
#include /* */ "secpanvw.h"
#endif //__SECPANVW_H__

// SECPanWnd - The OverView Window
#ifndef __SECPANWN_H__
#include /* */ "secpanwn.h"
#endif //__SECPANWN_H__

//  *************** Image Classes ******************

// SECImage - ABC for all images
#ifndef __SECIMAGE_H__
#include /* */ "secimage.h"
#endif //__SECIMAGE_H__

// SECDib
#ifndef __SECDIB_H__
#include /* */ "secdib.h"
#endif //_SECDIB_H__

// SECGif
#ifndef _SECGIF_H_
#include /* */ "secgif.h"
#endif //__SECGIF_H__

// SECJpeg
#ifndef __SECJPEG_H__
#include /* */ "secjpeg.h"
#endif //__SECJPEG_H__

// SECPcx
#ifndef __SECPCX_H__
#include /* */ "secpcx.h"
#endif //__SECPCX_H__

// SECTga
#ifndef __SECTARGA_H__
#include /* */ "sectarga.h"
#endif //__SECTARGA_H__

#ifndef WIN32
#pragma warning(disable : 4103)
#endif

// SECTiff
#ifndef __SECTIFF_H__
#include /* */ "sectiff.h"
#endif //__SECTIFF_H__

#ifndef WIN32
#pragma warning(default : 4103)
#endif

//  *************** MDI Alternatives ******************

// SECToplevelFrame
// MTI class
#ifndef __SECTLF_H__
#include /* */ "sectlf.h"
#endif //__SECTLF_H__

// SECWorksheet, SECWorkbookClient, SECWorkbook
// WDI classes
#ifndef __SECWB_H__
#include /* */ "secwb.h"
#endif //__SECWB_H__

//  *************** Window/Control classes ******************

// SECBitmapDialog
#ifndef __SECBMPDG_H__
#include /* */ "secbmpdg.h"
#endif //__SECBMPDG_H__

// SECMaskEdit
#ifndef __SECMASKEDIT_H__
#include /* */ "secmedit.h"
#endif //__SECMASKEDIT_H__


// SECSplashWnd
#ifndef __SECSPLSH_H__
#include /* */ "secsplsh.h"
#endif //__SECSPLSH_H__

// SECTabControlBase
#ifndef __TABCTRLB_H__
#include /* */ "tabctrlb.h"
#endif //__TABCTRLB_H__

// SECTabWndBase
#ifndef __TABWNDB_H__
#include /* */ "tabwndb.h"
#endif //__TABWNDB_H__

// SECTabControl
#ifndef __TABCTRL_H__
#include /* */ "tabctrl.h"
#endif //__TABCTRL_H__

// SECTabWnd
#ifndef __TABWND_H__
#include /* */ "tabwnd.h"
#endif //__TABWND_H__

// SEC3DTabControl
#ifndef __TABCTRL3_H__
#include /* */ "tabctrl3.h"
#endif //__TABCTRL3_H__

// SEC3DTabWnd
#ifndef __TABWND3_H__
#include /* */ "tabwnd3.h"
#endif //__TABWND3_H__

// SECTipOfDay
#ifndef __SECTOD_H__
#include /* */ "sectod.h"
#endif //__SECTOD_H__

//  *************** Thumbnail classes ******************

//SECTNBitmap
#ifndef __SECTNBITMAP_H__
#include /* */ "sectnbmp.h"
#endif //__SECTNBITMAP_H__

//SECTNDC
#ifndef __SECTNDC_H__
#include /* */ "sectndc.h"
#endif // __SECTNDC_H__

//SECTNDocument
#ifndef __SECTNDOC_H__
#include /* */ "sectndoc.h"
#endif //__SECTNDOC_H__

//SECTNFileDialog
#ifndef __SECTNFILEDLG_H__
#include /* */ "sectndlg.h"
#endif // __SECTNFILEDLG_H__


//SECTNView
#ifndef __SECTNVW_H__
#include /* */ "sectnvw.h"
#endif // __SECTNVW_H__

//SECTNWinApp
#ifndef __SECTNWIN_H__
#include /* */ "sectnwin.h"
#endif //__SECTNWIN_H__


//  *************** Utility classes ******************

// SECRegistry
#ifndef __SECREG_H__
#include /* */ "secreg.h"
#endif //__SECREG_H__

// SECRandom
#ifndef __SECRAND_H__
#include /* */ "secrand.h"
#endif //__SECRAND_H__

//  *************** Window Control classes ******************

// SECColorWell
#ifndef __SECWELL_H__
#include /* */ "secwell.h"
#endif // __SECWELL_H__

// SECBtns
#ifndef __SECBTNS_H__
#include /* */ "secbtns.h"
#endif // __SECBTNS_H__

// SECFileSystem
#ifndef __SECFILESYSTEM_H__
#include /* */ "filesys.h"
#endif //__SECFILESYSTEM_H__

// SECCryptFile
#ifndef __CRYPTOFIL_H__
#include /* */ "cryptfil.h"
#endif   //__CRYPTOFIL_H__

// SECShortcuts
#ifndef __SCUTACCL_H__
#include /* */ "scutaccl.h"
#endif //__SCUTACCL_H__

#ifndef __SCUTCMDS_H__
#include /* */ "scutcmds.h"
#endif //__SCUTCMDS_H__

#ifndef __SCUTDIALOG_H__
#include /* */ "scutdlg.h"
#endif //__SCUTDIALOG_H__

#ifndef __SCUTKDIALOG_H__
#include /* */ "scutkdlg.h"
#endif //__SCITKDIALOG_H__

// Internationalization
#ifndef __SLOCALE_H__
#include "slocale.h"
#endif // __SLOCALE_H__

// SECCalendar
#ifndef __SEC_CALENDAR_H__
#include /* */ "calendar.h"
#endif //__SEC_CALENDAR_H__

// SECCompressFile
#ifndef __SECCOMPRESS_FILE_H__
#include /* */ "seccomp.h"
#endif //__SECCOMPRESS_FILE_H__

// Files related to docking windows
#ifndef __SBARDLG_H__
#include /* */ "sbardlg.h"
#endif //__SBARDLG_H__

#ifndef __SECBARDOCK_H__
#include /* */ "sbardock.h"
#endif //__SECBARDOCK_H__

#ifndef __SBARCORE_H__
#include /* */ "sbarcore.h"
#endif //__SBARCORE_H__

#ifndef __SBARTOOL_H__
#include /* */ "sbartool.h"
#endif //__SBARTOOL_H__

#ifndef __SBARSTAT_H__
#include /* */ "sbarstat.h"
#endif //__SBARSTAT_H__

#ifndef __SDOCKCNT_H__
#include /* */ "sdockcnt.h"
#endif //__SDOCKCNT_H__

#ifndef __SDOCKSTA_H__
#include /* */ "sdocksta.h"
#endif

#ifndef __SBARMGR_H__
#include /* */ "sbarmgr.h"
#endif //__SBARMGR_H__

#ifndef __SWINMDI_H__
#include /* */ "swinmdi.h"
#endif //__SWINMDI_H__

#ifndef __SWINFRM_H__
#include /* */ "swinfrm.h"
#endif //__SWINFRM_H__

#ifndef __SECAUX_H__
#include /* */ "secaux.h"
#endif //__SECAUX_H__

#ifndef __SECWSMGR_H__
#include /* */ "secwsmgr.h"
#endif //__SECWSMGR_H__

#ifndef __FDIMAIN_H__
#include /* */ "fdimain.h"
#endif //__FDIMAIN_H__

#ifndef __FDICHILD_H__
#include /* */ "fdichild.h"
#endif //__FDICHILD_H__

#ifndef __SBARCUST_H__
#include /* */ "sbarcust.h"
#endif //__SBARCUST_H__

#ifndef __BROWEDIT_H__
#include /* */ "browedit.h"
#endif

#ifndef __LSTBXEDT_H__
#include /* */ "lstbxedt.h"
#endif

#ifndef __LSTBXCLR_H__
#include /* */ "lstbxclr.h"
#endif

#ifndef __USERTOOL_H__
#include /* */"usertool.h"
#endif

#ifndef __USRTLDLG_H__
#include /* */ "usrtldlg.h"
#endif

//  *************** Customizable Toolbar ******************

#ifndef __TBARCUST_H__
#include /* */ "tbarcust.h"
#endif

#ifndef __TBARMGR_H__
#include /* */ "tbarmgr.h"
#endif

#ifndef __TBARPAGE_H__
#include /* */ "tbarpage.h"
#endif

#ifndef __TBARSDLG_H__
#include /* */ "tbarsdlg.h"
#endif

#ifndef __TBARTRCK_H__
#include /* */ "tbartrck.h"
#endif

#ifndef __TBTN2PRT_H__
#include /* */ "tbtn2prt.h"
#endif

#ifndef __TBTNCMBO_H__
#include /* */ "tbtncmbo.h"
#endif

#ifndef __TBTNSTD_H__
#include /* */ "tbtnstd.h"
#endif

#ifndef __TBTNWND_H__
#include /* */ "tbtnwnd.h"
#endif

//  ********** Enhanced Win32 List Control *************

#ifndef __SLSTCTL_H__
#include /* */ "slstctl.h"
#endif

#ifndef __SLSTCTIP_H__
#include /* */ "slstctip.h"
#endif

//  ********** Enhanced Win32 Tree Control *************

#ifndef __TREENODE_H__
#include /* */ "TreeNode.h"
#endif

#ifndef __TRCTLX_H__
#include /* */ "TrCtlX.h"
#endif

/////////////////////////////////////////////////////////////
// Smart Edits

#ifndef __DROPEDIT_H__
#include /* */ "dropedit.h"
#endif

// Date/Time controls
#ifdef WIN32
#ifndef __DTCTRL_H__
#include /* */ "dtctrl.h"
#endif // __DTCTRL_H__

#ifndef __DTGADGET_H__
#include /* */ "dtgadget.h"
#endif // __DTGADGET_H__
#endif	// WIN32

// Calculator edit
#ifndef __CALCBTN_H__
#include /* */ "calcbtn.h"
#endif

#ifndef __SECCALC_H__
#include /* */ "seccalc.h"
#endif

// Currency edit
#ifndef __CURREDIT_H__
#include /* */ "curredit.h"
#endif

#if _MFC_VER >= 0x0300
// poppack.h restores the original #pragma pack value
#include /* */ "poppack.h"
#else
// restore default packing value
#pragma pack()
#endif

#endif // __SECALL_H__

