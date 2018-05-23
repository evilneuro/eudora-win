// FaceTime.cpp -- "Face Time" measurement
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

#include "stdafx.h"

#include <stdlib.h>
#include <time.h>

// now include our class header
#include "FaceTime.h"

#include <list>


#include "DebugNewHelpers.h"


// we'll define this privately so that those who include our header file
// won't have to suffer Microsoft's warning spew.
using namespace std;
typedef list<QCFaceTime*> FaceTimeList;

// all existing facetime objects can be found here
static FaceTimeList g_faceTimeList;

int QCFaceTime::s_appState = ASEInactive;
bool QCFaceTime::s_connected = false;
time_t QCFaceTime::s_startInterval = 0;


////////////////////////////////////////////////////////////////////////////////
// 'C' api for monkey-ing about with Face Time

// this is pretty much the same as the mac version, except that our handles
// are not double-ptrs, and we don't use globals for things like the current
// state of the application.

// handy-dandy cast macros
#define from_handle(h) (reinterpret_cast<QCFaceTime*>(h))
#define to_handle(h) (reinterpret_cast<long>(h))


// allocate a new face time block
FMBHANDLE NewFaceMeasure( void )
{
	return to_handle( QCFaceTime::Create() );
}


// smoke a face time block 
int DisposeFaceMeasure( FMBHANDLE fmb )
{
	QCFaceTime* pft = from_handle( fmb );
	ASSERT( pft );

	if ( pft ) {
		pft->Dispose();
		return ecOK;
	}

	return ecFailed;
}


// start keeping track of face time
int FaceMeasureBegin( FMBHANDLE fmb )
{
	QCFaceTime* pft = from_handle( fmb );
	ASSERT( pft );

	if ( pft ) {
		pft->Start();
		return ecOK;
	}

	return ecFailed;
}


// stop keeping track of face time
int FaceMeasureEnd( FMBHANDLE fmb )
{
	QCFaceTime* pft = from_handle( fmb );
	ASSERT( pft );

	if ( pft ) {
		pft->Stop();
		return ecOK;
	}

	return ecFailed;
}

// do-over!
int FaceMeasureReset( FMBHANDLE fmb )
{
	QCFaceTime* pft = from_handle( fmb );
	ASSERT( pft );

	if ( pft ) {
		pft->Reset();
		return ecOK;
	}

	return ecFailed;
}


// cause the fmb to be udated based on provided app state
int FaceMeasureUpdate( int appState, bool bConnected )
{
	QCFaceTime::Update( appState, bConnected );
	return ecOK;
}


// have a gander at the guts of a face time block
int FaceMeasureReport( FMBHANDLE fmb, time_t *faceTime, time_t *rearTime, 
		time_t *connectTime, time_t *totalTime )
{
	QCFaceTime* pft = from_handle( fmb );
	ASSERT( pft );
	
	if ( pft ) {
		// Must update before reporting!
		pft->Update();
		
		// Now the values are accurate.
		if (faceTime) *faceTime = pft->FaceTime();
		if (rearTime) *rearTime = pft->RearTime();
		if (connectTime) *connectTime = pft->ConnectTime();
		if (totalTime) *totalTime = pft->TotalTime();
		return ecOK;
	}

	return ecFailed;
}


////////////////////////////////////////////////////////////////////////////////
// QCFaceTime -- C++ object based on FaceMeasureBlock

// the only way to create a QCFaceTime object. calls new for ya, and adds
// the object to our global list of facetime objects.
/* static */ QCFaceTime* QCFaceTime::Create()
{
	QCFaceTime* pft = DEBUG_NEW_NOTHROW QCFaceTime;

	if ( pft )
		g_faceTimeList.push_back( pft );

	return pft;
}


// the only way to get rid of one
void QCFaceTime::Dispose()
{
	Stop();
	m_living = false;

	FaceTimeList::iterator it;
	
	for ( it = g_faceTimeList.begin(); it != g_faceTimeList.end(); it++ )
	{
		if ( this == *it )
		{
			g_faceTimeList.erase( it );
			delete this;
			break;
		}
	}
}


// make my face!
QCFaceTime::QCFaceTime()
{
	QCFaceTime::Reset();
}


// start keeping face time
void QCFaceTime::Start()
{
	ASSERT( this );
	startTime = gmtTime();
	m_active = true;

	if ( !absStartTime )
		absStartTime = startTime;
}


