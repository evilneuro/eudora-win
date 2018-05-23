#!../../BinTools/tcsh.exe

if ( $?EudoraBuildVerbosity == 0 ) then
    @ EudoraBuildVerbosity = 1
endif

if ( $EudoraBuildVerbosity >= 2 ) then
echo .
echo "Running $0 on $1"
echo .
endif

setenv PATH "..\..\BinTools;${PATH}"

if ( $EudoraBuildVerbosity >= 3 ) then
echo Win95 command.com is broken. Using tcsh to do post-processing.
echo UNIX tools come to the rescue.
endif

# Ensure that the destination directories exist.
if ( $EudoraBuildVerbosity >= 2 ) then
echo "......................................................"
echo ".                                                    ."
echo ". Ensuring that destination directories exist.       ."
echo ".                                                    ."
echo "......................................................"
endif


if ( ! -d "../../Lib" ) then
    mkdir ../../Lib
endif
if ( ! -d "../../Lib/Release" ) then
    mkdir ../../Lib/Release
endif

# Copy the appropriate files.
if ( $EudoraBuildVerbosity >= 2 ) then
echo "......................................................"
echo ".                                                    ."
echo ". Copying                                            ."
echo ".                                                    ."
echo ".          Build/Release/$1.lib"
echo ".                                                    ."
echo ". to                                                 ."
echo ".          ../../Lib/Release                         ."
echo ".                                                    ."
echo "......................................................"
endif
if ( -f "Build/Release/$1.lib" ) then
    cp Build/Release/$1.lib ../../Lib/Release
else
    echo "%% ERROR: Source file Build/Release/$1.lib missing."
endif

