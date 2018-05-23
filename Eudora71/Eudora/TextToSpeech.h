// TextToSpeech.h : header file
//
// Copyright (c) 1999-2000 by QUALCOMM, Incorporated
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

//

#ifndef EUD_TEXT_TO_SPEECH_H_
#define EUD_TEXT_TO_SPEECH_H_

class CVoiceText;

enum { SPEAK_WHO=1, SPEAK_SUBJECT=2, SPEAK_ALL=4 };

class CTTSHelper {
public:
	bool bInit;
	CVoiceText *pCVTxt;

	bool Initialize();
	HRESULT Speak(wchar_t *str, GUID vid);
	bool FillComboBox( CComboBox *pCombo, const char* strSel, int *pSel);

	CTTSHelper(){ bInit = false; pCVTxt = NULL;}
	~CTTSHelper();
};


bool FillComboWithVoices(CComboBox *pCombo, const char* sel, int *pSel);
int TextToSpeech(const char *strSpeakMe, const char* strGUID);
int SpeakFilterText(int nSpeakOptions, const char* strGUID, const char* strWho, const char* strSubject, BOOL bOutgoing = FALSE);

#endif
