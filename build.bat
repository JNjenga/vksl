@echo off
cd build

SET SRC_FILES=..\src\vksl.cpp
REM SET SRC_FILES=%SRC_FILES% ..\src\vksl.cpp
SET FLAGS=/Zi

cl.exe -MT -W4 -EHa- /Ot %FLAGS% %SRC_FILES% /link -subsystem:windows,5.1 User32.lib Gdi32.lib

if ERRORLEVEL 1 GOTO EXIT

call vksl.exe

EXIT:
cd ..
