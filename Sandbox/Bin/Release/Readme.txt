=============================================
EUDORA PRO FOR WINDOWS, VERSION 4.2 BETA
=============================================

NOTE:  We have progressed a lot since the early betas.  Most of the 
issues with the cursor and crashes have been addressed.  There is 
still the problem with the Labels in filter actions.  It is mentioned
below:

1.  Make Label filter action results in a grayed out list of labels.
    The cause of this problem is that the labels are not getting
    properly loaded.  An easy workaround is to go ahead and set 
    Make Label to None, close and re-open your filters window.  You
    should now have your list of Labels from which to select.

Be sure to look at the changes section which highlights what has 
changed in this version.

NOTE: This is beta software.  If you are not entirely comfortable 
using beta software, DO NOT install this version on your PC.  
We strongly recommend that BEFORE you install this software, you 
make a backup copy of your existing Eudora folder and put that 
copy in a separate area.

Eudora Pro 4.2 introduces some new features including 
a new search interface, field auto-completion, and bug fixes to 
Eudora Pro 4.1.  

This is one of a series of beta drops for 4.2.  Please be sure 
to read subsequent readme.txt files to see the changes in the 
versions.  As with any Beta software, some features may not be 
fully implemented. If you find any problems with this software, 
please send us a bug report to our mailing list 
win-eudora-bugs@qualcomm.com.  

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

To use Eudora Pro for Windows Version 4.2, you must have the 
following:

-  IBM PC or compatible.
-  Microsoft Windows 95/98 or Windows NT 4.0 or above.
   (note:  it is recommended that you have the latest
   service pack for each operating system)
-  Winsock 1.1 API compliant networking package
-  A mail account with an Internet Service Provider
-  Access to your mail account via a modem or
   Internet-style network connection.  


--------------
INSTALLATION
--------------

For our beta software, please do the following:

To install Eudora Pro:

1.   Exit any applications you are running.

2.   Run the Setup program.

3.   Read the Welcome screen, then click the Next button.

4.   Read the Software License Agreement, and if you agree to 
     the terms, click Yes. You can save the Agreement by selecting 
     its text, pressing Ctrl-C to copy it, and then pasting it into 
     Notepad.

5.   Qualcomm's PureVoice Voice Player/Recorder application is 
     installed along with Eudora.

6.   Specify a directory to install Eudora, then click Next.
     
     Note: It is strongly recommended that you install the beta into 
     a clean empty directory.

7.   Verify the settings, then click Next.

8.   Eudora is installed in the destination directory.

9.   It will ask if you want to place a shortcut to Eudora on your
     desktop.

10.  We recommend that you look at the Eudora README file which I 
     guess you must be if you got this far.

-----------------------------------
CHANGES IN 4.2
-----------------------------------

NEW FEATURES
------------

Note that Eudora's .pdf files, which are reference files for using the
program, are still geared for 4.1.  We will update them when they are
ready.  In the meantime, use this as a guide.

