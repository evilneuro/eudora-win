@echo off
del Eudora.ini
cp Eudora.sas Eudora.ini
cd statio~1
del test*.*
cd..
cd Sigs
del test*.*
cd..
del test*.mbx
del test*.toc
del test2.fol
rmdir test2.fol
del filters.pce
pause