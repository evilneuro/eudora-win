===========================================
EUDORA FOR WINDOWS, VERSION 7.1
INCLUDES ESP 1.2
===========================================

This README provides information on the following topics:

- Minimum Requirements
- Installation
- Eudora and Windows 2000 or Windows XP
- Notes Concerning Eudora and:
    * Apple QuickTime and
    * Microsoft Internet Explorer
- If the "Speak" Filter Action is Grayed Out
- Security Note
- Known Issues
- Bug Reports
- Requests for Features
- Eudora Version 7.1 Software License Agreement

See RelNotes.txt for detailed information on changes, bug fixes, and
known issues. (A single text file containing all the information
that's in Readme.txt and RelNotes.txt would be too large for Notepad
to open on some systems.)


----------------------
MINIMUM REQUIREMENTS
----------------------

To use Windows Eudora you must have the following:

-  IBM PC or compatible.
-  Microsoft Windows 98/2000/Me/XP.
-  An Internet e-mail account (POP or IMAP) with an Internet
   Service Provider or through a corporate network.
-  Access to your mail account via (1) a modem connection to your
   Internet Service Provider or (2) a direct Internet network
   connection.  


--------------
INSTALLATION
--------------

To install Eudora:

1.  After downloading the installer, exit any applications you are
    running.

2.  Click the Start button in the Windows task bar, select Run...,
    then click the "Browse" button.

3.  Go to the folder where you saved the installer. Choose that file
    and click "OK".

4.  Read the Welcome screen, then click the Next button.

    For the rest of the installation process, click the Next button
    to go on to the next step, or click the Back button to return to
    the previous step.
 
5.  Read the Software License Agreement, and if you agree to the
    terms, click Yes. Once you've installed Eudora, a copy of the
    license agreement will be available in the file License.txt in
    your Eudora directory.

6.  Pick the components you wish to install.

7.  Specify a folder in which to install Eudora (the installer
    refers to this as the Destination Program Folder), then click
    Next.

8.  Specify a folder in which Eudora data files (such as mail,
    settings, address books, and email attachments) should be stored.
    The installer refers to this as the Data Folder. NOTE: If you've
    already been using Eudora on this computer, be sure to specify
    the folder where your existing Eudora data files are located.

9.  The installer will show you the Destination Program Folder and
    the Data Folder once more. Use the Back button if you need to
    go back and change them. Click the Next button to proceed with
    the installation.

10. Eudora is installed in the destination directory.

11. The installer will offer to place a shortcut to Eudora on your
    desktop.

12. The installer will offer to take you to the Premium Services 
    Directory Page in your web browser. This gives you an
    opportunity to download additional tools that can help you use
    Eudora more effectively, such as Adobe Acrobat and Apple
    QuickTime. The interaction between Eudora and those two
    applications is described below.

13. The installer will offer to display the Readme file (this file).

14. We recommend that you read the Readme.txt and Relnotes.txt
    files in the Eudora directory following installation.


======================================
EUDORA AND WINDOWS 2000 or Windows XP
======================================

The security model of Windows 2000 and Windows XP has required us to
modify the way Eudora is installed and the way it determines where
to create data files -- including mailboxes, email attachments,
address books, and so on.

On a Windows 2000 or Windows XP system, only someone with Administrator
privileges (hereinafter, an Administrator) can install software in the
Program Files folder. Someone with User privileges (hereinafter, a User
with a capital U) cannot do so. In fact, a program launched by a User 
can’t store any data in the Program Files folder or any subfolder of
Program Files. Unfortunately, earlier versions of Eudora would try to
do exactly that by default; consequently, if an Administrator
installed Eudora and a User tried to run it, it would fail to work
properly.

With Eudora 5.x under Windows 2000 or Windows XP, the default location
to install the application is still under Program Files, but the data
files are kept in a User’s Application Data folder (typically
"C:\Documents and Settings\<username>\Application Data", where <username>
is the User’s login name). The installer lets you pick a different place
for Eudora to store data. If only an Administrator is going to be running
Eudora, the data can go anywhere at all, but if a User is going to be
running Eudora, the data should go in that user’s Application Data folder.

___Finding the data folder___