*    New, improved Search
     Under Edit-Find-Find Messages, you are introduced to our new 
     search which has many improvements over the old Find.  The hot 
     key is ctrl-F.  The old Find text is now assigned to 
     ctrl-shift-F.  The search allows you to make up to five criteria 
     which are all AND'ed or all OR'ed.  There are new fields which 
     you will be able to specify as well as new operators.  You can 
     also specify which mailboxes are to be searched.  The result is 
     a clickable and sortable list of all of the matches.

     - When selecting mailboxes, note that clicking on the checkbox of
       a folder that contains another folder will result in the 
       checkbox being grayed.  Clicking a second time will cause all 
       of the mailboxes and folders to be unselected.

     - You can right-click on the resulting found items and perform 
       the specified actions such as delete.

     - The fields should be self-explanatory, but Summary refers to 
       the From: and Subject: fields, Age refers to the age (in days) 
       of the message, and Personality refers to the personality 
       associated with the message.  Recall that Any recipient 
       represents the To:, Cc:, and Bcc: fields.

     - The operators (or conjunctions) have a couple of new items.
       "contains word" accepts exactly one word.  If you want more 
       than word to be found, then you will have to specific another 
       criteria.

     - "matches regexp" refers to a powerful tool called Regular
       Expressions.  There are various implementations of Regular 
       Expressions, but Eudora uses the POSIX implementation.  Without
       going into too much detail, here are some symbols which are 
       supported.  Note that this operator is case-sensitive.

       .  (period) represents any one character.
       [] (brackets) contain a set of characters from which a match 
          can be made.  It corresponds to one character in the search 
          string.
       \  (backslash) is an escape character which means that the next
          character will not have a special meaning.

       *  (asterisk) is a multiplier.  It will match zero or more of 
          the previous character.  Note:  do not confuse this will 
          the usual meaning of *
       ?  (question mark) is a multiplier.  It will match zero or one 
          of the previous character.
       +  (plus) is a multiplier.  It will match one or more of the 
          previous character.
       {} (squiggly brackets) contain a number which specifies an 
          exact number of the previous character.

       [^] (brackets containing caret and other characters) means any 
           characters except the character(s) after the caret symbol 
           in the brackets.

       ^  (caret) is the start of the line.
       $  (dollar) is the end of the line.
       \< represents the start of a word.
       \> represents the end of a word.

       [:alpha:] represents any alphabetic letter.
       [:digit:] represents any single-digit number.
       [:blank:] represents a space or tab.

       |  (pipe) is OR.  It requires that the joined expressions have
          parentheses around them.

       Examples:
       e.a  matches eta, eda, e1a, but not Eta
       [eE].a matches eta and Eta
       E.*a matches Eudora, Etcetera, Ea
       ho+p matches hop, hoop, hoooop, but not hp
       etc\. matches etc. but not etc

*    Filters also support Regular Expressions

*    Field Auto-completion
     You will find a new Option category under Tools-Options called
     Auto-completion.  This feature allows you to type as little as
     one character, and the program will try to help you finish the
     field.  This feature is implemented in the To:, Cc:, Bcc: fields
     of the composition window and also the Directory Services Query
     field.  If you have more than one match, then it will provide a
     drop down list.  You can use your mouse or arrow keys to select.

     - Note that if you have nicknames, then you might already have
       some names in the list.  There is a different icon to 
       distinguish between items in your address book and those that 
       you had typed.

*    Automatic spell-check
     There is a new option under Tools-Options-Spell Checking called
     Check spelling automatically as you type.  With this enabled, it
     will automatically spell check your words after you type a space
     after it.  If misspelled, it will give you a red double-underline
     under the word.  If you want to correct it, you can right-click 
     on the word and select from the options there.

*    Reply with selected text
     If you select text from the preview pane or an opened received
     message and then click on Reply/Reply All, then it will quote 
     only the text that you had selected in your reply.

*    Personality selection
     If you have more than one personality setup, then you can change
     the personality being used by clicking on the From button which 
     gives you a list to choose from.

*    Off-line IMAP
     IMAP mailboxes are still accessible even though you are not 
     currently connected to the server.  You are limited to what you 
     have already downloaded.

*    Directory Services History List
     As mentioned above, this history list can also be brought up by 
     starting to type a query.  You can also click on the down arrow
     (triangle) to get a list.

*    Format=flowed
     This feature allows a received plain-text message from 4.2 to 
     conform according to the window width.  When you reply to the
     message, you will initially be given an excerpt bar to the left
     of the text.  You can still adjust the width of the window and 
     see the original text adjust accordingly.  This feature is used 
     only when viewing messages and composing replies/forwards.  Note 
     that a plain-text original message with a plain-text reply will 
     be sent out as plain-text (the excerpt bar gets converted).

*    VCard plugin support
     You should be able to attach a vcard to your mail and receive
     mail containing the attachment.  We are working on allowing you
     to import the data to your address book.

*    Esoteric Settings plugin
     As a default, this plugin is not available, but you can enable it
     by copying esoteric.epi from your extrastuff subdirectory into 
     your main Eudora directory.  This will enable a UI for several 
     features in your Tools-Options window.


---------------------------
CHANGES/FIXES build 32
---------------------------

CREATE/SEND/EDITING/REPLYING/FORWARDING/REDIRECTING/VIEWING MAIL
*    When copy/cut and pasting from excerpted text, we no longer add
     the > symbol in front of the pasted text for headers.  There are
     still some issues when pasting into the text body when you have
     Tools-Options-Viewing Mail-Use Microsoft Viewer disabled.

