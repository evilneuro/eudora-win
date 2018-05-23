@echo off
del Eudora.ini
cp Eudora.blk Eudora.ini
cd statio~1
del test*.*
cd..
cd Sigs
del test*.*
cd..
deltree EudPriv\*.*
del RCPdbase.txt
del nndbase.toc
del nndbase.txt
del test*.mbx
del test*.toc
del test2.fol
rmdir test2.fol
del filters.pce
pause