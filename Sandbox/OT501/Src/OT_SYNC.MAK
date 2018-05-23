!IF "$(cfg)" == "Debug"

TGT=OTA50D.lib
PDB=OTA50D.pdb

BIN_DEBUG=..\..\Bin\Debug
LIB_DEBUG=..\..\Lib\Debug

all: $(LIB_DEBUG)\$(TGT)

$(LIB_DEBUG)\$(TGT): OTA50D\$(TGT)
#	@echo .
#	@echo .
#	@echo .
#	@echo "Copying OTA50D\$(TGT) to $(LIB_DEBUG)"
	@-copy OTA50D\$(TGT) $(LIB_DEBUG)
#	@echo "Copying OTA50D\$(PDB) to $(LIB_DEBUG)"
	@-copy OTA50D\$(PDB) $(LIB_DEBUG)
#	@-if exist $(BIN_DEBUG)\Eudora.exe del $(BIN_DEBUG)\Eudora.exe

!ELSE

TGT=OTA50R.lib

BIN_RELEASE=..\..\Bin\Release
LIB_RELEASE=..\..\Lib\Release

PDBR=OTA50R.pdb
PBI_PBS=..\..\..\PDB\PostBuildInfo\Eudora

all: $(LIB_RELEASE)\$(TGT)

$(LIB_RELEASE)\$(TGT): OTA50R\$(TGT)
#	@echo "Copying OTA50R\$(TGT) to $(LIB_RELEASE)"
	@-copy OTA50R\$(TGT) $(LIB_RELEASE)
	@-..\..\BinTools\tcsh CreateDirs.tcsh
#	@-if not exist "..\..\..\PDB" mkdir "..\..\..\PDB"
#	@-if not exist "..\..\..\PDB\PostBuildInfo" mkdir "..\..\..\PDB\PostBuildInfo"
#	@-if not exist "..\..\..\PDB\PostBuildInfo\Eudora" mkdir "..\..\..\PDB\PostBuildInfo\Eudora"
#	@echo "Copying OTA50R\$(PDBR) to $(PBI_PBS)"
	@-copy OTA50R\$(PDBR) $(PBI_PBS)
#	@-if exist $(BIN_RELEASE)\Eudora.exe del $(BIN_RELEASE)\Eudora.exe
#	@echo .
#	@echo .
#	@echo .

!ENDIF