*    Lines starting with "from" are now handled properly when in the
     original text of a reply.

*    We now handle the > symbol which Sendmail adds in front of every
     line that starts with "From ".

FIND/FIND MESSAGES
*    We now properly save your queries if you close and reopen the 
     Find Messages dialog.  Some queries were not being saved.

IMAP
*    You can now display messages that had been sent with 
     QP (quoted-printable) disabled and the message contained some 
     8-bit characters.


---------------------------
CHANGES/FIXES builds 30-31
---------------------------

STARTUP/INTERFACE
*    We modified the lower-left margin hints to New Message, Reply, 
     Reply All, Forward, and Redirect to reflect the new behavior with 
     the <shift> modifier.  As a clarification, we have changed this 
     functionality to support Ctrl-Shift-R for Reply All.

CREATE/SEND/EDITING/REPLYING/FORWARDING/REDIRECTING/VIEWING MAIL
*    We fixed the cursor disappearing issue in regards to tabbing down 
     to the text body of a forwarded message.

*    We fixed an issue where Eudora would crash/poof if you had 
     typed or pasted any text containing the combination of a 
     symbol or space followed by a single quote.

*    If you included the text "<HTML>" within your message then
     it would cause some problems.  It now only acts weird when
     it is at the start of a line (avoid doing this).

*    "From " in the beginning of a line usually gets translated to
     ">From " while sending mail.  It now is displayed as "From ".

*    Fixed crash associated with trying to do a print preview without 
     a default printer.

*    It now omits the weird characters at the end of the message 
     dialog for canceling a new message that had a really long list in 
     the To: field.

AUTO-COMPLETION
*    Names with commas are now added as one name instead of being
     split up.

SPELL CHECKING
*    Fixed the hang due to an infinite loop if you tried to type 
     really fast and then typed the backspace too quickly before the 
     auto-spell checking kicked in.

*    For a little while we were checking the current word before it 
     was completed.  We do not do this anymore.

CHECKING/FILTERING MAIL
*    Fixed crash when you restart Eudora after leaving the filters 
     window open with a filter without any filter actions.

FIND/FIND MESSAGES
*    Ctrl-F and Ctrl-Shift-F were not working when you had a
     message open while Use Microsoft Viewer was checked.

IMAP
*    Fixed a crash which occurred if you had transferred a message
     from your IMAP mailbox to your Out box and then tried to 
     delete it.

*    A persistent progress message no longer displays when you had
     cancelled the password dialog after trying to add/delete an
     IMAP mailbox.

*    It will now prompt you for your IMAP password even if the 
     previous IMAP login had failed.

*    If you remove an IMAP mailbox, it now closes the mailbox if it 
     was open as well.  Another scenario is changing the personality 
     from IMAP to POP (supported by a few servers) which would cause 
     the entire associated mailbox tree to be removed.

*    If you try to search through an empty IMAP folder, it now 
     suppresses the error message that you cannot SELECT the empty 
     folder.  The error related to searching through an empty POP 
     folder still exists.

*    IMAP now filters at startup if checking mail.

---------------------------
CHANGES/FIXES builds 26-29
---------------------------

STARTUP/INTERFACE
*    The focus problem has improved.  It used to be that after 
     opening a mailbox and using the keyboard to reply to a message,
     the focus would not be on the message after sending the reply.
     There is still a focus problem if you subsequently use the 
     keyboard to check mail.

*    Tools-Options dialogs for Spell Checking, Auto-completion, and
     Advanced Networks have improved field order when tabbing through
     the fields.

*    New Recovered status (?) for email recovered during a rebuild
     of a TOC.  This can occur if you had deleted a piece of mail but
     had not compacted the mailbox.  If the mailbox's corresponding
     .toc file got corrupted, then rebuilding it would leave the 
     deleted mail as recovered.  It is designated with the ? symbol.

CREATE/SEND/EDITING/REPLYING/FORWARDING/REDIRECTING/VIEWING MAIL
*    Many have noticed that the <shift> modifier has been changed in
     regards to new messages, replies, forwards, and redirects.  The
     old way brought up a dialog that allowed you to specify a
     personality and/or stationery.  Obviously we have modified this
     functionality.  We now support <shift>-reply or <ctrl>-<shift>-r
     which now corresponds to Reply All.  The idea is that the new
     From button allows you to change the personality on the fly.  We
     also have the stationery window as well as Message-New Message
     With-list of stationery which allows you to make an easy
     selection of your stationery.

