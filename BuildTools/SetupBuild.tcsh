#.............................................................................#
# This script is intended to setup a build for a particular product. Based
# on what build was built before, it sets up the next build. For instance,
# if the previous build was 4.2.0.1, it will build 4.2.0.2. If this is the
# first time this script is being used, the build information will be obtained
# from the user.
#
# This script takes one argument that indicates what product to build. For
# instance, Eudora, Eudora41, or Eudora42, or Eudora421
#
# Usage: tcsh SetupBuild Product
#
# author: Subhashis Mohanty
#.............................................................................#

setenv P4USER ebuilder

#.............................................................................#
# Exit code
# 0 --- success
# 1 --- no rm found
# 2 --- no awk found
# 3 --- one or more tools are missing
# 4 --- wrong arglist
# 5 --- wrong target
# 6 --- did not choose to remove old targets

#.............................................................................#
@ ExitCntr    = 1
set ExitArray = (1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21)

#.............................................................................#
# Some variables we will need for functions defined next.
#.............................................................................#
@ LF_CNTR = 0
set LOGFILE_TMP = $$
set LOGFILE
# What are we building
set PRODUCT
# Build components
@ MAJOR  = 0
@ MINOR  = 0
@ AUX    = 0
@ BLDNUM = 0
# OLDBLDNUM=MAJOR.MINOR.AUX.BLDNUM
set PRODUCTARG
set INTERACTIVE = "FALSE"
set RELEASE = "Beta"
set LABELSEED = "Eudora421Seed"
set EUDORABINDIR
set BUILDERID
set OLDBLDNUM
set THEBUILD
set BLDLOC
set BLDLOC2
set SEDSCRIPT
set P4CLIENT
set CHNGS
@ CHNGN = 0
@ OLDCHNGN = 0
#@ BLDPAIGE = 0

#.............................................................................#
# Set aliases
#.............................................................................#
source SetupBuild.aliases

#.............................................................................#
# Make sure that all the tools we need are available, and bail if not.
#.............................................................................#
# The most basic one we need is rm and awk.
which rm >  $LOGFILE_TMP.IsRm
if ($? == 1) then
    # rm is indeed absent. bail!
    echo "No rm found. Without rm, I am pretty much useless. Exiting"
    Exit $ExitArray[$ExitCntr]
endif
@ ExitCntr++
# Ok, rm is present!
which awk > $LOGFILE_TMP.IsAwk
if ($? == 1) then
    # awk is indeed absent. bail!
    echo "No awk found. Without awk, I am pretty much useless. Exiting"
    RemoveTempsAndExit $ExitArray[$ExitCntr]
endif
@ ExitCntr++

#............................................................................#
# We have rm and awk, see if we have the rest!
#............................................................................# 
SetupNewEchoSection
# To determine if a tool is available, add it to the Tools array, and
# increase the size of the ToolsAvailable array by adding a 0.
set Tools = (cat basename pwd grep wc date p4 sed mv xargs msdev whoami mkdir head)
set ToolsAvailable = (0 0 0 0 0 0 0 0 0 0 0 0 0 0)
@ Tools_Cntr = 0
@ NumToolsMissing = 0

foreach tool ($Tools[*])
    @ Tools_Cntr++
    EchoThisPrettyBoth "Checking if $Tools[$Tools_Cntr] is available"
    @ ToolsAvailable[$Tools_Cntr] = `which $tool | awk -f Is2ArgCommand.awk`
    if ($ToolsAvailable[$Tools_Cntr] == 1) then
	EchoThisPrettyBoth "Could not find $tool in your path."
	@ NumToolsMissing++
    else
	EchoThisPrettyBoth "Found `which $tool` in your path."
    endif
 end

if ($NumToolsMissing > 0) then
    if ($NumToolsMissing > 1) then
	EchoThisErrorBoth "$NumToolsMissing tools not found. Exiting"
    else
	EchoThisErrorBoth "$NumToolsMissing tool not found. Exiting"
    endif
    RemoveTempsAndExit $ExitArray[$ExitCntr]
else
    EchoThisPrettyBoth "All tools found"
endif

@ ExitCntr++

#.............................................................................#
# All tools were found. We can safely start logging to the log files now.
#.............................................................................#

#.............................................................................#
# Special case perforce port for author. He always accesses perforce through
# a firewalled ssh port-forwarded port.
#.............................................................................#
set BUILDERID = `whoami`
if ($BUILDERID == "smohanty") then
    setenv P4PORT "172.16.1.1:7005"
else
    setenv P4PORT "gecko:1666"
