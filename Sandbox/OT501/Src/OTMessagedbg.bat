@echo off
if not exist OTMessageDisplayed.dbg goto message

@del OTMessageDisplayed.dbg
goto done

:message
@echo .
@echo .
@echo .
@echo --- Building OTA50D.LIB ---------
@echo  ** 32-bit, static OT, AFXDLL MFC, release build, VC++ 5.0 **
@echo .
@echo .
@echo .
@echo  ** If you wish to RebuildAll OT501, you may do so by    **
@echo  ** deleting the contents of Eudora\OT501\Src\OTA50D.    **
@echo  ** Since we don't expect to be building OT501 on a      **
@echo  ** routine basis, we ignore RebuildAll. However, if     **
@echo  ** any OT501 file changes, RebuildAll or Build will     **
@echo  ** compile the file and create a new OTA50D.lib.        **
@echo  **                                                      **
@echo  **                                  --- The HashMan     **
@echo .
@echo .
@echo .
@echo OTMessageDisplayed.dbg >OTMessageDisplayed.dbg
goto done

:done