*    Alt-R has been activated to toggle the From button if you have
     more than just the <Dominant> personality.  

*    The disappearing cursor has been addressed for when you tab into 
     a new message.  The problem still occurs for some systems when 
     tabbing down to the text body for a forwarded message.

*    Reply with selected text that contains a header or colon will 
     be excerpted properly.  In certain cases, you will still get a 
     couple of hidden headers.

*    ReplyAttribution (default is something like At time, you wrote:)
     is re-added before excerpted text when you are replying and you
     have AlwaysExcerptResponse=0 which would cause the > symbols to
     display instead of the excerpt bars for replies to messages from
     previous versions to 4.2.

SPELL CHECKING
*    If you have Tools-Options-Spell Checking's "Warn me when sending/
     queuing message with misspellings" checked, then we have changed
     how this behaves.  If you try to send a message with at least one
     word misspelled, then you will be prompted with a Check Spelling
     dialog with the first misspelled word selected along with a list
     of suggestions.  There is a new button called [Just Send] or
     [Just queue] which allows you to immediately send/queue the 
     current message without checking anymore words.  Clicking on the
     dialog's X button in the upper right corner will simply cancel
     the dialog but not send/queue the message.

*    Improved spell checking when leaving word.

*    Improved handling when changing whether or not you are 
     auto-spell checking.  There used to be a persistent dialog that
     kept popping up if you had changed this status while a message
     was open.

*    Right-clicking on a group of selected words will provide the 
     correct menu instead of sometimes selecting the misspelled word
     and providing options for that word.

*    Single quotes around single words are no longer marked bad.

CHECKING/FILTERING MAIL
*    We addressed an issue where some people's filters looked like
     they were lost.  If they subsequently modified their filters and
     saved the changes, then yes, their old filters are gone.  The 
     problem was that if you had left your filters window open when
     you exited and restarted Eudora, then the filters were not
     displayed giving the impression that they were gone.  If you had
     closed and re-opened your filters, then they would have been
     okay.  We have addressed this issue, so that the filters are
     loaded if you had left your filters window open.

*    If you have Tools-Options-Incoming Mail-Offline checked, it now
     gives you a "Working Offline" dialog to [Allow this connection],
     [Allow all connections], or [Cancel] when you are checking mail.

FIND/FIND MESSAGES
*    Supports sorting by any of the columns.  Sorting is now 
     case-insensitive.  It sorts the Date properly.  Subject follows
     how the mailbox's TOC behaved which ignored Fwd: and Re:

*    Sorting is faster and should not leak memory.

*    Search button is disabled if you have invalid criteria.

*    It is more tolerant when you leave extra query lines empty.

*    Searching for an attachment no longer requires you to enter the
     attachment's path.  You can simply enter part/all of the name.

*    It supports search for part of a mailbox name which can be
     useful when you have select a lot of mailboxes but only want
     mailboxes which start with ...

IMAP
*    IMAP is now able to log bytes received from the network.  If you
     have the Esoteric Settings plugin in your Eudora directory, then
     in your Tools-Options-Logging section, check "All Bytes Received"
     This setting corresponds to your [Debug] section's LogLevel=64.

*    SUBSCRIBE and UNSUBSCRIBE commands have been corrected.

CHANGES/FIXES through build 25
-------------------------------

STARTUP/INTERFACE
*    Auto-save setting in Tools-Options-Miscellaneous is now checked
     for valid data.

*    NetscapeURLDDE=0 has returned to its single use of bringing up a
     separate window when clicking on a URL.  If you want to vary the
     control of the mailto: URL within Netscape, you can adjust it in 
     Eudora's Tools-Options-Miscellaneous.

*    Allows you to create a Eudora desktop shortcut.

*    Eudora is quicker during shutdown.

*    Whatever was the last current tabbed window in the foreground
     will be the same tabbed window in the foreground after exit and
     restart of Eudora.  

*    There is a larger display for the compact mailbox button in the
     lower-left corner of each mailbox.  We also make sure that it fits
     within the button.

*    Long label names are no longer cutoff.

