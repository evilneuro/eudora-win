@echo off
attrib -R OT5050UD.DEF
..\utils\dllxpt32 /win32 /o OT5050UD.DEF OT5050UD\*.obj
..\utils\prepdef OT5050UD.DEF
@echo Linking...
link.exe %1