endif
setenv P4USER "ebuilder"

# echo "P4PORT is $P4PORT"
# echo "Obtaining Port information"
# p4 info
# p4 files //depot/main/BuildTools/...
# exit

#.............................................................................#
# Construct the name of the file that will log the actions in this
# script.
#.............................................................................#
SetupNewEchoSection
set BASEPWD = `pwd`
EchoThisPrettyBoth "Constructing Logfile name"
set THEDATE = `date '+%B.%d.%y-%H.%M.%S%p'`
set LOGFILE = "$BASEPWD/$THEDATE"
EchoThisPrettyBoth "Current logfile is $LOGFILE.log"
PropagateRemoveTemps

#.............................................................................#
# Basename
#.............................................................................#
set script = `basename $0`

#.............................................................................#
# Argument Checking.
#.............................................................................#
EchoThisPrettyBothR "Checking the arguments"
EchoThisPrettyBothR "Number of Arguments is $#"
if ($# == 1 || $# == 2) then
    if ($# == 2) then
	if (("$1" != "-i") && ("$1" != "-a") && ("$1" != "-b") && ("$1" != "-r")) then
	    EchoThisErrorBothR "Usage: tcsh $script \[-i\|-a\|-b\|-r\] Product. Exiting"
	    Exit $ExitArray[$ExitCntr]
	else
            set PRODUCTARG = "$2"
            if ("$1" == "-i") then
	        set INTERACTIVE = "TRUE"
	    endif
	    if ("$1" == "-a") then
		set RELEASE = "Alpha"
	    endif
	    if ("$1" == "-b") then
		set RELEASE = "Beta"
	    endif
	    if ("$1" == "-r") then
		set RELEASE = "Release"
	    endif  
	endif
    else 
	set PRODUCTARG = "$1"
    endif
else
    EchoThisErrorBothR "Usage: tcsh $script \[-i\|-a\|-b\|-r\] Product. Exiting"
    Exit $ExitArray[$ExitCntr]
endif
@ ExitCntr++

#.............................................................................#
# Need to know what to build.
#.............................................................................#

#.............................................................................#
# Can only build specific product currently.
#.............................................................................#
switch ($PRODUCTARG)
    # case of Eudora
    case Eudora:
	set PRODUCT = "Eudora"
	breaksw
    # case of Eudora41
    case Eudora41:
	set PRODUCT = "Eudora41"
	breaksw
    # case of Eudora42
    case Eudora42:
	set PRODUCT = "Eudora42"
	breaksw
    # default case
    default:
	EchoThisErrorBothR "Do not know how to build $1. Exiting"
	Exit $ExitArray[$ExitCntr]
	breaksw
endsw
EchoThisPrettyBothR "Will build $PRODUCT"
@ ExitCntr++

#.............................................................................#
# For interactive builds query if old logfile should be deleted. Construct
# Build number.
#.............................................................................#
EchoThisPrettyBothR "Checking if buildfile exists"
set BUILDFILE = "$BASEPWD/$PRODUCT.Build"
EchoThisPrettyBothR "Buildfile is $BUILDFILE"
if (-f "$BUILDFILE") then
    EchoThisPrettyBothR "Will read existing settings from Buildfile $BUILDFILE"

    # Delete previous logfile after getting confirmation from user for
    # interactive builds.
    if ($INTERACTIVE == "TRUE") then
	set LF_OLD = `cat $BUILDFILE|awk -F "=" '$1==\"Logfile\" {print $2}'`
	if ($LF_OLD == "") then
	    EchoThisPrettyBothR "No record of logfile in $BUILDFILE"
	else
	    if (-f "$LF_OLD") then
		EchoThisPrettyBothR "Previous logfile $LF_OLD exists"
		EchoThisPrettyBothR "Delete old logfile $LF_OLD\? \[y\|n\]"
		set DelOLF = $<
		switch ($DelOLF)
		    # case of YES
		    case [yY][eE][sS]:
		    case [yY]:
			EchoThisPrettyR "User chose to delete previous logfile"
			rm -f $LF_OLD
			breaksw
		    # case of NO
		    case [nN][oO]:
		    case [nN]:
			EchoThisPrettyR "User chose to keep previous logfile"
			breaksw
		    # default case
		    default:
			EchoThisPrettyBothR "Unexpected $DelOLF. Ignoring"
			breaksw
		endsw
	    else
		EchoThisPrettyBothR "No previous logfile $LF_OLD in `pwd`"
	    endif
	endif
    endif

    # Generate previous build information
    EchoThisPrettyBothR "Constructing previous build number components"
    # Major
    @ MAJOR  = `cat $BUILDFILE | awk -F "=" '$1==\"Major\" {print $2}'`
    # Minor
    @ MINOR  = `cat $BUILDFILE | awk -F "=" '$1==\"Minor\" {print $2}'`
    # Auxillary
    @ AUX    = `cat $BUILDFILE | awk -F "=" '$1==\"Auxillary\" {print $2}'`
    # Build number
    @ BLDNUM = `cat $BUILDFILE | awk -F "=" '$1==\"BuildNum\" {print $2}'`
    set OLDBLDNUM = $MAJOR.$MINOR.$AUX.$BLDNUM
    EchoThisPrettyBothR "Previous build number was $OLDBLDNUM"

    # Delete old perforce build client if it exists.
    EchoThisPrettyBothR "Verifying if previous client exists"
    set IsOldClient = `p4 clients | grep -c $OLDBLDNUM`

    switch ($IsOldClient)
	# case of 0
	case 0:
	    EchoThisPrettyBothR "Previous client does not exist"
	    breaksw
	default:
	    EchoThisPrettyBothR "Previous client exists"
	    EchoThisPrettyBothR "Reverting files and deleting client"
	    p4 -c $OLDBLDNUM opened | awk -F "#" '{print $1}' | xargs -n 1 p4 -c $OLDBLDNUM revert
	    set DelRSLT = `p4 -c $OLDBLDNUM client -d $OLDBLDNUM`
	    EchoThisPrettyBothR "$DelRSLT"
	    breaksw
    endsw

    # Increment the build number and inform user what will be built.
    @ BLDNUM++

    set THEBUILD = $MAJOR.$MINOR.$AUX.$BLDNUM
    EchoThisPrettyBothR "Incremented build number. Will build $THEBUILD"

    # Get the build location.
    set BLDLOC = `cat $BUILDFILE | awk -F "=" '$1==\"Location\" {print $2}'` 
    EchoThisPrettyBothR "The build location is $BLDLOC"
else
    EchoThisPrettyBothR "Buildfile $BUILDFILE does not exist"

    # Generate previous build information
    EchoThisPrettyBothR "Constructing build number components"

    # Major
    EchoThisPrettyBothR "What is the Major build number\?"
    @ MAJOR = $<
    EchoThisPrettyBothR "Build component Major set to $MAJOR"

    # Minor
    EchoThisPrettyBothR "What is the Minor build number\?"
    @ MINOR = $<
    EchoThisPrettyBothR "Build component Minor set to $MINOR"

    # Auxillary
    EchoThisPrettyBothR "What is the Auxillary build number\?"
    @ AUX = $<
    EchoThisPrettyBothR "Build component Auxillary set to $AUX"

    # Build number
    EchoThisPrettyBothR "What is the actual build number\?"
    @ BLDNUM = $<
    EchoThisPrettyBothR "Actual build number set to $BLDNUM"

    # The BUILD!
    set THEBUILD = $MAJOR.$MINOR.$AUX.$BLDNUM

    # Build location
    EchoThisPrettyBothR "Where do you wish to build\?"
    EchoThisPrettyBothR "Please specify F.Q.P.N., I do not know relative paths"
    EchoThisPrettyBothR "Also, use double backslashes to separate directories"
    EchoThisPrettyBothR "Example - To build in C\:\\Code, type C\:\\\\\\\\Code"

    set BLDLOC = $<
    EchoThisPrettyBothR "Build location set to $BLDLOC"
endif

EchoThisPrettyBothR "Will build $THEBUILD in $BLDLOC"

#.............................................................................#
# If the BuildLocation exists, blow it. If user does not want
# it blown, perhaps he is not quite ready. Bail!
#.............................................................................#
EchoThisPrettyBothR "Checking if build location $BLDLOC exists"
if (-d "$BLDLOC") then
    if ($INTERACTIVE == "TRUE") then
	@ LoopCondition = 0
	EchoThisPrettyBothR "$BLDLOC exists. Ok to blow it away\? \[y\|n\|q\]"
	while ($LoopCondition == 0)
	    set BlowBldLoc = $<
	    switch ($BlowBldLoc)
		# case of YES
		case [yY][eE][sS]:
		case [yY]:
		    EchoThisPrettyR "User chose to blow away $BLDLOC"
		    EchoThisPrettyBothR "Blowing away $BLDLOC"
		    rm -rf $BLDLOC
		    @ LoopCondition++
		    breaksw
		# case of NO
		case [nN][oO]:
		case [nN]:
		    EchoThisPrettyR "User chose to not blow away $BLDLOC"
		    EchoThisPrettyBothR "Blow away old stuff first. Exiting"
		    Exit $ExitArray[$ExitCntr]
		    breaksw
		case [qQ]:
		    Exit $ExitArray[$ExitCntr]
		    breaksw
		# default case
		default:
		    EchoThisPrettyBothR "Unexpected $BlowBldLoc. Try again"
		    EchoThisPrettyBothR "$BLDLOC exists. Ok to blow it away\?"
		    EchoThisPrettyBothR "y for yes, n for no and q for quit"
		    breaksw
	    endsw
	end
    else 
	rm -rf $BLDLOC
    endif
else
    EchoThisPrettyBothR "Build Location $BLDLOC does not exist. Ignored"
endif
@ ExitCntr++

#.............................................................................#
# Update the build file.
#.............................................................................#
EchoThisPrettyBothR "Updating Build file"
echo "Major=$MAJOR"         >  $BUILDFILE
echo "Minor=$MINOR"         >> $BUILDFILE
echo "Auxillary=$AUX"       >> $BUILDFILE
echo "BuildNum=$BLDNUM"     >> $BUILDFILE
echo "Location=$BLDLOC"     >> $BUILDFILE
echo "Logfile=$LOGFILE.log" >> $BUILDFILE

#.............................................................................#
# Prepare the SedScript.
#.............................................................................#
EchoThisPrettyBothR "Preparing sed script for client creation"
set SEDSCRIPT = $BUILDFILE.SedScript
EchoThisPrettyBothR "The script is $SEDSCRIPT"
echo "/\/\/depot/{"                               >  $SEDSCRIPT
echo "s/.*/\ \ \ \ \ \ \ \ \/\/depot\/main\/$PRODUCT\/\.\.\. \/\/$THEBUILD\/$PRODUCT\/\.\.\./"                                   >> $SEDSCRIPT
echo "a\\"                                        >> $SEDSCRIPT
echo "\ \ \ \ \ \ \ \ \/\/depot\/main\/PDB\/PostBuildInfo\/$PRODUCT\/\.\.\. \/\/$THEBUILD\/PDB\/PostBuildInfo\/$PRODUCT\/\.\.\." >> $SEDSCRIPT
echo "a\\"                                        >> $SEDSCRIPT
echo "\ \ \ \ \ \ \ \ \/\/depot\/main\/BuildLogs\/$PRODUCT\/\.\.\. \/\/$THEBUILD\/BuildLogs\/$PRODUCT\/\.\.\."                   >> $SEDSCRIPT
echo "}"                                          >> $SEDSCRIPT
echo "/^Root:/{"                                  >> $SEDSCRIPT
echo "h"                                          >> $SEDSCRIPT
echo "s/.*/Root: $BLDLOC/"                        >> $SEDSCRIPT
echo "}"                                          >> $SEDSCRIPT
EchoThisPrettyR "The sed script contents are"
echo "#.............................................................................#"
echo "#.............................................................................#"
cat $SEDSCRIPT
echo "#.............................................................................#"
echo "#.............................................................................#"
cat $SEDSCRIPT >>$LOGFILE.log

#.............................................................................#
# Modifying sed script.
#.............................................................................#
EchoThisPrettyBothR "Modifying sed script with sed. sic!"
sed -f ModifySedScript $SEDSCRIPT > Temp.SedScript
mv Temp.SedScript $SEDSCRIPT
EchoThisPrettyR "The modified sed script contents are"
echo "#.............................................................................#"
echo "#.............................................................................#"
cat $SEDSCRIPT
echo "#.............................................................................#"
echo "#.............................................................................#"
cat $SEDSCRIPT >>$LOGFILE.log

#.............................................................................#
# Creating client
#.............................................................................#
EchoThisPrettyBothR "Creating client"
p4 -c $THEBUILD client -o | sed -f $SEDSCRIPT | p4 -c $THEBUILD client -i >>$LOGFILE.log
EchoThisPrettyBothR "This is how our p4 client looks"
p4 -c $THEBUILD client -o >>$LOGFILE.log
p4 -c $THEBUILD client -o
setenv P4CLIENT "$THEBUILD"

#.............................................................................#
# Syncing
#.............................................................................#
JustEchoBothR
p4 -c $THEBUILD info >>$LOGFILE.log
JustEchoBothR
EchoThisPrettyBothR "Syncing"

p4 -c $THEBUILD sync >> $LOGFILE.log

#.............................................................................#
# Checking out build files.
#.............................................................................#
EchoThisPrettyBothR "Checking out build files"
set CHNGS = "Checkout.SedScript"
echo "/enter\ description\ here/{" > $CHNGS
echo "s/.*/Build-$THEBUILD/" >>$CHNGS
echo "}" >>$CHNGS
EchoThisPrettyBothR "Creating new change number"
p4 -c $THEBUILD change -o | sed s/\<enter\ description\ here\>/"Build-$THEBUILD"/ | p4 -c $THEBUILD change -i >$LOGFILE_TMP.Change
@ CHNGN = `cat $LOGFILE_TMP.Change | awk '{print $2}'`
rm -f $LOGFILE_TMP.Change
EchoThisPrettyBothR "Created new change number $CHNGN"

#.............................................................................#
# Update the build file.
#.............................................................................#
EchoThisPrettyBothR "Updating Build file"
echo "ChangeNumber=$CHNGN" >>$BUILDFILE

set BLDLOC2 = `echo $BLDLOC | sed 's/\\/\//g'`
EchoThisPrettyBothR "Relative Build Location is $BLDLOC2"

#.............................................................................#
# Check out the files.
#.............................................................................#
EchoThisPrettyBothR "Checking out the relevant files"

#.............................................................................#
# The version files
#.............................................................................#
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/Version.h   >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/Version.txt >>$LOGFILE.log

#.............................................................................#
# The normal Eudora files.
#.............................................................................#
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/EuMemMgr.map    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/EuMemMgr.pdb    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Eudora.map    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Eudora.pdb    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Eudora32.map    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Eudora32.pdb    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/QCSocket.map    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/QCSocket.pdb    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/QCUtils.map    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/QCUtils.pdb    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/imap.map    >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/imap.pdb    >>$LOGFILE.log


#.............................................................................#
# The Buildlog files.
#.............................................................................#
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/BuildLogs/$PRODUCT/BuildScriptOutput.txt >>$LOGFILE.log
p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/BuildLogs/$PRODUCT/EudoraExe.txt >>$LOGFILE.log

#.............................................................................#
# Paige if necessary.
#.............................................................................#
@ OLDCHNGN = `p4 changes | awk '$6==\"'$P4USER@$OLDBLDNUM'\" && ($7==\"'\'Build-$OLDBLDNUM'\" || $8==\"'Build-$OLDBLDNUM'\") {print $2}'`
EchoThisPrettyBothR "The change number associated with the last build, $OLDBLDNUM, is $OLDCHNGN"

p4 files //depot/main/$PRODUCT/PaigeDLL/... | awk '$5>'$OLDCHNGN' {print $1}' >$LOGFILE_TMP.Paige
set BLDPAIGE = `cat $LOGFILE_TMP.Paige | wc -l`

EchoThisPrettyBothR "The number of files that have changed in Paige since $OLDCHNGN are $BLDPAIGE"

switch ($BLDPAIGE)
    # case of 0
    case 0:
	EchoThisPrettyBothR "So, Paige will not be built."
	echo "[Paige]" >Build.ini
	echo "ShouldBuild=False" >>Build.ini
	breaksw
    default:
	p4 files //depot/main/$PRODUCT/PaigeDLL/... | awk '$5>'$OLDCHNGN' {print $1}' >>$LOGFILE.log
	EchoThisPrettyBothR "So, Paige will be built"
	echo "[Paige]" >Build.ini
	echo "ShouldBuild=True" >>Build.ini
	p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/Bin/Release/Paige32.dll >>$LOGFILE.log
	p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/Lib/Release/Paige32.lib >>$LOGFILE.log
	p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Paige32.map    >>$LOGFILE.log
	p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Paige32.pdb    >>$LOGFILE.log
	breaksw
endsw

#.............................................................................#
# What type of build is it? (alpha, beta, release)
# The release build checks out some extra files.
#.............................................................................#
if ("$RELEASE" == "Release") then
    # The splash screen
    p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/Eudora/res/eudbyqc.bmp >>$LOGFILE.log
    # The .dsp's
    p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/Eudora/EudoraExe.dsp >>$LOGFILE.log
    p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/EuImap/EuImap.dsp >>$LOGFILE.log
    # The bind and stamper file.
    p4 -c $THEBUILD edit -c $CHNGN $BLDLOC2/$PRODUCT/BinTools/BindExe.bat >>$LOGFILE.log   
endif

echo "The files opened for edit are:"
p4 -c $THEBUILD opened 

#.............................................................................#
# Making modifications to build files.
#.............................................................................#
EchoThisPrettyBothR "Modifying build files"
echo "// Version.h" >$BLDLOC2/$PRODUCT/Version.h
echo ""             >>$BLDLOC2/$PRODUCT/Version.h
echo "#ifndef _VERSION_H_" >>$BLDLOC2/$PRODUCT/Version.h
echo "#define _VERSION_H_" >>$BLDLOC2/$PRODUCT/Version.h
echo ""             >>$BLDLOC2/$PRODUCT/Version.h
echo "#define EUDORA_VERSION1 $MAJOR" >>$BLDLOC2/$PRODUCT/Version.h
echo "#define EUDORA_VERSION2 $MINOR" >>$BLDLOC2/$PRODUCT/Version.h
echo "#define EUDORA_VERSION3 $AUX" >>$BLDLOC2/$PRODUCT/Version.h
echo "#define EUDORA_VERSION4 $BLDNUM" >>$BLDLOC2/$PRODUCT/Version.h
echo "#define EUDORA_BUILD_NUMBER    $MAJOR,$MINOR,$AUX,$BLDNUM" >>$BLDLOC2/$PRODUCT/Version.h

switch ($RELEASE)
    # case of Release
    case Release:
	echo "#define EUDORA_BUILD_DESC      "\"Version $MAJOR.$MINOR.$AUX "\\"0\" >>$BLDLOC2/$PRODUCT/Version.h
	echo "#define EUDORA_BUILD_VERSION   "\"Version $MAJOR.$MINOR.$AUX.$BLDNUM\""" >>$BLDLOC2/$PRODUCT/Version.h
	breaksw
    default:
	echo "#define EUDORA_BUILD_DESC      "\"Version $MAJOR.$MINOR.$AUX.$BLDNUM \($RELEASE\)"\\"0\" >>$BLDLOC2/$PRODUCT/Version.h
	echo "#define EUDORA_BUILD_VERSION   "\"Version $MAJOR.$MINOR.$AUX.$BLDNUM\""" >>$BLDLOC2/$PRODUCT/Version.h
	breaksw
endsw


#.............................................................................#
#
# This is the original line which has been moved into the switch statement
# above
#
#.............................................................................#
#$echo "#define EUDORA_BUILD_VERSION   "\"Version $MAJOR.$MINOR.$AUX.$BLDNUM\""" >>$BLDLOC2/$PRODUCT/Version.h

echo "" >>$BLDLOC2/$PRODUCT/Version.h
echo "#endif // _VERSION_H_" >>$BLDLOC2/$PRODUCT/Version.h
echo "" >>$BLDLOC2/$PRODUCT/Version.h
EchoThisPrettyBothR "Version.h"
cat $BLDLOC2/$PRODUCT/Version.h
cat $BLDLOC2/$PRODUCT/Version.h >>$LOGFILE.log
echo "$BLDNUM*Version $MAJOR.$MINOR" >$BLDLOC2/$PRODUCT/Version.txt
EchoThisPrettyBothR "Version.txt"
cat $BLDLOC2/$PRODUCT/Version.txt
cat $BLDLOC2/$PRODUCT/Version.txt >>$LOGFILE.log

#.....................................................................#
# Set things up correctly for the Release builds
# (as opposed to alpha or beta builds)
#.....................................................................#
if ("$RELEASE" == "Release") then
    cat $BLDLOC2/$PRODUCT/Eudora/EudoraExe.dsp | sed s/"\/D EXPIRING=1 "// >EudoraExe.dsp.temp
    mv EudoraExe.dsp.temp $BLDLOC2/$PRODUCT/Eudora/EudoraExe.dsp
    cat $BLDLOC2/$PRODUCT/EuImap/EuImap.dsp | sed s/"\/D EXPIRING=1 "// >EuImap.dsp.temp
    mv EuImap.dsp.temp $BLDLOC2/$PRODUCT/EuImap/EuImap.dsp
    cp eudbyqc.bmp $BLDLOC2/$PRODUCT/Eudora/res
    cp BindExe.bat $BLDLOC2/$PRODUCT/BinTools
endif

#.....................................................................#
# Build Setup done. Fire up MSDEV.
#.....................................................................#
EchoThisPrettyBothR "Done setting up build"

#.....................................................................#
# Before going ahead, and doing the build get confirmation from
# the user.
#.....................................................................#
if ("$RELEASE" == "Release") then
    EchoThisPrettyBothR "This is the release build"
    EchoThisPrettyBothR "Do you really want to do this\? \[y\|n\]"
    set MakeRelease = $<
    switch ($MakeRelease)
    # case of YES
	case [yY][eE][sS]:
	case [yY]:
	    EchoThisPrettyBothR "Firing up MSDEV to build $PRODUCT"
	    msdev -execute AutoBuild
	    EchoThisPrettyBothR "Done Building required components"
	    breaksw
    # case of NO
	case [nN][oO]:
	case [nN]:
	    EchoThisPrettyBothR "User choose not to build release version"
	    EchoThisPrettyBothR "Exiting"
	    Exit $ExitArray[$ExitCntr]
	    breaksw
    # default case
	default:
	    EchoThisPrettyBothR "Do not understand input $MakeRelease"
	    EchoThisPrettyBothR "Exiting"
	    Exit $ExitArray[$ExitCntr]
	    breaksw
    endsw
else
    EchoThisPrettyBothR "Firing up MSDEV to build $PRODUCT"
    msdev -execute AutoBuild
    EchoThisPrettyBothR "Done Building required components"
endif
@ ExitCntr++

EchoThisPrettyBothR "Copying log files to final destination"
#.....................................................................#
# Copy the log and plg files to final destination to be checked into the depot.
#.....................................................................#
if (-d $BLDLOC2/BuildLogs) then
    if (-d $BLDLOC2/BuildLogs/$PRODUCT) then
	cp $LOGFILE.log $BLDLOC2/BuildLogs/$PRODUCT/BuildScriptOutput.txt
	cp $BLDLOC2/$PRODUCT/Eudora/EudoraExe.plg $BLDLOC2/BuildLogs/$PRODUCT/EudoraExe.txt
    else
	mkdir $BLDLOC2/BuildLogs/$PRODUCT
	cp $LOGFILE.log $BLDLOC2/BuildLogs/$PRODUCT/BuildScriptOutput.txt
	cp $BLDLOC2/$PRODUCT/Eudora/EudoraExe.plg $BLDLOC2/BuildLogs/$PRODUCT/EudoraExe.txt
    endif
else
    mkdir $BLDLOC2/BuildLogs
    mkdir $BLDLOC2/BuildLogs/$PRODUCT
    cp $LOGFILE.log $BLDLOC2/BuildLogs/$PRODUCT/BuildScriptOutput.txt
    cp $BLDLOC2/$PRODUCT/Eudora/EudoraExe.plg $BLDLOC2/BuildLogs/$PRODUCT/EudoraExe.txt
endif

EchoThisPrettyBothR "Copying PDB/MAP files to final destination"
#.....................................................................#
# Copy the PDB files that are created in PDB/PostBuildInfo/Eudora
# to PDB/PostBuildInfo/$PRODUCT, but obviously don't have to do it for
# Eudora.
#.....................................................................#
if ($PRODUCT != "Eudora") then
    if (-d $BLDLOC2/PDB/PostBuildInfo/Eudora) then
	cp $BLDLOC2/PDB/PostBuildInfo/Eudora/*.* $BLDLOC2/PDB/PostBuildInfo/$PRODUCT
    endif
endif

#.....................................................................#
# Submit the files
#.....................................................................#
if ($INTERACTIVE == TRUE) then
    EchoThisPrettyBothR "Do you wish to submit changelist $CHNGN\? \[y\|n\]"
    set SubmitCL = $<
    switch ($SubmitCL)
    # case of YES
	case [yY][eE][sS]:
	case [yY]:
	    EchoThisPrettyR "User chose to submit changelist"
	    p4 -c $THEBUILD submit -c $CHNGN
	    breaksw
    # case of NO
	case [nN][oO]:
	case [nN]:
	    EchoThisPrettyR "User chose to not submit changelist"
	    breaksw
    # default case
	default:
	    EchoThisPrettyBothR "Do not understand $SubmitCL. Not submitting"
	    breaksw
    endsw
else
    p4 -c $THEBUILD submit -c $CHNGN    
endif

#.....................................................................#
# Create the label, and labelsync
#.....................................................................#
if ($INTERACTIVE == "TRUE") then
    EchoThisPrettyBothR "Do you wish to label the depot\? \[y\|n\]"
    set LabelD = $<
    switch ($LabelD)
    # case of YES
	case [yY][eE][sS]:
	case [yY]:
	    EchoThisPrettyR "User chose to label depot"
	    EchoThisPrettyBothR "Creating Label Build-$THEBUILD"
	    # Create label template based on seed
	    p4 -c $THEBUILD label -o $LABELSEED | sed s/"$LABELSEED"/"Build-$THEBUILD"/ | p4 -c $THEBUILD label -i
	    p4 -c $THEBUILD label -o Build-$THEBUILD | sed s/"\<locked\>"/unlocked/ | p4 -c $THEBUILD label -i
	    # Labelsync
	    p4 -c $THEBUILD labelsync -l Build-$THEBUILD >> $LOGFILE.log
	    # Lock the label
	    p4 -c $THEBUILD label -o Build-$THEBUILD | sed s/"\<unlocked\>"/locked/ | p4 -c $THEBUILD label -i
	    breaksw
    # case of NO
	case [nN][oO]:
	case [nN]:
	    EchoThisPrettyR "User chose to not label depot"
	    breaksw
    # default case
	default:
	    EchoThisPrettyBothR "Do not understand $LabelD. Not labelling"
	    breaksw
    endsw
else
    EchoThisPrettyBothR "Creating Label Build-$THEBUILD"
    # Create label template based on seed
    p4 -c $THEBUILD label -o $LABELSEED | sed s/"$LABELSEED"/"Build-$THEBUILD"/ | p4 -c $THEBUILD label -i
    p4 -c $THEBUILD label -o Build-$THEBUILD | sed s/"\<locked\>"/unlocked/ | p4 -c $THEBUILD label -i
    # Labelsync
    p4 -c $THEBUILD labelsync -l Build-$THEBUILD >> $LOGFILE.log
    # Lock the label
    p4 -c $THEBUILD label -o Build-$THEBUILD | sed s/"\<unlocked\>"/locked/ | p4 -c $THEBUILD label -i
endif

EchoToScreen "Creating Delta Report"
tcsh DeltaReport $PRODUCT Build-$OLDBLDNUM Build-$THEBUILD >$OLDBLDNUM-$THEBUILD.txt

set EUDORABINDIR = "C:/Builds/Build-$THEBUILD"

if (-d $EUDORABINDIR) then
    rm -rf $EUDORABINDIR
endif

if (! -d "C:/Builds") then
    mkdir C:/Builds
endif

mkdir $EUDORABINDIR
mkdir $EUDORABINDIR/Bin
mkdir $EUDORABINDIR/Pdb
mkdir $EUDORABINDIR/Logs

EchoToScreen "Copying build output to final destination"
cp $BLDLOC2/$PRODUCT/Bin/Release/*.* $EUDORABINDIR/Bin

cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Debugger.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/DirServ.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Eudora.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Eudora32.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/EudoraBk.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/EuMemMgr.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/imap.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/ISock.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/LDAP.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/ota50r.pdb $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Ph.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/QCSocket.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/QCUtils.* $EUDORABINDIR/Pdb
cp $BLDLOC2/PDB/PostBuildInfo/$PRODUCT/Paige32.* $EUDORABINDIR/Pdb

cp $BLDLOC2/BuildLogs/$PRODUCT/*.* $EUDORABINDIR/Logs
cp $OLDBLDNUM-$THEBUILD.txt $EUDORABINDIR
cp $OLDBLDNUM-$THEBUILD.txt C:/Builds/DeltaReports

EchoToScreen "Dossifying build outputs"
tcsh dossify.tcsh $EUDORABINDIR/Logs/BuildScriptOutput.txt
tcsh dossify.tcsh $EUDORABINDIR/Logs/EudoraExe.txt

EchoToScreen "Pushing current dir. to stack"
pushd .

EchoToScreen "Changing Directory to $EUDORABINDIR"
cd $EUDORABINDIR

EchoToScreen "Creating PDB-MAP.tar file"
tar cvf PDB-MAP.tar PDB
EchoToScreen "gziping PDB-MAP.tar"
gzip -9 PDB-MAP.tar

EchoToScreen "Going up one directory level"
cd ..

EchoToScreen "Will Map \\\\swamp\\Eudora to X:"
EchoToScreen "Deleting drive X:"
EchoToScreen "If X: does not exist, you will see the innocous error:"
EchoToScreen "- The network connection could not be found -"
net use /delete X:
EchoToScreen "Mapping \\\\swamp\\Eudora to X:"
net use X: \\\\swamp\\Eudora arodue /user:builder

EchoToScreen "Mirroring build output on swamp"
cp -r Build-$THEBUILD X:/421/Builds

EchoToScreen "Popping dir. from stack"
popd

EchoToScreen "Copying delta report to common repository"
cp $OLDBLDNUM-$THEBUILD.txt X:/421/Builds/DeltaReports


EchoToScreen "Done"
