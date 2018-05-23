/* ======================================================================

    Eudora Regcode definitions
    Author: Laurence Lundblade <lgl@qualcomm.com>
    Original Date: March 31, 2000
    Copyright 2000 QUALCOMM Inc
 Copyright (c) 2016, Computer History Museum 
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
DAMAGE. 


    File: regcode_eudora.h

    THIS FILE DOES NOT DEFINE POLICY. YOU NEED TO READ ELSEWHERE AND 
    CONSULT THE ORACLE BEFORE USING THESE CODES 

    Please see the reg code read me file for the authoratitive
    descriptions of policies and such. This does not define them.
    It's for programmers convenience.


   ====================================================================== */
    
 

/* ----------------------------------------------------------------------
    This is the product/policy code used for pdQsuite version 1.0
   --- */
#define REG_EUD_PDQSUITE_1 (1)


/* ----------------------------------------------------------------------
    This is the product/policy code used for EIS 2.0 
   --- */
#define REG_EUD_EIS_2 (2)


/* ----------------------------------------------------------------------
    The plan is to accept these in EIS 2.x as spare/escape hatches
   --- */
#define REG_EUD_EIS_RESERVED1 (3)
#define REG_EUD_EIS_RESERVED2 (4)
#define REG_EUD_EIS_RESERVED3 (5)


/* ----------------------------------------------------------------------
    Eudora client ad ware registration. Used mostly for ad ware users
    after they've profiles
   --- */
#define REG_EUD_AD_WARE (32)


/* ----------------------------------------------------------------------
    Eudora client light registration
   --- */
#define REG_EUD_LIGHT (33)


/* ----------------------------------------------------------------------
    Eudora client basic paid mode. The plan is this is paid for a year
    of updates.  This code should not be accepted with month > 20
   --- */
#define REG_EUD_PAID (34)


/* ----------------------------------------------------------------------
    Eudora client basic paid mode. The plan is this is paid for a year
    of updates.
   --- */
#define REG_EUD_50_PAID_TRIMODE (35)
#define REG_EUD_50_PAID_BOX_ESD (36)
#define REG_EUD_50_PAID_37_RSRV (37)
#define REG_EUD_50_PAID_38_RSRV (38)
#define REG_EUD_50_PAID_39_RSRV (39)
#define REG_EUD_50_PAID_40_RSRV (40)
#define REG_EUD_50_PAID_START (35)
#define REG_EUD_50_PAID_STOP (40)


/* ----------------------------------------------------------------------
    Eudora client box/esd temporary reg policy code
    NO CLIENT SHOULD ACCEPT THIS AS A VALID POLICY
   --- */
#define REG_EUD_50_TEMP_BOX_ESD (41)

/* ----------------------------------------------------------------------
    Eudora client graceful exit codes
    These codes are intended to allow us to have regcodes for the us english
    version that don't necessarily work on internation versions and don't
    necessarily enable X1
   --- */
#define REG_EUD_50_PAID_EN_ONLY (42)
#define REG_EUD_50_PAID_EN_NOT_X1 (43)


/* ----------------------------------------------------------------------
     The build month to check regocde against for Eudora 4.3.x clients.
     This is March of 2000
   --- */
#define REG_EUD_CLIENT_4_3_MONTH 				(14)	// March 2000
#define REG_EUD_CLIENT_5_0_MONTH 				(20)	// September 2000
#define REG_EUD_CLIENT_5_1_1_MONTH 			(39)	// April 2002
#define REG_EUD_CLIENT_5_1_3_MONTH 			(42)	// July 2002
#define REG_EUD_CLIENT_5_2_MONTH 				(45)	// October 2002
#define REG_EUD_CLIENT_5_2_1_MONTH 				(48)	// January 2003
#define REG_EUD_CLIENT_5_2_2_MONTH 				(51)	// April 2003
#define REG_EUD_CLIENT_6_0_MONTH 				(51)	// April 2003
#define REG_EUD_CLIENT_6_0_1_MONTH 				(57)	// October 2003
#define REG_EUD_CLIENT_6_0_2_MONTH 				(58)	// November 2003
#define REG_EUD_CLIENT_6_1_MONTH 				(61)	// Feb 2004
#define REG_EUD_CLIENT_6_1_1_MONTH 				REG_EUD_CLIENT_6_1_MONTH	// Feb 2004, backdated due to buffer overflow bug
#define REG_EUD_CLIENT_6_1_2_MONTH 				REG_EUD_CLIENT_6_1_MONTH	// Feb 2004, backdated due to buffer overflow bug
#define REG_EUD_CLIENT_6_2_MONTH 				(68)	// Sep 2004
#define REG_EUD_CLIENT_7_0_MONTH 				(80)	// Sep 2005
#define REG_EUD_CLIENT_7_1_MONTH 				(89)	// June 2006
#define REG_EUD_CLIENT_34_DEFUNCT_MONTH (23)	// do not honor policy 34, month >= 23 (December 2000)


/*
    THIS FILE DOES NOT DEFINE POLICY. YOU NEED TO READ ELSEWHERE AND 
    CONSULT THE ORACLE BEFORE USING THESE CODES 
*/

