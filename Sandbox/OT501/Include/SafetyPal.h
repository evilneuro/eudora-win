// SafetyPalette.h
//

#ifndef _SafetyPalette_h_
#define _SafetyPalette_h_

bool SetSafetyPalette(int SafetyPaletteResource);
bool SetMirrorPalette(void);
void FreePalettes(void);


class CWnd;

class CSafetyPalette
{
public:
	CSafetyPalette();
	~CSafetyPalette();
	void Create(bool bUseMirrorPalette); // called in constructor but ok to call again
	void Free(void);
	void Select(HDC hdc, bool bBackground = false);
	void Unselect(void);
	void Realize(HDC hdc, int* pNumColors = NULL);
	HPALETTE GetPalette(void);
	bool DoQueryNewPalette(CWnd* pThisWindow);
	void DoPaletteChanged(CWnd* pThisWindow, CWnd* pFocusWindow);
	void DoSysColorChange(CWnd* pThisWindow);
	bool IsUsingSafetyPalette(void) { return m_bCurrentPaletteIsSafetyPalette; }
	bool IsUsingMirrorPalette(void) { return m_bCurrentPaletteIsMirrorPalette; }
protected:
	bool IsPaletteDevice(HDC hdc);
	HPALETTE m_LasthPalette;
	HDC m_LastHDC;
	bool m_bCurrentPaletteIsSafetyPalette;
	bool m_bCurrentPaletteIsMirrorPalette;
	bool m_bMyPaletteSelected;
};

class CPaletteDC : public CDC
{
public:
	CPaletteDC();
	virtual ~CPaletteDC();
	BOOL CreateCDC(void);
	void Realize(int* pNumColors = NULL);
	BOOL BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC, int xSrc, int ySrc, DWORD dwRop);
	BOOL BitBltTransparent(int x, int y, int nWidth, int nHeight, CDC* pSrcDC, int xSrc, int ySrc, COLORREF xcolor);
	BOOL CreateCompatibleDC(CDC* pDC);
protected:
	CSafetyPalette m_Palette;
	bool bSelected;
};

extern PALETTEENTRY SafetyPaletteArray[256];
extern PALETTEENTRY MirrorPaletteArray[256];
extern HPALETTE ghPaletteSafety;
extern HPALETTE ghPaletteMirror;
extern bool gbSafetyPaletteReady;
extern bool gbMirrorPaletteReady;
extern bool gbUseMirrorPalette; // Set by app if it detects Mirror palette
extern bool gbMirrorPaletteChanged;

#endif // _SafetyPalette_h_