// stop keeping face time
void QCFaceTime::Stop()
{
	ASSERT( this );

	Update();
	endTime = gmtTime();
	m_active = false;
}

// start face time over
void QCFaceTime::Reset()
{
	ASSERT( this );

	absStartTime = 0;
	startTime = 0;
	endTime = 0;
	faceTime = 0;
	rearTime = 0;
	connectTime = 0;

	m_active = false;
	m_living = true;
}


// update face time based on app state. bConnected indicates whether or not
// we currently have a net connection.
// if bForceUpdate is set to true, then facetime WILL be updated.
/* static */ void QCFaceTime::Update( int appState, bool bConnected, bool bForceUpdate /* = false */ )
{
	if ( (bForceUpdate)  || 
		 (s_appState!=appState || s_connected!=bConnected) )
	{
		time_t gmtSecs = gmtTime();

#ifdef DEBUG
#define ASE_NAME(s) (s==ASEActive ? "ASEActive" : s==ASEInactive ? "ASEInactive" : s==ASEBackground ? "ASEBackground" : "Got Me")	
		TRACE("QCFTU: as %s->%s cn %d->%d diff %d\n",
			ASE_NAME(s_appState), ASE_NAME(appState),
			s_connected, bConnected,
			gmtSecs-s_startInterval);
#endif

		FaceTimeList::iterator it;
		
		for ( it = g_faceTimeList.begin(); it != g_faceTimeList.end(); )
		{
			if ( (*it)->IsLiving() )
			{
				(*it)->Update();
				it++;
			}
			else
			{
				delete *it;
				it = g_faceTimeList.erase( it );
			}
		}
		
		s_appState = appState;
		s_connected = bConnected;
		s_startInterval = gmtSecs;
	}
}


// everthing in a face time block is gmt
/*static*/ time_t QCFaceTime::gmtTime()
{
	time_t t = 0;
	time( &t );
	return mktime( gmtime( &t ) );
}


void QCFaceTime::Update()
{
	if ( m_active ) {
		time_t sex = gmtTime();
		time_t startSex = max( startTime, s_startInterval );

		switch( s_appState ) {
			case ASEInactive:
				break;
			case ASEActive:
				faceTime += sex - startSex;
				break;
			case ASEBackground:
				rearTime += sex - startSex;
				break;
			default:
				ASSERT( 0 );   // dude!
		}

		if ( s_connected )
			connectTime += sex - startSex;

		startTime = sex;
	}
}


////////////////////////////////////////////////////////////////////////////////
// test stub

#ifdef _TESTIES_
#include "stdio.h"

#define MAX_FMBS 10
#define report_spec "faceTime: %lu\nrearTime: %lu\nconnectTime: %lu\n"\
			"totalTime: %lu\n"

int main( void )
{
	puts( "Hello Face Time!" );
	FMBHANDLE fmbs[MAX_FMBS];

	for ( int i = 0; i < MAX_FMBS; i++ )
		fmbs[i] = NewFaceMeasure();

	if ( *fmbs ) {
		for ( int i = 0; i < MAX_FMBS; i++ )
			FaceMeasureBegin( fmbs[i] );

		for ( ;; ) {
			time_t faceTime = 0;
			time_t rearTime = 0;
			time_t connectTime = 0;
			time_t totalTime = 0;

			// pick a number between one and ten
			int j = 1 + (int)(10.0 * rand() / (RAND_MAX + 1.0));

			if ( j <= 5 )
				FaceMeasureUpdate( ASEBackground, false );
			else
				FaceMeasureUpdate( ASEActive, true );
			
			if ( getchar() == 'q' ) {
				break;
			}
			else {
				for ( int i = 0; i < MAX_FMBS; i++ ) {
					FaceMeasureReport( fmbs[i], &faceTime, &rearTime,
							&connectTime, &totalTime );
					printf( report_spec, faceTime, rearTime, connectTime,
							totalTime );
				}
			}

			// make a random facetime object go inactive
			FaceMeasureEnd( fmbs[--j] );
		}
	}

	for ( i = 0; i < MAX_FMBS; i++ ) {
		FaceMeasureEnd( fmbs[i] );
		DisposeFaceMeasure( fmbs[i] );
	}

	return 0;
}
#endif
