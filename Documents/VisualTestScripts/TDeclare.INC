'-----------------------------------------------------------
' TDeclare.INC
' Header declarations for Template Project
' Richard J. Wartner
' 7610 North Via De Manana
' Scottsdale, Arizona 85258
' RWartner@aol.com
' July 1997
'-----------------------------------------------------------
' This is where you nominate your INC files
' to create a precompiled HEADER.
'
' The first time you compile a program that
' uses this header it will take all the files
' below and compile them into one file.
'
' The resulting header file can then be used
' by any other programs.
'
' If you modify any of the INC files the header file
' need only be rebuilt once.
'
' So lets say you have 30 programs that use this header
' and you make a change to one of the INC files.
' You must recompile all 30 programs to effect the change
' but the header will be rebuilt the first time.
' The remaining 29 compiles will be MUCH faster.
'-----------------------------------------------------------
'$HEADER START 'TDeclare.HDR'
'$INCLUDE 'C:\Program Files\Microsoft Visual Studio\Vt6\include\MSTEST.INC'
'$INCLUDE 'C:\vt\GenUtil.INC'
'$INCLUDE 'C:\vt\GenAPI.INC'
'$HEADER STOP