The Windows 2000 or Windows XP Application Data folder usually has the
hidden attribute, which makes it invisible in Windows Explorer or My 
Computer (unless you’ve configured those tools to display hidden 
objects). There are good reasons, though, why you might want to 
explore the folder where your Eudora data is stored; in particular, 
the email attachments that you’ve received are there, in the Attach 
subfolder (unless, of course, you’ve told Eudora to store attachments 
elsewhere). To make this easier, Eudora creates a shortcut to your 
Eudora data folder in the Application Data folder’s parent folder, 
which is typically "C:\Documents and Settings\<username>".

___Precisely how Eudora determines its data folder___

Below is the exact sequence of how Eudora determines the folder in
which to keep its data files. This applies in all versions of
Windows. Most users will have no need of this gory detail, but here
it is, just in case:

1. If a folder is specified on Eudora's command line, then Eudora uses
   that folder.

2. Otherwise, if the program folder (that is, the folder where Eudora
   is installed) contains a Eudora.ini file, then Eudora uses that
   folder.

3. Otherwise, if there's a DEudora.ini file in the program folder,
   and if the [Settings] section of that file contains a DataFolder
   setting, and if the folder specified by that setting contains a
   Eudora.ini file, then Eudora uses that folder.

4. Otherwise, if there is not a UseAppData=1 entry in the [Settings]
   section of the DEudora.ini file in the program folder, and if
   either of the registry keys listed below specifies a folder,
   then Eudora uses that folder. (If both keys list a folder, the
   first one takes precedence.)
     \\HKEY_CURRENT_USER\Software\Qualcomm\Eudora\CommandLine 
     \\HKEY_CLASSES_ROOT\Software\Qualcomm\Eudora\CommandLine\Current 

5. Otherwise, if the system has version 4.71 or higher of Shell32.dll,
   Eudora uses the current user's Application Data folder.

6. Otherwise, Eudora uses the program folder.

As suggested by step 5 above, Eudora's ability to use your Application
Data folder really hinges on whether your version is 4.71 or higher of
Shell32.dll. This is always the case if you're running Windows 2000 & XP,
and it's also true if you’ve installed version 4.0 or higher of 
Microsoft Internet Explorer and included the integrated shell in that
installation.

-------------------------------------
NOTES CONCERNING EUDORA AND:
      * APPLE QUICKTIME
      * MICROSOFT INTERNET EXPLORER
-------------------------------------

Eudora's standard message viewer supports basic HTML, such as text
formatting and inline images (displaying pictures within the body of
a message rather than in a separate window). Some of the supported
image formats include .BMP, .JPG, and .PNG files. Installing Apple's
QuickTime application adds support to the standard viewer for inline
display of a wider range of image (picture) formats such as .GIF's.
Installing Microsoft Internet Explorer (IE), version 4.0 or above, 
adds an additional message viewer (hereinafter, the Microsoft viewer)
to Eudora that can accurately display just about any HTML message you
might receive.

You can find an installer for QuickTime on the Eudora web site at
<http://eudora.qualcomm.com/download/other>. You can obtain Internet
Explorer on the Microsoft web site at <http://www.microsoft.com>.

* Why should I care about displaying HTML messages?

HTML messages (email messages that use HTML, the language of web
pages) are becoming more and more common. Most web browsers allow a
web page to be sent as an HTML message. If someone sends you an HTML
message that uses advanced formatting such as tables, then to see it
as it was intended you need to tell Eudora to use the Microsoft viewer
instead of its standard message viewer.

* If Eudora has two viewers, how will I tell it which one to use?

Just check or uncheck the "Use Microsoft's viewer" checkbox in the
Viewing Mail panel of the Options dialog, under the Tools menu in
Eudora. The checkbox is checked by default if IE 4.0 or higher is on
your system. If IE 3.0 or higher is not on your system, the checkbox
is dimmed and you can use only the standard viewer; you can, however,
enhance it with QuickTime as described below.

* What if I already have Internet Explorer?

If you already have IE 4.0 or above on your system, Eudora will take
advantage of it automatically, as long as you have "Use Microsoft's
Viewer" checked as described above.

If you have IE 3.0 on your system, you have two choices:

1. You can upgrade to a more recent version of Internet Explorer.
2. You can leave it alone, in which case Eudora will use IE 3.0 if you
   choose "Use Microsoft's viewer" in Eudora's Options dialog.
   However, this doesn't provide all the functionality you get with
   IE 4.0 or above, or even with the standard viewer - see the details
   below.

