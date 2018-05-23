@echo off
attrib -R OT5050UR.DEF
..\utils\dllxpt32 /win32 /o OT5050UR.DEF OT5050UR\*.obj
..\utils\prepdef OT5050UR.DEF
@echo Linking...
link.exe %1
