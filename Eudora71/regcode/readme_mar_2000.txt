Regcode char sets update, Mar 7 2000. 


WHAT'S BEEN DONE:
Х regcode_charsets.[ch] has been created to replace MapString.[ch] as the proper way to canonicalize registeree names before passing to generator/verifier. The source is checked into Perforce and supports most of the important character sets. 

Х The "regutil" command line utility has been created to replace "reg-gen" and "reg-check". Binaries for Solaris, Windows and Linux will soon be in //depot/regcode/release/... These should NOT be distributed though, they're just development tools and to support the registration web pages. 

Х //depot/regcode/readme.txt describes how things should work (except we haven't figured out policy codes/month #'s yet).

Х The public registration web pages were fixed around the 1st of March to fold accented characters rather than ignore them.


WHAT'S LEFT TO DO:
Х Release the next MacOS standard Eudora build which now correctly handles accented chars. 

Х Fix public registration web page so the email it sends has the proper character set (use HTML entities) and attachment encoding (b64).

Х Fix the MacOS updater, ESD, box, etc., installers that self register to use the new regcode_charsets mapping. They are currently mapping using PSMapString which does not work for the MacOS character set.

Х Fix the Win standard build to use regcode_charsets with the Windows 1252 code page so that characters like К are handled correctly.

Х Fix the Win updater, ESD, Box self-registering installers to use regcode_charsets with the Windows 1252 code page so that characters like К are handled correctly.

Х Finish the internal-use reg code generation web page as the way for tech support or others to generate ad hoc reg codes. 

Х Consider reissuing reg codes for 1200 people in the database that have accented characters in their names ignored.


TIPS:
Х If a user has trouble registering ask them to try their name (e.g., John Bнgbтуtа€) first without any accent marks (John Bigbootay) and then with the accented characters removed (John Bgbt). 

Х If this doesn't work (in particular if they're from central/eastern Europe) they should be re-registered using a name with non-accented characters (John Bigbootay).


WHAT'S UNRESOLVED:
Х It seems that browsers don't tell the server what character set is in use, thus if the user has switched their browser to something other than US-ASCII, ISO8859-1 or the Windows Character Set, we're likely to have a problem - see tips for work around.

Х If Eudora is running on Mac/Win outside of Western Europe or the Americas they may have trouble with accented characters - see tips. If this becomes a big problem we can address it.

Х We're totally hosed for non-Latin character sets like Japanese, Arabic and Cyrillic. If this becomes a big problem we can address it.