If you have a version of IE earlier than 3.0 on your system,
installing IE 4.0 will not affect it. However, you must have at least
IE 3.0 in order for the "Use Microsoft's viewer" option to take
effect.

* What does Eudora's standard viewer support?

  + Bold, italic, and underlined text
  + Different font faces, sizes, and colors
  + Fixed-width text
  + Left and right justification and centering
  + Multiple indentation levels
  + Bulleted lists
  + Hyperlinks (clicking on a URL launches your browser and takes you
    to a web page) 
  + Horizontal ruling lines
  + Inline display of BMP images, some JPEG images, and PNG images

* What does QuickTime add to the standard viewer?

  + Inline display of a wider variety of images, including the full
    set of JPEG formats, GIF, TIFF, Macintosh PICT, etc.

* What does the Microsoft viewer do that the standard one doesn't?

  + Displays GIF and a wider variety of JPEG images inline
  + Automatic inline display of images from web sites
  + Numbered lists
  + Tables and forms
  + International character sets
  + Java applets, JavaScript, and ActiveX controls
  + Dynamic HTML (not if you're using IE 3.0)
  + Other embedded objects (audio, video)
  + Unlike QuickTime, it doesn't display Macintosh PICT images

* What DON'T I get in Eudora if I use IE 3.0 instead of IE 4.0?

  + Dynamic HTML
  + Editing of received messages
  + Running Eudora plug-ins (such as text translators) on received
    messages

If you're using IE 3.0, you can still edit received messages, and run 
Eudora plug-ins on them, provided you first switch to the standard
viewer by unchecking "Use Microsoft's viewer" in the Options dialog.


--------------------------------------------
IF THE "SPEAK" FILTER ACTION IS GRAYED OUT
--------------------------------------------

You have the option, when setting up a filter, to have Eudora audibly
announce, or speak, the subject and/or the name of the sender whenever
a message is filtered. (In the case of an outgoing filter, it will
speak the name of the recipient rather than the sender.)

In order to use this feature, you need to have a sound card and sound
card device driver that are compatible with the version of Windows
you're running. You also need to install Microsoft's Speech
Application Programming Interface (SAPI) and a SAPI-compliant
text-to-speech engine.

If those components are not present on your computer, the Speak action
will be grayed out in the Filters window. You can remedy this by
installing SAPI and Microsoft's text-to-speech engine; installers for
both are available on the web at
<http://www.eudora.com/download/other>, under the heading "Installing
Microsoft's Text-to-Speech Engine".


---------------
SECURITY NOTE
---------------

Recent email viruses and worms have heightened many people's awareness
of the need to be careful with email. For maximum protection, we
recommend the following:

1. Change your Eudora attachment directory to something other than the
   default.
2. Disable JavaScript in your web browser.
3. Leave "(Warn me when I) Launch a program from a message" checked
   in Tools/Options/Extra Warnings.
4. Leave "Allow executables in HTML content" unchecked in
   Tools/Options/Viewing Mail.

Consult <http://eudora.qualcomm.com/security.html> for additional
information, including instructions for changing your Eudora
attachment directory.


--------------
KNOWN ISSUES
--------------

1.  Eudora doesn't have dual monitor support.

2.  The usage percentages in the Statistics may not be accurate if you
    use Eudora for a long period without ever switching to a different
    application or letting Eudora sit idle for 60 seconds.
 
4.  Eudora Directory Services doesn't support version 3+ of LDAP.

5.  Eudora sometimes thinks there's an Internet connection when in
    fact there isn't.

6.  Using a reply-with filter action with a plain-text original
    message causes the tag </x-html> to appear at the end of the
    stationery reply.

7.  The ESP mailbox Folder view doesn't allow renaming of sub-folders.
    You have to select Open Folder from the Share Group Actions menu
    instead.

8.  Toolbar icons still display incorrectly on a few systems.

9.  The MoodWatch chilies in the composition toolbar don't show up, or
    display very faintly, in certain Windows color schemes, such as
    "Plum (high color)".


-------------
BUG REPORTS
-------------

If you experience something you think might be a bug in Eudora, please
report it by going to <http://www.eudora.com/contact_web.html>.
Describe what you did,what happened, what version of Eudora you have,
any error messages Eudora gave (the numbers in parentheses ( ) are
especially important), what kind of computer you have, which operating
system you're using, and anything else you think might be relevant.

Unless additional information is needed, you will not receive a 
direct response.

-----------------------
REQUESTS FOR FEATURES
-----------------------

Please submit any suggestions or requests for new features to 
<http://www.eudora.com/contact_web.html>.

Unless additional information is needed, you will not receive a 
direct response.

-------------------------------------------------------------
EUDORA(R) VERSION 7.1 SOFTWARE LICENSE AGREEMENT
-------------------------------------------------------------

QUALCOMM INCORPORATED(R) ("QUALCOMM") IS WILLING TO LICENSE
ITS EUDORA VERSION 7.1 SOFTWARE INCLUDING ACCOMPANYING
PUREVOICE(TM) PLAYER-RECORDER AND PUREVOICE(TM) PLUG-IN
(THE "SOFTWARE") AND THE ACCOMPANYING DOCUMENTATION TO YOU
ONLY ON THE CONDITION THAT YOU ACCEPT AND AGREE TO ALL OF
THE TERMS IN THIS AGREEMENT. BY CLICKING ON THE "YES"
BUTTON BELOW YOU ACKNOWLEDGE THAT YOU HAVE READ THIS
AGREEMENT, UNDERSTAND IT AND AGREE TO BE BOUND BY ITS TERMS
AND CONDITIONS. IF YOU DO NOT AGREE TO THESE TERMS,
QUALCOMM IS UNWILLING TO AND DOES NOT LICENSE THE SOFTWARE
TO YOU. IF YOU DO NOT AGREE TO THESE TERMS YOU MUST CLICK
ON THE "NO" BUTTON AND DISCONTINUE THE INSTALLATION
PROCESS. ANY USE OF THE SOFTWARE BY YOU IS SUBJECT TO THE
TERMS AND CONDITIONS SET FORTH HEREIN.

1. LICENSE. Subject to the terms and conditions of this 
License Agreement, QUALCOMM hereby grants you a 
nonexclusive license to install and use the Software in 
machine-readable form solely for testing and evaluation 
purposes on a single personal computer, provided that if 
you are an individual, on one or more personal computers 
in your household or home office owned or controlled by 
you for use by yourself and immediate family members 
residing in the same household only. You may copy the 
Software only for backup purposes, provided that you 
reproduce all copyright and other proprietary notices that 
are on the original copy of the Software.

2. RESTRICTIONS. QUALCOMM retains all right, title, and
interest in and to the Software, and any rights not
expressly granted to you herein are reserved by QUALCOMM.
You may not reverse engineer, disassemble, decompile, or
translate the Software, or otherwise attempt to derive the
source code of the Software, except to the extent (if at
all) expressly permitted under any applicable law.  If
applicable law expressly permits such activities, any
information so discovered or derived shall be deemed to be
the confidential proprietary information of QUALCOMM and
must be promptly disclosed by you to QUALCOMM.  Any
attempt to transfer any of the rights, duties or
obligations hereunder is void.  You may not rent, lease or
loan the Software.  You may not reproduce (except as
expressly permitted under Section 1), distribute, publicly
perform, publicly display, modify or create derivative
works of or based on the Software.

3.  TECHNICAL SUPPORT. Limited technical support (via
telephone or email response) may be available for a limited
period of time if you use the Software in paid mode.
Information regarding availability, nature and conditions
of limited technical support may be found at www.eudora.com.

4.  NO WARRANTY. YOU EXPRESSLY ACKNOWLEDGE AND AGREE THAT
THE USE OF THE SOFTWARE IS AT YOUR SOLE RISK. THE
SOFTWARE (AND TECHNICAL SUPPORT, IF ANY) IS PROVIDED "AS
IS" AND WITHOUT ANY WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED. TO THE MAXIMUM EXTENT PERMITTED UNDER APPLICABLE
LAWS, QUALCOMM AND ANY APPLICABLE LICENSORS (FOR THE
PURPOSES OF SECTIONS 4, 5 AND 6, QUALCOMM AND QUALCOMM'S
LICENSOR(S) SHALL BE COLLECTIVELY REFERRED TO AS QUALCOMM)
EXPRESSLY DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
NONINFRINGEMENT. QUALCOMM DOES NOT WARRANT THAT THE
FUNCTIONS CONTAINED IN THE SOFTWARE WILL MEET YOUR
REQUIREMENTS, OR THAT THE OPERATION OF THE SOFTWARE WILL
BE UNINTERRUPTED OR ERROR-FREE, OR THAT DEFECTS IN THE
SOFTWARE WILL BE CORRECTED. FURTHERMORE, QUALCOMM DOES
NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE
OR THE RESULTS OF THE USE OF THE SOFTWARE IN TERMS OF ITS
CORRECTNESS, ACCURACY, RELIABILITY, OR OTHERWISE. NO ORAL
OR WRITTEN INFORMATION OR ADVICE GIVEN BY QUALCOMM OR ITS
AUTHORIZED REPRESENTATIVES SHALL CREATE A WARRANTY OR IN
ANY WAY INCREASE THE SCOPE OF THIS WARRANTY. SOME
JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF IMPLIED
WARRANTIES, SO THE ABOVE EXCLUSION MAY NOT APPLY OR MAY BE
LIMITED.

5.  LIMITATION OF LIABILITY. TO THE MAXIMUM EXTENT
PERMITTED UNDER APPLICABLE LAWS, UNDER NO CIRCUMSTANCES,
INCLUDING NEGLIGENCE, SHALL QUALCOMM, ITS AFFILIATES OR
THEIR DIRECTORS, OFFICERS, EMPLOYEES OR AGENTS BE LIABLE
FOR ANY INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING BUT NOT LIMITED TO DAMAGES
FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS
OF BUSINESS INFORMATION AND THE LIKE) ARISING OUT OF THE USE
OR INABILITY TO USE THE SOFTWARE, EVEN IF QUALCOMM OR ITS
AUTHORIZED REPRESENTATIVE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES. SOME JURISDICTIONS DO NOT
ALLOW THE LIMITATION OR EXCLUSION OF LIABILITY FOR
INCIDENTAL OR CONSEQUENTIAL DAMAGES SO THE ABOVE LIMITATION
OR EXCLUSION MAY NOT APPLY. IN NO EVENT SHALL QUALCOMM'S
TOTAL LIABILITY TO YOU FOR ANY AND ALL DAMAGES, LOSSES,
CLAIMS AND CAUSES OF ACTIONS (WHETHER IN CONTRACT, TORT,
INCLUDING NEGLIGENCE, OR OTHERWISE) EXCEED THE GREATER OF
THE AMOUNT PAID BY YOU FOR THE SOFTWARE OR $10.

