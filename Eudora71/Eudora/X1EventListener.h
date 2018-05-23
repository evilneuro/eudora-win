//	X1EventListener.h
//
//	Responds to events from X1.
//
//	Copyright (c) 2005 by QUALCOMM, Incorporated
/* Copyright (c) 2016, Computer History Museum 
All rights reserved. 
Redistribution and use in source and binary forms, with or without modification, are permitted (subject to 
the limitations in the disclaimer below) provided that the following conditions are met: 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided with the distribution. 
 * Neither the name of Computer History Museum nor the names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission. 
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE. */



#ifndef __X1EventListener_H__
#define __X1EventListener_H__

#include <memory>

//	X1 Include
#include "X1Wrapper.h"


//	X1EventListener and X1DbEventListener implementation. 
class X1EventListener : public IUnknown
{
  public:
	  //	Forward declarations
	class X1GeneralEventListener;
	class X1DBEventListener;


											X1EventListener();
	virtual									~X1EventListener();

	virtual HRESULT __stdcall				QueryInterface(
												const IID &					in_iid,
												void **						out_ppvObject);
		//	Standard IUnknown::QueryInterface() implementation

	virtual ULONG __stdcall					AddRef();
		//	Standard IUnknown::AddRef() implementation

	virtual ULONG __stdcall					Release();
		//	Standard IUnknown::Release() implementation

  protected:
	long									m_nRefCount;
	std::auto_ptr<X1GeneralEventListener>	m_pIX1EventListener;
	std::auto_ptr<X1DBEventListener>		m_pIX1DBEventListener;
};


#endif // __X1EventListener_H__
