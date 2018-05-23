echo off

rem --------------------------------------------
rem
rem            UNISYS.BAT
rem
rem
rem  This Objective Toolkitbatch file decodes the Unisys
rem  LZW files for you.
rem
rem  secgif.cpp and tiff\tif_lzw.cpp are the files and
rem  they live in lzwcode.zip.
rem
rem  You must obtain the password from Stingray Software
rem  to be able to use these files.
rem 

echo .
echo .
echo .
echo .
echo .
echo    Installing Objective Toolkit source code that contains
echo    Unisys patented LZW technology....
echo    Please be prepared to enter the password.
echo
echo	It's ok to see 'file/directory not found' warnings here.
echo .
cd image
copy secgif.cpp secgif.bak
echo .
erase secgif.cpp
echo .
cd tiff
echo .
copy tif_lzw.cpp tif_lzw.bak
echo .
erase tif_lzw.cpp
cd ..\..

pkunzip -s lzwcode.zip

copy secgif.cpp image
erase secgif.cpp

copy tif_lzw.cpp image\tiff
erase tif_lzw.cpp

echo .
echo .
echo .
echo Deleting all old non-LZW object files now....
echo .

del sec16d\tif_lzw.obj > nul
del sec16d\secgif.obj > nul
del sec16r\tif_lzw.obj > nul
del sec16r\secgif.obj > nul

del sec1116d\tif_lzw.obj > nul
del sec1116d\secgif.obj > nul
del sec1116r\tif_lzw.obj > nul
del sec1116r\secgif.obj > nul

del seca16d\tif_lzw.obj > nul
del seca16d\secgif.obj > nul
del seca16r\tif_lzw.obj > nul
del seca16r\secgif.obj > nul

del sec40d\tif_lzw.obj > nul
del sec40d\secgif.obj > nul
del sec40r\tif_lzw.obj > nul
del sec40r\secgif.obj > nul
 
del sec1140d\tif_lzw.obj > nul
del sec1140d\secgif.obj > nul
del sec1140r\tif_lzw.obj > nul
del sec1140r\secgif.obj > nul

del seca40d\tif_lzw.obj > nul
del seca40d\secgif.obj > nul
del seca40r\tif_lzw.obj > nul
del seca40r\secgif.obj > nul
echo It's ok to see 'file/directory not found' warnings here.
echo .
echo LZW code installed, verifying installation...
echo .
echo .
dir image\secgif.cpp
dir image\tiff\tif_lzw.cpp
echo .
echo .
echo You should see a listing for secgif.cpp and tif_lzw.cpp
echo If you do not, please contact Stingray Software Technical
echo Support.  Your original tiff\tif_lzw.cpp and secgif.cpp files
echo are saved with the .bak extension if there are problems.
echo .
echo You now need to rebuild your Objective Toolkit libraries following
echo the directions in the Objective Toolkit Getting Started Guide.
echo .

