// ihole.h -- IIconHelper interface

// initguid.h makes the DEFINE_GUID macro do the right things
//#include "initguid.h"

#ifndef _IHOLE_H_
#define _IHOLE_H_

//////////////////////////////////////////////////////////////////////////////
// IIconHelperCallback -- the IconHelperCallback interface definition


// here's the IconHelperCallback interface. it is called whenever a callback
// helper becomes active (i.e. when the user double clicks it).
DECLARE_INTERFACE_(IIconHelperCallback, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * lplpObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

	STDMETHOD(OnActivate)(THIS) PURE;
};

// here's a pointer to an IconHelper interface
typedef IIconHelperCallback FAR * LPICONHELPERCALLBACK;

// IconHelperCallback interface GUID {2D0B6F90-FF70-11cf-9A81-00805FD2626C}
DEFINE_GUID(IID_IIconHelperCallback, 
	0x2d0b6f90, 0xff70, 0x11cf, 0x9a, 0x81, 0x0, 0x80, 0x5f, 0xd2, 0x62, 0x6c);

//////////////////////////////////////////////////////////////////////////////
// IIconHelper -- the IconHelper interface definition


// this is the function signature required by a callback helper
typedef BOOL (CALLBACK* ICONHELPERCALLBACK)(DWORD dwCookie);


// here's the IconHelper interface. it can be used to create either
// CallbackHelpers, or FileLinkHelpers
DECLARE_INTERFACE_(IIconHelper, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lplpObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IIconHelper methods ***
	STDMETHOD(CreateCallbackHelper)(THIS_ HICON hIcon, CSize& sizeIcon,
				CString& captionText, LPICONHELPERCALLBACK pIntfcCallback,
				DWORD dwCookie, INT nAlign) PURE;

	STDMETHOD(CreateFileLinkHelper)(THIS_ CString& fullPath,
				LPICONHELPERCALLBACK pIntfcCallback, INT nAlign) PURE;
};

// here's a pointer to an IconHelper interface
typedef IIconHelper FAR * LPICONHELPER;

// IconHelper interface GUID {608307E0-EBE9-11cf-9A6E-00805FD2626C}
DEFINE_GUID(IID_IIconHelper, 
	0x608307e0, 0xebe9, 0x11cf, 0x9a, 0x6e, 0x0, 0x80, 0x5f, 0xd2, 0x62, 0x6c);

/////////////////////////////////////////////////////////////////////////////
#endif		// _IHOLE_H_
