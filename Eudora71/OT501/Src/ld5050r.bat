@echo off
attrib -R OT5050R.DEF
..\utils\dllxpt32 /win32 /o OT5050R.DEF OT5050R\*.obj
..\utils\prepdef OT5050R.DEF
@echo Linking...
link.exe %1