6. INDEMNITY. You agree to defend and indemnify QUALCOMM
against all any and all claims, losses, liabilities,
damages, costs and expenses, including attorney's fees,
which QUALCOMM may incur as a result of or in connection
with your breach of this Agreement.

7.  TERM AND TERMINATION. This Agreement shall continue
until terminated. You may terminate the Agreement at any
time by deleting all copies of the Software and all
related information. This Agreement terminates
immediately and automatically, with or without notice, if
you fail to comply with any provision hereof.
Additionally, QUALCOMM may at any time terminate this
Agreement, without cause, upon notice to you. Upon
termination you must delete or destroy all copies of the
Software in your possession.  Sections 2, 4 through 6, 8,
9 and 11 survive termination.

8.  EXPORT. The Software contains strong encryption that
is subject to the laws and regulations of the US
Government. Licensee agrees and certifies that neither
the Software nor any other technical data received from
QUALCOMM, nor the direct product thereof, will be
exported, re-exported, or transferred except as authorized
and as permitted by the laws and regulations of the United
States Government. Prohibited exports include but are not
limited to; the export, re-export, or transfer of the
Software or technical data to any prohibited entities or
destinations subject to the US Government's current list
of restricted or embargoed countries, any parties
currently listed on the US Government's Denied Parties or
Specially Designated National list, and any proliferation
activities prohibited by the US Government such as
chemical, biological, nuclear or missile technology.

