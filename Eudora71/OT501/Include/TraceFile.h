
#ifndef _TraceFile_h_
#define _TraceFile_h_

// -----------------------------------------------------------------------------
// Tracing / debug global functions

#ifdef _DEBUG
void DMsg(LPCSTR s);
void TraceStart(void);
void TraceMsg(LPSTR string);
void GetTextFromFile(LPCSTR filename, CString& c);
#else
#define DMsg(s)
#define TraceStart()
#define TraceMsg(s)
#define GetTextFromFile(f, c)
#endif // _DEBUG

#endif // _TraceFile_h_
