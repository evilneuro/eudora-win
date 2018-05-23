============================
QUALCOMM PureVoice FOR WINDOWS
VERSION 1.3.2 for 32-bit
============================

You can find more inforamation about PureVoice Technology 
at <http://eudora.qualcomm.com/purevoice/>

This README provides information on the following topics:

- Minimum Requirements
- Installation
- New Features and Changes
- Bug Fixes
- Bug Reports
- Requests for Features
- License and Legal Information

----------------------
MINIMUM REQUIREMENTS
----------------------

To use PureVoice, you must have the following:

-  IBM PC compatible with 486/50 MHz or faster.
-  Microsoft Windows 95 or Windows NT 3.5 or higher for 32-bit version.
-  Windows compatible 16-bit sound system and microphone to create voice messages.
-  8 MB RAM


--------------------
INSTALLATION
--------------------

To install PureVoice, do the following:

1.    Exit any applications(especially Eudora) you are running.

2.    If you are installing from a software archive, put the archive into a
      directory on your PC and double-click on it to start the self-extracting installer.
  
3.    Read the Welcome screen, then click Next.
      The Software License Agreement is displayed.

4.    Read the Software License Agreement and click Next (you can also
      save or print the Agreement).

5.    Specify a directory to install PureVoice, then click Next.
      Note:	If you are installing on a PC with Eudora, you should install it under 
      the plugins directory(default) to take advantage of the EMSAPI plug-in of PureVoice.

6.    Verify that the displayed settings are correct, then click Next (or
      use the Back button if you need to go back and make changes).

7.    PureVoice is installed in the destination directory, and you are
      prompted for whether or not you want to view the README file.

-----------------------------------
NEW FEATURES AND CHANGES IN 1.3.2
-----------------------------------

*   Microphone & Speaker Volume control with the keyboard.

*   Uses longer, more readable default names, YYYYMMDD_HHMMSS.qcp

*   Displays actual compression rate (not Compression option that was chosen) of the file.
    The range of the compression rate can be 1kb/s - 13kb/s(was 8kb/s or 13kb/s before).

*   "Attach Site Info" option.

*   "Compact View" option. (Displays "Normal View" button when in Compact View mode).

*   "Bug Report" and "View Web Site" buttons on "About PureVoice" 

*   Includes new URL, http://eudora.qualcomm.com/purevoice/bugs.html to send bugs, 
    suggestions or questions.

*   Requires MFC42.DLL (PureVoice is not statically linked to this DLL file any more).
    File size of PureVoice.exe reduced to 165KB from 393KB.

*   Command line converter tool(.wav to .qcp and vice-versa) for Win is available from 
    the PureVoice web site, <http://eudora.qualcomm.com/purevoice>

*   PureVoice for Mac 68K is available.

*   Win PureVoice SDK is available.

*   PureVoice for UNIX SCO OpenServer Release 5 is available.

*   PureVoice for UNIX Solaris 2.5 or later is also available from the PureVoice web site.

*   Updated Help files to include FAQ.

------------------
BUG FIXES IN 1.3.2
------------------
   
*   Crash after 3 hours of recording

*   Use .wav suffix when attaching a PCM formatted file.

-----------------------------------
NEW FEATURES AND CHANGES IN 1.2.1
-----------------------------------

*   Doesn't require MFC42.DLL or MSVCRT.DLL(they're statically linked).

*   Added Exit option under File menu.

--------------------
BUG FIXES IN 1.2.1
--------------------
   
*   Selected icon turns into a shadow after running PureVoice on some Win95.

-----------------------------------
NEW FEATURES AND CHANGES IN 1.2
-----------------------------------

*   Loads PureVoice Icon onto Eudora Pro 4.0 tool bar(uses EMSAPI v4).

*   Works with ViaVoice plug-in of Eudora.

*   Uses MFC42.DLL and MSVCRT.DLL (not MFC40.DLL and MSVCRT40.DLL)


--------------------
BUG FIXES IN 1.2
--------------------
   
*   Fixed the problem of PureVoice becoming a zombie after termination on 
    some NT machines.

-----------------------------------
NEW FEATURES AND CHANGES IN 1.1
-----------------------------------

*   Uses Window's 3D color scheme.

--------------------
BUG FIXES IN 1.1
--------------------
   
*   Selected icon turns into a shadow after running PureVoice on Win95 with 
    "Show Icons using all possible colors" option has been fixed.

*   Missing a series of dots to indicate current record/play volume level with
    some sound drivers has been fixed.

*   Ineffective play/record volume control on some sound drivers has been fixed.

