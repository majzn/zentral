@ECHO OFF
if not exist %1 mkdir %1
if not exist "%1/bin" mkdir "%1/bin"
if not exist "%1/obj" mkdir "%1/obj"
gcc -Wall -pedantic -ansi -O2 -c %1.c -o %1/obj/%1.o 
gcc -Wall -pedantic -ansi -O2 %1/obj/%1.o -o %1/bin/%1.exe -lopengl32 -lkernel32 -lshlwapi -lwinmm -lgdi32 -ldsound
ECHO Compiled.
"%1/bin/%1" soylib.h