#!/usr/bin/sh

# compiler flags
compiler=-'I/usr/include/SDL2 -I/usr/include/lua5.3 -Wall -pedantic -std=c++2a'
linker='-lSDL2main -lSDL2 -lSDL2_image -llua5.3 -lreadline -limgui -lstb'

files='src/*.cpp src/*/*.cpp src/Gui/imgui/*.cpp'

# change this if needed
output_exe=emulator.exe

g++ $compiler $files $linker -O2 -o $output_exe