9.  GOVERNMENT END USERS. If you are or are acting on
behalf of an agency or instrumentality of the United
States Government, the Software, as applicable, is
"commercial computer software" and "commercial computer
software documentation" and pursuant to FAR 12.212 or
DFARS 227 7202 and their successors, as applicable, use,
reproduction and disclosure of the Software is governed by
the terms of this Agreement.

10.  CONTRACTING PARTIES. If the Software is installed on
any computer owned by a corporation or other legal entity,
then this Agreement is formed by and between QUALCOMM and
such entity. The individual executing this Agreement
represents and warrants to QUALCOMM that they have the
authority to bind such entity to the terms and conditions
of this Agreement.

11. GENERAL. This Agreement is governed and interpreted
in accordance with the laws of the State of California
without giving effect to its conflict of laws provisions.
he United Nations Convention on Contracts for the
International Sale of Goods is expressly disclaimed and
shall not apply. Any claim arising out of or related to
this Agreement must be brought exclusively in a federal or
state court located in San Diego County, California and
you consent to the jurisdiction of such courts. If any
provision of this Agreement shall be invalid, the validity
of the remaining provisions of this Agreement shall not be
affected. This Agreement is the entire and exclusive
agreement between QUALCOMM and you with respect to the
Software and supersedes all prior agreements (whether
written or oral) and other communications between QUALCOMM
and you with respect to the Software.

