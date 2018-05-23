 QUALCOMM/EUDORA APPLICATION REGCODES
 Laurence Lundblade and Steve Dorner
 Copyright 1999, 2000 QUALCOMM Inc.



 
 Contents
 ========
1. Implemenation Notes
2. Registeree's Name Character Set Canonicialization
3. Existing Polciy/Product Code Assignments
4. Policy Implementations
5. Creating New Policy/Product Codes
6. Executive Overview
7. Change history
 

 

 1. Implementation Notes
 =======================
Implementations can either compile in the C code as part of the application,
(e.g., the Eudora client) or they can use the UNIX shell command regutil 
(e.g., the registration web page). Regutil is simply a wrapper around 
regcode_v3.c and regcode_charset_v3.c
  

 Generation
 ----------
                            Random Number
                                  |
                                  v 
 Registeree's Name   --->+----------------------+
                         |  regcode_v3.c and    |
 Issuing month       --->| regcode_charset_v3.c | -> Regcode
                         |          or          |
 Product/Policy Code --->|       regutil        |
                         +----------------------+

                             
   
 Verification
 ------------
          
                      +-----------------------+
                      |   regcode_v3.c and    |  
 Regcode           -> | regcode_charset_v3.c  |---> Basic validity of code
                      |          or           |       (name matches code)
 Registeree's Name -> |       regutil         |              |
                      +-----------------------+              |
                             |              |                |
                             v              v                |
                       Issuing Month   Product/Polcy Code    | 
                             |              |                |
                             v              v                v 
                         +-------------------------------------+ 
                         |                                     | 
                         | application specific policy check   |
                         |                                     |
                         +-------------------------------------+ 
                                          |
                                          v
                                   yea/neah -- final
                                   verification result


The registeree's name is presented in the UI as the first name and the
last name. It should always presented the way the user entered it without
any case or character set folding or punctuation stripping. When the 
registeree's name is stored or passed to any registration functions it 
should include the character set it was entered in. See notes on character 
sets below.

The issuing month is the month in which the code was generated. There may
be some exceptions. It is a month number with Jan 1999 having a value of 0,
Feb 1999 being 1, Jan 2000 being 12 and so on. The scheme overflows in the
year 2020.

The product/policy code is an integer indicating which product is being
regisetered. It may also indicate something about the policy under which
the code was generated. See both the general description of and list of
policy/product codes below.


 

  2. Registeree's Name Character Set Canonicialization
  ====================================================
It is mandatory that regcodes be platform and character set
independent despite the significant complexity this adds. This allows
the regcodes to be generated on one platform to be used on another
platform even if the platforms use different character sets. For
example a user may use a web browser on a Windows machine to initially
register and obtain the code, and then enter the regcode in an
application running on a Mac or a Palm device.

Thus when the registeree's name is accepted as user input the
character set used must be known. That character set should be carried
with the name where ever it is stored or transfered. (Unicode/UTF-8 is
a likely alternative to this strategy in the future)

The functions in regcode_charsets_v3.[ch] do the folding and ignoring
and should always be used. The folding and ignoring are not done according
to any character set standard as none exists for this particular use. 

When adding support for new character sets to regcode_charsets_v3.c
overlap with existing character sets should be considered and the
characters in the new character set should be folded the same way as
they are mapped for other character sets.

Note that folding for the latin alphabets entails mostly reducing the
character to it's non-accented US-ASCII form. Thus a work around for
some users may be to enter their name without accented characters.

At this time regcodes are mostly useful for only Latin character
sets. Folding for latin characters is probably more important than
other languages because there are many overlapping character sets in
use for Latin characters. Cyrillic, Greek, Arabic, and Hebrew
languages are not handled. The specific mappings for character sets
are defined in the source for the character set folding. It is
required that all implementations for a particular character set use
the same mapping.

Japanese is not entirely supported, but the plan is to do minimal
folding. Perhaps only ingoring space. The character set supported will
be JIS. Once minimal mapping and folding is done, all in JIS, the JIS
will be fed in to the regcode_* functions. In particular the changes
in v3 were specifically for Japanese. They moved all character set
processing out of regcode and into regcode_charsets.
   
The registeree's name is divided into first and last name for user interface 
purposes. First and last names are concatenated for generation and 
verification. 




  3. Existing Polciy/Product Code Assignments
  ===========================================
  
This section of this file THE POLICY CODE ASSIGNMENT AUTHORITY.
Please update this any time a new product code is assigned, or a new
policy interpretation is applied to an existing product code.


Code = 1 -- pdQsuite 1.x
---------------------
This code is used for regcodes that were purchased to enable pdQsuite 1.0. It
is also used for the pdQsuite 1.0 self-registering installer. This code has
only been used with the v1 regcode scheme.