*    Eudora now checks for proper range of MainWindowPosition such
     that Eudora can startup.  A symptom was that Eudora would start
     but not display.

CREATE/SEND/EDITING/REPLYING/FORWARDING/REDIRECTING/VIEWING MAIL
(Note that there are related categories in Auto-completion,
Spell checking, and format-flowed)

*    All replies will now use excerpt bars for viewing, but based on
     the content, it will not send the excerpt bar unless it has
     formatted text.  There is a setting to turn this feature off
     that would be in your [Settings] section.  It is called:
     AlwaysExcerptResponse=0
     Note that it is a known problem that the ReplyAttribution
     (default:  At time..you wrote:) does not show up if this is
     disabled.

*    One of the focus problems related to changing focus while a 
     mailbox is open has been addressed.  This relates to selecting 
     Tools-Options and then canceling out.  There is a known issue 
     related to reply/forward/redirecting a message from a mailbox 
     and the focus does not return to the mailbox.  
     Workaround:  click on the message.

*    When forwarding a message, we now add a blank line before the 
     excerpted text.  This will avoid the tendency of immediately 
     typing and accidentally typing within the excerpted text.

*    The stationery window now allows you to double-click or press 
     <enter> to create a new message using the selected stationery.

*    We now check to see whether or not there is HTML code for 
     <iframe> which can contain malicious code.  It is checked 
     unless you have Tools-Options-"Viewing Mail-Allow Executables 
     in HTML content" checked.

*    Sendmail has problems when a line starts with From.  To fix this
     problem, we have added code that would encode the F if quoted-
     printable (QP) is enabled (default).

*    Focus will not be lost in a new message if you change focus to
     another application and return.

*    <shift>-<reply (all)> behaves differently.  It will clear the To:
     field to allow you to enter a new address.  The new From: button
     allows you to change the personality.

*    We now support a new URL format.  This x-Eudora-option format
     allows you to set settings in your .INI file.
     Examples:
     <x-Eudora-option:RealName>
     <x-Eudora-option:RealName=foo>
     <x-Eudora-option:RealName&section=Settings>
     <x-Eudora-option:RealName=foo&section=Settings>

*    Auto-save has been added which will automatically save your new
     message every X minutes.  This setting can be adjusted in Tools-
     Options-Miscellaneous.

*    Edit-Paste Special... has been added to allow you to paste in
     text using other formats (i.e. RTF, HTML).

*    We now pull out some of those extra spaces in the recipient 
     fields (To:, Cc:, Bcc:) when replying to a multiple-recipient
     email.

*    If you do not select any text, then a reply will quote the entire
     original message; otherwise it will quote whatever text you had
     selected.  There are known issues if you try to select headers.

