@echo off

pushd %~dp0%

:: compiler flags
set compiler=-I"/mingw32/include/SDL2" -I"/mingw32/include/lua5.3" -Wall -pedantic -std=c++2a
set linker=-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -llua5.3 -lreadline

set files=src/*.cpp src/*/*.cpp src/Gui/imgui/*.cpp

:: change this if needed
set output_exe=emulator.exe

echo cd "%cd%" > C:\msys64\tmp\temp.sh
echo g++ %compiler% %files% %linker% -O2 -o "%output_exe%" >> C:\msys64\tmp\temp.sh
echo if [ $? -ne 0 ]; then read -n 1 -s -r -p "Press any key to exit."; fi >> C:\msys64\tmp\temp.sh
c:\msys64\mingw64 /tmp/temp.sh

popd
