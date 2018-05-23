// FaceTime.h -- "Face Time" measurement
//
// Copyright (c) 1999 by QUALCOMM, Incorporated
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

#ifndef _FACE_TIME_H_
#define _FACE_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// basic Face Time object -- differs from the Mac impl only in that it does
// not include support for linked lists. choose your own collection, homey.

typedef struct FaceMeasureBlock
{
	time_t absStartTime;
	time_t startTime;
	time_t endTime;
	time_t faceTime;
	time_t rearTime;
	time_t connectTime;
} FaceMeasureBlock;


////////////////////////////////////////////////////////////////////////////////
// 'C' api for monkey-ing about with Face Time

enum { ecOK = 0, ecFailed = -1 };
enum { ASEInactive, ASEActive, ASEBackground };

#define kFaceIntervalAfter 60	// number of seconds after user action
								// we will consider him still active

typedef long FMBHANDLE;

#ifndef __cplusplus
typedef int bool;
#define true  1
#define false 0
#endif

FMBHANDLE NewFaceMeasure( void );
int DisposeFaceMeasure( FMBHANDLE fmb );
int FaceMeasureBegin( FMBHANDLE fmb );
int FaceMeasureEnd( FMBHANDLE fmb );
int FaceMeasureReset( FMBHANDLE fmb );
int FaceMeasureUpdate( int appState, bool bConnected );
int FaceMeasureReport( FMBHANDLE fmb, time_t *faceTime, time_t *rearTime, 
		time_t *connectTime, time_t *totalTime );

#ifdef __cplusplus
}   // end extern "C" 


////////////////////////////////////////////////////////////////////////////////
// QCFaceTime -- C++ object based on FaceMeasureBlock

class QCFaceTime : private FaceMeasureBlock
{
	static int s_appState;
	static bool s_connected;
	static time_t s_startInterval;

	bool m_active;
	bool m_living;

public:
	time_t FaceTime() {return faceTime;} 
	time_t RearTime() {return rearTime;}
	time_t ConnectTime() {return 30*((connectTime+15)/30);}
	time_t TotalTime() {return (m_active ? gmtTime() : endTime) - absStartTime;}

	void Start();
	void Stop();
	void Reset();

	static void Update( int appState, bool bConnected , bool bForceUpdate = false);

	bool IsActive() {return m_active;}
	bool IsLiving() {return m_living;}

	static QCFaceTime* Create();
	void Dispose();
	void Update();

private:
	QCFaceTime();
	~QCFaceTime(){}

	static time_t gmtTime();
};


#endif   // __cplusplus only

#endif   // _FACE_TIME_H_
