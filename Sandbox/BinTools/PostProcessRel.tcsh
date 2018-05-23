#!../BinTools/tcsh.exe

if ( $?EudoraBuildVerbosity == 0 ) then
    @ EudoraBuildVerbosity = 1
endif

if ( $EudoraBuildVerbosity >= 2 ) then
echo .
echo "Running $0 on $1"
echo .
endif

setenv PATH "..\BinTools;${PATH}"

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

if ( ! -d "../Lib" ) then
    mkdir ../Lib
endif
if ( ! -d "../Lib/Release" ) then
    mkdir ../Lib/Release
endif

if ( $1 == "NewMgr" || $1 == "EuImap" || $1 == "AccountWizard" ) then
    goto COPYING
endif

if ( ! -d "../Bin" ) then
    mkdir ../Bin
endif
if ( ! -d "../Bin/Release" ) then
    mkdir ../Bin/Release
endif
if ( ! -d "../../PDB" ) then
    mkdir ../../PDB
endif
if ( ! -d "../../PDB/PostBuildInfo" ) then
    mkdir ../../PDB/PostBuildInfo
endif
if ( ! -d "../../PDB/PostBuildInfo/Eudora" ) then
    mkdir ../../PDB/PostBuildInfo/Eudora
endif

if ( $1 == "Eudora" || $1 == "Debugger" ) then
    goto PDB_SECTION
endif

COPYING:
# Copy the appropriate files.
if ( $EudoraBuildVerbosity >= 2 ) then
echo "......................................................"
echo ".                                                    ."
echo ". Copying                                            ."
echo ".                                                    ."
echo ".          Build/Release/$1.lib"
echo ".                                                    ."
echo ". to                                                 ."
echo ".          ../Lib/Release                            ."
echo ".                                                    ."
echo "......................................................"
endif

if ( -f "Build/Release/$1.lib" ) then
    cp Build/Release/$1.lib ../Lib/Release
else
    echo "%% ERROR: Source file Build/Release/$1.lib missing."
endif

if ( $1 == "NewMgr" || $1 == "EuImap" || $1 == "AccountWizard" || $1 == "SearchEngine" ) then
    goto DONE
endif

if ( $1 == "Eudora" || $1 == "Debugger" ) then
    goto PDB_SECTION
endif

if ( $EudoraBuildVerbosity >= 2 ) then
echo "......................................................"
echo ".                                                    ."
echo ". Copying                                            ."
echo ".                                                    ."
echo ".          Build/Release/$1.dll"
echo ".                                                    ."
echo ". to                                                 ."
echo ".          ../Bin/Release                            ."
echo ".                                                    ."
echo "......................................................"
endif
if ( -f "Build/Release/$1.dll" ) then
    cp Build/Release/$1.dll ../Bin/Release
else
    echo "%% ERROR: Source file Build/Release/$1.dll missing."
endif
goto PDB_SECTION

PDB_SECTION:
if ( $EudoraBuildVerbosity >= 2 ) then
echo "......................................................"
echo ".                                                    ."
echo ". Copying                                            ."
echo ".                                                    ."
echo ".          Build/Release/$1.pdb"
echo ".                                                    ."
echo ". to                                                 ."
echo ".          ../../PDB/PostBuildInfo/Eudora            ."
echo ".                                                    ."
echo "......................................................"
endif
if ( -f "Build/Release/$1.pdb" ) then
    cp Build/Release/$1.pdb ../../PDB/PostBuildInfo/Eudora
else
    echo "%% ERROR: Source file Build/Release/$1.pdb missing."
endif

if ( $EudoraBuildVerbosity >= 2 ) then
echo "......................................................"
echo ".                                                    ."
echo ". Copying                                            ."
echo ".                                                    ."
echo ".          Build/Release/$1.map"
echo ".                                                    ."
echo ". to                                                 ."
echo ".          ../../PDB/PostBuildInfo/Eudora            ."
echo ".                                                    ."
echo "......................................................"
endif
if ( -f "Build/Release/$1.map" ) then
    cp Build/Release/$1.map ../../PDB/PostBuildInfo/Eudora
else
    echo "%% ERROR: Source file Build/Release/$1.map missing."
endif

goto DONE

DONE:


