// PROGRESS.H
//
// Routines for the Progress window
// 

#ifndef _PROGRESS_H_
#define _PROGRESS_H_


// Data Items

class CProgressWindow;
extern CProgressWindow* ProgWin;
extern BOOL InProgress;


// Function Prototypes

BOOL CanShowProgress();
void PushProgress();
void PopProgress();
void CloseProgress();
void MainProgress(const char* Message);
void CountdownProgress(const char* Message = NULL, int Number = -1);
void DecrementCountdownProgress();
void Progress(long SoFar, const char* Message = NULL, long Total = -1L);
void Progress(const char* Message);
void ProgressAdd(long Increment);

#endif
