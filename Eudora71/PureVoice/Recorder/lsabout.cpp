// for easter egg
#include "stdafx.h"
#include "QVoice.h"
#include "CSndBuf.h"
#include "CWaveCon.h"
#include "CConvert.h"
#include "LsStateAction.h"
#include "QVoiceDoc.h"
#include "QPL.h"

void LsAbout()
{
	char* data;
	DWORD size;
	HRSRC res;
	HGLOBAL hResData;
	HMODULE hMod = AfxGetResourceHandle();
	DWORD resId = MAKELONG( IDR_ABOUT, 0 );
	res = FindResource( hMod, LPCTSTR(resId), RT_RCDATA );
	if ( !res ) return;
	size = SizeofResource( hMod, res );
	if ( !size ) return;
	hResData = LoadResource( hMod, res );
	data = (char*)LockResource( hResData );
	if ( !data ) return;
	CSndBuffer srcBuf( TRUE, size );
	srcBuf.Put( data, size );
	POSITION pos = AfxGetApp()->GetFirstDocTemplatePosition();
	CDocTemplate *dt = AfxGetApp()->GetNextDocTemplate( pos );
	pos = dt->GetFirstDocPosition();
	CQVoiceDoc *doc = (CQVoiceDoc*)dt->GetNextDoc( pos );
	LsStateAction *a = doc->StateAction();
	t_qpl_codec_info codecInfo;
	codecInfo.Id = CID_Qcelp13k;
	CACodec *pCodec = a->m_PlugIn.getCodec( codecInfo );
	if ( !pCodec ) return;
	CWaveOutDecode waveOut( &srcBuf, NULL, pCodec );
	waveOut.Play();
	delete pCodec;
}