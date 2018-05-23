@echo off
if not exist OTMessageDisplayed.dbg goto message

@del OTMessageDisplayed.dbg
goto done

:message
@echo --- Building OTA50D.LIB ---------
@echo  ** 32-bit, static OT, AFXDLL MFC, release build, VC++ 5.0 **
@echo  ** If you wish to RebuildAll OT501, you may do so by    **
@echo  ** deleting the contents of Eudora\OT501\Src\OTA50D.    **
@echo OTMessageDisplayed.dbg >OTMessageDisplayed.dbg
goto done

:done
