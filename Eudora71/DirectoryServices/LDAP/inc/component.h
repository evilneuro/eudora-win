// component.h
#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "protocol.h"

/*
class CGenericProtocol : public CProtocol
{
public:
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HICON		__stdcall GetProtocolImage();
	virtual DWORD		__stdcall GetProtocolFlags();

protected:
	virtual HRESULT __stdcall CreateServer(IUnknown **ppunkNew);

private:
	// private members here
};


class CGenericServer : public CServer
{
	virtual HRESULT	__stdcall GetName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HICON		__stdcall GetImage();
	virtual HRESULT	__stdcall GetProtocolName(LPSTR lpszBuffer, UINT nBufferSize);
	virtual HICON		__stdcall GetProtocolImage();
	virtual int			__stdcall GetPropPageCount();
	virtual HRESULT	__stdcall GetPropPages(HPROPSHEETPAGE *paHPropPages);
};
*/


// Class GUIDs

// {DAC07220-02E5-11d1-886A-00805F4AB1BF}
#define CLASS_STR_DSPROTOCOLGEN  L"{DAC07220-02E5-11d1-886A-00805F4AB1BF}"
DEFINE_GUID(CLSID_DsProtocolGen, 0xdac07220, 0x2e5, 0x11d1, 0x88, 0x6a, 0x0, 0x80, 0x5f, 0x4a, 0xb1, 0xbf);


#endif	// __COMPONENT_H__

