@echo off
echo --- Cleaning %1
if exist %1\*.obj del %1\*.obj
if exist %1\*.lib del %1\*.lib
if exist %1\*.dll del %1\*.dll
if exist %1\*.pch del %1\*.pch
if exist %1\*.sbr del %1\*.sbr
if exist %1\*.bsc del %1\*.bsc
if exist %1\*.wsp del %1\*.wsp
if exist %1\*.vcp del %1\*.vcp
if exist %1\*.pdb del %1\*.pdb
if exist %1\*.exp del %1\*.exp
if exist %1\*.idb del %1\*.idb
if exist %1\*.res del %1\*.res
if exist %1\*.map del %1\*.map
if exist %1\*.vcw del %1\*.vcw


