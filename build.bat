@echo off
cd build

SET SRC_FILES=..\src\vksl.cpp
REM SET SRC_FILES=%SRC_FILES% ..\src\vksl.cpp

cl.exe /EHsc %SRC_FILES% User32.lib Gdi32.lib
vksl.exe

cd ..