COPYRIGHT (C) 2006 BY QUALCOMM INCORPORATED. ALL RIGHTS
RESERVED.

QUALCOMM® IS A REGISTERED TRADEMARK AND REGISTERED SERVICE
MARK OF QUALCOMM INCORPORATED. PUREVOICE(TM) IS A TRADEMARK
OF QUALCOMM INCORPORATED. ALL OTHER TRADEMARKS AND SERVICE
MARKS ARE THE PROPERTY OF THEIR RESPECTIVE OWNERS.


This product includes software developed by the OpenSSL Project for use in the 
OpenSSL Toolkit (http://www.openssl.org/)

OPEN SSL LICENSE ISSUES

The OpenSSL toolkit stays under a dual license, i.e. both the conditions of
the OpenSSL License and the original SSLeay license apply to the toolkit.
See below for the actual license texts. Actually both licenses are BSD-style
Open Source licenses. In case of any license issues related to OpenSSL
please contact openssl-core@openssl.org.

OpenSSL License
---------------

====================================================================
Copyright (c) 1998-2003 The OpenSSL Project.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
  
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

 3. All advertising materials mentioning features or use of this
    software must display the following acknowledgment:
    "This product includes software developed by the OpenSSL Project
    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"

 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
    endorse or promote products derived from this software without
    prior written permission. For written permission, please contact
    openssl-core@openssl.org.

 5. Products derived from this software may not be called "OpenSSL"
     nor may "OpenSSL" appear in their names without prior written
     permission of the OpenSSL Project.

 6. Redistributions of any form whatsoever must retain the following
     acknowledgment:
     "This product includes software developed by the OpenSSL Project
     for use in the OpenSSL Toolkit (http://www.openssl.org/)"
  
THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
====================================================================
 
This product includes cryptographic software written by Eric Young
(eay@cryptsoft.com).  This product includes software written by Tim
Hudson (tjh@cryptsoft.com).

Original SSLeay License
-----------------------

Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
All rights reserved.

This package is an SSL implementation written
by Eric Young (eay@cryptsoft.com).
The implementation was written so as to conform with Netscapes SSL.

This library is free for commercial and non-commercial use as long as
the following conditions are aheared to.  The following conditions
apply to all code found in this distribution, be it the RC4, RSA,
lhash, DES, etc., code; not just the SSL code.  The SSL documentation
included with this distribution is covered by the same copyright terms
except that the holder is Tim Hudson (tjh@cryptsoft.com).

Copyright remains Eric Young's, and as such any Copyright notices in
the code are not to be removed.
If this package is used in a product, Eric Young should be given attribution
as the author of the parts of the library used.
This can be in the form of a textual message at program startup or
in documentation (online or textual) provided with the package.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 1. Redistributions of source code must retain the copyright
     notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
     must display the following acknowledgement:
     "This product includes cryptographic software written by
     Eric Young (eay@cryptsoft.com)"
     The word 'cryptographic' can be left out if the rouines from the library
     being used are not cryptographic related :-).
 4. If you include any Windows specific code (or a derivative thereof) from
     the apps directory (application code) you must include an acknowledgement:
     "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
  
THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

The licence and distribution terms for any publically available version or
derivative of this code cannot be changed.  i.e. this code cannot simply be
copied and put under another distribution licence
[including the GNU Public Licence.]


BY CLICKING ON THE "YES" BUTTON YOU ACKNOWLEDGE THAT YOU
HAVE READ THIS AGREEMENT, UNDERSTAND IT AND AGREE TO BE
BOUND BY ITS TERMS AND CONDITIONS.


4/6/06
