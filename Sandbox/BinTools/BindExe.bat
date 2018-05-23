@echo off
cd %1%
echo ...............................................
echo .                Post Build BIND              .
echo ...............................................
if exist %windir%\system32 goto DoWinNT
goto DoWin95
:DoWin95
..\..\BinTools\bind -p .;%windir%;%windir%\system -u -v Eudora.exe DirServ.dll EudoraBk.dll Isock.dll Ldap.dll Paige32.dll Ph.dll SPELL32.DLL Eudora32.dll EuMemMgr.dll QCUtils.dll imap.dll QCSocket.dll
goto done
:DoWinNT
..\..\BinTools\bind -p .;%windir%;%windir%\system32 -u -v Eudora.exe DirServ.dll EudoraBk.dll Isock.dll Ldap.dll Paige32.dll Ph.dll SPELL32.DLL Eudora32.dll EuMemMgr.dll QCUtils.dll imap.dll QCSocket.dll
goto done
:done
echo ...............................................
echo .                Post Build BIND              .
echo ...............................................

echo ...............................................
echo .                Post Build STAMP              .
echo ...............................................
echo .
echo . STAMPING %1%\Eudora.exe with 2000 and 34
echo .
rem echo . Actually, the stamping is a stub now since
rem echo . #ifdef EXPIRING code is broken. Once that
rem echo . is fixed, we will go ahead and stamp the
rem echo . executable.
rem echo .
..\..\BinTools\stamper Eudora.exe 2000 34
echo ...............................................
echo .                Post Build STAMP              .
echo ...............................................
