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
	bool FillComboBox( CComboBox *pCombo, CString strSel, int *pSel);

	CTTSHelper(){ bInit = false; pCVTxt = NULL;}
	~CTTSHelper();
};


bool FillComboWithVoices(CComboBox *pCombo, CString sel, int *pSel);
int TextToSpeech(const char *strSpeakMe, CString strGUID);
int SpeakFilterText(int nSpeakOptions, CString strGUID, CString strWho, CString strSubject);

#endif