*    The carriage returns (CR's) in text attachments that contain CR's 
     without trailing LF's will not get converted to NULL's.

*    Fixed a bug where some text would not get sent if the reply 
     contains a line starting with From<space>, had some formatted 
     text, and was saved.

*    Reply (all) with selected text will now work when using 
     stationery.

*    Printing a new message with any really long headers no longer 
     crashes.

AUTO-COMPLETION
*    Removed the reference about Attachment field auto-completion.

*    Auto-completion can grab from your history and nicknames.

*    With auto-completion, if you type a name ABC@company.com and then
     another email to ABC, it will automatically fill the @company.com
     which may not be desirable.  To prevent from automatically 
     filling the rest of the line, use <backspace>.

*    When presented with a list for field auto-completion, you can use
     the arrow keys to navigate through the list, but you need to 
     manually lengthen the list using your mouse to see beyond the 
     given dimensions of the list.  It should retain the list 
     dimensions.  

*    Auto-completion drop down list box adjusts its dimensions 
     according to its contents.

*    If a nickname is automatically expanded, then the expanded 
     addresses do not get added to the history list.  This is intended
     behavior.

SPELL CHECKING
*    We have improved the spell checking while you are editing 
     previously entered text.  There are still some issues, but we 
     are working on them.

*    Auto-spell checking will be enabled as a default.  You can adjust
     this setting in Tools-Options-Spell Checking.

*    Auto-spell checking will recheck for misspelled words when 
     reopening a saved email.

*    We will be changing the warning dialog's text when you try to 
     send a message with some misspellings.

FORMAT=FLOWED
*    The format=flowed formatting is only supported from messages sent
     using Eudora Pro 4.2.x.  If you received a message from another 
     email program (or earlier Eudora), then replying will place 
     the > symbols in front of it.

*    You can disable this feature by setting the following in your 
     [Settings] section of your Eudora.ini file:
     ConvertFormatFlowedtoExcerpt=0
     InterpretFormatFlowed=0

*    New composition messages are no longer considered format=flowed.

CHECKING/FILTERING MAIL
*    Fixed the EHLO error message that would display when your mail
     server did not support EHLO.

*    Speak filter action settings will now continue to be disabled
     if the speak engine is not installed.  See below for more
     information on this feature.

*    Filters are updated and activated after saving instead of being
     activated after restarting Eudora.

*    Labels in the Filter actions may be grayed out when you try to
     select a Label.  A workaround is to go into Tools-Options-Labels
     and click on one of the label fields.  Click on OK.  You should 
     now have the labels available to you in the filter actions.

*    Memory management has improved in regards to handling mail and 
     mailboxes especially in regards to filtering.

*    Tools-Options-Background Tasks allows you to control the delay
     before processing your messages.  The new hot key is <ctrl>-\.

*    <x-Eudora-option:NoAutoResponseHeaders> allows for better 
     detecting of mailing lists to which you should not auto-respond.
     As a default, "errors-to:,list-" is the value.  Add any other
     headers which will identify it as a list (hence will not 
     auto-respond via filters).

*    Speak filter action will not cause crash if selected.  This 
     option can work if you have the required speech engine and SAPI
     found at <http://www.microsoft.com/iit>.  Install the SAPI SDK 
     (spchapi.exe) and the text-to-speech engine (MSTTS.exe).

FIND/FIND MESSAGES
*    The Stop button during a search will stop the entire search
     instead of just stopping the search in the current mailbox.

*    Trims spaces around single word when using "contains word" which
     allows for correct searches.

*    We switched the default action for Ctrl-F.  It now brings up the
     new Find Messages dialog.  Ctrl-Shift-F will now bring up the old
     style Find.  Remember that the old Find will search through your
     mailbox's TOC display if that is where the focus is located;
     otherwise, it will search through an open message.  The new Find
     Messages will be able to find messages throughout your selected
     mailboxes.

*    As a result of the previous change, we have added an Eudora.ini
     setting in your [Settings] section:
     SwitchFindKeyAccl=1
     The default setting is 0.  If set to 1, then Ctrl-F will 
     correspond to the old Find and Ctrl-Shift-F will correspond to 
     the new Find Messages.

*    Find Text (Edit-Find-Find or ctrl-F) while in the mailbox TOC 
     will allow you to search for a word in the Subject field.

*    If you use Find to find some text in an opened message and then
     move the Find dialog, the highlighted found word sometimes will 
     not remain highlighted.

IMAP
*    IMAP messages will now use <x-flowed> tags when format=flowed
     is used.

*    Fixed crash which would occur if you received a bounced message
     from certain servers.  This addresses related crashes too.

*    Trash is supported for an IMAP account.  Before, you had to mark
     a message for deletion and then delete it.  Now, you can specify
     a Trash mailbox (which you have to create) on your IMAP server
     and all IMAP deletions will move the messages to this mailbox.

*    Forget IMAP password will affect any of the account's open 
     mailboxes.  It will prompt you for the password if you try to 
     download a new message.

*    IMAP password is now saved if it did not previously exist.

ACAP
*    Improved support for some ACAP servers.  We are more in line with
     the ACAP specifications.


-------------
BUG REPORTS
-------------

If you experience something you think might be a bug in Eudora, 
please report it by sending a message to win-eudora-bugs@qualcomm.com. 
Describe what you did, what happened, what version of Eudora you have, 
any error messages Eudora gave (the numbers in parentheses ( ) are 
especially important), what kind of computer you have, which operating 
system you're using, and anything else you think might be relevant.

You will receive an automated response indicating that your bug report 
has been received and forwarded to our engineering staff. Unless 
additional information is needed, you will not receive a direct 
response.


-----------------------
REQUESTS FOR FEATURES
-----------------------

Please send any suggestions or requests for new features to 
eudora-suggest@qualcomm.com.

You will receive an automated response indicating that your suggestion 
has been received and forwarded to our engineering staff. Unless 
additional information is needed, you will not receive a direct 
response.


------------------------------------
Terms for Beta Testing Eudora Pro
Version 4.2  -  Windows Platform
------------------------------------

NOTICE:  THIS IS A CONTRACT.  BY INSTALLING AND USING THIS 
SOFTWARE, YOU ARE AGREEING TO BE BOUND BY ALL THE TERMS AND 
CONDITIONS OF THIS AGREEMENT.  PLEASE READ CAREFULLY BEFORE 
PROCEEDING.  IF YOU DO NOT AGREE TO THESE TERMS, PROMPTLY DESTROY 
ALL COPIES OF THE SOFTWARE IN YOUR POSSESSION.

QUALCOMM provides this beta version of Eudora Pro electronic mail 
software and any documentation provided with it (the "Software") 
to the authorized recipient ("Recipient") solely for end-user 
field performance testing under normal operating conditions by 
the Recipient.  No right to use the Software for any purpose 
other than testing the Software is granted.  Recipient agrees to 
test the Software by using it in daily operations under normal 
operating conditions and to report testing results promptly to 
QUALCOMM. Recipient agrees to restrict access to the Software so 
that no person other than Recipient may access or use the 
Software.  Copies of the Software may be made only as reasonably 
necessary for testing and backup purposes.

The Software is owned by QUALCOMM and its licensors, and its 
structure, organization and code are valuable trade secrets of 
QUALCOMM and its licensors, protected by U. S. copyright law and 
international treaty.  Recipient will not disclose, publish, or 
distribute the Software except as permitted above, and agrees to 
take reasonable precautions to prevent any unauthorized use, 
disclosure, publication, or distribution.  Proprietary rights 
notices on the Software shall be preserved on all copies made, 
and the Software shall not be modified, reverse engineered, or 
decompiled for any purpose, or translated into another computer 
language.

THE SOFTWARE IS PROVIDED "AS IS," AND WITHOUT ANY WARRANTY, 
EXPRESS OR IMPLIED, AS TO NON-INFRINGEMENT, MERCHANTABILITY OR 
FITNESS FOR ANY PARTICULAR PURPOSE, OR AS TO ITS PERFORMANCE, 
ACCURACY OR COMPLETENESS. QUALCOMM has not publicly announced the 
availability of the Software, and does not guarantee that the 
Software (or any similar product) will be available in the 
future.  Use of the Software is entirely at Recipient's risk.

QUALCOMM SHALL NOT BE LIABLE FOR ANY SPECIAL, INCIDENTAL OR 
CONSEQUENTIAL DAMAGES, WHETHER ARISING IN TORT (INCLUDING 
NEGLIGENCE), CONTRACT OR OTHERWISE, THAT ARISE FROM ANY USE OF 
THE SOFTWARE AND/OR OTHER CONFIDENTIAL INFORMATION OR QUALCOMM'S 
PERFORMANCE OR FAILURE TO PERFORM UNDER THIS AGREEMENT.

This License is effective until the expiration date of the 
Software licensed hereunder, unless earlier terminated as 
provided in this paragraph. Recipient may terminate this License 
at any time by destroying all copies of the Software. This 
License will terminate immediately without notice from QUALCOMM 
if the Recipient fails to comply with any provision of this 
License Upon termination Recipient must destroy all copies of 
the Software.

Recipient agrees that neither the Software nor the direct product 
thereof will be exported outside the United States except as 
authorized and as permitted by the laws and regulations of the 
United States.  If the Software has been rightfully obtained by 
Recipient outside the United States, Recipient agrees not to 
re-export the Software or the direct product thereof, except as 
permitted by the laws and regulations of the United States and 
the laws and regulations of the jurisdiction in which Recipient 
obtained the Software.

The Sentry Spelling-Checker Engine is Copyright (C) 1993 by
Wintertree Software Inc.

Windows (R) is a registered trademark of Microsoft Corporation.

Eudora (R), Eudora Pro (R), and Eudora Light (R) are a registered 
trademarks of QUALCOMM Incorporated.

QUALCOMM is a registered trademark and registered service mark of 
QUALCOMM Incorporated.

All other trademarks and service marks are the property of
their respective owners.

3/99 - 32

