@echo off
attrib -R OT5050D.DEF
..\utils\dllxpt32 /win32 /o OT5050D.DEF OT5050D\*.obj
..\utils\prepdef OT5050D.DEF
@echo Linking...
link.exe %1
