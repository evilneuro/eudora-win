@echo off
del Eudora.ini
cp Eudora.box Eudora.ini
del test*.mbx
del test*.toc
del test2.fol
rmdir test2.fol
del filters.pce
pause