Code = 16 --  Connecvity kit 1
------------------------------
Unknown. Ask Subahshis Mohanty


Code = 17 --  Connecvity kit 2
------------------------------
Unknown. Ask Subahshis Mohanty


Code = 18 --  Connecvity kit 3
------------------------------
Unknown. Ask Subahshis Mohanty


Code = 32 -- Eudora sponsored mode
----------------------------------
Used in regcodes for ad-ware. (Note that registration for ad-ware is
not required, but user's can register anyway).

It is anticipated that as long as a user stays with ad-ware they'll
have this regcode forever. It's not tied to a Eudora release version.


Code = 33 -- Eudora light mode
------------------------------
Used in regcodes for light. (Note that registration for light is not
required, but user's can register anyway).

It is anticipated that as long as a user stays with light they'll have
this regcode forever. It's not tied to a Eudora release version.


Code = 34 -- Eudora paid mode
-----------------------------
This is where it gets interesting. These regcodes with this policy are 
issued:
 - The user bought a reg code to upgrade to paid mode from ad mode
 - They owned 4.2 and used the updater to get to 4.3. In this case
   the month number is 255 as a substitute for what should have been
   a different policy code.
 - The user bought 4.3 in a box or ESD

The policy for this code is that the month number + 12 should be checked
against a value built into the application.  If the value built into the
application is less than or equal to this value, the regcode is valid.

Months beyond 20 are valid for no clients except Eudora 4.3.x.


Code = 35-40 -- Eudora 5.0 paid mode
------------------------------------
These codes are needed because we do not wish to allow 4.3 box/esd
purchasers to be able to generate regcodes that are valid for perpetuity.
The only way to obtain codes with this policy is to buy one from our
web site.  These codes MUST NOT be put in any generator given to users,
be it in an updater, esd, or box.

The policy for this code family is that the month number + 12 should be checked
against a value built into the application.  If the value built into the
application is less than or equal to this value, the regcode is valid.

Code = 41 -- Eudora 5.0 box/esd temp code
-----------------------------------------
This code is used by box/esd copies of Eudora in order to request a new code.
No client should accept this code as a valid code.


  4. Policy Implementations
  =========================

pdQsuite 1.0
------------
Accpets product code 1 and performs no checks on the month number. A license
is considered indefinite for pdQsuite 1.x (2.x may be different).

Note that this used version 1 regcodes. This will work correctly with ISO-8859-1
characters (a subset of Win 1252 and PalmOS), but NOT with the characters in
the Palm character set outside of ISO-8859-1. (S/Z Hacek and few others)

Also the regcode generator for the pdQsuite 1.0 purchase page was at
one time ignoring rather than folding accented characters (and still
may be).


Win Eudora 4.3.0 Downloader
---------------------------
Accects product codes 32, 33 and 34. The month number must be greater
than 14, the build month. This was an error due to a misunderstanding
of how the month numbers work. 

Improperly performs month number check on codes 32 and 33; the month
is checked but should not be.


Incorrectly performs month number check on code 34; fails to add 11 to the
month code before performing comparison.

Character set canonicalization is incorrect for characters in the Windows
characters that are outide ISO-8859-1. 


Mac Eudora 4.3.1 Downloader
---------------------------
(4.3.0 was an aborted release)
Accepts product codes 32, 33, and 34. Does not check the
month number at all.


Improperly fails to perform month number check on code 34, but this is
irrelevant as the check will always succeed anyway.

Mac Eudora 4.3.1 ignores all non-US-ASCII characters. This will cause it to fail
registration for any one with such characters in their name, though it was 
succeeding prior to early March when the reg code generation web page had
a complementary bug.


Win Eudora 4.3.0 updater
------------------------
Regcode accepting/checking is the same as Win Eudora 4.3.0 downloader.

* This installer generates regcodes * This updater looks for the
presence of Eudora 4.2 and if found generates a reg code. The product
number is 34 and the month number is always 255. This is an
overloading of the month number that should have been done with a
different policy number, the separate number was not assigned when the
4.3 non-updater version shipped. It is anticipated that Eudora 5.0
will not be freely available to those that upgraded from 4.2, thus
Eudora 5.0 will have to not run in paid mode for month number 255,
product code 34.

The updater also uses the old PSMapString character set canonicalizer and thus
has trouble with characters in Windows 1252 but outside ISO8859-1.


Mac Eudora 4.3.1 updater
------------------------
Regcode accepting/check is the same as Mac Eudora 4.3.1 downloader.

* This installer generates regcodes *
As with Windows 4.3.0 updater, this uses product code 34 and month
number 255 when generating a reg code.

Older versions used PSMapString for the user name which is incorrect.
Newer versions use the character folding in regcode_charsets, and put
the mapped username into the user's name in the application.  This is
necessary to avoid the character set bug in Mac Eudora 4.3.1.


Win Eudora 4.3.0 CD/Box installer
---------------------------------
Regcode accepting/checking is the same as Win Eudora 4.3.0 Downloader.

* This installer generates regcodes *
While not shipping when this was written the plan is to have the month
number be the month the installer runs in. The product/policy code should be 34.

This will probably use PSMapString and thus not deal with the full
windows code page.


Mac Eudora 4.3.1 CD/Box installer
--------------------------------- 
The installer uses the current month and policy code 34. 

* This installer generates regcodes *
This uses Mac Eudora 4.3.1 which incorrectly ignores non-us characters!

For character set canonicalization, it folds accented characters
before generating the regcode more or less like it should. The name
WITHOUT the accented characters is now considered the name the user
has registered under.  All is well unless the user manually types
their name in. It is expected this won't happen often.

Win Eudora 4.3.1 Updater
------------------------
Not released yet
Operates same way as Win Eudora 4.3.0 Updater.

Win Eudora 4.3.1 Downloader
------------------------
Same issues as Win Eudora 4.3.0 Downloader

Win Eudora 4.3.2 Downloader
---------------------------
Not released yet, but it should fix a few things...
- get character set canonicalization correct
- accept a few more "paid mode" policy codes yet to be determined
- not check the month for any non-paid mode policy codes 

Mac Eudora 4.3.2 Downloader
---------------------------
Not released yet, but should fix a few things which are the same as
the items for the Win Eudora 4.3.2 Downloader.

Mac Eudora 4.3.2 Updater
---------------------------
No released yet; same issues as Win Eudora 4.3.2 Updater


Win Eudora 4.3.2 Installer
--------------------------
No released yet.


Mac Eudora 4.3.2 Installer
--------------------------
Not released yet.




 5. Creating New Policy/Product Codes
 ====================================
 
This scheme is intended to be flexible and handle many different product 
registration and licensing policies. 

For example it is possible for a regcode to be valid for a limited amount of
time to implement a subscription policy with annual renewel. Standard 
indefinite licenses are possible, and the ability to have a previous version
license be valid for an upgrade to the current version if purchased small
number of months before the current version is released.

Policy MUST be enforced by checks of the product/policy code and issuing month
number in the application. Policy is likely to vary with different versions so
Eudora 5.X is likely to have different checks than Eudora 4.X. Putting the checks
in the application allows the policy to be determined when a new version of
the application ships, rather than when the regcode is issued.

However it is important to come up with enough different product/policy codes
for a particular application to anticipate the number of policies that will
be needed. Generally multiple codes for a single product are needed when there
are different ways of registering the product. In the Eudora case this might
include buying a regcode, buying a box, and a free upgrade from the previous
version. 

It is probably wise to assign a different product code for every different
means of getting a regcode even if they are all treated the same by current
versions of the application. This will allow flexibility for creating different
policies for the product later on when the next version ships.




 6. Executive Overview
 =====================
 
This scheme entails a 16 digit registration code that is constructed out of 
the registeree's name, the month the code was generated, and product code. When
a regcode is verified it's validity is checked and the month and product 
code are returned. An individual application or a particular version of
an application can and should have additionaly validity policy based on the
month and product codes.

The reg codes also contain some random data and some consistency checks. These
are to make the scheme harder to reverse.

These reg codes are significantly different from other reg schemes because
they require the user's name to be constructed. Thus it is not possible to 
create a list of regcodes to be handed out one by one. The customer's name
must be know before one can be generated. The reason for this is to discourage
sharing of reg codes. For example you would have to post both your name and
your reg code to a web site.

Reg codes are not gauranteed to be unique, but are very likely to be unique. 
There are 4 million possible codes for a given month and product. The probability
of a duplicate is 1/100 for 40,000 regcodes generated/month for a product. This
makes them useful for a customer ID, e.g., for technical support. A reg code
can also be partially verified if the user's name is not known.

The scheme does not need a data base or single storage point for coordination 
of regcodes. This was a design goal that comes from problems with coordination
of sequential regcodes in the past.




 7. Executive Overview
 =====================

V3 of regcode and regcode_charsets were created to move all character
set processing out of regcode and into regcode_charsets. The main
reason this was made a new version was so that they would be used
together. Don't mix and match the regcode.* and regcode_charset.*
versions. Other than that the syntax and semantics of the API have not
changed.