*   Unnecessary abbreviated menu bar(F...C...O...H...) has been fixed.

*   Added more Help contents for Eudora users.

*   Documented how to convert .wav files to PureVoice files and vice versa in Help
    Files (Alt + Shift + C)

-----------
BUG REPORTS
-----------

If you experience something you think might be a bug in PureVoice, please
check http://eudora.qualcomm.com/purevoice/bugs.html.  If you don't
find it there, please report it by sending a message using the form on that page.
Describe what you did, what happened, any error messages PureVoice gave, and
anything else you think might be relevant.


----------------------
REQUESTS FOR FEATURES
----------------------

From time to time, everyone comes up with an idea for something they'd
like their software to do differently. This is true of all applications,
no less PureVoice.  If you come across an idea that you think might make a
nice enhancement to PureVoice, your input is always welcome. Please send any
suggestions or requests for new features by sending a message using the form 
linked from http://www.eudora.com/techsupport/purevoice/purevoicebugs.html page.

----------------------
REQUESTS FOR LICENSING
----------------------

If you want to use PureVoice Technology (encoder/decoder library, not PureVoice.exe
application itself) for your project/software, please send a message to 
purevoice@qualcomm.com.


------------------------------
LICENSE AND LEGAL INFORMATION
------------------------------
QUALCOMM Incorporated

License Terms for PureVoice(TM) Player-Recorder Version 1.3.2 - Windows Platform
License Terms for PureVoice(TM) Plug-In for Eudora“ Version 4.1 - Windows Platform

QUALCOMM grants to the user a nonexclusive license to use this PureVoice software 
solely for user's own personal or internal business purposes.  The user may not 
commercially distribute, sublicense, resell, or otherwise transfer for any 
consideration, or reproduce for any such purposes, the PureVoice software or any 
modification or derivation thereof, either alone or in conjunction with any other 
product or program.  Further, the user may not modify the PureVoice software.

THE PUREVOICE SOFTWARE IS PROVIDED TO THE USER ''AS IS.''  QUALCOMM MAKES NO WARRANTIES, 
EITHER EXPRESS OR IMPLIED, WITH RESPECT TO THE PUREVOICE SOFTWARE AND/OR ASSOCIATED 
MATERIALS PROVIDED TO THE USER, INCLUDING BUT NOT LIMITED TO ANY WARRANTY OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR AGAINST INFRINGEMENT. QUALCOMM 
DOES NOT WARRANT THAT THE FUNCTIONS CONTAINED IN THE SOFTWARE WILL MEET YOUR REQUIREMENTS, 
OR THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT DEFECTS 
IN THE SOFTWARE WILL BE CORRECTED.  FURTHERMORE, QUALCOMM DOES NOT WARRANT OR MAKE ANY 
REPRESENTATIONS REGARDING THE USE OR THE RESULTS OF THE USE OF THE SOFTWARE OR ANY 
DOCUMENTATION PROVIDED THEREWITH IN TERMS OF THEIR CORRECTNESS, ACCURACY, RELIABILITY, 
OR OTHERWISE. NO ORAL OR WRITTEN INFORMATION OR ADVICE GIVEN BY QUALCOMM OR A QUALCOMM 
AUTHORIZED REPRESENTATIVE SHALL CREATE A WARRANTY OR IN ANY WAY INCREASE THE SCOPE OF T
HIS WARRANTY.

LIMITATION OF LIABILITY - QUALCOMM IS NOT LIABLE FOR ANY CLAIMS OR DAMAGES WHATSOEVER, 
INCLUDING PROPERTY DAMAGE, PERSONAL INJURY, INTELLECTUAL PROPERTY INFRINGEMENT, LOSS OF 
PROFITS, OR INTERRUPTION OF BUSINESS, OR FOR ANY SPECIAL, CONSEQUENTIAL OR INCIDENTAL 
DAMAGES, HOWEVER CAUSED, WHETHER ARISING OUT OF BREACH OF WARRANTY, CONTRACT, TORT 
(INCLUDING NEGLIGENCE), STRICT LIABILITY, OR OTHERWISE.

Copyright © 1998 by QUALCOMM Incorporated.  All rights reserved.

PureVoice(TM) is a registered trademark of QUALCOMM Incorporated.
QUALCOMM and Eudora are registered trademarks and QUALCOMM is a registered service 
mark of QUALCOMM Incorporated.  
Windows is a trademark of Microsoft Corporation registered in the U.S. and other countries.  

All other trademarks and service marks are the property of their respective owners.

9/98


